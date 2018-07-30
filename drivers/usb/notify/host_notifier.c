/*
 * Copyright (C) 2011 Samsung Electronics Co. Ltd.
 *  Hyuk Kang <hyuk78.kang@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/kthread.h>
#include <linux/wakelock.h>
#include <linux/host_notify.h>
#include <linux/usb_notify_sysfs.h>
#include <linux/timer.h>
#include <linux/kthread.h>
#include <mach/usb3-drd.h>

#if defined(CONFIG_HAS_EARLYSUSPEND) && defined(CONFIG_FAST_BOOT)
#include <linux/earlysuspend.h>
#endif

#define DEFAULT_OVC_POLL_SEC 3

struct  ovc {
	wait_queue_head_t	 delay_wait;
	struct completion	scanning_done;
	struct task_struct *th;
	struct mutex ovc_lock;
	int thread_remove;
	int can_ovc;
	int poll_period;
	int prev_state;
	void *data;
	int (*check_state)(void *);
};

struct  host_notifier_info {
	struct host_notifier_platform_data *pdata;
	struct usb_notify_dev udev;
	struct task_struct *th;
	struct wake_lock	wlock;
	struct delayed_work current_dwork;
	wait_queue_head_t	delay_wait;
	int	thread_remove;
	int currentlimit_irq;
	struct ovc ovc_info;
};

static struct host_notifier_info ninfo;

#if defined(CONFIG_HAS_EARLYSUSPEND) && defined(CONFIG_FAST_BOOT)
static struct early_suspend early_suspend;
#endif

void enable_ovc(int enable)
{
	ninfo.ovc_info.can_ovc = enable;
}

static int ovc_scan_thread(void *data)
{
	struct ovc *ovcinfo = NULL;
	int state;

	ovcinfo = &ninfo.ovc_info;
	while (!kthread_should_stop()) {
		wait_event_interruptible_timeout(ovcinfo->delay_wait,
			ovcinfo->thread_remove, (ovcinfo->poll_period)*HZ);
		if (ovcinfo->thread_remove)
			break;
		mutex_lock(&ninfo.ovc_info.ovc_lock);
		if (ovcinfo->check_state
			&& ovcinfo->data
				&& ovcinfo->can_ovc) {

			state = ovcinfo->check_state(data);

			if (ovcinfo->prev_state != state) {
				if (state == HNOTIFY_LOW) {
					pr_err("%s overcurrent detected\n",
							__func__);
					host_state_notify(&ninfo.pdata->ndev,
						NOTIFY_HOST_OVERCURRENT);
				} else if (state == HNOTIFY_HIGH) {
					pr_info("%s vbus draw detected\n",
							__func__);
					host_state_notify(&ninfo.pdata->ndev,
						NOTIFY_HOST_NONE);
				}
			}
			ovcinfo->prev_state = state;
		}
		mutex_unlock(&ninfo.ovc_info.ovc_lock);
		if (!ovcinfo->can_ovc)
			ovcinfo->thread_remove = 1;
	}
	pr_info("ovc_scan_thread exit\n");
	complete_and_exit(&ovcinfo->scanning_done, 0);
}

void ovc_start(void)
{
	if (!ninfo.ovc_info.can_ovc)
		goto skip;

	ninfo.ovc_info.prev_state = HNOTIFY_INITIAL;
	ninfo.ovc_info.poll_period = DEFAULT_OVC_POLL_SEC;
	INIT_COMPLETION(ninfo.ovc_info.scanning_done);
	ninfo.ovc_info.thread_remove = 0;
	ninfo.ovc_info.th = kthread_run(ovc_scan_thread,
			ninfo.ovc_info.data, "ovc-scan-thread");
	if (IS_ERR(ninfo.ovc_info.th)) {
		pr_err("Unable to start the ovc-scanning thread\n");
		complete(&ninfo.ovc_info.scanning_done);
	}
	pr_info("%s on\n", __func__);
	return;
skip:
	complete(&ninfo.ovc_info.scanning_done);
	pr_info("%s skip\n", __func__);
	return;
}

void ovc_stop(void)
{
	ninfo.ovc_info.thread_remove = 1;
	wake_up_interruptible(&ninfo.ovc_info.delay_wait);
	wait_for_completion(&ninfo.ovc_info.scanning_done);
	mutex_lock(&ninfo.ovc_info.ovc_lock);
	ninfo.ovc_info.check_state = NULL;
	ninfo.ovc_info.data = 0;
	mutex_unlock(&ninfo.ovc_info.ovc_lock);
	pr_info("%s\n", __func__);
}

static void ovc_init(struct host_notifier_info *pinfo)
{
	init_waitqueue_head(&pinfo->ovc_info.delay_wait);
	init_completion(&pinfo->ovc_info.scanning_done);
	mutex_init(&pinfo->ovc_info.ovc_lock);
	pinfo->ovc_info.prev_state = HNOTIFY_INITIAL;
	pr_info("%s\n", __func__);
}

int register_ovc_func(int (*check_state)(void *), void *data)
{
	int ret = 0;

	mutex_lock(&ninfo.ovc_info.ovc_lock);
	ninfo.ovc_info.check_state = check_state;
	ninfo.ovc_info.data = data;
	mutex_unlock(&ninfo.ovc_info.ovc_lock);
	pr_info("%s\n", __func__);
	return ret;
}

static int currentlimit_thread(void *data)
{
	struct host_notifier_info *ninfo = data;
	struct host_notify_dev *ndev = &ninfo->pdata->ndev;
	int gpio = ninfo->pdata->gpio;
	int prev = ndev->booster;
	int ret = 0;

	pr_info("host_notifier usbhostd: start %d\n", prev);

	while (!kthread_should_stop()) {
		wait_event_interruptible_timeout(ninfo->delay_wait,
				ninfo->thread_remove, 1 * HZ);

		ret = gpio_get_value(gpio);
		if (prev != ret) {
			pr_info("host_notifier usbhostd: gpio %d = %s\n",
					gpio, ret ? "HIGH" : "LOW");
			ndev->booster = ret ?
				NOTIFY_POWER_ON : NOTIFY_POWER_OFF;
			prev = ret;

			if (!ret && ndev->mode == NOTIFY_HOST_MODE) {
				host_state_notify(ndev,
						NOTIFY_HOST_OVERCURRENT);
				pr_err("host_notifier usbhostd: overcurrent\n");
				break;
			}
		}
	}

	ninfo->thread_remove = 1;

	pr_info("host_notifier usbhostd: exit %d\n", ret);
	return 0;
}

static int start_usbhostd_thread(void)
{
	if (!ninfo.th) {
		pr_info("host_notifier: start usbhostd thread\n");

		init_waitqueue_head(&ninfo.delay_wait);
		ninfo.thread_remove = 0;
		ninfo.th = kthread_run(currentlimit_thread,
				&ninfo, "usbhostd");

		if (IS_ERR(ninfo.th)) {
			pr_err("host_notifier: Unable to start usbhostd\n");
			ninfo.th = NULL;
			ninfo.thread_remove = 1;
			return -1;
		}
		host_state_notify(&ninfo.pdata->ndev, NOTIFY_HOST_ADD);
		wake_lock(&ninfo.wlock);

	} else
		pr_info("host_notifier: usbhostd already started!\n");

	return 0;
}

static int stop_usbhostd_thread(void)
{
	if (ninfo.th) {
		pr_info("host_notifier: stop thread\n");

		if (!ninfo.thread_remove)
			kthread_stop(ninfo.th);

		ninfo.th = NULL;
		host_state_notify(&ninfo.pdata->ndev, NOTIFY_HOST_REMOVE);
		wake_unlock(&ninfo.wlock);
	} else
		pr_info("host_notifier: no thread\n");

	return 0;
}

int start_usbhostd_wakelock(void)
{
	pr_info("host_notifier: start usbhostd wakelock\n");
	wake_lock(&ninfo.wlock);

	return 0;
}

int stop_usbhostd_wakelock(void)
{
	pr_info("host_notifier: stop usbhostd wakelock\n");
	wake_unlock(&ninfo.wlock);

	return 0;
}


#ifdef CONFIG_MACH_P4NOTE
void host_notifier_enable_irq(void)
{
	pr_info("host_notifier: %s\n", __func__);
	enable_irq(ninfo.currentlimit_irq);
}

void host_notifier_disable_irq(void)
{
	pr_info("host_notifier: %s\n", __func__);
	disable_irq(ninfo.currentlimit_irq);
}
#endif
static int start_usbhostd_notify(void)
{
	pr_info("host_notifier: start usbhostd notify\n");
#ifdef CONFIG_MACH_P4NOTE
	host_notifier_enable_irq();
#endif
	host_state_notify(&ninfo.pdata->ndev, NOTIFY_HOST_ADD);
	wake_lock(&ninfo.wlock);

	return 0;
}

static int stop_usbhostd_notify(void)
{
	pr_info("host_notifier: stop usbhostd notify\n");
#ifdef CONFIG_MACH_P4NOTE
	host_notifier_disable_irq();
#endif

	host_state_notify(&ninfo.pdata->ndev, NOTIFY_HOST_REMOVE);
	wake_unlock(&ninfo.wlock);

	return 0;
}

static int check_usb_block(int disable)
{
	if(ninfo.pdata->ndev.mode == NOTIFY_HOST_MODE)
	{
		switch(disable){
			case NOTIFY_BLOCK_TYPE_ALL:
			case NOTIFY_BLOCK_TYPE_HOST:
				#if defined(CONFIG_USB_EXYNOS5_USB3_DRD_CH0)
					exynos_drd_switch_id_event(&exynos5_device_usb3_drd0, 1);
				#else
					exynos_drd_switch_id_event(&exynos5_device_usb3_drd1, 1);
				#endif
				if(ninfo.pdata->usbhostd_stop)
					ninfo.pdata->usbhostd_stop();
				ninfo.pdata->booster(0);
				return 1;
				
			default:
				if(ninfo.pdata->block_type==NOTIFY_BLOCK_TYPE_ALL || ninfo.pdata->block_type==NOTIFY_BLOCK_TYPE_HOST){
					ninfo.pdata->booster(1);
					#if defined(CONFIG_USB_EXYNOS5_USB3_DRD_CH0)
						exynos_drd_switch_id_event(&exynos5_device_usb3_drd0, 0);
					#else
					exynos_drd_switch_id_event(&exynos5_device_usb3_drd1, 0);
					#endif
					if(ninfo.pdata->usbhostd_start)
						ninfo.pdata->usbhostd_start();
				}
		}
	}
	return 0;
	
}

static int set_usb_disable(struct usb_notify_dev *udev, int disable)
{
	if(check_usb_block(disable)){
		pr_info("check_usb_block : event = %d, ninfo.block_type = %d\n",disable,ninfo.pdata->block_type);
		host_state_notify(&ninfo.pdata->ndev, NOTIFY_HOST_BLOCK);
	}
	ninfo.pdata->block_type = disable;
	return 0;
}

static void host_notifier_booster(int enable)
{
	pr_info("host_notifier: booster %s\n", enable ? "ON" : "OFF");
#ifdef CONFIG_MACH_P4NOTE
	if (enable)
		host_notifier_enable_irq();
	else
		host_notifier_disable_irq();
#endif
	ninfo.pdata->booster(enable);

	if (ninfo.pdata->thread_enable) {
		if (enable)
			start_usbhostd_thread();
		else
			stop_usbhostd_thread();
	}
}

static irqreturn_t currentlimit_irq_thread(int irq, void *data)
{
	struct host_notifier_info *hostinfo = data;
	struct host_notify_dev *ndev = &hostinfo->pdata->ndev;
	int gpio = hostinfo->pdata->gpio;
	int prev = ndev->booster;
	int ret = 0;

	ret = gpio_get_value(gpio);
	pr_info("currentlimit_irq_thread gpio : %d, value : %d\n", gpio, ret);

	if (prev != ret) {
		pr_info("host_notifier currentlimit_irq_thread: gpio %d = %s\n",
				gpio, ret ? "HIGH" : "LOW");
		ndev->booster = ret ?
			NOTIFY_POWER_ON : NOTIFY_POWER_OFF;
		prev = ret;

		if (!ret && ndev->mode == NOTIFY_HOST_MODE) {
			host_state_notify(ndev,
					NOTIFY_HOST_OVERCURRENT);
			pr_err("host_notifier currentlimit_irq_thread: overcurrent\n");
		}
	}
	return IRQ_HANDLED;
}

static int currentlimit_irq_init(struct host_notifier_info *hostinfo)
{
	int ret = 0;

	hostinfo->currentlimit_irq = gpio_to_irq(hostinfo->pdata->gpio);

	ret = request_threaded_irq(hostinfo->currentlimit_irq, NULL,
			currentlimit_irq_thread,
			IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
			"overcurrent_detect", hostinfo);
	if (ret)
		pr_info("host_notifier: %s return : %d\n", __func__, ret);
#ifdef CONFIG_MACH_P4NOTE
	host_notifier_disable_irq();
#endif

	return ret;
}

static void currentlimit_irq_work(struct work_struct *work)
{
	int retval;
	struct host_notifier_info *hostinfo = container_of(work,
			struct host_notifier_info, current_dwork.work);

	retval = currentlimit_irq_init(hostinfo);

	if (retval)
		pr_err("host_notifier: %s retval : %d\n", __func__, retval);
	return;
}

#if defined(CONFIG_HAS_EARLYSUSPEND) && defined(CONFIG_FAST_BOOT)
static bool restart_hostd;
static void host_notifier_early_suspend(struct early_suspend *h)
{
	if (fake_shut_down) {
		pr_info("%s: fake shut down ", __func__);
		host_notifier_booster(0);
		stop_usbhostd_wakelock();
		restart_hostd = true;
	}
}

static void host_notifier_late_resume(struct early_suspend *h)
{
	if (restart_hostd) {
		pr_info("%s: fake shut down ", __func__);
		restart_hostd = false;
		if (ninfo.pdata->is_host_working) {
			host_notifier_booster(1);
			start_usbhostd_wakelock();
		}
	}
}
#endif

static int host_notifier_probe(struct platform_device *pdev)
{
	int ret = 0;

	if (pdev && pdev->dev.platform_data)
		ninfo.pdata = pdev->dev.platform_data;
	else {
		pr_err("host_notifier: platform_data is null.\n");
		return -ENODEV;
	}

	dev_info(&pdev->dev, "notifier_probe\n");

	if (ninfo.pdata->thread_enable) {
		ret = gpio_request(ninfo.pdata->gpio, "host_notifier");
		if (ret) {
			dev_err(&pdev->dev, "failed to request %d\n",
				ninfo.pdata->gpio);
			return -EPERM;
		}
		gpio_direction_input(ninfo.pdata->gpio);
		dev_info(&pdev->dev, "gpio = %d\n", ninfo.pdata->gpio);

		ninfo.pdata->ndev.set_booster = host_notifier_booster;
		ninfo.pdata->usbhostd_start = start_usbhostd_thread;
		ninfo.pdata->usbhostd_stop = stop_usbhostd_thread;
	} else if (ninfo.pdata->irq_enable) {
		INIT_DELAYED_WORK(&ninfo.current_dwork, currentlimit_irq_work);
		schedule_delayed_work(&ninfo.current_dwork,
				msecs_to_jiffies(10000));
		ninfo.pdata->ndev.set_booster = host_notifier_booster;
		ninfo.pdata->usbhostd_start = start_usbhostd_notify;
		ninfo.pdata->usbhostd_stop = stop_usbhostd_notify;
	} else {
		ninfo.pdata->ndev.set_booster = host_notifier_booster;
		ninfo.pdata->usbhostd_start = start_usbhostd_notify;
		ninfo.pdata->usbhostd_stop = stop_usbhostd_notify;
	}

	ret = host_notify_dev_register(&ninfo.pdata->ndev);
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to host_notify_dev_register\n");
		return ret;
	}
	ninfo.udev.name = "usb_control";
	ninfo.udev.set_disable = set_usb_disable;
	ret = usb_notify_dev_register(&ninfo.udev);
	if (ret < 0) {
		pr_err("usb_notify_dev_register is failed\n");
		return ret;
	}
	ninfo.pdata->block_type = NOTIFY_BLOCK_TYPE_NONE;

	ovc_init(&ninfo);

#if defined(CONFIG_HAS_EARLYSUSPEND) && defined(CONFIG_FAST_BOOT)
	early_suspend.level = EARLY_SUSPEND_LEVEL_DISABLE_FB - 1;
	early_suspend.suspend = host_notifier_early_suspend;
	early_suspend.resume = host_notifier_late_resume;
	register_early_suspend(&early_suspend);
#endif
	wake_lock_init(&ninfo.wlock, WAKE_LOCK_SUSPEND, "hostd");

	return 0;
}

static int host_notifier_remove(struct platform_device *pdev)
{
#if defined(CONFIG_HAS_EARLYSUSPEND) && defined(CONFIG_FAST_BOOT)
	unregister_early_suspend(&early_suspend);
#endif
	/* gpio_free(ninfo.pdata->gpio); */
	host_notify_dev_unregister(&ninfo.pdata->ndev);
	wake_lock_destroy(&ninfo.wlock);
	return 0;
}

static struct platform_driver host_notifier_driver = {
	.probe		= host_notifier_probe,
	.remove		= host_notifier_remove,
	.driver		= {
		.name	= "host_notifier",
		.owner	= THIS_MODULE,
	},
};


static int __init host_notifier_init(void)
{
	return platform_driver_register(&host_notifier_driver);
}

static void __init host_notifier_exit(void)
{
	platform_driver_unregister(&host_notifier_driver);
}

module_init(host_notifier_init);
module_exit(host_notifier_exit);

MODULE_AUTHOR("Hyuk Kang <hyuk78.kang@samsung.com>");
MODULE_DESCRIPTION("USB Host notifier");
MODULE_LICENSE("GPL");
