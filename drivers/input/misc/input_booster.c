/*
 *  Copyright (C) 2013, Samsung Electronics Co. Ltd. All Rights Reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */

#include "input_booster.h"

static const char * const booster_device_name[BOOSTER_DEVICE_MAX] = {
	"key",
	"touchkey",
	"touch",
	"pen",
};

static unsigned int DbgLevel;
static struct input_booster *g_data;

/* Key */
DECLARE_DVFS_DELAYED_WORK_FUNC(CHG, KEY)
{
	struct input_booster *data =
		container_of(work, struct input_booster,
		dvfses[BOOSTER_DEVICE_KEY].dvfs_chg_work.work);
	struct booster_dvfs *dvfs = &data->dvfses[BOOSTER_DEVICE_KEY];

	mutex_lock(&dvfs->lock);

	set_qos(&dvfs->cpu_qos, PM_QOS_CPU_FREQ_MIN,
		dvfs->freqs[BOOSTER_LEVEL1].cpu_freq);
	set_qos(&dvfs->mif_qos, PM_QOS_BUS_THROUGHPUT,
		dvfs->freqs[BOOSTER_LEVEL1].mif_freq);
	set_qos(&dvfs->int_qos, PM_QOS_DEVICE_THROUGHPUT,
		dvfs->freqs[BOOSTER_LEVEL1].int_freq);

	dvfs->lock_status = true;
	dvfs->short_press = false;

	DVFS_DEV_DBG(DBG_DVFS, data->dev,
		"%s : DVFS ON [level %d]\n", __func__, dvfs->level);

	mutex_unlock(&dvfs->lock);
}

DECLARE_DVFS_DELAYED_WORK_FUNC(OFF, KEY)
{
	struct input_booster *data =
		container_of(work, struct input_booster,
		dvfses[BOOSTER_DEVICE_KEY].dvfs_off_work.work);
	struct booster_dvfs *dvfs = &data->dvfses[BOOSTER_DEVICE_KEY];

	mutex_lock(&dvfs->lock);

	remove_qos(&dvfs->cpu_qos);
	remove_qos(&dvfs->mif_qos);
	remove_qos(&dvfs->int_qos);

	dvfs->lock_status = false;
	dvfs->short_press = false;

	DVFS_DEV_DBG(DBG_DVFS, data->dev,
		"%s : DVFS OFF\n", __func__);

	mutex_unlock(&dvfs->lock);
}

DECLARE_DVFS_WORK_FUNC(SET, KEY)
{
	struct input_booster *data = (struct input_booster *)booster_data;
	struct booster_dvfs *dvfs = &data->dvfses[BOOSTER_DEVICE_KEY];

	mutex_lock(&dvfs->lock);

	switch (booster_mode) {
	case BOOSTER_MODE_ON:
		cancel_delayed_work(&dvfs->dvfs_off_work);
		if (!dvfs->lock_status && !dvfs->short_press) {
			schedule_delayed_work(&dvfs->dvfs_chg_work,
				msecs_to_jiffies(dvfs->msec_chg_time));
			dvfs->short_press = true;
		}
		break;
	case BOOSTER_MODE_OFF:
		if (dvfs->short_press) {
			cancel_delayed_work(&dvfs->dvfs_chg_work);
			schedule_work(&dvfs->dvfs_chg_work.work);
			schedule_delayed_work(&dvfs->dvfs_off_work,
				msecs_to_jiffies(dvfs->msec_off_time));
			goto out;
		}
		if (dvfs->lock_status)
			schedule_delayed_work(&dvfs->dvfs_off_work,
				msecs_to_jiffies(dvfs->msec_off_time));
		break;
	case BOOSTER_MODE_FORCE_OFF:
		if (dvfs->lock_status) {
			cancel_delayed_work(&dvfs->dvfs_chg_work);
			cancel_delayed_work(&dvfs->dvfs_off_work);
			schedule_work(&dvfs->dvfs_off_work.work);
		}
		break;
	default:
		break;
	}

out:
	mutex_unlock(&dvfs->lock);
	return;
}

/* Touchkey */
DECLARE_DVFS_DELAYED_WORK_FUNC(CHG, TOUCHKEY)
{
	struct input_booster *data =
		container_of(work, struct input_booster,
		dvfses[BOOSTER_DEVICE_TOUCHKEY].dvfs_chg_work.work);
	struct booster_dvfs *dvfs = &data->dvfses[BOOSTER_DEVICE_TOUCHKEY];

	mutex_lock(&dvfs->lock);

	if (dvfs->level == BOOSTER_LEVEL1) {
		set_qos(&dvfs->cpu_qos, PM_QOS_CPU_FREQ_MIN,
			dvfs->freqs[BOOSTER_LEVEL1].cpu_freq);
		set_qos(&dvfs->mif_qos, PM_QOS_BUS_THROUGHPUT,
			dvfs->freqs[BOOSTER_LEVEL1].mif_freq);
		set_qos(&dvfs->int_qos, PM_QOS_DEVICE_THROUGHPUT,
			dvfs->freqs[BOOSTER_LEVEL1].int_freq);
	} else {
		set_qos(&dvfs->cpu_qos, PM_QOS_CPU_FREQ_MIN,
			dvfs->freqs[BOOSTER_LEVEL2].cpu_freq);
		set_qos(&dvfs->mif_qos, PM_QOS_BUS_THROUGHPUT,
			dvfs->freqs[BOOSTER_LEVEL2].mif_freq);
		set_qos(&dvfs->int_qos, PM_QOS_DEVICE_THROUGHPUT,
			dvfs->freqs[BOOSTER_LEVEL2].int_freq);
	}

	dvfs->lock_status = true;
	dvfs->short_press = false;

	DVFS_DEV_DBG(DBG_DVFS, data->dev,
		"%s : DVFS ON [level %d]\n", __func__, dvfs->level);

	mutex_unlock(&dvfs->lock);
}

