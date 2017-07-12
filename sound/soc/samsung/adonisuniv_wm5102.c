/*
 *  adonisuniv_wm5102.c
 *
 *  Copyright (c) 2012 Samsung Electronics Co. Ltd
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/input.h>
#include <linux/wakelock.h>
#include <linux/suspend.h>

#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/jack.h>
#include <linux/regmap.h>

#include <mach/regs-clock.h>
#include <mach/pmu.h>
#include <mach/gpio.h>
#include <mach/gpio-exynos.h>
#include <mach/exynos5-audio.h>
#include <linux/mfd/arizona/registers.h>
#include <linux/mfd/arizona/core.h>

#include "i2s.h"
#include "i2s-regs.h"
#include "../codecs/wm5102.h"

#define USE_BIAS_LEVEL_POST

#define ADONISUNIV_DEFAULT_MCLK1	24000000
#define ADONISUNIV_DEFAULT_MCLK2	32768

#define ADONISUNIV_TELCLK_RATE		(48000 * 512)

#define CLK_MODE_MEDIA 0
#define CLK_MODE_TELEPHONY 1

typedef enum {
	MICBIAS1ON,
	MICBIAS2ON,
	MICBIAS3ON,
	MICBIAS1OFF,
	MICBIAS2OFF,
	MICBIAS3OFF
} micbias_type;

struct wm5102_machine_priv {
	int clock_mode;
	struct snd_soc_jack jack;
	struct snd_soc_codec *codec;
	struct snd_soc_dai *aif[3];
	struct delayed_work mic_work;
	struct wake_lock jackdet_wake_lock;
	int aif2mode;
	int micbias_mode;

	int aif1rate;
	int aif2rate;
};
static int lhpf1_coeff;
static int lhpf2_coeff;
static int lhpf3_coeff;
static int lhpf4_coeff;
static int drc1_ng_exp;
static int drc1_knee2_op;
static int drc1_knee2_ip;
static int drc1_lo_comp;
static int drc1_hi_comp;
static int drc1_knee_op;
static int drc1_knee_ip;
static int drc1_qr_dcy;
static int drc1_qr_thr;
static int drc1_ng_mingain;
static int drc1_maxgain;
static int drc1_mingain;
static int drc1_dcy;
static int drc1_atk;
static int drc1_qr;
static int drc1_knee2_op_ena;
static int drc1_anticlip;
static int drc1_ng_ena;
static int drc1l_ena;
static int drc1r_ena;

static int drc1_sw_values[] = {
     0, 1
};

const char *drc1_sw_text[] = {
	"OFF", "ON"
};

static int drc1_atk_values[] = {
     1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
};

const char *drc1_atk_text[] = {
	"181us", "363us", "726us", "1.45ms", "2.9ms", "5.8ms", "11.6ms", "23.2ms", "46.4ms", "92.8ms", "185.6ms"
};

static int drc1_dcy_values[] = {
     0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
};

const char *drc1_dcy_text[] = {
	"1.45ms", "2.9ms", "5.8ms", "11.6ms", "23.25ms", "46.5ms", "93ms", "186ms", "372ms", "743ms", "1.49s", "2.97s" 
};

static int drc1_mingain_values[] = {
    0, 1, 2, 3, 4
};

const char *drc1_mingain_text[] = {
	"0dB", "-12dB", "-18dB", "-24dB", "-36dB"
};

static int drc1_maxgain_values[] = {
    0, 1, 2, 3
};

const char *drc1_maxgain_text[] = {
	"12dB", "18dB", "24dB", "36dB"
};

static int drc1_ng_mingain_values[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
};

const char *drc1_ng_mingain_text[] = {
	"-36dB", "-30dB", "-24dB", "-18dB", "-12dB", "-6dB", "0dB", "6dB", "12dB", "18dB", "24dB", "30dB", "36dB"
};

static int drc1_qr_thr_values[] = {
    0, 1, 2, 3
};

const char *drc1_qr_thr_text[] = {
	"12dB", "18dB", "24dB", "30dB"
};

static int drc1_qr_dcy_values[] = {
    0, 1, 2
};

const char *drc1_qr_dcy_text[] = {
	"0.725ms", "1.45ms", "5.8ms"
};

static int drc1_knee_ip_values[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60
};

const char *drc1_knee_ip_text[] = {
	"0dB", "-0.75dB", "-1.5dB", "-2.25dB", "-3dB", "-3.75dB", "-4.5dB", "-5.25dB", "-6dB", "-6.75dB", "-7.5dB", "-8.25dB", "-9dB", "-9.75dB", "-10.5dB", "-11.25dB", "-12dB", "-12.75dB", "-13.5dB", "-14.25dB", "-15dB", "-15.75dB", "-16.5dB", "-17.25dB", "-18dB", "-18.75dB", "-19.5dB", "-20.25dB", "-21dB", "-21.75dB", "-22.5dB", "-23.25dB", "-24dB", "-24.75dB", "-25.5dB", "-26.25dB", "-27dB", "-27.75dB", "-28.5dB", "-29.25dB", "-30dB", "-30.75dB", "-31.5dB", "-32.25dB", "-33dB", "-33.75dB", "-34.5dB", "-35.25dB", "-36dB", "-36.75dB", "-37.5dB", "-38.25dB", "-39dB", "-39.75dB", "-40.5dB", "-41.25dB", "-42dB", "-42.75dB", "-43.5dB", "-44.25dB", "-45dB"
};

static int drc1_knee_op_values[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30
};

const char *drc1_knee_op_text[] = {
	"0dB", "-0.75dB", "-1.5dB", "-2.25dB", "-3dB", "-3.75dB", "-4.5dB", "-5.25dB", "-6dB", "-6.75dB", "-7.5dB", "-8.25dB", "-9dB", "-9.75dB", "-10.5dB", "-11.25dB", "-12dB", "-12.75dB", "-13.5dB", "-14.25dB", "-15dB", "-15.75dB", "-16.5dB", "-17.25dB", "-18dB", "-18.75dB", "-19.5dB", "-20.25dB", "-21dB", "-21.75dB", "-22.5dB"
};

static int drc1_hi_comp_values[] = {
    0, 1, 2, 3, 4, 5
};

const char *drc1_hi_comp_text[] = {
	"1x", "x/2", "x/4", "x/8", "x/16", "0x"
};

static int drc1_lo_comp_values[] = {
    0, 1, 2, 3, 4
};

const char *drc1_lo_comp_text[] = {
	"1x", "x/2", "x/4", "x/8", "0x"
};

static int drc1_knee2_ip_values[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
};

const char *drc1_knee2_ip_text[] = {
	"-30dB", "-31.5dB", "-33dB", "-34.5dB", "-36dB", "-37.5dB", "-39dB", "-40.5dB", "-42dB", "-43.5dB", "-45dB", "-46.5dB", "-48dB", "-49.5dB", "-51dB", "-52.5dB", "-54dB", "-55.5dB", "-57dB", "-58.5dB", "-60dB", "-61.5dB", "-63dB", "-64.5dB", "-66dB", "-67.5dB", "-69dB", "-70.5dB", "-72dB", "-73.5dB", "-75dB", "-76.5dB", "-78dB", "-79.5dB", "-81dB", "-82.5dB"
};

static int drc1_knee2_op_values[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
};

const char *drc1_knee2_op_text[] = {
	"-30dB", "-31.5dB", "-33dB", "-34.5dB", "-36dB", "-37.5dB", "-39dB", "-40.5dB", "-42dB", "-43.5dB", "-45dB", "-46.5dB", "-48dB", "-49.5dB", "-51dB", "-52.5dB", "-54dB", "-55.5dB", "-57dB", "-58.5dB", "-60dB", "-61.5dB", "-63dB", "-64.5dB", "-66dB", "-67.5dB", "-69dB", "-70.5dB", "-72dB", "-73.5dB", "-75dB", "-76.5dB"
};

static int drc1_ng_exp_values[] = {
	0, 1, 2, 3
};

const char *drc1_ng_exp_text[] = {
	"1x", "2x", "4x", "8x"
};

static unsigned int lhpf_filter_values[] = {
	0xF002, 0xF007, 0xF00C, 0xF01D,  0xF045, 0xF069, 0xF084, 0xF0D1, 0xF11D, 0xF198, 0xF215, 0xF2DE, 0xF3B9, 0xF489, 0xF54F, 0xF60D, 0xF81C, 0xFB25, 0xFCD1,  0x86, 0x32F, 0x0000, 0x0000
};

const char *lhpf_filter_text[] = {
	"4Hz", "13Hz", "22Hz", "54Hz", "130Hz", "200Hz", "250Hz", "400Hz", "550Hz", "800Hz", "1062Hz", "1.5kHz", "2kHz", "2.5kHz", "3kHz", "3.5kHz", "5kHz", "7.5kHz", "9kHz", "12.5kHz", "15kHz", "user defined1", "user defined2"
};

const char *aif2_mode_text[] = {
	"Slave", "Master"
};

const char *micbias_mode_text[] = {
	"BIAS1ON", "BIAS2ON", "BIAS3ON", "BIAS1OFF", "BIAS2OFF", "BIAS3OFF"
};

static const struct soc_enum lhpf_filter_mode_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(lhpf_filter_text), lhpf_filter_text),
};

static const struct soc_enum drc1_sw_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(drc1_sw_text), drc1_sw_text),
};

static const struct soc_enum drc1_atk_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(drc1_atk_text), drc1_atk_text),
};

static const struct soc_enum drc1_dcy_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(drc1_dcy_text), drc1_dcy_text),
};

static const struct soc_enum drc1_mingain_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(drc1_mingain_text), drc1_mingain_text),
};

static const struct soc_enum drc1_maxgain_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(drc1_maxgain_text), drc1_maxgain_text),
};

static const struct soc_enum drc1_ng_mingain_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(drc1_ng_mingain_text), drc1_ng_mingain_text),
};

static const struct soc_enum drc1_qr_thr_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(drc1_qr_thr_text), drc1_qr_thr_text),
};

static const struct soc_enum drc1_qr_dcy_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(drc1_qr_dcy_text), drc1_qr_dcy_text),
};

static const struct soc_enum drc1_knee_ip_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(drc1_knee_ip_text), drc1_knee_ip_text),
};

static const struct soc_enum drc1_knee_op_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(drc1_knee_op_text), drc1_knee_op_text),
};

static const struct soc_enum drc1_hi_comp_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(drc1_hi_comp_text), drc1_hi_comp_text),
};

static const struct soc_enum drc1_lo_comp_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(drc1_lo_comp_text), drc1_lo_comp_text),
};

static const struct soc_enum drc1_knee2_ip_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(drc1_knee2_ip_text), drc1_knee2_ip_text),
};

static const struct soc_enum drc1_knee2_op_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(drc1_knee2_op_text), drc1_knee2_op_text),
};

static const struct soc_enum drc1_ng_exp_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(drc1_ng_exp_text), drc1_ng_exp_text),
};

static const struct soc_enum aif2_mode_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(aif2_mode_text), aif2_mode_text),
};

static const struct soc_enum micbias_mode_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(micbias_mode_text), micbias_mode_text),
};

static struct {
	int min;           /* Minimum impedance */
	int max;           /* Maximum impedance */
	unsigned int gain; /* Register value to set for this measurement */
} hp_gain_table[] = {
	{    0,      42, 0x6c | ARIZONA_OUT_VU },
	{   43,     100, 0x70 | ARIZONA_OUT_VU },
	{  101,     200, 0x74 | ARIZONA_OUT_VU },
	{  201,     450, 0x78 | ARIZONA_OUT_VU },
	{  451,    1000, 0x7c | ARIZONA_OUT_VU },
	{ 1001, INT_MAX, 0x6c | ARIZONA_OUT_VU },
};

