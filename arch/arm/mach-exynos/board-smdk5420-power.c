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
#include <linux/notifier.h>
#include <linux/gpio.h>
#include <plat/devs.h>
#include <plat/iic.h>
#include <plat/gpio-cfg.h>
#include <linux/platform_data/exynos_thermal.h>
#include <mach/tmu.h>

#include <mach/regs-pmu.h>
#include <mach/irqs.h>
#include <mach/hs-iic.h>
#include <mach/devfreq.h>

#include <linux/mfd/samsung/core.h>
#include <linux/mfd/samsung/s2mps11.h>

#include "board-smdk5420.h"

#define SMDK5420_PMIC_EINT	IRQ_EINT(26)

static struct regulator_consumer_supply s2m_buck1_consumer =
	REGULATOR_SUPPLY("vdd_mif", NULL);

static struct regulator_consumer_supply s2m_buck2_consumer =
	REGULATOR_SUPPLY("vdd_arm", NULL);

static struct regulator_consumer_supply s2m_buck3_consumer =
	REGULATOR_SUPPLY("vdd_int", NULL);

static struct regulator_consumer_supply s2m_buck4_consumer =
	REGULATOR_SUPPLY("vdd_g3d", NULL);

static struct regulator_consumer_supply s2m_buck6_consumer =
	REGULATOR_SUPPLY("vdd_kfc", NULL);

static struct regulator_consumer_supply s2m_ldo13_consumer =
	REGULATOR_SUPPLY("vqmmc", "dw_mmc.2");

static struct regulator_consumer_supply s2m_ldo15_consumer =
	REGULATOR_SUPPLY("vdd_ldo15", NULL);

static struct regulator_consumer_supply s2m_ldo16_consumer =
	REGULATOR_SUPPLY("vdd_ldo16", NULL);

static struct regulator_consumer_supply s2m_ldo17_consumer =
	REGULATOR_SUPPLY("tsp_avdd", NULL);

static struct regulator_consumer_supply s2m_ldo19_consumer =
	REGULATOR_SUPPLY("vmmc", "dw_mmc.2");

static struct regulator_consumer_supply s2m_ldo23_consumer =
	REGULATOR_SUPPLY("vdd_mifs", NULL);

static struct regulator_consumer_supply s2m_ldo24_consumer =
	REGULATOR_SUPPLY("tsp_io", NULL);

static struct regulator_consumer_supply s2m_ldo27_consumer =
	REGULATOR_SUPPLY("vdd_g3ds", NULL);

