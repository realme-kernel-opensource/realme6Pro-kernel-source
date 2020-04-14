/* SPDX-License-Identifier: GPL-2.0 */
/*  Himax Android Driver Sample Code for inspection functions
 *
 *  Copyright (C) 2019 Himax Corporation.
 *
 *  This software is licensed under the terms of the GNU General Public
 *  License version 2,  as published by the Free Software Foundation,  and
 *  may be copied,  distributed,  and modified under those terms.
 *
 *  This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#include "himax_inspection.h"

static int g_gap_vertical_partial = 3;
static int *g_gap_vertical_part;
static int g_gap_horizontal_partial = 3;
static int *g_gap_horizontal_part;

static int g_dc_max;
int black_test_item;

static int g_1kind_raw_size;
uint32_t g_rslt_data_len;
int **g_inspection_criteria;
int *g_inspt_crtra_flag;
int *g_test_item_flag;
int do_lpwg_test;
int HX_CRITERIA_ITEM;
int HX_CRITERIA_SIZE;
char *g_rslt_data;
bool file_w_flag;
static char g_file_path[256];
static char g_rslt_log[256];
static char g_start_log[256];
#define FAIL_IN_INDEX "%s: %s FAIL in index %d\n"

char *g_hx_head_str[] = {
	"TP_Info",
	"Project_Info",
	"TestItem",
	"TestCriteria_Weight",
	"TestCriteria",
	NULL
};

/*Need to map THP_INSPECTION_ENUM*/
char *g_himax_inspection_mode[] = {
	"HIMAX_OPEN",
	"HIMAX_MICRO_OPEN",
	"HIMAX_SHORT",
	"HIMAX_RAWDATA",
	"HIMAX_BPN_RAWDATA",
	"HIMAX_SC",
	"HIMAX_WEIGHT_NOISE",
	"HIMAX_ABS_NOISE",
	"HIMAX_SORTING",
	"HIMAX_GAPTEST_RAW",
	/*"HIMAX_GAPTEST_RAW_X",*/
	/*"HIMAX_GAPTEST_RAW_Y",*/

	"HIMAX_ACT_IDLE_RAWDATA",
	"HIMAX_ACT_IDLE_BPN_RAWDATA",
	"HIMAX_ACT_IDLE_NOISE",

	"HIMAX_LPWUG_RAWDATA",
	"HIMAX_LPWUG_BPN_RAWDATA",
	"HIMAX_LPWUG_WEIGHT_NOISE",
	"HIMAX_LPWUG_ABS_NOISE",
	"HIMAX_LPWUG_IDLE_RAWDATA",
	"HIMAX_LPWUG_IDLE_BPN_RAWDATA",
	"HIMAX_LPWUG_IDLE_NOISE",

	"HIMAX_BACK_NORMAL",
	NULL
};

/* for criteria */
char *g_hx_inspt_crtra_name[] = {
	"CRITERIA_RAW_MIN",
	"CRITERIA_RAW_MAX",
	"CRITERIA_RAW_BPN_MIN",
	"CRITERIA_RAW_BPN_MAX",
	"CRITERIA_SC_MIN",
	"CRITERIA_SC_MAX",
	"CRITERIA_SC_GOLDEN",
	"CRITERIA_SHORT_MIN",
	"CRITERIA_SHORT_MAX",
	"CRITERIA_OPEN_MIN",
	"CRITERIA_OPEN_MAX",
	"CRITERIA_MICRO_OPEN_MIN",
	"CRITERIA_MICRO_OPEN_MAX",
	"CRITERIA_NOISE_WT_MIN",
	"CRITERIA_NOISE_WT_MAX",
	"CRITERIA_NOISE_ABS_MIN",
	"CRITERIA_NOISE_ABS_MAX",
	"CRITERIA_SORT_MIN",
	"CRITERIA_SORT_MAX",

	"CRITERIA_GAP_RAW_HOR_MIN",
	"CRITERIA_GAP_RAW_HOR_MAX",
	"CRITERIA_GAP_RAW_VER_MIN",
	"CRITERIA_GAP_RAW_VER_MAX",

	"ACT_IDLE_NOISE_MIN",
	"ACT_IDLE_NOISE_MAX",
	"ACT_IDLE_RAWDATA_MIN",
	"ACT_IDLE_RAWDATA_MAX",
	"ACT_IDLE_RAW_BPN_MIN",
	"ACT_IDLE_RAW_BPN_MAX",

	"LPWUG_NOISE_WT_MIN",
	"LPWUG_NOISE_WT_MAX",
	"LPWUG_NOISE_ABS_MIN",
	"LPWUG_NOISE_ABS_MAX",
	"LPWUG_RAWDATA_MIN",
	"LPWUG_RAWDATA_MAX",
	"LPWUG_RAW_BPN_MIN",
	"LPWUG_RAW_BPN_MAX",

	"LPWUG_IDLE_NOISE_MIN",
	"LPWUG_IDLE_NOISE_MAX",
	"LPWUG_IDLE_RAWDATA_MIN",
	"LPWUG_IDLE_RAWDATA_MAX",
	"LPWUG_IDLE_RAW_BPN_MIN",
	"LPWUG_IDLE_RAW_BPN_MAX",
	NULL
};


void (*fp_himax_self_test_init)(void) = himax_inspection_init;

/*
static void himax_press_powerkey(void)
{
	I(" %s POWER KEY event %x press\n", __func__, KEY_POWER);
	input_report_key(private_ts->input_dev, KEY_POWER, 1);
	input_sync(private_ts->input_dev);

	msleep(100);

	I(" %s POWER KEY event %x release\n", __func__, KEY_POWER);
	input_report_key(private_ts->input_dev, KEY_POWER, 0);
	input_sync(private_ts->input_dev);
} */


static uint8_t NOISEMAX;
static uint8_t g_recal_thx;

static int arraydata_max1, arraydata_max2, arraydata_max3;
static int arraydata_min1, arraydata_min2, arraydata_min3;

void himax_get_arraydata_edge(uint32_t *RAW)
{
	int temp, i, j;
	int len = ic_data->HX_RX_NUM * ic_data->HX_TX_NUM;
	uint32_t ArrayData[len];

	for (i = 0; i < len; i++)
		ArrayData[i] = RAW[i];
	for (j = len-1; j > 0; j--) { /*min to max*/
		for (i = 0; i < j; i++) {
			if (ArrayData[i] > ArrayData[i+1]) {
				temp = ArrayData[i];
				ArrayData[i] = ArrayData[i+1];
				ArrayData[i+1] = temp;
			}
		}
	}

	arraydata_min1 = ArrayData[0];
	arraydata_min2 = ArrayData[1];
	arraydata_min3 = ArrayData[2];
	arraydata_max1 = ArrayData[len-3];
	arraydata_max2 = ArrayData[len-2];
	arraydata_max3 = ArrayData[len-1];

}

static int hx_test_data_get(uint32_t RAW[], char *start_log, char *result,
			 int now_item)
{
	uint32_t i;

	ssize_t len = 0;
	char *testdata = NULL;
	uint32_t SZ_SIZE = g_1kind_raw_size;

	I("%s: Entering, Now type=%s!\n", __func__,
		 g_himax_inspection_mode[now_item]);

	testdata = kzalloc(sizeof(char) * SZ_SIZE, GFP_KERNEL);
	if (testdata == NULL) {
		E("%s: Memory allocation falied!\n", __func__);
		return MEM_ALLOC_FAIL;
	}

	len += snprintf((testdata + len), SZ_SIZE - len, "%s", start_log);
	for (i = 0; i < ic_data->HX_TX_NUM*ic_data->HX_RX_NUM; i++) {
		if (i > 1 && ((i + 1) % ic_data->HX_RX_NUM) == 0)
			len += snprintf((testdata + len), SZ_SIZE - len,
				 "%5d,\n", RAW[i]);
		else
			len += snprintf((testdata + len), SZ_SIZE - len,
				 "%5d,", RAW[i]);
	}
	len += snprintf((testdata + len), SZ_SIZE - len, "\n%s", result);

	memcpy(&g_rslt_data[0], testdata, len);
	g_rslt_data_len = len;
	I("%s: g_rslt_data_len=%d!\n", __func__, g_rslt_data_len);

	/* dbg */
	/* for(i = 0; i < SZ_SIZE; i++)
	 * {
	 *	I("0x%04X, ", g_rslt_data[i + (now_item * SZ_SIZE)]);
	 *	if(i > 0 && (i % 16 == 15))
	 *		PI("\n");
	 * }
	 */

	kfree(testdata);
	I("%s: End!\n", __func__);
	return NO_ERR;
}

static int himax_switch_mode_inspection(int mode)
{
	uint8_t tmp_addr[4];
	uint8_t tmp_data[4] = {0};

	I("%s: Entering\n", __func__);

	/*Stop Handshaking*/
	himax_parse_assign_cmd(sram_adr_rawdata_addr, tmp_addr,
			sizeof(tmp_addr));
	g_core_fp.fp_register_write(tmp_addr, 4, tmp_data, 0);

	/*Swtich Mode*/
	switch (mode) {
	case HIMAX_SORTING:
		tmp_data[3] = 0x00; tmp_data[2] = 0x00;
		tmp_data[1] = PWD_SORTING_START;
		tmp_data[0] = PWD_SORTING_START;
		break;
	case HIMAX_OPEN:
		tmp_data[3] = 0x00; tmp_data[2] = 0x00;
		tmp_data[1] = PWD_OPEN_START;
		tmp_data[0] = PWD_OPEN_START;
		break;
	case HIMAX_MICRO_OPEN:
		tmp_data[3] = 0x00; tmp_data[2] = 0x00;
		tmp_data[1] = PWD_OPEN_START;
		tmp_data[0] = PWD_OPEN_START;
		break;
	case HIMAX_SHORT:
		tmp_data[3] = 0x00; tmp_data[2] = 0x00;
		tmp_data[1] = PWD_SHORT_START;
		tmp_data[0] = PWD_SHORT_START;
		break;

	case HIMAX_GAPTEST_RAW:
	case HIMAX_RAWDATA:
	case HIMAX_BPN_RAWDATA:
	case HIMAX_SC:
		tmp_data[3] = 0x00; tmp_data[2] = 0x00;
		tmp_data[1] = PWD_RAWDATA_START;
		tmp_data[0] = PWD_RAWDATA_START;
		break;

	case HIMAX_WEIGHT_NOISE:
	case HIMAX_ABS_NOISE:
		tmp_data[3] = 0x00; tmp_data[2] = 0x00;
		tmp_data[1] = PWD_NOISE_START;
		tmp_data[0] = PWD_NOISE_START;
		break;

	case HIMAX_ACT_IDLE_RAWDATA:
	case HIMAX_ACT_IDLE_BPN_RAWDATA:
	case HIMAX_ACT_IDLE_NOISE:
		tmp_data[3] = 0x00; tmp_data[2] = 0x00;
		tmp_data[1] = PWD_ACT_IDLE_START;
		tmp_data[0] = PWD_ACT_IDLE_START;
		break;

	case HIMAX_LPWUG_RAWDATA:
	case HIMAX_LPWUG_BPN_RAWDATA:
	case HIMAX_LPWUG_ABS_NOISE:
	case HIMAX_LPWUG_WEIGHT_NOISE:
		tmp_data[3] = 0x00; tmp_data[2] = 0x00;
		tmp_data[1] = PWD_LPWUG_START;
		tmp_data[0] = PWD_LPWUG_START;
		break;
	case HIMAX_LPWUG_IDLE_RAWDATA:
	case HIMAX_LPWUG_IDLE_BPN_RAWDATA:
	case HIMAX_LPWUG_IDLE_NOISE:
		tmp_data[3] = 0x00; tmp_data[2] = 0x00;
		tmp_data[1] = PWD_LPWUG_IDLE_START;
		tmp_data[0] = PWD_LPWUG_IDLE_START;
		break;

	default:
		I("%s,Nothing to be done!\n", __func__);
		break;
	}

	if (g_core_fp.fp_assign_sorting_mode != NULL)
		g_core_fp.fp_assign_sorting_mode(tmp_data);
	I("%s: End of setting!\n", __func__);

	return 0;

}

