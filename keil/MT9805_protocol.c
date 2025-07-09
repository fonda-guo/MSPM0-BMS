#include"MT9805_protocol.h"

#define TEST_ALL_MOD	1

#define QUERY_TABLE_LEN 166

#define TABLE_SORT_TYPE_BACKWARD     1
#define TABLE_SORT_TYPE_FORWARD      0


const uint16_t NTC_TABLE[QUERY_TABLE_LEN] =
{2098 ,2104 ,2110 ,2117 ,2124 ,2131 ,2139 ,2146 ,2155 ,
2163 ,2172 ,2182 ,2192 ,2202 ,2213 ,2224 ,2236 ,2248 ,
2261 ,2274 ,2288 ,2303 ,2318 ,2334 ,2350 ,2367 ,2385 ,
2404 ,2424 ,2444 ,2465 ,2487 ,2510 ,2534 ,2559 ,2584 ,
2611 ,2639 ,2668 ,2698 ,2727 ,2761 ,2795 ,2830 ,2866 ,
2903 ,2942 ,2982 ,3024 ,3067 ,3110 ,3158 ,3206 ,3256 ,
3307 ,3360 ,3415 ,3472 ,3531 ,3591 ,3654 ,3719 ,3786 ,
3855 ,3926 ,4000 ,4076 ,4155 ,4236 ,4319 ,4406 ,4494 ,
4586 ,4681 ,4778 ,4879 ,4982 ,5088 ,5198 ,5311 ,5428 ,
5547 ,5670 ,5797 ,5929 ,6063 ,6201 ,6343 ,6489 ,6639 ,
6798 ,6953 ,7116 ,7284 ,7457 ,7634 ,7816 ,8002 ,8194 ,
8392 ,8594 ,8800 ,9015 ,9233 ,9457 ,9686 ,9924 ,10167 ,
10414 ,10669 ,10929 ,11195 ,11470 ,11751 ,12040 ,12336 ,
12638 ,12947 ,13261 ,13587 ,13919 ,14255 ,14602 ,14962 ,
15324 ,15689 ,16075 ,16461 ,16859 ,17267 ,17686 ,18103 ,
18543 ,18978 ,19437 ,19905 ,20365 ,20850 ,21342 ,21841 ,
22346 ,22877 ,23413 ,23954 ,24497 ,25068 ,25641 ,26242 ,
26845 ,27445 ,28076 ,28702 ,29322 ,30011 ,30653 ,31326 ,
32030 ,32722 ,33447 ,34154 ,34895,35670,36423,37211,37971,38765};


typedef enum{
    ADREG_A_Group = 0,
    ADREG_B_Group,
    ADREG_C_Group,
    ADREG_D_Group,
    ADREG_E_Group,
    ADREG_F_Group,
	ADREG_ALL_Group
}MT9805_AdcValGroup;

typedef enum{
    AUXREG_A_Group = 0,
    AUXREG_B_Group,
    AUXREG_C_Group,
    AUXREG_D_Group,
	AUXREG_ALL_Group
}MT9805_AuxGroup;

typedef enum{
    STAREG_A_Group = 0,
    STAREG_B_Group,
    STAREG_ALL_Group
}MT9805_StatGroup;

typedef enum{
    CFGREG_A_Group = 0,
    CFGREG_B_Group
}MT9805_ConfgGroup;

typedef enum{
    ADCOPT_MD_27KHZ = 0, 
    ADCOPT_MD_14KHZ ,
    ADCOPT_MD_7KHZ ,
    ADCOPT_MD_3KHZ ,
    ADCOPT_MD_2KHZ ,
    ADCOPT_MD_422HZ ,
    ADCOPT_MD_1KHZ ,
    ADCOPT_MD_26HZ
}MT9805_ADCOPT_MD;

const uint16_t MT9805_CRC15_POLY = 0x4599;
const uint16_t PEC_Table[256] = {0x0,0xc599,0xceab,0xb32,0xd8cf,0x1d56,0x1664,
0xd3fd,0xf407,0x319e,0x3aac,0xff35,0x2cc8,0xe951,
0xe263,0x27fa,0xad97,0x680e,0x633c,0xa6a5,0x7558,
0xb0c1,0xbbf3,0x7e6a,0x5990,0x9c09,0x973b,0x52a2,
0x815f,0x44c6,0x4ff4,0x8a6d,0x5b2e,0x9eb7,0x9585,
0x501c,0x83e1,0x4678,0x4d4a,0x88d3,0xaf29,0x6ab0,
0x6182,0xa41b,0x77e6,0xb27f,0xb94d,0x7cd4,0xf6b9,
0x3320,0x3812,0xfd8b,0x2e76,0xebef,0xe0dd,0x2544,
0x2be,0xc727,0xcc15,0x98c,0xda71,0x1fe8,0x14da,
0xd143,0xf3c5,0x365c,0x3d6e,0xf8f7,0x2b0a,0xee93,
0xe5a1,0x2038,0x7c2,0xc25b,0xc969,0xcf0,0xdf0d,0x1a94,
0x11a6,0xd43f,0x5e52,0x9bcb,0x90f9,0x5560,0x869d,0x4304,
0x4836,0x8daf,0xaa55,0x6fcc,0x64fe,0xa167,0x729a,0xb703,
0xbc31,0x79a8,0xa8eb,0x6d72,0x6640,0xa3d9,0x7024,0xb5bd,
0xbe8f,0x7b16,0x5cec,0x9975,0x9247,0x57de,0x8423,0x41ba,
0x4a88,0x8f11,0x57c,0xc0e5,0xcbd7,0xe4e,0xddb3,0x182a,
0x1318,0xd681,0xf17b,0x34e2,0x3fd0,0xfa49,0x29b4,0xec2d,
0xe71f,0x2286,0xa213,0x678a,0x6cb8,0xa921,0x7adc,0xbf45,
0xb477,0x71ee,0x5614,0x938d,0x98bf,0x5d26,0x8edb,0x4b42,
0x4070,0x85e9,0xf84,0xca1d,0xc12f,0x4b6,0xd74b,0x12d2,
0x19e0,0xdc79,0xfb83,0x3e1a,0x3528,0xf0b1,0x234c,0xe6d5,
0xede7,0x287e,0xf93d,0x3ca4,0x3796,0xf20f,0x21f2,0xe46b,
0xef59,0x2ac0,0xd3a,0xc8a3,0xc391,0x608,0xd5f5,0x106c,
0x1b5e,0xdec7,0x54aa,0x9133,0x9a01,0x5f98,0x8c65,0x49fc,
0x42ce,0x8757,0xa0ad,0x6534,0x6e06,0xab9f,0x7862,0xbdfb,
0xb6c9,0x7350,0x51d6,0x944f,0x9f7d,0x5ae4,0x8919,0x4c80,
0x47b2,0x822b,0xa5d1,0x6048,0x6b7a,0xaee3,0x7d1e,0xb887,
0xb3b5,0x762c,0xfc41,0x39d8,0x32ea,0xf773,0x248e,0xe117,
0xea25,0x2fbc,0x846,0xcddf,0xc6ed,0x374,0xd089,0x1510,
0x1e22,0xdbbb,0xaf8,0xcf61,0xc453,0x1ca,0xd237,0x17ae,
0x1c9c,0xd905,0xfeff,0x3b66,0x3054,0xf5cd,0x2630,0xe3a9,
0xe89b,0x2d02,0xa76f,0x62f6,0x69c4,0xac5d,0x7fa0,0xba39,
0xb10b,0x7492,0x5368,0x96f1,0x9dc3,0x585a,0x8ba7,0x4e3e,
0x450c,0x8095};

#define STA_REG_COV_MASK(x)			(0x02<<((x%4)*2))
#define STA_REG_CUV_MASK(x)			(0x01<<((x%4)*2))



#define STA_REGB_REV(x)						((x>>4)&0x0f)
#define STA_REGB_RSVD(x)					((x>>2)&0x03)
#define STA_REGB_MUXFAIL(x)					((x>>1)&0x01)
#define STA_REGB_THSD(x)					((x>>0)&0x01)

#define MT9805_GPIO_1						0
#define MT9805_GPIO_2						1
#define MT9805_GPIO_3						2
#define MT9805_GPIO_4						3
#define MT9805_GPIO_5						4
#define MT9805_GPIO_6						5
#define MT9805_GPIO_7						6
#define MT9805_GPIO_8						7
#define MT9805_GPIO_9						8

/*CFG_REG*/
#define MT9805_CFG_REG_ADCOPT_MASK	  0x01
#define MT9805_CFG_REG_SWTRD_MASK			0x02
#define MT9805_CFG_REG_REFON_MASK			0x04
#define MT9805_CFG_REG_GPIO1_MASK			0x08
#define MT9805_CFG_REG_GPIO2_MASK			0x10
#define MT9805_CFG_REG_GPIO3_MASK			0x20
#define MT9805_CFG_REG_GPIO4_MASK			0x40
#define MT9805_CFG_REG_GPIO5_MASK			0x80

