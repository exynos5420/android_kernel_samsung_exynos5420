/*
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/lcd.h>
#include <plat/cpu.h>
#include <plat/clock.h>
#include <plat/devs.h>
#include <plat/dsim.h>
#include <plat/fb.h>
#include <plat/fb-core.h>
#include <plat/gpio-cfg.h>
#include <plat/regs-fb-v4.h>
#include <plat/mipi_dsi.h>
#include <plat/regs-mipidsim.h>
#include <mach/map.h>
#include <asm/system_info.h>
#include <linux/irq.h>


#define GPIO_MLCD_RST		EXYNOS5420_GPG1(2)
/* #define GPIO_PSR_TE		EXYNOS5420_GPJ4(0) */
//#define GPIO_ERR_FG		EXYNOS5420_GPG1(5)

static int err_fg_irq = -EINVAL;

unsigned int lcdtype;
EXPORT_SYMBOL(lcdtype);

static int __init lcdtype_setup(char *str)
{
	get_option(&str, &lcdtype);
	return 1;
}
__setup("lcdtype=", lcdtype_setup);

phys_addr_t bootloaderfb_start;
phys_addr_t bootloaderfb_size = 2560 * 1600 * 4;

static int __init bootloaderfb_start_setup(char *str)
{
	get_option(&str, &bootloaderfb_start);
	//bootloaderfb_start = 0; /* disable for copying bootloaderfb */
	return 1;
}
__setup("s3cfb.bootloaderfb=", bootloaderfb_start_setup);


static void klimt_fimd_gpio_setup_24bpp(void)
{
	unsigned int reg = 0;

	/*
	 * Set DISP1BLK_CFG register for Display path selection
	 *
	 * FIMD of DISP1_BLK Bypass selection : DISP1BLK_CFG[15]
	 * ---------------------
	 *  0 | MIE/MDNIE
	 *  1 | FIMD : selected
	 */
	reg = __raw_readl(S3C_VA_SYS + 0x0214);
	reg &= ~(1 << 15);	/* To save other reset values */
	reg |= (1 << 15);
	__raw_writel(reg, S3C_VA_SYS + 0x0214);

	/*
	 * Set DISP1BLK_CFG register for Display path selection
	 *
	 * MIC of DISP1_BLK Bypass selection: DISP1BLK_CFG[11]
	 * --------------------
	 *  0 | MIC
	 *  1 | Bypass : selected
	 */
	reg = __raw_readl(S3C_VA_SYS + 0x0214);
	reg &= ~(1 << 11);	/* Select MIC */
	__raw_writel(reg, S3C_VA_SYS + 0x0214);
}

static int lcd_power_on(struct lcd_device *ld, int enable)
{
	struct regulator *regulator_17;
	struct regulator *regulator_28;
	struct regulator *regulator_29;

	pr_debug(" Klimt LCD %s  : enable = %d\n", __func__, enable);

	regulator_17 = regulator_get(NULL, "vci_3.0v");
	regulator_28 = regulator_get(NULL, "vdd3_1.8v");
	regulator_29 = regulator_get(NULL, "vddr_1.6v");

	if (enable) {
		regulator_enable(regulator_28);
		regulator_enable(regulator_17);
		usleep_range(5000, 6000);
		regulator_enable(regulator_29);
		msleep(30);  /* EN -> LP11, 46ms */
	} else {
		regulator_disable(regulator_29);
		usleep_range(5000, 6000);
		regulator_disable(regulator_17);
		regulator_disable(regulator_28);
		gpio_request_one(GPIO_MLCD_RST, GPIOF_OUT_INIT_LOW, "GPG1");
		gpio_free(GPIO_MLCD_RST);
	}
	regulator_put(regulator_17);
	regulator_put(regulator_28);
	regulator_put(regulator_29);
	return 0;
}

static int lcd_reset(struct lcd_device *ld)
{
	int timeout = 10;

	pr_debug(" Klimt %s\n", __func__);

	gpio_request_one(GPIO_MLCD_RST, GPIOF_OUT_INIT_HIGH, "GPG1");
	usleep_range(1000, 2000);
	gpio_set_value(GPIO_MLCD_RST, 0);
	usleep_range(1000, 2000);
	gpio_set_value(GPIO_MLCD_RST, 1);
	usleep_range(10000, 15000);

	gpio_free(GPIO_MLCD_RST);

	return 0;
}

static struct lcd_platform_data s6e3ha1_platform_data = {
	.power_on = lcd_power_on,
	.reset = lcd_reset,
	.pdata = &err_fg_irq
};

