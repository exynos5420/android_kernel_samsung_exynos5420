/*
 *
 * (C) COPYRIGHT 2011 ARM Limited. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation, and any use by you of this program is subject to the terms of such GNU licence.
 *
 * A copy of the licence is included with the program, and can also be obtained from Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include <linux/ioport.h>
#include <linux/clk.h>
#include <mali_kbase.h>
#include <mali_kbase_defs.h>
#include <mali_kbase_config_linux.h>
#include <mach/map.h>
#include <plat/devs.h>
#include <linux/pm_runtime.h>
#include <platform/mali_kbase_platform.h>
#include <linux/suspend.h>
#include <platform/mali_kbase_dvfs.h>

#define HZ_IN_MHZ                           (1000000)
#ifdef CONFIG_MALI_T6XX_RT_PM
#define RUNTIME_PM_DELAY_TIME 100
#endif

static int mali_pm_notifier(struct notifier_block *nb, unsigned long event, void *cmd);
static struct notifier_block mali_pm_nb = {
	.notifier_call = mali_pm_notifier
};

static kbase_io_resources io_resources = {
	.job_irq_number   = JOB_IRQ_NUMBER,
	.mmu_irq_number   = MMU_IRQ_NUMBER,
	.gpu_irq_number   = GPU_IRQ_NUMBER,
	.io_memory_region = {
		.start = EXYNOS5_PA_G3D,
		.end   = EXYNOS5_PA_G3D + (4096 * 5) - 1
	}
};

int get_cpu_clock_speed(u32 *cpu_clock)
{
	struct clk *cpu_clk;
	u32 freq = 0;
	cpu_clk = clk_get(NULL, "armclk");
	if (IS_ERR(cpu_clk))
		return 1;
	freq = clk_get_rate(cpu_clk);
	*cpu_clock = (freq/HZ_IN_MHZ);
	return 0;
}

static int mali_pm_notifier(struct notifier_block *nb, unsigned long event, void *cmd)
{
	int err = NOTIFY_OK;
	switch (event) {
	case PM_SUSPEND_PREPARE:
#ifdef CONFIG_MALI_T6XX_DVFS
		if (kbase_platform_dvfs_enable(false, MALI_DVFS_CURRENT_FREQ) != MALI_TRUE)
			err = NOTIFY_BAD;
#endif
		break;
	case PM_POST_SUSPEND:
#ifdef CONFIG_MALI_T6XX_DVFS
		if (kbase_platform_dvfs_enable(true, MALI_DVFS_START_FREQ) != MALI_TRUE)
			err = NOTIFY_BAD;
#endif
		break;
	default:
		break;
	}
	return err;
}

/**
 *  * Exynos5 hardware specific initialization
 *   */
mali_bool kbase_platform_exynos5_init(kbase_device *kbdev)
{
	if (MALI_ERROR_NONE == kbase_platform_init(kbdev)) {
		if (register_pm_notifier(&mali_pm_nb)) {
			return MALI_FALSE;
		}
#ifdef CONFIG_MALI_T6XX_DEBUG_SYS
		if (kbase_platform_create_sysfs_file(kbdev->osdev.dev)) {
			return MALI_ERROR_FUNCTION_FAILED;
		}
#endif /* CONFIG_MALI_T6XX_DEBUG_SYS */
		return MALI_TRUE;
	}

	return MALI_FALSE;
}

/**
 *  * Exynos5 hardware specific termination
 *   */
void kbase_platform_exynos5_term(kbase_device *kbdev)
{
	unregister_pm_notifier(&mali_pm_nb);
#ifdef CONFIG_MALI_T6XX_DEBUG_SYS
	kbase_platform_remove_sysfs_file(kbdev->osdev.dev);
#endif /* CONFIG_MALI_T6XX_DEBUG_SYS */
	kbase_platform_term(kbdev);
}
kbase_platform_funcs_conf platform_funcs = {
	.platform_init_func = &kbase_platform_exynos5_init,
	.platform_term_func = &kbase_platform_exynos5_term,
};

#ifdef CONFIG_MALI_T6XX_RT_PM
static int pm_callback_power_on(kbase_device *kbdev)
{
	int result;
	int ret_val;
	struct kbase_os_device *osdev = &kbdev->osdev;
	struct exynos_context *platform;

	platform = (struct exynos_context *) kbdev->platform_context;

	if (pm_runtime_status_suspended(osdev->dev))
		ret_val = 1;
	else
		ret_val = 0;

	if (osdev->dev->power.disable_depth > 0) {
		if (platform->cmu_pmu_status == 0)
			kbase_platform_cmu_pmu_control(kbdev, 1);
		return ret_val;
	}
	result = pm_runtime_resume(osdev->dev);

	if (result < 0 && result == -EAGAIN)
		kbase_platform_cmu_pmu_control(kbdev, 1);
	else if (result < 0)
		printk(KERN_ERR "pm_runtime_get_sync failed (%d)\n", result);

	return ret_val;
}

static void pm_callback_power_off(kbase_device *kbdev)
{
	struct kbase_os_device *osdev = &kbdev->osdev;
	pm_schedule_suspend(osdev->dev, RUNTIME_PM_DELAY_TIME);
}

mali_error kbase_device_runtime_init(struct kbase_device *kbdev)
{
	pm_suspend_ignore_children(kbdev->osdev.dev, true);
	pm_runtime_enable(kbdev->osdev.dev);
	return MALI_ERROR_NONE;
}

void kbase_device_runtime_disable(struct kbase_device *kbdev)
{
	pm_runtime_disable(kbdev->osdev.dev);
}

