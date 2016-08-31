#ifndef __V1_POWER_SAVE_H__
#define __V1_POWER_SAVE_H__

enum tcon_mode {
    TCON_MODE_UI = 0,
    TCON_MODE_VIDEO,
    TCON_MODE_VIDEO_WARM,
    TCON_MODE_VIDEO_COLD,
    TCON_MODE_CAMERA,
    TCON_MODE_NAVI,
    TCON_MODE_GALLERY,
    TCON_MODE_VT,
    TCON_MODE_BROWSER,
    TCON_MODE_EBOOK,
    TCON_MODE_EMAIL,
    TCON_MODE_SNOTE1,
    TCON_MODE_SNOTE2,
    TCON_MODE_CALL,
    TCON_MODE_MAX, /* 14 */
};

enum tcon_illu {
    TCON_LEVEL_1 = 0,   /* 0 ~ 150 lux*/
    TCON_LEVEL_2,       /* 150 ~ 40k lux*/
    TCON_LEVEL_3,       /* 40k lux ~ */
    TCON_LEVEL_MAX,
};

enum tcon_auto_br {
    TCON_AUTO_BR_OFF = 0,
    TCON_AUTO_BR_ON, 
    TCON_AUTO_BR_MAX,
};

#define TCON_REG_MAX 30

/* If Limit MN_BL% duty */
#define CONFIG_TCON_SET_MNBL
#ifdef CONFIG_TCON_SET_MNBL
#define MN_BL	20
#define MNBL_INDEX1	8
#define MNBL_INDEX2	9
#endif



struct tcon_reg_info {
	int reg_cnt;
	unsigned short addr[TCON_REG_MAX];
	unsigned char data[TCON_REG_MAX];
};

#if defined(CONFIG_LCD_LSL122DL01_BATTERY)
struct tcon_reg_info TCON_UI_B = {
	.reg_cnt = 19,

	.addr = {
		0x0DB1, 0x0DB2, 0x0DB3, 0x0DB4, 0x0DB5,
		0x0DB6, 0x0DB7, 0x0DB8, 0x0DB9, 0x0DBA,
		0x0DBB, 0x0DBC, 0x0DBD, 0x0DBE, 0x0DBF, 0x0E39,
		0x0E3A, 0x0DC0, 0x0DC5,
		},
	.data = {
		0xB1,   0xFF,   0xF0,   0x1F,   0xF5,
		0xFE,   0x82,   0x46,   0x5A,   0xBF,
		0xC1,   0x04,   0x24,   0x26,   0x01,		0x41,
		0x3F,   0x43,	 0xF4,
		},
};

struct tcon_reg_info TCON_POWER_SAVE_B = {
	.reg_cnt = 19,

	.addr = {
		0x0DB1, 0x0DB2, 0x0DB3, 0x0DB4, 0x0DB5,
		0x0DB6, 0x0DB7, 0x0DB8, 0x0DB9, 0x0DBA,
		0x0DBB, 0x0DBC, 0x0DBD, 0x0DBE, 0x0DBF, 0x0E39,
		0x0E3A, 0x0DC0, 0x0DC5,
		},
	.data = {
		0xB1,   0xFF,   0xF0,   0x0E,   0xC9,
		0x4E,   0x92,   0x4A,   0x59,   0xBF,
		0xC1,   0x04,   0x24,   0x26,   0x01,		0x41,
		0x3F,   0x43,	 0xF4,
		},
};

struct tcon_reg_info TCON_VIDEO_B = {
	.reg_cnt = 19,

	.addr = {
		0x0DB1, 0x0DB2, 0x0DB3, 0x0DB4, 0x0DB5,
		0x0DB6, 0x0DB7, 0x0DB8, 0x0DB9, 0x0DBA,
		0x0DBB, 0x0DBC, 0x0DBD, 0x0DBE, 0x0DBF, 0x0E39,
		0x0E3A, 0x0DC0, 0x0DC5,
		},
	.data = {
		0xB1,   0xFF,   0xF0,   0x08,   0x85,
		0x5E,   0x92,   0x4A,   0x59,   0xBF,
		0xC1,   0x04,   0x24,   0x26,   0x01,		0x41,
		0x3F,   0x43,	 0xF4,
		},
};

