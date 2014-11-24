/*
 * Interface of Samsung S2ABB01
 *
 * Copyright (C) 2013-2014 Samsung Electronics Co.,  Ltd.
 *              http://www.samsung.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __LINUX_REGULATOR_SAMSUNG_S2ABB01_H
#define	__LINUX_REGULATOR_SAMSUNG_S2ABB01_H

#include <linux/regulator/machine.h>

/* S2ABB01 registers */
enum S2ABB01_reg {
	S2ABB01_REG_ID,
	S2ABB01_REG_BB_CTRL,
	S2ABB01_REG_BB_OUT,
	S2ABB01_REG_LDO_CTRL,
	S2ABB01_REG_LDO_DSCH,
};

enum S2ABB01_ID {
	S2ABB01_BUCK,
	S2ABB01_LDO,
};

struct s2abb01_dev {
	struct device *dev;
	struct regmap *regmap;
	struct i2c_client *i2c;
	bool suspend_on_ctrl;
};

struct s2abb01_platform_data {
	struct s2abb01_regulator_data *regulators;
	int num_regulators;
	bool suspend_on_ctrl;
};

struct s2abb01_regulator_data {
	int id;
	struct regulator_init_data *initdata;
};

#define S2ABB01_BBLDO_EN_MASK 0x80

#endif	/* __LINUX_MFD_SAMSUNG_S2ABB01_H */
