/* drivers/gpu/t6xx/kbase/src/platform/5260/gpu_control_exynos5260.c
 *
 * Copyright 2011 by S.LSI. Samsung Electronics Inc.
 * San#24, Nongseo-Dong, Giheung-Gu, Yongin, Korea
 *
 * Samsung SoC Mali-T604 DVFS driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software FoundatIon.
 */

/**
 * @file gpu_control_exynos5260.c
 * DVFS
 */

#include <kbase/src/common/mali_kbase.h>

#include <linux/regulator/driver.h>

#include <mach/asv-exynos.h>
#include <mach/pm_domains_v2.h>
#include <mach/regs-clock-exynos5260.h>

#include "mali_kbase_platform.h"
#include "gpu_dvfs_handler.h"
#include "gpu_control.h"

extern struct kbase_device *pkbdev;

#ifdef CONFIG_PM_RUNTIME
struct exynos_pm_domain *gpu_get_pm_domain(kbase_device *kbdev)
{
	struct exynos_pm_domain *pd = NULL;
	pd = exynos_get_power_domain("pd-g3d");
	return pd;
}
#endif

int get_cpu_clock_speed(u32 *cpu_clock)
{
	struct clk *cpu_clk;
	u32 freq = 0;
	cpu_clk = clk_get(NULL, "armclk");
	if (IS_ERR(cpu_clk))
		return -1;
	freq = clk_get_rate(cpu_clk);
	*cpu_clock = (freq/MHZ);
	return 0;
}

int gpu_is_power_on(void)
{
	return ((__raw_readl(EXYNOS5260_G3D_STATUS) & EXYNOS5260_INT_LOCAL_PWR_EN) == EXYNOS5260_INT_LOCAL_PWR_EN) ? 1 : 0;
}

int gpu_power_init(kbase_device *kbdev)
{
	struct exynos_context *platform = (struct exynos_context *) kbdev->platform_context;

	if (!platform)
		return -ENODEV;

	GPU_LOG(DVFS_INFO, "g3d power initialized\n");

	return 0;
}

static int gpu_update_clock(struct exynos_context *platform)
{
	if (!platform->aclk_g3d) {
		GPU_LOG(DVFS_ERROR, "aclk_g3d is not initialized\n");
		return -1;
	}

	platform->cur_clock = clk_get_rate(platform->aclk_g3d)/MHZ;
	return 0;
}

int gpu_clock_on(struct exynos_context *platform)
{
	if (!platform)
		return -ENODEV;

	if (!gpu_is_power_on()) {
		GPU_LOG(DVFS_WARNING, "can't set clock on in g3d power off status\n");
		return -1;
	}

	if (platform->clk_g3d_status == 1)
		return 0;

	if (platform->aclk_g3d)
		(void) clk_prepare_enable(platform->aclk_g3d);

	if (platform->g3d)
		(void) clk_prepare_enable(platform->g3d);

	platform->clk_g3d_status = 1;

	return 0;
}

int gpu_clock_off(struct exynos_context *platform)
{
	if (!platform)
		return -ENODEV;

	if (platform->clk_g3d_status == 0)
		return 0;

	if (platform->aclk_g3d)
		(void)clk_disable_unprepare(platform->aclk_g3d);

	if (platform->g3d)
		(void)clk_disable_unprepare(platform->g3d);

	platform->clk_g3d_status = 0;

	return 0;
}

#ifdef GPU_MO_RESTRICTION
static int gpu_set_multiple_outstanding_req(int val)
{
	volatile unsigned int reg;

	if(val > 0b1111)
		return -1;

	reg = kbase_os_reg_read(pkbdev, GPU_CONTROL_REG(L2_MMU_CONFIG));
	reg &= ~(0b1111 << 24);
	reg |= ((val & 0b1111) << 24);
	kbase_os_reg_write(pkbdev, GPU_CONTROL_REG(L2_MMU_CONFIG), reg);

	return 0;
}
#endif

