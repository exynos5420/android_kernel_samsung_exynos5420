/*
 * Copyright (c) 2013 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/export.h>
#include <linux/platform_device.h>
#include <linux/gpio_keys.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/regulator/consumer.h>
#ifdef CONFIG_TOUCHSCREEN_ATMEL_MXTS
#include <linux/i2c/mxts.h>
#endif
#ifdef CONFIG_TOUCHSCREEN_SYNAPTICS_DSX_I2C
#include <linux/i2c/synaptics_rmi.h>
#endif
#ifdef CONFIG_INPUT_WACOM
#include <linux/wacom_i2c.h>
#endif
#ifdef CONFIG_INPUT_BOOSTER
#include <linux/input/input_booster.h>
#endif
#include <plat/iic.h>
#include <mach/hs-iic.h>
#include <plat/devs.h>
#include <plat/gpio-cfg.h>
#include <mach/gpio.h>
#include <mach/sec_debug.h>

#include "board-universal5420.h"

#if !defined(CONFIG_SAMSUNG_PRODUCT_SHIP)
#define TSP_DEBUG_LOG
#endif

#ifdef CONFIG_TOUCHSCREEN_ATMEL_MXTS
#define MXT_BOOT_ADDRESS				0x26
#define MXT_APP_ADDRESS				0x4A

#if defined(CONFIG_N2A)
#define MXT_FIRMWARE_NAME_REVISION	"mXT1664S_n.fw"
#else
#define MXT_FIRMWARE_NAME_REVISION	"mXT1664S_v.fw"
#endif

#if defined(CONFIG_V1A_WIFI)
#define MXT_PROJECT_NAME	"SM-P900"
#elif defined(CONFIG_V1A_3G)
#define MXT_PROJECT_NAME	"SM-P901"
#elif defined(CONFIG_V2A_WIFI)
#define MXT_PROJECT_NAME	"SM-T900"
#elif defined(CONFIG_V2A_3G)
#define MXT_PROJECT_NAME	"SM-T901"
#elif defined(CONFIG_N2A_WIFI)
#define MXT_PROJECT_NAME	"SM-T520"
#elif defined(CONFIG_N2A_3G)
#define MXT_PROJECT_NAME	"SM-T521"
#endif

static struct mxt_callbacks *charger_callbacks;
void tsp_charger_infom(bool en)
{
	if (charger_callbacks && charger_callbacks->inform_charger)
		charger_callbacks->inform_charger(charger_callbacks, en);
}

static void mxt_register_callback(void *cb)
{
	charger_callbacks = cb;
}

static bool mxt_read_chg(void)
{
	return gpio_get_value(GPIO_TOUCH_CHG);
}

static int mxt_power_on(void)
{
	struct regulator *regulator;

#if defined(TSP_DEBUG_LOG)
	printk(KERN_DEBUG "[TSP] %s\n", __func__);
#endif

	/* enable I2C pullup */
	regulator = regulator_get(NULL, "tsp_vdd_1.8v");
	if (IS_ERR(regulator)) {
		printk(KERN_ERR "[TSP] %s : regulator_get failed\n",
			__func__);
		return -EIO;
	}
	regulator_enable(regulator);
	regulator_put(regulator);

	/* enable XVDD */
	gpio_set_value(GPIO_TOUCH_EN_1, 1);
	msleep(1);
	gpio_set_value(GPIO_TOUCH_EN, 1);

	msleep(30);

	gpio_set_value(GPIO_TOUCH_RESET, 1);

	/* touch interrupt pin */
	s3c_gpio_cfgpin(GPIO_TOUCH_CHG, S3C_GPIO_SFN(0xf));
	s3c_gpio_setpull(GPIO_TOUCH_CHG, S3C_GPIO_PULL_NONE);

	msleep(100);

	return 0;
}