struct tcon_reg_info TCON_BROWSER_B = {
	.reg_cnt = 19,

	.addr = {
		0x0DB1, 0x0DB2, 0x0DB3, 0x0DB4, 0x0DB5,
		0x0DB6, 0x0DB7, 0x0DB8, 0x0DB9, 0x0DBA,
		0x0DBB, 0x0DBC, 0x0DBD, 0x0DBE, 0x0DBF, 0x0E39,
		0x0E3A, 0x0DC0, 0x0DC5,
		},
	.data = {
		0xB1,   0xFF,   0xF0,   0x08,   0x85,
		0x5E,   0x92,   0x4A,   0x59,   0xBF,
		0xC1,   0x04,   0x24,   0x26,   0x01,		0x41,
		0x3F,   0x43,	 0xF4,
		},
};

struct tcon_reg_info TCON_OUTDOOR_B = {
	.reg_cnt = 19,

	.addr = {
		0x0DB1, 0x0DB2, 0x0DB3, 0x0DB4, 0x0DB5,
		0x0DB6, 0x0DB7, 0x0DB8, 0x0DB9, 0x0DBA,
		0x0DBB, 0x0DBC, 0x0DBD, 0x0DBE, 0x0DBF, 0x0E39,
		0x0E3A, 0x0DC0, 0x0DC5,
		},
	.data = {
		0xB0,  0x78,   0x10,   0x1F,   0xF5,
		0xFE,   0x82,   0x46,   0x5A,   0xBF,
		0xC1,  0x04,   0x24,   0x26,   0x01,		0x41,
		0x3F,   0x43,	 0xF4,
		},
};

struct tcon_reg_info TCON_SNOTE1_B = {
	.reg_cnt = 19,

	.addr = {
		0x0DB1, 0x0DB2, 0x0DB3, 0x0DB4, 0x0DB5,
		0x0DB6, 0x0DB7, 0x0DB8, 0x0DB9, 0x0DBA,
		0x0DBB, 0x0DBC, 0x0DBD, 0x0DBE, 0x0DBF, 0x0E39,
		0x0E3A, 0x0DC0, 0x0DC5,
		},
	.data = {
		0xB1,   0xFF,   0xF0,   0x1F,   0xF5,
		0xFE,   0x82,   0x42,   0x5A,   0xBF,
		0xC1,   0x04,   0x24,   0x26,   0x01,		0x41,
		0x3F,   0x43,	 0xF4,
		},
};

struct tcon_reg_info TCON_SNOTE2_B = {
	.reg_cnt = 19,

	.addr = {
		0x0DB1, 0x0DB2, 0x0DB3, 0x0DB4, 0x0DB5,
		0x0DB6, 0x0DB7, 0x0DB8, 0x0DB9, 0x0DBA,
		0x0DBB, 0x0DBC, 0x0DBD, 0x0DBE, 0x0DBF, 0x0E39,
		0x0E3A, 0x0DC0, 0x0DC5,
		},
	.data = {
		0xB1,   0xFF,   0xF0,   0x1F,   0xF5,
		0xFE,   0x82,   0x42,   0x7F,   0xFF,
		0xC1,   0x04,   0x24,   0x26,   0x01,		0x41,
		0x3F,   0x43,	 0xF4,
		},
};

struct tcon_reg_info TCON_CALL_B = {
	.reg_cnt = 19,

	.addr = {
		0x0DB1, 0x0DB2, 0x0DB3, 0x0DB4, 0x0DB5,
		0x0DB6, 0x0DB7, 0x0DB8, 0x0DB9, 0x0DBA,
		0x0DBB, 0x0DBC, 0x0DBD, 0x0DBE, 0x0DBF, 0x0E39,
		0x0E3A, 0x0DC0, 0x0DC5,
		},
	.data = {
		0xB1,   0xFF,   0xF0,   0x1F,   0xF5,
		0xFE,   0x82,   0x46,   0x7F,   0xFF,
		0xC1,   0x04,   0x24,   0x26,	  0x01,		0x41,
		0x3F,   0x43,	 0xF4,
		},
};

