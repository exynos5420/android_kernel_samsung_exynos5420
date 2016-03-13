/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2009-2012 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

/**
 * @file mali_platform.h
 * Platform specific Mali driver functions
 */

#include "mali_osk.h"

struct mali_pmu_core;

/** @brief Initialisation of MALI PMU
 * 
 * This is called from entry point of the driver in order to create and intialize the PMU resource
 *
 * @param resource it will be a pointer to a PMU resource
 * @param number_of_pp_cores Number of found PP resources in configuration
 * @param number_of_l2_caches Number of found L2 cache resources in configuration
 * @return The created PMU object, or NULL in case of failure.
 */
struct mali_pmu_core *mali_pmu_create(_mali_osk_resource_t *resource, u32 number_of_pp_cores, u32 number_of_l2_caches);

/** @brief It deallocates the PMU resource
 * 
 * This is called on the exit of the driver to terminate the PMU resource
 *
 * @param pmu Pointer to PMU core object to delete
 */
void mali_pmu_delete(struct mali_pmu_core *pmu);

/** @brief Reset PMU core
 *
 * @param pmu Pointer to PMU core object to reset
 * @return _MALI_OSK_ERR_OK on success, otherwise failure.
 */
_mali_osk_errcode_t mali_pmu_reset(struct mali_pmu_core *pmu);

/** @brief MALI GPU power down using MALI in-built PMU
 *
 * called to power down all cores
 *
 * @param pmu Pointer to PMU core object to power down
 * @return _MALI_OSK_ERR_OK on success otherwise, a suitable _mali_osk_errcode_t error.
 */
_mali_osk_errcode_t mali_pmu_powerdown_all(struct mali_pmu_core *pmu);


/** @brief MALI GPU power up using MALI in-built PMU
 * 
 * called to power up all cores
 *
 * @param pmu Pointer to PMU core object to power up
 * @return _MALI_OSK_ERR_OK on success otherwise, a suitable _mali_osk_errcode_t error.
 */
_mali_osk_errcode_t mali_pmu_powerup_all(struct mali_pmu_core *pmu);


/** @brief Retrieves the Mali PMU core object (if any)
 * 
 * @return The Mali PMU object, or NULL if no PMU exists.
 */
struct mali_pmu_core *mali_pmu_get_global_pmu_core(void);