static int mxt_power_off(void)
{
	struct regulator *regulator;

#if defined(TSP_DEBUG_LOG)
	printk(KERN_DEBUG "[TSP] %s\n", __func__);
#endif

	gpio_set_value(GPIO_TOUCH_RESET, 0);

	/* disable XVDD */
	gpio_set_value(GPIO_TOUCH_EN, 0);
	usleep_range(3000, 3000);
	gpio_set_value(GPIO_TOUCH_EN_1, 0);
	usleep_range(3000, 3000);

	/* disable I2C pullup */
	regulator = regulator_get(NULL, "tsp_vdd_1.8v");
	if (IS_ERR(regulator)) {
		printk(KERN_ERR "[TSP] %s : regulator_get failed\n",
			__func__);
		return -EIO;
	}

	if (regulator_is_enabled(regulator))
		regulator_disable(regulator);
	else
		regulator_force_disable(regulator);
	regulator_put(regulator);

	/* touch interrupt pin */
	s3c_gpio_cfgpin(GPIO_TOUCH_CHG, S3C_GPIO_INPUT);
	s3c_gpio_setpull(GPIO_TOUCH_CHG, S3C_GPIO_PULL_NONE);
	return 0;
}

#if ENABLE_TOUCH_KEY
#define MXT_KEYS(_name, _code, _x, _y, _delta, _val)	\
{						\
	.name = _name,		\
	.keycode = _code,		\
	.xnode = _x,			\
	.ynode = _y,			\
	.deltaobj = _delta,		\
	.value = _val,			\
}

struct mxt_touchkey mxt_touchkeys[] = {
	MXT_KEYS("d_back", KEY_DUMMY_BACK, 26, 51, 0, TOUCH_KEY_D_BACK),
	MXT_KEYS("back", KEY_BACK, 27, 51, 1, TOUCH_KEY_BACK),
	MXT_KEYS("d_home2", KEY_DUMMY_HOME2, 28, 51, 2, TOUCH_KEY_D_HOME_2),
	MXT_KEYS("d_home1", KEY_DUMMY_HOME1, 29, 51, 3, TOUCH_KEY_D_HOME_1),
	MXT_KEYS("recent", KEY_RECENT, 30, 51, 4, TOUCH_KEY_MENU),
	MXT_KEYS("d_menu", KEY_DUMMY_MENU, 31, 51, 5, TOUCH_KEY_D_MENU),
};

static int mxt_led_power_on(void)
{
	struct regulator *regulator;

#if defined(TSP_DEBUG_LOG)
	printk(KERN_DEBUG "[TSP] %s\n", __func__);
#endif

	regulator = regulator_get(NULL, "key_led_3.3v");
	if (IS_ERR(regulator)) {
		printk(KERN_ERR "[TSP] %s: regulator_get failed\n",
			__func__);
		return -EIO;
	}
	regulator_enable(regulator);
	regulator_put(regulator);
	return 0;
}

static int mxt_led_power_off(void)
{
	struct regulator *regulator;

#if defined(TSP_DEBUG_LOG)
	printk(KERN_DEBUG "[TSP] %s\n", __func__);
#endif

	regulator = regulator_get(NULL, "key_led_3.3v");
	if (IS_ERR(regulator)) {
		printk(KERN_ERR "[TSP] %s: regulator_get failed\n",
			__func__);
		return -EIO;
	}

	if (regulator_is_enabled(regulator))
		regulator_disable(regulator);
	else
		regulator_force_disable(regulator);
	regulator_put(regulator);
	return 0;
}
#endif

static struct mxt_platform_data mxt_data = {
	.num_xnode = 32,
	.num_ynode = 52,
	.max_x = 4095,
	.max_y = 4095,
	.irqflags = IRQF_TRIGGER_LOW | IRQF_ONESHOT,
	.boot_address = MXT_BOOT_ADDRESS,
	.project_name = MXT_PROJECT_NAME,
	.revision = MXT_REVISION_G,
	.read_chg = mxt_read_chg,
	.power_on = mxt_power_on,
	.power_off = mxt_power_off,
	.register_cb = mxt_register_callback,
	.firmware_name = MXT_FIRMWARE_NAME_REVISION,
#if ENABLE_TOUCH_KEY
	.num_touchkey = ARRAY_SIZE(mxt_touchkeys),
	.touchkey = mxt_touchkeys,
	.led_power_on = mxt_led_power_on,
	.led_power_off = mxt_led_power_off,
#endif
};

static struct i2c_board_info mxt_i2c_devs0[] __initdata = {
	{
		I2C_BOARD_INFO(MXT_DEV_NAME, MXT_APP_ADDRESS),
		.platform_data = &mxt_data,
	}
};

