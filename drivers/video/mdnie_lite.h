#ifndef __MDNIE_H__
#define __MDNIE_H__

typedef u8 mdnie_t;

enum MODE {
	DYNAMIC,
	STANDARD,
	NATURAL,
	MOVIE,
	AUTO,
	MODE_MAX
};

enum SCENARIO {
	UI_MODE,
	VIDEO_NORMAL_MODE,
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

enum BYPASS {
	BYPASS_OFF,
	BYPASS_ON,
	BYPASS_MAX
};

enum ACCESSIBILITY {
	ACCESSIBILITY_OFF,
	NEGATIVE,
	COLOR_BLIND,
	SCREEN_CURTAIN,
	ACCESSIBILITY_MAX
};

enum HBM {
	HBM_OFF,
	HBM_ON,
	HBM_MAX,
};

enum MDNIE_CMD {
	LEVEL1_KEY_UNLOCK,
	MDNIE_CMD1,
	MDNIE_CMD2,
	LEVEL1_KEY_LOCK,
	MDNIE_CMD_MAX,
};

struct mdnie_command {
	mdnie_t *sequence;
	unsigned int size;
};

struct mdnie_table {
	char *name;
	struct mdnie_command tune[MDNIE_CMD_MAX];
};

#if defined(CONFIG_FB_S5P_MDNIE_LITE)
#define MDNIE_SET(id)	\
{							\
	.name		= #id,				\
	.tune		= {				\
		{	.sequence = LEVEL1_UNLOCK, .size = ARRAY_SIZE(LEVEL1_UNLOCK)},	\
		{	.sequence = id##_1, .size = ARRAY_SIZE(id##_1)},		\
		{	.sequence = id##_2, .size = ARRAY_SIZE(id##_2)},		\
		{	.sequence = LEVEL1_LOCK, .size = ARRAY_SIZE(LEVEL1_LOCK)},	\
	}	\
}

typedef int (*mdnie_w)(void *devdata, const u8 *seq, u32 len);
typedef int (*mdnie_r)(void *devdata, u8 addr, u8 *buf, u32 len);
#else
#define MDNIE_SET(id)	\
{							\
	.name		= #id,				\
	.tune		= {				\
		{	.sequence = id, .size = ARRAY_SIZE(id)},	\
	}	\
}

typedef int (*mdnie_w)(unsigned int a, unsigned int v);
typedef int (*mdnie_r)(unsigned int a);
#endif

struct mdnie_info {
	struct clk		*bus_clk;
	struct clk		*clk;

	struct device		*dev;
	struct mutex		dev_lock;
	struct mutex		lock;

	unsigned int		enable;

	enum SCENARIO		scenario;
	enum MODE		mode;
	enum BYPASS		bypass;
	enum HBM		hbm;

	unsigned int		tuning;
	unsigned int		accessibility;
	unsigned int		color_correction;
	unsigned int		auto_brightness;

	char			path[50];

	unsigned int white_r;
	unsigned int white_g;
	unsigned int white_b;
	struct mdnie_table table_buffer;
	mdnie_t sequence_buffer[256];
};

extern int mdnie_calibration(int *r);
extern int mdnie_open_file(const char *path, char **fp);
extern int mdnie_register(struct device *p, void *data, mdnie_w w, mdnie_r r);
extern struct mdnie_table *mdnie_request_table(char *path, struct mdnie_table *s);

#endif /* __MDNIE_H__ */
