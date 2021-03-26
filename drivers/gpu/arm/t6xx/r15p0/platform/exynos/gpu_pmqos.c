/* drivers/gpu/arm/.../platform/gpu_pmqos.c
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
 * @file gpu_pmqos.c
 * DVFS
 */

#include <mali_kbase.h>

#include <linux/pm_qos.h>

#include "mali_kbase_platform.h"
#include "gpu_dvfs_handler.h"

struct pm_qos_request exynos5_g3d_mif_qos;
struct pm_qos_request exynos5_g3d_int_qos;
struct pm_qos_request exynos5_g3d_cpu_qos;
#if defined(CONFIG_ARM_EXYNOS_MP_CPUFREQ)
static struct pm_qos_request exynos5_g3d_kfc_qos;
#endif

#ifdef CONFIG_MALI_DVFS_USER
struct pm_qos_request proactive_mif_min_qos;
struct pm_qos_request proactive_int_min_qos;
struct pm_qos_request proactive_apollo_min_qos;
struct pm_qos_request proactive_atlas_min_qos;
#endif

int gpu_pm_qos_command(struct exynos_context *platform, gpu_pmqos_state state)
{
	DVFS_ASSERT(platform);

	if (!platform->devfreq_status)
		return 0;

	switch (state) {
	case GPU_CONTROL_PM_QOS_INIT:
		pm_qos_add_request(&exynos5_g3d_mif_qos, PM_QOS_BUS_THROUGHPUT, 0);
		if (!platform->pmqos_int_disable)
			pm_qos_add_request(&exynos5_g3d_int_qos, PM_QOS_DEVICE_THROUGHPUT, 0);
		pm_qos_add_request(&exynos5_g3d_cpu_qos, PM_QOS_CPU_FREQ_MIN, 0);
#if defined(CONFIG_ARM_EXYNOS_MP_CPUFREQ)
		pm_qos_add_request(&exynos5_g3d_kfc_qos, PM_QOS_KFC_FREQ_MIN, 0);
#endif
		break;
	case GPU_CONTROL_PM_QOS_DEINIT:
		pm_qos_remove_request(&exynos5_g3d_mif_qos);
		if (!platform->pmqos_int_disable)
			pm_qos_remove_request(&exynos5_g3d_int_qos);
		pm_qos_remove_request(&exynos5_g3d_cpu_qos);
#if defined(CONFIG_ARM_EXYNOS_MP_CPUFREQ)
	    pm_qos_remove_request(&exynos5_g3d_kfc_qos);
#endif
		break;
	case GPU_CONTROL_PM_QOS_SET:
		KBASE_DEBUG_ASSERT(platform->step >= 0);
		if (platform->perf_gathering_status) {
			gpu_mif_pmqos(platform, platform->table[platform->step].mem_freq);
		} else {
			pm_qos_update_request(&exynos5_g3d_mif_qos, platform->table[platform->step].mem_freq);
		}
		if (!platform->pmqos_int_disable)
			pm_qos_update_request(&exynos5_g3d_int_qos, platform->table[platform->step].int_freq);
		pm_qos_update_request(&exynos5_g3d_cpu_qos, platform->table[platform->step].cpu_freq);
#if defined(CONFIG_ARM_EXYNOS_MP_CPUFREQ)
		pm_qos_update_request(&exynos5_g3d_kfc_qos, platform->table[platform->step].kfc_freq);
#endif
		break;
	case GPU_CONTROL_PM_QOS_RESET:
		pm_qos_update_request(&exynos5_g3d_mif_qos, 0);
		if (!platform->pmqos_int_disable)
			pm_qos_update_request(&exynos5_g3d_int_qos, 0);
		pm_qos_update_request(&exynos5_g3d_cpu_qos, 0);
#if defined(CONFIG_ARM_EXYNOS_MP_CPUFREQ)
		pm_qos_update_request(&exynos5_g3d_kfc_qos, 0);
#endif
		break;
	case GPU_CONTROL_PM_QOS_EGL_SET:
		break;
	case GPU_CONTROL_PM_QOS_EGL_RESET:
		break;
	default:
		break;
	}

	return 0;
}

int gpu_mif_pmqos(struct exynos_context *platform, int mem_freq)
{
	static int prev_freq;
	DVFS_ASSERT(platform);

	if(!platform->devfreq_status)
		return 0;
	if(prev_freq != mem_freq)
		pm_qos_update_request(&exynos5_g3d_mif_qos, mem_freq);

	prev_freq = mem_freq;

	return 0;
}
