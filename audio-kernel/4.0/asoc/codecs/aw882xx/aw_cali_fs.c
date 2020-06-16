/*
 * aw_cali_fs.c cali_module
 *
 * Version: v0.1.4
 *
 * Copyright (c) 2019 AWINIC Technology CO., LTD
 *
 *  Author: Nick Li <liweilei@awinic.com.cn>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <asm/ioctls.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include "aw882xx.h"
#include "aw_cali_fs.h"

extern int g_aw882xx_cali_flag;
extern struct aw882xx *g_aw882xx;

#ifdef AW_CALI_STORE_EXAMPLE
/*write cali to persist file example*/
#define AWINIC_CALI_FILE  "/mnt/vendor/persist/audio/aw_cali.bin"
static int aw882xx_write_cali_re_to_file(int32_t cali_re)
{
	struct file *fp;
	char buf[50];
	loff_t pos = 0;
	mm_segment_t fs;

	fp = filp_open(AWINIC_CALI_FILE, O_RDWR | O_CREAT, 0644);
	if (IS_ERR(fp)) {
		pr_err("%s: open %s failed!", __func__, AWINIC_CALI_FILE);
		return -EINVAL;
	}


	snprintf(buf, PAGE_SIZE, "%d", cali_re);

	fs = get_fs();
	set_fs(KERNEL_DS);

	vfs_write(fp, buf, strlen(buf), &pos);

	set_fs(fs);

	pr_info("%s: %s", __func__, buf);

	filp_close(fp, NULL);
	return 0;
}

static int aw882xx_get_cali_re_from_file(int32_t *cali_re)
{
	struct file *fp;
	//struct inode *node;
	int f_size;
	char *buf;
	int32_t int_cali_re = 0;

	loff_t pos = 0;
	mm_segment_t fs;

	fp = filp_open(AWINIC_CALI_FILE, O_RDONLY, 0);
	if (IS_ERR(fp)) {
		pr_err("%s: open %s failed!", __func__, AWINIC_CALI_FILE);
		return -EINVAL;
	}

	//node = fp->f_dentry->d_inode;
	f_size = 32;
	buf = (char *)kmalloc(f_size + 1, GFP_ATOMIC);
	if (buf == NULL) {
		pr_err("%s: malloc mem %d failed!", __func__, f_size);
		filp_close(fp, NULL);
		return -EINVAL;
	}

	fs = get_fs();
	set_fs(KERNEL_DS);

	vfs_read(fp, buf, f_size, &pos);

	set_fs(fs);

	if (sscanf(buf, "%d", &int_cali_re) == 1)
		*cali_re = int_cali_re;
	else
		*cali_re = 7000;

	pr_info("%s: %d", __func__, int_cali_re);

	filp_close(fp, NULL);

	return  0;

}
#endif

 /*custom need add to set/get cali_re form/to nv*/
int aw882xx_set_cali_re_to_nvram(int32_t cali_re)
{
	/*custom add, if success return value is 0, else -1*/
#ifdef AW_CALI_STORE_EXAMPLE
	return aw882xx_write_cali_re_to_file(cali_re);
#else
	return -EBUSY;
#endif
}
int aw882xx_get_cali_re_from_nvram(int32_t *cali_re)
{
	/*custom add, if success return value is 0 , else -1*/
#ifdef AW_CALI_STORE_EXAMPLE
	return aw882xx_get_cali_re_from_file(cali_re);
#else
	return -EBUSY;
#endif
}


/******************************cali debug fs****************************************/
struct dentry *aw_debugfs_dir = NULL;
struct dentry *aw_debugfs_range = NULL;
struct dentry *aw_debugfs_cali = NULL;
struct dentry *aw_debugfs_status = NULL;
struct dentry *aw_debugfs_f0 = NULL;

int  aw_cali_range_open(struct inode *inode, struct file *file)
{
	pr_info("%s: open success", __func__);
	return 0;
}