int gpu_set_clock(struct exynos_context *platform, int freq)
{
	unsigned long g3d_rate = freq * MHZ;
	int ret = 0;

	if (platform->aclk_g3d == 0)
		return -1;

#ifdef CONFIG_PM_RUNTIME
	if (platform->exynos_pm_domain)
		mutex_lock(&platform->exynos_pm_domain->access_lock);
#endif /* CONFIG_PM_RUNTIME */

	if (!gpu_is_power_on()) {
		ret = -1;
		GPU_LOG(DVFS_WARNING, "gpu_set_clk_vol in the G3D power-off state!\n");
		goto err;
	}

#ifdef GPU_MO_RESTRICTION
	gpu_set_multiple_outstanding_req(L2CONFIG_MO_1BY4);				/* Restrict M.O */
#endif
	ret = clk_set_parent(platform->aclk_g3d, platform->ext_xtal);
	if (ret < 0) {
		GPU_LOG(DVFS_ERROR, "failed to exynos_set_parent [mout_g3d_pll]\n");
		goto err;
	}
	ret = clk_set_rate(platform->fout_vpll, g3d_rate);
	if (ret < 0) {
		GPU_LOG(DVFS_ERROR, "failed to exynos_set_rate [fout_vpll]\n");
		goto err;
	}
	ret = clk_set_parent(platform->aclk_g3d, platform->fout_vpll);
	if (ret < 0) {
		GPU_LOG(DVFS_ERROR, "failed to exynos_set_parent [aclk_g3d]\n");
		goto err;
	}

#ifdef GPU_MO_RESTRICTION
	gpu_set_multiple_outstanding_req(L2CONFIG_MO_NO_RESTRICT);		/* Set to M.O by maximum */
#endif

	gpu_update_clock(platform);
	KBASE_TRACE_ADD_EXYNOS(pkbdev, LSI_CLOCK_VALUE, NULL, NULL, 0u, g3d_rate / MHZ);
	GPU_LOG(DVFS_DEBUG, "[G3D] clock set: %ld\n", g3d_rate / MHZ);
	GPU_LOG(DVFS_DEBUG, "[G3D] clock get: %d\n", platform->cur_clock);
err:
#ifdef CONFIG_PM_RUNTIME
	if (platform->exynos_pm_domain)
		mutex_unlock(&platform->exynos_pm_domain->access_lock);
#endif /* CONFIG_PM_RUNTIME */
	return ret;
}

static int gpu_get_clock(kbase_device *kbdev)
{
	struct exynos_context *platform = (struct exynos_context *)kbdev->platform_context;
	if (!platform)
		return -ENODEV;

	platform->fout_vpll = clk_get(kbdev->osdev.dev, "fout_vpll");
	if (IS_ERR(platform->fout_vpll)) {
		platform->fout_vpll = NULL;
		GPU_LOG(DVFS_ERROR, "failed to clk_get[fout_vpll]\n");
		return -ENODEV;
	}

	platform->ext_xtal  = clk_get(kbdev->osdev.dev, "ext_xtal");
	if (IS_ERR(platform->ext_xtal)) {
		platform->ext_xtal = NULL;
		GPU_LOG(DVFS_ERROR, "failed to clk_get[ext_xtal]\n");
		return -ENODEV;
	}

	platform->aclk_g3d  = clk_get(kbdev->osdev.dev, "aclk_g3d");
	if (IS_ERR(platform->aclk_g3d)) {
		platform->aclk_g3d = NULL;
		GPU_LOG(DVFS_ERROR, "failed to clk_get[aclk_g3d]\n");
		return -ENODEV;
	}

	platform->g3d  = clk_get(kbdev->osdev.dev, "g3d");
	if (IS_ERR(platform->g3d)) {
		platform->g3d = NULL;
		GPU_LOG(DVFS_ERROR, "failed to clk_get[g3d]\n");
		return -ENODEV;
	}

	return 0;
}

