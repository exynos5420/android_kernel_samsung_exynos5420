/* linux/drivers/video/mdnie.c
 *
 * Register interface file for Samsung mDNIe driver
 *
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/mm.h>
#include <linux/device.h>
#include <linux/backlight.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/lcd.h>
#include <linux/fb.h>
#include <linux/pm_runtime.h>
#include <linux/mdnie.h>

#include "mdnie_lite.h"

#if defined(CONFIG_LCD_MIPI_S6E3HA1)
#include "mdnie_lite_table_kl.h"
#elif defined(CONFIG_LCD_MIPI_S6TNMR7)
#include "mdnie_lite_table_ch.h"

#endif

#if defined(CONFIG_TDMB)
#include "mdnie_lite_table_dmb.h"
#endif

#define MDNIE_SYSFS_PREFIX		"/sdcard/mdnie/"
#define PANEL_COORDINATE_PATH	"/sys/class/lcd/panel/color_coordinate"

#define IS_DMB(idx)			(idx == DMB_NORMAL_MODE)
#define IS_SCENARIO(idx)		((idx < SCENARIO_MAX) && !((idx > VIDEO_NORMAL_MODE) && (idx < CAMERA_MODE)))
#define IS_ACCESSIBILITY(idx)		(idx && (idx < ACCESSIBILITY_MAX))
#define IS_HBM(idx)			(idx >= 6)

#define SCENARIO_IS_VALID(idx)	(IS_DMB(idx) || IS_SCENARIO(idx))

/* Split 16 bit as 8bit x 2 */
#define GET_MSB_8BIT(x)		((x >> 8) & (BIT(8) - 1))
#define GET_LSB_8BIT(x)		((x >> 0) & (BIT(8) - 1))

static struct class *mdnie_class;

#ifdef ASCR_BIT_SHIFT
static void inline store_ascr(struct mdnie_table *table, int pos, mdnie_t data)
{
	mdnie_t * wbuf = &(table->tune[ASCR_CMD].sequence[pos]);
	unsigned short * tmp = (unsigned short *)wbuf;
	int bit_shift = ASCR_BIT_SHIFT;

	*tmp &= ~(cpu_to_be16(0xFF << bit_shift));
	*tmp |= cpu_to_be16(((u16)data) << bit_shift);
}

static mdnie_t inline read_ascr(struct mdnie_table *table, int pos)
{
	mdnie_t * wbuf = &(table->tune[ASCR_CMD].sequence[pos]);
	unsigned short * tmp = (unsigned short *)wbuf;
	int bit_shift = ASCR_BIT_SHIFT;

	return (mdnie_t) (be16_to_cpu(*tmp) >> bit_shift);
}
#else
static void inline store_ascr(struct mdnie_table *table, int pos, mdnie_t data)
{
	mdnie_t * wbuf = &(table->tune[ASCR_CMD].sequence[pos]);

	*wbuf = data;
}

static mdnie_t inline read_ascr(struct mdnie_table *table, int pos)
{
	mdnie_t * wbuf = &(table->tune[ASCR_CMD].sequence[pos]);

	return *wbuf;
}
#endif

/* Do not call mdnie write directly */
static int mdnie_write(struct mdnie_info *mdnie, struct mdnie_table *table)
{
	struct mdnie_device *md = to_mdnie_device(mdnie->dev);
	int i, ret = 0;

	if (!mdnie->enable || !md->ops->write)
		return -EIO;

#ifdef MDNIE_USE_SET_ADDR
	ret = md->ops->set_addr(md->dev.parent, MDNIE_SEQUENCE_OFFSET_1);
	ret = md->ops->write(md->dev.parent, table->tune[MDNIE_CMD1].sequence,
							table->tune[MDNIE_CMD1].size);
	ret = md->ops->set_addr(md->dev.parent, MDNIE_SEQUENCE_OFFSET_2);
	ret = md->ops->write(md->dev.parent, table->tune[MDNIE_CMD2].sequence,
							table->tune[MDNIE_CMD2].size);
#else
	for (i = 0; i < ARRAY_SIZE(table->tune); i++) {
			ret = md->ops->write(md->dev.parent, table->tune[i].sequence, table->tune[i].size);
	}
#endif

	return ret;
}