void __init atmel_tsp_init(void)
{
	int gpio = 0;

	gpio = GPIO_TOUCH_CHG;
	gpio_request(gpio, "TSP_INT");
	s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(0xf));
	s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
	s5p_register_gpio_interrupt(gpio);
	mxt_i2c_devs0[0].irq = gpio_to_irq(gpio);

	gpio = GPIO_TOUCH_RESET;
	gpio_request(gpio, "TSP_RESET");
	s3c_gpio_cfgpin(gpio, S3C_GPIO_OUTPUT);
	gpio_set_value(gpio, 0);
	s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);

	gpio = GPIO_TOUCH_EN;
	gpio_request(gpio, "TSP_EN");
	s3c_gpio_cfgpin(gpio, S3C_GPIO_OUTPUT);
	gpio_set_value(gpio, 0);
	s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);

	gpio = GPIO_TOUCH_EN_1;
	gpio_request(gpio, "GPIO_TOUCH_EN_1");
	s3c_gpio_cfgpin(gpio, S3C_GPIO_OUTPUT);
	gpio_set_value(gpio, 0);
	s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);

	if (lpcharge)
		printk(KERN_DEBUG"[TSP] disable tsp for lpm\n");
	else {
		s3c_i2c0_set_platdata(NULL);
		i2c_register_board_info(0, mxt_i2c_devs0,
			ARRAY_SIZE(mxt_i2c_devs0));
		printk(KERN_DEBUG "[TSP] %s system_rev[0x%x]\n",
			__func__, system_rev);
	}
}
#endif

/*	Synaptics Thin Driver	*/
#ifdef CONFIG_TOUCHSCREEN_SYNAPTICS_DSX_I2C
#define PROJECT_V2A_NAME	"SM-P9XX"
#define FW_IMAGE_NAME_V2 "tsp_synaptics/synaptics_v2.fw"
#define FW_IMAGE_NAME_V2_G2 "tsp_synaptics/synaptics_v2_g2.fw"

#define DSX_I2C_ADDR 0x20
#define DSX_ATTN_GPIO GPIO_TOUCH_CHG
#define DSX_RESET_GPIO GPIO_TOUCH_RESET
#define DSX_AVDD_GPIO	GPIO_TOUCH_EN

#define NUM_OF_RX	58
#define NUM_OF_TX	36

static int synaptics_power(bool on)
{
	struct regulator *regulator_vdd;
	struct regulator *regulator_avdd;
	static bool enabled;

	if (enabled == on)
		return 0;

	regulator_vdd = regulator_get(NULL, "tsp_vdd_1.8v");
	if (IS_ERR(regulator_vdd)) {
		printk(KERN_ERR "[TSP]ts_power_on : tsp_vdd regulator_get failed\n");
		return PTR_ERR(regulator_vdd);
	}

	if (on) {
		gpio_set_value(DSX_AVDD_GPIO, 1);

		regulator_enable(regulator_vdd);
		regulator_put(regulator_vdd);

		gpio_set_value(DSX_RESET_GPIO, 1);

		s3c_gpio_cfgpin(DSX_ATTN_GPIO, S3C_GPIO_SFN(0xf));
		s3c_gpio_setpull(DSX_ATTN_GPIO, S3C_GPIO_PULL_NONE);
	} else {
		gpio_set_value(DSX_RESET_GPIO, 0);

		/*
		 * TODO: If there is a case the regulator must be disabled
		 * (e,g firmware update?), consider regulator_force_disable.
		 */
		if (regulator_is_enabled(regulator_vdd))
			regulator_disable(regulator_vdd);
		regulator_put(regulator_vdd);

		gpio_set_value(DSX_AVDD_GPIO, 0);

		s3c_gpio_cfgpin(DSX_ATTN_GPIO, S3C_GPIO_INPUT);
		s3c_gpio_setpull(DSX_ATTN_GPIO, S3C_GPIO_PULL_NONE);
	}

	enabled = on;

	printk(KERN_ERR "[TSP] %s %s\n", __func__, on ? "on" : "off");

	return 0;
}

static int synaptics_gpio_setup(unsigned gpio, bool configure)
{
	int retval = 0;

	if (configure) {
		gpio_request(gpio, "TSP_INT");
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(0xf));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_UP);
		s5p_register_gpio_interrupt(gpio);
	} else {
		pr_warn("%s: No way to deconfigure gpio %d.",
		       __func__, gpio);
	}

	return retval;
}