/*?COMM???*/
#define MT9805_WRCOMM_H			0X07
#define MT9805_WRCOMM_L			0X21
#define MT9805_WRCOMM_PEC_H		0x24
#define MT9805_WRCOMM_PEC_L		0xb2

/*?COMM???*/
#define MT9805_RDCOMM_H			0X07
#define MT9805_RDCOMM_L			0X22
#define MT9805_RDCOMM_PEC_H		0x32
#define MT9805_RDCOMM_PEC_L		0xd6


#define MT9805_ICOM_START		0x60
#define MT9805_ICOM_STOP		0x10
#define MT9805_ICOM_BLANK		0x00

#define MT9805_FCOM_NO_TRANS	0x07
#define MT9805_FCOM_ACK			0x00
#define MT9805_FCOM_NACK		0x08
#define MT9805_FCOM_NACK_STOP	0x09

#define MT9805_CLRAUX_H			0x07
#define MT9805_CLRAUX_L			0x12

/* CMD ???????? */
#define CMD_BITS_MD_422_1K		(0x0000<<7)
#define CMD_BITS_MD_27K_14K		(0x0001<<7)
#define CMD_BITS_MD_7K_3K		(0x0002<<7)
#define CMD_BITS_MD_26_2K		(0x0003<<7)

#define CMD_BITS_DCP_0	0x00
#define CMD_BITS_DCP_1	0x80

#define CMD_BITS_CH_ALL			0x00
#define CMD_BITS_CH_1_7_13		0x01
#define CMD_BITS_CH_2_8_14		0x02
#define CMD_BITS_CH_3_9_15		0x03
#define CMD_BITS_CH_4_10_16		0x04
#define CMD_BITS_CH_5_11_17		0x05
#define CMD_BITS_CH_6_12_18		0x06

#define CMD_BITS_CHG_1to9_2ndREF	0x00
#define CMD_BITS_CHG_1_6			0x01
#define CMD_BITS_CHG_2_7			0x02
#define CMD_BITS_CHG_3_8			0x03
#define CMD_BITS_CHG_4_9			0x04
#define CMD_BITS_CHG_5				0x05
#define CMD_BITS_CHG_2ndREF			0x06

#define CMD_BITS_CHST_SC_ITMP_VA_VD	0x00
#define CMD_BITS_CHST_SC			0x01
#define CMD_BITS_CHST_ITMP			0x02
#define CMD_BITS_CHST_VA			0x03
#define CMD_BITS_CHST_VD			0x04

#define CMD_BITS_PUP_0	(0x00<<6)
#define CMD_BITS_PUP_1	(0x01<<6)

#define CMD_BITS_ST_1	(0x01<<5)
#define CMD_BITS_ST_2	(0x02<<5)

#define CMD_BITS_CHG_0	0x00
#define CMD_BITS_CHG_1	0x01
#define CMD_BITS_CHG_2	0x02
#define CMD_BITS_CHG_3	0x03
#define CMD_BITS_CHG_4	0x04
#define CMD_BITS_CHG_5	0x05
#define CMD_BITS_CHG_6	0x06

#define CMD_BITS_CHST_0	0x00
#define CMD_BITS_CHST_1	0x01
#define CMD_BITS_CHST_2	0x02
#define CMD_BITS_CHST_3	0x03
#define CMD_BITS_CHST_4	0x04
#define CMD_BITS_CHST_5	0x05
#define CMD_BITS_CHST_6	0x06

static uint8_t WCFG_A_buf[4 + MAX_MT9805_CHIP_SIZE*8];	/* ??? */
static uint8_t WCFG_B_buf[4 + MAX_MT9805_CHIP_SIZE*8];	/* ??? */
static uint8_t RCFG_A_buf[4 + MAX_MT9805_CHIP_SIZE*8];	/* ??? */
static uint8_t RCFG_B_buf[4 + MAX_MT9805_CHIP_SIZE*8];	/* ??? */
static uint8_t WCOMM_buf[4 + MAX_MT9805_CHIP_SIZE*8];	/* ?COMM */
static uint8_t RCOMM_buf[4 + MAX_MT9805_CHIP_SIZE*8];	/* ?COMM */
static uint8_t RD_X_buf[4 + MAX_MT9805_CHIP_SIZE*8];	/* ??? */
MT9805DataPackageTypeDef MT9805DataBase[MAX_MT9805_CHIP_SIZE];

const uint16_t ADCOPT_MD_MAPPER[8][2] = {
										{0,CMD_BITS_MD_27K_14K},
										{1,CMD_BITS_MD_27K_14K},
										{0,CMD_BITS_MD_7K_3K},
										{1,CMD_BITS_MD_7K_3K},
										{1,CMD_BITS_MD_26_2K},
										{1,CMD_BITS_MD_422_1K},
										{0,CMD_BITS_MD_422_1K},
										{0,CMD_BITS_MD_26_2K},//?maybe wrong needed to be consulted by FAE
										};
const char MD_26HzStr[] = "26HZ";
const char MD_422HzStr[] = "422HZ";
const char MD_1KHzStr[] = "1kHZ";
const char MD_2KHzStr[] = "2kHZ";
const char MD_3KHzStr[] = "3kHZ";
const char MD_7KHzStr[] = "7kHZ";
const char MD_14KHzStr[] = "14kHZ";
const char MD_27HzStr[] = "27kHZ";

const uint8_t ST_Sel[2]  = {CMD_BITS_ST_1,CMD_BITS_ST_2};
const uint8_t DCP_Sel[2] = {CMD_BITS_DCP_0,CMD_BITS_DCP_1};
static char* pMD_XXHzStr[8];
uint16_t GPIO_Vol_test[8];

enum  MT9805_WorkMode List_work;

void updata_MT9805_data(void);
static void STCOMM_IIC_SPI(uint8_t chipSize);
static void M9805_CmdOption (MT9805_CmdType type , uint16_t MD,uint8_t PUP,uint8_t St,uint8_t DCP,uint8_t CHx);
static void Write9805_CFGRA(uint8_t chipSize);
static void Write9805_CFGRB(uint8_t chipSize);
static uint8_t Compare_COMM(uint8_t chipSize);
static uint8_t Compare_CFRG_A(uint8_t chipSize);
static uint8_t Compare_CFRG_B(uint8_t chipSize);
static void Write_COMM(uint8_t chipSize);
static void Read_COMM(uint8_t chipSize);
static void Read_CFGRA(uint8_t chipSize);
static void Read_CFGRB(uint8_t chipSize);
static void Read_VolReg(uint8_t chipSize,MT9805_AdcValGroup group);
static void Read_AuxReg(uint8_t chipSize,MT9805_AuxGroup group);
static void Read_StaReg(uint8_t chipSize,MT9805_StatGroup group);
static void Read_SctrlReg(uint8_t chipSize,uint8_t *pRegsData);
static void Read_PwmReg(uint8_t chipSize,uint8_t *pRegsData);
static void Read_PDOL(uint8_t chipSize,MT9805_AdcValGroup group);
static void Start_Sctrl_Poll(uint8_t chipSize,uint8_t *pRegsData);

static void Print9805_CFGR(uint8_t chipSize,MT9805_ConfgGroup group);
static void Print9805_COMM(uint8_t chipSize);
static void Print9805_VoltReg(uint8_t chipSize,MT9805_AdcValGroup group);
static void Print9805_AuxReg(uint8_t chipSize,MT9805_AuxGroup group);
static void Print9805_StaReg(uint8_t chipSize,MT9805_StatGroup group);
static void Print9805_CVST(uint8_t chipSize,MT9805_AdcValGroup group);
static void Print9805_PDOL(uint8_t chipSize,MT9805_AdcValGroup group);
static void Print9805_IIC_COMM(uint8_t chipSize);
static void Print9805_nctTemp(uint8_t chipSize);

static void MT9805_Awake(uint8_t chipSize);
static void MT9805_SetDCC(MT9805DataPackageTypeDef *pMT9805DataPackage, uint8_t sw);
static void MT9805_SetGPIO(MT9805DataPackageTypeDef *pMT9805DataPackage, int8_t gpio,uint8_t val);
static void MT9805_SetADCOPT(uint8_t chipsize,uint8_t val);
static void MT9805_SetVUV(uint8_t chipsize,uint32_t val);	//unit:mv
static void MT9805_SetVOV(uint8_t chipsize,uint32_t val);	//unit:mv
static void MT9805_SetDCTO(uint8_t chipsize,uint8_t val);	
static void MT9805_SetFDRF(uint8_t chipsize,uint8_t val);	
static void MT9805_SetREFON(uint8_t chipsize,uint8_t val);	
static void MT9805_SetPS(uint8_t chipsize,uint8_t val);	
static void MT9805_SetDTMEN(uint8_t chipsize,uint8_t val);	
static void MT9805_PollADC(uint8_t chipsize,uint8_t *pPollRlt);
static void MT9805_OWcheck(uint8_t chipSize);
static uint16_t M9805_pec_calc (uint8_t *data , uint8_t len);