static uint32_t himax_get_rawdata(uint32_t RAW[], uint32_t datalen)
{
	uint8_t *tmp_rawdata;
	bool get_raw_rlst;
	uint8_t retry = 0;
	uint32_t i = 0;
	uint32_t j = 0;
	uint32_t index = 0;
	uint32_t Min_DATA = 0xFFFFFFFF;
	uint32_t Max_DATA = 0x00000000;

	/* We use two bytes to combine a value of rawdata.*/
	tmp_rawdata = kzalloc(sizeof(uint8_t) * (datalen * 2), GFP_KERNEL);
	if (tmp_rawdata == NULL) {
		E("%s: Memory allocation falied!\n", __func__);
		return HX_INSPECT_MEMALLCTFAIL;
	}

	while (retry < 200) {
		get_raw_rlst = g_core_fp.fp_get_DSRAM_data(tmp_rawdata, false);
		if (get_raw_rlst)
			break;
		retry++;
	}

	if (retry >= 200)
		goto DIRECT_END;

	/* Copy Data*/
	for (i = 0; i < ic_data->HX_TX_NUM*ic_data->HX_RX_NUM; i++)
		RAW[i] = tmp_rawdata[(i * 2) + 1] * 256 + tmp_rawdata[(i * 2)];

	for (j = 0; j < ic_data->HX_RX_NUM; j++) {
		if (j == 0)
			PI("      RX%2d", j + 1);
		else
			PI("  RX%2d", j + 1);
	}
	PI("\n");

	for (i = 0; i < ic_data->HX_TX_NUM; i++) {
		PI("TX%2d", i + 1);
		for (j = 0; j < ic_data->HX_RX_NUM; j++) {
			PI("%5d ", RAW[index]);
			if (RAW[index] > Max_DATA)
				Max_DATA = RAW[index];

			if (RAW[index] < Min_DATA)
				Min_DATA = RAW[index];

			index++;
		}
		PI("\n");
	}
	I("Max = %5d, Min = %5d\n", Max_DATA, Min_DATA);
DIRECT_END:
	kfree(tmp_rawdata);

	if (get_raw_rlst)
		return HX_INSPECT_OK;
	else
		return HX_INSPECT_EGETRAW;

}

static void himax_switch_data_type(uint8_t checktype)
{
	uint8_t datatype = 0x00;

	switch (checktype) {
	case HIMAX_SORTING:
		datatype = DATA_SORTING;
		break;
	case HIMAX_OPEN:
		datatype = DATA_OPEN;
		break;
	case HIMAX_MICRO_OPEN:
		datatype = DATA_MICRO_OPEN;
		break;
	case HIMAX_SHORT:
		datatype = DATA_SHORT;
		break;
	case HIMAX_RAWDATA:
	case HIMAX_BPN_RAWDATA:
	case HIMAX_SC:
	case HIMAX_GAPTEST_RAW:
		datatype = DATA_RAWDATA;
		break;

	case HIMAX_WEIGHT_NOISE:
	case HIMAX_ABS_NOISE:
		datatype = DATA_NOISE;
		break;
	case HIMAX_BACK_NORMAL:
		datatype = DATA_BACK_NORMAL;
		break;
	case HIMAX_ACT_IDLE_RAWDATA:
	case HIMAX_ACT_IDLE_BPN_RAWDATA:
		datatype = DATA_ACT_IDLE_RAWDATA;
		break;
	case HIMAX_ACT_IDLE_NOISE:
		datatype = DATA_ACT_IDLE_NOISE;
		break;

	case HIMAX_LPWUG_RAWDATA:
	case HIMAX_LPWUG_BPN_RAWDATA:
		datatype = DATA_LPWUG_RAWDATA;
		break;
	case HIMAX_LPWUG_WEIGHT_NOISE:
	case HIMAX_LPWUG_ABS_NOISE:
		datatype = DATA_LPWUG_NOISE;
		break;
	case HIMAX_LPWUG_IDLE_RAWDATA:
	case HIMAX_LPWUG_IDLE_BPN_RAWDATA:
		datatype = DATA_LPWUG_IDLE_RAWDATA;
		break;
	case HIMAX_LPWUG_IDLE_NOISE:
		datatype = DATA_LPWUG_IDLE_NOISE;
		break;

	default:
		E("Wrong type=%d\n", checktype);
		break;
	}
	g_core_fp.fp_diag_register_set(datatype, 0x00, false);
}

static void himax_bank_search_set(uint16_t Nframe, uint8_t checktype)
{
	uint8_t tmp_addr[4];
	uint8_t tmp_data[4];

	/*skip frame 0x100070F4*/
	himax_parse_assign_cmd(addr_skip_frame, tmp_addr, sizeof(tmp_addr));
	g_core_fp.fp_register_read(tmp_addr, 4, tmp_data, false);

	switch (checktype) {
	case HIMAX_ACT_IDLE_RAWDATA:
	case HIMAX_ACT_IDLE_BPN_RAWDATA:
	case HIMAX_ACT_IDLE_NOISE:
		tmp_data[0] = BS_ACT_IDLE;
		break;
	case HIMAX_LPWUG_RAWDATA:
	case HIMAX_LPWUG_BPN_RAWDATA:
	case HIMAX_LPWUG_ABS_NOISE:
	case HIMAX_LPWUG_WEIGHT_NOISE:
		tmp_data[0] = BS_LPWUG;
		break;
	case HIMAX_LPWUG_IDLE_RAWDATA:
	case HIMAX_LPWUG_IDLE_BPN_RAWDATA:
	case HIMAX_LPWUG_IDLE_NOISE:
		tmp_data[0] = BS_LPWUG_dile;
		break;
	case HIMAX_RAWDATA:
	case HIMAX_BPN_RAWDATA:
	case HIMAX_SC:
		tmp_data[0] = BS_RAWDATA;
		break;
	case HIMAX_WEIGHT_NOISE:
	case HIMAX_ABS_NOISE:
		tmp_data[0] = BS_NOISE;
		break;
	default:
		tmp_data[0] = BS_OPENSHORT;
		break;
	}
	g_core_fp.fp_register_write(tmp_addr, 4, tmp_data, 0);
}

static void himax_neg_noise_sup(uint8_t *data)
{
	uint8_t tmp_addr[4];
	uint8_t tmp_data[4];

	/*0x10007FD8 Check support negative value or not */
	himax_parse_assign_cmd(addr_neg_noise_sup, tmp_addr,
		sizeof(tmp_addr));
	g_core_fp.fp_register_read(tmp_addr, 4, tmp_data, false);

	if ((tmp_data[3] & 0x04) == 0x04) {
		himax_parse_assign_cmd(data_neg_noise, tmp_data,
			sizeof(tmp_data));
		data[2] = tmp_data[2]; data[3] = tmp_data[3];
	} else
		I("%s Not support negative noise\n", __func__);
}

static void himax_set_N_frame(uint16_t Nframe, uint8_t checktype)
{
	uint8_t tmp_addr[4];
	uint8_t tmp_data[4];

	himax_bank_search_set(Nframe, checktype);

	/*IIR MAX - 0x10007294*/
	himax_parse_assign_cmd(fw_addr_set_frame_addr,
			tmp_addr, sizeof(tmp_addr));
	tmp_data[3] = 0x00; tmp_data[2] = 0x00;
	tmp_data[1] = (uint8_t)((Nframe & 0xFF00) >> 8);
	tmp_data[0] = (uint8_t)(Nframe & 0x00FF);
	g_core_fp.fp_register_write(tmp_addr, 4, tmp_data, 0);

	if (checktype == HIMAX_WEIGHT_NOISE ||
		checktype == HIMAX_ABS_NOISE ||
		checktype == HIMAX_LPWUG_WEIGHT_NOISE ||
		checktype == HIMAX_LPWUG_ABS_NOISE)
		himax_neg_noise_sup(tmp_data);

	g_core_fp.fp_register_write(tmp_addr, 4, tmp_data, 0);
}

static void himax_get_noise_base(uint8_t checktype)/*Normal Threshold*/
{
	uint8_t tmp_addr[4];
	uint8_t tmp_data[4];

	switch (checktype) {
	case HIMAX_WEIGHT_NOISE:
		himax_parse_assign_cmd(addr_normal_noise_thx,
				tmp_addr, sizeof(tmp_addr));
		break;
	case HIMAX_LPWUG_WEIGHT_NOISE:
		himax_parse_assign_cmd(addr_lpwug_noise_thx,
				tmp_addr, sizeof(tmp_addr));
		break;
	default:
		I("%s Not support type\n", __func__);
	}

	/*normal : 0x1000708F, LPWUG:0x10007093*/
	g_core_fp.fp_register_read(tmp_addr, 4, tmp_data, false);
	NOISEMAX = tmp_data[3];

	himax_parse_assign_cmd(addr_recal_thx,
			tmp_addr, sizeof(tmp_addr));
	g_core_fp.fp_register_read(tmp_addr, 4, tmp_data, false);
	g_recal_thx = tmp_data[2];/*0x10007092*/
	I("%s: NOISEMAX=%d, g_recal_thx = %d\n", __func__,
		NOISEMAX, g_recal_thx);
}

static uint16_t himax_get_palm_num(void)/*Palm Number*/
{
	uint8_t tmp_addr[4];
	uint8_t tmp_data[4];
	uint16_t palm_num;

	himax_parse_assign_cmd(addr_palm_num,
			tmp_addr, sizeof(tmp_addr));
	g_core_fp.fp_register_read(tmp_addr, 4, tmp_data, false);
	palm_num = tmp_data[3];/*0x100070AB*/
	I("%s: palm_num = %d ", __func__, palm_num);

	return palm_num;
}

static int himax_get_noise_weight_test(uint8_t checktype)
{
	uint8_t tmp_addr[4];
	uint8_t tmp_data[4];
	uint16_t weight = 0;
	uint16_t value = 0;

	himax_parse_assign_cmd(addr_weight_sup,
			tmp_addr, sizeof(tmp_addr));

	/*0x100072C8 weighting value*/
	g_core_fp.fp_register_read(tmp_addr, 4, tmp_data, false);
	if (tmp_data[3] != tmp_addr[1] || tmp_data[2] != tmp_addr[0])
		return FW_NOT_READY;

	value = (tmp_data[1] << 8) | tmp_data[0];
	I("%s: value = %d, %d, %d ", __func__, value, tmp_data[2], tmp_data[3]);

	switch (checktype) {
	case HIMAX_WEIGHT_NOISE:
		himax_parse_assign_cmd(addr_normal_weight_a,
				tmp_addr, sizeof(tmp_addr));
		break;
	case HIMAX_LPWUG_WEIGHT_NOISE:
		himax_parse_assign_cmd(addr_lpwug_weight_a,
				tmp_addr, sizeof(tmp_addr));
		break;
	default:
		I("%s Not support type\n", __func__);
	}

	/*Normal:0x1000709C, LPWUG:0x100070A0 weighting threshold*/
	g_core_fp.fp_register_read(tmp_addr, 4, tmp_data, false);
	weight = tmp_data[0];

	himax_parse_assign_cmd(addr_weight_b, tmp_addr, sizeof(tmp_addr));

	g_core_fp.fp_register_read(tmp_addr, 4, tmp_data, false);
	weight = tmp_data[1] * weight;/*0x10007095 weighting threshold*/
	I("%s: weight = %d ", __func__, weight);

	if (value > weight)
		return ERR_TEST_FAIL;
	else
		return 0;
}

