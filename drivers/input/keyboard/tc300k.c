/* tc300k.c -- Linux driver for coreriver chip as touchkey
 *
 * Copyright (C) 2013 Samsung Electronics Co.Ltd
 * Author: Junkyeong Kim <jk0430.kim@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 */

#include <linux/delay.h>
#include <linux/firmware.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/leds.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/wakelock.h>
#include <linux/workqueue.h>
#include <linux/uaccess.h>
#include <linux/i2c/tc300k.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#ifdef CONFIG_INPUT_BOOSTER
#include <linux/input/input_booster.h>
#endif

/* registers */
#define TC300K_KEYCODE		0x00
#define TC300K_FWVER		0x01
#define TC300K_MDVER		0x02
#define TC300K_MODE			0x03
#define TC300K_CHECKS_H		0x04
#define TC300K_CHECKS_L		0x05
#define TC300K_THRES_H		0x06
#define TC300K_THRES_L		0x07
#define TC300K_1KEY_DATA	0x08
#define TC300K_2KEY_DATA	0x0E
#define TC300K_3KEY_DATA	0x14
#define TC300K_4KEY_DATA	0x1A
#define TC300K_5KEY_DATA	0x20
#define TC300K_6KEY_DATA	0x26

#define TC300K_CH_PCK_H_OFFSET	0x00
#define TC300K_CH_PCK_L_OFFSET	0x01
#define TC300K_DIFF_H_OFFSET	0x02
#define TC300K_DIFF_L_OFFSET	0x03
#define TC300K_RAW_H_OFFSET		0x04
#define TC300K_RAW_L_OFFSET		0x05

/* command */
#define TC300K_CMD_ADDR			0x00
#define TC300K_CMD_LED_ON		0x10
#define TC300K_CMD_LED_OFF		0x20
#define TC300K_CMD_GLOVE_ON		0x30
#define TC300K_CMD_GLOVE_OFF	0x40
#define TC300K_CMD_FAC_ON		0x50
#define TC300K_CMD_FAC_OFF		0x60
#define TC300K_CMD_CAL_CHECKSUM	0x70
#define TC300K_CMD_DELAY		50

/* mask */
#define TC300K_KEY_INDEX_MASK	0x07
#define TC300K_KEY_PRESS_MASK	0x08

/* firmware */
#define TC300K_FW_PATH_SDCARD	"/sdcard/tc300k.bin"

#define TK_UPDATE_PASS		0
#define TK_UPDATE_DOWN		1
#define TK_UPDATE_FAIL		2

/* ISP command */
#define TC300K_CSYNC1			0xA3
#define TC300K_CSYNC2			0xAC
#define TC300K_CSYNC3			0xA5
#define TC300K_CCFG				0x92
#define TC300K_PRDATA			0x81
#define TC300K_PEDATA			0x82
#define TC300K_PWDATA			0x83
#define TC300K_PECHIP			0x8A
#define TC300K_PEDISC			0xB0
#define TC300K_LDDATA			0xB2
#define TC300K_LDMODE			0xB8
#define TC300K_RDDATA			0xB9
#define TC300K_PCRST			0xB4
#define TC300K_PCRED			0xB5
#define TC300K_PCINC			0xB6
#define TC300K_RDPCH			0xBD

/* ISP delay */
#define TC300K_TSYNC1			300	/* us */
#define TC300K_TSYNC2			50	/* 1ms~50ms */
#define TC300K_TSYNC3			100	/* us */
#define TC300K_TDLY1			1	/* us */
#define TC300K_TDLY2			2	/* us */
#define TC300K_TFERASE			10	/* ms */
#define TC300K_TPROG			20	/* us */

#define TC300K_CHECKSUM_DELAY	500

enum {
	FW_INKERNEL,
	FW_SDCARD,
};

enum {
	NORMAL_MODE,
	FACTORY_MODE,
};

struct fw_image {
	u8 hdr_ver;
	u8 hdr_len;
	u16 first_fw_ver;
	u16 second_fw_ver;
	u16 third_ver;
	u32 fw_len;
	u16 checksum;
	u16 alignment_dummy;
	u8 data[0];
} __attribute__ ((packed));

struct tc300k_data {
	struct device *sec_touchkey;
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct tc300k_platform_data *pdata;
	struct mutex lock;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
	struct fw_image *fw_img;
	const struct firmware *fw;
	char phys[32];
	int irq;
	u16 checksum;
	u16 threhold;
	int key_num;
	int *keycode;
	int mode;
	int (*power) (bool on);
	u8 fw_ver;
	u8 md_ver;
	u8 fw_update_status;
	bool enabled;
	bool fw_downloding;
	bool glove_mode;
	bool factory_mode;
	bool led_on;
};

extern struct class *sec_class;

/*temporary*/
int get_tsp_status(void)
{
	return 0;
}

#ifdef CONFIG_PM
static int tc300k_tk_enabled = 1;
static int tc300k_suspend(struct device *dev);
static int tc300k_resume(struct device *dev);
#endif
#ifdef CONFIG_HAS_EARLYSUSPEND
static void tc300k_early_suspend(struct early_suspend *h);
static void tc300k_late_resume(struct early_suspend *h);
#endif

static void tc300k_input_close(struct input_dev *dev);
static int tc300k_input_open(struct input_dev *dev);

#ifdef LED_LDO_WITH_REGULATOR
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>

#define BL_STANDARD	3000
#define BL_MIN		2500
#define BL_MAX		3300

static unsigned int touchkey_voltage_brightness = BL_STANDARD;

static void change_touch_key_led_voltage(int vol_mv)
{
	struct regulator *tled_regulator;

	tled_regulator = regulator_get(NULL, TK_LED_REGULATOR_NAME);
	if (IS_ERR(tled_regulator)) {
		pr_err("%s: failed to get resource %s\n", __func__,
		       "touchkey_led");
		return;
	}
	regulator_set_voltage(tled_regulator, vol_mv * 1000, vol_mv * 1000);
	regulator_put(tled_regulator);
}

void update_touchkey_brightness(unsigned int level)
{
	if (level > 0 && level < 256) {
		printk(KERN_DEBUG "[TouchKey-LED] %s: %d\n", __func__, level);
		touchkey_voltage_brightness = BL_MIN + ((((level * 100 / 255) * (BL_MAX - BL_MIN)) / 100) / 50) * 50;
		change_touch_key_led_voltage(touchkey_voltage_brightness);
	} else {
		printk(KERN_DEBUG "[TouchKey-LED] %s: Ignoring brightness : %d\n", __func__, level);
	}
}
#endif

static void release_all_fingers(struct tc300k_data *data)
{
	struct i2c_client *client = data->client;
	int i;

	dev_dbg(&client->dev, "[TK] %s\n", __func__);

	for (i = 1; i < data->key_num; i++) {
		input_report_key(data->input_dev,
			data->keycode[i], 0);
#ifdef CONFIG_INPUT_BOOSTER
		INPUT_BOOSTER_SEND_EVENT(data->keycode[i],
			BOOSTER_MODE_FORCE_OFF);
#endif

	}
	input_sync(data->input_dev);
}

static void tc300k_reset(struct tc300k_data *data)
{
	release_all_fingers(data);

	disable_irq(data->irq);
	data->pdata->keyled(false);
	data->pdata->power(false);

	msleep(50);

	data->pdata->power(true);
	msleep(70);
	data->pdata->keyled(true);
	msleep(130);
	enable_irq(data->irq);
}