static void spi_cmd_2(uint8_t chipsize);
static void spi_cmd_1(uint8_t chipsize);
static void Spi_Cmd(uint8_t CMD1,uint8_t CMD2);
static void M24C16_WriteByte(uint8_t chipSize,uint16_t dataAddr,uint8_t byte);
static void M24C16_ReadByte(uint8_t chipSize,uint16_t dataAddr,uint8_t *pRlt);
static void M24C16_ReadBytes_TEST(uint8_t chipSize,uint16_t dataAddr,uint8_t *pByte);
static uint8_t VoltToTemp(uint8_t *pTemp,uint16_t adcVal,const uint16_t *pTable,uint8_t tableSize,uint8_t preTempVal,uint8_t sortType);

uint8_t CMD_BYTE2 = 0;
uint8_t CMD_BYTE1 = 0;
uint8_t ad_pCMD[2] = {0};
static uint8_t isoChipSize = 1;
uint16_t test = 0;
static const uint8_t WRCFGA_CMD[4]		= {0x00,0x01,0x3D,0x6E};
static const uint8_t WRCFGB_CMD[4]		= {0x00,0x24,0xB1,0x9E};
static const uint8_t RDCFGA_CMD[4]		= {0x00,0x02,0x2B,0x0A};
static const uint8_t RDCFGB_CMD[4]		= {0x00,0x26,0x2C,0xC8};
static const uint8_t RDCVA_CMD[4]		= {0x00,0x04,0x07,0xC2};
static const uint8_t RDCVB_CMD[4]		= {0x00,0x06,0x9A,0x94};
static const uint8_t RDCVC_CMD[4]		= {0x00,0x08,0x5E,0x52};
static const uint8_t RDCVD_CMD[4]		= {0x00,0x0A,0xC3,0x04};
static const uint8_t RDCVE_CMD[4]		= {0x00,0x09,0xD5,0x60};
static const uint8_t RDCVF_CMD[4]		= {0x00,0x0B,0x48,0x36};
static const uint8_t RDAUXA_CMD[4]		= {0x00,0x0C,0xEF,0xCC};
static const uint8_t RDAUXB_CMD[4]		= {0x00,0x0E,0x72,0x9A};
static const uint8_t RDAUXC_CMD[4]		= {0x00,0x0D,0x64,0xFE};
static const uint8_t RDAUXD_CMD[4]		= {0x00,0x0F,0xF9,0xA8};
static const uint8_t RDSTATA_CMD[4]		= {0x00,0x10,0xED,0x72};
static const uint8_t RDSTATB_CMD[4]		= {0x00,0x12,0x70,0x24};
static const uint8_t RDSCTRL_CMD[4]		= {0x00,0x16,0xC1,0xBA};
static const uint8_t RDPWM_CMD[4]		= {0x00,0x22,0x9D,0x56};
static const uint8_t STSCTRL_CMD[4]		= {0x00,0x19,0x8E,0x4E};

static const uint8_t CLRCELL_CMD[4]		= {0x07,0x11,0xC9,0xC0};
static const uint8_t CLRAUX_CMD[4]		= {0x07,0x12,0xDF,0xA4};
static const uint8_t CLRSTAT_CMD[4]		= {0x07,0x13,0x54,0x96};
static const uint8_t PLADC_CMD[4]		= {0x07,0x14,0xF3,0x6C};
static const uint8_t DIAGN_CMD[4]		= {0x07,0x15,0x78,0x5E};
static const uint8_t WRCOMM_CMD[4]		= {0x07,0x21,0x24,0xB2};
static const uint8_t RDCOMM_CMD[4]		= {0x07,0x22,0x32,0xD6};
static const uint8_t STCOMM_CMD[4]		= {0x07,0x23,0xB9,0xE4};
static const uint8_t Mute_CMD[4]		= {0x00,0x28,0xE8,0x0E};
static const uint8_t Unmute_CMD[4]		= {0x00,0x29,0x63,0x3C};
static const uint8_t CLRSCTRL_CMD[4]	= {0x00,0x18,0x05,0x7C};


/***********************************************************************************************
*
* //@brief   MT9805_GlobeVaribleInit MT9805 software data initiate
* //@param    none
* //@return   none
*
************************************************************************************************/
void MT9805_GlobeVaribleInit(void)
{
	uint8_t chip = 0;
	pMD_XXHzStr[7] =  (char *)MD_26HzStr ;
	pMD_XXHzStr[6] =  (char *)MD_422HzStr;
	pMD_XXHzStr[5] =  (char *)MD_1KHzStr;
	pMD_XXHzStr[4] =  (char *)MD_2KHzStr;
	pMD_XXHzStr[3] =  (char *)MD_3KHzStr;
	pMD_XXHzStr[2] =  (char *)MD_7KHzStr;
	pMD_XXHzStr[1] =  (char *)MD_14KHzStr;
	pMD_XXHzStr[0] =  (char *)MD_27HzStr;


	memcpy(&WCFG_A_buf[0],&WRCFGA_CMD[0],4);
	memcpy(&WCFG_B_buf[0],&WRCFGB_CMD[0],4);
	memcpy(&RCFG_A_buf[0],&RDCFGA_CMD[0],4);
	memcpy(&RCFG_B_buf[0],&RDCFGB_CMD[0],4);

	WCOMM_buf[0] = MT9805_WRCOMM_H;
	WCOMM_buf[1] = MT9805_WRCOMM_L;

	RCOMM_buf[0] = MT9805_RDCOMM_H;
	RCOMM_buf[1] = MT9805_RDCOMM_L;

	for(chip=0;chip<isoChipSize;chip++){
		MT9805DataBase[chip].ConfigureData_A = (MT9805Confg_A_TypeDef*)&WCFG_A_buf[4 + 8*(isoChipSize - chip -1)];
		
		MT9805DataBase[chip].ConfigureData_A->ADCOPT = 0;
		MT9805DataBase[chip].ConfigureData_A->DTEN = 1;
		MT9805DataBase[chip].ConfigureData_A->REFON = 1;
		MT9805DataBase[chip].ConfigureData_A->GPIO1_5 = 0x1f;
		MT9805DataBase[chip].ConfigureData_A->VUV0_7 = 0;
		MT9805DataBase[chip].ConfigureData_A->VUV8_11 = 0;
		MT9805DataBase[chip].ConfigureData_A->VOV0_3 = 0x0f;
		MT9805DataBase[chip].ConfigureData_A->VOV4_11 = 0xff;
		MT9805DataBase[chip].ConfigureData_A->DCC1_8 = 0;
		MT9805DataBase[chip].ConfigureData_A->DCC9_12 = 0;
		MT9805DataBase[chip].ConfigureData_A->DCTO0_3 = 0;

		MT9805DataBase[chip].ConfigureData_B = (MT9805Confg_B_TypeDef*)&WCFG_B_buf[4+8*(isoChipSize-chip-1)];
		MT9805DataBase[chip].ConfigureData_B->GPIO6_9	=0x0f;
		MT9805DataBase[chip].ConfigureData_B->DCC13_16	= 0;	
		MT9805DataBase[chip].ConfigureData_B->DCC17_18		= 0;	
		MT9805DataBase[chip].ConfigureData_B->DCC0		= 0;	
		MT9805DataBase[chip].ConfigureData_B->DTMEN		= 0;	
		MT9805DataBase[chip].ConfigureData_B->PS0_1		= 0;	
		MT9805DataBase[chip].ConfigureData_B->FDRF		= 0;	
		MT9805DataBase[chip].ConfigureData_B->MUTE		= 0;	
		MT9805DataBase[chip].ConfigureData_B->CFGBR2	= 0;	
		MT9805DataBase[chip].ConfigureData_B->CFGBR3	= 0;	
		MT9805DataBase[chip].ConfigureData_B->CFGBR4	= 0;	
		MT9805DataBase[chip].ConfigureData_B->CFGBR5	= 0;	

		MT9805DataBase[chip].ReConfigureDataA = (MT9805Confg_A_TypeDef*)&RCFG_A_buf[4+8*(isoChipSize-chip-1)];
		MT9805DataBase[chip].ReConfigureDataB = (MT9805Confg_B_TypeDef*)&RCFG_B_buf[4+8*(isoChipSize-chip-1)];

    MT9805DataBase[chip].Comm = &WCOMM_buf[4+8*(isoChipSize-chip-1)];

		MT9805DataBase[chip].ReComm= &RCOMM_buf[4+8*chip];

		MT9805DataBase[chip].BalanceSW = 0;
	}
	delayMS(10);
}
/***********************************************************************************************
*
* //@brief   MT9805_CommunicationInit MT9805 
* //@param    none
* //@return   none
*
************************************************************************************************/
void MT9805_CommunicationInit(void){
	    int8_t MD = ADCOPT_MD_27KHZ;
	    MT9805_Awake(isoChipSize);
	    Write9805_CFGRA(isoChipSize);
	    Write9805_CFGRB(isoChipSize);
	
	    Compare_CFRG_A(isoChipSize);
		  Compare_CFRG_B(isoChipSize);
	    MT9805_SetADCOPT(isoChipSize,ADCOPT_MD_MAPPER[MD][0]);
			MT9805_Awake(isoChipSize);
			Write9805_CFGRA(isoChipSize);
			MT9805_Awake(isoChipSize);
}
/***********************************************************************************************
*
* //@brief   MT9805_Init MT9805 
* //@param    none
* //@return   none
*
************************************************************************************************/
void MT9805_Init(void){

	MT9805_GlobeVaribleInit();
	MT9805_CommunicationInit();
}


