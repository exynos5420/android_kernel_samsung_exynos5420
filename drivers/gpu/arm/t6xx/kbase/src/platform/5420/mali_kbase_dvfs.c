/* drivers/gpu/t6xx/kbase/src/platform/mali_kbase_dvfs.c
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
 * @file mali_kbase_dvfs.c
 * DVFS
 */

#include <kbase/src/common/mali_kbase.h>
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
#include <linux/pm_qos.h>
#include <linux/pm_runtime.h>

#include <mach/bts.h>
#include <mach/map.h>
#include <linux/fb.h>
#include <linux/clk.h>
#include <mach/regs-clock.h>
#include <asm/delay.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>

#include <kbase/src/platform/mali_kbase_platform.h>
#include <kbase/src/platform/mali_kbase_dvfs.h>

#include <kbase/src/common/mali_kbase_gator.h>
#ifdef CONFIG_EXYNOS5_CPUFREQ
#include <mach/cpufreq.h>
#endif

#ifdef CONFIG_MALI_DEVFREQ
#include <linux/devfreq.h>
#endif

#include <mach/exynos5_bus.h>

#include <mach/sec_debug.h>

#ifdef MALI_DVFS_ASV_ENABLE
#include <mach/asv-exynos.h>
#define ASV_STATUS_INIT 1
#define ASV_STATUS_NOT_INIT 0
#define ASV_STATUS_DISABLE_REQ 2

#define ASV_CMD_DISABLE	-1
#define ASV_CMD_ENABLE 0
#endif

#if defined(CONFIG_EXYNOS_THERMAL)
#include <mach/tmu.h>
#define GPU_MAX_CLK 480
#define GPU_THROTTLING_90_95 480
#define GPU_THROTTLING_95_100 420
#define GPU_THROTTLING_100_105 350
#define GPU_THROTTLING_105_110 177
#define GPU_TRIPPING_110 100
#endif

#define COLD_MINIMUM_VOL 950000

#ifdef CONFIG_REGULATOR
static struct regulator *g3d_regulator;
#endif

#ifdef CONFIG_MALI_DEVFREQ
#define MALI_DEVFREQ_POLLING_MS 0
static struct devfreq *mali_devfreq;
#endif

#ifdef MALI_DEBUG
#define DEBUG_PRINT_INFO printk
#else
#define DEBUG_PRINT_INFO
#endif

#if defined(CONFIG_ARM_EXYNOS5420_BUS_DEVFREQ)
static struct pm_qos_request exynos5_g3d_mif_qos;
static struct pm_qos_request exynos5_g3d_int_qos;
static struct pm_qos_request exynos5_g3d_cpu_qos;
#endif

/***********************************************************/
/*  This table and variable are using the check time share of GPU Clock  */
/***********************************************************/

typedef struct _mali_dvfs_info{
	unsigned int voltage;
	unsigned int clock;
	int min_threshold;
	int	max_threshold;
	unsigned long long time;
	int mem_freq;
	int int_freq;
	int cpu_freq;
} mali_dvfs_info;

static mali_dvfs_info mali_dvfs_infotbl[] = {
#ifndef CONFIG_SUPPORT_WQXGA
	{812500, 100, 0, 90, 0, 160000, 83000, 250000},
	{812500, 177, 53, 90, 0, 160000, 83000, 250000},
#else
	{812500, 177, 0, 90, 0, 160000, 83000, 250000},
#endif /* CONFIG_SUPPORT_WQXGA */
	{862500, 266, 60, 90, 0, 400000, 222000, 250000},
	{912500, 350, 70, 90, 0, 667000, 333000, 250000},
	{962500, 420, 78, 99, 0, 800000, 400000, 250000},
	{1000000, 480, 98, 100, 0, 800000, 400000, 650000},
	{1037500, 533, 99, 100, 0, 800000, 400000, 1200000},
};

#define MALI_DVFS_STEP	ARRAY_SIZE(mali_dvfs_infotbl)

#ifdef CONFIG_MALI_T6XX_DVFS
typedef struct _mali_dvfs_status_type{
	kbase_device *kbdev;
	int step;
	int utilisation;
#ifdef CONFIG_MALI_T6XX_FREQ_LOCK
	int max_lock;
	int min_lock;
	int user_max_lock[NUMBER_LOCK];
	int user_min_lock[NUMBER_LOCK];
#endif
#ifdef MALI_DVFS_ASV_ENABLE
	int asv_status;
#endif
} mali_dvfs_status;

static struct workqueue_struct *mali_dvfs_wq = 0;
spinlock_t mali_dvfs_spinlock;
struct mutex mali_set_clock_lock;
struct mutex mali_enable_clock_lock;
#ifdef CONFIG_MALI_T6XX_DEBUG_SYS
static void update_time_in_state(int level);
#endif

/*dvfs status*/
static mali_dvfs_status mali_dvfs_status_current;
#ifdef MALI_DVFS_ASV_ENABLE
static const unsigned int mali_dvfs_vol_default[] = { 812500, 812500, 862500, 912500, 962500, 1000000, 1037500};


static int mali_dvfs_update_asv(int cmd)
{
	int i;
	int voltage = 0;

	if (cmd == ASV_CMD_DISABLE) {
		for (i = 0; i < MALI_DVFS_STEP; i++) {
#ifdef CONFIG_SUPPORT_WQXGA
			mali_dvfs_infotbl[i].voltage = mali_dvfs_vol_default[i+1];
#else
			mali_dvfs_infotbl[i].voltage = mali_dvfs_vol_default[i];
#endif  /* CONFIG_SUPPORT_WQXGA */
		}
		printk("mali_dvfs_update_asv use default table\n");
		return ASV_STATUS_INIT;
	}
	for (i = 0; i < MALI_DVFS_STEP; i++) {
		voltage = get_match_volt(ID_G3D, mali_dvfs_infotbl[i].clock*1000);
		if (voltage == 0) {
			return ASV_STATUS_NOT_INIT;
		}

		printk(KERN_INFO "G3D %dKhz ASV is %duV\n",
					mali_dvfs_infotbl[i].clock*1000, voltage);

		mali_dvfs_infotbl[i].voltage = voltage;
	}

	return ASV_STATUS_INIT;
}
#endif