static void tc300k_reset_probe(struct tc300k_data *data)
{
	data->pdata->keyled(false);
	data->pdata->power(false);

	msleep(50);

	data->pdata->power(true);
	msleep(70);
	data->pdata->keyled(true);
	msleep(130);
}

int get_fw_version(struct tc300k_data *data, bool probe)
{
	struct i2c_client *client = data->client;
	int retry = 3;
	int buf;

	if ((!data->enabled) || data->fw_downloding) {
		dev_err(&client->dev, "can't excute %s\n", __func__);
		return -1;
	}

	buf = i2c_smbus_read_byte_data(client, TC300K_FWVER);
	if (buf < 0) {
		while (retry--) {
			dev_err(&client->dev, "%s read fail(%d)\n",
				__func__, retry);
			if (probe)
				tc300k_reset_probe(data);
			else
				tc300k_reset(data);
			buf = i2c_smbus_read_byte_data(client, TC300K_FWVER);
			if (buf > 0)
				break;
		}
		if (retry <= 0) {
			dev_err(&client->dev, "%s read fail\n", __func__);
			data->fw_ver = 0;
			return -1;
		}
	}
	data->fw_ver = (u8)buf;
	dev_info(&client->dev, "fw_ver : 0x%x\n", data->fw_ver);

	return 0;
}

static irqreturn_t tc300k_interrupt(int irq, void *dev_id)
{
	struct tc300k_data *data = dev_id;
	struct i2c_client *client = data->client;
	int ret, retry;
	u8 key_val, index;
	bool press;

	if ((!data->enabled) || data->fw_downloding) {
		dev_err(&client->dev, "can't excute %s\n", __func__);
		return IRQ_HANDLED;
	}

	ret = i2c_smbus_read_byte_data(client, TC300K_KEYCODE);
	if (ret < 0) {
		retry = 3;
		while (retry--) {
			dev_err(&client->dev, "%s read fail ret=%d(retry:%d)\n",
				__func__, ret, retry);
			msleep(10);
			ret = i2c_smbus_read_byte_data(client, TC300K_KEYCODE);
			if (ret > 0)
				break;
		}
		if (retry <= 0) {
			tc300k_reset(data);
			return IRQ_HANDLED;
		}
	}
	key_val = (u8)ret;
	index = key_val & TC300K_KEY_INDEX_MASK;
	press = !!(key_val & TC300K_KEY_PRESS_MASK);

	if (data->keycode[index] == KEY_BACK  || data->keycode[index] == 254/*KEY_RECENTS*/)
	{
		if (press) {
			input_report_key(data->input_dev, data->keycode[index], 0);
#ifdef CONFIG_SAMSUNG_PRODUCT_SHIP
			dev_notice(&client->dev, "key R\n");
#else
			dev_notice(&client->dev,
				"key R : %d(%d)\n", data->keycode[index], key_val);
#endif
#ifdef CONFIG_INPUT_BOOSTER
			INPUT_BOOSTER_SEND_EVENT(data->keycode[index], BOOSTER_MODE_OFF);
#endif
		} else {
			input_report_key(data->input_dev, data->keycode[index], 1);
#ifdef CONFIG_SAMSUNG_PRODUCT_SHIP
			dev_notice(&client->dev, "key P\n");
#else
			dev_notice(&client->dev,
				"key P : %d(%d)\n", data->keycode[index], key_val);
#endif
#ifdef CONFIG_INPUT_BOOSTER
			INPUT_BOOSTER_SEND_EVENT(data->keycode[index], BOOSTER_MODE_ON);
#endif
		}
		input_sync(data->input_dev);
	}
	else
	{
		dev_notice(&client->dev, "Invalid key ignored %d (%d(%d))\n", press, data->keycode[index], key_val);
	}

	return IRQ_HANDLED;
}

static ssize_t tc300k_threshold_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct tc300k_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	int ret;
	u8 threshold_h, threshold_l;

	if ((!data->enabled) || data->fw_downloding) {
		dev_err(&client->dev, "can't excute %s\n", __func__);
		return -EPERM;
	}

	ret = i2c_smbus_read_byte_data(client, TC300K_THRES_H);
	if (ret < 0) {
		dev_err(&client->dev, "%s: failed to read threshold_h (%d)\n",
			__func__, ret);
		return ret;
	}
	threshold_h = ret;

	ret = i2c_smbus_read_byte_data(client, TC300K_THRES_L);
	if (ret < 0) {
		dev_err(&client->dev, "%s: failed to read threshold_l (%d)\n",
			__func__, ret);
		return ret;
	}
	threshold_l = ret;

	data->threhold = (threshold_h << 8) | threshold_l;

	return sprintf(buf, "%d\n", data->threhold);
}

static ssize_t tc300k_led_control(struct device *dev,
	 struct device_attribute *attr, const char *buf, size_t count)
{
	struct tc300k_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	int scan_buffer;
	int ret;
	u8 cmd;

	ret = sscanf(buf, "%d", &scan_buffer);
	if (ret != 1) {
		dev_err(&client->dev, "%s: cmd read err\n", __func__);
		return count;
	}

#ifdef LED_LDO_WITH_REGULATOR
	if (scan_buffer > 1 && tc300k_tk_enabled) {
		update_touchkey_brightness(scan_buffer);
	}
	scan_buffer = scan_buffer ? 1 : 0;
#endif

	if (!(scan_buffer == 0 || scan_buffer == 1)) {
		dev_err(&client->dev, "%s: wrong command(%d)\n",
			__func__, scan_buffer);
		return count;
	}

	if ((!data->enabled) || data->fw_downloding) {
		dev_err(&client->dev, "can't excute %s\n", __func__);
		if (scan_buffer == 1)
			data->led_on = true;
		return count;
	}

	if (scan_buffer == 1) {
		dev_notice(&client->dev, "led on\n");
		cmd = TC300K_CMD_LED_ON;
	} else {
		dev_notice(&client->dev, "led off\n");
		cmd = TC300K_CMD_LED_OFF;
	}
	ret = i2c_smbus_write_byte_data(client, TC300K_CMD_ADDR, cmd);
	if (ret < 0)
		dev_err(&client->dev, "%s fail(%d)\n", __func__, ret);

	msleep(TC300K_CMD_DELAY);

	return count;
}

static int load_fw_in_kernel(struct tc300k_data *data)
{
	struct i2c_client *client = data->client;
	int ret;

	ret = request_firmware(&data->fw, data->pdata->fw_name, &client->dev);
	if (ret) {
		dev_err(&client->dev, "%s fail(%d)\n", __func__, ret);
		return -1;
	}
	data->fw_img = (struct fw_image *)data->fw->data;

	dev_info(&client->dev, "0x%x firm (size=%d)\n",
		data->fw_img->first_fw_ver, data->fw_img->fw_len);
	dev_info(&client->dev, "%s done\n", __func__);

	return 0;
}