static int pm_callback_runtime_on(kbase_device *kbdev)
{
	int ret;
	struct clk *mout_vpll = NULL, *aclk_g3d_sw = NULL, *aclk_g3d_dout = NULL;
	struct device *dev =  kbdev->osdev.dev;
	struct exynos_context * platform = (struct exynos_context *) kbdev->platform_context;

	kbase_platform_clock_on(kbdev);
#ifdef CONFIG_MALI_T6XX_DVFS
	if (kbase_platform_dvfs_enable(true, MALI_DVFS_START_FREQ) != MALI_TRUE)
		return -EPERM;
#endif
	mout_vpll = clk_get(dev, "mout_vpll");
	if (IS_ERR(mout_vpll)) {
		KBASE_DEBUG_PRINT_ERROR(KBASE_CORE, "failed to clk_get [mout_vpll]\n");
		return 0;
	}
	aclk_g3d_dout = clk_get(dev, "aclk_g3d_dout");
	if (IS_ERR(aclk_g3d_dout)) {
		KBASE_DEBUG_PRINT_ERROR(KBASE_CORE, "failed to clk_get [aclk_g3d_dout]\n");
		return 0;
	}
	aclk_g3d_sw = clk_get(dev, "aclk_g3d_sw");
	if (IS_ERR(aclk_g3d_sw)) {
		KBASE_DEBUG_PRINT_ERROR(KBASE_CORE, "failed to clk_get [aclk_g3d_sw]\n");
		return 0;
	}

	ret = clk_set_parent(platform->aclk_g3d, aclk_g3d_sw);
	if (ret < 0) {
		KBASE_DEBUG_PRINT_ERROR(KBASE_CORE, "failed to clk_set_parent [platform->aclk_g3d]\n");
		return 0;
	}
	ret = clk_set_parent(aclk_g3d_sw, aclk_g3d_dout);
	if (ret < 0) {
		KBASE_DEBUG_PRINT_ERROR(KBASE_CORE, "failed to clk_set_parent [aclk_g3d_sw]\n");
		return 0;
	}
	ret = clk_set_parent(aclk_g3d_dout, mout_vpll);
	if (ret < 0) {
		KBASE_DEBUG_PRINT_ERROR(KBASE_CORE, "failed to clk_set_parent [aclk_g3d_dout]\n");
		return 0;
	}

	return 0;
}

static void pm_callback_runtime_off(kbase_device *kbdev)
{
	kbase_platform_clock_off(kbdev);
#ifdef CONFIG_MALI_T6XX_DVFS
	if (kbase_platform_dvfs_enable(false, MALI_DVFS_CURRENT_FREQ) != MALI_TRUE)
		printk("[err] disabling dvfs is faled\n");
#endif
}

static kbase_pm_callback_conf pm_callbacks = {
	.power_on_callback = pm_callback_power_on,
	.power_off_callback = pm_callback_power_off,
#ifdef CONFIG_PM_RUNTIME
	.power_runtime_init_callback = kbase_device_runtime_init,
	.power_runtime_term_callback = kbase_device_runtime_disable,
	.power_runtime_on_callback = pm_callback_runtime_on,
	.power_runtime_off_callback = pm_callback_runtime_off,
#else /* CONFIG_PM_RUNTIME */
	.power_runtime_init_callback = NULL,
	.power_runtime_term_callback = NULL,
	.power_runtime_on_callback = NULL,
	.power_runtime_off_callback = NULL,
#endif /* CONFIG_PM_RUNTIME */
};
#endif

static kbase_attribute config_attributes[] = {
#ifdef CONFIG_MALI_T6XX_RT_PM
	{
		KBASE_CONFIG_ATTR_POWER_MANAGEMENT_CALLBACKS,
		(uintptr_t)&pm_callbacks
	},
#endif
	{
		KBASE_CONFIG_ATTR_POWER_MANAGEMENT_DVFS_FREQ,
		100
	}, /* 100ms */
	{
		KBASE_CONFIG_ATTR_PLATFORM_FUNCS,
		(uintptr_t)&platform_funcs
	},
	{
		KBASE_CONFIG_ATTR_GPU_FREQ_KHZ_MAX,
		533000
	},

	{
		KBASE_CONFIG_ATTR_GPU_FREQ_KHZ_MIN,
		100000
	},
	{
		KBASE_CONFIG_ATTR_JS_SOFT_STOP_TICKS,
		10
	}, /*1000ms*/
	{
		KBASE_CONFIG_ATTR_JS_HARD_STOP_TICKS_SS,
		150
	},
	{
		KBASE_CONFIG_ATTR_JS_RESET_TICKS_SS,
		160
	},
	{
		KBASE_CONFIG_ATTR_JS_RESET_TIMEOUT_MS,
		500 /* 500ms before cancelling stuck jobs */
	},
	{
		KBASE_CONFIG_ATTR_CPU_SPEED_FUNC,
		(uintptr_t)&get_cpu_clock_speed
	},
	{
		KBASE_CONFIG_ATTR_END,
		0
	}
};

kbase_platform_config platform_config = {
		.attributes                = config_attributes,
		.io_resources              = &io_resources,
		.midgard_type              = KBASE_MALI_T604
};

int kbase_platform_early_init(void)
{
	kbase_platform_config *config;
	int attribute_count;

	config = &platform_config;
	attribute_count = kbasep_get_config_attribute_count(config->attributes);

	return platform_device_add_data(
		&exynos5_device_g3d,
		config->attributes,
		attribute_count * sizeof(config->attributes[0]));
}
