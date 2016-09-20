#include 	"gpu_sysfs_header.h"

/* *
 * *********************************************************************
 * Path defines for all the sysfs files.
 * *********************************************************************
 * */
#define  	GPU_MIN_CLOCK		"/sys/devices/platform/mali.0/min_clock"
#define  	GPU_MAX_CLOCK		"/sys/devices/platform/mali.0/max_clock"
#define  	GPU_FREQ		"/sys/devices/platform/mali.0/clock"
#define  	GPU_BUSY		"/sys/devices/platform/mali.0/utilization"
#define  	GPU_VOL			"/sys/devices/platform/mali.0/vol"
#define  	GPU_FREQ_TABLE		"/sys/devices/platform/mali.0/dvfs_table"
#define  	GPU_GOVERNOR		"/sys/devices/platform/mali.0/power_policy"
#define  	GPU_CORES_CONFIG	"/sys/devices/platform/mali.0/core_mask"
#define  	GPU_TMU			"/sys/devices/platform/mali.0/tmu"
#define  	GPU_MODEL		"/sys/devices/platform/mali.0/uevent"
#define  	GPU_VERSION		"/sys/devices/platform/mali.0/gpuinfo" /* Original was gpu_version */
#define  	GPU_MEM			"/sys/devices/platform/mali.0/mem_pool_size" /* Original was gpu_mem */
#define  	GPU_DVFS		"/sys/devices/platform/mali.0/dvfs"
#define  	GPU_THERMAL1		"/sys/devices/platform/mali.0/throttling1"
#define  	GPU_THERMAL2		"/sys/devices/platform/mali.0/throttling2"
#define  	GPU_THERMAL3		"/sys/devices/platform/mali.0/throttling3"
#define  	GPU_THERMAL4		"/sys/devices/platform/mali.0/throttling4"
#define  	GPU_THERMAL_TRIPPING	"/sys/devices/platform/mali.0/tripping"
#define		GPU_VOLT_TABLE		"/sys/devices/platform/mali.0/volt_table"

/* *
 * *********************************************************************
 * Device ATTR functions. Will be called when read from sysfs.
 * *********************************************************************
 * */
ssize_t gpu_min_clock_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	char   	input_buffer[INPUT_BUFFER_SIZE_256];
	int		status = 0;
	char 	output_str[INPUT_BUFFER_SIZE_16] = {0};

	status = open_file_and_read_buffer(GPU_FREQ_TABLE, input_buffer, INPUT_BUFFER_SIZE_256);

	if (status != SRUK_TRUE)
	{
		return sprintf(buf, "-1");
	}

	/* ******************* */
	/* Parse input to find
	 * Minimum Clock. This is
	 * target specific.
	 * */
	/* ******************* */
	{
		int len = strlen(input_buffer);
		char *end_of_string = input_buffer + (len -1);
		while(*end_of_string != ' ')
		{
			end_of_string--;
		}
		strcpy(output_str, end_of_string);
	}
	/* ******************* */

	return sprintf(buf, "%s", output_str);
}

ssize_t gpu_max_clock_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	char   	input_buffer[INPUT_BUFFER_SIZE_256];
	int		status = 0;
	char 	output_str[INPUT_BUFFER_SIZE_16] = {0};
	int 	i = 1; /* GPU Freq has one space at the start. Target specific. */

	status = open_file_and_read_buffer(GPU_FREQ_TABLE, input_buffer, INPUT_BUFFER_SIZE_256);

	if (status != SRUK_TRUE)
	{
		return sprintf(buf, "-1");
	}

	/* ******************* */
	/* Parse input to find
	 * Minimum Clock. This is
	 * target specific.
	 * */
	/* ******************* */
	{
		char *start_of_string = input_buffer + i;
		while(*start_of_string != ' ')
		{
			i++;
			start_of_string++;
		}
		strncpy(output_str, input_buffer, i);
	}
	/* ******************* */

	return sprintf(buf, "%s\n", output_str);
}

ssize_t gpu_busy_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	char   	input_buffer[INPUT_BUFFER_SIZE_32];
	int		status = 0;

	status = open_file_and_read_buffer(GPU_BUSY, input_buffer, INPUT_BUFFER_SIZE_32);

	if (status == SRUK_TRUE)
	{
		return sprintf(buf, "%s", input_buffer);
	}
	else
	{
		return sprintf(buf, "-1");
	}
}

