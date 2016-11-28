/*
 *
 * (C) COPYRIGHT 2013 ARM Limited. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation, and any use by you of this program is subject to the terms of such GNU licence.
 *
 * A copy of the licence is included with the program, and can also be obtained from Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include <linux/version.h>
#include <linux/ioport.h>
#include <linux/clk.h>
#include <linux/pm_runtime.h>
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
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <mach/map.h>
#include <linux/fb.h>
#include <linux/clk.h>
#include <mach/regs-clock.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0))
#include <asm/pmu.h>
#else
#include <mach/pmu.h>
#endif
#include <mach/regs-pmu.h>
#include <asm/delay.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <kbase/src/common/mali_kbase.h>
#include <kbase/src/common/mali_kbase_defs.h>
#include <kbase/src/common/mali_kbase_pm.h>
#include <kbase/src/common/mali_kbase_uku.h>
#include <kbase/src/common/mali_kbase_mem.h>
#include <kbase/src/common/mali_midg_regmap.h>
#include <kbase/src/linux/mali_kbase_config_linux.h>
#include <kbase/src/linux/mali_kbase_mem_linux.h>
#include <kbase/src/common/mali_kbase_gator.h>
#include <kbase/src/platform/arndale/mali_kbase_platform.h>
#include <kbase/src/platform/arndale/mali_kbase_dvfs.h>

#ifdef CONFIG_UMP
#include <linux/ump-common.h>
#endif	/* CONFIG_UMP */

/* CONFIG_MALI_T6XX_RT_PM_DEBUG is not currently supported in Kconfig */
#ifdef CONFIG_MALI_T6XX_RT_PM_DEBUG
#define MALI_RTPM_DEBUG 1
#else
#define MALI_RTPM_DEBUG 0
#endif

#define HZ_IN_MHZ       (1000000)

#define RUNTIME_PM_DELAY_TIME 100

/* Case the following are not defined */
#ifndef EXYNOS5_PA_G3D
#define EXYNOS5_PA_G3D 0x11800000
#endif
#ifndef JOB_IRQ_NUMBER
#define JOB_IRQ_NUMBER IRQ_SPI(118)
#endif
#ifndef MMU_IRQ_NUMBER
#define MMU_IRQ_NUMBER IRQ_SPI(119)
#endif
#ifndef GPU_IRQ_NUMBER
#define GPU_IRQ_NUMBER IRQ_SPI(117)
#endif

/* All things that are needed for the Linux port. */
void kbase_device_runtime_disable(struct kbase_device *kbdev);
void kbase_device_runtime_get_sync(struct device *dev);
void kbase_device_runtime_put_sync(struct device *dev);
mali_error kbase_device_runtime_init(struct kbase_device *kbdev);


static kbase_io_resources io_resources =
{
	.job_irq_number   = JOB_IRQ_NUMBER,
	.mmu_irq_number   = MMU_IRQ_NUMBER,
	.gpu_irq_number   = GPU_IRQ_NUMBER,
	.io_memory_region =
	{
		.start = EXYNOS5_PA_G3D,
		.end   = EXYNOS5_PA_G3D + (4096 * 5) - 1
	}
};

int kbase_platform_early_init(void)
{
	/* Just a dummy for this platform */
	return 0;
}

/**
 * Read the CPU clock speed
 */
int get_cpu_clock_speed(u32* cpu_clock)
{
	struct clk * cpu_clk;
	u32 freq=0;
	cpu_clk = clk_get(NULL, "armclk");
	if (IS_ERR(cpu_clk))
		return 1;
	freq = clk_get_rate(cpu_clk);
	*cpu_clock = (freq/HZ_IN_MHZ);
	return 0;
}

/**
 *  * Exynos5 hardware specific initialization
 *   */
mali_bool kbase_platform_exynos5_init(kbase_device *kbdev)
{
	if(MALI_ERROR_NONE == kbase_platform_init(kbdev))
	{
#ifdef CONFIG_MALI_T6XX_DEBUG_SYS
		if(kbase_platform_create_sysfs_file(kbdev->osdev.dev))
		{
			return MALI_TRUE;
		}
#endif /* CONFIG_MALI_T6XX_DEBUG_SYS */
		return MALI_TRUE;
	}

	return MALI_FALSE;
}

/**
 * Exynos5 hardware specific termination
 */
void kbase_platform_exynos5_term(kbase_device *kbdev)
{
#ifdef CONFIG_MALI_T6XX_DEBUG_SYS
	kbase_platform_remove_sysfs_file(kbdev->osdev.dev);
#endif /* CONFIG_MALI_T6XX_DEBUG_SYS */
	kbase_platform_term(kbdev);
}

kbase_platform_funcs_conf platform_funcs =
{
	.platform_init_func = &kbase_platform_exynos5_init,
	.platform_term_func = &kbase_platform_exynos5_term,
};


