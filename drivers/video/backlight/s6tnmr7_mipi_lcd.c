/* linux/drivers/video/backlight/s6tnmr7_mipi_lcd.c
 *
 * Samsung SoC MIPI LCD driver.
 *
 * Copyright (c) 2012 Samsung Electronics
 *
 * Haowei Li, <haowei.li@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/backlight.h>
#include <linux/ctype.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/lcd.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/reboot.h>
#include <linux/rtc.h>
#include <linux/wait.h>
#include <linux/workqueue.h>
#include <video/mipi_display.h>
#include <plat/dsim.h>
#include <plat/mipi_dsi.h>
#include <plat/gpio-cfg.h>
#include <asm/system_info.h>
#if defined(CONFIG_FB_S5P_MDNIE_LITE)
#include <linux/mdnie.h>
#endif

#include "s6tnmr7_param.h"

#include "dynamic_aid_s6tnmr7.h"
#include "dynamic_aid_s6tnmr7_RevF.h"
#include "dynamic_aid_s6tnmr7_RevC.h"

#include "../s5p_mipi_dsi_lowlevel.h"
#include <linux/notifier.h>
#include <linux/fb.h>


#define MIN_BRIGHTNESS		0
#define MAX_BRIGHTNESS		255
#define DEFAULT_BRIGHTNESS		135

#define DEFAULT_GAMMA_INDEX		IBRIGHTNESS_162NT


#define POWER_IS_ON(pwr)		(pwr <= FB_BLANK_NORMAL)
#define LEVEL_IS_HBM(level)		(level >= 6)

#define MAX_GAMMA			300
#define LINEAR_MIN_GAMMA			30

#define LDI_ID_REG			0x04
#define LDI_ID_LEN			3
#define LDI_IRQ_REG			0x99
#define LDI_IRQ_LEN			3
#define LDI_MTPR_REG			0xD200
#define LDI_MTPG_REG			0xD280
#define LDI_MTPB_REG			0xD300
#define MTP_VMAX			11
#define LDI_MTP_LEN			(MTP_VMAX * 3)
#define LDI_ELVSS_REG			0xB6
#define LDI_ELVSS_LEN			17

#define LDI_HBM_LEN			11
#define LDI_HBM_MAX			(LDI_HBM_LEN * 3)
#define LDI_HBMR_REG			0xCC10
#define LDI_HBMG_REG			0xCE10
#define LDI_HBMB_REG			0xD010
#define LDI_HBMELVSSON_REG		0xB395
#define LDI_HBMELVSSOFF_REG		0xBB35
#define LDI_MANUFACTUREDATE_REG	0xB38E
#define LDI_COORDINATE_REG		0xB38A

#define LDI_BURST_SIZE		128
#define LDI_PARAM_MSB		0xB1
#define LDI_PARAM_LSB		0xB0
#define LDI_PARAM_LSB_SIZE	2

#define LDI_MDNIE_SIZE		136
#define MDNIE_FIRST_SIZE	82
#define MDNIE_SECOND_SIZE	54

#define LDI_GAMMA_REG		0x83

#ifdef SMART_DIMMING_DEBUG
#define smtd_dbg(format, arg...)	printk(format, ##arg)
#else
#define smtd_dbg(format, arg...)
#endif

static const unsigned int DIM_TABLE[IBRIGHTNESS_MAX] = {
	2,	3,	4,	5,	6,	7,	8,	9,	10,	11,	12,	13,	14,	15,	16,	17,
	19,	20,	21,	22,	24,	25,	27,	29,	30,	32,	34,	37,	39,	41,	44,	47,
	50,	53,	56,	60,	64,	68,	72,	77,	82,	87,	93,	98,	105, 111, 119,
	126, 134, 143, 152,	162,	172,	183,	195,	207,	220,
	234, 249, 265, 282, 300, 400
};

struct lcd_info {
	unsigned int			bl;
	unsigned int			auto_brightness;
	unsigned int			acl_enable;
	unsigned int			siop_enable;
	unsigned int			current_acl;
	unsigned int			current_bl;
	unsigned int			ldi_enable;
	unsigned int			power;
	struct mutex			lock;
	struct mutex			bl_lock;

	struct device			*dev;
	struct lcd_device		*ld;
	struct backlight_device		*bd;
#if defined(CONFIG_FB_S5P_MDNIE_LITE)
	struct mdnie_device		*md;
	int				mdnie_addr;
#endif
	unsigned char			id[LDI_ID_LEN];
	unsigned char			**gamma_table;
	unsigned char			elvss_hbm[2];
	unsigned char			current_elvss;
	unsigned char			current_hbm;

	struct dynamic_aid_param_t	daid;
	unsigned char			aor[IBRIGHTNESS_MAX][ARRAY_SIZE(SEQ_AOR_CONTROL)];
	unsigned int			connected;
	int				temperature;
	unsigned int			coordinate[2];

	int				tcon_irq;
	unsigned int			err_count;
	struct delayed_work		err_worker;
	spinlock_t			slock;

	struct mipi_dsim_device		*dsim;

	int				elvss_delta;
	unsigned char			**elvss_table;
	unsigned int			*gamma_level;

	struct notifier_block	fb_notif;
	unsigned int			fb_unblank;
};

static struct lcd_info *g_lcd;
static int update_brightness(struct lcd_info *lcd, u8 force);

#ifdef CONFIG_FB_HW_TRIGGER
int lcd_get_mipi_state(struct device *dsim_device)
{
	struct lcd_info *lcd = g_lcd;

	if (lcd->connected && !lcd->err_count)
		return 0;
	else
		return -ENODEV;
}
#endif
static int s6tnmr7_write(struct lcd_info *lcd, const u8 *seq, u32 len)
{
	int ret;
	int retry;
	u8 cmd;

	if (!lcd->connected)
		return -EINVAL;

	mutex_lock(&lcd->lock);

	if (len > 2)
		cmd = MIPI_DSI_DCS_LONG_WRITE;
	else if (len == 2) /*use DCS long write until get patch from S.LSI*/
		//cmd = MIPI_DSI_DCS_SHORT_WRITE_PARAM;
		cmd = MIPI_DSI_DCS_LONG_WRITE;
	else if (len == 1)
		cmd = MIPI_DSI_DCS_SHORT_WRITE;
	else {
		ret = -EINVAL;
		goto write_err;
	}

	retry = 5;
write_data:
	if (!retry) {
		dev_err(&lcd->ld->dev, "%s failed: exceed retry count\n", __func__);
		/* print_reg_pm_disp1(); */
		ret = -EINVAL;
		goto write_err;
	}
	ret = s5p_mipi_dsi_wr_data(lcd->dsim, cmd, seq, len);
	if (ret != len) {
		dev_dbg(&lcd->ld->dev, "mipi_write failed retry ..\n");
		retry--;
		msleep(17); /* MIPI timeout for PSR */

		goto write_data;
	}

write_err:
	mutex_unlock(&lcd->lock);
	return ret;
}

