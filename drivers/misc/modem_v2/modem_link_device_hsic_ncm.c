/* /linux/drivers/misc/modem_v2/modem_link_device_hsic_ncm.c
 *
 * Copyright (C) 2010 Google, Inc.
 * Copyright (C) 2010 Samsung Electronics.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/irq.h>
#include <linux/poll.h>
#include <linux/gpio.h>
#include <linux/if_arp.h>
#include <linux/usb.h>
#include <linux/usb/cdc.h>
#include <linux/pm_runtime.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/wakelock.h>
#include <linux/suspend.h>
#include <linux/version.h>

#include <linux/platform_data/modem_v2.h>
#include <linux/ratelimit.h>
#include <plat/usb-phy.h>
#include "modem_prj.h"
#include "modem_utils.h"
#include "modem_link_device_hsic_ncm.h"

#include "link_usb_cdc_ncm.h"

/* /sys/module/modem_link_device_hsic_ncm/parameters/... */
static int tx_qlen = 5;
module_param(tx_qlen, int, S_IRUGO | S_IWUSR | S_IWGRP);
MODULE_PARM_DESC(tx_qlen, "tx qlen");

enum bit_debug_flags {
	LINK_DEBUG_LOG_IPC_TX,
	LINK_DEBUG_LOG_IPC_RX,
	LINK_DEBUG_LOG_RFS_TX,
	LINK_DEBUG_LOG_RFS_RX,
	LINK_DEBUG_LOG_RAW_TX,
	LINK_DEBUG_LOG_RAW_RX,
	LINK_DEBUG_LOG_NCM_TX,
	LINK_DEBUG_LOG_NCM_RX,

	LINK_DEBUG_RECOVER_CPDUMP,
	LINK_DEBUG_RECOVER_RESET,
	LINK_DEBUG_RECOVER_PANIC,
	LINK_DEBUG_LOG_BOOT_TX,
	LINK_DEBUG_LOG_BOOT_RX,
};

enum pipe_data_index {
	PIPE_ACM_SIPC,
	PIPE_ACM_LOG,
	PIPE_ACM_RESV,
	PIPE_NCM_RMNET0,
	PIPE_NCM_RMNET1,
	PIPE_NCM_RMNET2,
	PIPE_NCM_RMNET3,
};

#define MAX_TX_ERROR 30
#define MAX_RX_ERROR 30
static char last_link_err[64];
static unsigned long dflags = (
	1 << LINK_DEBUG_LOG_IPC_TX | 1 << LINK_DEBUG_LOG_IPC_RX |
	1 << LINK_DEBUG_RECOVER_CPDUMP);
module_param(dflags, ulong, S_IRUGO | S_IWUSR | S_IWGRP);
MODULE_PARM_DESC(dflags, "link device debug flags");

#ifdef CONFIG_PM_RUNTIME
static int usb_get_rpm_status(struct device *dev)
{
	return dev->power.runtime_status;
}
#else
static inline int usb_get_rpm_status(struct device *dev) { return 0; }
#endif
static void usb_rx_complete(struct urb *urb);
static int usb_rx_submit(struct if_usb_devdata *pipe_data,
		struct urb *urb, gfp_t gfp_flags);

static inline void reprocess_skb(struct if_usb_devdata *pipe_data,
		struct sk_buff *skb)
{

	if (pipe_data->idx < pipe_data->usb_ld->max_acm_ch)
		dev_kfree_skb_any(skb);
	else
		skb_recycle(skb);
}

static inline struct sk_buff *prepare_skb(struct if_usb_devdata *pipe_data,
		struct urb *urb)
{
	struct sk_buff *skb;
	unsigned int alloc_size = pipe_data->rx_buf_setup;

	if (pipe_data->idx < pipe_data->usb_ld->max_acm_ch) {
retry_alloc:
		skb = alloc_skb(alloc_size, GFP_ATOMIC | GFP_DMA);
		if (unlikely(!skb)) {
			alloc_size = alloc_size / 2 - 256;
			if (alloc_size >= PAGE_SIZE - 512) {
				mif_err("re-try to alloc skb %d\n", alloc_size);
				goto retry_alloc;
			}
		}
		pipe_data->rx_buf_size = alloc_size;
	} else {
		skb = (struct sk_buff *)urb->context;
	}

	return skb;
}

static void logging_ipc_data(enum mif_log_id id, struct io_device *iod,
							struct sk_buff *skb)
{
	struct sipc5_link_hdr *hdr;
	struct modem_shared *msd = iod->msd;
	int format = iod->format;

	if (unlikely(!skb))
		return;

	hdr = (struct sipc5_link_hdr *)skb->data;
	if (hdr->cfg == 0x7f) { /* IPC 4.1 */
		switch (format) {
		case IPC_FMT:
			/* To do something */
			break;
		case IPC_RFS:
			/* To do something */
			break;
		case IPC_MULTI_RAW:
			/* To do something */
			break;
		case IPC_RAW_NCM:
			/* To do something */
			break;
		default:
			break;
		}
	} else if (sipc5_start_valid(hdr)) {
		switch (hdr->ch) {
		case SIPC5_CH_ID_FMT_0 ... SIPC5_CH_ID_FMT_9:
		case SIPC5_CH_ID_RFS_0 ... SIPC5_CH_ID_RFS_9:
			mif_ipc_log(id, msd, skb->data, skb->len);
			break;
		case SIPC_CH_ID_CS_VT_DATA ... SIPC_CH_ID_TRANSFER_SCREEN:
		case SIPC_CH_ID_BT_DUN ... SIPC_CH_ID_CPLOG2:
			/* To do something */
			break;
		case SIPC_CH_ID_PDP_0 ... SIPC_CH_ID_PDP_14:
			/* To do something */
			break;
		default:
			break;
		}
	}
}

static void pr_tx_skb_with_format(int format, struct sk_buff *skb)
{
	struct sipc5_link_hdr *hdr;

	if (unlikely(!skb))
		return;

	if (format == IPC_RAW_NCM) {
		if (test_bit(LINK_DEBUG_LOG_NCM_TX, &dflags))
			pr_skb("NCM-TX", skb);
		return;
	} else if (format == IPC_BOOT) {
		if (test_bit(LINK_DEBUG_LOG_BOOT_TX, &dflags))
			pr_skb("BOOT-TX", skb);
		return;
	}

	hdr = (struct sipc5_link_hdr *)skb->data;
	if (hdr->cfg == 0x7f) { /* IPC 4.1 */
		switch (format) {
		case IPC_FMT:
			if (test_bit(LINK_DEBUG_LOG_IPC_TX, &dflags))
				pr_skb("IPC-TX", skb);
			break;
		case IPC_RFS:
			if (test_bit(LINK_DEBUG_LOG_RFS_TX, &dflags))
				pr_skb("RFS-TX", skb);
			break;
		case IPC_MULTI_RAW:
			if (test_bit(LINK_DEBUG_LOG_RAW_TX, &dflags))
				pr_skb("RAW-TX", skb);
			break;
		default:
			break;
		}
	} else if (sipc5_start_valid(hdr)) {
		switch (hdr->ch) {
		case SIPC5_CH_ID_FMT_0 ... SIPC5_CH_ID_FMT_9:
			if (test_bit(LINK_DEBUG_LOG_IPC_RX, &dflags))
				pr_skb("IPC-TX", skb);
			break;
		case SIPC5_CH_ID_RFS_0 ... SIPC5_CH_ID_RFS_9:
			if (test_bit(LINK_DEBUG_LOG_RFS_RX, &dflags))
				pr_skb("RFS-TX", skb);
			break;
		case SIPC_CH_ID_CS_VT_DATA ... SIPC_CH_ID_TRANSFER_SCREEN:
		case SIPC_CH_ID_BT_DUN ... SIPC_CH_ID_CPLOG2:
		case SIPC_CH_ID_PDP_0 ... SIPC_CH_ID_PDP_14:
			if (test_bit(LINK_DEBUG_LOG_RAW_RX, &dflags))
				pr_skb("RAW-TX", skb);
			break;
		default:
			break;
		}
	}
}

static void pr_rx_skb_with_format(int format, struct sk_buff *skb)
{
	struct sipc5_link_hdr *hdr;

	if (unlikely(!skb))
		return;

	if (format == IPC_RAW_NCM) {
		if (test_bit(LINK_DEBUG_LOG_NCM_RX, &dflags))
			pr_skb("NCM-RX", skb);
		return;
	} else if (format == IPC_BOOT) {
		if (test_bit(LINK_DEBUG_LOG_BOOT_RX, &dflags))
			pr_skb("BOOT-RX", skb);
		return;
	}

	hdr = (struct sipc5_link_hdr *)skb->data;
	if (hdr->cfg == 0x7f) { /* IPC 4.1 */
		switch (format) {
		case IPC_FMT:
			if (test_bit(LINK_DEBUG_LOG_IPC_RX, &dflags))
				pr_skb("IPC-RX", skb);
			break;
		case IPC_RFS:
			if (test_bit(LINK_DEBUG_LOG_RFS_RX, &dflags))
				pr_skb("RFS-RX", skb);
			break;
		case IPC_MULTI_RAW:
			if (test_bit(LINK_DEBUG_LOG_RAW_RX, &dflags))
				pr_skb("RAW-RX", skb);
			break;
		default:
			break;
		}
	} else if (sipc5_start_valid(hdr)) {
		switch (hdr->ch) {
		case SIPC5_CH_ID_FMT_0 ... SIPC5_CH_ID_FMT_9:
			if (test_bit(LINK_DEBUG_LOG_IPC_RX, &dflags))
				pr_skb("IPC-RX", skb);
			break;
		case SIPC5_CH_ID_RFS_0 ... SIPC5_CH_ID_RFS_9:
			if (test_bit(LINK_DEBUG_LOG_RFS_RX, &dflags))
				pr_skb("RFS-RX", skb);
			break;
		case SIPC_CH_ID_CS_VT_DATA ... SIPC_CH_ID_TRANSFER_SCREEN:
		case SIPC_CH_ID_BT_DUN ... SIPC_CH_ID_CPLOG2:
		case SIPC_CH_ID_PDP_0 ... SIPC_CH_ID_PDP_14:
			if (test_bit(LINK_DEBUG_LOG_RAW_RX, &dflags))
				pr_skb("RAW-RX", skb);
			break;
		default:
			break;
		}
	}
}

