/* drivers/video/decon_display/s6e3ha1_mipi_lcd.c
 *
 * Samsung SoC MIPI LCD driver.
 *
 * Copyright (c) 2013 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/ctype.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/backlight.h>
#include <linux/lcd.h>
#include <linux/rtc.h>
#include <linux/reboot.h>

#include <video/mipi_display.h>
#include <plat/dsim.h>
#include <plat/mipi_dsi.h>
#include <plat/gpio-cfg.h>
#include <asm/system_info.h>

#include "s6e3ha1_param.h"

#include "dynamic_aid_s6e3ha1.h"
#include "dynamic_aid_s6e3ha1_RevB.h"
#include "dynamic_aid_s6e3ha1_RevC.h"

#include "../s5p_mipi_dsi_lowlevel.h"
#include <linux/notifier.h>
#include <linux/fb.h>

#if defined(CONFIG_FB_S5P_MDNIE_LITE)
#include <linux/mdnie.h>
#endif

#define POWER_IS_ON(pwr)		(pwr <= FB_BLANK_NORMAL)
#define LEVEL_IS_HBM(level)		(level >= 6)

#define NORMAL_TEMPERATURE		25	/* 25 C */

#define MIN_BRIGHTNESS		0
#define MAX_BRIGHTNESS		255
#define DEFAULT_BRIGHTNESS		135

#define MIN_GAMMA			2
#define MAX_GAMMA			300
#define LINEAR_MIN_GAMMA			30


#define DEFAULT_GAMMA_INDEX		IBRIGHTNESS_162NT

#define LDI_ID_REG			0x04
#define LDI_ID_LEN			3
#define LDI_ID2_REG			0xD6
#define LDI_ID2_LEN			5
#define LDI_MTP_REG			0xC8
#define LDI_MTP_LEN			87	/* MTP + HBM */
#define LDI_ELVSS_REG			0xB6
#define LDI_ELVSS_LEN			ELVSS_PARAM_SIZE-1
#define LDI_TSET_REG			0xB8
#define LDI_TSET_LEN			5
#define TSET_PARAM_SIZE		(LDI_TSET_LEN + 1)
#define LDI_HBMELVSS_LEN	21

#define LDI_COORDINATE_REG		0xA1
#define LDI_COORDINATE_LEN		4
#define ERRFG_PEND_REG		0x14000A14	/*EXT_INT22_PEND*/
#define ERRFG_PEND_MASK		(1 << 5)	/*EXT_INT22_PEND[5]*/

#ifdef SMART_DIMMING_DEBUG
#define smtd_dbg(format, arg...)	printk(format, ##arg)
#else
#define smtd_dbg(format, arg...)
#endif

static const unsigned int DIM_TABLE[IBRIGHTNESS_MAX] = {
	2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	12, 13, 14, 15, 16, 17, 19, 20, 21, 22,
	24, 25, 27, 29, 30, 32, 34, 37, 39, 41,
	44, 47, 50, 53, 56, 60, 64, 68, 72, 77,
	82, 87, 93, 98, 105, 111, 119, 126, 134,
	143, 152, 162, 172, 183, 195, 207, 220,
	234, 249, 265, 282, 300, 400
};

union elvss_info {
	u16 value;
	struct {
		u8 mps;
		u8 offset;
	};
};


struct lcd_info {
	unsigned int			bl;
	unsigned int			auto_brightness;
	unsigned int			acl_enable;
	unsigned int			siop_enable;
	unsigned int			current_acl;
	unsigned int			current_bl;
	union elvss_info		current_elvss;
	unsigned int			current_tset;
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
	unsigned char			ddi_id[LDI_ID2_LEN];
	unsigned char			**gamma_table;
	unsigned char			**elvss_table[ACL_STATUS_MAX];
	struct dynamic_aid_param_t	daid;
	unsigned char			aor[IBRIGHTNESS_MAX][ARRAY_SIZE(SEQ_AOR_CONTROL)];
	unsigned int			connected;

	unsigned char			**tset_table;
	int				temperature;
	unsigned int			coordinate[2];
	unsigned int			partial_range[2];
	unsigned char			date[2];

	unsigned char			elvss_hbm[2];
	unsigned char			current_hbm;

	struct mipi_dsim_device		*dsim;
	unsigned int			*gamma_level;
	int				errfg_irq;
	unsigned int			err_count;
	struct delayed_work		err_worker;
	spinlock_t			slock;
	void __iomem			*errfg_pend;

	struct notifier_block	fb_notif;
	unsigned int			fb_unblank;
};

static void s6e3ha1_enable_errfg(struct lcd_info *lcd);


#ifdef CONFIG_FB_HW_TRIGGER
static struct lcd_info *g_lcd;

int lcd_get_mipi_state(struct device *dsim_device)
{
	struct lcd_info *lcd = g_lcd;

	if (lcd->connected && !lcd->err_count)
		return 0;
	else
		return -ENODEV;
}
#endif
int s6e3ha1_write(struct lcd_info *lcd, const u8 *seq, u32 len)
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
	//	cmd = MIPI_DSI_DCS_SHORT_WRITE_PARAM;
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


int s6e3ha1_read(struct lcd_info *lcd, u8 addr, u8 *buf, u32 len)
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
		goto read_err;
	}
	ret = s5p_mipi_dsi_rd_data(lcd->dsim, cmd, addr, len, buf, 1);
	if (ret != len) {
		dev_err(&lcd->ld->dev, "mipi_read failed retry ..\n");
		retry--;
		goto read_data;
	}

read_err:
	mutex_unlock(&lcd->lock);
	return ret;
}

#ifdef CONFIG_FB_S5P_MDNIE_LITE
int s6e3ha1_mdnie_read(struct device *dev, u8 addr, u8 *buf, u32 len)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	int ret;

	dev_info(&lcd->ld->dev, "%s\n", __func__);

	/* Base addr & read data */
	ret = s6e3ha1_read(lcd, addr, buf, len);
	if (ret < 1) {
		dev_info(&lcd->ld->dev, "panel is not connected well\n");
	}

	return ret;
}

