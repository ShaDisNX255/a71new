/*
 * =================================================================
 *
 *	Description:  samsung display sysfs common file
 *	Company:  Samsung Electronics
 *
 * ================================================================
 */
/*
<one line to give the program's name and a brief idea of what it does.>
Copyright (C) 2015, Samsung Electronics. All rights reserved.

 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "ss_dsi_panel_sysfs.h"

extern struct kset *devices_kset;

#define MAX_FILE_NAME 128
#ifdef MDNIE_TUNING
#define TUNING_FILE_PATH "/sdcard/"
static char tuning_file[MAX_FILE_NAME];

#if 0
/*
 * Do not use below code but only for Image Quality Debug in Developing Precess.
 * Do not comment in this code Because there are contained vulnerability.
 */
static char char_to_dec(char data1, char data2)
{
	char dec;

	dec = 0;

	if (data1 >= 'a') {
		data1 -= 'a';
		data1 += 10;
	} else if (data1 >= 'A') {
		data1 -= 'A';
		data1 += 10;
	} else
		data1 -= '0';

	dec = data1 << 4;

	if (data2 >= 'a') {
		data2 -= 'a';
		data2 += 10;
	} else if (data2 >= 'A') {
		data2 -= 'A';
		data2 += 10;
	} else
		data2 -= '0';

	dec |= data2;

	return dec;
}

static void sending_tune_cmd(struct device *dev, char *src, int len)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	int i;
	int data_pos;
	int cmd_step;
	int cmd_pos;
	int cmd_cnt = 0;

	char *mdnie_tuning[MDNIE_TUNE_MAX_SIZE];
	struct dsi_cmd_desc *mdnie_tune_cmd;
	struct dsi_panel_cmd_set *set;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd\n");
		return;
	}

	for (i = 0; i < MDNIE_TUNE_MAX_SIZE; i++) {
		if (vdd->mdnie.mdnie_tune_size[i]) {
			mdnie_tuning[i] = kzalloc(sizeof(char) * vdd->mdnie.mdnie_tune_size[i], GFP_KERNEL);
			cmd_cnt++;
			LCD_INFO("mdnie_tune_size[%d] (%d) \n", i+1, vdd->mdnie.mdnie_tune_size[i]);
		}
	}

	LCD_INFO("cmd_cnt : %d\n", cmd_cnt);
	if (!cmd_cnt) {
		LCD_ERR("No tuning cmds..\n");
		return;
	}
	vdd->mdnie.tuning_enable_tft = 1;

	mdnie_tune_cmd = kzalloc(cmd_cnt * sizeof(struct dsi_cmd_desc), GFP_KERNEL);

	cmd_step = 0;
	cmd_pos = 0;

	for (data_pos = 0; data_pos < len;) {
		if (*(src + data_pos) == '0') {
			if (*(src + data_pos + 1) == 'x') {
				if (!cmd_step)
					mdnie_tuning[0][cmd_pos] = char_to_dec(*(src + data_pos + 2), *(src + data_pos + 3));
				else if (cmd_step == 1)
					mdnie_tuning[1][cmd_pos] = char_to_dec(*(src + data_pos + 2), *(src + data_pos + 3));
				else if (cmd_step == 2 && vdd->mdnie.tuning_enable_tft)
					mdnie_tuning[2][cmd_pos] = char_to_dec(*(src + data_pos + 2), *(src + data_pos + 3));
				else if (cmd_step == 3 && vdd->mdnie.tuning_enable_tft)
					mdnie_tuning[3][cmd_pos] = char_to_dec(*(src + data_pos + 2), *(src + data_pos + 3));
				else if (cmd_step == 4 && vdd->mdnie.tuning_enable_tft)
					mdnie_tuning[4][cmd_pos] = char_to_dec(*(src + data_pos + 2), *(src + data_pos + 3));
				else if (cmd_step == 5 && vdd->mdnie.tuning_enable_tft)
					mdnie_tuning[5][cmd_pos] = char_to_dec(*(src + data_pos + 2), *(src + data_pos + 3));
				else
					mdnie_tuning[2][cmd_pos] = char_to_dec(*(src + data_pos + 2), *(src + data_pos + 3));

				data_pos += 3;
				cmd_pos++;

				if (cmd_pos == vdd->mdnie.mdnie_tune_size[0] && !cmd_step) {
					cmd_pos = 0;
					cmd_step = 1;
				} else if ((cmd_pos == vdd->mdnie.mdnie_tune_size[1]) && (cmd_step == 1)) {
					cmd_pos = 0;
					cmd_step = 2;
				} else if ((cmd_pos == vdd->mdnie.mdnie_tune_size[2]) && (cmd_step == 2) && vdd->mdnie.tuning_enable_tft) {
					cmd_pos = 0;
					cmd_step = 3;
				} else if ((cmd_pos == vdd->mdnie.mdnie_tune_size[3]) && (cmd_step == 3) && vdd->mdnie.tuning_enable_tft) {
					cmd_pos = 0;
					cmd_step = 4;
				} else if ((cmd_pos == vdd->mdnie.mdnie_tune_size[4]) && (cmd_step == 4) && vdd->mdnie.tuning_enable_tft) {
					cmd_pos = 0;
					cmd_step = 5;
				}
			} else
				data_pos++;
		} else {
			data_pos++;
		}
	}

	for (i = 0; i < cmd_cnt; i++) {
		mdnie_tune_cmd[i].msg.type = MIPI_DSI_DCS_LONG_WRITE;
		mdnie_tune_cmd[i].last_command = 1;
		mdnie_tune_cmd[i].msg.tx_len = vdd->mdnie.mdnie_tune_size[i];
		mdnie_tune_cmd[i].msg.tx_buf = mdnie_tuning[i];

		printk(KERN_ERR "mdnie_tuning%d (%d)\n", i, vdd->mdnie.mdnie_tune_size[i]);
		for (data_pos = 0; data_pos < vdd->mdnie.mdnie_tune_size[i] ; data_pos++)
			printk(KERN_ERR "0x%x \n", mdnie_tuning[i][data_pos]);
	}

	ss_send_cmd(vdd, TX_LEVEL1_KEY_ENABLE);
	set = ss_get_cmds(vdd, TX_MDNIE_TUNE);
	if (IS_ERR_OR_NULL(set)) {
		LCD_ERR("no cmds for TX_MDNIE_TUNE..\n");
		goto err;
	}

	set->state = DSI_CMD_SET_STATE_HS;
	set->cmds = mdnie_tune_cmd;
	set->count = vdd->mdnie.tuning_enable_tft ? 6 : 3;
	ss_send_cmd(vdd, TX_MDNIE_TUNE);
	ss_send_cmd(vdd, TX_LEVEL1_KEY_DISABLE);

err:
	for (i = 0; i < cmd_cnt; i++)
		kfree(mdnie_tuning[i]);

	kfree(mdnie_tune_cmd);

	return;
}

static void load_tuning_file(struct device *dev, char *filename)
{
	struct file *filp;
	char *dp;
	long l;
	loff_t pos;
	int ret;
	mm_segment_t fs;
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return;
	}

	LCD_INFO("called loading file name : [%s]\n",
	       filename);

	fs = get_fs();
	set_fs(get_ds());

	filp = filp_open(filename, O_RDONLY, 0);
	if (IS_ERR(filp)) {

		ret = PTR_ERR(filp);
		printk(KERN_ERR "%s File open failed ret = %d\n", __func__, ret);
		goto err;
	}

	l = filp->f_path.dentry->d_inode->i_size;
	LCD_INFO("Loading File Size : %ld(bytes)", l);

	dp = kmalloc(l + 10, GFP_KERNEL);
	if (dp == NULL) {
		LCD_INFO("Can't not alloc memory for tuning file load\n");
		filp_close(filp, current->files);
		goto err;
	}
	pos = 0;
	memset(dp, 0, l);

	LCD_INFO("before vfs_read()\n");
	ret = vfs_read(filp, (char __user *)dp, l, &pos);
	LCD_INFO("after vfs_read()\n");

	if (ret != l) {
		LCD_INFO("vfs_read() filed ret : %d\n", ret);
		kfree(dp);
		filp_close(filp, current->files);
		goto err;
	}

	filp_close(filp, current->files);

	set_fs(fs);

	sending_tune_cmd(dev, dp, l);

	kfree(dp);

	return;
err:
	set_fs(fs);
}
#endif

static ssize_t tuning_show(struct device *dev,
			   struct device_attribute *attr, char *buf)
{
	int ret = 0;

	ret = snprintf(buf, MAX_FILE_NAME, "Tunned File Name : %s\n", tuning_file);
	return ret;
}

static ssize_t tuning_store(struct device *dev,
			    struct device_attribute *attr, const char *buf,
			    size_t size)
{
/*
 * Do not use below code but only for Image Quality Debug in Developing Precess.
 * Do not comment in this code Because there are contained vulnerability.
 */
/*
	char *pt;
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return size;
	}

	if (!ss_is_ready_to_send_cmd(vdd)) {
		LCD_ERR("Panel is not ready. Panel State(%d)\n", vdd->panel_state);
		return size;
	}

	if (buf == NULL || strchr(buf, '.') || strchr(buf, '/'))
		return size;

	memset(tuning_file, 0, sizeof(tuning_file));
	snprintf(tuning_file, MAX_FILE_NAME, "%s%s", TUNING_FILE_PATH, buf);

	pt = tuning_file;

	while (*pt) {
		if (*pt == '\r' || *pt == '\n') {
			*pt = 0;
			break;
		}
		pt++;
	}

	LCD_INFO("%s\n", tuning_file);

	load_tuning_file(dev, tuning_file);
*/
	return size;
}
#endif

static ssize_t ss_disp_cell_id_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	static int string_size = 50;
	char temp[string_size];
	int *cell_id;
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return strnlen(buf, string_size);
	}

	vdd = ss_check_hall_ic_get_vdd(vdd);

	cell_id = vdd->cell_id_dsi;

	/*
	*	STANDARD FORMAT (Total is 11Byte)
	*	MAX_CELL_ID : 11Byte
	*	7byte(cell_id) + 2byte(Mdnie mdnie.mdnie_xx_postion) + 2byte(Mdnie y_postion)
	*/

	snprintf((char *)temp, sizeof(temp),
			"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
		cell_id[0], cell_id[1], cell_id[2], cell_id[3], cell_id[4],
		cell_id[5], cell_id[6],
		(vdd->mdnie.mdnie_x & 0xFF00) >> 8,
		vdd->mdnie.mdnie_x & 0xFF,
		(vdd->mdnie.mdnie_y & 0xFF00) >> 8,
		vdd->mdnie.mdnie_y & 0xFF);

	strlcat(buf, temp, string_size);

	LCD_INFO("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
		cell_id[0], cell_id[1], cell_id[2], cell_id[3], cell_id[4],
		cell_id[5], cell_id[6],
		(vdd->mdnie.mdnie_x & 0xFF00) >> 8,
		vdd->mdnie.mdnie_x & 0xFF,
		(vdd->mdnie.mdnie_y & 0xFF00) >> 8,
		vdd->mdnie.mdnie_y & 0xFF);

	return strnlen(buf, string_size);
}

static ssize_t ss_disp_octa_id_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	static int string_size = 50;
	char temp[string_size];
	u8 *octa_id;
	int site, rework, poc, max_brightness;
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return strnlen(buf, string_size);
	}

	vdd = ss_check_hall_ic_get_vdd(vdd);

	octa_id = vdd->octa_id_dsi;

	site = octa_id[0] & 0xf0;
	site >>= 4;
	rework = octa_id[0] & 0x0f;
	poc = octa_id[1] & 0x0f;
	max_brightness = octa_id[2] * 256 + octa_id[3];

	snprintf((char *)temp, sizeof(temp),
			"%d%d%d%02x%02x%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",
		site, rework, poc, octa_id[2], octa_id[3],
		octa_id[4] != 0 ? octa_id[4] : '0',
		octa_id[5] != 0 ? octa_id[5] : '0',
		octa_id[6] != 0 ? octa_id[6] : '0',
		octa_id[7] != 0 ? octa_id[7] : '0',
		octa_id[8] != 0 ? octa_id[8] : '0',
		octa_id[9] != 0 ? octa_id[9] : '0',
		octa_id[10] != 0 ? octa_id[10] : '0',
		octa_id[11] != 0 ? octa_id[11] : '0',
		octa_id[12] != 0 ? octa_id[12] : '0',
		octa_id[13] != 0 ? octa_id[13] : '0',
		octa_id[14] != 0 ? octa_id[14] : '0',
		octa_id[15] != 0 ? octa_id[15] : '0',
		octa_id[16] != 0 ? octa_id[16] : '0',
		octa_id[17] != 0 ? octa_id[17] : '0',
		octa_id[18] != 0 ? octa_id[18] : '0',
		octa_id[19] != 0 ? octa_id[19] : '0');

	strlcat(buf, temp, string_size);

	LCD_INFO("poc(%d)\n", poc);

	LCD_DEBUG("%d%d%d%02x%02x%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",
		site, rework, poc, octa_id[2], octa_id[3],
		octa_id[4] != 0 ? octa_id[4] : '0',
		octa_id[5] != 0 ? octa_id[5] : '0',
		octa_id[6] != 0 ? octa_id[6] : '0',
		octa_id[7] != 0 ? octa_id[7] : '0',
		octa_id[8] != 0 ? octa_id[8] : '0',
		octa_id[9] != 0 ? octa_id[9] : '0',
		octa_id[10] != 0 ? octa_id[10] : '0',
		octa_id[11] != 0 ? octa_id[11] : '0',
		octa_id[12] != 0 ? octa_id[12] : '0',
		octa_id[13] != 0 ? octa_id[13] : '0',
		octa_id[14] != 0 ? octa_id[14] : '0',
		octa_id[15] != 0 ? octa_id[15] : '0',
		octa_id[16] != 0 ? octa_id[16] : '0',
		octa_id[17] != 0 ? octa_id[17] : '0',
		octa_id[18] != 0 ? octa_id[18] : '0',
		octa_id[19] != 0 ? octa_id[19] : '0');

	return strnlen(buf, string_size);
}

static ssize_t ss_disp_lcdtype_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	static int string_size = 100;
	char temp[string_size];
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return strnlen(buf, string_size);
	}

	vdd = ss_check_hall_ic_get_vdd(vdd);

	if (vdd->dtsi_data.tft_common_support && vdd->dtsi_data.tft_module_name) {
		if (vdd->dtsi_data.panel_vendor)
			snprintf(temp, 20, "%s_%s\n", vdd->dtsi_data.panel_vendor, vdd->dtsi_data.tft_module_name);
		else
			snprintf(temp, 20, "SDC_%s\n", vdd->dtsi_data.tft_module_name);
	} else if (ss_panel_attached(vdd->ndx)) {
		if (vdd->dtsi_data.panel_vendor)
			snprintf(temp, 20, "%s_%06x\n", vdd->dtsi_data.panel_vendor, vdd->manufacture_id_dsi);
		else
			snprintf(temp, 20, "SDC_%06x\n", vdd->manufacture_id_dsi);
	} else {
		LCD_INFO("no manufacture id\n");
		snprintf(temp, 20, "SDC_000000\n");
	}

	strlcat(buf, temp, string_size);

	return strnlen(buf, string_size);
}

static ssize_t ss_disp_windowtype_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	static int string_size = 15;
	char temp[string_size];
	int id, id1, id2, id3;
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return strnlen(buf, string_size);
	}

	vdd = ss_check_hall_ic_get_vdd(vdd);

	/* If LCD_ID is needed before splash done(Multi Color Boot Animation), we should get LCD_ID form LK */
	if (vdd->manufacture_id_dsi == PBA_ID)
		id = get_lcd_attached("GET");
	else
		id = vdd->manufacture_id_dsi;

	id1 = (id & 0x00FF0000) >> 16;
	id2 = (id & 0x0000FF00) >> 8;
	id3 = id & 0xFF;

	LCD_INFO("%02x %02x %02x\n", id1, id2, id3);

	snprintf(temp, sizeof(temp), "%02x %02x %02x\n", id1, id2, id3);

	strlcat(buf, temp, string_size);

	return strnlen(buf, string_size);
}

static ssize_t ss_disp_manufacture_date_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	static int string_size = 30;
	char temp[string_size];
	int date;
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return strnlen(buf, string_size);
	}

	vdd = ss_check_hall_ic_get_vdd(vdd);
	date = vdd->manufacture_date_dsi;
	snprintf((char *)temp, sizeof(temp), "manufacture date : %d\n", date);

	strlcat(buf, temp, string_size);

	LCD_INFO("manufacture date : %d\n", date);

	return strnlen(buf, string_size);
}

