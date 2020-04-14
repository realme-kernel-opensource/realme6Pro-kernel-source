/***************************************************
 * File:touch.c
 * VENDOR_EDIT
 * Copyright (c)  2008- 2030  Oppo Mobile communication Corp.ltd.
 * Description:
 *             tp dev
 * Version:1.0:
 * Date created:2016/09/02
 * Author: hao.wang@Bsp.Driver
 * TAG: BSP.TP.Init
*/

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/serio.h>
#include "tp_devices.h"
#include "touchpanel_common.h"
#include <soc/oppo/oppo_project.h>
#include <soc/oppo/device_info.h>
#include "touch.h"

#define MAX_LIMIT_DATA_LENGTH         100

extern char *saved_command_line;
int g_tp_dev_vendor = TP_UNKNOWN;

/*if can not compile success, please update vendor/oppo_touchsreen*/
struct tp_dev_name tp_dev_names[] = {
    {TP_OFILM, "OFILM"},
    {TP_BIEL, "BIEL"},
    {TP_TRULY, "TRULY"},
    {TP_BOE, "BOE"},
    {TP_G2Y, "G2Y"},
    {TP_TPK, "TPK"},
    {TP_JDI, "JDI"},
    {TP_TIANMA, "TIANMA"},
    {TP_SAMSUNG, "SAMSUNG"},
    {TP_DSJM, "DSJM"},
    {TP_BOE_B8, "BOEB8"},
    {TP_UNKNOWN, "UNKNOWN"},
};

//int g_tp_dev_vendor = TP_UNKNOWN;
typedef enum {
    TP_INDEX_NULL,
    himax_83112a,
    ili9881_auo,
    ili9881_tm,
    nt36525b_boe,
    nt36525b_hlt,
    nt36672c,
    ili9881_inx
} TP_USED_INDEX;
TP_USED_INDEX tp_used_index  = TP_INDEX_NULL;


#define GET_TP_DEV_NAME(tp_type) ((tp_dev_names[tp_type].type == (tp_type))?tp_dev_names[tp_type].name:"UNMATCH")

bool __init tp_judge_ic_match(char *tp_ic_name)
{
    pr_err("[TP] tp_ic_name = %s \n", tp_ic_name);
    //pr_err("[TP] boot_command_line = %s \n", saved_command_line);

    switch(get_project()) {
		case 19759:
		case 19721:
            #ifdef ODM_LQ_EDIT
            /* modify begin by zhangchaofan@ODM_LQ@Multimedia.TP, for tp compatible 2020-01-03 */
			if (strstr(tp_ic_name, "novatek,nf_nt36672c") && (strstr(saved_command_line, "dsi_nt36672c_boe_video_display"))) {
				g_tp_dev_vendor = TP_BOE;
				tp_used_index = nt36672c;
				return true;
			}

			if (strstr(tp_ic_name, "novatek,nf_nt36672c") && (strstr(saved_command_line, "dsi_nt36672c_jdi_video_display"))) {
				g_tp_dev_vendor = TP_JDI;
				tp_used_index = nt36672c;
				return true;
			}

			if (strstr(tp_ic_name, "himax,hx83112a_nf") && (strstr(saved_command_line, "dsi_hx83112a_dsbj_vid_display"))) {
				g_tp_dev_vendor = TP_DSJM;
				tp_used_index = himax_83112a;
            	return true;
        	}
			#endif /*ODM_LQ_EDIT*/
			/* modify end by zhangchaofan@ODM_LQ@Multimedia.TP, for tp compatible 2020-01-03 */
        	pr_err("[TP] Driver does not match the project\n");

    	default:
        	pr_err("Invalid project\n");
        	break;
    }

    pr_err("Lcd module not found\n");

    return false;
}

int tp_util_get_vendor(struct hw_resource *hw_res, struct panel_info *panel_data)
{
    char* vendor;

    panel_data->test_limit_name = kzalloc(MAX_LIMIT_DATA_LENGTH, GFP_KERNEL);
    if (panel_data->test_limit_name == NULL) {
        pr_err("[TP]panel_data.test_limit_name kzalloc error\n");
    }

   panel_data->tp_type = g_tp_dev_vendor;

    if (panel_data->tp_type == TP_UNKNOWN) {
        pr_err("[TP]%s type is unknown\n", __func__);
        return 0;
    }

    vendor = GET_TP_DEV_NAME(panel_data->tp_type);

    strcpy(panel_data->manufacture_info.manufacture, vendor);

    snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
            "tp/%d/FW_%s_%s.img",
            get_project(), panel_data->chip_name, vendor);

    if (panel_data->test_limit_name) {
        snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
            "tp/%d/LIMIT_%s_%s.img",
            get_project(), panel_data->chip_name, vendor);
    }

	if (tp_used_index == himax_83112a) {
		memcpy(panel_data->manufacture_info.version, "HX_DSJM", 7);
		panel_data->firmware_headfile.firmware_data = FW_18621_HX83112A_NF_DSJM;
		panel_data->firmware_headfile.firmware_size = sizeof(FW_18621_HX83112A_NF_DSJM);
	} else if (tp_used_index == nt36672c) {
	#ifdef ODM_LQ_EDIT
	/* modify begin by zhangchaofan@ODM_LQ@Multimedia.TP, for tp devinfo 2020-01-03 */
		if (strstr(vendor, "BOE")) {
			memcpy(panel_data->manufacture_info.version, "NVT_BOE_", 8);
			panel_data->firmware_headfile.firmware_data = FW_19721_NT36672C_BOE;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_19721_NT36672C_BOE);
		}else if (strstr(vendor, "JDI")) {
			memcpy(panel_data->manufacture_info.version, "NVT_JDI_", 8);
			panel_data->firmware_headfile.firmware_data = FW_19721_NT36672C_JDI;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_19721_NT36672C_JDI);
		}
	/* modify end by zhangchaofan@ODM_LQ@Multimedia.TP, for tp devinfo 2020-01-03 */
	#endif /*ODM_LQ_EDIT*/
	}

    panel_data->manufacture_info.fw_path = panel_data->fw_name;

    pr_info("[TP]vendor:%s fw:%s limit:%s\n",
        vendor,
        panel_data->fw_name,
        panel_data->test_limit_name == NULL ? "NO Limit" : panel_data->test_limit_name);

    return 0;
}