static int s6tnmr7_read(struct lcd_info *lcd, u8 addr, u8 *buf, u32 len)
{
	int ret = 0;
	u8 cmd;
	int retry;

	if (!lcd->connected)
		return -EINVAL;

	mutex_lock(&lcd->lock);
	if (len > 2)
		cmd = MIPI_DSI_DCS_READ;
	else if (len == 2)
		cmd = MIPI_DSI_GENERIC_READ_REQUEST_2_PARAM;
	else if (len == 1)
		cmd = MIPI_DSI_GENERIC_READ_REQUEST_1_PARAM;
	else {
		ret = -EINVAL;
		goto read_err;
	}
	retry = 5;
read_data:
	if (!retry) {
		dev_err(&lcd->ld->dev, "%s failed: exceed retry count\n", __func__);
		ret = -EINVAL;
		goto read_err;
	}
	ret = s5p_mipi_dsi_rd_data(lcd->dsim, cmd, addr, len, buf, 0);
	if (ret != len) {
		dev_dbg(&lcd->ld->dev, "mipi_read failed retry ..\n");
		retry--;
		goto read_data;
	}
read_err:
	mutex_unlock(&lcd->lock);
	return ret;
}

static void s6tnmr7_read_id(struct lcd_info *lcd, u8 *buf)
{
	int ret;

	ret = s6tnmr7_read(lcd, LDI_ID_REG, buf, LDI_ID_LEN);
	if (ret < 1) {
		lcd->connected = 0;
		dev_info(&lcd->ld->dev, "panel is not connected well\n");
	}
}

static void s6tnmr7_update_seq(struct lcd_info *lcd)
{
	u8 id;

	id = lcd->id[2];

	if (id < 0x03) { /*Panel rev.C*/
		/* Panel Command */
		pELVSS_TABLE = ELVSS_TABLE_RevD;
		pelvss_delta = &ELVSS_DELTA_RevD;

		/* Dynamic AID parameta */
		paor_cmd = aor_cmd_RevC;
		pbrightness_base_table = brightness_base_table_RevC;
		pvregout_voltage_table = vregout_voltage_table_RevC;
		poffset_gradation = offset_gradation_RevC;
		poffset_color = offset_color_RevC;
	} else if (id < 0x04) { /* Panel rev.D */
		/* Panel Command */
		pELVSS_TABLE = ELVSS_TABLE_RevD;
		pelvss_delta = &ELVSS_DELTA_RevD;

		/* Dynamic AID parameta */
		pbrightness_base_table = brightness_base_table_RevF;
		pvregout_voltage_table = vregout_voltage_table_RevF;
		poffset_gradation = offset_gradation_RevF;
		poffset_color = offset_color_RevF;
		paor_cmd = aor_cmd_RevF;
	} else if (id < 0x06) { /*Panel RevF */
		/* Panel Command */
		pELVSS_TABLE = ELVSS_TABLE_RevF;

		/* Dynamic AID parameta */
		paor_cmd = aor_cmd_RevF;
		pbrightness_base_table = brightness_base_table_RevF;
		pvregout_voltage_table = vregout_voltage_table_RevF;
		poffset_gradation = offset_gradation_RevF;
		poffset_color = offset_color_RevF;
	}
}

static int s6tnmr7_tsp_te_enable(struct lcd_info *lcd, int onoff)
{
	int ret;

	ret = s6tnmr7_write(lcd, SEQ_TSP_TE_B0, ARRAY_SIZE(SEQ_TSP_TE_B0));
	if (ret < ARRAY_SIZE(SEQ_TSP_TE_B0))
		goto te_err;

	ret = s6tnmr7_write(lcd, SEQ_TSP_TE_EN, ARRAY_SIZE(SEQ_TSP_TE_EN));
	if (ret < ARRAY_SIZE(SEQ_TSP_TE_EN))
		goto te_err;

	return ret;

te_err:
	dev_err(&lcd->ld->dev, "%s onoff=%d fail\n", __func__, onoff);
	return ret;
}
#ifdef CONFIG_FB_S5P_MDNIE_LITE
static int s6tnmr7_mdnie_enable(struct lcd_info *lcd, int onoff)
{
	int ret;

	ret = s6tnmr7_write(lcd, SEQ_MDNIE_EN_B0, ARRAY_SIZE(SEQ_MDNIE_EN_B0));
	if (ret < ARRAY_SIZE(SEQ_MDNIE_EN_B0))
		goto enable_err;

	ret = s6tnmr7_write(lcd, SEQ_MDNIE_EN, ARRAY_SIZE(SEQ_MDNIE_EN));
	if (ret < ARRAY_SIZE(SEQ_MDNIE_EN))
		goto enable_err;

	return ret;

enable_err:
	dev_err(&lcd->ld->dev, "%s onoff=%d fail\n", __func__, onoff);
	return ret;
}

int s6tnmr7_mdnie_read(struct device *dev, u8 addr, u8 *buf, u32 len)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	unsigned char addr_buf[LDI_PARAM_LSB_SIZE];
	int ret;

	dev_info(&lcd->ld->dev, "%s\n", __func__);

	/* Offset addr */
	addr_buf[0] = LDI_PARAM_LSB;
	addr_buf[1] = (unsigned char) (lcd->mdnie_addr & 0xff);
	ret = s6tnmr7_write(lcd, addr_buf, LDI_PARAM_LSB_SIZE);
	if (ret < 2)
		return -EINVAL;
	msleep(120);

	/* Base addr & read data */
	ret = s6tnmr7_read(lcd, addr, buf, len);
	if (ret < 1) {
		dev_info(&lcd->ld->dev, "panel is not connected well\n");
	}

	return ret;
}

int s6tnmr7_mdnie_write(struct device *dev, const u8 *seq, u32 len)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	unsigned char addr_buf[LDI_PARAM_LSB_SIZE];
	int ret;
	u8 start_addr;

	/* lock/unlock key */
	if (!len)
		return 0;

	/* check base address */
	start_addr = ((lcd->mdnie_addr >> 8) & 0xff) + LDI_PARAM_MSB;
	if (seq[0] != start_addr) {
		dev_err(&lcd->ld->dev, "Invalid mdnie address (%x, %x)\n",
				start_addr, seq[0]);
		return -EFAULT;
	}
	mutex_lock(&lcd->bl_lock);

	/* Offset addr */
	addr_buf[0] = LDI_PARAM_LSB;
	addr_buf[1] = (unsigned char) (lcd->mdnie_addr & 0xff);
	ret = s6tnmr7_write(lcd, addr_buf, LDI_PARAM_LSB_SIZE);
	if (ret < LDI_PARAM_LSB_SIZE) {
		dev_err(&lcd->ld->dev, "%s failed LDI_PARAM_LSB\n", __func__);
		mutex_unlock(&lcd->bl_lock);
		return -EINVAL;
	}

	/* Base addr & read data */
	ret = s6tnmr7_write(lcd, seq, len);
	if (ret < len) {
		dev_err(&lcd->ld->dev, "%s failed data\n", __func__);
		mutex_unlock(&lcd->bl_lock);
		return -EINVAL;
	}
	mutex_unlock(&lcd->bl_lock);

	msleep(17);  /* wait 1 frame */

	return len;
}

int s6tnmr7_mdnie_set_addr(struct device *dev, int mdnie_addr)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);

	lcd->mdnie_addr = mdnie_addr;

	return 0;
}
#endif
static int s6tnmr7_ldi_init(struct lcd_info *lcd)
{
	int ret = 0;

	lcd->connected = 1;

	usleep_range(5000, 10000);

	s6tnmr7_read_id(lcd, lcd->id);
	dev_info(&lcd->ld->dev," %s : id [%x] [%x] [%x] \n", __func__,
		lcd->id[0], lcd->id[1], lcd->id[2]);

	update_brightness(lcd, 1);

#if defined(CONFIG_FB_S5P_MDNIE_LITE)
	if (!lcd->err_count)
		s6tnmr7_mdnie_enable(lcd, 1);
#endif
	s6tnmr7_tsp_te_enable(lcd, 1);

	return ret;
}

