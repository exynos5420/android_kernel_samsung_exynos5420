#
# Makefile for the sensors drivers.
#

# Each configuration option enables a list of files.

ccflags-y := $(KBUILD_FP_SENSOR_CFLAGS)

obj-$(CONFIG_SENSORS_FINGERPRINT_SYSFS)		+= fingerprint_sysfs.o
obj-$(CONFIG_SENSORS_VFS61XX_KO)		+= vfs61xx.o