static void mali_dvfs_decide_next_level(mali_dvfs_status *dvfs_status)
{
	unsigned long flags;
	struct exynos_context *platform;

	platform = (struct exynos_context *)dvfs_status->kbdev->platform_context;
#ifdef MALI_DVFS_ASV_ENABLE
	if (dvfs_status->asv_status == ASV_STATUS_DISABLE_REQ) {
		dvfs_status->asv_status = mali_dvfs_update_asv(ASV_CMD_DISABLE);
	} else if (dvfs_status->asv_status == ASV_STATUS_NOT_INIT) {
		dvfs_status->asv_status = mali_dvfs_update_asv(ASV_CMD_ENABLE);
	}
#endif
	spin_lock_irqsave(&mali_dvfs_spinlock, flags);

#ifdef CONFIG_EXYNOS_THERMAL
	if (dvfs_status->step == kbase_platform_dvfs_get_level(GPU_MAX_CLK)) {
		dvfs_status->step--;
		goto skip;
	}
#endif

	if (dvfs_status->utilisation > mali_dvfs_infotbl[dvfs_status->step].max_threshold) {
#ifdef PLATFORM_UTILIZATION
		if (dvfs_status->step == kbase_platform_dvfs_get_level(500)) {
			if (platform->utilisation > mali_dvfs_infotbl[dvfs_status->step].max_threshold) {
				dvfs_status->step++;
				DVFS_ASSERT(dvfs_status->step < MALI_DVFS_STEP);
			}
		} else {
#endif
			dvfs_status->step++;
			DVFS_ASSERT(dvfs_status->step < MALI_DVFS_STEP);
#ifdef PLATFORM_UTILIZATION
		}
#endif
	} else if ((dvfs_status->step > 0) &&
			(platform->time_tick == MALI_DVFS_TIME_INTERVAL) &&
			(platform->utilisation < mali_dvfs_infotbl[dvfs_status->step].min_threshold)) {
		DVFS_ASSERT(dvfs_status->step > 0);
		dvfs_status->step--;
	}
#ifdef CONFIG_EXYNOS_THERMAL
skip:
#endif
#ifdef CONFIG_MALI_T6XX_FREQ_LOCK
	if (dvfs_status->min_lock > 0) {
		if (dvfs_status->step < dvfs_status->min_lock)
			dvfs_status->step = dvfs_status->min_lock;
	}

	if ((dvfs_status->max_lock >= 0) && (dvfs_status->step > dvfs_status->max_lock)) {
		dvfs_status->step = dvfs_status->max_lock;
	}
#endif
	spin_unlock_irqrestore(&mali_dvfs_spinlock, flags);

}

#ifdef CONFIG_MALI_DEVFREQ
extern int update_devfreq(struct devfreq *devfreq);
#endif

static void mali_dvfs_event_proc(struct work_struct *w)
{
#ifdef CONFIG_MALI_DEVFREQ
	if (mali_devfreq) {
		mutex_lock(&mali_devfreq->lock);
		update_devfreq(mali_devfreq);
		mutex_unlock(&mali_devfreq->lock);
	}
	return 0;
#else
	mali_dvfs_status *dvfs_status;
	mutex_lock(&mali_enable_clock_lock);
	dvfs_status = &mali_dvfs_status_current;

	mali_dvfs_decide_next_level(dvfs_status);

	if (!pm_runtime_status_suspended(dvfs_status->kbdev->osdev.dev))
		kbase_platform_dvfs_set_level(dvfs_status->kbdev, dvfs_status->step);

	mutex_unlock(&mali_enable_clock_lock);
#endif
}

static DECLARE_WORK(mali_dvfs_work, mali_dvfs_event_proc);

int kbase_platform_dvfs_event(struct kbase_device *kbdev, u32 utilisation)
{
	unsigned long flags;
	struct exynos_context *platform;

	KBASE_DEBUG_ASSERT(kbdev != NULL);
	platform = (struct exynos_context *) kbdev->platform_context;

	spin_lock_irqsave(&mali_dvfs_spinlock, flags);
	if (platform->time_tick < MALI_DVFS_TIME_INTERVAL) {
		platform->time_tick++;
		platform->time_busy += kbdev->pm.metrics.time_busy;
		platform->time_idle += kbdev->pm.metrics.time_idle;
	} else {
		platform->time_busy = kbdev->pm.metrics.time_busy;
		platform->time_idle = kbdev->pm.metrics.time_idle;
		platform->time_tick = 0;
	}

	if ((platform->time_tick == MALI_DVFS_TIME_INTERVAL) &&
		(platform->time_idle + platform->time_busy > 0))
			platform->utilisation = (100*platform->time_busy) / (platform->time_idle + platform->time_busy);

	mali_dvfs_status_current.utilisation = utilisation;

#ifdef MALI_DEBUG
	printk(KERN_INFO "\n[mali_devfreq]utilization: %d\n", utilisation);
#endif
	spin_unlock_irqrestore(&mali_dvfs_spinlock, flags);

	queue_work_on(0, mali_dvfs_wq, &mali_dvfs_work);
	/*add error handle here*/
	return MALI_TRUE;
}