static int mdnie_write_table(struct mdnie_info *mdnie, struct mdnie_table *table)
{
	int i, ret = 0;
	struct mdnie_table *buf = NULL;

	for (i = 0; i < MDNIE_CMD_MAX; i++) {
		if (IS_ERR_OR_NULL(table->tune[i].sequence)) {
			dev_err(mdnie->dev, "mdnie sequence %s is null, %x\n", table->name, (u32)table->tune[i].sequence);
			return -EPERM;
		}
	}

	mutex_lock(&mdnie->dev_lock);

	buf = table;

	ret = mdnie_write(mdnie, buf);

	mutex_unlock(&mdnie->dev_lock);

	return ret;
}

static struct mdnie_table *mdnie_find_table(struct mdnie_info *mdnie)
{
	struct mdnie_table *table = NULL;

	mutex_lock(&mdnie->lock);

	if (IS_ACCESSIBILITY(mdnie->accessibility)) {
		table = &accessibility_table[mdnie->accessibility];
		goto exit;
	} else if (IS_HBM(mdnie->auto_brightness)) {
#if defined(CONFIG_LCD_MIPI_S6E3HA1) || defined(CONFIG_LCD_MIPI_S6TNMR7)
		if((mdnie->scenario == BROWSER_MODE)|| (mdnie->scenario == EBOOK_MODE))
			table = &hbm_table[HBM_ON_TEXT];
		else
#endif
		table = &hbm_table[mdnie->hbm];
		goto exit;
#if defined(CONFIG_TDMB)
	} else if (IS_DMB(mdnie->scenario)) {
		table = &dmb_table[mdnie->mode];
		goto exit;
#endif
	} else if (IS_SCENARIO(mdnie->scenario)) {
		table = &tuning_table[mdnie->scenario][mdnie->mode];
		goto exit;
	}

exit:
	mutex_unlock(&mdnie->lock);

	return table;
}

static void mdnie_update_sequence(struct mdnie_info *mdnie, struct mdnie_table *table)
{
	struct mdnie_table *t = NULL;

	if (mdnie->tuning) {
		t = mdnie_request_table(mdnie->path, table);
		if (!IS_ERR_OR_NULL(t) && !IS_ERR_OR_NULL(t->name))
			mdnie_write_table(mdnie, t);
		else
			mdnie_write_table(mdnie, table);
	} else
		mdnie_write_table(mdnie, table);
	return;
}

static void mdnie_update(struct mdnie_info *mdnie)
{
	struct mdnie_table *table = NULL;

	if (!mdnie->enable) {
		dev_err(mdnie->dev, "mdnie state is off\n");
		return;
	}

	table = mdnie_find_table(mdnie);
	if (!IS_ERR_OR_NULL(table) && !IS_ERR_OR_NULL(table->name)) {
		mdnie_update_sequence(mdnie, table);
		dev_info(mdnie->dev, "%s\n", table->name);

		mdnie->white_r = read_ascr(table, MDNIE_WHITE_R);
		mdnie->white_g = read_ascr(table, MDNIE_WHITE_G);
		mdnie->white_b = read_ascr(table, MDNIE_WHITE_B);
	}

	return;
}

static void update_color_position(struct mdnie_info *mdnie, unsigned int idx)
{
	u8 mode, scenario;
	struct mdnie_table *table = NULL;
	mdnie_t *wbuf;

	dev_info(mdnie->dev, "%s: idx=%d\n", __func__, idx);

	mutex_lock(&mdnie->lock);

	for (mode = 0; mode < MODE_MAX; mode++) {
		for (scenario = 0; scenario <= EMAIL_MODE; scenario++) {
			wbuf = tuning_table[scenario][mode].tune[ASCR_CMD].sequence;
			if (IS_ERR_OR_NULL(wbuf))
				continue;
			table = &tuning_table[scenario][mode];
			if ((read_ascr(table, MDNIE_WHITE_R) == 0xff)
				&& (read_ascr(table, MDNIE_WHITE_G) == 0xff)
				&& (read_ascr(table, MDNIE_WHITE_B) == 0xff)) {
				store_ascr(table, MDNIE_WHITE_R, coordinate_data[idx][0]);
				store_ascr(table, MDNIE_WHITE_G, coordinate_data[idx][1]);
				store_ascr(table, MDNIE_WHITE_B, coordinate_data[idx][2]);
			}
		}
	}

	mutex_unlock(&mdnie->lock);
}

