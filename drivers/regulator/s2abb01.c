/*
 * s2abb01.c - driver for the s2abb01 regulator
 *
 * Copyright (C) 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/regmap.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/s2abb01.h>


static struct regmap_config s2abb01_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
};


static int s2abb01_i2c_probe(struct i2c_client *i2c,
			      const struct i2c_device_id *id)
{
	struct s2abb01_platform_data *pdata = i2c->dev.platform_data;
	struct s2abb01_dev *s2abb01;
	int ret = 0;

	if (pdata == NULL) {
		pr_err("%s: no pdata\n", __func__);
		return -ENODEV;
	}

	s2abb01 = kzalloc(sizeof(struct s2abb01_dev), GFP_KERNEL);
	if (s2abb01 == NULL)
		return -ENOMEM;

	i2c_set_clientdata(i2c, s2abb01);
	s2abb01->dev = &i2c->dev;
	s2abb01->i2c = i2c;
	s2abb01->suspend_on_ctrl = pdata->suspend_on_ctrl;

	s2abb01->regmap = regmap_init_i2c(i2c, &s2abb01_regmap_config);
	if (IS_ERR(s2abb01->regmap)) {
		ret = PTR_ERR(s2abb01->regmap);
		dev_err(&i2c->dev, "Failed to allocate register map: %d\n",
			ret);
		goto err;
	}
	return ret;

err:
	kfree(s2abb01);
	return ret;
}

static int s2abb01_i2c_remove(struct i2c_client *i2c)
{
	struct s2abb01_dev *s2abb01 = i2c_get_clientdata(i2c);

	regmap_exit(s2abb01->regmap);
	kfree(s2abb01);
	return 0;
}

static const struct i2c_device_id s2abb01_i2c_id[] = {
	{ "s2abb01", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, s2abb01_i2c_id);

#ifdef CONFIG_PM
static int s2abb01_suspend(struct device *dev)
{
	struct i2c_client *i2c = container_of(dev, struct i2c_client, dev);
	struct s2abb01_dev *s2abb01 = i2c_get_clientdata(i2c);

	if (s2abb01->suspend_on_ctrl) {
		int ret = 0;
		/* Turn On the Enable bit */
		ret = regmap_update_bits(s2abb01->regmap, S2ABB01_REG_BB_OUT,
			S2ABB01_BBLDO_EN_MASK, S2ABB01_BBLDO_EN_MASK);
		if (ret)
			pr_err("%s: eMMC BUCK i2c update fail.(%d)", __func__, ret);
		ret = regmap_update_bits(s2abb01->regmap, S2ABB01_REG_LDO_CTRL,
			S2ABB01_BBLDO_EN_MASK, S2ABB01_BBLDO_EN_MASK);
		if (ret)
			pr_err("%s: eMMC LDO i2c update fail.(%d)", __func__, ret);
	}

	return 0;
}

static int s2abb01_resume(struct device *dev)
{
	struct i2c_client *i2c = container_of(dev, struct i2c_client, dev);
	struct s2abb01_dev *s2abb01 = i2c_get_clientdata(i2c);

	if (s2abb01->suspend_on_ctrl) {
		int ret = 0;
		ret = regmap_update_bits(s2abb01->regmap, S2ABB01_REG_BB_OUT,
			S2ABB01_BBLDO_EN_MASK, 0x00);
		if (ret)
			pr_err("%s: eMMC BUCK i2c update fail.(%d)", __func__, ret);
		ret = regmap_update_bits(s2abb01->regmap, S2ABB01_REG_LDO_CTRL,
			S2ABB01_BBLDO_EN_MASK, 0x00);
		if (ret)
			pr_err("%s: eMMC LDO i2c update fail.(%d)", __func__, ret);
	}

	return 0;
}
#else
#define s2abb01_suspend	NULL
#define s2abb01_resume		NULL
#endif /* CONFIG_PM */

const struct dev_pm_ops s2abb01_pm = {
	.suspend = s2abb01_suspend,
	.resume = s2abb01_resume,
};

static struct i2c_driver s2abb01_i2c_driver = {
	.driver = {
		.name = "s2abb01",
		.owner = THIS_MODULE,
		.pm = &s2abb01_pm,
	},
	.probe = s2abb01_i2c_probe,
	.remove = s2abb01_i2c_remove,
	.id_table = s2abb01_i2c_id,
};

static int __init s2abb01_i2c_init(void)
{
	return i2c_add_driver(&s2abb01_i2c_driver);
}
/* init early so consumer devices can complete system boot */
subsys_initcall(s2abb01_i2c_init);

static void __exit s2abb01_i2c_exit(void)
{
	i2c_del_driver(&s2abb01_i2c_driver);
}
module_exit(s2abb01_i2c_exit);

MODULE_DESCRIPTION("Samsung s2abb01 regulator driver");
MODULE_LICENSE("GPL");
