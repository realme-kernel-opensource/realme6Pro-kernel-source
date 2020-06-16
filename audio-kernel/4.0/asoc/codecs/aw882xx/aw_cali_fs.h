#ifndef __AWINIC_CALI_FS_H__
#define __AWINIC_CALI_FS_H__

#define AWINIC_ADSP_ENABLE
#define AW_CALI_STORE_EXAMPLE

//chengong@ODM_LQ@Multimedia.Audio,2019/12/03,add for defult cali value
#define DEFAULT_CALI_VALUE (6)
#define AFE_PARAM_ID_AWDSP_RX_SET_ENABLE        (0x10013D11)
#define AFE_PARAM_ID_AWDSP_RX_PARAMS            (0x10013D12)
#define AFE_PARAM_ID_AWDSP_TX_SET_ENABLE        (0x10013D13)
#define AFE_PARAM_ID_AWDSP_RX_VMAX_L            (0X10013D17)
#define AFE_PARAM_ID_AWDSP_RX_VMAX_R            (0X10013D18)
#define AFE_PARAM_ID_AWDSP_RX_CALI_CFG_L        (0X10013D19)
#define AFE_PARAM_ID_AWDSP_RX_CALI_CFG_R        (0x10013d1A)
#define AFE_PARAM_ID_AWDSP_RX_RE_L              (0x10013d1B)
#define AFE_PARAM_ID_AWDSP_RX_RE_R              (0X10013D1C)
#define AFE_PARAM_ID_AWDSP_RX_NOISE_L           (0X10013D1D)
#define AFE_PARAM_ID_AWDSP_RX_NOISE_R           (0X10013D1E)
#define AFE_PARAM_ID_AWDSP_RX_F0_L              (0X10013D1F)
#define AFE_PARAM_ID_AWDSP_RX_F0_R              (0X10013D20)
#define AFE_PARAM_ID_AWDSP_RX_REAL_DATA_L       (0X10013D21)
#define AFE_PARAM_ID_AWDSP_RX_REAL_DATA_R       (0X10013D22)

/*unit mOhms*/
static int R0_MAX = 7800;
static int R0_MIN = 5000;

#define AW882XX_CALI_CFG_NUM 3
#define AW882XX_CALI_DATA_NUM 6
#define AW882XX_PARAMS_NUM 400
struct cali_cfg{
	int32_t data[AW882XX_CALI_CFG_NUM];
};
struct cali_data{
	int32_t data[AW882XX_CALI_DATA_NUM];
};
struct params_data{
	int32_t data[AW882XX_PARAMS_NUM];
};

#define AW882XX_IOCTL_MAGIC                'a'
#define AW882XX_IOCTL_SET_CALI_CFG         _IOWR(AW882XX_IOCTL_MAGIC, 1, struct cali_cfg)
#define AW882XX_IOCTL_GET_CALI_CFG         _IOWR(AW882XX_IOCTL_MAGIC, 2, struct cali_cfg)
#define AW882XX_IOCTL_GET_CALI_DATA        _IOWR(AW882XX_IOCTL_MAGIC, 3, struct cali_data)
#define AW882XX_IOCTL_SET_NOISE            _IOWR(AW882XX_IOCTL_MAGIC, 4, int32_t)
#define AW882XX_IOCTL_GET_F0               _IOWR(AW882XX_IOCTL_MAGIC, 5, int32_t)
#define AW882XX_IOCTL_SET_CALI_RE          _IOWR(AW882XX_IOCTL_MAGIC, 6, int32_t)
#define AW882XX_IOCTL_GET_CALI_RE          _IOWR(AW882XX_IOCTL_MAGIC, 7, int32_t)
#define AW882XX_IOCTL_SET_VMAX             _IOWR(AW882XX_IOCTL_MAGIC, 8, int32_t)
#define AW882XX_IOCTL_GET_VMAX             _IOWR(AW882XX_IOCTL_MAGIC, 9, int32_t)
#define AW882XX_IOCTL_SET_PARAM            _IOWR(AW882XX_IOCTL_MAGIC, 10, struct params_data)
#define AW882XX_IOCTL_ENABLE_CALI          _IOWR(AW882XX_IOCTL_MAGIC, 11, int8_t)


int aw882xx_get_cali_re_from_nvram(int32_t *cali_re);
int aw882xx_set_cali_re_to_nvram(int32_t cali_re);

void init_aw882xx_misc_driver(void);
void remove_aw882xx_misc_driver(void);
void aw_debugfs_init(void);

#ifdef AWINIC_ADSP_ENABLE
extern int aw_send_afe_cal_apr(uint32_t param_id, void *buf, int cmd_size, bool write);
extern int aw_send_afe_rx_module_enable(void *buf, int size);
extern int aw_send_afe_tx_module_enable(void *buf, int size);
#else
static int aw_send_afe_cal_apr(uint32_t param_id, void *buf, int cmd_size, bool write)
{
	return 0;
}
int aw_send_afe_rx_module_enable(void *buf, int size)
{
	return 0;
}
int aw_send_afe_tx_module_enable(void *buf, int size)
{
	return 0;
}
#endif

#endif
