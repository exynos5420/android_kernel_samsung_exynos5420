/* drivers/gpu/t6xx/kbase/src/platform/mali_kbase_platform.c
 *
 * Copyright 2011 by S.LSI. Samsung Electronics Inc.
 * San#24, Nongseo-Dong, Giheung-Gu, Yongin, Korea
 *
 * Samsung SoC Mali-T604 platform-dependent codes
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software FoundatIon.
 */

/**
 * @file mali_kbase_platform.c
 * Platform-dependent init.
 */
#include <kbase/src/common/mali_kbase.h>
#include <kbase/src/common/mali_kbase_pm.h>
#include <kbase/src/common/mali_kbase_uku.h>
#include <kbase/src/common/mali_kbase_mem.h>
#include <kbase/src/common/mali_midg_regmap.h>
#include <kbase/src/linux/mali_kbase_mem_linux.h>

#include <linux/module.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/platform_device.h>
#include <linux/pci.h>
#include <linux/miscdevice.h>
#include <linux/list.h>
#include <linux/semaphore.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/io.h>

#include <mach/map.h>
#include <linux/fb.h>
#include <linux/clk.h>
#include <mach/regs-clock.h>
#include <mach/pmu.h>
#include <mach/regs-pmu.h>
#include <asm/delay.h>

#include <kbase/src/platform/mali_kbase_platform.h>
#include <kbase/src/platform/mali_kbase_dvfs.h>

#include <kbase/src/common/mali_kbase_gator.h>

#if defined(CONFIG_EXYNOS_THERMAL)
#include <mach/tmu.h>
#define VOLTAGE_OFFSET_MARGIN 37500
#endif

unsigned int gpu_voltage_margin;
bool tmu_on_off;

#define MALI_T6XX_DEFAULT_CLOCK (MALI_DVFS_START_FREQ*MHZ)

static struct clk *clk_g3d = NULL;
static struct clk *clk_ahb2apb_g3dp = NULL;

static int clk_g3d_status = 0;

#if defined(CONFIG_EXYNOS_THERMAL)
static int exynos5_g3d_tmu_notifier(struct notifier_block *notifier,
				unsigned long event, void *v)
{
	int volt_offset = 0;

	if (!tmu_on_off) {
		return NOTIFY_OK;
	}

	if (event == GPU_COLD) {
		volt_offset = VOLTAGE_OFFSET_MARGIN;
	} else if (event == TMU_NORMAL) {
		kbase_tmu_normal_work();
	} else if (event >= GPU_THROTTLING1 && event <= GPU_TRIPPING) {
		if (kbase_tmu_hot_check_and_work(event))
			pr_err("[kbase_tmu_hot_check_and_work] failed to open device");
	}

	kbase_set_power_margin(volt_offset);

	return NOTIFY_OK;
}

static struct notifier_block exynos5_g3d_tmu_nb = {
	.notifier_call = exynos5_g3d_tmu_notifier,
};
#endif