/***********************************************************************************************
*
* //@brief   MT9805_Awake MT9805 spi??
* //@param    uint8_t chipSize:???????
* //@return   none
*
************************************************************************************************/
static void MT9805_Awake(uint8_t chipSize)
{
	uint8_t chip;
	chip = 10;
	SPI_CS_LOW();
	while(chip--){
	  SPI_WriteReadByte(0xff);
	}
	SPI_CS_HIGH();
}
/***********************************************************************************************
*
* //@brief    Write9805_CFGRA
* //@param    uint8_t chipSize
* //@return   none
*
************************************************************************************************/  
static void Write9805_CFGRA(uint8_t chipSize)
{
	uint8_t chip;
	uint8_t i;
	uint16_t crc;
    
    for(chip=0;chip<chipSize;chip++){
        crc = M9805_pec_calc(&WCFG_A_buf[4+8*chip],6);
        WCFG_A_buf[4+8*chip+6] = (crc>>8)&0x00ff;
        WCFG_A_buf[4+8*chip+7] = (crc)&0x00ff;
    }


	SPI_CS_LOW();
	for (i=0;i<(4+chipSize*8);i++){
		SPI_WriteReadByte(WCFG_A_buf[i]);	
	}
	SPI_CS_HIGH();

}
/***********************************************************************************************
*
* //@brief    Write9805_CFGRB ??????
* //@param    uint8_t chipSize ???????
* //@return   none
*
************************************************************************************************/  
static void Write9805_CFGRB(uint8_t chipSize)
{
	uint8_t chip;
	uint8_t i;
	uint16_t crc;
	for(chip=0;chip<chipSize;chip++){
			crc = M9805_pec_calc(&WCFG_B_buf[4+8*chip],6);
			WCFG_B_buf[4+8*chip+6] = (crc>>8)&0x00ff;
			WCFG_B_buf[4+8*chip+7] = (crc)&0x00ff;
	}
	SPI_CS_LOW();
	for (i=0;i<(4+chipSize*8);i++){
		SPI_WriteReadByte(WCFG_B_buf[i]);	
	}
	SPI_CS_HIGH();

}
/***********************************************************************************************
*
* //@brief    Read_CFGRA ??????
* //@param    uint8_t chipSize ???????
* //@return   none
*
************************************************************************************************/  
static void Read_CFGRA(uint8_t chipSize)
{
	uint16_t crc;
    uint8_t chip;
	uint8_t i;
	/* ????? */
	SPI_CS_LOW();
	SPI_WriteReadByte(RCFG_A_buf[0]);
	SPI_WriteReadByte(RCFG_A_buf[1]);
	SPI_WriteReadByte(RCFG_A_buf[2]);
	SPI_WriteReadByte(RCFG_A_buf[3]);
	/* ????? */
	for (i=4;i<(4+chipSize*8);i++){
		RCFG_A_buf[i] = SPI_WriteReadByte(0xff);
	}
	SPI_CS_HIGH();
}
/***********************************************************************************************
*
* //@brief    Read_CFGRB ??????
* //@param    uint8_t chipSize ???????
* //@return   none
*
************************************************************************************************/  
static void Read_CFGRB(uint8_t chipSize)
{
	uint16_t crc;
    uint8_t chip;
	uint8_t i;
	/* ????? */
	SPI_CS_LOW();
	SPI_WriteReadByte(RCFG_B_buf[0]);
	SPI_WriteReadByte(RCFG_B_buf[1]);
	SPI_WriteReadByte(RCFG_B_buf[2]);
	SPI_WriteReadByte(RCFG_B_buf[3]);
	/* ????? */
	for (i=4;i<(4+chipSize*8);i++){
		RCFG_B_buf[i] = SPI_WriteReadByte(0xff);
	}
	SPI_CS_HIGH();
}
/***********************************************************************************************
*
* //@brief   MT9805_Cmd ?????????
* //@param    int ms: ?????ms
* //@return   none
*
************************************************************************************************/
static void MT9805_Cmd(uint8_t *pCmd)
{
	/* ?????? */
	SPI_CS_LOW();
	SPI_WriteReadByte(pCmd[0]);	
	SPI_WriteReadByte(pCmd[1]);	
	SPI_WriteReadByte(pCmd[2]);	
	SPI_WriteReadByte(pCmd[3]);
	SPI_CS_HIGH();
}
//
static void MT9805_SetADCOPT(uint8_t chipsize,uint8_t val)
{
	uint8_t chip;
	/* check par */
	if(val>1){
		return;
	}

	for(chip=0;chip<chipsize;chip++){
		MT9805DataBase[chip].ConfigureData_A->ADCOPT = val;
	}

}

/***********************************************************************************************
*
* //@brief   M9805_CmdOption MT9805 ??????
* //@param    uint8_t chipSize:???????
* //@return   none
*
************************************************************************************************/
static void M9805_CmdOption (MT9805_CmdType type , uint16_t MD,uint8_t PUP,uint8_t St,uint8_t DCP,uint8_t CHx)
{
	uint16_t CRC;
	uint8_t CMD[4];

	if(type == ADCV){
		CMD[0] = 0x02|(MD>>8);
		CMD[1] = 0x60|(MD&0xff)|DCP|CHx;
	}
	else if(type == ADOW){
		CMD[0] = 0x02|(MD>>8);
		CMD[1] = 0x28|(MD&0xff)|CHx|PUP|DCP;
	}
	else if(type == CVST){
		CMD[0] = 0x02|(MD>>8);
		CMD[1] = 0x07|(MD&0xff)|CHx|St;
	}
	else if(type == ADOL){
		CMD[0] = 0x02|(MD>>8);
		CMD[1] = 0x01|(MD&0xff)|DCP;
	}
	else if(type == ADAX){
		CMD[0] = 0x04|(MD>>8);
		CMD[1] = 0x60|(MD&0xff)|CHx;
	}
	else if(type == ADAXD){
		CMD[0] = 0x04|(MD>>8);
		CMD[1] = 0x00|(MD&0xff)|CHx;
	}
	else if(type == AXOW){
		CMD[0] = 0x04|(MD>>8);
		CMD[1] = 0x10|(MD&0xff)|CHx|PUP;
	}
	else if(type == AXST){
		CMD[0] = 0x04|(MD>>8);
		CMD[1] = 0x07|(MD&0xff)|St;
	}
	else if(type == ADSTAT){
		CMD[0] = 0x04|(MD>>8);
		CMD[1] = 0x68|(MD&0xff)|CHx;
	}
	else if(type == ADSTATD){
		CMD[0] = 0x04|(MD>>8);
		CMD[1] = 0x08|(MD&0xff)|CHx;
	}
	else if(type == STATST){
		CMD[0] = 0x04|(MD>>8);
		CMD[1] = 0x0F|(MD&0xff)|St;
	}
	else if(type == ADCVAX){
		CMD[0] = 0x04|(MD>>8);
		CMD[1] = 0x6f|(MD&0xff)|DCP;
	}
	else if(type == ADCVSC){
		CMD[0] = 0x04|(MD>>8);
		CMD[1] = 0x67|(MD&0xff)|DCP;
	}
	else{
		return;
	}
	CRC = M9805_pec_calc(&CMD[0],2);
	CMD[2] = (CRC>>8)&0xff;
	CMD[3] = (CRC>>0)&0xff;

	MT9805_Cmd(CMD);
}
/***********************************************************************************************
*
* //@brief    Compare_CFRG_A
* //@param    uint8_t chipSize
* //@return   none
*
************************************************************************************************/  
static uint8_t Compare_CFRG_A(uint8_t chipSize)
{
		uint8_t Result;
		uint8_t Counts;
		uint8_t chip;
		uint8_t i,m;
	
		MT9805_Awake(chipSize);
		Write9805_CFGRA(chipSize);
		delayMS(2);
		Read_CFGRA(chipSize);
		delayMS(2);
		for(chip=0;chip<chipSize;chip++){
	 	Counts = 0;
		for(i=(12*(chipSize-1) +4);i<(12*(chipSize-1) +10);i++){
			if(i==(12*(chipSize-1) +4)){
				if((WCFG_A_buf[i] &0x05) != (RCFG_A_buf[i] &0x05))
					Counts++;
			}
			else{
				if(WCFG_A_buf[i] != RCFG_A_buf[i])
					Counts ++;
			}
		}
	 }
	 return Result;
}