static int s6tnmr7_ldi_enable(struct lcd_info *lcd)
{
	int ret = 0;

	s6tnmr7_write(lcd, SEQ_DISPLAY_ON, ARRAY_SIZE(SEQ_DISPLAY_ON));
	dev_info(&lcd->ld->dev, "DISPLAY_ON\n");

	return ret;
}

static int s6tnmr7_ldi_disable(struct lcd_info *lcd)
{
	int ret = 0;

	dev_info(&lcd->ld->dev, "+ %s\n", __func__);

	s6tnmr7_write(lcd, SEQ_DISPLAY_OFF, ARRAY_SIZE(SEQ_DISPLAY_OFF));
	msleep(160);

	s5p_mipi_dsi_enable_ulps_clk_data(lcd->dsim, 1);
	msleep(10);

	dev_info(&lcd->ld->dev, "- %s\n", __func__);

	return ret;
}

static int s6tnmr7_power_on(struct lcd_info *lcd)
{
	int ret;

	dev_info(&lcd->ld->dev, "+ %s\n", __func__);

	ret = s6tnmr7_ldi_init(lcd);
	if (ret) {
		dev_err(&lcd->ld->dev, "failed to initialize ldi.\n");
		goto err;
	}
#if 0 /* move to fb_notifier_callback to write disp on command after fb_unblank */
	ret = s6tnmr7_ldi_enable(lcd);
	if (ret) {
		dev_err(&lcd->ld->dev, "failed to enable ldi.\n");
		goto err;
	}
#endif
	if (!lcd->err_count)
		enable_irq(lcd->tcon_irq);

	lcd->ldi_enable = 1;

	/* update_brightness(lcd, 1); */

	dev_info(&lcd->ld->dev, "- %s\n", __func__);
err:
	return ret;
}

static int s6tnmr7_power_off(struct lcd_info *lcd)
{
	int ret;

	dev_info(&lcd->ld->dev, "+ %s\n", __func__);

	lcd->ldi_enable = 0;
	if (!lcd->err_count)
		disable_irq(lcd->tcon_irq);

	ret = s6tnmr7_ldi_disable(lcd);

	dev_info(&lcd->ld->dev, "- %s\n", __func__);

	return ret;
}

static int s6tnmr7_power(struct lcd_info *lcd, int power)
{
	int ret = 0;

	if (POWER_IS_ON(power) && !POWER_IS_ON(lcd->power))
		ret = s6tnmr7_power_on(lcd);
	else if (!POWER_IS_ON(power) && POWER_IS_ON(lcd->power))
		ret = s6tnmr7_power_off(lcd);

	if (!ret)
		lcd->power = power;

	return ret;
}

static void err_detection_work(struct work_struct *work)
{
	struct lcd_info *lcd =
		container_of(work, struct lcd_info, err_worker.work);

	struct fb_info *fb_info = registered_fb[0];

	u8 buf[LDI_IRQ_LEN] = {0,};

	dev_info(&lcd->ld->dev, "%s, %d\n", __func__, lcd->err_count);

	if (!lcd->ldi_enable) {
		dev_info(&lcd->ld->dev, "%s ldi off\n", __func__);
		lcd->err_count = 0;
		return;
	}

	s5p_mipi_dsi_disable_by_fimd(lcd->dev);
	msleep(50);
	s5p_mipi_dsi_enable_by_fimd(lcd->dev);
	s6tnmr7_read(lcd, LDI_IRQ_REG, buf, LDI_IRQ_LEN);
	dev_info(&lcd->ld->dev, "state = [%02X][%02X][%02X]\n",
			buf[0], buf[1], buf[2]);
	s6tnmr7_ldi_enable(lcd);

	lcd->err_count = 0;
	enable_irq(lcd->tcon_irq);
}

static irqreturn_t s6tnmr7_irq(int irq, void *dev_id)
{
	struct lcd_info *lcd = dev_id;

	spin_lock(&lcd->slock);

	dev_info(&lcd->ld->dev,"%s, %d\n", __func__, lcd->err_count);

	lcd->err_count++;
	if (lcd->err_count == 1) {
		disable_irq_nosync(lcd->tcon_irq);
		schedule_delayed_work(&lcd->err_worker, HZ/16);
	}

	spin_unlock(&lcd->slock);

	return IRQ_HANDLED;
}

static int s6tnmr7_init_irq(struct lcd_info *lcd)
{
	struct lcd_platform_data *lcd_pd = NULL;
	int ret = 0;

	/* IRQ setting */
	if (lcd->dsim->pd->dsim_lcd_config->mipi_ddi_pd)
		lcd_pd = (struct lcd_platform_data *)lcd->dsim->pd->dsim_lcd_config->mipi_ddi_pd;

	if (lcd_pd && lcd_pd->pdata)
		lcd->tcon_irq = *((int *)lcd_pd->pdata);
	else
		lcd->tcon_irq = -EINVAL;

	dev_info(&lcd->ld->dev, "%s tcon_irq = %d\n", __func__, lcd->tcon_irq);

	if (lcd->tcon_irq >= 0) {
		INIT_DELAYED_WORK(&lcd->err_worker, err_detection_work);
		spin_lock_init(&lcd->slock);

		irq_set_irq_type(lcd->tcon_irq, IRQF_TRIGGER_HIGH);
		ret = devm_request_irq(lcd->dev, lcd->tcon_irq, s6tnmr7_irq,
				  IRQF_TRIGGER_HIGH, "s6tnmr7", lcd);
		if (ret)
			dev_err(&lcd->ld->dev, "irq request failed %d\n",
				lcd->tcon_irq);
	}

	return ret;
}

static ssize_t lcd_type_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);

	sprintf(buf, "SDC_%02X%02X%02X\n", lcd->id[0], lcd->id[1], lcd->id[2]);

	return strlen(buf);
}

static ssize_t window_type_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	char temp[15];

	sprintf(temp, "%x %x %x\n", lcd->id[0], lcd->id[1], lcd->id[2]);

	strcat(buf, temp);
	return strlen(buf);
}

static ssize_t gamma_table_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	int i, j;

	for (i = 0; i < IBRIGHTNESS_MAX; i++) {
		for (j = 0; j < GAMMA_PARAM_SIZE; j++)
			printk("0x%02x, ", lcd->gamma_table[i][j]);
		printk("\n");
	}

	printk("ELVSS Table\n acl_off: ");
	for (i = 0; i < ELVSS_STATUS_MAX; i++)
		printk("0x%02x, ", lcd->elvss_table[i][0]);
	printk("\n acl_on: ");
	for (i = 0; i < ELVSS_STATUS_MAX; i++)
		printk("0x%02x, ", lcd->elvss_table[i][1]);
	printk("\n");

	return strlen(buf);
}

static ssize_t auto_brightness_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	char temp[3];

	sprintf(temp, "%d\n", lcd->auto_brightness);
	strcpy(buf, temp);

	return strlen(buf);
}

static ssize_t auto_brightness_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	int value;
	int rc;

	rc = kstrtoul(buf, (unsigned int)0, (unsigned long *)&value);
	if (rc < 0)
		return rc;
	else {
		if (lcd->auto_brightness != value) {
			dev_info(dev, "%s: %d, %d\n", __func__, lcd->auto_brightness, value);
			mutex_lock(&lcd->bl_lock);
			lcd->auto_brightness = value;
			mutex_unlock(&lcd->bl_lock);
			if (lcd->ldi_enable)
				update_brightness(lcd, 0);
		}
	}
	return size;
}

