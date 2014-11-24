#include <linux/kernel.h>
#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>
#include <linux/gpio.h>
#include <linux/firmware.h>
#include <linux/delay.h>
#include <linux/input.h>

#include "cypress_touchkey.h"

#define ISP_SPEED_UP

#define TC380_FW_FLASH_RETRY	5
#define TC380_POWERON_DELAY	100
#define TC380_DCR_RD_RETRY	50
#define TC380_FW_VER_READ	5

#define TC380_KEY_DATA		0x00
#define TC380_KEY_INDEX_MASK	0x03
#define TC380_KEY_PRESS_MASK	0x08

#define TC380_CMD		0x00
#define TC380_CMD_FW_VER	0x01
#define TC380_CMD_RAW_MENU	0x04
#define TC380_CMD_RAW_BACK	0x0C

#define TC380_RAW_DIFFDATA_OFFSET	0x04
#define TC380_RAW_CHPCT_OFFSET		0x02
#define TC380_RAW_RAWDATA_OFFSET	0x06
#define TC380_RAW_DATA_SIZE		0x08

#define TC380_ISP_ACK			1
#define TC380_ISP_SP_SIGNAL		0b010101011111000
#define TC380_NUM_OF_ISP_SP_SIGNAL	15

#define TC380_CMD_LED_ON		0x10
#define TC380_CMD_LED_OFF		0x20
#define TC380_CMD_SLEEP			0x80

#define TC380_FW_ER_MAX_LEN		0X8000
#define TC380_FW_WT_MAX_LEN		0X3000

enum {
	STATE_NORMAL = 1,
	STATE_FLASH,
	STATE_FLASH_FAIL,
};

struct fdata_struct {
	struct device			*dummy_dev;
	u8				fw_flash_status;
};

static inline void setsda(struct touchkey_i2c *data, int state)
{
	if (state)
		gpio_direction_input(data->pdata->gpio_sda);
	else
		gpio_direction_output(data->pdata->gpio_sda, 0);
}

static inline void setscl(struct touchkey_i2c *data, int state)
{
	if (state)
		gpio_direction_input(data->pdata->gpio_scl);
	else
		gpio_direction_output(data->pdata->gpio_scl, 0);
}

static inline int getsda(struct touchkey_i2c *data)
{
	return gpio_get_value(data->pdata->gpio_sda);
}

static inline int getscl(struct touchkey_i2c *data)
{
	return gpio_get_value(data->pdata->gpio_scl);
}

static inline void sdalo(struct touchkey_i2c *data)
{
	setsda(data, 0);
	udelay((data->pdata->udelay + 1) / 2);
}

static inline void sdahi(struct touchkey_i2c *data)
{
	setsda(data, 1);
	udelay((data->pdata->udelay + 1) / 2);
}

static inline void scllo(struct touchkey_i2c *data)
{
	setscl(data, 0);
	udelay((data->pdata->udelay + 1) / 2);
}

static int sclhi(struct touchkey_i2c *data)
{
	int i;
	
	setscl(data, 1);

	for (i = 0 ; i < 20; ++i) {
		if (getscl(data))
			break;
		udelay(10);
	}

	udelay(data->pdata->udelay);

	return 0;
}

static void isp_start(struct touchkey_i2c *data)
{
	setsda(data, 0);
	udelay(data->pdata->udelay);
	scllo(data);
}

static void isp_stop(struct touchkey_i2c *data)
{
	sdalo(data);
	sclhi(data);
	setsda(data, 1);
	udelay(data->pdata->udelay);
}

static int isp_recvdbyte(struct touchkey_i2c *data)
{
	struct i2c_client *client = data->client;
	int i;
	u8 indata = 0;
	sdahi(data);
	for (i = 0; i < 8 ; i++) {
		if (sclhi(data) < 0) { /* timed out */
			dev_err(&client->dev, "%s: timeout at bit "
				"#%d\n", __func__, 7 - i);
			return -ETIMEDOUT;
		}

		indata = indata << 1;
		if (getsda(data))
			indata |= 0x01;

		setscl(data, 0);

		udelay(i == 7 ? data->pdata->udelay / 2 : data->pdata->udelay);
	}
	return indata;
}