/***********************************************************************************************
*
* //@brief    Compare_CFRG ???????B
* //@param    uint8_t chipSize ???????
* //@return   none
*
************************************************************************************************/  
static uint8_t Compare_CFRG_B(uint8_t chipSize)
{
		uint8_t Result;
		uint8_t Counts;
		uint8_t chip;
		uint8_t i,m;
	
		MT9805_Awake(chipSize);
		Write9805_CFGRB(chipSize);
		delayMS(2);
		Read_CFGRB(chipSize);
		delayMS(2);
		for(chip=0;chip<chipSize;chip++){
	 	Counts = 0;
		for(i=(12*(chipSize-1) +4);i<(12*(chipSize-1) +10);i++){
				if(WCFG_B_buf[i] != RCFG_B_buf[i])
					Counts ++;
		}
	 }
	 return Result;
}
/***********************************************************************************************
*
* //@brief   MT9805_SetGPIO ??GPIO??
* //@param    MT9805DataPackageTypeDef *pMT9805DataPackage: ?????ms
* //@param   	uint8_t gpio: ???????
* //@param    uint8_t val: ??????
* //@return   none
*
************************************************************************************************/
static void MT9805_SetGPIO(MT9805DataPackageTypeDef *pMT9805DataPackage, int8_t gpio,uint8_t val)
{
	if((gpio>=MT9805_GPIO_1)&&(gpio<=MT9805_GPIO_5)){
		if(val){
			pMT9805DataPackage->ConfigureData_A->GPIO1_5 |= (0x01<<gpio);
		}
		else{
			pMT9805DataPackage->ConfigureData_A->GPIO1_5 &= (~(0x01<<gpio));
		}
	}
	else if((gpio>=MT9805_GPIO_6)&&(gpio<=MT9805_GPIO_9)){
		if(val){
			pMT9805DataPackage->ConfigureData_B->GPIO6_9 |= (0x01<<(gpio-MT9805_GPIO_6));
		}
		else{
			pMT9805DataPackage->ConfigureData_B->GPIO6_9 &= (~(0x01<<(gpio-MT9805_GPIO_6)));
		}	
	}

}
/***********************************************************************************************
*
* //@brief   Read_VolReg: Read battery voltage register
* //@param    uint8_t chipSize: Daisy chain number
* //@param    MT9805_AdcValGroup group:Register group,A,B,C,D,E,F
* //@return   none
*
************************************************************************************************/
static void Read_VolReg(uint8_t chipSize,MT9805_AdcValGroup group)
{
	uint8_t i;
	uint8_t chip;
	uint8_t cell;
	if(chipSize>MAX_MT9805_CHIP_SIZE){
		return;
	}

	/* buf?? */
	memset(RD_X_buf,0,(4+8*chipSize));
	/* ???? */ 
	if(group == ADREG_A_Group){
		memcpy(&RD_X_buf[0],&RDCVA_CMD[0],4);
	}
	else if(group == ADREG_B_Group){
		memcpy(&RD_X_buf[0],&RDCVB_CMD[0],4);
	}
	else if(group == ADREG_C_Group){
		memcpy(&RD_X_buf[0],&RDCVC_CMD[0],4);
	}
	else if(group == ADREG_D_Group){
		memcpy(&RD_X_buf[0],&RDCVD_CMD[0],4);
	}
	else if(group == ADREG_E_Group){
		memcpy(&RD_X_buf[0],&RDCVE_CMD[0],4);
	}
	else if(group == ADREG_F_Group){
		memcpy(&RD_X_buf[0],&RDCVF_CMD[0],4);
	}
	else{
		return;
	}

  SPI_CS_LOW();
	for(i=0;i<(4+8*chipSize);i++){
		RD_X_buf[i] = SPI_WriteReadByte(RD_X_buf[i]);
	}
	SPI_CS_HIGH();
	 delayMS(2);
   for(chip=0;chip<isoChipSize;chip++)
   {
     	/* ???? */
		if(group == ADREG_A_Group){
			for(chip=0;chip<chipSize;chip++){
				MT9805DataBase[chip].VoltageData[0] = RD_X_buf[4+8*chip+0] + (uint16_t)RD_X_buf[4+8*chip+1]*256;
				MT9805DataBase[chip].VoltageData[1] = RD_X_buf[4+8*chip+2] + (uint16_t)RD_X_buf[4+8*chip+3]*256;
				MT9805DataBase[chip].VoltageData[2] = RD_X_buf[4+8*chip+4] + (uint16_t)RD_X_buf[4+8*chip+5]*256;
			}
		}
		else if(group == ADREG_B_Group){
			for(chip=0;chip<chipSize;chip++){
				MT9805DataBase[chip].VoltageData[3] = RD_X_buf[4+8*chip+0] + (uint16_t)RD_X_buf[4+8*chip+1]*256;
				MT9805DataBase[chip].VoltageData[4] = RD_X_buf[4+8*chip+2] + (uint16_t)RD_X_buf[4+8*chip+3]*256;
				MT9805DataBase[chip].VoltageData[5] = RD_X_buf[4+8*chip+4] + (uint16_t)RD_X_buf[4+8*chip+5]*256;
			}
		}
		else if(group == ADREG_C_Group){
			for(chip=0;chip<chipSize;chip++){
				MT9805DataBase[chip].VoltageData[6] = RD_X_buf[4+8*chip+0] + (uint16_t)RD_X_buf[4+8*chip+1]*256;
				MT9805DataBase[chip].VoltageData[7] = RD_X_buf[4+8*chip+2] + (uint16_t)RD_X_buf[4+8*chip+3]*256;
				MT9805DataBase[chip].VoltageData[8] = RD_X_buf[4+8*chip+4] + (uint16_t)RD_X_buf[4+8*chip+5]*256;
			}
		}
		else if(group == ADREG_D_Group){
			for(chip=0;chip<chipSize;chip++){
				MT9805DataBase[chip].VoltageData[9]  = RD_X_buf[4+8*chip+0] + (uint16_t)RD_X_buf[4+8*chip+1]*256;
				MT9805DataBase[chip].VoltageData[10] = RD_X_buf[4+8*chip+2] + (uint16_t)RD_X_buf[4+8*chip+3]*256;
				MT9805DataBase[chip].VoltageData[11] = RD_X_buf[4+8*chip+4] + (uint16_t)RD_X_buf[4+8*chip+5]*256;
			}
		}
		else if(group == ADREG_E_Group){
			for(chip=0;chip<chipSize;chip++){
				MT9805DataBase[chip].VoltageData[12]  = RD_X_buf[4+8*chip+0] + (uint16_t)RD_X_buf[4+8*chip+1]*256;
				MT9805DataBase[chip].VoltageData[13] = RD_X_buf[4+8*chip+2] + (uint16_t)RD_X_buf[4+8*chip+3]*256;
				MT9805DataBase[chip].VoltageData[14] = RD_X_buf[4+8*chip+4] + (uint16_t)RD_X_buf[4+8*chip+5]*256;
			}
		}
		else if(group == ADREG_F_Group){
			for(chip=0;chip<chipSize;chip++){
				MT9805DataBase[chip].VoltageData[15]  = RD_X_buf[4+8*chip+0] + (uint16_t)RD_X_buf[4+8*chip+1]*256;
				MT9805DataBase[chip].VoltageData[16] = RD_X_buf[4+8*chip+2] + (uint16_t)RD_X_buf[4+8*chip+3]*256;
				MT9805DataBase[chip].VoltageData[17] = RD_X_buf[4+8*chip+4] + (uint16_t)RD_X_buf[4+8*chip+5]*256;
			}
		}
		else{
			return;
		}
   }

}
/***********************************************************************************************
*
* //@brief   Read_AuxReg ???????
* //@param    uint8_t chipSize:???????
* //@param    MT9805_AuxGroup group:?????,A,B
* //@return   none
*
************************************************************************************************/
static void Read_AuxReg(uint8_t chipSize,MT9805_AuxGroup group)
{
	uint8_t i;
	uint8_t chip;
	uint8_t cell;
	uint32_t bit;
	
	if(chipSize>MAX_MT9805_CHIP_SIZE){
		return;
	}

	/* buf?? */
	memset(RD_X_buf,0,(4+8*chipSize));
	/* ???? */ 
	if(group == AUXREG_A_Group){
		memcpy(&RD_X_buf[0],&RDAUXA_CMD[0],4);
	}
	else if(group == AUXREG_B_Group){
		memcpy(&RD_X_buf[0],&RDAUXB_CMD[0],4);
	}
	else if(group == AUXREG_C_Group){
		memcpy(&RD_X_buf[0],&RDAUXC_CMD[0],4);
	}
	else if(group == AUXREG_D_Group){
		memcpy(&RD_X_buf[0],&RDAUXD_CMD[0],4);
	}
	else{
		return;
	}

	/* ?? */
	SPI_CS_LOW();
	for(i=0;i<(4+8*chipSize);i++){
		RD_X_buf[i] = SPI_WriteReadByte(RD_X_buf[i]);
	}
	SPI_CS_HIGH();
	/* ???? */
	if(group == AUXREG_A_Group){
		for(chip=0;chip<chipSize;chip++){
			MT9805DataBase[chip].GPIO_Voltage[0] = RD_X_buf[4+8*chip+0] + (uint16_t)RD_X_buf[4+8*chip+1]*256;
			MT9805DataBase[chip].GPIO_Voltage[1] = RD_X_buf[4+8*chip+2] + (uint16_t)RD_X_buf[4+8*chip+3]*256;
			MT9805DataBase[chip].GPIO_Voltage[2] = RD_X_buf[4+8*chip+4] + (uint16_t)RD_X_buf[4+8*chip+5]*256;
		}
	}
	else if(group == AUXREG_B_Group){
		for(chip=0;chip<chipSize;chip++){
			MT9805DataBase[chip].GPIO_Voltage[3] = RD_X_buf[4+8*chip+0] + (uint16_t)RD_X_buf[4+8*chip+1]*256;
			MT9805DataBase[chip].GPIO_Voltage[4] = RD_X_buf[4+8*chip+2] + (uint16_t)RD_X_buf[4+8*chip+3]*256;
			MT9805DataBase[chip].REF 			 = RD_X_buf[4+8*chip+4] + (uint16_t)RD_X_buf[4+8*chip+5]*256;
		}
	}
	else if(group == AUXREG_C_Group){
		for(chip=0;chip<chipSize;chip++){
			MT9805DataBase[chip].GPIO_Voltage[5] = RD_X_buf[4+8*chip+0] + (uint16_t)RD_X_buf[4+8*chip+1]*256;
			MT9805DataBase[chip].GPIO_Voltage[6] = RD_X_buf[4+8*chip+2] + (uint16_t)RD_X_buf[4+8*chip+3]*256;
			MT9805DataBase[chip].GPIO_Voltage[7] = RD_X_buf[4+8*chip+4] + (uint16_t)RD_X_buf[4+8*chip+5]*256;
		}
	}
	else if(group == AUXREG_D_Group){
		for(chip=0;chip<chipSize;chip++){
			MT9805DataBase[chip].GPIO_Voltage[8]	= RD_X_buf[4+8*chip+0] + (uint16_t)RD_X_buf[4+8*chip+1]*256;
			MT9805DataBase[chip].RSVD1				= RD_X_buf[4+8*chip+2] + (uint16_t)RD_X_buf[4+8*chip+3]*256;
			for(bit=13;bit<=18;bit++){
				/* CxUV */
				if(bit<17){
					if(RD_X_buf[4+8*chip+4]&(0x01<<((bit-13)*2))){
						MT9805DataBase[chip].CxUV |= (0x00000001<<(bit-1));
					}
					else{
						MT9805DataBase[chip].CxUV &= (~(0x00000001<<(bit-1)));
					}
				}
				else{
					if(RD_X_buf[4+8*chip+5]&(0x01<<((bit-17)*2))){
						MT9805DataBase[chip].CxUV |= (0x00000001<<(bit-1));
					}
					else{
						MT9805DataBase[chip].CxUV &= (~(0x00000001<<(bit-1)));
					}
				}
				/* CxOV */
				if(bit<17){
					if(RD_X_buf[4+8*chip+4]&(0x02<<((bit-13)*2))){
						MT9805DataBase[chip].CxOV |= (0x00000001<<(bit-1));
					}
					else{
						MT9805DataBase[chip].CxOV &= (~(0x00000001<<(bit-1)));
					}
				}
				else{
					if(RD_X_buf[4+8*chip+5]&(0x02<<((bit-17)*2))){
						MT9805DataBase[chip].CxOV |= (0x00000001<<(bit-1));
					}
					else{
						MT9805DataBase[chip].CxOV &= (~(0x00000001<<(bit-1)));
					}
				}

			}

		}
	}
	else{
		return;
	}
}