static int submit_anchored_urbs(struct if_usb_devdata *pipe_data)
{
	struct urb *urb;
	int ret;

	while ((urb = usb_get_from_anchor(&pipe_data->urbs))) {
		ret = usb_rx_submit(pipe_data, urb, GFP_ATOMIC);
		if (ret < 0) {
			usb_put_urb(urb);
			mif_err("usb_rx_submit error with (%d)\n", ret);
			return ret;
		}
		usb_put_urb(urb);
	}
	mif_info("All urbs was submitted\n");
	return 0;
}

static void kill_anchored_urbs(struct work_struct *work)
{
	struct if_usb_devdata *pipe_data = container_of(work,
		struct if_usb_devdata, kill_urbs_work.work);

	usb_kill_anchored_urbs(&pipe_data->reading);
	mif_info("All reading urbs was killed\n");
}

static void usb_free_urbs(struct usb_link_device *usb_ld,
		struct if_usb_devdata *pipe_data)
{
	struct urb *urb;

	while ((urb = usb_get_from_anchor(&pipe_data->urbs))) {
		usb_poison_urb(urb);
		usb_put_urb(urb);
		usb_free_urb(urb);
	}
}

#if defined(CONFIG_SEC_MODEM_XMM6360)
static void mif_net_suspend(struct if_usb_devdata *pipe_data, int flag)
{
	if (!pipe_data->iod->ndev || !flag)
		return;
	pipe_data->net_suspend |= flag;
	netif_stop_queue(pipe_data->iod->ndev);
	mif_debug("stop 0x%04x, mask=0x%04x\n", flag, pipe_data->net_suspend);
	return;
}

static void mif_net_resume(struct if_usb_devdata *pipe_data, int flag)
{
	if (!pipe_data->iod->ndev || !flag)
		return;
	pipe_data->net_suspend &= ~(flag);
	if (!pipe_data->net_suspend) {
		netif_wake_queue(pipe_data->iod->ndev);
		mif_debug("wake 0x%04x\n", flag);
	} else {
		mif_debug("net 0x%04x, mask=0x%04x\n", flag,
							pipe_data->net_suspend);
	}
	return;
}
#else
static inline void mif_net_suspend(struct if_usb_devdata *pipe_data, int flag)
{
	return;
}
static inline void mif_net_resume(struct if_usb_devdata *pipe_data, int flag)
{
	return;
}
#endif

static struct if_usb_devdata *get_pipe_from_multi_channel(
	struct usb_link_device *usb_ld, struct io_device *iod)
{
	switch (iod->format) {
	case IPC_FMT:
	case IPC_RFS:
	case IPC_CMD:
	case IPC_RAW:
		/* IMC Modem ACM channel composition compatibility
		 * SIPC5 : ACM0 - IPC/RFS/RAW
		 *        ACM1 - Silent logging
		 *        ACM2 - reserved
		 * SIPC4.1 : ACM0 - IPC
		 *          ACM1 - RAW / Silent logging
		 *          ACM2 - RFS
		 *          ACM3 - Control Message(XMM6262 only)
		 */
		if (iod->ipc_version == SIPC_VER_50)
			return &usb_ld->acm_data[0];
		else
			return &usb_ld->acm_data[iod->format];

	case IPC_BOOT:
		return &usb_ld->acm_data[0];

	case IPC_RAW_NCM:
		return &usb_ld->ncm_data[iod->id - PS_DATA_CH_01];

	case IPC_MULTI_RAW:
		if (iod->ipc_version == SIPC_VER_50) /* loopback start send*/
			return &usb_ld->acm_data[0];
		break;

	default:
		mif_err("unexpect iod format=%d\n", iod->format);
		break;
	}
	return NULL;
}

static inline struct if_usb_devdata *get_pipe_from_single_channel(
	struct usb_link_device *usb_ld, struct io_device *iod)
{
	return (iod->format == IPC_RAW_NCM) ?
		&usb_ld->ncm_data[0] : &usb_ld->acm_data[0];
}

static int start_ipc(struct link_device *ld, struct io_device *iod)
{
	struct sk_buff *skb;
	int ret;
	struct usb_link_device *usb_ld = to_usb_link_device(ld);
	struct if_usb_devdata *pipe_data = &usb_ld->acm_data[IF_USB_FMT_EP];

	if (!usb_ld->if_usb_connected) {
		mif_err("HSIC not connected, skip start ipc\n");
		ret = -ENODEV;
		goto exit;
	}

	if (ld->mc->phone_state != STATE_ONLINE) {
		mif_err("MODEM is not online, skip start ipc\n");
		ret = -ENODEV;
		goto exit;
	}

	skb = alloc_skb(16, GFP_ATOMIC);
	if (unlikely(!skb))
		return -ENOMEM;
	memcpy(skb_put(skb, 1), "a", 1);
	skbpriv(skb)->iod = iod;
	skbpriv(skb)->ld = ld;

	mif_info("sending 'a' to start ipc\n");
	ret = usb_tx_skb(pipe_data, skb);
	if (ret < 0) {
		mif_err("usb_tx_urb fail\n");
		dev_kfree_skb_any(skb);
	}
exit:
	return ret;
}

static void stop_ipc(struct link_device *ld)
{
	ld->com_state = COM_NONE;
}

static int usb_init_communication(struct link_device *ld, struct io_device *iod)
{
	struct task_struct *task = get_current();
	char str[TASK_COMM_LEN];
	struct usb_link_device *usb_ld = to_usb_link_device(ld);
	struct if_usb_devdata *pipe_data = get_pipe_from_multi_channel(usb_ld, iod);
	struct io_device *check_iod;

	mif_info("%d:%s\n", task->pid, get_task_comm(str, task));

	if (iod->format == IPC_BOOT) {
		mif_err("start ipc packet isn't required for boot iod\n");
		return 0;
	}
	if (!usb_ld->if_usb_connected) {
		mif_err("HSIC not connected, open fail\n");
		return -ENODEV;
	}
	if (ld->mc->phone_state != STATE_ONLINE) {
		mif_err("MODEM is not ONLINE yet! %s open failed\n", iod->name);
		return -ENODEV;
	}
	if (iod->format == IPC_RAW_NCM)
		pipe_data->ndev = iod->ndev;

	/* Send IPC Start ASCII 'a' */
	switch (iod->format) {
	case IPC_FMT:
		check_iod = link_get_iod_with_format(ld, IPC_RFS);
		if (check_iod ? atomic_read(&check_iod->opened) : true)
			return start_ipc(ld, iod);
		else
			mif_err("%s defined but not opened\n", check_iod->name);
		break;
	case IPC_RFS:
		check_iod = link_get_iod_with_format(ld, IPC_FMT);
		if (atomic_read(&check_iod->opened) && check_iod)
			return start_ipc(ld, iod);
		else
			mif_err("%s defined but not opened\n", check_iod->name);
		break;
	default:
		break;
	}

	return 0;
}

static void usb_terminate_communication(struct link_device *ld,
			struct io_device *iod)
{
	if (iod->format == IPC_BOOT)
		return;

	if (iod->mc->phone_state == STATE_CRASH_RESET ||
		iod->mc->phone_state == STATE_CRASH_EXIT)
		stop_ipc(ld);

	return;
}

/* CDC class interrupt in endpoint control*/
static int init_status(struct if_usb_devdata *pipe_data,
	struct usb_interface *intf)
{
	char		*buf = NULL;
	unsigned	pipe = 0;
	unsigned	maxp;
	unsigned	period;

	if (!pipe_data->info->intr_complete)
		return 0;

	pipe = usb_rcvintpipe(pipe_data->usbdev,
			pipe_data->status->desc.bEndpointAddress
				& USB_ENDPOINT_NUMBER_MASK);
	maxp = usb_maxpacket(pipe_data->usbdev, pipe, 0);

	/* avoid 1 msec chatter:  min 8 msec poll rate*/
	period = max((int)pipe_data->status->desc.bInterval,
		(pipe_data->usbdev->speed == USB_SPEED_HIGH) ? 7 : 3);
	buf = kmalloc(maxp, GFP_KERNEL);
	if (buf) {
		pipe_data->intr_urb = usb_alloc_urb(0, GFP_KERNEL);
		if (!pipe_data->intr_urb) {
			kfree(buf);
			return -ENOMEM;
		} else {
			usb_fill_int_urb(pipe_data->intr_urb, pipe_data->usbdev,
				pipe, buf, maxp, pipe_data->info->intr_complete,
				pipe_data, period);
			pipe_data->intr_urb->transfer_flags |= URB_FREE_BUFFER;
			mif_debug("status ep%din, %d bytes period %d\n",
				usb_pipeendpoint(pipe), maxp, period);
		}
	}
	return 0;
}

static int usb_rx_submit(struct if_usb_devdata *pipe_data,
		struct urb *urb, gfp_t gfp_flags)
{
	struct sk_buff *skb;
	int delay = 0;
	int ret = 0;

	if (pipe_data->disconnected)
		return -ENOENT;

	ehci_vendor_txfilltuning();

	skb = prepare_skb(pipe_data, urb);
	if (unlikely(!skb)) {
		ret = -ENOMEM;
		goto error;
	}

	skbpriv(skb)->context = pipe_data;
	urb->transfer_flags = 0;
	usb_fill_bulk_urb(urb, pipe_data->usbdev, pipe_data->rx_pipe,
		(void *)skb->data, pipe_data->rx_buf_size, usb_rx_complete,
		(void *)skb);

	usb_anchor_urb(urb, &pipe_data->reading);
	if (pipe_data->usbdev)
		usb_mark_last_busy(pipe_data->usbdev);
	ret = usb_submit_urb(urb, gfp_flags);
	if (ret) {
		mif_err("submit urb fail with ret (%d)\n", ret);
		usb_unanchor_urb(urb);
		reprocess_skb(pipe_data, skb);
		goto error;
	}