static struct regulator_init_data s2m_buck1_data = {
	.constraints	= {
		.name		= "vdd_mif range",
		.min_uV		=  700000,
		.max_uV		= 1300000,
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
	.consumer_supplies	= &s2m_buck1_consumer,
};

static struct regulator_init_data s2m_buck2_data = {
	.constraints	= {
		.name		= "vdd_arm range",
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
	.consumer_supplies	= &s2m_buck2_consumer,
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

static struct regulator_init_data s2m_ldo13_data = {
	.constraints	= {
		.name		= "vdd_ldo13 range",
		.min_uV		= 1800000,
		.max_uV		= 3300000,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
				  REGULATOR_CHANGE_STATUS,
		.always_on	= 1,
		.state_mem	= {
			.disabled	= 1,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &s2m_ldo13_consumer,
};

static struct regulator_init_data s2m_ldo15_data = {
	.constraints	= {
		.name		= "vdd_ldo15 range",
		.min_uV		= 3100000,
		.max_uV		= 3100000,
		.apply_uV	= 1,
		.always_on	= 1,
		.state_mem	= {
			.enabled	= 1,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &s2m_ldo15_consumer,
};

static struct regulator_init_data s2m_ldo16_data = {
	.constraints	= {
		.name		= "vdd_ldo16 range",
		.min_uV		= 2200000,
		.max_uV		= 2200000,
		.apply_uV	= 1,
		.always_on	= 1,
		.state_mem	= {
			.enabled	= 1,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &s2m_ldo16_consumer,
};

static struct regulator_init_data s2m_ldo17_data = {
	.constraints	= {
		.name		= "tsp_avdd range",
		.min_uV		= 3300000,
		.max_uV		= 3300000,
		.apply_uV	= 1,
		.always_on	= 1,
		.state_mem	= {
			.enabled	= 1,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &s2m_ldo17_consumer,
};

static struct regulator_init_data s2m_ldo19_data = {
	.constraints	= {
		.name		= "vdd_ldo19 range",
		.min_uV		= 1800000,
		.max_uV		= 2850000,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
				  REGULATOR_CHANGE_STATUS,
		.always_on	= 1,
		.state_mem	= {
			.disabled	= 1,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &s2m_ldo19_consumer,
};

static struct regulator_init_data s2m_ldo23_data = {
	.constraints	= {
		.name		= "vdd_mifs range",
		.min_uV		=  800000,
		.max_uV		= 1100000,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
				REGULATOR_CHANGE_STATUS,
		.apply_uV	= 1,
		.always_on	= 1,
		.state_mem	= {
			.enabled	= 1,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &s2m_ldo23_consumer,
};

static struct regulator_init_data s2m_ldo24_data = {
	.constraints	= {
		.name		= "tsp_io range",
		.min_uV		= 2800000,
		.max_uV		= 2800000,
		.apply_uV	= 1,
		.always_on	= 1,
		.state_mem	= {
			.enabled	= 1,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &s2m_ldo24_consumer,
};

static struct regulator_init_data s2m_ldo27_data = {
	.constraints	= {
		.name		= "vdd_g3ds range",
		.min_uV		=  800000,
		.max_uV		= 1100000,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
				REGULATOR_CHANGE_STATUS,
		.apply_uV	= 1,
		.always_on	= 1,
		.state_mem	= {
			.enabled	= 1,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &s2m_ldo27_consumer,
};

static struct sec_regulator_data exynos_regulators[] = {
	{S2MPS11_BUCK1, &s2m_buck1_data},
	{S2MPS11_BUCK2, &s2m_buck2_data},
	{S2MPS11_BUCK3, &s2m_buck3_data},
	{S2MPS11_BUCK4, &s2m_buck4_data},
	{S2MPS11_BUCK6, &s2m_buck6_data},
	{S2MPS11_LDO13, &s2m_ldo13_data},
	{S2MPS11_LDO15, &s2m_ldo15_data},
	{S2MPS11_LDO16, &s2m_ldo16_data},
	{S2MPS11_LDO17, &s2m_ldo17_data},
	{S2MPS11_LDO19, &s2m_ldo19_data},
	{S2MPS11_LDO23, &s2m_ldo23_data},
	{S2MPS11_LDO24, &s2m_ldo24_data},
	{S2MPS11_LDO27, &s2m_ldo27_data},
};

struct sec_opmode_data s2mps11_opmode_data[S2MPS11_REG_MAX] = {
	[S2MPS11_BUCK1] = {S2MPS11_BUCK1, SEC_OPMODE_STANDBY},
	[S2MPS11_BUCK2] = {S2MPS11_BUCK2, SEC_OPMODE_STANDBY},
	[S2MPS11_BUCK3] = {S2MPS11_BUCK3, SEC_OPMODE_STANDBY},
	[S2MPS11_BUCK4] = {S2MPS11_BUCK4, SEC_OPMODE_STANDBY},
	[S2MPS11_BUCK6] = {S2MPS11_BUCK6, SEC_OPMODE_STANDBY},
	[S2MPS11_LDO13] = {S2MPS11_LDO13, SEC_OPMODE_STANDBY},
	[S2MPS11_LDO15] = {S2MPS11_LDO15, SEC_OPMODE_STANDBY},
	[S2MPS11_LDO16] = {S2MPS11_LDO16, SEC_OPMODE_STANDBY},
	[S2MPS11_LDO17] = {S2MPS11_LDO17, SEC_OPMODE_STANDBY},
	[S2MPS11_LDO19] = {S2MPS11_LDO19, SEC_OPMODE_STANDBY},
	[S2MPS11_LDO23] = {S2MPS11_LDO23, SEC_OPMODE_STANDBY},
	[S2MPS11_LDO24] = {S2MPS11_LDO24, SEC_OPMODE_STANDBY},
	[S2MPS11_LDO27] = {S2MPS11_LDO27, SEC_OPMODE_STANDBY},
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
	.wtsr_smpl		= 1,
	.opmode_data		= s2mps11_opmode_data,
	.buck16_ramp_delay	= 12,
	.buck2_ramp_delay	= 12,
	.buck34_ramp_delay	= 12,
	.buck2_ramp_enable	= 1,
	.buck3_ramp_enable	= 1,
	.buck4_ramp_enable	= 1,
	.buck6_ramp_enable	= 1,
};

static struct i2c_board_info hs_i2c_devs0[] __initdata = {
	{
		I2C_BOARD_INFO("sec-pmic", 0xCC >> 1),
		.platform_data	= &exynos5_s2m_pdata,
		.irq		= SMDK5420_PMIC_EINT,
	},
};

#ifdef CONFIG_BATTERY_SAMSUNG
static struct platform_device samsung_device_battery = {
	.name	= "samsung-fake-battery",
	.id	= -1,
};
#endif

static struct exynos_tmu_platform_data exynos5_tmu_data = {
	.trigger_levels[0] = 90,
	.trigger_levels[1] = 95,
	.trigger_levels[2] = 110,
	.trigger_levels[3] = 115,
	.boost_trigger_levels[0] = 100,
	.boost_trigger_levels[1] = 105,
	.boost_trigger_levels[2] = 110,
	.boost_trigger_levels[3] = 115,
	.trigger_level0_en = 1,
	.trigger_level1_en = 1,
	.trigger_level2_en = 1,
	.trigger_level3_en = 1,
	.gain = 5,
	.reference_voltage = 16,
	.noise_cancel_mode = 4,
	.cal_type = TYPE_ONE_POINT_TRIMMING,
	.efuse_value = 55,
	.freq_tab[0] = {
		.freq_clip_max = 1700 * 1000,
		.temp_level = 90,
	},
	.freq_tab[1] = {
		.freq_clip_max = 1200 * 1000,
		.temp_level = 95,
	},
	.freq_tab[2] = {
		.freq_clip_max = 600 * 1000,
		.temp_level = 100,
	},
	.freq_tab[3] = {
		.freq_clip_max = 600 * 1000,
		.temp_level = 100,
	},
	.boost_freq_tab[0] = {
		.freq_clip_max = 1700 * 1000,
		.temp_level = 100,
	},
	.boost_freq_tab[1] = {
		.freq_clip_max = 1000 * 1000,
		.temp_level = 105,
	},
	.boost_freq_tab[2] = {
		.freq_clip_max = 1000 * 1000,
		.temp_level = 105,
	},
	.boost_freq_tab[3] = {
		.freq_clip_max = 1000 * 1000,
		.temp_level = 105,
	},
	.size[THERMAL_TRIP_ACTIVE] = 1,
	.size[THERMAL_TRIP_PASSIVE] = 3,
	.freq_tab_count = 4,
	.type = SOC_ARCH_EXYNOS5,
};

#ifdef CONFIG_ARM_EXYNOS5420_BUS_DEVFREQ
static struct platform_device exynos5_mif_devfreq = {
	.name	= "exynos5-busfreq-mif",
	.id	= -1,
};

static struct exynos_devfreq_platdata smdk5420_qos_mif_pd __initdata = {
	.default_qos = 160000,
};

static struct platform_device exynos5_int_devfreq = {
	.name	= "exynos5-busfreq-int",
	.id	= -1,
};

static struct exynos_devfreq_platdata smdk5420_qos_int_pd __initdata = {
	.default_qos = 83000,
};
#endif

static struct platform_device *smdk5420_power_devices[] __initdata = {
	&exynos5_device_hs_i2c0,
#ifdef CONFIG_BATTERY_SAMSUNG
	&samsung_device_battery,
#endif
#ifdef CONFIG_EXYNOS_THERMAL
	&exynos5420_device_tmu,
#endif
#ifdef CONFIG_ARM_EXYNOS5420_BUS_DEVFREQ
	&exynos5_mif_devfreq,
	&exynos5_int_devfreq,
#endif
};

void __init exynos5_smdk5420_power_init(void)
{
	exynos5_hs_i2c0_set_platdata(NULL);
	i2c_register_board_info(4, hs_i2c_devs0, ARRAY_SIZE(hs_i2c_devs0));
#ifdef CONFIG_EXYNOS_THERMAL
	exynos_tmu_set_platdata(&exynos5_tmu_data);
#endif

#ifdef CONFIG_ARM_EXYNOS5420_BUS_DEVFREQ
	s3c_set_platdata(&smdk5420_qos_mif_pd, sizeof(struct exynos_devfreq_platdata),
			&exynos5_mif_devfreq);
	s3c_set_platdata(&smdk5420_qos_int_pd, sizeof(struct exynos_devfreq_platdata),
			&exynos5_int_devfreq);
#endif

	platform_add_devices(smdk5420_power_devices,
			ARRAY_SIZE(smdk5420_power_devices));
}
