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
#include <linux/pwm_backlight.h>
#include <linux/lp855x.h>
#include <linux/platform_data/lsl122dl01_edp_lcd.h>

#ifdef CONFIG_FB_S5P_MDNIE
#include <linux/mdnie.h>
#endif

#include <video/platform_lcd.h>
#include <video/s5p-dp.h>

#include <plat/backlight.h>
#include <plat/cpu.h>
#include <plat/clock.h>
#include <plat/devs.h>
#include <plat/dp.h>
#include <plat/fb.h>
#include <plat/fb-core.h>
#include <plat/gpio-cfg.h>
#include <plat/regs-fb-v4.h>

#include <mach/map.h>

#define GPIO_LCD_EN		EXYNOS5420_GPG1(2)
#define GPIO_DP_HPD		EXYNOS5420_GPX0(7)

#define GPIO_LCD_PWM_IN_18V	EXYNOS5420_GPB2(0)
#define GPIO_LED_BL_RST		EXYNOS5420_GPG1(1)

#if defined (CONFIG_N1A) || defined(CONFIG_N2A)
#define GPIO_LCDP_SDA_18V	EXYNOS5420_GPA2(2)
#define GPIO_LCDP_SCL_18V	EXYNOS5420_GPA2(3)
#else
#define GPIO_LCDP_SDA_18V	EXYNOS5420_GPA2(4)
#define GPIO_LCDP_SCL_18V	EXYNOS5420_GPA2(5)
#endif

#define LCD_POWER_OFF_TIME_US   (500 * USEC_PER_MSEC)

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

static ktime_t lcd_on_time;
static int v1_bl_enable;
static int v1_lcd_enable;

static void v1_lcd_on(void)
{
	s64 us = ktime_us_delta(lcd_on_time, ktime_get_boottime());

	if (v1_lcd_enable)
		return;

	if (us > LCD_POWER_OFF_TIME_US) {
		pr_warn("lcd on sleep time too long\n");
		us = LCD_POWER_OFF_TIME_US;
	}

	if (us > 0) {
		pr_debug(" %s : %lld us\n", __func__, us);
		usleep_range(us, us);
	}

	gpio_set_value(GPIO_LCD_EN, 1);
	v1_lcd_enable = 1;
	usleep_range(200000, 200000);
}

static void v1_lcd_off(void)
{
	gpio_set_value(GPIO_LCD_EN, 0);
	v1_lcd_enable = 0;
	lcd_on_time = ktime_add_us(ktime_get_boottime(), LCD_POWER_OFF_TIME_US);
#if 1
	usleep_range(300000, 300000);
#endif

}

static void v1_lcd_set_power(struct plat_lcd_data *pd,
				unsigned int power)
{
	if (power)
		v1_lcd_on();
	else
		v1_lcd_off();
}

static struct plat_lcd_data v1_dp_lcd_data = {
	.set_power	= v1_lcd_set_power,
};

static struct platform_device v1_dp_lcd = {
	.name	= "platform-lcd",
	.dev	= {
		.parent		= &s5p_device_fimd1.dev,
		.platform_data	= &v1_dp_lcd_data,
	},
};

static int v1_backlight_set_power(int power)
{
	if (power && v1_bl_enable)
		return 0;

	if(power) {
		gpio_set_value(GPIO_LED_BL_RST, 1);
		v1_bl_enable = 1;
		usleep_range(10000, 10000);
	} else {
		/* LED_BACKLIGHT_RESET: XCI1RGB_5 => GPG0_5 */
		gpio_set_value(GPIO_LED_BL_RST, 0);
		v1_bl_enable = 0;
	}

	return 0;
}

static void v1_backlight_on(void)
{
	/* usleep_range(97000, 97000); */
	v1_backlight_set_power(1);
}

static void v1_backlight_off(void)
{
	v1_backlight_set_power(0);
}