#ifdef NO_0D_WHILE_2D
static unsigned char tm1940_cap_button_codes_gff[] = {
	KEY_DUMMY_MENU,
	KEY_RECENT,
	KEY_DUMMY_HOME1,
	KEY_DUMMY_HOME2,
	KEY_BACK,
	KEY_DUMMY_BACK,
};

static unsigned char tm1940_cap_button_codes_g2[] = {
	KEY_DUMMY_MENU,
	KEY_RECENT,
	KEY_BACK,
	KEY_DUMMY_BACK,
};

static struct synaptics_rmi_f1a_button_map tm1940_cap_button_map_gff = {
	.nbuttons = ARRAY_SIZE(tm1940_cap_button_codes_gff),
	.map = tm1940_cap_button_codes_gff,
};

static struct synaptics_rmi_f1a_button_map tm1940_cap_button_map_g2 = {
	.nbuttons = ARRAY_SIZE(tm1940_cap_button_codes_g2),
	.map = tm1940_cap_button_codes_g2,
};

static int ts_led_power_on(bool on)
{
	struct regulator *regulator;

	if (on) {
		regulator = regulator_get(NULL, "key_led_3.3v");
		if (IS_ERR(regulator)) {
			printk(KERN_ERR "[TSP_KEY] ts_led_power_on : TK_LED regulator_get failed\n");
			return -EIO;
		}

		regulator_enable(regulator);
		regulator_put(regulator);
	} else {
		regulator = regulator_get(NULL, "key_led_3.3v");
		if (IS_ERR(regulator)) {
			printk(KERN_ERR "[TSP_KEY] ts_led_power_on : TK_LED regulator_get failed\n");
			return -EIO;
		}

		if (regulator_is_enabled(regulator))
			regulator_force_disable(regulator);
		regulator_put(regulator);
	}

	printk(KERN_ERR "[TSP_KEY] %s %s\n", __func__, on ? "on" : "off");

	return 0;
}
#endif

static struct synaptics_rmi4_platform_data dsx_platformdata = {
	.sensor_max_x = 2559,
	.sensor_max_y = 1599,
	.max_touch_width = 28,
	.irq_type = IRQF_TRIGGER_LOW | IRQF_ONESHOT,/*IRQF_TRIGGER_FALLING,*/
	.power = synaptics_power,
	.gpio = DSX_ATTN_GPIO,
	.gpio_config = synaptics_gpio_setup,
#ifdef NO_0D_WHILE_2D
	.led_power_on = ts_led_power_on,
	.f1a_button_map = &tm1940_cap_button_map_g2,
#endif
	.firmware_name = FW_IMAGE_NAME_V2_G2,
	.fac_firmware_name = FW_IMAGE_NAME_V2_G2,
	.project_name = PROJECT_V2A_NAME,
	.get_ddi_type = NULL,
	.num_of_rx = NUM_OF_RX,
	.num_of_tx = NUM_OF_TX,
};

static struct i2c_board_info synaptics_dsx_i2c_devs0[] = {
	{
		I2C_BOARD_INFO("synaptics_dsx_i2c", DSX_I2C_ADDR),
		.platform_data = &dsx_platformdata,
	},
};