DECLARE_DVFS_DELAYED_WORK_FUNC(OFF, TOUCHKEY)
{
	struct input_booster *data =
		container_of(work, struct input_booster,
		dvfses[BOOSTER_DEVICE_TOUCHKEY].dvfs_off_work.work);
	struct booster_dvfs *dvfs = &data->dvfses[BOOSTER_DEVICE_TOUCHKEY];

	mutex_lock(&dvfs->lock);

	remove_qos(&dvfs->cpu_qos);
	remove_qos(&dvfs->mif_qos);
	remove_qos(&dvfs->int_qos);

	dvfs->lock_status = false;
	dvfs->short_press = false;

	DVFS_DEV_DBG(DBG_DVFS, data->dev,
		"%s : DVFS OFF\n", __func__);

	mutex_unlock(&dvfs->lock);
}

DECLARE_DVFS_WORK_FUNC(SET, TOUCHKEY)
{
	struct input_booster *data = (struct input_booster *)booster_data;
	struct booster_dvfs *dvfs = &data->dvfses[BOOSTER_DEVICE_TOUCHKEY];

	mutex_lock(&dvfs->lock);

	if (!dvfs->level) {
		dev_err(data->dev,
			"%s : Skip to set booster due to level 0\n", __func__);
		goto out;
	}

	switch (booster_mode) {
	case BOOSTER_MODE_ON:
		cancel_delayed_work(&dvfs->dvfs_off_work);
		if (!dvfs->lock_status && !dvfs->short_press) {
			schedule_delayed_work(&dvfs->dvfs_chg_work,
				msecs_to_jiffies(dvfs->msec_chg_time));
			dvfs->short_press = true;
		}
		break;
	case BOOSTER_MODE_OFF:
		if (dvfs->short_press) {
			cancel_delayed_work(&dvfs->dvfs_chg_work);
			schedule_work(&dvfs->dvfs_chg_work.work);
			schedule_delayed_work(&dvfs->dvfs_off_work,
				msecs_to_jiffies(dvfs->msec_off_time));
			goto out;
		}
		if (dvfs->lock_status)
			schedule_delayed_work(&dvfs->dvfs_off_work,
				msecs_to_jiffies(dvfs->msec_off_time));
		break;
	case BOOSTER_MODE_FORCE_OFF:
		if (dvfs->lock_status) {
			cancel_delayed_work(&dvfs->dvfs_chg_work);
			cancel_delayed_work(&dvfs->dvfs_off_work);
			schedule_work(&dvfs->dvfs_off_work.work);
		}
		break;
	default:
		break;
	}

out:
	mutex_unlock(&dvfs->lock);
	return;
}

/* Touch */
DECLARE_DVFS_DELAYED_WORK_FUNC(CHG, TOUCH)
{
	struct input_booster *data =
		container_of(work, struct input_booster,
		dvfses[BOOSTER_DEVICE_TOUCH].dvfs_chg_work.work);
	struct booster_dvfs *dvfs = &data->dvfses[BOOSTER_DEVICE_TOUCH];

	mutex_lock(&dvfs->lock);

	switch (dvfs->level) {
	case BOOSTER_LEVEL0:
	case BOOSTER_LEVEL1:
	case BOOSTER_LEVEL3:
		remove_qos(&dvfs->cpu_qos);
		remove_qos(&dvfs->mif_qos);
		remove_qos(&dvfs->int_qos);
		dvfs->lock_status = false;
		DVFS_DEV_DBG(DBG_DVFS, data->dev,
			"%s : DVFS OFF\n", __func__);
		break;
	case BOOSTER_LEVEL2:
		set_qos(&dvfs->cpu_qos, PM_QOS_CPU_FREQ_MIN,
			dvfs->freqs[BOOSTER_LEVEL2].cpu_freq);
		set_qos(&dvfs->mif_qos, PM_QOS_BUS_THROUGHPUT,
			dvfs->freqs[BOOSTER_LEVEL2].mif_freq);
		set_qos(&dvfs->int_qos, PM_QOS_DEVICE_THROUGHPUT,
			dvfs->freqs[BOOSTER_LEVEL2].int_freq);
		DVFS_DEV_DBG(DBG_DVFS, data->dev,
			"%s : DVFS CHANGED [level %d]\n", __func__, dvfs->level);
		break;
	case BOOSTER_LEVEL5:
		if (dvfs->touch_level5_phase2) {
			remove_qos(&dvfs->cpu_qos);
			remove_qos(&dvfs->mif_qos);
			remove_qos(&dvfs->int_qos);
			dvfs->lock_status = false;
			dvfs->touch_level5_phase2 = false;
			DVFS_DEV_DBG(DBG_DVFS, data->dev,
				"%s : DVFS OFF\n", __func__);
		} else {
			set_qos(&dvfs->cpu_qos, PM_QOS_CPU_FREQ_MIN,
				dvfs->freqs[BOOSTER_LEVEL5_CHG].cpu_freq);
			set_qos(&dvfs->mif_qos, PM_QOS_BUS_THROUGHPUT,
				dvfs->freqs[BOOSTER_LEVEL5_CHG].mif_freq);
			set_qos(&dvfs->int_qos, PM_QOS_DEVICE_THROUGHPUT,
				dvfs->freqs[BOOSTER_LEVEL5_CHG].int_freq);
			dvfs->touch_level5_phase2 = true;
			schedule_delayed_work(&dvfs->dvfs_chg_work,
				msecs_to_jiffies(BOOSTER_LEVEL5_PHASE2_TIME));
			DVFS_DEV_DBG(DBG_DVFS, data->dev,
				"%s : DVFS CHANGED [level %d]\n", __func__, dvfs->level);
		}
		break;
	case BOOSTER_LEVEL9:
		set_qos(&dvfs->cpu_qos, PM_QOS_CPU_FREQ_MIN,
			dvfs->freqs[BOOSTER_LEVEL9_CHG].cpu_freq);
		set_qos(&dvfs->mif_qos, PM_QOS_BUS_THROUGHPUT,
			dvfs->freqs[BOOSTER_LEVEL9_CHG].mif_freq);
		set_qos(&dvfs->int_qos, PM_QOS_DEVICE_THROUGHPUT,
			dvfs->freqs[BOOSTER_LEVEL9_CHG].int_freq);
		DVFS_DEV_DBG(DBG_DVFS, data->dev,
			"%s : DVFS CHANGED [level %d]\n", __func__, dvfs->level);
		break;
	default:
		dev_err(data->dev,
			"%s : Undefined type passed[%d]\n",
			__func__, dvfs->level);
		break;
	}

	mutex_unlock(&dvfs->lock);
}

