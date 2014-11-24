#ifndef __MDNIE_TABLE_H__
#define __MDNIE_TABLE_H__

/* 2014.06.02 */
/* SCR Position can be different each panel */
#define ASCR_CMD		MDNIE_CMD1

/* SCR Position can be different each panel */
#define MDNIE_RED_R		104		/* ASCR_WIDE_CR[7:0] */
#define MDNIE_RED_G		106		/* ASCR_WIDE_CG[7:0] */
#define MDNIE_RED_B		108		/* ASCR_WIDE_CB[7:0] */
#define MDNIE_BLUE_R		110		/* ASCR_WIDE_MR[7:0] */
#define MDNIE_BLUE_G		112		/* ASCR_WIDE_MG[7:0] */
#define MDNIE_BLUE_B		114		/* ASCR_WIDE_MB[7:0] */
#define MDNIE_GREEN_R		116		/* ASCR_WIDE_YR[7:0] */
#define MDNIE_GREEN_G		118		/* ASCR_WIDE_YG[7:0] */
#define MDNIE_GREEN_B		120		/* ASCR_WIDE_YB[7:0] */
#define MDNIE_WHITE_R		122		/* ASCR_WIDE_WR[7:0] */
#define MDNIE_WHITE_G		124		/* ASCR_WIDE_WG[7:0] */
#define MDNIE_WHITE_B		126		/* ASCR_WIDE_WB[7:0] */

#define MDNIE_COLOR_BLIND_OFFSET	MDNIE_RED_R

#define COLOR_OFFSET_F1(x, y)		(((y << 10) - (((x << 10) * 164) / 151) + (8 << 10)) >> 10)
#define COLOR_OFFSET_F2(x, y)		(((y << 10) - (((x << 10) * 70) / 67) - (7 << 10)) >> 10)
#define COLOR_OFFSET_F3(x, y)		(((y << 10) + (((x << 10) * 181) / 35) - (18852 << 10)) >> 10)
#define COLOR_OFFSET_F4(x, y)		(((y << 10) + (((x << 10) * 157) / 52) - (12055 << 10)) >> 10)

#define HBM_ON_TEXT  0

/* color coordination order is WR, WG, WB */
static unsigned char coordinate_data[][3] = {
	{0xff, 0xff, 0xff}, /* dummy */
	{0xff, 0xfa, 0xfa}, /* Tune_1 */
	{0xff, 0xfb, 0xfe}, /* Tune_2 */
	{0xfc, 0xfb, 0xff}, /* Tune_3 */
	{0xff, 0xfd, 0xfb}, /* Tune_4 */
	{0xff, 0xff, 0xff}, /* Tune_5 */
	{0xfb, 0xfc, 0xff}, /* Tune_6 */
	{0xfd, 0xff, 0xfa}, /* Tune_7 */
	{0xfc, 0xff, 0xfc}, /* Tune_8 */
	{0xfb, 0xff, 0xff}, /* Tune_9 */
};

////////////////// UI /// /////////////////////
static unsigned char SCREEN_CURTAIN_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x02, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char SCREEN_CURTAIN_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x00, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x15,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x00,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x00, /* curve_5_b */
	0x20, /* curve_5_a */
	0x00, /* curve_6_b */
	0x20, /* curve_6_a */
	0x00, /* curve_7_b */
	0x20, /* curve_7_a */
	0x00, /* curve_8_b */
	0x20, /* curve_8_a */
	0x00, /* curve_9_b */
	0x20, /* curve_9_a */
	0x00, /* curve10_b */
	0x20, /* curve10_a */
	0x00, /* curve11_b */
	0x20, /* curve11_a */
	0x00, /* curve12_b */
	0x20, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xff, /* curve24_a */
	0x20, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x17, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x27, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x59,
	0x0b,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x34,
	0x83,
	0xff, /* ascr_skin_Rr */
	0x00, /* ascr_skin_Rg */
	0x00, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0x00, /* ascr_Rr */
	0x00, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0x00, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0x00, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x00, /* ascr_Mg */
	0x00, /* ascr_Gg */
	0x00, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0x00, /* ascr_Yr */
	0x00, /* ascr_Br */
	0x00, /* ascr_Yg */
	0x00, /* ascr_Bg */
	0x00, /* ascr_Yb */
	0x00, /* ascr_Bb */
	0x00, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0x00, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0x00, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char STANDARD_UI_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x02, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char STANDARD_UI_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x00, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x10,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x00,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x00, /* curve_5_b */
	0x20, /* curve_5_a */
	0x00, /* curve_6_b */
	0x20, /* curve_6_a */
	0x00, /* curve_7_b */
	0x20, /* curve_7_a */
	0x00, /* curve_8_b */
	0x20, /* curve_8_a */
	0x00, /* curve_9_b */
	0x20, /* curve_9_a */
	0x00, /* curve10_b */
	0x20, /* curve10_a */
	0x00, /* curve11_b */
	0x20, /* curve11_a */
	0x00, /* curve12_b */
	0x20, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xff, /* curve24_a */
	0x20, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x17, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x27, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x59,
	0x0b,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x34,
	0x83,
	0xff, /* ascr_skin_Rr */
	0x00, /* ascr_skin_Rg */
	0x00, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0xFF, /* ascr_Rr */
	0xF8, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0xF3, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0xFF, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x14, /* ascr_Mg */
	0xF4, /* ascr_Gg */
	0xE6, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0xFF, /* ascr_Yr */
	0x2D, /* ascr_Br */
	0xFA, /* ascr_Yg */
	0x1A, /* ascr_Bg */
	0x37, /* ascr_Yb */
	0xF8, /* ascr_Bb */
	0xFF, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xF8, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xEE, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char NATURAL_UI_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x32, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char NATURAL_UI_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x00, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x10,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x00,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x00, /* curve_5_b */
	0x20, /* curve_5_a */
	0x00, /* curve_6_b */
	0x20, /* curve_6_a */
	0x00, /* curve_7_b */
	0x20, /* curve_7_a */
	0x00, /* curve_8_b */
	0x20, /* curve_8_a */
	0x00, /* curve_9_b */
	0x20, /* curve_9_a */
	0x00, /* curve10_b */
	0x20, /* curve10_a */
	0x00, /* curve11_b */
	0x20, /* curve11_a */
	0x00, /* curve12_b */
	0x20, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xff, /* curve24_a */
	0x20, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x17, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x27, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x59,
	0x0b,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x34,
	0x83,
	0xff, /* ascr_skin_Rr */
	0x00, /* ascr_skin_Rg */
	0x00, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x87, /* ascr_Cr */
	0xE7, /* ascr_Rr */
	0xFF, /* ascr_Cg */
	0x23, /* ascr_Rg */
	0xF2, /* ascr_Cb */
	0x1E, /* ascr_Rb */
	0xF3, /* ascr_Mr */
	0x7D, /* ascr_Gr */
	0x2E, /* ascr_Mg */
	0xF7, /* ascr_Gg */
	0xF4, /* ascr_Mb */
	0x3C, /* ascr_Gb */
	0xFA, /* ascr_Yr */
	0x2D, /* ascr_Br */
	0xF5, /* ascr_Yg */
	0x19, /* ascr_Bg */
	0x3C, /* ascr_Yb */
	0xED, /* ascr_Bb */
	0xFF, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xF8, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xEE, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char DYNAMIC_UI_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x32, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char DYNAMIC_UI_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x03, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x20,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x20,
	0x00, /* curve_1_b */
	0x14, /* curve_1_a */
	0x00, /* curve_2_b */
	0x14, /* curve_2_a */
	0x00, /* curve_3_b */
	0x14, /* curve_3_a */
	0x00, /* curve_4_b */
	0x14, /* curve_4_a */
	0x03, /* curve_5_b */
	0x9a, /* curve_5_a */
	0x03, /* curve_6_b */
	0x9a, /* curve_6_a */
	0x03, /* curve_7_b */
	0x9a, /* curve_7_a */
	0x03, /* curve_8_b */
	0x9a, /* curve_8_a */
	0x07, /* curve_9_b */
	0x9e, /* curve_9_a */
	0x07, /* curve10_b */
	0x9e, /* curve10_a */
	0x07, /* curve11_b */
	0x9e, /* curve11_a */
	0x07, /* curve12_b */
	0x9e, /* curve12_a */
	0x0a, /* curve13_b */
	0xa0, /* curve13_a */
	0x0a, /* curve14_b */
	0xa0, /* curve14_a */
	0x0a, /* curve15_b */
	0xa0, /* curve15_a */
	0x0a, /* curve16_b */
	0xa0, /* curve16_a */
	0x16, /* curve17_b */
	0xa6, /* curve17_a */
	0x16, /* curve18_b */
	0xa6, /* curve18_a */
	0x16, /* curve19_b */
	0xa6, /* curve19_a */
	0x16, /* curve20_b */
	0xa6, /* curve20_a */
	0x05, /* curve21_b */
	0x21, /* curve21_a */
	0x0b, /* curve22_b */
	0x20, /* curve22_a */
	0x87, /* curve23_b */
	0x0f, /* curve23_a */
	0x00, /* curve24_b */
	0xFF, /* curve24_a */
	0x30, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x37, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x47, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x25,
	0x3d,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x1c,
	0xd8,
	0xff, /* ascr_skin_Rr */
	0x50, /* ascr_skin_Rg */
	0x60, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0xff, /* ascr_Rr */
	0xff, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0xff, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0xff, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x00, /* ascr_Mg */
	0xff, /* ascr_Gg */
	0xff, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0xff, /* ascr_Yr */
	0x00, /* ascr_Br */
	0xff, /* ascr_Yg */
	0x00, /* ascr_Bg */
	0x00, /* ascr_Yb */
	0xff, /* ascr_Bb */
	0xff, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xff, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xff, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char MOVIE_UI_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x32, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char MOVIE_UI_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x02, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x10,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x20,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x00, /* curve_5_b */
	0x20, /* curve_5_a */
	0x00, /* curve_6_b */
	0x20, /* curve_6_a */
	0x00, /* curve_7_b */
	0x20, /* curve_7_a */
	0x00, /* curve_8_b */
	0x20, /* curve_8_a */
	0x00, /* curve_9_b */
	0x20, /* curve_9_a */
	0x00, /* curve10_b */
	0x20, /* curve10_a */
	0x00, /* curve11_b */
	0x20, /* curve11_a */
	0x00, /* curve12_b */
	0x20, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xff, /* curve24_a */
	0x20, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x17, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x27, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x59,
	0x0b,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x34,
	0x83,
	0xff, /* ascr_skin_Rr */
	0x00, /* ascr_skin_Rg */
	0x00, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x7a, /* ascr_Cr */
	0xff, /* ascr_Rr */
	0xff, /* ascr_Cg */
	0x2b, /* ascr_Rg */
	0xf1, /* ascr_Cb */
	0x2d, /* ascr_Rb */
	0xff, /* ascr_Mr */
	0x68, /* ascr_Gr */
	0x15, /* ascr_Mg */
	0xff, /* ascr_Gg */
	0xfa, /* ascr_Mb */
	0x31, /* ascr_Gb */
	0xf8, /* ascr_Yr */
	0x34, /* ascr_Br */
	0xff, /* ascr_Yg */
	0x20, /* ascr_Bg */
	0x4b, /* ascr_Yb */
	0xff, /* ascr_Bb */
	0xff, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xf8, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xef, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char AUTO_UI_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x32, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char AUTO_UI_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x03, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x15,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x20,
	0x00, /* curve_1_b */
	0x14, /* curve_1_a */
	0x00, /* curve_2_b */
	0x14, /* curve_2_a */
	0x00, /* curve_3_b */
	0x14, /* curve_3_a */
	0x00, /* curve_4_b */
	0x14, /* curve_4_a */
	0x03, /* curve_5_b */
	0x9a, /* curve_5_a */
	0x03, /* curve_6_b */
	0x9a, /* curve_6_a */
	0x03, /* curve_7_b */
	0x9a, /* curve_7_a */
	0x03, /* curve_8_b */
	0x9a, /* curve_8_a */
	0x07, /* curve_9_b */
	0x9e, /* curve_9_a */
	0x07, /* curve10_b */
	0x9e, /* curve10_a */
	0x07, /* curve11_b */
	0x9e, /* curve11_a */
	0x07, /* curve12_b */
	0x9e, /* curve12_a */
	0x0a, /* curve13_b */
	0xa0, /* curve13_a */
	0x0a, /* curve14_b */
	0xa0, /* curve14_a */
	0x0a, /* curve15_b */
	0xa0, /* curve15_a */
	0x0a, /* curve16_b */
	0xa0, /* curve16_a */
	0x16, /* curve17_b */
	0xa6, /* curve17_a */
	0x16, /* curve18_b */
	0xa6, /* curve18_a */
	0x16, /* curve19_b */
	0xa6, /* curve19_a */
	0x16, /* curve20_b */
	0xa6, /* curve20_a */
	0x05, /* curve21_b */
	0x21, /* curve21_a */
	0x0b, /* curve22_b */
	0x20, /* curve22_a */
	0x87, /* curve23_b */
	0x0f, /* curve23_a */
	0x00, /* curve24_b */
	0xFF, /* curve24_a */
	0x30, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x37, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x47, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x25,
	0x3d,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x1c,
	0xd8,
	0xff, /* ascr_skin_Rr */
	0x7c, /* ascr_skin_Rg */
	0x88, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xf4, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0xff, /* ascr_Rr */
	0xff, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0xff, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0xff, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x00, /* ascr_Mg */
	0xff, /* ascr_Gg */
	0xff, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0xff, /* ascr_Yr */
	0x00, /* ascr_Br */
	0xff, /* ascr_Yg */
	0x00, /* ascr_Bg */
	0x00, /* ascr_Yb */
	0xff, /* ascr_Bb */
	0xff, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xff, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xff, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

