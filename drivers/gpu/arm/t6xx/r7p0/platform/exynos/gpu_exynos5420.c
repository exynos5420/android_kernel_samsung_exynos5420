/* drivers/gpu/arm/.../platform/gpu_exynos5433.c
 *
 * Copyright 2011 by S.LSI. Samsung Electronics Inc.
 * San#24, Nongseo-Dong, Giheung-Gu, Yongin, Korea
 *
 * Samsung SoC Mali-T Series DVFS driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software FoundatIon.
 */

/**
 * @file gpu_exynos5433.c
 * DVFS
 */

#include <mali_kbase.h>

#include <linux/regulator/driver.h>
#include <linux/pm_qos.h>
#include <linux/mfd/samsung/core.h>

#include <mach/asv-exynos.h>
#include <mach/pm_domains.h>
#if (defined(CONFIG_EXYNOS5433_BTS) || defined(CONFIG_EXYNOS5420_BTS))
#include <mach/bts.h>
#endif /* CONFIG_EXYNOS5433_BTS || CONFIG_EXYNOS5420_BTS */

#include "mali_kbase_platform.h"
#include "gpu_dvfs_handler.h"
#include "gpu_dvfs_governor.h"
#include "gpu_control.h"

extern struct kbase_device *pkbdev;
#define EXYNOS5420_G3D_STATUS               EXYNOS5410_G3D_STATUS

#define CPU_MAX PM_QOS_CPU_FREQ_MAX_DEFAULT_VALUE
#define G3D_RBB_VALUE	0x8d

#define GPU_OSC_CLK	24000

/*  clk,vol,abb,min,max,down stay,time_in_state,pm_qos mem,pm_qos int,pm_qos cpu_kfc_min,pm_qos cpu_egl_max */
static gpu_dvfs_info gpu_dvfs_table_default[] = {
#if !defined(CONFIG_ARM_EXYNOS_MP_CPUFREQ)
	{533, 1037500, 0, 99, 100, 1, 0, 800000, 400000, 1200000, CPU_MAX},
	{480, 1000000, 0, 98, 100, 1, 0, 800000, 400000,  650000, CPU_MAX},
	{420,  962500, 0, 78,  99, 1, 0, 800000, 400000,  250000, CPU_MAX},
	{350,  912500, 0, 70,  90, 1, 0, 667000, 333000,  250000, CPU_MAX},
	{266,  862500, 0, 60,  90, 1, 0, 400000, 222000,  250000, CPU_MAX},
#ifdef CONFIG_SUPPORT_WQXGA
	{177,  812500, 0,  0,  90, 1, 0, 160000,  83000,  250000, CPU_MAX},
#else
	{177,  812500, 0, 53,  90, 1, 0, 160000,  83000,  250000, CPU_MAX},
	{100,  812500, 0,  0,  90, 3, 0, 160000,  83000,  250000, CPU_MAX},
#endif
#else
	{533, 1037500, 0, 99, 100, 1, 0, 800000, 400000, 1200000, CPU_MAX, 1300000},
	{480, 1000000, 0, 98, 100, 1, 0, 800000, 400000,  650000, CPU_MAX, 1300000},
	{420,  962500, 0, 78,  99, 1, 0, 800000, 400000,  250000, CPU_MAX, 500000},
	{350,  912500, 0, 70,  90, 1, 0, 667000, 333000,  250000, CPU_MAX, 500000},
	{266,  862500, 0, 60,  90, 1, 0, 400000, 222000,  250000, CPU_MAX, 500000},
#ifdef CONFIG_SUPPORT_WQXGA
	{177,  812500, 0,  0,  90, 1, 0, 160000,  83000,  250000, CPU_MAX, 500000},
#else
	{177,  812500, 0, 53,  90, 1, 0, 160000,  83000,  250000, CPU_MAX, 500000},
	{100,  812500, 0,  0,  90, 3, 0, 160000,  83000,  250000, CPU_MAX, 500000},
#endif
#endif
};

static int mif_min_table[] = {
	 160000,  200000,  266000,
	 400000,  533000,  667000,
	 733000,  800000,
};

//static int available_max_clock[] = {GPU_L2, GPU_L2, GPU_L0, GPU_L0, GPU_L0};

