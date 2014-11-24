/*
 * Copyright (C) 2013 Samsung Electronics. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#ifndef _TMD3782_PLATFORMDATA_H_
#define _TMD3782_PLATFORMDATA_H_

struct tmd3782_platform_data {
	int atime_ms;
	int dgf;
	int coef_b;
	int coef_c;
	int coef_d;
	int ct_coef;
	int ct_offset;
	int integration_cycle;
	int prox_default_thd_high;
	int prox_default_thd_low;
	int prox_rawdata_trim;
};
#endif