static ssize_t siop_enable_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	char temp[3];

	sprintf(temp, "%d\n", lcd->siop_enable);
	strcpy(buf, temp);

	return strlen(buf);
}

static ssize_t siop_enable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	int value;
	int rc;

	rc = kstrtoul(buf, (unsigned int)0, (unsigned long *)&value);
	if (rc < 0)
		return rc;
	else {
		if (lcd->siop_enable != value) {
			dev_info(dev, "%s: %d, %d\n", __func__, lcd->siop_enable, value);
			mutex_lock(&lcd->bl_lock);
			lcd->siop_enable = value;
			mutex_unlock(&lcd->bl_lock);
			if (lcd->ldi_enable)
				update_brightness(lcd, 1);
		}
	}
	return size;
}

static ssize_t temperature_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	char temp[] = "-20, -19, 0, 1\n";

	strcat(buf, temp);
	return strlen(buf);
}

static ssize_t temperature_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	int value, rc, temperature = 0 ;

	rc = kstrtoint(buf, 10, &value);

	if (rc < 0)
		return rc;
	else {
		switch (value) {
		case 1:
		case 0:
		case -19:
			temperature = value;
			break;
		case -20:
			temperature = value;
			break;
		}

		mutex_lock(&lcd->bl_lock);
		lcd->temperature = temperature;
		mutex_unlock(&lcd->bl_lock);

		if (lcd->ldi_enable)
			update_brightness(lcd, 1);

		dev_info(dev, "%s: %d, %d\n", __func__, value, lcd->temperature );
	}

	return size;
}

static int s6tnmr7_set_power(struct lcd_device *ld, int power)
{
	struct lcd_info *lcd = lcd_get_data(ld);

	if (power != FB_BLANK_UNBLANK && power != FB_BLANK_POWERDOWN &&
		power != FB_BLANK_NORMAL) {
		dev_err(&lcd->ld->dev, "power value should be 0, 1 or 4.\n");
		return -EINVAL;
	}

	return s6tnmr7_power(lcd, power);
}

static int s6tnmr7_get_power(struct lcd_device *ld)
{
	struct lcd_info *lcd = lcd_get_data(ld);

	return lcd->power;
}

static int s6tnmr7_check_fb(struct lcd_device *ld, struct fb_info *fb)
{
	return 0;
}

static ssize_t manufacture_date_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	int manufacture_addr;
	u8 month, day, manufacture_data[2] = {0,};
	u16 year;
	u8 manufacture_offset[2] = {0xB0, };

	if (lcd->ldi_enable) {
		manufacture_addr = LDI_MANUFACTUREDATE_REG;
		manufacture_offset[1] = (u8) (manufacture_addr & 0xff);
		s6tnmr7_write(lcd, manufacture_offset, 2);
		s6tnmr7_read(lcd, (u8) (manufacture_addr >> 8), manufacture_data, 2);
		pr_info("%x, %x\n", manufacture_data[0], manufacture_data[1]);

		year = ((manufacture_data[0] & 0xF0) >> 4) + 2011;
		month = manufacture_data[0] & 0xF;
		day = manufacture_data[1] & 0x1F;

		sprintf(buf, "%d, %d, %d\n", year, month, day);
		return strlen(buf);
	}
	else
		return 0;

}

static ssize_t color_coordinate_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);

	sprintf(buf, "%d, %d\n", lcd->coordinate[0], lcd->coordinate[1]);

	return strlen(buf);
}

static ssize_t power_reduce_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	char temp[3];

	sprintf(temp, "%d\n", lcd->acl_enable);
	strcpy(buf, temp);

	return strlen(buf);
}

static ssize_t power_reduce_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	int value;
	int rc;

	rc = kstrtoul(buf, (unsigned int)0, (unsigned long *)&value);
	if (rc < 0)
		return rc;
	else {
		if (lcd->acl_enable != value) {
			dev_info(dev, "%s: %d, %d\n", __func__, lcd->acl_enable, value);
			mutex_lock(&lcd->bl_lock);
			lcd->acl_enable = value;
			mutex_unlock(&lcd->bl_lock);
			if (lcd->ldi_enable)
				update_brightness(lcd, 1);
		}
	}
	return size;
}

static int s6tnmr7_get_brightness(struct backlight_device *bd)
{
	return bd->props.brightness;
}

static void s6tnmr7_read_coordinate(struct lcd_info *lcd)
{
	int ret, i;
	u8 coordinate_offset[2] = {0xB0, };
	unsigned char buf[4] = {0,};
	int coordinate_addr;

	coordinate_addr = LDI_COORDINATE_REG;
	coordinate_offset[1] = (u8) (coordinate_addr & 0xff);
	s6tnmr7_write(lcd, coordinate_offset, 2);
	s6tnmr7_read(lcd, (u8) (coordinate_addr >> 8), buf, 4);

	lcd->coordinate[0] = buf[0] << 8 | buf[1];	/* X */
	lcd->coordinate[1] = buf[2] << 8 | buf[3];	/* Y */
	smtd_dbg("coordinate value is %d,%d \n", lcd->coordinate[0], lcd->coordinate[1]);
}

static int s6tnmr7_read_hbm(struct lcd_info *lcd, u8 *buf)
{
	int ret = 0, i;
	int hbm_addr[3];
	unsigned char hbm_offset[2] = {0xB0,};

	hbm_addr[0] = LDI_HBMR_REG;
	hbm_addr[1] = LDI_HBMG_REG;
	hbm_addr[2] = LDI_HBMB_REG;

	for (i = 0; i < CI_MAX; i++) {
		hbm_offset[1] = (u8) (hbm_addr[i] & 0xff);
		s6tnmr7_write(lcd, hbm_offset, 2);

		ret = s6tnmr7_read(lcd, (u8) (hbm_addr[i] >> 8),
			buf + i*LDI_HBM_LEN, LDI_HBM_LEN);

		if (ret < 1) {
			dev_err(&lcd->ld->dev, "%s failed\n", __func__);
			return ret;
		}
	}

/*VT set 0 */
	buf[10] = 0;
	buf[21] = 0;
	buf[32] = 0;

	for (i = 0; i < LDI_HBM_MAX; i++)
		smtd_dbg("%02dth hbm value is %02x\n", i+1, (int)buf[i]);

	return ret;
}

static int s6tnmr7_read_hbmelvss(struct lcd_info *lcd, u8 *buf)
{
	int ret, i;
	int elvss_addr;
	u8 elvss_offset[2] = {0xB0, };

	elvss_addr = LDI_HBMELVSSOFF_REG;
	elvss_offset[1] = (u8) (elvss_addr & 0xff);
	s6tnmr7_write(lcd, elvss_offset, 2);
	ret = s6tnmr7_read(lcd, (u8) (elvss_addr >> 8), buf, 1);

	elvss_addr = LDI_HBMELVSSON_REG;
	elvss_offset[1] = (u8) (elvss_addr & 0xff);
	s6tnmr7_write(lcd, elvss_offset, 2);
	ret += s6tnmr7_read(lcd, (u8) (elvss_addr >> 8), buf+1, 1);

	for (i = 0; i < 2; i++)
		smtd_dbg("%02dth hbmelvss value is %02x\n", i+1, (int)buf[i]);

	return ret;
}