	if (pipe_data->usbdev)
		usb_mark_last_busy(pipe_data->usbdev);
	return 0;

error:
	usb_anchor_urb(urb, &pipe_data->urbs);
	return ret;
}

static void usb_rx_complete(struct urb *urb)
{
	struct sk_buff *skb = urb->context;
	struct if_usb_devdata *pipe_data = skbpriv(skb)->context;
	struct usb_link_device *usb_ld = pipe_data->usb_ld;
	struct io_device *iod = pipe_data->iod;
	int ret;

	if (pipe_data->usbdev)
		usb_mark_last_busy(pipe_data->usbdev);

	switch (urb->status) {
	case -ENOENT:
	case 0:
		usb_ld->rx_err = 0;
		if (!urb->actual_length) {
			mif_debug("urb has zero length!\n");
			reprocess_skb(pipe_data, skb);
			goto rx_submit;
		}

		usb_ld->rx_cnt++;
		/* call iod recv */
		skb_put(skb, urb->actual_length);
		if (pipe_data->info->rx_fixup) {
			pr_rx_skb_with_format(iod->format, skb);
			pipe_data->info->rx_fixup(pipe_data, skb);
			reprocess_skb(pipe_data, skb);
			goto rx_submit;
		}
		/* flow control CMD by CP, not use io device */
		if (unlikely(pipe_data->format == IPC_CMD)) {
			ret = link_rx_flowctl_cmd(&usb_ld->ld,
					(char *)urb->transfer_buffer,
					urb->actual_length);
			if (ret < 0)
				mif_err("no multi raw device (%d)\n", ret);
			dev_kfree_skb_any(skb);
			goto rx_submit;
		}

		pr_rx_skb_with_format(iod->format, skb);
		logging_ipc_data(MIF_IPC_CP2AP, iod, skb);
		ret = iod->recv_skb(iod, &usb_ld->ld, skb);
		if (ret < 0) {
			mif_err("io device recv error (%d)\n", ret);
			pr_urb("HDLC error", urb);
			dev_kfree_skb_any(skb);
			break;
		}
rx_submit:
		if (urb->status == 0) {
			if (pipe_data->usbdev)
				usb_mark_last_busy(pipe_data->usbdev);

			if (!atomic_read(&pipe_data->kill_urb)) {
				ret = usb_rx_submit(pipe_data, urb, GFP_ATOMIC);
				if (ret < 0)
					mif_err("Failed to submit urb(%d)\n", ret);
				return;
			}
		}
		break;
	default:
		mif_err("urb error status = %d\n", urb->status);
		if (usb_ld->rx_err++ > MAX_RX_ERROR
					&& usb_ld->ld.com_state == COM_ONLINE) {
			usb_ld->rx_err = 0;
			sprintf(last_link_err, "RX error %d times\n",
								MAX_RX_ERROR);
			mif_err("%s\n", last_link_err);
			set_bit(LINK_EVENT_RECOVERY, &usb_ld->events);
			schedule_delayed_work(&usb_ld->link_event, 0);
		}
		reprocess_skb(pipe_data, skb);
		break;
	}

	usb_anchor_urb(urb, &pipe_data->urbs);
}

static int usb_send(struct link_device *ld, struct io_device *iod,
				struct sk_buff *skb)
{
	int ret;
	size_t tx_size;
	struct if_usb_devdata *pipe_data = NULL;
	struct usb_link_device *usb_ld = to_usb_link_device(ld);

	switch (iod->format) {
	case IPC_RAW:
		if (unlikely(ld->raw_tx_suspended)) {
			/* Unlike misc_write, vnet_xmit is in interrupt.
			 * Despite call netif_stop_queue on CMD_SUSPEND,
			 * packets can be reached here.
			 */
			if (in_irq()) {
				mif_err("raw tx is suspended, drop size=%d",
						skb->len);
				return -EBUSY;
			}

			mif_err("wait RESUME CMD...\n");
			INIT_COMPLETION(ld->raw_tx_resumed_by_cp);
			wait_for_completion(&ld->raw_tx_resumed_by_cp);
			mif_err("resumed done.\n");
		}
		break;
	case IPC_RAW_NCM:
	case IPC_BOOT:
	case IPC_FMT:
	case IPC_RFS:
	default:
		break;
	}
	/* store the tx size before run the tx_delayed_work*/
	tx_size = skb->len;

	/* drop packet, when link is not online */
	if (ld->com_state == COM_BOOT && iod->format != IPC_BOOT)
		goto exit;

	pipe_data = usb_ld->get_pipe(usb_ld, iod);
	if (!pipe_data)
		goto exit;

	skbpriv(skb)->iod = iod;
	skbpriv(skb)->ld = ld;

	/* Synchronous tx */
	ret = usb_tx_skb(pipe_data, skb);
	if (ret < 0) {
		/* TODO: */
		mif_err("usb_tx_skb fail(%d)\n", ret);
	}

	return tx_size;
exit:
	mif_err("%s: drop packet, size=%d, com_state=%d\n",
			iod->name, skb->len, ld->com_state);
	dev_kfree_skb_any(skb);
	return -EINVAL;
}

static void usb_tx_complete(struct urb *urb)
{
	struct sk_buff *skb = urb->context;
	struct io_device *iod = skbpriv(skb)->iod;
	struct link_device *ld = skbpriv(skb)->ld;
	struct usb_link_device *usb_ld = to_usb_link_device(ld);
	struct if_usb_devdata *pipe_data = skbpriv(skb)->context;
	unsigned long flag;
	bool retry_tx_urb = false;

	switch (urb->status) {
	case 0:
		if (urb->actual_length != urb->transfer_buffer_length)
			mif_err("TX len=%d, Complete len=%d\n",
				urb->transfer_buffer_length, urb->actual_length);

		logging_ipc_data(MIF_IPC_AP2CP, iod, skb);
		if (pipe_data->txpend_ts.tv_sec) {
			/* TX flowctl was resumed */
			mif_info("flowctl %s CH%d(%d) (%02d:%02d:%02d.%09lu)\n",
				"resume", pipe_data->idx, pipe_data->tx_pend,
				pipe_data->txpend_tm.tm_hour,
				pipe_data->txpend_tm.tm_min,
				pipe_data->txpend_tm.tm_sec,
				pipe_data->txpend_ts.tv_nsec);
			pipe_data->txpend_ts.tv_sec = 0;
		}
		break;
	case -ECONNRESET:
		/* This urb was returned by suspend unlink urb and it will send
		 * again at next resume time for XMM6360  CDC-NCM flowctl*/
		if (urb->actual_length)
			mif_err("ECONNRESET: TX len=%d, Complete len=%d\n",
				urb->transfer_buffer_length, urb->actual_length);
		if (pipe_data->format == IPC_RAW_NCM
						&& !pipe_data->net_connected) {
			retry_tx_urb = false;
			pipe_data->txpend_ts.tv_sec = 0;
			mif_info("NCM net stop(EP:%d), free remain NTB\n",
						usb_pipeendpoint(urb->pipe));
		} else {
			retry_tx_urb = true;
		}
		break;
	case -EPROTO:
		/* Keep the IPC/RFS packet if status -EPROTO, it will be send
		 * next time.
		 * If real USB protocol error, runtime resume will be failed
		 * and the RECOVERY EVENT will rise */
		if (iod->format == IPC_FMT || iod->format == IPC_RFS) {
			retry_tx_urb = true;
			mif_info("WARN: TX retry IPC/RFS packet\n");
		}
	case -ENOENT:
	case -ESHUTDOWN:
	default:
		if (iod->format != IPC_BOOT) {
			mif_com_log(iod->msd, "TX error (%d), EP(%d)\n",
				urb->status, usb_pipeendpoint(urb->pipe));
			mif_info("TX error (%d), EP(%d)\n", urb->status,
						usb_pipeendpoint(urb->pipe));
		}
	}

	spin_lock_irqsave(&pipe_data->sk_tx_q.lock, flag);
	__skb_unlink(skb, &pipe_data->sk_tx_q);
	if (pipe_data->sk_tx_q.qlen < tx_qlen && !pipe_data->txpend_ts.tv_sec)
		mif_net_resume(pipe_data, MIF_NET_SUSPEND_RF_STOP);
	spin_unlock_irqrestore(&pipe_data->sk_tx_q.lock, flag);

	if (retry_tx_urb) {
		/* If tx error case, retry_tx_urb flags will retry tx submit
		 * next runtime resume, it keep the urb to tx_deferd_urb and
		 * interface resume(if_usb_resume) will be re-submit these urbs.
		 */
		usb_anchor_urb(urb, &pipe_data->tx_deferd_urbs);
		if (urb->status != -ECONNRESET)
			mif_info("TX deferd urbs (%d) EP(%d)\n", urb->status,
						usb_pipeendpoint(urb->pipe));
	} else {
		dev_kfree_skb_any(skb);
		usb_free_urb(urb);
	}

	if (pipe_data->usbdev && usb_ld->if_usb_connected)
		usb_mark_last_busy(pipe_data->usbdev);
}