void Get_Volt(void){
	    int8_t MD = ADCOPT_MD_27KHZ;
			MT9805_Cmd((uint8_t *)CLRCELL_CMD);
			MT9805_Awake(isoChipSize);
			M9805_CmdOption(ADCV,ADCOPT_MD_MAPPER[MD][1],0,0,0,CMD_BITS_CH_ALL);
			MT9805_Awake(isoChipSize);
	    delayMS(10);
	
      Read_VolReg(isoChipSize,ADREG_A_Group);
		  Read_VolReg(isoChipSize,ADREG_B_Group);
			Read_VolReg(isoChipSize,ADREG_C_Group);
			Read_VolReg(isoChipSize,ADREG_D_Group);
			Read_VolReg(isoChipSize,ADREG_E_Group);
			Read_VolReg(isoChipSize,ADREG_F_Group);
}
void testfunc(){
  /******************  NTC5~8 *******************/
			/*ADAX */
	    uint8_t chip = 0;
	    uint32_t vref2 = 0;
	    uint16_t extNtcTempAdcVal;
			MT9805_Awake(isoChipSize);
			M9805_CmdOption(ADAX,CMD_BITS_MD_27K_14K,0,0,0,CMD_BITS_CHG_1to9_2ndREF);
			delayMS(10);
			
			MT9805_Awake(isoChipSize);
			Read_AuxReg(isoChipSize,AUXREG_C_Group);
			Read_AuxReg(isoChipSize,AUXREG_B_Group);
			Read_AuxReg(isoChipSize,AUXREG_D_Group);
	    
	    GPIO_Vol_test[4] = MT9805DataBase[0].GPIO_Voltage[MT9805_GPIO_6];
	    GPIO_Vol_test[5] = MT9805DataBase[0].GPIO_Voltage[MT9805_GPIO_7];
	    GPIO_Vol_test[6] = MT9805DataBase[0].GPIO_Voltage[MT9805_GPIO_8];
	    GPIO_Vol_test[7] = MT9805DataBase[0].GPIO_Voltage[MT9805_GPIO_9];
	
		  //
	    
	    MT9805_SetGPIO(&MT9805DataBase[chip], MT9805_GPIO_1,0);
			MT9805_SetGPIO(&MT9805DataBase[chip], MT9805_GPIO_2,0);
	    MT9805_Awake(isoChipSize);
			Write9805_CFGRA(isoChipSize);
			delayMS(1);
			Write9805_CFGRB(isoChipSize);
			delayMS(1);
			/* ??ADAX */
			M9805_CmdOption(ADAX,ADCOPT_MD_MAPPER[ADCOPT_MD_27KHZ][1],0,0,0,CMD_BITS_CHG_3_8);
			delayMS(100);
			/* ??????? */
			MT9805_Awake(isoChipSize);
			Read_AuxReg(isoChipSize,AUXREG_A_Group);
			GPIO_Vol_test[0] = MT9805DataBase[0].GPIO_Voltage[MT9805_GPIO_6];
			//
	    MT9805_SetGPIO(&MT9805DataBase[chip], MT9805_GPIO_1,1);
			MT9805_SetGPIO(&MT9805DataBase[chip], MT9805_GPIO_2,0);
			
			
	    MT9805_Awake(isoChipSize);
			Write9805_CFGRA(isoChipSize);
			delayMS(1);
			Write9805_CFGRB(isoChipSize);
			delayMS(1);
			M9805_CmdOption(ADAX,CMD_BITS_MD_27K_14K,0,0,0,CMD_BITS_CHG_3_8);
			delayMS(100);
			MT9805_Awake(isoChipSize);
			Read_AuxReg(isoChipSize,AUXREG_A_Group);
			GPIO_Vol_test[1] = MT9805DataBase[0].GPIO_Voltage[MT9805_GPIO_6];
			//
			MT9805_SetGPIO(&MT9805DataBase[chip], MT9805_GPIO_1,0);
			MT9805_SetGPIO(&MT9805DataBase[chip], MT9805_GPIO_2,1);
	    MT9805_Awake(isoChipSize);
			Write9805_CFGRA(isoChipSize);
			delayMS(1);
			Write9805_CFGRB(isoChipSize);
			delayMS(1);
			M9805_CmdOption(ADAX,CMD_BITS_MD_27K_14K,0,0,0,CMD_BITS_CHG_3_8);
			delayMS(100);
			MT9805_Awake(isoChipSize);
			Read_AuxReg(isoChipSize,AUXREG_A_Group);
			GPIO_Vol_test[2] = MT9805DataBase[0].GPIO_Voltage[MT9805_GPIO_6];
			//
			MT9805_SetGPIO(&MT9805DataBase[chip], MT9805_GPIO_1,1);
			MT9805_SetGPIO(&MT9805DataBase[chip], MT9805_GPIO_2,1);
	    MT9805_Awake(isoChipSize);
			Write9805_CFGRA(isoChipSize);
			delayMS(1);
			Write9805_CFGRB(isoChipSize);
			delayMS(1);
			M9805_CmdOption(ADAX,CMD_BITS_MD_27K_14K,0,0,0,CMD_BITS_CHG_3_8);
			delayMS(100);
			MT9805_Awake(isoChipSize);
			Read_AuxReg(isoChipSize,AUXREG_A_Group);
			GPIO_Vol_test[3] = MT9805DataBase[0].GPIO_Voltage[MT9805_GPIO_6];
}
void Get_Temp(void){
      /******************  NTC5~8 *******************/
			/*ADAX */
	    uint8_t chip = 0;
	    uint32_t vref2;
	    uint16_t extNtcTempAdcVal;
			MT9805_Awake(isoChipSize);
			M9805_CmdOption(ADAX,CMD_BITS_MD_7K_3K,0,0,0,CMD_BITS_CHG_1to9_2ndREF);
			delayMS(10);
			/* ??????? */
			MT9805_Awake(isoChipSize);
			Read_AuxReg(isoChipSize,AUXREG_C_Group);
			Read_AuxReg(isoChipSize,AUXREG_B_Group);
			Read_AuxReg(isoChipSize,AUXREG_D_Group);
			 
			for(chip=0;chip<isoChipSize;chip++){
				vref2 = (MT9805DataBase[chip].GPIO_Voltage[MT9805_GPIO_9])*2000;
				if(MT9805DataBase[chip].GPIO_Voltage[MT9805_GPIO_6])
				{
				    vref2 = (vref2/MT9805DataBase[chip].GPIO_Voltage[MT9805_GPIO_6]);
				}
				extNtcTempAdcVal = (uint16_t) vref2;
				VoltToTemp(&MT9805DataBase[chip].ntcTemp[4],extNtcTempAdcVal,NTC_TABLE,QUERY_TABLE_LEN,MT9805DataBase[chip].ntcTemp[4],TABLE_SORT_TYPE_FORWARD);

				vref2 = (MT9805DataBase[chip].GPIO_Voltage[MT9805_GPIO_9])*2000;
				if(MT9805DataBase[chip].GPIO_Voltage[MT9805_GPIO_7])
				{
				    vref2 = (vref2/MT9805DataBase[chip].GPIO_Voltage[MT9805_GPIO_7]);
				}
				extNtcTempAdcVal = (uint16_t) vref2;
				VoltToTemp(&MT9805DataBase[chip].ntcTemp[5],extNtcTempAdcVal,NTC_TABLE,QUERY_TABLE_LEN,MT9805DataBase[chip].ntcTemp[5],TABLE_SORT_TYPE_FORWARD);

				vref2 = (MT9805DataBase[chip].GPIO_Voltage[MT9805_GPIO_9])*2000;
				if(MT9805DataBase[chip].GPIO_Voltage[MT9805_GPIO_8])
				{
				    vref2 = (vref2/MT9805DataBase[chip].GPIO_Voltage[MT9805_GPIO_8]);
				}
				extNtcTempAdcVal = (uint16_t) vref2;
				VoltToTemp(&MT9805DataBase[chip].ntcTemp[6],extNtcTempAdcVal,NTC_TABLE,QUERY_TABLE_LEN,MT9805DataBase[chip].ntcTemp[6],TABLE_SORT_TYPE_FORWARD);

				vref2 = (MT9805DataBase[chip].GPIO_Voltage[MT9805_GPIO_9])*2000;
				if(MT9805DataBase[chip].GPIO_Voltage[MT9805_GPIO_9])
				{
				    vref2 = (vref2/MT9805DataBase[chip].GPIO_Voltage[MT9805_GPIO_9]);
				}
				extNtcTempAdcVal = (uint16_t) vref2;
				VoltToTemp(&MT9805DataBase[chip].ntcTemp[7],extNtcTempAdcVal,NTC_TABLE,QUERY_TABLE_LEN,MT9805DataBase[chip].ntcTemp[7],TABLE_SORT_TYPE_FORWARD);
			}

			/******************* GPIO1=0,GPIO2=0  NTC1 *******************/
			for(chip=0;chip<isoChipSize;chip++){
				MT9805_SetGPIO(&MT9805DataBase[chip], MT9805_GPIO_1,0);
				MT9805_SetGPIO(&MT9805DataBase[chip], MT9805_GPIO_2,0);
			}
			MT9805_Awake(isoChipSize);
			Write9805_CFGRA(isoChipSize);
			delayMS(1);
			Write9805_CFGRB(isoChipSize);
			delayMS(1);
			/* ??ADAX */
			M9805_CmdOption(ADAX,ADCOPT_MD_MAPPER[ADCOPT_MD_27KHZ][1],0,0,0,CMD_BITS_CHG_3_8);
			delayMS(100);
			/* ??????? */
			MT9805_Awake(isoChipSize);
			Read_AuxReg(isoChipSize,AUXREG_A_Group);
			for(chip=0;chip<isoChipSize;chip++){
				vref2 = (MT9805DataBase[chip].GPIO_Voltage[MT9805_GPIO_9])*2000;
				if(MT9805DataBase[chip].GPIO_Voltage[MT9805_GPIO_3])
				{
				    vref2 = (vref2/MT9805DataBase[chip].GPIO_Voltage[MT9805_GPIO_3]);
			  }
				extNtcTempAdcVal = (uint16_t) vref2;
				VoltToTemp(&MT9805DataBase[chip].ntcTemp[0],extNtcTempAdcVal,NTC_TABLE,QUERY_TABLE_LEN,MT9805DataBase[chip].ntcTemp[0],TABLE_SORT_TYPE_FORWARD);
			}
			/******************* GPIO1=0,GPIO2=0  NTC2 *******************/
			for(chip=0;chip<isoChipSize;chip++){
				MT9805_SetGPIO(&MT9805DataBase[chip], MT9805_GPIO_1,1);
				MT9805_SetGPIO(&MT9805DataBase[chip], MT9805_GPIO_2,0);
			}
			MT9805_Awake(isoChipSize);
			Write9805_CFGRA(isoChipSize);
			delayMS(1);
			Write9805_CFGRB(isoChipSize);
			delayMS(1);
			/* ??ADAX */
			M9805_CmdOption(ADAX,CMD_BITS_MD_7K_3K,0,0,0,CMD_BITS_CHG_3_8);
			delayMS(100);
			/* ??????? */
			MT9805_Awake(isoChipSize);
			Read_AuxReg(isoChipSize,AUXREG_A_Group);
			for(chip=0;chip<isoChipSize;chip++){
				vref2 = (MT9805DataBase[chip].GPIO_Voltage[MT9805_GPIO_9])*2000;
				if(MT9805DataBase[chip].GPIO_Voltage[MT9805_GPIO_3])
				{
				    vref2 = (vref2/MT9805DataBase[chip].GPIO_Voltage[MT9805_GPIO_3]);
				}
				extNtcTempAdcVal = (uint16_t) vref2;
				VoltToTemp(&MT9805DataBase[chip].ntcTemp[1],extNtcTempAdcVal,NTC_TABLE,QUERY_TABLE_LEN,MT9805DataBase[chip].ntcTemp[1],TABLE_SORT_TYPE_FORWARD);
			}

			/******************* GPIO1=0,GPIO2=0  NTC3 *******************/
			for(chip=0;chip<isoChipSize;chip++){
				MT9805_SetGPIO(&MT9805DataBase[chip], MT9805_GPIO_1,0);
				MT9805_SetGPIO(&MT9805DataBase[chip], MT9805_GPIO_2,1);
			}
			MT9805_Awake(isoChipSize);
			Write9805_CFGRA(isoChipSize);
			delayMS(1);
			Write9805_CFGRB(isoChipSize);
			delayMS(1);
			/* ??ADAX */
			M9805_CmdOption(ADAX,CMD_BITS_MD_7K_3K,0,0,0,CMD_BITS_CHG_3_8);
			delayMS(100);
			/* ??????? */
			MT9805_Awake(isoChipSize);
			Read_AuxReg(isoChipSize,AUXREG_A_Group);
			for(chip=0;chip<isoChipSize;chip++){
				vref2 = (MT9805DataBase[chip].GPIO_Voltage[MT9805_GPIO_9])*2000;
				if(MT9805DataBase[chip].GPIO_Voltage[MT9805_GPIO_3])
				{
				    vref2 = (vref2/MT9805DataBase[chip].GPIO_Voltage[MT9805_GPIO_3]);
				}
				extNtcTempAdcVal = (uint16_t) vref2;
				VoltToTemp(&MT9805DataBase[chip].ntcTemp[2],extNtcTempAdcVal,NTC_TABLE,QUERY_TABLE_LEN,MT9805DataBase[chip].ntcTemp[2],TABLE_SORT_TYPE_FORWARD);
			}
			/******************* GPIO1=0,GPIO2=0  NTC4 *******************/
			for(chip=0;chip<isoChipSize;chip++){
				MT9805_SetGPIO(&MT9805DataBase[chip], MT9805_GPIO_1,1);
				MT9805_SetGPIO(&MT9805DataBase[chip], MT9805_GPIO_2,1);
			}
			MT9805_Awake(isoChipSize);
			Write9805_CFGRA(isoChipSize);
			delayMS(1);
			Write9805_CFGRB(isoChipSize);
			delayMS(1);
			/* ??ADAX */
			M9805_CmdOption(ADAX,CMD_BITS_MD_7K_3K,0,0,0,CMD_BITS_CHG_3_8);
			delayMS(100);
			/* ??????? */
			MT9805_Awake(isoChipSize);
			Read_AuxReg(isoChipSize,AUXREG_A_Group);
			for(chip=0;chip<isoChipSize;chip++){
				vref2 = (MT9805DataBase[chip].GPIO_Voltage[MT9805_GPIO_9])*2000;
				if(MT9805DataBase[chip].GPIO_Voltage[MT9805_GPIO_3])
				{
				    vref2 = (vref2/MT9805DataBase[chip].GPIO_Voltage[MT9805_GPIO_3]);
				}
				extNtcTempAdcVal = (uint16_t) vref2;
				VoltToTemp(&MT9805DataBase[chip].ntcTemp[3],extNtcTempAdcVal,NTC_TABLE,QUERY_TABLE_LEN,MT9805DataBase[chip].ntcTemp[3],TABLE_SORT_TYPE_FORWARD);
			}

}
/***********************************************************************************************
*
* //@brief   M9805_pec_calc MT9805????,???
* //@param    uint8_t *data:??????
* //@param    uint8_t len:??????
* //@return   uint16_t: ????
*
************************************************************************************************/
static uint16_t M9805_pec_calc (uint8_t *data , uint8_t len)
{
    uint16_t remainder;
    uint16_t address;
    uint8_t i;
    remainder = 16;//PEC seed
    for (i = 0; i < len; i++)
    {
        address = ((remainder >> 7) ^ data[i]) & 0xff;//calculate PEC table address
        remainder = (remainder << 8 ) ^ PEC_Table[address];
    }
    return (remainder*2);//The CRC15 has a 0 in the LSB so the final value must be multiplied by 2
}