static int get_panel_coordinate(struct mdnie_info *mdnie, int *result)
{
	int ret = 0;
	char *fp = NULL;
	unsigned short x, y;

	ret = mdnie_open_file(PANEL_COORDINATE_PATH, &fp);
	if (IS_ERR_OR_NULL(fp) || ret <= 0) {
		dev_info(mdnie->dev, "%s: open skip: %s, %d\n", __func__, PANEL_COORDINATE_PATH, ret);
		ret = -EINVAL;
		goto skip_color_correction;
	}

	ret = sscanf(fp, "%hu, %hu", &x, &y);
	if ((ret != 2) || (!x && !y)) {
		dev_info(mdnie->dev, "%s: %d, %d\n", __func__, x, y);
		ret = -EINVAL;
		goto skip_color_correction;
	}

	result[1] = COLOR_OFFSET_F1(x, y);
	result[2] = COLOR_OFFSET_F2(x, y);
	result[3] = COLOR_OFFSET_F3(x, y);
	result[4] = COLOR_OFFSET_F4(x, y);

	ret = mdnie_calibration(result);
	dev_info(mdnie->dev, "%s: %d, %d, idx=%d\n", __func__, x, y, ret);

skip_color_correction:
	mdnie->color_correction = 1;
	if (!IS_ERR_OR_NULL(fp))
		kfree(fp);

	return ret;
}

static ssize_t mode_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mdnie_info *mdnie = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", mdnie->mode);
}

static ssize_t mode_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct mdnie_info *mdnie = dev_get_drvdata(dev);
	unsigned int value = 0;
	int ret;
	int result[5] = {0,};

	ret = kstrtoul(buf, 0, (unsigned long *)&value);
	if (ret < 0)
		return ret;

	dev_info(dev, "%s: value=%d\n", __func__, value);

	if (value >= MODE_MAX) {
		value = STANDARD;
		return -EINVAL;
	}

	mutex_lock(&mdnie->lock);
	mdnie->mode = value;
	mutex_unlock(&mdnie->lock);

	if (!mdnie->color_correction) {
		ret = get_panel_coordinate(mdnie, result);
		if (ret > 0)
			update_color_position(mdnie, ret);
	}

	mdnie_update(mdnie);

	return count;
}


static ssize_t scenario_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mdnie_info *mdnie = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", mdnie->scenario);
}

static ssize_t scenario_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct mdnie_info *mdnie = dev_get_drvdata(dev);
	unsigned int value;
	int ret;

	ret = kstrtoul(buf, 0, (unsigned long *)&value);
	if (ret < 0)
		return ret;

	dev_info(dev, "%s: value=%d\n", __func__, value);

	if (!SCENARIO_IS_VALID(value))
		value = UI_MODE;

	mutex_lock(&mdnie->lock);
	mdnie->scenario = value;
	mutex_unlock(&mdnie->lock);

	mdnie_update(mdnie);

	return count;
}