static int s6tnmr7_read_mtp(struct lcd_info *lcd, u8 *buf)
{
	int ret = 0, i;
	int mtp_addr[3];
	u8 mtp_offset[2] = {0xB0,};

	mtp_addr[0] = LDI_MTPR_REG;
	mtp_addr[1] = LDI_MTPG_REG;
	mtp_addr[2] = LDI_MTPB_REG;

	for (i = 0; i < CI_MAX; i++) {
		mtp_offset[1] = (u8) (mtp_addr[i] & 0xff);
		s6tnmr7_write(lcd, mtp_offset, 2);

		ret = s6tnmr7_read(lcd, (u8) (mtp_addr[i] >> 8),
			buf + i*MTP_VMAX, MTP_VMAX);

		if (ret < 1) {
			dev_err(&lcd->ld->dev, "%s failed\n", __func__);
			return ret;
		}
	}

	for (i = 0; i < LDI_MTP_LEN ; i++)
		smtd_dbg("%02dth mtp value is %02x\n", i+1, (int)buf[i]);

	return LDI_MTP_LEN;
}

static int init_backlight_level_from_brightness(struct lcd_info *lcd)
{
	int i, j, gamma;

	lcd->gamma_level = kzalloc((MAX_BRIGHTNESS+1) * sizeof(int), GFP_KERNEL); //0~255 + HBM
	if (!lcd->gamma_level) {
		pr_err("failed to allocate gamma_level table\n");
		return -1;
	}

	/* 0~19 */
	i = 0;
	lcd->gamma_level[i++] = IBRIGHTNESS_2NT; // 0
	lcd->gamma_level[i++] = IBRIGHTNESS_2NT; // 1
	lcd->gamma_level[i++] = IBRIGHTNESS_2NT; // 2
	lcd->gamma_level[i++] = IBRIGHTNESS_3NT; // 3
	lcd->gamma_level[i++] = IBRIGHTNESS_4NT;
	lcd->gamma_level[i++] = IBRIGHTNESS_5NT;
	lcd->gamma_level[i++] = IBRIGHTNESS_6NT;
	lcd->gamma_level[i++] = IBRIGHTNESS_7NT;
	lcd->gamma_level[i++] = IBRIGHTNESS_8NT;
	lcd->gamma_level[i++] = IBRIGHTNESS_9NT;
	lcd->gamma_level[i++] = IBRIGHTNESS_10NT;
	lcd->gamma_level[i++] = IBRIGHTNESS_11NT;
	lcd->gamma_level[i++] = IBRIGHTNESS_12NT;
	lcd->gamma_level[i++] = IBRIGHTNESS_13NT;
	lcd->gamma_level[i++] = IBRIGHTNESS_14NT;
	lcd->gamma_level[i++] = IBRIGHTNESS_15NT;
	lcd->gamma_level[i++] = IBRIGHTNESS_16NT;
	lcd->gamma_level[i++] = IBRIGHTNESS_17NT;
	lcd->gamma_level[i++] = IBRIGHTNESS_17NT; //17~18 : 17NT
	lcd->gamma_level[i++] = IBRIGHTNESS_19NT; // 19

	lcd->gamma_level[i++] = IBRIGHTNESS_20NT;
	lcd->gamma_level[i++] = IBRIGHTNESS_21NT;
	lcd->gamma_level[i++] = IBRIGHTNESS_22NT;
	lcd->gamma_level[i++] = IBRIGHTNESS_22NT;
	lcd->gamma_level[i++] = IBRIGHTNESS_24NT;
	lcd->gamma_level[i++] = IBRIGHTNESS_25NT;
	lcd->gamma_level[i++] = IBRIGHTNESS_27NT;
	lcd->gamma_level[i++] = IBRIGHTNESS_27NT;
	lcd->gamma_level[i++] = IBRIGHTNESS_29NT;
	lcd->gamma_level[i++] = IBRIGHTNESS_30NT;
	lcd->gamma_level[i++] = IBRIGHTNESS_30NT;

	/* 255~20*/
	for(i = MAX_BRIGHTNESS; i >= LINEAR_MIN_GAMMA; i--) {
		gamma = ((i - LINEAR_MIN_GAMMA) * (300 - LINEAR_MIN_GAMMA) / (255 - LINEAR_MIN_GAMMA)) + LINEAR_MIN_GAMMA;
		for (j = IBRIGHTNESS_300NT; j >= 0; j--) {
			if (DIM_TABLE[j] < gamma)
				break;
			lcd->gamma_level[i] =j;
		}
	}
	return 0;
}


static int s6tnmr7_gamma_ctl(struct lcd_info *lcd)
{
	s6tnmr7_write(lcd, lcd->gamma_table[lcd->bl], GAMMA_PARAM_SIZE);

	return 0;
}

static int s6tnmr7_aid_parameter_ctl(struct lcd_info *lcd, u8 force)
{
	if (force)
		goto aid_update;
	else if (lcd->aor[lcd->bl][1] !=  lcd->aor[lcd->current_bl][1])
		goto aid_update;
	else if (lcd->aor[lcd->bl][2] !=  lcd->aor[lcd->current_bl][2])
		goto aid_update;
	else
		goto exit;

aid_update:
	s6tnmr7_write(lcd, lcd->aor[lcd->bl], AID_PARAM_SIZE);

exit:

	return 0;
}

static int s6tnmr7_gamma_update(struct lcd_info *lcd)
{
	s6tnmr7_write(lcd, SEQ_GLOBAL_PARAM_47RD, ARRAY_SIZE(SEQ_GLOBAL_PARAM_47RD));
	s6tnmr7_write(lcd, SEQ_GAMMA_UPDATE, ARRAY_SIZE(SEQ_GAMMA_UPDATE));

	return 0;
}


static int s6tnmr7_set_acl(struct lcd_info *lcd, u8 force)
{
	int ret = 0, level = 0;

	level = ACL_STATUS_15P;

	if (lcd->siop_enable) 
		goto acl_update;

	if ((!lcd->acl_enable) ||  LEVEL_IS_HBM(lcd->auto_brightness))
		level = ACL_STATUS_0P;

acl_update:
	if (force || lcd->current_acl != ACL_CUTOFF_TABLE[level][1]) {
		s6tnmr7_write(lcd, SEQ_GLOBAL_PARAM_ACL, ARRAY_SIZE(SEQ_GLOBAL_PARAM_ACL));
		ret = s6tnmr7_write(lcd, ACL_CUTOFF_TABLE[level], ACL_PARAM_SIZE);

		s6tnmr7_write(lcd, SEQ_GLOBAL_PARAM_OPRAVR_CAL, 2);
		ret += s6tnmr7_write(lcd, SEQ_ACL_OPR_AVR_CAL, 2);

		s6tnmr7_write(lcd, SEQ_GLOBAL_PARAM_ACLUPDATE, 2);
		ret += s6tnmr7_write(lcd, SEQ_ACL_UPDATE, 2);

		lcd->current_acl = ACL_CUTOFF_TABLE[level][1];
		dev_info(&lcd->ld->dev, "acl: %d, auto_brightness: %d\n", lcd->current_acl, lcd->auto_brightness);
	}

	if (!ret)
		ret = -EPERM;

	return ret;
}