static struct snd_soc_codec *the_codec;

void adonisuniv_wm5102_hpdet_cb(unsigned int meas)
{
	int i;

	WARN_ON(!the_codec);
	if (!the_codec)
		return;

	for (i = 0; i < ARRAY_SIZE(hp_gain_table); i++) {
		if (meas < hp_gain_table[i].min || meas > hp_gain_table[i].max)
			continue;

		dev_crit(the_codec->dev, "SET GAIN %x for %d ohms\n",
			 hp_gain_table[i].gain, meas);
		snd_soc_write(the_codec, ARIZONA_DAC_DIGITAL_VOLUME_1L,
				hp_gain_table[i].gain);
		snd_soc_write(the_codec, ARIZONA_DAC_DIGITAL_VOLUME_1R,
				hp_gain_table[i].gain);
	}
}

static int adonisuniv_start_sysclk(struct snd_soc_card *card)
{
	struct wm5102_machine_priv *priv = snd_soc_card_get_drvdata(card);
	int ret;
	int fs;

	if (priv->aif1rate >= 192000)
		fs = 256;
	else
		fs = 512;

	ret = snd_soc_codec_set_pll(priv->codec, WM5102_FLL1,
				     ARIZONA_CLK_SRC_MCLK1,
				    ADONISUNIV_DEFAULT_MCLK1,
				    priv->aif1rate * fs);
	if (ret != 0) {
		dev_err(priv->codec->dev, "Failed to start FLL1: %d\n", ret);
		return ret;
	}

	return ret;
}

static int adonisuniv_stop_sysclk(struct snd_soc_card *card)
{
	struct wm5102_machine_priv *priv = snd_soc_card_get_drvdata(card);
	int ret;

	ret = snd_soc_codec_set_pll(priv->codec, WM5102_FLL1, 0, 0, 0);
	if (ret != 0) {
		dev_err(priv->codec->dev, "Failed to stop FLL1: %d\n", ret);
		return ret;
	}

	ret = snd_soc_codec_set_sysclk(priv->codec, ARIZONA_CLK_SYSCLK, 0,
				0, 0);
	if (ret != 0) {
		dev_err(priv->codec->dev, "Failed to stop SYSCLK: %d\n", ret);
		return ret;
	}

	return ret;
}

#ifdef USE_BIAS_LEVEL_POST
static int adonisuniv_set_bias_level(struct snd_soc_card *card,
				     struct snd_soc_dapm_context *dapm,
				     enum snd_soc_bias_level level)
{
	struct snd_soc_dai *codec_dai = card->rtd[0].codec_dai;

	if (dapm->dev != codec_dai->dev)
		return 0;

	dev_info(card->dev, "%s: %d\n", __func__, level);

	switch (level) {
	case SND_SOC_BIAS_PREPARE:
		if (dapm->bias_level != SND_SOC_BIAS_STANDBY)
		break;

		adonisuniv_start_sysclk(card);
		break;
	default:
		break;
	}

	return 0;
}

static int adonisuniv_set_bias_level_post(struct snd_soc_card *card,
				     struct snd_soc_dapm_context *dapm,
				     enum snd_soc_bias_level level)
{
	struct snd_soc_dai *codec_dai = card->rtd[0].codec_dai;

	if (dapm->dev != codec_dai->dev)
		return 0;

	dev_info(card->dev, "%s: %d\n", __func__, level);

	switch (level) {
	case SND_SOC_BIAS_STANDBY:
		adonisuniv_stop_sysclk(card);
		break;
	default:
		break;
	}