static ssize_t ss_disp_manufacture_code_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	static int string_size = 30;
	char temp[string_size];
	int *ddi_id;
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return strnlen(buf, string_size);
	}

	vdd = ss_check_hall_ic_get_vdd(vdd);
	ddi_id = vdd->ddi_id_dsi;

	snprintf((char *)temp, sizeof(temp), "%02x%02x%02x%02x%02x\n",
		ddi_id[0], ddi_id[1], ddi_id[2], ddi_id[3], ddi_id[4]);

	strlcat(buf, temp, string_size);

	LCD_INFO("%02x %02x %02x %02x %02x\n",
		ddi_id[0], ddi_id[1], ddi_id[2], ddi_id[3], ddi_id[4]);

	return strnlen(buf, string_size);
}

static ssize_t ss_disp_SVC_OCTA_DDI_CHIPID_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	static int string_size = 30;
	char temp[string_size];
	int *ddi_id;
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return strnlen(buf, string_size);
	}

	vdd = ss_check_hall_ic_get_vdd(vdd);
	ddi_id = vdd->ddi_id_dsi;

	snprintf((char *)temp, sizeof(temp), "%02x%02x%02x%02x%02x\n",
		ddi_id[0], ddi_id[1], ddi_id[2], ddi_id[3], ddi_id[4]);

	strlcat(buf, temp, string_size);

	LCD_INFO("%02x %02x %02x %02x %02x\n",
		ddi_id[0], ddi_id[1], ddi_id[2], ddi_id[3], ddi_id[4]);

	return strnlen(buf, string_size);
}

static ssize_t ss_disp_acl_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	int rc = 0;
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return rc;
	}

	rc = snprintf((char *)buf, sizeof(vdd->acl_status), "%d\n", vdd->acl_status);

	LCD_INFO("acl status: %d\n", *buf);

	return rc;
}

static ssize_t ss_disp_acl_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	int acl_set = 0;
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return size;
	}

	if (sysfs_streq(buf, "1"))
		acl_set = true;
	else if (sysfs_streq(buf, "0"))
		acl_set = false;
	else
		LCD_INFO("Invalid argument!!");

	LCD_INFO("(%d)\n", acl_set);

	if ((acl_set && !vdd->acl_status) ||
			(!acl_set && vdd->acl_status)) {
		vdd->acl_status = acl_set;
		if (!ss_is_ready_to_send_cmd(vdd)) {
			LCD_ERR("Panel is not ready. Panel State(%d)\n", vdd->panel_state);
			return size;
		}
		ss_brightness_dcs(vdd, USE_CURRENT_BL_LEVEL, BACKLIGHT_NORMAL);
	} else {
		vdd->acl_status = acl_set;
		LCD_INFO("skip acl update!! acl %d", vdd->acl_status);
	}

	return size;
}


int fd_status = 0;
static ssize_t ss_disp_fd_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	int rc = 0;
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return rc;
	}
	rc = snprintf((char *)buf, sizeof(fd_status), "%d\n", fd_status);

	LCD_INFO("fd status: %d\n", *buf);

	return rc;
}

static ssize_t ss_disp_fd_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	int rc = 0;
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return rc;
	}
	if (!ss_is_ready_to_send_cmd(vdd)) {
		LCD_ERR("Panel is not ready. Panel State(%d)\n", vdd->panel_state);
		return size;
	}

	if (sysfs_streq(buf, "1"))
	{
		if (vdd->panel_func.ub_fd_control)
			vdd->panel_func.ub_fd_control(vdd, true);
		else
			ss_send_cmd(vdd, TX_FD_ON); //TX_FD_ON - 239
		fd_status = true;
	}
	else if (sysfs_streq(buf, "0"))
	{

		if (vdd->panel_func.ub_fd_control)
			vdd->panel_func.ub_fd_control(vdd, false);
		else
			ss_send_cmd(vdd, TX_FD_OFF); //TX_FD_OFF - 240
		fd_status = false;
	}
	else
		LCD_INFO("Invalid argument!!");

	LCD_INFO("(%d)\n", fd_status);

	return size;
}

static ssize_t ss_disp_siop_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	int rc = 0;
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return rc;
	}

	rc = snprintf((char *)buf, sizeof(vdd->siop_status), "%d\n", vdd->siop_status);

	LCD_INFO("siop status: %d\n", *buf);

	return rc;
}

static ssize_t ss_disp_siop_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	int siop_set = 0;
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return size;
	}


	if (sysfs_streq(buf, "1"))
		siop_set = true;
	else if (sysfs_streq(buf, "0"))
		siop_set = false;
	else
		LCD_INFO("Invalid argument!!");

	LCD_INFO("(%d)\n", siop_set);

	vdd->siop_status = siop_set;

	if (!ss_is_ready_to_send_cmd(vdd)) {
		LCD_ERR("Panel is not ready. Panel State(%d)\n", vdd->panel_state);
		return size;
	}
	if (siop_set && !vdd->siop_status) {
		ss_brightness_dcs(vdd, USE_CURRENT_BL_LEVEL, BACKLIGHT_NORMAL);
	} else if (!siop_set && vdd->siop_status) {
		ss_brightness_dcs(vdd, USE_CURRENT_BL_LEVEL, BACKLIGHT_NORMAL);
	} else {
		LCD_INFO("skip siop ss_brightness_dcs!! acl %d", vdd->acl_status);
	}

	return size;
}

static ssize_t ss_aid_log_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	int rc = 0;
	struct samsung_display_driver_data *vdd;

	list_for_each_entry_reverse(vdd, &vdds_list, vdd_list) {
		if (vdd->smart_dimming_dsi && vdd->smart_dimming_dsi->print_aid_log)
			vdd->smart_dimming_dsi->print_aid_log(vdd->smart_dimming_dsi);
		else
			LCD_ERR("ndx=%d, smart dimming is not loaded\n", vdd->ndx);

		if (vdd->dtsi_data.hmt_enabled) {
			if (vdd->smart_dimming_dsi_hmt && vdd->smart_dimming_dsi_hmt->print_aid_log)
				vdd->smart_dimming_dsi_hmt->print_aid_log(vdd->smart_dimming_dsi_hmt);
			else
				LCD_ERR("ndx=%d, smart dimming hmt is not loaded\n", vdd->ndx);
		}
	}

	return rc;
}

static int buffer_backup(u8 *buf, int size, char *name)
{
	struct file *fp;
	mm_segment_t old_fs;

	if (!name)
		return -1;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	LCD_ERR("%s filename %s size %d\n", __func__, name, size);
	fp = filp_open(name, O_CREAT | O_TRUNC | O_WRONLY | O_SYNC, 0660);
	if (IS_ERR(fp)) {
		pr_err("%s, fail to open %s file\n", __func__, name);
		goto open_err;
	}

	vfs_write(fp, (u8 __user *)buf, size, &fp->f_pos);
	LCD_ERR("%s filename %s write %d bytes done!!\n",
			__func__, name, size);

	filp_close(fp, current->files);
	set_fs(old_fs);

	return 0;

 open_err:
	set_fs(old_fs);
	return -1;
}

char *line[4096];
#define LINE_SIZE SZ_1K
static ssize_t ss_aid_log_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	char *output, *line_buf;
	int count = 0, len = 0, idx = 1, br_step = 0;
	int i, j, k;
	int input;

	struct candela_map_table *normal_table, *hbm_table, *hmd_table;

	struct PRINT_TABLE *print_table;
	int print_size;

	const char * const path[] = {
		"/data/brightness.csv",
/*		"/data/hmd_brightness.csv",*/
	};

	unsigned char read_buf[256] = {0x0, };

	LCD_INFO("++ \n");

	if (sscanf(buf, "%d", &input) != 1)
		return size;

	print_table = vdd->panel_br_info.print_table;
	print_size = vdd->panel_br_info.print_size;

	if (!print_size) {
		LCD_ERR("No print_table (%d)\n", print_size);
		return size;
	}

	line_buf = kmalloc(LINE_SIZE, GFP_KERNEL);
	if (!line_buf)
		return size;
	line[count++] = line_buf;

	len = snprintf(line_buf, LINE_SIZE, "NO,");
	len += snprintf(line_buf + len, LINE_SIZE - len, "FROM,");
	len += snprintf(line_buf + len, LINE_SIZE - len, "TO,");

	for (i = 0; i < print_size; i++) {
		for (j = 1; j <= print_table[i].read_size; j++)
			len += snprintf(line_buf + len, LINE_SIZE - len, "%s_%d,", print_table[i].name, j);
	}

	if (vdd->br.pac)
		normal_table = &vdd->dtsi_data.candela_map_table[PAC_NORMAL][vdd->panel_revision];
	else
		normal_table = &vdd->dtsi_data.candela_map_table[NORMAL][vdd->panel_revision];

	/* NORMAL TABLE */
	for (i = 0; i < normal_table->tab_size; i++) {

		/* only print for original step */
		if (input == 1) {
			if (normal_table->end[i] != vdd->panel_br_info.orig_normal_table[br_step].platform * 100)
				continue;
			else
				br_step++;
		}

		line_buf = kmalloc(LINE_SIZE, GFP_KERNEL);
		if (!line_buf)
			return size;
		line[count++] = line_buf;

		len = snprintf(line_buf, LINE_SIZE, "%4d,", idx++);
		len += snprintf(line_buf + len, LINE_SIZE - len, "%5d,", normal_table->from[i]);
		len += snprintf(line_buf + len, LINE_SIZE - len, "%5d,", normal_table->end[i]);

		/* Write Brightness */
		ss_brightness_dcs(vdd, normal_table->end[i], BACKLIGHT_NORMAL);

		/* Read Brightness */
		for (j = 0; j < print_size; j++) {
			ss_read_mtp(vdd, print_table[j].read_addr, print_table[j].read_size, print_table[j].read_pos, read_buf);
			for (k = 0; k < print_table[j].read_size; k++)
				len += snprintf(line_buf + len, LINE_SIZE - len, "0x%02x,", read_buf[k]);
		}
	}

	if (vdd->br.pac)
		hbm_table = &vdd->dtsi_data.candela_map_table[PAC_HBM][vdd->panel_revision];
	else
		hbm_table = &vdd->dtsi_data.candela_map_table[HBM][vdd->panel_revision];

	br_step = 1;

	/* HBM TABLE */
	for (i = 0; i < hbm_table->tab_size; i++) {

		/* only print for original step */
		if (input == 1) {
			if (hbm_table->from[i] != vdd->panel_br_info.orig_hbm_table[br_step].platform * 100)
				continue;
			else
				br_step++;
		}

		line_buf = kmalloc(LINE_SIZE, GFP_KERNEL);
		if (!line_buf)
			return size;
		line[count++] = line_buf;

		len = snprintf(line_buf, LINE_SIZE, "%4d,", idx++);
		len += snprintf(line_buf + len, LINE_SIZE - len, "%5d,", hbm_table->from[i]);
		len += snprintf(line_buf + len, LINE_SIZE - len, "%5d,", hbm_table->end[i]);

		/* Write Brightness */
		ss_brightness_dcs(vdd, hbm_table->from[i], BACKLIGHT_NORMAL);

		/* Read Brightness */
		for (j = 0; j < print_size; j++) {
			ss_read_mtp(vdd, print_table[j].read_addr, print_table[j].read_size, print_table[j].read_pos, read_buf);
			for (k = 0; k < print_table[j].read_size; k++)
				len += snprintf(line_buf + len, LINE_SIZE - len, "0x%02x,", read_buf[k]);
		}
	}

	/* HMD */
	hmd_table = &vdd->dtsi_data.candela_map_table[HMT][vdd->panel_revision];

	LCD_ERR("hmd table size (%d)\n", hmd_table->tab_size);
	for (i = 0; i < hmd_table->tab_size; i++) {
		line_buf = kmalloc(LINE_SIZE, GFP_KERNEL);
		if (!line_buf)
			return size;
		line[count++] = line_buf;

		len = snprintf(line_buf, LINE_SIZE, "%4d,", idx++);
		len += snprintf(line_buf + len, LINE_SIZE - len, "%5d,", hmd_table->from[i]);
		len += snprintf(line_buf + len, LINE_SIZE - len, "%5d,", hmd_table->end[i]);

		/* Write Brightness */
		ss_brightness_dcs_hmt(vdd, hmd_table->from[i]);

		/* Read Brightness */
		for (j = 0; j < print_size; j++) {
			ss_read_mtp(vdd, print_table[j].read_addr, print_table[j].read_size, print_table[j].read_pos, read_buf);
			for (k = 0; k < print_table[j].read_size; k++)
				len += snprintf(line_buf + len, LINE_SIZE - len, "0x%02x,", read_buf[k]);
		}
	}

	output = kmalloc(LINE_SIZE * count, GFP_KERNEL);
	if (output) {
		len = 0;
		for (i = 0; i < count; i++)
			len += snprintf(output + len,
					LINE_SIZE * count - len, "%s\n", line[i]);
		buffer_backup(output, len, (char *)path[0]);
		kfree(output);
	}

	if (line_buf)
		kfree(line_buf);

	LCD_INFO("-- \n");

	return size;
}


#if defined(CONFIG_BACKLIGHT_CLASS_DEVICE)
static ssize_t ss_disp_brightness_step(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int rc = 0;
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return rc;
	}

	vdd = ss_check_hall_ic_get_vdd(vdd);

	rc = snprintf((char *)buf, 20, "%d\n", vdd->dtsi_data.candela_map_table[NORMAL][vdd->panel_revision].tab_size);

	LCD_INFO("brightness_step : %d", vdd->dtsi_data.candela_map_table[NORMAL][vdd->panel_revision].tab_size);

	return rc;
}

#if 0
static ssize_t ss_disp_color_weakness_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int rc = 0;
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return rc;
	}

	vdd = ss_check_hall_ic_get_vdd(vdd);

	rc = snprintf((char *)buf, 20, "%d %d\n", vdd->color_weakness_mode, vdd->color_weakness_level);

	LCD_INFO("color weakness : %d %d", vdd->color_weakness_mode, vdd->color_weakness_level);

	return rc;
}

static ssize_t ss_disp_color_weakness_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	unsigned int mode, level;
	u8 value = 0;
	u8 *tx_buf;
	struct dsi_panel_cmd_set *pcmds;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		goto end;
	}

	if (!ss_is_ready_to_send_cmd(vdd)) {
		LCD_ERR("Panel is not ready. Panel State(%d)\n", vdd->panel_state);
		return size;
	}

	sscanf(buf, "%x %x", &mode, &level);

	if (mode >= 0 && mode <= 9)
		vdd->color_weakness_mode = mode;
	else
		LCD_ERR("mode (%x) is not correct !\n", mode);

	if (level >= 0 && level <= 9)
		vdd->color_weakness_level = level;
	else
		LCD_ERR("level (%x) is not correct !\n", level);

	value = level << 4 | mode;

	LCD_ERR("level (%x) mode (%x) value (%x)\n", level, mode, value);

	pcmds = ss_get_cmds(vdd, TX_COLOR_WEAKNESS_ENABLE);
	tx_buf = pcmds->cmds[1].msg.tx_buf;
	tx_buf[1] = value;

	if (mode)
		ss_send_cmd(vdd, TX_COLOR_WEAKNESS_ENABLE);
	else
		ss_send_cmd(vdd, TX_COLOR_WEAKNESS_DISABLE);

end:
	return size;
}
#endif
#endif

static ssize_t ss_gamma_interpolation_test_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	int val1, val2, val3, val4, val5, val6;
	u8 *tx_buf;
	struct dsi_panel_cmd_set *pcmds;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		goto end;
	}

	if (!ss_is_ready_to_send_cmd(vdd)) {
		LCD_ERR("Panel is not ready. Panel State(%d)\n", vdd->panel_state);
		return size;
	}

	if (sscanf(buf, "%x %x %x %x %x %x", &val1, &val2, &val3, &val4, &val5, &val6) != 6)
		return size;

	pcmds = ss_get_cmds(vdd, TX_GAMMA_MODE1_INTERPOLATION);
	if (IS_ERR_OR_NULL(pcmds)) {
		LCD_ERR("no cmds for TX_GAMMA_MODE1_INTERPOLATION..\n");
		goto end;
	}

	tx_buf = pcmds->cmds[2].msg.tx_buf;
	tx_buf[1] = val1;
	tx_buf[2] = val2;
	tx_buf[3] = val3;
	tx_buf[4] = val4;
	tx_buf[5] = val5;
	tx_buf[6] = val6;

	ss_send_cmd(vdd, TX_GAMMA_MODE1_INTERPOLATION);