static void synaptics_dsx_gpio_init(void)
{
	/* touch interrupt */
	gpio_request(DSX_ATTN_GPIO, "TSP_INT");
	s3c_gpio_cfgpin(DSX_ATTN_GPIO, S3C_GPIO_INPUT);
	s3c_gpio_setpull(DSX_ATTN_GPIO, S3C_GPIO_PULL_NONE);
	s5p_register_gpio_interrupt(DSX_ATTN_GPIO);

	s3c_gpio_setpull(GPIO_TSP_SCL_18V, S3C_GPIO_PULL_NONE);
	s3c_gpio_setpull(GPIO_TSP_SDA_18V, S3C_GPIO_PULL_NONE);

	gpio_request_one(DSX_RESET_GPIO,
		GPIOF_OUT_INIT_LOW, "synaptics_dsx_i2c nRESET");

	gpio_request(DSX_AVDD_GPIO, "GPIO_TOUCH_EN");
	s3c_gpio_cfgpin(DSX_AVDD_GPIO, S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(DSX_AVDD_GPIO, S3C_GPIO_PULL_NONE);

	s5p_gpio_set_pd_cfg(DSX_ATTN_GPIO, S5P_GPIO_PD_PREV_STATE);
	s5p_gpio_set_pd_pull(DSX_ATTN_GPIO, S5P_GPIO_PD_UPDOWN_DISABLE);
	s5p_gpio_set_pd_cfg(DSX_RESET_GPIO, S5P_GPIO_PD_PREV_STATE);
	s5p_gpio_set_pd_cfg(DSX_AVDD_GPIO, S5P_GPIO_PD_PREV_STATE);
}

void __init synaptics_dsx_tsp_init(void)
{
	synaptics_dsx_gpio_init();

	synaptics_dsx_i2c_devs0[0].irq = gpio_to_irq(DSX_ATTN_GPIO);
#if defined(CONFIG_V2A_3G)
	if (system_rev < 4) {
#ifdef NO_0D_WHILE_2D
		dsx_platformdata.f1a_button_map = &tm1940_cap_button_map_gff;
#endif
		dsx_platformdata.firmware_name = FW_IMAGE_NAME_V2;
		dsx_platformdata.fac_firmware_name = FW_IMAGE_NAME_V2;
	}
#endif
	s3c_i2c0_set_platdata(NULL);
	i2c_register_board_info(0, synaptics_dsx_i2c_devs0,
		 ARRAY_SIZE(synaptics_dsx_i2c_devs0));

	printk(KERN_ERR "%s touch : %d\n",
		 __func__, synaptics_dsx_i2c_devs0[0].irq);
}
#endif

#ifdef CONFIG_INPUT_WACOM
static struct wacom_g5_callbacks *wacom_callbacks;
static bool wacom_power_enabled;

int wacom_power(bool on)
{
#ifdef GPIO_PEN_LDO_EN
	gpio_direction_output(GPIO_PEN_LDO_EN, on);
#else
	struct regulator *regulator_vdd;

	if (wacom_power_enabled == on)
		return 0;

	regulator_vdd = regulator_get(NULL, "wacom_3.0v");
	if (IS_ERR(regulator_vdd)) {
		printk(KERN_ERR"epen: %s reg get err\n", __func__);
		return PTR_ERR(regulator_vdd);
	}

	if (on) {
		regulator_enable(regulator_vdd);
	} else {
		if (regulator_is_enabled(regulator_vdd))
			regulator_disable(regulator_vdd);
	}
	regulator_put(regulator_vdd);

	wacom_power_enabled = on;
#endif
	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static int wacom_early_suspend_hw(void)
{
#ifdef GPIO_PEN_RESET_N_18V
	gpio_direction_output(GPIO_PEN_RESET_N_18V, 0);
#endif
	wacom_power(0);

	return 0;
}

static int wacom_late_resume_hw(void)
{
#ifdef GPIO_PEN_RESET_N_18V
	gpio_direction_output(GPIO_PEN_RESET_N_18V, 1);
#endif
	gpio_direction_output(GPIO_PEN_PDCT_18V, 1);
	wacom_power(1);
	msleep(100);
	gpio_direction_input(GPIO_PEN_PDCT_18V);
	return 0;
}
#endif

static int wacom_suspend_hw(void)
{
#ifdef GPIO_PEN_RESET_N_18V
	gpio_direction_output(GPIO_PEN_RESET_N_18V, 0);
#endif
	wacom_power(0);

	s3c_gpio_cfgpin(GPIO_PEN_IRQ_18V, S3C_GPIO_INPUT);
	s3c_gpio_setpull(GPIO_PEN_IRQ_18V, S3C_GPIO_PULL_DOWN);

	return 0;
}

static int wacom_resume_hw(void)
{
#ifdef GPIO_PEN_RESET_N_18V
	gpio_direction_output(GPIO_PEN_RESET_N_18V, 1);
#endif
	gpio_direction_output(GPIO_PEN_PDCT_18V, 1);
	wacom_power(1);
	/*msleep(100);*/
	gpio_direction_input(GPIO_PEN_PDCT_18V);

	s3c_gpio_cfgpin(GPIO_PEN_IRQ_18V, S3C_GPIO_SFN(0xf));
	s3c_gpio_setpull(GPIO_PEN_IRQ_18V, S3C_GPIO_PULL_NONE);

	return 0;
}

static int wacom_reset_hw(void)
{
	wacom_suspend_hw();
	msleep(100);
	wacom_resume_hw();

	return 0;
}

static void wacom_register_callbacks(struct wacom_g5_callbacks *cb)
{
	wacom_callbacks = cb;
};

static void wacom_compulsory_flash_mode(bool en)
{
	gpio_direction_output(GPIO_PEN_FWE1_18V, en);
}

static int wacom_get_irq_state(void)
{
	return gpio_get_value(GPIO_PEN_IRQ_18V);
}

static struct wacom_g5_platform_data wacom_platform_data = {
	.x_invert = WACOM_X_INVERT,
	.y_invert = WACOM_Y_INVERT,
	.xy_switch = WACOM_XY_SWITCH,
	.min_x = 0,
	.max_x = WACOM_MAX_COORD_X,
	.min_y = 0,
	.max_y = WACOM_MAX_COORD_Y,
	.min_pressure = 0,
	.max_pressure = WACOM_MAX_PRESSURE,
	.gpio_pendct = GPIO_PEN_PDCT_18V,
	/*.init_platform_hw = wacom_init,*/
	/*      .exit_platform_hw =,    */
	.suspend_platform_hw = wacom_suspend_hw,
	.resume_platform_hw = wacom_resume_hw,
#ifdef CONFIG_HAS_EARLYSUSPEND
	.early_suspend_platform_hw = wacom_early_suspend_hw,
	.late_resume_platform_hw = wacom_late_resume_hw,
#endif
	.reset_platform_hw = wacom_reset_hw,
	.register_cb = wacom_register_callbacks,
	.compulsory_flash_mode = wacom_compulsory_flash_mode,
	.gpio_pen_insert = GPIO_WACOM_SENSE,
	.get_irq_state = wacom_get_irq_state,
};

/* I2C */
static struct i2c_board_info wacom_i2c_devs[] __initdata = {
	{
		I2C_BOARD_INFO("wacom_g5sp_i2c", WACOM_I2C_ADDR),
		.platform_data = &wacom_platform_data,
	},
};

#define WACOM_SET_I2C(ch, pdata, i2c_info)	\
do {		\
	s3c_i2c##ch##_set_platdata(pdata);	\
	i2c_register_board_info(ch, i2c_info,	\
	ARRAY_SIZE(i2c_info));	\
	platform_device_register(&s3c_device_i2c##ch);	\
} while (0);

void __init wacom_init(void)
{
	int gpio;
	int ret;

#ifdef GPIO_PEN_RESET_N_18V
	/*Reset*/
	gpio = GPIO_PEN_RESET_N_18V;
	ret = gpio_request(gpio, "PEN_RESET_N");
	if (ret) {
		printk(KERN_ERR "epen:failed to request PEN_RESET_N.(%d)\n",
			ret);
		return ;
	}
	s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(0x1));
	s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
	gpio_direction_output(gpio, 0);
#endif

	/*SLP & FWE1*/
	gpio = GPIO_PEN_FWE1_18V;
	ret = gpio_request(gpio, "PEN_FWE1");
	if (ret) {
		printk(KERN_ERR "epen:failed to request PEN_FWE1.(%d)\n",
			ret);
		return ;
	}
	s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(0x1));
	s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
	gpio_direction_output(gpio, 0);

	/*PDCT*/
	gpio = GPIO_PEN_PDCT_18V;
	ret = gpio_request(gpio, "PEN_PDCT");
	if (ret) {
		printk(KERN_ERR "epen:failed to request PEN_PDCT.(%d)\n",
			ret);
		return ;
	}
	s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(0xf));
	s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
	s5p_register_gpio_interrupt(gpio);
	gpio_direction_input(gpio);

	irq_set_irq_type(gpio_to_irq(gpio), IRQ_TYPE_EDGE_BOTH);

	/*IRQ*/
	gpio = GPIO_PEN_IRQ_18V;
	ret = gpio_request(gpio, "PEN_IRQ");
	if (ret) {
		printk(KERN_ERR "epen:failed to request PEN_IRQ.(%d)\n",
			ret);
		return ;
	}
	s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
	s5p_register_gpio_interrupt(gpio);
	gpio_direction_input(gpio);

	wacom_i2c_devs[0].irq = gpio_to_irq(gpio);
	irq_set_irq_type(wacom_i2c_devs[0].irq, IRQ_TYPE_EDGE_RISING);
	s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(0xf));

	/*LDO_EN*/