static gpu_attribute gpu_config_attributes[] = {
	{GPU_MAX_CLOCK, 480},
	{GPU_MAX_CLOCK_LIMIT, 480},
#ifdef CONFIG_SUPPORT_WQXGA
	{GPU_MIN_CLOCK, 177},
	{GPU_DVFS_START_CLOCK, 266},
#else
	{GPU_MIN_CLOCK, 100},
	{GPU_DVFS_START_CLOCK, 177},
#endif
	{GPU_DVFS_BL_CONFIG_CLOCK, 350},
	{GPU_GOVERNOR_TYPE, G3D_DVFS_GOVERNOR_DEFAULT},
	{GPU_GOVERNOR_START_CLOCK_DEFAULT, 266},
	{GPU_GOVERNOR_START_CLOCK_INTERACTIVE, 266},
	{GPU_GOVERNOR_START_CLOCK_STATIC, 266},
	{GPU_GOVERNOR_START_CLOCK_BOOSTER, 266},
	{GPU_GOVERNOR_TABLE_DEFAULT, (uintptr_t)&gpu_dvfs_table_default},
	{GPU_GOVERNOR_TABLE_INTERACTIVE, (uintptr_t)&gpu_dvfs_table_default},
	{GPU_GOVERNOR_TABLE_STATIC, (uintptr_t)&gpu_dvfs_table_default},
	{GPU_GOVERNOR_TABLE_BOOSTER, (uintptr_t)&gpu_dvfs_table_default},
	{GPU_GOVERNOR_TABLE_SIZE_DEFAULT, GPU_DVFS_TABLE_LIST_SIZE(gpu_dvfs_table_default)},
	{GPU_GOVERNOR_TABLE_SIZE_INTERACTIVE, GPU_DVFS_TABLE_LIST_SIZE(gpu_dvfs_table_default)},
	{GPU_GOVERNOR_TABLE_SIZE_STATIC, GPU_DVFS_TABLE_LIST_SIZE(gpu_dvfs_table_default)},
	{GPU_GOVERNOR_TABLE_SIZE_BOOSTER, GPU_DVFS_TABLE_LIST_SIZE(gpu_dvfs_table_default)},
	{GPU_GOVERNOR_INTERACTIVE_HIGHSPEED_CLOCK, 420},
	{GPU_GOVERNOR_INTERACTIVE_HIGHSPEED_LOAD, 95},
	{GPU_GOVERNOR_INTERACTIVE_HIGHSPEED_DELAY, 0},
	{GPU_DEFAULT_VOLTAGE, 937500},
	{GPU_COLD_MINIMUM_VOL, 0},
	{GPU_VOLTAGE_OFFSET_MARGIN, 37500},
	{GPU_TMU_CONTROL, 1},
	{GPU_TEMP_THROTTLING1, 420},
	{GPU_TEMP_THROTTLING2, 350},
	{GPU_TEMP_THROTTLING3, 266},
	{GPU_TEMP_THROTTLING4, 160},
	{GPU_TEMP_TRIPPING, 160},
	{GPU_BOOST_MIN_LOCK, 0},
	{GPU_BOOST_EGL_MIN_LOCK, 1300000},
	{GPU_POWER_COEFF, 46}, /* all core on param */
	{GPU_DVFS_TIME_INTERVAL, 5},
	{GPU_DEFAULT_WAKEUP_LOCK, 1},
	{GPU_BUS_DEVFREQ, 1},
	{GPU_DYNAMIC_ABB, 0},
	{GPU_EARLY_CLK_GATING, 0},
	{GPU_DVS, 0},
	{GPU_PERF_GATHERING, 0},
#ifdef MALI_SEC_HWCNT
	{GPU_HWCNT_GATHERING, 1},
	{GPU_HWCNT_POLLING_TIME, 90},
	{GPU_HWCNT_UP_STEP, 3},
	{GPU_HWCNT_DOWN_STEP, 2},
	{GPU_HWCNT_GPR, 1},
	{GPU_HWCNT_DUMP_PERIOD, 50}, /* ms */
	{GPU_HWCNT_CHOOSE_JM , 0},
	{GPU_HWCNT_CHOOSE_SHADER , 0x560},
	{GPU_HWCNT_CHOOSE_TILER , 0},
	{GPU_HWCNT_CHOOSE_L3_CACHE , 0},
	{GPU_HWCNT_CHOOSE_MMU_L2 , 0},
#endif
	{GPU_RUNTIME_PM_DELAY_TIME, 50},
	{GPU_DVFS_POLLING_TIME, 100},
	{GPU_PMQOS_INT_DISABLE, 0},
	{GPU_PMQOS_MIF_MAX_CLOCK, 0},
	{GPU_PMQOS_MIF_MAX_CLOCK_BASE, 0},
	{GPU_CL_DVFS_START_BASE, 0},
	{GPU_DEBUG_LEVEL, DVFS_WARNING},
	{GPU_TRACE_LEVEL, TRACE_ALL},
#ifdef CONFIG_MALI_DVFS_USER
	{GPU_UDVFS_ENABLE, 0},
#endif
};

