/* linux/drivers/video/backlight/lsl122dl01_edp_lcd.c
 *
 * Samsung SoC LCD driver.
 *
 * Copyright (c) 2012 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/backlight.h>
#include <linux/delay.h>
#include <linux/lcd.h>
#include <linux/err.h>
#include <linux/fb.h>
#include <video/s5p-dp.h>
#include "../s5p-dp-core.h"
#include "../secfb_notify.h"
#include <linux/platform_data/lsl122dl01_edp_lcd.h>

#define DDI_VIDEO_ENHANCE_TUNING
#if defined(DDI_VIDEO_ENHANCE_TUNING)
#include <asm/uaccess.h>
#endif

#if defined(CONFIG_N2A)
#include "n2_power_save.h"
#elif defined(CONFIG_N1A)
#include "n1_power_save.h"
#else
#include "v1_power_save.h"
#endif

#define LCD_NAME	"panel"
#define BL_NAME		"tcon"

#if defined(CONFIG_N1A)||defined(CONFIG_N2A)
#define PANEL_NAME	"INH_LSL101DL01"
#else
#define PANEL_NAME	"INH_LSL122DL01"
#endif

static struct class *tcon_class;

struct lsl122dl01 {
	const char *chipname;
	struct device *dev;
	struct i2c_client *client;
	struct lcd_device *lcd;
	struct backlight_device *bl;
	struct device *tcon_dev;
	struct s5p_dp_device *dp;
	struct notifier_block secfb_notif;
	
	int dblc_mode;
	int dblc_lux;
	int dblc_auto_br;
	int dblc_power_save;
	int dblc_duty;
#if defined(CONFIG_LCD_LSL122DL01_BATTERY)
	int dblc_battery;
#endif
	struct lsl122dl01_platform_data *pdata;
#ifdef CONFIG_S5P_DP_PSR
	struct notifier_block notifier;
	int psr_hfreq_change;
	int psr_hfreq;
	int current_hfreq_data;
	int hfreqdata_min;
	int hfreqdata_max;
#endif
	int i2c_slave;

	struct mutex ops_lock;
};

static ssize_t lcd_type_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%s\n", PANEL_NAME);
}

static ssize_t window_type_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "0 0 0\n");
}

static DEVICE_ATTR(lcd_type, S_IRUGO, lcd_type_show, NULL);
static DEVICE_ATTR(window_type, S_IRUGO, window_type_show, NULL);

static struct attribute *lcd_attributes[] = {
	&dev_attr_lcd_type.attr,
	&dev_attr_window_type.attr,
	NULL,
};

static const struct attribute_group lcd_attr_group = {
	.attrs = lcd_attributes,
};

static int lsl122dl01_lcd_match(struct lcd_device *lcd, struct fb_info *info)
{
	struct lsl122dl01 *plcd = lcd_get_data(lcd);
#if 0
	return plcd->dev->parent == info->device;
#else
	return 0;
#endif
}

static struct lcd_ops lsl122dl01_lcd_ops = {
	.check_fb	= lsl122dl01_lcd_match,
};

/**********************************
 * lcd tcon 
 *********************************/
static int lsl122dl01_i2c_read(struct i2c_client *client, u16 reg, u8 *data)
{
	int ret;
	struct i2c_msg msg[2];
	u8 buf1[] = { reg >> 8, reg & 0xFF };

	msg[0].addr  = client->addr;
	msg[0].flags = 0x00;
	msg[0].len   = 2;
	msg[0].buf   = buf1;

	msg[1].addr  = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len   = 1;
	msg[1].buf   = data;

	ret = i2c_transfer(client->adapter, msg, 2);

	if  (ret == 2)
		dev_dbg(&client->dev, "%s ok", __func__);
	else
		dev_err(&client->dev, "%s fail err = %d", __func__, ret);

	return ret;
}

static int lsl122dl01_i2c_write(struct i2c_client *client, u16 reg, u8 data)
{
	int ret = 0;
	struct i2c_msg msg[1];
	u8 buf1[3] = { reg >> 8, reg & 0xFF,  data};

	msg[0].addr  = client->addr;
	msg[0].flags = 0x00;
	msg[0].len   = sizeof(buf1);
	msg[0].buf   = buf1;

	ret = i2c_transfer(client->adapter, msg, 1);

	dev_dbg(&client->dev, "write addr : 0x%x data : 0x%x", reg, data);

	if  (ret == 1)
		dev_dbg(&client->dev, "%s ok", __func__);
	else
		dev_err(&client->dev, "%s fail err = %d", __func__, ret);

	return ret;
}

