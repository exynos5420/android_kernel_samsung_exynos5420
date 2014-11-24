#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
//#include <mach/gpio-exynos4.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#include <plat/devs.h>
//#include <plat/ehci.h>
#include <linux/msm_charm.h>
#include <mach/mdm2.h>
#include "mdm_private.h"

/* Modem Interface GPIOs - MDM */
#define GPIO_MDM2AP_HSIC_READY		EXYNOS5420_GPX1(0)
#define GPIO_AP2MDM_STATUS		EXYNOS5420_GPJ4(1)
#define GPIO_MDM2AP_HSIC_PWR_ACTIVE	EXYNOS5420_GPX2(7)	/* AP2MDM_IPC2 */
//#define GPIO_WCN_PRIORITY		EXYNOS5420_GPF2(3)
#define GPIO_AP2MDM_ERR_FATAL		EXYNOS5420_GPG1(6)
#define GPIO_AP2MDM_PON_RESET_N		EXYNOS5420_GPG1(4)
#define GPIO_AP2MDM_WAKEUP		EXYNOS5420_GPG1(3)
#define GPIO_AP2MDM_SOFT_RESET		EXYNOS5420_GPM3(3)
#define GPIO_MDM2AP_STATUS		EXYNOS5420_GPX1(2)
#define GPIO_MDM2AP_HSIC_RESUME_REQ	EXYNOS5420_GPX3(3)	/* AP2MDM_IPC3 */
//#define GPIO_AP2MDM_VDDMIN		EXYNOS5420_GPX1(0)
//#define GPIO_MDM2AP_VDDMIN		EXYNOS5420_GPX1(1)
#define GPIO_MDM2AP_ERR_FATAL		EXYNOS5420_GPX2(6)
#define GPIO_AP2MDM_HSIC_PORT_ACTIVE	EXYNOS5420_GPG1(7)	/* AP2MDM_IPC1 */
#define GPIO_AP2MDM_PMIC_RESET_N	EXYNOS5420_GPG0(4)

static struct resource mdm_resources[] = {
	{
		.start	= GPIO_MDM2AP_ERR_FATAL,
		.end	= GPIO_MDM2AP_ERR_FATAL,
		.name	= "MDM2AP_ERRFATAL",
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= GPIO_AP2MDM_ERR_FATAL,
		.end	= GPIO_AP2MDM_ERR_FATAL,
		.name	= "AP2MDM_ERRFATAL",
		.flags	= IORESOURCE_IO,
	},
	{
		.start	= GPIO_MDM2AP_STATUS,
		.end	= GPIO_MDM2AP_STATUS,
		.name	= "MDM2AP_STATUS",
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= GPIO_AP2MDM_STATUS,
		.end	= GPIO_AP2MDM_STATUS,
		.name	= "AP2MDM_STATUS",
		.flags	= IORESOURCE_IO,
	},
	{
		.start	= GPIO_AP2MDM_PON_RESET_N,
		.end	= GPIO_AP2MDM_PON_RESET_N,
		.name	= "AP2MDM_SOFT_RESET",
		.flags	= IORESOURCE_IO,
	},
	{
		.start	= GPIO_AP2MDM_PMIC_RESET_N,
		.end	= GPIO_AP2MDM_PMIC_RESET_N,
		.name	= "AP2MDM_PMIC_PWR_EN",
		.flags	= IORESOURCE_IO,
	},
	{
		.start	= GPIO_AP2MDM_WAKEUP,
		.end	= GPIO_AP2MDM_WAKEUP,
		.name	= "AP2MDM_WAKEUP",
		.flags	= IORESOURCE_IO,
	},
	{
		.start	= GPIO_MDM2AP_HSIC_READY,
		.end	= GPIO_MDM2AP_HSIC_READY,
		.name	= "MDM2AP_PBLRDY",
		.flags	= IORESOURCE_IO,
	},
#ifdef CONFIG_SIM_DETECT
	{
		.start	= GPIO_SIM_DETECT,
		.end	= GPIO_SIM_DETECT,
		.name	= "SIM_DETECT",
		.flags	= IORESOURCE_IO,
	},
#endif

};

#ifdef CONFIG_MDM_HSIC_PM
static struct resource mdm_pm_resource[] = {
	{
		.start	= GPIO_AP2MDM_HSIC_PORT_ACTIVE,
		.end	= GPIO_AP2MDM_HSIC_PORT_ACTIVE,
		.name	= "AP2MDM_HSIC_ACTIVE",
		.flags	= IORESOURCE_IO,
	},
	{
		.start	= GPIO_MDM2AP_HSIC_PWR_ACTIVE,
		.end	= GPIO_MDM2AP_HSIC_PWR_ACTIVE,
		.name	= "MDM2AP_DEVICE_PWR_ACTIVE",
		.flags	= IORESOURCE_IO,
	},
	{
		.start	= GPIO_MDM2AP_HSIC_RESUME_REQ,
		.end	= GPIO_MDM2AP_HSIC_RESUME_REQ,
		.name	= "MDM2AP_RESUME_REQ",
		.flags	= IORESOURCE_IO,
	},
};

struct platform_device mdm_pm_device = {
	.name		= "mdm_hsic_pm0",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(mdm_pm_resource),
	.resource	= mdm_pm_resource,
};
#endif

static struct mdm_platform_data mdm_platform_data = {
	.mdm_version = "3.0",
	.ramdump_delay_ms = 3000,
	.early_power_on = 1,
	.sfr_query = 0,
	.vddmin_resource = NULL,
#ifdef CONFIG_USB_EHCI_S5P
	.peripheral_platform_device_ehci = &s5p_device_ehci,
#endif
#ifdef CONFIG_USB_OHCI_S5P
	.peripheral_platform_device_ohci = &s5p_device_ohci,
#endif
	.ramdump_timeout_ms = 120000,
#if (defined(CONFIG_MACH_P4NOTE) || defined(CONFIG_MACH_SP7160LTE) || defined(CONFIG_MACH_TAB3)) && defined(CONFIG_QC_MODEM) \
	&& defined(CONFIG_SIM_DETECT)
	.sim_polarity = 0,
#endif
#if (defined(CONFIG_MACH_GC1_USA_VZW) || defined(CONFIG_TARGET_LOCALE_EUR)) \
	&& defined(CONFIG_QC_MODEM) && defined(CONFIG_SIM_DETECT)
	.sim_polarity = 1,
#endif
};

struct platform_device mdm_device = {
	.name		= "mdm2_modem",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(mdm_resources),
	.resource	= mdm_resources,
};

static int __init init_mdm_modem(void)
{
	int ret;
	pr_info("%s: registering modem dev, pm dev\n", __func__);

#ifdef CONFIG_MDM_HSIC_PM
	ret = platform_device_register(&mdm_pm_device);
	if (ret < 0) {
		pr_err("%s: fail to register mdm hsic pm dev(err:%d)\n",
								__func__, ret);
		return ret;
	}
#endif
	mdm_device.dev.platform_data = &mdm_platform_data;
	ret = platform_device_register(&mdm_device);
	if (ret < 0) {
		pr_err("%s: fail to register mdm modem dev(err:%d)\n",
								__func__, ret);
		return ret;
	}
	return 0;
}
module_init(init_mdm_modem);