////////////////// GALLERY /////////////////////
static unsigned char STANDARD_GALLERY_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x32, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char STANDARD_GALLERY_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x04, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x10,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x00,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x00, /* curve_5_b */
	0x20, /* curve_5_a */
	0x00, /* curve_6_b */
	0x20, /* curve_6_a */
	0x00, /* curve_7_b */
	0x20, /* curve_7_a */
	0x00, /* curve_8_b */
	0x20, /* curve_8_a */
	0x00, /* curve_9_b */
	0x20, /* curve_9_a */
	0x00, /* curve10_b */
	0x20, /* curve10_a */
	0x00, /* curve11_b */
	0x20, /* curve11_a */
	0x00, /* curve12_b */
	0x20, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xff, /* curve24_a */
	0x20, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x17, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x27, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x59,
	0x0b,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x34,
	0x83,
	0xff, /* ascr_skin_Rr */
	0x00, /* ascr_skin_Rg */
	0x00, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0xFF, /* ascr_Rr */
	0xF8, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0xF3, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0xFF, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x14, /* ascr_Mg */
	0xF4, /* ascr_Gg */
	0xE6, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0xFF, /* ascr_Yr */
	0x2D, /* ascr_Br */
	0xFA, /* ascr_Yg */
	0x1A, /* ascr_Bg */
	0x37, /* ascr_Yb */
	0xF8, /* ascr_Bb */
	0xFF, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xF8, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xEE, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char NATURAL_GALLERY_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x32, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char NATURAL_GALLERY_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x04, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x10,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x00,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x00, /* curve_5_b */
	0x20, /* curve_5_a */
	0x00, /* curve_6_b */
	0x20, /* curve_6_a */
	0x00, /* curve_7_b */
	0x20, /* curve_7_a */
	0x00, /* curve_8_b */
	0x20, /* curve_8_a */
	0x00, /* curve_9_b */
	0x20, /* curve_9_a */
	0x00, /* curve10_b */
	0x20, /* curve10_a */
	0x00, /* curve11_b */
	0x20, /* curve11_a */
	0x00, /* curve12_b */
	0x20, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xff, /* curve24_a */
	0x20, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x17, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x27, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x59,
	0x0b,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x34,
	0x83,
	0xff, /* ascr_skin_Rr */
	0x00, /* ascr_skin_Rg */
	0x00, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x87, /* ascr_Cr */
	0xE7, /* ascr_Rr */
	0xFF, /* ascr_Cg */
	0x23, /* ascr_Rg */
	0xF2, /* ascr_Cb */
	0x1E, /* ascr_Rb */
	0xF3, /* ascr_Mr */
	0x7D, /* ascr_Gr */
	0x2E, /* ascr_Mg */
	0xF7, /* ascr_Gg */
	0xF4, /* ascr_Mb */
	0x3C, /* ascr_Gb */
	0xFA, /* ascr_Yr */
	0x2D, /* ascr_Br */
	0xF5, /* ascr_Yg */
	0x19, /* ascr_Bg */
	0x3C, /* ascr_Yb */
	0xED, /* ascr_Bb */
	0xFF, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xF8, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xEE, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char DYNAMIC_GALLERY_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x32, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char DYNAMIC_GALLERY_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x07, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x20,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x20,
	0x00, /* curve_1_b */
	0x14, /* curve_1_a */
	0x00, /* curve_2_b */
	0x14, /* curve_2_a */
	0x00, /* curve_3_b */
	0x14, /* curve_3_a */
	0x00, /* curve_4_b */
	0x14, /* curve_4_a */
	0x03, /* curve_5_b */
	0x9a, /* curve_5_a */
	0x03, /* curve_6_b */
	0x9a, /* curve_6_a */
	0x03, /* curve_7_b */
	0x9a, /* curve_7_a */
	0x03, /* curve_8_b */
	0x9a, /* curve_8_a */
	0x07, /* curve_9_b */
	0x9e, /* curve_9_a */
	0x07, /* curve10_b */
	0x9e, /* curve10_a */
	0x07, /* curve11_b */
	0x9e, /* curve11_a */
	0x07, /* curve12_b */
	0x9e, /* curve12_a */
	0x0a, /* curve13_b */
	0xa0, /* curve13_a */
	0x0a, /* curve14_b */
	0xa0, /* curve14_a */
	0x0a, /* curve15_b */
	0xa0, /* curve15_a */
	0x0a, /* curve16_b */
	0xa0, /* curve16_a */
	0x16, /* curve17_b */
	0xa6, /* curve17_a */
	0x16, /* curve18_b */
	0xa6, /* curve18_a */
	0x16, /* curve19_b */
	0xa6, /* curve19_a */
	0x16, /* curve20_b */
	0xa6, /* curve20_a */
	0x05, /* curve21_b */
	0x21, /* curve21_a */
	0x0b, /* curve22_b */
	0x20, /* curve22_a */
	0x87, /* curve23_b */
	0x0f, /* curve23_a */
	0x00, /* curve24_b */
	0xFF, /* curve24_a */
	0x30, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x37, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x47, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x25,
	0x3d,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x1c,
	0xd8,
	0xff, /* ascr_skin_Rr */
	0x50, /* ascr_skin_Rg */
	0x60, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0xff, /* ascr_Rr */
	0xff, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0xff, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0xff, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x00, /* ascr_Mg */
	0xff, /* ascr_Gg */
	0xff, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0xff, /* ascr_Yr */
	0x00, /* ascr_Br */
	0xff, /* ascr_Yg */
	0x00, /* ascr_Bg */
	0x00, /* ascr_Yb */
	0xff, /* ascr_Bb */
	0xff, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xff, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xff, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char MOVIE_GALLERY_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x32, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char MOVIE_GALLERY_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x06, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x10,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x20,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x00, /* curve_5_b */
	0x20, /* curve_5_a */
	0x00, /* curve_6_b */
	0x20, /* curve_6_a */
	0x00, /* curve_7_b */
	0x20, /* curve_7_a */
	0x00, /* curve_8_b */
	0x20, /* curve_8_a */
	0x00, /* curve_9_b */
	0x20, /* curve_9_a */
	0x00, /* curve10_b */
	0x20, /* curve10_a */
	0x00, /* curve11_b */
	0x20, /* curve11_a */
	0x00, /* curve12_b */
	0x20, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xff, /* curve24_a */
	0x20, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x17, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x27, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x59,
	0x0b,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x34,
	0x83,
	0xff, /* ascr_skin_Rr */
	0x00, /* ascr_skin_Rg */
	0x00, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x7a, /* ascr_Cr */
	0xff, /* ascr_Rr */
	0xff, /* ascr_Cg */
	0x2b, /* ascr_Rg */
	0xf1, /* ascr_Cb */
	0x2d, /* ascr_Rb */
	0xff, /* ascr_Mr */
	0x68, /* ascr_Gr */
	0x15, /* ascr_Mg */
	0xff, /* ascr_Gg */
	0xfa, /* ascr_Mb */
	0x31, /* ascr_Gb */
	0xf8, /* ascr_Yr */
	0x34, /* ascr_Br */
	0xff, /* ascr_Yg */
	0x20, /* ascr_Bg */
	0x4b, /* ascr_Yb */
	0xff, /* ascr_Bb */
	0xff, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xf8, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xef, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char AUTO_GALLERY_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x32, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char AUTO_GALLERY_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x04, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x15,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x00,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x00, /* curve_5_b */
	0x20, /* curve_5_a */
	0x00, /* curve_6_b */
	0x20, /* curve_6_a */
	0x00, /* curve_7_b */
	0x20, /* curve_7_a */
	0x00, /* curve_8_b */
	0x20, /* curve_8_a */
	0x00, /* curve_9_b */
	0x20, /* curve_9_a */
	0x00, /* curve10_b */
	0x20, /* curve10_a */
	0x00, /* curve11_b */
	0x20, /* curve11_a */
	0x00, /* curve12_b */
	0x20, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xff, /* curve24_a */
	0x30, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x17, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x27, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x59,
	0x0b,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x34,
	0x83,
	0xff, /* ascr_skin_Rr */
	0x70, /* ascr_skin_Rg */
	0x80, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xf8, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0xff, /* ascr_Rr */
	0xff, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0xff, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0xff, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x00, /* ascr_Mg */
	0xff, /* ascr_Gg */
	0xff, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0xff, /* ascr_Yr */
	0x00, /* ascr_Br */
	0xff, /* ascr_Yg */
	0x00, /* ascr_Bg */
	0x00, /* ascr_Yb */
	0xff, /* ascr_Bb */
	0xff, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xff, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xff, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

