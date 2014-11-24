#ifndef __MDNIE_H__
#define __MDNIE_H__

#define END_SEQ			0xffff

enum MODE {
	DYNAMIC,
	STANDARD,
#if !defined(CONFIG_FB_MDNIE_PWM) && !defined(CONFIG_FB_DBLC_PWM)
	NATURAL,
#endif
	MOVIE,
	AUTO,
	MODE_MAX,
};

enum SCENARIO {
	UI_MODE,
	VIDEO_MODE,
	CAMERA_MODE = 4,
	NAVI_MODE,
	GALLERY_MODE,
	VT_MODE,
	BROWSER_MODE,
	EBOOK_MODE,
	EMAIL_MODE,
	SCENARIO_MAX,
	DMB_NORMAL_MODE = 20,
	DMB_MODE_MAX,
};

enum CABC {
	CABC_OFF,
#if defined(CONFIG_FB_MDNIE_PWM)
	CABC_ON,
#endif
	CABC_MAX,
};

enum POWER_LUT {
	LUT_DEFAULT,
	LUT_VIDEO,
	LUT_MAX,
};

enum BYPASS {
	BYPASS_OFF,
	BYPASS_ON,
	BYPASS_MAX,
};

enum ACCESSIBILITY {
	ACCESSIBILITY_OFF,
	NEGATIVE,
	COLOR_BLIND,
	SCREEN_CURTAIN,
	ACCESSIBILITY_MAX,
};

struct mdnie_tuning_info {
	const char *name;
	unsigned short *sequence;
};

struct mdnie_info {
	struct clk		*bus_clk;
	struct clk		*clk;

	struct device		*dev;
	struct mutex		dev_lock;
	struct mutex		lock;

	unsigned int		enable;

	enum SCENARIO scenario;
	enum MODE mode;
	enum CABC cabc;
	enum BYPASS bypass;
	unsigned int tuning;
	unsigned int accessibility;
	unsigned int color_correction;
	char path[50];

	struct notifier_block fb_notif;

#if defined(CONFIG_FB_MDNIE_PWM)
	struct backlight_device		*bd;
	int				*br_table;
	struct clk			*pwm_clk;
	unsigned int			support_pwm;
#endif
#if defined(CONFIG_FB_DBLC_PWM)
	unsigned int outdoormode;
	struct notifier_block secfb_notif;
#endif

	unsigned int scr_white_red;
	unsigned int scr_white_green;
	unsigned int scr_white_blue;
	struct mdnie_tuning_info table_buffer;
	unsigned short sequence_buffer[512];

	struct platform_mdnie_data *pd;
};

extern struct mdnie_info *g_mdnie;

int s3c_mdnie_hw_init(void);
int s3c_mdnie_set_size(void);

void mdnie_s3cfb_resume(void);
void mdnie_s3cfb_suspend(void);

void mdnie_update(struct mdnie_info *mdnie, u8 force);

extern int mdnie_calibration(unsigned short x, unsigned short y, int *r);
extern int mdnie_request_firmware(const char *path, u16 **buf, const char *name);
extern int mdnie_open_file(const char *path, char **fp);
#if defined(CONFIG_FB_S5P_MDNIE_LITE)
extern int mdnie_lite_write(const unsigned short *seq, int size);
#endif

#endif /* __MDNIE_H__ */