int gpu_dvfs_decide_max_clock(struct exynos_context *platform)
{
	//unused
	return 0;
}

#ifdef CONFIG_MALI_DVFS_USER
unsigned int gpu_get_config_attr_size(void)
{
	return sizeof(gpu_config_attributes);
}
#endif

void *gpu_get_config_attributes(void)
{
	return &gpu_config_attributes;
}

uintptr_t gpu_get_max_freq(void)
{
	return gpu_get_attrib_data(gpu_config_attributes, GPU_MAX_CLOCK) * 1000;
}

uintptr_t gpu_get_min_freq(void)
{
	return gpu_get_attrib_data(gpu_config_attributes, GPU_MIN_CLOCK) * 1000;
}

struct clk *ext_xtal;
struct clk *fout_vpll;
struct clk *aclk_g3d;
struct clk *mout_vpll;
struct clk *aclk_g3d_dout;
struct clk *clk_g3d = NULL;
struct clk *clk_ahb2apb_g3dp = NULL;

#ifdef CONFIG_REGULATOR
struct regulator *g3d_regulator;
#endif /* CONFIG_REGULATOR */

int gpu_is_power_on(void)
{
	return ((__raw_readl(EXYNOS5420_G3D_STATUS) & 0x7) == 0x7) ? 1 : 0;
}

int gpu_power_init(struct kbase_device *kbdev)
{
	struct exynos_context *platform = (struct exynos_context *) kbdev->platform_context;

	if (!platform)
		return -ENODEV;

	GPU_LOG(DVFS_INFO, DUMMY, 0u, 0u, "power initialized\n");

	return 0;
}

int gpu_get_cur_clock(struct exynos_context *platform)
{
	if (!platform)
		return -ENODEV;

	if (!aclk_g3d) {
		GPU_LOG(DVFS_ERROR, DUMMY, 0u, 0u, "%s: clock is not initialized\n", __func__);
		return -1;
	}

	return clk_get_rate(aclk_g3d)/MHZ;
}

int gpu_is_clock_on(void)
{
	//unused
	return 0;
}

static int gpu_clock_on(struct exynos_context *platform)
{
	int ret = 0;
	if (!platform)
		return -ENODEV;

	if (!gpu_is_power_on()) {
		GPU_LOG(DVFS_WARNING, DUMMY, 0u, 0u, "%s: can't set clock on in power off status\n", __func__);
		ret = -1;
		goto err_return;
	}

	if (platform->clk_g3d_status == 1) {
		ret = 0;
		goto err_return;
	}

	if (aclk_g3d) {
		(void) clk_enable(clk_g3d);
		(void) clk_enable(clk_ahb2apb_g3dp);
		(void) clk_enable(aclk_g3d);
		GPU_LOG(DVFS_DEBUG, LSI_CLOCK_ON, 0u, 0u, "clock is enabled\n");
	}

	platform->clk_g3d_status = 1;

err_return:
	return ret;
}

static int gpu_clock_off(struct exynos_context *platform)
{
	int ret = 0;

	if (!platform)
		return -ENODEV;

	if (!gpu_is_power_on()) {
		GPU_LOG(DVFS_INFO, DUMMY, 0u, 0u, "%s: can't set clock off in power off status\n", __func__);
		ret = -1;
		goto err_return;
	}

	if (platform->clk_g3d_status == 0) {
		ret = 0;
		goto err_return;
	}

	if (aclk_g3d) {
		(void) clk_disable(clk_ahb2apb_g3dp);
		(void) clk_disable(clk_g3d);
		(void) clk_disable(aclk_g3d);

		GPU_LOG(DVFS_DEBUG, LSI_CLOCK_OFF, 0u, 0u, "clock is disabled\n");
	}

	platform->clk_g3d_status = 0;

err_return:
	return ret;
}