struct tcon_reg_info TCON_UI = {
	.reg_cnt = 17,

	.addr = {
		0x0DB1, 0x0DB2, 0x0DB3, 0x0DB4, 0x0DB5, 
		0x0DB6, 0x0DB7, 0x0DB8, 0x0DB9, 0x0DBA, 
		0x0DBB, 0x0DBC, 0x0DBD, 0x0DBE, 0x0E39, 
		0x0E3A, 0x0DC5,
		},
	.data = {
		0xB1,   0xFF,   0xF0,   0x1F,   0xF5,
		0xFE,   0x82,   0x46,   0x5A,   0xBF, 
		0xC1,   0x04,   0x24,   0x26,   0x41, 
		0x3F,   0xF4,
		},
};

struct tcon_reg_info TCON_POWER_SAVE = {
	.reg_cnt = 17,

	.addr = {
		0x0DB1, 0x0DB2, 0x0DB3, 0x0DB4, 0x0DB5, 
		0x0DB6, 0x0DB7, 0x0DB8, 0x0DB9, 0x0DBA, 
		0x0DBB, 0x0DBC, 0x0DBD, 0x0DBE, 0x0E39, 
		0x0E3A, 0x0DC5,
		},
	.data = {
		0xB1,   0xFF,   0xF0,   0x0E,   0xC9, 
		0x4E,   0x92,   0x4A,   0x59,   0xBF, 
		0xC1,   0x04,   0x24,   0x26,   0x41, 
		0x3F,   0xF4,
		},
};

struct tcon_reg_info TCON_VIDEO = {
	.reg_cnt = 17,

	.addr = {
		0x0DB1, 0x0DB2, 0x0DB3, 0x0DB4, 0x0DB5, 
		0x0DB6, 0x0DB7, 0x0DB8, 0x0DB9, 0x0DBA, 
		0x0DBB, 0x0DBC, 0x0DBD, 0x0DBE, 0x0E39, 
		0x0E3A, 0x0DC5,
		},
	.data = {
		0xB1,   0xFF,   0xF0,   0x08,   0x85,
		0x5E,   0x92,   0x4A,   0x59,   0xBF,
		0xC1,   0x04,   0x24,   0x26,   0x41,
		0x3F,   0xF4,
		},
};

struct tcon_reg_info TCON_BROWSER = {
	.reg_cnt = 17,

	.addr = {
		0x0DB1, 0x0DB2, 0x0DB3, 0x0DB4, 0x0DB5, 
		0x0DB6, 0x0DB7, 0x0DB8, 0x0DB9, 0x0DBA, 
		0x0DBB, 0x0DBC, 0x0DBD, 0x0DBE, 0x0E39, 
		0x0E3A, 0x0DC5,
		},
	.data = {
		0xB1,   0xFF,   0xF0,   0x08,   0x85, 
		0x5E,   0x92,   0x4A,   0x59,   0xBF, 
		0xC1,   0x04,   0x24,   0x26,   0x41, 
		0x3F,   0xF4,
		},
};

struct tcon_reg_info TCON_OUTDOOR = {
	.reg_cnt = 17,

	.addr = {
		0x0DB1, 0x0DB2, 0x0DB3, 0x0DB4, 0x0DB5, 
		0x0DB6, 0x0DB7, 0x0DB8, 0x0DB9, 0x0DBA, 
		0x0DBB, 0x0DBC, 0x0DBD, 0x0DBE, 0x0E39, 
		0x0E3A, 0x0DC5,
		},
	.data = {
		0xB0,  0x78,   0x10,   0x1F,   0xF5,
		0xFE,   0x82,   0x46,   0x5A,   0xBF, 
		0xC1,  0x04,   0x24,   0x26,   0x41, 
		0x3F,   0xF4,
		},
};

