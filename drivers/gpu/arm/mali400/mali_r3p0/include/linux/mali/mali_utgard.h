/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2010, 2012 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef __MALI_UTGARD_H__
#define	__MALI_UTGARD_H__ 

/** @brief MALI GPU power down using MALI in-built PMU
 * 
 * called to power down all cores 
 */
int mali_pmu_powerdown(void);


/** @brief MALI GPU power up using MALI in-built PMU
 * 
 * called to power up all cores 
 */
int mali_pmu_powerup(void);

#endif /* __MALI_UTGARD_H__ */