int gpu_register_dump(void)
{
	if (gpu_is_power_on()) {
		/* G3D PMU */
		GPU_LOG(DVFS_DEBUG, LSI_REGISTER_DUMP, 0x105C4064, __raw_readl(EXYNOS5420_G3D_STATUS),
							"REG_DUMP: EXYNOS5420_G3D_STATUS %x\n", __raw_readl(EXYNOS5420_G3D_STATUS));

#if 0
		/* G3D PLL */
		GPU_LOG(DVFS_DEBUG, LSI_REGISTER_DUMP, 0x14AA0000, __raw_readl(EXYNOS5420_G3D_PLL_LOCK),
							"REG_DUMP: EXYNOS5420_G3D_PLL_LOCK %x\n", __raw_readl(EXYNOS5430_G3D_PLL_LOCK));
		GPU_LOG(DVFS_DEBUG, LSI_REGISTER_DUMP, 0x14AA0100, __raw_readl(EXYNOS5420_G3D_PLL_CON0),
							"REG_DUMP: EXYNOS5420_G3D_PLL_CON0 %x\n", __raw_readl(EXYNOS5430_G3D_PLL_CON0));
		GPU_LOG(DVFS_DEBUG, LSI_REGISTER_DUMP, 0x14AA0104, __raw_readl(EXYNOS5420_G3D_PLL_CON1),
							"REG_DUMP: EXYNOS5420_G3D_PLL_CON1 %x\n", __raw_readl(EXYNOS5430_G3D_PLL_CON1));
		GPU_LOG(DVFS_DEBUG, LSI_REGISTER_DUMP, 0x14AA010c, __raw_readl(EXYNOS5420_G3D_PLL_FREQ_DET),
							"REG_DUMP: EXYNOS5420_G3D_PLL_FREQ_DET %x\n", __raw_readl(EXYNOS5430_G3D_PLL_FREQ_DET));

		/* G3D SRC */
		GPU_LOG(DVFS_DEBUG, LSI_REGISTER_DUMP, 0x14AA0200, __raw_readl(EXYNOS5420_SRC_SEL_G3D),
							"REG_DUMP: EXYNOS5420_SRC_SEL_G3D %x\n", __raw_readl(EXYNOS5430_SRC_SEL_G3D));
		GPU_LOG(DVFS_DEBUG, LSI_REGISTER_DUMP, 0x14AA0300, __raw_readl(EXYNOS5420_SRC_ENABLE_G3D),
							"REG_DUMP: EXYNOS5420_SRC_ENABLE_G3D %x\n", __raw_readl(EXYNOS5430_SRC_ENABLE_G3D));
		GPU_LOG(DVFS_DEBUG, LSI_REGISTER_DUMP, 0x14AA0400, __raw_readl(EXYNOS5420_SRC_STAT_G3D),
							"REG_DUMP: EXYNOS5420_SRC_STAT_G3D %x\n", __raw_readl(EXYNOS5430_SRC_STAT_G3D));

		/* G3D DIV */
		GPU_LOG(DVFS_DEBUG, LSI_REGISTER_DUMP, 0x14AA0600, __raw_readl(EXYNOS5420_DIV_G3D),
							"REG_DUMP: EXYNOS5420_DIV_G3D %x\n", __raw_readl(EXYNOS5430_DIV_G3D));
		GPU_LOG(DVFS_DEBUG, LSI_REGISTER_DUMP, 0x14AA0604, __raw_readl(EXYNOS5420_DIV_G3D_PLL_FREQ_DET),
							"REG_DUMP: EXYNOS5420_DIV_G3D_PLL_FREQ_DET %x\n", __raw_readl(EXYNOS5430_DIV_G3D_PLL_FREQ_DET));
		GPU_LOG(DVFS_DEBUG, LSI_REGISTER_DUMP, 0x14AA0700, __raw_readl(EXYNOS5420_DIV_STAT_G3D),
							"REG_DUMP: EXYNOS5420_DIV_STAT_G3D %x\n", __raw_readl(EXYNOS5430_DIV_STAT_G3D));
		GPU_LOG(DVFS_DEBUG, LSI_REGISTER_DUMP, 0x14AA0704, __raw_readl(EXYNOS5420_DIV_STAT_G3D_PLL_FREQ_DET),
							"REG_DUMP: EXYNOS5420_DIV_STAT_G3D_PLL_FREQ_DET %x\n", __raw_readl(EXYNOS5430_DIV_STAT_G3D_PLL_FREQ_DET));

		/* G3D ENABLE */
		GPU_LOG(DVFS_DEBUG, LSI_REGISTER_DUMP, 0x14AA0800, __raw_readl(EXYNOS5420_ENABLE_ACLK_G3D),
							"REG_DUMP: EXYNOS5420_ENABLE_ACLK_G3D %x\n", __raw_readl(EXYNOS5430_ENABLE_ACLK_G3D));
		GPU_LOG(DVFS_DEBUG, LSI_REGISTER_DUMP, 0x14AA0900, __raw_readl(EXYNOS5420_ENABLE_PCLK_G3D),
							"REG_DUMP: EXYNOS5420_ENABLE_PCLK_G3D %x\n", __raw_readl(EXYNOS5430_ENABLE_PCLK_G3D));
		GPU_LOG(DVFS_DEBUG, LSI_REGISTER_DUMP, 0x14AA0A00, __raw_readl(EXYNOS5420_ENABLE_SCLK_G3D),
							"REG_DUMP: EXYNOS5420_ENABLE_SCLK_G3D %x\n", __raw_readl(EXYNOS5430_ENABLE_SCLK_G3D));
		GPU_LOG(DVFS_DEBUG, LSI_REGISTER_DUMP, 0x14AA0B00, __raw_readl(EXYNOS5420_ENABLE_IP_G3D0),
							"REG_DUMP: EXYNOS5420_ENABLE_IP_G3D0 %x\n", __raw_readl(EXYNOS5430_ENABLE_IP_G3D0));
		GPU_LOG(DVFS_DEBUG, LSI_REGISTER_DUMP, 0x14AA0B0A, __raw_readl(EXYNOS5420_ENABLE_IP_G3D1),
							"REG_DUMP: EXYNOS5420_ENABLE_IP_G3D1 %x\n", __raw_readl(EXYNOS5430_ENABLE_IP_G3D1));
#endif
	}

	return 0;
}