static uint32_t himax_check_mode(uint8_t checktype)
{
	uint8_t tmp_data[4] = {0};
	uint8_t wait_pwd[2] = {0};

	switch (checktype) {
	case HIMAX_SORTING:
		wait_pwd[0] = PWD_SORTING_END;
		wait_pwd[1] = PWD_SORTING_END;
		break;
	case HIMAX_OPEN:
		wait_pwd[0] = PWD_OPEN_END;
		wait_pwd[1] = PWD_OPEN_END;
		break;
	case HIMAX_MICRO_OPEN:
		wait_pwd[0] = PWD_OPEN_END;
		wait_pwd[1] = PWD_OPEN_END;
		break;
	case HIMAX_SHORT:
		wait_pwd[0] = PWD_SHORT_END;
		wait_pwd[1] = PWD_SHORT_END;
		break;
	case HIMAX_RAWDATA:
	case HIMAX_BPN_RAWDATA:
	case HIMAX_SC:
	case HIMAX_GAPTEST_RAW:
		wait_pwd[0] = PWD_RAWDATA_END;
		wait_pwd[1] = PWD_RAWDATA_END;
		break;

	case HIMAX_WEIGHT_NOISE:
	case HIMAX_ABS_NOISE:
		wait_pwd[0] = PWD_NOISE_END;
		wait_pwd[1] = PWD_NOISE_END;
		break;

	case HIMAX_ACT_IDLE_RAWDATA:
	case HIMAX_ACT_IDLE_BPN_RAWDATA:
	case HIMAX_ACT_IDLE_NOISE:
		wait_pwd[0] = PWD_ACT_IDLE_END;
		wait_pwd[1] = PWD_ACT_IDLE_END;
		break;

	case HIMAX_LPWUG_RAWDATA:
	case HIMAX_LPWUG_BPN_RAWDATA:
	case HIMAX_LPWUG_ABS_NOISE:
	case HIMAX_LPWUG_WEIGHT_NOISE:
		wait_pwd[0] = PWD_LPWUG_END;
		wait_pwd[1] = PWD_LPWUG_END;
		break;
	case HIMAX_LPWUG_IDLE_RAWDATA:
	case HIMAX_LPWUG_IDLE_BPN_RAWDATA:
	case HIMAX_LPWUG_IDLE_NOISE:
		wait_pwd[0] = PWD_LPWUG_IDLE_END;
		wait_pwd[1] = PWD_LPWUG_IDLE_END;
		break;

	default:
		E("Wrong type=%d\n", checktype);
		break;
	}

	if (g_core_fp.fp_check_sorting_mode != NULL)
		g_core_fp.fp_check_sorting_mode(tmp_data);

	if ((wait_pwd[0] == tmp_data[0]) && (wait_pwd[1] == tmp_data[1])) {
		I("Change to mode=%s\n", g_himax_inspection_mode[checktype]);
		return 0;
	} else {
		return 1;
	}
}

#define TEMP_LOG \
"%s:%s,tmp_data[0]=%x,tmp_data[1]=%x,tmp_data[2]=%x,tmp_data[3]=%x\n"

static uint32_t himax_wait_sorting_mode(uint8_t checktype)
{
	uint8_t tmp_addr[4] = {0};
	uint8_t tmp_data[4] = {0};
	uint8_t wait_pwd[2] = {0};
	int count = 0;

	switch (checktype) {
	case HIMAX_SORTING:
		wait_pwd[0] = PWD_SORTING_END;
		wait_pwd[1] = PWD_SORTING_END;
		break;
	case HIMAX_OPEN:
		wait_pwd[0] = PWD_OPEN_END;
		wait_pwd[1] = PWD_OPEN_END;
		break;
	case HIMAX_MICRO_OPEN:
		wait_pwd[0] = PWD_OPEN_END;
		wait_pwd[1] = PWD_OPEN_END;
		break;
	case HIMAX_SHORT:
		wait_pwd[0] = PWD_SHORT_END;
		wait_pwd[1] = PWD_SHORT_END;
		break;
	case HIMAX_RAWDATA:
	case HIMAX_BPN_RAWDATA:
	case HIMAX_SC:
	case HIMAX_GAPTEST_RAW:
		wait_pwd[0] = PWD_RAWDATA_END;
		wait_pwd[1] = PWD_RAWDATA_END;
		break;
	case HIMAX_WEIGHT_NOISE:
	case HIMAX_ABS_NOISE:
		wait_pwd[0] = PWD_NOISE_END;
		wait_pwd[1] = PWD_NOISE_END;
		break;
	case HIMAX_ACT_IDLE_RAWDATA:
	case HIMAX_ACT_IDLE_BPN_RAWDATA:
	case HIMAX_ACT_IDLE_NOISE:
		wait_pwd[0] = PWD_ACT_IDLE_END;
		wait_pwd[1] = PWD_ACT_IDLE_END;
		break;

	case HIMAX_LPWUG_RAWDATA:
	case HIMAX_LPWUG_BPN_RAWDATA:
	case HIMAX_LPWUG_ABS_NOISE:
	case HIMAX_LPWUG_WEIGHT_NOISE:
		wait_pwd[0] = PWD_LPWUG_END;
		wait_pwd[1] = PWD_LPWUG_END;
		break;
	case HIMAX_LPWUG_IDLE_RAWDATA:
	case HIMAX_LPWUG_IDLE_BPN_RAWDATA:
	case HIMAX_LPWUG_IDLE_NOISE:
		wait_pwd[0] = PWD_LPWUG_IDLE_END;
		wait_pwd[1] = PWD_LPWUG_IDLE_END;
		break;

	default:
		I("No Change Mode and now type=%d\n", checktype);
		break;
	}

	do {
		if (g_core_fp.fp_check_sorting_mode != NULL)
			g_core_fp.fp_check_sorting_mode(tmp_data);
		if ((wait_pwd[0] == tmp_data[0]) &&
			(wait_pwd[1] == tmp_data[1]))
			return HX_INSPECT_OK;

		himax_parse_assign_cmd(fw_addr_chk_fw_status,
				tmp_addr, sizeof(tmp_addr));
		g_core_fp.fp_register_read(tmp_addr, 4, tmp_data, false);
		I(TEMP_LOG, __func__, "0x900000A8",
			tmp_data[0], tmp_data[1], tmp_data[2], tmp_data[3]);

		himax_parse_assign_cmd(fw_addr_flag_reset_event,
				tmp_addr, sizeof(tmp_addr));
		g_core_fp.fp_register_read(tmp_addr, 4, tmp_data, false);
		I(TEMP_LOG, __func__, "0x900000E4",
			tmp_data[0], tmp_data[1], tmp_data[2], tmp_data[3]);

		himax_parse_assign_cmd(fw_addr_fw_dbg_msg_addr,
				tmp_addr, sizeof(tmp_addr));
		g_core_fp.fp_register_read(tmp_addr, 4, tmp_data, false);
		I(TEMP_LOG, __func__, "0x10007F40",
			tmp_data[0], tmp_data[1], tmp_data[2], tmp_data[3]);
		I("Now retry %d times!\n", count++);
		msleep(50);
	} while (count < 50);

	return HX_INSPECT_ESWITCHMODE;
}

static int hx_turn_on_mp_func(int on)
{
	int rslt = 0;
	int retry = 3;
	uint8_t tmp_addr[4] = {0};
	uint8_t tmp_data[4] = {0};
	uint8_t tmp_read[4] = {0};
	/* char *tmp_chipname = private_ts->chip_name; */

	himax_parse_assign_cmd(addr_ctrl_mpap_ovl, tmp_addr,
		sizeof(tmp_addr));
	if (on) {
		I("%s : Turn on!\n", __func__);
		if (strcmp(HX_83102D_SERIES_PWON, private_ts->chip_name) == 0) {
			I("%s: need to enter Mp mode!\n", __func__);
			himax_parse_assign_cmd(PWD_TURN_ON_MPAP_OVL,
					tmp_data, sizeof(tmp_data));
			do {
				g_core_fp.fp_register_write(tmp_addr, 4,
					tmp_data, 0);
				usleep_range(10000, 10001);
				g_core_fp.fp_register_read(tmp_addr, 4,
					tmp_read, false);

				I("%s:read2=0x%02X,read1=0x%02X,read0=0x%02X\n",
					__func__,
					tmp_read[2],
					tmp_read[1],
					tmp_read[0]);

				retry--;
			} while (((retry > 0)
				&& (tmp_read[2] != tmp_data[2]
				&& tmp_read[1] != tmp_data[1]
				&& tmp_read[0] != tmp_data[0])));
		} else {
			I("%s:Nothing to be done!\n", __func__);
		}
	} else {
		I("%s : Turn off!\n", __func__);
		if (strcmp(HX_83102D_SERIES_PWON, private_ts->chip_name) == 0) {
			I("%s: need to enter Mp mode!\n", __func__);

			himax_parse_assign_cmd(ic_cmd_rst, tmp_data,
				sizeof(tmp_data));
			do {
				g_core_fp.fp_register_write(tmp_addr, 4,
					tmp_data, 0);
				usleep_range(10000, 10001);
				g_core_fp.fp_register_read(tmp_addr, 4,
					tmp_read, false);

				I("%s:read2=0x%02X,read1=0x%02X,read0=0x%02X\n",
					__func__,
					tmp_read[2],
					tmp_read[1],
					tmp_read[0]);

				retry--;
			} while ((retry > 0)
				&& (tmp_read[2] != tmp_data[2]
				&& tmp_read[1] != tmp_data[1]
				&& tmp_read[0] != tmp_data[0]));
		} else {
			I("%s Nothing to be done!\n", __func__);
		}
	}
	return rslt;
}

/* HX_GAP START gap test function */
/* extern int himax_write_to_ic_flash_flow(uint32_t start_addr,*/
/*		uint32_t *write_data, uint32_t write_len);*/

static int himax_gap_test_vertical_setting(void)
{
	g_gap_vertical_part = kcalloc(g_gap_vertical_partial,
				sizeof(int), GFP_KERNEL);
	if (g_gap_vertical_part == NULL) {
		E("%s: Memory allocation falied!\n", __func__);
		return MEM_ALLOC_FAIL;
	}
	g_gap_vertical_part[0] = 0;
	g_gap_vertical_part[1] = 4;
	g_gap_vertical_part[2] = 8;

	return NO_ERR;
}

static void himax_cal_gap_data_vertical(int start, int end_idx, int direct,
		uint32_t *org_raw, uint32_t *result_raw)
{
	int i = 0;
	int rx_num = ic_data->HX_RX_NUM;

	I("%s:start=%d,end_idx=%d\n", __func__, start, end_idx);

	for (i = start; i < (start + rx_num*end_idx); i++) {
		if (direct == 0) { /* up - down */
			if (i < start+rx_num)
				result_raw[i] = 0;
			else
				result_raw[i] = org_raw[i-rx_num] - org_raw[i];

		} else { /* down - up */
			if (i > (start + rx_num*(end_idx-1)-1))
				result_raw[i] = 0;
			else
				result_raw[i] = org_raw[i+rx_num] - org_raw[i];

		}
	}
}