DECLARE_DVFS_DELAYED_WORK_FUNC(OFF, TOUCH)
{
	struct input_booster *data =
		container_of(work, struct input_booster,
		dvfses[BOOSTER_DEVICE_TOUCH].dvfs_off_work.work);
	struct booster_dvfs *dvfs = &data->dvfses[BOOSTER_DEVICE_TOUCH];

	mutex_lock(&dvfs->lock);

	cancel_delayed_work(&dvfs->dvfs_chg_work);

	remove_qos(&dvfs->cpu_qos);
	remove_qos(&dvfs->mif_qos);
	remove_qos(&dvfs->int_qos);

	dvfs->lock_status = false;
	dvfs->touch_level5_phase2 = false;

	DVFS_DEV_DBG(DBG_DVFS, data->dev,
		"%s : DVFS OFF\n", __func__);

	mutex_unlock(&dvfs->lock);
}

DECLARE_DVFS_WORK_FUNC(SET, TOUCH)
{
	struct input_booster *data = (struct input_booster *)booster_data;
	struct booster_dvfs *dvfs = &data->dvfses[BOOSTER_DEVICE_TOUCH];

	mutex_lock(&dvfs->lock);

	if (!dvfs->level) {
		dev_err(data->dev,
			"%s : Skip to set booster due to level 0\n", __func__);
		goto out;
	}

	switch (booster_mode) {
	case BOOSTER_MODE_ON:
		cancel_delayed_work(&dvfs->dvfs_off_work);
		cancel_delayed_work(&dvfs->dvfs_chg_work);
		switch (dvfs->level) {
		case BOOSTER_LEVEL1:
		case BOOSTER_LEVEL2:
			set_qos(&dvfs->cpu_qos, PM_QOS_CPU_FREQ_MIN,
				dvfs->freqs[BOOSTER_LEVEL1].cpu_freq);
			set_qos(&dvfs->mif_qos, PM_QOS_BUS_THROUGHPUT,
				dvfs->freqs[BOOSTER_LEVEL1].mif_freq);
			set_qos(&dvfs->int_qos, PM_QOS_DEVICE_THROUGHPUT,
				dvfs->freqs[BOOSTER_LEVEL1].int_freq);
			break;
		case BOOSTER_LEVEL3:
			set_qos(&dvfs->cpu_qos, PM_QOS_CPU_FREQ_MIN,
				dvfs->freqs[BOOSTER_LEVEL3].cpu_freq);
			set_qos(&dvfs->mif_qos, PM_QOS_BUS_THROUGHPUT,
				dvfs->freqs[BOOSTER_LEVEL3].mif_freq);
			set_qos(&dvfs->int_qos, PM_QOS_DEVICE_THROUGHPUT,
				dvfs->freqs[BOOSTER_LEVEL3].int_freq);
			break;
		case BOOSTER_LEVEL5:
			set_qos(&dvfs->cpu_qos, PM_QOS_CPU_FREQ_MIN,
				dvfs->freqs[BOOSTER_LEVEL5].cpu_freq);
			set_qos(&dvfs->mif_qos, PM_QOS_BUS_THROUGHPUT,
				dvfs->freqs[BOOSTER_LEVEL5].mif_freq);
			set_qos(&dvfs->int_qos, PM_QOS_DEVICE_THROUGHPUT,
				dvfs->freqs[BOOSTER_LEVEL5].int_freq);
			break;
		case BOOSTER_LEVEL9:
			set_qos(&dvfs->cpu_qos, PM_QOS_CPU_FREQ_MIN,
				dvfs->freqs[BOOSTER_LEVEL9].cpu_freq);
			set_qos(&dvfs->mif_qos, PM_QOS_BUS_THROUGHPUT,
				dvfs->freqs[BOOSTER_LEVEL9].mif_freq);
			set_qos(&dvfs->int_qos, PM_QOS_DEVICE_THROUGHPUT,
				dvfs->freqs[BOOSTER_LEVEL9].int_freq);
			break;
		default:
			dev_err(data->dev, "%s : Undefined type passed[%d]\n",
				__func__, dvfs->level);
			break;
		}
		DVFS_DEV_DBG(DBG_DVFS, data->dev,
			"%s : DVFS ON [level %d]\n",	__func__, dvfs->level);
		if (dvfs->level == BOOSTER_LEVEL5)
			schedule_delayed_work(&dvfs->dvfs_chg_work,
				msecs_to_jiffies(BOOSTER_LEVEL5_PHASE1_TIME));
		else
			schedule_delayed_work(&dvfs->dvfs_chg_work,
				msecs_to_jiffies(dvfs->msec_chg_time));
		dvfs->lock_status = true;
		break;
	case BOOSTER_MODE_OFF:
		if (dvfs->lock_status)
			schedule_delayed_work(&dvfs->dvfs_off_work,
				msecs_to_jiffies(dvfs->msec_off_time));
		break;
	case BOOSTER_MODE_FORCE_OFF:
		if (dvfs->lock_status) {
			cancel_delayed_work(&dvfs->dvfs_chg_work);
			cancel_delayed_work(&dvfs->dvfs_off_work);
			schedule_work(&dvfs->dvfs_off_work.work);
		}
		break;
	default:
		break;
	}

out:
	mutex_unlock(&dvfs->lock);
	return;
}

