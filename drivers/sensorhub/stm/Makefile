#
# Makefile for the sensor drivers.
#

# Each configuration option enables a list of files.
obj-$(CONFIG_SENSORS_SSP)		+= ssp_dev.o ssp_i2c.o ssp_data.o ssp_sysfs.o\
						ssp_input.o ssp_firmware.o ssp_debug.o
obj-$(CONFIG_SENSORS_SYSFS)		+= sensors_core.o

obj-$(CONFIG_IIO)			+= ssp_iio_ring.o ssp_iio_trigger.o

obj-$(CONFIG_SENSORS_SSP_MPU6500)	+= accel_mpu6500.o gyro_mpu6500.o
obj-$(CONFIG_SENSORS_SSP_YAS532)	+= magnetic_yas532.o
obj-$(CONFIG_SENSORS_SSP_AK09911)	+= magnetic_ak09911.o

obj-$(CONFIG_SENSORS_SSP_CM3320)	+= light_cm3320.o
obj-$(CONFIG_SENSORS_SSP_MAX88920)	+= prox_max88920.o gesture_max88920.o
obj-$(CONFIG_SENSORS_SSP_MAX88921)	+= light_max88921.o prox_max88920.o\
					gesture_max88922.o

obj-$(CONFIG_SENSORS_SSP_LPS25H)	+= pressure_lps25h.o
obj-$(CONFIG_SENSORS_SSP_SHTC1)		+= temphumidity_shtc1.o

obj-$(CONFIG_SENSORS_SSP_STM32F401)	+= mcu_stm32f401.o

obj-$(CONFIG_SENSORS_SSP_SENSORHUB)	+= ssp_sensorhub.o