int kbase_platform_dvfs_get_utilisation(void)
{
	unsigned long flags;
	int utilisation = 0;

	spin_lock_irqsave(&mali_dvfs_spinlock, flags);
	utilisation = mali_dvfs_status_current.utilisation;
	spin_unlock_irqrestore(&mali_dvfs_spinlock, flags);

	return utilisation;
}

int kbase_platform_dvfs_get_enable_status(void)
{
	struct kbase_device *kbdev;
	unsigned long flags;
	int enable;

	kbdev = mali_dvfs_status_current.kbdev;
	spin_lock_irqsave(&kbdev->pm.metrics.lock, flags);
	enable = kbdev->pm.metrics.timer_active;
	spin_unlock_irqrestore(&kbdev->pm.metrics.lock, flags);

	return enable;
}

int kbase_platform_dvfs_enable(bool enable, int freq)
{
	mali_dvfs_status *dvfs_status;
	struct kbase_device *kbdev;
	unsigned long flags;
	struct exynos_context *platform;
	int mif_qos, int_qos, cpu_qos;

	dvfs_status = &mali_dvfs_status_current;
	kbdev = mali_dvfs_status_current.kbdev;

	KBASE_DEBUG_ASSERT(kbdev != NULL);
	platform = (struct exynos_context *)kbdev->platform_context;

	mutex_lock(&mali_enable_clock_lock);

	if (freq != MALI_DVFS_CURRENT_FREQ) {
		spin_lock_irqsave(&mali_dvfs_spinlock, flags);
		platform->time_tick = 0;
		platform->time_busy = 0;
		platform->time_idle = 0;
		platform->utilisation = 0;
		dvfs_status->step = kbase_platform_dvfs_get_level(freq);
		spin_unlock_irqrestore(&mali_dvfs_spinlock, flags);

		if (freq == MALI_DVFS_START_FREQ) {
			if (dvfs_status->min_lock != -1)
				dvfs_status->step = MAX(dvfs_status->min_lock, dvfs_status->step);
			if (dvfs_status->max_lock != -1)
				dvfs_status->step = MIN(dvfs_status->max_lock, dvfs_status->step);
		}

		kbase_platform_dvfs_set_level(dvfs_status->kbdev, dvfs_status->step);
	}

	if (enable != kbdev->pm.metrics.timer_active) {
		if (enable) {
			spin_lock_irqsave(&kbdev->pm.metrics.lock, flags);
			kbdev->pm.metrics.timer_active = MALI_TRUE;
			spin_unlock_irqrestore(&kbdev->pm.metrics.lock, flags);
			hrtimer_start(&kbdev->pm.metrics.timer,
					HR_TIMER_DELAY_MSEC(KBASE_PM_DVFS_FREQUENCY),
					HRTIMER_MODE_REL);

			DVFS_ASSERT(dvfs_status->step >= 0);

			mif_qos = mali_dvfs_infotbl[dvfs_status->step].mem_freq;
			int_qos = mali_dvfs_infotbl[dvfs_status->step].int_freq;
			cpu_qos = mali_dvfs_infotbl[dvfs_status->step].cpu_freq;
#if defined(CONFIG_ARM_EXYNOS5420_BUS_DEVFREQ)
			pm_qos_update_request(&exynos5_g3d_mif_qos, mif_qos);
			pm_qos_update_request(&exynos5_g3d_int_qos, int_qos);
			pm_qos_update_request(&exynos5_g3d_cpu_qos, cpu_qos);
#endif
		} else {
			spin_lock_irqsave(&kbdev->pm.metrics.lock, flags);
			kbdev->pm.metrics.timer_active = MALI_FALSE;
			spin_unlock_irqrestore(&kbdev->pm.metrics.lock, flags);
			hrtimer_cancel(&kbdev->pm.metrics.timer);
#if defined(CONFIG_ARM_EXYNOS5420_BUS_DEVFREQ)
			pm_qos_update_request(&exynos5_g3d_mif_qos, 0);
			pm_qos_update_request(&exynos5_g3d_int_qos, 0);
			pm_qos_update_request(&exynos5_g3d_cpu_qos, 0);
#endif
		}
	}
	mutex_unlock(&mali_enable_clock_lock);

	return MALI_TRUE;
}

int kbase_platform_dvfs_init(struct kbase_device *kbdev)
{
	unsigned long flags;
	int i;
	/*default status
	  add here with the right function to get initilization value.
	 */
	if (!mali_dvfs_wq)
		mali_dvfs_wq = create_singlethread_workqueue("mali_dvfs");

	spin_lock_init(&mali_dvfs_spinlock);
	mutex_init(&mali_set_clock_lock);
	mutex_init(&mali_enable_clock_lock);

#if defined(CONFIG_ARM_EXYNOS5420_BUS_DEVFREQ)
	pm_qos_add_request(&exynos5_g3d_mif_qos, PM_QOS_BUS_THROUGHPUT, 0);
	pm_qos_add_request(&exynos5_g3d_int_qos, PM_QOS_DEVICE_THROUGHPUT, 0);
	pm_qos_add_request(&exynos5_g3d_cpu_qos, PM_QOS_CPU_FREQ_MIN, 0);
#endif

	/*add a error handling here*/
	spin_lock_irqsave(&mali_dvfs_spinlock, flags);
	mali_dvfs_status_current.kbdev = kbdev;
	mali_dvfs_status_current.utilisation = 100;
	mali_dvfs_status_current.step = MALI_DVFS_STEP-1;
#ifdef CONFIG_MALI_T6XX_FREQ_LOCK
	mali_dvfs_status_current.max_lock = -1;
	mali_dvfs_status_current.min_lock = -1;
	for (i = 0; i < NUMBER_LOCK; i++) {
		mali_dvfs_status_current.user_max_lock[i] = -1;
		mali_dvfs_status_current.user_min_lock[i] = -1;
	}
#endif
#ifdef MALI_DVFS_ASV_ENABLE
	mali_dvfs_status_current.asv_status = ASV_STATUS_NOT_INIT;
#endif
	spin_unlock_irqrestore(&mali_dvfs_spinlock, flags);

	return MALI_TRUE;
}

