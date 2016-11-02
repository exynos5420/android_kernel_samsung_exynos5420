/*
 * haptic motor driver for max77803 - max77673_haptic.c
 *
 * Copyright (C) 2011 ByungChang Cha <bc.cha@samsung.com>
 * Copyright (C) 2012 The CyanogenMod Project
 *                    Daniel Hillenbrand <codeworkx@cyanogenmod.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timed_output.h>
#include <linux/hrtimer.h>
#include <linux/pwm.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/i2c.h>
#include <linux/regulator/consumer.h>
#include <linux/mfd/max77803.h>
#include <linux/mfd/max77803-private.h>

#define TEST_MODE_TIME 10000
#define MAX_INTENSITY 10000

struct max77803_haptic_data {
	struct max77803_dev *max77803;
	struct i2c_client *i2c;
	struct i2c_client *pmic_i2c;
	struct max77803_haptic_platform_data *pdata;

	struct pwm_device *pwm;
#if !defined(CONFIG_V2A)
	struct regulator *regulator;
#endif
	struct timed_output_dev tout_dev;
	struct hrtimer timer;
	unsigned int timeout;

	struct workqueue_struct *workqueue;
	struct work_struct work;
	spinlock_t lock;
	bool running;
	bool resumed;
	
	u32 duty;
	u32 intensity;
};

static void max77803_haptic_i2c(struct max77803_haptic_data *hap_data, bool en)
{
	int ret;
	u8 value = hap_data->pdata->reg2;
	u8 lscnfg_val = 0x00;

	pr_debug("[VIB] %s %d\n", __func__, en);

	if (en) {
		value |= MOTOR_EN;
		lscnfg_val = 0x80;
	}

	ret = max77803_update_reg(hap_data->pmic_i2c, MAX77803_PMIC_REG_LSCNFG,
				lscnfg_val, 0x80);
	if (ret)
		pr_err("[VIB] i2c update error %d\n", ret);

	ret = max77803_write_reg(hap_data->i2c,
				 MAX77803_HAPTIC_REG_CONFIG2, value);
	if (ret)
		pr_err("[VIB] i2c write error %d\n", ret);
}

static ssize_t intensity_store(struct device *dev,
		struct device_attribute *devattr, const char *buf, size_t count)
{
	struct timed_output_dev *tdev = dev_get_drvdata(dev);
	struct max77803_haptic_data *drvdata
		= container_of(tdev, struct max77803_haptic_data, tout_dev);
	int duty = drvdata->pdata->period >> 1;
	int intensity = 0, ret = 0;

	ret = kstrtoint(buf, 0, &intensity);

	if (intensity < 0 || intensity > (MAX_INTENSITY / 100)) {
		pr_err("out of range\n");
		return -EINVAL;
	}

	if (intensity == (MAX_INTENSITY / 100))
		duty = drvdata->pdata->duty;
	else if (intensity >= 0) {
		long tmp = drvdata->pdata->duty >> 1;

		tmp *= (intensity);
		duty += (int)(tmp / 100);
	}

	drvdata->intensity = intensity * 100;
	drvdata->duty = duty;

	pwm_config(drvdata->pwm, duty, drvdata->pdata->period);

	return count;
}

static ssize_t intensity_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct timed_output_dev *tdev = dev_get_drvdata(dev);
	struct max77803_haptic_data *drvdata
		= container_of(tdev, struct max77803_haptic_data, tout_dev);

	return sprintf(buf, "%u\n", (drvdata->intensity / 100));
}

static ssize_t pwm_default_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", 50);
}

static ssize_t pwm_max_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", 100);
}

static ssize_t pwm_min_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", 0);
}

static ssize_t pwm_threshold_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", 75);
}

static DEVICE_ATTR(pwm_default, 0444, pwm_default_show, NULL);
static DEVICE_ATTR(pwm_max, 0444, pwm_max_show, NULL);
static DEVICE_ATTR(pwm_min, 0444, pwm_min_show, NULL);
static DEVICE_ATTR(pwm_threshold, 0444, pwm_threshold_show, NULL);
static DEVICE_ATTR(pwm_value, 0664, intensity_show, intensity_store);

static int haptic_get_time(struct timed_output_dev *tout_dev)
{
	struct max77803_haptic_data *hap_data
		= container_of(tout_dev, struct max77803_haptic_data, tout_dev);

	struct hrtimer *timer = &hap_data->timer;
	if (hrtimer_active(timer)) {
		ktime_t remain = hrtimer_get_remaining(timer);
		struct timeval t = ktime_to_timeval(remain);
		return t.tv_sec * 1000 + t.tv_usec / 1000;
	}
	return 0;
}

static void haptic_enable(struct timed_output_dev *tout_dev, int value)
{
	struct max77803_haptic_data *hap_data
		= container_of(tout_dev, struct max77803_haptic_data, tout_dev);

	struct hrtimer *timer = &hap_data->timer;
	unsigned long flags;


	cancel_work_sync(&hap_data->work);
	hrtimer_cancel(timer);
	hap_data->timeout = value;
	queue_work(hap_data->workqueue, &hap_data->work);
	spin_lock_irqsave(&hap_data->lock, flags);
	if (value > 0 && value != TEST_MODE_TIME) {
		pr_debug("%s value %d\n", __func__, value);
		value = min(value, (int)hap_data->pdata->max_timeout);
		hrtimer_start(timer, ns_to_ktime((u64)value * NSEC_PER_MSEC),
			HRTIMER_MODE_REL);
	}
	spin_unlock_irqrestore(&hap_data->lock, flags);
}

static enum hrtimer_restart haptic_timer_func(struct hrtimer *timer)
{
	struct max77803_haptic_data *hap_data
		= container_of(timer, struct max77803_haptic_data, timer);
	unsigned long flags;

	hap_data->timeout = 0;
	queue_work(hap_data->workqueue, &hap_data->work);
	return HRTIMER_NORESTART;
}

static void haptic_work(struct work_struct *work)
{
	struct max77803_haptic_data *hap_data
		= container_of(work, struct max77803_haptic_data, work);

	pr_debug("[VIB] %s\n", __func__);
	if (hap_data->timeout > 0 && hap_data->intensity) {
		if (hap_data->running)
			return;

		max77803_haptic_i2c(hap_data, true);

		pwm_config(hap_data->pwm, hap_data->duty,
			   hap_data->pdata->period);
		pwm_enable(hap_data->pwm);
#if !defined(CONFIG_V2A)
		if (hap_data->pdata->motor_en)
			hap_data->pdata->motor_en(true);
		else
			regulator_enable(hap_data->regulator);
#endif
		hap_data->running = true;
	} else {
		if (!hap_data->running)
			return;
#if !defined(CONFIG_V2A)
		if (hap_data->pdata->motor_en)
			hap_data->pdata->motor_en(false);
		else
			regulator_disable(hap_data->regulator);
#endif
		pwm_disable(hap_data->pwm);

		max77803_haptic_i2c(hap_data, false);

		hap_data->running = false;
	}
	return;
}

static int max77803_haptic_probe(struct platform_device *pdev)
{
	int error = 0;
	struct max77803_dev *max77803 = dev_get_drvdata(pdev->dev.parent);
	struct max77803_platform_data *max77803_pdata
		= dev_get_platdata(max77803->dev);
	struct max77803_haptic_platform_data *pdata
		= max77803_pdata->haptic_data;
	struct max77803_haptic_data *hap_data;

	pr_debug("[VIB] ++ %s\n", __func__);
	 if (pdata == NULL) {
		pr_err("%s: no pdata\n", __func__);
		return -ENODEV;
	}

	hap_data = kzalloc(sizeof(struct max77803_haptic_data), GFP_KERNEL);
	if (!hap_data)
		return -ENOMEM;

	platform_set_drvdata(pdev, hap_data);
	hap_data->max77803 = max77803;
	hap_data->i2c = max77803->haptic;
	hap_data->pmic_i2c = max77803->i2c;
	hap_data->pdata = pdata;
	hap_data->intensity = MAX_INTENSITY;
	hap_data->duty = pdata->duty;

	hap_data->workqueue = create_singlethread_workqueue("hap_work");
	if (!(hap_data->workqueue)) {
		pr_err("%s: fail to create single thread workqueue\n",
								__func__);
		error = -EFAULT;
	}

	INIT_WORK(&(hap_data->work), haptic_work);
	spin_lock_init(&(hap_data->lock));

	hap_data->pwm = pwm_request(hap_data->pdata->pwm_id, "vibrator");
	if (IS_ERR(hap_data->pwm)) {
		pr_err("[VIB] Failed to request pwm\n");
		error = -EFAULT;
		goto err_pwm_request;
	}
	pwm_config(hap_data->pwm, pdata->period / 2, pdata->period);

#if !defined(CONFIG_V2A)
	if (pdata->init_hw)
		pdata->init_hw();
	else
		hap_data->regulator
			= regulator_get(NULL, pdata->regulator_name);

	if (IS_ERR(hap_data->regulator)) {
		pr_err("[VIB] Failed to get vmoter regulator.\n");
		error = -EFAULT;
		goto err_regulator_get;
	}
#endif
	/* hrtimer init */
	hrtimer_init(&hap_data->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hap_data->timer.function = haptic_timer_func;

	/* timed_output_dev init*/
	hap_data->tout_dev.name = "vibrator";
	hap_data->tout_dev.get_time = haptic_get_time;
	hap_data->tout_dev.enable = haptic_enable;

	hap_data->resumed = false;

#ifdef CONFIG_ANDROID_TIMED_OUTPUT
	error = timed_output_dev_register(&hap_data->tout_dev);
	if (error < 0) {
		pr_err("[VIB] Failed to register timed_output : %d\n", error);
		error = -EFAULT;
		goto err_timed_output_register;
	}

	error = sysfs_create_file(&hap_data->tout_dev.dev->kobj,
				&dev_attr_pwm_default.attr);
	if (error < 0) {
		pr_err("[VIB] Failed to register pwm_default sysfs : %d\n", error);
		goto err_timed_output_register;
	}

	error = sysfs_create_file(&hap_data->tout_dev.dev->kobj,
				&dev_attr_pwm_max.attr);
	if (error < 0) {
		pr_err("[VIB] Failed to register pwm_max sysfs : %d\n", error);
		goto err_timed_output_register;
	}

	error = sysfs_create_file(&hap_data->tout_dev.dev->kobj,
				&dev_attr_pwm_min.attr);
	if (error < 0) {
		pr_err("[VIB] Failed to register pwm_min sysfs : %d\n", error);
		goto err_timed_output_register;
	}

	error = sysfs_create_file(&hap_data->tout_dev.dev->kobj,
				&dev_attr_pwm_threshold.attr);
	if (error < 0) {
		pr_err("[VIB] Failed to register pwm_threshold sysfs : %d\n", error);
		goto err_timed_output_register;
	}

	error = sysfs_create_file(&hap_data->tout_dev.dev->kobj,
				&dev_attr_pwm_value.attr);
	if (error < 0) {
		pr_err("[VIB] Failed to register pwm_value sysfs : %d\n", error);
		goto err_timed_output_register;
	}
#endif

	pr_debug("[VIB] -- %s\n", __func__);

	return error;

err_timed_output_register:
#if !defined(CONFIG_V2A)
	regulator_put(hap_data->regulator);
#endif
err_regulator_get:
	pwm_free(hap_data->pwm);
err_pwm_request:
	kfree(hap_data);
	return error;
}