end:
	return size;
}

#define read_buf_max (256)
unsigned char readbuf[read_buf_max] = {0x0, };
unsigned int readaddr, readlen, readpos;

static ssize_t ss_read_mtp_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int i, len = 0;
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return -ENODEV;
	}

	if (readlen && (readlen < read_buf_max)) {
		for (i = 0; i < readlen; i++)
			len += snprintf(buf + len, 10, "%02x%s", readbuf[i], (i == readlen - 1) ? "\n" : " ");
	} else {
		len += snprintf(buf + len, 100, "No read data.. \n");
		len += snprintf(buf + len, 100, "Please write (addr gpara len) to read_mtp store to read mtp\n");
		len += snprintf(buf + len, 100, "ex) echo CA 1 20 > read_mtp \n");
		len += snprintf(buf + len, 100, "- read 20bytes of CAh register from 2nd para. \n");
	}

	return strlen(buf);
}

static ssize_t ss_read_mtp_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	unsigned int temp[3];

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return -ENODEV;
	}

	if (!ss_is_ready_to_send_cmd(vdd)) {
		LCD_ERR("Panel is not ready. Panel State(%d)\n", vdd->panel_state);
		return size;
	}

	if (sscanf(buf, "%x %d %d", &temp[0], &temp[1], &temp[2]) != 3)
		return size;

	readaddr = temp[0];
	readpos = temp[1];
	readlen = temp[2];

	if (readaddr > 0xFF || readpos > 0xFF || readlen > 0xFF) {
		readaddr = readpos = readlen = 0;
		goto err;
	}

	LCD_INFO("addr 0x(%x) pos(%d) len (%d)\n", readaddr, readpos, readlen);

	ss_read_mtp(vdd, readaddr, readlen, readpos, readbuf);

err:
	return size;
}

static ssize_t ss_write_mtp_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	char *p, *arg = (char *)buf;
	u8 *tx_buf = NULL;
	int len;
	u8 val = 0;
	int i = 0;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return -ENODEV;
	}

	if (!ss_is_ready_to_send_cmd(vdd)) {
		LCD_ERR("Panel is not ready. Panel State(%d)\n", vdd->panel_state);
		return size;
	}

	p = strsep(&arg, " ");
	if (sscanf(p, "%d", &len) != 1) {
		LCD_INFO("No size for mtp store\n");
		return size;
	}

	if (len <= 0) {
		LCD_INFO("size is wrong.. %d\n", len);
		goto err;
	}

	LCD_INFO("size : %d\n", len);

	tx_buf = kzalloc(len, GFP_KERNEL);
	if (!tx_buf) {
		LCD_INFO("Fail to kmalloc for tx_buf..\n");
		goto err;
	}

	while ((p = strsep(&arg, " ")) != NULL && i < len) {
		if (sscanf(p, "%02x", &val) != 1)
			LCD_INFO("fail to sscanf..\n");
		tx_buf[i++] = val;
		LCD_INFO("arg [%02x] \n", val);
	}

	ss_write_mtp(vdd, len, tx_buf);

	kfree(tx_buf);
	tx_buf = NULL;

err:
	return size;
}


static ssize_t ss_temperature_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int rc = 0;
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return rc;
	}

	if (vdd->br.elvss_interpolation_temperature == -15)
		rc = snprintf((char *)buf, 40, "-15, -14, 0, 1, 30, 40\n");
	else
		rc = snprintf((char *)buf, 40, "-20, -19, 0, 1, 30, 40\n");

	LCD_INFO("temperature : %d elvss_interpolation_temperature : %d\n", vdd->temperature, vdd->br.elvss_interpolation_temperature);

	return rc;
}

static ssize_t ss_temperature_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	int pre_temp = 0, temp;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return size;
	}

	pre_temp = vdd->temperature;

	if (sscanf(buf, "%d", &temp) != 1)
		return size;

	vdd->temperature = temp;

	/* When temperature changed, hbm_mode must setted 0 for EA8061 hbm setting. */
	if (pre_temp != vdd->temperature && vdd->display_status_dsi.hbm_mode == 1)
		vdd->display_status_dsi.hbm_mode = 0;

	if (!ss_is_ready_to_send_cmd(vdd)) {
		LCD_ERR("Panel is not ready. Panel State(%d)\n", vdd->panel_state);
		return size;
	}

	ss_brightness_dcs(vdd, USE_CURRENT_BL_LEVEL, BACKLIGHT_NORMAL);

	LCD_INFO("temperature : %d", vdd->temperature);

	return size;
}

static ssize_t ss_lux_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int rc = 0;
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return rc;
	}

	rc = snprintf((char *)buf, 40, "%d\n", vdd->lux);

	LCD_INFO("lux : %d\n", vdd->lux);

	return rc;
}

static ssize_t ss_lux_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	int pre_lux = 0, temp;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return size;
	}


	pre_lux = vdd->lux;

	if (sscanf(buf, "%d", &temp) != 1)
		return size;

	vdd->lux = temp;

	if (!ss_is_ready_to_send_cmd(vdd)) {
		LCD_ERR("Panel is not ready. Panel State(%d)\n", vdd->panel_state);
		return size;
	}

	if (vdd->mdnie.support_mdnie && pre_lux != vdd->lux)
		update_dsi_tcon_mdnie_register(vdd);

	LCD_INFO("lux : %d", vdd->lux);

	return size;
}

/**
 * ss_read_copr_show()
 *
 * This function read copr and shows current copr value.
 */
static ssize_t ss_read_copr_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	int ret = 0;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd\n");
		return ret;
	}

	if (vdd->copr.copr_on) {
		mutex_lock(&vdd->copr.copr_lock);
		if (ss_copr_read(vdd)) {
			ret = snprintf((char *)buf, 20, "-1\n");
		} else {
			LCD_INFO("%d\n", vdd->copr.current_copr);
			ret = snprintf((char *)buf, 20, "%d\n", vdd->copr.current_copr);
		}
		mutex_unlock(&vdd->copr.copr_lock);
	} else {
		ret = snprintf((char *)buf, 20, "-1 -1\n");
	}

	return ret;
}

/**
 * ss_copr_show()
 *
 * This function shows current copr values.
 */
static ssize_t ss_copr_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	int ret = 0;
	int i, len = 0;
	struct COPR_CMD cmd;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd.\n");
		return ret;
	}

	mutex_lock(&vdd->copr.copr_lock);

	cmd = vdd->copr.cur_cmd;

	len += snprintf(buf + len, 128, "copr_mask=%d copr_cnt_re=%d copr_ilc=%d copr_gamma=%d copr_en=%d ",
							cmd.COPR_MASK, cmd.CNT_RE, cmd.COPR_ILC, cmd.COPR_GAMMA, cmd.COPR_EN);
	len += snprintf(buf + len, 128, "copr_er=%d copr_eg=%d copr_eb=%d copr_erc=%d copr_egc=%d copr_ebc=%d ",
							cmd.COPR_ER, cmd.COPR_EG, cmd.COPR_EB, cmd.COPR_ERC, cmd.COPR_EGC, cmd.COPR_EBC);
	len += snprintf(buf + len, 128, "copr_max_cnt=%d copr_roi_on=%d copr_roi_ctrl=%d ",
							cmd.MAX_CNT, cmd.ROI_ON, cmd.COPR_ROI_CTRL);

	for (i = 0; i < MAX_COPR_ROI_CNT; i++)
		len += snprintf(buf + len, 128, "copr_roi%d_x_s=%d copr_roi%d_y_s=%d copr_roi%d_x_e=%d copr_roi%d_y_e=%d%s",
			i+1, cmd.roi[i].ROI_X_S, i+1, cmd.roi[i].ROI_Y_S, i+1, cmd.roi[i].ROI_X_E, i+1, cmd.roi[i].ROI_Y_E,
			(i == MAX_COPR_ROI_CNT - 1) ? "\n" : " ");

	mutex_unlock(&vdd->copr.copr_lock);

	return strlen(buf);
}

/**
 * ss_copr_store()
 *
 * debugging purpose for light sensor compensation only.
 * user can write copr register and then read copr using read_copr sysfs.
 */
static ssize_t ss_copr_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	struct COPR_CMD cmd;

	char *p, *arg = (char *)buf;
	int rc = 0;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return size;
	}

	if (!ss_is_panel_on(vdd)) {
		LCD_ERR("panel stste (%d) \n", vdd->panel_state);
		return size;
	}

	LCD_INFO("++\n");

	/* copy current copr cmds */
	memcpy(&cmd, &vdd->copr.cur_cmd, sizeof(cmd));

	print_copr_cmd(cmd);

	while ((p = strsep(&arg, " \t")) != NULL) {
		if (!*p) continue;
		rc = ss_copr_set_cmd_offset(&cmd, p);
		if (rc) {
			LCD_ERR("fail to set copr cmd by offset.. \n");
			return size;
		}
	}

	mutex_lock(&vdd->copr.copr_lock);

	/* set copr enable cmds */
	ss_copr_set_cmd(vdd, &cmd);

	print_copr_cmd(cmd);

	ss_send_cmd(vdd, TX_COPR_ENABLE);

	mutex_unlock(&vdd->copr.copr_lock);

	LCD_INFO("--\n");
	return size;
}

/**
 * ss_copr_roi_show()
 *
 * This function read copr for each roi's r/g/b and show the result.
 * If driver has no ROI table, shows -1.
 */
static ssize_t ss_copr_roi_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	int ret = 0;
	int i, len = 0;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd\n");
		return ret;
	}

	if (!vdd->copr.copr_on) {
		LCD_ERR("copr is not on (%d) \n", vdd->copr.copr_on);
		return ret;
	}

	if (!ss_is_panel_on(vdd)) {
		LCD_ERR("panel stste (%d) \n", vdd->panel_state);
		return ret;
	}

	mutex_lock(&vdd->copr.copr_lock);

	LCD_INFO("++ \n");

	if (vdd->copr.afc_roi_cnt) {
		ret = ss_copr_get_roi_opr(vdd);
		if (ret) {
			LCD_ERR("fail to get roi opr..\n");
			ret = snprintf((char *)buf, 20, "-1\n");
		} else {
			for (i = 0; i < vdd->copr.afc_roi_cnt; i++) {
				len += snprintf(buf + len, 20, "%d %d %d%s",
							vdd->copr.roi_opr[i].R_OPR,
							vdd->copr.roi_opr[i].G_OPR,
							vdd->copr.roi_opr[i].B_OPR,
							(i == vdd->copr.afc_roi_cnt - 1) ? "\n" : " ");
			}
		}
	} else {
		LCD_INFO("No ROI table..\n");
		ret = snprintf((char *)buf, 20, "-1\n");
	}

	LCD_INFO("-- \n");

	mutex_unlock(&vdd->copr.copr_lock);

	return strlen(buf);
}

/**
 * ss_copr_roi_store()
 *
 * This function stores roi's r/g/b table.
 * To support AFC, mDNIe service needs opr values for each ROI's.
 */
static ssize_t ss_copr_roi_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	struct COPR_ROI roi[MAX_COPR_ROI_CNT];
	int i, rc = 0;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return size;
	}

	if (!vdd->copr.copr_on) {
		LCD_ERR("copr is not on (%d) \n", vdd->copr.copr_on);
		return size;
	}

	if (!ss_is_panel_on(vdd)) {
		LCD_ERR("panel stste (%d) \n", vdd->panel_state);
		return size;
	}

	mutex_lock(&vdd->copr.copr_lock);

	LCD_INFO("++\n");

	rc = sscanf(buf, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
			&roi[0].ROI_X_S, &roi[0].ROI_Y_S, &roi[0].ROI_X_E, &roi[0].ROI_Y_E,
			&roi[1].ROI_X_S, &roi[1].ROI_Y_S, &roi[1].ROI_X_E, &roi[1].ROI_Y_E,
			&roi[2].ROI_X_S, &roi[2].ROI_Y_S, &roi[2].ROI_X_E, &roi[2].ROI_Y_E,
			&roi[3].ROI_X_S, &roi[3].ROI_Y_S, &roi[3].ROI_X_E, &roi[3].ROI_Y_E,
			&roi[4].ROI_X_S, &roi[4].ROI_Y_S, &roi[4].ROI_X_E, &roi[4].ROI_Y_E);
	if (rc != 20)
		goto err;

	vdd->copr.afc_roi_cnt = rc / 4;

	for (i = 0; i < vdd->copr.afc_roi_cnt; i++) {
		if (roi[i].ROI_X_E == -1 || roi[i].ROI_X_S == -1 ||
			roi[i].ROI_Y_E == -1 || roi[i].ROI_Y_S == -1)
			continue;

		memcpy(&vdd->copr.afc_roi[i], &roi[i], sizeof(struct COPR_ROI));

		LCD_INFO("roi[%d] %d %d %d %d\n",
			i + 1, roi[i].ROI_X_S, roi[i].ROI_Y_S, roi[i].ROI_X_E, roi[i].ROI_Y_E);
	}

	LCD_INFO("--\n");

err:
	mutex_unlock(&vdd->copr.copr_lock);

	return size;
}

/**
 * ss_brt_avg_show()
 *
 * This function shows cd avg (AFC).
 * If not this function returns -1.
 */
static ssize_t ss_brt_avg_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	int ret = 0;
	int idx = COPR_CD_INDEX_1;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd\n");
		return ret;
	}

	if (!ss_is_panel_on(vdd)) {
		LCD_ERR("panel stste (%d) \n", vdd->panel_state);
		return ret;
	}

	LCD_DEBUG("++ \n");

	if (vdd->copr.copr_on) {
		mutex_lock(&vdd->copr.copr_lock);

		/* get cd avg */
		ss_set_copr_sum(vdd, idx);
		vdd->copr.copr_cd[idx].cd_avr = vdd->copr.copr_cd[idx].cd_sum / vdd->copr.copr_cd[idx].total_t;
		LCD_INFO("[%d] cd_avr (%d) cd_sum (%lld) total_t (%lld)\n",
			idx,
			vdd->copr.copr_cd[idx].cd_avr, vdd->copr.copr_cd[idx].cd_sum,
			vdd->copr.copr_cd[idx].total_t);

		vdd->copr.copr_cd[idx].cd_sum = 0;
		vdd->copr.copr_cd[idx].total_t = 0;

		ret = snprintf((char *)buf, 10, "%d\n", vdd->copr.copr_cd[idx].cd_avr);

		mutex_unlock(&vdd->copr.copr_lock);
	} else {
		ret = snprintf((char *)buf, 10, "-1\n");
	}

	LCD_DEBUG("-- \n");

	return strlen(buf);
}

static ssize_t ss_self_mask_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	int enable = 0;
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return size;
	}

	if (!ss_is_ready_to_send_cmd(vdd)) {
		LCD_ERR("Panel is not ready. Panel State(%d)\n", vdd->panel_state);
		return size;
	}

	if (!vdd->self_disp.is_support) {
		LCD_ERR("self display is not supported..(%d) \n",
								vdd->self_disp.is_support);
		return -ENODEV;
	}


	if (sscanf(buf, "%d", &enable) != 1)
		return size;

	LCD_INFO("SELF MASK %s! (%d)\n", enable ? "enable" : "disable", enable);
	if (vdd->self_disp.self_mask_on)
		vdd->self_disp.self_mask_on(vdd, enable);
	else
		LCD_ERR("Self Mask Function is NULL\n");

	return size;
}

/* Dynamic HLPM On/Off Test for Factory */
static ssize_t ss_dynamic_hlpm_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	int enable = 0;
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return size;
	}

	if (!ss_is_panel_lpm(vdd)) {
		LCD_ERR("Dynamic HLPM should be tested in LPM state Only. Panel State(%d)\n", vdd->panel_state);
		return size;
	}

	if (!vdd->self_disp.is_support) {
		LCD_ERR("self display is not supported..(%d) \n",
								vdd->self_disp.is_support);
		return -ENODEV;
	}

	if (sscanf(buf, "%d", &enable) != 1)
		return size;

	LCD_INFO("Dynamic HLPM %s! (%d)\n", enable ? "Enable" : "Disable", enable);

	if (enable)
		ss_send_cmd(vdd, TX_DYNAMIC_HLPM_ENABLE);
	else
		ss_send_cmd(vdd, TX_DYNAMIC_HLPM_DISABLE);

	return size;
}