static int s6tnmr7_set_elvss(struct lcd_info *lcd, u8 force)
{
	int ret = 0, i, elvss_level;
	u32 nit;
	unsigned char SEQ_ELVSS_HBM[2] = {0xBB, };
	unsigned char SEQ_ELVSS[2] = {0xBB, };

	nit = DIM_TABLE[lcd->bl];
	elvss_level = ELVSS_STATUS_300;
	for (i = 0; i < ELVSS_STATUS_MAX; i++) {
		if (nit <= ELVSS_DIM_TABLE[i]) {
			elvss_level = i;
			break;
		}
	}

	if (lcd->temperature == TSET_MINUS_0_DEGREES)
		SEQ_ELVSS[1] = lcd->elvss_table[elvss_level][lcd->acl_enable];
	else
		SEQ_ELVSS[1] = lcd->elvss_table[elvss_level][lcd->acl_enable] - lcd->elvss_delta;

	if((force) || (lcd->current_elvss != SEQ_ELVSS[1])) {
		ret = s6tnmr7_write(lcd, SEQ_GLOBAL_PARAM_53RD, ARRAY_SIZE(SEQ_GLOBAL_PARAM_53RD));
		if (ret < 0)
			goto elvss_err;

		ret = s6tnmr7_write(lcd, SEQ_ELVSS, ELVSS_PARAM_SIZE);
		if (ret < 0)
			goto elvss_err;
		lcd->current_elvss = SEQ_ELVSS[1];
		dev_dbg(&lcd->ld->dev, "elvss_level = %d, SEQ_ELVSS_HBM = {%x, %x}\n", lcd->current_elvss, SEQ_ELVSS[0], SEQ_ELVSS[1]);
	}
	//HBM setting
	if (elvss_level == ELVSS_STATUS_HBM)
		SEQ_ELVSS_HBM[1] = lcd->elvss_hbm[1];
	else
		SEQ_ELVSS_HBM[1] = lcd->elvss_hbm[0];

	if ((force) || (lcd->current_hbm != SEQ_ELVSS_HBM[1])) {

		ret = s6tnmr7_write(lcd, SEQ_GLOBAL_PARAM_ELVSSHBM, ARRAY_SIZE(SEQ_GLOBAL_PARAM_ELVSSHBM));
		if (ret < 0)
			goto elvss_err;

		ret = s6tnmr7_write(lcd, SEQ_ELVSS_HBM, ARRAY_SIZE(SEQ_ELVSS_HBM));
		if (ret < 0)
			goto elvss_err;

		lcd->current_hbm = SEQ_ELVSS_HBM[1];
		dev_dbg(&lcd->ld->dev, "hbm elvss_level = %d, SEQ_ELVSS_HBM = {%x, %x}\n", elvss_level, SEQ_ELVSS_HBM[0], SEQ_ELVSS_HBM[1]);
	}

	return 0;

elvss_err:
	dev_err(&lcd->ld->dev, "elvss write error\n");
	return ret;
}


void init_dynamic_aid(struct lcd_info *lcd)
{

	lcd->daid.vreg = pvregout_voltage_table[0];
	lcd->daid.vref_h = pvregout_voltage_table[1];
	lcd->daid.br_base = pbrightness_base_table;
	lcd->daid.gc_tbls = gamma_curve_tables;
	lcd->daid.offset_gra = poffset_gradation;
	lcd->daid.offset_color = poffset_color;

	lcd->daid.iv_tbl = index_voltage_table;
	lcd->daid.iv_max = IV_MAX;
	lcd->daid.mtp = kzalloc(IV_MAX * CI_MAX * sizeof(int), GFP_KERNEL);
	lcd->daid.gamma_default = gamma_default;
	lcd->daid.formular = gamma_formula;
	lcd->daid.vt_voltage_value = vt_voltage_value;

	lcd->daid.ibr_tbl = index_brightness_table;
	lcd->daid.ibr_max = IBRIGHTNESS_MAX -1; //except hbm state

	lcd->daid.gc_lut = gamma_curve_lut;

}

static void init_mtp_data(struct lcd_info *lcd, const u8 *mtp_data)
{
	int i, c, j;
	int mtp_val;
	int *mtp;
	int mtp_v0[3];

	mtp = lcd->daid.mtp;


	for (c = 0; c < CI_MAX ; c++) {
		for (i = IV_11, j = 0; i < IV_MAX; i++, j++)
			mtp[i*CI_MAX + c] = mtp_data[MTP_VMAX*c + j];

		mtp[IV_3*CI_MAX + c] = mtp_data[MTP_VMAX*c + j++];
		mtp_v0[c] = mtp_data[MTP_VMAX*c + j++];
		mtp[IV_VT*CI_MAX + c] = mtp_data[MTP_VMAX*c + j++];
	}

	for (c = 0; c < CI_MAX ; c++) {
		for (i = IV_3, j = 0; i <= IV_203; i++, j++)
			if (mtp[i*CI_MAX + c] & 0x80) {
				mtp[i*CI_MAX + c] = mtp[i*CI_MAX + c] & 0x7f;
				mtp[i*CI_MAX + c] *= (-1);
			}
		if (mtp_v0[c] & 0x80)
			mtp[IV_255*CI_MAX + c] *= (-1);
	}

	for (i = 0, j = 0; i <= IV_MAX; i++)
		for (c=0; c<CI_MAX ; c++, j++)
			smtd_dbg("mtp_data[%d] = %d\n",j, mtp_data[j]);

	for (i = 0, j = 0; i < IV_MAX; i++)
		for (c=0; c<CI_MAX ; c++, j++)
			smtd_dbg("mtp[%d] = %d\n",j, mtp[j]);
}

static int init_gamma_table(struct lcd_info *lcd , const u8 *mtp_data)
{
	int i, c, j, v;
	int ret = 0;
	int *pgamma;
	int **gamma;
	unsigned char	value;

	/* allocate memory for local gamma table */
	gamma = kzalloc(IBRIGHTNESS_MAX * sizeof(int *), GFP_KERNEL);
	if (!gamma) {
		pr_err("failed to allocate gamma table\n");
		ret = -ENOMEM;
		goto err_alloc_gamma_table;
	}

	for (i = 0; i < IBRIGHTNESS_MAX; i++) {
		gamma[i] = kzalloc(IV_MAX*CI_MAX * sizeof(int), GFP_KERNEL);
		if (!gamma[i]) {
			pr_err("failed to allocate gamma\n");
			ret = -ENOMEM;
			goto err_alloc_gamma;
		}
	}

	/* allocate memory for gamma table */
	lcd->gamma_table = kzalloc(IBRIGHTNESS_MAX * sizeof(u8 *), GFP_KERNEL);
	if (!lcd->gamma_table) {
		pr_err("failed to allocate gamma table 2\n");
		ret = -ENOMEM;
		goto err_alloc_gamma_table2;
	}

	for (i = 0; i < IBRIGHTNESS_MAX; i++) {
		lcd->gamma_table[i] = kzalloc(GAMMA_PARAM_SIZE * sizeof(u8), GFP_KERNEL);
		if (!lcd->gamma_table[i]) {
			pr_err("failed to allocate gamma 2\n");
			ret = -ENOMEM;
			goto err_alloc_gamma2;
		}
		lcd->gamma_table[i][0] = LDI_GAMMA_REG;
	}

	/* calculate gamma table */
	init_mtp_data(lcd, mtp_data);
	dynamic_aid(lcd->daid, gamma);

	/* relocate gamma order */
	for (i = 0; i < IBRIGHTNESS_MAX - 1; i++) {
		/* Brightness table */
		for (c = 0, j = 1; c < CI_MAX ; c++, pgamma++) {
			for (v = IV_11; v < IV_MAX; v++) {
				pgamma = &gamma[i][v * CI_MAX + c];
				value = (char)((*pgamma) & 0xff);
				lcd->gamma_table[i][j++] = value;
			}
			pgamma = &gamma[i][IV_3 * CI_MAX + c];
			value = (char)((*pgamma) & 0xff);
			lcd->gamma_table[i][j++] = value;

			pgamma = &gamma[i][IV_255 * CI_MAX + c];
			value = (*pgamma & 0x100) ? 0x80 : 0x00;
			lcd->gamma_table[i][j++] = value;

			pgamma = &gamma[i][IV_VT * CI_MAX + c];
			value = (char)((*pgamma) & 0xff);
			lcd->gamma_table[i][j++] = value;
		}

		for (v = 0; v < GAMMA_PARAM_SIZE; v++)
			smtd_dbg("%d ", lcd->gamma_table[i][v]);
		smtd_dbg("\n");
	}

	/* free local gamma table */
	for (i = 0; i < IBRIGHTNESS_MAX; i++)
		kfree(gamma[i]);
	kfree(gamma);

	return 0;

err_alloc_gamma2:
	while (i > 0) {
		kfree(lcd->gamma_table[i-1]);
		i--;
	}
	kfree(lcd->gamma_table);
err_alloc_gamma_table2:
	i = IBRIGHTNESS_MAX;
err_alloc_gamma:
	while (i > 0) {
		kfree(gamma[i-1]);
		i--;
	}
	kfree(gamma);
err_alloc_gamma_table:
	return ret;
}

