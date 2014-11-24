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
#include <linux/i2c-gpio.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/regulator/consumer.h>
#ifdef CONFIG_TOUCHSCREEN_SYNAPTICS_I2C_DSX
#include <linux/i2c/synaptics_rmi.h>
#endif
#ifdef CONFIG_TOUCHSCREEN_ATMEL_MXT1664T
#include <linux/i2c/mxtt.h>
#endif
#ifdef CONFIG_KEYBOARD_CYPRESS_TOUCH
#include <linux/i2c/touchkey_i2c.h>
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

extern unsigned int system_rev;
extern unsigned int lcdtype;

#if !defined(CONFIG_SAMSUNG_PRODUCT_SHIP)
#define TSP_DEBUG_LOG
#endif

/*	Synaptics Thin Driver	*/
#ifdef CONFIG_TOUCHSCREEN_SYNAPTICS_I2C_DSX
#define PROJECT_KLIMT_NAME	"SM-T700"
#define FW_IMAGE_NAME_5707 "tsp_synaptics/synaptics_klimt_5707.fw"

#define DSX_I2C_ADDR 0x20
#define DSX_ATTN_GPIO GPIO_TSP_INT

#define NUM_OF_RX	45
#define NUM_OF_TX	28

static int synaptics_power(void *data, bool on)
{
	struct regulator *regulator_vdd;
	struct regulator *regulator_avdd;
	static bool enabled;

	if (enabled == on)
		return 0;

	regulator_vdd = regulator_get(NULL, "tsp_1.8v");
	if (IS_ERR(regulator_vdd)) {
		printk(KERN_ERR "[TSP]ts_power_on : tsp_vdd regulator_get failed\n");
		return PTR_ERR(regulator_vdd);
	}

	regulator_avdd = regulator_get(NULL, "tsp_3.3v");
	if (IS_ERR(regulator_avdd)) {
		printk(KERN_ERR "[TSP]ts_power_on : tsp_vdd regulator_get failed\n");
		return PTR_ERR(regulator_avdd);
	}

	if (on) {
		regulator_enable(regulator_avdd);
		regulator_enable(regulator_vdd);
		gpio_direction_output(GPIO_TSP_RST, 1);

		s3c_gpio_cfgpin(DSX_ATTN_GPIO, S3C_GPIO_SFN(0xf));
		s3c_gpio_setpull(DSX_ATTN_GPIO, S3C_GPIO_PULL_NONE);
	} else {
		/*
		 * TODO: If there is a case the regulator must be disabled
		 * (e,g firmware update?), consider regulator_force_disable.
		 */
		if (regulator_is_enabled(regulator_vdd))
			regulator_disable(regulator_vdd);
		gpio_direction_output(GPIO_TSP_RST, 0);
		if (regulator_is_enabled(regulator_avdd))
			regulator_disable(regulator_avdd);

		s3c_gpio_cfgpin(DSX_ATTN_GPIO, S3C_GPIO_INPUT);
		s3c_gpio_setpull(DSX_ATTN_GPIO, S3C_GPIO_PULL_NONE);
	}

	regulator_put(regulator_avdd);
	regulator_put(regulator_vdd);

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
static unsigned char button_codes[] = {
	KEY_DUMMY_MENU,
	KEY_RECENT,
	KEY_BACK,
	KEY_DUMMY_BACK,
};

static struct synaptics_rmi_f1a_button_map button_map = {
	.nbuttons = ARRAY_SIZE(button_codes),
	.map = button_codes,
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
	.sensor_max_x = 1599,
	.sensor_max_y = 2559,
	.num_of_rx = NUM_OF_RX,
	.num_of_tx = NUM_OF_TX,
	.max_touch_width = 28,
	.panel_revision = 1,
	.gpio = DSX_ATTN_GPIO,
	.irq_type = IRQF_TRIGGER_LOW | IRQF_ONESHOT,
	.power = synaptics_power,
#ifdef NO_0D_WHILE_2D
	.led_power_on = ts_led_power_on,
	.f1a_button_map = &button_map
#endif
	.firmware_name = FW_IMAGE_NAME_5707,
	.project_name = PROJECT_KLIMT_NAME,
};

static struct i2c_board_info synaptics_dsx_i2c_devs[] = {
	{
		I2C_BOARD_INFO("synaptics_rmi4_i2c", DSX_I2C_ADDR),
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

	s5p_gpio_set_pd_cfg(DSX_ATTN_GPIO, S5P_GPIO_PD_PREV_STATE);
	s5p_gpio_set_pd_pull(DSX_ATTN_GPIO, S5P_GPIO_PD_UPDOWN_DISABLE);

	gpio_request(GPIO_TSP_RST, "TSP_RST");
}

void __init synaptics_dsx_tsp_init(void)
{
	synaptics_dsx_gpio_init();

	if (lpcharge) {
		printk(KERN_DEBUG "%s : lpcharge. tsp driver unload\n", __func__);
		return;
	}

	if (lcdtype == 0) {
		printk(KERN_ERR "%s lcdtype 0. tsp driver unload\n", __func__);
		return;
	}

	synaptics_dsx_i2c_devs[0].irq = gpio_to_irq(DSX_ATTN_GPIO);
	s3c_i2c0_set_platdata(NULL);
	i2c_register_board_info(0, synaptics_dsx_i2c_devs,
		 ARRAY_SIZE(synaptics_dsx_i2c_devs));

	printk(KERN_DEBUG "%s touch : %d, lcdtype = %d\n",
		__func__, synaptics_dsx_i2c_devs[0].irq, lcdtype);
}
#endif

#ifdef CONFIG_TOUCHSCREEN_ATMEL_MXT1664T
#define MXT_PROJECT_NAME		"SM-T700"
#define MXT_BOOT_ADDRESS		0x26
#define MXT_FIRMWARE_NAME		"mxtt_klimt.fw"

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

static bool enabled;
int mxt_power(void *ddata, bool on)
{
	struct regulator *regulator_dvdd;
	struct regulator *regulator_avdd;
	int ret = 0;

	if (enabled == on)
		return ret;

	regulator_dvdd = regulator_get(NULL, "tsp_1.8v");
	if (IS_ERR(regulator_dvdd)) {
		printk(KERN_ERR "%s: Failed to get tsp_1.8v regulator.\n",
			__func__);
		return PTR_ERR(regulator_dvdd);
	}
	regulator_avdd = regulator_get(NULL, "tsp_3.3v");
	if (IS_ERR(regulator_avdd)) {
		printk(KERN_ERR "%s: Failed to get tsp_3.3v regulator.\n",
			__func__);
		return PTR_ERR(regulator_avdd);
	}

	if (on) {
		ret = regulator_enable(regulator_dvdd);
		if (ret) {
			printk(KERN_ERR "%s: Failed to enable vdd: %d\n", __func__, ret);
			return ret;
		}
		ret = regulator_enable(regulator_avdd);
		if (ret) {
			printk(KERN_ERR "%s: Failed to enable avdd: %d\n", __func__, ret);
			return ret;
		}

		/* de-assert reset */
		msleep(50);		/* need to be checked... */
		gpio_set_value(GPIO_TSP_RST, 1);

		s3c_gpio_cfgpin(GPIO_TSP_INT, S3C_GPIO_SFN(0xf));
		s3c_gpio_setpull(GPIO_TSP_INT, S3C_GPIO_PULL_NONE);

		msleep(100);
	} else {
		/* assert reset */
		gpio_set_value(GPIO_TSP_RST, 0);

		if (regulator_is_enabled(regulator_avdd))
			regulator_disable(regulator_avdd);
		if (regulator_is_enabled(regulator_dvdd))
			regulator_disable(regulator_dvdd);

		s3c_gpio_cfgpin(GPIO_TSP_INT, S3C_GPIO_INPUT);
		s3c_gpio_setpull(GPIO_TSP_INT, S3C_GPIO_PULL_NONE);
	}

	regulator_put(regulator_dvdd);
	regulator_put(regulator_avdd);

	enabled = on;

	printk(KERN_INFO "[TSP] %s %s\n", __func__, on ? "on" : "off");

	return ret;
}

static struct mxt_platform_data mxt_data = {
	.num_xnode = 28,
	.num_ynode = 45,
	.max_x = 4095,
	.max_y = 4095,
	.irqflags = IRQF_TRIGGER_LOW | IRQF_ONESHOT,
	.boot_address = MXT_BOOT_ADDRESS,
	.project_name = MXT_PROJECT_NAME,
	.register_cb = mxt_register_callback,
	.power_ctrl = mxt_power,
	.gpio_irq = GPIO_TSP_INT,
	.gpio_reset = GPIO_TSP_RST,
	.firmware_name = MXT_FIRMWARE_NAME,
};

static struct i2c_board_info mxt_i2c_devs[] = {
	{
		I2C_BOARD_INFO(MXT_DEV_NAME, 0x4a),
		.platform_data = &mxt_data,
	},
};

static void mxt_gpio_init(void)
{
	/* touch interrupt */
	gpio_request(GPIO_TSP_INT, "TSP_INT");
	s3c_gpio_cfgpin(GPIO_TSP_INT, S3C_GPIO_INPUT);
	s3c_gpio_setpull(GPIO_TSP_INT, S3C_GPIO_PULL_NONE);
	s5p_register_gpio_interrupt(GPIO_TSP_INT);

	s3c_gpio_setpull(GPIO_TSP_SCL_18V, S3C_GPIO_PULL_NONE);
	s3c_gpio_setpull(GPIO_TSP_SDA_18V, S3C_GPIO_PULL_NONE);

	gpio_request(GPIO_TSP_RST, "TSP_RST");
	gpio_set_value(GPIO_TSP_RST, 0);
}

void __init mxt_tsp_init(void)
{
	mxt_gpio_init();

	if (lpcharge) {
		printk(KERN_DEBUG "%s : lpcharge. tsp driver unload\n", __func__);
		return;
	}

	if (lcdtype == 0) {
		printk(KERN_ERR "%s lcdtype 0. tsp driver unload\n", __func__);
		return;
	}

	mxt_i2c_devs[0].irq = gpio_to_irq(GPIO_TSP_INT);

	s3c_i2c0_set_platdata(NULL);
	i2c_register_board_info(0, mxt_i2c_devs,
		 ARRAY_SIZE(mxt_i2c_devs));

	printk(KERN_INFO "%s touch : %d\n",
		__func__, mxt_i2c_devs[0].irq);
}
#endif

#ifdef CONFIG_KEYBOARD_CYPRESS_TOUCH
#define GPIO_TOUCH_INT EXYNOS5420_GPX0(0)
#define GPIO_TOUCH_SDA EXYNOS5420_GPD1(4)
#define GPIO_TOUCH_SCL EXYNOS5420_GPD1(5)
static struct i2c_board_info touchkey_i2c_info[];

#ifdef TK_INFORM_CHARGER
static struct touchkey_callbacks *tk_charger_callbacks;

void touchkey_charger_infom(bool en)
{
	if (tk_charger_callbacks && tk_charger_callbacks->inform_charger)
		tk_charger_callbacks->inform_charger(tk_charger_callbacks, en);
}

static void touchkey_register_callback(void *cb)
{
	tk_charger_callbacks = cb;
}
#endif
static void touchkey_init_hw(void)
{
#ifndef LED_LDO_WITH_REGULATOR
	gpio_request(GPIO_3_TOUCH_EN, "gpio_3_touch_en");
#endif
#ifndef LDO_WITH_REGULATOR
	gpio_request(GPIO_VTOUCH_LDO_EN, "gpio_vtouch_ldo_en");
#endif
	gpio_request(GPIO_TOUCH_INT, "TOUCH_INT");
	s3c_gpio_setpull(GPIO_TOUCH_INT, S3C_GPIO_PULL_NONE);
	s5p_register_gpio_interrupt(GPIO_TOUCH_INT);
	gpio_direction_input(GPIO_TOUCH_INT);

	touchkey_i2c_info[0].irq = gpio_to_irq(GPIO_TOUCH_INT);
	irq_set_irq_type(gpio_to_irq(GPIO_TOUCH_INT), IRQF_TRIGGER_FALLING);
	s3c_gpio_cfgpin(GPIO_TOUCH_INT, S3C_GPIO_SFN(0xf));

	printk(KERN_ERR "%s touchkey : %d\n",
		__func__, touchkey_i2c_info[0].irq);

	s3c_gpio_setpull(GPIO_TOUCH_SCL, S3C_GPIO_PULL_DOWN);
	s3c_gpio_setpull(GPIO_TOUCH_SDA, S3C_GPIO_PULL_DOWN);
}

static int touchkey_suspend(void)
{
#ifdef LDO_WITH_REGULATOR
	struct regulator *regulator;

	regulator = regulator_get(NULL, TK_REGULATOR_NAME);
	if (IS_ERR(regulator)) {
		printk(KERN_ERR
		"[Touchkey] touchkey_suspend : TK regulator_get failed\n");
		return -EIO;
	}

	if (regulator_is_enabled(regulator))
		regulator_disable(regulator);

	regulator_put(regulator);
#else
	gpio_direction_output(GPIO_VTOUCH_LDO_EN, 0);
#endif
	s3c_gpio_setpull(GPIO_TOUCH_SCL, S3C_GPIO_PULL_DOWN);
	s3c_gpio_setpull(GPIO_TOUCH_SDA, S3C_GPIO_PULL_DOWN);

	return 1;
}

static int touchkey_resume(void)
{
#ifdef LDO_WITH_REGULATOR
	struct regulator *regulator;

	regulator = regulator_get(NULL, TK_REGULATOR_NAME);
	if (IS_ERR(regulator)) {
		printk(KERN_ERR
		"[Touchkey] touchkey_resume : TK regulator_get failed\n");
		return -EIO;
	}

	regulator_enable(regulator);
	regulator_put(regulator);
#else
	gpio_direction_output(GPIO_VTOUCH_LDO_EN, 1);
#endif
	s3c_gpio_setpull(GPIO_TOUCH_SCL, S3C_GPIO_PULL_NONE);
	s3c_gpio_setpull(GPIO_TOUCH_SDA, S3C_GPIO_PULL_NONE);

	return 1;
}

static int touchkey_power_on(bool on)
{
	int ret;

	if (on) {
		ret = touchkey_resume();

		gpio_direction_output(GPIO_TOUCH_INT, 1);
		irq_set_irq_type(gpio_to_irq(GPIO_TOUCH_INT),
			IRQF_TRIGGER_FALLING);
		s3c_gpio_cfgpin(GPIO_TOUCH_INT, S3C_GPIO_SFN(0xf));
		s3c_gpio_setpull(GPIO_TOUCH_INT, S3C_GPIO_PULL_UP);
	} else {
		gpio_direction_input(GPIO_TOUCH_INT);
		s3c_gpio_setpull(GPIO_TOUCH_INT, S3C_GPIO_PULL_NONE);
		ret = touchkey_suspend();
	}

	return ret;
}
static bool touchkey_led_on;
static int touchkey_led_power_on(bool on)
{
#ifdef LED_LDO_WITH_REGULATOR
	struct regulator *regulator;

	if (touchkey_led_on == on)
		return 0;

	regulator = regulator_get(NULL, TK_LED_REGULATOR_NAME);
	if (IS_ERR(regulator)) {
		printk(KERN_ERR
		"[Touchkey] touchkey_led_power_on : TK_LED regulator_get failed\n");
		return -EIO;
	}

	if (on) {
		regulator_enable(regulator);
	} else {
		if (regulator_is_enabled(regulator))
			regulator_disable(regulator);
	}
	regulator_put(regulator);
#else
	if (on)
		gpio_direction_output(GPIO_3_TOUCH_EN, 1);
	else
		gpio_direction_output(GPIO_3_TOUCH_EN, 0);
#endif
	touchkey_led_on = on;

	return 1;
}

static struct touchkey_platform_data touchkey_pdata = {
	.gpio_sda = GPIO_TOUCH_SDA,
	.gpio_scl = GPIO_TOUCH_SCL,
	.gpio_int = GPIO_TOUCH_INT,
	.init_platform_hw = touchkey_init_hw,
	.suspend = touchkey_suspend,
	.resume = touchkey_resume,
	.power_on = touchkey_power_on,
	.led_power_on = touchkey_led_power_on,
	.led_control_by_ldo = false,
#ifdef TK_INFORM_CHARGER
	.register_cb = touchkey_register_callback,
#endif
};

static struct i2c_gpio_platform_data gpio_i2c_data12 = {
	.sda_pin = GPIO_TOUCH_SDA,
	.scl_pin = GPIO_TOUCH_SCL,
	.udelay = 1,
};

static struct i2c_board_info touchkey_i2c_info[] = {
	{
		I2C_BOARD_INFO("sec_touchkey", 0x20),
		.platform_data = &touchkey_pdata,
	},
};

struct platform_device s3c_device_i2c12 = {
	.name = "i2c-gpio",
	.id = 12,
	.dev.platform_data = &gpio_i2c_data12,
};
#endif /*CONFIG_KEYBOARD_CYPRESS_TOUCH*/

#if defined(CONFIG_KEYBOARD_CYPRESS_TOUCH)
static void touchkey_init(void)
{
	printk(KERN_INFO "%s, system_rev : %d\n", __func__, system_rev);

	if (lpcharge) {
		printk(KERN_DEBUG "%s : lpcharge. touchkey driver unload\n", __func__);
		return;
	}

	if (lcdtype == 0) {
		printk(KERN_ERR "%s lcdtype 0. touchkey driver unload\n", __func__);
		return;
	}

	if (system_rev > 7)
		touchkey_pdata.led_control_by_ldo = true;

	touchkey_init_hw();
	i2c_register_board_info(12, touchkey_i2c_info,
		ARRAY_SIZE(touchkey_i2c_info));

	s3c_gpio_setpull(GPIO_TOUCH_SCL, S3C_GPIO_PULL_DOWN);
	s3c_gpio_setpull(GPIO_TOUCH_SDA, S3C_GPIO_PULL_DOWN);
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
	case KEY_RECENT:
	case KEY_BACK:
		return BOOSTER_DEVICE_TOUCHKEY;
		break;
	case KEY_BOOSTER_TOUCH:
		return BOOSTER_DEVICE_TOUCH;
		break;
	default:
		return BOOSTER_DEVICE_NOT_DEFINED;
		break;
	}
}

static const struct dvfs_freq key_freq_table[BOOSTER_LEVEL_MAX] = {
	[BOOSTER_LEVEL1] = BOOSTER_DVFS_FREQ(650000, 400000, 111000),
};

static const struct dvfs_freq touchkey_freq_table[BOOSTER_LEVEL_MAX] = {
	[BOOSTER_LEVEL1] = BOOSTER_DVFS_FREQ(1600000, 667000, 333000),
	[BOOSTER_LEVEL2] = BOOSTER_DVFS_FREQ(650000, 400000, 111000),
};

static const struct dvfs_freq touch_freq_table[BOOSTER_LEVEL_MAX] = {
	[BOOSTER_LEVEL1] = BOOSTER_DVFS_FREQ(1600000, 667000, 333000),
	[BOOSTER_LEVEL2] = BOOSTER_DVFS_FREQ(650000, 400000, 111000),
	[BOOSTER_LEVEL3] = BOOSTER_DVFS_FREQ(650000, 667000, 333000),
	[BOOSTER_LEVEL9] = BOOSTER_DVFS_FREQ(1900000, 800000, 400000),
	[BOOSTER_LEVEL9_CHG] = BOOSTER_DVFS_FREQ(1500000, 800000, 400000),
};

static struct booster_key booster_keys[] = {
	BOOSTER_KEYS("HOMEPAGE", KEY_HOMEPAGE,
		BOOSTER_DEFAULT_ON_TIME,
		BOOSTER_DEFAULT_OFF_TIME,
		key_freq_table),
	BOOSTER_KEYS("RECENT", KEY_RECENT,
		BOOSTER_DEFAULT_ON_TIME,
		BOOSTER_DEFAULT_OFF_TIME,
		touchkey_freq_table),
	BOOSTER_KEYS("BACK", KEY_BACK,
		BOOSTER_DEFAULT_ON_TIME,
		BOOSTER_DEFAULT_OFF_TIME,
		touchkey_freq_table),
	BOOSTER_KEYS("TOUCH", KEY_BOOSTER_TOUCH,
		BOOSTER_DEFAULT_CHG_TIME,
		BOOSTER_DEFAULT_OFF_TIME,
		touch_freq_table),
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
#if defined(CONFIG_KEYBOARD_CYPRESS_TOUCH)
	&s3c_device_i2c12,
#endif
	&input_debug,
#ifdef CONFIG_INPUT_BOOSTER
	&input_booster,
#endif
};

#ifdef CONFIG_INPUT_TOUCHSCREEN
static void klimt_tsp_init(void)
{
	u8 panel_rev;

	panel_rev = ((lcdtype >> 12) & 0xf);

	printk(KERN_INFO "%s panel rev : %d\n", __func__, panel_rev);

	if (panel_rev == 0) {
#ifdef CONFIG_TOUCHSCREEN_ATMEL_MXT1664T
		mxt_tsp_init();
#endif
		;
	} else {
#ifdef CONFIG_TOUCHSCREEN_SYNAPTICS_I2C_DSX
		synaptics_dsx_tsp_init();
#endif
		;
	}
}
#endif

void __init exynos5_universal5420_input_init(void)
{
#ifdef CONFIG_INPUT_TOUCHSCREEN
	klimt_tsp_init();
#endif
#if defined(CONFIG_KEYBOARD_CYPRESS_TOUCH)
	touchkey_init();
#endif
#ifdef CONFIG_SENSORS_HALL
	s3c_gpio_setpull(GPIO_HALL_SENSOR_INT, S3C_GPIO_PULL_UP);
	gpio_request(GPIO_HALL_SENSOR_INT, "GPIO_HALL_SENSOR_INT");
	s3c_gpio_cfgpin(GPIO_HALL_SENSOR_INT, S3C_GPIO_SFN(0xf));
	s5p_register_gpio_interrupt(GPIO_HALL_SENSOR_INT);
	gpio_direction_input(GPIO_HALL_SENSOR_INT);
#endif
	platform_add_devices(input_devices,
			ARRAY_SIZE(input_devices));
}