static int kbase_platform_power_clock_init(kbase_device *kbdev)
{
	struct device *dev =  kbdev->osdev.dev;
	struct clk *fout_vpll = NULL, *mout_vpll = NULL, *aclk_g3d_sw = NULL, *aclk_g3d_dout = NULL;
	int timeout, ret;
	struct exynos_context *platform;

	platform = (struct exynos_context *) kbdev->platform_context;
	if (NULL == platform) {
		panic("oops");
	}

	/* Turn on G3D power */
	__raw_writel(0x7, EXYNOS5420_G3D_CONFIGURATION);

	/* Wait for G3D power stability for 1ms */
	timeout = 10;
	while ((__raw_readl(EXYNOS5420_G3D_STATUS) & 0x7) != 0x7) {
		if (timeout == 0) {
			/* need to call panic  */
			panic("failed to turn on g3d power\n");
			goto out;
		}
		timeout--;
		udelay(100);
	}

	/* Turn on G3D clock */
	clk_g3d = clk_get(dev, "g3d");
	if (IS_ERR(clk_g3d)) {
		clk_g3d = NULL;
		KBASE_DEBUG_PRINT_ERROR(KBASE_CORE, "failed to clk_get [g3d]\n");
	} else {
		clk_enable(clk_g3d);
		/* Turn on G3D AHB2APB clock */
		clk_ahb2apb_g3dp = clk_get(dev, "clk_ahb2apb_g3dp");
		if (IS_ERR(clk_ahb2apb_g3dp)) {
			clk_ahb2apb_g3dp = NULL;
			KBASE_DEBUG_PRINT_ERROR(KBASE_CORE, "failed to clk_get [clk_ahb2apb_g3dp]\n");
		} else {
			clk_enable(clk_ahb2apb_g3dp);
			clk_g3d_status = 1;
		}
	}

	fout_vpll = clk_get(dev, "fout_vpll");
	if (IS_ERR(fout_vpll)) {
		KBASE_DEBUG_PRINT_ERROR(KBASE_CORE, "failed to clk_get [fout_vpll]\n");
		goto out;
	}
	mout_vpll = clk_get(dev, "mout_vpll");
	if (IS_ERR(mout_vpll)) {
		KBASE_DEBUG_PRINT_ERROR(KBASE_CORE, "failed to clk_get [mout_vpll]\n");
		goto out;
	}
	aclk_g3d_dout = clk_get(dev, "aclk_g3d_dout");
	if (IS_ERR(aclk_g3d_dout)) {
		KBASE_DEBUG_PRINT_ERROR(KBASE_CORE, "failed to clk_get [aclk_g3d_dout]\n");
		goto out;
	}
	aclk_g3d_sw = clk_get(dev, "aclk_g3d_sw");
	if (IS_ERR(aclk_g3d_sw)) {
		KBASE_DEBUG_PRINT_ERROR(KBASE_CORE, "failed to clk_get [aclk_g3d_sw]\n");
		goto out;
	}
	platform->aclk_g3d = clk_get(dev, "aclk_g3d");
	if (IS_ERR(platform->aclk_g3d)) {
		KBASE_DEBUG_PRINT_ERROR(KBASE_CORE, "failed to clk_get [aclk_g3d]\n");
		goto out;
	}

	ret = clk_set_parent(platform->aclk_g3d, aclk_g3d_sw);
	if (ret < 0) {
		KBASE_DEBUG_PRINT_ERROR(KBASE_CORE, "failed to clk_set_parent [aclk_g3d]\n");
		goto out;
	}
	ret = clk_set_parent(aclk_g3d_sw, aclk_g3d_dout);
	if (ret < 0) {
		KBASE_DEBUG_PRINT_ERROR(KBASE_CORE, "failed to clk_set_parent [aclk_g3d_sw]\n");
		goto out;
	}
	ret = clk_set_parent(aclk_g3d_dout, mout_vpll);
	if (ret < 0) {
		KBASE_DEBUG_PRINT_ERROR(KBASE_CORE, "failed to clk_set_parent [aclk_g3d_dout]\n");
		goto out;
	}

	clk_set_rate(fout_vpll, MALI_T6XX_DEFAULT_CLOCK);
	if (IS_ERR(fout_vpll)) {
		KBASE_DEBUG_PRINT_ERROR(KBASE_CORE, "failed to clk_set_rate [fout_vpll] = %d\n", MALI_T6XX_DEFAULT_CLOCK);
		goto out;
	}
	clk_set_rate(aclk_g3d_dout, MALI_T6XX_DEFAULT_CLOCK);
	if (IS_ERR(aclk_g3d_dout)) {
		KBASE_DEBUG_PRINT_ERROR(KBASE_CORE, "failed to clk_set_rate [aclk_g3d_dout] = %d\n", MALI_T6XX_DEFAULT_CLOCK);
		goto out;
	}

	(void) clk_enable(platform->aclk_g3d);
#if defined(CONFIG_EXYNOS_THERMAL)
	exynos_gpu_add_notifier(&exynos5_g3d_tmu_nb);
	tmu_on_off = true;
#else
	tmu_on_off = false;
#endif
	return 0;
out:
	return -EPERM;
}

int kbase_platform_clock_on(struct kbase_device *kbdev)
{
	struct exynos_context *platform;
	if (!kbdev)
		return -ENODEV;

	platform = (struct exynos_context *) kbdev->platform_context;
	if (!platform)
		return -ENODEV;

	if (clk_g3d_status == 1)
		return 0;

	if (clk_g3d) {
		(void) clk_enable(clk_g3d);
		(void) clk_enable(clk_ahb2apb_g3dp);
		(void) clk_enable(platform->aclk_g3d);
	}
	clk_g3d_status = 1;

	return 0;
}

int kbase_platform_clock_off(struct kbase_device *kbdev)
{
	struct exynos_context *platform;
	if (!kbdev)
		return -ENODEV;

	platform = (struct exynos_context *) kbdev->platform_context;
	if (!platform)
		return -ENODEV;

	if (clk_g3d_status == 0)
		return 0;

	if (clk_g3d) {
		(void)clk_disable(clk_ahb2apb_g3dp);
		(void)clk_disable(clk_g3d);
		(void)clk_disable(platform->aclk_g3d);
	}
	clk_g3d_status = 0;

	return 0;
}

int kbase_platform_is_power_on(void)
{
	return ((__raw_readl(EXYNOS5420_G3D_STATUS) & 0x7) == 0x7) ? 1 : 0;
}

static int kbase_platform_power_on(void)
{
	int timeout;

	/* Turn on G3D  */
	__raw_writel(0x7, EXYNOS5420_G3D_CONFIGURATION);

	/* Wait for G3D power stability */
	timeout = 1000;

	while ((__raw_readl(EXYNOS5420_G3D_STATUS) & 0x7) != 0x7) {
		if (timeout == 0) {
			/* need to call panic  */
			panic("failed to turn on g3d via g3d_configuration\n");
			return -ETIMEDOUT;
		}
		timeout--;
		udelay(10);
	}

	return 0;
}

static int kbase_platform_power_off(void)
{
	int timeout;

	/* Turn off G3D  */
	__raw_writel(0x0, EXYNOS5420_G3D_CONFIGURATION);

	/* Wait for G3D power stability */
	timeout = 1000;

	while (__raw_readl(EXYNOS5420_G3D_STATUS) & 0x7) {
		if (timeout == 0) {
			/* need to call panic */
			panic("failed to turn off g3d via g3d_configuration\n");
			return -ETIMEDOUT;
		}
		timeout--;
		udelay(10);
	}

	return 0;
}

