/*
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/regulator/machine.h>
#include <linux/gpio.h>
#include <plat/devs.h>
#include <plat/iic.h>
#include <plat/gpio-cfg.h>
#include <mach/irqs.h>
#include <mach/hs-iic.h>

#include <linux/mfd/samsung/core.h>
#include <linux/mfd/samsung/s2mps11.h>

#if defined(CONFIG_REGULATOR_S2ABB01)
#include <linux/regulator/s2abb01.h>
#endif

#define SMDK5420_PMIC_EINT	IRQ_EINT(24)

static struct regulator_consumer_supply s2m_buck1_consumer =
	REGULATOR_SUPPLY("vdd_mif", NULL);

static struct regulator_consumer_supply s2m_buck2_consumer =
	REGULATOR_SUPPLY("vdd_arm", NULL);

static struct regulator_consumer_supply s2m_buck3_consumer =
	REGULATOR_SUPPLY("vdd_int", NULL);

static struct regulator_consumer_supply s2m_buck4_consumer =
	REGULATOR_SUPPLY("vdd_g3d", NULL);

static struct regulator_consumer_supply s2m_buck5v123_consumer =
	REGULATOR_SUPPLY("vmem2_1.2v_ap", NULL);

static struct regulator_consumer_supply s2m_buck6_consumer =
	REGULATOR_SUPPLY("vdd_kfc", NULL);

#ifdef CONFIG_SND_SOC_WM8994
static struct regulator_consumer_supply s2m_ldo2_consumer[] = {
	REGULATOR_SUPPLY("AVDD2", NULL),
	REGULATOR_SUPPLY("CPVDD", NULL),
	REGULATOR_SUPPLY("DBVDD1", NULL),
	REGULATOR_SUPPLY("DBVDD2", NULL),
	REGULATOR_SUPPLY("DBVDD3", NULL),
};
#elif defined (CONFIG_MFD_WM5102)
static struct regulator_consumer_supply s2m_ldo2_consumer[] = {
	REGULATOR_SUPPLY("AVDD", "spi2.0"),
	REGULATOR_SUPPLY("LDOVDD", "spi2.0"),
	REGULATOR_SUPPLY("DBVDD1", "spi2.0"),

	REGULATOR_SUPPLY("CPVDD", "wm5102-codec"),
	REGULATOR_SUPPLY("DBVDD2", "wm5102-codec"),
	REGULATOR_SUPPLY("DBVDD3", "wm5102-codec"),
	REGULATOR_SUPPLY("SPKVDDL", "wm5102-codec"),
	REGULATOR_SUPPLY("SPKVDDR", "wm5102-codec"),
};
#else
static struct regulator_consumer_supply s2m_ldo2_consumer[] = {};
#endif

static struct regulator_consumer_supply s2m_ldo4_consumer[] = {
	REGULATOR_SUPPLY("vadc_1.8v_ap", NULL),
};

static struct regulator_consumer_supply s2m_ldo6_consumer[] = {
	REGULATOR_SUPPLY("vmipi_1.0v_ap", NULL),
};

static struct regulator_consumer_supply s2m_ldo7_consumer[] = {
	REGULATOR_SUPPLY("vmipi_1.8v_ap", NULL),
};

static struct regulator_consumer_supply s2m_ldo9_consumer[] = {
	REGULATOR_SUPPLY("vuotg_3.0v_ap", NULL),
};

static struct regulator_consumer_supply s2m_ldo10_consumer[] = {
	REGULATOR_SUPPLY("vddq_pre_1.8v_ap", NULL),
};

static struct regulator_consumer_supply s2m_ldo11_consumer[] = {
	REGULATOR_SUPPLY("vhsic_1.0v_ap", NULL),
};

static struct regulator_consumer_supply s2m_ldo12_consumer[] = {
	REGULATOR_SUPPLY("vhsic_1.8v_ap", NULL),
};

static struct regulator_consumer_supply s2m_ldo13_consumer[] = {
	REGULATOR_SUPPLY("vqmmc", "dw_mmc.2"),
};

static struct regulator_consumer_supply s2m_ldo14_consumer[] = {
	REGULATOR_SUPPLY("vcc_3.0v_motor", NULL),
};

static struct regulator_consumer_supply s2m_ldo16_consumer[] = {
	REGULATOR_SUPPLY("vcc_2.8v_ap", NULL),
};

static struct regulator_consumer_supply s2m_ldo17_consumer[] = {
	REGULATOR_SUPPLY("irled_3.3v", NULL),
};

static struct regulator_consumer_supply s2m_ldo19_consumer[] = {
	REGULATOR_SUPPLY("vtf_2.8v", NULL),
};

static struct regulator_consumer_supply s2m_ldo20_consumer[] = {
	REGULATOR_SUPPLY("vt_cam_1.8v", NULL),
};

static struct regulator_consumer_supply s2m_ldo21_consumer[] = {
	REGULATOR_SUPPLY("cam_isp_sensor_io_1.8v", NULL),
};

static struct regulator_consumer_supply s2m_ldo22_consumer[] = {
	REGULATOR_SUPPLY("cam_sensor_core_1.2v", NULL),
};

static struct regulator_consumer_supply s2m_ldo23_consumer[] = {
	REGULATOR_SUPPLY("vdd_mifs", NULL),
};

static struct regulator_consumer_supply s2m_ldo24_consumer[] = {
	REGULATOR_SUPPLY("tsp_3.3v", NULL),
};

static struct regulator_consumer_supply s2m_ldo26_consumer[] = {
	REGULATOR_SUPPLY("cam_af_2.8v_pm", NULL),
};

static struct regulator_consumer_supply s2m_ldo27_consumer[] = {
	REGULATOR_SUPPLY("vdd_g3ds", NULL),
};

static struct regulator_consumer_supply s2m_ldo29_consumer[] = {
 	REGULATOR_SUPPLY("vtcon_1.9v", NULL),
};

static struct regulator_consumer_supply s2m_ldo30_consumer[] = {
	REGULATOR_SUPPLY("vtouch_1.8v", NULL),
};

static struct regulator_consumer_supply s2m_ldo31_consumer[] = {
	REGULATOR_SUPPLY("grip_1.8v", NULL),
};

static struct regulator_consumer_supply s2m_ldo32_consumer[] = {
	REGULATOR_SUPPLY("tsp_1.8v", NULL),
};

static struct regulator_consumer_supply s2m_ldo33_consumer[] = {
	REGULATOR_SUPPLY("vcc_1.8v_mhl", NULL),
};

static struct regulator_consumer_supply s2m_ldo34_consumer[] = {
	REGULATOR_SUPPLY("vcc_3.3v_mhl", NULL),
};

static struct regulator_consumer_supply s2m_ldo35_consumer[] = {
	REGULATOR_SUPPLY("vsil_1.2a", NULL),
};

static struct regulator_consumer_supply s2m_ldo38_consumer[] = {
	REGULATOR_SUPPLY("key_led_3.3v", NULL),
};

#define SREGULATOR_INIT(_ldo, _name, _min_uV, _max_uV, _always_on,	\
		_boot_on, _ops_mask, _enabled)				\
	static struct regulator_init_data s2m_##_ldo##_data = {		\
		.constraints = {					\
			.name = _name,					\
			.min_uV = _min_uV,				\
			.max_uV = _max_uV,				\
			.always_on	= _always_on,			\
			.boot_on	= _boot_on,			\
			.apply_uV	= 1,				\
			.valid_ops_mask = _ops_mask,			\
			.state_mem = {					\
				.disabled =				\
					(_enabled == -1 ? 0 : !(_enabled)),\
				.enabled =				\
					(_enabled == -1 ? 0 : _enabled),\
		},							\
		},							\
		.num_consumer_supplies = ARRAY_SIZE(s2m_##_ldo##_consumer),\
		.consumer_supplies = &s2m_##_ldo##_consumer[0],		\
	};

SREGULATOR_INIT(ldo2, "vcc_1.8v_ap", 1800000, 1800000, 1, 1,
		REGULATOR_CHANGE_STATUS, 1);
SREGULATOR_INIT(ldo4, "vadc_1.8v_ap", 1800000, 1800000, 1, 1,
		REGULATOR_CHANGE_STATUS, 1);
SREGULATOR_INIT(ldo6, "vmipi_1.0v_ap", 1000000, 1000000, 1, 1,
		REGULATOR_CHANGE_STATUS, 0);
SREGULATOR_INIT(ldo7, "vmipi_1.8v_ap", 1800000, 1800000, 1, 1,
		REGULATOR_CHANGE_STATUS, 0);
SREGULATOR_INIT(ldo9, "vuotg_3.0v_ap", 3000000, 3000000, 1, 1,
		REGULATOR_CHANGE_STATUS, 1);
SREGULATOR_INIT(ldo10, "vddq_pre_1.8v_ap", 1800000, 1800000, 1, 1,
		REGULATOR_CHANGE_STATUS, 1);
SREGULATOR_INIT(ldo11, "vhsic_1.0v_ap", 1000000, 1000000, 1, 1,
		REGULATOR_CHANGE_STATUS, 1);
SREGULATOR_INIT(ldo12, "vhsic_1.8v_ap", 1800000, 1800000, 1, 1,
		REGULATOR_CHANGE_STATUS, 1);
SREGULATOR_INIT(ldo13, "VMMC2_2.8V_AP", 1800000, 2800000, 0, 1,
		REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS, 0);
SREGULATOR_INIT(ldo14, "vcc_3.0v_motor", 3000000, 3000000, 0, 0,
		REGULATOR_CHANGE_STATUS, 0);
SREGULATOR_INIT(ldo16, "vcc_2.8v_ap", 2800000, 2800000, 1, 1,
		REGULATOR_CHANGE_STATUS, 1);
SREGULATOR_INIT(ldo17, "irled_3.3v", 3300000, 3300000, 1, 1,
		REGULATOR_CHANGE_STATUS, 0);
SREGULATOR_INIT(ldo19, "vtf_2.8v", 2800000, 2800000, 0, 0,
		REGULATOR_CHANGE_STATUS, 0);
SREGULATOR_INIT(ldo20, "vt_cam_1.8v", 1800000, 1800000, 0, 0,
		REGULATOR_CHANGE_STATUS, 0);
SREGULATOR_INIT(ldo21, "cam_isp_sensor_io_1.8v", 1800000, 1800000, 0, 0,
		REGULATOR_CHANGE_STATUS, 0);
SREGULATOR_INIT(ldo22, "cam_sensor_core_1.2v", 1050000, 1200000, 0, 0,
		REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS, 0);
SREGULATOR_INIT(ldo23, "vmifs_1.1v_ap", 800000, 1100000, 1, 0,
		REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS, 1);
SREGULATOR_INIT(ldo24, "tsp_3.3v", 3300000, 3300000, 0, 0,
		REGULATOR_CHANGE_STATUS, 0);
SREGULATOR_INIT(ldo26, "cam_af_2.8v_pm", 2800000, 2800000, 0, 0,
		REGULATOR_CHANGE_STATUS, 0);
SREGULATOR_INIT(ldo27, "vg3ds_1.0v_ap", 800000, 1000000, 1, 0,
		REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_STATUS, 1);
SREGULATOR_INIT(ldo29, "vtcon_1.9v", 1900000, 1900000, 0, 0,
		REGULATOR_CHANGE_STATUS, 0);
SREGULATOR_INIT(ldo30, "vtouch_1.8v", 1900000, 1900000, 0, 0,
		REGULATOR_CHANGE_STATUS, 0);
SREGULATOR_INIT(ldo31, "grip_1.8v", 1800000, 1800000, 1, 1,
		REGULATOR_CHANGE_STATUS, 0);
SREGULATOR_INIT(ldo32, "tsp_1.8v", 1900000, 1900000, 0, 0,
		REGULATOR_CHANGE_STATUS, 0);
SREGULATOR_INIT(ldo33, "vcc_1.8v_mhl", 1800000, 1800000, 0, 0,
		REGULATOR_CHANGE_STATUS, 0);
SREGULATOR_INIT(ldo34, "vcc_3.3v_mhl", 3300000, 3300000, 0, 0,
		REGULATOR_CHANGE_STATUS, 0);
SREGULATOR_INIT(ldo35, "vsil_1.2a", 1200000, 1200000, 0, 0,
		REGULATOR_CHANGE_STATUS, 0);
SREGULATOR_INIT(ldo38, "key_led_3.3v", 3300000, 3300000, 0, 0,
		REGULATOR_CHANGE_STATUS, 0);

static struct regulator_init_data s2m_buck1_data = {
	.constraints = {
		.name = "vdd_mif range",
		.min_uV = 700000,
		.max_uV = 1300000,
		.apply_uV	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
				REGULATOR_CHANGE_STATUS,
		.always_on = 1,
		.boot_on = 1,
		.state_mem	= {
			.uV		= 1100000,
			.mode		= REGULATOR_MODE_NORMAL,
			.disabled	= 1,
		},
	},
	.num_consumer_supplies = 1,
	.consumer_supplies = &s2m_buck1_consumer,
};

static struct regulator_init_data s2m_buck2_data = {
	.constraints = {
		.name = "vdd_arm range",
		.min_uV = 800000,
		.max_uV = 1500000,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
				REGULATOR_CHANGE_STATUS,
		.always_on = 1,
		.boot_on = 1,
		.state_mem	= {
			.disabled	= 1,
		},
	},
	.num_consumer_supplies = 1,
	.consumer_supplies = &s2m_buck2_consumer,
};

static struct regulator_init_data s2m_buck3_data = {
	.constraints	= {
		.name		= "vdd_int range",
		.min_uV		=  800000,
		.max_uV		= 1400000,
		.apply_uV	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
				REGULATOR_CHANGE_STATUS,
		.always_on = 1,
		.boot_on = 1,
		.state_mem	= {
			.uV		= 1100000,
			.mode		= REGULATOR_MODE_NORMAL,
			.disabled	= 1,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &s2m_buck3_consumer,
};

static struct regulator_init_data s2m_buck4_data = {
	.constraints	= {
		.name		= "vdd_g3d range",
		.min_uV		=  700000,
		.max_uV		= 1400000,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
				REGULATOR_CHANGE_STATUS,
		.always_on = 1,
		.boot_on = 1,
		.state_mem	= {
			.disabled	= 1,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &s2m_buck4_consumer,
};

static struct regulator_init_data s2m_buck5v123_data = {
	.constraints	= {
		.name		= "vmem2_1.2v_ap range",
		.min_uV		= 1200000,
		.max_uV		= 1200000,
		.valid_ops_mask	= REGULATOR_CHANGE_STATUS,
		.always_on = 1,
		.boot_on = 1,
		.state_mem	= {
			.disabled	= 1,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &s2m_buck5v123_consumer,
};

static struct regulator_init_data s2m_buck6_data = {
	.constraints	= {
		.name		= "vdd_kfc range",
		.min_uV		=  800000,
		.max_uV		= 1500000,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
				REGULATOR_CHANGE_STATUS,
		.always_on = 1,
		.boot_on = 1,
		.state_mem	= {
			.disabled	= 1,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &s2m_buck6_consumer,
};

static struct sec_regulator_data exynos_regulators[] = {
	{S2MPS11_BUCK1, &s2m_buck1_data},
	{S2MPS11_BUCK2, &s2m_buck2_data},
	{S2MPS11_BUCK3, &s2m_buck3_data},
	{S2MPS11_BUCK4, &s2m_buck4_data},
	{S2MPS11_BUCK5V123, &s2m_buck5v123_data},
	{S2MPS11_BUCK6, &s2m_buck6_data},
	{S2MPS11_LDO2, &s2m_ldo2_data},
	{S2MPS11_LDO4, &s2m_ldo4_data},
	{S2MPS11_LDO6, &s2m_ldo6_data},
	{S2MPS11_LDO7, &s2m_ldo7_data},
	{S2MPS11_LDO9, &s2m_ldo9_data},
	{S2MPS11_LDO10, &s2m_ldo10_data},
	{S2MPS11_LDO11, &s2m_ldo11_data},
	{S2MPS11_LDO12, &s2m_ldo12_data},
	{S2MPS11_LDO13, &s2m_ldo13_data},
	{S2MPS11_LDO14, &s2m_ldo14_data},
	{S2MPS11_LDO16, &s2m_ldo16_data},
	{S2MPS11_LDO17, &s2m_ldo17_data},
	{S2MPS11_LDO19, &s2m_ldo19_data},
	{S2MPS11_LDO20, &s2m_ldo20_data},
	{S2MPS11_LDO21, &s2m_ldo21_data},
	{S2MPS11_LDO22, &s2m_ldo22_data},
	{S2MPS11_LDO23, &s2m_ldo23_data},
	{S2MPS11_LDO24, &s2m_ldo24_data},
	{S2MPS11_LDO26, &s2m_ldo26_data},
	{S2MPS11_LDO27, &s2m_ldo27_data},
	{S2MPS11_LDO29, &s2m_ldo29_data},
	{S2MPS11_LDO30, &s2m_ldo30_data},
	{S2MPS11_LDO31, &s2m_ldo31_data},
	{S2MPS11_LDO32, &s2m_ldo32_data},
	{S2MPS11_LDO33, &s2m_ldo33_data},
	{S2MPS11_LDO34, &s2m_ldo34_data},
	{S2MPS11_LDO35, &s2m_ldo35_data},
	{S2MPS11_LDO38, &s2m_ldo38_data},
};

struct sec_opmode_data s2mps11_opmode_data[S2MPS11_REG_MAX] = {
	[S2MPS11_BUCK1] = {S2MPS11_BUCK1, SEC_OPMODE_STANDBY},
	[S2MPS11_BUCK2] = {S2MPS11_BUCK2, SEC_OPMODE_STANDBY},
	[S2MPS11_BUCK3] = {S2MPS11_BUCK3, SEC_OPMODE_STANDBY},
	[S2MPS11_BUCK4] = {S2MPS11_BUCK4, SEC_OPMODE_STANDBY},
	[S2MPS11_BUCK5V123] = {S2MPS11_BUCK5V123, SEC_OPMODE_STANDBY},
	[S2MPS11_BUCK6] = {S2MPS11_BUCK6, SEC_OPMODE_STANDBY},
	[S2MPS11_LDO4] = {S2MPS11_LDO4, SEC_OPMODE_STANDBY},
	[S2MPS11_LDO6] = {S2MPS11_LDO6, SEC_OPMODE_STANDBY},
	[S2MPS11_LDO7] = {S2MPS11_LDO7, SEC_OPMODE_STANDBY},
	[S2MPS11_LDO9] = {S2MPS11_LDO9, SEC_OPMODE_STANDBY},
	[S2MPS11_LDO10] = {S2MPS11_LDO10, SEC_OPMODE_STANDBY},
	[S2MPS11_LDO11] = {S2MPS11_LDO11, SEC_OPMODE_STANDBY},
	[S2MPS11_LDO12] = {S2MPS11_LDO12, SEC_OPMODE_STANDBY},
	[S2MPS11_LDO14] = {S2MPS11_LDO14, SEC_OPMODE_STANDBY},
	[S2MPS11_LDO17] = {S2MPS11_LDO17, SEC_OPMODE_STANDBY},
	[S2MPS11_LDO19] = {S2MPS11_LDO19, SEC_OPMODE_STANDBY},
	[S2MPS11_LDO20] = {S2MPS11_LDO20, SEC_OPMODE_STANDBY},
	[S2MPS11_LDO21] = {S2MPS11_LDO21, SEC_OPMODE_STANDBY},
	[S2MPS11_LDO22] = {S2MPS11_LDO22, SEC_OPMODE_STANDBY},
	[S2MPS11_LDO23] = {S2MPS11_LDO23, SEC_OPMODE_STANDBY},
	[S2MPS11_LDO24] = {S2MPS11_LDO24, SEC_OPMODE_STANDBY},
	[S2MPS11_LDO26] = {S2MPS11_LDO26, SEC_OPMODE_STANDBY},
	[S2MPS11_LDO27] = {S2MPS11_LDO27, SEC_OPMODE_STANDBY},
	[S2MPS11_LDO29] = {S2MPS11_LDO29, SEC_OPMODE_STANDBY},
	[S2MPS11_LDO30] = {S2MPS11_LDO30, SEC_OPMODE_STANDBY},
	[S2MPS11_LDO32] = {S2MPS11_LDO32, SEC_OPMODE_STANDBY},
	[S2MPS11_LDO33] = {S2MPS11_LDO33, SEC_OPMODE_STANDBY},
	[S2MPS11_LDO34] = {S2MPS11_LDO34, SEC_OPMODE_STANDBY},
	[S2MPS11_LDO35] = {S2MPS11_LDO35, SEC_OPMODE_STANDBY},
	[S2MPS11_LDO38] = {S2MPS11_LDO38, SEC_OPMODE_STANDBY},
};

static int sec_cfg_irq(void)
{
	unsigned int pin = irq_to_gpio(SMDK5420_PMIC_EINT);

	s3c_gpio_cfgpin(pin, S3C_GPIO_SFN(0xF));
	s3c_gpio_setpull(pin, S3C_GPIO_PULL_UP);

	return 0;
}

static struct sec_pmic_platform_data exynos5_s2m_pdata = {
	.device_type		= S2MPS11X,
	.irq_base		= IRQ_BOARD_START,
	.num_regulators		= ARRAY_SIZE(exynos_regulators),
	.regulators		= exynos_regulators,
	.cfg_pmic_irq		= sec_cfg_irq,
	.wakeup			= 1,
	.opmode_data		= s2mps11_opmode_data,
	.buck16_ramp_delay	= 12,
	.buck2_ramp_delay	= 12,
	.buck34_ramp_delay	= 12,
	.buck2_ramp_enable	= 1,
	.buck3_ramp_enable	= 1,
	.buck4_ramp_enable	= 1,
	.buck6_ramp_enable	= 1,
	.wtsr_smpl		= true,
	.jig_smpl_disable	= true,
};

static struct i2c_board_info hs_i2c_devs3_s2mps11[] __initdata = {
	{
		I2C_BOARD_INFO("sec-pmic", 0xCC >> 1),
		.platform_data	= &exynos5_s2m_pdata,
		.irq		= SMDK5420_PMIC_EINT,
	},
};

#if defined(CONFIG_REGULATOR_S2ABB01)
static struct s2abb01_regulator_data exynos_s2abb01_regulators[] = {
};

static struct s2abb01_platform_data exynos5_s2abb01_pdata = {
	.num_regulators		= 0,
	.regulators		= exynos_s2abb01_regulators,
	.suspend_on_ctrl	= true,
};

static struct i2c_board_info hs_i2c_devs4_s2abb01[] __initdata = {
	{
		I2C_BOARD_INFO("s2abb01", (0xAA >> 1)),
		.platform_data	= &exynos5_s2abb01_pdata,
	}
};
#endif

struct exynos5_platform_i2c hs_i2c3_data __initdata = {
	.bus_number = 7,
	.operation_mode = HSI2C_POLLING,
	.speed_mode = HSI2C_FAST_SPD,
	.fast_speed = 400000,
	.high_speed = 2500000,
	.cfg_gpio = NULL,
};

void __init board_chagall_pmic_init(void)
{
	int ret = 0;

	exynos5_hs_i2c3_set_platdata(&hs_i2c3_data);
	i2c_register_board_info(7, hs_i2c_devs3_s2mps11, ARRAY_SIZE(hs_i2c_devs3_s2mps11));
	platform_device_register(&exynos5_device_hs_i2c3);
#if defined(CONFIG_REGULATOR_S2ABB01)
	exynos5_hs_i2c4_set_platdata(NULL);
	ret = i2c_register_board_info(8, hs_i2c_devs4_s2abb01, ARRAY_SIZE(hs_i2c_devs4_s2abb01));
	if (ret < 0) {
		pr_err("%s, hs_i2c_devs4_s2abb01 adding i2c fail(err=%d)\n", __func__, ret);
	}
	ret = platform_device_register(&exynos5_device_hs_i2c4);
	if (ret < 0)
		pr_err("%s, s2abb01 platform device register failed (err=%d)\n",
			__func__, ret);
#endif
}