static ssize_t tuning_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mdnie_info *mdnie = dev_get_drvdata(dev);
	char *pos = buf;
	struct mdnie_table *table = NULL;
	int i;

	pos += sprintf(pos, "++ %s: %s\n", __func__, mdnie->path);

	if (!mdnie->tuning) {
		pos += sprintf(pos, "tunning mode is off\n");
		goto exit;
	}

	if (strncmp(mdnie->path, MDNIE_SYSFS_PREFIX, sizeof(MDNIE_SYSFS_PREFIX) - 1)) {
		pos += sprintf(pos, "file path is invalid, %s\n", mdnie->path);
		goto exit;
	}

	table = mdnie_find_table(mdnie);
	if (!IS_ERR_OR_NULL(table) && !IS_ERR_OR_NULL(table->name)) {
		table = mdnie_request_table(mdnie->path, table);
		for (i = 0; i < table->tune[MDNIE_CMD1].size; i++)
			pos += sprintf(pos, "0x%02x ", table->tune[MDNIE_CMD1].sequence[i]);
		pos += sprintf(pos, "\n");
		if (MDNIE_CMD1 != MDNIE_CMD2) {
			for (i = 0; i < table->tune[MDNIE_CMD2].size; i++)
				pos += sprintf(pos, "0x%02x ", table->tune[MDNIE_CMD2].sequence[i]);
		}
	}

exit:
	pos += sprintf(pos, "-- %s\n", __func__);

	return pos - buf;
}

static ssize_t tuning_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct mdnie_info *mdnie = dev_get_drvdata(dev);
	int ret;

	if (sysfs_streq(buf, "0") || sysfs_streq(buf, "1")) {
		ret = kstrtoul(buf, 0, (unsigned long *)&mdnie->tuning);
		if (ret < 0)
			return ret;
		if (!mdnie->tuning)
			memset(mdnie->path, 0, sizeof(mdnie->path));

		dev_info(dev, "%s: %s\n", __func__, mdnie->tuning ? "enable" : "disable");
	} else {
		if (!mdnie->tuning)
			return count;

		if (count > (sizeof(mdnie->path) - sizeof(MDNIE_SYSFS_PREFIX))) {
			dev_err(dev, "file name %s is too long\n", mdnie->path);
			return -ENOMEM;
		}

		memset(mdnie->path, 0, sizeof(mdnie->path));
		snprintf(mdnie->path, sizeof(MDNIE_SYSFS_PREFIX) + count-1, "%s%s", MDNIE_SYSFS_PREFIX, buf);
		dev_info(dev, "%s: %s\n", __func__, mdnie->path);

		mdnie_update(mdnie);
	}

	return count;
}

static ssize_t accessibility_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mdnie_info *mdnie = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", mdnie->accessibility);
}

static ssize_t accessibility_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct mdnie_info *mdnie = dev_get_drvdata(dev);
	unsigned int value, s[9], i = 0;
	int ret;
	struct mdnie_table *table;

	ret = sscanf(buf, "%d %x %x %x %x %x %x %x %x %x",
		&value, &s[0], &s[1], &s[2], &s[3],
		&s[4], &s[5], &s[6], &s[7], &s[8]);

	dev_info(dev, "%s: value=%d\n", __func__, value);

	if (ret < 0)
		return ret;
	else {
		if (value >= ACCESSIBILITY_MAX)
			value = ACCESSIBILITY_OFF;

		mutex_lock(&mdnie->lock);
		mdnie->accessibility = value;
		if (value == COLOR_BLIND) {
			if (ret != 10) {
				mutex_unlock(&mdnie->lock);
				return -EINVAL;
			}

			table = &accessibility_table[COLOR_BLIND];
			while (i < ARRAY_SIZE(s)) {
				store_ascr( table, MDNIE_COLOR_BLIND_OFFSET + i * 2 + 0,
						GET_LSB_8BIT(s[i]));
				store_ascr( table, MDNIE_COLOR_BLIND_OFFSET + i * 2 + 1,
						GET_MSB_8BIT(s[i]));
				i++;
			}

			dev_info(dev, "%s: %s\n", __func__, buf);
		}
		mutex_unlock(&mdnie->lock);

		mdnie_update(mdnie);
	}

	return count;
}

static ssize_t color_correct_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mdnie_info *mdnie = dev_get_drvdata(dev);
	char *pos = buf;
	int i, idx, result[5] = {0,};

	if (!mdnie->color_correction)
		return -EINVAL;

	idx = get_panel_coordinate(mdnie, result);

	for (i = 1; i < ARRAY_SIZE(result); i++)
		pos += sprintf(pos, "f%d: %d, ", i, result[i]);
	pos += sprintf(pos, "tune%d\n", idx);

	return pos - buf;
}

