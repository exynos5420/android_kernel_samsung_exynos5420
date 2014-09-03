#
# Makefile for the kernel mobicore drivers
#

GUD_ROOT_FOLDER := $(src)

# add our modules to kernel.
obj-$(CONFIG_MOBICORE_API) += mcKernelApi.o
obj-$(CONFIG_MOBICORE_DRIVER) += mcDrvModule.o

mcDrvModule-objs := MobiCoreDriver/logging.o \
		MobiCoreDriver/ops.o \
		MobiCoreDriver/mem.o \
		MobiCoreDriver/api.o \
		MobiCoreDriver/pm.o \
		MobiCoreDriver/main.o

mcKernelApi-objs := MobiCoreKernelApi/main.o \
		MobiCoreKernelApi/clientlib.o \
		MobiCoreKernelApi/device.o \
		MobiCoreKernelApi/session.o \
		MobiCoreKernelApi/connection.o

CFLAGS_MobiCoreDriver/main.o := -include $(GUD_ROOT_FOLDER)/build_tag.h

subdir-ccflags-y += -Wno-declaration-after-statement -DNDEBUG
subdir-ccflags-$(CONFIG_MOBICORE_DEBUG) += -DDEBUG
subdir-ccflags-$(CONFIG_MOBICORE_VERBOSE) += -DDEBUG_VERBOSE

# The original makefile used ls | tail -1 to pick a random folder for the
# platform configuration!!!
#
# For kicks, let's make a stand and pick one.  It doesn't matter right now
# anyway because they are the same.
MOBICORE_PLATFORM := EXYNOS_5_STD

# Use the available platform folder
subdir-ccflags-y += -I$(GUD_ROOT_FOLDER)/MobiCoreDriver/platforms/$(MOBICORE_PLATFORM)
# MobiCore Driver includes
subdir-ccflags-y += -I$(GUD_ROOT_FOLDER)/MobiCoreDriver/public
# MobiCore KernelApi required incldes
subdir-ccflags-y += -I$(GUD_ROOT_FOLDER)/MobiCoreKernelApi/include \
		-I$(GUD_ROOT_FOLDER)/MobiCoreKernelApi/public
