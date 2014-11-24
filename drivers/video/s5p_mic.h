#ifndef __S5P_MIC_H__
#define __S5P_MIC_H__

/* MIC Register Map */
#define S5P_MIC_OP              0x00
#define S5P_MIC_UPDATE_REG	(1 << 31)
#define S5P_MIC_ON_REG          (1 << 30)
#define S5P_MIC_SWAP_BIT_STREAM	(1 << 16)
#define S5P_MIC_2D_MODE         (0 << 12)
#define S5P_MIC_SW_RST          (1 << 4)
#define S5P_MIC_PSR_DISABLE     (0 << 5)
#define S5P_MIC_NEW_CORE	(0 << 2)
#define S5P_MIC_OLD_CORE	(1 << 2)
#define S5P_MIC_VIDEO_MODE	(0 << 1)
#define S5P_MIC_COMMAND_MODE	(1 << 1)
#define S5P_MIC_CORE_ENABLE	(1 << 0)
#define S5P_MIC_CORE_DISABLE	(0 << 0)

#define S5P_MIC_VER     		0x04

#define S5P_MIC_V_TIMING_0      	0x08
#define S5P_MIC_V_PULSE_WIDTH_SHIFT	16
#define S5P_MIC_V_PERIOD_LINE_SHIFT	0

#define S5P_MIC_V_TIMING_1      	0x0C
#define S5P_MIC_VBP_SIZE_SHIFT          16
#define S5P_MIC_VFP_SIZE_SHIFT          0

#define S5P_MIC_IMG_SIZE        	0x10
#define S5P_MIC_IMG_V_SIZE_SHIFT	16
#define S5P_MIC_IMG_H_SIZE_SHIFT	0

#define S5P_MIC_INPUT_TIMING_0          0x14
#define S5P_MIC_H_PULSE_WIDTH_SHIFT     16
#define S5P_MIC_H_PERIOD_PIXEL_SHIFT    0

#define S5P_MIC_INPUT_TIMING_1          0x18
#define S5P_MIC_HBP_SIZE_SHIFT          16
#define S5P_MIC_HFP_SIZE_SHIFT          0

#define S5P_MIC_2D_OUTPUT_TIMING_0	0x1C
#define S5P_MIC_H_PULSE_WIDTH_2D_SHIFT  16
#define S5P_MIC_H_PERIOD_PIXEL_2D_SHIFT 0

#define S5P_MIC_2D_OUTPUT_TIMING_1	0x20
#define S5P_MIC_HBP_SIZE_2D_SHIFT       16
#define S5P_MIC_HFP_SIZE_2D_SHIFT       0

#define S5P_MIC_2D_OUTPUT_TIMING_2	0x24

#define S5P_MIC_3D_OUTPUT_TIMING_0	0x28
#define S5P_MIC_H_PULSE_WIDTH_3D_SHIFT  16
#define S5P_MIC_H_PERIOD_PIXEL_3D_SHIFT 0

#define S5P_MIC_3D_OUTPUT_TIMING_1      0x2C
#define S5P_MIC_HBP_SIZE_3D_SHIFT       16
#define S5P_MIC_HFP_SIZE_3D_SHIFT       0

#define S5P_MIC_3D_OUTPUT_TIMING_2	0x30

#define S5P_MIC_ALG_PARA_0              0x34
#define S5P_ENC_CORE_PARA_W_SHIFT       24
#define S5P_ENC_CORE_PARA_M_SHIFT       16
#define S5P_ENC_CORE_PARA_S_SHIFT       8
#define S5P_ENC_CORE_PARA_B_SHIFT       0

#define S5P_MIC_ALG_PARA_1              0x38
#define S5P_ENC_CORE_PARA_EB_SHIFT      24
#define S5P_ENC_CORE_PARA_EG_SHIFT      16
#define S5P_ENC_CORE_PARA_ER_SHIFT      8
#define S5P_ENC_CORE_PARA_Y_SHIFT       0

#define S5P_MIC_CRC_CTRL	0x40
#define S5P_MIC_CRC_DATA	0x44

#endif
