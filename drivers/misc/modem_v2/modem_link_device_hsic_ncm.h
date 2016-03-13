/*
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

#ifndef __MODEM_LINK_DEVICE_USB_H__
#define __MODEM_LINK_DEVICE_USB_H__

#include <linux/rtc.h>

/* FOR XMM6360 */
enum {
	IF_USB_BOOT_EP = 0,
	IF_USB_FMT_EP = 0,
	IF_USB_RAW_EP,
	IF_USB_RFS_EP,
	IF_USB_CMD_EP,
	_IF_USB_ACMNUM_MAX,
};

#define PS_DATA_CH_01	0xa
#define RX_POOL_SIZE 2
#define MULTI_URB 4

enum {
	BOOT_DOWN = 0,
	IPC_CHANNEL
};

enum ch_state {
	STATE_SUSPENDED,
	STATE_RESUMED,
};

struct link_pm_info {
	struct usb_link_device *usb_ld;
};

struct if_usb_devdata;
struct usb_id_info {
	int intf_id;
	int urb_cnt;
	struct usb_link_device *usb_ld;
	unsigned max_ch;

	char *description;
	int (*bind)(struct if_usb_devdata *, struct usb_interface *,
			struct usb_link_device *);
	void (*unbind)(struct if_usb_devdata *, struct usb_interface *);
	int (*rx_fixup)(struct if_usb_devdata *, struct sk_buff *skb);
	struct sk_buff *(*tx_fixup)(struct if_usb_devdata *dev,
			struct sk_buff *skb, gfp_t flags);
	void (*intr_complete)(struct urb *urb);
};

#define MIF_NET_SUSPEND_RF_STOP         (0x1<<0)
#define MIF_NET_SUSPEND_LINK_WAKE	(0x1<<1)
#define MIF_NET_SUSPEND_GET_TX_BUF	(0x1<<2)
#define MIF_NET_SUSPEND_TX_RETRY	(0x1<<3)

struct mif_skb_pool;
struct if_usb_devdata {
	struct usb_interface *data_intf;
	struct usb_link_device *usb_ld;
	struct usb_device *usbdev;
	unsigned int tx_pipe;
	unsigned int rx_pipe;
	struct usb_host_endpoint *status;
	u8 disconnected;

	int format;
	int idx;

	/* Multi-URB style*/
	struct usb_anchor urbs;
	struct usb_anchor reading;

	struct urb *intr_urb;
	unsigned int rx_buf_size;
	unsigned int rx_buf_setup;
	enum ch_state state;

	struct usb_id_info *info;
	/* SubClass expend data - optional */
	void *sedata;
	struct io_device *iod;
	unsigned long flags;

	struct sk_buff_head free_rx_q;
	struct sk_buff_head sk_tx_q;
	unsigned tx_pend;
	struct timespec txpend_ts;
	struct rtc_time txpend_tm;
	struct usb_anchor tx_deferd_urbs;
	struct net_device *ndev;
	int net_suspend;
	bool net_connected;

	struct mif_skb_pool *ntb_pool;

	int (*submit_urbs)(struct if_usb_devdata *);
	struct delayed_work kill_urbs_work;
	atomic_t kill_urb;
};

enum resume_by_status {
	HSIC_RESUMEBY_UNKNOWN,
	HSIC_RESUMEBY_AP,
	HSIC_RESUMEBY_CP,
};

struct usb_link_device {
	/*COMMON LINK DEVICE*/
	struct link_device ld;

	/*USB SPECIFIC LINK DEVICE*/
	struct usb_device *usbdev;
	int max_link_ch;
	int max_acm_ch;
	int acm_cnt;
	int ncm_cnt;
	struct if_usb_devdata *acm_data;
	struct if_usb_devdata *ncm_data;
	unsigned int suspended;
	int if_usb_connected;

	struct mif_skb_pool skb_pool;
	struct delayed_work link_event;
	unsigned long events;

	struct notifier_block phy_nfb;
	struct notifier_block pm_nfb;

	/* for debug */
	unsigned debug_pending;
	unsigned tx_cnt;
	unsigned rx_cnt;
	unsigned tx_err;
	unsigned rx_err;
	unsigned resumeby;

	/* to decide the suitable function regarding pipe_data */
	struct if_usb_devdata *(*get_pipe)(struct usb_link_device *ld,
			struct io_device *iod);
	int (*link_reconnect)(void);
	struct delayed_work link_reconnect_work;
	int link_reconnect_cnt;
};

enum bit_link_events {
	LINK_EVENT_RECOVERY,
};

/* converts from struct link_device* to struct xxx_link_device* */
#define to_usb_link_device(linkdev) \
			container_of(linkdev, struct usb_link_device, ld)

#define get_pipedata_with_idx(usb_ld, idx) \
	((idx < usb_ld->max_acm_ch) ? \
	 &usb_ld->acm_data[idx] : \
	 &usb_ld->ncm_data[idx - usb_ld->max_acm_ch])

#ifdef FOR_TEGRA
extern void tegra_ehci_txfilltuning(void);
#define ehci_vendor_txfilltuning tegra_ehci_txfilltuning
#else
#define ehci_vendor_txfilltuning()
#endif

int usb_tx_skb(struct if_usb_devdata *pipe_data, struct sk_buff *skb);
extern int usb_resume(struct device *dev, pm_message_t msg);
#if defined(CONFIG_UMTS_MODEM_XMM6360)
int usb_linkpm_request_resume(struct usb_device *udev);
#endif
#endif
