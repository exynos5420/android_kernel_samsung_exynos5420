#
# Makefile for the sensors drivers.
#

# Each configuration option enables a list of files.

obj-$(CONFIG_SENSORS)		+= sensors_core.o

# combo chipset
obj-$(CONFIG_SENSORS_BMI055)	+= bma255.o bmg160.o
obj-$(CONFIG_SENSORS_BMI058)	+= bma280.o bmg160.o
# magnetic_sensor
obj-$(CONFIG_SENSORS_AK8963C)	+= ak8963c.o
obj-$(CONFIG_SENSORS_AK09911C)	+= ak09911c.o
# optical_sensor
obj-$(CONFIG_SENSORS_CM3323)	+= cm3323.o
obj-$(CONFIG_SENSORS_TMD3782)	+= tmd3782.o
# grip_sensor
obj-$(CONFIG_SENSORS_SX9500)	+= sx9500.o