static int tcon_i2c_slave_enable(struct lsl122dl01 *plcd)
{
	int ret = 0;
	u8 cmd1_buf[3] = {0x03, 0x13, 0xBB};
	u8 cmd2_buf[3] = {0x03, 0x14, 0xBB};
#ifdef CONFIG_S5P_DP_ESD_RECOVERY
	u8 cmd3_buf[3] = {0x81, 0x68, 0x04};
#endif

#ifdef CONFIG_S5P_DP_PSR
	if (plcd->dp->user_disabled) {
		dev_err(plcd->dev, "%s: DP state shutdown\n", __func__);
		return -EINVAL;
	}
#endif

	mutex_lock(&plcd->dp->lock);

	if (!plcd->dp->enabled) {
		dev_err(plcd->dev, "%s: DP state power off\n", __func__);
		goto err_dp;
	}

	ret = s5p_dp_write_bytes_to_dpcd(plcd->dp, 0x491,
			ARRAY_SIZE(cmd1_buf), cmd1_buf);
	if (ret < 0)
		goto err_dp;

	ret = s5p_dp_write_bytes_to_dpcd(plcd->dp, 0x491,
			ARRAY_SIZE(cmd2_buf), cmd2_buf);
	if (ret < 0)
		goto err_dp;

#ifdef CONFIG_S5P_DP_ESD_RECOVERY
	ret = s5p_dp_write_bytes_to_dpcd(plcd->dp, 0x491,
			ARRAY_SIZE(cmd3_buf), cmd3_buf);
	if (ret < 0)
		goto err_dp;
#endif

	mutex_unlock(&plcd->dp->lock);
	return 0;

err_dp:
	dev_err(plcd->dev, "%s: eDP DPCD write fail\n", __func__);

	mutex_unlock(&plcd->dp->lock);
	return -EINVAL;
}

static int tcon_black_frame_bl_on(struct lsl122dl01 *plcd)
{
	struct tcon_reg_info *tune_value;
	int loop;

	tune_value = &TCON_BLACK_IMAGE_BLU_ENABLE;
	for(loop = 0; loop < tune_value->reg_cnt; loop++) {
		lsl122dl01_i2c_write(plcd->client,
			  tune_value->addr[loop], tune_value->data[loop]);
	}

	//Enable double bufferd regiset
	lsl122dl01_i2c_write(plcd->client, 0x0F10, 0x80);

	return 0;
}

#ifdef CONFIG_TCON_SET_MNBL
static int tcon_set_under_mnbl_duty(struct lsl122dl01 *plcd, unsigned int duty)
{
	struct tcon_reg_info *tune_value;

	if (duty == 0)
		return 0;

	if (duty <= MN_BL) {
		lsl122dl01_i2c_write(plcd->client, 0x0DB9, 0x7F);
		lsl122dl01_i2c_write(plcd->client, 0x0DBA, 0xFF);
	} else {
		if (plcd->dblc_power_save) {
			switch (plcd->dblc_mode) {
			case TCON_MODE_VIDEO:
			case TCON_MODE_VIDEO_WARM:
			case TCON_MODE_VIDEO_COLD:
#if defined(CONFIG_LCD_LSL122DL01_BATTERY)
				if(plcd->dblc_battery)
					tune_value = &TCON_VIDEO_B;
				else
					tune_value = &TCON_VIDEO;
#else
				tune_value = &TCON_VIDEO;
#endif
				break;
			default:
#if defined(CONFIG_LCD_LSL122DL01_BATTERY)
				if(plcd->dblc_battery)
					tune_value = &TCON_POWER_SAVE_B;
				else
					tune_value = &TCON_POWER_SAVE;
#else
				tune_value = &TCON_POWER_SAVE;
#endif
				break;
			}
		} else {
#if defined(CONFIG_LCD_LSL122DL01_BATTERY)
			tune_value = tcon_tune_value[plcd->dblc_battery][plcd->dblc_auto_br][plcd->dblc_lux][plcd->dblc_mode];
#else
			tune_value = tcon_tune_value[plcd->dblc_auto_br][plcd->dblc_lux][plcd->dblc_mode];
#endif
		}
		if (!tune_value) {
			dev_err(plcd->dev, "%s: tcon value is null\n", __func__);
			return -EINVAL;
		}

		lsl122dl01_i2c_write(plcd->client,
					tune_value->addr[MNBL_INDEX1], tune_value->data[MNBL_INDEX1]);
		lsl122dl01_i2c_write(plcd->client,
					tune_value->addr[MNBL_INDEX2], tune_value->data[MNBL_INDEX2]);
	}

	//Enable double bufferd regiset
	lsl122dl01_i2c_write(plcd->client, 0x0F10, 0x80);

	return 0;
}
#else
static int tcon_set_under_mnbl_duty(struct lsl122dl01 *plcd, unsigned int duty)
{
	return 0;
}
#endif

static int tcon_tune_read(struct lsl122dl01 *plcd)
{
	struct tcon_reg_info *tune_value;
	int loop;
	int ret;
	u8 data = 0;

#if defined(CONFIG_LCD_LSL122DL01_BATTERY)
		tune_value = tcon_tune_value[plcd->dblc_battery][plcd->dblc_auto_br][plcd->dblc_lux][plcd->dblc_mode];
#else
	tune_value = tcon_tune_value[plcd->dblc_auto_br][plcd->dblc_lux][plcd->dblc_mode];
#endif
	dev_info(plcd->dev, "%s: at=%d, lx=%d, md=%d\n", __func__,
		plcd->dblc_auto_br, plcd->dblc_lux, plcd->dblc_mode);

	for(loop = 0; loop < tune_value->reg_cnt; loop++) {
		ret = lsl122dl01_i2c_read(plcd->client,
			  tune_value->addr[loop],&data);
		dev_info(plcd->dev, "addr = %x, data = %x\n",tune_value->addr[loop], data );
		if (ret < 0)
			return -EIO;
	}
	return 0;


}

