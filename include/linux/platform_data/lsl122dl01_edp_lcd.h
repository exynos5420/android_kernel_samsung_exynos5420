
/*
 *
 * Samsung SoC LCD driver.
 *
 * Copyright (c) 2013 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/


#ifndef LSL122DL01_H
#define LSL122DL01_H


/**
 * struct 
 * @psr_default_hsync : lcd default setting hsync(hz)
 * @psr_default_settingdata : setting data to set default hsync
 *@dev : s5p_device_dp.dev
 */
struct lsl122dl01_platform_data {
#ifdef CONFIG_S5P_DP_PSR
	int psr_default_hfreq;
	int psr_default_hfreq_data;
	int psr_hfreq_data_max;
	int psr_hfreq_data_min;
#endif
	struct device *dev;
};

#endif