	dapm->bias_level = level;

	return 0;
}
#endif

int adonisuniv_set_media_clocking(struct wm5102_machine_priv *priv)
{
	struct snd_soc_codec *codec = priv->codec;
	int ret;
	int fs;

	if (priv->aif1rate >= 192000)
		fs = 256;
	else
		fs = 512;

	ret = snd_soc_codec_set_pll(codec, WM5102_FLL1_REFCLK,
				    ARIZONA_FLL_SRC_NONE, 0, 0);
	if (ret != 0) {
		dev_err(codec->dev, "Failed to start FLL1 REF: %d\n", ret);
		return ret;
	}
	ret = snd_soc_codec_set_pll(codec, WM5102_FLL1, ARIZONA_CLK_SRC_MCLK1,
				    ADONISUNIV_DEFAULT_MCLK1,
				    priv->aif1rate * fs);
	if (ret != 0) {
		dev_err(codec->dev, "Failed to start FLL1: %d\n", ret);
		return ret;
	}

	ret = snd_soc_codec_set_sysclk(codec,
				       ARIZONA_CLK_SYSCLK,
				       ARIZONA_CLK_SRC_FLL1,
				       priv->aif1rate * fs,
				       SND_SOC_CLOCK_IN);
	if (ret < 0)
		dev_err(codec->dev, "Failed to set SYSCLK to FLL1: %d\n", ret);

	ret = snd_soc_codec_set_sysclk(codec, ARIZONA_CLK_ASYNCCLK,
				       ARIZONA_CLK_SRC_FLL2,
				       ADONISUNIV_TELCLK_RATE,
				       SND_SOC_CLOCK_IN);
	if (ret < 0)
		dev_err(codec->dev,
				 "Unable to set ASYNCCLK to FLL2: %d\n", ret);

	/* AIF1 from SYSCLK, AIF2 and 3 from ASYNCCLK */
	ret = snd_soc_dai_set_sysclk(priv->aif[0], ARIZONA_CLK_SYSCLK, 0, 0);
	if (ret < 0)
		dev_err(codec->dev, "Can't set AIF1 to SYSCLK: %d\n", ret);

	ret = snd_soc_dai_set_sysclk(priv->aif[1], ARIZONA_CLK_ASYNCCLK, 0, 0);
	if (ret < 0)
		dev_err(codec->dev, "Can't set AIF2 to ASYNCCLK: %d\n", ret);

	ret = snd_soc_dai_set_sysclk(priv->aif[2], ARIZONA_CLK_ASYNCCLK, 0, 0);
	if (ret < 0)
		dev_err(codec->dev, "Can't set AIF3 to ASYNCCLK: %d\n", ret);

	return 0;
}

static void adonisuniv_gpio_init(void)
{
#ifdef GPIO_MICBIAS_EN
	int err;

	/* Main Microphone BIAS */
	err = gpio_request(GPIO_MICBIAS_EN, "MAINMIC_BIAS");
	if (err) {
		pr_err(KERN_ERR "MIC_BIAS_EN GPIO set error!\n");
		return;
	}
	gpio_direction_output(GPIO_MICBIAS_EN, 1);

	/*This is tempary code to enable for main mic.(force enable GPIO) */
	gpio_set_value(GPIO_MICBIAS_EN, 0);
#endif
#ifdef GPIO_SUB_MICBIAS_EN
	int ret;

	/* Sub Microphone BIAS */
	ret = gpio_request(GPIO_SUB_MICBIAS_EN, "SUBMIC_BIAS");
	if (ret) {
		pr_err(KERN_ERR "SUBMIC_BIAS_EN GPIO set error!\n");
		return;
	}
	gpio_direction_output(GPIO_SUB_MICBIAS_EN, 1);
	gpio_set_value(GPIO_SUB_MICBIAS_EN, 0);
#endif
}

/*
 * AdnoisUniv wm5102 GPIO enable configure.
 */
static int adonisuniv_ext_mainmicbias(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol,  int event)
{
	struct snd_soc_card *card = w->dapm->card;
	struct snd_soc_codec *codec = card->rtd[0].codec;

#ifdef GPIO_MICBIAS_EN
	switch (event) {

	case SND_SOC_DAPM_PRE_PMU:
		gpio_set_value(GPIO_MICBIAS_EN,  1);
		break;

	case SND_SOC_DAPM_POST_PMD:
		gpio_set_value(GPIO_MICBIAS_EN,  0);
		break;
	}

	dev_dbg(codec->dev, "Main Mic BIAS: %d\n", event);
#endif

	return 0;
}

static int adonisuniv_ext_submicbias(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol,  int event)
{
#ifdef GPIO_SUB_MICBIAS_EN
	struct snd_soc_card *card = w->dapm->card;
	struct snd_soc_codec *codec = card->rtd[0].codec;

	switch (event) {

	case SND_SOC_DAPM_PRE_PMU:
		gpio_set_value(GPIO_SUB_MICBIAS_EN,  1);
		break;

	case SND_SOC_DAPM_POST_PMD:
		gpio_set_value(GPIO_SUB_MICBIAS_EN,  0);
		break;
	}

	dev_dbg(codec->dev, "Sub Mic BIAS: %d\n", event);
#endif
	return 0;
}

static int get_lhpf1_coeff(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = lhpf1_coeff;
	return 0;
}

static int set_lhpf1_coeff(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct regmap *regmap = codec->control_data;

	lhpf1_coeff = ucontrol->value.integer.value[0];

	dev_info(codec->dev, "%s: lhpf1 mode=%d, val=0x%x\n", __func__,
				lhpf1_coeff, lhpf_filter_values[lhpf1_coeff]);

	regmap_update_bits(regmap, ARIZONA_HPLPF1_2, ARIZONA_LHPF1_COEFF_MASK,
			lhpf_filter_values[lhpf1_coeff]);
	return 0;
}

static int get_lhpf2_coeff(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = lhpf2_coeff;
	return 0;
}

static int set_lhpf2_coeff(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct regmap *regmap = codec->control_data;

	lhpf2_coeff = ucontrol->value.integer.value[0];

	dev_info(codec->dev, "%s: lhpf2 mode=%d, val=0x%x (%pK)\n", __func__,
			lhpf2_coeff, lhpf_filter_values[lhpf2_coeff], codec);

	regmap_update_bits(regmap, ARIZONA_HPLPF2_2, ARIZONA_LHPF2_COEFF_MASK,
			lhpf_filter_values[lhpf2_coeff]);
	return 0;
}

static int get_lhpf3_coeff(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = lhpf3_coeff;
	return 0;
}

static int set_lhpf3_coeff(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct regmap *regmap = codec->control_data;

	lhpf3_coeff = ucontrol->value.integer.value[0];

	dev_info(codec->dev, "%s: lhpf3 mode=%d, val=0x%x (%pK)\n", __func__,
			lhpf3_coeff, lhpf_filter_values[lhpf3_coeff], codec);

	regmap_update_bits(regmap, ARIZONA_HPLPF3_2, ARIZONA_LHPF3_COEFF_MASK,
			lhpf_filter_values[lhpf3_coeff]);
	return 0;
}

static int get_lhpf4_coeff(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = lhpf4_coeff;
	return 0;
}

static int set_lhpf4_coeff(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct regmap *regmap = codec->control_data;

	lhpf4_coeff = ucontrol->value.integer.value[0];

	dev_info(codec->dev, "%s: lhpf4 mode=%d, val=0x%x (%pK)\n", __func__,
			lhpf4_coeff, lhpf_filter_values[lhpf4_coeff], codec);

	regmap_update_bits(regmap, ARIZONA_HPLPF4_2, ARIZONA_LHPF4_COEFF_MASK,
			lhpf_filter_values[lhpf4_coeff]);
	return 0;
}