ssize_t aw_cali_range_read(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
	int ret;
	char local_buf[50];

	memset(local_buf, 0, sizeof(local_buf));
	if (len < sizeof(local_buf)) {
		pr_err("%s: buf len not enough \n", __func__);
		return -ENOSPC;
	}

	ret = snprintf(local_buf, PAGE_SIZE,
			" Min:%d mOhms, Max:%d mOhms\n", R0_MIN, R0_MAX);
	//ret = copy_to_user(buf, local_buf, sizeof(local_buf));
	ret = simple_read_from_buffer(buf, len, ppos, local_buf, ret);
	if (ret < 0) {
		pr_err("%s: copy failed!\n", __func__);
		return -ENOMEM;
	}
	return ret;
}

/*ssize_t aw_cali_range_write (struct file *file, const char __user *buf, size_t len, loff_t *ppos)
{
	return 0;
}*/

static const struct file_operations aw_cali_range_fops = {
	.open = aw_cali_range_open,
	.read = aw_cali_range_read,
	/*.write = aw_cali_range_write,*/
};
static int aw_cali_start_up(int32_t *cali_re)
{
	int ret;
	struct cali_cfg set_cfg, store_cfg;
	struct cali_data cali_data;

	/*get cali cfg*/
	ret = aw_send_afe_cal_apr(AFE_PARAM_ID_AWDSP_RX_CALI_CFG_L,
					(void *)&store_cfg, sizeof(struct cali_cfg), false);
	if (ret) {
		pr_err("%s:read cali cfg data failed! \n", __func__);
		return -EBUSY;
	}
	set_cfg.data[0] = 0;
	set_cfg.data[1] = 0;
	set_cfg.data[2] = -1;

	/*set cali cfg start cali*/
	ret = aw_send_afe_cal_apr(AFE_PARAM_ID_AWDSP_RX_CALI_CFG_L,
					(void *)&set_cfg, sizeof(struct cali_cfg), true);
	if (ret) {
		pr_err("%s:start cali failed !\n", __func__);
		goto cali_failed;
	}

	/*keep 10 s ,wait data stable*/
	msleep(10*1000);

	/*get cali data*/
	ret = aw_send_afe_cal_apr(AFE_PARAM_ID_AWDSP_RX_REAL_DATA_L,
					(void *)&cali_data, sizeof(struct cali_data), false);
	if (ret) {
		pr_err("%s:read cali data failed! \n", __func__);
		goto cali_failed;
	}

	pr_info("%s:cali_re : 0x%x \n", __func__, cali_data.data[0]);
	*cali_re = cali_data.data[0];

	/*repair cali cfg to normal status*/
	ret = aw_send_afe_cal_apr(AFE_PARAM_ID_AWDSP_RX_CALI_CFG_L,
					(void *)&store_cfg, sizeof(struct cali_cfg), true);
	return 0;

cali_failed:
	ret = aw_send_afe_cal_apr(AFE_PARAM_ID_AWDSP_RX_CALI_CFG_L,
					(void *)&store_cfg, sizeof(struct cali_cfg), true);
	return -EBUSY;
}

int  aw_cali_open(struct inode *inode, struct file *file)
{
	pr_debug("%s: open success \n", __func__);
	return 0;
}

ssize_t aw_cali_read (struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
	int ret;
	char ret_value[50];
	int local_len = 0;
	int32_t re_cali = 0;

	memset(ret_value, 0, sizeof(ret_value));
	if (len < sizeof(ret_value)) {
		pr_err("%s:buf len no enough \n", __func__);
		return 0;
	}
	/*set cali flag*/
	g_aw882xx_cali_flag = true;
	ret = aw_cali_start_up(&re_cali);
	if (ret != 0) {
		pr_err("%s:cali failed \n", __func__);
		g_aw882xx_cali_flag = false;
		return 0;
	}
	g_aw882xx_cali_flag = false;

	/*factor form 12bit(4096) to 1000*/
	re_cali = (re_cali * 1000) >> 12;

	ret = snprintf(ret_value + local_len, PAGE_SIZE - local_len,
				" Prim:%d mOhms, Sec:0 mOhms\n", re_cali);

	//ret = copy_to_user(buf, ret_value, sizeof(ret_value));
	ret = simple_read_from_buffer(buf, len, ppos, ret_value, ret);
	//chengong
	//aw882xx_write_cali_re_to_file(re_cali);
	if (ret < 0) {
		pr_err("%s:copy failed!\n", __func__);
		return -ENOMEM;
	}
	return ret;
}