static int isp_sendbyte(struct touchkey_i2c *data, u8 c)
{
	struct i2c_client *client = data->client;
	int i;
	int sb;
	int ack = 0;

	/* assert: scl is low */
	for (i = 7; i >= 0; i--) {
		sb = (c >> i) & 0x1;
		setsda(data, sb);
		udelay((data->pdata->udelay + 1) / 2);

		if (sclhi(data) < 0) { /* timed out */
			dev_err(&client->dev, "%s: %#x, timeout at bit #%d\n",
				__func__, (int)c, i);
			return -ETIMEDOUT;
		}
		scllo(data);
	}
	sdahi(data);

	if (sclhi(data) < 0) { /* timed out */
		dev_err(&client->dev, "%s: %#x, timeout at bit #%d\n",
			__func__, (int)c, i);
		return -ETIMEDOUT;
	}

	ack = !getsda(data);

	scllo(data);

#if defined(ISP_VERY_VERBOSE_DEBUG)
	dev_info(&client->dev, "%s: %#x %s\n", __func__, (int)c,
		 ack ? "A" : "NA");
#endif
	return ack;
}

static int isp_master_recv(struct touchkey_i2c *data, u8 addr, u8 *val)
{
	struct i2c_client *client = data->client;
	int ret;
	int retries = 2;

retry:
	isp_start(data);

	ret = isp_sendbyte(data, addr);
	if (ret != TC380_ISP_ACK) {
		dev_err(&client->dev, "%s: %#x %s\n", __func__, addr, "NA");
		if (retries-- > 0) {
			dev_err(&client->dev, "%s: retry (%d)\n", __func__,
				retries);
			goto retry;
		}
		return -EIO;
	}
	*val = isp_recvdbyte(data);
	isp_stop(data);

	return 0;
}

static int isp_master_send(struct touchkey_i2c *data, u8 msg_1, u8 msg_2)
{
	struct i2c_client *client = data->client;
	int ret;
	int retries = 2;

retry:
	isp_start(data);
	ret = isp_sendbyte(data, msg_1);
	if (ret != TC380_ISP_ACK) {
		dev_err(&client->dev, "%s: %#x %s\n", __func__, msg_1, "NA");
		if (retries-- > 0) {
			dev_err(&client->dev, "%s: retry (%d)\n", __func__,
				retries);
			goto retry;
		}
		return -EIO;
	}
	ret = isp_sendbyte(data, msg_2);
	if (ret != TC380_ISP_ACK) {
		dev_err(&client->dev, "%s: %#x %s\n", __func__, msg_2, "NA");
		if (retries-- > 0) {
			dev_err(&client->dev, "%s: retry (%d)\n", __func__,
				retries);
			goto retry;
		}
		return -EIO;
	}
	isp_stop(data);

	return 0;
}

static void isp_sp_signal(struct touchkey_i2c *data)
{
	int i;
	unsigned long flags;

	local_irq_save(flags);
	for (i = TC380_NUM_OF_ISP_SP_SIGNAL - 1; i >= 0; i--) {
		int sb = (TC380_ISP_SP_SIGNAL >> i) & 0x1;
		setscl(data, sb);
		udelay(3);
		setsda(data, 0);
		udelay(10);
		setsda(data, 1);
		udelay(10);

		if (i == 5)
			udelay(30);
	}

	sclhi(data);
	local_irq_restore(flags);
}

static int raw_dbgir3(struct touchkey_i2c *data, u8 data2, u8 data1, u8 data0)
{
	struct i2c_client *client = data->client;
	int ret = 0;

	ret = isp_master_send(data, 0xc2, data2);
	if (ret < 0)
		goto err_isp_master_send;
	ret = isp_master_send(data, 0xc4, data1);
	if (ret < 0)
		goto err_isp_master_send;
	ret = isp_master_send(data, 0xc6, data0);
	if (ret < 0)
		goto err_isp_master_send;
	ret = isp_master_send(data, 0xc0, 0x80);
	if (ret < 0)
		goto err_isp_master_send;

	return 0;
err_isp_master_send:
	dev_err(&client->dev, "fail to dbgir3 %#x,%#x,%#x (%d)\n",
		data2, data1, data0, ret);
	return ret;
}

