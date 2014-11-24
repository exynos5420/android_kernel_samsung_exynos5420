/*
 * Samsung Exynos5 SoC series FIMC-IS eeprom i2c driver
 *
 * exynos5 fimc-is eeprom i2c functions
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/regulator/consumer.h>
#include <linux/videodev2.h>
#include <linux/videodev2_exynos_camera.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <mach/regs-gpio.h>
#include <mach/regs-clock.h>
#include <mach/exynos-clock.h>
#include <plat/clock.h>
#include <plat/gpio-cfg.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-subdev.h>
#include <media/exynos_fimc_is_sensor.h>

#include "fimc-is-sec-i2c.h"

#define SEC_CAMERA_EEPROM_ID	11
#define SEC_CAMERA_EEPROM_NAME	"cameraeeprom"
struct i2c_client *camera_eeprom_i2c_client;

int fimc_is_sec_i2c_read(unsigned char *saddr, unsigned char *sbuf, int size)
{
	int ret;
	struct i2c_msg msg[2];

	ret = 0;

	if (!camera_eeprom_i2c_client) {
		printk(KERN_ERR "%s : Could not find client!!\n", __func__);
		return -ENODEV;
	}

	if (!camera_eeprom_i2c_client->adapter) {
		printk(KERN_ERR "%s : Could not find adapter!\n", __func__);
		return -ENODEV;
	}

	/* set eeprom address */
	msg[0].addr = camera_eeprom_i2c_client->addr;
	msg[0].flags = 0; /* write */
	msg[0].len = 2;
	msg[0].buf = saddr;

	/* 2. I2C operation for reading data. */
	msg[1].addr = camera_eeprom_i2c_client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = size;
	msg[1].buf = sbuf;

	ret = i2c_transfer(camera_eeprom_i2c_client->adapter, msg, 2);
	if (ret < 0) {
		printk(KERN_ERR "%s : i2c transfer fail\n", __func__);
	}
	return ret;
}
int fimc_is_sec_i2c_write(unsigned char *saddr, unsigned char *sbuf, int size)
{
	struct i2c_msg msg[1];
	int ret;

	ret = 0;

	if (!camera_eeprom_i2c_client) {
		printk(KERN_ERR "%s : Could not find client!!\n", __func__);
		return -ENODEV;
	}

	if (!camera_eeprom_i2c_client->adapter) {
		printk(KERN_ERR "%s : Could not find adapter!\n", __func__);
		return -ENODEV;
	}

	msg->addr = camera_eeprom_i2c_client->addr;
	msg->flags = 0;
	msg->len = size;
	msg->buf = sbuf;

	ret = i2c_transfer(camera_eeprom_i2c_client->adapter, msg, 1);
	if (ret < 0) {
		printk(KERN_ERR "%s : i2c transfer fail\n", __func__);
	}

	return ret;
}
static int __devinit fimc_is_sec_i2c_probe(struct i2c_client *client,
						const struct i2c_device_id *id)
{
	camera_eeprom_i2c_client = client;
	printk(KERN_INFO "%s : Probe success!!\n", __func__);
	printk(KERN_INFO "%s : Client addr %d\n", __func__, client->addr);
	return 0;
}
static int __devexit fimc_is_sec_i2c_remove(struct i2c_client *client)
{
	return 0;
}
static const struct i2c_device_id camera_eeprom_id[] = {
	{ SEC_CAMERA_EEPROM_NAME, 0 },
};
MODULE_DEVICE_TABLE(i2c, camera_eeprom_id);

static struct i2c_driver camera_eeprom_i2c_driver = {
	.driver = {
		.name	= SEC_CAMERA_EEPROM_NAME,
	},
	.id_table	= camera_eeprom_id,
	.probe		= fimc_is_sec_i2c_probe,
	.remove		= fimc_is_sec_i2c_remove,
};

static int __init fimc_is_sec_i2c_init(void)
{
	printk(KERN_INFO "%s : module init Enter\n", __func__);
	return i2c_add_driver(&camera_eeprom_i2c_driver);
}
static void __exit fimc_is_sec_i2c_exit(void)
{
	printk(KERN_INFO "%s : module exit\n", __func__);
	i2c_del_driver(&camera_eeprom_i2c_driver);
}

module_init(fimc_is_sec_i2c_init);
module_exit(fimc_is_sec_i2c_exit);
MODULE_AUTHOR("Donghyun Chang <dh348.chang@samsung.com>");
MODULE_DESCRIPTION("FIMC-IS EEPROM I2C driver");
MODULE_LICENSE("GPL");
