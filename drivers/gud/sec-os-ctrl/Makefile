#
# Makefile for Secure OS control sysfs
#
obj-$(CONFIG_SECURE_OS_CONTROL) += sec_os_ctrl.o
# Choose Exynos SoC

ifeq ($(CONFIG_SOC_EXYNOS3472), y)
EXYNOS_SOC := exynos3472
endif

ifeq ($(CONFIG_SOC_EXYNOS4415), y)
EXYNOS_SOC := exynos4415
endif

# MobiCore kernel driver path
GUD_ROOT_FOLDER := drivers/gud/gud-$(EXYNOS_SOC)

# Includes MobiCore kernel driver
ccflags-y += -I$(GUD_ROOT_FOLDER)/MobiCoreDriver
ccflags-y += -I$(GUD_ROOT_FOLDER)/MobiCoreDriver/public