static int __devexit max77803_haptic_remove(struct platform_device *pdev)
{
	struct max77803_haptic_data *data = platform_get_drvdata(pdev);
#ifdef CONFIG_ANDROID_TIMED_OUTPUT
	timed_output_dev_unregister(&data->tout_dev);
#endif

#if !defined(CONFIG_V2A)
	regulator_put(data->regulator);
#endif
	pwm_free(data->pwm);
	destroy_workqueue(data->workqueue);
	kfree(data);

	return 0;
}

static int max77803_haptic_suspend(struct platform_device *pdev,
			pm_message_t state)
{
	pr_info("[VIB] %s\n", __func__);
	return 0;
}
static int max77803_haptic_resume(struct platform_device *pdev)
{
	pr_info("[VIB] %s\n", __func__);
	return 0;
}

static struct platform_driver max77803_haptic_driver = {
	.probe		= max77803_haptic_probe,
	.remove		= max77803_haptic_remove,
	.suspend	= max77803_haptic_suspend,
	.resume		= max77803_haptic_resume,
	.driver = {
		.name	= "max77803-haptic",
		.owner	= THIS_MODULE,
	},
};

static int __init max77803_haptic_init(void)
{
	pr_debug("[VIB] %s\n", __func__);
	return platform_driver_register(&max77803_haptic_driver);
}
module_init(max77803_haptic_init);

static void __exit max77803_haptic_exit(void)
{
	platform_driver_unregister(&max77803_haptic_driver);
}
module_exit(max77803_haptic_exit);

MODULE_AUTHOR("ByungChang Cha <bc.cha@samsung.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MAX77803 haptic driver");