static ssize_t bypass_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mdnie_info *mdnie = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", mdnie->bypass);
}

static ssize_t bypass_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct mdnie_info *mdnie = dev_get_drvdata(dev);
	struct mdnie_table *table = NULL;
	unsigned int value;
	int ret;

	ret = kstrtoul(buf, 0, (unsigned long *)&value);
	if (ret)
		return ret;

	dev_info(dev, "%s :: value=%d\n", __func__, value);

	if (ret < 0)
		return ret;
	else {
		if (value >= BYPASS_MAX)
			value = BYPASS_OFF;

		value = (value) ? BYPASS_ON : BYPASS_OFF;

		mutex_lock(&mdnie->lock);
		mdnie->bypass = value;
		mutex_unlock(&mdnie->lock);

		table = &bypass_table[value];
		if (!IS_ERR_OR_NULL(table)) {
			mdnie_write_table(mdnie, table);
			dev_info(mdnie->dev, "%s\n", table->name);
		}
	}

	return count;
}

static ssize_t auto_brightness_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mdnie_info *mdnie = dev_get_drvdata(dev);

	return sprintf(buf, "%d, hbm: %d\n", mdnie->auto_brightness, mdnie->hbm);
}

static ssize_t auto_brightness_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct mdnie_info *mdnie = dev_get_drvdata(dev);
	unsigned int value;
	int ret;

	ret = kstrtoul(buf, 0, (unsigned long *)&value);
	if (ret < 0)
		return ret;

	dev_info(dev, "%s: value=%d\n", __func__, value);

	mutex_lock(&mdnie->lock);
	ret = IS_HBM(value) ? HBM_ON : HBM_OFF;
	if(mdnie->hbm == ret) {
		mutex_unlock(&mdnie->lock);
		return count;
	}

	mdnie->hbm = ret;
	mdnie->auto_brightness = value;
	mutex_unlock(&mdnie->lock);

	mdnie_update(mdnie);

	return count;
}

/* Temporary solution: Do not use this sysfs as official purpose */
static ssize_t mdnie_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mdnie_device *md = to_mdnie_device(dev);
	struct mdnie_info *mdnie = dev_get_drvdata(dev);
	char *pos = buf;
	struct mdnie_table *table = NULL;
	int i, j;
	u8 *buffer;

	if (!mdnie->enable) {
		dev_err(mdnie->dev, "mdnie state is off\n");
		goto exit;
	}

	table = mdnie_find_table(mdnie);

	for (i = 0; i < MDNIE_CMD_MAX; i++) {
		if (IS_ERR_OR_NULL(table->tune[i].sequence)) {
			dev_err(mdnie->dev, "mdnie sequence %s is null, %x\n", table->name, (u32)table->tune[i].sequence);
			goto exit;
		}
	}

	md->ops->write(md->dev.parent, table->tune[LEVEL1_KEY_UNLOCK].sequence, table->tune[LEVEL1_KEY_UNLOCK].size);

	pos += sprintf(pos, "+ %s\n", table->name);

	for (j = MDNIE_CMD1; j <= MDNIE_CMD2; j++) {
		buffer = kzalloc(table->tune[j].size, GFP_KERNEL);

#ifdef MDNIE_USE_SET_ADDR
		if (j == MDNIE_CMD1)
			md->ops->set_addr(md->dev.parent, MDNIE_SEQUENCE_OFFSET_1);
		else if (j == MDNIE_CMD2)
			md->ops->set_addr(md->dev.parent, MDNIE_SEQUENCE_OFFSET_2);
#endif

		md->ops->read(md->dev.parent, table->tune[j].sequence[0], buffer, table->tune[j].size - 1);

		for (i = 0; i < table->tune[j].size - 1; i++) {
			pos += sprintf(pos, "%3d:\t0x%02x\t0x%02x", i + 1, table->tune[j].sequence[i+1], buffer[i]);
			if (table->tune[j].sequence[i+1] != buffer[i])
				pos += sprintf(pos, "\t(X)");
			pos += sprintf(pos, "\n");
		}

		kfree(buffer);
	}

	pos += sprintf(pos, "- %s\n", table->name);

	md->ops->write(md->dev.parent, table->tune[LEVEL1_KEY_LOCK].sequence, table->tune[LEVEL1_KEY_LOCK].size);