static int init_aid_dimming_table(struct lcd_info *lcd)
{
	int i, j;
	pSEQ_AOR_CONTROL = SEQ_AOR_CONTROL;

	for (i = 0; i < IBRIGHTNESS_MAX; i++)
		memcpy(lcd->aor[i], pSEQ_AOR_CONTROL, ARRAY_SIZE(SEQ_AOR_CONTROL));

	for (i = 0; i < IBRIGHTNESS_MAX -1; i++) {
		lcd->aor[i][1] = paor_cmd[i][1];
		lcd->aor[i][2] = paor_cmd[i][2];
	}

	for (i = 0; i < IBRIGHTNESS_MAX; i++) {
		for (j = 0; j < ARRAY_SIZE(SEQ_AOR_CONTROL); j++)
			smtd_dbg("%02X ", lcd->aor[i][j]);
		smtd_dbg("\n");
	}

	return 0;
}

static int init_hbm_parameter(struct lcd_info *lcd,
	const u8 *mtp_data, const u8 *hbm_data, const u8 *hbmelvss_data)
{
	int i;

	lcd->gamma_table[IBRIGHTNESS_HBM][0] = LDI_GAMMA_REG;

	/* C8 34~39, 73~87 -> CA 1~21 */
	for (i = 0; i < LDI_HBM_MAX; i++)
		lcd->gamma_table[IBRIGHTNESS_HBM][i + 1] = hbm_data[i];

	lcd->elvss_hbm[0] = hbmelvss_data[0];
	lcd->elvss_hbm[1] = hbmelvss_data[1];
	pr_info("%s: elvss_hbm[0] = %d, elvss_hbm[1] = %d\n", __func__, lcd->elvss_hbm[0], lcd->elvss_hbm[1]);

	return 0;

}
static int init_elvss_table(struct lcd_info *lcd)
{
	int i, j, ret = 0;

	lcd->elvss_table = kzalloc(ELVSS_STATUS_MAX * sizeof(u8 *), GFP_KERNEL);

	if (IS_ERR_OR_NULL(lcd->elvss_table)) {
		pr_err("failed to allocate elvss table\n");
		ret = -ENOMEM;
		goto err_alloc_elvss_table;
	}

	for (i = 0; i < ELVSS_STATUS_MAX; i++) {
		lcd->elvss_table[i] = kzalloc(ELVSS_TABLE_NUM * sizeof(u8), GFP_KERNEL);
		if (IS_ERR_OR_NULL(lcd->elvss_table[i])) {
			pr_err("failed to allocate elvss\n");
			ret = -ENOMEM;
			goto err_alloc_elvss;
		}
		lcd->elvss_table[i][0] = pELVSS_TABLE[i][0];
		lcd->elvss_table[i][1] = pELVSS_TABLE[i][1];
	}

	for (i = 0; i < ELVSS_STATUS_MAX; i++) {
		for (j = 0; j < ELVSS_TABLE_NUM; j++)
			smtd_dbg("0x%02x, ", lcd->elvss_table[i][j]);
		smtd_dbg("\n");
	}
	lcd->elvss_delta = *pelvss_delta;

	return 0;

err_alloc_elvss:
	while (i > 0) {
		kfree(lcd->elvss_table[i-1]);
		i--;
	}
	kfree(lcd->elvss_table);
err_alloc_elvss_table:
	return ret;
}

static int update_brightness(struct lcd_info *lcd, u8 force)
{
	u32 brightness;


	if(lcd->id[0] == 0 && lcd->id[1] == 0 && lcd->id[2] == 0)
		return 0;

	mutex_lock(&lcd->bl_lock);

	brightness = lcd->bd->props.brightness;

	lcd->bl = lcd->gamma_level[brightness];

	if ((lcd->id[2] >= 0x03)&& LEVEL_IS_HBM(lcd->auto_brightness) && (brightness == lcd->bd->props.max_brightness))
		lcd->bl = IBRIGHTNESS_HBM;

	if ((force) || ((lcd->ldi_enable) && (lcd->current_bl != lcd->bl))) {
		s6tnmr7_gamma_ctl(lcd);
		s6tnmr7_aid_parameter_ctl(lcd, force);
		s6tnmr7_set_elvss(lcd, force);
		s6tnmr7_set_acl(lcd, force);
		s6tnmr7_gamma_update(lcd);

		lcd->current_bl = lcd->bl;
		dev_info(&lcd->ld->dev, "brightness=%d, bl=%d, candela=%d\n", \
			brightness, lcd->bl, DIM_TABLE[lcd->bl]);
	}

	mutex_unlock(&lcd->bl_lock);

	return 0;
}


static int s6tnmr7_set_brightness(struct backlight_device *bd)
{
	int ret = 0;
	int brightness = bd->props.brightness;
	struct lcd_info *lcd = bl_get_data(bd);

	/* dev_info(&lcd->ld->dev, "%s: brightness=%d\n", __func__, brightness); */

	if (brightness < MIN_BRIGHTNESS ||
		brightness > bd->props.max_brightness) {
		dev_err(&bd->dev, "lcd brightness should be %d to %d. now %d\n",
			MIN_BRIGHTNESS, lcd->bd->props.max_brightness, brightness);
		return -EINVAL;
	}

	if (lcd->ldi_enable && lcd->fb_unblank) {
		ret = update_brightness(lcd, 0);
		if (ret < 0) {
			dev_err(&lcd->ld->dev, "err in %s\n", __func__);
			return -EINVAL;
		}
	}

	return ret;
}


static struct lcd_ops s6tnmr7_lcd_ops = {
	.set_power = s6tnmr7_set_power,
	.get_power = s6tnmr7_get_power,
	.check_fb  = s6tnmr7_check_fb,
};

static const struct backlight_ops s6tnmr7_backlight_ops = {
	.get_brightness = s6tnmr7_get_brightness,
	.update_status = s6tnmr7_set_brightness,
};