/* Pen */
DECLARE_DVFS_DELAYED_WORK_FUNC(CHG, PEN)
{
	struct input_booster *data =
		container_of(work, struct input_booster,
		dvfses[BOOSTER_DEVICE_PEN].dvfs_chg_work.work);
	struct booster_dvfs *dvfs = &data->dvfses[BOOSTER_DEVICE_PEN];

	mutex_lock(&dvfs->lock);

	switch (dvfs->level) {
	case BOOSTER_LEVEL0:
	case BOOSTER_LEVEL1:
	case BOOSTER_LEVEL3:
		remove_qos(&dvfs->cpu_qos);
		remove_qos(&dvfs->mif_qos);
		remove_qos(&dvfs->int_qos);
		dvfs->lock_status = false;
		DVFS_DEV_DBG(DBG_DVFS, data->dev,
			"%s : DVFS OFF\n", __func__);
		break;
	case BOOSTER_LEVEL2:
		set_qos(&dvfs->cpu_qos, PM_QOS_CPU_FREQ_MIN,
			dvfs->freqs[BOOSTER_LEVEL2].cpu_freq);
		set_qos(&dvfs->mif_qos, PM_QOS_BUS_THROUGHPUT,
			dvfs->freqs[BOOSTER_LEVEL2].mif_freq);
		set_qos(&dvfs->int_qos, PM_QOS_DEVICE_THROUGHPUT,
			dvfs->freqs[BOOSTER_LEVEL2].int_freq);
		DVFS_DEV_DBG(DBG_DVFS, data->dev,
			"%s : DVFS CHANGED [level %d]\n", __func__, dvfs->level);
		break;
	case BOOSTER_LEVEL9:
		set_qos(&dvfs->cpu_qos, PM_QOS_CPU_FREQ_MIN,
			dvfs->freqs[BOOSTER_LEVEL9_CHG].cpu_freq);
		set_qos(&dvfs->mif_qos, PM_QOS_BUS_THROUGHPUT,
			dvfs->freqs[BOOSTER_LEVEL9_CHG].mif_freq);
		set_qos(&dvfs->int_qos, PM_QOS_DEVICE_THROUGHPUT,
			dvfs->freqs[BOOSTER_LEVEL9_CHG].int_freq);
		DVFS_DEV_DBG(DBG_DVFS, data->dev,
			"%s : DVFS CHANGED [level %d]\n", __func__, dvfs->level);
		break;
	default:
		dev_err(data->dev,
			"%s : Undefined type passed[%d]\n", __func__, dvfs->level);
		break;
	}

	mutex_unlock(&dvfs->lock);
}

DECLARE_DVFS_DELAYED_WORK_FUNC(OFF, PEN)
{
	struct input_booster *data =
		container_of(work, struct input_booster,
		dvfses[BOOSTER_DEVICE_PEN].dvfs_off_work.work);
	struct booster_dvfs *dvfs = &data->dvfses[BOOSTER_DEVICE_PEN];

	mutex_lock(&dvfs->lock);

	remove_qos(&dvfs->cpu_qos);
	remove_qos(&dvfs->mif_qos);
	remove_qos(&dvfs->int_qos);

	dvfs->lock_status = false;

	DVFS_DEV_DBG(DBG_DVFS, data->dev,
		"%s : DVFS OFF\n", __func__);

	mutex_unlock(&dvfs->lock);
}