int usb_tx_skb(struct if_usb_devdata *pipe_data, struct sk_buff *skb)
{
	int ret;
	struct usb_link_device *usb_ld = pipe_data->usb_ld;
	struct device *dev = &usb_ld->usbdev->dev;
	struct urb *urb;
	gfp_t mem_flags = in_interrupt() ? GFP_ATOMIC : GFP_KERNEL;
	unsigned long flag;

	if (!usb_ld->usbdev) {
		mif_info("usbdev is invalid\n");
		return -EINVAL;
	}

	if (!usb_ld->if_usb_connected ||
		usb_ld->ld.com_state == COM_NONE) {
		printk_ratelimited(KERN_ERR "usb dev is not connected\n");
		return -ENODEV;
	}

	usb_ld->tx_cnt++;
#if defined(CONFIG_UMTS_MODEM_XMM6360)
	pm_runtime_get_noresume(dev); /* get usage */
	if (usb_get_rpm_status(dev) != RPM_ACTIVE) {
		ret = usb_linkpm_request_resume(usb_ld->usbdev);
		if (ret < 0)
			pm_request_resume(dev);
	}
#else
	pm_runtime_get(dev);
#endif

	if (pipe_data->info->tx_fixup) {
		/* check the modem status before sending IP packet */
		if (usb_ld->ld.mc->phone_state != STATE_ONLINE
						&& pipe_data->iod->ndev) {
			mif_info("not STATE_ONLINE, netif_carrier_off\n");
			netif_carrier_off(pipe_data->iod->ndev);
			ret = -EINVAL;
			goto done;
		}
		pr_tx_skb_with_format(pipe_data->iod->format, skb);
		skb = pipe_data->info->tx_fixup(pipe_data, skb, mem_flags);
		if (!skb) {
			mif_debug("CDC-NCM gathers skbs to NTB\n");
			ret = 0;
			goto done;
		}
	}
	if (!skb) {
		mif_err("invalid skb\n");
		ret = -EINVAL;
		goto done;
	}

	urb = usb_alloc_urb(0, mem_flags);
	if (!urb) {
		mif_err("alloc urb error\n");
		ret = -ENOMEM;
		goto done;
	}
	if (!skbpriv(skb)->nzlp)
		urb->transfer_flags = URB_ZERO_PACKET;

	usb_fill_bulk_urb(urb, pipe_data->usbdev, pipe_data->tx_pipe, skb->data,
			skb->len, usb_tx_complete, (void *)skb);

	skbpriv(skb)->context = pipe_data;
	skbpriv(skb)->urb = urb; /* kill urb when suspend if tx not complete*/

	/* Check usb link_pm status and if suspend,  transmission will be done
	 * in resume.
	 * If TX packet was sent from rild/network while interface suspending,
	 * It will fail because link pm will be changed to L2 as soon as tx
	 * submit. It should keep the TX packet and send next time when
	 * RPM_SUSPENDING status.
	 * In RPM_RESUMMING status, pipe_data->state flag can help the submit
	 * timming after sending previous kept anchor urb submit.
	 */
	if (pipe_data->state == STATE_SUSPENDED
				|| usb_get_rpm_status(dev) == RPM_SUSPENDING) {
		mif_info("<%s> Tx pkt will be send after intf resumed\n",
				pipe_data->iod->name);
		usb_anchor_urb(urb, &pipe_data->tx_deferd_urbs);
		if (pipe_data->ndev)
			mif_net_suspend(pipe_data, MIF_NET_SUSPEND_LINK_WAKE);
		ret = 0;
		goto done;
	}

	spin_lock_irqsave(&pipe_data->sk_tx_q.lock, flag);
	__skb_queue_tail(&pipe_data->sk_tx_q, skb);
	if (pipe_data->sk_tx_q.qlen > tx_qlen)
		mif_net_suspend(pipe_data, MIF_NET_SUSPEND_RF_STOP);
	spin_unlock_irqrestore(&pipe_data->sk_tx_q.lock, flag);

	pr_tx_skb_with_format(pipe_data->iod->format, skb);
	logging_ipc_data(MIF_IPC_RL2AP, pipe_data->iod, skb);
	ret = usb_submit_urb(urb, mem_flags);
	if (ret < 0) {
		mif_err("usb_submit_urb with ret(%d)\n", ret);
		spin_lock_irqsave(&pipe_data->sk_tx_q.lock, flag);
		__skb_unlink(skb, &pipe_data->sk_tx_q);
		spin_unlock_irqrestore(&pipe_data->sk_tx_q.lock, flag);
		usb_anchor_urb(urb, &pipe_data->tx_deferd_urbs);
		goto done;
	}
done:
	if (pipe_data->usbdev)
		usb_mark_last_busy(pipe_data->usbdev);
	pm_runtime_put(dev);

	return  ret;
}

static void if_change_modem_state(struct usb_link_device *usb_ld,
						enum modem_state state)
{
	struct modem_ctl *mc = usb_ld->ld.mc;

	if (!mc->iod || usb_ld->ld.com_state != COM_ONLINE)
		return;

	mif_err("set modem state %d by %pF\n", state,
		__builtin_return_address(0));
	mc->iod->modem_state_changed(mc->iod, state);
	mc->bootd->modem_state_changed(mc->bootd, state);
}

static void if_usb_event_work(struct work_struct *work)
{
	struct usb_link_device *usb_ld =
		container_of(work, struct usb_link_device, link_event.work);
	struct modem_ctl *mc = usb_ld->ld.mc;

	mif_info("event = 0x%lx, dflags = 0x%lx\n", usb_ld->events, dflags);
	if (test_bit(LINK_EVENT_RECOVERY, &usb_ld->events)) {
		clear_bit(LINK_EVENT_RECOVERY, &usb_ld->events);
		if (test_bit(LINK_DEBUG_RECOVER_CPDUMP, &dflags)) {
			mif_err("AP_DUMP_INT: %s\n", last_link_err);
			mc->ops.modem_force_crash_exit(mc);
			goto exit;
		}
		if (test_bit(LINK_DEBUG_RECOVER_RESET, &dflags)) {
			mif_err("CP_RESET: %s\n", last_link_err);
			if_change_modem_state(usb_ld, STATE_CRASH_RESET);
			goto exit;
		}
		if (test_bit(LINK_DEBUG_RECOVER_PANIC, &dflags)) {
			panic("HSIC: %s\n", last_link_err);
			goto exit;
		}
	}
exit:
	sprintf(last_link_err, "%s", "");
	return;
}

static void if_usb_reconnect_work(struct work_struct *work)
{
	struct usb_link_device *usb_ld =
		container_of(work, struct usb_link_device, link_reconnect_work.work);
	struct modem_ctl *mc = usb_ld->ld.mc;

	if (!mc || usb_ld->if_usb_connected)
		return;

	if (usb_ld->ld.com_state != COM_ONLINE)
		return;

	if (usb_ld->link_reconnect_cnt--) {
		if (mc->phone_state == STATE_ONLINE && !usb_ld->link_reconnect()) {
			/* try reconnect and check */
			mif_info("re-try cnt %d, Run delayed_work\n",
						usb_ld->link_reconnect_cnt);
			schedule_delayed_work(&usb_ld->link_reconnect_work,
							msecs_to_jiffies(200));
		} else {
			/* under cp crash or reset, just return */
			mif_info("mc->phone_state is not STATE_ONLINE\n");
			return;
		}
	} else {
		mif_info("Expire re-try cnt, Triggering CP_Reset\n");
		set_bit(LINK_EVENT_RECOVERY, &usb_ld->events);
		schedule_delayed_work(&usb_ld->link_event, 0);
	}
}

static int mif_unlink_urbs(struct if_usb_devdata *pipe_data,
							struct sk_buff_head *q)
{
	unsigned long flags;
	int count = 0;

	spin_lock_irqsave(&q->lock, flags);
	if (!skb_queue_empty(q)) {
		struct sk_buff *skb;
		struct urb *urb;
		int ret = 0;
		skb_queue_walk(q, skb) {
			urb = skbpriv(skb)->urb;
			usb_get_urb(urb);
			ret = usb_unlink_urb(urb);
			if (ret != -EINPROGRESS && ret != 0)
				mif_info("unlink urb err = %d\n", ret);
			else
				count++;
			usb_put_urb(urb);
		}
	}
	spin_unlock_irqrestore(&q->lock, flags);
	return count;
}

static int if_usb_suspend(struct usb_interface *intf, pm_message_t message)
{
	struct if_usb_devdata *pipe_data = usb_get_intfdata(intf);
	struct usb_link_device *usb_ld = pipe_data->usb_ld;

	if (!pipe_data->disconnected && pipe_data->state == STATE_RESUMED) {
		atomic_inc(&pipe_data->kill_urb);
		usb_kill_anchored_urbs(&pipe_data->reading);
		atomic_dec(&pipe_data->kill_urb);
		if (pipe_data->info->intr_complete && pipe_data->status)
			usb_kill_urb(pipe_data->intr_urb);
		/* release TX urbs */
		pipe_data->tx_pend = mif_unlink_urbs(pipe_data,
							&pipe_data->sk_tx_q);
		if (pipe_data->tx_pend) {
			mif_net_suspend(pipe_data, MIF_NET_SUSPEND_RF_STOP);
			/* Mark the last flowctl start time */
			if (!pipe_data->txpend_ts.tv_sec) {
				getnstimeofday(&pipe_data->txpend_ts);
				rtc_time_to_tm(pipe_data->txpend_ts.tv_sec,
							&pipe_data->txpend_tm);
			}
			mif_info("flowctl %s CH%d(%d) (%02d:%02d:%02d.%09lu)\n",
				"suspend", pipe_data->idx, pipe_data->tx_pend,
				pipe_data->txpend_tm.tm_hour,
				pipe_data->txpend_tm.tm_min,
				pipe_data->txpend_tm.tm_sec,
				pipe_data->txpend_ts.tv_nsec);
		}
		pipe_data->state = STATE_SUSPENDED;
	}
	usb_ld->suspended++;

	if (usb_ld->suspended == usb_ld->max_link_ch * 2) {
		mif_info("suspended resumeby(%d)\n", usb_ld->resumeby);
		mif_com_log(pipe_data->iod->msd, "Called %s func\n", __func__);
		/* Check HSIC interface error for debugging */
		if (usb_ld->resumeby == HSIC_RESUMEBY_CP && !usb_ld->rx_cnt) {
			if (usb_ld->debug_pending++ > 20) {
				usb_ld->debug_pending = 0;
				sprintf(last_link_err,
					"No TX/RX after CP initiated resume");
				mif_err("%s\n", last_link_err);
				set_bit(LINK_EVENT_RECOVERY, &usb_ld->events);
				schedule_delayed_work(&usb_ld->link_event, 0);
			}
		} else {
			usb_ld->debug_pending = 0;
			usb_ld->tx_cnt = 0;
			usb_ld->rx_cnt = 0;
		}
	}
	return 0;
}