void kbase_platform_dvfs_term(void)
{
	if (mali_dvfs_wq)
		destroy_workqueue(mali_dvfs_wq);

#if defined(CONFIG_ARM_EXYNOS5420_BUS_DEVFREQ)
	pm_qos_remove_request(&exynos5_g3d_mif_qos);
	pm_qos_remove_request(&exynos5_g3d_int_qos);
	pm_qos_remove_request(&exynos5_g3d_cpu_qos);
#endif

	mali_dvfs_wq = NULL;
}
#endif /*CONFIG_MALI_T6XX_DVFS*/

int mali_get_dvfs_max_locked_freq(void)
{
	int locked_level = -1;

#ifdef CONFIG_MALI_T6XX_FREQ_LOCK
	unsigned long flags;
	if (mali_dvfs_status_current.max_lock < 0)
		return locked_level;


	spin_lock_irqsave(&mali_dvfs_spinlock, flags);
	locked_level = mali_dvfs_infotbl[mali_dvfs_status_current.max_lock].clock;
	spin_unlock_irqrestore(&mali_dvfs_spinlock, flags);
#endif
	return locked_level;
}

int mali_get_dvfs_min_locked_freq(void)
{
	int locked_level = -1;

#ifdef CONFIG_MALI_T6XX_FREQ_LOCK
	unsigned long flags;
	if (mali_dvfs_status_current.min_lock < 0)
		return locked_level;

	spin_lock_irqsave(&mali_dvfs_spinlock, flags);
	locked_level = mali_dvfs_infotbl[mali_dvfs_status_current.min_lock].clock;
	spin_unlock_irqrestore(&mali_dvfs_spinlock, flags);
#endif
	return locked_level;
}

int mali_get_dvfs_current_level(void)
{
	int current_level = -1;

#ifdef CONFIG_MALI_T6XX_FREQ_LOCK
	unsigned long flags;

	spin_lock_irqsave(&mali_dvfs_spinlock, flags);
	current_level = mali_dvfs_status_current.step;
	spin_unlock_irqrestore(&mali_dvfs_spinlock, flags);
#endif
	return current_level;
}

int mali_dvfs_freq_max_lock(int level, gpu_lock_type user_lock)
{
#ifdef CONFIG_MALI_T6XX_FREQ_LOCK
	unsigned long flags;
	int i, step = 0;

	spin_lock_irqsave(&mali_dvfs_spinlock, flags);

	step = mali_get_dvfs_step();
	if (step-1 < level) {
		spin_unlock_irqrestore(&mali_dvfs_spinlock, flags);
		return -1;
	}

	if (user_lock < TMU_LOCK || user_lock >= NUMBER_LOCK) {
		spin_unlock_irqrestore(&mali_dvfs_spinlock, flags);
		return -1;
	}

	mali_dvfs_status_current.user_max_lock[user_lock] = level;
	mali_dvfs_status_current.max_lock = level;

	if (mali_dvfs_status_current.max_lock != -1) {
		for (i = 0; i < NUMBER_LOCK; i++)
			if (mali_dvfs_status_current.user_max_lock[i] != -1)
				mali_dvfs_status_current.max_lock = MIN(mali_dvfs_status_current.max_lock, mali_dvfs_status_current.user_max_lock[i]);
	} else {
		mali_dvfs_status_current.max_lock = level;
	}

	spin_unlock_irqrestore(&mali_dvfs_spinlock, flags);

	printk("[G3D] Lock max clk: %d\n", mali_dvfs_infotbl[level].clock);
#endif
	return 0;
}
void mali_dvfs_freq_max_unlock(gpu_lock_type user_lock)
{
#ifdef CONFIG_MALI_T6XX_FREQ_LOCK
	unsigned long flags;
	int i;
	bool dirty = false;

	spin_lock_irqsave(&mali_dvfs_spinlock, flags);

	if (user_lock < TMU_LOCK || user_lock >= NUMBER_LOCK) {
		spin_unlock_irqrestore(&mali_dvfs_spinlock, flags);
		return;
	}

	mali_dvfs_status_current.user_max_lock[user_lock] = -1;
	mali_dvfs_status_current.max_lock = kbase_platform_dvfs_get_level(GPU_MAX_CLK);

	for (i = 0; i < NUMBER_LOCK; i++) {
		if (mali_dvfs_status_current.user_max_lock[i] != -1) {
			dirty = true;
			mali_dvfs_status_current.max_lock = MIN(mali_dvfs_status_current.user_max_lock[i], mali_dvfs_status_current.max_lock);
		}
	}

	if (!dirty)
		mali_dvfs_status_current.max_lock = -1;

	spin_unlock_irqrestore(&mali_dvfs_spinlock, flags);
#endif

	printk("[G3D] Unlock max clk\n");
}