int kbase_platform_cmu_pmu_control(struct kbase_device *kbdev, int control)
{
	unsigned long flags;
	struct exynos_context *platform;
	if (!kbdev) {
		return -ENODEV;
	}

	platform = (struct exynos_context *) kbdev->platform_context;
	if (!platform) {
		return -ENODEV;
	}

	spin_lock_irqsave(&platform->cmu_pmu_lock, flags);

	/* off */
	if (control == 0) {
		if (platform->cmu_pmu_status == 0) {
			spin_unlock_irqrestore(&platform->cmu_pmu_lock, flags);
			return 0;
		}
		if (kbase_platform_power_off())
			panic("failed to turn off g3d power\n");
		if (kbase_platform_clock_off(kbdev))
			panic("failed to turn off aclk_g3d\n");
		platform->cmu_pmu_status = 0;
	} else {
		/* on */
		if (platform->cmu_pmu_status == 1) {
			spin_unlock_irqrestore(&platform->cmu_pmu_lock, flags);
			return 0;
		}
		if (kbase_platform_clock_on(kbdev))
			panic("failed to turn on aclk_g3d\n");
		if (kbase_platform_power_on())
			panic("failed to turn on g3d power\n");
		platform->cmu_pmu_status = 1;
	}
	spin_unlock_irqrestore(&platform->cmu_pmu_lock, flags);

	return 0;
}

#ifdef CONFIG_MALI_T6XX_DEBUG_SYS
static ssize_t show_clock(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct kbase_device *kbdev;
	struct exynos_context *platform;
	ssize_t ret = 0;
	unsigned int clkrate;

	kbdev = dev_get_drvdata(dev);

	if (!kbdev)
		return -ENODEV;

	platform = (struct exynos_context *) kbdev->platform_context;
	if (!platform)
		return -ENODEV;

	if (!platform->aclk_g3d)
		return -ENODEV;

	clkrate = clk_get_rate(platform->aclk_g3d);
	ret += snprintf(buf+ret, PAGE_SIZE-ret, "%d", clkrate/1000000);

	if (ret < PAGE_SIZE - 1) {
		ret += snprintf(buf+ret, PAGE_SIZE-ret, "\n");
	} else {
		buf[PAGE_SIZE-2] = '\n';
		buf[PAGE_SIZE-1] = '\0';
		ret = PAGE_SIZE-1;
	}

	return ret;
}

static ssize_t set_clock(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct kbase_device *kbdev;
	struct exynos_context *platform;
	unsigned int tmp = 0, freq = 0;
	int i, step, ret, clock = 0;
	kbdev = dev_get_drvdata(dev);

	if (!kbdev)
		return -ENODEV;

	platform = (struct exynos_context *) kbdev->platform_context;
	if (!platform)
		return -ENODEV;

	if (!platform->aclk_g3d)
		return -ENODEV;

	step = mali_get_dvfs_step();
	ret = kstrtoint(buf, 0, &clock);
	if (ret) {
		dev_err(dev, "set_clock: invalid value\n");
		return -ENOENT;
	}

	for (i = step-1; i >= 0; i--) {
		if (clock == mali_get_dvfs_clock(i)) {
			freq = clock;
			break;
		}
	}

	if (i == -1) {
		dev_err(dev, "set_clock: invalid value\n");
		return -ENOENT;
	}

	kbase_platform_dvfs_set_level(kbdev, kbase_platform_dvfs_get_level(freq));
	/* Waiting for clock is stable */
	do {
		tmp = __raw_readl(EXYNOS5_CLKDIV_STAT_TOP2);
	} while (tmp & 0x10000);

	return count;
}

static ssize_t show_fbdev(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct kbase_device *kbdev;
	ssize_t ret = 0;
	int i;

	kbdev = dev_get_drvdata(dev);

	if (!kbdev)
		return -ENODEV;

	for (i = 0 ; i < num_registered_fb ; i++) {
		ret += snprintf(buf+ret, PAGE_SIZE-ret, "fb[%d] xres=%d, yres=%d, addr=0x%lx\n", i, registered_fb[i]->var.xres, registered_fb[i]->var.yres, registered_fb[i]->fix.smem_start);
	}

	if (ret < PAGE_SIZE - 1) {
		ret += snprintf(buf+ret, PAGE_SIZE-ret, "\n");
	} else {
		buf[PAGE_SIZE-2] = '\n';
		buf[PAGE_SIZE-1] = '\0';
		ret = PAGE_SIZE-1;
	}

	return ret;
}

typedef enum {
	L1_I_tag_RAM = 0x00,
	L1_I_data_RAM = 0x01,
	L1_I_BTB_RAM = 0x02,
	L1_I_GHB_RAM = 0x03,
	L1_I_TLB_RAM = 0x04,
	L1_I_indirect_predictor_RAM = 0x05,
	L1_D_tag_RAM = 0x08,
	L1_D_data_RAM = 0x09,
	L1_D_load_TLB_array = 0x0A,
	L1_D_store_TLB_array = 0x0B,
	L2_tag_RAM = 0x10,
	L2_data_RAM = 0x11,
	L2_snoop_tag_RAM = 0x12,
	L2_data_ECC_RAM = 0x13,
	L2_dirty_RAM = 0x14,
	L2_TLB_RAM = 0x18
} RAMID_type;