DECLARE_DVFS_WORK_FUNC(SET, PEN)
{
	struct input_booster *data = (struct input_booster *)booster_data;
	struct booster_dvfs *dvfs = &data->dvfses[BOOSTER_DEVICE_PEN];

	mutex_lock(&dvfs->lock);

	if (!dvfs->level) {
		dev_err(data->dev,
			"%s : Skip to set booster due to level 0\n", __func__);
		goto out;
	}

	switch (booster_mode) {
	case BOOSTER_MODE_ON:
		cancel_delayed_work(&dvfs->dvfs_off_work);
		cancel_delayed_work(&dvfs->dvfs_chg_work);
		switch (dvfs->level) {
		case BOOSTER_LEVEL1:
		case BOOSTER_LEVEL2:
			set_qos(&dvfs->cpu_qos, PM_QOS_CPU_FREQ_MIN,
				dvfs->freqs[BOOSTER_LEVEL1].cpu_freq);
			set_qos(&dvfs->mif_qos, PM_QOS_BUS_THROUGHPUT,
				dvfs->freqs[BOOSTER_LEVEL1].mif_freq);
			set_qos(&dvfs->int_qos, PM_QOS_DEVICE_THROUGHPUT,
				dvfs->freqs[BOOSTER_LEVEL1].int_freq);
			break;
		case BOOSTER_LEVEL3:
			set_qos(&dvfs->cpu_qos, PM_QOS_CPU_FREQ_MIN,
				dvfs->freqs[BOOSTER_LEVEL3].cpu_freq);
			set_qos(&dvfs->mif_qos, PM_QOS_BUS_THROUGHPUT,
				dvfs->freqs[BOOSTER_LEVEL3].mif_freq);
			set_qos(&dvfs->int_qos, PM_QOS_DEVICE_THROUGHPUT,
				dvfs->freqs[BOOSTER_LEVEL3].int_freq);
			break;
		case BOOSTER_LEVEL9:
			set_qos(&dvfs->cpu_qos, PM_QOS_CPU_FREQ_MIN,
				dvfs->freqs[BOOSTER_LEVEL9].cpu_freq);
			set_qos(&dvfs->mif_qos, PM_QOS_BUS_THROUGHPUT,
				dvfs->freqs[BOOSTER_LEVEL9].mif_freq);
			set_qos(&dvfs->int_qos, PM_QOS_DEVICE_THROUGHPUT,
				dvfs->freqs[BOOSTER_LEVEL9].int_freq);
			break;
		default:
			dev_err(data->dev,
				"%s : Undefined type passed[%d]\n",
				__func__, dvfs->level);
			break;
		}
		DVFS_DEV_DBG(DBG_DVFS, data->dev,
			"%s : DVFS ON [level %d]\n",	__func__, dvfs->level);
		schedule_delayed_work(&dvfs->dvfs_chg_work,
			msecs_to_jiffies(dvfs->msec_chg_time));
		dvfs->lock_status = true;
		break;
	case BOOSTER_MODE_OFF:
		if (dvfs->lock_status)
			schedule_delayed_work(&dvfs->dvfs_off_work,
				msecs_to_jiffies(dvfs->msec_off_time));
		break;
	case BOOSTER_MODE_FORCE_OFF:
		if (dvfs->lock_status) {
			cancel_delayed_work(&dvfs->dvfs_chg_work);
			cancel_delayed_work(&dvfs->dvfs_off_work);
			schedule_work(&dvfs->dvfs_off_work.work);
		}
		break;
	default:
		break;
	}

out:
	mutex_unlock(&dvfs->lock);
	return;
}

static void input_booster_event_work(struct work_struct *work)
{
	struct input_booster *data =
		container_of(work, struct input_booster, event_work);
	struct booster_event event;
	int i;

	for (i = 0; data->front != data->rear; i++) {
		spin_lock(&data->buffer_lock);
		event = data->buffer[data->front];
		data->front = (data->front + 1) % data->buffer_size;
		spin_unlock(&data->buffer_lock);

		DVFS_DEV_DBG(DBG_EVENT, data->dev,
			"%s :[%d] Device type[%s] mode[%d]\n", __func__,
			i, booster_device_name[event.device_type], event.mode);

		switch (event.device_type) {
		case BOOSTER_DEVICE_KEY:
			DVFS_WORK_FUNC(SET, KEY)(data, event.mode);
			break;
		case BOOSTER_DEVICE_TOUCHKEY:
			DVFS_WORK_FUNC(SET, TOUCHKEY)(data, event.mode);
			break;
		case BOOSTER_DEVICE_TOUCH:
			DVFS_WORK_FUNC(SET, TOUCH)(data, event.mode);
			break;
		case BOOSTER_DEVICE_PEN:
			DVFS_WORK_FUNC(SET, PEN)(data, event.mode);
			break;
		default:
			break;
		}
	}
}

void change_tsp_level(u8 level)
{
	struct booster_dvfs *dvfs = &g_data->dvfses[BOOSTER_DEVICE_TOUCH];
	dvfs->level = level;
	DVFS_DEV_DBG(DBG_DVFS, g_data->dev,
		"TSP DVFS [level %d]\n", level);
}

void change_touchkey_level(u8 level)
{
	struct booster_dvfs *dvfs = &g_data->dvfses[BOOSTER_DEVICE_TOUCHKEY];
	dvfs->level = level;
	DVFS_DEV_DBG(DBG_DVFS, g_data->dev,
		"TouchKey DVFS [level %d]\n", level);
}

#ifdef BOOSTER_USE_INPUT_HANDLER
static void input_booster_event(struct input_handle *handle, unsigned int type,
			   unsigned int code, int value)
{
	struct input_booster *data = handle->private;
	enum booster_device_type device_type;
	struct booster_event event;

	if (!test_bit(code, data->keybit))
		return;

	DVFS_DEV_DBG(DBG_EVENT, data->dev,
		"Event type[%u] code[%u] value[%u] is reached from %s\n",
		type, code, value, handle->dev->name);

	event.device_type = data->get_device_type(code);
	if (event.device_type == BOOSTER_DEVICE_NOT_DEFINED)
		return;

	event.mode = value;
	/* put the event in to the buffer */
	spin_lock(&data->buffer_lock);
	data->buffer[data->rear] = event;
	data->rear = (data->rear + 1) % data->buffer_size;
	spin_unlock(&data->buffer_lock);

	/* call the workqueue */
	schedule_work(&data->event_work);
}

