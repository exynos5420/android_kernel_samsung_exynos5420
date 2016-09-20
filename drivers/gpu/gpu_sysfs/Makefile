obj-m := gpu_sysfs.o


# Common files.
gpu_sysfs-y :=	gpu_sysfs_main.o \
                gpu_sysfs_util.o \
                gpu_sysfs_target_exynos5420.o

clean:
	rm -f *.o .*.cmd modules.order Module.symvers gpu_sysfs.ko gpu_sysfs.mod.c
	rm -rf .tmp_versions