static inline void asm_ramindex_mrc(u32 *DL1Data0, u32 *DL1Data1, u32 *DL1Data2, u32 *DL1Data3)
{
	u32 val;

	if (DL1Data0) {
		asm volatile("mrc p15, 0, %0, c15, c1, 0" : "=r" (val));
		*DL1Data0 = val;
	}
	if (DL1Data1) {
		asm volatile("mrc p15, 0, %0, c15, c1, 1" : "=r" (val));
		*DL1Data1 = val;
	}
	if (DL1Data2) {
		asm volatile("mrc p15, 0, %0, c15, c1, 2" : "=r" (val));
		*DL1Data2 = val;
	}
	if (DL1Data3) {
		asm volatile("mrc p15, 0, %0, c15, c1, 3" : "=r" (val));
		*DL1Data3 = val;
	}
}

static inline void asm_ramindex_mcr(u32 val)
{
	asm volatile("mcr p15, 0, %0, c15, c4, 0" : : "r" (val));
	asm volatile("dsb");
	asm volatile("isb");
}

static void get_tlb_array(u32 val, u32 *DL1Data0, u32 *DL1Data1, u32 *DL1Data2, u32 *DL1Data3)
{
	asm_ramindex_mcr(val);
	asm_ramindex_mrc(DL1Data0, DL1Data1, DL1Data2, DL1Data3);
}

static RAMID_type ramindex = L1_D_load_TLB_array;
static ssize_t show_dtlb(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct kbase_device *kbdev;
	ssize_t ret = 0;
	int entries, ways;
	u32 DL1Data0 = 0, DL1Data1 = 0, DL1Data2 = 0, DL1Data3 = 0;

	kbdev = dev_get_drvdata(dev);

	if (!kbdev)
		return -ENODEV;

	/* L1-I tag RAM */
	if (ramindex == L1_I_tag_RAM) {
		printk("Not implemented yet\n");
	} /* L1-I data RAM */
	else if (ramindex == L1_I_data_RAM) {
		printk("Not implemented yet\n");
	} /* L1-I BTB RAM */
	else if (ramindex == L1_I_BTB_RAM) {
		printk("Not implemented yet\n");
	} /* L1-I GHB RAM */
	else if (ramindex == L1_I_GHB_RAM) {
		printk("Not implemented yet\n");
	} /* L1-I TLB RAM */
	else if (ramindex == L1_I_TLB_RAM) {
		printk("L1-I TLB RAM\n");
		for(entries = 0 ; entries < 32 ; entries++) {
			get_tlb_array((((u8)ramindex) << 24) + entries, &DL1Data0, &DL1Data1, &DL1Data2, NULL);
			printk("entries[%d], DL1Data0=%08x, DL1Data1=%08x DL1Data2=%08x\n", entries, DL1Data0, DL1Data1 & 0xffff, 0x0);
		}
	} /* L1-I indirect predictor RAM */
	else if (ramindex == L1_I_indirect_predictor_RAM) {
		printk("Not implemented yet\n");
	}
	/* L1-D tag RAM */
	else if (ramindex == L1_D_tag_RAM) {
		printk("Not implemented yet\n");
	}
	/* L1-D data RAM */
	else if (ramindex == L1_D_data_RAM) {
		printk("Not implemented yet\n");
	}
	/* L1-D load TLB array */
	else if (ramindex == L1_D_load_TLB_array) {
		printk("L1-D load TLB array\n");
		for (entries = 0 ; entries < 32 ; entries++) {
			get_tlb_array((((u8)ramindex) << 24) + entries, &DL1Data0, &DL1Data1, &DL1Data2, &DL1Data3);
			printk("entries[%d], DL1Data0=%08x, DL1Data1=%08x, DL1Data2=%08x, DL1Data3=%08x\n", entries, DL1Data0, DL1Data1, DL1Data2, DL1Data3 & 0x3f);
		}
	}
	/* L1-D store TLB array */
	else if (ramindex == L1_D_store_TLB_array) {
		printk("\nL1-D store TLB array\n");
		for (entries = 0; entries < 32; entries++) {
			get_tlb_array((((u8)ramindex) << 24) + entries, &DL1Data0, &DL1Data1, &DL1Data2, &DL1Data3);
			printk("entries[%d], DL1Data0=%08x, DL1Data1=%08x, DL1Data2=%08x, DL1Data3=%08x\n", entries, DL1Data0, DL1Data1, DL1Data2, DL1Data3 & 0x3f);
		}
	}
	/* L2 tag RAM */
	else if (ramindex == L2_tag_RAM) {
		printk("Not implemented yet\n");
	}
	/* L2 data RAM */
	else if (ramindex == L2_data_RAM) {
		printk("Not implemented yet\n");
	}
	/* L2 snoop tag RAM */
	else if (ramindex == L2_snoop_tag_RAM) {
		printk("Not implemented yet\n");
	}
	/* L2 data ECC RAM */
	else if (ramindex == L2_data_ECC_RAM) {
		printk("Not implemented yet\n");
	}
	/* L2 dirty RAM */
	else if (ramindex == L2_dirty_RAM) {
		printk("Not implemented yet\n");
	}
	/* L2 TLB array */
	else if (ramindex == L2_TLB_RAM) {
		printk("\nL2 TLB array\n");
		for (ways = 0 ; ways < 4 ; ways++) {
			for (entries = 0 ; entries < 512 ; entries++) {
				get_tlb_array((ramindex << 24) + (ways << 18) + entries, &DL1Data0, &DL1Data1, &DL1Data2, &DL1Data3);
				printk("ways[%d]:entries[%d], DL1Data0=%08x, DL1Data1=%08x, DL1Data2=%08x, DL1Data3=%08x\n", ways, entries, DL1Data0, DL1Data1, DL1Data2, DL1Data3);
			}
		}
	} else {
	}

	ret += snprintf(buf+ret, PAGE_SIZE-ret, "Succeeded...\n");

	if (ret < PAGE_SIZE - 1) {
		ret += snprintf(buf+ret, PAGE_SIZE-ret, "\n");
	} else {
		buf[PAGE_SIZE-2] = '\n';
		buf[PAGE_SIZE-1] = '\0';
		ret = PAGE_SIZE-1;
	}
	return ret;
}

