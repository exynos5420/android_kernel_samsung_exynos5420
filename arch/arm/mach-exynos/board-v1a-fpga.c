/*
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/regulator/consumer.h>

#include <plat/gpio-cfg.h>

#include <mach/gpio.h>
#include <mach/regs-gpio.h>

#include "board-universal5420.h"

#define I2C_BUS_ID_IRLED	23

static unsigned int fpga_gpio_table[][4] = {
	{GPIO_IRDA_IRQ,		0, 2, S3C_GPIO_PULL_NONE}, /* IRDA_IRQ */
	{GPIO_FPGA_CRESET_B,	1, 2, S3C_GPIO_PULL_NONE}, /* FPGA_CRESET_B */
	{GPIO_FPGA_CDONE,	0, 2, S3C_GPIO_PULL_NONE}, /* FPGA_CDONE */
	{GPIO_FPGA_SPI_SI,	1, 2, S3C_GPIO_PULL_NONE}, /* FPGA_SPI_SI */
	{GPIO_FPGA_SPI_CLK,	1, 2, S3C_GPIO_PULL_NONE}, /* FPGA_SPI_CLK */
	{GPIO_FPGA_SPI_EN,	1, 2, S3C_GPIO_PULL_NONE}, /* FPGA_SPI_EN */
};

void fpga_config_gpio_table(int array_size, unsigned int (*gpio_table)[4])
{
	u32 i, gpio;

	for (i = 0; i < array_size; i++) {
		gpio = gpio_table[i][0];
		s3c_gpio_setpull(gpio, gpio_table[i][3]);
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(gpio_table[i][1]));
		if (gpio_table[i][2] != 2)
			gpio_set_value(gpio, gpio_table[i][2]);
	}
};

static void irda_device_init(void)
{
	int ret;
	printk(KERN_ERR "%s called!\n", __func__);
	ret = gpio_request(GPIO_IRDA_IRQ, "irda_irq");
	if (ret) {
		printk(KERN_ERR "%s: gpio_request fail[%d], ret = %d\n",
			__func__, GPIO_IRDA_IRQ, ret);
		return;
	}
	s3c_gpio_cfgpin(GPIO_IRDA_IRQ, S3C_GPIO_INPUT);
	s3c_gpio_setpull(GPIO_IRDA_IRQ, S3C_GPIO_PULL_UP);
	gpio_direction_input(GPIO_IRDA_IRQ);
	printk(KERN_ERR "%s complete\n", __func__);
	return;
};

/*
 * When using SPI and I2C in same line, fw must be updated before I2C registration
 * FW uploading can be operated faster in driver probe
 * So, we will move fw update and I2C registration to driver init
 */
void __init exynos5_universal5420_fpga_init(void)
{
	printk(KERN_ERR "[%s] initialization start!\n", __func__);

	fpga_config_gpio_table(ARRAY_SIZE(fpga_gpio_table),fpga_gpio_table);

	irda_device_init();
};
