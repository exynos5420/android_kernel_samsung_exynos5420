/* tc300k.h -- Linux driver for coreriver chip as touchkey
 *
 * Copyright (C) 2013 Samsung Electronics Co.Ltd
 * Author: Junkyeong Kim <jk0430.kim@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 */

#ifndef __LINUX_TC300K_H
#define __LINUX_TC300K_H

#define TC300K_NAME "tc300k"

struct tc300k_platform_data {
	int gpio_int;
	int gpio_sda;
	int gpio_scl;
	int (*power) (bool on);
	int (*power_isp) (bool on);
	int (*keyled) (bool on);
	int *keycode;
	int key_num;
	const char *fw_name;
	bool panel_connect;
	u8 fw_version;
	u8 sensing_ch_num;
};

#endif /* __LINUX_TC300K_H */