#if defined(CONFIG_FB_S5P_MDNIE_LITE)
static struct mdnie_ops s6tnmr7_mdnie_ops = {
	.write = s6tnmr7_mdnie_write,
	.read = s6tnmr7_mdnie_read,
	.set_addr = s6tnmr7_mdnie_set_addr,
};
#endif
static int s6tnmr7_fb_notifier_callback(struct notifier_block *self,
		unsigned long event, void *data)
{
	struct lcd_info *lcd;
	struct fb_event *blank = (struct fb_event*) data;
	unsigned int *value = (unsigned int*)blank->data;

	lcd = container_of(self, struct lcd_info, fb_notif);

	if (event == FB_EVENT_BLANK) {
		switch (*value) {
		case FB_BLANK_POWERDOWN:
		case FB_BLANK_NORMAL:
			lcd->fb_unblank = 0;
			break;
		case FB_BLANK_UNBLANK:
			s6tnmr7_ldi_enable(lcd);
			lcd->fb_unblank = 1;
			update_brightness(lcd, 0);
			break;
		default:
			break;
		}
	}

	return 0;
}

static int s6tnmr7_register_fb(struct lcd_info *lcd)
{
	memset(&lcd->fb_notif, 0, sizeof(lcd->fb_notif));
	lcd->fb_notif.notifier_call = s6tnmr7_fb_notifier_callback;

	return fb_register_client(&lcd->fb_notif);
}

static DEVICE_ATTR(power_reduce, 0664, power_reduce_show, power_reduce_store);
static DEVICE_ATTR(auto_brightness, 0644, auto_brightness_show, auto_brightness_store);
static DEVICE_ATTR(siop_enable, 0664, siop_enable_show, siop_enable_store);
static DEVICE_ATTR(temperature, 0664, temperature_show, temperature_store);
static DEVICE_ATTR(gamma_table, 0444, gamma_table_show, NULL);
static DEVICE_ATTR(manufacture_date, 0444, manufacture_date_show, NULL);
static DEVICE_ATTR(color_coordinate, 0444, color_coordinate_show, NULL);
static DEVICE_ATTR(lcd_type, 0444, lcd_type_show, NULL);
static DEVICE_ATTR(window_type, 0444, window_type_show, NULL);

static struct attribute *s6tnmr7_attributes[] = {
	&dev_attr_power_reduce.attr,
	&dev_attr_siop_enable.attr,
	&dev_attr_temperature.attr,
	&dev_attr_gamma_table.attr,
	&dev_attr_manufacture_date.attr,
	&dev_attr_color_coordinate.attr,
	&dev_attr_lcd_type.attr,
	&dev_attr_window_type.attr,
	NULL,
};

static const struct attribute_group s6tnmr7_attr_group = {
	.attrs = s6tnmr7_attributes,
};

static int s6tnmr7_probe(struct mipi_dsim_device *dsim)
{
	int ret;
	struct lcd_info *lcd;
	u8 mtp_data[LDI_MTP_LEN] = {0,};
	u8 hbmelvss_data[2] = {0, 0};
	u8 hbm_data[LDI_HBM_MAX] = {0,};


	lcd = kzalloc(sizeof(struct lcd_info), GFP_KERNEL);
	if (!lcd) {
		pr_err("failed to allocate for lcd\n");
		ret = -ENOMEM;
		goto err_alloc;
	}

	g_lcd = lcd;

	lcd->ld = lcd_device_register("panel", dsim->dev, lcd, &s6tnmr7_lcd_ops);
	if (IS_ERR(lcd->ld)) {
		pr_err("failed to register lcd device\n");
		ret = PTR_ERR(lcd->ld);
		goto out_free_lcd;
	}

	lcd->bd = backlight_device_register("panel", dsim->dev, lcd, &s6tnmr7_backlight_ops, NULL);
	if (IS_ERR(lcd->bd)) {
		pr_err("failed to register backlight device\n");
		ret = PTR_ERR(lcd->bd);
		goto out_free_backlight;
	}

	lcd->dev = dsim->dev;
	lcd->dsim = dsim;
	lcd->bd->props.max_brightness = MAX_BRIGHTNESS;
	lcd->bd->props.brightness = DEFAULT_BRIGHTNESS;
	lcd->bl = DEFAULT_GAMMA_INDEX;
	lcd->current_bl = lcd->bl;
	lcd->power = FB_BLANK_UNBLANK;
	lcd->auto_brightness = 0;
	lcd->connected = 1;
	lcd->siop_enable = 0;
	lcd->acl_enable = 0;
	lcd->current_acl = 0;
	lcd->temperature = 1;
	lcd->current_elvss = 0;
	lcd->current_hbm = 0;
	lcd->fb_unblank = 1;

	/* dev_set_drvdata(dsim->dev, lcd); */
	ret = device_create_file(&lcd->bd->dev, &dev_attr_auto_brightness);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "failed to add sysfs entries, %d\n", __LINE__);

	ret = sysfs_create_group(&lcd->ld->dev.kobj, &s6tnmr7_attr_group);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "failed to add sysfs entries\n");

	ret = s6tnmr7_register_fb(lcd);
	if (ret)
		dev_err(&lcd->ld->dev, "failed to register fb notifier chain\n");


	mutex_init(&lcd->lock);
	mutex_init(&lcd->bl_lock);

	s6tnmr7_read_id(lcd, lcd->id);
	s6tnmr7_read_mtp(lcd, mtp_data);
	s6tnmr7_read_coordinate(lcd);

	dev_info(&lcd->ld->dev, "ID: %x, %x, %x\n", lcd->id[0], lcd->id[1], lcd->id[2]);

	s6tnmr7_update_seq(lcd);
	ret = init_backlight_level_from_brightness(lcd);
	if(ret < 0)
		dev_info(&lcd->ld->dev, "gamma level generation is failed\n");

	init_dynamic_aid(lcd);

	ret = init_gamma_table(lcd, mtp_data);
	ret += init_aid_dimming_table(lcd);
	ret += init_elvss_table(lcd);
	if(lcd->id[2] >= 0x03) {
		s6tnmr7_read_hbmelvss(lcd, hbmelvss_data);
		s6tnmr7_read_hbm(lcd, hbm_data);
		ret += init_hbm_parameter(lcd, mtp_data, hbm_data, hbmelvss_data);
	}

	if (ret)
		dev_info(&lcd->ld->dev, "gamma table generation is failed\n");

	if (lcd->power == FB_BLANK_POWERDOWN)
		s6tnmr7_power(lcd, FB_BLANK_UNBLANK);
	else
		update_brightness(lcd, 1);

#if defined(CONFIG_FB_S5P_MDNIE_LITE)
	lcd->md = mdnie_device_register("mdnie", &lcd->ld->dev, &s6tnmr7_mdnie_ops);
#endif
	s6tnmr7_init_irq(lcd);

	lcd->ldi_enable = 1;
	dev_info(&lcd->ld->dev, "%s lcd panel driver has been probed.\n", __FILE__);

	return 0;

out_free_backlight:
	lcd_device_unregister(lcd->ld);
	kfree(lcd);
	return ret;

out_free_lcd:
	kfree(lcd);
	return ret;

err_alloc:
	return ret;
}

static int s6tnmr7_displayon(struct mipi_dsim_device *dsim)
{
	struct lcd_info *lcd = g_lcd;

	s6tnmr7_power(lcd, FB_BLANK_UNBLANK);

	return 0;
}

static int s6tnmr7_suspend(struct mipi_dsim_device *dsim)
{
	struct lcd_info *lcd = g_lcd;

	s6tnmr7_power(lcd, FB_BLANK_POWERDOWN);

	return 0;
}

static int s6tnmr7_resume(struct mipi_dsim_device *dsim)
{
	return 0;
}

struct mipi_dsim_lcd_driver s6tnmr7_mipi_lcd_driver = {
	.probe		= s6tnmr7_probe,
	.displayon	= s6tnmr7_displayon,
	.suspend	= s6tnmr7_suspend,
	.resume		= s6tnmr7_resume,
};