/**
 * @description: ADC
 * //@param {type} 
 * //@return: 
 */
static uint8_t VoltToTemp(uint8_t *pTemp,uint16_t adcVal,const uint16_t *pTable,uint8_t tableSize,uint8_t preTempVal,uint8_t sortType)
{
    uint8_t diff[3];
    uint8_t i;
    uint8_t mid;
    uint8_t left;
    uint8_t right;
    uint8_t rlt;
    uint16_t leftVal;
    uint16_t rightVal;
    uint16_t midVal;
    uint8_t initFlg;
    uint8_t count = 0;
    left = 0;
    right = tableSize-1;
    initFlg = 1;

    if(TABLE_SORT_TYPE_BACKWARD==sortType){
        if(adcVal>=pTable[0]){
            *pTemp = 0;
            return count;
        }
        else if(adcVal<=pTable[tableSize-1]){
            *pTemp = tableSize-1;
            return count;  
        }
    }
    else if(TABLE_SORT_TYPE_FORWARD==sortType){
        if(adcVal<=pTable[0]){
            *pTemp = 0;
            return count;
        }
        else if(adcVal>=pTable[tableSize-1]){
            *pTemp = tableSize-1;
            return count;  
        }
    }
 
    while(left<right){
        count++;
        // ??mid
        if(initFlg){
            mid = preTempVal;

        }
        else{
            mid = (left+right)/2;
        }
        // ???????
        leftVal = *(pTable+left);
        rightVal = *(pTable+right);
        midVal = *(pTable+mid);  
        // ??????
        if(initFlg){
            if(preTempVal == 0){
                if(TABLE_SORT_TYPE_BACKWARD==sortType){
                    if(adcVal>pTable[1]){
                        if((pTable[0]-adcVal)<(adcVal-pTable[1])){
                            *pTemp = 0;
                            return count;
                        }
                        else{
                            *pTemp = 1;
                            return count;
                        }
                    }
                }
                else{
                    if(adcVal<pTable[1]){
                        if((adcVal - pTable[0])<(pTable[1] - adcVal)){
                            *pTemp = 0;
                            return count;
                        }
                        else{
                            *pTemp = 1;
                            return count;
                        }
                    }
                }

            }
            else if(preTempVal == (tableSize-1)){
                if(TABLE_SORT_TYPE_BACKWARD==sortType){
                    if(adcVal<pTable[tableSize-2]){
                        if((pTable[tableSize-2]-adcVal)<(adcVal-pTable[tableSize-1])){
                            *pTemp = tableSize-2;
                            return count;
                        }
                        else{
                            *pTemp = tableSize-1;
                            return count;
                        }
                    }
                }
                else{
                    if(adcVal>pTable[tableSize-2]){
                        if((adcVal - pTable[tableSize-2])<(pTable[tableSize-1] - adcVal)){
                            *pTemp = tableSize-2;
                            return count;
                        }
                        else{
                            *pTemp = tableSize-1;
                            return count;
                        }
                    }
                }
            }
            else{
                if(TABLE_SORT_TYPE_BACKWARD==sortType){
                   if ((adcVal<pTable[mid-1])&&(adcVal>=midVal)){
                       if((pTable[mid-1]-adcVal)<(adcVal-midVal)){
                            *pTemp = mid-1;
                            return count;
                       }
                       else{
                            *pTemp = mid;
                            return count;
                       }
                   }
                   else if((adcVal>pTable[mid+1])&&(adcVal<midVal)){
                       if((adcVal - pTable[mid+1])<(midVal - adcVal)){
                            *pTemp = mid+1;
                            return count;
                       }
                       else{
                            *pTemp = mid;
                            return count;
                       }
                   }
                }
                else{
                   if ((adcVal>pTable[mid-1])&&(adcVal<=midVal)){
                       if((adcVal - pTable[mid-1])<(midVal - adcVal)){
                            *pTemp = mid-1;
                            return count;
                       }
                       else{
                            *pTemp = mid;
                            return count;
                       }
                   }
                   else if((adcVal<pTable[mid+1])&&(adcVal>midVal)){
                       if((pTable[mid+1] - adcVal)<(adcVal - midVal)){
                            *pTemp = mid+1;
                            return count;
                       }
                       else{
                            *pTemp = mid;
                            return count;
                       }
                   }
                }
            }
        }
        // table???
        if(sortType==TABLE_SORT_TYPE_FORWARD){
            // ?????
            if(left+1==right){
                if((adcVal-leftVal)<(rightVal-adcVal)){
                    *pTemp = left;
                    return count;
                }
                else{
                    *pTemp = right;
                    return count;
                }
            }
            // ????
            if(midVal<adcVal){
                left= mid;  
            }
            else{
               right= mid;
            }
        }
        // table???
        else if(sortType==TABLE_SORT_TYPE_BACKWARD){
                // ?????
                if(left+1==right){
                    if((leftVal-adcVal)<(adcVal-rightVal)){
                        *pTemp = left;
                        return count;
                    }
                    else{
                        *pTemp = right;
                        return count;
                    }
                }
                // ????
                if(midVal>adcVal){
                    left= mid;  
                }
                else{
                right= mid;
                }
            }
            else{
                return 0;
            }
						
		initFlg = 0;
    }
    return 0;
}