#define SMDK5420_HBP 10
#define SMDK5420_HFP 10
#define SMDK5420_HSP 5
#define SMDK5420_VBP 4
#define SMDK5420_VFP 11
#define SMDK5420_VSW 1
#define SMDK5420_XRES 1600
#define SMDK5420_YRES 2560
#define SMDK5420_VIRTUAL_X 1600
#define SMDK5420_VIRTUAL_Y (2560*2)
#define SMDK5420_WIDTH 113 /* 112.8 */
#define SMDK5420_HEIGHT 180 /* 180.48 */
#define SMDK5420_MAX_BPP 32
#define SMDK5420_DEFAULT_BPP 24

static struct s3c_fb_pd_win klimt_fb_win0 = {
	.win_mode = {
		.left_margin	= SMDK5420_HBP,
		.right_margin	= SMDK5420_HFP,
		.upper_margin	= SMDK5420_VBP,
		.lower_margin	= SMDK5420_VFP,
		.hsync_len	= SMDK5420_HSP,
		.vsync_len	= SMDK5420_VSW,
		.xres		= SMDK5420_XRES,
		.yres		= SMDK5420_YRES,
	},
	.virtual_x		= SMDK5420_VIRTUAL_X,
	.virtual_y		= SMDK5420_VIRTUAL_Y,
	.max_bpp		= SMDK5420_MAX_BPP,
	.default_bpp		= SMDK5420_DEFAULT_BPP,
	.width			= SMDK5420_WIDTH,
	.height			= SMDK5420_HEIGHT,
};

static struct s3c_fb_platdata klimt_lcd1_pdata __initdata = {
	.win[0]		= &klimt_fb_win0,
	.win[1]		= &klimt_fb_win0,
	.win[2]		= &klimt_fb_win0,
	.win[3]		= &klimt_fb_win0,
	.win[4]		= &klimt_fb_win0,
	.default_win	= 0,
	.vidcon0	= VIDCON0_VIDOUT_RGB | VIDCON0_PNRMODE_RGB,
	.vidcon1	= VIDCON1_INV_VCLK,
	.setup_gpio	= klimt_fimd_gpio_setup_24bpp,
	.ip_version	= EXYNOS5_813,
	.dsim_on	= s5p_mipi_dsi_enable_by_fimd,
	.dsim_off	= s5p_mipi_dsi_disable_by_fimd,
	.dsim_clk_on	= s5p_mipi_dsi_clk_enable_by_fimd,
	.dsim_clk_off	= s5p_mipi_dsi_clk_disable_by_fimd,
	.dsim1_device   = &s5p_device_mipi_dsim1.dev,
#ifdef CONFIG_FB_HW_TRIGGER
	.dsim_get_state = lcd_get_mipi_state,
#endif
};

#define DSIM_L_MARGIN SMDK5420_HBP
#define DSIM_R_MARGIN SMDK5420_HFP
#define DSIM_UP_MARGIN SMDK5420_VBP
#define DSIM_LOW_MARGIN SMDK5420_VFP
#define DSIM_HSYNC_LEN SMDK5420_HSP
#define DSIM_VSYNC_LEN SMDK5420_VSW
#define DSIM_WIDTH SMDK5420_XRES
#define DSIM_HEIGHT SMDK5420_YRES

static struct mipi_dsim_lcd_config dsim_lcd_info = {
	.rgb_timing.left_margin		= DSIM_L_MARGIN,
	.rgb_timing.right_margin	= DSIM_R_MARGIN,
	.rgb_timing.upper_margin	= DSIM_UP_MARGIN,
	.rgb_timing.lower_margin	= DSIM_LOW_MARGIN,
	.rgb_timing.hsync_len		= DSIM_HSYNC_LEN,
	.rgb_timing.vsync_len		= DSIM_VSYNC_LEN,
	.lcd_size.width			= DSIM_WIDTH,
	.lcd_size.height		= DSIM_HEIGHT,
	.rgb_timing.stable_vfp		= 1,
	.rgb_timing.cmd_allow		= 6,
	.cpu_timing.cs_setup		= 1,
	.cpu_timing.wr_setup		= 0,
	.cpu_timing.wr_act		= 1,
	.cpu_timing.wr_hold		= 0,
	.mipi_ddi_pd			= &s6e3ha1_platform_data,
};

static struct mipi_dsim_config dsim_info = {
	.e_interface	= DSIM_VIDEO,
	.e_pixel_format = DSIM_24BPP_888,

	.eot_disable	= true,

	.hse = false,
	.hfp = true,
	.hbp = false,
	.hsa = false,

	.e_no_data_lane = DSIM_DATA_LANE_4,
	.e_byte_clk	= DSIM_PLL_OUT_DIV8,
	.e_burst_mode	= DSIM_BURST,

	/* 1024Mbps */
	.p = 3,
	.m = 64,
	.s = 0,