/******************************************************************/

static int get_drc1l_ena_ctl(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = drc1l_ena;
	return 0;
}

static int set_drc1l_ena_ctl(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct regmap *regmap = codec->control_data;

	drc1l_ena = ucontrol->value.integer.value[0];

	dev_info(codec->dev, "%s: drc1l_ena mode=%d, val=0x%x\n", __func__,
				drc1l_ena, drc1_sw_values[drc1l_ena]);

	regmap_update_bits(regmap, ARIZONA_DRC1_CTRL1, ARIZONA_DRC1L_ENA_MASK,
			drc1_sw_values[drc1l_ena]);
	return 0;
}

static int get_drc1r_ena_ctl(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = drc1r_ena;
	return 0;
}

static int set_drc1r_ena_ctl(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct regmap *regmap = codec->control_data;

	drc1r_ena = ucontrol->value.integer.value[0];

	dev_info(codec->dev, "%s: drc1r_ena mode=%d, val=0x%x\n", __func__,
				drc1r_ena, drc1_sw_values[drc1r_ena]);

	regmap_update_bits(regmap, ARIZONA_DRC1_CTRL1, ARIZONA_DRC1R_ENA_MASK,
			drc1_sw_values[drc1r_ena]);
	return 0;
}

static int get_drc1_ng_ena_ctl(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = drc1_ng_ena;
	return 0;
}

static int set_drc1_ng_ena_ctl(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct regmap *regmap = codec->control_data;

	drc1_ng_ena = ucontrol->value.integer.value[0];

	dev_info(codec->dev, "%s: drc1_ng_ena mode=%d, val=0x%x\n", __func__,
				drc1_ng_ena, drc1_sw_values[drc1_ng_ena]);

	regmap_update_bits(regmap, ARIZONA_DRC1_CTRL1, ARIZONA_DRC1_NG_ENA_MASK,
			drc1_sw_values[drc1_ng_ena]);
	return 0;
}

static int get_drc1_anticlip_ctl(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = drc1_anticlip;
	return 0;
}

static int set_drc1_anticlip_ctl(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct regmap *regmap = codec->control_data;

	drc1_anticlip = ucontrol->value.integer.value[0];

	dev_info(codec->dev, "%s: drc_hp_anticlip mode=%d, val=0x%x\n", __func__,
				drc1_anticlip, drc1_sw_values[drc1_anticlip]);

	regmap_update_bits(regmap, ARIZONA_DRC1_CTRL1, ARIZONA_DRC1_ANTICLIP_MASK,
			drc1_sw_values[drc1_anticlip]);
	return 0;
}

static int get_drc1_knee2_op_ena_ctl(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = drc1_knee2_op_ena;
	return 0;
}

static int set_drc1_knee2_op_ena_ctl(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct regmap *regmap = codec->control_data;

	drc1_knee2_op_ena = ucontrol->value.integer.value[0];

	dev_info(codec->dev, "%s: drc1_knee2_op_ena mode=%d, val=0x%x\n", __func__,
				drc1_knee2_op_ena, drc1_sw_values[drc1_knee2_op_ena]);

	regmap_update_bits(regmap, ARIZONA_DRC1_CTRL1, ARIZONA_DRC1_KNEE2_OP_ENA_MASK,
			drc1_sw_values[drc1_knee2_op_ena]);
	return 0;
}
        
static int get_drc1_qr_ctl(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = drc1_qr;
	return 0;
}

static int set_drc1_qr_ctl(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct regmap *regmap = codec->control_data;

	drc1_qr = ucontrol->value.integer.value[0];

	dev_info(codec->dev, "%s: drc1_qr mode=%d, val=0x%x\n", __func__,
				drc1_qr, drc1_sw_values[drc1_qr]);

	regmap_update_bits(regmap, ARIZONA_DRC1_CTRL1, ARIZONA_DRC1_QR_MASK,
			drc1_sw_values[drc1_qr]);
	return 0;
}

static int get_drc1_atk(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = drc1_atk;
	return 0;
}

static int set_drc1_atk(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct regmap *regmap = codec->control_data;

	drc1_atk = ucontrol->value.integer.value[0];

	dev_info(codec->dev, "%s: drc1_atk mode=%d, val=0x%x\n", __func__,
				drc1_atk, drc1_atk_values[drc1_atk]);

	regmap_update_bits(regmap, ARIZONA_DRC1_CTRL2, ARIZONA_DRC1_ATK_MASK,
			drc1_atk_values[drc1_atk]);
	return 0;
}

static int get_drc1_dcy(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = drc1_dcy;
	return 0;
}

static int set_drc1_dcy(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct regmap *regmap = codec->control_data;

	drc1_dcy = ucontrol->value.integer.value[0];

	dev_info(codec->dev, "%s: drc1_dcy mode=%d, val=0x%x\n", __func__,
				drc1_dcy, drc1_dcy_values[drc1_dcy]);

	regmap_update_bits(regmap, ARIZONA_DRC1_CTRL2, ARIZONA_DRC1_DCY_MASK,
			drc1_dcy_values[drc1_dcy]);
	return 0;
}

static int get_drc1_mingain(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = drc1_mingain;
	return 0;
}

static int set_drc1_mingain(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct regmap *regmap = codec->control_data;

	drc1_mingain = ucontrol->value.integer.value[0];

	dev_info(codec->dev, "%s: drc1_mingain mode=%d, val=0x%x\n", __func__,
				drc1_mingain, drc1_mingain_values[drc1_mingain]);

	regmap_update_bits(regmap, ARIZONA_DRC1_CTRL2, ARIZONA_DRC1_MINGAIN_MASK,
			drc1_mingain_values[drc1_mingain]);
	return 0;
}

static int get_drc1_maxgain(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = drc1_maxgain;
	return 0;
}

static int set_drc1_maxgain(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct regmap *regmap = codec->control_data;

	drc1_maxgain = ucontrol->value.integer.value[0];

	dev_info(codec->dev, "%s: drc1_maxgain mode=%d, val=0x%x\n", __func__,
				drc1_maxgain, drc1_maxgain_values[drc1_maxgain]);

	regmap_update_bits(regmap, ARIZONA_DRC1_CTRL2, ARIZONA_DRC1_MAXGAIN_MASK,
			drc1_maxgain_values[drc1_maxgain]);
	return 0;
}

static int get_drc1_ng_mingain(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = drc1_ng_mingain;
	return 0;
}

static int set_drc1_ng_mingain(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct regmap *regmap = codec->control_data;

	drc1_ng_mingain = ucontrol->value.integer.value[0];

	dev_info(codec->dev, "%s: drc1_ng_mingain mode=%d, val=0x%x\n", __func__,
				drc1_ng_mingain, drc1_ng_mingain_values[drc1_ng_mingain]);

	regmap_update_bits(regmap, ARIZONA_DRC1_CTRL3, ARIZONA_DRC1_NG_MINGAIN_MASK,
			drc1_ng_mingain_values[drc1_ng_mingain]);
	return 0;
}

static int get_drc1_qr_thr(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = drc1_qr_thr;
	return 0;
}

static int set_drc1_qr_thr(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct regmap *regmap = codec->control_data;

	drc1_qr_thr = ucontrol->value.integer.value[0];

	dev_info(codec->dev, "%s: drc1_qr_thr mode=%d, val=0x%x\n", __func__,
				drc1_qr_thr, drc1_qr_thr_values[drc1_qr_thr]);

	regmap_update_bits(regmap, ARIZONA_DRC1_CTRL3, ARIZONA_DRC1_QR_THR_MASK,
			drc1_qr_thr_values[drc1_qr_thr]);
	return 0;
}

