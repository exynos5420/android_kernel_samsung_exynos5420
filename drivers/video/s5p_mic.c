/* linux/drivers/video/s5p_mic.c
 *
 * Copyright 2013-2015 Samsung Electronics
 *      Haowei Li <haowei.li@samsung.com>
 *
 * Samsung MIC(Mobile Image Compression) driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software FoundatIon.
*/

#include <linux/fb.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <plat/fb.h>
#include "s5p_mic.h"

#define S5P_MIC_PARA_W	0x2
#define S5P_MIC_PARA_M	0x0
#define S5P_MIC_PARA_S	0x18
#define S5P_MIC_PARA_B	0x0
#define S5P_MIC_PARA_EB	0x10
#define S5P_MIC_PARA_EG	0x08
#define S5P_MIC_PARA_ER	0x08
#define S5P_MIC_PARA_Y	0x02

enum mic_on_off {
	S5P_MIC_OFF = 0,
	S5P_MIC_ON = 1
};

struct s5p_mic {
	struct device *dev;
	void __iomem *reg_base;
	struct fb_videomode *lcd;
	bool s5p_mic_on;
	struct mutex ops_lock;
};
struct s5p_mic *g_mic;
EXPORT_SYMBOL(g_mic);

static void s5p_mic_set_image_size(struct s5p_mic *mic)
{
	u32 data = 0;
	struct fb_videomode *lcd = mic->lcd;

	data = (lcd->yres << S5P_MIC_IMG_V_SIZE_SHIFT)
		| (lcd->xres << S5P_MIC_IMG_H_SIZE_SHIFT);

	writel(data, mic->reg_base + S5P_MIC_IMG_SIZE);
}

static unsigned int s5p_mic_calc_bs_size(struct s5p_mic *mic)
{
	struct fb_videomode *lcd = mic->lcd;
	u32 temp1, temp2, bs_size;

	temp1 = lcd->xres / 4 * 2;
	temp2 = lcd->xres % 4;
	bs_size = temp1 + temp2;

	return bs_size;
}

static void s5p_mic_set_2d_bit_stream_size(struct s5p_mic *mic)
{
	u32 data;

	data = s5p_mic_calc_bs_size(mic);

	writel(data, mic->reg_base + S5P_MIC_2D_OUTPUT_TIMING_2);
}

static void s5p_mic_set_mic_base_operation(struct s5p_mic *mic, bool enable)
{
	u32 data = readl(mic->reg_base);
	struct fb_videomode *lcd = mic->lcd;

	if (enable) {
		data |= S5P_MIC_NEW_CORE | S5P_MIC_CORE_ENABLE
			| S5P_MIC_2D_MODE | S5P_MIC_PSR_DISABLE
			| S5P_MIC_UPDATE_REG | S5P_MIC_ON_REG
			| S5P_MIC_SWAP_BIT_STREAM;
	} else {
		data &= ~S5P_MIC_CORE_ENABLE;
		data |= S5P_MIC_UPDATE_REG;
	}

	writel(data, mic->reg_base);
}

static void s5p_mic_sw_reset(struct s5p_mic *mic)
{
	writel(S5P_MIC_SW_RST, mic->reg_base + S5P_MIC_OP);
}

static void s5p_mic_set_porch_timing(struct s5p_mic *mic)
{
	struct fb_videomode *lcd = mic->lcd;
	u32 data, v_period, h_period;

	v_period = lcd->vsync_len + lcd->yres + lcd->upper_margin
			+ lcd->lower_margin;
	data = lcd->vsync_len << S5P_MIC_V_PULSE_WIDTH_SHIFT
			| v_period << S5P_MIC_V_PERIOD_LINE_SHIFT;
	writel(data, mic->reg_base + S5P_MIC_V_TIMING_0);

	data = lcd->upper_margin << S5P_MIC_VBP_SIZE_SHIFT
			| lcd->lower_margin << S5P_MIC_VFP_SIZE_SHIFT;
	writel(data, mic->reg_base + S5P_MIC_V_TIMING_1);

	h_period = lcd->hsync_len + lcd->xres + lcd->left_margin
			+ lcd->right_margin;
	data = lcd->hsync_len << S5P_MIC_H_PULSE_WIDTH_SHIFT
			| h_period << S5P_MIC_H_PERIOD_PIXEL_SHIFT;

	writel(data, mic->reg_base + S5P_MIC_INPUT_TIMING_0);

	data = lcd->left_margin << S5P_MIC_HBP_SIZE_SHIFT
			| lcd->right_margin << S5P_MIC_HFP_SIZE_SHIFT;

	writel(data, mic->reg_base + S5P_MIC_INPUT_TIMING_1);
}

static void s5p_mic_set_2d_output_timing(struct s5p_mic *mic)
{
	struct fb_videomode *lcd = mic->lcd;
	u32 data, h_period_2d;
	u32 hsa_2d = lcd->hsync_len;
	u32 hbp_2d = lcd->left_margin;
	u32 bs_2d = s5p_mic_calc_bs_size(mic);
	u32 hfp_2d = lcd->right_margin + bs_2d;

	h_period_2d = hsa_2d + hbp_2d + bs_2d + hfp_2d;

	data = hsa_2d << S5P_MIC_H_PULSE_WIDTH_2D_SHIFT
			| h_period_2d << S5P_MIC_H_PERIOD_PIXEL_2D_SHIFT;

	writel(data, mic->reg_base + S5P_MIC_2D_OUTPUT_TIMING_0);

	data = hbp_2d << S5P_MIC_HBP_SIZE_2D_SHIFT
			| hfp_2d << S5P_MIC_HFP_SIZE_2D_SHIFT;

	writel(data, mic->reg_base + S5P_MIC_2D_OUTPUT_TIMING_1);

	writel(bs_2d, mic->reg_base + S5P_MIC_2D_OUTPUT_TIMING_2);
}