#if defined(CONFIG_SUPPORT_WQXGA)
unsigned long get_dpll_freq(int curr, int targ)
{
    unsigned long dpll_clk;
    int divider;

    switch(targ)
    {
        case 480: case 420: case 350:
            divider = 2;
            break;
        case 266:
            divider = 2 + (targ < curr ? 0:1);
            break;
        case 177:
            divider = 3 + (targ < curr ? 0:1);
            break;
        case 100:
            divider = 4;
            break;
        default:
            divider = 1;
            break;
    }
    dpll_clk = curr / divider + 5;

    return (dpll_clk*1000000);
}
#endif

static int gpu_set_clock(struct exynos_context *platform, int clk)
{
#if defined(CONFIG_SUPPORT_WQXGA)
	struct clk *mout_dpll = NULL;
	struct clk *mout_vpll = NULL;
	struct clk *fout_vpll = NULL;
	struct clk *aclk_g3d_dout = NULL;
	struct clk *parent= NULL;
	static long vpll_rate_prev = -1;
	unsigned long vpll_rate = clk * 1000000;
	unsigned long aclk_rate = clk * 1000000;

	unsigned long tmp = 0;

	if (NULL == platform) {
		panic("oops");
	}

	if (aclk_g3d == 0)
		return -1;

	aclk_g3d_dout = clk_get(pkbdev->dev, "aclk_g3d_dout");
	if (IS_ERR(aclk_g3d_dout)) {
		printk(KERN_ERR "[kbase_platform_dvfs_set_clock] failed to clk_get [aclk_g3d_dout] = %ld\n", aclk_rate);
		return -1;
	}

	/* if changed the VPLL rate, set rate for VPLL and wait for lock time */
	if (vpll_rate != vpll_rate_prev) {
		mout_dpll = clk_get(pkbdev->dev, "mout_dpll");
		mout_vpll = clk_get(pkbdev->dev, "mout_vpll");
		fout_vpll = clk_get(pkbdev->dev, "fout_vpll");
		if (IS_ERR(mout_dpll) || IS_ERR(mout_vpll) || IS_ERR(fout_vpll)) {
			printk(KERN_ERR "[kbase_platform_dvfs_set_clock] failed to clk_get ext_xtal, aclk_g3d_sw or fout_vpll\n");
			return -1;
		}

		/*for stable clock input.*/
		clk_set_rate(aclk_g3d_dout, get_dpll_freq(clk_get_rate(aclk_g3d)/1000000,clk));
		clk_set_parent(aclk_g3d_dout, mout_dpll);

		/*change vpll*/
		clk_set_rate(fout_vpll, vpll_rate);

		/*restore parent*/
		clk_set_parent(aclk_g3d_dout, mout_vpll);
		vpll_rate_prev = vpll_rate;
	}
#else
	struct clk *ext_xtal = NULL;
	struct clk *aclk_g3d_sw = NULL;
	struct clk *fout_vpll = NULL;
	struct clk *aclk_g3d_dout = NULL;
	static long vpll_rate_prev = -1;
	unsigned long vpll_rate = clk * 1000000;
	unsigned long aclk_rate = clk * 1000000;

	unsigned long tmp = 0;

	if (NULL == platform) {
		panic("oops");
	}

	if (aclk_g3d == 0)
		return -1;

	aclk_g3d_dout = clk_get(pkbdev->dev, "aclk_g3d_dout");
	if (IS_ERR(aclk_g3d_dout)) {
		printk(KERN_ERR "[kbase_platform_dvfs_set_clock] failed to clk_get [aclk_g3d_dout] = %ld\n", aclk_rate);

		return -1;
	}

	/* if changed the VPLL rate, set rate for VPLL and wait for lock time */
	if (vpll_rate != vpll_rate_prev) {
		ext_xtal = clk_get(pkbdev->dev, "ext_xtal");
		aclk_g3d_sw = clk_get(pkbdev->dev, "aclk_g3d_sw");
		fout_vpll = clk_get(pkbdev->dev, "fout_vpll");
		if (IS_ERR(ext_xtal) || IS_ERR(aclk_g3d_sw) || IS_ERR(fout_vpll)) {
			printk(KERN_ERR "[kbase_platform_dvfs_set_clock] failed to clk_get ext_xtal, aclk_g3d_sw or fout_vpll\n");
			return -1;
		}

		/*for stable clock input.*/
		clk_set_rate(aclk_g3d_dout, 100000000);
		clk_set_parent(aclk_g3d, ext_xtal);

		/*change vpll*/
		clk_set_rate(fout_vpll, vpll_rate);

		/*restore parent*/
		clk_set_parent(aclk_g3d, aclk_g3d_sw);
		vpll_rate_prev = vpll_rate;
	}
#endif

	clk_set_rate(aclk_g3d_dout, aclk_rate);

	/* Waiting for clock is stable */
	do {
		tmp = __raw_readl(EXYNOS5_CLKDIV_STAT_TOP2);
	} while (tmp & 0x10000);

	platform->cur_clock = gpu_get_cur_clock(platform);

#ifdef MALI_DEBUG
	pr_info("===clock set: %ld\n", aclk_rate);
	pr_info("===clock get: %ld\n", clk_get_rate(aclk_g3d));
	pr_info("===clock get: %ld\n", clk_get_rate(aclk_g3d_dout));
#endif

	return 0;
}