ssize_t aw_cali_write (struct file *file, const char __user *buf, size_t len, loff_t *ppos)
{
	return 0;
}


static const struct file_operations aw_cali_fops = {
	.open = aw_cali_open,
	.read = aw_cali_read,
	.write = aw_cali_write,
};

int  aw_f0_open(struct inode *inode, struct file *file)
{
	pr_debug("%s: open success \n", __func__);
	return 0;
}

static int aw_cali_get_f0(int32_t *cali_f0)
{
	int ret;
	struct cali_cfg set_cfg, store_cfg;
	int32_t read_f0;

	/*get cali cfg*/
	ret = aw_send_afe_cal_apr(AFE_PARAM_ID_AWDSP_RX_CALI_CFG_L,
					(void *)&store_cfg, sizeof(struct cali_cfg), false);
	if (ret) {
		pr_err("%s:read cali cfg data failed! \n", __func__);
		return 	-EBUSY;
	}
	set_cfg.data[0] = 0;
	set_cfg.data[1] = 0;
	set_cfg.data[2] = -1;

	/*set cali cfg start cali*/
	ret = aw_send_afe_cal_apr(AFE_PARAM_ID_AWDSP_RX_CALI_CFG_L,
					(void *)&set_cfg, sizeof(struct cali_cfg), true);
	if (ret) {
		pr_err("%s:start cali failed !\n", __func__);
		goto cali_failed;
	}

	/*keep 6s ,wait data stable*/
	msleep(6*1000);

	/*get cali data*/
	ret = aw_send_afe_cal_apr(AFE_PARAM_ID_AWDSP_RX_F0_L,
					(void *)&read_f0, sizeof(int32_t), false);
	if (ret) {
		pr_err("%s:read cali data failed! \n", __func__);
		goto cali_failed;
	}

	pr_info("%s:cali_f0 : %d \n", __func__, read_f0);
	*cali_f0 = read_f0;

	/*repair cali cfg to normal status*/
	ret = aw_send_afe_cal_apr(AFE_PARAM_ID_AWDSP_RX_CALI_CFG_L,
					(void *)&store_cfg, sizeof(struct cali_cfg), true);
	return 0;

cali_failed:
	ret = aw_send_afe_cal_apr(AFE_PARAM_ID_AWDSP_RX_CALI_CFG_L,
					(void *)&store_cfg, sizeof(struct cali_cfg), true);
	return -EBUSY;
}


ssize_t aw_f0_read (struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
	int ret;
	char ret_value[20];
	int local_len = 0;
	int32_t ret_f0 = 0;

	memset(ret_value, 0, sizeof(ret_value));
	if (len < sizeof(ret_value)) {
		pr_err("%s:buf len no enough \n", __func__);
		return 0;
	}
	/*set cali flag*/
	g_aw882xx_cali_flag = true;
	ret = aw_cali_get_f0(&ret_f0);
	if (ret != 0) {
		pr_err("%s:cali failed \n", __func__);
		g_aw882xx_cali_flag = false;
		return 0;
	}
	g_aw882xx_cali_flag = false;

	ret = snprintf(ret_value + local_len, PAGE_SIZE - local_len, "%d\n", ret_f0);

	//ret = copy_to_user(buf, ret_value, sizeof(ret_value));
	ret = simple_read_from_buffer(buf, len, ppos, ret_value, ret);
	if (ret < 0) {
		pr_err("%s:copy failed!\n", __func__);
		return -ENOMEM;
	}
	return ret;
}

static const struct file_operations aw_f0_fops = {
	.open = aw_f0_open,
	.read = aw_f0_read,
};
int  aw_cali_status_open(struct inode *inode, struct file *file)
{
	pr_debug("%s: open success \n", __func__);
	return 0;
}