static ssize_t set_dtlb(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct kbase_device *kbdev;
	kbdev = dev_get_drvdata(dev);

	if (!kbdev)
		return -ENODEV;

	if (sysfs_streq("L1_I_tag_RAM", buf)) {
		ramindex = L1_I_tag_RAM;
	} else if (sysfs_streq("L1_I_data_RAM", buf)) {
		ramindex = L1_I_data_RAM;
	} else if (sysfs_streq("L1_I_BTB_RAM", buf)) {
		ramindex = L1_I_BTB_RAM;
	} else if (sysfs_streq("L1_I_GHB_RAM", buf)) {
		ramindex = L1_I_GHB_RAM;
	} else if (sysfs_streq("L1_I_TLB_RAM", buf)) {
		ramindex = L1_I_TLB_RAM;
	} else if (sysfs_streq("L1_I_indirect_predictor_RAM", buf)) {
		ramindex = L1_I_indirect_predictor_RAM;
	} else if (sysfs_streq("L1_D_tag_RAM", buf)) {
		ramindex = L1_D_tag_RAM;
	} else if (sysfs_streq("L1_D_data_RAM", buf)) {
		ramindex = L1_D_data_RAM;
	} else if (sysfs_streq("L1_D_load_TLB_array", buf)) {
		ramindex = L1_D_load_TLB_array;
	} else if (sysfs_streq("L1_D_store_TLB_array", buf)) {
		ramindex = L1_D_store_TLB_array;
	} else if (sysfs_streq("L2_tag_RAM", buf)) {
		ramindex = L2_tag_RAM;
	} else if (sysfs_streq("L2_data_RAM", buf)) {
		ramindex = L2_data_RAM;
	} else if (sysfs_streq("L2_snoop_tag_RAM", buf)) {
		ramindex = L2_snoop_tag_RAM;
	} else if (sysfs_streq("L2_data_ECC_RAM", buf)) {
		ramindex = L2_data_ECC_RAM;
	} else if (sysfs_streq("L2_dirty_RAM", buf)) {
		ramindex = L2_dirty_RAM;
	} else if (sysfs_streq("L2_TLB_RAM", buf)) {
		ramindex = L2_TLB_RAM;
	} else {
		printk("Invalid value....\n\n");
		printk("Available options are one of below\n");
		printk("L1_I_tag_RAM, L1_I_data_RAM, L1_I_BTB_RAM\n");
		printk("L1_I_GHB_RAM, L1_I_TLB_RAM, L1_I_indirect_predictor_RAM\n");
		printk("L1_D_tag_RAM, L1_D_data_RAM, L1_D_load_TLB_array, L1_D_store_TLB_array\n");
		printk("L2_tag_RAM, L2_data_RAM, L2_snoop_tag_RAM, L2_data_ECC_RAM\n");
		printk("L2_dirty_RAM, L2_TLB_RAM\n");
	}

	return count;
}

static ssize_t show_vol(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct kbase_device *kbdev;
	ssize_t ret = 0;
	int vol;

	kbdev = dev_get_drvdata(dev);

	if (!kbdev)
		return -ENODEV;

	kbase_platform_get_voltage(dev, &vol);
	ret += snprintf(buf+ret, PAGE_SIZE-ret, "%d", vol);

	if (ret < PAGE_SIZE - 1) {
		ret += snprintf(buf+ret, PAGE_SIZE-ret, "\n");
	} else {
		buf[PAGE_SIZE-2] = '\n';
		buf[PAGE_SIZE-1] = '\0';
		ret = PAGE_SIZE-1;
	}

	return ret;
}

static ssize_t show_utilization(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct kbase_device *kbdev;
	ssize_t ret = 0;

	kbdev = dev_get_drvdata(dev);

	if (!kbdev)
		return -ENODEV;

#ifdef CONFIG_MALI_T6XX_DVFS
	ret += snprintf(buf+ret, PAGE_SIZE-ret, "%d", kbase_platform_dvfs_get_utilisation());
#else
	ret += snprintf(buf+ret, PAGE_SIZE-ret, "-1");
#endif

	if (ret < PAGE_SIZE - 1) {
		ret += snprintf(buf+ret, PAGE_SIZE-ret, "\n");
	} else {
		buf[PAGE_SIZE-2] = '\n';
		buf[PAGE_SIZE-1] = '\0';
		ret = PAGE_SIZE-1;
	}

	return ret;
}

static ssize_t show_dvfs(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct kbase_device *kbdev;
	ssize_t ret = 0;

	kbdev = dev_get_drvdata(dev);

	if (!kbdev)
		return -ENODEV;

#ifdef CONFIG_MALI_T6XX_DVFS
	if (kbase_platform_dvfs_get_enable_status())
		ret += snprintf(buf+ret, PAGE_SIZE-ret, "1");
	else
		ret += snprintf(buf+ret, PAGE_SIZE-ret, "0");
#else
	ret += snprintf(buf+ret, PAGE_SIZE-ret, "-1");
#endif

	if (ret < PAGE_SIZE - 1) {
		ret += snprintf(buf+ret, PAGE_SIZE-ret, "\n");
	} else {
		buf[PAGE_SIZE-2] = '\n';
		buf[PAGE_SIZE-1] = '\0';
		ret = PAGE_SIZE-1;
	}

	return ret;
}

