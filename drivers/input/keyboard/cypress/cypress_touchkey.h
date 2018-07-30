#ifndef _CYPRESS_TOUCHKEY_H
#define _CYPRESS_TOUCHKEY_H

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/hrtimer.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/wakelock.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#ifdef CONFIG_INPUT_BOOSTER
#include <linux/input/input_booster.h>
#endif

#include <linux/i2c/touchkey_i2c.h>

/* Touchkey Register */
#define CYPRESS_REG_STATUS	0x00
#define CYPRESS_REG_FW_VER	0X01
#define CYPRESS_REG_MD_VER	0X02
#define CYPRESS_REG_COMMAND	0X03
#define CYPRESS_REG_THRESHOLD	0X04
#define CYPRESS_REG_AUTOCAL	0X05
#define CYPRESS_REG_IDAC	0X06
#define CYPRESS_REG_DIFF	0X0A
#define CYPRESS_REG_RAW		0X0E
#define CYPRESS_REG_BASE	0X12

#define KEYCODE_REG			0x00

#define TK_BIT_PRESS_EV		0x08
#define TK_BIT_KEYCODE		0x07

#define TK_BIT_AUTOCAL		0x80
#define TK_BIT_GLOVE		0x40
#define TK_BIT_TA_ON		0x10
#define TK_BIT_FW_ID_55		0x20
#define TK_BIT_FW_ID_65		0x04

#define TK_CMD_LED_ON		0x10
#define TK_CMD_LED_OFF		0x20

#define I2C_M_WR 0		/* for i2c */

#define TK_UPDATE_DOWN		1
#define TK_UPDATE_FAIL		-1
#define TK_UPDATE_PASS		0

/* update condition */
#define TK_RUN_UPDATE 1
#define TK_EXIT_UPDATE 2
#define TK_RUN_CHK 3

/* multi touch */
#define TK_CMD_DUAL_DETECTION	0x01
#define TK_BIT_DETECTION_CONFIRM	0xEE
#define CYPRESS_DETECTION_FLAG		0x1B
#define TK_DUAL_REG 0x18
#if defined(CONFIG_KLIMT)
#define TK_MULTI_FW_VER  0x08
#endif
#if defined(CONFIG_HA)
#define TK_MULTI_FW_VER  0x11
#endif

/* Flip cover*/
#ifndef CONFIG_KLIMT
#define TKEY_FLIP_MODE
#endif

#ifdef TKEY_FLIP_MODE
#define TK_BIT_FLIP	0x08
#endif

/* Autocalibration */
#define TK_HAS_AUTOCAL

/* Generalized SMBus access */
#define TK_USE_GENERAL_SMBUS

/* Boot-up Firmware Update */
#define TK_HAS_FIRMWARE_UPDATE
#ifdef CONFIG_HA
#define TK_UPDATABLE_BD_ID	6
#else
#define TK_UPDATABLE_BD_ID	0
#endif

/* for HA */
#if defined(CONFIG_KLIMT)
#define FW_PATH "cypress/cypress_klimt.fw"
#else
#define FW_PATH "cypress/cypress_ha_m09.fw"
#endif
#define TKEY_MODULE07_HWID 8
#define TKEY_FW_PATH "/sdcard/cypress/fw.bin"

#if defined(CONFIG_KLIMT)
#define TK_USE_RECENT
#endif

#if defined(CONFIG_MACH_HLLTE) || \
	defined(CONFIG_MACH_HL3G)
#define TK_SUPPORT_MT
#endif

/*#define TK_USE_2KEY_TYPE_M0*/

/* LCD Type check*/
#if defined(CONFIG_HA) || defined(CONFIG_KLIMT)
#define TK_USE_LCDTYPE_CHECK
#endif

#if defined(TK_USE_4KEY_TYPE_ATT)\
	|| defined(TK_USE_4KEY_TYPE_NA)
#define TK_USE_4KEY
#elif defined(TK_USE_2KEY_TYPE_M0)\
	|| defined(TK_USE_2KEY_TYPE_U1)
#define TK_USE_2KEY
#endif