static int get_drc1_qr_dcy(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = drc1_qr_dcy;
	return 0;
}

static int set_drc1_qr_dcy(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct regmap *regmap = codec->control_data;

	drc1_qr_dcy = ucontrol->value.integer.value[0];

	dev_info(codec->dev, "%s: drc1_qr_dcy mode=%d, val=0x%x\n", __func__,
				drc1_qr_dcy, drc1_qr_dcy_values[drc1_qr_dcy]);

	regmap_update_bits(regmap, ARIZONA_DRC1_CTRL3, ARIZONA_DRC1_QR_DCY_MASK,
			drc1_qr_dcy_values[drc1_qr_dcy]);
	return 0;
}

static int get_drc1_knee_ip(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = drc1_knee_ip;
	return 0;
}

static int set_drc1_knee_ip(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct regmap *regmap = codec->control_data;

	drc1_knee_ip = ucontrol->value.integer.value[0];

	dev_info(codec->dev, "%s: drc1_knee_ip mode=%d, val=0x%x\n", __func__,
				drc1_knee_ip, drc1_knee_ip_values[drc1_knee_ip]);

	regmap_update_bits(regmap, ARIZONA_DRC1_CTRL4, ARIZONA_DRC1_KNEE_IP_MASK,
			drc1_knee_ip_values[drc1_knee_ip]);
	return 0;
}

static int get_drc1_knee_op(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = drc1_knee_op;
	return 0;
}

static int set_drc1_knee_op(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct regmap *regmap = codec->control_data;

	drc1_knee_op = ucontrol->value.integer.value[0];

	dev_info(codec->dev, "%s: drc1_knee_op mode=%d, val=0x%x\n", __func__,
				drc1_knee_op, drc1_knee_op_values[drc1_knee_op]);

	regmap_update_bits(regmap, ARIZONA_DRC1_CTRL4, ARIZONA_DRC1_KNEE_OP_MASK,
			drc1_knee_op_values[drc1_knee_op]);
	return 0;
}

static int get_drc1_hi_comp(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = drc1_hi_comp;
	return 0;
}

static int set_drc1_hi_comp(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct regmap *regmap = codec->control_data;

	drc1_hi_comp = ucontrol->value.integer.value[0];

	dev_info(codec->dev, "%s: drc1_hi_comp mode=%d, val=0x%x\n", __func__,
				drc1_hi_comp, drc1_hi_comp_values[drc1_hi_comp]);

	regmap_update_bits(regmap, ARIZONA_DRC1_CTRL3, ARIZONA_DRC1_HI_COMP_MASK,
			drc1_hi_comp_values[drc1_hi_comp]);
	return 0;
}

static int get_drc1_lo_comp(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = drc1_lo_comp;
	return 0;
}

static int set_drc1_lo_comp(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct regmap *regmap = codec->control_data;

	drc1_lo_comp = ucontrol->value.integer.value[0];

	dev_info(codec->dev, "%s: drc1_lo_comp mode=%d, val=0x%x\n", __func__,
				drc1_lo_comp, drc1_lo_comp_values[drc1_lo_comp]);

	regmap_update_bits(regmap, ARIZONA_DRC1_CTRL3, ARIZONA_DRC1_LO_COMP_MASK,
			drc1_lo_comp_values[drc1_lo_comp]);
	return 0;
}

static int get_drc1_knee2_ip(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = drc1_knee2_ip;
	return 0;
}

static int set_drc1_knee2_ip(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct regmap *regmap = codec->control_data;

	drc1_knee2_ip = ucontrol->value.integer.value[0];

	dev_info(codec->dev, "%s: drc1_knee2_ip mode=%d, val=0x%x\n", __func__,
				drc1_knee2_ip, drc1_knee2_ip_values[drc1_knee2_ip]);

	regmap_update_bits(regmap, ARIZONA_DRC1_CTRL5, ARIZONA_DRC1_KNEE2_IP_MASK,
			drc1_knee2_ip_values[drc1_knee2_ip]);
	return 0;
}

static int get_drc1_knee2_op(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = drc1_knee2_op;
	return 0;
}

static int set_drc1_knee2_op(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct regmap *regmap = codec->control_data;

	drc1_knee2_op = ucontrol->value.integer.value[0];

	dev_info(codec->dev, "%s: drc1_knee2_op mode=%d, val=0x%x\n", __func__,
				drc1_knee2_op, drc1_knee2_op_values[drc1_knee2_op]);

	regmap_update_bits(regmap, ARIZONA_DRC1_CTRL5, ARIZONA_DRC1_KNEE2_OP_MASK,
			drc1_knee2_op_values[drc1_knee2_op]);
	return 0;
}

static int get_drc1_ng_exp(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = drc1_ng_exp;
	return 0;
}

static int set_drc1_ng_exp(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct regmap *regmap = codec->control_data;

	drc1_ng_exp = ucontrol->value.integer.value[0];

	dev_info(codec->dev, "%s: drc1_ng_exp mode=%d, val=0x%x\n", __func__,
				drc1_ng_exp, drc1_ng_exp_values[drc1_ng_exp]);

	regmap_update_bits(regmap, ARIZONA_DRC1_CTRL3, ARIZONA_DRC1_NG_EXP_MASK,
			drc1_ng_exp_values[drc1_ng_exp]);
	return 0;
}

/******************************************************************/

static int get_aif2_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct wm5102_machine_priv *priv
		= snd_soc_card_get_drvdata(codec->card);

	ucontrol->value.integer.value[0] = priv->aif2mode;
	return 0;
}

static int set_aif2_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct wm5102_machine_priv *priv
		= snd_soc_card_get_drvdata(codec->card);
	struct snd_soc_dai *codec_dai = codec->card->rtd[0].codec_dai;

	if((priv->aif2mode == 1) && (ucontrol->value.integer.value[0] == 0)) {
		int ret;
		ret = snd_soc_dai_set_pll(codec_dai, WM5102_FLL2, 0, 0, 0);
		if (ret != 0)
			dev_err(codec->dev,
					"Failed to stop FLL2: %d\n", ret);
	}

	priv->aif2mode = ucontrol->value.integer.value[0];

	dev_info(codec->dev, "set aif2 mode: %s\n",
					 aif2_mode_text[priv->aif2mode]);
	return  0;
}

static int get_micbias_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct wm5102_machine_priv *priv
		= snd_soc_card_get_drvdata(codec->card);

	ucontrol->value.integer.value[0] = priv->micbias_mode;
	return 0;
}

static int set_micbias_mode(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);
	struct regmap *regmap = codec->control_data;
	micbias_type micbias = ucontrol->value.integer.value[0];

	switch (micbias) {

	case MICBIAS1ON:
		regmap_update_bits(regmap, ARIZONA_MIC_BIAS_CTRL_1, ARIZONA_MICB1_ENA_MASK,
				ARIZONA_MICB1_ENA);
		break;
	case MICBIAS2ON:
		regmap_update_bits(regmap, ARIZONA_MIC_BIAS_CTRL_2, ARIZONA_MICB2_ENA_MASK,
				ARIZONA_MICB2_ENA);
		break;
	case MICBIAS3ON:
		regmap_update_bits(regmap, ARIZONA_MIC_BIAS_CTRL_3, ARIZONA_MICB3_ENA_MASK,
				ARIZONA_MICB3_ENA);
		break;
	case MICBIAS1OFF:
		regmap_update_bits(regmap, ARIZONA_MIC_BIAS_CTRL_1, ARIZONA_MICB1_ENA_MASK,
				0);
		break;
	case MICBIAS2OFF:
		regmap_update_bits(regmap, ARIZONA_MIC_BIAS_CTRL_2, ARIZONA_MICB2_ENA_MASK,
				0);
		break;
	case MICBIAS3OFF:
		regmap_update_bits(regmap, ARIZONA_MIC_BIAS_CTRL_3, ARIZONA_MICB3_ENA_MASK,
				0);
		break;
	default:
		break;
	}
	dev_info(codec->dev, "set micbias mode: %s\n",
					 micbias_mode_text[micbias]);
	return  0;
}