static int load_fw_sdcard(struct tc300k_data *data)
{
	struct i2c_client *client = data->client;
	struct file *fp;
	mm_segment_t old_fs;
	long fsize, nread;
	int ret = 0;

	old_fs = get_fs();
	set_fs(get_ds());
	fp = filp_open(TC300K_FW_PATH_SDCARD, O_RDONLY, S_IRUSR);
	if (IS_ERR(fp)) {
		dev_err(&client->dev, "%s %s open error\n",
			__func__, TC300K_FW_PATH_SDCARD);
		ret = -ENOENT;
		goto fail_sdcard_open;
	}

	fsize = fp->f_path.dentry->d_inode->i_size;

	data->fw_img = kzalloc((size_t)fsize, GFP_KERNEL);
	if (!data->fw_img) {
		dev_err(&client->dev, "%s fail to kzalloc for fw\n", __func__);
		filp_close(fp, current->files);
		ret = -ENOMEM;
		goto fail_sdcard_kzalloc;
	}

	nread = vfs_read(fp, (char __user *)data->fw_img, fsize, &fp->f_pos);
	if (nread != fsize) {
		dev_err(&client->dev,
				"%s fail to vfs_read file\n", __func__);
		ret = -EINVAL;
		goto fail_sdcard_size;
	}
	filp_close(fp, current->files);
	set_fs(old_fs);

	dev_info(&client->dev, "fw_size : %lu\n", nread);
	dev_info(&client->dev, "%s done\n", __func__);

	return ret;

fail_sdcard_size:
	kfree(&data->fw_img);
fail_sdcard_kzalloc:
	filp_close(fp, current->files);
fail_sdcard_open:
	set_fs(old_fs);

	return ret;
}

static inline void setsda(struct tc300k_data *data, int state)
{
	if (state)
		gpio_direction_output(data->pdata->gpio_sda, 1);
	else
		gpio_direction_output(data->pdata->gpio_sda, 0);
}

static inline void setscl(struct tc300k_data *data, int state)
{
	if (state)
		gpio_direction_output(data->pdata->gpio_scl, 1);
	else
		gpio_direction_output(data->pdata->gpio_scl, 0);
}

static inline int getsda(struct tc300k_data *data)
{
	return gpio_get_value(data->pdata->gpio_sda);
}

static inline int getscl(struct tc300k_data *data)
{
	return gpio_get_value(data->pdata->gpio_scl);
}

static void send_9bit(struct tc300k_data *data, u8 buff)
{
	int i;

	setscl(data, 1);
	ndelay(20);
	setsda(data, 0);
	ndelay(20);
	setscl(data, 0);
	ndelay(20);

	for (i = 0; i < 8; i++) {
		setscl(data, 1);
		ndelay(20);
		setsda(data, (buff >> i) & 0x01);
		ndelay(20);
		setscl(data, 0);
		ndelay(20);
	}

	setsda(data, 0);
}

static u8 wait_9bit(struct tc300k_data *data)
{
	int i;
	int buf;
	u8 send_buf = 0;

	gpio_direction_input(data->pdata->gpio_sda);
	s3c_gpio_setpull(data->pdata->gpio_sda, S3C_GPIO_PULL_NONE);
	getsda(data);
	ndelay(10);
	setscl(data, 1);
	ndelay(40);
	setscl(data, 0);
	ndelay(20);

	for (i = 0; i < 8; i++) {
		setscl(data, 1);
		ndelay(20);
		buf = getsda(data);
		ndelay(20);
		setscl(data, 0);
		ndelay(20);
		send_buf |= (buf & 0x01) << i;
	}
	setsda(data, 0);

	return send_buf;
}

static void tc300k_reset_for_isp(struct tc300k_data *data, bool start)
{
	if (start) {
		data->pdata->keyled(false);
		data->pdata->power_isp(false);

		msleep(100);

		data->pdata->power_isp(true);

		usleep_range(5000, 6000);
	} else {
		data->pdata->keyled(false);

		msleep(100);

		data->pdata->power(true);
		msleep(70);
		data->pdata->keyled(true);
		msleep(130);

		gpio_direction_input(data->pdata->gpio_sda);
		gpio_direction_input(data->pdata->gpio_scl);
	}
}

static void load(struct tc300k_data *data, u8 buff)
{
    send_9bit(data, TC300K_LDDATA);
    udelay(1);
    send_9bit(data, buff);
    udelay(1);
}

static void step(struct tc300k_data *data, u8 buff)
{
    send_9bit(data, TC300K_CCFG);
    udelay(1);
    send_9bit(data, buff);
    udelay(2);
}

static void setpc(struct tc300k_data *data, u16 addr)
{
    u8 buf[4];
    int i;

    buf[0] = 0x02;
    buf[1] = addr >> 8;
    buf[2] = addr & 0xff;
    buf[3] = 0x00;

    for (i = 0; i < 4; i++)
        step(data, buf[i]);
}

static void configure_isp(struct tc300k_data *data)
{
    u8 buf[7];
    int i;

    buf[0] = 0x75;    buf[1] = 0xFC;    buf[2] = 0xAC;
    buf[3] = 0x75;    buf[4] = 0xFC;    buf[5] = 0x35;
    buf[6] = 0x00;

    /* Step(cmd) */
    for (i = 0; i < 7; i++)
        step(data, buf[i]);
}

static int tc300k_erase_fw(struct tc300k_data *data)
{
	struct i2c_client *client = data->client;
	int i;
	u8 state = 0;

	tc300k_reset_for_isp(data, true);

	/* isp_enable_condition */
	send_9bit(data, TC300K_CSYNC1);
	udelay(9);
	send_9bit(data, TC300K_CSYNC2);
	udelay(9);
	send_9bit(data, TC300K_CSYNC3);
	usleep_range(150, 160);

	state = wait_9bit(data);
	if (state != 0x01) {
		dev_err(&client->dev, "%s isp enable error %d\n", __func__, state);
		return -1;
	}

	configure_isp(data);

	/* Full Chip Erase */
	send_9bit(data, TC300K_PCRST);
	udelay(1);
	send_9bit(data, TC300K_PECHIP);
	usleep_range(15000, 15500);


	state = 0;
	for (i = 0; i < 100; i++) {
		udelay(2);
		send_9bit(data, TC300K_CSYNC3);
		udelay(1);

		state = wait_9bit(data);
		if ((state & 0x04) == 0x00)
			break;
	}

	if (i == 100) {
		dev_err(&client->dev, "%s fail\n", __func__);
		return -1;
	}

	dev_info(&client->dev, "%s success\n", __func__);
	return 0;
}

static int tc300k_write_fw(struct tc300k_data *data)
{
	struct i2c_client *client = data->client;
	u16 addr = 0;
	u8 code_data;

	setpc(data, addr);
	load(data, TC300K_PWDATA);
	send_9bit(data, TC300K_LDMODE);
	udelay(1);

	while (addr < data->fw_img->fw_len) {
		code_data = data->fw_img->data[addr++];
		load(data, code_data);
		usleep_range(20, 21);
	}

	send_9bit(data, TC300K_PEDISC);
	udelay(1);

	return 0;
}