#ifdef GPIO_PEN_LDO_EN
	gpio = GPIO_PEN_LDO_EN;
	ret = gpio_request(gpio, "PEN_LDO_EN");
	if (ret) {
		printk(KERN_ERR "epen:failed to request PEN_LDO_EN.(%d)\n",
			ret);
		return ;
	}
	s3c_gpio_cfgpin(gpio, S3C_GPIO_OUTPUT);
	gpio_direction_output(gpio, 0);
#else
	wacom_power(0);
#endif

	/*WACOM_SET_I2C(3, NULL, wacom_i2c_devs);*/
	if (lpcharge)
		printk(KERN_DEBUG"epen:disable wacom for lpm\n");
	else {
		exynos5_hs_i2c4_set_platdata(NULL);
		i2c_register_board_info(8, wacom_i2c_devs,
			ARRAY_SIZE(wacom_i2c_devs));
	}

	printk(KERN_INFO "epen:init\n");
}
#endif

#define GPIO_KEYS(_name, _code, _gpio, _active_low, _iswake)	\
{									\
	.desc = _name,					\
	.code = _code,					\
	.gpio = _gpio,					\
	.active_low = _active_low,			\
	.type = EV_KEY,					\
	.wakeup = _iswake,				\
	.debounce_interval = 10,			\
	.value = 1						\
}

