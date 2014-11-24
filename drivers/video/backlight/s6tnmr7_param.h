#ifndef __S6E3FA0_PARAM_H__
#define __S6E3FA0_PARAM_H__

#define GAMMA_PARAM_SIZE	34
#define ACL_PARAM_SIZE	ARRAY_SIZE(SEQ_ACL_OFF)
#define ELVSS_PARAM_SIZE	ARRAY_SIZE(SEQ_ELVSS_CONDITION_SET)
#define AID_PARAM_SIZE	ARRAY_SIZE(SEQ_AOR_CONTROL)
#define ELVSS_TABLE_NUM 2


static const unsigned char SEQ_READ_ID[] = {
	0x04,
	0x5A, 0x5A,
};

static const unsigned char SEQ_TEST_KEY_ON_F0[] = {
	0xF0,
	0x5A, 0x5A,
};

static const unsigned char SEQ_TEST_KEY_ON_F1[] = {
	0xF1,
	0x5A, 0x5A,
};

static const unsigned char SEQ_TEST_KEY_ON_FC[] = {
	0xFC,
	0x5A, 0x5A,
};

static const unsigned char SEQ_TEST_KEY_OFF_FC[] = {
	0xFC,
	0xA5, 0xA5,
};

/* 545h, 03h */
static const unsigned char SEQ_MDNIE_EN_B0[] = {
	 0xB0,
	 0x45
};

static const unsigned char SEQ_MDNIE_EN[] = {
	 0xB6,
	 0x03
};

static const unsigned char SEQ_MDNIE_START_B0[] = {
	 0xB0,
	 0x82
};

/* TSP TE */
static const unsigned char SEQ_TSP_TE_B0[] = {
	 0xB0,
	 0xC7
};

static const unsigned char SEQ_TSP_TE_EN[] = {
	 0xB1,
	 0x07, 0x08
};


static const unsigned char SEQ_ERR_FG[] = {
	0xED,
	0x0C, 0x04
};

static const unsigned char SEQ_APPLY_LEVEL_2_KEY[] = {
	 0xF0,
	 0x5A, 0x5A
};

static const unsigned char SEQ_GAMMA_CONTROL_SET_300CD[] = {
	0x83,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x80, 0x80,
	0x00,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x80, 0x80,
	0x00,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x80, 0x80,
	0x00,
};;

static const unsigned char SEQ_ACL_CONDITION[] = {
	0xB5,
	0x03, 0x99, 0x27, 0x35, 0x45, 0x0A,
};

static const unsigned char SEQ_AOR_CONTROL[] = {
	0x85,
	0x06, 0x00,
};

const unsigned char *pSEQ_AOR_CONTROL = SEQ_AOR_CONTROL;

static const unsigned char SEQ_ELVSS_CONDITION_SET[] = {
	0xBB,
	0x19,
};

static const unsigned char SEQ_GAMMA_UPDATE[] = {
	0xBB,
	0x01,
};

static const unsigned char SEQ_GLOBAL_PARAM_47RD[] = {
	0xB0,
	0x2E,
};

static const unsigned char SEQ_GLOBAL_PARAM_53RD[] = {
	0xB0,
	0x34,
};

static const unsigned char SEQ_SLEEP_OUT[] = {
	0x11,
};

static const unsigned char SEQ_PENTILE_CONDITION[] = {
	0xC0,
	0x00, 0x02, 0x03, 0x32, 0x03, 0x44, 0x44, 0xC0, 0x00, 0x1C,
	0x20, 0xE8,
};

static const unsigned char SEQ_RE_SETTING_EVT1_1[] = {
	0xC0,
	0x00, 0x02, 0x03, 0x32, 0x03, 0x44, 0x44, 0xC0, 0x00, 0x1C,
	0x20, 0xE8,
};

static const unsigned char SEQ_RE_SETTING_EVT1_2[] = {
	0xE3,
	0xFF, 0xFF, 0xFF, 0xFF,
};

static const unsigned char SEQ_RE_SETTING_EVT1_3[] = {
	0xFE,
	0x00, 0x03,
};

static const unsigned char SEQ_RE_SETTING_EVT1_4[] = {
	0xB0,
	0x2B,
};