static int himax_gap_test_vertical_raw(int test_type, uint32_t *org_raw)
{
	int i_partial = 0;
	int tmp_start = 0;
	int tmp_end_idx = 0;
	uint32_t *result_raw;
	int i = 0;
	int ret_val = NO_ERR;

	int tx_num = ic_data->HX_TX_NUM;
	int rx_num = ic_data->HX_RX_NUM;

	if (himax_gap_test_vertical_setting())
		return MEM_ALLOC_FAIL;

	I("Print vertical ORG RAW\n");
	for (i = 0; i < tx_num*rx_num; i++) {
		I("%04d,", org_raw[i]);
		if (i > 0 && i%rx_num == (rx_num-1))
			I("\n");
	}

	result_raw = kcalloc(tx_num*rx_num, sizeof(uint32_t), GFP_KERNEL);
	if (result_raw == NULL) {
		E("%s: Memory allocation falied!\n", __func__);
		goto alloc_result_raw_failed;
	}

	for (i_partial = 0; i_partial < g_gap_vertical_partial; i_partial++) {

		tmp_start = g_gap_vertical_part[i_partial]*rx_num;
		if (i_partial+1 == g_gap_vertical_partial)
			tmp_end_idx = tx_num - g_gap_vertical_part[i_partial];
		else
			tmp_end_idx = g_gap_vertical_part[i_partial+1] -
				 g_gap_vertical_part[i_partial];

		if (i_partial % 2 == 0)
			himax_cal_gap_data_vertical(tmp_start, tmp_end_idx, 0,
						org_raw, result_raw);
		else
			himax_cal_gap_data_vertical(tmp_start, tmp_end_idx, 1,
						org_raw, result_raw);

	}

	I("Print Vertical New RAW\n");
	for (i = 0; i < tx_num*rx_num; i++) {
		I("%04d,", result_raw[i]);
		if (i > 0 && i%rx_num == (rx_num-1))
			I("\n");
	}

	for (i = 0; i < tx_num*rx_num; i++) {
		if (result_raw[i] < g_inspection_criteria[IDX_GAP_VER_RAWMIN][i]
		 &&
		 result_raw[i] > g_inspection_criteria[IDX_GAP_VER_RAWMAX][i]) {
			ret_val = NO_ERR - i;
			break;
		}
	}

	/* himax_write_to_ic_flash_flow(0x1A000,result_raw,tx_num*rx_num); */
	kfree(result_raw);
alloc_result_raw_failed:
	kfree(g_gap_vertical_part);
	g_gap_vertical_part = NULL;

	return ret_val;
}

static int himax_gap_test_horizontal_setting(void)
{
	g_gap_horizontal_part = kcalloc(g_gap_horizontal_partial,
		sizeof(int), GFP_KERNEL);
	if (g_gap_horizontal_part == NULL) {
		E("%s: Memory allocation falied!\n", __func__);
		return MEM_ALLOC_FAIL;
	}
	g_gap_horizontal_part[0] = 0;
	g_gap_horizontal_part[1] = 8;
	g_gap_horizontal_part[2] = 24;

	return NO_ERR;
}

static void himax_cal_gap_data_horizontal(int start, int end_idx, int direct,
		uint32_t *org_raw, uint32_t *result_raw)
{
	int i = 0;
	int j = 0;
	int rx_num = ic_data->HX_RX_NUM;
	int tx_num = ic_data->HX_TX_NUM;

	I("start=%d,end_idx=%d\n", start, end_idx);

	for (j = 0; j < tx_num; j++) {
		for (i = (start + (j*rx_num));
			i < (start + (j*rx_num) + end_idx); i++) {
			/* left - right */
			if (direct == 0) {
				if (i == (start + (j*rx_num)))
					result_raw[i] = 0;
				else
					result_raw[i] =
						org_raw[i-1] - org_raw[i];

			} else { /* right - left */
				if (i == ((start + (j*rx_num) + end_idx) - 1))
					result_raw[i] = 0;
				else
					result_raw[i] =
						org_raw[i + 1] - org_raw[i];
			}
		}
	}
}

static int himax_gap_test_honrizontal_raw(int test_type, uint32_t *raw)
{
	int rx_num = ic_data->HX_RX_NUM;
	int tx_num = ic_data->HX_TX_NUM;
	int tmp_start = 0;
	int tmp_end_idx = 0;
	int i_partial = 0;
	uint32_t *result_raw;
	int i = 0;
	int ret_val = NO_ERR;

	if (himax_gap_test_horizontal_setting())
		return MEM_ALLOC_FAIL;

	result_raw = kcalloc(ic_data->HX_TX_NUM * ic_data->HX_RX_NUM,
			sizeof(uint32_t), GFP_KERNEL);
	if (result_raw == NULL) {
		E("%s: Memory allocation falied!\n", __func__);
		goto alloc_result_raw_failed;
	}

	I("Print Horizontal ORG RAW\n");
	for (i = 0; i < tx_num*rx_num; i++) {
		I("%04d,", raw[i]);
		if (i > 0 && i%rx_num == (rx_num-1))
			I("\n");
	}

	for (i_partial = 0;
	i_partial < g_gap_horizontal_partial;
	i_partial++) {
		tmp_start	= g_gap_horizontal_part[i_partial];
		if (i_partial+1 == g_gap_horizontal_partial)
			tmp_end_idx = rx_num - g_gap_horizontal_part[i_partial];
		else
			tmp_end_idx = g_gap_horizontal_part[i_partial+1] -
				g_gap_horizontal_part[i_partial];

		if (i_partial % 2 == 0)
			himax_cal_gap_data_horizontal(tmp_start, tmp_end_idx,
						0, raw, result_raw);
		else
			himax_cal_gap_data_horizontal(tmp_start, tmp_end_idx,
						1, raw, result_raw);

	}
	I("Print Horizontal New RAW\n");
	for (i = 0; i < tx_num*rx_num; i++) {
		I("%04d,", result_raw[i]);
		if (i > 0 && i%rx_num == (rx_num-1))
			I("\n");
	}

	for (i = 0; i < tx_num*rx_num; i++) {
		if (result_raw[i] < g_inspection_criteria[IDX_GAP_HOR_RAWMIN][i]
		&&
		result_raw[i] > g_inspection_criteria[IDX_GAP_HOR_RAWMAX][i]) {
			ret_val = NO_ERR - i;
			break;
		}
	}

	/* himax_write_to_ic_flash_flow(0x1A800,result_raw,tx_num*rx_num); */
	kfree(result_raw);
alloc_result_raw_failed:
	kfree(g_gap_horizontal_part);
	g_gap_horizontal_part = NULL;

	return ret_val;
}

static uint32_t himax_data_campare(uint8_t checktype, uint32_t *RAW,
		int ret_val)
{
	int i = 0;
	int idx_max = 0;
	int idx_min = 0;
	int block_num = ic_data->HX_TX_NUM*ic_data->HX_RX_NUM;
	uint16_t palm_num = 0;
	uint16_t noise_count = 0;

	switch (checktype) {
	case HIMAX_SORTING:
		idx_min = IDX_SORTMIN;
		break;
	case HIMAX_OPEN:
		idx_max = IDX_OPENMAX;
		idx_min = IDX_OPENMIN;
		break;

	case HIMAX_MICRO_OPEN:
		idx_max = IDX_M_OPENMAX;
		idx_min = IDX_M_OPENMIN;
		break;

	case HIMAX_SHORT:
		idx_max = IDX_SHORTMAX;
		idx_min = IDX_SHORTMIN;
		break;

	case HIMAX_RAWDATA:
		idx_max = IDX_RAWMAX;
		idx_min = IDX_RAWMIN;
		break;

	case HIMAX_BPN_RAWDATA:
		idx_max = IDX_BPN_RAWMAX;
		idx_min = IDX_BPN_RAWMIN;
		break;
	case HIMAX_SC:
		idx_max = IDX_SCMAX;
		idx_min = IDX_SCMIN;
		break;
	case HIMAX_WEIGHT_NOISE:
		idx_max = IDX_WT_NOISEMAX;
		idx_min = IDX_WT_NOISEMIN;
		break;
	case HIMAX_ABS_NOISE:
		idx_max = IDX_ABS_NOISEMAX;
		idx_min = IDX_ABS_NOISEMIN;
		break;
	case HIMAX_GAPTEST_RAW:
		break;

	case HIMAX_ACT_IDLE_RAWDATA:
		idx_max = IDX_ACT_IDLE_RAWDATA_MAX;
		idx_min = IDX_ACT_IDLE_RAWDATA_MIN;
		break;

	case HIMAX_ACT_IDLE_BPN_RAWDATA:
		idx_max = IDX_ACT_IDLE_RAW_BPN_MAX;
		idx_min = IDX_ACT_IDLE_RAW_BPN_MIN;
		break;

	case HIMAX_ACT_IDLE_NOISE:
		idx_max = IDX_ACT_IDLE_NOISE_MAX;
		idx_min = IDX_ACT_IDLE_NOISE_MIN;
		break;

	case HIMAX_LPWUG_RAWDATA:
		idx_max = IDX_LPWUG_RAWDATA_MAX;
		idx_min = IDX_LPWUG_RAWDATA_MIN;
		break;

	case HIMAX_LPWUG_BPN_RAWDATA:
		idx_max = IDX_LPWUG_RAW_BPN_MAX;
		idx_min = IDX_LPWUG_RAW_BPN_MIN;
		break;

	case HIMAX_LPWUG_WEIGHT_NOISE:
		idx_max = IDX_LPWUG_WT_NOISEMAX;
		idx_min = IDX_LPWUG_WT_NOISEMIN;
		break;

	case HIMAX_LPWUG_ABS_NOISE:
		idx_max = IDX_LPWUG_NOISE_ABS_MAX;
		idx_min = IDX_LPWUG_NOISE_ABS_MIN;
		break;

	case HIMAX_LPWUG_IDLE_RAWDATA:
		idx_max = IDX_LPWUG_IDLE_RAWDATA_MAX;
		idx_min = IDX_LPWUG_IDLE_RAWDATA_MIN;
		break;

	case HIMAX_LPWUG_IDLE_BPN_RAWDATA:
		idx_max = IDX_LPWUG_IDLE_RAW_BPN_MAX;
		idx_min = IDX_LPWUG_IDLE_RAW_BPN_MIN;
		break;

	case HIMAX_LPWUG_IDLE_NOISE:
		idx_max = IDX_LPWUG_IDLE_NOISE_MAX;
		idx_min = IDX_LPWUG_IDLE_NOISE_MIN;
		break;

	default:
		E("Wrong type=%d\n", checktype);
		break;
	}

	/*data process*/
	switch (checktype) {
	case HIMAX_SORTING:
		for (i = 0; i < block_num; i++)
			g_inspection_criteria[idx_max][i] = 999999;
		break;
	case HIMAX_BPN_RAWDATA:
	case HIMAX_ACT_IDLE_BPN_RAWDATA:
	case HIMAX_LPWUG_BPN_RAWDATA:
	case HIMAX_LPWUG_IDLE_BPN_RAWDATA:
		for (i = 0; i < block_num; i++)
			RAW[i] = (int)RAW[i] * 100 / g_dc_max;
		break;
	case HIMAX_SC:
		for (i = 0; i < block_num; i++) {
			RAW[i] = ((int)RAW[i]
				- g_inspection_criteria[IDX_SC_GOLDEN][i])
				* 100 / g_inspection_criteria[IDX_SC_GOLDEN][i];
		}
		break;
	}

	/*data campare*/
	switch (checktype) {
	case HIMAX_GAPTEST_RAW:
		if (
		himax_gap_test_vertical_raw(HIMAX_GAPTEST_RAW, RAW) != NO_ERR) {
			E("%s: HIMAX_GAPTEST_RAW FAIL\n", __func__);
			ret_val |= 1 << (checktype + ERR_SFT);
			break;
		}
		if (himax_gap_test_honrizontal_raw(HIMAX_GAPTEST_RAW, RAW)
		!= NO_ERR) {
			E("%s: HIMAX_GAPTEST_RAW FAIL\n", __func__);
			ret_val |= 1 << (checktype + ERR_SFT);
			break;
		}
		break;

	case HIMAX_WEIGHT_NOISE:
	case HIMAX_LPWUG_WEIGHT_NOISE:
		noise_count = 0;
		himax_get_noise_base(checktype);
		palm_num = himax_get_palm_num();
		for (i = 0; i < (ic_data->HX_TX_NUM * ic_data->HX_RX_NUM);
		i++) {
			if ((int)RAW[i] > NOISEMAX)
				noise_count++;
		}
		I("noise_count=%d\n", noise_count);
		if (noise_count > palm_num) {
			E("%s: noise test FAIL\n", __func__);
			ret_val |= 1 << (checktype + ERR_SFT);
			break;
		}
		snprintf(g_start_log, 256 * sizeof(char), "\n Threshold = %d\n",
				NOISEMAX);
		/*Check weightingt*/
		if (himax_get_noise_weight_test(checktype) < 0) {
			I("%s: %s FAIL %X\n", __func__,
				g_himax_inspection_mode[checktype], ret_val);
			ret_val |= 1 << (checktype + ERR_SFT);
			break;
		}

		/*Check negative side noise*/
		for (i = 0; i < block_num; i++) {
			if ((int)RAW[i]
			> (g_inspection_criteria[idx_max][i]
			* g_recal_thx / 100)
			|| (int)RAW[i]
			< (g_inspection_criteria[idx_min][i]
			* g_recal_thx / 100)) {
				E(FAIL_IN_INDEX, __func__,
					g_himax_inspection_mode[checktype], i);
				ret_val |= 1 << (checktype + ERR_SFT);
				break;
			}
		}
		break;

	case HIMAX_LPWUG_IDLE_RAWDATA:
	case HIMAX_LPWUG_IDLE_BPN_RAWDATA:
	case HIMAX_LPWUG_IDLE_NOISE:
	case HIMAX_ACT_IDLE_RAWDATA:
	case HIMAX_ACT_IDLE_BPN_RAWDATA:
	case HIMAX_ACT_IDLE_NOISE:
		block_num = ic_data->ic_adc_num;
	case HIMAX_SORTING:
	case HIMAX_OPEN:
	case HIMAX_MICRO_OPEN:
	case HIMAX_SHORT:
	case HIMAX_RAWDATA:
	case HIMAX_BPN_RAWDATA:
	case HIMAX_SC:
	case HIMAX_ABS_NOISE:
	case HIMAX_LPWUG_RAWDATA:
	case HIMAX_LPWUG_BPN_RAWDATA:
	case HIMAX_LPWUG_ABS_NOISE:
		for (i = 0; i < block_num; i++) {
			if ((int)RAW[i] > g_inspection_criteria[idx_max][i]
			|| (int)RAW[i] < g_inspection_criteria[idx_min][i]) {
				E(FAIL_IN_INDEX, __func__,
					g_himax_inspection_mode[checktype], i);
				ret_val |= 1 << (checktype + ERR_SFT);
				break;
			}
		}
		break;

	default:
		E("Wrong type=%d\n", checktype);
		break;
	}

	I("%s: %s %s\n", __func__, g_himax_inspection_mode[checktype],
			(ret_val == HX_INSPECT_OK)?"PASS":"FAIL");

	return ret_val;
}