struct tcon_reg_info TCON_SNOTE1 = {
	.reg_cnt = 17,

	.addr = {
		0x0DB1, 0x0DB2, 0x0DB3, 0x0DB4, 0x0DB5,
		0x0DB6, 0x0DB7, 0x0DB8, 0x0DB9, 0x0DBA,
		0x0DBB, 0x0DBC, 0x0DBD, 0x0DBE, 0x0E39,
		0x0E3A, 0x0DC5,
		},
	.data = {
		0xB1,   0xFF,   0xF0,   0x1F,   0xF5,
		0xFE,   0x82,   0x42,   0x5A,   0xBF,
		0xC1,   0x04,   0x24,   0x26,   0x41,
		0x3F,   0xF4,
		},
};

struct tcon_reg_info TCON_SNOTE2 = {
	.reg_cnt = 17,

	.addr = {
		0x0DB1, 0x0DB2, 0x0DB3, 0x0DB4, 0x0DB5,
		0x0DB6, 0x0DB7, 0x0DB8, 0x0DB9, 0x0DBA,
		0x0DBB, 0x0DBC, 0x0DBD, 0x0DBE, 0x0E39,
		0x0E3A, 0x0DC5,
		},
	.data = {
		0xB1,   0xFF,   0xF0,   0x1F,   0xF5,
		0xFE,   0x82,   0x42,   0x7F,   0xFF,
		0xC1,   0x04,   0x24,   0x26,   0x41,
		0x3F,   0xF4,
		},
};

struct tcon_reg_info TCON_CALL = {
	.reg_cnt = 17,

	.addr = {
		0x0DB1, 0x0DB2, 0x0DB3, 0x0DB4, 0x0DB5,
		0x0DB6, 0x0DB7, 0x0DB8, 0x0DB9, 0x0DBA,
		0x0DBB, 0x0DBC, 0x0DBD, 0x0DBE, 0x0E39,
		0x0E3A, 0x0DC5,
		},
	.data = {
		0xB1,   0xFF,   0xF0,   0x1F,   0xF5,
		0xFE,   0x82,   0x46,   0x7F,   0xFF,
		0xC1,   0x04,   0x24,   0x26,	0x41,
		0x3F,   0xF4,
		},
};

struct tcon_reg_info TCON_BLACK_IMAGE_BLU_ENABLE = {
	.reg_cnt = 1,
	.addr = {
		0x0DB1,
		},
	.data = {
		0xB0,
		},
};

