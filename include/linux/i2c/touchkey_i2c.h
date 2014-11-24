#ifndef _LINUX_CYPRESS_TOUCHKEY_I2C_H
#define _LINUX_CYPRESS_TOUCHKEY_I2C_H

#include <linux/types.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

/* LDO Regulator */
#define	TK_REGULATOR_NAME	"vtouch_1.8v"

#if defined(CONFIG_KLIMT)
#define LDO_WITH_REGULATOR
#define TK_INFORM_CHARGER
#endif

/* LED LDO Regulator */
#if defined(CONFIG_CHAGALL) || defined(CONFIG_KLIMT)
#define	TK_LED_REGULATOR_NAME	"key_led_3.3v"
#else
#define	TK_LED_REGULATOR_NAME	"vtouch_3.3v"
#endif

/* LED LDO Type*/
#define LED_LDO_WITH_REGULATOR

struct touchkey_platform_data {
	int gpio_sda;
	int gpio_scl;
	int gpio_int;
	void (*init_platform_hw)(void);
	int (*suspend) (void);
	int (*resume) (void);
	int (*power_on) (bool);
	int (*led_power_on) (bool);
	int (*reset_platform_hw)(void);
	void (*register_cb)(void *);
	bool led_control_by_ldo;
};

#if defined(TK_INFORM_CHARGER)
void touchkey_charger_infom(bool en);
struct touchkey_callbacks {
	void (*inform_charger)(struct touchkey_callbacks *, bool);
};
#endif
#endif /* _LINUX_CYPRESS_TOUCHKEY_I2C_H */