static int himax_get_max_dc(void)
{
	uint8_t tmp_data[DATA_LEN_4];
	uint8_t tmp_addr[DATA_LEN_4];
	int dc_max = 0;

	himax_parse_assign_cmd(addr_max_dc, tmp_addr, sizeof(tmp_addr));

	g_core_fp.fp_register_read(tmp_addr, DATA_LEN_4, tmp_data, 0);
	I("%s: tmp_data[0]=%x,tmp_data[1]=%x\n", __func__,
			tmp_data[0], tmp_data[1]);

	dc_max = tmp_data[1] << 8 | tmp_data[0];
	I("%s: dc max = %d\n", __func__, dc_max);
	return dc_max;
}

/*	 HX_GAP END*/
static uint32_t mpTestFunc(uint8_t checktype, uint32_t datalen)
{
	uint32_t len = 0;
	uint32_t *RAW;
	int n_frame = 0;
	uint32_t ret_val = HX_INSPECT_OK;

	/*uint16_t* pInspectGridData = &gInspectGridData[0];*/
	/*uint16_t* pInspectNoiseData = &gInspectNoiseData[0];*/

	I("Now Check type = %d\n", checktype);

	RAW = kzalloc(datalen * sizeof(uint32_t), GFP_KERNEL);

	if (himax_check_mode(checktype)) {
		/*himax_check_mode(checktype);*/

		I("Need Change Mode ,target=%s\n",
		g_himax_inspection_mode[checktype]);

		g_core_fp.fp_sense_off(true);
		hx_turn_on_mp_func(1);

#if !defined(HX_ZERO_FLASH)
		if (g_core_fp.fp_reload_disable != NULL)
			g_core_fp.fp_reload_disable(1);
#endif

		himax_switch_mode_inspection(checktype);

		switch (checktype) {
		case HIMAX_WEIGHT_NOISE:
		case HIMAX_ABS_NOISE:
			n_frame = NOISEFRAME;
			break;
		case HIMAX_ACT_IDLE_RAWDATA:
		case HIMAX_ACT_IDLE_NOISE:
		case HIMAX_ACT_IDLE_BPN_RAWDATA:
			n_frame = NORMAL_IDLE_RAWDATA_NOISEFRAME;
			break;
		case HIMAX_LPWUG_RAWDATA:
		case HIMAX_LPWUG_BPN_RAWDATA:
			n_frame = LPWUG_RAWDATAFRAME;
			break;
		case HIMAX_LPWUG_WEIGHT_NOISE:
		case HIMAX_LPWUG_ABS_NOISE:
			n_frame = LPWUG_NOISEFRAME;
			break;
		case HIMAX_LPWUG_IDLE_RAWDATA:
		case HIMAX_LPWUG_IDLE_BPN_RAWDATA:
			n_frame = LPWUG_IDLE_RAWDATAFRAME;
			break;
		case HIMAX_LPWUG_IDLE_NOISE:
			n_frame = LPWUG_IDLE_NOISEFRAME;
			break;
		default:
			n_frame = OTHERSFRAME;
		}
		himax_set_N_frame(n_frame, checktype);

		g_core_fp.fp_sense_on(1);

	}

	ret_val |= himax_wait_sorting_mode(checktype);
	if (ret_val) {
		E("%s: himax_wait_sorting_mode FAIL\n", __func__);
		ret_val |= (1 << (checktype + ERR_SFT));
		goto fail_wait_sorting_mode;
	}
	himax_switch_data_type(checktype);

	ret_val |= himax_get_rawdata(RAW, datalen);
	if (ret_val) {
		E("%s: himax_get_rawdata FAIL\n", __func__);
		ret_val |= (1 << (checktype + ERR_SFT));
		goto fail_get_rawdata;
	}

	/*get Max DC from FW*/
	g_dc_max = himax_get_max_dc();

	/* back to normal */
	himax_switch_data_type(HIMAX_BACK_NORMAL);

	I("%s: Init OK, start to test!\n", __func__);

	len += snprintf(g_start_log+len, 256 * sizeof(char), "\n%s%s\n",
		g_himax_inspection_mode[checktype], ": data as follow!\n");

	ret_val |= himax_data_campare(checktype, RAW, ret_val);

	himax_get_arraydata_edge(RAW);

	len += snprintf(g_start_log + len, 256 * sizeof(char) - len,
			"\n arraydata_min1 = %d,", arraydata_min1);
	len += snprintf(g_start_log + len, 256 * sizeof(char) - len,
			"  arraydata_min2 = %d,", arraydata_min2);
	len += snprintf(g_start_log + len, 256 * sizeof(char) - len,
			"  arraydata_min3 = %d,", arraydata_min3);
	len += snprintf(g_start_log + len, 256 * sizeof(char) - len,
			"\n arraydata_max1 = %d,", arraydata_max1);
	len += snprintf(g_start_log + len, 256 * sizeof(char) - len,
			"  arraydata_max2 = %d,", arraydata_max2);
	len += snprintf(g_start_log + len, 256 * sizeof(char) - len,
			"  arraydata_max3 = %d\n", arraydata_max3);

	if (!ret_val) {/*PASS*/
		snprintf(g_rslt_log, 256 * sizeof(char), "\n%s%s\n",
			g_himax_inspection_mode[checktype], ":Test Pass!");
		I("pass write log\n");
	} else {/*FAIL*/
		snprintf(g_rslt_log, 256 * sizeof(char), "\n%s%s\n",
			g_himax_inspection_mode[checktype], ":Test Fail!");
	  I("fail write log\n");
	}

	hx_test_data_get(RAW, g_start_log, g_rslt_log, checktype);
fail_get_rawdata:
fail_wait_sorting_mode:
	kfree(RAW);
	return ret_val;
}

/* claculate 10's power function */
static int himax_power_cal(int pow, int number)
{
	int i = 0;
	int result = 1;

	for (i = 0; i < pow; i++)
		result *= 10;
	result = result * number;

	return result;

}

/* String to int */
static int hiamx_parse_str2int(char *str)
{
	int i = 0;
	int temp_cal = 0;
	int result = 0;
	int str_len = strlen(str);
	int negtive_flag = 0;

	for (i = 0; i < strlen(str); i++) {
		if (str[i] != '-' && str[i] > '9' && str[i] < '0') {
			E("%s: Parsing fail!\n", __func__);
			result = -9487;
			negtive_flag = 0;
			break;
		}
		if (str[i] == '-') {
			negtive_flag = 1;
			continue;
		}
		temp_cal = str[i] - '0';
		result += himax_power_cal(str_len-i-1, temp_cal);
		/* str's the lowest char is the number's the highest number
		 * So we should reverse this number before using the power
		 * function
		 * -1: starting number is from 0 ex:10^0 = 1,10^1=10
		 */
	}

	if (negtive_flag == 1)
		result = 0 - result;

	return result;
}


/* Get sub-string from original string by using some charaters
 * return size of result
 */
static int himax_saperate_comma(char *str_data, int str_size,
				char **result, int item_str_size)
{
	int count = 0;
	int str_count = 0; /* now string*/
	int char_count = 0; /* now char count in string*/
	int stop_cnt = ic_data->HX_TX_NUM * ic_data->HX_RX_NUM + 1;

	do {
		switch (str_data[count]) {
		case ASCII_COMMA:
		case ACSII_SPACE:
		case ASCII_CR:
		case ASCII_LF:
			count++;
			/* If end of line as above condifiton,
			 * differencing the count of char.
			 * If char_count != 0
			 * it's meaning this string is parsing over .
			 * The Next char is belong to next string
			 */
			if (char_count != 0) {
				char_count = 0;
				str_count++;
				if (str_count >= stop_cnt)
					break;
			}
			break;
		default:
			result[str_count][char_count++] =
			str_data[count];
			count++;
			break;
		}
	} while (count < str_size && str_count < item_str_size);

	return str_count;
}

static int hx_diff_str(char *str1, char *str2)
{
	int i = 0;
	int result = 0; /* zero is all same, non-zero is not same index*/
	int str1_len = strlen(str1);
	int str2_len = strlen(str2);

	if (str1_len != str2_len) {
		if (private_ts->debug_log_level & BIT(4))
			I("%s:Size different!\n", __func__);
		return LENGTH_FAIL;
	}

	for (i = 0; i < str1_len; i++) {
		if (str1[i] != str2[i]) {
			result = i+1;
			/*I("%s: different in %d!\n", __func__, result);*/
			return result;
		}
	}

	return result;
}