/****************************************/
/* cell balance*/
/****************************************/
void cellBalanceTaskTest(void){
	// over Temp or else condition terminate the cell balance to be done
  if(!cellBalanceTaskCondition()) return;
  uint16_t minvol = 0xffff;
	uint16_t deltvol = 300;
	uint32_t cellbalancebits = 0;
	//find min voltage
	for(uint8_t i = 0; i < 18; i++){
	  if(minvol > MT9805DataBase[0].VoltageData[i])
		{
		    minvol = MT9805DataBase[0].VoltageData[i];
		}
	}
	//assert the bit
  for(uint8_t i = 0; i < 18; i++){
	  if(MT9805DataBase[0].VoltageData[i] > minvol + deltvol)
		{
		    cellbalancebits |= (1<<i);
		}
	}	
	MT9805DataBase[0].ConfigureData_A->DCC1_8   = (cellbalancebits & 0xff);
	MT9805DataBase[0].ConfigureData_A->DCC9_12  = ((cellbalancebits & 0xf00)  >> 8);
  MT9805DataBase[0].ConfigureData_B->DCC13_16 = ((cellbalancebits & 0xf000) >> 12);
	MT9805DataBase[0].ConfigureData_B->DCC17_18 = ((cellbalancebits & 0x30000) >> 16);
	Write9805_CFGRA(isoChipSize);
	Write9805_CFGRB(isoChipSize);
}
bool cellBalanceTaskCondition(void){
  return true;
}