static int tcon_tune_write(struct lsl122dl01 *plcd, int force)
{
	struct tcon_reg_info *tune_value;
	int loop;
	int ret;

	if (!plcd->i2c_slave) {
		dev_err(plcd->dev, "%s: tcon i2c is not slave\n", __func__);
		return -EIO;
	}

	if (!force && plcd->dblc_duty == 0) {
		dev_info(plcd->dev, "%s: duty is 0 skip\n", __func__);
		return 0;
	}

	dev_info(plcd->dev, "%s: at=%d, lx=%d, md=%d, ps=%d fc=%d\n", __func__,
			plcd->dblc_auto_br, plcd->dblc_lux, plcd->dblc_mode,
			plcd->dblc_power_save, force);

	if (plcd->dblc_power_save) {
		switch (plcd->dblc_mode) {
		case TCON_MODE_VIDEO:
		case TCON_MODE_VIDEO_WARM:
		case TCON_MODE_VIDEO_COLD:
#if defined(CONFIG_LCD_LSL122DL01_BATTERY)
			if(plcd->dblc_battery)
				tune_value = &TCON_VIDEO_B;
			else
			tune_value = &TCON_VIDEO;
#else
			tune_value = &TCON_VIDEO;
#endif
			break;
		default:
#if defined(CONFIG_LCD_LSL122DL01_BATTERY)
			if(plcd->dblc_battery)
				tune_value = &TCON_POWER_SAVE_B;
			else
			tune_value = &TCON_POWER_SAVE;
#else
			tune_value = &TCON_POWER_SAVE;
#endif
			break;
		}
	} else {
#if defined(CONFIG_LCD_LSL122DL01_BATTERY)
			tune_value = tcon_tune_value[plcd->dblc_battery][plcd->dblc_auto_br][plcd->dblc_lux][plcd->dblc_mode];
#else
			tune_value = tcon_tune_value[plcd->dblc_auto_br][plcd->dblc_lux][plcd->dblc_mode];
#endif
	}

	if (!tune_value) {
		dev_err(plcd->dev, "%s: tcon value is null\n", __func__);
		return -EINVAL;
	}

	for(loop = 0; loop < tune_value->reg_cnt; loop++) {
		ret = lsl122dl01_i2c_write(plcd->client,
			  tune_value->addr[loop], tune_value->data[loop]);
		if (ret < 0)
			return -EIO;
	}
	
	if (plcd->dblc_duty <= MN_BL)
		tcon_set_under_mnbl_duty(plcd, plcd->dblc_duty);

	//Enable double bufferd regiset
	lsl122dl01_i2c_write(plcd->client, 0x0F10, 0x80);

	return 0;
}

static ssize_t tcon_mode_store(struct device *dev,
			    struct device_attribute *dev_attr,
			    const char *buf, size_t count)
{
  	struct lsl122dl01 *plcd = dev_get_drvdata(dev);
	int ret;
	unsigned int value;

	ret = kstrtouint(buf, 10, &value);

	if (ret)
		return ret;

	dev_info(plcd->dev, "%s: value = %d\n\n", __func__, value);

	if (value >= TCON_MODE_MAX) {
		dev_err(plcd->dev, "undef tcon mode value : %d\n\n", value);
		return count;
	}

	mutex_lock(&plcd->ops_lock);
	if (value != plcd->dblc_mode) {
		plcd->dblc_mode = value;
		ret = tcon_tune_write(plcd, 0);

		if (ret)
			dev_err(plcd->dev, "failed to tune tcon\n");
	}
	mutex_unlock(&plcd->ops_lock);

	return count;
}

static ssize_t tcon_mode_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lsl122dl01 *plcd = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", plcd->dblc_mode);
}

static ssize_t tcon_lux_store(struct device *dev,
			    struct device_attribute *dev_attr,
			    const char *buf, size_t count)
{
  	struct lsl122dl01 *plcd = dev_get_drvdata(dev);
	int ret;
	unsigned int value;

	ret = kstrtouint(buf, 10, &value);

	if (ret)
		return ret;

	dev_info(plcd->dev, "%s: value = %d\n\n", __func__, value);

	if (value >= TCON_LEVEL_MAX) {
		dev_err(plcd->dev, "undef tcon illumiate value : %d\n\n", value);
		return count;
	}

	mutex_lock(&plcd->ops_lock);
	if (value != plcd->dblc_lux) {
		plcd->dblc_lux = value;
		ret = tcon_tune_write(plcd, 0);

		if (ret)
			dev_err(plcd->dev, "failed to tune tcon\n");
	#if	defined(CONFIG_FB_DBLC_PWM)
		if(plcd->dblc_lux == 2) {
			value = 1;
			secfb_notifier_call_chain(SECFB_EVENT_MDNIE_OUTDOOR, &value);
		} else {
			value = 0;
			secfb_notifier_call_chain(SECFB_EVENT_MDNIE_OUTDOOR, &value);
		}
	#endif
	}
	mutex_unlock(&plcd->ops_lock);

	return count;
}