static struct gpio_keys_button gpio_button[] = {
	GPIO_KEYS("KEY_POWER", KEY_POWER,
		GPIO_nPOWER, true, true),
	GPIO_KEYS("KEY_VOLUMEUP", KEY_VOLUMEUP,
		GPIO_VOL_UP, true, false),
	GPIO_KEYS("KEY_VOLUMEDOWN", KEY_VOLUMEDOWN,
		GPIO_VOL_DOWN, true, false),
	GPIO_KEYS("KEY_HOME", KEY_HOMEPAGE,
		GPIO_HOME_KEY, true, true),
};

static struct gpio_keys_platform_data gpiokeys_platform_data = {
	gpio_button,
	ARRAY_SIZE(gpio_button),
#ifdef CONFIG_SENSORS_HALL
	.gpio_flip_cover = GPIO_HALL_SENSOR_INT,
#endif
};

static struct platform_device gpio_keys = {
	.name   = "gpio-keys",
	.dev    = {
		.platform_data = &gpiokeys_platform_data,
	},
};

static struct input_debug_key_state kstate[] = {
	SET_DEBUG_KEY(KEY_POWER, false),
	SET_DEBUG_KEY(KEY_VOLUMEUP, false),
	SET_DEBUG_KEY(KEY_VOLUMEDOWN, true),
	SET_DEBUG_KEY(KEY_HOMEPAGE, false),
};

static struct input_debug_pdata input_debug_platform_data = {
	.nkeys = ARRAY_SIZE(kstate),
	.key_state = kstate,
};

static struct platform_device input_debug = {
	.name	= SEC_DEBUG_NAME,
	.dev	= {
		.platform_data = &input_debug_platform_data,
	},
};

#ifdef CONFIG_INPUT_BOOSTER
static enum booster_device_type get_booster_device(int code)
{
	switch (code) {
	case KEY_HOMEPAGE:
		return BOOSTER_DEVICE_KEY;
		break;
#if defined(ENABLE_TOUCH_KEY)
	case KEY_RECENT:
	case KEY_BACK:
		return BOOSTER_DEVICE_TOUCHKEY;
		break;
#endif
#if defined(CONFIG_INPUT_TOUCHSCREEN)
	case KEY_BOOSTER_TOUCH:
		return BOOSTER_DEVICE_TOUCH;
		break;
#endif
#if defined(CONFIG_INPUT_WACOM)
	case KEY_BOOSTER_PEN:
		return BOOSTER_DEVICE_PEN;
		break;
#endif
	default:
		return BOOSTER_DEVICE_NOT_DEFINED;
		break;
	}
}