#define  TOUCHKEY_FW_UPDATEABLE_HW_REV  11
#if !defined(CONFIG_INPUT_BOOSTER)
#define TOUCHKEY_BOOSTER
#endif
#ifdef TOUCHKEY_BOOSTER
#include <linux/pm_qos.h>
#define TKEY_BOOSTER_ON_TIME	500
#define TKEY_BOOSTER_OFF_TIME	500
#define TKEY_BOOSTER_CHG_TIME	130

enum BOOST_LEVEL {
	TKEY_BOOSTER_DISABLE = 0,
	TKEY_BOOSTER_LEVEL1,
	TKEY_BOOSTER_LEVEL2,
};

#define TKEY_BOOSTER_CPU_FREQ1 1600000
#define TKEY_BOOSTER_MIF_FREQ1 667000
#define TKEY_BOOSTER_INT_FREQ1 333000

#define TKEY_BOOSTER_CPU_FREQ2 650000
#define TKEY_BOOSTER_MIF_FREQ2 400000
#define TKEY_BOOSTER_INT_FREQ2 111000
#endif

/* #define TK_USE_OPEN_DWORK */
#ifdef TK_USE_OPEN_DWORK
#define	TK_OPEN_DWORK_TIME	10
#endif
#ifdef CONFIG_GLOVE_TOUCH
#define	TK_GLOVE_DWORK_TIME	300
#endif

enum {
	FW_NONE = 0,
	FW_BUILT_IN,
	FW_HEADER,
	FW_IN_SDCARD,
	FW_EX_SDCARD,
};

#if 0
/* header ver 0 */
struct fw_image {
	u8 hdr_ver;
	u8 hdr_len;
	u16 first_fw_ver;
	u16 second_fw_ver;
	u16 third_ver;
	u32 fw_len;
	u8 data[0];
} __attribute__ ((packed));
#endif

/* header ver 1 */
struct fw_image {
	u8 hdr_ver;
	u8 hdr_len;
	u16 first_fw_ver;
	u16 second_fw_ver;
	u16 third_ver;
	u32 fw_len;
	u16 checksum;
	u16 alignment_dummy;
	u8 data[0];
} __attribute__ ((packed));

/*Parameters for i2c driver*/
struct touchkey_i2c {
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct completion init_done;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;
#endif
	struct mutex lock;
	struct wake_lock fw_wakelock;
	struct device	*dev;
	int irq;
	int md_ver_ic; /*module ver*/
	int fw_ver_ic;
	int firmware_id;
	struct touchkey_platform_data *pdata;
	char *name;
	int (*power)(int on);
	int update_status;
	bool enabled;
#ifdef TOUCHKEY_BOOSTER
	bool tsk_dvfs_lock_status;
	struct delayed_work tsk_work_dvfs_off;
	struct delayed_work tsk_work_dvfs_chg;
	struct mutex tsk_dvfs_lock;
	struct pm_qos_request cpu_qos;
	struct pm_qos_request mif_qos;
	struct pm_qos_request int_qos;
	unsigned char boost_level;
	bool dvfs_signal;
#endif
#ifdef TK_USE_OPEN_DWORK
	struct delayed_work open_work;
#endif
#ifdef CONFIG_GLOVE_TOUCH
	struct delayed_work glove_change_work;
	bool tsk_glove_lock_status;
	bool tsk_glove_mode_status;
	struct mutex tsk_glove_lock;
#endif
#ifdef TK_INFORM_CHARGER
	struct touchkey_callbacks callbacks;
	bool charging_mode;
#endif
#ifdef TKEY_FLIP_MODE
	bool enabled_flip;
#endif
	bool status_update;
	struct work_struct update_work;
	struct workqueue_struct *fw_wq;
	u8 fw_path;
	const struct firmware *firm_data;
	struct fw_image *fw_img;
	bool do_checksum;
	int support_multi_touch;
};

extern struct class *sec_class;

extern unsigned int lcdtype;
extern unsigned int system_rev;
#endif /* _CYPRESS_TOUCHKEY_H */