static ssize_t tcon_lux_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lsl122dl01 *plcd = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", plcd->dblc_lux);
}

static ssize_t tcon_auto_br_store(struct device *dev,
			    struct device_attribute *dev_attr,
			    const char *buf, size_t count)
{
  	struct lsl122dl01 *plcd = dev_get_drvdata(dev);
	int ret;
	unsigned int value;

	ret = kstrtouint(buf, 10, &value);

	if (ret)
		return ret;

	dev_info(plcd->dev, "%s: value = %d\n\n", __func__, value);

	if (value >= TCON_LEVEL_MAX) {
		dev_err(plcd->dev, "undef tcon auto br value : %d\n", value);
		return count;
	}

	mutex_lock(&plcd->ops_lock);
	if (value != plcd->dblc_auto_br) {
		plcd->dblc_auto_br = value;
		ret = tcon_tune_write(plcd, 0);
#if	defined(CONFIG_FB_DBLC_PWM)
		if(plcd->dblc_lux == 2) {
			if(plcd->dblc_auto_br) {
				value = 1;
				secfb_notifier_call_chain(SECFB_EVENT_MDNIE_OUTDOOR, &value);
			}
			else {
				value = 0;
				secfb_notifier_call_chain(SECFB_EVENT_MDNIE_OUTDOOR, &value);
			}
		}
#endif

		if (ret)
			dev_err(plcd->dev, "failed to tune tcon\n");
	}
	mutex_unlock(&plcd->ops_lock);

	return count;
}


static ssize_t tcon_auto_br_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lsl122dl01 *plcd = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", plcd->dblc_auto_br);
}

static ssize_t tcon_power_save_store(struct device *dev,
			    struct device_attribute *dev_attr,
			    const char *buf, size_t count)
{
  	struct lsl122dl01 *plcd = dev_get_drvdata(dev);
	int ret;
	unsigned int value;

	ret = kstrtouint(buf, 10, &value);

	if (ret)
		return ret;

	dev_info(plcd->dev, "%s: value = %d\n\n", __func__, value);

	if (value > 1) {
		dev_err(plcd->dev, "undef tcon dblc_power_save value : %d\n\n", value);
		return count;
	}

	mutex_lock(&plcd->ops_lock);
	if (value != plcd->dblc_power_save) {
		plcd->dblc_power_save = value;

		ret = tcon_tune_write(plcd, 0);
		if (ret)
			dev_err(plcd->dev, "failed to tune tcon\n");
	}
	mutex_unlock(&plcd->ops_lock);

	return count;
}

static ssize_t tcon_power_save_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lsl122dl01 *plcd = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", plcd->dblc_power_save);
}

static ssize_t tcon_black_test_store(struct device *dev,
			    struct device_attribute *dev_attr,
			    const char *buf, size_t count)
{
  	struct lsl122dl01 *plcd = dev_get_drvdata(dev);
	int ret;
	unsigned int value;

	ret = kstrtouint(buf, 10, &value);

	if (ret)
		return ret;

	dev_info(plcd->dev, "%s: value = %d\n\n", __func__, value);

	if (value > 1) {
		dev_err(plcd->dev, "undef tcon black_test value : %d\n\n", value);
		return count;
	}

	mutex_lock(&plcd->ops_lock);

	if (value)
		ret = tcon_black_frame_bl_on(plcd);
	else
		ret = tcon_tune_write(plcd, 1);

	if (ret)
		dev_err(plcd->dev, "failed to tune tcon\n");

	mutex_unlock(&plcd->ops_lock);

	return count;
}
#if defined(CONFIG_LCD_LSL122DL01_BATTERY)
static ssize_t tcon_battery_save_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lsl122dl01 *plcd = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", plcd->dblc_power_save);
}

static ssize_t tcon_battery_save_store(struct device *dev,
			    struct device_attribute *dev_attr,
			    const char *buf, size_t count)
{
  	struct lsl122dl01 *plcd = dev_get_drvdata(dev);
	int ret;
	unsigned int value;

	ret = kstrtouint(buf, 10, &value);

	if (ret)
		return ret;

	dev_info(plcd->dev, "%s: value = %d\n\n", __func__, value);

	mutex_lock(&plcd->ops_lock);

	plcd->dblc_battery = value;

	ret = tcon_tune_write(plcd, 1);
	if (ret)
		dev_err(plcd->dev, "failed to tune tcon\n");

	mutex_unlock(&plcd->ops_lock);
		return count;
}
#endif

#if defined(DDI_VIDEO_ENHANCE_TUNING)

#define MAX_FILE_NAME 128
#define TUNING_FILE_PATH "/sdcard/"
#define MDNIE_TUNE_SIZE 98
static char tuning_file[MAX_FILE_NAME];
static short mdni_addr[MDNIE_TUNE_SIZE];
static char mdni_tuning_val[MDNIE_TUNE_SIZE];

#define TCON_TUNE_SIZE 20
static short tcon_addr[TCON_TUNE_SIZE];
static char tcon_tuning_val[TCON_TUNE_SIZE];