static const struct dvfs_freq key_freq_table[BOOSTER_LEVEL_MAX] = {
	[BOOSTER_LEVEL1] = BOOSTER_DVFS_FREQ(650000,	400000,	111000),
};

#if defined(ENABLE_TOUCH_KEY)
static const struct dvfs_freq touchkey_freq_table[BOOSTER_LEVEL_MAX] = {
	[BOOSTER_LEVEL1] = BOOSTER_DVFS_FREQ(1600000,	667000,	333000),
	[BOOSTER_LEVEL2] = BOOSTER_DVFS_FREQ(650000,	400000,	111000),
};
#endif

#if defined(CONFIG_INPUT_TOUCHSCREEN) || defined(CONFIG_INPUT_WACOM)
static const struct dvfs_freq touch_freq_table[BOOSTER_LEVEL_MAX] = {
	[BOOSTER_LEVEL1] = BOOSTER_DVFS_FREQ(1600000,	667000,	333000),
	[BOOSTER_LEVEL2] = BOOSTER_DVFS_FREQ(650000,	400000,	111000),
	[BOOSTER_LEVEL3] = BOOSTER_DVFS_FREQ(650000,	667000,	333000),
	[BOOSTER_LEVEL9] = BOOSTER_DVFS_FREQ(1900000,	800000,	400000),
	[BOOSTER_LEVEL9_CHG] = BOOSTER_DVFS_FREQ(1500000,	800000,	400000),
};
#endif

static struct booster_key booster_keys[] = {
	BOOSTER_KEYS("HOMEPAGE", KEY_HOMEPAGE,
		BOOSTER_DEFAULT_ON_TIME,
		BOOSTER_DEFAULT_OFF_TIME,
		key_freq_table),
#if defined(ENABLE_TOUCH_KEY)
	BOOSTER_KEYS("MENU", KEY_RECENT,
		BOOSTER_DEFAULT_ON_TIME,
		BOOSTER_DEFAULT_OFF_TIME,
		touchkey_freq_table),
	BOOSTER_KEYS("BACK", KEY_BACK,
		BOOSTER_DEFAULT_ON_TIME,
		BOOSTER_DEFAULT_OFF_TIME,
		touchkey_freq_table),
#endif
#if defined(CONFIG_INPUT_TOUCHSCREEN)
	BOOSTER_KEYS("TOUCH", KEY_BOOSTER_TOUCH,
		BOOSTER_DEFAULT_CHG_TIME,
		BOOSTER_DEFAULT_OFF_TIME,
		touch_freq_table),
#endif
#if defined(CONFIG_INPUT_WACOM)
	BOOSTER_KEYS("PEN", KEY_BOOSTER_PEN,
		BOOSTER_DEFAULT_CHG_TIME,
		BOOSTER_DEFAULT_OFF_TIME,
		touch_freq_table),
#endif
};

/* Caution : keys, nkeys, get_device_type should be defined */
static struct input_booster_platform_data input_booster_pdata = {
	.keys = booster_keys,
	.nkeys = ARRAY_SIZE(booster_keys),
	.get_device_type = get_booster_device,
};

static struct platform_device input_booster = {
	.name = INPUT_BOOSTER_NAME,
	.dev.platform_data = &input_booster_pdata,
};
#endif

static struct platform_device *input_devices[] __initdata = {
	&s3c_device_i2c0,
	&gpio_keys,
	&input_debug,
#ifdef CONFIG_INPUT_WACOM
	&exynos5_device_hs_i2c4,
#endif
#ifdef CONFIG_INPUT_BOOSTER
	&input_booster,
#endif
};

void __init exynos5_universal5420_input_init(void)
{
#if defined(CONFIG_TOUCHSCREEN_SYNAPTICS_DSX_I2C)
#if defined(CONFIG_V2A)
	if (0x6 <= system_rev)
		atmel_tsp_init();
	else
#endif
	synaptics_dsx_tsp_init();
#elif defined(CONFIG_TOUCHSCREEN_ATMEL_MXTS)
	atmel_tsp_init();
#endif

#ifdef CONFIG_INPUT_WACOM
	wacom_init();
#endif
	platform_add_devices(input_devices,
			ARRAY_SIZE(input_devices));
}