struct tcon_reg_info *tcon_tune_value[2][TCON_AUTO_BR_MAX][TCON_LEVEL_MAX][TCON_MODE_MAX] = {
		/*
			UI_APP = 0,
			VIDEO_APP,
			VIDEO_WARM_APP,
			VIDEO_COLD_APP,
			CAMERA_APP,
			NAVI_APP,
			GALLERY_APP,
			VT_APP,
			BROWSER_APP,
			eBOOK_APP,
			EMAIL_APP,
		*/
	{ /* For dblc battery [0] is not set, [1] is set */
	   	 /* auto brightness off */
		{

			/* Illumiatation Level 1 */
			{
				&TCON_UI,
				&TCON_UI,
				&TCON_UI,
				&TCON_UI,
				&TCON_UI, /* default */
				&TCON_UI, /* default */
				&TCON_UI, /* default */
				&TCON_UI, /* default */
				&TCON_BROWSER,
				&TCON_UI, /* default */
				&TCON_UI, /* default */
				&TCON_SNOTE1,
				&TCON_SNOTE2,
				&TCON_CALL,
			},
			/* Illumiatation Level 2 */
			{
				&TCON_UI,
				&TCON_UI,
				&TCON_UI,
				&TCON_UI,
				&TCON_UI, /* default */
				&TCON_UI, /* default */
				&TCON_UI, /* default */
				&TCON_UI, /* default */
				&TCON_BROWSER,
				&TCON_UI, /* default */
				&TCON_UI, /* default */
				&TCON_SNOTE1,
				&TCON_SNOTE2,
				&TCON_CALL,
			},
			/* Illumiatation Level 3 */
			{
				&TCON_UI,
				&TCON_UI,
				&TCON_UI,
				&TCON_UI,
				&TCON_UI, /* default */
				&TCON_UI, /* default */
				&TCON_UI, /* default */
				&TCON_UI, /* default */
				&TCON_BROWSER,
				&TCON_UI, /* default */
				&TCON_UI, /* default */
				&TCON_SNOTE1,
				&TCON_SNOTE2,
				&TCON_CALL,
			},
		},
		/* auto brightness on */
		{
			/* Illumiatation Level 1 */
			{
				&TCON_UI,
				&TCON_UI,
				&TCON_UI,
				&TCON_UI,
				&TCON_UI, /* default */
				&TCON_UI, /* default */
				&TCON_UI, /* default */
				&TCON_UI, /* default */
				&TCON_BROWSER,
				&TCON_UI, /* default */
				&TCON_UI, /* default */
				&TCON_SNOTE1,
				&TCON_SNOTE2,
				&TCON_CALL,
			},
			/* Illumiatation Level 2 */
			{
				&TCON_POWER_SAVE,
				&TCON_UI,
				&TCON_UI,
				&TCON_UI,
				&TCON_POWER_SAVE, /* default */
				&TCON_POWER_SAVE, /* default */
				&TCON_POWER_SAVE, /* default */
				&TCON_POWER_SAVE, /* default */
				&TCON_BROWSER,
				&TCON_POWER_SAVE, /* default */
				&TCON_POWER_SAVE, /* default */
				&TCON_SNOTE1,
				&TCON_SNOTE2,
				&TCON_CALL,
			},
			/* Illumiatation Level 3 */
			{
				&TCON_OUTDOOR,
				&TCON_OUTDOOR,
				&TCON_OUTDOOR,
				&TCON_OUTDOOR,
				&TCON_OUTDOOR,
				&TCON_OUTDOOR,
				&TCON_OUTDOOR,
				&TCON_OUTDOOR,
				&TCON_OUTDOOR,
				&TCON_OUTDOOR,
				&TCON_OUTDOOR,
				&TCON_OUTDOOR,
				&TCON_OUTDOOR,
				&TCON_OUTDOOR,
			}
		}
	},
	{

		{

			/* Illumiatation Level 1 */
			{
				&TCON_UI_B,
				&TCON_UI_B,
				&TCON_UI_B,
				&TCON_UI_B,
				&TCON_UI_B, /* default */
				&TCON_UI_B, /* default */
				&TCON_UI_B, /* default */
				&TCON_UI_B, /* default */
				&TCON_BROWSER_B,
				&TCON_UI_B, /* default */
				&TCON_UI_B, /* default */
				&TCON_SNOTE1_B,
				&TCON_SNOTE2_B,
				&TCON_CALL_B,
			},
			/* Illumiatation Level 2 */
			{
				&TCON_UI_B,
				&TCON_UI_B,
				&TCON_UI_B,
				&TCON_UI_B,
				&TCON_UI_B, /* default */
				&TCON_UI_B, /* default */
				&TCON_UI_B, /* default */
				&TCON_UI_B, /* default */
				&TCON_BROWSER_B,
				&TCON_UI_B, /* default */
				&TCON_UI_B, /* default */
				&TCON_SNOTE1_B,
				&TCON_SNOTE2_B,
				&TCON_CALL_B,
			},
			/* Illumiatation Level 3 */
			{
				&TCON_UI_B,
				&TCON_UI_B,
				&TCON_UI_B,
				&TCON_UI_B,
				&TCON_UI_B, /* default */
				&TCON_UI_B, /* default */
				&TCON_UI_B, /* default */
				&TCON_UI_B, /* default */
				&TCON_BROWSER_B,
				&TCON_UI_B, /* default */
				&TCON_UI_B, /* default */
				&TCON_SNOTE1_B,
				&TCON_SNOTE2_B,
				&TCON_CALL_B,
			},
		},
		/* auto brightness on */
		{
			/* Illumiatation Level 1 */
			{
				&TCON_UI_B,
				&TCON_UI_B,
				&TCON_UI_B,
				&TCON_UI_B,
				&TCON_UI_B, /* default */
				&TCON_UI_B, /* default */
				&TCON_UI_B, /* default */
				&TCON_UI_B, /* default */
				&TCON_BROWSER_B,
				&TCON_UI_B, /* default */
				&TCON_UI_B, /* default */
				&TCON_SNOTE1_B,
				&TCON_SNOTE2_B,
				&TCON_CALL_B,
			},
			/* Illumiatation Level 2 */
			{
				&TCON_POWER_SAVE_B,
				&TCON_UI_B,
				&TCON_UI_B,
				&TCON_UI_B,
				&TCON_POWER_SAVE_B, /* default */
				&TCON_POWER_SAVE_B, /* default */
				&TCON_POWER_SAVE_B, /* default */
				&TCON_POWER_SAVE_B, /* default */
				&TCON_BROWSER_B,
				&TCON_POWER_SAVE_B, /* default */
				&TCON_POWER_SAVE_B, /* default */
				&TCON_SNOTE1_B,
				&TCON_SNOTE2_B,
				&TCON_CALL_B,
			},
			/* Illumiatation Level 3 */
			{
				&TCON_OUTDOOR_B,
				&TCON_OUTDOOR_B,
				&TCON_OUTDOOR_B,
				&TCON_OUTDOOR_B,
				&TCON_OUTDOOR_B,
				&TCON_OUTDOOR_B,
				&TCON_OUTDOOR_B,
				&TCON_OUTDOOR_B,
				&TCON_OUTDOOR_B,
				&TCON_OUTDOOR_B,
				&TCON_OUTDOOR_B,
				&TCON_OUTDOOR_B,
				&TCON_OUTDOOR_B,
				&TCON_OUTDOOR_B,
			}
		}
	}
};


