/*
 * Samsung Exynos5 SoC series FIMC-IS eeprom i2c driver
 *
 *
 * Copyright (c) 2014 Samsung Electronics Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/i2c.h>
#include <linux/i2c-gpio.h>

#define EEPROM_SIZE	8*1024

int fimc_is_sec_i2c_read(unsigned char *saddr, unsigned char *sbuf, int size);
int fimc_is_sec_i2c_write(unsigned char *saddr, unsigned char *sbuf, int size);