static const unsigned char SEQ_RE_SETTING_EVT1_5[] = {
	0xFE,
	0xE4,
};

static const unsigned char SEQ_GLOBAL_PARAM_TEMP_OFFSET[] = {
	0xB0,
	0x34,
};

static const unsigned char SEQ_TEMP_OFFSET_CONDITION[] = {
	0xB6,
	0x00, 0x06, 0x66, 0x6C, 0x0C,
};

static const unsigned char SEQ_GLOBAL_PARAM_SOURCE_AMP[] = {
	0xB0,
	0x24,
};

static const unsigned char SEQ_SOURCE_AMP_A[] = {
	0xD7,
	0xA5,
};

static const unsigned char SEQ_GLOBAL_PARAM_BIAS_CURRENT[] = {
	0xB0,
	0x1F,
};

static const unsigned char SEQ_BIAS_CURRENT[] = {
	0xD7,
	0x0A,
};

static const unsigned char SEQ_GLOBAL_PARAM_ILVL[] = {
	0xB0,
	0x01,
};

static const unsigned char SEQ_ILVL[] = {
	0xE8,
	0x34,
};

static const unsigned char SEQ_GLOBAL_PARAM_VLIN1[] = {
	0xB0,
	0x02,
};

static const unsigned char SEQ_VLIN1[] = {
	0xB8,
	0x40,
};

static const unsigned char SEQ_GLOBAL_PARAM_TSET[] = {
	0xB0,
	0x05,
};

static const unsigned char SEQ_TSET[] = {
	0xB8,
	0x19,
};

static const unsigned char SEQ_GLOBAL_PARAM_ELVSSHBM[] = {
	0xB0,
	0x35,
};

static const unsigned char SEQ_GLOBAL_PARAM_ACL[] = {
	0xB0,
	0x45,
};

static const unsigned char SEQ_GLOBAL_PARAM_OPRAVR_CAL[] = {
	0xB0,
	0x4A,
};

static const unsigned char SEQ_GLOBAL_PARAM_ACLUPDATE[] = {
	0xB0,
	0x2E,
};

static const unsigned char SEQ_TE_ON[] = {
	0x35,
	0x00
};

static const unsigned char SEQ_TE_OFF[] = {
	0x34,
};

static const unsigned char SEQ_DISPLAY_ON[] = {
	0x29,
};

static const unsigned char SEQ_DISPLAY_OFF[] = {
	0x28,
};

static const unsigned char SEQ_SLEEP_IN[] = {
	0x10,
};

static const unsigned char SEQ_PARTIAL_DISP_OFF[] = {
	0x13,
};

static const unsigned char SEQ_PARTIAL_DISP_ON[] = {
	0x12,
};

static const unsigned char SEQ_LDI_FPS_READ[] = {
	0xD7,
};

static const unsigned char SEQ_LDI_FPS_POS[] = {
	0xB0,
	0x1A,
};

static const unsigned char SEQ_PCD_SET_DET_LOW[] = {
	0xCC,
	0x5C, 0x51,
};

static const unsigned char SEQ_TOUCHKEY_OFF[] = {
	0xFF,
	0x00,
};

static const unsigned char SEQ_TOUCHKEY_ON[] = {
	0xFF,
	0x01,
};





#if !defined(CONFIG_I80_COMMAND_MODE)
static const unsigned char SEQ_DISPCTL[] = {
	0xF2,
	0x02, 0x03, 0xC, 0xA0, 0x01, 0x48,
};
#endif

enum {
	TSET_25_DEGREES,
	TSET_MINUS_0_DEGREES,
	TSET_MINUS_20_DEGREES,
	TSET_STATUS_MAX,
};

static const unsigned char TSET_TABLE[TSET_STATUS_MAX] = {
	0x19,	/* +25 degree */
	0x00,	/* -0 degree */
	0x94,	/* -20 degree */
};