ssize_t gpu_vol_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	char   	input_buffer[INPUT_BUFFER_SIZE_32];
	int		status = 0;

	status = open_file_and_read_buffer(GPU_VOL, input_buffer, INPUT_BUFFER_SIZE_32);

	if (status == SRUK_TRUE)
	{
		return sprintf(buf, "%s", input_buffer);
	}
	else
	{
		return sprintf(buf, "-1");
	}
}

ssize_t gpu_freq_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	char   	input_buffer[INPUT_BUFFER_SIZE_32];
	int		status = 0;

	status = open_file_and_read_buffer(GPU_FREQ, input_buffer, INPUT_BUFFER_SIZE_32);

	if (status == SRUK_TRUE)
	{
		return sprintf(buf, "%s", input_buffer);
	}
	else
	{
		return sprintf(buf, "-1");
	}
}

ssize_t gpu_freq_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	pr_info("SRUK ----------- %s -- %d", __FUNCTION__, __LINE__);
	if (open_file_and_write_buffer(GPU_GOVERNOR, "always_on", strlen("always_on")) == 0)
	{
		pr_info("SRUK ----------- %s -- %d", __FUNCTION__, __LINE__);
		return 0;
	}

    pr_info("SRUK ----------- %s -- %d", __FUNCTION__, __LINE__);
	if (open_file_and_write_buffer(GPU_DVFS, "0", strlen("0")) == 0)
	{
		pr_info("SRUK ----------- %s -- %d", __FUNCTION__, __LINE__);
		return 0;
	}

    pr_info("SRUK ----------- %s -- %d", __FUNCTION__, __LINE__);
	if (open_file_and_write_buffer(GPU_FREQ, buf, strlen(buf)) == 0)
	{
		pr_info("SRUK ----------- %s -- %d", __FUNCTION__, __LINE__);
		return 0;
	}

	pr_info("SRUK ----------- %s -- %d", __FUNCTION__, __LINE__);

	/* Return success status. */
	return count;
}

ssize_t gpu_freq_table_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	char   	input_buffer[INPUT_BUFFER_SIZE_128];
	int		status = 0;

	status = open_file_and_read_buffer(GPU_FREQ_TABLE, input_buffer, INPUT_BUFFER_SIZE_128);

	if (status == SRUK_TRUE)
	{
		return sprintf(buf, "%s", input_buffer);
	}
	else
	{
		return sprintf(buf, "-1");
	}
}

ssize_t gpu_governor_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	char   	input_buffer[INPUT_BUFFER_SIZE_128];
	int		status = 0;

	status = open_file_and_read_buffer(GPU_GOVERNOR, input_buffer, INPUT_BUFFER_SIZE_128);

	if (status == SRUK_TRUE)
	{
		return sprintf(buf, "%s", input_buffer);
	}
	else
	{
		return sprintf(buf, "-1");
	}
}

ssize_t gpu_governor_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	pr_info("SRUK ----------- %s -- %d", __FUNCTION__, __LINE__);
	if (open_file_and_write_buffer(GPU_GOVERNOR, buf, strlen(buf)) == 0)
	{
		pr_info("SRUK ----------- %s -- %d", __FUNCTION__, __LINE__);
		return 0;
	}

	pr_info("SRUK ----------- %s -- %d", __FUNCTION__, __LINE__);

	/* Return success status. */
	return count;
}

ssize_t gpu_available_governor_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	char   	input_buffer[INPUT_BUFFER_SIZE_128];
	int		status = 0;

	status = open_file_and_read_buffer(GPU_GOVERNOR, input_buffer, INPUT_BUFFER_SIZE_128);

	if (status == SRUK_TRUE)
	{
		return sprintf(buf, "%s", input_buffer);
	}
	else
	{
		return sprintf(buf, "-1");
	}
}

ssize_t gpu_cores_config_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	char   	input_buffer[INPUT_BUFFER_SIZE_32];
	int		status = 0;

	status = open_file_and_read_buffer(GPU_CORES_CONFIG, input_buffer, INPUT_BUFFER_SIZE_32);

	if (status == SRUK_TRUE)
	{
		return sprintf(buf, "%s", input_buffer);
	}
	else
	{
		return sprintf(buf, "-1");
	}
}

