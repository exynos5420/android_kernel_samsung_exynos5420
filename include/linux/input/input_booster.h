#ifndef _LINUX_INPUT_BOOSTER_H
#define _LINUX_INPUT_BOOSTER_H

#define INPUT_BOOSTER_NAME "input_booster"

/* We can use input handler to catch up specific events which are
 * used for the input booster.
 */
 #if 0
#define BOOSTER_USE_INPUT_HANDLER
#endif

#define BOOSTER_DEFAULT_ON_TIME		500	/* msec */
#define BOOSTER_DEFAULT_OFF_TIME	500	/* msec */
#define BOOSTER_DEFAULT_CHG_TIME	130	/* msec */
#define BOOSTER_LEVEL5_PHASE1_TIME	40
#define BOOSTER_LEVEL5_PHASE2_TIME	80

enum booster_level {
	BOOSTER_LEVEL0 = 0,	/* OFF */
	BOOSTER_LEVEL1,
	BOOSTER_LEVEL2,
	BOOSTER_LEVEL3,
	BOOSTER_LEVEL5 = 5,
	BOOSTER_LEVEL9 = 9,
	BOOSTER_LEVEL5_CHG,
	BOOSTER_LEVEL9_CHG,
	BOOSTER_LEVEL_MAX,
};

enum booster_device_type {
	BOOSTER_DEVICE_KEY = 0,
	BOOSTER_DEVICE_TOUCHKEY,
	BOOSTER_DEVICE_TOUCH,
	BOOSTER_DEVICE_PEN,
	BOOSTER_DEVICE_MAX,
	BOOSTER_DEVICE_NOT_DEFINED,
};

enum booster_mode {
	BOOSTER_MODE_OFF = 0,
	BOOSTER_MODE_ON,
	BOOSTER_MODE_FORCE_OFF,
};

#define BOOSTER_DVFS_FREQ(_cpu_freq, _mif_freq, _int_freq)	\
{								\
	.cpu_freq = _cpu_freq,		\
	.mif_freq = _mif_freq,		\
	.int_freq = _int_freq,		\
}

struct dvfs_freq {
	s32 cpu_freq;
	s32 mif_freq;
	s32 int_freq;
};

#define BOOSTER_KEYS(_name, _code, _chg_time, _off_time, _freq_table)	\
{									\
	.desc = _name,					\
	.code = _code,					\
	.msec_chg_time = _chg_time,		\
	.msec_off_time = _off_time,		\
	.freq_table = _freq_table,		\
}

struct booster_key {
	const char *desc;
	int code;
	int msec_chg_time;
	int msec_off_time;
	const struct dvfs_freq *freq_table;
};

struct input_booster_platform_data {
	struct booster_key *keys;
	int nkeys;
	enum booster_device_type (*get_device_type) (int code);
};

#ifdef BOOSTER_USE_INPUT_HANDLER
#define INPUT_BOOSTER_SEND_EVENT(code, value)
#define INPUT_BOOSTER_REPORT_KEY_EVENT(dev, code, value)	\
do {									\
	input_report_key(dev, code, value);	\
} while(0)
#else
extern void input_booster_send_event(unsigned int code, int value);

#define INPUT_BOOSTER_SEND_EVENT(code, value)	\
		input_booster_send_event(code, value)
#define INPUT_BOOSTER_REPORT_KEY_EVENT(dev, code, value)
#endif

extern void change_tsp_level(u8 level);
extern void change_touchkey_level(u8 level);

#endif /* _LINUX_INPUT_BOOSTER_H */
