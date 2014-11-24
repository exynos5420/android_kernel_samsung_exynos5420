#ifndef __MDNIE_H__
#define __MDNIE__

#if defined(CONFIG_FB_S5P_MDNIE_LITE)
struct mdnie_device;

struct mdnie_ops {
	int (*write)(struct device *, const u8 *seq, u32 len);
	int (*read)(struct device *, u8 addr, u8 *buf, u32 len);
	/* Only for specific hardware */
	int (*set_addr)(struct device *, int mdnie_addr);
};

struct mdnie_device {
	/* This protects the 'ops' field. If 'ops' is NULL, the driver that
	   registered this device has been unloaded, and if class_get_devdata()
	   points to something in the body of that driver, it is also invalid. */
	struct mutex ops_lock;
	/* If this is NULL, the backing module is unloaded */
	struct mdnie_ops *ops;
	/* The framebuffer notifier block */
	/* Serialise access to set_power method */
	struct mutex update_lock;
	struct notifier_block fb_notif;

	struct device dev;
};

#define to_mdnie_device(obj) container_of(obj, struct mdnie_device, dev)

static inline void * mdnie_get_data(struct mdnie_device *md_dev)
{
	return dev_get_drvdata(&md_dev->dev);
}

extern struct mdnie_device *mdnie_device_register(const char *name,
		struct device *parent, struct mdnie_ops *ops);
extern void mdnie_device_unregister(struct mdnie_device *md);

#endif

struct platform_mdnie_data {
	unsigned int	display_type;
    unsigned int    support_pwm;
#if defined (CONFIG_S5P_MDNIE_PWM)
    int pwm_out_no;
	int pwm_out_func;
    char *name;
    int *br_table;
    int dft_bl;
#endif
	int (*trigger_set)(struct device *fimd);
	struct device *fimd1_device;
	struct lcd_platform_data	*lcd_pd;
};

#ifdef CONFIG_FB_I80IF
extern int s3c_fb_enable_trigger_by_mdnie(struct device *fimd);
#endif

#endif
