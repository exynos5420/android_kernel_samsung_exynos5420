/*
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * Header file for exynos scaler support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __EXYNOS_SCALER_H
#define __EXYNOS_SCALER_H __FILE__

#define SC_MAX_CLKSEL	5

enum scaler_id {
	SCID_UNKNOWN,
	SCID_AD,
	SCID_AR,
	SCID_HE,
	SCID_RH,
	SCID_VE
};

#define SCID_IS_AD(id)		((id) == SCID_AD)
#define SCID_IS_AR(id)		((id) == SCID_AR)
#define SCID_IS_HE(id)		((id) == SCID_HE)
#define SCID_IS_RH(id)		((id) == SCID_RH)
#define SCID_IS_VE(id)		((id) == SCID_VE)

struct exynos_scaler_platdata {
	int platid;
	unsigned int clk_rate;
	const char *gate_clk;
	const char *clk[SC_MAX_CLKSEL];
	const char *clksrc[SC_MAX_CLKSEL];
	int use_pclk;
	void *(*setup_clocks)(void);
	bool (*init_clocks)(void *p);
	void (*clean_clocks)(void *p);
};

extern struct exynos_scaler_platdata exynos5_scaler_pd;
extern struct exynos_scaler_platdata exynos5410_scaler_pd;
#endif
