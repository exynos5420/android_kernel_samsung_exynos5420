#ifndef _LINUX_WACOM_I2C_H
#define _LINUX_WACOM_I2C_H

#include <linux/types.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

/*I2C address for digitizer and its boot loader*/
#define WACOM_I2C_ADDR 0x56
#if defined(CONFIG_EPEN_WACOM_G9PL) \
	|| defined(CONFIG_EPEN_WACOM_G9PLL) \
	|| defined(CONFIG_EPEN_WACOM_G10PM)
#define WACOM_I2C_BOOT 0x09
#else
#define WACOM_I2C_BOOT 0x57
#endif

#if defined(CONFIG_V1A) || defined(CONFIG_CHAGALL)
#define WACOM_X_INVERT 0
#define WACOM_XY_SWITCH 0

#define WACOM_MAX_COORD_X 26266
#define WACOM_MAX_COORD_Y 16416
#define WACOM_MAX_PRESSURE 1023

#elif defined(CONFIG_N1A)
#define WACOM_X_INVERT 0
#define WACOM_XY_SWITCH 0

#define WACOM_MAX_COORD_X 21658
#define WACOM_MAX_COORD_Y 13538
#define WACOM_MAX_PRESSURE 1023

#elif defined(CONFIG_HA)
#define WACOM_MAX_COORD_X 12576
#define WACOM_MAX_COORD_Y 7074
#define WACOM_MAX_PRESSURE 1023
#endif


#ifndef WACOM_X_INVERT
#define WACOM_X_INVERT 1
#endif
#ifndef WACOM_Y_INVERT
#define WACOM_Y_INVERT 0
#endif
#ifndef WACOM_XY_SWITCH
#define WACOM_XY_SWITCH 1
#endif


/*sec_class sysfs*/
extern struct class *sec_class;

struct wacom_g5_callbacks {
	int (*check_prox)(struct wacom_g5_callbacks *);
};

#define LONG_PRESS_TIME 500
#define MIN_GEST_DIST 384

/*Parameters for i2c driver*/
struct wacom_i2c {
	struct i2c_client *client;
	struct i2c_client *client_boot;
	struct input_dev *input_dev;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
	struct mutex lock;
#if defined(CONFIG_SEC_LT03_PROJECT) || defined(CONFIG_SEC_VIENNA_PROJECT)
	struct mutex irq_lock;
#endif
#ifdef WACOM_BOOSTER
	struct delayed_work	work_dvfs_off;
	struct delayed_work	work_dvfs_chg;
	struct mutex		dvfs_lock;
	bool dvfs_lock_status;
	int dvfs_old_stauts;
	int dvfs_boost_mode;
	int dvfs_freq;
#endif

	struct device	*dev;
	int irq;
#ifdef WACOM_PDCT_WORK_AROUND
	int irq_pdct;
	bool rdy_pdct;
#endif
	int pen_pdct;
	int gpio;
	int irq_flag;
	int pen_prox;
	int pen_pressed;
	int side_pressed;
	int tool;
	s16 last_x;
	s16 last_y;
#ifdef WACOM_PEN_DETECT
	int irq_pen_insert;
	struct delayed_work pen_insert_dwork;
	bool pen_insert;
	int gpio_pen_insert;
#endif
#ifdef WACOM_RESETPIN_DELAY
	struct delayed_work work_wacom_reset;
#endif
	int invert_pen_insert;
#ifdef WACOM_HAVE_FWE_PIN
	int gpio_fwe;
	bool have_fwe_pin;
#endif
#ifdef WACOM_IMPORT_FW_ALGO
	bool use_offset_table;
	bool use_aveTransition;
#endif
	bool checksum_result;
	const char name[NAMEBUF];
	struct wacom_features *wac_feature;
	struct wacom_g5_platform_data *wac_pdata;
	struct wacom_g5_callbacks callbacks;
	int (*power)(int on);
	struct delayed_work resume_work;

#ifdef BATTERY_SAVING_MODE
	bool battery_saving_mode;
#endif
	bool power_enable;
	bool boot_mode;
	bool query_status;
	int ic_mpu_ver;
	int boot_ver;
	bool init_fail;
#ifdef USE_WACOM_LCD_WORKAROUND
	unsigned int vsync;
	struct delayed_work read_vsync_work;
	struct delayed_work boot_done_work;
	bool wait_done;
	bool boot_done;
	unsigned int delay_time;
#endif
#ifdef USE_WACOM_BLOCK_KEYEVENT
	struct delayed_work	touch_pressed_work;
	unsigned int key_delay_time;
	bool touchkey_skipped;
	bool touch_pressed;
#endif
	bool enabled;

	int enabled_gestures;
	int gesture_key;
	int gesture_start_x;
	int gesture_start_y;
	ktime_t gesture_start_time;
};

struct wacom_g5_platform_data {
	char *name;
	int x_invert;
	int y_invert;
	int xy_switch;
	int min_x;
	int max_x;
	int min_y;
	int max_y;
	int max_pressure;
	int min_pressure;
	int gpio_pendct;
	int gpio_pen_insert;
	void (*compulsory_flash_mode)(bool);
	int (*init_platform_hw)(void);
	int (*exit_platform_hw)(void);
	int (*suspend_platform_hw)(void);
	int (*resume_platform_hw)(void);
#ifdef CONFIG_HAS_EARLYSUSPEND
	int (*early_suspend_platform_hw)(void);
	int (*late_resume_platform_hw)(void);
#endif
	int (*reset_platform_hw)(void);
	void (*register_cb)(struct wacom_g5_callbacks *);
	int (*get_irq_state)(void);
};

#endif /* _LINUX_WACOM_I2C_H */