static bool input_booster_match(struct input_handler *handler, struct input_dev *dev)
{
	int i;
	struct input_booster *data =
		container_of(handler, struct input_booster, input_handler);

	for (i = 0; i < KEY_MAX; i++) {
		if (test_bit(i, data->keybit) && test_bit(i, dev->keybit))
			break;
	}

	if (i == KEY_MAX)
		return false;

	return true;
}

static int input_booster_connect(struct input_handler *handler,
					  struct input_dev *dev,
					  const struct input_device_id *id)
{
	int ret;
	struct input_handle *handle;
	struct input_booster *data =
		container_of(handler, struct input_booster, input_handler);


	handle = kzalloc(sizeof(*handle), GFP_KERNEL);
	if (!handle)
		return -ENOMEM;

	handle->dev = dev;
	handle->handler = handler;
	handle->name = INPUT_BOOSTER_NAME;
	handle->private = data;

	ret = input_register_handle(handle);
	if (ret)
		goto err_input_register_handle;

	ret = input_open_device(handle);
	if (ret)
		goto err_input_open_device;

	dev_info(data->dev,
		"Connect input dev %s for input booster\n",
		dev->name);

	return 0;

err_input_open_device:
	input_unregister_handle(handle);
err_input_register_handle:
	kfree(handle);
	return ret;
}

static void input_booster_disconnect(struct input_handle *handle)
{
	struct input_booster *data = handle->private;

	dev_info(data->dev,
		"Disconnect input dev %s for input booster\n",
		handle->dev->name);

	input_close_device(handle);
	input_unregister_handle(handle);
	kfree(handle);
}

static const struct input_device_id input_booster_ids[] = {
	{
		.flags = INPUT_DEVICE_ID_MATCH_EVBIT,
		.evbit = { BIT_MASK(EV_KEY) },
	},
	{ },
};
MODULE_DEVICE_TABLE(input, input_booster_ids);
#else
/* Caution: do not use amu sleep code in below function */
void input_booster_send_event(unsigned int code, int value)
{
	enum booster_device_type device_type;
	struct booster_event event;

	if (!g_data || !test_bit(code, g_data->keybit))
		return;

	event.device_type = g_data->get_device_type(code);
	if (event.device_type == BOOSTER_DEVICE_NOT_DEFINED)
		return;
	event.mode = value;

	spin_lock(&g_data->buffer_lock);
	g_data->buffer[g_data->rear] = event;
	g_data->rear = (g_data->rear + 1) % g_data->buffer_size;
	spin_unlock(&g_data->buffer_lock);

	/* call the workqueue */
	schedule_work(&g_data->event_work);
}
EXPORT_SYMBOL(input_booster_send_event);
#endif

static void	input_booster_lookup_freqs(struct booster_dvfs *dvfs)
{
	struct input_booster *data =
		container_of(dvfs, struct input_booster, dvfses[dvfs->device_type]);
	int i;

	dev_info(data->dev,
		"#############################################################\n");
	for (i = 0; i < BOOSTER_LEVEL_MAX; i++) {
		dev_info(data->dev, "%s: %s's LEVEL%d cpu %d mif %d int %d\n",
			__func__, dvfs->name, i, dvfs->freqs[i].cpu_freq,
			dvfs->freqs[i].mif_freq, dvfs->freqs[i].int_freq);
	}
	dev_info(data->dev,
		"#############################################################\n");
}

#ifdef BOOSTER_SYSFS
static ssize_t input_booster_get_debug_level(struct class *dev,
		struct class_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", DbgLevel);
}

static ssize_t input_booster_set_debug_level(struct class *dev,
		struct class_attribute *attr, const char *buf, size_t count)
{
	unsigned long val;

	if (strict_strtoul(buf, 0, &val) < 0)
		return -EINVAL;

	DbgLevel = (unsigned int)val;

	return count;
}

static CLASS_ATTR(debug_level, S_IRUGO | S_IWUSR,
	input_booster_get_debug_level, input_booster_set_debug_level);

static ssize_t input_booster_get_dvfs_level(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct booster_dvfs *dvfs = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", dvfs->level);
}

static ssize_t input_booster_set_dvfs_level(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct booster_dvfs *dvfs = dev_get_drvdata(dev);
	struct input_booster *data =
		container_of(dvfs, struct input_booster, dvfses[dvfs->device_type]);
	unsigned long val;

	if (strict_strtoul(buf, 0, &val) < 0)
		return -EINVAL;

	dvfs->level = (unsigned int)val;

	dev_info(data->dev, "%s: %s's LEVEL [%d]\n",
			__func__, dvfs->name, dvfs->level);

	return count;
}

static ssize_t input_booster_get_dvfs_freq(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct booster_dvfs *dvfs = dev_get_drvdata(dev);

	input_booster_lookup_freqs(dvfs);

	return 0;
}