static ssize_t set_dvfs(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct kbase_device *kbdev;
	kbdev = dev_get_drvdata(dev);

	if (!kbdev)
		return -ENODEV;

#ifdef CONFIG_MALI_T6XX_DVFS
	if (sysfs_streq("0", buf)) {
		kbase_platform_dvfs_enable(false, MALI_DVFS_BL_CONFIG_FREQ);
	} else if (sysfs_streq("1", buf)) {
		kbase_platform_dvfs_enable(true, MALI_DVFS_START_FREQ);
	}
#else
	printk("-1\n");
#endif
	return count;
}

static ssize_t show_dvfs_table(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct kbase_device *kbdev;
	ssize_t ret = 0;

	kbdev = dev_get_drvdata(dev);

	if (!kbdev)
		return -ENODEV;

#ifdef CONFIG_MALI_T6XX_DVFS
	ret += mali_get_dvfs_table(buf+ret, (size_t)PAGE_SIZE-ret);
#else
	ret += snprintf(buf+ret, PAGE_SIZE-ret, "-1");
#endif

	if (ret < PAGE_SIZE - 1) {
		ret += snprintf(buf+ret, PAGE_SIZE-ret, "\n");
	} else {
		buf[PAGE_SIZE-2] = '\n';
		buf[PAGE_SIZE-1] = '\0';
		ret = PAGE_SIZE-1;
	}

	return ret;
}

static ssize_t show_max_lock_dvfs(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct kbase_device *kbdev;
	ssize_t ret = 0;
#ifdef CONFIG_MALI_T6XX_DVFS
	int locked_level = -1;
#endif

	kbdev = dev_get_drvdata(dev);

	if (!kbdev)
		return -ENODEV;

#ifdef CONFIG_MALI_T6XX_DVFS
	locked_level = mali_get_dvfs_max_locked_freq();
	if (locked_level > 0)
		ret += snprintf(buf+ret, PAGE_SIZE-ret, "%d", locked_level);
	else
		ret += snprintf(buf+ret, PAGE_SIZE-ret, "-1");
#else
	ret += snprintf(buf+ret, PAGE_SIZE-ret, "-1");
#endif

	if (ret < PAGE_SIZE - 1) {
		ret += snprintf(buf+ret, PAGE_SIZE-ret, "\n");
	} else {
		buf[PAGE_SIZE-2] = '\n';
		buf[PAGE_SIZE-1] = '\0';
		ret = PAGE_SIZE-1;
	}

	return ret;
}

static ssize_t set_max_lock_dvfs(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct kbase_device *kbdev;
	int i, step, ret, clock = 0;
	kbdev = dev_get_drvdata(dev);

	if (!kbdev)
		return -ENODEV;

#ifdef CONFIG_MALI_T6XX_DVFS
	if (sysfs_streq("0", buf)) {
		mali_dvfs_freq_max_unlock(SYSFS_LOCK);
	} else {
		step = mali_get_dvfs_step();
		ret = kstrtoint(buf, 0, &clock);
		if (ret) {
			dev_err(dev, "set_clock: invalid value\n");
			return -ENOENT;
		}

		if (clock == mali_get_dvfs_clock(step-1))
			mali_dvfs_freq_max_unlock(SYSFS_LOCK);

		for (i = step-2; i >= 0; i--) {
			if (clock == mali_get_dvfs_clock(i)) {
				mali_dvfs_freq_max_lock(i, SYSFS_LOCK);
				break;
			}
		}

		if (i == -1) {
			dev_err(dev, "set_clock: invalid value\n");
			return -ENOENT;
		}
	}
#else /* CONFIG_MALI_T6XX_DVFS */
	printk("G3D DVFS is disabled. You can not set\n");
#endif
	return count;
}

static ssize_t show_min_lock_dvfs(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct kbase_device *kbdev;
	ssize_t ret = 0;
#ifdef CONFIG_MALI_T6XX_DVFS
	int locked_level = -1;
#endif

	kbdev = dev_get_drvdata(dev);

	if (!kbdev)
		return -ENODEV;

#ifdef CONFIG_MALI_T6XX_DVFS
	locked_level = mali_get_dvfs_min_locked_freq();
	if (locked_level > 0)
		ret += snprintf(buf+ret, PAGE_SIZE-ret, "%d", locked_level);
	else
		ret += snprintf(buf+ret, PAGE_SIZE-ret, "-1");
#else
	ret += snprintf(buf+ret, PAGE_SIZE-ret, "-1");
#endif

	if (ret < PAGE_SIZE - 1) {
		ret += snprintf(buf+ret, PAGE_SIZE-ret, "\n");
	} else {
		buf[PAGE_SIZE-2] = '\n';
		buf[PAGE_SIZE-1] = '\0';
		ret = PAGE_SIZE-1;
	}

	return ret;
}