ssize_t aw_cali_status_read (struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
	int ret;
	char status_value[20];
	int local_len = 0;
	struct cali_data cali_data;
	int32_t real_r0;

	if (len < sizeof(status_value)) {
		pr_err("%s:buf len no enough \n", __func__);
		return -ENOSPC;
	}

	/*get cali data*/
	ret = aw_send_afe_cal_apr(AFE_PARAM_ID_AWDSP_RX_REAL_DATA_L,
					(void *)&cali_data, sizeof(struct cali_data), false);
	if (ret) {
		pr_err("%s:read speaker status failed! \n", __func__);
		return -EBUSY;
	}
	/*R0 factor form 4096 to 1000*/
	real_r0 = (cali_data.data[0] * 1000) >> 12;
	ret = snprintf(status_value + local_len, PAGE_SIZE - local_len,
				"%d : %d\n", real_r0, cali_data.data[1]);

	//ret = copy_to_user(buf, status_value, sizeof(status_value));
	ret = simple_read_from_buffer(buf, len, ppos, status_value, ret);
	if (ret < 0) {
		pr_err("%s:copy failed!", __func__);
		return -ENOMEM;
	}
	return ret;
}

/*ssize_t aw_cali_status_write (struct file *file, const char __user *buf, size_t len, loff_t *ppos)
{
	return 0;
}*/

static const struct file_operations aw_cali_status_fops = {
	.open = aw_cali_status_open,
	.read = aw_cali_status_read,
	/*.write = aw_cali_status_write,*/
};

void aw_debugfs_init (void)
{
	aw_debugfs_dir = debugfs_create_dir("awinic_cali", NULL);
	if (aw_debugfs_dir == NULL) {
		pr_err("create cali debugfs failed ! \n");
		return ;
	}
	aw_debugfs_range = debugfs_create_file("range", S_IFREG|S_IRUGO,
		aw_debugfs_dir, NULL, &aw_cali_range_fops);
	if (aw_debugfs_range == NULL) {
		pr_err("create cali debugfs range failed ! \n");
		return ;
	}
	aw_debugfs_cali = debugfs_create_file("cali", S_IFREG|S_IRUGO|S_IWUGO,
		aw_debugfs_dir, NULL, &aw_cali_fops);
	if (aw_debugfs_cali == NULL) {
		pr_err("create cali debugfs cali failed ! \n");
		return ;
	}
	aw_debugfs_f0 = debugfs_create_file("f0", S_IFREG|S_IRUGO,
		aw_debugfs_dir, NULL, &aw_f0_fops);
	if (aw_debugfs_f0 == NULL) {
		pr_err("create cali debugfs cali failed ! \n");
		return ;
	}
	aw_debugfs_status = debugfs_create_file("status", S_IFREG|S_IRUGO,
		aw_debugfs_dir, NULL, &aw_cali_status_fops);
	if (aw_debugfs_status == NULL) {
		pr_err("create cali debugfs status failed ! \n");
		return ;
	}
}

/**************************************cali misc device******************************************************/
#define AW882XX_SMARTPA_NAME "aw882xx_smartpa"
static int aw882xx_file_open(struct inode *inode, struct file *file)
{
	if (!try_module_get(THIS_MODULE))
		return -ENODEV;
	file->private_data = (void *)g_aw882xx;

	pr_debug("open success");
	return 0;
}

static int aw882xx_file_release(struct inode *inode, struct file *file)
{
	file->private_data = (void *)NULL;

	pr_debug("release successi\n");
	return 0;
}