int mali_dvfs_freq_min_lock(int level, gpu_lock_type user_lock)
{
#ifdef CONFIG_MALI_T6XX_FREQ_LOCK
	unsigned long flags;
	int i, step = 0;

	spin_lock_irqsave(&mali_dvfs_spinlock, flags);

	step = mali_get_dvfs_step();
	if (step-1 < level) {
		spin_unlock_irqrestore(&mali_dvfs_spinlock, flags);
		return -1;
	}

	if (user_lock < TMU_LOCK || user_lock >= NUMBER_LOCK) {
		spin_unlock_irqrestore(&mali_dvfs_spinlock, flags);
		return -1;
	}

	mali_dvfs_status_current.user_min_lock[user_lock] = level;
	mali_dvfs_status_current.min_lock = level;

	if (mali_dvfs_status_current.min_lock != -1) {
		for (i = 0; i < NUMBER_LOCK; i++)
			if (mali_dvfs_status_current.user_min_lock[i] != -1)
				mali_dvfs_status_current.min_lock = MIN(mali_dvfs_status_current.min_lock, mali_dvfs_status_current.user_min_lock[i]);
	} else {
		mali_dvfs_status_current.min_lock = level;
	}

	spin_unlock_irqrestore(&mali_dvfs_spinlock, flags);

	printk("[G3D] Lock min clk: %d\n", mali_dvfs_infotbl[level].clock);
#endif
	return 0;
}
void mali_dvfs_freq_min_unlock(gpu_lock_type user_lock)
{
#ifdef CONFIG_MALI_T6XX_FREQ_LOCK
	unsigned long flags;
	int i;
	bool dirty = false;

	spin_lock_irqsave(&mali_dvfs_spinlock, flags);

	if (user_lock < TMU_LOCK || user_lock >= NUMBER_LOCK) {
		spin_unlock_irqrestore(&mali_dvfs_spinlock, flags);
		return;
	}

	mali_dvfs_status_current.user_min_lock[user_lock] = -1;
	mali_dvfs_status_current.min_lock = kbase_platform_dvfs_get_level(GPU_MAX_CLK);

	for (i = 0; i < NUMBER_LOCK; i++) {
		if (mali_dvfs_status_current.user_min_lock[i] != -1) {
			dirty = true;
			mali_dvfs_status_current.min_lock = MIN(mali_dvfs_status_current.min_lock, mali_dvfs_status_current.user_min_lock[i]);
		}
	}

	if (!dirty)
		mali_dvfs_status_current.min_lock = -1;

	spin_unlock_irqrestore(&mali_dvfs_spinlock, flags);
#endif
	printk("[G3D] Unlock min clk\n");
}

int kbase_platform_regulator_init(void)
{

#ifdef CONFIG_REGULATOR
	int mali_gpu_vol = 0;
	g3d_regulator = regulator_get(NULL, "vdd_g3d");
	if (IS_ERR(g3d_regulator)) {
		printk("[kbase_platform_regulator_init] failed to get mali t6xx regulator, 0x%p\n", g3d_regulator);
		g3d_regulator = NULL;
		return -1;
	}

	if (regulator_enable(g3d_regulator) != 0) {
		printk("[kbase_platform_regulator_init] failed to enable mali t6xx regulator\n");
		g3d_regulator = NULL;
		return -1;
	}
#ifdef MALI_DVFS_ASV_ENABLE
	mali_gpu_vol = get_match_volt(ID_G3D, MALI_DVFS_BL_CONFIG_FREQ*1000);
#endif
	if (mali_gpu_vol == 0)
		mali_gpu_vol = mali_dvfs_infotbl[ARRAY_SIZE(mali_dvfs_infotbl)-1].voltage;

	if (regulator_set_voltage(g3d_regulator, mali_gpu_vol, mali_gpu_vol) != 0) {
		printk("[kbase_platform_regulator_init] failed to set mali t6xx operating voltage [%d]\n", mali_gpu_vol);
		return -1;
	}
#endif
	return 0;
}

int kbase_platform_regulator_disable(void)
{
#ifdef CONFIG_REGULATOR
	if (!g3d_regulator) {
		printk("[kbase_platform_regulator_disable] g3d_regulator is not initialized\n");
		return -1;
	}

	if (regulator_disable(g3d_regulator) != 0) {
		printk("[kbase_platform_regulator_disable] failed to disable g3d regulator\n");
		return -1;
	}
#endif
	return 0;
}

int kbase_platform_regulator_enable(void)
{
#ifdef CONFIG_REGULATOR
	if (!g3d_regulator) {
		printk("[kbase_platform_regulator_enable] g3d_regulator is not initialized\n");
		return -1;
	}

	if (regulator_enable(g3d_regulator) != 0) {
		printk("[kbase_platform_regulator_enable] failed to enable g3d regulator\n");
		return -1;
	}
#endif
	return 0;
}

int kbase_platform_get_voltage(struct device *dev, int *vol)
{
#ifdef CONFIG_REGULATOR
	if (!g3d_regulator) {
		printk("[kbase_platform_get_voltage] g3d_regulator is not initialized\n");
		return -1;
	}

	*vol = regulator_get_voltage(g3d_regulator);
#else
	*vol = 0;
#endif
	return 0;
}

int kbase_platform_set_voltage(struct device *dev, int vol)
{
#ifdef CONFIG_REGULATOR
	if (!g3d_regulator) {
		printk("[kbase_platform_set_voltage] g3d_regulator is not initialized\n");
		return -1;
	}

	if (regulator_set_voltage(g3d_regulator, vol, vol) != 0) {
		printk("[kbase_platform_set_voltage] failed to set voltage\n");
		return -1;
	}
#endif
	return 0;
}