static const struct snd_kcontrol_new adonisuniv_codec_controls[] = {
	SOC_ENUM_EXT("LHPF1 COEFF FILTER", lhpf_filter_mode_enum[0],
		get_lhpf1_coeff, set_lhpf1_coeff),

	SOC_ENUM_EXT("LHPF2 COEFF FILTER", lhpf_filter_mode_enum[0],
		get_lhpf2_coeff, set_lhpf2_coeff),

	SOC_ENUM_EXT("LHPF3 COEFF FILTER", lhpf_filter_mode_enum[0],
		get_lhpf3_coeff, set_lhpf3_coeff),

	SOC_ENUM_EXT("LHPF4 COEFF FILTER", lhpf_filter_mode_enum[0],
		get_lhpf4_coeff, set_lhpf4_coeff),

	SOC_ENUM_EXT("AIF2 Mode", aif2_mode_enum[0],
		get_aif2_mode, set_aif2_mode),

	SOC_ENUM_EXT("MICBIAS Mode", micbias_mode_enum[0],
		get_micbias_mode, set_micbias_mode),

	SOC_ENUM_EXT("DRC1L_ENA", drc1_sw_enum[0],
		get_drc1l_ena_ctl, set_drc1l_ena_ctl),

	SOC_ENUM_EXT("DRC1R_ENA", drc1_sw_enum[0],
		get_drc1r_ena_ctl, set_drc1r_ena_ctl),

	SOC_ENUM_EXT("DRC1_NG_ENA", drc1_sw_enum[0],
		get_drc1_ng_ena_ctl, set_drc1_ng_ena_ctl),

	SOC_ENUM_EXT("DRC1_ANTICLIP", drc1_sw_enum[0],
		get_drc1_anticlip_ctl, set_drc1_anticlip_ctl),

	SOC_ENUM_EXT("DRC1_QR", drc1_sw_enum[0],
		get_drc1_qr_ctl, set_drc1_qr_ctl),

	SOC_ENUM_EXT("DRC1_ATK", drc1_atk_enum[0],
		get_drc1_atk, set_drc1_atk),

	SOC_ENUM_EXT("DRC1_MINGAIN", drc1_mingain_enum[0],
		get_drc1_mingain, set_drc1_mingain),

	SOC_ENUM_EXT("DRC1_MAXGAIN", drc1_maxgain_enum[0],
		get_drc1_maxgain, set_drc1_maxgain),

	SOC_ENUM_EXT("DRC1_QR_THR", drc1_qr_thr_enum[0],
		get_drc1_qr_thr, set_drc1_qr_thr),

	SOC_ENUM_EXT("DRC1_QR_DCY", drc1_qr_dcy_enum[0],
		get_drc1_qr_dcy, set_drc1_qr_dcy),

	SOC_ENUM_EXT("DRC1_KNEE_IP", drc1_knee_ip_enum[0],
		get_drc1_knee_ip, set_drc1_knee_ip),

	SOC_ENUM_EXT("DRC1_KNEE_OP", drc1_knee_op_enum[0],
		get_drc1_knee_op, set_drc1_knee_op),

	SOC_ENUM_EXT("DRC1_HI_COMP", drc1_hi_comp_enum[0],
		get_drc1_hi_comp, set_drc1_hi_comp),

	SOC_ENUM_EXT("DRC1_LO_COMP", drc1_lo_comp_enum[0],
		get_drc1_lo_comp, set_drc1_lo_comp),

	SOC_ENUM_EXT("DRC1_NG_MINGAIN", drc1_ng_mingain_enum[0],
		get_drc1_ng_mingain, set_drc1_ng_mingain),

	SOC_ENUM_EXT("DRC1_KNEE2_IP", drc1_knee2_ip_enum[0],
		get_drc1_knee2_ip, set_drc1_knee2_ip),

	SOC_ENUM_EXT("DRC1_KNEE2_OP_ENA", drc1_sw_enum[0],
		get_drc1_knee2_op_ena_ctl, set_drc1_knee2_op_ena_ctl),

	SOC_ENUM_EXT("DRC1_NG_EXP", drc1_ng_exp_enum[0],
		get_drc1_ng_exp, set_drc1_ng_exp),

	SOC_ENUM_EXT("DRC1_KNEE2_OP", drc1_knee2_op_enum[0],
		get_drc1_knee2_op, set_drc1_knee2_op),
};

static const struct snd_kcontrol_new adonisuniv_controls[] = {
	SOC_DAPM_PIN_SWITCH("HP"),
	SOC_DAPM_PIN_SWITCH("SPK"),
	SOC_DAPM_PIN_SWITCH("RCV"),
	SOC_DAPM_PIN_SWITCH("VPS"),
	SOC_DAPM_PIN_SWITCH("HDMI"),
	SOC_DAPM_PIN_SWITCH("Main Mic"),
	SOC_DAPM_PIN_SWITCH("Sub Mic"),
	SOC_DAPM_PIN_SWITCH("3rd Mic"),
	SOC_DAPM_PIN_SWITCH("Headset Mic"),
};

const struct snd_soc_dapm_widget adonisuniv_dapm_widgets[] = {
	SND_SOC_DAPM_OUTPUT("HDMIL"),
	SND_SOC_DAPM_OUTPUT("HDMIR"),
	SND_SOC_DAPM_HP("HP", NULL),
	SND_SOC_DAPM_SPK("SPK", NULL),
	SND_SOC_DAPM_SPK("RCV", NULL),
	SND_SOC_DAPM_LINE("VPS", NULL),
	SND_SOC_DAPM_LINE("HDMI", NULL),

	SND_SOC_DAPM_MIC("Headset Mic", NULL),
	SND_SOC_DAPM_MIC("Main Mic", adonisuniv_ext_mainmicbias),
	SND_SOC_DAPM_MIC("Sub Mic", adonisuniv_ext_submicbias),
	SND_SOC_DAPM_MIC("3rd Mic", NULL),
};

const struct snd_soc_dapm_route adonisuniv_dapm_routes[] = {
	{ "HDMIL", NULL, "AIF1RX1" },
	{ "HDMIR", NULL, "AIF1RX2" },
	{ "HDMI", NULL, "HDMIL" },
	{ "HDMI", NULL, "HDMIR" },

	{ "HP", NULL, "HPOUT1L" },
	{ "HP", NULL, "HPOUT1R" },

	{ "SPK", NULL, "SPKOUTLN" },
	{ "SPK", NULL, "SPKOUTLP" },
	{ "SPK", NULL, "SPKOUTRN" },
	{ "SPK", NULL, "SPKOUTRP" },

	{ "VPS", NULL, "HPOUT2L" },
	{ "VPS", NULL, "HPOUT2R" },

	{ "RCV", NULL, "EPOUTN" },
	{ "RCV", NULL, "EPOUTP" },

	{ "IN1L", NULL, "Main Mic" },
	{ "Main Mic", NULL, "MICVDD" },

	{ "Headset Mic", NULL, "MICBIAS1" },
	{ "IN1R", NULL, "Headset Mic" },

	{ "Sub Mic", NULL, "MICBIAS3" },
	{ "IN2L", NULL, "Sub Mic" },

	{ "3rd Mic", NULL, "MICBIAS2" },
	{ "IN2R", NULL, "3rd Mic" },
};

