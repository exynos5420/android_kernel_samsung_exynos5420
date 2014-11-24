ifeq ($(CONFIG_MALI_UMP_R2P4), y)
obj-$(CONFIG_MALI_400MP_UMP)	+= ump/
obj-$(CONFIG_MALI_400MP_UMP)	+= mali/
else
obj-$(CONFIG_MALI_400MP_UMP)	+= ump_r3p0/
obj-$(CONFIG_MALI_400MP_UMP)	+= mali_r3p0/
endif

EXTRA_CFLAGS += -Idrivers/gpu/arm/mali400