#else

struct tcon_reg_info TCON_UI = {
	.reg_cnt = 17,

	.addr = {
		0x0DB1, 0x0DB2, 0x0DB3, 0x0DB4, 0x0DB5,
		0x0DB6, 0x0DB7, 0x0DB8, 0x0DB9, 0x0DBA,
		0x0DBB, 0x0DBC, 0x0DBD, 0x0DBE, 0x0E39,
		0x0E3A, 0x0DC5,
		},
	.data = {
		0xB1,   0xFF,   0xF0,   0x1F,   0xF5,
		0xFE,   0x82,   0x46,   0x5A,   0xBF,
		0xC1,   0x04,   0x24,   0x26,   0x41,
		0x3F,   0xF4,
		},
};

struct tcon_reg_info TCON_POWER_SAVE = {
	.reg_cnt = 17,

	.addr = {
		0x0DB1, 0x0DB2, 0x0DB3, 0x0DB4, 0x0DB5,
		0x0DB6, 0x0DB7, 0x0DB8, 0x0DB9, 0x0DBA,
		0x0DBB, 0x0DBC, 0x0DBD, 0x0DBE, 0x0E39,
		0x0E3A, 0x0DC5,
		},
	.data = {
		0xB1,   0xFF,   0xF0,   0x0E,   0xC9,
		0x4E,   0x92,   0x4A,   0x59,   0xBF,
		0xC1,   0x04,   0x24,   0x26,   0x41,
		0x3F,   0xF4,
		},
};

struct tcon_reg_info TCON_VIDEO = {
	.reg_cnt = 17,

	.addr = {
		0x0DB1, 0x0DB2, 0x0DB3, 0x0DB4, 0x0DB5,
		0x0DB6, 0x0DB7, 0x0DB8, 0x0DB9, 0x0DBA,
		0x0DBB, 0x0DBC, 0x0DBD, 0x0DBE, 0x0E39,
		0x0E3A, 0x0DC5,
		},
	.data = {
		0xB1,   0xFF,   0xF0,   0x08,   0x85,
		0x5E,   0x92,   0x4A,   0x59,   0xBF,
		0xC1,   0x04,   0x24,   0x26,   0x41,
		0x3F,   0xF4,
		},
};