static int adonisuniv_wm5102_aif1_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_codec *codec = rtd->codec;
	struct wm5102_machine_priv *priv =
					snd_soc_card_get_drvdata(codec->card);
	int ret;

	dev_info(codec_dai->dev, "aif1: %dch, %dHz, %dbytes\n",
						params_channels(params),
						params_rate(params),
						params_buffer_bytes(params));

	priv->aif1rate = params_rate(params);

	adonisuniv_set_media_clocking(priv);

	ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_I2S
					| SND_SOC_DAIFMT_NB_NF
					| SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		dev_err(codec_dai->dev,
			"Failed to set audio format in codec: %d\n", ret);
		return ret;
	}

	/* Set the cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S
					| SND_SOC_DAIFMT_NB_NF
					| SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		dev_err(codec_dai->dev,
			"Failed to set audio format in cpu: %d\n", ret);
		return ret;
	}

	ret = snd_soc_dai_set_sysclk(cpu_dai, SAMSUNG_I2S_OPCLK,
				     0, MOD_OPCLK_PCLK);
	if (ret < 0) {
		dev_err(codec_dai->dev,
			"Failed to set system clock in cpu: %d\n", ret);
		return ret;
	}

	return 0;
}

static int adonisuniv_wm5102_aif1_hw_free(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_card *card = rtd->card;

	dev_info(card->dev, "%s\n", __func__);

	return 0;
}

/*
 * AdnoisUniv wm5102 DAI operations.
 */
static struct snd_soc_ops adonisuniv_wm5102_aif1_ops = {
	.hw_params = adonisuniv_wm5102_aif1_hw_params,
	.hw_free = adonisuniv_wm5102_aif1_hw_free,
};

static int adonisuniv_wm5102_aif2_hw_params(struct snd_pcm_substream *substream,
					struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct wm5102_machine_priv *priv = snd_soc_card_get_drvdata(rtd->card);
	int ret;
	int bclk;

	dev_info(codec_dai->dev, "aif2: %dch, %dHz, %dbytes\n",
						params_channels(params),
						params_rate(params),
						params_buffer_bytes(params));

	if (priv->aif2rate != params_rate(params)) {
		ret = snd_soc_dai_set_pll(codec_dai, WM5102_FLL2, 0, 0, 0);
		if (ret != 0)
			dev_err(codec_dai->dev,
					"Failed to stop FLL2: %d\n", ret);
		priv->aif2rate = params_rate(params);
	}

	switch (priv->aif2rate) {
	case 8000:
		bclk = 256000;
		break;
	case 16000:
		bclk = 512000;
		break;
	default:
		dev_warn(codec_dai->dev,
				"Unsupported LRCLK %d, falling back to 8000Hz\n",
				(int)params_rate(params));
		bclk = 256000;
	}

	/* Set the codec DAI configuration, aif2_mode:0 is slave */

	if (priv->aif2mode == 0)
		ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_I2S
					| SND_SOC_DAIFMT_NB_NF
					| SND_SOC_DAIFMT_CBS_CFS);
	else
		ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_I2S
					| SND_SOC_DAIFMT_NB_NF
					| SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		dev_err(codec_dai->dev,
			"Failed to set audio format in codec: %d\n", ret);
		return ret;
	}

	if (priv->aif2mode  == 0) {
		ret = snd_soc_dai_set_pll(codec_dai, WM5102_FLL2_REFCLK,
					  ARIZONA_FLL_SRC_MCLK1,
					  ADONISUNIV_DEFAULT_MCLK1,
					  ADONISUNIV_TELCLK_RATE);
		if (ret != 0) {
			dev_err(codec_dai->dev,
					"Failed to start FLL2 REF: %d\n", ret);
			return ret;
		}

		ret = snd_soc_dai_set_pll(codec_dai, WM5102_FLL2,
					  ARIZONA_FLL_SRC_AIF2BCLK,
					  bclk,
					  ADONISUNIV_TELCLK_RATE);
		if (ret != 0) {
			dev_err(codec_dai->dev,
					 "Failed to start FLL2%d\n", ret);
			return ret;
		}
	} else {
		ret = snd_soc_dai_set_pll(codec_dai, WM5102_FLL2, 0, 0, 0);
		if (ret != 0)
			dev_err(codec_dai->dev,
					"Failed to stop FLL2: %d\n", ret);

		ret = snd_soc_dai_set_pll(codec_dai, WM5102_FLL2_REFCLK,
					  ARIZONA_FLL_SRC_NONE, 0, 0);
		if (ret != 0) {
			dev_err(codec_dai->dev,
				 "Failed to start FLL2 REF: %d\n", ret);
			return ret;
		}
		ret = snd_soc_dai_set_pll(codec_dai, WM5102_FLL2,
					  ARIZONA_CLK_SRC_MCLK1,
					  ADONISUNIV_DEFAULT_MCLK1,
					  ADONISUNIV_TELCLK_RATE);
		if (ret != 0) {
			dev_err(codec_dai->dev,
					"Failed to start FLL2: %d\n", ret);
			return ret;
		}
	}

	return 0;
}

static struct snd_soc_ops adonisuniv_wm5102_aif2_ops = {
	.hw_params = adonisuniv_wm5102_aif2_hw_params,
};

static int adonisuniv_wm5102_aif3_hw_params(struct snd_pcm_substream *substream,
					struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	int ret;

	dev_info(codec_dai->dev, "aif3: %dch, %dHz, %dbytes\n",
						params_channels(params),
						params_rate(params),
						params_buffer_bytes(params));

	ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_I2S
				| SND_SOC_DAIFMT_NB_NF
				| SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0) {
		dev_err(codec_dai->dev, "Failed to set BT mode: %d\n", ret);
		return ret;
	}

	return 0;
}

static struct snd_soc_ops adonisuniv_wm5102_aif3_ops = {
	.hw_params = adonisuniv_wm5102_aif3_hw_params,
};

static int adonisuniv_late_probe(struct snd_soc_card *card)
{
	struct snd_soc_codec *codec = card->rtd[0].codec;
	struct snd_soc_dai *codec_dai = card->rtd[0].codec_dai;
	struct snd_soc_dai *cpu_dai = card->rtd[0].cpu_dai;
	struct wm5102_machine_priv *priv
		= snd_soc_card_get_drvdata(codec->card);
	int i, ret;

	priv->codec = codec;
	the_codec = codec;

	for (i = 0; i < 3; i++)
		priv->aif[i] = card->rtd[i].codec_dai;

	codec_dai->driver->playback.channels_max =
				cpu_dai->driver->playback.channels_max;
	/* close codec device immediately when pcm is closed */
	codec->ignore_pmdown_time = true;

	snd_soc_dapm_ignore_suspend(&codec->dapm, "RCV");
	snd_soc_dapm_ignore_suspend(&codec->dapm, "VPS");
	snd_soc_dapm_ignore_suspend(&codec->dapm, "SPK");
	snd_soc_dapm_ignore_suspend(&codec->dapm, "HP");
	snd_soc_dapm_ignore_suspend(&codec->dapm, "AIF2 Playback");
	snd_soc_dapm_ignore_suspend(&codec->dapm, "AIF2 Capture");
	snd_soc_dapm_ignore_suspend(&codec->dapm, "AIF3 Playback");
	snd_soc_dapm_ignore_suspend(&codec->dapm, "AIF3 Capture");

	adonisuniv_set_media_clocking(priv);

	ret = snd_soc_add_codec_controls(codec, adonisuniv_codec_controls,
					ARRAY_SIZE(adonisuniv_codec_controls));
	if (ret < 0) {
		dev_err(codec->dev,
				"Failed to add controls to codec: %d\n", ret);
		return ret;
	}

	dev_info(codec->dev, "%s: Successfully created\n", __func__);

	return snd_soc_dapm_sync(&codec->dapm);
}

