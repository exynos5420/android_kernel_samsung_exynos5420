/*
 * Copyright (C) 2011 Samsung Electronics Co.Ltd
 * Author: Joonyoung Shim <jy0922.shim@samsung.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#ifndef __PLAT_SAMSUNG_USB_PHY_H
#define __PLAT_SAMSUNG_USB_PHY_H __FILE__

enum s5p_usb_phy_type {
	S5P_USB_PHY_DEVICE,
	S5P_USB_PHY_HOST,
	S5P_USB_PHY_DRD,
};

extern int s5p_usb_phy_init(struct platform_device *pdev, int type);
extern int s5p_usb_phy_exit(struct platform_device *pdev, int type);
extern int s5p_usb_phy_tune(struct platform_device *pdev, int type);
extern int s5p_usb_phy_suspend(struct platform_device *pdev, int type);
extern int s5p_usb_phy_resume(struct platform_device *pdev, int type);
extern int exynos5_usb_phy_crport_ctrl(struct platform_device *pdev,
					u32 addr, u32 data);
extern int exynos_check_usb_op(void);
#if defined(CONFIG_LINK_DEVICE_HSIC)
extern int usb2phy_notifier(unsigned cmd, void *arg);
extern int register_usb2phy_notifier(struct notifier_block *nfb);
extern int unregister_usb2phy_notifier(struct notifier_block *nfb);

enum hsic_lpa_states {
	STATE_HSIC_LPA_ENTER,
	STATE_HSIC_LPA_WAKE,
	STATE_HSIC_LPA_PHY_INIT,
	STATE_HSIC_LPA_CHECK,
	STATE_HSIC_PHY_SHUTDOWN,
	STATE_HSIC_CHECK_HOSTWAKE,
};

#define is_cp_wait_for_resume() \
	(usb2phy_notifier(STATE_HSIC_CHECK_HOSTWAKE, NULL) == NOTIFY_BAD)
#endif

#if defined(CONFIG_MDM_HSIC_PM)
extern int usb2phy_notifier(unsigned cmd, void *arg);
extern int register_usb2phy_notifier(struct notifier_block *nfb);
extern int unregister_usb2phy_notifier(struct notifier_block *nfb);
#endif

#endif /* __PLAT_SAMSUNG_USB_PHY_H */