static int gpu_set_clock_to_osc(struct exynos_context *platform)
{
	int ret = 0;

	if (!gpu_is_power_on()) {
		ret = -1;
		GPU_LOG(DVFS_INFO, DUMMY, 0u, 0u, "%s: can't control clock in the power-off state!\n", __func__);
		goto err;
	}

	/* change the mux to osc */
	ret = clk_set_parent(mout_vpll, ext_xtal);
	if (ret < 0) {
		GPU_LOG(DVFS_ERROR, DUMMY, 0u, 0u, "%s: failed to clk_set_parent [mout_vpll]\n", __func__);
		goto err;
	}

	GPU_LOG(DVFS_DEBUG, LSI_CLOCK_VALUE, platform->cur_clock, gpu_get_cur_clock(platform),
		"clock set to soc: %d (%d)\n", gpu_get_cur_clock(platform), platform->cur_clock);
err:
	return ret;
}

static int gpu_set_clock_pre(struct exynos_context *platform, int clk, bool is_up)
{
	if (!platform)
		return -ENODEV;

	if (!is_up) {
#if defined(CONFIG_EXYNOS5433_BTS)
		if (clk < platform->table[GPU_L4].clock)
			bts_scen_update(TYPE_G3D_SCENARIO, 0);
		else
			bts_scen_update(TYPE_G3D_SCENARIO, 1);
#endif /* CONFIG_EXYNOS5433_BTS */
#ifdef CONFIG_EXYNOS5420_BTS
		bts_change_g3d_state(clk);
#endif
	}

	return 0;
}

static int gpu_set_clock_post(struct exynos_context *platform, int clk, bool is_up)
{
	if (!platform)
		return -ENODEV;

	if (is_up) {
#if defined(CONFIG_EXYNOS5433_BTS)
		if (clk < platform->table[GPU_L4].clock)
			bts_scen_update(TYPE_G3D_SCENARIO, 0);
		else
			bts_scen_update(TYPE_G3D_SCENARIO, 1);
#endif /* CONFIG_EXYNOS5433_BTS */
#ifdef CONFIG_EXYNOS5420_BTS
		bts_change_g3d_state(clk);
#endif
	}

	return 0;
}