static int tc300k_verify_fw(struct tc300k_data *data)
{
	struct i2c_client *client = data->client;
	u16 addr = 0;
	u8 code_data;

	setpc(data, addr);

	dev_info(&client->dev, "fw code size = %#x (%u)",
		data->fw_img->fw_len, data->fw_img->fw_len);
	while (addr < data->fw_img->fw_len) {
		if ((addr % 0x40) == 0)
			dev_info(&client->dev, "fw verify addr = %#x\n", addr);

		send_9bit(data, TC300K_PRDATA);
		udelay(2);
		code_data = wait_9bit(data);
		udelay(1);

		if (code_data != data->fw_img->data[addr++]) {
			dev_err(&client->dev,
				"%s addr : %#x data error (0x%2x)\n",
				__func__, addr - 1, code_data );
			return -1;
		}
	}
	dev_info(&client->dev, "%s success\n", __func__);

	return 0;
}

static void t300k_release_fw(struct tc300k_data *data, u8 fw_path)
{
	if (fw_path == FW_INKERNEL)
		release_firmware(data->fw);
	else if (fw_path == FW_SDCARD)
		kfree(data->fw_img);
}

static int tc300k_flash_fw(struct tc300k_data *data, u8 fw_path)
{
	struct i2c_client *client = data->client;
	int retry = 5;
	int ret;

	do {
		ret = tc300k_erase_fw(data);
		if (ret)
			dev_err(&client->dev, "%s erase fail(retry=%d)\n",
				__func__, retry);
		else
			break;
	} while (retry-- > 0);
	if (retry < 0)
		goto err_tc300k_flash_fw;

	retry = 5;
	do {
		tc300k_write_fw(data);

		ret = tc300k_verify_fw(data);
		if (ret)
			dev_err(&client->dev, "%s verify fail(retry=%d)\n",
				__func__, retry);
		else
			break;
	} while (retry-- > 0);

	tc300k_reset_for_isp(data, false);

	if (retry < 0)
		goto err_tc300k_flash_fw;

	return 0;

err_tc300k_flash_fw:

	return -1;
}

static int tc300k_crc_check(struct tc300k_data *data)
{
	struct i2c_client *client = data->client;
	int ret;
	u16 checksum;
	u8 cmd;
	u8 checksum_h, checksum_l;

	if ((!data->enabled) || data->fw_downloding) {
		dev_err(&client->dev, "%s can't excute\n", __func__);
		return -1;
	}

	cmd = TC300K_CMD_CAL_CHECKSUM;
	ret = i2c_smbus_write_byte_data(client, TC300K_CMD_ADDR, cmd);
	if (ret) {
		dev_err(&client->dev, "%s command fail (%d)\n", __func__, ret);
		return ret;
	}

	msleep(TC300K_CHECKSUM_DELAY);

	ret = i2c_smbus_read_byte_data(client, TC300K_CHECKS_H);
	if (ret < 0) {
		dev_err(&client->dev, "%s: failed to read checksum_h (%d)\n",
			__func__, ret);
		return ret;
	}
	checksum_h = ret;

	ret = i2c_smbus_read_byte_data(client, TC300K_CHECKS_L);
	if (ret < 0) {
		dev_err(&client->dev, "%s: failed to read checksum_l (%d)\n",
			__func__, ret);
		return ret;
	}
	checksum_l = ret;

	checksum = (checksum_h << 8) | checksum_l;

	if (data->fw_img->checksum != checksum) {
		dev_err(&client->dev,
			"%s checksum fail - firm checksum(%d), compute checksum(%d)\n",
			__func__, data->fw_img->checksum, checksum);
		return -1;
	}

	dev_info(&client->dev, "%s success (%d)\n", __func__, checksum);

	return 0;
}

static int tc300k_fw_update(struct tc300k_data *data, u8 fw_path, bool force)
{
	struct i2c_client *client = data->client;
	int retry = 4;
	int ret;

	if (fw_path == FW_INKERNEL) {
		if (!force) {
			ret = get_fw_version(data, false);
			if (ret)
				return -1;
		}

		ret = load_fw_in_kernel(data);
		if (ret)
			return -1;

		if (!force && (data->fw_ver >= data->fw_img->first_fw_ver)) {
			dev_notice(&client->dev, "do not need firm update (0x%x, 0x%x)\n",
				data->fw_ver, data->fw_img->first_fw_ver);
			t300k_release_fw(data, fw_path);
			return 0;
		}
	} else if (fw_path == FW_SDCARD) {
		ret = load_fw_sdcard(data);
		if (ret)
			return -1;
	}

	while (retry--) {
		data->fw_downloding = true;
		ret = tc300k_flash_fw(data, fw_path);
		data->fw_downloding = false;
		if (ret) {
			dev_err(&client->dev, "%s tc300k_flash_fw fail (%d)\n",
				__func__, retry);
			continue;
		}

		ret = get_fw_version(data, false);
		if (ret) {
			dev_err(&client->dev, "%s get_fw_version fail (%d)\n",
				__func__, retry);
			continue;
		}
		if (data->fw_ver != data->fw_img->first_fw_ver) {
			dev_err(&client->dev, "%s fw version fail (0x%x, 0x%x)(%d)\n",
				__func__, data->fw_ver, data->fw_img->first_fw_ver, retry);
			continue;
		}

		ret = tc300k_crc_check(data);
		if (ret) {
			dev_err(&client->dev, "%s crc check fail (%d)\n",
				__func__, retry);
			continue;
		}
		break;
	}

	if (retry > 0)
		dev_info(&client->dev, "%s success\n", __func__);

	t300k_release_fw(data, fw_path);

	return ret;
}

static ssize_t tc300k_update_store(struct device *dev,
	 struct device_attribute *attr, const char *buf, size_t count)
{
	struct tc300k_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	int ret;
	u8 fw_path;

	switch(*buf) {
	case 's':
	case 'S':
		fw_path = FW_INKERNEL;
		break;
	case 'i':
	case 'I':
		fw_path = FW_SDCARD;
		break;
	default:
		dev_err(&client->dev, "%s wrong command fail\n", __func__);
		data->fw_update_status = TK_UPDATE_FAIL;
		return count;
	}

	data->fw_update_status = TK_UPDATE_DOWN;

	disable_irq(data->irq);
	ret = tc300k_fw_update(data, fw_path, false);
	enable_irq(data->irq);
	if (ret < 0) {
		dev_err(&client->dev, "%s fail\n", __func__);
		data->fw_update_status = TK_UPDATE_FAIL;
	} else
		data->fw_update_status = TK_UPDATE_PASS;

	return count;
}

static ssize_t tc300k_firm_status_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct tc300k_data *data = dev_get_drvdata(dev);
	int ret;

	if (data->fw_update_status == TK_UPDATE_PASS)
		ret = sprintf(buf, "PASS\n");
	else if (data->fw_update_status == TK_UPDATE_DOWN)
		ret = sprintf(buf, "DOWNLOADING\n");
	else if (data->fw_update_status == TK_UPDATE_FAIL)
		ret = sprintf(buf, "FAIL\n");
	else
		ret = sprintf(buf, "NG\n");

	return ret;
}

static ssize_t tc300k_firm_version_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct tc300k_data *data = dev_get_drvdata(dev);

	return sprintf(buf, "0x%02x\n", data->pdata->fw_version);
}

static ssize_t tc300k_firm_version_read_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct tc300k_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	int ret;

	ret = get_fw_version(data, false);
	if (ret < 0)
		dev_err(&client->dev, "%s: failed to read firmware version (%d)\n",
			__func__, ret);

	return sprintf(buf, "0x%02x\n", data->fw_ver);
}