void kbase_set_power_margin(int volt_offset)
{
	int getVol;

#ifdef CONFIG_MALI_T6XX_DVFS
	mutex_lock(&mali_set_clock_lock);
#endif
	kbase_platform_get_voltage(NULL, &getVol);
	if (volt_offset) {
		if (volt_offset != gpu_voltage_margin) {
			if (getVol-gpu_voltage_margin+volt_offset <= COLD_MINIMUM_VOL) {
				kbase_platform_set_voltage(NULL, COLD_MINIMUM_VOL);
				pr_info("we set the voltage : %d\n", COLD_MINIMUM_VOL);
			}
			else {
				kbase_platform_set_voltage(NULL, getVol-gpu_voltage_margin+volt_offset);
				pr_info("we set the voltage : %d\n", getVol-gpu_voltage_margin+volt_offset);
			}
			gpu_voltage_margin = volt_offset;
		}
	} else {
		if (gpu_voltage_margin) {
			kbase_platform_set_voltage(NULL, getVol-gpu_voltage_margin);
			pr_info("we set the voltage : %d\n", getVol-gpu_voltage_margin);
			gpu_voltage_margin = 0;
		}
	}
#ifdef CONFIG_MALI_T6XX_DVFS
	mutex_unlock(&mali_set_clock_lock);
#endif
}

#if defined(CONFIG_EXYNOS_THERMAL)
int kbase_tmu_hot_check_and_work(unsigned long event)
{
	struct kbase_device *kbdev;
	mali_dvfs_status *dvfs_status;
	struct exynos_context *platform;
	unsigned int clkrate;
	int lock_level;
#ifdef CONFIG_MALI_T6XX_DVFS
	dvfs_status = &mali_dvfs_status_current;
	kbdev = dvfs_status->kbdev;

	KBASE_DEBUG_ASSERT(kbdev != NULL);

	platform = (struct exynos_context *) kbdev->platform_context;
	if (!platform)
		return -ENODEV;

	if (!platform->aclk_g3d)
		return -ENODEV;

	clkrate = clk_get_rate(platform->aclk_g3d);

	switch(event) {
		case GPU_THROTTLING1:
			lock_level = GPU_THROTTLING_90_95;
			printk("[G3D] GPU_THROTTLING_90_95\n");
			break;
		case GPU_THROTTLING2:
			lock_level = GPU_THROTTLING_95_100;
			printk("[G3D] GPU_THROTTLING_95_100\n");
			break;
		case GPU_THROTTLING3:
			lock_level = GPU_THROTTLING_100_105;
			printk("[G3D] GPU_THROTTLING_100_105\n");
			break;
		case GPU_THROTTLING4:
			lock_level = GPU_THROTTLING_105_110;
			printk("[G3D] GPU_THROTTLING_105_110\n");
			break;
		case GPU_TRIPPING:
			lock_level = GPU_TRIPPING_110;
			printk("[G3D] GPU_THROTTLING_110\n");
		default:
			return 0;
	}

	mali_dvfs_freq_max_lock(kbase_platform_dvfs_get_level(lock_level), TMU_LOCK);
#endif
	return 0;
}

void kbase_tmu_normal_work(void)
{
#ifdef CONFIG_MALI_T6XX_DVFS
	mali_dvfs_freq_max_unlock(TMU_LOCK);
#endif
}
#endif

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