static ssize_t ss_self_display_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	int input[20];
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return size;
	}

	if (!ss_is_ready_to_send_cmd(vdd)) {
		LCD_ERR("Panel is not ready. Panel State(%d)\n", vdd->panel_state);
		return size;
	}

	vdd->debug_data->print_cmds = true;

	if (sscanf(buf, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
			&input[0], &input[1], &input[2], &input[3],
			&input[4], &input[5], &input[6], &input[7],
			&input[8], &input[9], &input[10], &input[11],
			&input[12], &input[13], &input[14], &input[15],
			&input[16], &input[17], &input[18], &input[19]) != 20)
			return size;

	switch (input[0]) {
	case 0: /* SELF_MOVE */
		if (vdd->self_disp.self_move_set)
			vdd->self_disp.self_move_set(vdd, input[1]);
		break;
 	case 1: /* SELF_ICON */
		vdd->self_disp.si_info.en = input[1];
		vdd->self_disp.si_info.pos_x = input[2];
		vdd->self_disp.si_info.pos_y = input[3];
		vdd->self_disp.si_info.width = input[4];
		vdd->self_disp.si_info.height = input[5];
		vdd->self_disp.si_info.color = input[6];
		if (vdd->self_disp.self_icon_set)
			vdd->self_disp.self_icon_set(vdd);
		break;
	case 2: /* SELF_GRID */
		vdd->self_disp.sg_info.en = input[1];
		vdd->self_disp.sg_info.s_pos_x = input[2];
		vdd->self_disp.sg_info.s_pos_y = input[3];
		vdd->self_disp.sg_info.e_pos_x = input[4];
		vdd->self_disp.sg_info.e_pos_y = input[5];
		if (vdd->self_disp.self_grid_set)
			vdd->self_disp.self_grid_set(vdd);
		break;
	case 3: /* SELF_ANALOG_CLOCK */
		vdd->self_disp.sa_info.en = input[1];
		vdd->self_disp.sa_info.pos_x = input[2];
		vdd->self_disp.sa_info.pos_y = input[3];
		vdd->self_disp.sa_info.rotate = input[4];
		vdd->self_disp.sa_info.mem_mask_en = input[5];
		vdd->self_disp.sa_info.mem_reuse_en = input[6];
		if (vdd->self_disp.self_aclock_set)
			vdd->self_disp.self_aclock_set(vdd);
		break;
	case 4: /* SELF_DIGITAL_CLOCK */
		vdd->self_disp.sd_info.en = input[1];
		vdd->self_disp.sd_info.en_hh = input[2];
		vdd->self_disp.sd_info.en_mm = input[3];
		vdd->self_disp.sd_info.pos1_x = input[4];
		vdd->self_disp.sd_info.pos1_y = input[5];
		vdd->self_disp.sd_info.pos2_x = input[6];
		vdd->self_disp.sd_info.pos2_y = input[7];
		vdd->self_disp.sd_info.pos3_x = input[8];
		vdd->self_disp.sd_info.pos3_y = input[9];
		vdd->self_disp.sd_info.pos4_x = input[10];
		vdd->self_disp.sd_info.pos4_y = input[11];
		vdd->self_disp.sd_info.img_width = input[12];
		vdd->self_disp.sd_info.img_height = input[13];
		vdd->self_disp.sd_info.color = input[14];
		vdd->self_disp.sd_info.unicode_attr = input[15];
		vdd->self_disp.sd_info.unicode_width = input[16];
		if (vdd->self_disp.self_dclock_set)
			vdd->self_disp.self_dclock_set(vdd);
		break;
	case 5: /* SELF_TIME_SET */
		vdd->self_disp.st_info.cur_h = input[1];
		vdd->self_disp.st_info.cur_m = input[2];
		vdd->self_disp.st_info.cur_s = input[3];
		vdd->self_disp.st_info.cur_ms = input[4];
		vdd->self_disp.st_info.disp_24h = input[5];
		vdd->self_disp.st_info.interval = input[6];
		if (vdd->self_disp.self_time_set)
			vdd->self_disp.self_time_set(vdd, false);
		break;
	case 6: /* SELF_PARTIAL_HLPM_SCAN_SET */
		vdd->self_disp.sphs_info.hlpm_en = input[1];
		vdd->self_disp.sphs_info.hlpm_mode_sel = input[2];
		vdd->self_disp.sphs_info.hlpm_area_1 = input[3];
		vdd->self_disp.sphs_info.hlpm_area_2 = input[4];
		vdd->self_disp.sphs_info.hlpm_area_3 = input[5];
		vdd->self_disp.sphs_info.hlpm_area_4 = input[6];
		vdd->self_disp.sphs_info.scan_en = input[7];
		vdd->self_disp.sphs_info.scan_sl = input[8];
		vdd->self_disp.sphs_info.scan_el = input[9];
		if (vdd->self_disp.self_partial_hlpm_scan_set)
			vdd->self_disp.self_partial_hlpm_scan_set(vdd);
		break;
	}

	vdd->debug_data->print_cmds = false;

	return size;
}

static ssize_t ss_disp_partial_disp_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	LCD_DEBUG("TDB");

	return 0;
}

static ssize_t ss_disp_partial_disp_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	LCD_DEBUG("TDB");

	return size;
}

static ssize_t ss_self_mask_check_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	int i, len = 0, res = -1;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		res = -ENODEV;
	}

	if (!vdd->self_disp.is_support) {
		LCD_ERR("self display is not supported..(%d) \n",
								vdd->self_disp.is_support);
		return -ENODEV;
	}

	if (vdd->self_disp.self_mask_check)
		res = vdd->self_disp.self_mask_check(vdd);
	else {
		LCD_ERR("Do not support self mask check..\n");
	}

	len += snprintf(buf + len, 60, "%d ", res);

	if (vdd->self_disp.mask_crc_size) {
		for (i = 0; i < vdd->self_disp.mask_crc_size; i++) {
			len += snprintf(buf + len, 60, "%02x ", vdd->self_disp.mask_crc_read_data[i]);
			vdd->self_disp.mask_crc_read_data[i] = 0x00;
		}
	}

	len += snprintf(buf + len, 60, "\n");

	return strlen(buf);
}

/*
 * Panel LPM related functions
 */
static ssize_t ss_panel_lpm_mode_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int rc = 0;
	struct samsung_display_driver_data *vdd =
	(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	struct panel_func *pfunc;
	u8 current_status = 0;

	pfunc = &vdd->panel_func;

	if (IS_ERR_OR_NULL(pfunc)) {
		LCD_ERR("no pfunc");
		return rc;
	}

	if (vdd->dtsi_data.panel_lpm_enable)
		current_status = vdd->panel_lpm.mode;

	rc = snprintf((char *)buf, 30, "%d\n", current_status);
	LCD_INFO("[Panel LPM] Current status : %d\n", (int)current_status);

	return rc;
}

#define LPM_VER_MASK 0xff0000
#define LPM_MODE_MASK 0xff

static void set_lpm_mode_and_brightness(struct samsung_display_driver_data *vdd)
{
	/*default*/
	u8 mode = ALPM_MODE_ON;
	int bl_level = LPM_2NIT;

	if (vdd->panel_lpm.ver == LPM_VER1)
		mode = vdd->panel_lpm.origin_mode;
	else {
		switch (vdd->panel_lpm.origin_mode) {
		case LPM_MODE_OFF:
			mode = LPM_MODE_OFF;
			break;
		case ALPM_MODE_ON_60NIT:
			mode = ALPM_MODE_ON;
			bl_level = LPM_60NIT;
			break;
		case HLPM_MODE_ON_2NIT:
			mode = HLPM_MODE_ON;
			bl_level = LPM_2NIT;
			break;
		case HLPM_MODE_ON_60NIT:
			mode = HLPM_MODE_ON;
			bl_level = LPM_60NIT;
			break;
		default:
		case ALPM_MODE_ON_2NIT:
			mode = ALPM_MODE_ON;
			bl_level = LPM_2NIT;
			break;
		}
		/*set bl also only in LPM_VER0*/
		vdd->panel_lpm.lpm_bl_level = bl_level;
	}
	vdd->panel_lpm.mode = mode;
}

/*
 *	mode: 0x00aa00bb
 *	0xaa: ver 0: old lpm feature, ver 1: new lpm feature
 *	0xbb: lpm mode
 *		ver 0: 0/1/2/3/4
 *			(off/alpm 2nit/hlpm 2bit/alpm 60nit/ hlpm 60nit)
 *		ver 1: 1/2
 *			(alpm/hlpm)
 */
static void ss_panel_set_lpm_mode(
		struct samsung_display_driver_data *vdd, unsigned int mode)
{

	if (!vdd->dtsi_data.panel_lpm_enable) {
		LCD_INFO("[Panel LPM] LPM(ALPM/HLPM) is not supported\n");
		return;
	}

	mutex_lock(&vdd->panel_lpm.lpm_lock);
	vdd->panel_lpm.origin_mode = (u8)(mode & LPM_MODE_MASK);

	if ((mode & LPM_VER_MASK) >> 16 == LPM_VER0)
		vdd->panel_lpm.ver = LPM_VER0;
	else
		vdd->panel_lpm.ver = LPM_VER1;

	set_lpm_mode_and_brightness(vdd);
	mutex_unlock(&vdd->panel_lpm.lpm_lock);

	if (unlikely(vdd->is_factory_mode)) {
		if (vdd->panel_lpm.mode == LPM_MODE_OFF)
			ss_panel_lpm_ctrl(vdd, false);
		else
			ss_panel_lpm_ctrl(vdd, true);
	}

	// DO not call lpm_ctrl from lpm sysfs.
	// since sdm845, aod service only use HLPM mode. (P180508-04638)
	// If we need to change lpm mode(alpm<->hlpm) while lpm status, consider it again.
	/*
	else {
		if (ss_is_panel_on_ready(vdd)) {
			if (vdd->panel_lpm.mode == LPM_MODE_OFF)
				ss_panel_lpm_ctrl(vdd, false);
			else
				ss_panel_lpm_ctrl(vdd, true);
		}
	}
	*/

	LCD_INFO("[Panel LPM]: ver(%d) mode(%d)brightness(%d)\n",
		vdd->panel_lpm.ver,
		vdd->panel_lpm.mode,
		vdd->panel_lpm.lpm_bl_level);

}

static ssize_t ss_panel_lpm_mode_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	unsigned int mode = 0;
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return size;
	}

	if (sscanf(buf, "%d", &mode) != 1)
		return size;

	LCD_INFO("[Panel LPM] Mode : %d(%x) Index(%d)\n", mode, mode, vdd->ndx);
	ss_panel_set_lpm_mode(vdd, mode);

	return size;
}

static ssize_t mipi_samsung_hmt_bright_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int rc;

	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return -ENODEV;
	}

	if (!vdd->dtsi_data.hmt_enabled) {
		LCD_ERR("hmt is not supported..\n");
		return -ENODEV;
	}

	rc = snprintf((char *)buf, 30, "%d\n", vdd->hmt_stat.hmt_bl_level);
	LCD_INFO("[HMT] hmt bright : %d\n", *buf);

	return rc;
}

static ssize_t mipi_samsung_hmt_bright_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	int input;

	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return -ENODEV;
	}

	if (!vdd->dtsi_data.hmt_enabled) {
		LCD_ERR("hmt is not supported..\n");
		return -ENODEV;
	}

	if (sscanf(buf, "%d", &input) != 1)
		return size;

	LCD_INFO("[HMT] input (%d) ++\n", input);

	if (!vdd->hmt_stat.hmt_on) {
		LCD_INFO("[HMT] hmt is off!\n");
		goto end;
	}

	if (!ss_is_panel_on(vdd)) {
		LCD_ERR("[HMT] panel is not on state (%d) \n", vdd->panel_state);
		vdd->hmt_stat.hmt_bl_level = input;
		goto end;
	}

	if (vdd->hmt_stat.hmt_bl_level == input) {
		LCD_ERR("[HMT] hmt bright already %d!\n", vdd->hmt_stat.hmt_bl_level);
		goto end;
	}

	vdd->hmt_stat.hmt_bl_level = input;
	hmt_bright_update(vdd);
	LCD_INFO("[HMT] input (%d) --\n", input);

end:
	return size;
}

static ssize_t mipi_samsung_hmt_on_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int rc;

	struct samsung_display_driver_data *vdd =
			(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return -ENODEV;
	}

	if (!vdd->dtsi_data.hmt_enabled) {
		LCD_ERR("hmt is not supported..\n");
		return -ENODEV;
	}

	rc = snprintf((char *)buf, 30, "%d\n", vdd->hmt_stat.hmt_on);
	LCD_INFO("[HMT] hmt on input : %d\n", *buf);

	return rc;
}

static ssize_t mipi_samsung_hmt_on_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	int input;
	struct samsung_display_driver_data *vdd =
			(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return -ENODEV;
	}

	if (!vdd->dtsi_data.hmt_enabled) {
		LCD_ERR("hmt is not supported..\n");
		return -ENODEV;
	}

	if (sscanf(buf, "%d", &input) != 1)
		return size;

	LCD_INFO("[HMT] input (%d) ++\n", input);

	if (!ss_is_panel_on(vdd)) {
		LCD_ERR("[HMT] panel is not on state (%d) \n", vdd->panel_state);
		vdd->hmt_stat.hmt_on = input;
		return size;
	}

	if (vdd->hmt_stat.hmt_on == input) {
		LCD_INFO("[HMT] hmt already %s !\n", vdd->hmt_stat.hmt_on?"ON":"OFF");
		return size;
	}

	vdd->hmt_stat.hmt_on = input;

	hmt_enable(vdd);
	hmt_reverse_update(vdd, vdd->hmt_stat.hmt_on);

	hmt_bright_update(vdd);

	LCD_INFO("[HMT] input hmt (%d) --\n",
		vdd->hmt_stat.hmt_on);

	return size;
}

void ss_cabc_update(struct samsung_display_driver_data *vdd)
{
	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return;
	}

	if (!ss_is_ready_to_send_cmd(vdd)) {
		LCD_ERR("Panel is not ready. Panel State(%d)\n", vdd->panel_state);
		return;
	}

	if (vdd->br.auto_level) {
		LCD_INFO("auto brightness is on, cabc cmds are already sent--\n");
		return;
	}

	if (vdd->siop_status) {
		if (vdd->panel_func.samsung_lvds_write_reg)
			vdd->panel_func.samsung_brightness_tft_pwm(vdd, vdd->br.bl_level);
		else {
			ss_send_cmd(vdd, TX_CABC_OFF_DUTY);
			ss_send_cmd(vdd, TX_CABC_ON);
			if (vdd->dtsi_data.cabc_delay && !vdd->display_status_dsi.disp_on_pre)
				usleep_range(vdd->dtsi_data.cabc_delay, vdd->dtsi_data.cabc_delay);
			ss_send_cmd(vdd, TX_CABC_ON_DUTY);
		}
	} else {
		if (vdd->panel_func.samsung_lvds_write_reg)
			vdd->panel_func.samsung_brightness_tft_pwm(vdd, vdd->br.bl_level);
		else {
			ss_send_cmd(vdd, TX_CABC_OFF_DUTY);
			ss_send_cmd(vdd, TX_CABC_OFF);
		}
	}
}

int config_cabc(struct samsung_display_driver_data *vdd, int value)
{
	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return value;
	}

	if (!ss_is_ready_to_send_cmd(vdd)) {
		LCD_ERR("Panel is not ready. Panel State(%d)\n", vdd->panel_state);
		return value;
	}

	if (vdd->siop_status == value) {
		LCD_INFO("No change in cabc state, update not needed--\n");
		return value;
	}

	vdd->siop_status = value;
	ss_cabc_update(vdd);

	return 0;
}

static ssize_t mipi_samsung_mcd_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	int input;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		goto end;
	}

	if (!ss_is_ready_to_send_cmd(vdd)) {
		LCD_ERR("Panel is not ready. Panel State(%d)\n", vdd->panel_state);
		return size;
	}

	if (sscanf(buf, "%d", &input) != 1)
		return size;

	LCD_INFO("(%d)\n", input);

	if (input)
		ss_send_cmd(vdd, TX_MCD_ON);
	else
		ss_send_cmd(vdd, TX_MCD_OFF);
end:
	return size;
}