static int aw882xx_cali_operation(struct aw882xx *aw882xx,
			unsigned int cmd, unsigned long arg)
{
	int16_t data_len  = _IOC_SIZE(cmd);
	int ret = 0;
	char *data_ptr = NULL;


	data_ptr = kmalloc(data_len, GFP_KERNEL);
	if (data_ptr == NULL) {
		pr_err("%s : malloc failed !\n", __func__);
		return -EFAULT;
	}

	pr_info("cmd : %d, data_len%d\n", cmd , data_len);
	switch (cmd) {
		case AW882XX_IOCTL_ENABLE_CALI: {
			if (copy_from_user(data_ptr,
					(void __user *)arg, data_len)) {
				ret = -EFAULT;
				goto exit;
			}
			g_aw882xx_cali_flag = (int8_t)data_ptr[0];
			pr_info("%s:set cali %s", __func__,
				(g_aw882xx_cali_flag == 0) ? ("disable") : ("enable"));
		} break;
		case AW882XX_IOCTL_SET_CALI_CFG: {
			if (copy_from_user(data_ptr,
					(void __user *)arg, data_len)) {
				ret = -EFAULT;
				goto exit;
			}
			ret = aw_send_afe_cal_apr(
				AFE_PARAM_ID_AWDSP_RX_CALI_CFG_L,
							data_ptr, data_len, true);
			if (ret) {
				pr_err("%s: dsp_msg_write error: 0x%x\n",
					__func__, AFE_PARAM_ID_AWDSP_RX_CALI_CFG_L);
				ret =  -EFAULT;
				goto exit;
			}
		} break;
		case AW882XX_IOCTL_GET_CALI_CFG: {
			ret = aw_send_afe_cal_apr(
				AFE_PARAM_ID_AWDSP_RX_CALI_CFG_L,
						data_ptr, data_len, false);
			if (ret) {
				pr_err("%s: dsp_msg_read error: 0x%x\n",
					__func__, AFE_PARAM_ID_AWDSP_RX_CALI_CFG_L);
				ret = -EFAULT;
				goto exit;
			}
			if (copy_to_user((void __user *)arg,
				data_ptr, data_len)) {
				ret = -EFAULT;
				goto exit;
			}
		} break;
		case AW882XX_IOCTL_GET_CALI_DATA: {
			ret = aw_send_afe_cal_apr(
				AFE_PARAM_ID_AWDSP_RX_REAL_DATA_L,
						data_ptr, data_len, false);
			if (ret) {
				pr_err("%s: dsp_msg_read error: 0x%x\n",
					__func__, AFE_PARAM_ID_AWDSP_RX_REAL_DATA_L);
				ret = -EFAULT;
				goto exit;
			}
			if (copy_to_user((void __user *)arg,
				data_ptr, data_len)) {
				ret = -EFAULT;
				goto exit;
			}
		} break;
		case AW882XX_IOCTL_SET_NOISE: {
			if (copy_from_user(data_ptr,
				(void __user *)arg, data_len)) {
				ret = -EFAULT;
				goto exit;
			}
			ret = aw_send_afe_cal_apr(AFE_PARAM_ID_AWDSP_RX_NOISE_L,
						data_ptr, data_len, true);
			if (ret) {
				pr_err("%s: dsp_msg_write error: 0x%x\n",
					__func__, AFE_PARAM_ID_AWDSP_RX_NOISE_L);
				ret = -EFAULT;
				goto exit;
			}
		} break;
		case AW882XX_IOCTL_GET_F0: {
			ret = aw_send_afe_cal_apr(AFE_PARAM_ID_AWDSP_RX_F0_L,
						data_ptr, data_len, false);
			if (ret) {
				pr_err("%s: dsp_msg_read error: 0x%x\n",
					__func__, AFE_PARAM_ID_AWDSP_RX_F0_L);
				ret = -EFAULT;
				goto exit;
			}
			if (copy_to_user((void __user *)arg,
				data_ptr, data_len)) {
				ret = -EFAULT;
				goto exit;
			}
		} break;
		case AW882XX_IOCTL_SET_CALI_RE: {
			if (copy_from_user(data_ptr,
				(void __user *)arg, data_len)) {
				ret = -EFAULT;
				goto exit;
			}
			ret = aw_send_afe_cal_apr(AFE_PARAM_ID_AWDSP_RX_RE_L,
						data_ptr, data_len, true);
			if (ret) {
				pr_err("%s: dsp_msg_write error: 0x%x\n",
					__func__, AFE_PARAM_ID_AWDSP_RX_RE_L);
				ret = -EFAULT;
				goto exit;
			}
			aw882xx_set_cali_re(aw882xx, *((int32_t *)data_ptr));
			ret = aw882xx_set_cali_re_to_nvram(*((int32_t *)data_ptr));
			if (ret < 0) {
				pr_err("%s: write cali re to nv error\n", __func__);
				ret = -EFAULT;
				goto exit;
			}
		} break;
		case AW882XX_IOCTL_GET_CALI_RE: {
			ret = aw_send_afe_cal_apr(AFE_PARAM_ID_AWDSP_RX_RE_L,
						data_ptr, data_len, false);
			if (ret) {
				pr_err("%s: dsp_msg_read error: 0x%x\n",
					__func__, AFE_PARAM_ID_AWDSP_RX_RE_L);
				ret = -EFAULT;
				goto exit;
			}
			if (copy_to_user((void __user *)arg,
					data_ptr, data_len)) {
				ret = -EFAULT;
				goto exit;
			}
		} break;
		case AW882XX_IOCTL_GET_VMAX: {
			ret = aw_send_afe_cal_apr(AFE_PARAM_ID_AWDSP_RX_VMAX_L,
				data_ptr, data_len, false);
			if (ret) {
				pr_err("%s: dsp_msg_read error:0x%x\n",
					__func__, AFE_PARAM_ID_AWDSP_RX_VMAX_L);
				ret = -EFAULT;
				goto exit;
			}
			if (copy_to_user((void __user *)arg,
				data_ptr, data_len)) {
				ret = -EFAULT;
				goto exit;
			}
		} break;
		case AW882XX_IOCTL_SET_VMAX: {
			if (copy_from_user(data_ptr,
				(void __user *)arg, data_len)) {
				ret = -EFAULT;
				goto exit;
			}
			ret = aw_send_afe_cal_apr(AFE_PARAM_ID_AWDSP_RX_VMAX_L,
						data_ptr, data_len, true);
			if (ret) {
				pr_err("%s: dsp_msg_write error: 0x%x\n",
					__func__, AFE_PARAM_ID_AWDSP_RX_VMAX_L);
				ret = -EFAULT;
				goto exit;
			}
		} break;
		case AW882XX_IOCTL_SET_PARAM: {
			if (copy_from_user(data_ptr,
				(void __user *)arg, data_len)) {
				ret = -EFAULT;
				goto exit;
			}
			ret = aw_send_afe_cal_apr(AFE_PARAM_ID_AWDSP_RX_PARAMS,
						data_ptr, data_len, true);
			if (ret) {
				pr_err("%s: dsp_msg_write error: 0x%x\n",
				__func__, AFE_PARAM_ID_AWDSP_RX_PARAMS);
				ret = -EFAULT;
				goto exit;
			}
		} break;
		default: {
			pr_err("%s : cmd %d\n", __func__, cmd);
		} break;
	}
exit:
	kfree(data_ptr);
	return ret;
}

static long aw882xx_file_unlocked_ioctl(struct file *file,
			unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	struct aw882xx *aw882xx = NULL;

	if (((_IOC_TYPE(cmd)) != (AW882XX_IOCTL_MAGIC))) {
	    pr_err("%s: cmd magic err\n", __func__);
	    return -EINVAL;
	}
	aw882xx = (struct aw882xx *)file->private_data;
	ret = aw882xx_cali_operation(aw882xx, cmd, arg);
	if (ret)
		return -EINVAL;

	return 0;
}

static const struct file_operations aw882xx_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = aw882xx_file_unlocked_ioctl,
	.open = aw882xx_file_open,
	.release = aw882xx_file_release,
};

static struct miscdevice aw882xx_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = AW882XX_SMARTPA_NAME,
	.fops = &aw882xx_fops,
};

void init_aw882xx_misc_driver(void)
{
	int ret;

	ret = misc_register(&aw882xx_misc);
	if (ret) {
		pr_err("%s: misc register fail: %d\n", __func__, ret);
		return;
	}
	pr_debug("%s: misc register success", __func__);
}
void remove_aw882xx_misc_driver(void)
{
	misc_deregister(&aw882xx_misc);
}