void kbase_platform_dvfs_set_clock(kbase_device *kbdev, int freq)
{
#if defined(CONFIG_SUPPORT_WQXGA)
	struct clk *mout_dpll = NULL;
	struct clk *mout_vpll = NULL;
	struct clk *fout_vpll = NULL;
	struct clk *aclk_g3d_dout = NULL;
	struct clk *parent= NULL;
	static long vpll_rate_prev = -1;
	unsigned long vpll_rate = freq * 1000000;
	unsigned long aclk_rate = freq * 1000000;

	unsigned long tmp = 0;
	struct exynos_context *platform;

	if (!kbdev)
		panic("oops");

	platform = (struct exynos_context *) kbdev->platform_context;
	if (NULL == platform) {
		panic("oops");
	}

	if (platform->aclk_g3d == 0)
		return;

	aclk_g3d_dout = clk_get(kbdev->osdev.dev, "aclk_g3d_dout");
	if (IS_ERR(aclk_g3d_dout)) {
		printk(KERN_ERR "[kbase_platform_dvfs_set_clock] failed to clk_get [aclk_g3d_dout] = %ld\n", aclk_rate);
		return;
	}

	/* if changed the VPLL rate, set rate for VPLL and wait for lock time */
	if (vpll_rate != vpll_rate_prev) {
		mout_dpll = clk_get(kbdev->osdev.dev, "mout_dpll");
		mout_vpll = clk_get(kbdev->osdev.dev, "mout_vpll");
		fout_vpll = clk_get(kbdev->osdev.dev, "fout_vpll");
		if (IS_ERR(mout_dpll) || IS_ERR(mout_vpll) || IS_ERR(fout_vpll)) {
			printk(KERN_ERR "[kbase_platform_dvfs_set_clock] failed to clk_get ext_xtal, aclk_g3d_sw or fout_vpll\n");
			return;
		}

		/*for stable clock input.*/
		clk_set_rate(aclk_g3d_dout, get_dpll_freq(clk_get_rate(platform->aclk_g3d)/1000000,freq));
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
	unsigned long vpll_rate = freq * 1000000;
	unsigned long aclk_rate = freq * 1000000;

	unsigned long tmp = 0;
	struct exynos_context *platform;

	if (!kbdev)
		panic("oops");

	platform = (struct exynos_context *) kbdev->platform_context;
	if (NULL == platform) {
		panic("oops");
	}

	if (platform->aclk_g3d == 0)
		return;

	aclk_g3d_dout = clk_get(kbdev->osdev.dev, "aclk_g3d_dout");
	if (IS_ERR(aclk_g3d_dout)) {
		printk(KERN_ERR "[kbase_platform_dvfs_set_clock] failed to clk_get [aclk_g3d_dout] = %ld\n", aclk_rate);

		return;
	}

	/* if changed the VPLL rate, set rate for VPLL and wait for lock time */
	if (vpll_rate != vpll_rate_prev) {
		ext_xtal = clk_get(kbdev->osdev.dev, "ext_xtal");
		aclk_g3d_sw = clk_get(kbdev->osdev.dev, "aclk_g3d_sw");
		fout_vpll = clk_get(kbdev->osdev.dev, "fout_vpll");
		if (IS_ERR(ext_xtal) || IS_ERR(aclk_g3d_sw) || IS_ERR(fout_vpll)) {
			printk(KERN_ERR "[kbase_platform_dvfs_set_clock] failed to clk_get ext_xtal, aclk_g3d_sw or fout_vpll\n");
			return;
		}

		/*for stable clock input.*/
		clk_set_rate(aclk_g3d_dout, 100000000);
		clk_set_parent(platform->aclk_g3d, ext_xtal);

		/*change vpll*/
		clk_set_rate(fout_vpll, vpll_rate);

		/*restore parent*/
		clk_set_parent(platform->aclk_g3d, aclk_g3d_sw);
		vpll_rate_prev = vpll_rate;
	}
#endif

	clk_set_rate(aclk_g3d_dout, aclk_rate);

	/* Waiting for clock is stable */
	do {
		tmp = __raw_readl(EXYNOS5_CLKDIV_STAT_TOP2);
	} while (tmp & 0x10000);

#ifdef MALI_DEBUG
	DEBUG_PRINT_INFO("===clock set: %ld\n", aclk_rate);
	DEBUG_PRINT_INFO("===clock get: %ld\n", clk_get_rate(platform->aclk_g3d));
	DEBUG_PRINT_INFO("===clock get: %ld\n", clk_get_rate(aclk_g3d_dout));
#endif

	return;
}

static void kbase_platform_dvfs_set_vol(unsigned int vol)
{
	static int _vol = -1;

#ifdef MALI_DEBUG
	int gotvol = -1;
#endif

	if (_vol == vol)
		return;

	kbase_platform_set_voltage(NULL, vol);
	_vol = vol;

#ifdef MALI_DEBUG
	DEBUG_PRINT_INFO("***set voltage:%d\n", vol);
	kbase_platform_get_voltage(NULL, &gotvol);
	DEBUG_PRINT_INFO("***get voltage:%d\n", gotvol);
#endif

	return;
}

int kbase_platform_dvfs_get_level(int freq)
{
	int i;
	for (i = 0; i < MALI_DVFS_STEP; i++) {
		if (mali_dvfs_infotbl[i].clock == freq)
		return i;
	}

	return -1;
}

void kbase_platform_dvfs_set_level(kbase_device *kbdev, int level)
{
	static int prev_level = -1;
	int mif_qos, int_qos, cpu_qos;

#ifdef MALI_DEBUG
	printk(KERN_INFO "\n[mali_devfreq]dvfs level:%d\n", level);
#endif
	if (level == prev_level)
		return;

	if (WARN_ON((level >= MALI_DVFS_STEP) || (level < 0)))
		panic("invalid level");

#ifdef CONFIG_MALI_T6XX_DVFS
	mutex_lock(&mali_set_clock_lock);
#endif

	sec_debug_aux_log(SEC_DEBUG_AUXLOG_CPU_BUS_CLOCK_CHANGE,
			"old:%7d new:%7d (G3D)",
			mali_dvfs_infotbl[prev_level].clock*1000, 
			mali_dvfs_infotbl[level].clock*1000);

	mif_qos = mali_dvfs_infotbl[level].mem_freq;
	int_qos = mali_dvfs_infotbl[level].int_freq;
	cpu_qos = mali_dvfs_infotbl[level].cpu_freq;

	if (level > prev_level) {
#if defined(CONFIG_ARM_EXYNOS5420_BUS_DEVFREQ)
		pm_qos_update_request(&exynos5_g3d_mif_qos, mif_qos);
		pm_qos_update_request(&exynos5_g3d_int_qos, int_qos);
		pm_qos_update_request(&exynos5_g3d_cpu_qos, cpu_qos);
#endif
		kbase_platform_dvfs_set_vol(mali_dvfs_infotbl[level].voltage + gpu_voltage_margin);
		kbase_platform_dvfs_set_clock(kbdev, mali_dvfs_infotbl[level].clock);
		bts_change_g3d_state(mali_dvfs_infotbl[level].clock);
	} else {
		bts_change_g3d_state(mali_dvfs_infotbl[level].clock);
		kbase_platform_dvfs_set_clock(kbdev, mali_dvfs_infotbl[level].clock);
		kbase_platform_dvfs_set_vol(mali_dvfs_infotbl[level].voltage + gpu_voltage_margin);
#if defined(CONFIG_ARM_EXYNOS5420_BUS_DEVFREQ)
		pm_qos_update_request(&exynos5_g3d_mif_qos, mif_qos);
		pm_qos_update_request(&exynos5_g3d_int_qos, int_qos);
		pm_qos_update_request(&exynos5_g3d_cpu_qos, cpu_qos);
#endif
	}
#if defined(CONFIG_MALI_T6XX_DEBUG_SYS) && defined(CONFIG_MALI_T6XX_DVFS)
	update_time_in_state(prev_level);
#endif
	prev_level = level;
#ifdef CONFIG_MALI_T6XX_DVFS
	mutex_unlock(&mali_set_clock_lock);
#endif
}

int kbase_platform_dvfs_sprint_avs_table(char *buf, size_t buf_size)
{
#ifdef MALI_DVFS_ASV_ENABLE
	int i, cnt = 0;
	if (buf == NULL)
		return 0;

	for (i = MALI_DVFS_STEP-1; i >= 0; i--) {
		cnt += snprintf(buf+cnt, buf_size-cnt, "%dMhz:%d\n",
				mali_dvfs_infotbl[i].clock, mali_dvfs_infotbl[i].voltage);
	}
	return cnt;
#else
	return 0;
#endif
}

int kbase_platform_dvfs_set(int enable)
{
#ifdef MALI_DVFS_ASV_ENABLE
	unsigned long flags;

	spin_lock_irqsave(&mali_dvfs_spinlock, flags);
	if (enable) {
		mali_dvfs_status_current.asv_status = ASV_STATUS_NOT_INIT;
	} else {
		mali_dvfs_status_current.asv_status = ASV_STATUS_DISABLE_REQ;
	}
	spin_unlock_irqrestore(&mali_dvfs_spinlock, flags);
#endif
	return 0;
}

#ifdef CONFIG_MALI_T6XX_DVFS
int mali_get_dvfs_step(void)
{
	return MALI_DVFS_STEP;
}

int mali_get_dvfs_clock(int level)
{
	return mali_dvfs_infotbl[level].clock;
}

int mali_get_dvfs_table(char *buf, size_t buf_size)
{
	int i, cnt = 0;
	if (buf == NULL)
		return 0;

	for (i = MALI_DVFS_STEP-1; i >= 0; i--)
		cnt += snprintf(buf+cnt, buf_size-cnt, " %d", mali_dvfs_infotbl[i].clock);
	return cnt;
}
#endif

#ifdef CONFIG_MALI_T6XX_DEBUG_SYS
#ifdef CONFIG_MALI_T6XX_DVFS
static void update_time_in_state(int level)
{
	u64 current_time;
	static u64 prev_time = 0;

	if (!kbase_platform_dvfs_get_enable_status())
		return;

	if (prev_time == 0)
		prev_time = get_jiffies_64();

	current_time = get_jiffies_64();
	mali_dvfs_infotbl[level].time += current_time-prev_time;
	prev_time = current_time;
}
#endif

ssize_t show_time_in_state(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct kbase_device *kbdev;
	ssize_t ret = 0;
	int i;

	kbdev = dev_get_drvdata(dev);

#ifdef CONFIG_MALI_T6XX_DVFS
	update_time_in_state(mali_dvfs_status_current.step);
#endif
	if (!kbdev)
		return -ENODEV;

	for (i = MALI_DVFS_STEP - 1; i >= 0; i--) {
		ret += snprintf(buf+ret, PAGE_SIZE-ret, "%d %llu\n",
				mali_dvfs_infotbl[i].clock,
				mali_dvfs_infotbl[i].time);
	}

	if (ret >= PAGE_SIZE - 1) {
		buf[PAGE_SIZE-2] = '\n';
		buf[PAGE_SIZE-1] = '\0';
		ret = PAGE_SIZE-1;
	}

	return ret;
}

ssize_t set_time_in_state(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int i;

	for (i = 0; i < MALI_DVFS_STEP; i++) {
		mali_dvfs_infotbl[i].time = 0;
	}

	return count;
}
#endif


#ifdef CONFIG_MALI_DEVFREQ
static int mali_governor_init(struct devfreq *df)
{
	DEBUG_PRINT_INFO("\n[mali_devfreq]mali_governor_init");
	return 0;
}

static int mali_governor_get_target_freq(struct devfreq *df, unsigned long *freq)
{
	mali_dvfs_status *dvfs_status;

	mutex_lock(&mali_enable_clock_lock);
	dvfs_status = &mali_dvfs_status_current;
	mali_dvfs_decide_next_level(dvfs_status);
	mutex_unlock(&mali_enable_clock_lock);

	*freq = dvfs_status->step;

	DEBUG_PRINT_INFO("\n[mali_devfreq] get_target_freq:%d", *freq);
	return 0;
}

static struct devfreq_governor exynos5_g3d_abs_governor = {
	.name = "absolute table",
	.get_target_freq = mali_governor_get_target_freq,
	.init = mali_governor_init,
};

static int mali_devfreq_target(struct device *dev, unsigned long *freq, u32 flags)
{
	mali_dvfs_status *dvfs_status;
	struct kbase_device *kbdev;

	mutex_lock(&mali_enable_clock_lock);

	dvfs_status = &mali_dvfs_status_current;
	kbdev = mali_dvfs_status_current.kbdev;

	KBASE_DEBUG_ASSERT(kbdev != NULL);

	kbase_platform_dvfs_set_level(dvfs_status->kbdev, *freq);
	mutex_unlock(&mali_enable_clock_lock);

	DEBUG_PRINT_INFO("\n[mali_devfreq] set_target:%d", *freq);
	return 0;
}

static struct devfreq_dev_profile exynos5_g3d_devfreq_profile = {
	.polling_ms = MALI_DEVFREQ_POLLING_MS,
	.target = mali_devfreq_target,
};

int mali_devfreq_add(struct kbase_device *kbdev)
{
	struct device *dev = kbdev->osdev.dev;

	mali_devfreq = devfreq_add_device(dev, &exynos5_g3d_devfreq_profile, &exynos5_g3d_abs_governor, NULL);
	if (mali_devfreq < 0)
	       return MALI_FALSE;

	DEBUG_PRINT_INFO("\n[mali_devfreq]mali_devfreq_add");
	return MALI_TRUE;
}

int mali_devfreq_remove(void)
{
	if (mali_devfreq)
	       devfreq_remove_device(mali_devfreq);

	DEBUG_PRINT_INFO("\n[mali_devfreq]mali_devfreq_remove");
	return MALI_TRUE;
}
#endif