static int gpu_get_clock(struct kbase_device *kbdev)
{
	struct exynos_context *platform = (struct exynos_context *) kbdev->platform_context;
	if (!platform)
		return -ENODEV;

	KBASE_DEBUG_ASSERT(kbdev != NULL);

	clk_g3d = clk_get(kbdev->dev, "g3d");
	if (IS_ERR(clk_g3d)) {
		GPU_LOG(DVFS_ERROR, DUMMY, 0u, 0u, "%s: failed to clk_get [clk_g3d]\n", __func__);
		return -1;
	}

	ext_xtal = clk_get(kbdev->dev, "ext_xtal");
	if (IS_ERR(ext_xtal) || (ext_xtal == NULL)) {
		GPU_LOG(DVFS_ERROR, DUMMY, 0u, 0u, "%s: failed to clk_get [ext_xtal]\n", __func__);
		return -1;
	}

	fout_vpll = clk_get(NULL, "fout_vpll");
	if (IS_ERR(fout_vpll)) {
		GPU_LOG(DVFS_ERROR, DUMMY, 0u, 0u, "%s: failed to clk_get [fout_vpll]\n", __func__);
		return -1;
	}

	aclk_g3d = clk_get(kbdev->dev, "aclk_g3d");
	if (IS_ERR(aclk_g3d)) {
		GPU_LOG(DVFS_ERROR, DUMMY, 0u, 0u, "%s: failed to clk_get [aclk_g3d]\n", __func__);
		return -1;
	}

	aclk_g3d_dout = clk_get(kbdev->dev, "aclk_g3d_dout");
	if (IS_ERR(aclk_g3d_dout)) {
		GPU_LOG(DVFS_ERROR, DUMMY, 0u, 0u, "%s: failed to clk_get [aclk_g3d_dout]\n", __func__);
		return -1;
	}

	mout_vpll = clk_get(kbdev->dev, "mout_vpll");
	if (IS_ERR(mout_vpll)) {
		GPU_LOG(DVFS_ERROR, DUMMY, 0u, 0u, "%s: failed to clk_get [mout_vpll]\n", __func__);
		return -1;
	}
	
	clk_ahb2apb_g3dp = clk_get(kbdev->dev, "clk_ahb2apb_g3dp");
	if (IS_ERR(clk_ahb2apb_g3dp)) {
		GPU_LOG(DVFS_ERROR, DUMMY, 0u, 0u, "%s: failed to clk_get [clk_ahb2apb_g3dp]\n", __func__);
		return -1;
	}

	(void) clk_enable(aclk_g3d);
	(void) clk_enable(clk_g3d);
	(void) clk_enable(clk_ahb2apb_g3dp);

	return 0;
}

int gpu_clock_init(struct kbase_device *kbdev)
{
	int ret;

	KBASE_DEBUG_ASSERT(kbdev != NULL);

	ret = gpu_get_clock(kbdev);
	if (ret < 0)
		return -1;

	GPU_LOG(DVFS_INFO, DUMMY, 0u, 0u, "clock initialized\n");

	return 0;
}

int gpu_get_cur_voltage(struct exynos_context *platform)
{
	int ret = 0;
#ifdef CONFIG_REGULATOR
	if (!g3d_regulator) {
		GPU_LOG(DVFS_ERROR, DUMMY, 0u, 0u, "%s: regulator is not initialized\n", __func__);
		return -1;
	}

	ret = regulator_get_voltage(g3d_regulator);
#endif /* CONFIG_REGULATOR */
	return ret;
}

static int gpu_set_voltage(struct exynos_context *platform, int vol)
{
	if (gpu_get_cur_voltage(platform) == vol)
		return 0;

	if (!gpu_is_power_on()) {
		GPU_LOG(DVFS_INFO, DUMMY, 0u, 0u, "%s: can't set voltage in the power-off state!\n", __func__);
		return -1;
	}

#ifdef CONFIG_REGULATOR
	if (!g3d_regulator) {
		GPU_LOG(DVFS_ERROR, DUMMY, 0u, 0u, "%s: regulator is not initialized\n", __func__);
		return -1;
	}

	if (regulator_set_voltage(g3d_regulator, vol, vol) != 0) {
		GPU_LOG(DVFS_ERROR, DUMMY, 0u, 0u, "%s: failed to set voltage, voltage: %d\n", __func__, vol);
		return -1;
	}
#endif /* CONFIG_REGULATOR */

	platform->cur_voltage = gpu_get_cur_voltage(platform);

	GPU_LOG(DVFS_DEBUG, LSI_VOL_VALUE, vol, platform->cur_voltage, "voltage set: %d, voltage get:%d\n", vol, platform->cur_voltage);

	return 0;
}