/* usb_anchor_urb_head - add urb to anchor head
 * @urb: pointer to the urb to anchor
 * @anchor: pointer to the anchor
 *
 * If tx send fail, this add the urb to head of anchor, then it can
 * retry the tx urb after link recovery.
 */

static void usb_anchor_urb_head(struct urb *urb, struct usb_anchor *anchor)
{
	unsigned long flags;

	spin_lock_irqsave(&anchor->lock, flags);
	usb_get_urb(urb);
	list_add(&urb->anchor_list, &anchor->urb_list);
	urb->anchor = anchor;

	if (unlikely(anchor->poisoned))
		atomic_inc(&urb->reject);
	spin_unlock_irqrestore(&anchor->lock, flags);
}

static int usb_defered_tx_purge_anchor(struct if_usb_devdata *pipe_data)
{
	struct urb *urb;
	struct sk_buff *skb;
	int cnt = 0;

	mif_info("Previous TX packet purge\n");
	while ((urb = usb_get_from_anchor(&pipe_data->tx_deferd_urbs))) {
		usb_put_urb(urb);
		skb = (struct sk_buff *)urb->context;
		pr_tx_skb_with_format(pipe_data->iod->format, skb);
		dev_kfree_skb_any(skb);
		usb_free_urb(urb);
		cnt++;
	}
	if (cnt)
		mif_info("purge tx urb=%d(CH%d)\n", cnt, pipe_data->idx);
	return 0;
}

static int usb_defered_tx_from_anchor(struct if_usb_devdata *pipe_data)
{
	struct urb *urb;
	struct sk_buff *skb;
	int cnt = 0;
	int ret = 0;
	unsigned long flag;
	bool refrash = true;

	while ((urb = usb_get_from_anchor(&pipe_data->tx_deferd_urbs))) {
		usb_put_urb(urb);
		if (!pipe_data->usb_ld->if_usb_connected) {
			usb_anchor_urb_head(urb, &pipe_data->tx_deferd_urbs);
			ret = -ENODEV;
			goto exit;
		}
		skb = (struct sk_buff *)urb->context;
		skb_queue_tail(&pipe_data->sk_tx_q, skb);
		if (refrash) {
			usb_fill_bulk_urb(urb, pipe_data->usbdev,
				pipe_data->tx_pipe, skb->data, skb->len,
				usb_tx_complete, (void *)skb);
		}
		pr_tx_skb_with_format(pipe_data->iod->format, skb);
		logging_ipc_data(MIF_IPC_RL2AP, pipe_data->iod, skb);
		ret = usb_submit_urb(urb, GFP_ATOMIC);
		if (ret < 0) {
			/* TODO: deferd TX again */
			mif_err("resume deferd TX fail(%d)\n", ret);
			spin_lock_irqsave(&pipe_data->sk_tx_q.lock, flag);
			__skb_unlink(skb, &pipe_data->sk_tx_q);
			spin_unlock_irqrestore(&pipe_data->sk_tx_q.lock, flag);
			usb_anchor_urb_head(urb, &pipe_data->tx_deferd_urbs);
			goto exit;
		}
		cnt++;
	}
exit:
	if (cnt)
		mif_info("deferd tx urb=%d(CH%d)\n", cnt, pipe_data->idx);
	return ret;
}

static int if_usb_resume(struct usb_interface *intf)
{
	int ret;
	struct if_usb_devdata *pipe_data = usb_get_intfdata(intf);
	struct urb *urb;
	unsigned long flag;

	if (pipe_data->state != STATE_SUSPENDED) {
		mif_debug("aleady resume!\n");
		goto done;
	}

	/* Submit interrupt_in control RX urbs */
	if (pipe_data->info->intr_complete && pipe_data->status) {
		ret = usb_submit_urb(pipe_data->intr_urb, GFP_NOIO);
		if (ret < 0) {
			mif_err("control rx submit failed(%d)\n", ret);
			return ret;
		}
	}
	pipe_data->state = STATE_RESUMED;
	/* Send defferd TX urbs */
	ret = usb_defered_tx_from_anchor(pipe_data);
	if (ret < 0)
		goto resume_exit;

	if (pipe_data->net_connected) {
		ret = submit_anchored_urbs(pipe_data);
		if (ret < 0) {
			mif_err("submit_anchored_urbs error with (%d)\n", ret);
			return ret;
		}
	}
	mif_net_resume(pipe_data, MIF_NET_SUSPEND_LINK_WAKE);
	spin_lock_irqsave(&pipe_data->sk_tx_q.lock, flag);
	/* safety resume if tx queue is under max qlen from defered queue */
	if (pipe_data->sk_tx_q.qlen < tx_qlen)
		mif_net_resume(pipe_data, MIF_NET_SUSPEND_RF_STOP);
	spin_unlock_irqrestore(&pipe_data->sk_tx_q.lock, flag);
done:
	pipe_data->usb_ld->suspended--;

	if (!pipe_data->usb_ld->suspended) {
		mif_info("resumed resumeby(%d)\n", pipe_data->usb_ld->resumeby);
		mif_com_log(pipe_data->iod->msd, "Called %s func\n", __func__);
	}
	return 0;

resume_exit:
	return ret;
}

static int if_usb_reset_resume(struct usb_interface *intf)
{
	int ret;
	struct if_usb_devdata *pipe_data = usb_get_intfdata(intf);

	ret = if_usb_resume(intf);
	pipe_data->usb_ld->debug_pending = 0;
	mif_com_log(pipe_data->iod->msd, "Called %s func\n", __func__);

	return ret;
}

static void if_usb_disconnect(struct usb_interface *intf)
{
	struct if_usb_devdata *pipe_data = usb_get_intfdata(intf);
	struct usb_device *udev = interface_to_usbdev(intf);
	struct usb_link_device *usb_ld;

	if (!pipe_data || pipe_data->disconnected)
		return;

	usb_ld = pipe_data->usb_ld;
	mif_com_log(pipe_data->iod->msd, "Called %s func\n", __func__);
	usb_ld->if_usb_connected = 0;

	if (pipe_data->info->unbind) {
		mif_info("unbind(%pf)\n", pipe_data->info->unbind);
		pipe_data->info->unbind(pipe_data, intf);
	}

	usb_kill_anchored_urbs(&pipe_data->reading);
	if (pipe_data->idx < usb_ld->max_acm_ch)
		usb_free_urbs(usb_ld, pipe_data);

	/* TODO: kill interrupt_in urb */
	if (pipe_data->info->intr_complete && pipe_data->status) {
		usb_kill_urb(pipe_data->intr_urb);
		usb_free_urb(pipe_data->intr_urb);
	}

	mif_debug("put dev 0x%p\n", udev);
	usb_put_dev(udev);

	usb_ld->suspended = 0;
	cancel_delayed_work_sync(&pipe_data->usb_ld->link_event);

	if (usb_ld->ld.com_state != COM_ONLINE) {
		if (pipe_data->iod->format != IPC_BOOT)
			cancel_delayed_work(&usb_ld->link_reconnect_work);
	} else {
		if (pipe_data->iod->format == IPC_FMT)
			schedule_delayed_work(&usb_ld->link_reconnect_work,
					msecs_to_jiffies(500));
	}
	return;
}

static int cdc_acm_bind(struct if_usb_devdata *pipe_data,
		struct usb_interface *intf, struct usb_link_device *usb_ld)
{
	int ret;
	const struct usb_cdc_union_desc *union_hdr = NULL;
	const struct usb_host_interface *data_desc;
	unsigned char *buf = intf->altsetting->extra;
	int buflen = intf->altsetting->extralen;
	struct usb_interface *data_intf = NULL;
	struct usb_interface *control_intf;
	struct usb_device *usbdev = interface_to_usbdev(intf);
	struct usb_driver *usbdrv = to_usb_driver(intf->dev.driver);

	if (!buflen) {
		if (intf->cur_altsetting->endpoint->extralen &&
				    intf->cur_altsetting->endpoint->extra) {
			buflen = intf->cur_altsetting->endpoint->extralen;
			buf = intf->cur_altsetting->endpoint->extra;
		} else {
			data_desc = intf->cur_altsetting;
			if (!data_desc) {
				mif_err("data_desc is NULL\n");
				return -EINVAL;
			}
			mif_err("cdc-data desc - bootrom\n");
			goto found_data_desc;
		}
	}

	while (buflen > 0) {
		if (buf[1] == USB_DT_CS_INTERFACE) {
			switch (buf[2]) {
			case USB_CDC_UNION_TYPE:
				if (union_hdr)
					break;
				union_hdr = (struct usb_cdc_union_desc *)buf;
				break;
			default:
				break;
			}
		}
		buf += buf[0];
		buflen -= buf[0];
	}

	if (!union_hdr) {
		mif_err("USB CDC is not union type\n");
		return -EINVAL;
	}

	control_intf = usb_ifnum_to_if(usbdev, union_hdr->bMasterInterface0);
	if (!control_intf) {
		mif_err("control_inferface is NULL\n");
		return -ENODEV;
	}

	pipe_data->status = control_intf->altsetting->endpoint;
	if (!usb_endpoint_dir_in(&pipe_data->status->desc)) {
		mif_err("not initerrupt_in ep\n");
		pipe_data->status = NULL;
	}

	data_intf = usb_ifnum_to_if(usbdev, union_hdr->bSlaveInterface0);
	if (!data_intf) {
		mif_err("data_inferface is NULL\n");
		return -ENODEV;
	}