static void v1_fimd_gpio_setup_24bpp(void)
{
	unsigned int reg = 0;

	/* Set Hotplug detect for DP */
	gpio_request(GPIO_DP_HPD, "DP_HPD");
	s3c_gpio_cfgpin(GPIO_DP_HPD, S3C_GPIO_SFN(3));

	/*
	 * Set DISP1BLK_CFG register for Display path selection
	 *
	 * FIMD of DISP1_BLK Bypass selection : DISP1BLK_CFG[15]
	 * ---------------------
	 *  0 | MIE/MDNIE
	 *  1 | FIMD : selected
	 */

#ifdef CONFIG_FB_S5P_MDNIE
	/* SYSREG SETTING */
	reg = 0;
	reg = __raw_readl(S3C_VA_SYS + 0x0214);
	reg &= ~(1>>23); /*FIFO software Reset*/
	__raw_writel(reg, S3C_VA_SYS + 0x0214);

	reg &= ~(7 << 29);	/*sync*/
	reg &= ~(1 << 27);	/*DISP0_SRC not userd*/
	reg &= ~(3 << 24);	/*VT_DIP1 - RGB*/
	reg |= (1 << 23);	/*FIFORST_DISP -1*/
	reg &= ~(1 << 15);	/*FIMDBYPASS_DISP1 -0*/
	reg |= (1 << 14);	/*MIE_DISP1 - MDNIE -1 */
	reg |= (1 << 0);	/*MIE_LBLK1 - MDNIE -1*/
	__raw_writel(reg, S3C_VA_SYS + 0x0214);
#else
	reg = __raw_readl(S3C_VA_SYS + 0x0214);
	reg &= ~(1 << 15);	/* To save other reset values */
	reg |= (1 << 15);
	__raw_writel(reg, S3C_VA_SYS + 0x0214);
#endif

	/* Reference clcok selection for DPTX_PHY: PAD_OSC_IN */
	reg = __raw_readl(S3C_VA_SYS + 0x04d4);
	reg &= ~(1 << 0);
	__raw_writel(reg, S3C_VA_SYS + 0x04d4);

	/* DPTX_PHY: XXTI */
	reg = __raw_readl(S3C_VA_SYS + 0x04d8);
	reg &= ~(1 << 3);
	__raw_writel(reg, S3C_VA_SYS + 0x04d8);

	/*
	 * Set DISP1BLK_CFG register for Display path selection
	 *
	 * MIC of DISP1_BLK Bypass selection: DISP1BLK_CFG[11]
	 * --------------------
	 *  0 | MIC
	 *  1 | Bypass : selected
	 */
	reg = __raw_readl(S3C_VA_SYS + 0x0214);
	reg |= (1 << 11);
	__raw_writel(reg, S3C_VA_SYS + 0x0214);
}
#if defined(CONFIG_N1A) || defined(CONFIG_N2A)
	#define SMDK5420_HBP 33
	#define SMDK5420_HFP 40
	#define SMDK5420_VBP 37
	#define SMDK5420_VFP 3
	#define SMDK5420_HSP 22
	#define SMDK5420_VSW 6
	#define SMDK5420_XRES 2560
	#define SMDK5420_YRES 1600
	#define SMDK5420_FREQ 61
#else
	#define SMDK5420_HBP 70
	#define SMDK5420_HFP 62
	#define SMDK5420_HSP 22
	#define SMDK5420_VBP 21
	#define SMDK5420_VFP 6
	#define SMDK5420_VSW 6
	#define SMDK5420_XRES 2560
	#define SMDK5420_YRES 1600
	#define SMDK5420_FREQ 60
#endif

static struct s3c_fb_pd_win v1_fb_win0 = {
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
	.virtual_x		= 2560,
	.virtual_y		= 1600 * 2,
	.max_bpp		= 32,
	.default_bpp		= 24,
#if defined(CONFIG_N1A) || defined(CONFIG_N2A)
	.width			= 216,
	.height			= 135,
#else
	.width			= 262,
	.height			= 164,
#endif
};

static struct s3c_fb_platdata v1_lcd1_pdata __initdata = {
	.win[0]		= &v1_fb_win0,
	.win[1]		= &v1_fb_win0,
	.win[2]		= &v1_fb_win0,
	.win[3]		= &v1_fb_win0,
	.win[4]		= &v1_fb_win0,
	.default_win	= 0,
	.vidcon0	= VIDCON0_VIDOUT_RGB | VIDCON0_PNRMODE_RGB,
	.vidcon1	= 0,
	.setup_gpio	= v1_fimd_gpio_setup_24bpp,
	.ip_version	= EXYNOS5_813,
	.backlight_off  = v1_backlight_off,
	.lcd_off	= v1_lcd_off,
};