////////////////// VIDEO /////////////////////
static unsigned char STANDARD_VIDEO_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x32, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char STANDARD_VIDEO_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x04, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x10,
	0x00, /* sharpen_maxplus 11 */
	0x40,
	0x00, /* sharpen_maxminus 11 */
	0x40,
	0x01, /* cs_gain 10 */
	0x00,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x00, /* curve_5_b */
	0x20, /* curve_5_a */
	0x00, /* curve_6_b */
	0x20, /* curve_6_a */
	0x00, /* curve_7_b */
	0x20, /* curve_7_a */
	0x00, /* curve_8_b */
	0x20, /* curve_8_a */
	0x00, /* curve_9_b */
	0x20, /* curve_9_a */
	0x00, /* curve10_b */
	0x20, /* curve10_a */
	0x00, /* curve11_b */
	0x20, /* curve11_a */
	0x00, /* curve12_b */
	0x20, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xff, /* curve24_a */
	0x20, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x17, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x27, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x59,
	0x0b,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x34,
	0x83,
	0xff, /* ascr_skin_Rr */
	0x00, /* ascr_skin_Rg */
	0x00, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0xFF, /* ascr_Rr */
	0xF8, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0xF3, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0xFF, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x14, /* ascr_Mg */
	0xF4, /* ascr_Gg */
	0xE6, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0xFF, /* ascr_Yr */
	0x2D, /* ascr_Br */
	0xFA, /* ascr_Yg */
	0x1A, /* ascr_Bg */
	0x37, /* ascr_Yb */
	0xF8, /* ascr_Bb */
	0xFF, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xF8, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xEE, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char NATURAL_VIDEO_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x32, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char NATURAL_VIDEO_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x04, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x10,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x00,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x00, /* curve_5_b */
	0x20, /* curve_5_a */
	0x00, /* curve_6_b */
	0x20, /* curve_6_a */
	0x00, /* curve_7_b */
	0x20, /* curve_7_a */
	0x00, /* curve_8_b */
	0x20, /* curve_8_a */
	0x00, /* curve_9_b */
	0x20, /* curve_9_a */
	0x00, /* curve10_b */
	0x20, /* curve10_a */
	0x00, /* curve11_b */
	0x20, /* curve11_a */
	0x00, /* curve12_b */
	0x20, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xff, /* curve24_a */
	0x20, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x17, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x27, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x59,
	0x0b,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x34,
	0x83,
	0xff, /* ascr_skin_Rr */
	0x00, /* ascr_skin_Rg */
	0x00, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x87, /* ascr_Cr */
	0xE7, /* ascr_Rr */
	0xFF, /* ascr_Cg */
	0x23, /* ascr_Rg */
	0xF2, /* ascr_Cb */
	0x1E, /* ascr_Rb */
	0xF3, /* ascr_Mr */
	0x7D, /* ascr_Gr */
	0x2E, /* ascr_Mg */
	0xF7, /* ascr_Gg */
	0xF4, /* ascr_Mb */
	0x3C, /* ascr_Gb */
	0xFA, /* ascr_Yr */
	0x2D, /* ascr_Br */
	0xF5, /* ascr_Yg */
	0x19, /* ascr_Bg */
	0x3C, /* ascr_Yb */
	0xED, /* ascr_Bb */
	0xFF, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xF8, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xEE, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char DYNAMIC_VIDEO_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x32, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char DYNAMIC_VIDEO_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x07, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x20,
	0x00, /* sharpen_maxplus 11 */
	0x40,
	0x00, /* sharpen_maxminus 11 */
	0x40,
	0x01, /* cs_gain 10 */
	0x20,
	0x00, /* curve_1_b */
	0x14, /* curve_1_a */
	0x00, /* curve_2_b */
	0x14, /* curve_2_a */
	0x00, /* curve_3_b */
	0x14, /* curve_3_a */
	0x00, /* curve_4_b */
	0x14, /* curve_4_a */
	0x03, /* curve_5_b */
	0x9a, /* curve_5_a */
	0x03, /* curve_6_b */
	0x9a, /* curve_6_a */
	0x03, /* curve_7_b */
	0x9a, /* curve_7_a */
	0x03, /* curve_8_b */
	0x9a, /* curve_8_a */
	0x07, /* curve_9_b */
	0x9e, /* curve_9_a */
	0x07, /* curve10_b */
	0x9e, /* curve10_a */
	0x07, /* curve11_b */
	0x9e, /* curve11_a */
	0x07, /* curve12_b */
	0x9e, /* curve12_a */
	0x0a, /* curve13_b */
	0xa0, /* curve13_a */
	0x0a, /* curve14_b */
	0xa0, /* curve14_a */
	0x0a, /* curve15_b */
	0xa0, /* curve15_a */
	0x0a, /* curve16_b */
	0xa0, /* curve16_a */
	0x16, /* curve17_b */
	0xa6, /* curve17_a */
	0x16, /* curve18_b */
	0xa6, /* curve18_a */
	0x16, /* curve19_b */
	0xa6, /* curve19_a */
	0x16, /* curve20_b */
	0xa6, /* curve20_a */
	0x05, /* curve21_b */
	0x21, /* curve21_a */
	0x0b, /* curve22_b */
	0x20, /* curve22_a */
	0x87, /* curve23_b */
	0x0f, /* curve23_a */
	0x00, /* curve24_b */
	0xFF, /* curve24_a */
	0x30, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x37, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x47, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x25,
	0x3d,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x1c,
	0xd8,
	0xff, /* ascr_skin_Rr */
	0x7c, /* ascr_skin_Rg */
	0x88, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xf4, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0xff, /* ascr_Rr */
	0xff, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0xff, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0xff, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x00, /* ascr_Mg */
	0xff, /* ascr_Gg */
	0xff, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0xff, /* ascr_Yr */
	0x00, /* ascr_Br */
	0xff, /* ascr_Yg */
	0x00, /* ascr_Bg */
	0x00, /* ascr_Yb */
	0xff, /* ascr_Bb */
	0xff, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xff, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xff, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char MOVIE_VIDEO_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x32, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char MOVIE_VIDEO_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x06, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x10,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x20,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x00, /* curve_5_b */
	0x20, /* curve_5_a */
	0x00, /* curve_6_b */
	0x20, /* curve_6_a */
	0x00, /* curve_7_b */
	0x20, /* curve_7_a */
	0x00, /* curve_8_b */
	0x20, /* curve_8_a */
	0x00, /* curve_9_b */
	0x20, /* curve_9_a */
	0x00, /* curve10_b */
	0x20, /* curve10_a */
	0x00, /* curve11_b */
	0x20, /* curve11_a */
	0x00, /* curve12_b */
	0x20, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xff, /* curve24_a */
	0x20, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x17, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x27, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x59,
	0x0b,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x34,
	0x83,
	0xff, /* ascr_skin_Rr */
	0x00, /* ascr_skin_Rg */
	0x00, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x7a, /* ascr_Cr */
	0xff, /* ascr_Rr */
	0xff, /* ascr_Cg */
	0x2b, /* ascr_Rg */
	0xf1, /* ascr_Cb */
	0x2d, /* ascr_Rb */
	0xff, /* ascr_Mr */
	0x68, /* ascr_Gr */
	0x15, /* ascr_Mg */
	0xff, /* ascr_Gg */
	0xfa, /* ascr_Mb */
	0x31, /* ascr_Gb */
	0xf8, /* ascr_Yr */
	0x34, /* ascr_Br */
	0xff, /* ascr_Yg */
	0x20, /* ascr_Bg */
	0x4b, /* ascr_Yb */
	0xff, /* ascr_Bb */
	0xff, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xf8, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xef, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char AUTO_VIDEO_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x32, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char AUTO_VIDEO_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x07, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x20,
	0x00, /* sharpen_maxplus 11 */
	0x40,
	0x00, /* sharpen_maxminus 11 */
	0x40,
	0x01, /* cs_gain 10 */
	0x20,
	0x00, /* curve_1_b */
	0x14, /* curve_1_a */
	0x00, /* curve_2_b */
	0x14, /* curve_2_a */
	0x00, /* curve_3_b */
	0x14, /* curve_3_a */
	0x00, /* curve_4_b */
	0x14, /* curve_4_a */
	0x03, /* curve_5_b */
	0x9a, /* curve_5_a */
	0x03, /* curve_6_b */
	0x9a, /* curve_6_a */
	0x03, /* curve_7_b */
	0x9a, /* curve_7_a */
	0x03, /* curve_8_b */
	0x9a, /* curve_8_a */
	0x07, /* curve_9_b */
	0x9e, /* curve_9_a */
	0x07, /* curve10_b */
	0x9e, /* curve10_a */
	0x07, /* curve11_b */
	0x9e, /* curve11_a */
	0x07, /* curve12_b */
	0x9e, /* curve12_a */
	0x0a, /* curve13_b */
	0xa0, /* curve13_a */
	0x0a, /* curve14_b */
	0xa0, /* curve14_a */
	0x0a, /* curve15_b */
	0xa0, /* curve15_a */
	0x0a, /* curve16_b */
	0xa0, /* curve16_a */
	0x16, /* curve17_b */
	0xa6, /* curve17_a */
	0x16, /* curve18_b */
	0xa6, /* curve18_a */
	0x16, /* curve19_b */
	0xa6, /* curve19_a */
	0x16, /* curve20_b */
	0xa6, /* curve20_a */
	0x05, /* curve21_b */
	0x21, /* curve21_a */
	0x0b, /* curve22_b */
	0x20, /* curve22_a */
	0x87, /* curve23_b */
	0x0f, /* curve23_a */
	0x00, /* curve24_b */
	0xFF, /* curve24_a */
	0x30, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x37, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x47, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x25,
	0x3d,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x1c,
	0xd8,
	0xff, /* ascr_skin_Rr */
	0x7c, /* ascr_skin_Rg */
	0x88, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xf4, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0xff, /* ascr_Rr */
	0xff, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0xff, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0xff, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x00, /* ascr_Mg */
	0xff, /* ascr_Gg */
	0xff, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0xff, /* ascr_Yr */
	0x00, /* ascr_Br */
	0xff, /* ascr_Yg */
	0x00, /* ascr_Bg */
	0x00, /* ascr_Yb */
	0xff, /* ascr_Bb */
	0xff, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xff, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xff, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