int s6e3ha1_mdnie_write(struct device *dev, const u8 *seq, u32 len)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	int ret;
	u8 start_addr;

	/* lock/unlock key */
	if (!len)
		return 0;

	/* Base addr & read data */
	ret = s6e3ha1_write(lcd, seq, len);
	if (ret < len)
		return -EINVAL;

	return len;
}
#endif

static void s6e3ha1_read_coordinate(struct lcd_info *lcd)
{
	int ret;
	unsigned char buf[LDI_COORDINATE_LEN] = {0,};

	ret = s6e3ha1_read(lcd, LDI_COORDINATE_REG, buf, LDI_COORDINATE_LEN);

	if (ret < 1)
		dev_err(&lcd->ld->dev, "%s failed\n", __func__);

	lcd->coordinate[0] = buf[0] << 8 | buf[1];	/* X */
	lcd->coordinate[1] = buf[2] << 8 | buf[3];	/* Y */
}

static void s6e3ha1_read_id(struct lcd_info *lcd, u8 *buf)
{
	int ret;

	ret = s6e3ha1_read(lcd, LDI_ID_REG, buf, LDI_ID_LEN);
	if (ret < 1) {
		lcd->connected = 0;
		dev_info(&lcd->ld->dev, "panel is not connected well\n");
	}
}

static void s6e3ha1_read_ddi_id(struct lcd_info *lcd, u8 *buf)
{
	int ret;

	ret = s6e3ha1_read(lcd, LDI_ID2_REG, buf, LDI_ID2_LEN);

	if (ret < 1)
		dev_info(&lcd->ld->dev, "%s failed\n", __func__);
}

static void s6e3ha1_update_seq(struct lcd_info *lcd)
{
	u8 id;

	if(lcd->id[0] == 0)
		return;

	id = lcd->id[2];

	if(id < 0x02) {
		dev_info(&lcd->ld->dev, "%s id = %d\n", __func__, id);
		paor_cmd = aor_cmd_RevB;
		pbrightness_base_table = brightness_base_table_RevB;
		poffset_gradation = offset_gradation_RevB;
		poffset_color = offset_color_RevB;
		pELVSS_TABLE = ELVSS_TABLE_RevB;
	} else if(id < 0x03) {
		dev_info(&lcd->ld->dev, "%s id = %d\n", __func__, id);
		paor_cmd = aor_cmd_RevC;
		pbrightness_base_table = brightness_base_table_RevC;
		poffset_gradation = offset_gradation_RevC;
		poffset_color = offset_color_RevC;
	}

}
static int s6e3ha1_read_mtp(struct lcd_info *lcd, u8 *buf)
{
	int ret, i;

	ret = s6e3ha1_read(lcd, LDI_MTP_REG, buf, LDI_MTP_LEN);

	if (ret < 1)
		dev_err(&lcd->ld->dev, "%s failed\n", __func__);

	/* HBM on ELVSS setting */
	lcd->elvss_hbm[1] = buf[39];

	/* manufacture date */
	lcd->date[0] = buf[40];
	lcd->date[1] = buf[41];

	smtd_dbg("%s: %02xh\n", __func__, LDI_MTP_REG);
	for (i = 0; i < LDI_MTP_LEN; i++)
		smtd_dbg("%02dth value is %02x\n", i+1, (int)buf[i]);

	return ret;
}

static int s6e3ha1_read_elvss_hbm(struct lcd_info *lcd)
{
	int ret, i;
	u8 buf[LDI_HBMELVSS_LEN];

	/* ELVSS setting if it is not HBM mode */
	ret = s6e3ha1_read(lcd, LDI_ELVSS_REG, buf, LDI_HBMELVSS_LEN);
	lcd->elvss_hbm[0] = buf[LDI_HBMELVSS_LEN-1];

	return ret;
}
#if 0
static int s6e3ha1_read_tset(struct lcd_info *lcd)
{
	int ret, i;

	lcd->tset_table[0] = LDI_TSET_REG;

	ret = s6e3ha1_read(lcd, LDI_TSET_REG, &lcd->tset_table[1], LDI_TSET_LEN);

	smtd_dbg("%s: %02xh\n", __func__, LDI_TSET_REG);
	for (i = 0; i < LDI_TSET_LEN; i++)
		smtd_dbg("%02dth value is %02x\n", i, lcd->tset_table[i]);

	return ret;
}

#endif

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


static int s6e3ha1_gamma_ctl(struct lcd_info *lcd)
{
	int ret = 0;

	ret = s6e3ha1_write(lcd, lcd->gamma_table[lcd->bl], GAMMA_PARAM_SIZE);
	if (!ret)
		ret = -EPERM;

	return ret;
}

static int s6e3ha1_aid_parameter_ctl(struct lcd_info *lcd, u8 force)
{
	int ret = 0;

	if (force)
		goto aid_update;
	else if (lcd->aor[lcd->bl][1] !=  lcd->aor[lcd->current_bl][1])
		goto aid_update;
	else if (lcd->aor[lcd->bl][2] !=  lcd->aor[lcd->current_bl][2])
		goto aid_update;
	else
		goto exit;

aid_update:
	ret = s6e3ha1_write(lcd, lcd->aor[lcd->bl], AID_PARAM_SIZE);
	if (!ret)
		ret = -EPERM;

exit:
	return ret;
}

static int s6e3ha1_set_acl(struct lcd_info *lcd, u8 force)
{
	int ret = 0, level = ACL_STATUS_15P;

	if (lcd->siop_enable)
		goto acl_update;

	if ((!lcd->acl_enable) || LEVEL_IS_HBM(lcd->auto_brightness))
		level = ACL_STATUS_0P;

acl_update:
	if (force || lcd->current_acl != ACL_CUTOFF_TABLE[level][1]) {
		ret = s6e3ha1_write(lcd, ACL_CUTOFF_TABLE[level], ACL_PARAM_SIZE);
		ret += s6e3ha1_write(lcd, ACL_OPR_TABLE[level], OPR_PARAM_SIZE);
		lcd->current_acl = ACL_CUTOFF_TABLE[level][1];
		dev_info(&lcd->ld->dev, "acl: %d, auto_brightness: %d\n", lcd->current_acl, lcd->auto_brightness);
	}

	if (!ret)
		ret = -EPERM;

	return ret;
}