static struct fb_videomode dp_video_config = {
	.left_margin	= SMDK5420_HBP,
	.right_margin	= SMDK5420_HFP,
	.upper_margin	= SMDK5420_VBP,
	.lower_margin	= SMDK5420_VFP,
	.hsync_len	= SMDK5420_HSP,
	.vsync_len	= SMDK5420_VSW,
	.xres		= SMDK5420_XRES,
	.yres		= SMDK5420_YRES,
	.refresh	= SMDK5420_FREQ,
};

static struct video_info v1_dp_config = {
	.name			= "WQXGA(2560x1600) LCD",

	.h_sync_polarity	= 0,
	.v_sync_polarity	= 0,
	.interlaced		= 0,

	.color_space		= COLOR_RGB,
	.dynamic_range		= VESA,
	.ycbcr_coeff		= COLOR_YCBCR601,
	.color_depth		= COLOR_8,

	.link_rate		= LINK_RATE_2_70GBPS,
	.lane_count		= LANE_COUNT4,
#ifdef CONFIG_S5P_DP_PSR
	.video_mode = &dp_video_config,
#endif
};

static struct s5p_dp_platdata v1_dp_data __initdata = {
	.video_info	= &v1_dp_config,
	.phy_init	= s5p_dp_phy_init,
	.phy_exit	= s5p_dp_phy_exit,
	.backlight_off	= v1_backlight_off,
	.lcd_on		= v1_lcd_on,
	.lcd_off	= v1_lcd_off,
};

#ifdef CONFIG_FB_S5P_MDNIE
static struct platform_mdnie_data mdnie_data = {
	.display_type	= -1,
};

struct platform_device mdnie_device = {
	.name	= "mdnie",
	.id	= -1,
	.dev	= {
		.parent		= &s5p_device_fimd1.dev,
		.platform_data = &mdnie_data,
	},
};

static void __init mdnie_device_register(void)
{
	int ret;

	ret = platform_device_register(&mdnie_device);
	if (ret)
		printk(KERN_ERR "failed to register mdnie device: %d\n",
				ret);
}
#endif

/* Keep on clock of FIMD during boot time  */
static int keep_lcd_clk(struct device *dev)
{
	struct device *dp_dev = &s5p_device_dp.dev;
	struct clk *lcd_clk;

	/* Use the init name until the kobject becomes available */
	dp_dev->init_name = s5p_device_dp.name;

	lcd_clk = clk_get(dev, "lcd");
	if (IS_ERR(lcd_clk)) {
		pr_err("failed to get fimd clock for keep screen on\n");
	} else {
		clk_enable(lcd_clk);
		clk_put(lcd_clk);
	}

	lcd_clk = clk_get(dev, "axi_disp1");
	if (IS_ERR(lcd_clk)) {
		pr_err("failed to get axi_disp1 clock for keep screen on\n");
	} else {
		clk_enable(lcd_clk);
		clk_put(lcd_clk);
	}

	lcd_clk = clk_get(dev, "aclk_200_disp1");
	if (IS_ERR(lcd_clk)) {
		pr_err("failed to get aclk_200_disp1 clock for keep screen on\n");
	} else {
		clk_enable(lcd_clk);
		clk_put(lcd_clk);
	}

	lcd_clk = clk_get(dev, "mout_spll");
	if (IS_ERR(lcd_clk)) {
		pr_err("failed to get mout_spll clock for keep screen on\n");
	} else {
		clk_enable(lcd_clk);
		clk_put(lcd_clk);
	}

	lcd_clk = clk_get(dev, "sclk_mdnie_pwm1");
	if (IS_ERR(lcd_clk)) {
		pr_err("failed to get mout_spll clock for keep screen on\n");
	} else {
		clk_enable(lcd_clk);
		clk_put(lcd_clk);
	}

	lcd_clk = clk_get(dp_dev, "dp");
	if (IS_ERR(lcd_clk)) {
		pr_err("failed to get dp clock for keep screen on\n");
	} else {
		clk_enable(lcd_clk);
		clk_put(lcd_clk);
	}

	return 0;
}