static int adonisuniv_suspend_post(struct snd_soc_card *card)
{
	struct snd_soc_codec *codec = card->rtd[0].codec;
	int ret;

	if (codec->active) {
		dev_info(codec->dev, "sound card is still active state");
		return 0;
	}

	ret = snd_soc_codec_set_pll(codec, WM5102_FLL1,
				    ARIZONA_CLK_SRC_MCLK1,
				    ADONISUNIV_DEFAULT_MCLK1, 0);
	if (ret != 0) {
		dev_err(codec->dev, "Failed to stop FLL1: %d\n", ret);
		return ret;
	}

	ret = snd_soc_codec_set_pll(codec, WM5102_FLL2, 0, 0, 0);
	if (ret != 0) {
		dev_err(codec->dev, "Failed to stop FLL2: %d\n", ret);
		return ret;
	}

	ret = snd_soc_codec_set_sysclk(codec, ARIZONA_CLK_SYSCLK, 0, 0, 0);
	if (ret != 0) {
		dev_err(codec->dev, "Failed to stop SYSCLK: %d\n", ret);
		return ret;
	}

	exynos5_audio_set_mclk(0, 1);

	return 0;
}

static int adonisuniv_resume_pre(struct snd_soc_card *card)
{
	struct wm5102_machine_priv *wm5102_priv
					 = snd_soc_card_get_drvdata(card);

	exynos5_audio_set_mclk(1, 0);

	adonisuniv_set_media_clocking(wm5102_priv);

	return 0;
}

static struct snd_soc_dai_driver adonisuniv_ext_dai[] = {
	{
		.name = "adonisuniv.cp",
		.playback = {
			.channels_min = 1,
			.channels_max = 2,
			.rate_min = 8000,
			.rate_max = 48000,
			.rates = (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_48000),
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
		},
		.capture = {
			.channels_min = 1,
			.channels_max = 2,
			.rate_min = 8000,
			.rate_max = 48000,
			.rates = (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 |
				SNDRV_PCM_RATE_48000),
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
		},
	},
	{
		.name = "adonisuniv.bt",
		.playback = {
			.channels_min = 1,
			.channels_max = 2,
			.rate_min = 8000,
			.rate_max = 16000,
			.rates = (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000),
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
		},
		.capture = {
			.channels_min = 1,
			.channels_max = 2,
			.rate_min = 8000,
			.rate_max = 16000,
			.rates = (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000),
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
		},
	},
};

static struct snd_soc_dai_link adonisuniv_dai[] = {
	{
		.name = "AdonisUniv_WM5102 Multi Ch",
		.stream_name = "Pri_Dai",
		.cpu_dai_name = "samsung-i2s.0",
		.codec_dai_name = "wm5102-aif1",
		.platform_name = "samsung-audio",
		.codec_name = "wm5102-codec",
		.ops = &adonisuniv_wm5102_aif1_ops,
	},
	{
		.name = "AdonisUniv_WM5102 Voice",
		.stream_name = "Voice Tx/Rx",
		.cpu_dai_name = "adonisuniv.cp",
		.codec_dai_name = "wm5102-aif2",
		.platform_name = "snd-soc-dummy",
		.codec_name = "wm5102-codec",
		.ops = &adonisuniv_wm5102_aif2_ops,
		.ignore_suspend = 1,
	},
	{
		.name = "AdonisUniv_WM5102 BT",
		.stream_name = "BT Tx/Rx",
		.cpu_dai_name = "adonisuniv.bt",
		.codec_dai_name = "wm5102-aif3",
		.platform_name = "snd-soc-dummy",
		.codec_name = "wm5102-codec",
		.ops = &adonisuniv_wm5102_aif3_ops,
		.ignore_suspend = 1,
	},
	{
		.name = "AdonisUniv_WM5102 Playback",
		.stream_name = "Sec_Dai",
		.cpu_dai_name = "samsung-i2s.4",
		.codec_dai_name = "wm5102-aif1",
#ifdef CONFIG_SND_SAMSUNG_USE_IDMA
		.platform_name = "samsung-idma",
#else
		.platform_name = "samsung-audio",
#endif
		.codec_name = "wm5102-codec",
		.ops = &adonisuniv_wm5102_aif1_ops,
	},
};

static struct snd_soc_card adonisuniv = {
	.name = "AdonisUniv Sound Card",
	.owner = THIS_MODULE,

	.dai_link = adonisuniv_dai,
	.num_links = ARRAY_SIZE(adonisuniv_dai),

	.controls = adonisuniv_controls,
	.num_controls = ARRAY_SIZE(adonisuniv_controls),
	.dapm_widgets = adonisuniv_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(adonisuniv_dapm_widgets),
	.dapm_routes = adonisuniv_dapm_routes,
	.num_dapm_routes = ARRAY_SIZE(adonisuniv_dapm_routes),

	.late_probe = adonisuniv_late_probe,

	.suspend_post = adonisuniv_suspend_post,
	.resume_pre = adonisuniv_resume_pre,

#ifdef USE_BIAS_LEVEL_POST
	.set_bias_level = adonisuniv_set_bias_level,
	.set_bias_level_post = adonisuniv_set_bias_level_post,
#endif
};

static int __devinit snd_adonisuniv_probe(struct platform_device *pdev)
{
	int ret;
	struct wm5102_machine_priv *wm5102;

	wm5102 = kzalloc(sizeof *wm5102, GFP_KERNEL);
	if (!wm5102) {
		pr_err("Failed to allocate memory\n");
		return -ENOMEM;
	}

	exynos5_audio_set_mclk(1, 0);

	ret = snd_soc_register_dais(&pdev->dev, adonisuniv_ext_dai,
					ARRAY_SIZE(adonisuniv_ext_dai));
	if (ret != 0)
		pr_err("Failed to register external DAIs: %d\n", ret);

	snd_soc_card_set_drvdata(&adonisuniv, wm5102);

	adonisuniv.dev = &pdev->dev;
	ret = snd_soc_register_card(&adonisuniv);
	if (ret) {
		dev_err(&pdev->dev, "snd_soc_register_card failed %d\n", ret);
		kfree(wm5102);
	}

	adonisuniv_gpio_init();

	return ret;
}

static int __devexit snd_adonisuniv_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = &adonisuniv;
	struct wm5102_machine_priv *wm5102 = snd_soc_card_get_drvdata(card);

	snd_soc_unregister_card(&adonisuniv);
	kfree(wm5102);

	exynos5_audio_set_mclk(0, 0);

#ifdef GPIO_MICBIAS_EN
	gpio_free(GPIO_MICBIAS_EN);
#endif
#ifdef GPIO_SUB_MICBIAS_EN
	gpio_free(GPIO_SUB_MICBIAS_EN);
#endif

	return 0;
}

static struct platform_driver snd_adonisuniv_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "wm5102-card",
		.pm = &snd_soc_pm_ops,
	},
	.probe = snd_adonisuniv_probe,
	.remove = __devexit_p(snd_adonisuniv_remove),
};

module_platform_driver(snd_adonisuniv_driver);

MODULE_AUTHOR("JS. Park <aitdark.park@samsung.com>");
MODULE_DESCRIPTION("ALSA SoC AdonisUniv wm5102");
MODULE_LICENSE("GPL");