ssize_t gpu_tmu_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	char   	input_buffer[INPUT_BUFFER_SIZE_32];
	int		status = 0;

	status = open_file_and_read_buffer(GPU_TMU, input_buffer, INPUT_BUFFER_SIZE_32);

	if (status == SRUK_TRUE)
	{
		return sprintf(buf, "%s", input_buffer);
	}
	else
	{
		return sprintf(buf, "-1");
	}
}

ssize_t gpu_model_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	char   		 	input_buffer[INPUT_BUFFER_SIZE_128];
	char   		 	model_buffer[INPUT_BUFFER_SIZE_32];
	int				status = 0;
	char 			*parse_pointer = input_buffer;
	int    			char_counter = 0, equal_char_counter = 0;

	status = open_file_and_read_buffer(GPU_MODEL, input_buffer, INPUT_BUFFER_SIZE_128);

	if (status != SRUK_TRUE)
	{
		return sprintf(buf, "-1");
	}

	/* ************************************ */
	/* Parse input to find gpu version.
	 * This is target specific.
	 * The driver gives information in following
	 * format:
	 *    DRIVER=mali
	 *    OF_NAME=mali
	 *    OF_FULLNAME=/mali
	 *    OF_COMPATIBLE_0=arm,mali
	 *    OF_COMPATIBLE_N=1
	 *    MODALIAS=of:NmaliT<NULL>Carm,mali
	 * */
	/* ************************************ */
	while (*parse_pointer != '\0')
	{
		/* Look for '='. */
		if (*parse_pointer == '=')
		{
			equal_char_counter=char_counter;
			equal_char_counter=equal_char_counter+1; /* Point to next char. */
		}

		if ((*parse_pointer == '\n') || (*parse_pointer == '\r'))
		{
			break;
		}

		/* */
		parse_pointer++;
		char_counter++;
	}

	strncpy(model_buffer, input_buffer + equal_char_counter, (char_counter-equal_char_counter));

	/* Adding null terminator, in case if not there already. */
	if (*(model_buffer + (char_counter-equal_char_counter)) != '\0')
		*(model_buffer + (char_counter-equal_char_counter)) = '\0';

	/* Copy the model string to the output string.*/
	return sprintf(buf, "%s\n", model_buffer);
}

ssize_t gpu_version_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	char   	input_buffer[INPUT_BUFFER_SIZE_64];
	int		status = 0;

	status = open_file_and_read_buffer(GPU_VERSION, input_buffer, INPUT_BUFFER_SIZE_64);

	if (status == SRUK_TRUE)
	{
		return sprintf(buf, "%s", input_buffer);
	}
	else
	{
		return sprintf(buf, "-1");
	}
}

ssize_t gpu_mem_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	char   	input_buffer[INPUT_BUFFER_SIZE_64];
	int		status = 0;

	status = open_file_and_read_buffer(GPU_MEM, input_buffer, INPUT_BUFFER_SIZE_64);

	if (status == SRUK_TRUE)
	{
		return sprintf(buf, "%s", input_buffer);
	}
	else
	{
		return sprintf(buf, "-1");
	}
}

ssize_t gpu_thermal1_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	char   	input_buffer[INPUT_BUFFER_SIZE_128];
	int		status = 0;

	status = open_file_and_read_buffer(GPU_THERMAL1, input_buffer, INPUT_BUFFER_SIZE_128);

	if (status == SRUK_TRUE)
	{
		return sprintf(buf, "%s", input_buffer);
	}
	else
	{
		return sprintf(buf, "-1");
	}
}

ssize_t gpu_thermal1_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	pr_info("SRUK ----------- %s -- %d", __FUNCTION__, __LINE__);
	if (open_file_and_write_buffer(GPU_THERMAL1, buf, strlen(buf)) == 0)
	{
		pr_info("SRUK ----------- %s -- %d", __FUNCTION__, __LINE__);
		return 0;
	}

	pr_info("SRUK ----------- %s -- %d", __FUNCTION__, __LINE__);

	/* Return success status. */
	return count;
}