static ssize_t mipi_samsung_mst_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	int input;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		goto end;
	}

	if (!ss_is_ready_to_send_cmd(vdd)) {
		LCD_ERR("Panel is not ready. Panel State(%d)\n", vdd->panel_state);
		return size;
	}

	if (sscanf(buf, "%d", &input) != 1)
		return size;

	LCD_INFO("(%d)\n", input);

	if (input)
		ss_send_cmd(vdd, TX_MICRO_SHORT_TEST_ON);
	else
		ss_send_cmd(vdd, TX_MICRO_SHORT_TEST_OFF);
end:
	return size;
}

static ssize_t mipi_samsung_grayspot_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	int input;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		goto end;
	}

	if (!ss_is_ready_to_send_cmd(vdd)) {
		LCD_ERR("Panel is not ready. Panel State(%d)\n", vdd->panel_state);
		return size;
	}

	if (sscanf(buf, "%d", &input) != 1)
		return size;

	LCD_INFO("(%d)\n", input);

	if (input) {
		ss_send_cmd(vdd, TX_GRAY_SPOT_TEST_ON);
		vdd->grayspot = 1;
	} else {
		ss_send_cmd(vdd, TX_GRAY_SPOT_TEST_OFF);
		vdd->grayspot = 0;
		/* restore VINT, ELVSS */
		ss_brightness_dcs(vdd, USE_CURRENT_BL_LEVEL, BACKLIGHT_NORMAL);
	}

end:
	return size;
}

static ssize_t mipi_samsung_isc_defect_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	int input;
	struct dsi_display *display = NULL;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		goto end;
	}

	display = GET_DSI_DISPLAY(vdd);
	if (IS_ERR_OR_NULL(display)) {
		LCD_ERR("no display");
		return size;
	}

	if (sscanf(buf, "%d", &input) != 1)
		goto end;

	mutex_lock(&display->display_lock);

	LCD_INFO("(%d)\n", input);

	if (input) {
		ss_send_cmd(vdd, TX_ISC_DEFECT_TEST_ON);
	} else {
		ss_send_cmd(vdd, TX_ISC_DEFECT_TEST_OFF);
	}
	mutex_unlock(&display->display_lock);
end:
	return size;
}

#define MAX_POC_SHOW_WAIT 500
static ssize_t mipi_samsung_poc_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	static int string_size = 20;
	static char check_sum[5] = {0,};
	static char EB_value[4] = {0,};

	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	char temp[string_size];
	int poc;
	u8 *octa_id;
	int wait_cnt;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return -ENODEV;
	}

	if (!ss_is_ready_to_send_cmd(vdd)) {
		LCD_ERR("Panel is not ready. Panel State(%d)\n", vdd->panel_state);
		return -EBUSY;
	}

	// POC : From OCTA_ID
	vdd = ss_check_hall_ic_get_vdd(vdd);
	octa_id = vdd->octa_id_dsi;
	poc = octa_id[1] & 0x0f; /* S6E3HA6_AMS622MR01 poc offset is 1*/
	LCD_INFO("POC :(%d)\n", poc);

	/*
		This is temp code. Shoud be modified with locking or waiting logic.
		UNBLANK & RX operation should be protected mutually.
	*/
	for (wait_cnt = 0; wait_cnt < MAX_POC_SHOW_WAIT; wait_cnt++) {
		msleep(10);

		if (ss_is_panel_on(vdd))
			break;
	}

	if (wait_cnt < MAX_POC_SHOW_WAIT) {
		/*
			Retrun to App
			POC         : 0 or 1
			CHECKSUM    : 0 or 1   (0 : OK / 1 : Fail)
			EB 4th para : 33 or FF
		*/

		ss_panel_data_read(vdd, RX_POC_CHECKSUM, check_sum, LEVEL1_KEY);
		ss_panel_data_read(vdd, RX_POC_STATUS, EB_value, LEVEL1_KEY);
	} else
		LCD_INFO("FAIL TO RX -- wait_cnt : %d max_cnt : %d --\n",
				wait_cnt, MAX_POC_SHOW_WAIT);

	LCD_INFO("CHECKSUM : (%d)(%d)(%d)(%d)(%d)\n",
			check_sum[0], check_sum[1], check_sum[2],
			check_sum[3], check_sum[4]);
	LCD_INFO("POC STATUS : (%d)(%d)(%d)(%d)\n",
			EB_value[0], EB_value[1], EB_value[2], EB_value[3]);
	LCD_INFO("%d %d %02x\n", poc, check_sum[4], EB_value[3]);

	snprintf((char *)temp, sizeof(temp), "%d %d %02x\n",
			poc, check_sum[4], EB_value[3]);
	strlcat(buf, temp, string_size);

	return strnlen(buf, string_size);
}

static ssize_t mipi_samsung_poc_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	unsigned int value;
	int ret = 0;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return -ENODEV;
	}

	if (!ss_is_ready_to_send_cmd(vdd)) {
		LCD_ERR("Panel is not ready. Panel State(%d)\n", vdd->panel_state);
		return size;
	}

	sscanf(buf, "%d ", &value);
	LCD_INFO("INPUT : (%d)\n", value);

	if (value == 1) {
		LCD_INFO("ERASE \n");
		if (vdd->panel_func.samsung_poc_ctrl) {
			ret = vdd->panel_func.samsung_poc_ctrl(vdd, POC_OP_ERASE, buf);
		}
	} else if (value == 2) {
		LCD_INFO("WRITE \n");
	} else if (value == 3) {
		LCD_INFO("READ \n");
	} else if (value == 4) {
		LCD_INFO("STOP\n");
		atomic_set(&vdd->poc_driver.cancel, 1);
	} else if (value == 7) {
		LCD_INFO("ERASE_SECTOR \n");
		if (vdd->panel_func.samsung_poc_ctrl) {
			ret = vdd->panel_func.samsung_poc_ctrl(vdd, POC_OP_ERASE_SECTOR, buf);
		}
	} else {
		LCD_INFO("input check !! \n");
	}

	return size;
}

static ssize_t mipi_samsung_poc_mca_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct samsung_display_driver_data *vdd = (struct samsung_display_driver_data *)dev_get_drvdata(dev);
	int i, len = 0;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return -ENODEV;
	}

	if (!ss_is_ready_to_send_cmd(vdd)) {
		LCD_ERR("Panel is not ready. Panel State(%d)\n", vdd->panel_state);
		return -EBUSY;
	}

	vdd = ss_check_hall_ic_get_vdd(vdd);

	ss_poc_read_mca(vdd);

	if (vdd->poc_driver.mca_data) {
		for (i = 0; i < vdd->poc_driver.mca_size; i++)
			len += snprintf(buf + len, 60, "%02x ", vdd->poc_driver.mca_data[i]);

		len += snprintf(buf + len, 60, "\n");
	}

	return strlen(buf);
}

static ssize_t xtalk_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	int input;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		goto end;
	}

	if (!ss_is_ready_to_send_cmd(vdd)) {
		LCD_ERR("Panel is not ready. Panel State(%d)\n", vdd->panel_state);
		return size;
	}

	if (sscanf(buf, "%d", &input) != 1)
		goto end;

	LCD_INFO("(%d)\n", input);

	if (input) {
		vdd->xtalk_mode = 1;
	} else {
		vdd->xtalk_mode = 0;
	}
	ss_brightness_dcs(vdd, USE_CURRENT_BL_LEVEL, BACKLIGHT_NORMAL);
end:
	return size;
}

#define MAX_GCT_RLT_LEN	14
static ssize_t gct_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	int res = -EINVAL;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		res = -ENODEV;
		goto end;
	}

	if (!ss_is_ready_to_send_cmd(vdd)) {
		LCD_ERR("Panel is not ready. Panel State(%d)\n", vdd->panel_state);
		return -EBUSY;
	}

	if (!vdd->gct.is_support || !vdd->panel_func.samsung_gct_read) {
		res = -EPERM;
		goto end;
	}

	res = vdd->panel_func.samsung_gct_read(vdd);
end:
	snprintf(buf, MAX_GCT_RLT_LEN, "%d 0x%x%x%x%x", res,
			vdd->gct.checksum[3], vdd->gct.checksum[2],
			vdd->gct.checksum[1], vdd->gct.checksum[0]);

	return strlen(buf);
}


static ssize_t gct_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	int ret;

	LCD_INFO("+\n");
	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return -ENODEV;
	}

	if (!ss_is_ready_to_send_cmd(vdd)) {
		LCD_ERR("Panel is not ready. Panel State(%d)\n", vdd->panel_state);
		return -EBUSY;
	}

	if (vdd->gct.is_support && vdd->panel_func.samsung_gct_write)
		ret = vdd->panel_func.samsung_gct_write(vdd);

	LCD_INFO("-\n");

	return size;
}

static ssize_t ss_irc_mode_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	int rc = 0;
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return rc;
	}

	rc = snprintf((char *)buf, 50, "%d\n", vdd->br.irc_mode);
	LCD_INFO("irc_mode : %d\n", vdd->br.irc_mode);

	return rc;
}

static ssize_t ss_irc_mode_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	int input_mode;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		goto end;
	}

	if (!ss_is_ready_to_send_cmd(vdd)) {
		LCD_ERR("Panel is not ready. Panel State(%d)\n", vdd->panel_state);
		return size;
	}

	if (sscanf(buf, "%d", &input_mode) != 1)
		goto end;

	if (input_mode >= IRC_MAX_MODE) {
		LCD_INFO("Invalid arg: %d\n", input_mode);
		goto end;
	}

	if (vdd->br.irc_mode != input_mode) {
		LCD_INFO("irc mode: %d -> %d\n", vdd->br.irc_mode, input_mode);
		vdd->br.irc_mode = input_mode;

		if (!vdd->dtsi_data.tft_common_support)
			ss_brightness_dcs(vdd, USE_CURRENT_BL_LEVEL, BACKLIGHT_NORMAL);
	}
end:
	return size;
}

#if 0
static ssize_t ss_ldu_correction_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return strlen(buf);
}

static ssize_t ss_ldu_correction_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	int value;

	sscanf(buf, "%d", &value);

	if ((value < 0) || (value > 7)) {
		LCD_ERR("out of range %d\n", value);
		return -EINVAL;
	}

	vdd->ldu_correction_state = value;

	LCD_INFO("ldu_correction_state : %d\n", vdd->ldu_correction_state);

	return size;
}
#endif

static ssize_t mipi_samsung_hw_cursor_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	int input[10];

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		goto end;
	}

	if (!ss_is_ready_to_send_cmd(vdd)) {
		LCD_ERR("Panel is not ready. Panel State(%d)\n", vdd->panel_state);
		return size;
	}

	if (sscanf(buf, "%d %d %d %d %d %d %d %x %x %x", &input[0], &input[1],
			&input[2], &input[3], &input[4], &input[5], &input[6],
			&input[7], &input[8], &input[9]) != 10)
			goto end;

	if (!IS_ERR_OR_NULL(vdd->panel_func.ddi_hw_cursor))
		vdd->panel_func.ddi_hw_cursor(vdd, input);
	else
		LCD_ERR("ddi_hw_cursor function is NULL\n");

end:
	return size;
}

static ssize_t ss_adaptive_control_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	int value;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return size;
	}

	if (sscanf(buf, "%d", &value) != 1)
		return size;

	LCD_INFO("ACL value : %x\n", value);
	vdd->gradual_acl_val = value;

	if (!vdd->gradual_acl_val)
		vdd->acl_status = 0;
	else
		vdd->acl_status = 1;

	if (!ss_is_ready_to_send_cmd(vdd)) {
		LCD_ERR("Panel is not ready. Panel State(%d)\n", vdd->panel_state);
		return size;
	}

	if (!vdd->dtsi_data.tft_common_support)
		ss_brightness_dcs(vdd, USE_CURRENT_BL_LEVEL, BACKLIGHT_NORMAL);

	return size;
}

static ssize_t ss_cover_control_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	int rc = 0;

	rc = snprintf((char *)buf, 30, "mode = %d\n", vdd->cover_control);

	return rc;
}

static ssize_t ss_cover_control_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	int value;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return size;
	}

	if (!ss_is_ready_to_send_cmd(vdd)) {
		LCD_ERR("Panel is not ready. Panel State(%d)\n", vdd->panel_state);
		return size;
	}

	if (sscanf(buf, "%d", &value) != 1)
		return size;

	vdd->cover_control = value;

	if (vdd->panel_func.samsung_cover_control)
		vdd->panel_func.samsung_cover_control(vdd);
	else
		LCD_ERR("No cover control function..\n");

	LCD_INFO("Cover Control Status = %d\n", vdd->cover_control);
	return size;
}

static ssize_t ss_disp_SVC_OCTA_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	static int string_size = 50;
	char temp[string_size];
	int *cell_id;
	struct samsung_display_driver_data *vdd;

	vdd = ss_get_vdd(PRIMARY_DISPLAY_NDX);
	cell_id = vdd->cell_id_dsi;

	/*
	*	STANDARD FORMAT (Total is 11Byte)
	*	MAX_CELL_ID : 11Byte
	*	7byte(cell_id) + 2byte(Mdnie x_postion) + 2byte(Mdnie y_postion)
	*/

	snprintf((char *)temp, sizeof(temp),
			"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
		cell_id[0], cell_id[1], cell_id[2], cell_id[3], cell_id[4],
		cell_id[5], cell_id[6],
		(vdd->mdnie.mdnie_x & 0xFF00) >> 8,
		vdd->mdnie.mdnie_x & 0xFF,
		(vdd->mdnie.mdnie_y & 0xFF00) >> 8,
		vdd->mdnie.mdnie_y & 0xFF);

	strlcat(buf, temp, string_size);

	return strnlen(buf, string_size);
}

static ssize_t ss_disp_SVC_OCTA2_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	static int string_size = 50;
	char temp[string_size];
	int *cell_id;
	struct samsung_display_driver_data *vdd;

	vdd = ss_get_vdd(SECONDARY_DISPLAY_NDX);
	cell_id = vdd->cell_id_dsi;

	/*
	*	STANDARD FORMAT (Total is 11Byte)
	*	MAX_CELL_ID : 11Byte
	*	7byte(cell_id) + 2byte(Mdnie x_postion) + 2byte(Mdnie y_postion)
	*/

	snprintf((char *)temp, sizeof(temp),
			"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
		cell_id[0], cell_id[1], cell_id[2], cell_id[3], cell_id[4],
		cell_id[5], cell_id[6],
		(vdd->mdnie.mdnie_x & 0xFF00) >> 8,
		vdd->mdnie.mdnie_x & 0xFF,
		(vdd->mdnie.mdnie_y & 0xFF00) >> 8,
		vdd->mdnie.mdnie_y & 0xFF);

	strlcat(buf, temp, string_size);

	return strnlen(buf, string_size);
}

static ssize_t ss_disp_SVC_OCTA_CHIPID_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	static int string_size = 50;
	char temp[string_size];
	u8 *octa_id;
	int site, rework, poc, max_brightness;
	struct samsung_display_driver_data *vdd;

	vdd = ss_get_vdd(PRIMARY_DISPLAY_NDX);
	octa_id = vdd->octa_id_dsi;

	site = octa_id[0] & 0xf0;
	site >>= 4;
	rework = octa_id[0] & 0x0f;
	poc = octa_id[1] & 0x0f;
	max_brightness = octa_id[2] * 256 + octa_id[3];

	snprintf((char *)temp, sizeof(temp),
			"%d%d%d%02x%02x%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",
		site, rework, poc, octa_id[2], octa_id[3],
		octa_id[4], octa_id[5], octa_id[6], octa_id[7],
		octa_id[8], octa_id[9], octa_id[10], octa_id[11],
		octa_id[12], octa_id[13], octa_id[14], octa_id[15],
		octa_id[16], octa_id[17], octa_id[18], octa_id[19]);

	strlcat(buf, temp, string_size);

	return strnlen(buf, string_size);
}

static ssize_t ss_disp_SVC_OCTA2_CHIPID_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	static int string_size = 50;
	char temp[string_size];
	u8 *octa_id;
	int site, rework, poc, max_brightness;
	struct samsung_display_driver_data *vdd;

	vdd = ss_get_vdd(SECONDARY_DISPLAY_NDX);
	octa_id = vdd->octa_id_dsi;

	site = octa_id[0] & 0xf0;
	site >>= 4;
	rework = octa_id[0] & 0x0f;
	poc = octa_id[1] & 0x0f;
	max_brightness = octa_id[2] * 256 + octa_id[3];

	snprintf((char *)temp, sizeof(temp),
			"%d%d%d%02x%02x%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",
		site, rework, poc, octa_id[2], octa_id[3],
		octa_id[4], octa_id[5], octa_id[6], octa_id[7],
		octa_id[8], octa_id[9], octa_id[10], octa_id[11],
		octa_id[12], octa_id[13], octa_id[14], octa_id[15],
		octa_id[16], octa_id[17], octa_id[18], octa_id[19]);

	strlcat(buf, temp, string_size);

	return strnlen(buf, string_size);
}

