/* linux/driver/video/samsung/lcdfreq.c
 *
 * EXYNOS4 - support LCD PixelClock change at runtime
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/reboot.h>
#include <linux/suspend.h>
#include <linux/cpufreq.h>
#include <linux/sysfs.h>
#include <linux/gcd.h>
#include <linux/clk.h>
#include <linux/spinlock.h>
#include <linux/fb.h>

#include <plat/clock.h>
#include <plat/clock-clksrc.h>
#include <plat/regs-fb.h>
#include <plat/regs-ielcd.h>

#define reg_mask(shift, size)		((0xffffffff >> (32 - size)) << shift)
#define VSTATUS_IS_ACTIVE(reg)	(reg == VIDCON1_VSTATUS_ACTIVE)
#define VSTATUS_IS_FRONT(reg)		(reg == VIDCON1_VSTATUS_FRONTPORCH)

enum lcdfreq_level {
	NORMAL,
	LIMIT,
	LEVEL_MAX,
};

struct lcdfreq_t {
	enum lcdfreq_level level;
	u32 hz;
	u32 vclk;
	u32 cmu_div;
	u32 pixclock;
};

struct lcdfreq_info {
	struct lcdfreq_t	table[LEVEL_MAX];
	enum lcdfreq_level	level;
	atomic_t		usage;
	struct mutex		lock;
	spinlock_t		slock;

	struct device		*dev;
	struct clksrc_clk 	*clksrc;

	unsigned int		enable;
	struct notifier_block	fb_notif;
	struct notifier_block	pm_noti;
	struct notifier_block	reboot_noti;

	struct delayed_work	work;

	void __iomem		*fimd_regs;
	void __iomem		*ielcd_regs;
};

static inline struct lcdfreq_info *dev_get_lcdfreq(struct device *dev)
{
	struct fb_info *fb = dev_get_drvdata(dev);

	return (struct lcdfreq_info *)(fb->fix.reserved[1] << 16 | fb->fix.reserved[0]);
}

static unsigned char get_fimd_divider(struct device *dev)
{
	struct lcdfreq_info *lcdfreq = dev_get_lcdfreq(dev);
	unsigned int reg;

	reg = readl(lcdfreq->fimd_regs + VIDCON0);
	reg &= VIDCON0_CLKVAL_F_MASK;
	reg >>= VIDCON0_CLKVAL_F_SHIFT;

	return reg;
}

static struct clksrc_clk *get_clksrc(struct device *dev)
{
	struct clk *clk;
	struct clksrc_clk *clksrc;

	clk = clk_get(dev, "sclk_fimd");

	clksrc = container_of(clk, struct clksrc_clk, clk);

	return clksrc;
}

static unsigned int get_vstatus(struct device *dev)
{
	struct lcdfreq_info *lcdfreq = dev_get_lcdfreq(dev);
	u32 reg;

	reg = readl(lcdfreq->ielcd_regs + IELCD_VIDCON1);
	reg &= VIDCON1_VSTATUS_MASK;

	return reg;
}

static void reset_div(struct device *dev)
{
	struct lcdfreq_info *lcdfreq = dev_get_lcdfreq(dev);
	struct clksrc_clk *clksrc = lcdfreq->clksrc;
	u32 reg;

	reg = __raw_readl(clksrc->reg_div.reg);
	reg &= ~(0xff);
	reg |= lcdfreq->table[lcdfreq->level].cmu_div;

	writel(reg, clksrc->reg_div.reg);
}

static int get_div(struct device *dev)
{
	struct lcdfreq_info *lcdfreq = dev_get_lcdfreq(dev);
	struct clksrc_clk *clksrc = lcdfreq->clksrc;
	u32 reg = __raw_readl(clksrc->reg_div.reg);
	u32 mask = reg_mask(clksrc->reg_div.shift, clksrc->reg_div.size);

	reg &= mask;
	reg >>= clksrc->reg_div.shift;

	return reg;
}

static int set_div(struct device *dev, u32 div)
{
	struct lcdfreq_info *lcdfreq = dev_get_lcdfreq(dev);
	struct clksrc_clk *clksrc = lcdfreq->clksrc;
	u32 mask = reg_mask(clksrc->reg_div.shift, clksrc->reg_div.size);

	unsigned long flags;
	u32 reg, count = 1000000;

	do {
		spin_lock_irqsave(&lcdfreq->slock, flags);
		reg = __raw_readl(clksrc->reg_div.reg);

		if ((reg & mask) == (div & mask)) {
			spin_unlock_irqrestore(&lcdfreq->slock, flags);
			return -EINVAL;
		}

		reg &= ~(0xff);
		reg |= div;

		if (VSTATUS_IS_ACTIVE(get_vstatus(dev))) {
			if (VSTATUS_IS_FRONT(get_vstatus(dev))) {
				writel(reg, clksrc->reg_div.reg);
				spin_unlock_irqrestore(&lcdfreq->slock, flags);
				dev_info(dev, "%x, %d\n", __raw_readl(clksrc->reg_div.reg), 1000000-count);
				return 0;
			}
		}
		spin_unlock_irqrestore(&lcdfreq->slock, flags);
		count--;
	} while (count);

	dev_err(dev, "%s fail, div=%d\n", __func__, div);

	return -EINVAL;
}

static int get_divider(struct device *dev)
{
	struct fb_info *fb = dev_get_drvdata(dev);
	struct lcdfreq_info *lcdfreq = dev_get_lcdfreq(dev);
	struct clksrc_clk *clksrc;
	struct clk *clk;
	u32 rate, reg;
	u8 fimd_div, i;

	lcdfreq->clksrc = clksrc = get_clksrc(dev->parent);
	clk = clk_get_parent(&clksrc->clk);
	rate = clk_get_rate(clk);

	lcdfreq->table[NORMAL].cmu_div =
		DIV_ROUND_CLOSEST(rate, lcdfreq->table[NORMAL].vclk);

	lcdfreq->table[LIMIT].cmu_div =
		DIV_ROUND_CLOSEST(rate, lcdfreq->table[LIMIT].vclk);

	if (lcdfreq->table[LIMIT].cmu_div > (1 << clksrc->reg_div.size))
		fimd_div = gcd(lcdfreq->table[NORMAL].cmu_div, lcdfreq->table[LIMIT].cmu_div);
	else
		fimd_div = 1;

	dev_info(dev, "%s rate is %d, fimd div=%d\n", clk->name, rate, fimd_div);

	reg = get_fimd_divider(dev) + 1;

	if ((!fimd_div) || (fimd_div > 256) || (fimd_div != reg)) {
		dev_info(dev, "%s skip, fimd div=%d, reg=%d\n", __func__, fimd_div, reg);
		goto err;
	}

	for (i = 0; i < LEVEL_MAX; i++) {
		lcdfreq->table[i].cmu_div /= fimd_div;
		if (lcdfreq->table[i].cmu_div > (1 << clksrc->reg_div.size)) {
			dev_info(fb->dev, "%s skip, cmu div=%d\n", __func__, lcdfreq->table[i].cmu_div);
			goto err;
		}
		dev_info(dev, "%dhz div is %d\n", lcdfreq->table[i].hz, lcdfreq->table[i].cmu_div);
		lcdfreq->table[i].cmu_div--;
	}

	reg = get_div(dev);
	if (lcdfreq->table[NORMAL].cmu_div != reg) {
		dev_info(dev, "%s skip, cmu div=%d, reg=%d\n", __func__, lcdfreq->table[NORMAL].cmu_div, reg);
		goto err;
	}

	for (i = 0; i < LEVEL_MAX; i++) {
		reg = lcdfreq->table[i].cmu_div;
		lcdfreq->table[i].cmu_div = (reg << clksrc->reg_div.size) | reg;
	}

	return 0;

err:
	return -EINVAL;
}

static int set_lcdfreq_div(struct device *dev, enum lcdfreq_level level)
{
	struct lcdfreq_info *lcdfreq = dev_get_lcdfreq(dev);

	u32 ret;

	mutex_lock(&lcdfreq->lock);

	if (!lcdfreq->enable) {
		dev_err(dev, "%s reject. enable flag is %d\n", __func__, lcdfreq->enable);
		ret = -EINVAL;
		goto exit;
	}

	ret = set_div(dev, lcdfreq->table[level].cmu_div);

	if (ret) {
		dev_err(dev, "fail to change lcd freq\n");
		goto exit;
	}

	lcdfreq->level = level;

exit:
	mutex_unlock(&lcdfreq->lock);

	return ret;
}

static int lcdfreq_lock(struct device *dev)
{
	struct lcdfreq_info *lcdfreq = dev_get_lcdfreq(dev);

	int ret;

	if (!atomic_read(&lcdfreq->usage))
		ret = set_lcdfreq_div(dev, LIMIT);
	else {
		dev_err(dev, "lcd freq is already limit state\n");
		return -EINVAL;
	}

	if (!ret) {
		mutex_lock(&lcdfreq->lock);
		atomic_inc(&lcdfreq->usage);
		mutex_unlock(&lcdfreq->lock);
		schedule_delayed_work(&lcdfreq->work, 0);
	}

	return ret;
}

static int lcdfreq_lock_free(struct device *dev)
{
	struct lcdfreq_info *lcdfreq = dev_get_lcdfreq(dev);

	int ret;

	if (atomic_read(&lcdfreq->usage))
		ret = set_lcdfreq_div(dev, NORMAL);
	else {
		dev_err(dev, "lcd freq is already normal state\n");
		return -EINVAL;
	}

	if (!ret) {
		mutex_lock(&lcdfreq->lock);
		atomic_dec(&lcdfreq->usage);
		mutex_unlock(&lcdfreq->lock);
		cancel_delayed_work(&lcdfreq->work);
	}

	return ret;
}

static ssize_t level_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct lcdfreq_info *lcdfreq = dev_get_lcdfreq(dev);

	if (!lcdfreq->enable) {
		dev_err(dev, "%s reject. enable flag is %d\n", __func__, lcdfreq->enable);
		return -EINVAL;
	}

	return sprintf(buf, "%dhz, div=%d\n", lcdfreq->table[lcdfreq->level].hz, get_div(dev));
}

static ssize_t level_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int value;
	int ret;

	ret = strict_strtoul(buf, 0, (unsigned long *)&value);

	dev_info(dev, "%s :: value=%d\n", __func__, value);

	if (value >= LEVEL_MAX)
		return -EINVAL;

	if (value)
		ret = lcdfreq_lock(dev);
	else
		ret = lcdfreq_lock_free(dev);

	if (ret) {
		dev_err(dev, "%s fail\n", __func__);
		return -EINVAL;
	}
	return count;
}

static ssize_t usage_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct lcdfreq_info *lcdfreq = dev_get_lcdfreq(dev);

	return sprintf(buf, "%d\n", atomic_read(&lcdfreq->usage));
}

static DEVICE_ATTR(level, S_IRUGO|S_IWUSR, level_show, level_store);
static DEVICE_ATTR(usage, S_IRUGO, usage_show, NULL);

static struct attribute *lcdfreq_attributes[] = {
	&dev_attr_level.attr,
	&dev_attr_usage.attr,
/*	&dev_attr_freq.attr, */
	NULL,
};