static int s6e3ha1_set_elvss(struct lcd_info *lcd, u8 force)
{
	int ret = 0, i, elvss_level;
	u32 nit, temperature;
	unsigned char SEQ_ELVSS_HBM[2] = {0xB6, };
	union elvss_info elvss;

	nit = DIM_TABLE[lcd->bl];
	elvss_level = ELVSS_STATUS_300;
	for (i = 0; i < ELVSS_STATUS_MAX; i++) {
		if (nit <= ELVSS_DIM_TABLE[i]) {
			elvss_level = i;
			break;
		}
	}

	elvss.mps = lcd->elvss_table[lcd->acl_enable][elvss_level][1];
	elvss.offset = lcd->elvss_table[lcd->acl_enable][elvss_level][2];

	if((force) || (lcd->current_elvss.value != elvss.value)) {
		ret = s6e3ha1_write(lcd, lcd->elvss_table[lcd->acl_enable][elvss_level], ELVSS_PARAM_SIZE);
		if (ret < 0)
			goto exit;
		lcd->current_elvss.value = elvss.value;
		dev_info(&lcd->ld->dev, "elvss set = {%x, %x}\n", lcd->current_elvss.mps, lcd->current_elvss.offset);
	}

	if (elvss_level == ELVSS_STATUS_HBM)
		SEQ_ELVSS_HBM[1] = lcd->elvss_hbm[1];
	else
		SEQ_ELVSS_HBM[1] = lcd->elvss_hbm[0];

	if ((force) || (lcd->current_hbm != SEQ_ELVSS_HBM[1])) {

		ret = s6e3ha1_write(lcd, SEQ_GLOBAL_PARAM_ELVSSHBM, ARRAY_SIZE(SEQ_GLOBAL_PARAM_ELVSSHBM));
		if (ret < 0)
			goto exit;

		ret = s6e3ha1_write(lcd, SEQ_ELVSS_HBM, ARRAY_SIZE(SEQ_ELVSS_HBM));
		if (ret < 0)
			goto exit;

		lcd->current_hbm = SEQ_ELVSS_HBM[1];
		dev_info(&lcd->ld->dev, "hbm elvss_level = %d, SEQ_ELVSS_HBM = {%x, %x}\n", elvss_level, SEQ_ELVSS_HBM[0], SEQ_ELVSS_HBM[1]);
	}

exit:
	return -1;

}

static int s6e3ha1_set_tset(struct lcd_info *lcd, u8 force)
{
	int ret = 0, tset_level = 0;

	switch (lcd->temperature) {
	case 1:
		tset_level = TSET_25_DEGREES;
		break;
	case 0:
	case -19:
		tset_level = TSET_MINUS_0_DEGREES;
		break;
	case -20:
		tset_level = TSET_MINUS_20_DEGREES;
		break;
	}

	if (force || lcd->current_tset != tset_level) {
		s6e3ha1_write(lcd, SEQ_GLOBAL_PARAM_TSET, ARRAY_SIZE(SEQ_GLOBAL_PARAM_TSET));
		ret = s6e3ha1_write(lcd, lcd->tset_table[tset_level], 2);
		lcd->current_tset = tset_level;
		dev_info(&lcd->ld->dev, "tset: %d\n", lcd->current_tset);
	}

	if (!ret) {
		ret = -EPERM;
		goto err;
	}

err:
	return ret;
}


static void init_dynamic_aid(struct lcd_info *lcd)
{
	lcd->daid.vreg = VREG_OUT_X1000;
	lcd->daid.iv_tbl = index_voltage_table;
	lcd->daid.iv_max = IV_MAX;
	lcd->daid.mtp = kzalloc(IV_MAX * CI_MAX * sizeof(int), GFP_KERNEL);
	lcd->daid.gamma_default = gamma_default;
	lcd->daid.formular = gamma_formula;
	lcd->daid.vt_voltage_value = vt_voltage_value;

	lcd->daid.ibr_tbl = index_brightness_table;
	lcd->daid.ibr_max = IBRIGHTNESS_MAX;
	lcd->daid.gc_tbls = gamma_curve_tables;
	lcd->daid.gc_lut = gamma_curve_lut;

	lcd->daid.br_base = pbrightness_base_table;
	lcd->daid.offset_gra = poffset_gradation;
	lcd->daid.offset_color = (const struct rgb_t(*)[])poffset_color;
}

static void init_mtp_data(struct lcd_info *lcd, const u8 *mtp_data)
{
	int i, c, j;

	int *mtp;

	mtp = lcd->daid.mtp;

	for (c = 0, j = 0; c < CI_MAX; c++, j++) {
		if (mtp_data[j++] & 0x01)
			mtp[(IV_MAX-1)*CI_MAX+c] = mtp_data[j] * (-1);
		else
			mtp[(IV_MAX-1)*CI_MAX+c] = mtp_data[j];
	}

	for (i = IV_MAX - 2; i >= 0; i--) {
		for (c = 0; c < CI_MAX; c++, j++) {
			if (mtp_data[j] & 0x80)
				mtp[CI_MAX*i+c] = (mtp_data[j] & 0x7F) * (-1);
			else
				mtp[CI_MAX*i+c] = mtp_data[j];
		}
	}

	for (i = 0, j = 0; i <= IV_MAX; i++)
		for (c = 0; c < CI_MAX; c++, j++)
			smtd_dbg("mtp_data[%d] = %d\n", j, mtp_data[j]);

	for (i = 0, j = 0; i < IV_MAX; i++)
		for (c = 0; c < CI_MAX; c++, j++)
			smtd_dbg("mtp[%d] = %d\n", j, mtp[j]);

	for (i = 0, j = 0; i < IV_MAX; i++) {
		for (c = 0; c < CI_MAX; c++, j++)
			smtd_dbg("%04d ", mtp[j]);
		smtd_dbg("\n");
	}
}