	data_desc = data_intf->altsetting;
	if (!data_desc) {
		mif_err("data_desc is NULL\n");
		return -ENODEV;
	}

found_data_desc:
	/* if_usb_set_pipe */
	if ((usb_pipein(data_desc->endpoint[0].desc.bEndpointAddress)) &&
	    (usb_pipeout(data_desc->endpoint[1].desc.bEndpointAddress))) {
		pipe_data->rx_pipe = usb_rcvbulkpipe(usbdev,
				data_desc->endpoint[0].desc.bEndpointAddress);
		pipe_data->tx_pipe = usb_sndbulkpipe(usbdev,
				data_desc->endpoint[1].desc.bEndpointAddress);
	} else if ((usb_pipeout(data_desc->endpoint[0].desc.bEndpointAddress))
		&& (usb_pipein(data_desc->endpoint[1].desc.bEndpointAddress))) {
		pipe_data->rx_pipe = usb_rcvbulkpipe(usbdev,
				data_desc->endpoint[1].desc.bEndpointAddress);
		pipe_data->tx_pipe = usb_sndbulkpipe(usbdev,
				data_desc->endpoint[0].desc.bEndpointAddress);
	} else {
		mif_err("undefined endpoint\n");
		return -EINVAL;
	}

	mif_debug("EP tx:%x, rx:%x\n",
		data_desc->endpoint[0].desc.bEndpointAddress,
		data_desc->endpoint[1].desc.bEndpointAddress);

	pipe_data->usbdev = usb_get_dev(usbdev);
	pipe_data->usb_ld = usb_ld;
	pipe_data->data_intf = data_intf;
	pipe_data->disconnected = 0;
	pipe_data->state = STATE_RESUMED;

	if (data_intf) {
		ret = usb_driver_claim_interface(usbdrv, data_intf,
							(void *)pipe_data);
		if (ret < 0) {
			mif_err("usb_driver_claim() failed\n");
			return ret;
		}
	}
	usb_set_intfdata(intf, (void *)pipe_data);

	return 0;
}

static void cdc_acm_unbind(struct if_usb_devdata *pipe_data,
	struct usb_interface *intf)
{
	usb_driver_release_interface(to_usb_driver(intf->dev.driver), intf);
	pipe_data->data_intf = NULL;
	pipe_data->usbdev = NULL;
	pipe_data->disconnected = 1;
	pipe_data->state = STATE_SUSPENDED;

	usb_set_intfdata(intf, NULL);
	return;
}

static int mif_usb2phy_notify(struct notifier_block *nfb,
		unsigned long events, void *arg)
{
	struct usb_link_device *usb_ld =
		container_of(nfb, struct usb_link_device, phy_nfb);
	struct modem_ctl *mc = usb_ld->ld.mc;

	if (!gpio_get_value(mc->gpio_cp_reset)
			|| !gpio_get_value(mc->gpio_phone_active)) {
		mif_debug("CP is not active\n");
		return NOTIFY_DONE;
	}

	switch (events) {
	case STATE_HSIC_LPA_ENTER:
		gpio_set_value(mc->gpio_pda_active, 0);
		mif_info("lpa enter(%ld): pda active(%d)\n",
			events, gpio_get_value(mc->gpio_pda_active));
		break;
	case STATE_HSIC_LPA_WAKE:
	case STATE_HSIC_LPA_PHY_INIT:
		gpio_set_value(mc->gpio_pda_active, 1);
		mif_info("phy_init(%ld): pda active(%d)\n",
			events, gpio_get_value(mc->gpio_pda_active));
		break;
	}

	return NOTIFY_DONE;
}

/* For checking to L3 stats(kernel suspend) */
static int mif_device_pm_notify(struct notifier_block *nfb,
						unsigned long event, void *arg)
{
	struct usb_link_device *usb_ld =
		container_of(nfb, struct usb_link_device, pm_nfb);
	struct modem_ctl *mc = usb_ld->ld.mc;

	if (!gpio_get_value(mc->gpio_cp_reset)
			|| !gpio_get_value(mc->gpio_phone_active)) {
		mif_debug("CP is not active\n");
		return NOTIFY_DONE;
	}

	switch (event) {
	case PM_POST_SUSPEND:
		usb_ld->debug_pending = 0;
		mif_debug("(%ld) clear debug_pending (%d)\n",
			event, usb_ld->debug_pending);
		break;
	}
	return NOTIFY_DONE;
}

void cdc_acm_intr_complete(struct urb *urb)
{
	int ret;

	mif_debug("status = %d\n", urb->status);

	switch (urb->status) {
	/* success */
	case -ENOENT:		/* urb killed by L2 suspend */
	case 0:
		break;
	case -ESHUTDOWN:	/* hardware gone */
		mif_err("intr shutdown, code %d, ep = %d\n", urb->status,
						usb_pipeendpoint(urb->pipe));
		return;

	/* NOTE:  not throttling like RX/TX, since this endpoint
	 * already polls infrequently
	 */
	default:
		mif_err("intr status %d, ep = %d\n", urb->status,
						usb_pipeendpoint(urb->pipe));
		break;
	}

	if (!urb->status) { /*skip -ENOENT L2 enter status */
		memset(urb->transfer_buffer, 0, urb->transfer_buffer_length);
		ret = usb_submit_urb(urb, GFP_ATOMIC);
		mif_debug("status: usb_submit_urb ret=%d\n", ret);
		if (ret != 0)
			mif_err("intr resubmit --> %d\n", ret);
	}
}

