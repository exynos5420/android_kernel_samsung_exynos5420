/**
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2010, 2012 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

/**
 * @file mali_pmu_power_up_down.c
 */

#include <linux/version.h>
#include <linux/sched.h>
#include <linux/module.h>
#include "mali_osk.h"
#include "mali_kernel_common.h"
#include "mali_pmu.h"
#include "linux/mali/mali_utgard.h"

/* Mali PMU power up/down APIs */

int mali_pmu_powerup(void)
{
	struct mali_pmu_core *pmu = mali_pmu_get_global_pmu_core();

	MALI_DEBUG_PRINT(5, ("Mali PMU: Power up\n"));

	if (NULL == pmu)
	{
		return -ENXIO;
	}

	if (_MALI_OSK_ERR_OK != mali_pmu_powerup_all(pmu))
	{
		return -EFAULT;
	}

	return 0;
}

EXPORT_SYMBOL(mali_pmu_powerup);

int mali_pmu_powerdown(void)
{
	struct mali_pmu_core *pmu = mali_pmu_get_global_pmu_core();

	MALI_DEBUG_PRINT(5, ("Mali PMU: Power down\n"));

	if (NULL == pmu)
	{
		return -ENXIO;
	}

	if (_MALI_OSK_ERR_OK != mali_pmu_powerdown_all(pmu))
	{
		return -EFAULT;
	}

	return 0;
}

EXPORT_SYMBOL(mali_pmu_powerdown);