static void restore_lcd_clk_init(struct device *dev)
{
	struct clk *lcd_clk;
	struct device *dp_dev = &s5p_device_dp.dev;

	lcd_clk = clk_get(dev, "lcd");
	if (IS_ERR(lcd_clk)) {
		pr_err("failed to get fimd clock for keep screen on\n");
	} else {
		clk_disable(lcd_clk);
		clk_put(lcd_clk);
	}

	lcd_clk = clk_get(dev, "axi_disp1");
	if (IS_ERR(lcd_clk)) {
		pr_err("failed to get axi_disp1 clock for keep screen on\n");
	} else {
		clk_disable(lcd_clk);
		clk_put(lcd_clk);
	}

	lcd_clk = clk_get(dev, "aclk_200_disp1");
	if (IS_ERR(lcd_clk)) {
		pr_err("failed to get aclk_200_disp1 clock for keep screen on\n");
	} else {
		clk_disable(lcd_clk);
		clk_put(lcd_clk);
	}

	lcd_clk = clk_get(dev, "mout_spll");
	if (IS_ERR(lcd_clk)) {
		pr_err("failed to get mout_spll clock for keep screen on\n");
		return;
	} else {
		clk_disable(lcd_clk);
		clk_put(lcd_clk);
	}

	lcd_clk = clk_get(dev, "sclk_mdnie_pwm1");
	if (IS_ERR(lcd_clk)) {
		pr_err("failed to get mout_spll clock for keep screen on\n");
		return;
	} else {
		clk_disable(lcd_clk);
		clk_put(lcd_clk);
	}

	lcd_clk = clk_get(dp_dev, "dp");
	if (IS_ERR(lcd_clk)) {
		pr_err("failed to get dp clock for keep screen on\n");
		return;
	} else {
		clk_disable(lcd_clk);
		clk_put(lcd_clk);
	}
}

static int v1_backlight_post_init(struct device *dev)
{
	struct clk *pwm_clk;

	s3c_gpio_cfgpin(GPIO_LCD_PWM_IN_18V, S3C_GPIO_SFN(2));

	restore_lcd_clk_init(&s5p_device_fimd1.dev);

	return 0;
}

static struct i2c_gpio_platform_data gpio_i2c_data24 = {
	.sda_pin = GPIO_LCDP_SDA_18V,
	.scl_pin = GPIO_LCDP_SCL_18V,
};

struct platform_device s3c_device_i2c24 = {
	.name = "i2c-gpio",
	.id = 24,
	.dev.platform_data = &gpio_i2c_data24,
};

#ifdef CONFIG_BACKLIGHT_LP855X
#define EPROM_CFG98_ADDR		0x98
#define EPROM_CFG1_ADDR		0xA1
#define EPROM_CFG3_ADDR		0xA3
#define EPROM_CFG5_ADDR		0xA5
#define EPROM_CFG8_ADDR		0xA8
#define EPROM_CFG9_ADDR		0xA9

static struct lp855x_rom_data lp8556_eprom_arr[] = {
	{EPROM_CFG98_ADDR, 0xA1},
#if defined(CONFIG_LCD_LSL122DL01_BATTERY)
	{EPROM_CFG1_ADDR, 0x3F},
#else
	{EPROM_CFG1_ADDR, 0x5F},
#endif
	{EPROM_CFG3_ADDR, 0x5E},
	{EPROM_CFG5_ADDR, 0x04},
	{EPROM_CFG8_ADDR, 0x00},
	{EPROM_CFG9_ADDR, 0xA0},
};

static struct lp855x_platform_data lp8856_bl_pdata = {
	.name		= "panel",
	.device_control	= (PWM_CONFIG(LP8556) | LP8556_FAST_CONFIG),
#if defined(CONFIG_N1A) || defined(CONFIG_N2A)
	.initial_brightness = 114,
#else
	.initial_brightness = 128,
#endif
	.pwm_id		= 0,
	.period_ns	= 100000,
#if defined(CONFIG_N1A) || defined(CONFIG_N2A)
	.min_brightness = 3,
	.max_brightness = 255,
	.lth_brightness = 3,
	.uth_brightness = 255,
#else
	.min_brightness = 2,
	.max_brightness = 255,
	.lth_brightness = 2,
	.uth_brightness = 255,
#endif
	.load_new_rom_data = 1,
	.size_program	= ARRAY_SIZE(lp8556_eprom_arr),
	.rom_data	= lp8556_eprom_arr,
	.set_power	= v1_backlight_set_power,
	.post_init_device = v1_backlight_post_init,
};
#endif