static int init_gamma_table(struct lcd_info *lcd , const u8 *mtp_data)
{
	int i, c, j, v;
	int ret = 0;
	int *pgamma;
	int **gamma;

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
		lcd->gamma_table[i][0] = 0xCA;
	}

	/* calculate gamma table */
	init_mtp_data(lcd, mtp_data);
	dynamic_aid(lcd->daid, gamma);

	/* relocate gamma order */
	for (i = 0; i < IBRIGHTNESS_MAX; i++) {
		/* Brightness table */
		v = IV_MAX - 1;
		pgamma = &gamma[i][v * CI_MAX];
		for (c = 0, j = 1; c < CI_MAX; c++, pgamma++) {
			if (*pgamma & 0x100)
				lcd->gamma_table[i][j++] = 1;
			else
				lcd->gamma_table[i][j++] = 0;

			lcd->gamma_table[i][j++] = *pgamma & 0xff;
		}

		for (v = IV_MAX - 2; v >= 0; v--) {
			pgamma = &gamma[i][v * CI_MAX];
			for (c = 0; c < CI_MAX; c++, pgamma++)
				lcd->gamma_table[i][j++] = *pgamma;
		}
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
	int i;

	for (i = 0; i < IBRIGHTNESS_MAX; i++)
		memcpy(lcd->aor[i], pSEQ_AOR_CONTROL, ARRAY_SIZE(SEQ_AOR_CONTROL));

	for (i = 0; i < IBRIGHTNESS_MAX; i++) {
		lcd->aor[i][1] = paor_cmd[i][0];
		lcd->aor[i][2] = paor_cmd[i][1];
	}

	return 0;
}

static int init_tset_table(struct lcd_info *lcd)
{
	int i, j, ret;

	lcd->tset_table = kzalloc(TSET_STATUS_MAX * sizeof(u8 *), GFP_KERNEL);
	if (!lcd->tset_table) {
		pr_err("failed to allocate tset table\n");
		ret = -ENOMEM;
		goto err_alloc_tset_table;
	}

	for (i = 0; i < TSET_STATUS_MAX; i++) {
		lcd->tset_table[i] = kzalloc(LDI_TSET_LEN * sizeof(u8), GFP_KERNEL);
		if (!lcd->tset_table[i]) {
			pr_err("failed to allocate tset\n");
			ret = -ENOMEM;
			goto err_alloc_tset;
		}

		lcd->tset_table[i][0] = SEQ_TSET[0];
		lcd->tset_table[i][1] = TSET_TABLE[i];
	}

	for (i = 0; i < TSET_STATUS_MAX; i++) {
		for (j = 0; j < 2; j++)
			smtd_dbg("0x%02x, ", lcd->tset_table[i][j]);
		smtd_dbg("\n");
	}

	return 0;

err_alloc_tset:
	while (i > 0)
		kfree(lcd->tset_table[--i]);

err_alloc_tset_table:
	return ret;
}


static int init_elvss_table(struct lcd_info *lcd)
{
	int i, acl, ret;

	for (acl = 0; acl < ACL_STATUS_MAX; acl++) {
		lcd->elvss_table[acl] = kzalloc(ELVSS_STATUS_MAX * sizeof(u8 *), GFP_KERNEL);

		if (IS_ERR_OR_NULL(lcd->elvss_table[acl])) {
			pr_err("failed to allocate elvss table\n");
			ret = -ENOMEM;
			goto err_alloc_elvss_table;
		}

		for (i = 0; i < ELVSS_STATUS_MAX; i++) {
			lcd->elvss_table[acl][i] = kzalloc(ELVSS_PARAM_SIZE * sizeof(u8), GFP_KERNEL);
			if (IS_ERR_OR_NULL(lcd->elvss_table[acl][i])) {
				pr_err("failed to allocate elvss\n");
				ret = -ENOMEM;
				goto err_alloc_elvss;
			}

			lcd->elvss_table[acl][i][0] = SEQ_ELVSS_SET[0];
			lcd->elvss_table[acl][i][1] = MPS_TABLE[acl];
			lcd->elvss_table[acl][i][2] = ELVSS_TABLE[acl][i];
		}
	}

	return 0;

err_alloc_elvss:
	/* should be kfree elvss with acl */
	i = ELVSS_STATUS_MAX;
	while (i > 0)
			kfree(lcd->elvss_table[acl][--i]);

err_alloc_elvss_table:

	return ret;
}

static int init_hbm_parameter(struct lcd_info *lcd, const u8 *mtp_data)
{
	int i;

	/* CA: 1~6 = C8: 34~39 */
	for (i = 0; i < 6; i++)
		lcd->gamma_table[IBRIGHTNESS_400NT][1 + i] = mtp_data[33 + i];

	/* CA: 7~21 = C8: 73~87 */
	for (i = 0; i < 15; i++)
		lcd->gamma_table[IBRIGHTNESS_400NT][7 + i] = mtp_data[72 + i];

	return 0;

}

static void show_lcd_table(struct lcd_info *lcd)
{
	int i, j, acl;

	for (i = 0; i < IBRIGHTNESS_MAX; i++) {
		smtd_dbg("%03d: ", index_brightness_table[i]);
		for (j = 0; j < GAMMA_PARAM_SIZE; j++)
			smtd_dbg("%02X, ", lcd->gamma_table[i][j]);
		smtd_dbg("\n");
	}
	smtd_dbg("\n");

	for (i = 0; i < IBRIGHTNESS_MAX; i++) {
		smtd_dbg("%03d: ", index_brightness_table[i]);
		for (j = 0; j < AID_PARAM_SIZE; j++)
			smtd_dbg("%02X ", lcd->aor[i][j]);
		smtd_dbg("\n");
	}
	smtd_dbg("\n");


	for (acl = 0; acl < ACL_STATUS_MAX; acl++) {
		smtd_dbg("acl: %d,\n", acl);
		for (i = 0; i < ELVSS_STATUS_MAX; i++) {
			smtd_dbg("%03d: ", ELVSS_DIM_TABLE[i]);
			for (j = 0; j < ELVSS_PARAM_SIZE; j++)
				smtd_dbg("%02X, ", lcd->elvss_table[acl][i][j]);
			smtd_dbg("\n");
		}
		smtd_dbg("\n");
	}
}