////////////////// VT /////////////////////
static unsigned char STANDARD_VT_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x32, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char STANDARD_VT_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x04, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x10,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x00,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x00, /* curve_5_b */
	0x20, /* curve_5_a */
	0x00, /* curve_6_b */
	0x20, /* curve_6_a */
	0x00, /* curve_7_b */
	0x20, /* curve_7_a */
	0x00, /* curve_8_b */
	0x20, /* curve_8_a */
	0x00, /* curve_9_b */
	0x20, /* curve_9_a */
	0x00, /* curve10_b */
	0x20, /* curve10_a */
	0x00, /* curve11_b */
	0x20, /* curve11_a */
	0x00, /* curve12_b */
	0x20, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xff, /* curve24_a */
	0x20, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x17, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x27, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x59,
	0x0b,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x34,
	0x83,
	0xff, /* ascr_skin_Rr */
	0x00, /* ascr_skin_Rg */
	0x00, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0xFF, /* ascr_Rr */
	0xF8, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0xF3, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0xFF, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x14, /* ascr_Mg */
	0xF4, /* ascr_Gg */
	0xE6, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0xFF, /* ascr_Yr */
	0x2D, /* ascr_Br */
	0xFA, /* ascr_Yg */
	0x1A, /* ascr_Bg */
	0x37, /* ascr_Yb */
	0xF8, /* ascr_Bb */
	0xFF, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xF8, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xEE, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char NATURAL_VT_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x32, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char NATURAL_VT_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x04, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x10,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x00,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x00, /* curve_5_b */
	0x20, /* curve_5_a */
	0x00, /* curve_6_b */
	0x20, /* curve_6_a */
	0x00, /* curve_7_b */
	0x20, /* curve_7_a */
	0x00, /* curve_8_b */
	0x20, /* curve_8_a */
	0x00, /* curve_9_b */
	0x20, /* curve_9_a */
	0x00, /* curve10_b */
	0x20, /* curve10_a */
	0x00, /* curve11_b */
	0x20, /* curve11_a */
	0x00, /* curve12_b */
	0x20, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xff, /* curve24_a */
	0x20, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x17, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x27, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x59,
	0x0b,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x34,
	0x83,
	0xff, /* ascr_skin_Rr */
	0x00, /* ascr_skin_Rg */
	0x00, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x87, /* ascr_Cr */
	0xE7, /* ascr_Rr */
	0xFF, /* ascr_Cg */
	0x23, /* ascr_Rg */
	0xF2, /* ascr_Cb */
	0x1E, /* ascr_Rb */
	0xF3, /* ascr_Mr */
	0x7D, /* ascr_Gr */
	0x2E, /* ascr_Mg */
	0xF7, /* ascr_Gg */
	0xF4, /* ascr_Mb */
	0x3C, /* ascr_Gb */
	0xFA, /* ascr_Yr */
	0x2D, /* ascr_Br */
	0xF5, /* ascr_Yg */
	0x19, /* ascr_Bg */
	0x3C, /* ascr_Yb */
	0xED, /* ascr_Bb */
	0xFF, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xF8, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xEE, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char DYNAMIC_VT_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x32, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char DYNAMIC_VT_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x07, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x20,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x20,
	0x00, /* curve_1_b */
	0x14, /* curve_1_a */
	0x00, /* curve_2_b */
	0x14, /* curve_2_a */
	0x00, /* curve_3_b */
	0x14, /* curve_3_a */
	0x00, /* curve_4_b */
	0x14, /* curve_4_a */
	0x03, /* curve_5_b */
	0x9a, /* curve_5_a */
	0x03, /* curve_6_b */
	0x9a, /* curve_6_a */
	0x03, /* curve_7_b */
	0x9a, /* curve_7_a */
	0x03, /* curve_8_b */
	0x9a, /* curve_8_a */
	0x07, /* curve_9_b */
	0x9e, /* curve_9_a */
	0x07, /* curve10_b */
	0x9e, /* curve10_a */
	0x07, /* curve11_b */
	0x9e, /* curve11_a */
	0x07, /* curve12_b */
	0x9e, /* curve12_a */
	0x0a, /* curve13_b */
	0xa0, /* curve13_a */
	0x0a, /* curve14_b */
	0xa0, /* curve14_a */
	0x0a, /* curve15_b */
	0xa0, /* curve15_a */
	0x0a, /* curve16_b */
	0xa0, /* curve16_a */
	0x16, /* curve17_b */
	0xa6, /* curve17_a */
	0x16, /* curve18_b */
	0xa6, /* curve18_a */
	0x16, /* curve19_b */
	0xa6, /* curve19_a */
	0x16, /* curve20_b */
	0xa6, /* curve20_a */
	0x05, /* curve21_b */
	0x21, /* curve21_a */
	0x0b, /* curve22_b */
	0x20, /* curve22_a */
	0x87, /* curve23_b */
	0x0f, /* curve23_a */
	0x00, /* curve24_b */
	0xFF, /* curve24_a */
	0x30, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x37, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x47, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x25,
	0x3d,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x1c,
	0xd8,
	0xff, /* ascr_skin_Rr */
	0x50, /* ascr_skin_Rg */
	0x60, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0xff, /* ascr_Rr */
	0xff, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0xff, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0xff, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x00, /* ascr_Mg */
	0xff, /* ascr_Gg */
	0xff, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0xff, /* ascr_Yr */
	0x00, /* ascr_Br */
	0xff, /* ascr_Yg */
	0x00, /* ascr_Bg */
	0x00, /* ascr_Yb */
	0xff, /* ascr_Bb */
	0xff, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xff, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xff, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char MOVIE_VT_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x32, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char MOVIE_VT_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x06, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x10,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x20,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x00, /* curve_5_b */
	0x20, /* curve_5_a */
	0x00, /* curve_6_b */
	0x20, /* curve_6_a */
	0x00, /* curve_7_b */
	0x20, /* curve_7_a */
	0x00, /* curve_8_b */
	0x20, /* curve_8_a */
	0x00, /* curve_9_b */
	0x20, /* curve_9_a */
	0x00, /* curve10_b */
	0x20, /* curve10_a */
	0x00, /* curve11_b */
	0x20, /* curve11_a */
	0x00, /* curve12_b */
	0x20, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xff, /* curve24_a */
	0x20, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x17, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x27, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x59,
	0x0b,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x34,
	0x83,
	0xff, /* ascr_skin_Rr */
	0x00, /* ascr_skin_Rg */
	0x00, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x7a, /* ascr_Cr */
	0xff, /* ascr_Rr */
	0xff, /* ascr_Cg */
	0x2b, /* ascr_Rg */
	0xf1, /* ascr_Cb */
	0x2d, /* ascr_Rb */
	0xff, /* ascr_Mr */
	0x68, /* ascr_Gr */
	0x15, /* ascr_Mg */
	0xff, /* ascr_Gg */
	0xfa, /* ascr_Mb */
	0x31, /* ascr_Gb */
	0xf8, /* ascr_Yr */
	0x34, /* ascr_Br */
	0xff, /* ascr_Yg */
	0x20, /* ascr_Bg */
	0x4b, /* ascr_Yb */
	0xff, /* ascr_Bb */
	0xff, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xf8, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xef, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char BYPASS_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x00, /* ascr_roi 1 ascr 00 1 0 */
	0x02, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char BYPASS_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x00, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x00,
	0x07, /* sharpen_maxplus 11 */
	0xff,
	0x07, /* sharpen_maxminus 11 */
	0xff,
	0x01, /* cs_gain 10 */
	0x00,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x00, /* curve_5_b */
	0x20, /* curve_5_a */
	0x00, /* curve_6_b */
	0x20, /* curve_6_a */
	0x00, /* curve_7_b */
	0x20, /* curve_7_a */
	0x00, /* curve_8_b */
	0x20, /* curve_8_a */
	0x00, /* curve_9_b */
	0x20, /* curve_9_a */
	0x00, /* curve10_b */
	0x20, /* curve10_a */
	0x00, /* curve11_b */
	0x20, /* curve11_a */
	0x00, /* curve12_b */
	0x20, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xff, /* curve24_a */
	0x20, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x0c, /* ascr_dist_up */
	0x0c, /* ascr_dist_down */
	0x0c, /* ascr_dist_right */
	0x0c, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0xaa,
	0xab,
	0x00, /* ascr_div_down */
	0xaa,
	0xab,
	0x00, /* ascr_div_right */
	0xaa,
	0xab,
	0x00, /* ascr_div_left */
	0xaa,
	0xab,
	0xff, /* ascr_skin_Rr */
	0x00, /* ascr_skin_Rg */
	0x00, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0xff, /* ascr_Rr */
	0xff, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0xff, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0xff, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x00, /* ascr_Mg */
	0xff, /* ascr_Gg */
	0xff, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0xff, /* ascr_Yr */
	0x00, /* ascr_Br */
	0xff, /* ascr_Yg */
	0x00, /* ascr_Bg */
	0x00, /* ascr_Yb */
	0xff, /* ascr_Bb */
	0xff, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xff, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xff, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char AUTO_VT_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x32, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char AUTO_VT_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x04, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x15,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x00,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x00, /* curve_5_b */
	0x20, /* curve_5_a */
	0x00, /* curve_6_b */
	0x20, /* curve_6_a */
	0x00, /* curve_7_b */
	0x20, /* curve_7_a */
	0x00, /* curve_8_b */
	0x20, /* curve_8_a */
	0x00, /* curve_9_b */
	0x20, /* curve_9_a */
	0x00, /* curve10_b */
	0x20, /* curve10_a */
	0x00, /* curve11_b */
	0x20, /* curve11_a */
	0x00, /* curve12_b */
	0x20, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xff, /* curve24_a */
	0x20, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x17, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x27, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x59,
	0x0b,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x34,
	0x83,
	0xff, /* ascr_skin_Rr */
	0x00, /* ascr_skin_Rg */
	0x00, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0xff, /* ascr_Rr */
	0xff, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0xff, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0xff, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x00, /* ascr_Mg */
	0xff, /* ascr_Gg */
	0xff, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0xff, /* ascr_Yr */
	0x00, /* ascr_Br */
	0xff, /* ascr_Yg */
	0x00, /* ascr_Bg */
	0x00, /* ascr_Yb */
	0xff, /* ascr_Bb */
	0xff, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xff, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xff, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

////////////////// CAMERA /////////////////////
static unsigned char CAMERA_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x02, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char CAMERA_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x00, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x15,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x00,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x00, /* curve_5_b */
	0x20, /* curve_5_a */
	0x00, /* curve_6_b */
	0x20, /* curve_6_a */
	0x00, /* curve_7_b */
	0x20, /* curve_7_a */
	0x00, /* curve_8_b */
	0x20, /* curve_8_a */
	0x00, /* curve_9_b */
	0x20, /* curve_9_a */
	0x00, /* curve10_b */
	0x20, /* curve10_a */
	0x00, /* curve11_b */
	0x20, /* curve11_a */
	0x00, /* curve12_b */
	0x20, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xff, /* curve24_a */
	0x20, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x17, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x27, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x59,
	0x0b,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x34,
	0x83,
	0xff, /* ascr_skin_Rr */
	0x00, /* ascr_skin_Rg */
	0x00, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0xff, /* ascr_Rr */
	0xff, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0xff, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0xff, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x00, /* ascr_Mg */
	0xff, /* ascr_Gg */
	0xff, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0xff, /* ascr_Yr */
	0x00, /* ascr_Br */
	0xff, /* ascr_Yg */
	0x00, /* ascr_Bg */
	0x00, /* ascr_Yb */
	0xff, /* ascr_Bb */
	0xff, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xff, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xff, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char AUTO_CAMERA_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x02, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char AUTO_CAMERA_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x00, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x15,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x00,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x00, /* curve_5_b */
	0x20, /* curve_5_a */
	0x00, /* curve_6_b */
	0x20, /* curve_6_a */
	0x00, /* curve_7_b */
	0x20, /* curve_7_a */
	0x00, /* curve_8_b */
	0x20, /* curve_8_a */
	0x00, /* curve_9_b */
	0x20, /* curve_9_a */
	0x00, /* curve10_b */
	0x20, /* curve10_a */
	0x00, /* curve11_b */
	0x20, /* curve11_a */
	0x00, /* curve12_b */
	0x20, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xff, /* curve24_a */
	0x30, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x17, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x27, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x59,
	0x0b,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x34,
	0x83,
	0xff, /* ascr_skin_Rr */
	0x70, /* ascr_skin_Rg */
	0x80, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xf8, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0xff, /* ascr_Rr */
	0xff, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0xff, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0xff, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x00, /* ascr_Mg */
	0xff, /* ascr_Gg */
	0xff, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0xff, /* ascr_Yr */
	0x00, /* ascr_Br */
	0xff, /* ascr_Yg */
	0x00, /* ascr_Bg */
	0x00, /* ascr_Yb */
	0xff, /* ascr_Bb */
	0xff, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xff, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xff, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