static struct attribute_group lcdfreq_attr_group = {
	.name = "lcdfreq",
	.attrs = lcdfreq_attributes,
};

static int lcdfreq_pm_notifier_event(struct notifier_block *this,
	unsigned long event, void *ptr)
{
	struct lcdfreq_info *lcdfreq =
		container_of(this, struct lcdfreq_info, pm_noti);

	dev_info(lcdfreq->dev, "%s :: event=%ld\n", __func__, event);

	switch (event) {
	case PM_SUSPEND_PREPARE:
		mutex_lock(&lcdfreq->lock);
		lcdfreq->enable = false;
		lcdfreq->level = NORMAL;
		reset_div(lcdfreq->dev);
		atomic_set(&lcdfreq->usage, 0);
		mutex_unlock(&lcdfreq->lock);
		return NOTIFY_OK;
	case PM_POST_RESTORE:
	case PM_POST_SUSPEND:
		mutex_lock(&lcdfreq->lock);
		lcdfreq->enable = true;
		mutex_unlock(&lcdfreq->lock);
		return NOTIFY_OK;
	}
	return NOTIFY_DONE;
}

static int lcdfreq_reboot_notify(struct notifier_block *this,
		unsigned long code, void *unused)
{
	struct lcdfreq_info *lcdfreq =
		container_of(this, struct lcdfreq_info, reboot_noti);

	mutex_lock(&lcdfreq->lock);
	lcdfreq->enable = false;
	lcdfreq->level = NORMAL;
	reset_div(lcdfreq->dev);
	atomic_set(&lcdfreq->usage, 0);
	mutex_unlock(&lcdfreq->lock);

	dev_info(lcdfreq->dev, "%s\n", __func__);

	return NOTIFY_DONE;
}