static int gpu_set_voltage_pre(struct exynos_context *platform, bool is_up)
{
	if (!platform)
		return -ENODEV;

	if (!is_up && platform->dynamic_abb_status)
		set_match_abb(ID_G3D, gpu_dvfs_get_cur_asv_abb());

	return 0;
}

static int gpu_set_voltage_post(struct exynos_context *platform, bool is_up)
{
	if (!platform)
		return -ENODEV;

	if (is_up && platform->dynamic_abb_status)
		set_match_abb(ID_G3D, gpu_dvfs_get_cur_asv_abb());

	return 0;
}

static struct gpu_control_ops ctr_ops = {
	.is_power_on = gpu_is_power_on,
	.set_voltage = gpu_set_voltage,
	.set_voltage_pre = gpu_set_voltage_pre,
	.set_voltage_post = gpu_set_voltage_post,
	.set_clock_to_osc = gpu_set_clock_to_osc,
	.set_clock = gpu_set_clock,
	.set_clock_pre = gpu_set_clock_pre,
	.set_clock_post = gpu_set_clock_post,
	.enable_clock = gpu_clock_on,
	.disable_clock = gpu_clock_off,
};

struct gpu_control_ops *gpu_get_control_ops(void)
{
	return &ctr_ops;
}

#ifdef CONFIG_REGULATOR
int gpu_enable_dvs(struct exynos_context *platform)
{
#if 0
	if (!platform->dvs_status)
		return 0;

#if defined(CONFIG_REGULATOR_S2MPS13)
	if (s2m_set_dvs_pin(true) != 0) {
		GPU_LOG(DVFS_ERROR, DUMMY, 0u, 0u, "%s: failed to enable dvs\n", __func__);
		return -1;
	}
#endif /* CONFIG_REGULATOR_S2MPS13 */

//	if (!cal_get_fs_abb()) {
//		if (set_match_abb(ID_G3D, G3D_RBB_VALUE)) {
		if (set_match_abb(ID_G3D, platform->devfreq_g3d_asv_abb[platform->step])) {
			GPU_LOG(DVFS_ERROR, DUMMY, 0u, 0u, "%s: failed to restore RBB setting\n", __func__);
			return -1;
		}
//	}

	GPU_LOG(DVFS_INFO, DUMMY, 0u, 0u, "dvs is enabled (vol: %d)\n", gpu_get_cur_voltage(platform));
#endif
	return 0;
}

int gpu_disable_dvs(struct exynos_context *platform)
{
#if 0
	if (!platform->dvs_status)
		return 0;
#if 0
	if (!cal_get_fs_abb()) {
		if (set_match_abb(ID_G3D, gpu_dvfs_get_cur_asv_abb())) {
			GPU_LOG(DVFS_ERROR, DUMMY, 0u, 0u, "%s: failed to restore RBB setting\n", __func__);
			return -1;
		}
	}
#endif

#if defined(CONFIG_REGULATOR_S2MPS13)
	if (s2m_set_dvs_pin(false) != 0) {
		GPU_LOG(DVFS_ERROR, DUMMY, 0u, 0u, "%s: failed to disable dvs\n", __func__);
		return -1;
	}
#endif /* CONFIG_REGULATOR_S2MPS13 */

	GPU_LOG(DVFS_INFO, DUMMY, 0u, 0u, "dvs is disabled (vol: %d)\n", gpu_get_cur_voltage(platform));
#endif
	return 0;
}

int gpu_regulator_init(struct exynos_context *platform)
{
	int gpu_voltage = 0;

	g3d_regulator = regulator_get(NULL, "vdd_g3d");
	if (IS_ERR(g3d_regulator)) {
		GPU_LOG(DVFS_ERROR, DUMMY, 0u, 0u, "%s: failed to get vdd_g3d regulator, 0x%p\n", __func__, g3d_regulator);
		g3d_regulator = NULL;
		return -1;
	}

	gpu_voltage = get_match_volt(ID_G3D, platform->gpu_dvfs_config_clock*1000);

	if (gpu_voltage == 0)
		gpu_voltage = platform->gpu_default_vol;

	if (gpu_set_voltage(platform, gpu_voltage) != 0) {
		GPU_LOG(DVFS_ERROR, DUMMY, 0u, 0u, "%s: failed to set voltage [%d]\n", __func__, gpu_voltage);
		return -1;
	}

	GPU_LOG(DVFS_INFO, DUMMY, 0u, 0u, "regulator initialized\n");

	return 0;
}
#endif /* CONFIG_REGULATOR */

int *get_mif_table(int *size)
{
	*size = ARRAY_SIZE(mif_min_table);
	return mif_min_table;
}