static char char_to_dec(char data1, char data2)
{
	char dec;

	dec = 0;

	if (data1 >= 'a') {
		data1 -= 'a';
		data1 += 10;
	} else if (data1 >= 'A') {
		data1 -= 'A';
		data1 += 10;
	} else
		data1 -= '0';

	dec = data1 << 4;

	if (data2 >= 'a') {
		data2 -= 'a';
		data2 += 10;
	} else if (data2 >= 'A') {
		data2 -= 'A';
		data2 += 10;
	} else
		data2 -= '0';

	dec |= data2;

	return dec;
}

static void sending_tune_cmd(struct lsl122dl01 *info, char *src, int len)
{
	int data_pos;
	int cmd_step;
	int cmd_pos;
	char data[3] = {0,};

	cmd_step = 0;
	cmd_pos = 0;

	for (data_pos = 0; data_pos < len;) {
		/* skip comments*/
		while(*(src+data_pos++) != 0x0A) ;
		if(*(src + data_pos) == '0') {
			//Addr
			mdni_addr[cmd_pos] = char_to_dec(*(src + data_pos),*(src + data_pos + 1))<<8 | char_to_dec(*(src + data_pos + 2),*(src + data_pos + 3));
			data_pos += 5;
			if((*(src + data_pos) == '0') && (*(src + data_pos + 1) == 'x'))
				mdni_tuning_val[cmd_pos] = char_to_dec(*(src + data_pos+2),*(src + data_pos + 3));
			data_pos  += 4;
			cmd_pos += 1;
		}
	}

	printk(KERN_INFO "\n");
	for (data_pos = 0; data_pos < MDNIE_TUNE_SIZE ; data_pos++) {
		printk(KERN_INFO "0x%04x,0x%02x  \n", mdni_addr[data_pos],mdni_tuning_val[data_pos]);
		//Send Tune Commands
		lsl122dl01_i2c_write(info->client, mdni_addr[data_pos], mdni_tuning_val[data_pos]);
	}
	printk(KERN_INFO "\n");

	for(data_pos = 0;data_pos < cmd_pos ;data_pos++) {
		lsl122dl01_i2c_read(info->client, mdni_addr[data_pos], data);
		pr_info("0x%04x,0x%02x\n",mdni_addr[data_pos], data[0]);
	}

}

static void sending_tcon_tune_cmd(struct lsl122dl01 *info, char *src, int len)
{
	int data_pos;
	int cmd_step;
	int cmd_pos;
	char data;

	u16 i2c_addr;
	u8 i2c_data;

	cmd_step = 0;
	cmd_pos = 0;

	for (data_pos = 0; data_pos < len;) {
		if(*(src + data_pos) == '0') {
			//Addr
			tcon_addr[cmd_pos] = char_to_dec(*(src + data_pos),*(src + data_pos + 1))<<8 | char_to_dec(*(src + data_pos + 2),*(src + data_pos + 3));
			data_pos += 5;
			if((*(src + data_pos) == '0') && (*(src + data_pos + 1) == 'x'))
				tcon_tuning_val[cmd_pos] = char_to_dec(*(src + data_pos+2),*(src + data_pos + 3));
			data_pos  += 4;
			cmd_pos += 1;
		} else
			data_pos++;
	}

	pr_info("cmd_pos : %d", cmd_pos);
	for (data_pos = 0; data_pos < cmd_pos ; data_pos++) {
		pr_info("0x%04x,0x%02x", tcon_addr[data_pos],tcon_tuning_val[data_pos]);
		//Send Tune Commands
		lsl122dl01_i2c_write(info->client, tcon_addr[data_pos], tcon_tuning_val[data_pos]);
	}

	//Enable double bufferd regiset
	i2c_addr = 0x0F10; i2c_data = 0x80;
	lsl122dl01_i2c_write(info->client, i2c_addr, i2c_data);

	for(data_pos = 0;data_pos < cmd_pos ;data_pos++) {
		lsl122dl01_i2c_read(info->client, tcon_addr[data_pos], &data);
		pr_info("0x%04x,0x%02x\n",tcon_addr[data_pos], data);
	}

}

static void load_tuning_file(struct lsl122dl01 *info, char *filename, int mdnie)
{
	struct file *filp;
	char *dp;
	long l;
	loff_t pos;
	int ret;
	mm_segment_t fs;

	pr_info("%s called loading file name : [%s]\n", __func__,
	       filename);

	fs = get_fs();
	set_fs(get_ds());

	filp = filp_open(filename, O_RDONLY, 0);
	if (IS_ERR(filp)) {
		printk(KERN_ERR "%s File open failed\n", __func__);
		return;
	}

	l = filp->f_path.dentry->d_inode->i_size;
	pr_info("%s Loading File Size : %ld(bytes)", __func__, l);

	dp = kmalloc(l + 10, GFP_KERNEL);
	if (dp == NULL) {
		pr_info("Can't not alloc memory for tuning file load\n");
		filp_close(filp, current->files);
		return;
	}
	pos = 0;
	memset(dp, 0, l);

	pr_info("%s before vfs_read()\n", __func__);
	ret = vfs_read(filp, (char __user *)dp, l, &pos);
	pr_info("%s after vfs_read()\n", __func__);

	if (ret != l) {
		pr_info("vfs_read() filed ret : %d\n", ret);
		kfree(dp);
		filp_close(filp, current->files);
		return;
	}

	filp_close(filp, current->files);

	set_fs(fs);

	if (mdnie)
		sending_tune_cmd(info, dp, l);
	else
		sending_tcon_tune_cmd(info, dp, l);

	kfree(dp);
}