static ssize_t recent_key_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct tc300k_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	int ret;
	u8 buff[6];
	int value;

	if ((!data->enabled) || data->fw_downloding) {
		dev_err(&client->dev, "can't excute %s\n", __func__);
		return -1;
	}

	ret = i2c_smbus_read_i2c_block_data(client, TC300K_2KEY_DATA, 6, buff);
	if (ret != 6) {
		dev_err(&client->dev, "%s read fail(%d)\n", __func__, ret);
		return -1;
	}
	value = (buff[TC300K_CH_PCK_H_OFFSET] << 8) |
		buff[TC300K_CH_PCK_L_OFFSET];

	return sprintf(buf, "%d\n", value);
}

static ssize_t recent_key_ref_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct tc300k_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	int ret;
	u8 buff[6];
	int value;

	if ((!data->enabled) || data->fw_downloding) {
		dev_err(&client->dev, "can't excute %s\n", __func__);
		return -1;
	}

	if (data->pdata->sensing_ch_num < 6)
		return sprintf(buf, "%d\n", 0);

	ret = i2c_smbus_read_i2c_block_data(client, TC300K_6KEY_DATA, 6, buff);
	if (ret != 6) {
		dev_err(&client->dev, "%s read fail(%d)\n", __func__, ret);
		return -1;
	}
	value = (buff[TC300K_CH_PCK_H_OFFSET] << 8) |
		buff[TC300K_CH_PCK_L_OFFSET];

	return sprintf(buf, "%d\n", value);
}

static ssize_t back_key_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct tc300k_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	int ret;
	u8 buff[6];
	int value;

	if ((!data->enabled) || data->fw_downloding) {
		dev_err(&client->dev, "can't excute %s\n", __func__);
		return -1;
	}

	ret = i2c_smbus_read_i2c_block_data(client, TC300K_1KEY_DATA, 6, buff);
	if (ret != 6) {
		dev_err(&client->dev, "%s read fail(%d)\n", __func__, ret);
		return -1;
	}
	value = (buff[TC300K_CH_PCK_H_OFFSET] << 8) |
		buff[TC300K_CH_PCK_L_OFFSET];

	return sprintf(buf, "%d\n", value);
}

static ssize_t back_key_ref_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct tc300k_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	int ret;
	u8 buff[6];
	int value;

	if ((!data->enabled) || data->fw_downloding) {
		dev_err(&client->dev, "can't excute %s\n", __func__);
		return -1;
	}

	if (data->pdata->sensing_ch_num < 6)
		return sprintf(buf, "%d\n", 0);

	ret = i2c_smbus_read_i2c_block_data(client, TC300K_5KEY_DATA, 6, buff);
	if (ret != 6) {
		dev_err(&client->dev, "%s read fail(%d)\n", __func__, ret);
		return -1;
	}
	value = (buff[TC300K_CH_PCK_H_OFFSET] << 8) |
		buff[TC300K_CH_PCK_L_OFFSET];

	return sprintf(buf, "%d\n", value);
}

static ssize_t dummy_recent_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct tc300k_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	int ret;
	u8 buff[6];
	int value;

	if ((!data->enabled) || data->fw_downloding) {
		dev_err(&client->dev, "can't excute %s\n", __func__);
		return -1;
	}

	ret = i2c_smbus_read_i2c_block_data(client, TC300K_4KEY_DATA, 6, buff);
	if (ret != 6) {
		dev_err(&client->dev, "%s read fail(%d)\n", __func__, ret);
		return -1;
	}
	value = (buff[TC300K_CH_PCK_H_OFFSET] << 8) |
		buff[TC300K_CH_PCK_L_OFFSET];

	return sprintf(buf, "%d\n", value);
}

static ssize_t dummy_back_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct tc300k_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	int ret;
	u8 buff[6];
	int value;

	if ((!data->enabled) || data->fw_downloding) {
		dev_err(&client->dev, "can't excute %s\n", __func__);
		return -1;
	}

	ret = i2c_smbus_read_i2c_block_data(client, TC300K_3KEY_DATA, 6, buff);
	if (ret != 6) {
		dev_err(&client->dev, "%s read fail(%d)\n", __func__, ret);
		return -1;
	}
	value = (buff[TC300K_CH_PCK_H_OFFSET] << 8) |
		buff[TC300K_CH_PCK_L_OFFSET];

	return sprintf(buf, "%d\n", value);
}

static ssize_t recent_key_raw(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct tc300k_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	int ret;
	u8 buff[6];
	int value;

	if ((!data->enabled) || data->fw_downloding) {
		dev_err(&client->dev, "can't excute %s\n", __func__);
		return -1;
	}

	ret = i2c_smbus_read_i2c_block_data(client, TC300K_2KEY_DATA, 6, buff);
	if (ret != 6) {
		dev_err(&client->dev, "%s read fail(%d)\n", __func__, ret);
		return -1;
	}
	value = (buff[TC300K_RAW_H_OFFSET] << 8) |
		buff[TC300K_RAW_L_OFFSET];

	return sprintf(buf, "%d\n", value);
}

static ssize_t recent_key_raw_ref(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct tc300k_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	int ret;
	u8 buff[6];
	int value;

	if ((!data->enabled) || data->fw_downloding) {
		dev_err(&client->dev, "can't excute %s\n", __func__);
		return -1;
	}

	if (data->pdata->sensing_ch_num < 6)
		return sprintf(buf, "%d\n", 0);

	ret = i2c_smbus_read_i2c_block_data(client, TC300K_6KEY_DATA, 6, buff);
	if (ret != 6) {
		dev_err(&client->dev, "%s read fail(%d)\n", __func__, ret);
		return -1;
	}
	value = (buff[TC300K_RAW_H_OFFSET] << 8) |
		buff[TC300K_RAW_L_OFFSET];

	return sprintf(buf, "%d\n", value);
}

static ssize_t back_key_raw(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct tc300k_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	int ret;
	u8 buff[6];
	int value;

	if ((!data->enabled) || data->fw_downloding) {
		dev_err(&client->dev, "can't excute %s\n", __func__);
		return -1;
	}

	ret = i2c_smbus_read_i2c_block_data(client, TC300K_1KEY_DATA, 6, buff);
	if (ret != 6) {
		dev_err(&client->dev, "%s read fail(%d)\n", __func__, ret);
		return -1;
	}
	value = (buff[TC300K_RAW_H_OFFSET] << 8) |
		buff[TC300K_RAW_L_OFFSET];

	return sprintf(buf, "%d\n", value);
}

static ssize_t back_key_raw_ref(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct tc300k_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	int ret;
	u8 buff[6];
	int value;

	if ((!data->enabled) || data->fw_downloding) {
		dev_err(&client->dev, "can't excute %s\n", __func__);
		return -1;
	}

	if (data->pdata->sensing_ch_num < 6)
		return sprintf(buf, "%d\n", 0);

	ret = i2c_smbus_read_i2c_block_data(client, TC300K_5KEY_DATA, 6, buff);
	if (ret != 6) {
		dev_err(&client->dev, "%s read fail(%d)\n", __func__, ret);
		return -1;
	}
	value = (buff[TC300K_RAW_H_OFFSET] << 8) |
		buff[TC300K_RAW_L_OFFSET];

	return sprintf(buf, "%d\n", value);
}