static void lcdfreq_status_work(struct work_struct *work)
{
	struct lcdfreq_info *lcdfreq =
		container_of(work, struct lcdfreq_info, work.work);

	u32 hz = lcdfreq->table[lcdfreq->level].hz;

	cancel_delayed_work(&lcdfreq->work);

	dev_info(lcdfreq->dev, "hz=%d, usage=%d\n", hz, atomic_read(&lcdfreq->usage));

	schedule_delayed_work(&lcdfreq->work, HZ*120);
}

static int fb_notifier_callback(struct notifier_block *self,
				 unsigned long event, void *data)
{
	struct lcdfreq_info *lcdfreq;
	struct fb_event *evdata = data;
	int fb_blank;

	switch (event) {
	case FB_EVENT_BLANK:
		break;
	default:
		return 0;
	}

	lcdfreq = container_of(self, struct lcdfreq_info, fb_notif);
	if (!lcdfreq)
		return 0;

	fb_blank = *(int *)evdata->data;

	dev_info(lcdfreq->dev, "%s: %d\n", __func__, fb_blank);

	if (fb_blank == FB_BLANK_UNBLANK) {
		mutex_lock(&lcdfreq->lock);
		lcdfreq->enable = true;
		mutex_unlock(&lcdfreq->lock);
	} else if (fb_blank == FB_BLANK_POWERDOWN) {
		mutex_lock(&lcdfreq->lock);
		lcdfreq->enable = false;
		lcdfreq->level = NORMAL;
		reset_div(lcdfreq->dev);
		atomic_set(&lcdfreq->usage, 0);
		mutex_unlock(&lcdfreq->lock);
	}

	return 0;
}