#ifdef CONFIG_S5P_DP_PSR
#if defined(CONFIG_V1A) || defined(CONFIG_V2A) || defined(CONFIG_CHAGALL)
	#define DEFAULT_PSR_HFREQ	98000
	#define DEFAULT_PSR_HFREQ_DATA	181
	#define HFREQ_DATA_MAX			241
	#define HFREQ_DATA_MIN			121
#else
	#define DEFAULT_PSR_HFREQ	0
	#define DEFAULT_PSR_HFREQ_DATA	0
	#define HFREQ_DATA_MAX			0
	#define HFREQ_DATA_MIN			0
#endif
#endif

static struct lsl122dl01_platform_data	lsl122dl01_pdata = {
#ifdef CONFIG_S5P_DP_PSR
	.psr_default_hfreq = DEFAULT_PSR_HFREQ,
	.psr_default_hfreq_data = DEFAULT_PSR_HFREQ_DATA,
	.psr_hfreq_data_max = HFREQ_DATA_MAX,
	.psr_hfreq_data_min = HFREQ_DATA_MIN,
#endif
	.dev = &s5p_device_dp.dev,

};

static struct i2c_board_info i2c_devs24_emul[] __initdata = {
#ifdef CONFIG_BACKLIGHT_LP855X
	{
		I2C_BOARD_INFO("lp8556", (0x58 >> 1)),
		.platform_data	= &lp8856_bl_pdata,
	},
#endif
#ifdef CONFIG_LCD_LSL122DL01
	{
		I2C_BOARD_INFO("lsl122dl01", 0x30),
		.platform_data = &lsl122dl01_pdata,
	},
#endif
};

static struct platform_device *v1_display_devices[] __initdata = {
	&s5p_device_fimd1,
	&v1_dp_lcd,
	&s5p_device_dp,
	&s3c_device_i2c24,
};

void __init exynos5_universal5420_display_init(void)
{
	struct resource *res;
	struct clk *mout_mdnie1;
	struct clk *mout_mpll;

	/* LCD_EN */
	gpio_request(GPIO_LCD_EN, "LCD_EN");
	/* gpio_request_one(GPIO_LCD_EN, GPIOF_OUT_INIT_HIGH, "LCD_EN"); */
	v1_lcd_enable = 1;

	s5p_dp_set_platdata(&v1_dp_data);
	s5p_fimd1_set_platdata(&v1_lcd1_pdata);

	platform_add_devices(v1_display_devices,
			ARRAY_SIZE(v1_display_devices));

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


#ifdef CONFIG_FB_S5P_MDNIE
	exynos5_fimd1_setup_clock(&s5p_device_fimd1.dev,
		"sclk_mdnie", "mout_mdnie1", 266 * MHZ);

	mdnie_device_register();
#endif
	keep_lcd_clk(&s5p_device_fimd1.dev);

#ifdef CONFIG_BACKLIGHT_LP855X
	/* LED_BACKLIGHT_RESET */
	gpio_request(GPIO_LED_BL_RST, "LED_BL_RST");
	/* gpio_request_one(GPIO_LED_BL_RST, GPIOF_OUT_INIT_HIGH, "LED_BL_RST"); */

	gpio_request(GPIO_LCD_PWM_IN_18V, "Backlight");
	/* s3c_gpio_cfgpin(GPIO_LCD_PWM_IN_18V, S3C_GPIO_SFN(2)); */

	platform_device_register(&s3c_device_timer[0]);

	v1_bl_enable = 1;
#endif

	i2c_register_board_info(24, i2c_devs24_emul,
				ARRAY_SIZE(i2c_devs24_emul));

	res = platform_get_resource(&s5p_device_fimd1, IORESOURCE_MEM, 1);
	if (res) {
		res->start = bootloaderfb_start;
		res->end = res->start + bootloaderfb_size - 1;
		pr_info("bootloader fb located at %8X-%8X\n", res->start, res->end);
	} else {
		pr_err("failed to find bootloader fb resource\n");
	}
}