/* get idx of criteria whe parsing file */
int hx_find_crtra_id(char *input)
{
	int i = 0;
	int result = 0;

	for (i = 0 ; i < HX_CRITERIA_SIZE ; i++) {
		if (hx_diff_str(g_hx_inspt_crtra_name[i], input) == 0) {
			result = i;
			I("find the str=%s,idx=%d\n",
			  g_hx_inspt_crtra_name[i], i);
			break;
		}
	}
	if (i > (HX_CRITERIA_SIZE - 1)) {
		E("%s: find Fail!\n", __func__);
		return LENGTH_FAIL;
	}

	return result;
}

int hx_print_crtra_after_parsing(void)
{
	int i = 0, j = 0;
	int all_mut_len = ic_data->HX_TX_NUM*ic_data->HX_RX_NUM;

	for (i = 0; i < HX_CRITERIA_SIZE; i++) {
		I("Now is %s\n", g_hx_inspt_crtra_name[i]);
		if (g_inspt_crtra_flag[i] == 1) {
			for (j = 0; j < all_mut_len; j++) {
				I("%d, ", g_inspection_criteria[i][j]);
				if (j % 16 == 15)
					PI("\n");
			}
		} else {
			I("No this Item in this criteria file!\n");
		}
		PI("\n");
	}

	return 0;
}

static int hx_get_crtra_by_name(char **result, int size_of_result_str)
{
	int i = 0;
	/* count of criteria type */
	int count_type = 0;
	/* count of criteria data */
	int count_data = 0;
	int err = HX_INSPECT_OK;
	int temp = 0;

	/* get criteria and assign to a global array(2-Dimensional/int) */
	/* basiclly the size of criteria will be
	 * (crtra_count * (all_mut_len) + crtra_count)
	 * but we use file size to be the end of counter
	 */
	for (i = 0; i < size_of_result_str && result[i] != NULL; i++) {
		/* It have get one page(all mutual) criteria data!
		 * And we should skip the string of criteria name!
		 */
		if (i == 0) {
			count_data = 0;

			if (private_ts->debug_log_level & BIT(4))
				I("Now find str=%s ,idx=%d\n", result[i], i);

			/* check the item of criteria is in criteria file
			 * or not
			 */
			count_type = hx_find_crtra_id(result[i]);
			if (count_type < 0) {
				E("1. %s:Name Not match!\n", __func__);
				/* E("can recognize[%d]=%s\n", count_type,
				 * g_hx_inspt_crtra_name[count_type]);
				 */
				E("get from file[%d]=%s\n", i, result[i]);
				E("Please check criteria file again!\n");
				err = HX_INSPECT_EFILE;
				goto END_FUNCTION;
			} else {
				/*I("Now str=%s, idx=%d\n",*/
				/*g_hx_inspt_crtra_name[count_type],*/
				/*count_type);*/
				g_inspt_crtra_flag[count_type] = 1;
			}
			continue;
		}
		/* change string to int*/
		temp = hiamx_parse_str2int(result[i]);
		if (temp != -9487)
			g_inspection_criteria[count_type][count_data] = temp;
		else {
			E("%s: Parsing Fail in %d\n", __func__, i);
			E("in range:[%d]=%s\n", count_type,
			  g_hx_inspt_crtra_name[count_type]);
			E("btw, get from file[%d]=%s\n", i, result[i]);
			break;
		}
		/* dbg
		 * I("[%d]g_inspection_criteria[%d][%d]=%d\n",
		 * i, count_type, count_data,
		 * g_inspection_criteria[count_type][count_data]);
		 */
		count_data++;

	}

	if (private_ts->debug_log_level & BIT(4)) {
		/* dbg:print all of criteria from parsing file */
		hx_print_crtra_after_parsing();
	}

	/*I("Total loop=%d\n", i);*/
END_FUNCTION:
	return err;
}

static int himax_parse_criteria_str(char *str_data, int str_size)
{
	int err = HX_INSPECT_OK;
	char **result;
	int i = 0;
	int crtra_count = 1;
	int data_size = 0; /* The maximum of number Data*/
	int all_mut_len = ic_data->HX_TX_NUM*ic_data->HX_RX_NUM;
	int str_max_len = 0;
	int size_of_result_str = 0;

	I("%s,Entering\n", __func__);

	/* size of criteria include name string */
	data_size = all_mut_len + crtra_count;
	while (g_hx_inspt_crtra_name[i] != NULL) {
		if (strlen(g_hx_inspt_crtra_name[i]) > str_max_len)
			str_max_len = strlen(g_hx_inspt_crtra_name[i]);
		i++;
	}

	/* init the array which store original criteria and include
	 *  name string
	 */
	result = kcalloc(data_size, sizeof(char *), GFP_KERNEL);
	if (result != NULL) {
		for (i = 0 ; i < data_size; i++) {
			result[i] = kcalloc(str_max_len, sizeof(char),
				GFP_KERNEL);
			if (result[i] == NULL) {
				E("%s: rst_arr Memory allocation falied!\n",
					__func__);
				err = HX_INSPECT_MEMALLCTFAIL;
				goto rst_arr_mem_alloc_failed;
			}
		}
	} else {
		E("%s: Memory allocation falied!\n", __func__);
		err = HX_INSPECT_MEMALLCTFAIL;
		goto rst_mem_alloc_failed;
	}

	/* dbg */
	if (private_ts->debug_log_level & BIT(4)) {
		I("first 4 bytes 0x%2X,0x%2X,0x%2X,0x%2X !\n",
				str_data[0], str_data[1],
				str_data[2], str_data[3]);
	}

	/* parse value in to result array(1-Dimensional/String) */
	size_of_result_str =
		himax_saperate_comma(str_data, str_size, result, data_size);

	I("%s: now size_of_result_str=%d\n", __func__, size_of_result_str);

	err = hx_get_crtra_by_name(result, size_of_result_str);
	if (err != HX_INSPECT_OK) {
		E("%s:Load criteria from file fail, go end!\n", __func__);
		goto END_FUNC;
	}


END_FUNC:
rst_arr_mem_alloc_failed:
	for (i = 0 ; i < data_size; i++)
		if (result[i] != NULL)
			kfree(result[i]);
	kfree(result);
rst_mem_alloc_failed:

	I("%s,END\n", __func__);
	return err;
	/* parsing Criteria end */
}

static int himax_test_item_parse(char *str_data, int str_size)
{
	int size = str_size;
	char *str_ptr = str_data;
	char *end_ptr = NULL;
	int i = 0;
	int ret = HX_INSPECT_EFILE;

	I("%s,str_data: %p, str_size: %d\n", __func__, str_data, str_size);

	do {
		str_ptr = strnstr(str_ptr, "HIMAX", size);
		end_ptr = strnstr(str_ptr, "\x0d\x0a", size);
		if (str_ptr != NULL && end_ptr != NULL) {
			while (g_himax_inspection_mode[i]) {
				if (strncmp(str_ptr, g_himax_inspection_mode[i],
				end_ptr - str_ptr) == 0) {
					I("%s,Find item : %s\n", __func__,
						g_himax_inspection_mode[i]);
					g_test_item_flag[i] = 1;
					ret = HX_INSPECT_OK;
					break;
				}
				i++;
			}
			size = str_size - (end_ptr - str_data);
			str_ptr = end_ptr++;
			i = 0;
		} else {
			I("%s,Can't find %s or %s\n", __func__,
				"HIMAX", "\x0d\x0a");
			break;
		}
	} while (size > strlen("HIMAX"));

	return ret;
}


static int himax_parse_criteria(const struct firmware *file_entry)
{
	int ret = 0;
	int i = 0;
	int start_str_len = 0;
	char *start_ptr = NULL;
	int start_str_end_len = 0;
	int fix_len = ic_data->HX_RX_NUM * ic_data->HX_TX_NUM * 7;

	while (g_hx_inspt_crtra_name[i] != NULL) {

		start_ptr = strnstr(file_entry->data,
			g_hx_inspt_crtra_name[i], file_entry->size);

		if (start_ptr != NULL) {
			I("g_hx_inspt_crtra_name[%d] = %s\n",
				i, g_hx_inspt_crtra_name[i]);
			start_str_len = strlen(g_hx_inspt_crtra_name[i]);
			start_str_end_len = start_str_len + fix_len;

			ret |= himax_parse_criteria_str(start_ptr,
				start_str_end_len);
		}
		i++;
	}

	return ret;
}


static int himax_parse_test_dir_file(const struct firmware *file_entry)
{
	int start_str_len = 0;
	int str_size = 0;
	char *start_ptr = NULL;
	char *end_ptr = NULL;
	int i = 0;
	int j = 0;
	char str[2][60]; /*[0]->Start string, [1]->End string*/
	char *str_tail[2] = {"_Begin]\x0d\x0a", "_End]\x0d\x0a"};
	int ret = HX_INSPECT_OK;

	while (g_hx_head_str[i]) {
		/*compose header string of .dri file*/
		for (j = 0; j < 2; j++) {
			strlcpy(str[j], "[", sizeof(str[j]));
			strlcat(str[j], g_hx_head_str[i], sizeof(str[j]));
			strlcat(str[j], str_tail[j], sizeof(str[j]));
			/*I("%s string[%d] : %s\n", __func__, j, str[j]);*/
		}

		/*find each group of .dri file*/
		start_str_len = strlen(str[0]);
		start_ptr = strnstr(file_entry->data, str[0], file_entry->size);
		end_ptr = strnstr(file_entry->data, str[1], file_entry->size);
		str_size = end_ptr - start_ptr - start_str_len;
		/*I("%s,String Length = %d\n", __func__, str_size);*/

		if (start_ptr == NULL || end_ptr == NULL)
			E("%s,Can't find string %s\n", __func__,
				g_hx_head_str[i]);
		else {
			/*parse each sub group string*/
			/*if (strncmp(g_hx_head_str[i], "Project_Info",*/
			/*strlen(g_hx_head_str[i])) == 0) {*/
				/* get project informaion - Not Use*/
			/*}*/
			if (strncmp(g_hx_head_str[i], "TestItem",
			strlen(g_hx_head_str[i])) == 0) {
				/*get Test Item*/
				I("%s,Start to parse %s\n", __func__,
					g_hx_head_str[i]);
				ret |= himax_test_item_parse(start_ptr
					+ start_str_len,
					str_size);
			}
			/*if (strncmp(g_hx_head_str[i], "TestCriteria_Weight",*/
			/*strlen(g_hx_head_str[i])) == 0) {*/
				/*get Test Criteria Weight - Not Use*/
			/*}*/
			if (strncmp(g_hx_head_str[i], "TestCriteria",
			strlen(g_hx_head_str[i])) == 0) {
				/*get Test Criteria*/
				I("%s,Start to parse %s\n", __func__,
				g_hx_head_str[i]);
				ret |= himax_parse_criteria(file_entry);
			}
		}
		i++;
	}

	return ret;
}