static ssize_t mipi_samsung_esd_check_show(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	struct irqaction *action = NULL;
	struct irq_desc *desc = NULL;
	int rc = 0;
	int i;
	int irq;
	unsigned long flags;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("%s: Invalid data vdd : 0x%zx\n", __func__, (size_t)vdd);
		goto end;
	}

	if (!vdd->esd_recovery.num_of_gpio) {
		LCD_INFO("None of gpio registered for esd irq\n");
		goto end;
	}

	LCD_INFO("num gpio (%d)\n", vdd->esd_recovery.num_of_gpio);

	for (i = 0; i < vdd->esd_recovery.num_of_gpio; i++) {
		irq = gpio_to_irq(vdd->esd_recovery.esd_gpio[i]);
		desc = irq_to_desc(irq);
		action = desc->action;

		if (action && action->thread_fn) {
			LCD_ERR("[%d] gpio (%d) irq (%d) handler (%s)\n",
				i, vdd->esd_recovery.esd_gpio[i], irq, action->name);

			spin_lock_irqsave(&vdd->esd_recovery.irq_lock, flags);
			generic_handle_irq(irq);
			spin_unlock_irqrestore(&vdd->esd_recovery.irq_lock, flags);

		} else {
			LCD_ERR("No handler for irq(%d)\n", irq);
		}
	}

end:
	return rc;
}

static ssize_t ss_rf_info_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("vdd is null or error\n");
		return -ENODEV;
	}

	snprintf(buf, 50, "RF INFO: RAT(%d), BAND(%d), ARFCN(%d)\n",
		vdd->dyn_mipi_clk.rf_info.rat,
		vdd->dyn_mipi_clk.rf_info.band,
		vdd->dyn_mipi_clk.rf_info.arfcn);

	return strlen(buf);
}

static ssize_t ss_rf_info_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	int temp[3];

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("vdd is null or error\n");
		return -ENODEV;
	}

	if (!vdd->dyn_mipi_clk.is_support) {
		LCD_ERR("ndx: %d, dynamic mipi clk is not supported..\n", vdd->ndx);
		return size;
	}

	if (sscanf(buf, "%d %d %d\n", &temp[0], &temp[1], &temp[2]) != 3)
		return size;

	vdd->dyn_mipi_clk.rf_info.rat = temp[0];
	vdd->dyn_mipi_clk.rf_info.band = temp[1];
	vdd->dyn_mipi_clk.rf_info.arfcn = temp[2];

	queue_work(vdd->dyn_mipi_clk.change_clk_wq, &vdd->dyn_mipi_clk.change_clk_work);

	LCD_INFO("RAT(%d), BAND(%d), ARFCN(%d)\n",
			vdd->dyn_mipi_clk.rf_info.rat,
			vdd->dyn_mipi_clk.rf_info.band,
			vdd->dyn_mipi_clk.rf_info.arfcn);
	return size;
}

static ssize_t ss_dynamic_freq_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	struct clk_timing_table timing_table;
	int i, len = 0;


	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("vdd is null or error\n");
		return -ENODEV;
	}

	timing_table = vdd->dyn_mipi_clk.clk_timing_table;

	len += snprintf(buf + len, 100, "idx clk_rate\n");
	for (i = 1; i < timing_table.tab_size; i++)
		len += snprintf(buf + len, 100, "[%d] %d\n", i, timing_table.clk_rate[i]);
	len += snprintf(buf + len, 100, "Write [idx] to dynamic_freq node to set clk_rate.\n");
	len += snprintf(buf + len, 100, "To revert it (use rf info), Write 0 to dynamic_freq node.\n");

	return strlen(buf);
}

/*
 * ss_dynamic_freq_store()
 * 0 : revert fixed idx (use rf_info notifier)
 * others : fix table idx for mipi_clk/ffc to tesst purpose
 */
static ssize_t ss_dynamic_freq_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	int val;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("vdd is null or error\n");
		return -ENODEV;
	}

	if (!vdd->dyn_mipi_clk.is_support) {
		LCD_ERR("ndx: %d, dynamic mipi clk is not supported..\n", vdd->ndx);
		return size;
	}

	if (sscanf(buf, "%d\n", &val) != 1)
		return size;

	vdd->dyn_mipi_clk.force_idx = val;

	queue_work(vdd->dyn_mipi_clk.change_clk_wq, &vdd->dyn_mipi_clk.change_clk_work);

	LCD_INFO("dyn_mipi_clk.force_idx = %d\n", vdd->dyn_mipi_clk.force_idx);

	return size;
}


static int dpui_notifier_callback(struct notifier_block *self,
				 unsigned long event, void *data)
{
	struct samsung_display_driver_data *vdd = container_of(self,
			struct samsung_display_driver_data, dpui_notif);
	struct dpui_info *dpui = data;
	char tbuf[MAX_DPUI_VAL_LEN];
	int *cell_id;
	int year, mon, day, hour, min, sec;
	int lcd_id;
	int size;
	u8 *octa_id;
	int site, rework, poc;
	int flash_gamma_status;

	if (dpui == NULL) {
		LCD_ERR("err: dpui is null\n");
		return 0;
	}

	if (vdd == NULL) {
		LCD_ERR("err: vdd is null\n");
		return 0;
	}

	vdd = ss_check_hall_ic_get_vdd(vdd);

	/* manufacture date and time */
	cell_id = vdd->cell_id_dsi;
	year = ((cell_id[0] >> 4) & 0xF) + 2011;
	mon = cell_id[0] & 0xF;
	day = cell_id[1] & 0x1F;
	hour = cell_id[2] & 0x1F;
	min = cell_id[3] & 0x3F;
	sec = cell_id[4] & 0x3F;

	size = snprintf(tbuf, MAX_DPUI_VAL_LEN, "%04d%02d%02d %02d%02d%02d", year, mon, day, hour, min, sec);
	set_dpui_field(DPUI_KEY_MAID_DATE, tbuf, size);

	/* lcd id */
	lcd_id = vdd->manufacture_id_dsi;

	size = snprintf(tbuf, MAX_DPUI_VAL_LEN, "%d", ((lcd_id  & 0xFF0000) >> 16));
	set_dpui_field(DPUI_KEY_LCDID1, tbuf, size);
	size = snprintf(tbuf, MAX_DPUI_VAL_LEN, "%d", ((lcd_id  & 0xFF00) >> 8));
	set_dpui_field(DPUI_KEY_LCDID2, tbuf, size);
	size = snprintf(tbuf, MAX_DPUI_VAL_LEN, "%d", (lcd_id  & 0xFF));
	set_dpui_field(DPUI_KEY_LCDID3, tbuf, size);

	/* cell id */
	size = snprintf(tbuf, MAX_DPUI_VAL_LEN, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
			cell_id[0], cell_id[1], cell_id[2], cell_id[3], cell_id[4],
			cell_id[5], cell_id[6],
			(vdd->mdnie.mdnie_x & 0xFF00) >> 8,
			vdd->mdnie.mdnie_x & 0xFF,
			(vdd->mdnie.mdnie_y & 0xFF00) >> 8,
			vdd->mdnie.mdnie_y & 0xFF);

	set_dpui_field(DPUI_KEY_CELLID, tbuf, size);

	/* OCTA ID */
	octa_id = vdd->octa_id_dsi;

	site = octa_id[0] & 0xf0;
	site >>= 4;
	rework = octa_id[0] & 0x0f;
	poc = octa_id[1] & 0x0f;

	size =  snprintf(tbuf, MAX_DPUI_VAL_LEN, "%d%d%d%02x%02x%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
		site, rework, poc, octa_id[2], octa_id[3],
		octa_id[4] != 0 ? octa_id[4] : '0',
		octa_id[5] != 0 ? octa_id[5] : '0',
		octa_id[6] != 0 ? octa_id[6] : '0',
		octa_id[7] != 0 ? octa_id[7] : '0',
		octa_id[8] != 0 ? octa_id[8] : '0',
		octa_id[9] != 0 ? octa_id[9] : '0',
		octa_id[10] != 0 ? octa_id[10] : '0',
		octa_id[11] != 0 ? octa_id[11] : '0',
		octa_id[12] != 0 ? octa_id[12] : '0',
		octa_id[13] != 0 ? octa_id[13] : '0',
		octa_id[14] != 0 ? octa_id[14] : '0',
		octa_id[15] != 0 ? octa_id[15] : '0',
		octa_id[16] != 0 ? octa_id[16] : '0',
		octa_id[17] != 0 ? octa_id[17] : '0',
		octa_id[18] != 0 ? octa_id[18] : '0',
		octa_id[19] != 0 ? octa_id[19] : '0');

	set_dpui_field(DPUI_KEY_OCTAID, tbuf, size);

	/*  Panel Gamma Flash Loading Result */
	flash_gamma_status = flash_gamma_mode_check(vdd);
	size = snprintf(tbuf, MAX_DPUI_VAL_LEN, "%d", flash_gamma_status);
	set_dpui_field(DPUI_KEY_PNGFLS, tbuf, size);

	/* ub_con cnt */
	inc_dpui_u32_field(DPUI_KEY_UB_CON, vdd->ub_con_det.ub_con_cnt);
	vdd->ub_con_det.ub_con_cnt = 0;

	return 0;
}

static int ss_register_dpui(struct samsung_display_driver_data *vdd)
{
	memset(&vdd->dpui_notif, 0,
			sizeof(vdd->dpui_notif));
	vdd->dpui_notif.notifier_call = dpui_notifier_callback;

	return dpui_logging_register(&vdd->dpui_notif, DPUI_TYPE_PANEL);
}

/*
 * HW PARAM LOGGING SYSFS NODE
 */
static ssize_t ss_dpui_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;

	update_dpui_log(DPUI_LOG_LEVEL_INFO, DPUI_TYPE_PANEL);
	ret = get_dpui_log(buf, DPUI_LOG_LEVEL_INFO, DPUI_TYPE_PANEL);
	if (ret < 0) {
		pr_err("%s failed to get log %d\n", __func__, ret);
		return ret;
	}

	pr_info("%s\n", buf);

	return ret;
}

static ssize_t ss_dpui_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	if (buf[0] == 'C' || buf[0] == 'c')
		clear_dpui_log(DPUI_LOG_LEVEL_INFO, DPUI_TYPE_PANEL);

	return size;
}

/*
 * [DEV ONLY]
 * HW PARAM LOGGING SYSFS NODE
 */
static ssize_t ss_dpui_dbg_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;
	pr_info("ss_dpui_dbg_show++++++++++++\n");

	update_dpui_log(DPUI_LOG_LEVEL_DEBUG, DPUI_TYPE_PANEL);
	ret = get_dpui_log(buf, DPUI_LOG_LEVEL_DEBUG, DPUI_TYPE_PANEL);
	if (ret < 0) {
		pr_err("%s failed to get log %d\n", __func__, ret);
		return ret;
	}
	pr_info("ss_dpui_dbg_show-----------\n");
	pr_info("%s\n", buf);

	return ret;
}

static ssize_t ss_dpui_dbg_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	if (buf[0] == 'C' || buf[0] == 'c')
		clear_dpui_log(DPUI_LOG_LEVEL_DEBUG, DPUI_TYPE_PANEL);

	return size;
}

/*
 * [AP DEPENDENT ONLY]
 * HW PARAM LOGGING SYSFS NODE
 */
static ssize_t ss_dpci_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;

	update_dpui_log(DPUI_LOG_LEVEL_INFO, DPUI_TYPE_CTRL);
	ret = get_dpui_log(buf, DPUI_LOG_LEVEL_INFO, DPUI_TYPE_CTRL);
	if (ret < 0) {
		pr_err("%s failed to get log %d\n", __func__, ret);
		return ret;
	}

	pr_info("%s\n", buf);

	return ret;
}

static ssize_t ss_dpci_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	if (buf[0] == 'C' || buf[0] == 'c')
		clear_dpui_log(DPUI_LOG_LEVEL_INFO, DPUI_TYPE_CTRL);

	return size;
}

/*
 * [AP DEPENDENT DEV ONLY]
 * HW PARAM LOGGING SYSFS NODE
 */
static ssize_t ss_dpci_dbg_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int ret;

	update_dpui_log(DPUI_LOG_LEVEL_DEBUG, DPUI_TYPE_CTRL);
	ret = get_dpui_log(buf, DPUI_LOG_LEVEL_DEBUG, DPUI_TYPE_CTRL);
	if (ret < 0) {
		pr_err("%s failed to get log %d\n", __func__, ret);
		return ret;
	}

	pr_info("%s\n", buf);
	return ret;
}

static ssize_t ss_dpci_dbg_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	if (buf[0] == 'C' || buf[0] == 'c')
		clear_dpui_log(DPUI_LOG_LEVEL_DEBUG, DPUI_TYPE_CTRL);

	return size;
}

#if 0
// u8 csc_update = 1;
// u8 csc_change = 0;

static ssize_t csc_read_cfg(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	ssize_t ret = 0;

	ret = snprintf(buf, PAGE_SIZE, "%d\n", csc_update);
	return ret;
}

static ssize_t csc_write_cfg(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	ssize_t ret = strnlen(buf, PAGE_SIZE);
	int err;
	int mode;

	err =  kstrtoint(buf, 0, &mode);
	if (err)
		return ret;

	csc_update = (u8)mode;
	csc_change = 1;
	LCD_INFO("csc ctrl set to csc_update(%d)\n", csc_update);

	return ret;
}
#endif


#if defined(CONFIG_FOLDER_HALL)
static ssize_t ss_force_flip_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	int value;

	if (sscanf(buf, "%d", &value) != 1)
		return size;

	LCD_INFO("flip to %s panel\n", value ? "sub" : "main");

	if (value != 0) //flip to sub panel
		samsung_display_hall_ic_status(NULL, LCD_FLIP_NOT_REFRESH | HALL_IC_CLOSE, NULL);
	else //flip to main panel
		samsung_display_hall_ic_status(NULL, LCD_FLIP_NOT_REFRESH | HALL_IC_OPEN, NULL);

	return size;
}
#endif


static ssize_t ss_reading_mode_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int rc = 0;
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return -ENODEV;
	}

	if(!vdd->reading_mode.support_reading_mode) {
		LCD_ERR("not support\n");
		return 0;
	}

	rc = snprintf((char *)buf, sizeof(vdd->reading_mode.cur_level), "%d\n", vdd->reading_mode.cur_level);

	LCD_INFO("reading mode: %d\n", vdd->reading_mode.cur_level);

	return rc;
}

static ssize_t ss_reading_mode_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	int enable = 0, idx = 0;
	int level;
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return -ENODEV;
	}

	vdd = ss_check_hall_ic_get_vdd(vdd);
	if(!vdd->reading_mode.support_reading_mode) {
		LCD_ERR("not support\n");
		return size;
	}

	sscanf(buf, "%d %d", &enable, &idx);
	LCD_ERR("%s: enable = %d, idx = %d\n", __func__, enable, idx);

	if(enable == 0)
		level = vdd->reading_mode.max_level;
	else {
		level = idx;
		if(level < 0)
			level = 0;
		else if(level >= vdd->reading_mode.max_level)
			level = vdd->reading_mode.max_level - 1;
	}

	if(level != vdd->reading_mode.cur_level) {
		vdd->reading_mode.cur_level = level;
		if (!ss_is_ready_to_send_cmd(vdd)) {
			LCD_ERR("Panel is not ready. Panel State(%d)\n", vdd->panel_state);
			return size;
		}
		ss_reading_mode_dcs(vdd);
	}
	return size;
}


#define MAX_FLASH_GAMMA_LEN	128
enum FLASH_GAMMA_TEST {
	CHECKSUM_0XC8_FAIL_NOT_LOADING = -4,
	CHECKSUM_FLASH_FAIL_NOT_LOADING = -3,
	WRITE_CHECK_NOT_SET_NOT_LOADING = -2,
	READ_FAIL_NOT_LOADING = -1,
	READING_OPERATION_PROGRESS = 0,
	READING_OPERATION_DONE =1,
};