static int raw_dbgir2(struct touchkey_i2c *data, u8 data1, u8 data0)
{
	struct i2c_client *client = data->client;
	int ret = 0;

	ret = isp_master_send(data, 0xc2, data1);
	if (ret < 0)
		goto err_raw_dbir2;
	ret = isp_master_send(data, 0xc4, data0);
	if (ret < 0)
		goto err_raw_dbir2;
	ret = isp_master_send(data, 0xc0, 0x80);
	if (ret < 0)
		goto err_raw_dbir2;

	return 0;
err_raw_dbir2:
	dev_err(&client->dev, "fail to dbgir2 %#x,%#x (%d)\n",
		data1, data0, ret);
	return ret;
}

static int raw_spchl(struct touchkey_i2c *data, u8 data1, u8 data0)
{
	struct i2c_client *client = data->client;
	int ret = 0;

	ret = isp_master_send(data, 0xd0, data1);
	ret = isp_master_send(data, 0xd2, data0);

	if (ret < 0) {
		dev_err(&client->dev, "fail to spchl %#x,%#x (%d)\n",
			data1, data0, ret);
		return ret;
	}

	return 0;
}

static int isp_common_set(struct touchkey_i2c *data)
{
	struct i2c_client *client = data->client;
	int ret;
	int i;
	int size;
	u8 cmd[][3] = {
		{0x75, 0x8f, 0x00},
		{0x75, 0xc6, 0x0e},
		{0x75, 0xf7, 0xc1},
		{0x75, 0xf7, 0x1e},
		{0x75, 0xf7, 0xec},
		{0x75, 0xf7, 0x81},
	};

	size = ARRAY_SIZE(cmd);
	for (i = 0 ; i < size; ++i) {
		ret = raw_dbgir3(data, cmd[i][0], cmd[i][1], cmd[i][2]);

		if (ret < 0)
			goto err_write_common_set;
	}

	return 0;

err_write_common_set:
	dev_err(&client->dev, "fail to %s (%d)\n", __func__, ret);
	return ret;
}

static int isp_ers_timing_set(struct touchkey_i2c *data)
{
	struct i2c_client *client = data->client;
	int ret = 0;
	int i;
	int size;
	u8 cmd[][3] = {
		{0x75, 0xf2, 0x90},
		{0x75, 0xf3, 0xD0},
		{0x75, 0xf4, 0x03},
	};

	size = ARRAY_SIZE(cmd);
	for (i = 0 ; i < size; ++i) {
		ret = raw_dbgir3(data, cmd[i][0], cmd[i][1], cmd[i][2]);

		if (ret < 0)
			goto err_ers_timing_set;
	}

	return 0;
err_ers_timing_set:
	dev_err(&client->dev, "fail to %s (%d)\n", __func__, ret);
	return ret;
}

static int isp_pgm_timing_set(struct touchkey_i2c *data)
{
	struct i2c_client *client = data->client;
	int ret = 0;
	int i;
	int size;
	u8 cmd[][3] = {
		{0x75, 0xf2, 0x94},
		{0x75, 0xf3, 0x01},
		{0x75, 0xf4, 0x00},
	};

	size = ARRAY_SIZE(cmd);
	for (i = 0 ; i < size; ++i) {
		ret = raw_dbgir3(data, cmd[i][0], cmd[i][1], cmd[i][2]);

		if (ret < 0)
			goto err_pgm_timing_set;
	}

	return 0;
err_pgm_timing_set:
	dev_err(&client->dev, "fail to %s (%d)\n", __func__, ret);
	return ret;
}

static void reset_for_isp(struct touchkey_i2c *data)
{
	data->pdata->suspend();

	gpio_direction_output(data->pdata->gpio_scl, 0);
	gpio_direction_output(data->pdata->gpio_sda, 0);
	gpio_direction_output(data->pdata->gpio_int, 0);

	msleep(TC380_POWERON_DELAY);

	gpio_direction_output(data->pdata->gpio_scl, 1);
	gpio_direction_output(data->pdata->gpio_sda, 1);
	gpio_direction_input(data->pdata->gpio_int);

	data->pdata->resume();
	usleep_range(4700, 5000);
}