static ssize_t input_booster_set_dvfs_freq(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct booster_dvfs *dvfs = dev_get_drvdata(dev);
	struct input_booster *data =
		container_of(dvfs, struct input_booster, dvfses[dvfs->device_type]);
	int level;
	unsigned long cpu_freq, mif_freq, int_freq;

	if (sscanf(buf, "%d%lu%lu%lu",
		&level, &cpu_freq, &mif_freq, &int_freq) != 4) {
		dev_err(data->dev,
			"### Keep this format : [level cpu_freq mif_freq int_freq] (Ex: 1 1600000 667000 333000 ###\n");
		goto out;
	}

	if (level >= BOOSTER_LEVEL_MAX) {
		dev_err(data->dev,
			"%s : Entered level is not permitted\n", __func__);
		goto out;
	}

	dvfs->freqs[level].cpu_freq = cpu_freq;
	dvfs->freqs[level].mif_freq = mif_freq;
	dvfs->freqs[level].int_freq = int_freq;

	input_booster_lookup_freqs(dvfs);

out:
	return count;
}

static DEVICE_ATTR(level, S_IRUGO | S_IWUSR,
	input_booster_get_dvfs_level, input_booster_set_dvfs_level);
static DEVICE_ATTR(freq, S_IRUGO | S_IWUSR,
	input_booster_get_dvfs_freq, input_booster_set_dvfs_freq);

static struct attribute *dvfs_attributes[] = {
	&dev_attr_level.attr,
	&dev_attr_freq.attr,
	NULL,
};

static struct attribute_group dvfs_attr_group = {
	.attrs = dvfs_attributes,
};

static void input_booster_free_sysfs(struct input_booster *data)
{
	class_remove_file(data->class, &class_attr_debug_level);
	class_destroy(data->class);
}

static int __devinit input_booster_init_sysfs(struct input_booster *data)
{
	int ret;

	data->class = class_create(THIS_MODULE, "input_booster");
	if (IS_ERR(data->class)) {
		dev_err(data->dev, "Failed to create class\n");
		ret = IS_ERR(data->class);
		goto err_create_class;
	}

	ret = class_create_file(data->class, &class_attr_debug_level);
	if (ret) {
		dev_err(data->dev, "Failed to create class\n");
		goto err_create_class_file;
	}

	return 0;

err_create_class_file:
	class_destroy(data->class);
err_create_class:
	return ret;
}

static void input_booster_free_dvfs_sysfs(struct booster_dvfs *dvfs)
{
	struct input_booster *data =
		container_of(dvfs, struct input_booster, dvfses[dvfs->device_type]);

	sysfs_remove_group(&dvfs->sysfs_dev->kobj,
		&dvfs_attr_group);
	device_destroy(data->class, 0);
}

static void input_booster_init_dvfs_sysfs(struct booster_dvfs *dvfs)
{
	int ret;
	struct input_booster *data =
		container_of(dvfs, struct input_booster, dvfses[dvfs->device_type]);

	dvfs->sysfs_dev = device_create(data->class,
			NULL, 0, dvfs, dvfs->name);
	if (IS_ERR(dvfs->sysfs_dev)) {
		ret = IS_ERR(dvfs->sysfs_dev);
		dev_err(data->dev, "Failed to create %s sysfs device[%d]\n",
			dvfs->name, ret);
		goto err_create_device;
	}

	ret = sysfs_create_group(&dvfs->sysfs_dev->kobj,
			&dvfs_attr_group);
	if (ret) {
		dev_err(data->dev, "Failed to create %s sysfs group\n",
			dvfs->name);
		goto err_create_group;
	}

	return;

err_create_group:
err_create_device:

	return ;
}
#endif


static void input_booster_free_dvfs(struct input_booster *data)
{
	int i;

	cancel_work_sync(&data->event_work);
	for (i = 0; i < BOOSTER_DEVICE_MAX; i++) {
		if (data->dvfses[i].initialized) {
			cancel_delayed_work(&data->dvfses[i].dvfs_chg_work);
			cancel_delayed_work(&data->dvfses[i].dvfs_off_work);
#ifdef BOOSTER_SYSFS
			input_booster_free_dvfs_sysfs(&data->dvfses[i]);
#endif
		}
	}
}

static void input_booster_init_dvfs(struct input_booster *data,
	struct booster_key *key)
{
	enum booster_device_type device_type = data->get_device_type(key->code);
	struct booster_dvfs *dvfs;

	if (device_type >= BOOSTER_DEVICE_MAX) {
		dev_err(data->dev,
			"%s: Fail to init booster dvfs due to irregal device type[%d][%s]\n",
			__func__, device_type, key->desc);
		return;
	}

	if (data->dvfses[device_type].initialized) {
		dev_info(data->dev, "%s: device %s type is is already initialized [key = %s]\n",
			__func__, booster_device_name[device_type], key->desc);
		return;
	}

	dvfs = &data->dvfses[device_type];