static int update_brightness(struct lcd_info *lcd, u8 force)
{
	u32 brightness;

	mutex_lock(&lcd->bl_lock);

	brightness = lcd->bd->props.brightness;

	lcd->bl = lcd->gamma_level[brightness];

	if (LEVEL_IS_HBM(lcd->auto_brightness) && (brightness == lcd->bd->props.max_brightness))
		lcd->bl = IBRIGHTNESS_400NT;

	if (force || (lcd->ldi_enable && (lcd->current_bl != lcd->bl))) {
		s6e3ha1_write(lcd, SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
		s6e3ha1_gamma_ctl(lcd);
		s6e3ha1_aid_parameter_ctl(lcd, force);
		s6e3ha1_set_elvss(lcd, force);
		s6e3ha1_set_acl(lcd, force);
		s6e3ha1_set_tset(lcd, force);
		s6e3ha1_write(lcd, SEQ_GAMMA_UPDATE, ARRAY_SIZE(SEQ_GAMMA_UPDATE));
		s6e3ha1_write(lcd, SEQ_GAMMA_UPDATE_L, ARRAY_SIZE(SEQ_GAMMA_UPDATE_L));
		s6e3ha1_write(lcd, SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));

		lcd->current_bl = lcd->bl;

		dev_info(&lcd->ld->dev, "brightness=%d, bl=%d, candela=%d\n",
			brightness, lcd->bl, DIM_TABLE[lcd->bl]);
	}

	mutex_unlock(&lcd->bl_lock);

	return 0;
}

static int s6e3ha1_ldi_init(struct lcd_info *lcd)
{
	int ret = 0;

	lcd->connected = 1;
	s6e3ha1_read_id(lcd, lcd->id);
	dev_info(&lcd->ld->dev," %s : id [%x] [%x] [%x] \n", __func__,
			lcd->id[0], lcd->id[1], lcd->id[2]);

	s6e3ha1_write(lcd, SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
	s6e3ha1_write(lcd, SEQ_TEST_KEY_ON_FC, ARRAY_SIZE(SEQ_TEST_KEY_ON_FC));

	/*2.1 TE(Vsync) ON/OFF*/
	s6e3ha1_write(lcd, SEQ_TE_ON, ARRAY_SIZE(SEQ_TE_ON));

	/* 1. Interface Control (Single DSI, MIC) */
	s6e3ha1_write(lcd, SEQ_MIPI_SINGLE_DSI_SET1, ARRAY_SIZE(SEQ_MIPI_SINGLE_DSI_SET1));
	s6e3ha1_write(lcd, SEQ_MIPI_SINGLE_DSI_SET2, ARRAY_SIZE(SEQ_MIPI_SINGLE_DSI_SET2));
	s6e3ha1_write(lcd, SEQ_PSR_ON, ARRAY_SIZE(SEQ_PSR_ON));

	s6e3ha1_write(lcd, SEQ_SLEEP_OUT, ARRAY_SIZE(SEQ_SLEEP_OUT));

	/* Wait 20ms */
	msleep(120);

	/* 1.1 Interface Control (Single DSI, MIC) */
	s6e3ha1_write(lcd, SEQ_MIPI_SINGLE_DSI_SET1, ARRAY_SIZE(SEQ_MIPI_SINGLE_DSI_SET1));

	s6e3ha1_write(lcd, SEQ_MIPI_SINGLE_DSI_SET2, ARRAY_SIZE(SEQ_MIPI_SINGLE_DSI_SET2));
	s6e3ha1_write(lcd, SEQ_PSR_ON, ARRAY_SIZE(SEQ_PSR_ON));


	/* 2.2 TSP TE(Hsync, Vsync) Control */
	s6e3ha1_write(lcd, SEQ_TOUCH_HSYNC_ON, ARRAY_SIZE(SEQ_TOUCH_HSYNC_ON));

	/* 2.3 Pentile Setting */
	s6e3ha1_write(lcd, SEQ_PENTILE_CONTROL, ARRAY_SIZE(SEQ_PENTILE_CONTROL));

	/* 2.4 POC setting */
	s6e3ha1_write(lcd, SEQ_GLOBAL_PARA_33rd, ARRAY_SIZE(SEQ_GLOBAL_PARA_33rd));
	s6e3ha1_write(lcd, SEQ_POC_SETTING, ARRAY_SIZE(SEQ_POC_SETTING));
	s6e3ha1_write(lcd, SEQ_SETUP_MARGIN, ARRAY_SIZE(SEQ_SETUP_MARGIN));

	/* 3. Brightness Control */
	/* 4. ELVSS Control */
	update_brightness(lcd, 1);

	/* 4. Etc Condition */

	s6e3ha1_write(lcd, SEQ_TEST_KEY_OFF_FC, ARRAY_SIZE(SEQ_TEST_KEY_OFF_FC));
	s6e3ha1_write(lcd, SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));

	return ret;
}

static int s6e3ha1_ldi_enable(struct lcd_info *lcd)
{
	int ret = 0;

	msleep(16);
	s6e3ha1_write(lcd, SEQ_DISPLAY_ON, ARRAY_SIZE(SEQ_DISPLAY_ON));

	dev_info(&lcd->ld->dev, "DISPLAY_ON\n");

	mutex_lock(&lcd->bl_lock);
	lcd->ldi_enable = 1;
	mutex_unlock(&lcd->bl_lock);

	return ret;
}

static int s6e3ha1_ldi_disable(struct lcd_info *lcd)
{
	int ret = 0;

	dev_info(&lcd->ld->dev, "+ %s\n", __func__);

	s6e3ha1_write(lcd, SEQ_DISPLAY_OFF, ARRAY_SIZE(SEQ_DISPLAY_OFF));

	dev_info(&lcd->ld->dev, "DISPLAY_OFF\n");

	/* Wait 10ms */
	msleep(10);

	/* after display off there is okay to send the commands via MIPI DSI Command
	because we don't need to worry about screen blinking. */
	s6e3ha1_write(lcd, SEQ_SLEEP_IN, ARRAY_SIZE(SEQ_SLEEP_IN));

	/* Wait 120ms */
	msleep(120);

	dev_info(&lcd->ld->dev, "- %s\n", __func__);

	return ret;
}