static void himax_test_item_chk(int csv_test)
{
	int i = 0;

	if (csv_test)
		for (i = 0; i < HX_CRITERIA_ITEM - 1; i++)
			g_test_item_flag[i] = 1;

	g_test_item_flag[HIMAX_OPEN] &=
		(g_inspt_crtra_flag[IDX_OPENMIN] == 1
		&& g_inspt_crtra_flag[IDX_OPENMAX] == 1) ? 1 : 0;

	g_test_item_flag[HIMAX_MICRO_OPEN] &=
		(g_inspt_crtra_flag[IDX_M_OPENMIN] == 1
		&& g_inspt_crtra_flag[IDX_M_OPENMAX] == 1) ? 1 : 0;

	g_test_item_flag[HIMAX_SHORT] &=
		(g_inspt_crtra_flag[IDX_SHORTMIN] == 1
		&& g_inspt_crtra_flag[IDX_SHORTMAX] == 1) ? 1 : 0;

	g_test_item_flag[HIMAX_RAWDATA] &=
		(g_inspt_crtra_flag[IDX_RAWMIN] == 1
		&& g_inspt_crtra_flag[IDX_RAWMAX] == 1) ? 1 : 0;

	g_test_item_flag[HIMAX_BPN_RAWDATA] &=
		(g_inspt_crtra_flag[IDX_BPN_RAWMIN] == 1
		&& g_inspt_crtra_flag[IDX_BPN_RAWMAX] == 1) ? 1 : 0;

	g_test_item_flag[HIMAX_SC] &=
		(g_inspt_crtra_flag[IDX_SCMIN] == 1
		&& g_inspt_crtra_flag[IDX_SCMAX] == 1
		&& g_inspt_crtra_flag[IDX_SC_GOLDEN] == 1) ? 1 : 0;

	g_test_item_flag[HIMAX_WEIGHT_NOISE] &=
		(g_inspt_crtra_flag[IDX_WT_NOISEMIN] == 1
		&& g_inspt_crtra_flag[IDX_WT_NOISEMAX] == 1) ? 1 : 0;

	g_test_item_flag[HIMAX_ABS_NOISE] &=
		(g_inspt_crtra_flag[IDX_ABS_NOISEMIN] == 1
		&& g_inspt_crtra_flag[IDX_ABS_NOISEMAX] == 1) ? 1 : 0;

	g_test_item_flag[HIMAX_SORTING] &=
		(g_inspt_crtra_flag[IDX_SORTMIN] == 1
		&& g_inspt_crtra_flag[IDX_SORTMAX] == 1) ? 1 : 0;

	g_test_item_flag[HIMAX_GAPTEST_RAW] &=
		(g_inspt_crtra_flag[IDX_GAP_HOR_RAWMAX] == 1
		&& g_inspt_crtra_flag[IDX_GAP_HOR_RAWMIN] == 1
		&& g_inspt_crtra_flag[IDX_GAP_VER_RAWMAX] == 1
		&& g_inspt_crtra_flag[IDX_GAP_VER_RAWMIN] == 1) ? 1 : 0;

	g_test_item_flag[HIMAX_ACT_IDLE_RAWDATA] &=
		(g_inspt_crtra_flag[IDX_ACT_IDLE_RAWDATA_MIN] == 1
		&& g_inspt_crtra_flag[IDX_ACT_IDLE_RAWDATA_MAX] == 1) ? 1 : 0;

	g_test_item_flag[HIMAX_ACT_IDLE_BPN_RAWDATA] &=
		(g_inspt_crtra_flag[IDX_ACT_IDLE_RAW_BPN_MIN] == 1
		&& g_inspt_crtra_flag[IDX_ACT_IDLE_RAW_BPN_MAX] == 1) ? 1 : 0;

	g_test_item_flag[HIMAX_ACT_IDLE_NOISE] &=
		(g_inspt_crtra_flag[IDX_ACT_IDLE_NOISE_MIN] == 1
		&& g_inspt_crtra_flag[IDX_ACT_IDLE_NOISE_MAX] == 1) ? 1 : 0;

	g_test_item_flag[HIMAX_LPWUG_RAWDATA] &=
		(g_inspt_crtra_flag[IDX_LPWUG_RAWDATA_MIN] == 1
		&& g_inspt_crtra_flag[IDX_LPWUG_RAWDATA_MAX] == 1) ? 1 : 0;

	g_test_item_flag[HIMAX_LPWUG_BPN_RAWDATA] &=
		(g_inspt_crtra_flag[IDX_LPWUG_RAW_BPN_MIN] == 1
		&& g_inspt_crtra_flag[IDX_LPWUG_RAW_BPN_MAX] == 1) ? 1 : 0;

	g_test_item_flag[HIMAX_LPWUG_WEIGHT_NOISE] &=
		(g_inspt_crtra_flag[IDX_LPWUG_WT_NOISEMAX] == 1
		&& g_inspt_crtra_flag[IDX_LPWUG_WT_NOISEMIN] == 1) ? 1 : 0;

	g_test_item_flag[HIMAX_LPWUG_ABS_NOISE] &=
		(g_inspt_crtra_flag[IDX_LPWUG_NOISE_ABS_MAX] == 1
		&& g_inspt_crtra_flag[IDX_LPWUG_NOISE_ABS_MIN] == 1) ? 1 : 0;

	g_test_item_flag[HIMAX_LPWUG_IDLE_RAWDATA] &=
		(g_inspt_crtra_flag[IDX_LPWUG_IDLE_RAWDATA_MAX] == 1
		&& g_inspt_crtra_flag[IDX_LPWUG_IDLE_RAWDATA_MIN] == 1) ? 1 : 0;

	g_test_item_flag[HIMAX_LPWUG_IDLE_BPN_RAWDATA] &=
		(g_inspt_crtra_flag[IDX_LPWUG_IDLE_RAW_BPN_MIN] == 1
		&& g_inspt_crtra_flag[IDX_LPWUG_IDLE_RAW_BPN_MAX] == 1) ? 1 : 0;

	g_test_item_flag[HIMAX_LPWUG_IDLE_NOISE] &=
		(g_inspt_crtra_flag[IDX_LPWUG_IDLE_NOISE_MAX] == 1
		&& g_inspt_crtra_flag[IDX_LPWUG_IDLE_NOISE_MIN] == 1) ? 1 : 0;

	do_lpwg_test = g_test_item_flag[HIMAX_LPWUG_RAWDATA]
			| g_test_item_flag[HIMAX_LPWUG_BPN_RAWDATA]
			| g_test_item_flag[HIMAX_LPWUG_WEIGHT_NOISE]
			| g_test_item_flag[HIMAX_LPWUG_ABS_NOISE]
			| g_test_item_flag[HIMAX_LPWUG_IDLE_RAWDATA]
			| g_test_item_flag[HIMAX_LPWUG_IDLE_BPN_RAWDATA]
			| g_test_item_flag[HIMAX_LPWUG_IDLE_NOISE];

	for (i = 0; i < HX_CRITERIA_ITEM - 1; i++)
		I("g_test_item_flag[%d] = %d\n", i, g_test_item_flag[i]);
}

int hx_get_size_str_arr(char **input)
{
	int i = 0;
	int result = 0;

	while (input[i] != NULL)
		i++;

	result = i;
	if (private_ts->debug_log_level & BIT(4))
		I("There is %d in [0]=%s\n", result, input[0]);

	return result;
}

static void hx_print_ic_id(void)
{
	uint8_t i;
	int len = 0;
	char *prt_data = NULL;

	prt_data = kzalloc(sizeof(char) * HX_SZ_ICID, GFP_KERNEL);
	if (prt_data == NULL) {
		E("%s: Memory allocation falied!\n", __func__);
		return;
	}

	len += snprintf(prt_data + len, HX_SZ_ICID - len,
			"IC ID : ");
	for (i = 0; i < 13; i++) {
		len += snprintf(prt_data + len, HX_SZ_ICID - len,
				"%02X", ic_data->vendor_ic_id[i]);
	}
	len += snprintf(prt_data + len, HX_SZ_ICID - len,
			"\n");

	memcpy(&g_rslt_data[0], prt_data, len);
	g_rslt_data_len = len;
	I("%s: g_rslt_data_len=%d!\n", __func__, g_rslt_data_len);

	kfree(prt_data);
}

int himax_self_test_data_init(void)
{
	const struct firmware *file_entry = NULL;
	struct himax_ts_data *ts = private_ts;
#ifdef ODM_LQ_EDIT
/* modify begin by zhangchaofan@ODM_LQ@Multimedia.TP, for tp fw path 2019-12-13 */
	char *file_name_1 = "tp/19721/hx_criteria.dri";
	char *file_name_2 = "tp/19721/hx_criteria.csv";
/* modify end by zhangchaofan@ODM_LQ@Multimedia.TP, for tp fw path 2019-12-13 */
#endif /*ODM_LQ_EDIT*/
	int ret = HX_INSPECT_OK;
	int err = 0;
	int i = 0;

	/*
	 * 5: one value will not over than 99999, so get this size of string
	 * 2: get twice size
	 */
	g_1kind_raw_size = 5 * ic_data->HX_RX_NUM * ic_data->HX_TX_NUM * 2;

	/* get test item and its items of criteria*/
	HX_CRITERIA_ITEM = hx_get_size_str_arr(g_himax_inspection_mode);
	HX_CRITERIA_SIZE = hx_get_size_str_arr(g_hx_inspt_crtra_name);
	I("There is %d HX_CRITERIA_ITEM and %d HX_CRITERIA_SIZE\n",
	  HX_CRITERIA_ITEM, HX_CRITERIA_SIZE);

	/* init criteria data*/
	g_inspt_crtra_flag = kcalloc(HX_CRITERIA_SIZE, sizeof(int), GFP_KERNEL);
	g_inspection_criteria = kcalloc(HX_CRITERIA_SIZE,
			sizeof(int *), GFP_KERNEL);
	g_test_item_flag = kcalloc(HX_CRITERIA_ITEM, sizeof(int), GFP_KERNEL);
	if (g_inspt_crtra_flag == NULL
	|| g_inspection_criteria == NULL
	|| g_test_item_flag == NULL) {
		E("%s: %d, Memory allocation falied!\n", __func__, __LINE__);
		return MEM_ALLOC_FAIL;
	}

	for (i = 0; i < HX_CRITERIA_SIZE; i++) {
		g_inspection_criteria[i] = kcalloc((ic_data->HX_TX_NUM
				* ic_data->HX_RX_NUM),
				sizeof(int), GFP_KERNEL);
		if (g_inspection_criteria[i] == NULL) {
			E("%s: %d, Memory allocation falied!\n",
				__func__, __LINE__);
			return MEM_ALLOC_FAIL;
		}
	}

	/* default path is /system/etc/firmware */
	/* request criteria file*/
	err = request_firmware(&file_entry, file_name_1, ts->dev);
	if (err < 0) {
		E("%s,Fail to get %s\n", __func__, file_name_1);
		err = request_firmware(&file_entry, file_name_2, ts->dev);
		if (err < 0) {
			E("%s,Fail to get %s\n", __func__, file_name_2);
			I("No criteria file file");
			ret |= HX_INSPECT_EFILE;
		} else {
			I("%s,Success to get %s\n", __func__, file_name_2);
			/* parsing criteria from file .csv*/
			ret |= himax_parse_criteria(file_entry);
			himax_test_item_chk(true);
			release_firmware(file_entry);
		}
	} else {
		/* parsing test file .dri*/
		I("%s,Success to get %s\n", __func__, file_name_1);
		ret |= himax_parse_test_dir_file(file_entry);
		himax_test_item_chk(false);
		release_firmware(file_entry);
	}

	if (private_ts->debug_log_level & BIT(4)) {
		/* print get criteria string */
		for (i = 0 ; i < HX_CRITERIA_SIZE ; i++) {
			if (g_inspt_crtra_flag[i] != 0)
				I("%s: [%d]There is String=%s\n",
					__func__, i, g_hx_inspt_crtra_name[i]);
		}
	}

	if (g_rslt_data == NULL) {
		g_rslt_data = kcalloc(g_1kind_raw_size, sizeof(char),
			GFP_KERNEL);
		if (g_rslt_data == NULL) {
			E("%s: %d, Memory allocation falied!\n",
				__func__, __LINE__);
			ret =  MEM_ALLOC_FAIL;
		}
	} else {
		memset(g_rslt_data, 0x00, g_1kind_raw_size  * sizeof(char));
	}

	snprintf(g_file_path, (int)(strlen(HX_RSLT_OUT_PATH)
			+ strlen(HX_RSLT_OUT_FILE)+1),
			"%s%s", HX_RSLT_OUT_PATH, HX_RSLT_OUT_FILE);

	file_w_flag = true;
	return ret;
}