struct tcon_reg_info TCON_BROWSER = {
	.reg_cnt = 17,

	.addr = {
		0x0DB1, 0x0DB2, 0x0DB3, 0x0DB4, 0x0DB5,
		0x0DB6, 0x0DB7, 0x0DB8, 0x0DB9, 0x0DBA,
		0x0DBB, 0x0DBC, 0x0DBD, 0x0DBE, 0x0E39,
		0x0E3A, 0x0DC5,
		},
	.data = {
		0xB1,   0xFF,   0xF0,   0x08,   0x85,
		0x5E,   0x92,   0x4A,   0x59,   0xBF,
		0xC1,   0x04,   0x24,   0x26,   0x41,
		0x3F,   0xF4,
		},
};

struct tcon_reg_info TCON_OUTDOOR = {
	.reg_cnt = 17,

	.addr = {
		0x0DB1, 0x0DB2, 0x0DB3, 0x0DB4, 0x0DB5,
		0x0DB6, 0x0DB7, 0x0DB8, 0x0DB9, 0x0DBA,
		0x0DBB, 0x0DBC, 0x0DBD, 0x0DBE, 0x0E39,
		0x0E3A, 0x0DC5,
		},
	.data = {
		0xB0,  0x78,   0x10,   0x1F,   0xF5,
		0xFE,   0x82,   0x46,   0x5A,   0xBF,
		0xC1,  0x04,   0x24,   0x26,   0x41,
		0x3F,   0xF4,
		},
};

struct tcon_reg_info TCON_SNOTE1 = {
	.reg_cnt = 17,

	.addr = {
		0x0DB1, 0x0DB2, 0x0DB3, 0x0DB4, 0x0DB5,
		0x0DB6, 0x0DB7, 0x0DB8, 0x0DB9, 0x0DBA,
		0x0DBB, 0x0DBC, 0x0DBD, 0x0DBE, 0x0E39,
		0x0E3A, 0x0DC5,
		},
	.data = {
		0xB1,   0xFF,   0xF0,   0x1F,   0xF5,
		0xFE,   0x82,   0x42,   0x5A,   0xBF,
		0xC1,   0x04,   0x24,   0x26,   0x41,
		0x3F,   0xF4,
		},
};

struct tcon_reg_info TCON_SNOTE2 = {
	.reg_cnt = 17,

	.addr = {
		0x0DB1, 0x0DB2, 0x0DB3, 0x0DB4, 0x0DB5,
		0x0DB6, 0x0DB7, 0x0DB8, 0x0DB9, 0x0DBA,
		0x0DBB, 0x0DBC, 0x0DBD, 0x0DBE, 0x0E39,
		0x0E3A, 0x0DC5,
		},
	.data = {
		0xB1,   0xFF,   0xF0,   0x1F,   0xF5,
		0xFE,   0x82,   0x42,   0x7F,   0xFF,
		0xC1,   0x04,   0x24,   0x26,   0x41,
		0x3F,   0xF4,
		},
};

struct tcon_reg_info TCON_CALL = {
	.reg_cnt = 17,

	.addr = {
		0x0DB1, 0x0DB2, 0x0DB3, 0x0DB4, 0x0DB5,
		0x0DB6, 0x0DB7, 0x0DB8, 0x0DB9, 0x0DBA,
		0x0DBB, 0x0DBC, 0x0DBD, 0x0DBE, 0x0E39,
		0x0E3A, 0x0DC5,
		},
	.data = {
		0xB1,   0xFF,   0xF0,   0x1F,   0xF5,
		0xFE,   0x82,   0x46,   0x7F,   0xFF,
		0xC1,   0x04,   0x24,   0x26,	0x41,
		0x3F,   0xF4,
		},
};