static int s6e3ha1_power_on(struct lcd_info *lcd)
{
	int ret = 0;

	dev_info(&lcd->ld->dev, "+ %s\n", __func__);

	ret = s6e3ha1_ldi_init(lcd);
	if (ret) {
		dev_err(&lcd->ld->dev, "failed to initialize ldi.\n");
		goto err;
	}

#if 0 /* move to s6e3fa0_fb_notifier_callback to write disp on command after fb_unblank */
	ret = s6e3ha1_ldi_enable(lcd);
	if (ret) {
		dev_err(&lcd->ld->dev, "failed to enable ldi.\n");
		goto err;
	}

	mutex_lock(&lcd->bl_lock);
	lcd->ldi_enable = 1;
	mutex_unlock(&lcd->bl_lock);
#endif

//	update_brightness(lcd, 1);

	if (!lcd->err_count)
		s6e3ha1_enable_errfg(lcd);

	dev_info(&lcd->ld->dev, "- %s\n", __func__);
err:
	return ret;
}

static int s6e3ha1_power_off(struct lcd_info *lcd)
{
	int ret = 0;

	dev_info(&lcd->ld->dev, "+ %s\n", __func__);

	if (!lcd->err_count)
		disable_irq(lcd->errfg_irq);

	mutex_lock(&lcd->bl_lock);
	lcd->ldi_enable = 0;
	mutex_unlock(&lcd->bl_lock);

	ret = s6e3ha1_ldi_disable(lcd);

	dev_info(&lcd->ld->dev, "- %s\n", __func__);

	return ret;
}

static int s6e3ha1_power(struct lcd_info *lcd, int power)
{
	int ret = 0;

	if (POWER_IS_ON(power) && !POWER_IS_ON(lcd->power))
		ret = s6e3ha1_power_on(lcd);
	else if (!POWER_IS_ON(power) && POWER_IS_ON(lcd->power))
		ret = s6e3ha1_power_off(lcd);

	if (!ret)
		lcd->power = power;

	return ret;
}

static int s6e3ha1_set_power(struct lcd_device *ld, int power)
{
	struct lcd_info *lcd = lcd_get_data(ld);

	if (power != FB_BLANK_UNBLANK && power != FB_BLANK_POWERDOWN &&
		power != FB_BLANK_NORMAL) {
		dev_err(&lcd->ld->dev, "power value should be 0, 1 or 4.\n");
		return -EINVAL;
	}

	return s6e3ha1_power(lcd, power);
}

static int s6e3ha1_get_power(struct lcd_device *ld)
{
	struct lcd_info *lcd = lcd_get_data(ld);

	return lcd->power;
}

static int s6e3ha1_check_fb(struct lcd_device *ld, struct fb_info *fb)
{
	return 0;
}

static int s6e3ha1_get_brightness(struct backlight_device *bd)
{
	struct lcd_info *lcd = bl_get_data(bd);

	return DIM_TABLE[lcd->bl];
}

static int s6e3ha1_set_brightness(struct backlight_device *bd)
{
	int ret = 0;
	int brightness = bd->props.brightness;
	struct lcd_info *lcd = bl_get_data(bd);

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

static int check_fb_brightness(struct backlight_device *bd, struct fb_info *fb)
{
	return 0;
}

static struct lcd_ops s6e3ha1_lcd_ops = {
	.set_power = s6e3ha1_set_power,
	.get_power = s6e3ha1_get_power,
	.check_fb  = s6e3ha1_check_fb,
};

static const struct backlight_ops s6e3ha1_backlight_ops = {
	.get_brightness = s6e3ha1_get_brightness,
	.update_status = s6e3ha1_set_brightness,
	.check_fb = check_fb_brightness,
};


#if defined(CONFIG_FB_S5P_MDNIE_LITE)
static struct mdnie_ops s6e3ha1_mdnie_ops = {
	.write = s6e3ha1_mdnie_write,
	.read = s6e3ha1_mdnie_read,
};
#endif

static int s6e3ha1_fb_notifier_callback(struct notifier_block *self,
		unsigned long event, void *data)
{
	struct lcd_info *lcd;
	struct fb_event *blank = (struct fb_event*) data;
	unsigned int *value = (unsigned int*)blank->data;
	int ret = 0;

	lcd = container_of(self, struct lcd_info, fb_notif);

	if (event == FB_EVENT_BLANK) {
		switch (*value) {
		case FB_BLANK_POWERDOWN:
		case FB_BLANK_NORMAL:
			lcd->fb_unblank = 0;
			break;
		case FB_BLANK_UNBLANK:
			s6e3ha1_ldi_enable(lcd);
			lcd->fb_unblank = 1;
			update_brightness(lcd, 0);
			break;
		default:
			break;
		}
	}

	return 0;
}

static int s6e3ha1_register_fb(struct lcd_info *lcd)
{
	memset(&lcd->fb_notif, 0, sizeof(lcd->fb_notif));
	lcd->fb_notif.notifier_call = s6e3ha1_fb_notifier_callback;

	return fb_register_client(&lcd->fb_notif);
}


static ssize_t power_reduce_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);

	sprintf(buf, "%u\n", lcd->acl_enable);

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
static void s6e3ha1_enable_errfg(struct lcd_info *lcd)
{
	u32 irq_ctrl_reg;

	if (!IS_ERR_OR_NULL(lcd->errfg_pend)) {
		irq_ctrl_reg = readl(lcd->errfg_pend);
		if (irq_ctrl_reg & ERRFG_PEND_MASK)
			writel(ERRFG_PEND_MASK, lcd->errfg_pend);
	}
	enable_irq(lcd->errfg_irq);
}

static void err_detection_work(struct work_struct *work)
{
	struct lcd_info *lcd =
		container_of(work, struct lcd_info, err_worker.work);

	dev_info(&lcd->ld->dev, "%s, %d\n", __func__, lcd->err_count);

	if (!lcd->ldi_enable) {
		dev_info(&lcd->ld->dev, "%s ldi off\n", __func__);
		lcd->err_count = 0;
		return;
	}
#if 0
	s5p_mipi_dsi_disable_by_fimd(lcd->dev);
	msleep(50);
	s5p_mipi_dsi_enable_by_fimd(lcd->dev);
#else //HS toggle
	s5p_dsim_frame_done_interrupt_enable(lcd->dev);
#endif
	lcd->err_count = 0;
	enable_irq(lcd->errfg_irq);
}