static int tc380_erase_fw(struct touchkey_i2c *data)
{
	struct i2c_client *client = data->client;
	int ret;
	u16 addr = 0;
	int dcr_rd_cnt;
	u8 val;

	reset_for_isp(data);

	isp_sp_signal(data);
	ret = isp_common_set(data);
	if (ret < 0) {
		dev_err(&client->dev, "fail to %s (%d)\n", __func__, ret);
		return ret;
	}

	ret = isp_ers_timing_set(data);
	if (ret < 0) {
		dev_err(&client->dev, "fail to %s (%d)\n", __func__, ret);
		return ret;
	}

	/* set break point */
	ret = isp_master_send(data, 0xf8, 0x01);
	if (ret < 0) {
		dev_err(&client->dev, "fail to %s (%d)\n", __func__, ret);
		return ret;
	}
	ret = isp_master_send(data, 0xc8, 0xff);
	if (ret < 0) {
		dev_err(&client->dev, "fail to %s (%d)\n", __func__, ret);
		return ret;
	}
	ret = isp_master_send(data, 0xca, 0x42);
	if (ret < 0) {
		dev_err(&client->dev, "fail to %s (%d)\n", __func__, ret);
		return ret;
	}

	/* erase */
	while (addr < TC380_FW_ER_MAX_LEN) {
#if defined(ISP_DEBUG)
		dev_info(&client->dev, "fw erase addr=x0%4x\n", addr);
#endif
		ret = raw_dbgir3(data, 0x75, 0xf1, 0x80);
		if (ret < 0) {
			dev_err(&client->dev, "fail to %s (%d)\n", __func__, ret);
			return ret;
		}
		ret = raw_dbgir3(data, 0x90, (u8)(addr >> 8), (u8)(addr & 0xff));
		if (ret < 0) {
			dev_err(&client->dev, "fail to %s (%d)\n", __func__, ret);
			return ret;
		}

		raw_spchl(data, 0xff, 0x3a);
		isp_master_send(data, 0xc0, 0x14);

		val = 0;
		dcr_rd_cnt = TC380_DCR_RD_RETRY;
		do {
			isp_master_recv(data, 0xc1, &val);
			if (dcr_rd_cnt-- < 0) {
				dev_err(&client->dev, "%s: fail to update "
					"dcr\n", __func__);
				return -ENOSYS;
			}
			usleep_range(10000, 15000);
		} while (val != 0x12);
#if defined(ISP_VERBOSE_DEBUG)
			dev_info(&client->dev, "dcr_rd_cnt=%d\n", dcr_rd_cnt);
#endif
		addr += 0x400;
	}

	return 0;
}

static int tc380_write_fw(struct touchkey_i2c *data)
{
	struct i2c_client *client = data->client;
	u16 addr = 0;
	int dcr_rd_cnt;
	u8 val;
	int ret;

	reset_for_isp(data);

	isp_sp_signal(data);
	ret = isp_common_set(data);
	if (ret < 0) {
		dev_err(&client->dev, "fail to %s (%d)\n", __func__, ret);
		return ret;
	}

	dev_info(&client->dev, "start addr = %#x fw_len = %#x\n",
		addr, data->fw_img->fw_len);

	ret = isp_pgm_timing_set(data);
	if (ret < 0) {
		dev_err(&client->dev, "fail to %s (%d)\n", __func__, ret);
		return ret;
	}

	/* set break point */
	ret = isp_master_send(data, 0xf8, 0x01);
	if (ret < 0) {
		dev_err(&client->dev, "fail to %s (%d)\n", __func__, ret);
		return ret;
	}
	ret = isp_master_send(data, 0xc8, 0xff);
	if (ret < 0) {
		dev_err(&client->dev, "fail to %s (%d)\n", __func__, ret);
		return ret;
	}
	ret = isp_master_send(data, 0xca, 0x20);
	if (ret < 0) {
		dev_err(&client->dev, "fail to %s (%d)\n", __func__, ret);
		return ret;
	}

	ret = raw_dbgir3(data, 0x90, (u8)(addr >> 8), (u8)(addr & 0xff));
	if (ret < 0) {
		dev_err(&client->dev, "fail to %s (%d)\n", __func__, ret);
		return ret;
	}
	ret = raw_spchl(data, 0xff, 0x1e);
	if (ret < 0) {
		dev_err(&client->dev, "fail to %s (%d)\n", __func__, ret);
		return ret;
	}

	while (addr < data->fw_img->fw_len) {
		u8 __fw_data = data->fw_img->data[addr];
#if defined(ISP_DEBUG)
		dev_info(&client->dev, "fw write addr=%#x\n", addr);
#endif	
		ret = raw_dbgir2(data, 0x74, __fw_data);
		if (ret < 0) {
			dev_err(&client->dev, "fail to %s (%d)\n", __func__, ret);
			return ret;
		}
		ret = raw_dbgir3(data, 0x75, 0xf1, 0x80);
		if (ret < 0) {
			dev_err(&client->dev, "fail to %s (%d)\n", __func__, ret);
			return ret;
		}
		ret = isp_master_send(data, 0xc0, 0x14);
		if (ret < 0) {
			dev_err(&client->dev, "fail to %s (%d)\n", __func__, ret);
			return ret;
		}
#if !defined(ISP_SPEED_UP)
		val = 0;
		dcr_rd_cnt = TC380_DCR_RD_RETRY;
		do {
			isp_master_recv(data, 0xc1, &val);
			if (dcr_rd_cnt-- < 0) {
				dev_err(&client->dev, "%s: fail to "
					"update dcr\n", __func__);
				return -ENOSYS;
			}
			usleep_range(900, 1000);
		} while (val != 0x12);
#endif
		isp_master_recv(data, 0xd9, &val);

		if (data->fw_img->data[addr] != val) {
			dev_err(&client->dev, "fail to verify at %#x (%#x)\n",
				addr, data->fw_img->data[addr]);
			return -EIO;
		}

#if defined(ISP_VERBOSE_DEBUG)
		dev_info(&client->dev, "dcr_rd_cnt=%d\n", dcr_rd_cnt);
#endif

		/* increase address */
		isp_master_send(data, 0xc0, 0x08);
		addr++;
	}

	return 0;
}