struct tcon_reg_info TCON_BLACK_IMAGE_BLU_ENABLE = {
	.reg_cnt = 1,
	.addr = {
		0x0DB1,
		},
	.data = {
		0xB0,
		},
};

struct tcon_reg_info *tcon_tune_value[TCON_AUTO_BR_MAX][TCON_LEVEL_MAX][TCON_MODE_MAX] = {
		/*
			UI_APP = 0,
			VIDEO_APP,
			VIDEO_WARM_APP,
			VIDEO_COLD_APP,
			CAMERA_APP,
			NAVI_APP,
			GALLERY_APP,
			VT_APP,
			BROWSER_APP,
			eBOOK_APP,
			EMAIL_APP,
		*/
   	 /* auto brightness off */
	{

		/* Illumiatation Level 1 */
		{
			&TCON_UI,
			&TCON_UI,
			&TCON_UI,
			&TCON_UI,
			&TCON_UI, /* default */
			&TCON_UI, /* default */
			&TCON_UI, /* default */
			&TCON_UI, /* default */
			&TCON_BROWSER,
			&TCON_UI, /* default */
			&TCON_UI, /* default */
			&TCON_SNOTE1,
			&TCON_SNOTE2,
			&TCON_CALL,
		},
		/* Illumiatation Level 2 */
		{
			&TCON_UI,
			&TCON_UI,
			&TCON_UI,
			&TCON_UI,
			&TCON_UI, /* default */
			&TCON_UI, /* default */
			&TCON_UI, /* default */
			&TCON_UI, /* default */
			&TCON_BROWSER,
			&TCON_UI, /* default */
			&TCON_UI, /* default */
			&TCON_SNOTE1,
			&TCON_SNOTE2,
			&TCON_CALL,
		},
		/* Illumiatation Level 3 */
		{
			&TCON_UI,
			&TCON_UI,
			&TCON_UI,
			&TCON_UI,
			&TCON_UI, /* default */
			&TCON_UI, /* default */
			&TCON_UI, /* default */
			&TCON_UI, /* default */
			&TCON_BROWSER,
			&TCON_UI, /* default */
			&TCON_UI, /* default */
			&TCON_SNOTE1,
			&TCON_SNOTE2,
			&TCON_CALL,
		},
	},
	/* auto brightness on */
	{
		/* Illumiatation Level 1 */
		{
			&TCON_UI,
			&TCON_UI,
			&TCON_UI,
			&TCON_UI,
			&TCON_UI, /* default */
			&TCON_UI, /* default */
			&TCON_UI, /* default */
			&TCON_UI, /* default */
			&TCON_BROWSER,
			&TCON_UI, /* default */
			&TCON_UI, /* default */
			&TCON_SNOTE1,
			&TCON_SNOTE2,
			&TCON_CALL,
		},
		/* Illumiatation Level 2 */
		{
			&TCON_POWER_SAVE,
			&TCON_UI,
			&TCON_UI,
			&TCON_UI,
			&TCON_POWER_SAVE, /* default */
			&TCON_POWER_SAVE, /* default */
			&TCON_POWER_SAVE, /* default */
			&TCON_POWER_SAVE, /* default */
			&TCON_BROWSER,
			&TCON_POWER_SAVE, /* default */
			&TCON_POWER_SAVE, /* default */
			&TCON_SNOTE1,
			&TCON_SNOTE2,
			&TCON_CALL,
		},
		/* Illumiatation Level 3 */
		{
			&TCON_OUTDOOR,
			&TCON_OUTDOOR,
			&TCON_OUTDOOR,
			&TCON_OUTDOOR,
			&TCON_OUTDOOR,
			&TCON_OUTDOOR,
			&TCON_OUTDOOR,
			&TCON_OUTDOOR,
			&TCON_OUTDOOR,
			&TCON_OUTDOOR,
			&TCON_OUTDOOR,
			&TCON_OUTDOOR,
			&TCON_OUTDOOR,
			&TCON_OUTDOOR,
		}
	}
};

#endif
#endif