int gpu_clock_init(kbase_device *kbdev)
{
	int ret;
	struct exynos_context *platform = (struct exynos_context *) kbdev->platform_context;

	if (!platform)
		return -ENODEV;

	KBASE_DEBUG_ASSERT(kbdev != NULL);

	ret = gpu_get_clock(kbdev);
	if (ret < 0)
		return -1;

	GPU_LOG(DVFS_INFO, "g3d clock initialized\n");

	return 0;
}

static int gpu_update_voltage(struct exynos_context *platform)
{
#ifdef CONFIG_REGULATOR
	if (!platform->g3d_regulator) {
		GPU_LOG(DVFS_ERROR, "g3d_regulator is not initialized\n");
		return -1;
	}

	platform->cur_voltage = regulator_get_voltage(platform->g3d_regulator);
#endif /* CONFIG_REGULATOR */
	return 0;
}

int gpu_set_voltage(struct exynos_context *platform, int vol)
{
	static int _vol = -1;

	if (_vol == vol)
		return 0;

#ifdef CONFIG_REGULATOR
	if (!platform->g3d_regulator) {
		GPU_LOG(DVFS_ERROR, "g3d_regulator is not initialized\n");
		return -1;
	}

	if (regulator_set_voltage(platform->g3d_regulator, vol, vol) != 0) {
		GPU_LOG(DVFS_ERROR, "failed to set voltage, voltage: %d\n", vol);
		return -1;
	}
#endif /* CONFIG_REGULATOR */

	_vol = vol;

	gpu_update_voltage(platform);
	KBASE_TRACE_ADD_EXYNOS(pkbdev, LSI_VOL_VALUE, NULL, NULL, 0u, vol);
	GPU_LOG(DVFS_DEBUG, "[G3D] voltage set:%d\n", vol);
	GPU_LOG(DVFS_DEBUG, "[G3D] voltage get:%d\n", platform->cur_voltage);

	return 0;
}

#ifdef CONFIG_REGULATOR
int gpu_regulator_enable(struct exynos_context *platform)
{
	if (!platform->g3d_regulator) {
		GPU_LOG(DVFS_ERROR, "g3d_regulator is not initialized\n");
		return -1;
	}

	if (regulator_enable(platform->g3d_regulator) != 0) {
		GPU_LOG(DVFS_ERROR, "failed to enable g3d regulator\n");
		return -1;
	}
	return 0;
}

int gpu_regulator_disable(struct exynos_context *platform)
{
	if (!platform->g3d_regulator) {
		GPU_LOG(DVFS_ERROR, "g3d_regulator is not initialized\n");
		return -1;
	}

	if (regulator_disable(platform->g3d_regulator) != 0) {
		GPU_LOG(DVFS_ERROR, "failed to disable g3d regulator\n");
		return -1;
	}
	return 0;
}

int gpu_regulator_init(struct exynos_context *platform)
{
	int gpu_voltage = 0;

	platform->g3d_regulator = regulator_get(NULL, "vdd_g3d");
	if (IS_ERR(platform->g3d_regulator)) {
		GPU_LOG(DVFS_ERROR, "failed to get mali t6xx regulator, 0x%p\n", platform->g3d_regulator);
		platform->g3d_regulator = NULL;
		return -1;
	}

	if (gpu_regulator_enable(platform) != 0) {
		GPU_LOG(DVFS_ERROR, "failed to enable mali t6xx regulator\n");
		platform->g3d_regulator = NULL;
		return -1;
	}

	gpu_voltage = get_match_volt(ID_G3D, MALI_DVFS_BL_CONFIG_FREQ*1000);
	if (gpu_voltage == 0)
		gpu_voltage = GPU_DEFAULT_VOLTAGE;

	if (gpu_set_voltage(platform, gpu_voltage) != 0) {
		GPU_LOG(DVFS_ERROR, "failed to set mali t6xx operating voltage [%d]\n", gpu_voltage);
		return -1;
	}

	GPU_LOG(DVFS_INFO, "g3d regulator initialized\n");

	return 0;
}
#endif /* CONFIG_REGULATOR */