#if 0
static unsigned char CAMERA_OUTDOOR_2[] = {
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x32, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char CAMERA_OUTDOOR_1[] = {
	0xEC,
	0x18, /* lce_gain 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x07, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x10,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x00,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x02, /* curve_5_b */
	0x1b, /* curve_5_a */
	0x02, /* curve_6_b */
	0x1b, /* curve_6_a */
	0x02, /* curve_7_b */
	0x1b, /* curve_7_a */
	0x02, /* curve_8_b */
	0x1b, /* curve_8_a */
	0x09, /* curve_9_b */
	0xa6, /* curve_9_a */
	0x09, /* curve10_b */
	0xa6, /* curve10_a */
	0x09, /* curve11_b */
	0xa6, /* curve11_a */
	0x09, /* curve12_b */
	0xa6, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xFF, /* curve24_a */
	0x2c, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x4e, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x5f, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x1a,
	0x74,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x15,
	0x8f,
	0xff, /* ascr_skin_Rr */
	0x20, /* ascr_skin_Rg */
	0x20, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0xff, /* ascr_Rr */
	0xff, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0xff, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0xff, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x00, /* ascr_Mg */
	0xff, /* ascr_Gg */
	0xff, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0xff, /* ascr_Yr */
	0x00, /* ascr_Br */
	0xff, /* ascr_Yg */
	0x00, /* ascr_Bg */
	0x00, /* ascr_Yb */
	0xff, /* ascr_Bb */
	0xfc, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xff, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xfd, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char COLD_2[] = {
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x32, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char COLD_1[] = {
	0xEC,
	0x18, /* lce_gain 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x07, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x10,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x00,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x02, /* curve_5_b */
	0x1b, /* curve_5_a */
	0x02, /* curve_6_b */
	0x1b, /* curve_6_a */
	0x02, /* curve_7_b */
	0x1b, /* curve_7_a */
	0x02, /* curve_8_b */
	0x1b, /* curve_8_a */
	0x09, /* curve_9_b */
	0xa6, /* curve_9_a */
	0x09, /* curve10_b */
	0xa6, /* curve10_a */
	0x09, /* curve11_b */
	0xa6, /* curve11_a */
	0x09, /* curve12_b */
	0xa6, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xFF, /* curve24_a */
	0x2c, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x4e, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x5f, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x1a,
	0x74,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x15,
	0x8f,
	0xff, /* ascr_skin_Rr */
	0x20, /* ascr_skin_Rg */
	0x20, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0xff, /* ascr_Rr */
	0xff, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0xff, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0xff, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x00, /* ascr_Mg */
	0xff, /* ascr_Gg */
	0xff, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0xff, /* ascr_Yr */
	0x00, /* ascr_Br */
	0xff, /* ascr_Yg */
	0x00, /* ascr_Bg */
	0x00, /* ascr_Yb */
	0xff, /* ascr_Bb */
	0xfc, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xff, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xfd, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char COLD_OUTDOOR_2[] = {
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x32, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char COLD_OUTDOOR_1[] = {
	0xEC,
	0x18, /* lce_gain 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x07, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x10,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x00,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x02, /* curve_5_b */
	0x1b, /* curve_5_a */
	0x02, /* curve_6_b */
	0x1b, /* curve_6_a */
	0x02, /* curve_7_b */
	0x1b, /* curve_7_a */
	0x02, /* curve_8_b */
	0x1b, /* curve_8_a */
	0x09, /* curve_9_b */
	0xa6, /* curve_9_a */
	0x09, /* curve10_b */
	0xa6, /* curve10_a */
	0x09, /* curve11_b */
	0xa6, /* curve11_a */
	0x09, /* curve12_b */
	0xa6, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xFF, /* curve24_a */
	0x2c, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x4e, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x5f, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x1a,
	0x74,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x15,
	0x8f,
	0xff, /* ascr_skin_Rr */
	0x20, /* ascr_skin_Rg */
	0x20, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0xff, /* ascr_Rr */
	0xff, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0xff, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0xff, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x00, /* ascr_Mg */
	0xff, /* ascr_Gg */
	0xff, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0xff, /* ascr_Yr */
	0x00, /* ascr_Br */
	0xff, /* ascr_Yg */
	0x00, /* ascr_Bg */
	0x00, /* ascr_Yb */
	0xff, /* ascr_Bb */
	0xfc, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xff, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xfd, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char WARM_2[] = {
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x32, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char WARM_1[] = {
	0xEC,
	0x18, /* lce_gain 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x07, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x10,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x00,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x02, /* curve_5_b */
	0x1b, /* curve_5_a */
	0x02, /* curve_6_b */
	0x1b, /* curve_6_a */
	0x02, /* curve_7_b */
	0x1b, /* curve_7_a */
	0x02, /* curve_8_b */
	0x1b, /* curve_8_a */
	0x09, /* curve_9_b */
	0xa6, /* curve_9_a */
	0x09, /* curve10_b */
	0xa6, /* curve10_a */
	0x09, /* curve11_b */
	0xa6, /* curve11_a */
	0x09, /* curve12_b */
	0xa6, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xFF, /* curve24_a */
	0x2c, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x4e, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x5f, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x1a,
	0x74,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x15,
	0x8f,
	0xff, /* ascr_skin_Rr */
	0x20, /* ascr_skin_Rg */
	0x20, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0xff, /* ascr_Rr */
	0xff, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0xff, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0xff, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x00, /* ascr_Mg */
	0xff, /* ascr_Gg */
	0xff, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0xff, /* ascr_Yr */
	0x00, /* ascr_Br */
	0xff, /* ascr_Yg */
	0x00, /* ascr_Bg */
	0x00, /* ascr_Yb */
	0xff, /* ascr_Bb */
	0xfc, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xff, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xfd, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char WARM_OUTDOOR_2[] = {
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x32, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char WARM_OUTDOOR_1[] = {
	0xEC,
	0x18, /* lce_gain 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x07, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x10,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x00,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x02, /* curve_5_b */
	0x1b, /* curve_5_a */
	0x02, /* curve_6_b */
	0x1b, /* curve_6_a */
	0x02, /* curve_7_b */
	0x1b, /* curve_7_a */
	0x02, /* curve_8_b */
	0x1b, /* curve_8_a */
	0x09, /* curve_9_b */
	0xa6, /* curve_9_a */
	0x09, /* curve10_b */
	0xa6, /* curve10_a */
	0x09, /* curve11_b */
	0xa6, /* curve11_a */
	0x09, /* curve12_b */
	0xa6, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xFF, /* curve24_a */
	0x2c, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x4e, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x5f, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x1a,
	0x74,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x15,
	0x8f,
	0xff, /* ascr_skin_Rr */
	0x20, /* ascr_skin_Rg */
	0x20, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0xff, /* ascr_Rr */
	0xff, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0xff, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0xff, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x00, /* ascr_Mg */
	0xff, /* ascr_Gg */
	0xff, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0xff, /* ascr_Yr */
	0x00, /* ascr_Br */
	0xff, /* ascr_Yg */
	0x00, /* ascr_Bg */
	0x00, /* ascr_Yb */
	0xff, /* ascr_Bb */
	0xfc, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xff, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xfd, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};
#endif

static unsigned char NEGATIVE_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x02, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char NEGATIVE_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x00, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x00,
	0x07, /* sharpen_maxplus 11 */
	0xff,
	0x07, /* sharpen_maxminus 11 */
	0xff,
	0x01, /* cs_gain 10 */
	0x00,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x00, /* curve_5_b */
	0x20, /* curve_5_a */
	0x00, /* curve_6_b */
	0x20, /* curve_6_a */
	0x00, /* curve_7_b */
	0x20, /* curve_7_a */
	0x00, /* curve_8_b */
	0x20, /* curve_8_a */
	0x00, /* curve_9_b */
	0x20, /* curve_9_a */
	0x00, /* curve10_b */
	0x20, /* curve10_a */
	0x00, /* curve11_b */
	0x20, /* curve11_a */
	0x00, /* curve12_b */
	0x20, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xff, /* curve24_a */
	0x20, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x0c, /* ascr_dist_up */
	0x0c, /* ascr_dist_down */
	0x0c, /* ascr_dist_right */
	0x0c, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0xaa,
	0xab,
	0x00, /* ascr_div_down */
	0xaa,
	0xab,
	0x00, /* ascr_div_right */
	0xaa,
	0xab,
	0x00, /* ascr_div_left */
	0xaa,
	0xab,
	0x00, /* ascr_skin_Rr */
	0xff, /* ascr_skin_Rg */
	0xff, /* ascr_skin_Rb */
	0x00, /* ascr_skin_Yr */
	0x00, /* ascr_skin_Yg */
	0xff, /* ascr_skin_Yb */
	0x00, /* ascr_skin_Mr */
	0xff, /* ascr_skin_Mg */
	0x00, /* ascr_skin_Mb */
	0x00, /* ascr_skin_Wr */
	0x00, /* ascr_skin_Wg */
	0x00, /* ascr_skin_Wb */
	0xff, /* ascr_Cr */
	0x00, /* ascr_Rr */
	0x00, /* ascr_Cg */
	0xff, /* ascr_Rg */
	0x00, /* ascr_Cb */
	0xff, /* ascr_Rb */
	0x00, /* ascr_Mr */
	0xff, /* ascr_Gr */
	0xff, /* ascr_Mg */
	0x00, /* ascr_Gg */
	0x00, /* ascr_Mb */
	0xff, /* ascr_Gb */
	0x00, /* ascr_Yr */
	0xff, /* ascr_Br */
	0x00, /* ascr_Yg */
	0xff, /* ascr_Bg */
	0xff, /* ascr_Yb */
	0x00, /* ascr_Bb */
	0x00, /* ascr_Wr */
	0xff, /* ascr_Kr */
	0x00, /* ascr_Wg */
	0xff, /* ascr_Kg */
	0x00, /* ascr_Wb */
	0xff, /* ascr_Kb */
	/* end */
};

#if 0
static unsigned char OUTDOOR_VIDEO_2[] = {
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x32, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char OUTDOOR_VIDEO_1[] = {
	0xEC,
	0x18, /* lce_gain 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x07, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x10,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x00,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x02, /* curve_5_b */
	0x1b, /* curve_5_a */
	0x02, /* curve_6_b */
	0x1b, /* curve_6_a */
	0x02, /* curve_7_b */
	0x1b, /* curve_7_a */
	0x02, /* curve_8_b */
	0x1b, /* curve_8_a */
	0x09, /* curve_9_b */
	0xa6, /* curve_9_a */
	0x09, /* curve10_b */
	0xa6, /* curve10_a */
	0x09, /* curve11_b */
	0xa6, /* curve11_a */
	0x09, /* curve12_b */
	0xa6, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xFF, /* curve24_a */
	0x2c, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x4e, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x5f, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x1a,
	0x74,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x15,
	0x8f,
	0xff, /* ascr_skin_Rr */
	0x20, /* ascr_skin_Rg */
	0x20, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0xff, /* ascr_Rr */
	0xff, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0xff, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0xff, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x00, /* ascr_Mg */
	0xff, /* ascr_Gg */
	0xff, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0xff, /* ascr_Yr */
	0x00, /* ascr_Br */
	0xff, /* ascr_Yg */
	0x00, /* ascr_Bg */
	0x00, /* ascr_Yb */
	0xff, /* ascr_Bb */
	0xfc, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xff, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xfd, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};
#endif

static unsigned char COLOR_BLIND_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x02, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char COLOR_BLIND_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x00, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x00,
	0x07, /* sharpen_maxplus 11 */
	0xff,
	0x07, /* sharpen_maxminus 11 */
	0xff,
	0x01, /* cs_gain 10 */
	0x00,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x00, /* curve_5_b */
	0x20, /* curve_5_a */
	0x00, /* curve_6_b */
	0x20, /* curve_6_a */
	0x00, /* curve_7_b */
	0x20, /* curve_7_a */
	0x00, /* curve_8_b */
	0x20, /* curve_8_a */
	0x00, /* curve_9_b */
	0x20, /* curve_9_a */
	0x00, /* curve10_b */
	0x20, /* curve10_a */
	0x00, /* curve11_b */
	0x20, /* curve11_a */
	0x00, /* curve12_b */
	0x20, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xff, /* curve24_a */
	0x20, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x0c, /* ascr_dist_up */
	0x0c, /* ascr_dist_down */
	0x0c, /* ascr_dist_right */
	0x0c, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0xaa,
	0xab,
	0x00, /* ascr_div_down */
	0xaa,
	0xab,
	0x00, /* ascr_div_right */
	0xaa,
	0xab,
	0x00, /* ascr_div_left */
	0xaa,
	0xab,
	0xff, /* ascr_skin_Rr */
	0x00, /* ascr_skin_Rg */
	0x00, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0xff, /* ascr_Rr */
	0xff, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0xff, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0xff, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x00, /* ascr_Mg */
	0xff, /* ascr_Gg */
	0xff, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0xff, /* ascr_Yr */
	0x00, /* ascr_Br */
	0xff, /* ascr_Yg */
	0x00, /* ascr_Bg */
	0x00, /* ascr_Yb */
	0xff, /* ascr_Bb */
	0xff, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xff, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xff, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

////////////////// BROWSER /////////////////////
static unsigned char STANDARD_BROWSER_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x02, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char STANDARD_BROWSER_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x00, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x10,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x00,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x00, /* curve_5_b */
	0x20, /* curve_5_a */
	0x00, /* curve_6_b */
	0x20, /* curve_6_a */
	0x00, /* curve_7_b */
	0x20, /* curve_7_a */
	0x00, /* curve_8_b */
	0x20, /* curve_8_a */
	0x00, /* curve_9_b */
	0x20, /* curve_9_a */
	0x00, /* curve10_b */
	0x20, /* curve10_a */
	0x00, /* curve11_b */
	0x20, /* curve11_a */
	0x00, /* curve12_b */
	0x20, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xff, /* curve24_a */
	0x20, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x17, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x27, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x59,
	0x0b,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x34,
	0x83,
	0xff, /* ascr_skin_Rr */
	0x00, /* ascr_skin_Rg */
	0x00, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0xFF, /* ascr_Rr */
	0xF8, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0xF3, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0xFF, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x14, /* ascr_Mg */
	0xF4, /* ascr_Gg */
	0xE6, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0xFF, /* ascr_Yr */
	0x2D, /* ascr_Br */
	0xFA, /* ascr_Yg */
	0x1A, /* ascr_Bg */
	0x37, /* ascr_Yb */
	0xF8, /* ascr_Bb */
	0xFF, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xF8, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xEE, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char NATURAL_BROWSER_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x32, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char NATURAL_BROWSER_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x00, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x10,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x00,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x00, /* curve_5_b */
	0x20, /* curve_5_a */
	0x00, /* curve_6_b */
	0x20, /* curve_6_a */
	0x00, /* curve_7_b */
	0x20, /* curve_7_a */
	0x00, /* curve_8_b */
	0x20, /* curve_8_a */
	0x00, /* curve_9_b */
	0x20, /* curve_9_a */
	0x00, /* curve10_b */
	0x20, /* curve10_a */
	0x00, /* curve11_b */
	0x20, /* curve11_a */
	0x00, /* curve12_b */
	0x20, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xff, /* curve24_a */
	0x20, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x17, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x27, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x59,
	0x0b,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x34,
	0x83,
	0xff, /* ascr_skin_Rr */
	0x00, /* ascr_skin_Rg */
	0x00, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x87, /* ascr_Cr */
	0xE7, /* ascr_Rr */
	0xFF, /* ascr_Cg */
	0x23, /* ascr_Rg */
	0xF2, /* ascr_Cb */
	0x1E, /* ascr_Rb */
	0xF3, /* ascr_Mr */
	0x7D, /* ascr_Gr */
	0x2E, /* ascr_Mg */
	0xF7, /* ascr_Gg */
	0xF4, /* ascr_Mb */
	0x3C, /* ascr_Gb */
	0xFA, /* ascr_Yr */
	0x2D, /* ascr_Br */
	0xF5, /* ascr_Yg */
	0x19, /* ascr_Bg */
	0x3C, /* ascr_Yb */
	0xED, /* ascr_Bb */
	0xFF, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xF8, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xEE, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char DYNAMIC_BROWSER_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x32, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char DYNAMIC_BROWSER_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x03, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x20,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x20,
	0x00, /* curve_1_b */
	0x14, /* curve_1_a */
	0x00, /* curve_2_b */
	0x14, /* curve_2_a */
	0x00, /* curve_3_b */
	0x14, /* curve_3_a */
	0x00, /* curve_4_b */
	0x14, /* curve_4_a */
	0x03, /* curve_5_b */
	0x9a, /* curve_5_a */
	0x03, /* curve_6_b */
	0x9a, /* curve_6_a */
	0x03, /* curve_7_b */
	0x9a, /* curve_7_a */
	0x03, /* curve_8_b */
	0x9a, /* curve_8_a */
	0x07, /* curve_9_b */
	0x9e, /* curve_9_a */
	0x07, /* curve10_b */
	0x9e, /* curve10_a */
	0x07, /* curve11_b */
	0x9e, /* curve11_a */
	0x07, /* curve12_b */
	0x9e, /* curve12_a */
	0x0a, /* curve13_b */
	0xa0, /* curve13_a */
	0x0a, /* curve14_b */
	0xa0, /* curve14_a */
	0x0a, /* curve15_b */
	0xa0, /* curve15_a */
	0x0a, /* curve16_b */
	0xa0, /* curve16_a */
	0x16, /* curve17_b */
	0xa6, /* curve17_a */
	0x16, /* curve18_b */
	0xa6, /* curve18_a */
	0x16, /* curve19_b */
	0xa6, /* curve19_a */
	0x16, /* curve20_b */
	0xa6, /* curve20_a */
	0x05, /* curve21_b */
	0x21, /* curve21_a */
	0x0b, /* curve22_b */
	0x20, /* curve22_a */
	0x87, /* curve23_b */
	0x0f, /* curve23_a */
	0x00, /* curve24_b */
	0xFF, /* curve24_a */
	0x30, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x37, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x47, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x25,
	0x3d,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x1c,
	0xd8,
	0xff, /* ascr_skin_Rr */
	0x50, /* ascr_skin_Rg */
	0x60, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0xff, /* ascr_Rr */
	0xff, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0xff, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0xff, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x00, /* ascr_Mg */
	0xff, /* ascr_Gg */
	0xff, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0xff, /* ascr_Yr */
	0x00, /* ascr_Br */
	0xff, /* ascr_Yg */
	0x00, /* ascr_Bg */
	0x00, /* ascr_Yb */
	0xff, /* ascr_Bb */
	0xff, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xff, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xff, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char MOVIE_BROWSER_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x32, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char MOVIE_BROWSER_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x02, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x10,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x20,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x00, /* curve_5_b */
	0x20, /* curve_5_a */
	0x00, /* curve_6_b */
	0x20, /* curve_6_a */
	0x00, /* curve_7_b */
	0x20, /* curve_7_a */
	0x00, /* curve_8_b */
	0x20, /* curve_8_a */
	0x00, /* curve_9_b */
	0x20, /* curve_9_a */
	0x00, /* curve10_b */
	0x20, /* curve10_a */
	0x00, /* curve11_b */
	0x20, /* curve11_a */
	0x00, /* curve12_b */
	0x20, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xff, /* curve24_a */
	0x20, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x17, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x27, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x59,
	0x0b,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x34,
	0x83,
	0xff, /* ascr_skin_Rr */
	0x00, /* ascr_skin_Rg */
	0x00, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x7a, /* ascr_Cr */
	0xff, /* ascr_Rr */
	0xff, /* ascr_Cg */
	0x2b, /* ascr_Rg */
	0xf1, /* ascr_Cb */
	0x2d, /* ascr_Rb */
	0xff, /* ascr_Mr */
	0x68, /* ascr_Gr */
	0x15, /* ascr_Mg */
	0xff, /* ascr_Gg */
	0xfa, /* ascr_Mb */
	0x31, /* ascr_Gb */
	0xf8, /* ascr_Yr */
	0x34, /* ascr_Br */
	0xff, /* ascr_Yg */
	0x20, /* ascr_Bg */
	0x4b, /* ascr_Yb */
	0xff, /* ascr_Bb */
	0xff, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xf8, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xef, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char AUTO_BROWSER_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x02, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char AUTO_BROWSER_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x00, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x15,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x00,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x00, /* curve_5_b */
	0x20, /* curve_5_a */
	0x00, /* curve_6_b */
	0x20, /* curve_6_a */
	0x00, /* curve_7_b */
	0x20, /* curve_7_a */
	0x00, /* curve_8_b */
	0x20, /* curve_8_a */
	0x00, /* curve_9_b */
	0x20, /* curve_9_a */
	0x00, /* curve10_b */
	0x20, /* curve10_a */
	0x00, /* curve11_b */
	0x20, /* curve11_a */
	0x00, /* curve12_b */
	0x20, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xff, /* curve24_a */
	0x30, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x17, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x27, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x59,
	0x0b,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x34,
	0x83,
	0xff, /* ascr_skin_Rr */
	0x70, /* ascr_skin_Rg */
	0x80, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xf8, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0xff, /* ascr_Rr */
	0xff, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0xff, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0xff, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x00, /* ascr_Mg */
	0xff, /* ascr_Gg */
	0xff, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0xff, /* ascr_Yr */
	0x00, /* ascr_Br */
	0xff, /* ascr_Yg */
	0x00, /* ascr_Bg */
	0x00, /* ascr_Yb */
	0xff, /* ascr_Bb */
	0xff, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xff, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xff, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

////////////////// eBOOK /////////////////////
static unsigned char DYNAMIC_EBOOK_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x02, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char DYNAMIC_EBOOK_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x00, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x15,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x00,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x00, /* curve_5_b */
	0x20, /* curve_5_a */
	0x00, /* curve_6_b */
	0x20, /* curve_6_a */
	0x00, /* curve_7_b */
	0x20, /* curve_7_a */
	0x00, /* curve_8_b */
	0x20, /* curve_8_a */
	0x00, /* curve_9_b */
	0x20, /* curve_9_a */
	0x00, /* curve10_b */
	0x20, /* curve10_a */
	0x00, /* curve11_b */
	0x20, /* curve11_a */
	0x00, /* curve12_b */
	0x20, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xff, /* curve24_a */
	0x20, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x17, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x27, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x59,
	0x0b,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x34,
	0x83,
	0xff, /* ascr_skin_Rr */
	0x00, /* ascr_skin_Rg */
	0x00, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0xff, /* ascr_Rr */
	0xff, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0xff, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0xff, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x00, /* ascr_Mg */
	0xff, /* ascr_Gg */
	0xff, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0xff, /* ascr_Yr */
	0x00, /* ascr_Br */
	0xff, /* ascr_Yg */
	0x00, /* ascr_Bg */
	0x00, /* ascr_Yb */
	0xff, /* ascr_Bb */
	0xff, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xf6, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xea, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char STANDARD_EBOOK_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x02, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char STANDARD_EBOOK_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x00, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x15,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x00,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x00, /* curve_5_b */
	0x20, /* curve_5_a */
	0x00, /* curve_6_b */
	0x20, /* curve_6_a */
	0x00, /* curve_7_b */
	0x20, /* curve_7_a */
	0x00, /* curve_8_b */
	0x20, /* curve_8_a */
	0x00, /* curve_9_b */
	0x20, /* curve_9_a */
	0x00, /* curve10_b */
	0x20, /* curve10_a */
	0x00, /* curve11_b */
	0x20, /* curve11_a */
	0x00, /* curve12_b */
	0x20, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xff, /* curve24_a */
	0x20, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x17, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x27, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x59,
	0x0b,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x34,
	0x83,
	0xff, /* ascr_skin_Rr */
	0x00, /* ascr_skin_Rg */
	0x00, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0xff, /* ascr_Rr */
	0xff, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0xff, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0xff, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x00, /* ascr_Mg */
	0xff, /* ascr_Gg */
	0xff, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0xff, /* ascr_Yr */
	0x00, /* ascr_Br */
	0xff, /* ascr_Yg */
	0x00, /* ascr_Bg */
	0x00, /* ascr_Yb */
	0xff, /* ascr_Bb */
	0xff, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xf6, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xea, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char NATURAL_EBOOK_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x02, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char NATURAL_EBOOK_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x00, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x15,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x00,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x00, /* curve_5_b */
	0x20, /* curve_5_a */
	0x00, /* curve_6_b */
	0x20, /* curve_6_a */
	0x00, /* curve_7_b */
	0x20, /* curve_7_a */
	0x00, /* curve_8_b */
	0x20, /* curve_8_a */
	0x00, /* curve_9_b */
	0x20, /* curve_9_a */
	0x00, /* curve10_b */
	0x20, /* curve10_a */
	0x00, /* curve11_b */
	0x20, /* curve11_a */
	0x00, /* curve12_b */
	0x20, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xff, /* curve24_a */
	0x20, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x17, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x27, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x59,
	0x0b,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x34,
	0x83,
	0xff, /* ascr_skin_Rr */
	0x00, /* ascr_skin_Rg */
	0x00, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0xff, /* ascr_Rr */
	0xff, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0xff, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0xff, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x00, /* ascr_Mg */
	0xff, /* ascr_Gg */
	0xff, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0xff, /* ascr_Yr */
	0x00, /* ascr_Br */
	0xff, /* ascr_Yg */
	0x00, /* ascr_Bg */
	0x00, /* ascr_Yb */
	0xff, /* ascr_Bb */
	0xff, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xf6, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xea, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char MOVIE_EBOOK_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x02, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char MOVIE_EBOOK_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x00, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x15,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x00,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x00, /* curve_5_b */
	0x20, /* curve_5_a */
	0x00, /* curve_6_b */
	0x20, /* curve_6_a */
	0x00, /* curve_7_b */
	0x20, /* curve_7_a */
	0x00, /* curve_8_b */
	0x20, /* curve_8_a */
	0x00, /* curve_9_b */
	0x20, /* curve_9_a */
	0x00, /* curve10_b */
	0x20, /* curve10_a */
	0x00, /* curve11_b */
	0x20, /* curve11_a */
	0x00, /* curve12_b */
	0x20, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xff, /* curve24_a */
	0x20, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x17, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x27, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x59,
	0x0b,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x34,
	0x83,
	0xff, /* ascr_skin_Rr */
	0x00, /* ascr_skin_Rg */
	0x00, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0xff, /* ascr_Rr */
	0xff, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0xff, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0xff, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x00, /* ascr_Mg */
	0xff, /* ascr_Gg */
	0xff, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0xff, /* ascr_Yr */
	0x00, /* ascr_Br */
	0xff, /* ascr_Yg */
	0x00, /* ascr_Bg */
	0x00, /* ascr_Yb */
	0xff, /* ascr_Bb */
	0xff, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xf6, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xea, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char AUTO_EBOOK_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x02, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char AUTO_EBOOK_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x00, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x15,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x00,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x00, /* curve_5_b */
	0x20, /* curve_5_a */
	0x00, /* curve_6_b */
	0x20, /* curve_6_a */
	0x00, /* curve_7_b */
	0x20, /* curve_7_a */
	0x00, /* curve_8_b */
	0x20, /* curve_8_a */
	0x00, /* curve_9_b */
	0x20, /* curve_9_a */
	0x00, /* curve10_b */
	0x20, /* curve10_a */
	0x00, /* curve11_b */
	0x20, /* curve11_a */
	0x00, /* curve12_b */
	0x20, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xff, /* curve24_a */
	0x20, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x17, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x27, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x59,
	0x0b,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x34,
	0x83,
	0xff, /* ascr_skin_Rr */
	0x00, /* ascr_skin_Rg */
	0x00, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0xff, /* ascr_Rr */
	0xff, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0xff, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0xff, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x00, /* ascr_Mg */
	0xff, /* ascr_Gg */
	0xff, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0xff, /* ascr_Yr */
	0x00, /* ascr_Br */
	0xff, /* ascr_Yg */
	0x00, /* ascr_Bg */
	0x00, /* ascr_Yb */
	0xff, /* ascr_Bb */
	0xff, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xf6, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xea, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char AUTO_EMAIL_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x02, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char AUTO_EMAIL_1[] = {
	0xEC,
	0x18, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x24, /* lce_color_gain 00 0000 */
	0x10, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0xb3, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0x0e,
	0x01, /* lce_ref_gain 9 */
	0x00,
	0x66, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x2d, /* lce_bin_size_ratio */
	0x03, /* lce_dark_th 000 */
	0x96, /* lce_min_ref_offset */
	0x00, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x15,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x00,
	0x00, /* curve_1_b */
	0x20, /* curve_1_a */
	0x00, /* curve_2_b */
	0x20, /* curve_2_a */
	0x00, /* curve_3_b */
	0x20, /* curve_3_a */
	0x00, /* curve_4_b */
	0x20, /* curve_4_a */
	0x00, /* curve_5_b */
	0x20, /* curve_5_a */
	0x00, /* curve_6_b */
	0x20, /* curve_6_a */
	0x00, /* curve_7_b */
	0x20, /* curve_7_a */
	0x00, /* curve_8_b */
	0x20, /* curve_8_a */
	0x00, /* curve_9_b */
	0x20, /* curve_9_a */
	0x00, /* curve10_b */
	0x20, /* curve10_a */
	0x00, /* curve11_b */
	0x20, /* curve11_a */
	0x00, /* curve12_b */
	0x20, /* curve12_a */
	0x00, /* curve13_b */
	0x20, /* curve13_a */
	0x00, /* curve14_b */
	0x20, /* curve14_a */
	0x00, /* curve15_b */
	0x20, /* curve15_a */
	0x00, /* curve16_b */
	0x20, /* curve16_a */
	0x00, /* curve17_b */
	0x20, /* curve17_a */
	0x00, /* curve18_b */
	0x20, /* curve18_a */
	0x00, /* curve19_b */
	0x20, /* curve19_a */
	0x00, /* curve20_b */
	0x20, /* curve20_a */
	0x00, /* curve21_b */
	0x20, /* curve21_a */
	0x00, /* curve22_b */
	0x20, /* curve22_a */
	0x00, /* curve23_b */
	0x20, /* curve23_a */
	0x00, /* curve24_b */
	0xff, /* curve24_a */
	0x20, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x17, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x27, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x59,
	0x0b,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x34,
	0x83,
	0xff, /* ascr_skin_Rr */
	0x00, /* ascr_skin_Rg */
	0x00, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0xff, /* ascr_Rr */
	0xff, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0xff, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0xff, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x00, /* ascr_Mg */
	0xff, /* ascr_Gg */
	0xff, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0xff, /* ascr_Yr */
	0x00, /* ascr_Br */
	0xff, /* ascr_Yg */
	0x00, /* ascr_Bg */
	0x00, /* ascr_Yb */
	0xff, /* ascr_Bb */
	0xff, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xfa, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xef, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char LOCAL_CE_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x33, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char LOCAL_CE_1[] = {
	0xEC,
	0x85, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x30, /* lce_color_gain 00 0000 */
	0x00, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0x90, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0xbf,
	0x00, /* lce_ref_gain 9 */
	0xb0,
	0x77, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x7f, /* lce_bin_size_ratio */
	0x00, /* lce_dark_th 000 */
	0x40, /* lce_min_ref_offset */
	0x05, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x40,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 10 */
	0x40,
	0x00, /* curve_1_b */
	0x6b, /* curve_1_a */
	0x03, /* curve_2_b */
	0x48, /* curve_2_a */
	0x08, /* curve_3_b */
	0x32, /* curve_3_a */
	0x08, /* curve_4_b */
	0x32, /* curve_4_a */
	0x08, /* curve_5_b */
	0x32, /* curve_5_a */
	0x08, /* curve_6_b */
	0x32, /* curve_6_a */
	0x08, /* curve_7_b */
	0x32, /* curve_7_a */
	0x10, /* curve_8_b */
	0x28, /* curve_8_a */
	0x10, /* curve_9_b */
	0x28, /* curve_9_a */
	0x10, /* curve10_b */
	0x28, /* curve10_a */
	0x10, /* curve11_b */
	0x28, /* curve11_a */
	0x10, /* curve12_b */
	0x28, /* curve12_a */
	0x19, /* curve13_b */
	0x22, /* curve13_a */
	0x49, /* curve14_b */
	0xdf, /* curve14_a */
	0x49, /* curve15_b */
	0xdf, /* curve15_a */
	0x49, /* curve16_b */
	0xdf, /* curve16_a */
	0x49, /* curve17_b */
	0xdf, /* curve17_a */
	0x50, /* curve18_b */
	0x1c, /* curve18_a */
	0x5b, /* curve19_b */
	0x18, /* curve19_a */
	0x6a, /* curve20_b */
	0x14, /* curve20_a */
	0x7a, /* curve21_b */
	0x11, /* curve21_a */
	0x87, /* curve22_b */
	0x0f, /* curve22_a */
	0x87, /* curve23_b */
	0x0f, /* curve23_a */
	0x00, /* curve24_b */
	0xff, /* curve24_a */
	0x30, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x17, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x27, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x59,
	0x0b,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x34,
	0x83,
	0xff, /* ascr_skin_Rr */
	0x50, /* ascr_skin_Rg */
	0x60, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0xff, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0xff, /* ascr_Rr */
	0xff, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0xff, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0xff, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x00, /* ascr_Mg */
	0xff, /* ascr_Gg */
	0xff, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0xff, /* ascr_Yr */
	0x00, /* ascr_Br */
	0xff, /* ascr_Yg */
	0x00, /* ascr_Bg */
	0x00, /* ascr_Yb */
	0xff, /* ascr_Bb */
	0xff, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xf8, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xff, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char LOCAL_CE_TEXT_2[] = {
	/* start */
	0xEB,
	0x01, /* mdnie_en */
	0x00, /* data_width mask 00 0000 */
	0x03, /* ascr_roi 1 ascr 00 1 0 */
	0x33, /* algo_roi 1 algo lce_roi 1 lce 00 1 0 00 1 0 */
	0x00, /* roi_ctrl 00 */
	0x00, /* roi0_x_start 12 */
	0x00,
	0x00, /* roi0_x_end */
	0x00,
	0x00, /* roi0_y_start */
	0x00,
	0x00, /* roi0_y_end */
	0x00,
	0x00, /* roi1_x_strat */
	0x00,
	0x00, /* roi1_x_end */
	0x00,
	0x00, /* roi1_y_start */
	0x00,
	0x00, /* roi1_y_end */
	0x00,
};

static unsigned char LOCAL_CE_TEXT_1[] = {
	0xEC,
	0x85, /* lce_on 0 lce_gain 0 0 00 0000 */
	0x30, /* lce_color_gain 00 0000 */
	0x00, /* lce_scene_change_on scene_trans 0 0000 */
	0x14, /* lce_min_diff */
	0x90, /* lce_illum_gain */
	0x01, /* lce_ref_offset 9 */
	0xbf,
	0x00, /* lce_ref_gain 9 */
	0xb0,
	0x77, /* lce_block_size h v 0000 0000 */
	0xfa, /* lce_bright_th */
	0x7f, /* lce_bin_size_ratio */
	0x00, /* lce_dark_th 000 */
	0x40, /* lce_min_ref_offset */
	0x06, /* nr sharp cs gamma 0000 */
	0xff, /* nr_mask_th */
	0x00, /* sharpen_weight 10 */
	0x40,
	0x00, /* sharpen_maxplus 11 */
	0xa0,
	0x00, /* sharpen_maxminus 11 */
	0xa0,
	0x01, /* cs_gain 01 */
	0x40,
	0x00, /* curve_1_b */
	0x6b, /* curve_1_a */
	0x03, /* curve_2_b */
	0x48, /* curve_2_a */
	0x08, /* curve_3_b */
	0x32, /* curve_3_a */
	0x08, /* curve_4_b */
	0x32, /* curve_4_a */
	0x08, /* curve_5_b */
	0x32, /* curve_5_a */
	0x08, /* curve_6_b */
	0x32, /* curve_6_a */
	0x08, /* curve_7_b */
	0x32, /* curve_7_a */
	0x10, /* curve_8_b */
	0x28, /* curve_8_a */
	0x10, /* curve_9_b */
	0x28, /* curve_9_a */
	0x10, /* curve10_b */
	0x28, /* curve10_a */
	0x10, /* curve11_b */
	0x28, /* curve11_a */
	0x10, /* curve12_b */
	0x28, /* curve12_a */
	0x19, /* curve13_b */
	0x22, /* curve13_a */
	0x49, /* curve14_b */
	0xdf, /* curve14_a */
	0x49, /* curve15_b */
	0xdf, /* curve15_a */
	0x49, /* curve16_b */
	0xdf, /* curve16_a */
	0x49, /* curve17_b */
	0xdf, /* curve17_a */
	0x50, /* curve18_b */
	0x1c, /* curve18_a */
	0x5b, /* curve19_b */
	0x18, /* curve19_a */
	0x6a, /* curve20_b */
	0x14, /* curve20_a */
	0x7a, /* curve21_b */
	0x11, /* curve21_a */
	0x87, /* curve22_b */
	0x0f, /* curve22_a */
	0x87, /* curve23_b */
	0x0f, /* curve23_a */
	0x00, /* curve24_b */
	0xff, /* curve24_a */
	0x30, /* ascr_skin_on strength 0 00000 */
	0x67, /* ascr_skin_cb */
	0xa9, /* ascr_skin_cr */
	0x56, /* ascr_dist_up */
	0x29, /* ascr_dist_down */
	0x19, /* ascr_dist_right */
	0x67, /* ascr_dist_left */
	0x00, /* ascr_div_up 20 */
	0x17,
	0xd0,
	0x00, /* ascr_div_down */
	0x31,
	0xf4,
	0x00, /* ascr_div_right */
	0x51,
	0xec,
	0x00, /* ascr_div_left */
	0x13,
	0xe2,
	0xff, /* ascr_skin_Rr */
	0xa0, /* ascr_skin_Rg */
	0xa0, /* ascr_skin_Rb */
	0xff, /* ascr_skin_Yr */
	0x90, /* ascr_skin_Yg */
	0x00, /* ascr_skin_Yb */
	0xff, /* ascr_skin_Mr */
	0x00, /* ascr_skin_Mg */
	0xff, /* ascr_skin_Mb */
	0xff, /* ascr_skin_Wr */
	0xff, /* ascr_skin_Wg */
	0xff, /* ascr_skin_Wb */
	0x00, /* ascr_Cr */
	0xff, /* ascr_Rr */
	0xff, /* ascr_Cg */
	0x00, /* ascr_Rg */
	0xff, /* ascr_Cb */
	0x00, /* ascr_Rb */
	0xff, /* ascr_Mr */
	0x00, /* ascr_Gr */
	0x00, /* ascr_Mg */
	0xff, /* ascr_Gg */
	0xff, /* ascr_Mb */
	0x00, /* ascr_Gb */
	0xff, /* ascr_Yr */
	0x00, /* ascr_Br */
	0xff, /* ascr_Yg */
	0x00, /* ascr_Bg */
	0x00, /* ascr_Yb */
	0xff, /* ascr_Bb */
	0xff, /* ascr_Wr */
	0x00, /* ascr_Kr */
	0xf8, /* ascr_Wg */
	0x00, /* ascr_Kg */
	0xff, /* ascr_Wb */
	0x00, /* ascr_Kb */
	/* end */
};

static unsigned char LEVEL1_UNLOCK[] = {
	0xF0,
	0x5A, 0x5A
};

static unsigned char LEVEL1_LOCK[] = {
	0xF0,
	0xA5, 0xA5
};

struct mdnie_table bypass_table[BYPASS_MAX] = {
	[BYPASS_ON] = MDNIE_SET(BYPASS)
};

struct mdnie_table accessibility_table[ACCESSIBILITY_MAX] = {
	[NEGATIVE] = MDNIE_SET(NEGATIVE),
	MDNIE_SET(COLOR_BLIND),
	MDNIE_SET(SCREEN_CURTAIN)
};

struct mdnie_table hbm_table[HBM_MAX] = {
	[HBM_ON_TEXT] = MDNIE_SET(LOCAL_CE_TEXT),
	[HBM_ON] = MDNIE_SET(LOCAL_CE)
};

struct mdnie_table tuning_table[SCENARIO_MAX][MODE_MAX] = {
	{
		MDNIE_SET(DYNAMIC_UI),
		MDNIE_SET(STANDARD_UI),
		MDNIE_SET(NATURAL_UI),
		MDNIE_SET(MOVIE_UI),
		MDNIE_SET(AUTO_UI)
	}, {
		MDNIE_SET(DYNAMIC_VIDEO),
		MDNIE_SET(STANDARD_VIDEO),
		MDNIE_SET(NATURAL_VIDEO),
		MDNIE_SET(MOVIE_VIDEO),
		MDNIE_SET(AUTO_VIDEO)
	},
	[CAMERA_MODE] = {
		MDNIE_SET(CAMERA),
		MDNIE_SET(CAMERA),
		MDNIE_SET(CAMERA),
		MDNIE_SET(CAMERA),
		MDNIE_SET(AUTO_CAMERA)
	},
	[GALLERY_MODE] = {
		MDNIE_SET(DYNAMIC_GALLERY),
		MDNIE_SET(STANDARD_GALLERY),
		MDNIE_SET(NATURAL_GALLERY),
		MDNIE_SET(MOVIE_GALLERY),
		MDNIE_SET(AUTO_GALLERY)
	}, {
		MDNIE_SET(DYNAMIC_VT),
		MDNIE_SET(STANDARD_VT),
		MDNIE_SET(NATURAL_VT),
		MDNIE_SET(MOVIE_VT),
		MDNIE_SET(AUTO_VT)
	}, {
		MDNIE_SET(DYNAMIC_BROWSER),
		MDNIE_SET(STANDARD_BROWSER),
		MDNIE_SET(NATURAL_BROWSER),
		MDNIE_SET(MOVIE_BROWSER),
		MDNIE_SET(AUTO_BROWSER)
	}, {
		MDNIE_SET(DYNAMIC_EBOOK),
		MDNIE_SET(STANDARD_EBOOK),
		MDNIE_SET(NATURAL_EBOOK),
		MDNIE_SET(MOVIE_EBOOK),
		MDNIE_SET(AUTO_EBOOK)
	}, {
		MDNIE_SET(AUTO_EMAIL),
		MDNIE_SET(AUTO_EMAIL),
		MDNIE_SET(AUTO_EMAIL),
		MDNIE_SET(AUTO_EMAIL),
		MDNIE_SET(AUTO_EMAIL)
	},
};

#endif