static ssize_t store_tcon_test(struct device *dev,
			    struct device_attribute *dev_attr,
			    const char *buf, size_t count)
{
	char *pt;	
  	struct lsl122dl01 *plcd = dev_get_drvdata(dev);

	memset(tuning_file, 0, sizeof(tuning_file));
	snprintf(tuning_file, MAX_FILE_NAME, "%s%s", TUNING_FILE_PATH, buf);

	pt = tuning_file;
	while (*pt) {
		if (*pt == '\r' || *pt == '\n') {
			*pt = 0;
			break;
		}
		pt++;
	}

	pr_info("%s:%s\n", __func__, tuning_file);

	load_tuning_file(plcd, tuning_file, 0);

	return count;
}

#endif



#ifdef CONFIG_S5P_DP_PSR

static ssize_t psr_hfreq_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lsl122dl01 *plcd = dev_get_drvdata(dev);

	sprintf((char *)buf, "%d\n", plcd->current_hfreq_data);

	dev_info(dev, "%s: current hfreq setting data = %d\n", __func__, plcd->current_hfreq_data);

	return strlen(buf);
}

static ssize_t psr_hfreq_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct lsl122dl01 *plcd = dev_get_drvdata(dev);
	int value = 0, rc = 0;

	rc = kstrtoint(buf, 10, &value);

	if (rc < 0)
		return rc;

	if(value < 96000 || value > 100000) {
		dev_info(plcd->dev," Invalid input_hfreq : %d\n", value);
		return -EINVAL;
	}
	else {
		plcd->psr_hfreq = value;
		plcd->psr_hfreq_change = 1;
		dev_info(plcd->dev,"%s : psr_hfreq = %d\n", __func__, value);
	}

	return size;



}

#endif

static struct device_attribute tcon_device_attributes[] = {
	__ATTR(mode, 0664, tcon_mode_show, tcon_mode_store),
	__ATTR(lux, 0664, tcon_lux_show, tcon_lux_store),
	__ATTR(auto_br, 0664, tcon_auto_br_show, tcon_auto_br_store),
	__ATTR(power_save, 0664, tcon_power_save_show, tcon_power_save_store),
	__ATTR(black_test, 0664, NULL, tcon_black_test_store),
#if defined(DDI_VIDEO_ENHANCE_TUNING)
	__ATTR(tcon_test, 0660, NULL, store_tcon_test),
#endif
#ifdef CONFIG_S5P_DP_PSR
	__ATTR(psr_hsync, 0664, psr_hfreq_show, psr_hfreq_store),
#endif
#if defined(CONFIG_LCD_LSL122DL01_BATTERY)
	__ATTR(battery_save, 0664, tcon_battery_save_show, tcon_battery_save_store),
#endif
	__ATTR_NULL,
};

static int tcon_bl_update_status(struct backlight_device *bl)
{
	struct lsl122dl01 *plcd = bl_get_data(bl);
	unsigned int bl_fbstate;
	int ret;

	bl_fbstate = bl->props.state & BL_CORE_FBBLANK;

	if (bl_fbstate) {
#ifdef CONFIG_S5P_DP_PSR
		plcd->current_hfreq_data = plcd->pdata->psr_default_hfreq_data;
		plcd->psr_hfreq = plcd->pdata->psr_default_hfreq;
		plcd->psr_hfreq_change = 0;
#endif
		plcd->i2c_slave = 0;
		return 0;
	} else {
	  	usleep_range(100000, 100000);

		ret = tcon_i2c_slave_enable(plcd);
		if (ret < 0) {
			dev_err(plcd->dev,
				"failed to set tcon_i2c_slave_enable!\n");
			return 0;
		}
		plcd->i2c_slave = 1;
		dev_info(plcd->dev, "%s tcon set slave\n", __func__);
	}

	mutex_lock(&plcd->ops_lock);

	ret = tcon_tune_write(plcd, 1);
	if (ret)
		dev_err(plcd->dev, "failed to tune tcon\n");

	mutex_unlock(&plcd->ops_lock);

	return ret;
}

static const struct backlight_ops tcon_bl_ops = {
	.update_status = tcon_bl_update_status,
};


#ifdef CONFIG_S5P_DP_PSR

