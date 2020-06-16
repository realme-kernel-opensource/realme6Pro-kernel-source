#ifndef _AW882XX_H_
#define _AW882XX_H_

/*#define AW_DEBUG*/

/*
 * i2c transaction on Linux limited to 64k
 * (See Linux kernel documentation: Documentation/i2c/writing-clients)
*/
#define MAX_I2C_BUFFER_SIZE					65536

#define AW882XX_FLAG_START_ON_MUTE			(1 << 0)
#define AW882XX_FLAG_SKIP_INTERRUPTS		(1 << 1)
#define AW882XX_FLAG_SAAM_AVAILABLE			(1 << 2)
#define AW882XX_FLAG_STEREO_DEVICE			(1 << 3)
#define AW882XX_FLAG_MULTI_MIC_INPUTS		(1 << 4)

#define AW882XX_NUM_RATES					9
#define AW882XX_SYSST_CHECK_MAX				10
#define AW882XX_MODE_SHIFT_MAX				2


enum aw882xx_init {
	AW882XX_INIT_ST = 0,
	AW882XX_INIT_OK = 1,
	AW882XX_INIT_NG = 2,
};

enum aw882xx_chipid {
	AW882XX_ID = 0x1852,
};

enum aw882xx_modeshift {
	AW882XX_MODE_SPK_SHIFT = 0,
	AW882XX_MODE_RCV_SHIFT = 1,
};

enum aw882xx_mode_spk_rcv {
	AW882XX_SPEAKER_MODE = 0,
	AW882XX_RECEIVER_MODE = 1,
};
/*smartpa monitor*/
enum aw882xx_ipeak {
	IPEAK_3P50_A = 0x08,
	IPEAK_3P00_A = 0x06,
	IPEAK_2P75_A = 0x05,
	IPEAK_2P50_A = 0x04,
	IPEAK_NONE   = 0xFF,
};

enum aw882xx_gain {
	GAIN_NEG_0P0_DB = 0x00,
	GAIN_NEG_0P5_DB = 0x01,
	GAIN_NEG_1P0_DB = 0x02,
	GAIN_NEG_1P5_DB = 0x03,
	GAIN_NEG_3P0_DB = 0x06,
	GAIN_NEG_4P5_DB = 0x09,
	GAIN_NEG_6P0_DB = 0x10,
	GAIN_NONE       = 0xFF,
};

enum aw882xx_vmax_percentage {
	VMAX_100_PERCENTAGE  = 0x00000000,
	VMAX_086_PERCENTAGE  = 0xFFED714D,
	VMAX_075_PERCENTAGE  = 0xFFD80505,
	VMAX_063_PERCENTAGE  = 0xFFBEAE7E,
	VMAX_NONE            = 0xFFFFFFFF,
};

#define AW882XX_MONITOR_DEFAULT_FLAG 0
#define AW882XX_MONITOR_DEFAULT_TIMER_VAL 30000
#define AW882XX_MONITOR_VBAT_RANGE 6025
#define AW882XX_MONITOR_INT_10BIT 1023
#define AW882XX_MONITOR_TEMP_SIGN_MASK (1<<9)
#define AW882XX_MONITOR_TEMP_NEG_MASK (0XFC00)
#define AW882XX_BIT_SYSCTRL2_BST_IPEAK_MASK ( 15<< 0)
#define AW882XX_BIT_HAGCCFG4_GAIN_SHIFT (8)
#define AW882XX_BIT_HAGCCFG4_GAIN_MASK (0x00ff)

struct aw882xx_low_vol {
	uint32_t vol;
	uint8_t ipeak;
	uint8_t gain;
};

struct aw882xx_low_temp {
	int16_t temp;
	uint8_t ipeak;
	uint8_t gain;
	uint32_t vmax;
};

struct aw882xx_monitor{
	struct hrtimer timer;
	uint32_t timer_val;
	struct work_struct work;
	uint32_t is_enable;
	uint16_t pre_vol;
	int16_t pre_temp;
#ifdef AW_DEBUG
	uint16_t test_vol;
	int16_t test_temp;
#endif
};

struct aw882xx {
	struct regmap *regmap;
	struct i2c_client *i2c;
	struct snd_soc_codec *codec;
	struct device *dev;
	struct mutex lock;

	struct aw882xx_monitor monitor;
	int sysclk;
	int rate;
	int pstream;
	int cstream;

	int reset_gpio;
	int irq_gpio;

	unsigned char reg_addr;

	unsigned int flags;
	unsigned int chipid;
	unsigned int init;
	unsigned int spk_rcv_mode;
	int32_t cali_re;
	unsigned int cfg_num;
};

struct aw882xx_container {
	int len;
	unsigned char data[];
};

int aw882xx_set_cali_re(struct aw882xx *aw882xx, int32_t cali_re);

#endif