	switch (device_type) {
	case BOOSTER_DEVICE_KEY:
		INIT_DELAYED_WORK(&dvfs->dvfs_chg_work,
			DVFS_WORK_FUNC(CHG, KEY));
		INIT_DELAYED_WORK(&dvfs->dvfs_off_work,
			DVFS_WORK_FUNC(OFF, KEY));
		dvfs->set_dvfs_lock = DVFS_WORK_FUNC(SET, KEY);
		break;
	case BOOSTER_DEVICE_TOUCHKEY:
		INIT_DELAYED_WORK(&dvfs->dvfs_chg_work,
			DVFS_WORK_FUNC(CHG, TOUCHKEY));
		INIT_DELAYED_WORK(&dvfs->dvfs_off_work,
			DVFS_WORK_FUNC(OFF, TOUCHKEY));
		dvfs->set_dvfs_lock = DVFS_WORK_FUNC(SET, TOUCHKEY);
		break;
	case BOOSTER_DEVICE_TOUCH:
		INIT_DELAYED_WORK(&dvfs->dvfs_chg_work,
			DVFS_WORK_FUNC(CHG, TOUCH));
		INIT_DELAYED_WORK(&dvfs->dvfs_off_work,
			DVFS_WORK_FUNC(OFF, TOUCH));
		dvfs->set_dvfs_lock = DVFS_WORK_FUNC(SET, TOUCH);
		break;
	case BOOSTER_DEVICE_PEN:
		INIT_DELAYED_WORK(&dvfs->dvfs_chg_work,
			DVFS_WORK_FUNC(CHG, PEN));
		INIT_DELAYED_WORK(&dvfs->dvfs_off_work,
			DVFS_WORK_FUNC(OFF, PEN));
		dvfs->set_dvfs_lock = DVFS_WORK_FUNC(SET, PEN);
		break;
	default:
		dev_err(data->dev,
			"%s: Fail to init booster dvfs due to irregal device type[%d]\n",
			__func__, device_type);
		return;
		break;
	}

	dvfs->level = BOOSTER_LEVEL2;
	dvfs->msec_chg_time = key->msec_chg_time;
	dvfs->msec_off_time = key->msec_off_time;
	memcpy(dvfs->freqs, key->freq_table,
		sizeof(struct dvfs_freq) * BOOSTER_LEVEL_MAX);

	dvfs->device_type = device_type;
	dvfs->name = booster_device_name[device_type];
#ifdef BOOSTER_SYSFS
	input_booster_init_dvfs_sysfs(dvfs);
#endif
	mutex_init(&dvfs->lock);
	dvfs->initialized = true;

	dev_info(data->dev,
		"%s: %s is registered and intialized for %s dvfs\n",
		__func__, key->desc, dvfs->name);
#ifdef DEBUG
	input_booster_lookup_freqs(dvfs);
#endif
}

static int __devinit input_booster_probe(struct platform_device *pdev)
{
	int ret = 0, i;
	struct input_booster *data;
	const struct input_booster_platform_data *pdata =
		pdev->dev.platform_data;

	if (!pdata)
		return -EINVAL;

	if (!pdata->nkeys || !pdata->keys) {
		dev_err(&pdev->dev,
			"There are no keys for booster\n");
		return -EINVAL;
	}

	if (!pdata->get_device_type) {
		dev_err(&pdev->dev,
			"There are no get_device_type for booster\n");
		return -EINVAL;
	}

	data = kzalloc(sizeof(*data)
			+ sizeof(*data->keys) * pdata->nkeys, GFP_KERNEL);

	if (!data)
		return -ENOMEM;

	memcpy(data->keys, pdata->keys,
		sizeof(struct booster_key) * pdata->nkeys);

	data->nkeys = pdata->nkeys;
	data->get_device_type = pdata->get_device_type;
	data->dev = &pdev->dev;
#ifdef BOOSTER_USE_INPUT_HANDLER
	data->input_handler.event = input_booster_event;
	data->input_handler.match = input_booster_match;
	data->input_handler.connect = input_booster_connect;
	data->input_handler.disconnect = input_booster_disconnect;
	data->input_handler.name = INPUT_BOOSTER_NAME;
	data->input_handler.id_table = input_booster_ids;
#endif

	g_data = data;

#ifdef BOOSTER_SYSFS
	ret = input_booster_init_sysfs(data);
	if (ret)
		goto err_init_sysfs;
#endif

	spin_lock_init(&data->buffer_lock);
	data->buffer_size = sizeof(data->buffer)/sizeof(struct booster_event);
	INIT_WORK(&data->event_work, input_booster_event_work);

	for (i = 0; i < data->nkeys; i++) {
		if (data->keys[i].code >= KEY_MAX)
			continue;

		input_booster_init_dvfs(data, &data->keys[i]);
		__set_bit(data->keys[i].code, data->keybit);
	}

#ifdef BOOSTER_USE_INPUT_HANDLER
	ret = input_register_handler(&data->input_handler);
	if (ret)
		goto err_register_handler;
#endif
	platform_set_drvdata(pdev, data);

	return 0;

err_register_handler:
	input_booster_free_dvfs(data);
#ifdef BOOSTER_SYSFS
	input_booster_free_sysfs(data);
#endif
err_init_sysfs:
	kfree(data);

	return ret;
}

static int __devexit input_booster_remove(struct platform_device *pdev)
{
	struct input_booster *data = platform_get_drvdata(pdev);
#ifdef BOOSTER_USE_INPUT_HANDLER
	input_unregister_handler(&data->input_handler);
#endif
	input_booster_free_dvfs(data);
#ifdef BOOSTER_SYSFS
	input_booster_free_sysfs(data);
#endif
	kfree(data);

	return 0;
}

static struct platform_driver input_booster_driver = {
	.driver.name = INPUT_BOOSTER_NAME,
	.probe = input_booster_probe,
	.remove = input_booster_remove,
};

static int __init input_booster_init(void)
{
	return platform_driver_register(&input_booster_driver);
}

static void __exit input_booster_exit(void)
{
	return platform_driver_unregister(&input_booster_driver);
}

late_initcall(input_booster_init);
module_exit(input_booster_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SAMSUNG Electronics");
MODULE_DESCRIPTION("SEC INPUT BOOSTER DEVICE");