enum {
	ELVSS_STATUS_105,
	ELVSS_STATUS_111,
	ELVSS_STATUS_119,
	ELVSS_STATUS_126,
	ELVSS_STATUS_134,
	ELVSS_STATUS_143,
	ELVSS_STATUS_152,
	ELVSS_STATUS_162,
	ELVSS_STATUS_172,
	ELVSS_STATUS_183,
	ELVSS_STATUS_195,
	ELVSS_STATUS_207,
	ELVSS_STATUS_220,
	ELVSS_STATUS_234,
	ELVSS_STATUS_249,
	ELVSS_STATUS_265,
	ELVSS_STATUS_282,
	ELVSS_STATUS_300,
	ELVSS_STATUS_HBM,
	ELVSS_STATUS_MAX
};

static const unsigned int ELVSS_DIM_TABLE[ELVSS_STATUS_MAX] = {
	105,111,119,126,134,143,152,162,172,183,195,207,220,234,249,265,282,300,400	
};



static const int  ELVSS_DELTA_RevD = 8;
static const unsigned char ELVSS_TABLE_RevD[ELVSS_STATUS_MAX][ELVSS_TABLE_NUM] = {
	{0x1F, 0x1F},
	{0x1F, 0x1F},
	{0x1F, 0x1F},
	{0x1F, 0x1F},
	{0x1F, 0x1F},
	{0x1F, 0x1F},
	{0x1F, 0x1F},
	{0x1F, 0x1F},
	{0x1F, 0x1F},
	{0x1F, 0x1F},
	{0x1F, 0x1F},
	{0x1F, 0x1F},
	{0x1E, 0x1F},
	{0x1D, 0x1F},
	{0x1C, 0x1F},
	{0x1B, 0x1E},
	{0x1A, 0x1D},
	{0x19, 0x1C},
	{0x19, 0x19},//hbm
};

static const unsigned char ELVSS_TABLE_RevF[ELVSS_STATUS_MAX][ELVSS_TABLE_NUM] = {
	{0x1F, 0x1F},
	{0x1F, 0x1F},
	{0x1F, 0x1F},
	{0x1F, 0x1F},
	{0x1F, 0x1F},
	{0x1E, 0x1F},
	{0x1D, 0x1F},
	{0x1D, 0x1F},
	{0x1D, 0x1F},
	{0x1D, 0x1F},
	{0x1D, 0x1F},
	{0x1D, 0x1F},
	{0x1D, 0x1F},
	{0x1D, 0x1F},
	{0x1C, 0x1F},
	{0x1B, 0x1E},
	{0x1A, 0x1D},
	{0x19, 0x1C},
	{0x19, 0x19},//hbm
};

static const int  ELVSS_DELTA = 11;
static const unsigned char ELVSS_TABLE[ELVSS_STATUS_MAX][ELVSS_TABLE_NUM] = {
	{0x1F, 0x1F},
	{0x1F, 0x1F},
	{0x1F, 0x1F},
	{0x1F, 0x1F},
	{0x1F, 0x1F},
	{0x1E, 0x1F},
	{0x1D, 0x1F},
	{0x1D, 0x1F},
	{0x1D, 0x1F},
	{0x1D, 0x1F},
	{0x1D, 0x1F},
	{0x1D, 0x1F},
	{0x1D, 0x1F},
	{0x1D, 0x1F},
	{0x1C, 0x1E},
	{0x1B, 0x1D},
	{0x1A, 0x1C},
	{0x19, 0x1B},
	{0x19, 0x19},//hbm
};


const unsigned char (*pELVSS_TABLE)[ELVSS_TABLE_NUM] = ELVSS_TABLE;
const int *pelvss_delta = &ELVSS_DELTA;

enum {
	ACL_STATUS_0P,
	ACL_STATUS_15P,
	ACL_STATUS_MAX
};

static const unsigned char SEQ_ACL_OFF[] = {
	0xBB,
	0x10
};

static const unsigned char SEQ_ACL_15[] = {
	0xBB,
	0x12,
};


static const unsigned char SEQ_ACL_OPR_AVR_CAL[] = {
	0xBB,
	0x1A,
};

static const unsigned char SEQ_ACL_UPDATE[] = {
	0xBB,
	0x01,
};


static const unsigned char *ACL_CUTOFF_TABLE[ACL_STATUS_MAX] = {
	SEQ_ACL_OFF,
	SEQ_ACL_15,
};
#endif /* __S6E3FA0_PARAM_H__ */