/**
 * Power Management callback - power ON
 */
static int pm_callback_power_on(kbase_device *kbdev)
{
#ifdef CONFIG_PM_RUNTIME
	pm_runtime_resume(kbdev->osdev.dev);
#endif /* CONFIG_PM_RUNTIME */
	return 0;
}

/**
 * Power Management callback - power OFF
 */
static void pm_callback_power_off(kbase_device *kbdev)
{
#ifdef CONFIG_PM_RUNTIME
	pm_schedule_suspend(kbdev->osdev.dev, RUNTIME_PM_DELAY_TIME);
#endif /* CONFIG_PM_RUNTIME */
}

/**
 * Power Management callback - runtime power ON
 */
#ifdef CONFIG_PM_RUNTIME
static int pm_callback_runtime_power_on(kbase_device *kbdev)
{
#if MALI_RTPM_DEBUG
	printk(KERN_DEBUG "kbase_device_runtime_resume\n");
#endif	/* MALI_RTPM_DEBUG */
	return kbase_platform_cmu_pmu_control(kbdev, 1);
}
#endif	/* CONFIG_PM_RUNTIME */

/**
 * Power Management callback - runtime power OFF
 */
#ifdef CONFIG_PM_RUNTIME
static void pm_callback_runtime_power_off(kbase_device *kbdev)
{
#if MALI_RTPM_DEBUG
	printk(KERN_DEBUG "kbase_device_runtime_suspend\n");
#endif	/* MALI_RTPM_DEBUG */
	kbase_platform_cmu_pmu_control(kbdev, 0);
}
#endif /* CONFIG_PM_RUNTIME */

static kbase_pm_callback_conf pm_callbacks =
{
	.power_on_callback  = pm_callback_power_on,
	.power_off_callback = pm_callback_power_off,
#ifdef CONFIG_PM_RUNTIME
	.power_runtime_init_callback = kbase_device_runtime_init,
	.power_runtime_term_callback = kbase_device_runtime_disable,
	.power_runtime_on_callback   = pm_callback_runtime_power_on,
	.power_runtime_off_callback  = pm_callback_runtime_power_off,
#else /* CONFIG_PM_RUNTIME */
	.power_runtime_init_callback = NULL,
	.power_runtime_term_callback = NULL,
	.power_runtime_on_callback   = NULL,
	.power_runtime_off_callback  = NULL,
#endif /* CONFIG_PM_RUNTIME */
};