static int lcdfreq_register_fb(struct lcdfreq_info *lcdfreq)
{
	memset(&lcdfreq->fb_notif, 0, sizeof(lcdfreq->fb_notif));
	lcdfreq->fb_notif.notifier_call = fb_notifier_callback;
	return fb_register_client(&lcdfreq->fb_notif);
}

static struct fb_videomode *get_videmode(struct list_head *list)
{
	struct fb_modelist *modelist;
	struct list_head *pos;
	struct fb_videomode *m = NULL;

	if (!list->prev || !list->next || list_empty(list))
		goto exit;

	list_for_each(pos, list) {
		modelist = list_entry(pos, struct fb_modelist, list);
		m = &modelist->mode;
	}

exit:
	return m;
}

int lcdfreq_init(void *fimd, void *ielcd)
{
	struct fb_info *fb = registered_fb[0];
	struct fb_videomode *m;
	struct lcdfreq_info *lcdfreq = NULL;
	u32 vclk, freq_limit = 40;
	int ret = 0;

	m = get_videmode(&fb->modelist);
	if (!m)
		goto err_1;

	lcdfreq = kzalloc(sizeof(struct lcdfreq_info), GFP_KERNEL);
	if (!lcdfreq) {
		pr_err("fail to allocate for lcdfreq\n");
		ret = -ENOMEM;
		goto err_1;
	}

	if (!freq_limit) {
		ret = -EINVAL;
		goto err_2;
	}

	fb->fix.reserved[0] = (u32)lcdfreq;
	fb->fix.reserved[1] = (u32)lcdfreq >> 16;

	lcdfreq->dev = fb->dev;
	lcdfreq->fimd_regs = fimd;
	lcdfreq->ielcd_regs = ielcd;
	lcdfreq->level = NORMAL;

	vclk = (m->left_margin + m->right_margin + m->hsync_len + m->xres) *
		(m->upper_margin + m->lower_margin + m->vsync_len + m->yres);

	lcdfreq->table[NORMAL].level = NORMAL;
	lcdfreq->table[NORMAL].vclk = vclk * m->refresh;
	lcdfreq->table[NORMAL].pixclock = m->pixclock;
	lcdfreq->table[NORMAL].hz = m->refresh;

	lcdfreq->table[LIMIT].level = LIMIT;
	lcdfreq->table[LIMIT].vclk = vclk * freq_limit;
	lcdfreq->table[LIMIT].pixclock = KHZ2PICOS((vclk * freq_limit)/1000);
	lcdfreq->table[LIMIT].hz = freq_limit;

	ret = get_divider(fb->dev);
	if (ret < 0) {
		pr_err("skip %s", __func__);
		fb->fix.reserved[0] = 0;
		fb->fix.reserved[1] = 0;
		goto err_1;
	}

	atomic_set(&lcdfreq->usage, 0);
	mutex_init(&lcdfreq->lock);
	spin_lock_init(&lcdfreq->slock);

	INIT_DELAYED_WORK_DEFERRABLE(&lcdfreq->work, lcdfreq_status_work);

	ret = sysfs_create_group(&fb->dev->kobj, &lcdfreq_attr_group);
	if (ret < 0) {
		pr_err("fail to add sysfs entries, %d\n", __LINE__);
		goto err_2;
	}

	lcdfreq->pm_noti.notifier_call = lcdfreq_pm_notifier_event;
	lcdfreq->reboot_noti.notifier_call = lcdfreq_reboot_notify;

	if (register_reboot_notifier(&lcdfreq->reboot_noti)) {
		pr_err("fail to setup reboot notifier\n");
		goto err_3;
	}

	lcdfreq->enable = true;

	lcdfreq_register_fb(lcdfreq);

	dev_info(lcdfreq->dev, "%s is done\n", __func__);

	return 0;

err_3:
	sysfs_remove_group(&fb->dev->kobj, &lcdfreq_attr_group);

err_2:
	kfree(lcdfreq);

err_1:
	return ret;

}

EXPORT_SYMBOL(lcdfreq_init);