exit:
	return pos - buf;
}

static ssize_t sensorRGB_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct mdnie_info *mdnie = dev_get_drvdata(dev);
	return sprintf(buf, "%d %d %d\n", mdnie->white_r,
		mdnie->white_g, mdnie->white_b);
}

static ssize_t sensorRGB_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct mdnie_info *mdnie = dev_get_drvdata(dev);
	struct mdnie_table *table = NULL;
	unsigned int white_red, white_green, white_blue;
	int ret;

	ret = sscanf(buf, "%d %d %d"
		, &white_red, &white_green, &white_blue);
	if (ret < 0)
		return ret;

	if (mdnie->enable && (mdnie->accessibility == ACCESSIBILITY_OFF)
		&& (mdnie->mode == AUTO)
		&& ((mdnie->scenario == BROWSER_MODE)
		|| (mdnie->scenario == EBOOK_MODE))) {
		dev_info(dev, "%s, white_r %d, white_g %d, white_b %d\n",
			__func__, white_red, white_green, white_blue);

		table = mdnie_find_table(mdnie);

		memcpy(&(mdnie->table_buffer),
			table, sizeof(struct mdnie_table));
		memcpy(mdnie->sequence_buffer,
			table->tune[ASCR_CMD].sequence,
			table->tune[ASCR_CMD].size);
		mdnie->table_buffer.tune[ASCR_CMD].sequence
			= mdnie->sequence_buffer;

		store_ascr(&mdnie->table_buffer, MDNIE_WHITE_R,
				(unsigned char)white_red);
		store_ascr(&mdnie->table_buffer, MDNIE_WHITE_G,
				(unsigned char)white_green);
		store_ascr(&mdnie->table_buffer, MDNIE_WHITE_B,
				(unsigned char)white_blue);

		mdnie->white_r = white_red;
		mdnie->white_g = white_green;
		mdnie->white_b = white_blue;

		mdnie_update_sequence(mdnie, &(mdnie->table_buffer));
	}

	return count;
}

static struct device_attribute mdnie_attributes[] = {
	__ATTR(mode, 0664, mode_show, mode_store),
	__ATTR(scenario, 0664, scenario_show, scenario_store),
	__ATTR(tuning, 0664, tuning_show, tuning_store),
	__ATTR(accessibility, 0664, accessibility_show, accessibility_store),
	__ATTR(color_correct, 0444, color_correct_show, NULL),
	__ATTR(bypass, 0664, bypass_show, bypass_store),
	__ATTR(auto_brightness, 0664, auto_brightness_show, auto_brightness_store),
	__ATTR(mdnie, 0444, mdnie_show, NULL),
	__ATTR(sensorRGB, 0664, sensorRGB_show, sensorRGB_store),
	__ATTR_NULL,
};

static int fb_notifier_callback(struct notifier_block *self,
				 unsigned long event, void *data)
{
	struct mdnie_device *md;
	struct mdnie_info *mdnie;
	struct fb_event *evdata = data;
	int fb_blank;

	switch (event) {
	case FB_EVENT_BLANK:
		break;
	default:
		return 0;
	}

	md = container_of(self, struct mdnie_device, fb_notif);
	if (!md)
		return 0;

	mdnie = (struct mdnie_info *)mdnie_get_data(md);

	fb_blank = *(int *)evdata->data;

	dev_info(mdnie->dev, "%s: %d\n", __func__, fb_blank);

	if (fb_blank == FB_BLANK_UNBLANK) {
		mutex_lock(&mdnie->lock);
		mdnie->enable = 1;
		mutex_unlock(&mdnie->lock);

		mdnie_update(mdnie);
	} else if (fb_blank == FB_BLANK_POWERDOWN) {
		mutex_lock(&mdnie->lock);
		mdnie->enable = 0;
		mutex_unlock(&mdnie->lock);
	}

	return 0;
}