static ssize_t dummy_recent_raw(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct tc300k_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	int ret;
	u8 buff[6];
	int value;

	if ((!data->enabled) || data->fw_downloding) {
		dev_err(&client->dev, "can't excute %s\n", __func__);
		return -1;
	}

	ret = i2c_smbus_read_i2c_block_data(client, TC300K_4KEY_DATA, 6, buff);
	if (ret != 6) {
		dev_err(&client->dev, "%s read fail(%d)\n", __func__, ret);
		return -1;
	}
	value = (buff[TC300K_RAW_H_OFFSET] << 8) |
		buff[TC300K_RAW_L_OFFSET];

	return sprintf(buf, "%d\n", value);
}

static ssize_t dummy_back_raw(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct tc300k_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	int ret;
	u8 buff[6];
	int value;

	if ((!data->enabled) || data->fw_downloding) {
		dev_err(&client->dev, "can't excute %s\n", __func__);
		return -1;
	}

	ret = i2c_smbus_read_i2c_block_data(client, TC300K_3KEY_DATA, 6, buff);
	if (ret != 6) {
		dev_err(&client->dev, "%s read fail(%d)\n", __func__, ret);
		return -1;
	}
	value = (buff[TC300K_RAW_H_OFFSET] << 8) |
		buff[TC300K_RAW_L_OFFSET];

	return sprintf(buf, "%d\n", value);
}

static int tc300k_factory_mode_enable(struct i2c_client *client, u8 cmd)
{
	int ret;

	ret = i2c_smbus_write_byte_data(client, TC300K_CMD_ADDR, cmd);
	msleep(TC300K_CMD_DELAY);

	return ret;
}

static ssize_t tc300k_factory_mode(struct device *dev,
	 struct device_attribute *attr, const char *buf, size_t count)
{
	struct tc300k_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	int scan_buffer;
	int ret;
	u8 cmd;

	ret = sscanf(buf, "%d", &scan_buffer);
	if (ret != 1) {
		dev_err(&client->dev, "%s: cmd read err\n", __func__);
		return count;
	}

	if (!(scan_buffer == 0 || scan_buffer == 1)) {
		dev_err(&client->dev, "%s: wrong command(%d)\n",
			__func__, scan_buffer);
		return count;
	}

	if (data->factory_mode == (bool)scan_buffer) {
		dev_info(&client->dev, "%s same command(%d)\n",
			__func__, scan_buffer);
		return count;
	}

	if (scan_buffer == 1) {
		dev_notice(&client->dev, "factory mode\n");
		cmd = TC300K_CMD_FAC_ON;
	} else {
		dev_notice(&client->dev, "normal mode\n");
		cmd = TC300K_CMD_FAC_OFF;
	}

	if ((!data->enabled) || data->fw_downloding) {
		dev_err(&client->dev, "can't excute %s\n", __func__);
		data->factory_mode = (bool)scan_buffer;
		return count;
	}

	ret = tc300k_factory_mode_enable(client, cmd);
	if (ret < 0)
		dev_err(&client->dev, "%s fail(%d)\n", __func__, ret);

	data->factory_mode = (bool)scan_buffer;

	return count;
}

static ssize_t tc300k_factory_mode_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct tc300k_data *data = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", data->factory_mode);
}

static int tc300k_glove_mode_enable(struct i2c_client *client, u8 cmd)
{
	int ret;

	ret = i2c_smbus_write_byte_data(client, TC300K_CMD_ADDR, cmd);
	msleep(TC300K_CMD_DELAY);

	return ret;
}

static ssize_t tc300k_glove_mode(struct device *dev,
	 struct device_attribute *attr, const char *buf, size_t count)
{
	struct tc300k_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	int scan_buffer;
	int ret;
	u8 cmd;

	ret = sscanf(buf, "%d", &scan_buffer);
	if (ret != 1) {
		dev_err(&client->dev, "%s: cmd read err\n", __func__);
		return count;
	}

	if (!(scan_buffer == 0 || scan_buffer == 1)) {
		dev_err(&client->dev, "%s: wrong command(%d)\n",
			__func__, scan_buffer);
		return count;
	}

	if (data->glove_mode == (bool)scan_buffer) {
		dev_info(&client->dev, "%s same command(%d)\n",
			__func__, scan_buffer);
		return count;
	}

	if (scan_buffer == 1) {
		dev_notice(&client->dev, "glove mode\n");
		cmd = TC300K_CMD_GLOVE_ON;
	} else {
		dev_notice(&client->dev, "normal mode\n");
		cmd = TC300K_CMD_GLOVE_OFF;
	}

	if ((!data->enabled) || data->fw_downloding) {
		dev_err(&client->dev, "can't excute %s\n", __func__);
		data->glove_mode = (bool)scan_buffer;
		return count;
	}
	ret = tc300k_glove_mode_enable(client, cmd);
	if (ret < 0)
		dev_err(&client->dev, "%s fail(%d)\n", __func__, ret);

	data->glove_mode = (bool)scan_buffer;

	return count;
}

static ssize_t tc300k_glove_mode_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct tc300k_data *data = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", data->glove_mode);
}

static ssize_t tc300k_modecheck_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct tc300k_data *data = dev_get_drvdata(dev);
	struct i2c_client *client = data->client;
	int ret;
	u8 mode, glove, factory;

	if ((!data->enabled) || data->fw_downloding) {
		dev_err(&client->dev, "can't excute %s\n", __func__);
		return -EPERM;
	}

	ret = i2c_smbus_read_byte_data(client, TC300K_MODE);
	if (ret < 0) {
		dev_err(&client->dev, "%s: failed to read threshold_h (%d)\n",
			__func__, ret);
		return ret;
	}
	mode = ret;

	glove = ((mode & 0xf0) >> 4);
	factory = mode & 0x0f;

	return sprintf(buf, "glove:%d, factory:%d\n", glove, factory);
}

#if defined(CONFIG_PM)
static ssize_t tc300k_enabled_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct tc300k_data *data = i2c_get_clientdata(client);
	unsigned int input;
	if (sscanf(buf, "%u", &input) != 1)
		return -EINVAL;

	if (input == 0){
		tc300k_tk_enabled = 0;
		tc300k_suspend(&data->client->dev);
	}
	else if (input == 1){
		tc300k_tk_enabled = 1;
		tc300k_resume(&data->client->dev);
        } else {
		return -EINVAL;
        }

	return size;
}

static ssize_t tc300k_enabled_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct tc300k_data *data = i2c_get_clientdata(client);

	return snprintf(buf, PAGE_SIZE, "%u\n", data->enabled);
}
#endif

static DEVICE_ATTR(touchkey_threshold, S_IRUGO, tc300k_threshold_show, NULL);
static DEVICE_ATTR(brightness, S_IRUGO | S_IWUSR | S_IWGRP, NULL,
		tc300k_led_control);
static DEVICE_ATTR(touchkey_firm_update, S_IRUGO | S_IWUSR | S_IWGRP,
		NULL, tc300k_update_store);
static DEVICE_ATTR(touchkey_firm_update_status, S_IRUGO,
		tc300k_firm_status_show, NULL);
static DEVICE_ATTR(touchkey_firm_version_phone, S_IRUGO,
		tc300k_firm_version_show, NULL);