void himax_self_test_data_deinit(void)
{
	int i = 0;

	/*dbg*/
	/* for (i = 0; i < HX_CRITERIA_ITEM; i++)
	 *	I("%s:[%d]%d\n", __func__, i, g_inspection_criteria[i]);
	 */
	if (g_inspection_criteria != NULL) {
		for (i = 0; i < HX_CRITERIA_SIZE; i++) {
			if (g_inspection_criteria[i] != NULL)
				kfree(g_inspection_criteria[i]);
		}
		kfree(g_inspection_criteria);
		I("Now it have free the g_inspection_criteria!\n");
	} else {
		I("No Need to free g_inspection_criteria!\n");
	}

	if (g_inspt_crtra_flag != NULL) {
		kfree(g_inspt_crtra_flag);
		g_inspt_crtra_flag = NULL;
	}

	g_rslt_data_len = 0;
}

static int himax_chip_self_test(void)
{
	uint32_t ret = HX_INSPECT_OK;
	uint32_t test_size = ic_data->HX_TX_NUM	* ic_data->HX_RX_NUM
			+ ic_data->HX_TX_NUM + ic_data->HX_RX_NUM;
	int i = 0;
	uint8_t tmp_addr[DATA_LEN_4] = {0x94, 0x72, 0x00, 0x10};
	uint8_t tmp_data[DATA_LEN_4] = {0x01, 0x00, 0x00, 0x00};
#if defined(HX_CODE_OVERLAY) || defined(OPPO_PROC_NODE)
#ifdef ODM_LQ_EDIT
/* modify end by zhangchaofan@ODM_LQ@Multimedia.TP, for tp fw path 2019-12-13 */
	uint8_t normalfw[32] = "tp/19721/Himax_firmware.bin";
	uint8_t mpapfw[32] = "tp/19721/Himax_mpfw.bin";
/* modify end by zhangchaofan@ODM_LQ@Multimedia.TP, for tp fw path 2019-12-13 */
#endif /*ODM_LQ_EDIT*/
#endif
	struct file *raw_file = NULL;
	struct filename *vts_name = NULL;
	mm_segment_t fs;
	loff_t pos = 0;
	uint32_t rslt = HX_INSPECT_OK;

	I("%s:IN\n", __func__);

	private_ts->suspend_resume_done = 0;

#if defined(HX_CODE_OVERLAY) || defined(OPPO_PROC_NODE)
	g_core_fp.fp_0f_op_file_dirly(mpapfw);
	g_core_fp.fp_reload_disable(0);
	g_core_fp.fp_sense_on(0x00);
#endif

	ret = himax_self_test_data_init();

	if (!kp_getname_kernel) {
		E("kp_getname_kernel is NULL, not open file!\n");
		file_w_flag = false;
	} else
		vts_name = kp_getname_kernel(g_file_path);

	if (raw_file == NULL && file_w_flag) {
		raw_file = kp_file_open_name(vts_name,
			O_TRUNC|O_CREAT|O_RDWR, 0660);

		if (IS_ERR(raw_file)) {
			E("%s open file failed = %ld\n",
				__func__, PTR_ERR(raw_file));
			file_w_flag = false;
		}
	}

	fs = get_fs();
	set_fs(get_ds());

	hx_print_ic_id();
	if (file_w_flag) {
		vfs_write(raw_file, g_rslt_data, g_rslt_data_len, &pos);
		pos += g_rslt_data_len;
	}

	/*Do normal test items*/
	for (i = 0; i < HX_CRITERIA_ITEM; i++) {
		if (i < HIMAX_LPWUG_RAWDATA) {
			if (g_test_item_flag[i] == 1) {
				I("%d. %s Start\n", i,
					g_himax_inspection_mode[i]);
				rslt = mpTestFunc(i, test_size);
				if (file_w_flag &&
					((rslt & HX_INSPECT_EGETRAW) == 0) &&
					((rslt & HX_INSPECT_ESWITCHMODE) == 0)) {
					vfs_write(raw_file, g_rslt_data,
					g_rslt_data_len, &pos);
					pos += g_rslt_data_len;
				}
				ret |= rslt;

				I("%d. %s End, ret = %d\n", i,
					g_himax_inspection_mode[i], ret);
			}
		} else {
			break;
		}
	}

	/* Press power key and do LPWUG test items*/
	if (do_lpwg_test && (ret == 0)) {
		black_test_item = i;
		g_core_fp.fp_sense_off(true);

		/*Prepare for LPWUG TEST*/
		#if defined(HX_SMART_WAKEUP)
		if (private_ts->SMWP_enable == 0)
		g_core_fp.fp_set_SMWP_enable(1, false);
	    #endif

		himax_switch_mode_inspection(HIMAX_LPWUG_RAWDATA);
		
		if (raw_file != NULL)
		filp_close(raw_file, NULL);
		set_fs(fs);
	
		if (ret == 0)
			ret = 0xAA;
		I("running status = %d \n", ret);
		I("%s:OUT\n", __func__);
		return ret;		
	}
	if (raw_file != NULL)
		filp_close(raw_file, NULL);
	set_fs(fs);

#if defined(HX_CODE_OVERLAY) || defined(OPPO_PROC_NODE)
	private_ts->in_self_test = 0;
	g_core_fp.fp_0f_op_file_dirly(normalfw);
	hx_turn_on_mp_func(0);
	/* set N frame back to default value 1*/
	g_core_fp.fp_register_write(tmp_addr, 4, tmp_data, 0);
	g_core_fp.fp_reload_disable(0);
	g_core_fp.fp_sense_on(0);
#else
	g_core_fp.fp_sense_off(true);
	hx_turn_on_mp_func(0);
	/*himax_set_N_frame(1, HIMAX_INSPECTION_WEIGHT_NOISE);*/
	/* set N frame back to default value 1*/
	g_core_fp.fp_register_write(tmp_addr, 4, tmp_data, 0);
#if !defined(HX_ZERO_FLASH)
	if (g_core_fp.fp_reload_disable != NULL)
		g_core_fp.fp_reload_disable(0);
#endif
	g_core_fp.fp_sense_on(0);
#endif

	himax_self_test_data_deinit();


	I("running status = %X\n", ret);

	if (ret == 0)
			ret = 0xAA;

	I("%s:OUT\n", __func__);
	return ret;
}

#if defined(OPPO_PROC_NODE)
#if defined(HX_SMART_WAKEUP)
int himax_black_chip_self_test(void)
{
	uint32_t ret = HX_INSPECT_OK;
	uint32_t test_size = ic_data->HX_TX_NUM	* ic_data->HX_RX_NUM
			+ ic_data->HX_TX_NUM + ic_data->HX_RX_NUM;
	int i = black_test_item;
	uint8_t tmp_addr[DATA_LEN_4] = {0x94, 0x72, 0x00, 0x10};
	uint8_t tmp_data[DATA_LEN_4] = {0x01, 0x00, 0x00, 0x00};
#if defined(HX_CODE_OVERLAY) || defined(OPPO_PROC_NODE)
#ifdef ODM_LQ_EDIT
/* modify begin by zhangchaofan@ODM_LQ@Multimedia.TP, for tp fw path 2019-12-13 */
	uint8_t normalfw[32] = "tp/19721/Himax_firmware.bin";
/* modify end by zhangchaofan@ODM_LQ@Multimedia.TP, for tp fw path 2019-12-13 */
#endif /*ODM_LQ_EDIT*/
	//uint8_t mpapfw[32] = "Himax_mpfw.bin";
#endif
	struct file *raw_file = NULL;
	struct filename *vts_name = NULL;
	mm_segment_t fs;
	loff_t pos = 0;
	uint32_t rslt = HX_INSPECT_OK;
	
	I("%s:Black IN\n", __func__);
	
	private_ts->suspend_resume_done = 0;
	
	snprintf(g_file_path, (int)(strlen(HX_RSLT_OUT_PATH)
			+ strlen(HX_BLACK_RSLT_OUT_FILE)+1),
			"%s%s", HX_RSLT_OUT_PATH, HX_BLACK_RSLT_OUT_FILE);

	file_w_flag = true;


	//ret = himax_self_test_data_init();

	if (!kp_getname_kernel) {
		E("kp_getname_kernel is NULL, not open file!\n");
		file_w_flag = false;
	} else
		vts_name = kp_getname_kernel(g_file_path);

	if (raw_file == NULL && file_w_flag) {
		raw_file = kp_file_open_name(vts_name,
			O_TRUNC|O_CREAT|O_RDWR, 0660);

		if (IS_ERR(raw_file)) {
			E("%s open file failed = %ld\n",
				__func__, PTR_ERR(raw_file));
			file_w_flag = false;
		}
	}

	fs = get_fs();
	set_fs(get_ds());

	hx_print_ic_id();
	if (file_w_flag) {
		vfs_write(raw_file, g_rslt_data, g_rslt_data_len, &pos);
		pos += g_rslt_data_len;
	}
	
			for (; i < HX_CRITERIA_ITEM; i++) {
			if (g_test_item_flag[i] == 1) {
				I("%d.%s Start\n", i,
					g_himax_inspection_mode[i]);
				rslt = mpTestFunc(i, test_size);
				if (file_w_flag &&
					((rslt & HX_INSPECT_EGETRAW) == 0) &&
					((rslt & HX_INSPECT_ESWITCHMODE) == 0)) {
					vfs_write(raw_file, g_rslt_data,
						g_rslt_data_len, &pos);
					pos += g_rslt_data_len;
				}
				ret |= rslt;

				I("%d.%s End\n", i, g_himax_inspection_mode[i]);
			}
		}
		
		if (raw_file != NULL)
		filp_close(raw_file, NULL);
		set_fs(fs);

	private_ts->in_baseline_test = 0;
#if defined(HX_CODE_OVERLAY) || defined(OPPO_PROC_NODE)
	private_ts->in_self_test = 0;
	g_core_fp.fp_0f_op_file_dirly(normalfw);
	hx_turn_on_mp_func(0);
	/* set N frame back to default value 1*/
	g_core_fp.fp_register_write(tmp_addr, 4, tmp_data, 0);
	g_core_fp.fp_reload_disable(0);
	g_core_fp.fp_sense_on(0);
#else
	g_core_fp.fp_sense_off(true);
	hx_turn_on_mp_func(0);
	/*himax_set_N_frame(1, HIMAX_INSPECTION_WEIGHT_NOISE);*/
	/* set N frame back to default value 1*/
	g_core_fp.fp_register_write(tmp_addr, 4, tmp_data, 0);
#if !defined(HX_ZERO_FLASH)
	if (g_core_fp.fp_reload_disable != NULL)
		g_core_fp.fp_reload_disable(0);
#endif
	g_core_fp.fp_sense_on(0);
#endif

	if (ret == 0)
		ret = 0xAA;
	
	himax_self_test_data_deinit();

	I("%s:OUT\n", __func__);
	return ret;
}
#endif
#endif
void himax_inspect_data_clear(void)
{
	if (!g_rslt_data) {
		kfree(g_rslt_data);
		g_rslt_data = NULL;
	}
}

void himax_inspection_init(void)
{
	I("%s: enter, %d\n", __func__, __LINE__);

	g_core_fp.fp_chip_self_test = himax_chip_self_test;
}