	/* D-PHY PLL stable time spec :min = 200usec ~ max 400usec */
	.pll_stable_time = DPHY_PLL_STABLE_TIME,

	.esc_clk = 16 * MHZ, /* escape clk : 8MHz */

	/* stop state holding counter after bta change count 0 ~ 0xfff */
	.stop_holding_cnt = 0x10,
	.bta_timeout = 0xff,		/* bta timeout 0 ~ 0xff */
	.rx_timeout = 0xffff,		/* lp rx timeout 0 ~ 0xffff */

	.dsim_ddi_pd = &s6e3ha1_mipi_lcd_driver,
};

static struct s5p_platform_mipi_dsim dsim_platform_data = {
	.clk_name		= "dsim1",
	.dsim_config		= &dsim_info,
	.dsim_lcd_config	= &dsim_lcd_info,

	.mipi_power		= NULL,
	.init_d_phy		= s5p_dsim_init_d_phy,
	.get_fb_frame_done	= NULL,
	.trigger		= NULL,

};


static const char * const keep_clock_arr[] = {
	"lcd",
	"axi_disp1",
	"aclk_200_disp1",
	"mout_spll",
	"sclk_mipi1"
};

/* Keep on clock of FIMD during boot time  */
static int keep_lcd_clk(struct device *dev, int en)
{
	struct clk *lcd_clk;
	int i;

	for (i = 0; i < ARRAY_SIZE(keep_clock_arr); i++) {
		lcd_clk = clk_get(dev, keep_clock_arr[i]);
		if (IS_ERR(lcd_clk)) {
			pr_err("failed to get %s for keep screen on\n",
					keep_clock_arr[i]);
		} else {
			if (en)
				clk_enable(lcd_clk);
			else
				clk_disable(lcd_clk);

			clk_put(lcd_clk);
		}
	}

	return 0;
}

static int __init restore_lcd_clk_late_init(void)
{
	keep_lcd_clk(&s5p_device_fimd1.dev, 0);
	return 0;
}

late_initcall_sync(restore_lcd_clk_late_init);

static struct platform_device *klimt_display_devices[] __initdata = {
	&s5p_device_mipi_dsim1,
	&s5p_device_mic,
	&s5p_device_fimd1,
};

void __init exynos5_universal5420_display_init(void)
{
	struct resource *res;
	struct clk *mout_mdnie1;
	struct clk *mout_mpll;

	gpio_request(GPIO_ERR_FG, "ERR_FG");
	s3c_gpio_setpull(GPIO_ERR_FG, S3C_GPIO_PULL_DOWN);
	s5p_register_gpio_interrupt(GPIO_ERR_FG);
	err_fg_irq = gpio_to_irq(GPIO_ERR_FG);
	irq_set_irq_type(err_fg_irq, IRQF_TRIGGER_RISING);
	s3c_gpio_cfgpin(GPIO_ERR_FG, S3C_GPIO_SFN(0xF));


#if defined(CONFIG_FB_HW_TRIGGER)
	gpio_request(GPIO_PSR_TE, "PSR_TE");
	s3c_gpio_cfgpin(GPIO_PSR_TE, S3C_GPIO_SFN(2));
#endif

	s5p_dsim1_set_platdata(&dsim_platform_data);
	s5p_fimd1_set_platdata(&klimt_lcd1_pdata);
	s5p_mic_set_platdata(&klimt_fb_win0);

	platform_add_devices(klimt_display_devices,
			ARRAY_SIZE(klimt_display_devices));

	mout_mdnie1 = clk_get(NULL, "mout_mdnie1");
	if ((IS_ERR(mout_mdnie1)))
		pr_err("Can't get clock[%s]\n", "mout_mdnie1");

	mout_mpll = clk_get(NULL, "mout_mpll");
	if ((IS_ERR(mout_mpll)))
		pr_err("Can't get clock[%s]\n", "mout_mpll");

	if (mout_mdnie1 && mout_mpll)
		clk_set_parent(mout_mdnie1, mout_mpll);

	if (mout_mdnie1)
		clk_put(mout_mdnie1);
	if (mout_mpll)
		clk_put(mout_mpll);

	exynos5_fimd1_setup_clock(&s5p_device_fimd1.dev,
			"sclk_fimd", "mout_mdnie1", 266 * MHZ);

	keep_lcd_clk(&s5p_device_fimd1.dev, 1);

	res = platform_get_resource(&s5p_device_fimd1, IORESOURCE_MEM, 1);
	if (res) {
		res->start = bootloaderfb_start;
		res->end = res->start + bootloaderfb_size - 1;
		pr_info("bootloader fb located at %8X-%8X\n", res->start, res->end);
	} else {
		pr_err("failed to find bootloader fb resource\n");
	}
}

