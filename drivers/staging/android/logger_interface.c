/*
 * Author: andip71, 21.04.2013
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */


#include <linux/kobject.h>
#include <linux/sysfs.h>
#include "logger_interface.h"


int logger_mode;


/* sysfs interface for logger mode */

static ssize_t logger_mode_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{

	// print current mode
	if (logger_mode == 0)
	{
		return sprintf(buf, "logger mode: %d (disabled)", logger_mode);
	}
	else
	{
		return sprintf(buf, "logger mode: %d (enabled)", logger_mode);
	}

}


static ssize_t logger_mode_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{

	unsigned int ret = -EINVAL;
	int val;

	// read value from input buffer
	ret = sscanf(buf, "%d", &val);

	// check value and store if valid
	if ((val == 0) ||  (val == 1))
	{
		logger_mode = val;
	}

	return count;
}


/* Initialize logger_mode sysfs folder */

static struct kobj_attribute logger_mode_attribute =
__ATTR(logger_mode, 0666, logger_mode_show, logger_mode_store);

static struct attribute *logger_mode_attrs[] = {
&logger_mode_attribute.attr,
NULL,
};

static struct attribute_group logger_mode_attr_group = {
.attrs = logger_mode_attrs,
};

static struct kobject *logger_mode_kobj;

int logger_mode_init(void)
{
	int logger_mode_retval;

        logger_mode_kobj = kobject_create_and_add("logger_mode", kernel_kobj);

        if (!logger_mode_kobj) {
                return -ENOMEM;
        }

        logger_mode_retval = sysfs_create_group(logger_mode_kobj, &logger_mode_attr_group);

        if (logger_mode_retval)
	{
			kobject_put(logger_mode_kobj);
	}

	// initialize logger mode to 0 (disabled) as default
	logger_mode = 0;

        return (logger_mode_retval);
}


void logger_mode_exit(void)
{
	kobject_put(logger_mode_kobj);
}


/* define driver entry points */
module_init(logger_mode_init);
module_exit(logger_mode_exit);