static ssize_t set_min_lock_dvfs(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	struct kbase_device *kbdev;
	int i, step, ret, clock = 0;
	kbdev = dev_get_drvdata(dev);

	if (!kbdev)
		return -ENODEV;

#ifdef CONFIG_MALI_T6XX_DVFS
	if (sysfs_streq("0", buf)) {
		mali_dvfs_freq_min_unlock(SYSFS_LOCK);
	} else {
		step = mali_get_dvfs_step();
		ret = kstrtoint(buf, 0, &clock);
		if (ret) {
			dev_err(dev, "set_clock: invalid value\n");
			return -ENOENT;
		}

		if (clock == mali_get_dvfs_clock(0))
			mali_dvfs_freq_min_unlock(SYSFS_LOCK);

		for (i = 1; i < step; i++) {
			if (clock == mali_get_dvfs_clock(i)) {
				mali_dvfs_freq_min_lock(i, SYSFS_LOCK);
				break;
			}
		}

		if (i == step) {
			dev_err(dev, "set_clock: invalid value\n");
			return -ENOENT;
		}
	}
#else /* CONFIG_MALI_T6XX_DVFS */
	printk("G3D DVFS is disabled. You can not set\n");
#endif

	return count;
}

static ssize_t show_asv(struct device *dev, struct device_attribute *attr, char *buf)
{

	struct kbase_device *kbdev;
	ssize_t ret = 0;

	kbdev = dev_get_drvdata(dev);

	if (!kbdev)
		return -ENODEV;

	ret = kbase_platform_dvfs_sprint_avs_table(buf, (size_t)PAGE_SIZE);

	return ret;
}
static ssize_t set_asv(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	if (sysfs_streq("0", buf)) {
		kbase_platform_dvfs_set(0);
	} else if (sysfs_streq("1", buf)) {
		kbase_platform_dvfs_set(1);
	} else {
		printk("invalid val -only [0 or 1] is accepted\n");
	}
	return count;
}

static ssize_t show_tmu(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;

	if(tmu_on_off)
		ret += snprintf(buf+ret, PAGE_SIZE-ret, "1");
	else
		ret += snprintf(buf+ret, PAGE_SIZE-ret, "0");

	if (ret < PAGE_SIZE - 1) {
		ret += snprintf(buf+ret, PAGE_SIZE-ret, "\n");
	} else {
		buf[PAGE_SIZE-2] = '\n';
		buf[PAGE_SIZE-1] = '\0';
		ret = PAGE_SIZE-1;
	}

	return ret;
}

static ssize_t set_tmu_control(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	if (sysfs_streq("0", buf)) {
		if (gpu_voltage_margin != 0)
			kbase_set_power_margin(0);
		mali_dvfs_freq_max_unlock(TMU_LOCK);
		tmu_on_off = false;
	}
	else if (sysfs_streq("1", buf))
		tmu_on_off = true;
	else
		printk("invalid val -only [0 or 1] is accepted\n");

	return count;
}

static ssize_t show_power_state(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;

	if((u32)kbase_platform_is_power_on())
		ret += snprintf(buf+ret, PAGE_SIZE-ret, "1");
	else
		ret += snprintf(buf+ret, PAGE_SIZE-ret, "0");

	if (ret < PAGE_SIZE - 1) {
		ret += snprintf(buf+ret, PAGE_SIZE-ret, "\n");
	} else {
		buf[PAGE_SIZE-2] = '\n';
		buf[PAGE_SIZE-1] = '\0';
		ret = PAGE_SIZE-1;
	}

	return ret;
}

/** The sysfs file @c clock, fbdev.
 *
 * This is used for obtaining information about the mali t6xx operating clock & framebuffer address,
 */
DEVICE_ATTR(clock, S_IRUGO|S_IWUSR, show_clock, set_clock);
DEVICE_ATTR(fbdev, S_IRUGO, show_fbdev, NULL);
DEVICE_ATTR(dtlb, S_IRUGO|S_IWUSR, show_dtlb, set_dtlb);
DEVICE_ATTR(vol, S_IRUGO|S_IWUSR, show_vol, NULL);
DEVICE_ATTR(dvfs, S_IRUGO|S_IWUSR, show_dvfs, set_dvfs);
DEVICE_ATTR(dvfs_max_lock, S_IRUGO|S_IWUSR, show_max_lock_dvfs, set_max_lock_dvfs);
DEVICE_ATTR(dvfs_min_lock, S_IRUGO|S_IWUSR, show_min_lock_dvfs, set_min_lock_dvfs);
DEVICE_ATTR(asv, S_IRUGO|S_IWUSR, show_asv, set_asv);
DEVICE_ATTR(time_in_state, S_IRUGO|S_IWUSR, show_time_in_state, set_time_in_state);
DEVICE_ATTR(tmu, S_IRUGO|S_IWUSR, show_tmu, set_tmu_control);
DEVICE_ATTR(utilization, S_IRUGO|S_IWUSR, show_utilization, NULL);
DEVICE_ATTR(dvfs_table, S_IRUGO|S_IWUSR, show_dvfs_table, NULL);
DEVICE_ATTR(power_state, S_IRUGO|S_IWUSR, show_power_state, NULL);