static int tc380_verify_fw(struct touchkey_i2c *data)
{
	struct i2c_client *client = data->client;
	u16 addr = 0;
	int dcr_rd_cnt;
	u8 val;
	int ret;

	reset_for_isp(data);

	isp_sp_signal(data);
	ret = isp_common_set(data);
	if (ret < 0) {
		dev_err(&client->dev, "fail to %s (%d)\n", __func__, ret);
		return ret;
	}

	ret = isp_master_send(data, 0xf8, 0x01);
	if (ret < 0) {
		dev_err(&client->dev, "fail to %s (%d)\n", __func__, ret);
		return ret;
	}
	ret = isp_master_send(data, 0xc8, 0xff);
	if (ret < 0) {
		dev_err(&client->dev, "fail to %s (%d)\n", __func__, ret);
		return ret;
	}
	ret = isp_master_send(data, 0xca, 0x2a);
	if (ret < 0) {
		dev_err(&client->dev, "fail to %s (%d)\n", __func__, ret);
		return ret;
	}
	ret = raw_dbgir3(data, 0x90, (u8)(addr >> 8), (u8)(addr & 0xff));
	if (ret < 0) {
		dev_err(&client->dev, "fail to %s (%d)\n", __func__, ret);
		return ret;
	}
	ret = raw_spchl(data, 0xff, 0x28);
	if (ret < 0) {
		dev_err(&client->dev, "fail to %s (%d)\n", __func__, ret);
		return ret;
	}

	while (addr < data->fw_img->fw_len) {

#if defined(ISP_DEBUG)
		dev_info(&client->dev, "fw read addr=%#x\n", addr);
#endif

		do {
			isp_master_send(data, 0xc0, 0x14);

#if !defined(ISP_SPEED_UP)
			val = 0;
			dcr_rd_cnt = TC380_DCR_RD_RETRY;
			do {
				isp_master_recv(data, 0xc1, &val);
				if (dcr_rd_cnt-- < 0) {
					dev_err(&client->dev, "%s: fail to "
						"update dcr\n", __func__);
					return -ENOSYS;
				}
				usleep_range(900, 1000);
			} while (val != 0x12);
#endif
#if defined(ISP_VERBOSE_DEBUG)
			dev_info(&client->dev, "dcr_rd_cnt=%d\n", dcr_rd_cnt);
#endif
			isp_master_send(data, 0xc0, 0x08);
			isp_master_recv(data, 0xd9, &val);

			if (data->fw_img->data[addr] != val) {
				dev_err(&client->dev, "fail to verify at "
					"%#x (%#x)\n", addr,
					data->fw_img->data[addr]);
				return -EIO;
			}
			addr++;
		} while (addr % 0x20);
	}

	return 0;
}

