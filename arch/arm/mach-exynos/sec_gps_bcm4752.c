#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/poll.h>
#include <linux/miscdevice.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/io.h>
#include <linux/workqueue.h>
#include <linux/unistd.h>
#include <linux/bug.h>

#include <mach/sec_gps.h>
#include <mach/gpio.h>

#include <plat/gpio-cfg.h>
#include <mach/sec_gps.h>


static struct device *gps_dev;
extern unsigned int system_rev;

static ssize_t hwrev_show(struct device *dev, \
struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%x\n", system_rev);
}

static DEVICE_ATTR(hwrev, S_IRUGO, hwrev_show, NULL);


static int __init gps_bcm4752_init(void)
{
	BUG_ON(!sec_class);
	gps_dev = device_create(sec_class, NULL, 0, NULL, "gps");
	BUG_ON(!gps_dev);

	s3c_gpio_cfgpin(GPIO_GPS_RXD, S3C_GPIO_SFN(GPIO_GPS_RXD_AF));
	s3c_gpio_setpull(GPIO_GPS_RXD, S3C_GPIO_PULL_UP);
	s3c_gpio_cfgpin(GPIO_GPS_TXD, S3C_GPIO_SFN(GPIO_GPS_TXD_AF));
	s3c_gpio_setpull(GPIO_GPS_TXD, S3C_GPIO_PULL_NONE);
	s3c_gpio_cfgpin(GPIO_GPS_CTS, S3C_GPIO_SFN(GPIO_GPS_CTS_AF));
	s3c_gpio_setpull(GPIO_GPS_CTS, S3C_GPIO_PULL_NONE);
	s3c_gpio_cfgpin(GPIO_GPS_RTS, S3C_GPIO_SFN(GPIO_GPS_RTS_AF));
	s3c_gpio_setpull(GPIO_GPS_RTS, S3C_GPIO_PULL_NONE);

	if (device_create_file(gps_dev, &dev_attr_hwrev) < 0) {
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_hwrev.attr.name);
	}
	if (gpio_request(GPIO_GPS_PWR_EN, "GPS_PWR_EN")) {
		WARN(1, "fail to request gpio (GPS_PWR_EN)\n");
//		gpio_free(sec_class, gps_dev->devt);
		return 1;
	}

	s3c_gpio_setpull(GPIO_GPS_PWR_EN, S3C_GPIO_PULL_NONE);
	s3c_gpio_cfgpin(GPIO_GPS_PWR_EN, S3C_GPIO_OUTPUT);
	gpio_direction_output(GPIO_GPS_PWR_EN, 0);

	gpio_export(GPIO_GPS_PWR_EN, 1);

	gpio_export_link(gps_dev, "GPS_PWR_EN", GPIO_GPS_PWR_EN);

	printk(KERN_DEBUG "%s - system_rev : %x\n", __func__, system_rev);

	return 0;
}

device_initcall(gps_bcm4752_init);