static void s5p_mic_set_3d_output_timing(struct s5p_mic *mic)
{
	struct fb_videomode *lcd = mic->lcd;
	u32 data, h_period_3d;
	u32 hsa_3d = lcd->hsync_len;
	u32 hbp_3d = lcd->left_margin;
	u32 bs_3d = s5p_mic_calc_bs_size(mic);
	u32 hfp_3d = lcd->right_margin + bs_3d;

	h_period_3d = hsa_3d + hbp_3d + bs_3d + hfp_3d;

	data = hsa_3d << S5P_MIC_H_PULSE_WIDTH_3D_SHIFT
			| h_period_3d << S5P_MIC_H_PERIOD_PIXEL_3D_SHIFT;

	writel(data, mic->reg_base + S5P_MIC_3D_OUTPUT_TIMING_0);

	data = hbp_3d << S5P_MIC_HBP_SIZE_3D_SHIFT
			| hfp_3d << S5P_MIC_HFP_SIZE_3D_SHIFT;

	writel(data, mic->reg_base + S5P_MIC_3D_OUTPUT_TIMING_1);

	writel(bs_3d, mic->reg_base + S5P_MIC_3D_OUTPUT_TIMING_2);
}

static void s5p_mic_set_alg_para(struct s5p_mic *mic)
{
	u32 data;

	data = S5P_MIC_PARA_W << S5P_ENC_CORE_PARA_W_SHIFT
		| S5P_MIC_PARA_M << S5P_ENC_CORE_PARA_M_SHIFT
		| S5P_MIC_PARA_S << S5P_ENC_CORE_PARA_S_SHIFT
		| S5P_MIC_PARA_B << S5P_ENC_CORE_PARA_B_SHIFT;
	writel(data, mic->reg_base + S5P_MIC_ALG_PARA_0);

	data = S5P_MIC_PARA_EB << S5P_ENC_CORE_PARA_EB_SHIFT
		| S5P_MIC_PARA_EG << S5P_ENC_CORE_PARA_EG_SHIFT
		| S5P_MIC_PARA_ER << S5P_ENC_CORE_PARA_ER_SHIFT
		| S5P_MIC_PARA_Y << S5P_ENC_CORE_PARA_Y_SHIFT;
	writel(data, mic->reg_base + S5P_MIC_ALG_PARA_1);
}

int s5p_mic_enable(struct s5p_mic *mic)
{
	if (mic->s5p_mic_on == true)
		return 0;

	s5p_mic_sw_reset(mic);
	s5p_mic_set_porch_timing(mic);
	s5p_mic_set_image_size(mic);
	s5p_mic_set_2d_output_timing(mic);
	s5p_mic_set_3d_output_timing(mic);
	s5p_mic_set_alg_para(mic);
	s5p_mic_set_mic_base_operation(mic, S5P_MIC_ON);

	mic->s5p_mic_on = true;

	dev_dbg(mic->dev, "MIC driver is ON;\n");

	return 0;
}

int s5p_mic_disable(struct s5p_mic *mic)
{
	if (mic->s5p_mic_on == false)
		return 0;

	/* s5p_mic_set_mic_base_operation(mic, S5P_MIC_OFF); */

	mic->s5p_mic_on = false;

	dev_dbg(mic->dev, "MIC driver is OFF;\n");

	return 0;
}

static int s5p_mic_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct s5p_mic *mic;
	struct resource *res;
	struct s3c_fb_pd_win *pd_win = pdev->dev.platform_data;
	int ret;


	mic = devm_kzalloc(dev, sizeof(struct s5p_mic), GFP_KERNEL);
	if (!mic) {
		dev_err(dev, "no memory for mic driver");
		return -ENOMEM;
	}

	mic->dev = dev;
	mic->lcd = &pd_win->win_mode;
	mutex_init(&mic->ops_lock);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (IS_ERR_OR_NULL(res)) {
		dev_err(dev, "failed to find resource\n");
		ret = -ENOENT;
	}

	mic->reg_base = ioremap(res->start, resource_size(res));
	if (!mic->reg_base) {
		dev_err(dev, "failed to map registers\n");
		ret = -ENXIO;
	}

	platform_set_drvdata(pdev, mic);


	g_mic = mic;
	mic->s5p_mic_on = true;

	dev_dbg(dev, "MIC driver has been probed\n");
	return 0;
}

static struct platform_driver s5p_mic_driver = {
	.probe = s5p_mic_probe,
	.driver = {
		.name = "s5p-mic",
		.owner = THIS_MODULE,
	},
};

static int __init s5p_mic_init(void)
{
	return platform_driver_register(&s5p_mic_driver);
}

static void __init s5p_mic_cleanup(void)
{
	platform_driver_unregister(&s5p_mic_driver);
}

late_initcall(s5p_mic_init);
module_exit(s5p_mic_cleanup);

MODULE_AUTHOR("Haowei Li <Haowei.li@samsung.com>");
MODULE_DESCRIPTION("Samsung MIC driver");
MODULE_LICENSE("GPL");