static irqreturn_t s6e3ha1_irq(int irq, void *dev_id)
{
	struct lcd_info *lcd = dev_id;

	spin_lock(&lcd->slock);

	dev_info(&lcd->ld->dev,"%s, %d\n", __func__, lcd->err_count);

	lcd->err_count++;
	if (lcd->err_count == 1) {
		disable_irq_nosync(lcd->errfg_irq);
		schedule_delayed_work(&lcd->err_worker, msecs_to_jiffies(500));
	}

	spin_unlock(&lcd->slock);

	return IRQ_HANDLED;
}

static int s6e3ha1_init_irq(struct lcd_info *lcd)
{
	struct lcd_platform_data *lcd_pd = NULL;
	int ret = 0;

	/* IRQ setting */
	if (lcd->dsim->pd->dsim_lcd_config->mipi_ddi_pd)
		lcd_pd = (struct lcd_platform_data *)lcd->dsim->pd->dsim_lcd_config->mipi_ddi_pd;

	if (lcd_pd && lcd_pd->pdata)
		lcd->errfg_irq = *((int *)lcd_pd->pdata);
	else
		lcd->errfg_irq = -EINVAL;

	dev_info(&lcd->ld->dev, "%s errfg_irq = %d\n", __func__, lcd->errfg_irq);

	if (lcd->errfg_irq >= 0) {
		lcd->errfg_pend = ioremap(ERRFG_PEND_REG, 0x4);
		if (IS_ERR_OR_NULL(lcd->errfg_pend))
			pr_err("%s: fail to ioremap\n", __func__);

		INIT_DELAYED_WORK(&lcd->err_worker, err_detection_work);
		spin_lock_init(&lcd->slock);

		ret = devm_request_irq(lcd->dev, lcd->errfg_irq, s6e3ha1_irq,
				  IRQF_TRIGGER_RISING, "s6e3ha1", lcd);
		if (ret)
			dev_err(&lcd->ld->dev, "irq request failed %d\n",
				lcd->errfg_irq);
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

	sprintf(buf, "%02X %02X %02X\n", lcd->id[0], lcd->id[1], lcd->id[2]);

	return strlen(buf);
}

static ssize_t ddi_id_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);

	sprintf(buf, "ddi id : %02X %02X %02X %02X %02X\n",
		lcd->ddi_id[0], lcd->ddi_id[1], lcd->ddi_id[2], lcd->ddi_id[3], lcd->ddi_id[4]);

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

	return strlen(buf);
}

static ssize_t auto_brightness_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);

	sprintf(buf, "%u\n", lcd->auto_brightness);

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

	sprintf(buf, "%u\n", lcd->siop_enable);

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

static ssize_t color_coordinate_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);

	sprintf(buf, "%u, %u\n", lcd->coordinate[0], lcd->coordinate[1]);

	return strlen(buf);
}

static ssize_t manufacture_date_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	u16 year;
	u8 month;

	year = ((lcd->date[0] & 0xF0) >> 4) + 2011;
	month = lcd->date[0] & 0xF;

	sprintf(buf, "%d, %d, %d\n", year, month, lcd->date[1]);

	return strlen(buf);
}


static ssize_t aid_log_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	u8 temp[256];
	int i, j, k;
	int *mtp;

	mtp = lcd->daid.mtp;
	for (i = 0, j = 0; i < IV_MAX; i++, j += 3) {
		if (i == 0)
			dev_info(dev, "MTP Offset VT R:%d G:%d B:%d\n",
					mtp[j], mtp[j + 1], mtp[j + 2]);
		else
			dev_info(dev, "MTP Offset V%d R:%d G:%d B:%d\n",
					lcd->daid.iv_tbl[i], mtp[j], mtp[j + 1], mtp[j + 2]);
	}

	for (i = 0; i < IBRIGHTNESS_MAX; i++) {
		memset(temp, 0, sizeof(temp));
		for (j = 1; j < GAMMA_PARAM_SIZE; j++) {
			if (j == 1 || j == 3 || j == 5)
				k = lcd->gamma_table[i][j++] * 256;
			else
				k = 0;
			snprintf(temp + strnlen(temp, 256), 256, " %d",
				lcd->gamma_table[i][j] + k);
		}

		dev_info(dev, "nit : %3d  %s\n", lcd->daid.ibr_tbl[i], temp);
	}

	dev_info(dev, "%s\n", __func__);

	return strlen(buf);
}

static DEVICE_ATTR(power_reduce, 0664, power_reduce_show, power_reduce_store);
static DEVICE_ATTR(lcd_type, 0444, lcd_type_show, NULL);
static DEVICE_ATTR(window_type, 0444, window_type_show, NULL);
static DEVICE_ATTR(ddi_id, 0444, ddi_id_show, NULL);
static DEVICE_ATTR(gamma_table, 0444, gamma_table_show, NULL);
static DEVICE_ATTR(auto_brightness, 0644, auto_brightness_show, auto_brightness_store);
static DEVICE_ATTR(siop_enable, 0664, siop_enable_show, siop_enable_store);
static DEVICE_ATTR(temperature, 0664, temperature_show, temperature_store);
static DEVICE_ATTR(color_coordinate, 0444, color_coordinate_show, NULL);
static DEVICE_ATTR(manufacture_date, 0444, manufacture_date_show, NULL);
static DEVICE_ATTR(aid_log, 0444, aid_log_show, NULL);