static int tc380_get_fw_ver(struct touchkey_i2c *data)
{
	struct i2c_client *client = data->client;
	int ver;
	int retries = 2;

read_version:
	ver = i2c_smbus_read_byte_data(client, TC380_CMD_FW_VER);
	if (ver < 0) {
		dev_err(&client->dev, "failed to read fw ver (%d)\n", ver);
		if (retries-- > 0) {
			data->pdata->suspend();
			msleep(TC380_POWERON_DELAY);
			data->pdata->resume();
			msleep(TC380_POWERON_DELAY);
			goto read_version;
		}
	}

	return ver;
}

int tc380_fw_update(struct touchkey_i2c *data)
{
	struct i2c_client *client = data->client;
	int retries;
	int ret;

	data->pdata->pin_configure(true);

	retries = TC380_FW_FLASH_RETRY;
erase_fw:
	dev_info(&client->dev, "start erasing fw\n");
	ret = tc380_erase_fw(data);
	if (ret < 0) {
		dev_err(&client->dev, "fail to erase fw (%d)\n", ret);
		if (retries-- > 0) {
			dev_info(&client->dev, "retry esasing fw (%d)\n",
				 retries);
			goto erase_fw;
		} else {
			goto err;
		}
	}
	dev_info(&client->dev, "succeed in erasing fw\n");

	retries = TC380_FW_FLASH_RETRY;
write_fw:
	dev_info(&client->dev, "start writing fw\n");
	ret = tc380_write_fw(data);
	if (ret < 0) {
		dev_err(&client->dev, "fail to write fw (%d)\n", ret);
		if (retries-- > 0) {
			dev_info(&client->dev, "retry writing fw (%d)\n",
				 retries);
			goto write_fw;
		} else {
			goto err;
		}
	}
	dev_info(&client->dev, "succeed in writing fw\n");

#if 0
	retries = TC380_FW_FLASH_RETRY;
verify_fw:
	ret = tc380_verify_fw(data);
	if (ret < 0) {
		dev_err(&client->dev, "fail to verify fw (%d)\n", ret);
		if (retries-- > 0) {
			dev_info(&client->dev, "retry verifing fw (%d)\n",
				 retries);
			goto verify_fw;
		} else {
			goto err;
		}
	}
	dev_info(&client->dev, "succeed in verifing fw\n");
#endif
	data->pdata->suspend();
	msleep(TC380_POWERON_DELAY);

	data->pdata->power_on(true);
	msleep(TC380_POWERON_DELAY*2);

#if 0
	retries = TC380_FW_VER_READ;
read_flashed_fw_ver:

ret = tc380_get_fw_ver(data);
	if (ret < 0) {
		dev_err(&client->dev, "fail to read fw ver (%d)\n", ret);
		if (retries-- > 0) {
			dev_info(&client->dev, "retry read flash fw ver (%d)\n",
				 retries);
			goto read_flashed_fw_ver;
		} else {
			goto err;
		}
	}

	dev_info(&client->dev, "succeed in reading fw ver %#x\n", (u8)ret);

	ret = 0;

	//data->pdata->pin_configure(false);

if (data->cur_fw_path == FW_BUILT_IN)
		release_firmware(data->fw);
	else if (data->cur_fw_path == FW_IN_SDCARD ||
		 data->cur_fw_path == FW_EX_SDCARD)
		kfree(data->fw_img);
#endif

	data->enabled = true;

#if defined(SEC_FAC_TK)
	if (data->fdata->fw_flash_status == DOWNLOADING)
		data->fdata->fw_flash_status = PASS;
#endif

	dev_info(&client->dev, "succeed in flashing fw\n");

	return ret;
err:
	data->pdata->pin_configure(false);

#if defined(SEC_FAC_TK)
	if (data->fdata->fw_flash_status == DOWNLOADING) {
		dev_err(&client->dev, "fail to fw flash.\n");
		data->fdata->fw_flash_status = FAIL;
		return;
	}
#endif
	dev_err(&client->dev, "fail to fw flash. driver is removed\n");
	return ret;
}