int flash_gamma_mode_check(struct samsung_display_driver_data *vdd)
{
	int res = READ_FAIL_NOT_LOADING;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return res;
	}

	if (!vdd->dtsi_data.flash_gamma_support) {
		LCD_ERR("not support flash_gamma");
		return res;
	}

	if (vdd->panel_br_info.flash_data.init_done) {
		if (vdd->panel_br_info.flash_data.write_check == FLASH_GAMMA_BURN_EMPTY)
			res = WRITE_CHECK_NOT_SET_NOT_LOADING;
		else if (vdd->panel_br_info.flash_data.check_sum_flash_data !=
				vdd->panel_br_info.flash_data.check_sum_cal_data)
			res = CHECKSUM_FLASH_FAIL_NOT_LOADING;
		else if (vdd->panel_br_info.flash_data.c8_register.check_sum_flash_data !=
				vdd->panel_br_info.flash_data.c8_register.check_sum_cal_data)
			res = CHECKSUM_0XC8_FAIL_NOT_LOADING;
		else if (vdd->panel_br_info.flash_data.c8_register.check_sum_flash_data !=
				vdd->panel_br_info.flash_data.c8_register.check_sum_mtp_data)
			res = CHECKSUM_0XC8_FAIL_NOT_LOADING;
		else if (vdd->panel_br_info.flash_data.c8_register.check_sum_cal_data !=
				vdd->panel_br_info.flash_data.c8_register.check_sum_mtp_data)
			res = CHECKSUM_0XC8_FAIL_NOT_LOADING;
		else
			res = READING_OPERATION_DONE;
	} else {
		res = READING_OPERATION_PROGRESS;
	}

	LCD_INFO("flash_gamma_mode (%d)", res);
	return res;
}

static ssize_t ss_disp_flash_gamma_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct samsung_display_driver_data *vdd =
			(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	int res = READ_FAIL_NOT_LOADING;
	int len = 0;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		goto end;
	}

	if (!vdd->dtsi_data.flash_gamma_support) {
		LCD_ERR("not support flash_gamma");
		goto end;
	}

	res = flash_gamma_mode_check(vdd);

end:
	len = snprintf(buf, MAX_FLASH_GAMMA_LEN, "%d %08x %08x %08x %08x %08x\n",
			res,
			vdd->panel_br_info.flash_data.check_sum_cal_data,
			vdd->panel_br_info.flash_data.check_sum_flash_data,
			vdd->panel_br_info.flash_data.c8_register.check_sum_mtp_data,
			vdd->panel_br_info.flash_data.c8_register.check_sum_cal_data,
			vdd->panel_br_info.flash_data.c8_register.check_sum_flash_data);

	LCD_INFO("%s", buf);

	return len;
}

static ssize_t ss_disp_flash_gamma_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	struct dsi_display *display = NULL;

	int input;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		goto end;
	}

	display = GET_DSI_DISPLAY(vdd);
	if (IS_ERR_OR_NULL(display)) {
		LCD_ERR("no display");
		return size;
	}

	if (!ss_is_ready_to_send_cmd(vdd)) {
		LCD_ERR("Panel is not ready. Panel State(%d)\n", vdd->panel_state);
		return size;
	}

	if (!vdd->dtsi_data.flash_gamma_support)
		goto end;

	if (sscanf(buf, "%d", &input) != 1)
		return size;

	LCD_INFO("(%d)\n", input);

	vdd->panel_br_info.flash_data.force_update = true;
	vdd->panel_br_info.flash_data.init_done = false;

	vdd->panel_br_info.flash_data.write_check = FLASH_GAMMA_BURN_EMPTY;

	vdd->panel_br_info.flash_data.check_sum_flash_data = MMC_CHECK_SUM_INIT;
	vdd->panel_br_info.flash_data.check_sum_cal_data = MMC_CHECK_SUM_INIT;

	vdd->panel_br_info.flash_data.c8_register.check_sum_mtp_data = MMC_CHECK_SUM_INIT;
	vdd->panel_br_info.flash_data.c8_register.check_sum_flash_data = MMC_CHECK_SUM_INIT;
	vdd->panel_br_info.flash_data.c8_register.check_sum_cal_data = MMC_CHECK_SUM_INIT;

	set_bit(BOOST_DSI_CLK, vdd->panel_br_info.flash_br_boosting);
	set_bit(BOOST_MNOC, vdd->panel_br_info.flash_br_boosting);
	set_bit(BOOST_CPU, vdd->panel_br_info.flash_br_boosting);

	if (input == 0)
		vdd->panel_br_info.flash_data.force_table_interpolatioin = 1;
	else
		vdd->panel_br_info.flash_data.force_table_interpolatioin = 0;

	/* execution flash gamma read operation */
	queue_delayed_work(vdd->flash_br_workqueue, &vdd->flash_br_work, msecs_to_jiffies(0));

end:
	return size;
}

unsigned char flash_readbuf[SZ_256] = {0x0, };
unsigned int flash_readaddr, flash_readlen;

static ssize_t ss_read_flash_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int i, len = 0;
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return -ENODEV;
	}

	if (flash_readlen && (flash_readlen < SZ_256)) {
		for (i = 0; i < flash_readlen; i++)
			len += snprintf(buf + len, 10, "%02x%s", flash_readbuf[i],
					((i + 1) % 16) == 0 || (i == flash_readlen - 1) ? "\n" : " ");
	} else {
		len += snprintf(buf + len, 100, "No read data.. \n");
		len += snprintf(buf + len, 100, "Please write (addr len) to read_flash store to read flash data\n");
		len += snprintf(buf + len, 100, "MAX read size is 256byte at once\n");
		len += snprintf(buf + len, 100, "ex) echo 0x0A0000 0x100 > read_flash \n");
		len += snprintf(buf + len, 100, "- read 0x100(256) bytes from 0x0A000 flash address\n");
	}

	return strlen(buf);
}

static ssize_t ss_read_flash_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	struct dsi_display *display = NULL;
	int addr, loop;
	unsigned int temp[2];

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return -ENODEV;
	}

	display = GET_DSI_DISPLAY(vdd);
	if (IS_ERR_OR_NULL(display)) {
		LCD_ERR("no display");
		return -ENODEV;
	}

	if (sscanf(buf, "%x %x", &temp[0], &temp[1]) != 2)
		return size;

	mutex_lock(&display->display_lock);

	flash_readaddr = temp[0];
	flash_readlen = temp[1];

	if (flash_readaddr > SZ_1M || flash_readlen > SZ_256)
		goto err;

	LCD_INFO("addr 0x(%x) len 0x(%x)\n", flash_readaddr, flash_readlen);

	mutex_lock(&vdd->exclusive_tx.ex_tx_lock);
	vdd->exclusive_tx.permit_frame_update = 1;
	vdd->exclusive_tx.enable = 1;

	ss_set_exclusive_tx_packet(vdd, TX_FLASH_GAMMA_PRE1, 1);
	ss_set_exclusive_tx_packet(vdd, TX_FLASH_GAMMA_PRE2, 1);
	ss_set_exclusive_tx_packet(vdd, TX_FLASH_GAMMA, 1);
	ss_set_exclusive_tx_packet(vdd, TX_FLASH_GAMMA_POST, 1);
	ss_set_exclusive_tx_packet(vdd, RX_FLASH_GAMMA, 1);

	if (vdd->poc_driver.check_read_case) {
		if (vdd->poc_driver.read_case == READ_CASE1)
			ss_send_cmd(vdd, TX_FLASH_GAMMA_PRE1);
		else if (vdd->poc_driver.read_case == READ_CASE2)
			ss_send_cmd(vdd, TX_FLASH_GAMMA_PRE2);
	} else
		ss_send_cmd(vdd, TX_FLASH_GAMMA_PRE1);

	memset(flash_readbuf, 0x0, sizeof(flash_readbuf));

	for (loop = 0, addr = flash_readaddr; addr < (flash_readaddr + flash_readlen);addr++, loop++)
		flash_readbuf[loop] = flash_read_one_byte(vdd, addr);

	ss_send_cmd(vdd, TX_FLASH_GAMMA_POST);

	ss_set_exclusive_tx_packet(vdd, TX_FLASH_GAMMA_PRE1, 0);
	ss_set_exclusive_tx_packet(vdd, TX_FLASH_GAMMA_PRE2, 0);
	ss_set_exclusive_tx_packet(vdd, TX_FLASH_GAMMA, 0);
	ss_set_exclusive_tx_packet(vdd, TX_FLASH_GAMMA_POST, 0);
	ss_set_exclusive_tx_packet(vdd, RX_FLASH_GAMMA, 0);

	vdd->exclusive_tx.permit_frame_update = 0;
	vdd->exclusive_tx.enable = 0;
	wake_up_all(&vdd->exclusive_tx.ex_tx_waitq);
	mutex_unlock(&vdd->exclusive_tx.ex_tx_lock);

err:
	mutex_unlock(&display->display_lock);

	return size;
}

static ssize_t ss_spi_if_sel_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	int val;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		goto end;
	}

	if (!ss_is_ready_to_send_cmd(vdd)) {
		LCD_ERR("Panel is not ready. Panel State(%d)\n", vdd->panel_state);
		return size;
	}

	if (sscanf(buf, "%d", &val) != 1)
		return size;

	if (val)
		ss_send_cmd(vdd, TX_SPI_IF_SEL_ON);
	else
		ss_send_cmd(vdd, TX_SPI_IF_SEL_OFF);

	LCD_INFO(" %d\n", val);

end:
	return size;
}

/**
 * ss_ccd_state_show()
 *
 * This function reads ccd state.
 */
static ssize_t ss_ccd_state_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	int ret = 0;
	char ccd[1];

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd\n");
		return ret;
	}

	if (!ss_is_ready_to_send_cmd(vdd)) {
		LCD_ERR("Panel is not ready. Panel State(%d)\n", vdd->panel_state);
		return ret;
	}

	ss_send_cmd(vdd, TX_CCD_ON);

	ret = ss_panel_data_read(vdd, RX_CCD_STATE, ccd, LEVEL1_KEY);
	if (!ret) {
		LCD_INFO("CCD return (0x%02x)\n", ccd[0]);

		if (ccd[0] == vdd->ccd_pass_val)
			ret = snprintf((char *)buf, 6, "1\n");
		else if (ccd[0] == vdd->ccd_fail_val)
			ret = snprintf((char *)buf, 6, "0\n");
		else
			ret = snprintf((char *)buf, 6, "-1\n");
	} else {
		ret = snprintf((char *)buf, 6, "-1\n");
	}

	ss_send_cmd(vdd, TX_CCD_OFF);

	return ret;
}

static ssize_t ss_isc_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	struct dsi_panel_cmd_set *isc_cmds = ss_get_cmds(vdd, TX_ISC_DATA_THRESHOLD);
	int val = 0;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		goto end;
	}

	if (!ss_is_ready_to_send_cmd(vdd)) {
		LCD_ERR("Panel is not ready. Panel State(%d)\n", vdd->panel_state);
		return size;
	}

	if (SS_IS_CMDS_NULL(isc_cmds)) {
		LCD_ERR("no cmds for TX_ISC_DATA_THRESHOLD..\n");
		return size;
	}

	if (sscanf(buf, "%d", &val) != 1)
		return size;

	if (val > 0xFF) {
		LCD_ERR("abnoral value (%x)\n", val);
		val = 0xFF;
	}

	isc_cmds->cmds[2].msg.tx_buf[1] = val;

	ss_send_cmd(vdd, TX_ISC_DATA_THRESHOLD);

	LCD_INFO("isc data threshold : %02x\n", val);

end:
	return size;
}

static ssize_t ss_stm_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	struct STM_CMD cmd;

	char *p, *arg = (char *)buf;
	int rc = 0;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return size;
	}

	if (!ss_is_panel_on(vdd)) {
		LCD_ERR("panel stste (%d) \n", vdd->panel_state);
		return size;
	}

	if (!vdd->stm.stm_on) {
		LCD_ERR("stm is not on (%d) \n", vdd->stm.stm_on);
		return size;
	}

	LCD_INFO("++\n");

	/* stm current stm cmds */
	memcpy(&cmd, &vdd->stm.cur_cmd, sizeof(cmd));

	print_stm_cmd(cmd);

	while ((p = strsep(&arg, " \t")) != NULL) {
		if (!*p) continue;
		rc = ss_stm_set_cmd_offset(&cmd, p);
		if (rc) {
			LCD_ERR("fail to set stm cmd by offset.. \n");
			return size;
		}
	}

	/* set stm enable cmds */
	ss_stm_set_cmd(vdd, &cmd);

	print_stm_cmd(cmd);

	ss_send_cmd(vdd, TX_STM_ENABLE);

	LCD_INFO("--\n");
	return size;
}

/* SAMSUNG_FINGERPRINT */
static ssize_t ss_finger_hbm_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	int value;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return size;
	}

	sscanf(buf, "%d", &value);

	LCD_INFO("mask_bl_level value : %d\n", value);
	vdd->br.finger_mask_bl_level = value;

	return size;

}

static ssize_t ss_finger_hbm_updated_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);

	if (vdd->finger_mask)
		sprintf(buf, "%d\n", vdd->br.finger_mask_bl_level);
	else
		sprintf(buf, "%d\n", vdd->finger_mask);

	LCD_INFO("vdd->br.actual_mask_brightness value : %x\n", vdd->finger_mask);

	return strlen(buf);
}

static ssize_t ss_fp_green_circle_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	int val = 0;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		goto end;
	}

	if (!ss_is_ready_to_send_cmd(vdd)) {
		LCD_ERR("Panel is not ready. Panel State(%d)\n", vdd->panel_state);
		return size;
	}

	if (sscanf(buf, "%d", &val) != 1)
		return size;

#if defined(CONFIG_SEC_FACTORY)
	if (val)
		ss_send_cmd(vdd, TX_SELF_MASK_GREEN_CIRCLE_ON_FACTORY);
	else
		ss_send_cmd(vdd, TX_SELF_MASK_GREEN_CIRCLE_OFF_FACTORY);
#else
	if (val)
		ss_send_cmd(vdd, TX_SELF_MASK_GREEN_CIRCLE_ON);
	else
		ss_send_cmd(vdd, TX_SELF_MASK_GREEN_CIRCLE_OFF);
#endif
	LCD_INFO("Finger Print Green Circle : %d\n", val);

end:
	return size;
}

static ssize_t ss_ub_con_det_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	int ret = 0;

	if (!gpio_is_valid(vdd->ub_con_det.gpio)) {
		LCD_ERR("No ub_con_det gpio..\n");
		ret = snprintf(buf, 20, "-1\n");
	} else
		ret = snprintf(buf, 20, gpio_get_value(vdd->ub_con_det.gpio) ? "disconnected\n" : "connected\n");

	return ret;
}

static ssize_t ss_ub_con_det_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct samsung_display_driver_data *vdd =
		(struct samsung_display_driver_data *)dev_get_drvdata(dev);
	int value;
	int ub_con_gpio;

	if (IS_ERR_OR_NULL(vdd)) {
		LCD_ERR("no vdd");
		return size;
	}

	if (sscanf(buf, "%d", &value) != 1)
		return size;

	if (!gpio_is_valid(vdd->ub_con_det.gpio)) {
		LCD_ERR("ub_con_det gpio is not valid ..\n");
		value = 0;
	}

	if (value) {
		vdd->ub_con_det.enabled = true;
		ub_con_gpio = gpio_get_value(vdd->ub_con_det.gpio);
		LCD_ERR("ub con gpio = %d\n", ub_con_gpio);
		/* Once enable ub con det, check ub status first */
		if (ub_con_gpio)
			ss_send_ub_uevent(vdd);
	} else {
		vdd->ub_con_det.enabled = false;
	}

	LCD_INFO("ub_con_det - %s \n", vdd->ub_con_det.enabled ? "[enabled]" : "[disabled]");

	return size;
}