int kbase_platform_create_sysfs_file(struct device *dev)
{
	if (device_create_file(dev, &dev_attr_clock)) {
		dev_err(dev, "Couldn't create sysfs file [clock]\n");
		goto out;
	}

	if (device_create_file(dev, &dev_attr_fbdev)) {
		dev_err(dev, "Couldn't create sysfs file [fbdev]\n");
		goto out;
	}

	if (device_create_file(dev, &dev_attr_dtlb)) {
		dev_err(dev, "Couldn't create sysfs file [dtlb]\n");
		goto out;
	}

	if (device_create_file(dev, &dev_attr_vol)) {
		dev_err(dev, "Couldn't create sysfs file [vol]\n");
		goto out;
	}

	if (device_create_file(dev, &dev_attr_dvfs)) {
		dev_err(dev, "Couldn't create sysfs file [dvfs]\n");
		goto out;
	}

	if (device_create_file(dev, &dev_attr_dvfs_max_lock)) {
		dev_err(dev, "Couldn't create sysfs file [dvfs_max_lock]\n");
		goto out;
	}

	if (device_create_file(dev, &dev_attr_dvfs_min_lock)) {
		dev_err(dev, "Couldn't create sysfs file [dvfs_min_lock]\n");
		goto out;
	}

	if (device_create_file(dev, &dev_attr_asv)) {
		dev_err(dev, "Couldn't create sysfs file [asv]\n");
		goto out;
	}

	if (device_create_file(dev, &dev_attr_time_in_state)) {
		dev_err(dev, "Couldn't create sysfs file [time_in_state]\n");
		goto out;
	}

	if (device_create_file(dev, &dev_attr_tmu)) {
		dev_err(dev, "Couldn't create sysfs file [tmu]\n");
		goto out;
	}

	if (device_create_file(dev, &dev_attr_utilization)) {
		dev_err(dev, "Couldn't create sysfs file [utilization]\n");
		goto out;
	}

	if (device_create_file(dev, &dev_attr_dvfs_table)) {
		dev_err(dev, "Couldn't create sysfs file [dvfs_table]\n");
		goto out;
	}

	if (device_create_file(dev, &dev_attr_power_state)) {
		dev_err(dev, "Couldn't create sysfs file [power_state]\n");
		goto out;
	}

	return 0;
out:
	return -ENOENT;
}

void kbase_platform_remove_sysfs_file(struct device *dev)
{
	device_remove_file(dev, &dev_attr_clock);
	device_remove_file(dev, &dev_attr_fbdev);
	device_remove_file(dev, &dev_attr_dtlb);
	device_remove_file(dev, &dev_attr_vol);
	device_remove_file(dev, &dev_attr_dvfs);
	device_remove_file(dev, &dev_attr_dvfs_max_lock);
	device_remove_file(dev, &dev_attr_dvfs_min_lock);
	device_remove_file(dev, &dev_attr_asv);
	device_remove_file(dev, &dev_attr_time_in_state);
	device_remove_file(dev, &dev_attr_tmu);
	device_remove_file(dev, &dev_attr_utilization);
	device_remove_file(dev, &dev_attr_dvfs_table);
	device_remove_file(dev, &dev_attr_power_state);
}
#endif /* CONFIG_MALI_T6XX_DEBUG_SYS */

mali_error kbase_platform_init(struct kbase_device *kbdev)
{
	struct exynos_context *platform;

	platform = kmalloc(sizeof(struct exynos_context), GFP_KERNEL);

	if (NULL == platform) {
		return MALI_ERROR_OUT_OF_MEMORY;
	}

	kbdev->platform_context = (void *) platform;

	platform->cmu_pmu_status = 0;
#ifdef CONFIG_MALI_T6XX_DVFS
	platform->utilisation = 0;
	platform->time_busy = 0;
	platform->time_idle = 0;
	platform->time_tick = 0;
#endif

	spin_lock_init(&platform->cmu_pmu_lock);

	if (kbase_platform_power_clock_init(kbdev)) {
		goto clock_init_fail;
	}

#ifdef CONFIG_REGULATOR
	if (kbase_platform_regulator_init()) {
		goto regulator_init_fail;
	}
#endif /* CONFIG_REGULATOR */

#ifdef CONFIG_MALI_T6XX_DVFS
	kbase_platform_dvfs_init(kbdev);
#endif /* CONFIG_MALI_T6XX_DVFS */
#ifdef CONFIG_MALI_DEVFREQ
	mali_devfreq_add(kbdev);
#endif
	/* Enable power */
	kbase_platform_cmu_pmu_control(kbdev, 1);
#if defined(CONFIG_EXYNOS_THERMAL)
	gpu_voltage_margin = 0;
#endif
	return MALI_ERROR_NONE;

regulator_init_fail:
clock_init_fail:
#ifdef CONFIG_REGULATOR
	kbase_platform_regulator_disable();
#endif /* CONFIG_REGULATOR */
	kfree(platform);

	return MALI_ERROR_FUNCTION_FAILED;
}

void kbase_platform_term(kbase_device *kbdev)
{
	struct exynos_context *platform;

	platform = (struct exynos_context *) kbdev->platform_context;

#ifdef CONFIG_MALI_T6XX_DVFS
	kbase_platform_dvfs_term();
#endif /* CONFIG_MALI_T6XX_DVFS */
#ifdef CONFIG_MALI_DEVFREQ
	mali_devfreq_remove();
#endif
	/* Disable power */
	kbase_platform_cmu_pmu_control(kbdev, 0);
#ifdef CONFIG_REGULATOR
	kbase_platform_regulator_disable();
#endif /* CONFIG_REGULATOR */
	kfree(kbdev->platform_context);
	kbdev->platform_context = 0;

	return;
}