static int __devinit if_usb_probe(struct usb_interface *intf,
					const struct usb_device_id *id)
{
	int ret;
	int dev_index = 0;
	struct if_usb_devdata *pipe_data;
	int subclass = intf->altsetting->desc.bInterfaceSubClass;
	struct usb_device *usbdev = interface_to_usbdev(intf);
	struct usb_id_info *info = (struct usb_id_info *)id->driver_info;
	struct usb_link_device *usb_ld = info->usb_ld;
	struct modem_ctl *mc = usb_ld->ld.mc;
	struct urb *urb;

	pr_info("%s: Class=%d, SubClass=%d, Protocol=%d\n", __func__,
		intf->altsetting->desc.bInterfaceClass,
		intf->altsetting->desc.bInterfaceSubClass,
		intf->altsetting->desc.bInterfaceProtocol);

	/* if usb disconnected, AP try to reconnect 5 times.
	 * but because if_sub_connected is configured
	 * at the end of if_usb_probe, there was a chance
	 * that swk will be called again during enumeration.
	 * so.. cancel reconnect work_queue in this case. */
	if (usb_ld->ld.com_state == COM_ONLINE)
		cancel_delayed_work(&usb_ld->link_reconnect_work);

	usb_ld->usbdev = usbdev;
	pm_runtime_forbid(&usbdev->dev);

	switch (subclass) {
	case USB_CDC_SUBCLASS_NCM:
		if (info->max_ch)
			usb_ld->max_link_ch = usb_ld->max_acm_ch + info->max_ch;
		dev_index = intf->altsetting->desc.bInterfaceNumber / 2;
		if (dev_index >= usb_ld->max_link_ch) {
			mif_err("Over the NCM device Max number(%d/%d)\n",
				dev_index, usb_ld->max_link_ch);
			return -EINVAL;
		}
		pipe_data = &usb_ld->ncm_data[dev_index - usb_ld->max_acm_ch];
		pipe_data->format = IPC_RAW_NCM;
		pipe_data->idx = dev_index;
		pipe_data->net_suspend = 0;
		pipe_data->net_connected = false;
		pipe_data->submit_urbs = submit_anchored_urbs;
		pipe_data->iod = link_get_iod_with_channel(&usb_ld->ld,
				PS_DATA_CH_01 + dev_index - usb_ld->max_acm_ch);
		break;

	case USB_CDC_SUBCLASS_ACM:
	default:
		if (info->intf_id == BOOT_DOWN) {
			dev_index = 0;
			pipe_data = &usb_ld->acm_data[dev_index];
			pipe_data->idx = dev_index;
			pipe_data->net_connected = true;
			pipe_data->iod =
				link_get_iod_with_format(&usb_ld->ld, IPC_BOOT);
			pipe_data->rx_buf_size = 15872;	/* 15.5KB */
		} else {
			if (info->max_ch)
				usb_ld->max_acm_ch = info->max_ch;
			dev_index = intf->altsetting->desc.bInterfaceNumber / 2;
			if (dev_index >= usb_ld->max_acm_ch) {
				mif_err("Over the data ch Max number(%d/%d)\n",
						dev_index, usb_ld->max_acm_ch);
				return -EINVAL;
			}
			pipe_data = &usb_ld->acm_data[dev_index];
			pipe_data->idx = dev_index;
			pipe_data->net_connected = true;
			pipe_data->format = dev_index;
			if (mc->fixed_log_ch && dev_index == mc->fixed_log_ch)
				pipe_data->iod = link_get_iod_with_channel(
					&usb_ld->ld, SIPC_CH_ID_CPLOG1);
			else
				pipe_data->iod = link_get_iod_with_format(
							&usb_ld->ld, dev_index);
			pipe_data->rx_buf_size = (0xE00); /* 3.5KB */
		}

		/* backup rx_buf_size */
		pipe_data->rx_buf_setup = pipe_data->rx_buf_size;

		/* prepare rx_urb for ACM */
		urb = usb_alloc_urb(0, GFP_KERNEL);
		if (!urb) {
			mif_err("alloc urb fail\n");
			return -ENOMEM;
		}
		usb_anchor_urb(urb, &pipe_data->urbs);
		break;
	}

	pipe_data->info = info;
	atomic_set(&pipe_data->kill_urb, 0);

	if (info->bind) {
		mif_info("bind(%pf), pipe(%d), format(%d)\n", info->bind,
					dev_index, pipe_data->iod->format);
		ret = info->bind(pipe_data, intf, usb_ld);
		if (ret < 0) {
			mif_err("SubClass bind(%pf) err=%d\n", info->bind, ret);
			goto error_exit;
		}

		if (pipe_data->net_connected) {
			ret = submit_anchored_urbs(pipe_data);
			if (ret < 0)
				goto error_exit;
		}
	} else {
		mif_err("SubClass bind func was not defined\n");
		ret = -EINVAL;
		goto error_exit;
	}
	/* For CDC control interface interrupt in endpoint RX */
	if (info->intr_complete && pipe_data->status) {
		ret = init_status(pipe_data, intf);
		if (ret < 0) {
			mif_err("Control interface EP setup fail=(%d)\n", ret);
			goto error_exit;
		}
		ret = usb_submit_urb(pipe_data->intr_urb, GFP_KERNEL);
		if (ret < 0)
			goto error_exit;
	}

	usb_ld->suspended = 0;
	pm_suspend_ignore_children(&usbdev->dev, true);

	switch (info->intf_id) {
	case BOOT_DOWN:
		/* As of now, 2 links means 1 ACM + 1 NCM case */
		if (usb_ld->max_link_ch == 2)
			usb_ld->get_pipe = get_pipe_from_single_channel;
		else
			usb_ld->get_pipe = get_pipe_from_multi_channel;
		usb_defered_tx_purge_anchor(pipe_data);
		usb_ld->ld.com_state = COM_BOOT;
		usb_ld->if_usb_connected = 1;
		usb_ld->debug_pending = 0;
		usb_ld->rx_err = 0;
		usb_ld->tx_err = 0;
		mif_com_log(pipe_data->iod->msd, "<%s> BOOT_DOWN\n", __func__);
		break;

	case IPC_CHANNEL:
		/* HSIC main comm channel has been established */
#ifndef CONFIG_USB_NET_CDC_NCM
		if (dev_index == usb_ld->max_link_ch - 1) {
#else
		if (dev_index == usb_ld->max_acm_ch - 1) {
#endif
			usb_ld->ld.com_state = COM_ONLINE;
			usb_ld->link_reconnect_cnt = 2;
			if_change_modem_state(usb_ld, STATE_ONLINE);
			usb_ld->if_usb_connected = 1;
			mif_com_log(pipe_data->iod->msd,
						"<%s> IPC_CHANNEL\n", __func__);

#if defined(CONFIG_UMTS_MODEM_XMM6360) || defined(CONFIG_LTE_MODEM_XMM7260)
			/* Share the USB1-2 driver data with link_pm */
			dev_set_drvdata(&usbdev->dev, usb_ld);
#endif
		}
		/* send defered IPC after reconnect */
		usb_defered_tx_from_anchor(pipe_data);
		break;
	default:
		mif_err("undefined interface value(0x%x)\n", info->intf_id);
		break;
	}
	mif_info("successfully done, acm=%d, all=%d\n", usb_ld->max_acm_ch, usb_ld->max_link_ch);


	return 0;

error_exit:
	usb_free_urbs(usb_ld, pipe_data);
	return ret;
}

/* Vendor specific wrapper functions */
int xmm6360_cdc_ncm_bind(struct if_usb_devdata *pipe_data,
		struct usb_interface *intf, struct usb_link_device *usb_ld)
{
	pipe_data->iod->ipc_version = NO_SIPC_VER;
	return cdc_ncm_bind(pipe_data, intf, usb_ld);
}

static struct usb_id_info hsic_boot_down_info = {
	.description = "HSIC boot",
	.intf_id = BOOT_DOWN,
	.bind = cdc_acm_bind,
	.unbind = cdc_acm_unbind,
};
static struct usb_id_info xmm6360_cdc_2acm_info = {
	.description = "IMC IPC CDC-ACM",
	.intf_id = IPC_CHANNEL,
	.max_ch = 2,
	.bind = cdc_acm_bind,
	.unbind = cdc_acm_unbind,
	.intr_complete = cdc_acm_intr_complete,
};
static struct usb_id_info xmm6360_cdc_3acm_info = {
	.description = "IMC IPC CDC-ACM",
	.intf_id = IPC_CHANNEL,
	.max_ch = 3,
	.bind = cdc_acm_bind,
	.unbind = cdc_acm_unbind,
	.intr_complete = cdc_acm_intr_complete,
};
static struct usb_id_info xmm626x_cdc_acm_info = {
	.description = "IMC IPC CDC-ACM",
	.intf_id = IPC_CHANNEL,
	.bind = cdc_acm_bind,
	.unbind = cdc_acm_unbind,
};
static struct usb_id_info xmm6360_cdc_ncm_info = {
	.description = "IMC IPC CDC-NCM",
	.intf_id = IPC_CHANNEL,
	.urb_cnt = 2,
	.max_ch = 4,
	.bind = xmm6360_cdc_ncm_bind,
	.unbind = cdc_ncm_unbind,
	.tx_fixup = cdc_ncm_tx_fixup,
	.rx_fixup = cdc_ncm_rx_fixup_copyskb,
	.intr_complete = cdc_ncm_intr_complete,
};
static struct usb_id_info brcm_boot_down_info = {
	.description = "BRCM HSIC boot",
	.intf_id = BOOT_DOWN,
	.bind = cdc_acm_bind,
	.unbind = cdc_acm_unbind,
};
static struct usb_id_info brcm_cdc_acm_info = {
	.description = "BRCM IPC CDC-ACM",
	.intf_id = IPC_CHANNEL,
	.bind = cdc_acm_bind,
	.unbind = cdc_acm_unbind,
	.intr_complete = cdc_acm_intr_complete,
};
static struct usb_id_info brcm_cdc_ncm_info = {
	.description = "BRCM IPC CDC-NCM",
	.intf_id = IPC_CHANNEL,
	.urb_cnt = MULTI_URB,
	.bind = cdc_ncm_bind,
	.unbind = cdc_ncm_unbind,
	.tx_fixup = cdc_ncm_tx_fixup,
	.rx_fixup = cdc_ncm_rx_fixup,
	.intr_complete = cdc_ncm_intr_complete,
};
static struct usb_id_info ste_boot_down_info = {
	.description = "Ericsson HSIC boot",
	.intf_id = BOOT_DOWN,
	.bind = cdc_acm_bind,
	.unbind = cdc_acm_unbind,
};
static struct usb_id_info ste_cdc_acm_info = {
	.description = "Ericsson IPC CDC-ACM",
	.intf_id = IPC_CHANNEL,
	.bind = cdc_acm_bind,
	.unbind = cdc_acm_unbind,
	.intr_complete = cdc_acm_intr_complete,
};
static struct usb_id_info ste_cdc_ncm_info = {
	.description = "Ericsson IPC CDC-NCM",
	.intf_id = IPC_CHANNEL,
	.urb_cnt = MULTI_URB,
	.bind = cdc_ncm_bind,
	.unbind = cdc_ncm_unbind,
	.tx_fixup = cdc_ncm_tx_fixup,
	.rx_fixup = cdc_ncm_rx_fixup,
	.intr_complete = cdc_ncm_intr_complete,
};
static struct usb_id_info cmc_boot_down_info = {
	.description = "CMC HSIC boot",
	.intf_id = BOOT_DOWN,
	.bind = cdc_acm_bind,
	.unbind = cdc_acm_unbind,
};
static struct usb_id_info cmc_cdc_acm_info = {
	.description = "CMC IPC CDC-ACM",
	.intf_id = IPC_CHANNEL,
	.bind = cdc_acm_bind,
	.unbind = cdc_acm_unbind,
};
static struct usb_id_info cmc_cdc_ncm_info = {
	.description = "CMC IPC CDC-NCM",
	.intf_id = IPC_CHANNEL,
	.urb_cnt = MULTI_URB,
	.bind = cdc_ncm_bind,
	.unbind = cdc_ncm_unbind,
	.tx_fixup = cdc_ncm_tx_fixup,
	.rx_fixup = cdc_ncm_rx_fixup,
	.intr_complete = cdc_ncm_intr_complete,
};

static struct usb_device_id if_usb_ids[] = {
	{ USB_DEVICE_AND_INTERFACE_INFO(0x8087, 0x0716, USB_CLASS_CDC_DATA,
		0, USB_CDC_PROTO_NONE),
	.driver_info = (unsigned long)&hsic_boot_down_info,
	}, /* IMC XMM6360 BOOTROM */
	{ USB_DEVICE_AND_INTERFACE_INFO(0x1519, 0x0443, USB_CLASS_CDC_DATA,
		0, USB_CDC_PROTO_NONE),
	.driver_info = (unsigned long)&xmm6360_cdc_3acm_info,
	}, /* IMC XMM6360 BOOTROM */
	{ USB_DEVICE_AND_INTERFACE_INFO(0x1519, 0x0443, USB_CLASS_COMM,
		USB_CDC_SUBCLASS_ACM, 1),
	.driver_info = (unsigned long)&xmm6360_cdc_3acm_info,
	}, /* IMC XMM6360 MAIN */
	{ USB_DEVICE_AND_INTERFACE_INFO(0x8087, 0x0940, USB_CLASS_COMM,
		USB_CDC_SUBCLASS_ACM, 1),
	.driver_info = (unsigned long)&xmm6360_cdc_2acm_info,
	}, /* IMC XMM6360 MAIN, 2ACM + 4NCM ver */
	{ USB_DEVICE_AND_INTERFACE_INFO(0x058b, 0x0041,	USB_CLASS_COMM,
		USB_CDC_SUBCLASS_ACM, USB_CDC_PROTO_NONE),
	.driver_info = (unsigned long)&hsic_boot_down_info,
	}, /* IMC XMM626x BOOTROM */
	{ USB_DEVICE_AND_INTERFACE_INFO(0x1519, 0x0020,	USB_CLASS_COMM,
		USB_CDC_SUBCLASS_ACM, 1),
	.driver_info = (unsigned long)&xmm626x_cdc_acm_info,
	}, /* IMC XMM626x MAIN */
	{ USB_DEVICE_AND_INTERFACE_INFO(0x0a5c, 0xe820, 0xff, 0xff, 0xff),
	.driver_info = (unsigned long)&brcm_boot_down_info,
	}, /* BRCM2189x BOOT */
	{ USB_DEVICE_AND_INTERFACE_INFO(0x0a5c, 0x0510, USB_CLASS_COMM,
		USB_CDC_SUBCLASS_ACM, USB_CDC_PROTO_NONE),
	.driver_info = (unsigned long)&brcm_cdc_acm_info,
	}, /* BRCM2189x MAIN */
	{ USB_DEVICE_AND_INTERFACE_INFO(0x04cc, 0x0500, 0xff, 0x01, 0xff),
	.driver_info = (unsigned long)&ste_boot_down_info,
	}, /* Ericsson M74XX BOOT */
	{ USB_DEVICE_AND_INTERFACE_INFO(0x8087, 0x07ef, USB_CLASS_CDC_DATA,
		0, USB_CDC_PROTO_NONE),
	.driver_info = (unsigned long)&hsic_boot_down_info,
	}, /* IMC XMM7260 BOOTROM_HSIC */
	{ USB_DEVICE_AND_INTERFACE_INFO(0x8087, 0x07ed, 0xdc,
		0, USB_CDC_PROTO_NONE),
	.driver_info = (unsigned long)&hsic_boot_down_info,
	}, /* IMC XMM7260 BOOTROM_UART */
	{ USB_DEVICE_AND_INTERFACE_INFO(0x04cc, 0x2342,	USB_CLASS_COMM,
		USB_CDC_SUBCLASS_ACM, 1),
	.driver_info = (unsigned long)&ste_cdc_acm_info,
	}, /* Ericsson M74XX MAIN */
	{ USB_DEVICE_AND_INTERFACE_INFO(0x04e8, 0x7000, USB_CLASS_COMM,
		USB_CDC_SUBCLASS_ACM, 1),
	.driver_info = (unsigned long)&cmc_boot_down_info,
	}, /* CMC22x CDC ACM BOOT LOADER */
	{ USB_DEVICE_AND_INTERFACE_INFO(0x04e8, 0x7001, USB_CLASS_COMM,
		USB_CDC_SUBCLASS_ACM, 1),
	.driver_info = (unsigned long)&cmc_cdc_acm_info,
	}, /* CMC22x CDC ACM */
#ifndef CONFIG_USB_NET_CDC_NCM
	{ USB_DEVICE_AND_INTERFACE_INFO(0x1519, 0x0443,	USB_CLASS_COMM,
		USB_CDC_SUBCLASS_NCM, USB_CDC_PROTO_NONE),
	.driver_info = (unsigned long)&xmm6360_cdc_ncm_info,
	}, /* IMC XMM6360 MAIN NCM */
	{ USB_DEVICE_AND_INTERFACE_INFO(0x8087, 0x0940,	USB_CLASS_COMM,
		USB_CDC_SUBCLASS_NCM, USB_CDC_PROTO_NONE),
	.driver_info = (unsigned long)&xmm6360_cdc_ncm_info,
	}, /* IMC XMM6360 MAIN NCM, 2ACM + 4NCM ver */
	{ USB_DEVICE_AND_INTERFACE_INFO(0x04cc, 0x2342,	USB_CLASS_COMM,
		USB_CDC_SUBCLASS_NCM, USB_CDC_PROTO_NONE),
	.driver_info = (unsigned long)&ste_cdc_ncm_info,
	}, /* Ericsson M74XX MAIN NCM */
	{ USB_DEVICE_AND_INTERFACE_INFO(0x0a5c, 0x0510, USB_CLASS_COMM,
		USB_CDC_SUBCLASS_NCM, USB_CDC_PROTO_NONE),
	.driver_info = (unsigned long)&brcm_cdc_ncm_info,
	}, /* BRCM2189x MAIN NCM */
	{ USB_DEVICE_AND_INTERFACE_INFO(0x04e8, 0x7001,	USB_CLASS_COMM,
		USB_CDC_SUBCLASS_NCM, USB_CDC_PROTO_NONE),
	.driver_info = (unsigned long)&cmc_cdc_ncm_info,
	}, /* CMC22x CDC NCM */
#endif
/*	{ USB_INTERFACE_INFO(USB_CLASS_COMM, USB_CDC_SUBCLASS_ACM,
		USB_CDC_PROTO_NONE),
	},
	{ USB_INTERFACE_INFO(USB_CLASS_COMM, USB_CDC_SUBCLASS_NCM,
		USB_CDC_PROTO_NONE),
	},
*/
	{}
};
MODULE_DEVICE_TABLE(usb, if_usb_ids);

static struct usb_driver if_usb_driver = {
	.name =		"cdc_modem",
	.probe =	if_usb_probe,
	.disconnect =	if_usb_disconnect,
	.id_table =	if_usb_ids,
	.suspend =	if_usb_suspend,
	.resume =	if_usb_resume,
	.reset_resume =	if_usb_reset_resume,
	.supports_autosuspend = 1,
};

static int if_init_rx_urb(struct if_usb_devdata *pipe_data,
		struct usb_link_device *usb_ld, int idx)
{
	struct urb *urb;
	struct sk_buff *skb;
	// int alloc_size = CDC_NCM_NTB_MAX_SIZE_RX + NET_IP_ALIGN;
	int alloc_size = CDC_NCM_NTB_MAX_SIZE_RX;
	int cnt;

	for (cnt = 0; cnt < MULTI_URB; cnt++) {
		urb = usb_alloc_urb(0, GFP_KERNEL);
		if (!urb) {
			mif_err("Failed to alloc urb\n");
			return -ENOMEM;
		}

		skb = alloc_skb(alloc_size, GFP_KERNEL | GFP_DMA);
		if (!skb) {
			usb_free_urb(urb);
			mif_err("Failed to alloc rx_skb\n");
			return -ENOMEM;
		}

		urb->context = skb;
		usb_anchor_urb(urb, &pipe_data->urbs);
	}

	mif_info("Complete init_rx_urb(%d, %d)\n", idx, cnt);
	return 0;
}

static int if_usb_init(struct link_device *ld)
{
	int ret;
	int i;
	struct usb_link_device *usb_ld = to_usb_link_device(ld);
	struct if_usb_devdata *pipe_data;
	struct usb_id_info *id_info;

	/* to connect usb link device with usb interface driver */
	for (i = 0; i < ARRAY_SIZE(if_usb_ids); i++) {
		id_info = (struct usb_id_info *)if_usb_ids[i].driver_info;
		if (id_info)
			id_info->usb_ld = usb_ld;
	}

	ret = usb_register(&if_usb_driver);
	if (ret) {
		mif_err("usb_register_driver() fail : %d\n", ret);
		return ret;
	}

	/* common devdata initialize */
	for (i = 0; i < usb_ld->max_link_ch; i++) {
		pipe_data = get_pipedata_with_idx(usb_ld, i);

		init_usb_anchor(&pipe_data->urbs);
		init_usb_anchor(&pipe_data->reading);

		skb_queue_head_init(&pipe_data->free_rx_q);
		skb_queue_head_init(&pipe_data->sk_tx_q);
		init_usb_anchor(&pipe_data->tx_deferd_urbs);
		INIT_DELAYED_WORK(&pipe_data->kill_urbs_work,
			kill_anchored_urbs);

		/* static memory allocation for NCM */
		if (i > usb_ld->max_acm_ch - 1) {
			ret = if_init_rx_urb(pipe_data, usb_ld, i);
			if (ret) {
				mif_err("Failed init_rx_urb\n");
				return ret;
			}
		}
	}

	INIT_DELAYED_WORK(&usb_ld->link_event, if_usb_event_work);
	INIT_DELAYED_WORK(&usb_ld->link_reconnect_work, if_usb_reconnect_work);
	usb_ld->phy_nfb.notifier_call = mif_usb2phy_notify;
	register_usb2phy_notifier(&usb_ld->phy_nfb);
	usb_ld->pm_nfb.notifier_call = mif_device_pm_notify;
	register_pm_notifier(&usb_ld->pm_nfb);

	mif_info("if_usb_init() done : %d, usb_ld (0x%p)\n", ret, usb_ld);
	return 0;
}

/* Export private data symbol for ramdump debugging */
static struct usb_link_device *g_mif_usbld;

struct link_device *hsic_create_link_device(void *data)
{
	int ret;
	struct platform_device *pdev = (struct platform_device *)data;
	struct modem_data *pdata = pdev->dev.platform_data;
	struct usb_link_device *usb_ld;
	struct link_device *ld;

	usb_ld = kzalloc(sizeof(struct usb_link_device), GFP_KERNEL);
	if (!usb_ld) {
		mif_err("get usb_ld fail -ENOMEM\n");
		return NULL;
	}
	g_mif_usbld = usb_ld;

#define XMM626X_ACM_NUM 4
	usb_ld->link_reconnect = pdata->link_reconnect;
	usb_ld->max_acm_ch = pdata->max_acm_channel ?: XMM626X_ACM_NUM;
	usb_ld->max_link_ch = pdata->max_link_channel ?: XMM626X_ACM_NUM;
	if (pdata->max_tx_qlen) {
		tx_qlen = pdata->max_tx_qlen;
		mif_info("Get tx qlen : %d\n", tx_qlen);
	}
	usb_ld->acm_data = kzalloc(usb_ld->max_acm_ch
			* sizeof(struct if_usb_devdata), GFP_KERNEL);
	usb_ld->ncm_data = kzalloc((usb_ld->max_link_ch - usb_ld->max_acm_ch)
			* sizeof(struct if_usb_devdata), GFP_KERNEL);
	if (!usb_ld->acm_data || !usb_ld->ncm_data) {
		mif_err("get devdata fail -ENOMEM\n");
		goto error;
	}

	INIT_LIST_HEAD(&usb_ld->ld.list);

	ld = &usb_ld->ld;

	ld->name = "usb";
	ld->init_comm = usb_init_communication;
	ld->terminate_comm = usb_terminate_communication;
	ld->send = usb_send;
	ld->com_state = COM_NONE;
	ld->raw_tx_suspended = false;
	init_completion(&ld->raw_tx_resumed_by_cp);

	ret = if_usb_init(ld);
	if (ret)
		goto error;

	mif_info("%s : create_link_device DONE\n", usb_ld->ld.name);
	return (void *)ld;
error:
	kfree(usb_ld->acm_data);
	kfree(usb_ld->ncm_data);
	kfree(usb_ld);
	return NULL;
}

static void __exit if_usb_exit(void)
{
	usb_deregister(&if_usb_driver);
}