static DEVICE_ATTR(lcd_type, S_IRUGO, ss_disp_lcdtype_show, NULL);
static DEVICE_ATTR(cell_id, S_IRUGO, ss_disp_cell_id_show, NULL);
static DEVICE_ATTR(octa_id, S_IRUGO, ss_disp_octa_id_show, NULL);
static DEVICE_ATTR(window_type, S_IRUGO, ss_disp_windowtype_show, NULL);
static DEVICE_ATTR(manufacture_date, S_IRUGO, ss_disp_manufacture_date_show, NULL);
static DEVICE_ATTR(manufacture_code, S_IRUGO, ss_disp_manufacture_code_show, NULL);
static DEVICE_ATTR(power_reduce, S_IRUGO | S_IWUSR | S_IWGRP, ss_disp_acl_show, ss_disp_acl_store);
static DEVICE_ATTR(enable_fd, S_IRUGO | S_IWUSR | S_IWGRP, ss_disp_fd_show, ss_disp_fd_store);
static DEVICE_ATTR(siop_enable, S_IRUGO | S_IWUSR | S_IWGRP, ss_disp_siop_show, ss_disp_siop_store);
static DEVICE_ATTR(read_mtp, S_IRUGO | S_IWUSR | S_IWGRP, ss_read_mtp_show, ss_read_mtp_store);
static DEVICE_ATTR(write_mtp, S_IRUGO | S_IWUSR | S_IWGRP, ss_read_mtp_show, ss_write_mtp_store);
static DEVICE_ATTR(temperature, S_IRUGO | S_IWUSR | S_IWGRP, ss_temperature_show, ss_temperature_store);
static DEVICE_ATTR(lux, S_IRUGO | S_IWUSR | S_IWGRP, ss_lux_show, ss_lux_store);
static DEVICE_ATTR(copr, S_IRUGO | S_IWUSR | S_IWGRP, ss_copr_show, ss_copr_store);
static DEVICE_ATTR(copr_roi, S_IRUGO | S_IWUSR | S_IWGRP, ss_copr_roi_show, ss_copr_roi_store);
static DEVICE_ATTR(brt_avg, S_IRUGO | S_IWUSR | S_IWGRP, ss_brt_avg_show, NULL);
static DEVICE_ATTR(self_mask, S_IRUGO | S_IWUSR | S_IWGRP, NULL, ss_self_mask_store);
static DEVICE_ATTR(self_mask_check, S_IRUGO | S_IWUSR | S_IWGRP, ss_self_mask_check_show, NULL);
static DEVICE_ATTR(dynamic_hlpm, S_IRUGO | S_IWUSR | S_IWGRP, NULL, ss_dynamic_hlpm_store);
static DEVICE_ATTR(self_display, S_IRUGO | S_IWUSR | S_IWGRP, NULL, ss_self_display_store);
static DEVICE_ATTR(read_copr, S_IRUGO | S_IWUSR | S_IWGRP, ss_read_copr_show, NULL);
static DEVICE_ATTR(aid_log, S_IRUGO | S_IWUSR | S_IWGRP, ss_aid_log_show, ss_aid_log_store);
static DEVICE_ATTR(gamma_interpolation_test, S_IRUGO | S_IWUSR | S_IWGRP, ss_aid_log_show, ss_gamma_interpolation_test_store);
static DEVICE_ATTR(partial_disp, S_IRUGO | S_IWUSR | S_IWGRP, ss_disp_partial_disp_show, ss_disp_partial_disp_store);
static DEVICE_ATTR(alpm, S_IRUSR | S_IRGRP | S_IWUSR | S_IWGRP, ss_panel_lpm_mode_show, ss_panel_lpm_mode_store);
static DEVICE_ATTR(hmt_bright, S_IRUGO | S_IWUSR | S_IWGRP, mipi_samsung_hmt_bright_show, mipi_samsung_hmt_bright_store);
static DEVICE_ATTR(hmt_on, S_IRUGO | S_IWUSR | S_IWGRP,	mipi_samsung_hmt_on_show, mipi_samsung_hmt_on_store);
static DEVICE_ATTR(mcd_mode, S_IRUGO | S_IWUSR | S_IWGRP, NULL, mipi_samsung_mcd_store);
static DEVICE_ATTR(mst, S_IRUGO | S_IWUSR | S_IWGRP, NULL, mipi_samsung_mst_store);
static DEVICE_ATTR(poc, S_IRUGO | S_IWUSR | S_IWGRP, mipi_samsung_poc_show, mipi_samsung_poc_store);
static DEVICE_ATTR(poc_mca, S_IRUGO | S_IWUSR | S_IWGRP, mipi_samsung_poc_mca_show, NULL);
static DEVICE_ATTR(irc_mode, S_IRUGO | S_IWUSR | S_IWGRP, ss_irc_mode_show, ss_irc_mode_store);
//static DEVICE_ATTR(ldu_correction, S_IRUGO | S_IWUSR | S_IWGRP, ss_ldu_correction_show, ss_ldu_correction_store);
static DEVICE_ATTR(adaptive_control, S_IRUGO | S_IWUSR | S_IWGRP, NULL, ss_adaptive_control_store);
static DEVICE_ATTR(hw_cursor, S_IRUGO | S_IWUSR | S_IWGRP, NULL, mipi_samsung_hw_cursor_store);
static DEVICE_ATTR(cover_control, S_IRUGO | S_IWUSR | S_IWGRP, ss_cover_control_show, ss_cover_control_store);
static DEVICE_ATTR(SVC_OCTA, S_IRUGO, ss_disp_SVC_OCTA_show, NULL);
static DEVICE_ATTR(SVC_OCTA2, S_IRUGO, ss_disp_SVC_OCTA2_show, NULL);
static DEVICE_ATTR(SVC_OCTA_CHIPID, S_IRUGO, ss_disp_SVC_OCTA_CHIPID_show, NULL);
static DEVICE_ATTR(SVC_OCTA2_CHIPID, S_IRUGO, ss_disp_SVC_OCTA2_CHIPID_show, NULL);
static DEVICE_ATTR(SVC_OCTA_DDI_CHIPID, S_IRUGO, ss_disp_SVC_OCTA_DDI_CHIPID_show, NULL);
static DEVICE_ATTR(esd_check, S_IRUGO, mipi_samsung_esd_check_show, NULL);
static DEVICE_ATTR(rf_info, S_IRUGO | S_IWUSR | S_IWGRP, ss_rf_info_show, ss_rf_info_store);
static DEVICE_ATTR(dynamic_freq, S_IRUGO | S_IWUSR | S_IWGRP, ss_dynamic_freq_show, ss_dynamic_freq_store);
#ifdef MDNIE_TUNING
static DEVICE_ATTR(tuning, 0664, tuning_show, tuning_store);
#endif
//static DEVICE_ATTR(csc_cfg, S_IRUGO | S_IWUSR, csc_read_cfg, csc_write_cfg);
static DEVICE_ATTR(xtalk_mode, S_IRUGO | S_IWUSR | S_IWGRP, NULL, xtalk_store);
static DEVICE_ATTR(gct, S_IRUGO | S_IWUSR | S_IWGRP, gct_show, gct_store);
static DEVICE_ATTR(grayspot, S_IRUGO | S_IWUSR | S_IWGRP, NULL, mipi_samsung_grayspot_store);
static DEVICE_ATTR(isc_defect, S_IRUGO | S_IWUSR | S_IWGRP, NULL, mipi_samsung_isc_defect_store);
static DEVICE_ATTR(dpui, S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP, ss_dpui_show, ss_dpui_store);
static DEVICE_ATTR(dpui_dbg, S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP, ss_dpui_dbg_show, ss_dpui_dbg_store);
static DEVICE_ATTR(dpci, S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP, ss_dpci_show, ss_dpci_store);
static DEVICE_ATTR(dpci_dbg, S_IRUSR|S_IRGRP|S_IWUSR|S_IWGRP, ss_dpci_dbg_show, ss_dpci_dbg_store);
#if defined(CONFIG_FOLDER_HALL)
static DEVICE_ATTR(force_flip, S_IWUSR | S_IWGRP, NULL, ss_force_flip_store);
#endif
static DEVICE_ATTR(gamma_flash, S_IRUGO | S_IWUSR | S_IWGRP, ss_disp_flash_gamma_show, ss_disp_flash_gamma_store);
static DEVICE_ATTR(read_flash, S_IRUGO | S_IWUSR | S_IWGRP, ss_read_flash_show, ss_read_flash_store);
static DEVICE_ATTR(spi_if_sel, S_IRUGO | S_IWUSR | S_IWGRP, NULL, ss_spi_if_sel_store);
static DEVICE_ATTR(ccd_state, S_IRUGO | S_IWUSR | S_IWGRP, ss_ccd_state_show, NULL);
static DEVICE_ATTR(isc, S_IRUGO | S_IWUSR | S_IWGRP, NULL, ss_isc_store);
static DEVICE_ATTR(stm, S_IRUGO | S_IWUSR | S_IWGRP, NULL, ss_stm_store);
/* SAMSUNG_FINGERPRINT */
static DEVICE_ATTR(mask_brightness, S_IRUGO | S_IWUSR | S_IWGRP, NULL, ss_finger_hbm_store);
static DEVICE_ATTR(actual_mask_brightness, S_IRUGO | S_IWUSR | S_IWGRP, ss_finger_hbm_updated_show, NULL);
static DEVICE_ATTR(reading_mode, S_IRUGO | S_IWUSR | S_IWGRP, ss_reading_mode_show, ss_reading_mode_store);
static DEVICE_ATTR(fp_green_circle, S_IRUGO | S_IWUSR | S_IWGRP, NULL, ss_fp_green_circle_store);
static DEVICE_ATTR(conn_det, S_IRUGO | S_IWUSR | S_IWGRP, ss_ub_con_det_show, ss_ub_con_det_store);


static struct attribute *panel_sysfs_attributes[] = {
	&dev_attr_lcd_type.attr,
	&dev_attr_cell_id.attr,
	&dev_attr_octa_id.attr,
	&dev_attr_window_type.attr,
	&dev_attr_manufacture_date.attr,
	&dev_attr_manufacture_code.attr,
	&dev_attr_power_reduce.attr,
	&dev_attr_enable_fd.attr,
	&dev_attr_siop_enable.attr,
	&dev_attr_aid_log.attr,
	&dev_attr_gamma_interpolation_test.attr,
	&dev_attr_read_mtp.attr,
	&dev_attr_write_mtp.attr,
	&dev_attr_read_copr.attr,
	&dev_attr_copr.attr,
	&dev_attr_copr_roi.attr,
	&dev_attr_brt_avg.attr,
	&dev_attr_self_mask.attr,
	&dev_attr_dynamic_hlpm.attr,
	&dev_attr_self_display.attr,
	&dev_attr_self_mask_check.attr,
	&dev_attr_temperature.attr,
	&dev_attr_lux.attr,
	&dev_attr_partial_disp.attr,
	&dev_attr_alpm.attr,
	&dev_attr_hmt_bright.attr,
	&dev_attr_hmt_on.attr,
	&dev_attr_mcd_mode.attr,
	&dev_attr_irc_mode.attr,
//	&dev_attr_ldu_correction.attr,
	&dev_attr_adaptive_control.attr,
	&dev_attr_hw_cursor.attr,
	&dev_attr_cover_control.attr,
	&dev_attr_SVC_OCTA.attr,
	&dev_attr_SVC_OCTA2.attr,
	&dev_attr_SVC_OCTA_CHIPID.attr,
	&dev_attr_SVC_OCTA2_CHIPID.attr,
	&dev_attr_SVC_OCTA_DDI_CHIPID.attr,
	&dev_attr_esd_check.attr,
	&dev_attr_rf_info.attr,
	&dev_attr_dynamic_freq.attr,
	&dev_attr_xtalk_mode.attr,
	&dev_attr_gct.attr,
	&dev_attr_mst.attr,
	&dev_attr_grayspot.attr,
	&dev_attr_isc_defect.attr,
	&dev_attr_poc.attr,
	&dev_attr_poc_mca.attr,
	&dev_attr_dpui.attr,
	&dev_attr_dpui_dbg.attr,
	&dev_attr_dpci.attr,
	&dev_attr_dpci_dbg.attr,
#if defined(CONFIG_FOLDER_HALL)
	&dev_attr_force_flip.attr,
#endif
	&dev_attr_gamma_flash.attr,
	&dev_attr_read_flash.attr,
	&dev_attr_spi_if_sel.attr,
	&dev_attr_ccd_state.attr,
	&dev_attr_isc.attr,
	&dev_attr_stm.attr,
	&dev_attr_mask_brightness.attr,
	&dev_attr_actual_mask_brightness.attr,
	&dev_attr_conn_det.attr,
	&dev_attr_reading_mode.attr,
	&dev_attr_fp_green_circle.attr,
	NULL
};
static const struct attribute_group panel_sysfs_group = {
	.attrs = panel_sysfs_attributes,
};

#if defined(CONFIG_BACKLIGHT_CLASS_DEVICE)
static DEVICE_ATTR(brightness_step, S_IRUGO | S_IWUSR | S_IWGRP,
			ss_disp_brightness_step,
			NULL);
#if 0
static DEVICE_ATTR(weakness_ccb, S_IRUGO | S_IWUSR | S_IWGRP,
			ss_disp_color_weakness_show,
			ss_disp_color_weakness_store);
#endif
static struct attribute *bl_sysfs_attributes[] = {
	&dev_attr_brightness_step.attr,
//	&dev_attr_weakness_ccb.attr,
	NULL
};

static const struct attribute_group bl_sysfs_group = {
	.attrs = bl_sysfs_attributes,
};
#endif /* END CONFIG_LCD_CLASS_DEVICE*/

int ss_create_sysfs(struct samsung_display_driver_data *vdd)
{
	int rc = 0;
	struct lcd_device *lcd_device;
#if defined(CONFIG_BACKLIGHT_CLASS_DEVICE)
	struct backlight_device *bd = NULL;
#endif
	struct kernfs_node *SVC_sd;
	struct kobject *SVC;
	char dirname[10];

	if (vdd->ndx == PRIMARY_DISPLAY_NDX)
		sprintf(dirname, "panel");
	else
		sprintf(dirname, "panel%d", vdd->ndx);

	lcd_device = lcd_device_register(dirname, NULL, vdd, NULL);
	vdd->lcd_dev = lcd_device;

	if (IS_ERR_OR_NULL(lcd_device)) {
		rc = PTR_ERR(lcd_device);
		LCD_ERR("Failed to register lcd device..\n");
		return rc;
	}

	rc = sysfs_create_group(&lcd_device->dev.kobj, &panel_sysfs_group);
	if (rc) {
		LCD_ERR("Failed to create panel sysfs group..\n");
		sysfs_remove_group(&lcd_device->dev.kobj, &panel_sysfs_group);
		return rc;
	}

	/* To find SVC kobject */
	SVC_sd = sysfs_get_dirent(devices_kset->kobj.sd, "svc");
	if (IS_ERR_OR_NULL(SVC_sd)) {
		/* try to create SVC kobject */
		SVC = kobject_create_and_add("svc", &devices_kset->kobj);
		if (IS_ERR_OR_NULL(SVC))
			LCD_ERR("Failed to create sys/devices/svc already exist");
		else
			LCD_INFO("Success to create sys/devices/svc");
	} else {
		SVC = (struct kobject *)SVC_sd->priv;
		LCD_INFO("Success to find SVC\n");
	}

	if (!IS_ERR_OR_NULL(SVC)) {
		if (vdd->ndx == PRIMARY_DISPLAY_NDX)
			sprintf(dirname, "OCTA");
		else
			sprintf(dirname, "OCTA%d", vdd->ndx);

		rc = sysfs_create_link(SVC, &lcd_device->dev.kobj, dirname);
		if (rc)
			LCD_ERR("Failed to create panel sysfs svc/%s\n", dirname);
		else
			LCD_INFO("Success to create panel sysfs svc/%s\n", dirname);
	} else
		LCD_ERR("Failed to find svc kobject\n");

#if defined(CONFIG_BACKLIGHT_CLASS_DEVICE)
	if (vdd->ndx == PRIMARY_DISPLAY_NDX)
		sprintf(dirname, "panel");
	else
		sprintf(dirname, "panel%d", vdd->ndx);

	bd = backlight_device_register(dirname, &lcd_device->dev,
						vdd, NULL, NULL);
	if (IS_ERR(bd)) {
		rc = PTR_ERR(bd);
		LCD_ERR("backlight : failed to register device\n");
		return rc;
	}

	rc = sysfs_create_group(&bd->dev.kobj, &bl_sysfs_group);
	if (rc) {
		LCD_ERR("Failed to create backlight sysfs group..\n");
		sysfs_remove_group(&bd->dev.kobj, &bl_sysfs_group);
		return rc;
	}
#endif
#ifdef MDNIE_TUNING
	rc = sysfs_create_file(&lcd_device->dev.kobj, &dev_attr_tuning.attr);
	if (rc) {
		LCD_ERR("sysfs create fail-%s\n", dev_attr_tuning.attr.name);
		return rc;
	}
#endif
	ss_register_dpui(vdd);

	LCD_INFO("done!!\n");

	return rc;
}