ssize_t gpu_thermal2_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	char   	input_buffer[INPUT_BUFFER_SIZE_128];
	int		status = 0;

	status = open_file_and_read_buffer(GPU_THERMAL2, input_buffer, INPUT_BUFFER_SIZE_128);

	if (status == SRUK_TRUE)
	{
		return sprintf(buf, "%s", input_buffer);
	}
	else
	{
		return sprintf(buf, "-1");
	}
}

ssize_t gpu_thermal2_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	pr_info("SRUK ----------- %s -- %d", __FUNCTION__, __LINE__);
	if (open_file_and_write_buffer(GPU_THERMAL2, buf, strlen(buf)) == 0)
	{
		pr_info("SRUK ----------- %s -- %d", __FUNCTION__, __LINE__);
		return 0;
	}

	pr_info("SRUK ----------- %s -- %d", __FUNCTION__, __LINE__);

	/* Return success status. */
	return count;
}

ssize_t gpu_thermal3_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	char   	input_buffer[INPUT_BUFFER_SIZE_128];
	int		status = 0;

	status = open_file_and_read_buffer(GPU_THERMAL3, input_buffer, INPUT_BUFFER_SIZE_128);

	if (status == SRUK_TRUE)
	{
		return sprintf(buf, "%s", input_buffer);
	}
	else
	{
		return sprintf(buf, "-1");
	}
}

ssize_t gpu_thermal3_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	pr_info("SRUK ----------- %s -- %d", __FUNCTION__, __LINE__);
	if (open_file_and_write_buffer(GPU_THERMAL3, buf, strlen(buf)) == 0)
	{
		pr_info("SRUK ----------- %s -- %d", __FUNCTION__, __LINE__);
		return 0;
	}

	pr_info("SRUK ----------- %s -- %d", __FUNCTION__, __LINE__);

	/* Return success status. */
	return count;
}

ssize_t gpu_thermal4_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	char   	input_buffer[INPUT_BUFFER_SIZE_128];
	int		status = 0;

	status = open_file_and_read_buffer(GPU_THERMAL4, input_buffer, INPUT_BUFFER_SIZE_128);

	if (status == SRUK_TRUE)
	{
		return sprintf(buf, "%s", input_buffer);
	}
	else
	{
		return sprintf(buf, "-1");
	}
}

ssize_t gpu_thermal4_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	pr_info("SRUK ----------- %s -- %d", __FUNCTION__, __LINE__);
	if (open_file_and_write_buffer(GPU_THERMAL4, buf, strlen(buf)) == 0)
	{
		pr_info("SRUK ----------- %s -- %d", __FUNCTION__, __LINE__);
		return 0;
	}

	pr_info("SRUK ----------- %s -- %d", __FUNCTION__, __LINE__);

	/* Return success status. */
	return count;
}

ssize_t gpu_thermal_tripping(struct device *dev, struct device_attribute *attr, char *buf)
{
	char   	input_buffer[INPUT_BUFFER_SIZE_128];
	int		status = 0;

	status = open_file_and_read_buffer(GPU_THERMAL_TRIPPING, input_buffer, INPUT_BUFFER_SIZE_128);

	if (status == SRUK_TRUE)
	{
		return sprintf(buf, "%s", input_buffer);
	}
	else
	{
		return sprintf(buf, "-1");
	}
}
ssize_t gpu_volt_table_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	char   	input_buffer[INPUT_BUFFER_SIZE_128];
	int		status = 0;

	status = open_file_and_read_buffer(GPU_VOLT_TABLE, input_buffer, INPUT_BUFFER_SIZE_128);

	if (status == SRUK_TRUE)
	{
		return sprintf(buf, "%s", input_buffer);
	}
	else
	{
		return sprintf(buf, "-1");
	}
}

ssize_t gpu_volt_table_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	pr_info("SRUK ----------- %s -- %d", __FUNCTION__, __LINE__);
	if (open_file_and_write_buffer(GPU_VOLT_TABLE, buf, strlen(buf)) == 0)
	{
		pr_info("SRUK ----------- %s -- %d", __FUNCTION__, __LINE__);
		return 0;
	}

	pr_info("SRUK ----------- %s -- %d", __FUNCTION__, __LINE__);

	/* Return success status. */
	return count;
}