static DEVICE_ATTR(touchkey_firm_version_panel, S_IRUGO,
		tc300k_firm_version_read_show, NULL);
static DEVICE_ATTR(touchkey_recent, S_IRUGO, recent_key_show, NULL);
static DEVICE_ATTR(touchkey_recent_ref, S_IRUGO, recent_key_ref_show, NULL);
static DEVICE_ATTR(touchkey_back, S_IRUGO, back_key_show, NULL);
static DEVICE_ATTR(touchkey_back_ref, S_IRUGO, back_key_ref_show, NULL);
static DEVICE_ATTR(touchkey_d_menu, S_IRUGO, dummy_recent_show, NULL);
static DEVICE_ATTR(touchkey_d_back, S_IRUGO, dummy_back_show, NULL);
static DEVICE_ATTR(touchkey_recent_raw, S_IRUGO, recent_key_raw, NULL);
static DEVICE_ATTR(touchkey_recent_raw_ref, S_IRUGO, recent_key_raw_ref, NULL);
static DEVICE_ATTR(touchkey_back_raw, S_IRUGO, back_key_raw, NULL);
static DEVICE_ATTR(touchkey_back_raw_ref, S_IRUGO, back_key_raw_ref, NULL);
static DEVICE_ATTR(touchkey_d_menu_raw, S_IRUGO, dummy_recent_raw, NULL);
static DEVICE_ATTR(touchkey_d_back_raw, S_IRUGO, dummy_back_raw, NULL);
static DEVICE_ATTR(touchkey_factory_mode, S_IRUGO | S_IWUSR | S_IWGRP,
		tc300k_factory_mode_show, tc300k_factory_mode);
static DEVICE_ATTR(glove_mode, S_IRUGO | S_IWUSR | S_IWGRP,
		tc300k_glove_mode_show, tc300k_glove_mode);
static DEVICE_ATTR(modecheck, S_IRUGO, tc300k_modecheck_show, NULL);
#if defined(CONFIG_PM)
static DEVICE_ATTR(touchkey_enabled, S_IRUGO | S_IWUSR | S_IWGRP,
		tc300k_enabled_show, tc300k_enabled_store);
#endif

static struct attribute *sec_touchkey_attributes[] = {
	&dev_attr_touchkey_threshold.attr,
	&dev_attr_brightness.attr,
	&dev_attr_touchkey_firm_update.attr,
	&dev_attr_touchkey_firm_update_status.attr,
	&dev_attr_touchkey_firm_version_phone.attr,
	&dev_attr_touchkey_firm_version_panel.attr,
	&dev_attr_touchkey_recent.attr,
	&dev_attr_touchkey_recent_ref.attr,
	&dev_attr_touchkey_back.attr,
	&dev_attr_touchkey_back_ref.attr,
	&dev_attr_touchkey_d_menu.attr,
	&dev_attr_touchkey_d_back.attr,
	&dev_attr_touchkey_recent_raw.attr,
	&dev_attr_touchkey_recent_raw_ref.attr,
	&dev_attr_touchkey_back_raw.attr,
	&dev_attr_touchkey_back_raw_ref.attr,
	&dev_attr_touchkey_d_menu_raw.attr,
	&dev_attr_touchkey_d_back_raw.attr,
	&dev_attr_touchkey_factory_mode.attr,
	&dev_attr_glove_mode.attr,
	&dev_attr_modecheck.attr,
#if defined(CONFIG_PM)
	&dev_attr_touchkey_enabled.attr,
#endif
	NULL,
};

static struct attribute_group sec_touchkey_attr_group = {
	.attrs = sec_touchkey_attributes,
};

static int tc300k_fw_check(struct tc300k_data *data)
{
	struct i2c_client *client = data->client;
	int ret;
	bool update = false;

	ret = get_fw_version(data, true);
	if (ret < 0) {
		if (data->pdata->panel_connect) {
			/* tsp connect check */
			dev_err(&client->dev,
				"%s: i2c fail. but lcd connected\n",
				__func__);
			dev_err(&client->dev,
				"excute firm update\n");
			update = true;
		} else {
			dev_err(&client->dev,
				"%s: i2c fail...[%d], addr[%d]\n",
				__func__, ret, data->client->addr);
			dev_err(&client->dev,
				"%s: touchkey driver unload\n", __func__);
			return ret;
		}
	}

	if (!update &&
			((data->fw_ver < data->pdata->fw_version) ||
			(data->fw_ver == 0xFF))) {
		dev_notice(&client->dev,
			"fw version check excute firmware update(0x%x -> 0x%x)\n",
			data->fw_ver, data->pdata->fw_version);
		update = true;
	}

	if (update) {
		ret = tc300k_fw_update(data, FW_INKERNEL, true);
		if (ret)
			return -1;
	}

	return 0;
}

static int __devinit tc300k_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
	struct input_dev *input_dev;
	struct tc300k_data *data;
	int ret;
	int i;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev,
			"i2c_check_functionality fail\n");
		return -EIO;
	}

	data = kzalloc(sizeof(struct tc300k_data), GFP_KERNEL);
	if (!data) {
		dev_err(&client->dev, "Failed to allocate memory\n");
		ret = -ENOMEM;
		goto err_alloc_data;
	}

	input_dev = input_allocate_device();
	if (!input_dev) {
		dev_err(&client->dev,
			"Failed to allocate memory for input device\n");
		ret = -ENOMEM;
		goto err_alloc_input;
	}

	data->client = client;
	data->input_dev = input_dev;
	data->pdata = client->dev.platform_data;
	if (data->pdata == NULL) {
		pr_err("failed to get platform data\n");
		ret = -EINVAL;
		goto err_platform_data;
	}
	data->irq = -1;
	mutex_init(&data->lock);

	data->key_num = data->pdata->key_num;
	dev_info(&client->dev, "number of keys = %d\n", data->key_num);
	data->keycode = data->pdata->keycode;
	dev_err(&client->dev, "fw_ver_bin = 0x%x\n", data->pdata->fw_version);
#if !defined(CONFIG_SAMSUNG_PRODUCT_SHIP)
	for (i = 1; i < data->key_num; i++)
		dev_info(&client->dev, "keycode[%d]= %3d\n", i, data->keycode[i]);
#endif
	i2c_set_clientdata(client, data);

	data->pdata->power(true);
	msleep(70);
	data->pdata->keyled(true);
	msleep(130);
	data->enabled = true;

	ret = tc300k_fw_check(data);
	if (ret) {
		dev_err(&client->dev,
			"failed to firmware check(%d)\n", ret);
		goto err_fw_check;
	}

	snprintf(data->phys, sizeof(data->phys),
		"%s/input0", dev_name(&client->dev));
	input_dev->name = "sec_touchkey";
	input_dev->phys = data->phys;
	input_dev->id.bustype = BUS_I2C;
	input_dev->dev.parent = &client->dev;
	input_dev->open = tc300k_input_open;
	input_dev->close = tc300k_input_close;

	set_bit(EV_KEY, input_dev->evbit);
	set_bit(EV_LED, input_dev->evbit);
	set_bit(LED_MISC, input_dev->ledbit);
	for (i = 1; i < data->key_num; i++)
		set_bit(data->keycode[i], input_dev->keybit);
	input_set_drvdata(input_dev, data);

	ret = input_register_device(input_dev);
	if (ret) {
		dev_err(&client->dev, "fail to register input_dev (%d)\n",
			ret);
		goto err_register_input_dev;
	}

	ret = request_threaded_irq(client->irq, NULL, tc300k_interrupt,
				IRQF_TRIGGER_FALLING, TC300K_NAME, data);
	if (ret < 0) {
		dev_err(&client->dev, "fail to request irq (%d).\n",
			client->irq);
		goto err_request_irq;
	}
	data->irq = client->irq;