static kbase_attribute config_attributes[] = {
#if CONFIG_UMP == 1
	{
		KBASE_CONFIG_ATTR_UMP_DEVICE,
		UMP_DEVICE_Z_SHIFT
	},
#endif /* CONFIG_UMP == 1 */
	{
		KBASE_CONFIG_ATTR_MEMORY_OS_SHARED_MAX,
		2048 * 1024 * 1024UL /* 2048MB */
	},

	{
		KBASE_CONFIG_ATTR_MEMORY_OS_SHARED_PERF_GPU,
		KBASE_MEM_PERF_FAST
	},
	{
		KBASE_CONFIG_ATTR_POWER_MANAGEMENT_DVFS_FREQ,
		KBASE_PM_DVFS_FREQUENCY
	},
	{
		KBASE_CONFIG_ATTR_POWER_MANAGEMENT_CALLBACKS,
		(uintptr_t)&pm_callbacks
	},
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
		KBASE_CONFIG_ATTR_JS_RESET_TIMEOUT_MS,
		500 /* 500ms before canceling stuck jobs */
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

kbase_platform_config platform_config =
{
		.attributes                = config_attributes,
		.io_resources              = &io_resources,
};


#ifdef CONFIG_MALI_T6XX_RT_PM

struct mutex runtime_pm_lock;


static void kbase_device_runtime_workqueue_callback(struct work_struct *work)
{
  	int result;
  	struct kbase_device *kbdev;

  	kbdev = container_of(work, struct kbase_device, runtime_pm_workqueue.work);
  	/********************************************
  	 *
  	 *  This is workaround about occurred kernel panic when you turn off the system.
  	 *
  	 *  System kernel will call the "__pm_runtime_disable" when you turn off the system.
  	 *  After that function, System kernel do not run the runtimePM API more.
  	 *
  	 *  So, this code is check the "dev->power.disable_depth" value is not zero.
  	 *
  	********************************************/

  	if(kbdev->osdev.dev->power.disable_depth > 0)
  		return;

  	mutex_lock(&runtime_pm_lock);
  	result = pm_runtime_suspend(kbdev->osdev.dev);
  	kbase_platform_clock_off(kbdev);
  	mutex_unlock(&runtime_pm_lock);

  #if MALI_GATOR_SUPPORT
  	kbase_trace_mali_timeline_event(GATOR_MAKE_EVENT(ACTIVITY_RTPM_CHANGED, ACTIVITY_RTPM));
  #endif
  #if MALI_RTPM_DEBUG
  	printk( "kbase_device_runtime_workqueue_callback, usage_count=%d\n", atomic_read(&kbdev->osdev.dev->power.usage_count));
  #endif

  	if(result < 0 && result != -EAGAIN)
  		printk("pm_runtime_put_sync failed (%d)\n", result);
}

void kbase_device_runtime_init_workqueue(struct device *dev)
{
  	struct kbase_device *kbdev;
  	kbdev = dev_get_drvdata(dev);
  	INIT_DELAYED_WORK(&kbdev->runtime_pm_workqueue, kbase_device_runtime_workqueue_callback);
}
/** Disable runtime pm
 *
 * @param dev	The device to enable rpm
 *
 * @return A standard Linux error code
 */
void kbase_device_runtime_disable(struct kbase_device *kbdev)
{
  	pm_runtime_disable(kbdev->osdev.dev);
}

/** Initialize runtiem pm fields in given device
 *
 * @param dev	The device to initialize
 *
 * @return A standard Linux error code
 */

mali_error kbase_device_runtime_init(struct kbase_device *kbdev)
{
  	pm_suspend_ignore_children(kbdev->osdev.dev, true);
  	pm_runtime_enable(kbdev->osdev.dev);
  	kbase_device_runtime_init_workqueue(kbdev->osdev.dev);
  	mutex_init(&runtime_pm_lock);
  	return MALI_ERROR_NONE;
}

void kbase_device_runtime_get_sync(struct device *dev)
{
  	int result;
  	struct kbase_device *kbdev;
  	struct exynos_context *platform;
  	kbdev = dev_get_drvdata(dev);

  	platform = (struct exynos_context *) kbdev->platform_context;
  	if(!platform)
  		return;

  	/********************************************
  	 *
  	 *  This is workaround about occurred kernel panic when you turn off the system.
  	 *
  	 *  System kernel will call the "__pm_runtime_disable" when you turn off the system.
  	 *  After that function, System kernel do not run the runtimePM API more.
  	 *
  	 *  So, this code is check the "dev->power.disable_depth" value is not zero.
  	 *
  	********************************************/

  	if(dev->power.disable_depth > 0) {
  		if(platform->cmu_pmu_status == 0)
  			kbase_platform_cmu_pmu_control(kbdev, 1);
  		return;
  	}

  	if(delayed_work_pending(&kbdev->runtime_pm_workqueue)) {
  		cancel_delayed_work_sync(&kbdev->runtime_pm_workqueue);
  	}

  	mutex_lock(&runtime_pm_lock);
  	kbase_platform_clock_on(kbdev);
  	pm_runtime_get_noresume(dev);
  	result = pm_runtime_resume(dev);
  	mutex_unlock(&runtime_pm_lock);

  #if MALI_GATOR_SUPPORT
  	kbase_trace_mali_timeline_event(GATOR_MAKE_EVENT(ACTIVITY_RTPM_CHANGED, ACTIVITY_RTPM) | 1);
  #endif
  #if MALI_RTPM_DEBUG
  	printk( "kbase_device_runtime_get_sync, usage_count=%d\n", atomic_read(&dev->power.usage_count));
  #endif

  	/********************************************
  	 *
  	 *  This check is re-confirm about maybe context switch by another cpu when turn off the system.
  	 *
  	 *  runtimePM put_sync -------- runtimePM get_sync -------- runtimePM put_sync          : CPU 0
  	 *                                      \
  	 *                                       \ ( context running by another core. )
  	 *                                        \
  	 *                                         - (turn off the system) runtimePM disable    : CPU 1
  	 *                                                                    \
  	 *                                                                     \
  	 *                                                                      => do not success implement runtimePM API
  	********************************************/
  	if(result < 0 && result == -EAGAIN)
  		kbase_platform_cmu_pmu_control(kbdev, 1);
  	else if(result < 0)
  		printk(KERN_ERR "pm_runtime_get_sync failed (%d)\n", result);
}

void kbase_device_runtime_put_sync(struct device *dev)
{
  	struct kbase_device *kbdev;
  	kbdev = dev_get_drvdata(dev);

  	if(delayed_work_pending(&kbdev->runtime_pm_workqueue)) {
  		cancel_delayed_work_sync(&kbdev->runtime_pm_workqueue);
  	}

  	pm_runtime_put_noidle(kbdev->osdev.dev);
  	schedule_delayed_work_on(0, &kbdev->runtime_pm_workqueue, RUNTIME_PM_DELAY_TIME/(1000/HZ));
  #if MALI_RTPM_DEBUG
  	 printk( "---kbase_device_runtime_put_sync, usage_count=%d\n", atomic_read(&kbdev->osdev.dev->power.usage_count));
  #endif
}
#endif