static int mdnie_register_fb(struct mdnie_device *md)
{
	memset(&md->fb_notif, 0, sizeof(md->fb_notif));
	md->fb_notif.notifier_call = fb_notifier_callback;
	return fb_register_client(&md->fb_notif);
}

static void mdnie_unregister_fb(struct mdnie_device *md)
{
	fb_unregister_client(&md->fb_notif);
}

static void mdnie_device_release(struct device *dev)
{
	struct mdnie_device *md = to_mdnie_device(dev);
	kfree(md);
}

struct mdnie_device *mdnie_device_register(const char *name,
		struct device *parent, struct mdnie_ops *ops)
{
	struct mdnie_device *new_md;
	struct mdnie_info *mdnie;
	int rc;

	mdnie = kzalloc(sizeof(struct mdnie_info), GFP_KERNEL);
	if (!mdnie) {
		pr_err("failed to allocate mdnie\n");
		rc = -ENOMEM;
		goto error0;
	}

	new_md = kzalloc(sizeof(struct mdnie_device), GFP_KERNEL);
	if (!new_md) {
		pr_err("failed to allocate mdnie\n");
		rc = -ENOMEM;
		goto error1;
	}

	mutex_init(&new_md->ops_lock);
	mutex_init(&new_md->update_lock);

	new_md->dev.class = mdnie_class;
	new_md->dev.parent = parent;
	new_md->dev.release = mdnie_device_release;

	dev_set_name(&new_md->dev, name);

	mdnie->dev = &new_md->dev;
	mdnie->scenario = UI_MODE;
	mdnie->mode = AUTO;
	mdnie->enable = 0;
	mdnie->tuning = 0;
	mdnie->accessibility = ACCESSIBILITY_OFF;
	mdnie->bypass = BYPASS_OFF;

	mutex_init(&mdnie->lock);
	mutex_init(&mdnie->dev_lock);

	dev_set_drvdata(&new_md->dev, mdnie);

	rc = device_register(&new_md->dev);
	if (rc) {
		pr_err("failed to device_register mdnie\n");
		goto error2;
	}

	rc = mdnie_register_fb(new_md);
	if (rc) {
		device_unregister(&new_md->dev);
		goto error1;
	}

	new_md->ops = ops;

	mdnie->enable = 1;
	mdnie_update(mdnie);

	dev_info(mdnie->dev, "registered successfully\n");

	return new_md;

error2:
	kfree(new_md);
error1:
	kfree(mdnie);
error0:
	return ERR_PTR(rc);
}

/**
 * mdnie_device - unregisters a object of mdnie_device class.
 * @ld: the mdnie device object to be unregistered and freed.
 *
 * Unregisters a previously registered via mdnie_device_register object.
 */
void mdnie_device_unregister(struct mdnie_device *md)
{
	if (!md)
		return;

	mutex_lock(&md->ops_lock);
	md->ops = NULL;
	mutex_unlock(&md->ops_lock);
	mdnie_unregister_fb(md);

	device_unregister(&md->dev);
}

static void __exit mdnie_class_exit(void)
{
	class_destroy(mdnie_class);
}

static int __init mdnie_class_init(void)
{
	mdnie_class = class_create(THIS_MODULE, "mdnie");
	if (IS_ERR(mdnie_class)) {
		printk(KERN_WARNING "Unable to create mdnie class; errno = %ld\n",
				PTR_ERR(mdnie_class));
		return PTR_ERR(mdnie_class);
	}

	mdnie_class->dev_attrs = mdnie_attributes;
	return 0;
}

/*
 * if this is compiled into the kernel, we need to ensure that the
 * class is registered before users of the class try to register lcd's
 */
postcore_initcall(mdnie_class_init);
module_exit(mdnie_class_exit);

MODULE_DESCRIPTION("mDNIe Driver");
MODULE_LICENSE("GPL");