#ifdef CONFIG_HAS_EARLYSUSPEND
	data->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	data->early_suspend.suspend = tc300k_early_suspend;
	data->early_suspend.resume = tc300k_late_resume;
	register_early_suspend(&data->early_suspend);
#endif

	data->sec_touchkey = device_create(sec_class,
		NULL, 0, data, "sec_touchkey");
	if (IS_ERR(data->sec_touchkey))
		dev_err(&client->dev,
			"Failed to create device for the touchkey sysfs\n");

	ret = sysfs_create_group(&data->sec_touchkey->kobj,
		&sec_touchkey_attr_group);
	if (ret)
		dev_err(&client->dev, "Failed to create sysfs group\n");

	ret = sysfs_create_link(&data->sec_touchkey->kobj,
		&data->input_dev->dev.kobj, "input");
	if (ret)
		dev_err(&client->dev, "Failed to connect link\n");

	dev_info(&client->dev, "%s done\n", __func__);
	return 0;

err_request_irq:
	input_unregister_device(input_dev);
err_register_input_dev:
err_fw_check:
	mutex_destroy(&data->lock);
	data->pdata->keyled(false);
	data->pdata->power(false);
err_platform_data:
	input_free_device(input_dev);
err_alloc_input:
	kfree(data);
err_alloc_data:
	return ret;
}

static int __devexit tc300k_remove(struct i2c_client *client)
{
	struct tc300k_data *data = i2c_get_clientdata(client);

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&data->early_suspend);
#endif
	free_irq(data->irq, data);
	input_unregister_device(data->input_dev);
	input_free_device(data->input_dev);
	mutex_destroy(&data->lock);
	data->pdata->keyled(false);
	data->pdata->power(false);
	gpio_free(data->pdata->gpio_int);
	gpio_free(data->pdata->gpio_sda);
	gpio_free(data->pdata->gpio_scl);
	kfree(data);

	return 0;
}

static void tc300k_shutdown(struct i2c_client *client)
{
	struct tc300k_data *data = i2c_get_clientdata(client);

	data->pdata->keyled(false);
	data->pdata->power(false);
}

#if defined(CONFIG_PM)
static int tc300k_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct tc300k_data *data = i2c_get_clientdata(client);

	mutex_lock(&data->lock);

	if (!data->enabled) {
		mutex_unlock(&data->lock);
		return 0;
	}

	dev_notice(&data->client->dev, "%s: users=%d\n",
		__func__, data->input_dev->users);

	disable_irq(data->irq);
	data->enabled = false;
	release_all_fingers(data);
	data->pdata->keyled(false);
	data->pdata->power(false);
	data->led_on = false;

	mutex_unlock(&data->lock);

	return 0;
}

static int tc300k_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct tc300k_data *data = i2c_get_clientdata(client);
	int ret;
	u8 cmd;

	mutex_lock(&data->lock);

	if (data->enabled || tc300k_tk_enabled == 0) {
		mutex_unlock(&data->lock);
		return 0;
	}

	dev_notice(&data->client->dev, "%s: users=%d\n",
		__func__, data->input_dev->users);

	data->pdata->power(true);
	msleep(70);
	data->pdata->keyled(true);
	msleep(130);
#ifdef LED_LDO_WITH_REGULATOR
    change_touch_key_led_voltage(touchkey_voltage_brightness);
#endif
	enable_irq(data->irq);

	data->enabled = true;
	if (data->led_on == true) {
		data->led_on = false;
		dev_notice(&client->dev, "led on(resume)\n");
		cmd = TC300K_CMD_LED_ON;
		ret = i2c_smbus_write_byte_data(client, TC300K_CMD_ADDR, cmd);
		if (ret < 0)
			dev_err(&client->dev, "%s led on fail(%d)\n", __func__, ret);
		else
			msleep(TC300K_CMD_DELAY);
	}

	if (data->glove_mode) {
		ret = tc300k_glove_mode_enable(client, TC300K_CMD_GLOVE_ON);
		if (ret < 0)
			dev_err(&client->dev, "%s glovemode fail(%d)\n", __func__, ret);
	}

	if (data->factory_mode) {
		ret = tc300k_factory_mode_enable(client, TC300K_CMD_FAC_ON);
		if (ret < 0)
			dev_err(&client->dev, "%s factorymode fail(%d)\n", __func__, ret);
	}
	mutex_unlock(&data->lock);

	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void tc300k_early_suspend(struct early_suspend *h)
{
	struct tc300k_data *data;
	data = container_of(h, struct tc300k_data, early_suspend);
	tc300k_suspend(&data->client->dev);
}

static void tc300k_late_resume(struct early_suspend *h)
{
	struct tc300k_data *data;
	data = container_of(h, struct tc300k_data, early_suspend);
	tc300k_resume(&data->client->dev);
}
#endif

static void tc300k_input_close(struct input_dev *dev)
{
	struct tc300k_data *data = input_get_drvdata(dev);
	dev_info(&data->client->dev, "%s: users=%d\n", __func__,
		   data->input_dev->users);

	tc300k_suspend(&data->client->dev);
}

static int tc300k_input_open(struct input_dev *dev)
{
	struct tc300k_data *data = input_get_drvdata(dev);

	dev_info(&data->client->dev, "%s: users=%d\n", __func__,
		   data->input_dev->users);

	tc300k_resume(&data->client->dev);

	return 0;
}
#endif /* CONFIG_PM */

#if 0
#if defined(CONFIG_PM) || defined(CONFIG_HAS_EARLYSUSPEND)
static const struct dev_pm_ops tc300k_pm_ops = {
	.suspend = tc300k_suspend,
	.resume = tc300k_resume,
};
#endif
#endif

static const struct i2c_device_id tc300k_id[] = {
	{TC300K_NAME, 0},
	{ }
};
MODULE_DEVICE_TABLE(i2c, tc300k_id);

static struct i2c_driver tc300k_driver = {
	.probe = tc300k_probe,
	.remove = __devexit_p(tc300k_remove),
	.shutdown = tc300k_shutdown,
	.driver = {
		.name = TC300K_NAME,
		.owner = THIS_MODULE,
#if 0
#if defined(CONFIG_PM) && !defined(CONFIG_HAS_EARLYSUSPEND)
		.pm	= &tc370_pm_ops,
#endif
#endif
	},
	.id_table = tc300k_id,
};

static int __devinit tc300k_init(void)
{
	return i2c_add_driver(&tc300k_driver);
}

static void __exit tc300k_exit(void)
{
	i2c_del_driver(&tc300k_driver);
}

late_initcall(tc300k_init);
module_exit(tc300k_exit);

MODULE_AUTHOR("Samsung Electronics");
MODULE_DESCRIPTION("Touchkey driver for Coreriver TC300K");
MODULE_LICENSE("GPL");