static int set_hsync_for_psr(struct lsl122dl01 *plcd, int hsync)
{
	int ret = 0, offset = 0;
	u8 cmd1_buf[3] = {0x04, 0xC3, 0x00};
	u8 cmd2_buf[3] = {0x04, 0xC4, 0x72};

	struct lsl122dl01_platform_data *pdata = plcd->pdata;

	if (plcd->dp->user_disabled) {
		dev_err(plcd->dev, "%s: DP state shutdown\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&plcd->dp->lock);

	if (!plcd->dp->enabled) {
		dev_err(plcd->dev, "%s: DP state power off\n", __func__);
		goto err_dp;
	}
	offset = plcd->current_hfreq_data + (hsync - pdata->psr_default_hfreq)*30/1000;
	if(offset < plcd->hfreqdata_min|| offset > plcd->hfreqdata_max) {
		dev_err(plcd->dev, "%s: over range offset = %d\n", __func__, offset);
		goto err_dp;
	}
	cmd2_buf[2] = offset;
	plcd->current_hfreq_data = offset;
	dev_info(plcd->dev, "%s: offset = %d, cmd2_buf[2] = %d\n", __func__, offset, cmd2_buf[2]);

	ret = s5p_dp_write_bytes_to_dpcd(plcd->dp, 0x491,
			ARRAY_SIZE(cmd1_buf), cmd1_buf);
	if (ret < 0)
		goto err_dp;

	ret = s5p_dp_write_bytes_to_dpcd(plcd->dp, 0x491,
			ARRAY_SIZE(cmd2_buf), cmd2_buf);
	if (ret < 0)
		goto err_dp;

	mutex_unlock(&plcd->dp->lock);
	return 0;

err_dp:
	dev_err(plcd->dev, "%s: eDP DPCD write fail\n", __func__);

	mutex_unlock(&plcd->dp->lock);
	return -EINVAL;


	return 0;
}

static int psr_notify(struct notifier_block *nb,
	unsigned long action, void *data)
{
	struct lsl122dl01 *plcd;
	unsigned int *value;
	int ret = 0;

	if(action != FB_EVENT_PSR_WACOM_CHECK)
		return 0;

	plcd = container_of(nb, struct lsl122dl01, notifier);

	switch(action) {
		case FB_EVENT_PSR_WACOM_CHECK:
			if(plcd->psr_hfreq_change) {
				dev_info(plcd->dev,"%s : need to chenage = %d, set hfreq = %d\n",
				__func__, plcd->psr_hfreq_change, plcd->psr_hfreq);

				ret = set_hsync_for_psr(plcd, plcd->psr_hfreq);
				if(ret)
					dev_err(plcd->dev,"%s: set_hfreq_for_psr failed\n", __func__);
				else
					plcd->psr_hfreq_change = 0;
			}
			break;
	}
	return 0;

}

#endif
static int tcon_notifier_callback(struct notifier_block *self,
				unsigned long event, void *data)
{
	struct lsl122dl01 *plcd;
	unsigned int *value;
	int ret;

	/* If we aren't interested in this event, skip it immediately ... */
	if (event != SECFB_EVENT_CABC_READ &&
	    event != SECFB_EVENT_CABC_WRITE &&
	    event != SECFB_EVENT_BL_UPDATE)
		return 0;

	plcd = container_of(self, struct lsl122dl01, secfb_notif);

	mutex_lock(&plcd->ops_lock);

	switch(event) {
	case SECFB_EVENT_CABC_WRITE:
		value = (unsigned int*) data;
		
		if (*value == plcd->dblc_power_save)
			break;

		plcd->dblc_power_save = *value;
		ret = tcon_tune_write(plcd, 0);
		if (ret)
			dev_err(plcd->dev, "failed to tune tcon\n");
	
		break;

	case SECFB_EVENT_CABC_READ:
		value = (unsigned int*) data;

		*value = plcd->dblc_power_save;

		break;

	case SECFB_EVENT_BL_UPDATE:
		value = (unsigned int*) data;

		dev_dbg(plcd->dev," SECFB_EVENT_BL_UPDATE  duty = %d\n", *value);
		
		if (*value) {
			int curr_bl = (*value <= MN_BL) ? 1 : 0;
			int pre_bl = (plcd->dblc_duty <= MN_BL) ? 1 : 0;

			if (pre_bl != curr_bl)
				tcon_set_under_mnbl_duty(plcd, *value);
		}
		plcd->dblc_duty = *value;
		break;

	default:
		dev_err(plcd->dev, "invalid event\n");
		break;
	}

	mutex_unlock(&plcd->ops_lock);

	return 0;
}

static int lsl122dl01_probe(struct i2c_client *cl,
			    const struct i2c_device_id *id)
{
	struct lsl122dl01 *plcd;
	struct platform_device * pdev_dp;
	struct backlight_device *bl;
	struct backlight_properties props;
	struct lsl122dl01_platform_data *pdata = cl->dev.platform_data;

	int ret;

	dev_dbg(&cl->dev, "%s\n", __func__);

	if (!cl->dev.platform_data) {
		dev_err(&cl->dev, "no platform data supplied\n");
		return -EINVAL;
	}

	pdev_dp = to_platform_device(pdata->dev);
	if (!pdev_dp->dev.driver) {
		dev_err(&cl->dev, "no DP driver supplied\n");
		return -EINVAL;
	}

	if (!i2c_check_functionality(cl->adapter, I2C_FUNC_SMBUS_I2C_BLOCK))
		return -EIO;

	plcd = devm_kzalloc(&cl->dev, sizeof(struct lsl122dl01), GFP_KERNEL);
	if (!plcd)
		return -ENOMEM;

	mutex_init(&plcd->ops_lock);

	plcd->client = cl;
	plcd->dev = &cl->dev;
	plcd->dp = platform_get_drvdata(pdev_dp);
	plcd->pdata = pdata;
#if defined(CONFIG_LCD_LSL122DL01_BATTERY)
	plcd->dblc_battery = 1;
#endif

	plcd->lcd = lcd_device_register(LCD_NAME, &cl->dev,
					plcd, &lsl122dl01_lcd_ops);

	if (IS_ERR(plcd->lcd)) {
		dev_err(&cl->dev, "cannot register lcd device\n");
		ret = PTR_ERR(plcd->lcd);
		goto err_lcddev;
	}

	props.type = BACKLIGHT_PLATFORM;
	plcd->bl = backlight_device_register(BL_NAME, &cl->dev, plcd,
				       &tcon_bl_ops, &props);
	if (IS_ERR(plcd->bl)) {
		dev_err(&cl->dev, "cannot register backlight device\n");
		ret = PTR_ERR(plcd->bl);
		goto err_bldev;
	}

	plcd->tcon_dev = device_create(tcon_class, NULL, 0, plcd, BL_NAME);

	if (IS_ERR(plcd->tcon_dev)) {
		pr_err("%s:%s= Failed to create device(tcon)!\n",
				__FILE__, __func__);
		ret = PTR_ERR(plcd->tcon_dev);
		goto err_cdev;
	}

	ret = sysfs_create_group(&plcd->lcd->dev.kobj, &lcd_attr_group);
	if (ret < 0) {
		dev_err(&cl->dev, "Sysfs registration failed\n");
		goto err_sysfs;
	}

#ifdef CONFIG_FB_S3C
	memset(&plcd->secfb_notif, 0, sizeof(plcd->secfb_notif));
	plcd->secfb_notif.notifier_call = tcon_notifier_callback;
	secfb_register_client(&plcd->secfb_notif);
#endif
#ifdef CONFIG_S5P_DP_PSR
	if(pdata->psr_default_hfreq!= 0) {
		memset(&plcd->notifier, 0, sizeof(plcd->notifier));
		plcd->notifier.notifier_call = psr_notify;
		fb_register_client(&plcd->notifier);
		plcd->psr_hfreq = pdata->psr_default_hfreq;
		plcd->current_hfreq_data = pdata->psr_default_hfreq_data;
		plcd->hfreqdata_max = pdata->psr_hfreq_data_max;
		plcd->hfreqdata_min = pdata->psr_hfreq_data_min;
		plcd->psr_hfreq_change = 0;
	}
#endif
	i2c_set_clientdata(cl, plcd);

	backlight_update_status(plcd->bl);
	
	return 0;

err_sysfs:
	device_unregister(plcd->tcon_dev);
err_cdev:
	backlight_device_unregister(plcd->bl);
err_bldev:
	lcd_device_unregister(plcd->lcd);	
err_lcddev:
	return ret;
}

static int __devexit lsl122dl01_remove(struct i2c_client *cl)
{
	struct lsl122dl01 *plcd = i2c_get_clientdata(cl);

#ifdef CONFIG_FB_S3C
	secfb_unregister_client(&plcd->secfb_notif);
#endif
	sysfs_remove_group(&plcd->lcd->dev.kobj, &lcd_attr_group);
	lcd_device_unregister(plcd->lcd);
	backlight_device_unregister(plcd->bl);
	device_unregister(plcd->tcon_dev);
	return 0;
}

static const struct i2c_device_id lsl122dl01_ids[] = {
	{"lsl122dl01", 0},
	{ }
};

static struct i2c_driver lsl122dl01_driver = {
	.driver = {
		.name = "lsl122dl01",
		.owner		= THIS_MODULE,
	},
	.probe = lsl122dl01_probe,
	.remove = __devexit_p(lsl122dl01_remove),
	.id_table = lsl122dl01_ids,
};

static int __init lsl122dl01_init(void)
{
	int retval;

	tcon_class = class_create(THIS_MODULE, BL_NAME);
	if (IS_ERR(tcon_class)) {
		printk(KERN_WARNING "Unable to create tcon class; errno = %ld\n",
				PTR_ERR(tcon_class));
		return PTR_ERR(tcon_class);
	}

	tcon_class->dev_attrs = tcon_device_attributes;

	retval = i2c_add_driver(&lsl122dl01_driver);
	if (retval) {
	  	class_destroy(tcon_class);
		printk(KERN_INFO "%s: failed registering lsl122dl01\n",
		       __func__);
	}

	return retval;
}

static void __exit lsl122dl01_exit(void)
{
	i2c_del_driver(&lsl122dl01_driver);
	class_destroy(tcon_class);
}

late_initcall_sync(lsl122dl01_init);
module_exit(lsl122dl01_exit);

MODULE_DESCRIPTION("LSL122DL01 LCD T-CON Driver");
MODULE_LICENSE("GPL");