static int s6e3ha1_probe(struct mipi_dsim_device *dsim)
{
	int ret;
	struct lcd_info *lcd;

	u8 mtp_data[LDI_MTP_LEN] = {0,};

	lcd = kzalloc(sizeof(struct lcd_info), GFP_KERNEL);
	if (!lcd) {
		pr_err("failed to allocate for lcd\n");
		ret = -ENOMEM;
		goto err_alloc;
	}

	lcd->ld = lcd_device_register("panel", dsim->dev, lcd, &s6e3ha1_lcd_ops);
	if (IS_ERR(lcd->ld)) {
		pr_err("failed to register lcd device\n");
		ret = PTR_ERR(lcd->ld);
		goto out_free_lcd;
	}
	dsim->lcd = lcd->ld;

	lcd->bd = backlight_device_register("panel", dsim->dev, lcd, &s6e3ha1_backlight_ops, NULL);
	if (IS_ERR(lcd->bd)) {
		pr_err("failed to register backlight device\n");
		ret = PTR_ERR(lcd->bd);
		goto out_free_backlight;
	}
#ifdef CONFIG_FB_HW_TRIGGER
	g_lcd = lcd;
#endif
	lcd->dev = dsim->dev;
	lcd->dsim = dsim;
	lcd->bd->props.max_brightness = MAX_BRIGHTNESS;
	lcd->bd->props.brightness = DEFAULT_BRIGHTNESS;
	lcd->bl = DEFAULT_GAMMA_INDEX;
	lcd->current_bl = lcd->bl;
	lcd->acl_enable = 0;
	lcd->current_acl = 0;
#ifdef CONFIG_S5P_LCD_INIT
	lcd->power = FB_BLANK_POWERDOWN;
#else
	lcd->power = FB_BLANK_UNBLANK;
#endif
	lcd->auto_brightness = 0;
	lcd->connected = 1;
	lcd->siop_enable = 0;
	lcd->temperature = NORMAL_TEMPERATURE;
	lcd->fb_unblank = 1;

	ret = device_create_file(&lcd->ld->dev, &dev_attr_power_reduce);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "failed to add sysfs entries, %d\n", __LINE__);

	ret = device_create_file(&lcd->ld->dev, &dev_attr_lcd_type);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "failed to add sysfs entries, %d\n", __LINE__);

	ret = device_create_file(&lcd->ld->dev, &dev_attr_window_type);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "failed to add sysfs entries, %d\n", __LINE__);

	ret = device_create_file(&lcd->ld->dev, &dev_attr_ddi_id);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "failed to add sysfs entries, %d\n", __LINE__);

	ret = device_create_file(&lcd->ld->dev, &dev_attr_gamma_table);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "failed to add sysfs entries, %d\n", __LINE__);

	ret = device_create_file(&lcd->bd->dev, &dev_attr_auto_brightness);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "failed to add sysfs entries, %d\n", __LINE__);

	ret = device_create_file(&lcd->ld->dev, &dev_attr_siop_enable);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "failed to add sysfs entries, %d\n", __LINE__);

	ret = device_create_file(&lcd->ld->dev, &dev_attr_temperature);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "failed to add sysfs entries, %d\n", __LINE__);

	ret = device_create_file(&lcd->ld->dev, &dev_attr_color_coordinate);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "failed to add sysfs entries, %d\n", __LINE__);

	ret = device_create_file(&lcd->ld->dev, &dev_attr_manufacture_date);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "failed to add sysfs entries, %d\n", __LINE__);

	ret = device_create_file(&lcd->ld->dev, &dev_attr_aid_log);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "failed to add sysfs entries, %d\n", __LINE__);

	ret = s6e3ha1_register_fb(lcd);
	if (ret)
		dev_err(&lcd->ld->dev, "failed to register fb notifier chain\n");

	mutex_init(&lcd->lock);
	mutex_init(&lcd->bl_lock);

	s6e3ha1_write(lcd, SEQ_TEST_KEY_ON_F0, ARRAY_SIZE(SEQ_TEST_KEY_ON_F0));
	s6e3ha1_read_id(lcd, lcd->id);
	s6e3ha1_read_ddi_id(lcd, lcd->ddi_id);
	s6e3ha1_read_coordinate(lcd);
	s6e3ha1_read_mtp(lcd, mtp_data);
	s6e3ha1_read_elvss_hbm(lcd);
	s6e3ha1_write(lcd, SEQ_TEST_KEY_OFF_F0, ARRAY_SIZE(SEQ_TEST_KEY_OFF_F0));

	dev_info(&lcd->ld->dev, "ID: %x, %x, %x\n", lcd->id[0], lcd->id[1], lcd->id[2]);

	s6e3ha1_update_seq(lcd);

	init_tset_table(lcd);
	ret = init_backlight_level_from_brightness(lcd);
	if(ret < 0)
		dev_info(&lcd->ld->dev, "gamma level generation is failed\n");

	init_dynamic_aid(lcd);

	ret = init_gamma_table(lcd, mtp_data);
	ret += init_aid_dimming_table(lcd);
	ret += init_elvss_table(lcd);
	ret += init_hbm_parameter(lcd, mtp_data);

	if (ret)
		dev_info(&lcd->ld->dev, "gamma table generation is failed\n");

	show_lcd_table(lcd);

	lcd->ldi_enable = 1;

	s6e3ha1_init_irq(lcd);

	update_brightness(lcd, 1);

#if defined(CONFIG_FB_S5P_MDNIE_LITE)
		lcd->md = mdnie_device_register("mdnie", &lcd->ld->dev, &s6e3ha1_mdnie_ops);
#endif

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

static int s6e3ha1_displayon(struct mipi_dsim_device *dsim)
{
	struct lcd_info *lcd = dev_get_drvdata(&dsim->lcd->dev);

	s6e3ha1_power(lcd, FB_BLANK_UNBLANK);

	return 0;
}

static int s6e3ha1_suspend(struct mipi_dsim_device *dsim)
{
	struct lcd_info *lcd = dev_get_drvdata(&dsim->lcd->dev);

	s6e3ha1_power(lcd, FB_BLANK_POWERDOWN);

	return 0;
}

static int s6e3ha1_resume(struct mipi_dsim_device *dsim)
{
	return 0;
}

struct mipi_dsim_lcd_driver s6e3ha1_mipi_lcd_driver = {
	.probe		= s6e3ha1_probe,
	.displayon	= s6e3ha1_displayon,
	.suspend	= s6e3ha1_suspend,
	.resume		= s6e3ha1_resume,
};

static int s6e3ha1_init(void)
{
	return 0;
}

static void s6e3ha1_exit(void)
{
	return;
}

module_init(s6e3ha1_init);
module_exit(s6e3ha1_exit);

MODULE_DESCRIPTION("MIPI-DSI S6E3HA1 (1600*2560) Panel Driver");
MODULE_LICENSE("GPL");

