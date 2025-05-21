#ifndef MT9805_PROTOCOL_H_
#define MT9805_PROTOCOL_H_

#include"SPI_comm.h"

#define MAX_MT9805_CHIP_SIZE 4
#define DEMO_NTC_SIZE	8

typedef struct
{
	uint8_t ADCOPT	  :1;
	uint8_t DTEN		  :1;
	uint8_t REFON		  :1;
	uint8_t GPIO1_5		:5;
	uint8_t VUV0_7		:8;
	uint8_t VUV8_11		:4;
	uint8_t VOV0_3		:4;
	uint8_t VOV4_11		:8;
	uint8_t DCC1_8		:8;
	uint8_t DCC9_12		:4;
	uint8_t DCTO0_3		:4;
}MT9805Confg_A_TypeDef;

typedef struct
{
	uint8_t GPIO6_9		:4;
	uint8_t DCC13_16	:4;
	uint8_t DCC17_18	:2;
	uint8_t DCC0		  :1;
	uint8_t DTMEN		  :1;
	uint8_t PS0_1		  :2;
	uint8_t FDRF		  :1;
	uint8_t MUTE		  :1;
	uint8_t CFGBR2		:8;
	uint8_t CFGBR3		:8;
	uint8_t CFGBR4		:8;
	uint8_t CFGBR5		:8;
}MT9805Confg_B_TypeDef;

typedef struct
{
	uint16_t VoltageData[18];
	uint16_t CellPup1[18];
	uint16_t CellPup0[18];
	uint16_t IIC_COMM[3];
	uint16_t GPIO_Voltage[9];
	uint8_t ntcTemp[DEMO_NTC_SIZE];			/* ??+40 */
	uint16_t REF;
	uint16_t RSVD1;
	MT9805Confg_A_TypeDef *ConfigureData_A; 
	MT9805Confg_B_TypeDef *ConfigureData_B; 
	MT9805Confg_A_TypeDef *ReConfigureDataA;	
	MT9805Confg_B_TypeDef *ReConfigureDataB;	
	uint8_t *Comm; 
	uint8_t *ReComm;                                              
	uint16_t SOC;
	uint16_t ITMP;
	uint16_t VA;
	uint16_t VD;
	uint32_t CxOV;
	uint32_t CxUV;
	uint8_t REV_RSVD_MUXFAIL_THSD;
	uint8_t BalanceSW;
	uint8_t *configdata_S;
	uint8_t *Rconfigdata_S;
	uint8_t *configdata_PWM;
	uint8_t *configdata_PWMS;
}MT9805DataPackageTypeDef;


enum  MT9805_BalanceSW
{
	SW_AllOn = 1,
	SW_AllOff,
	SW_S1,
	SW_S2,
	SW_S3,
	SW_S4,
	SW_S5,
	SW_S6,
	SW_S7,
	SW_S8,
	SW_S9,
	SW_S10,
	SW_S11,
	SW_S12,
	SW_S13,
	SW_S14,
	SW_S15,
	SW_S16,
	SW_S17,
	SW_S18,
	DCC_0,
};

enum  MT9805_WorkMode
{
	null = 0,
	Init_9805 = 1,
	Clear_RegVoltage ,
	Clear_RegAUX,
	Clear_RegState,
	Write_RegConfig_A = 5,
	Write_RegConfig_B = 6,
	Write_RegCOMM,
	Write_s_config,
	Read_s_config,
	Write_COMMD,
	Read_RegConfig = 20,
	Read_RegCOMM,
	IIC_COMMM,
	Read_AllVoltage,
	Read_AllAUX,
	Read_AllStae,
	Open_circuit_inspection,
	Read_VoltageA,
	Read_VoltageB,
	Read_VoltageC,
	Read_VoltageD,
	Read_VoltageE,
	Read_VoltageF,
	ADOL_TEST,
	Read_AUX_A,
	Read_AUX_B,
	Read_AUX_C,
	Read_AUX_D,
	Read_State_A,
	Read_State_B,
	StartRead_ALL,
	Set_IsoChipSize,
	Set_BalacneSw,
	SET_DCC,
	CVST_self,
	Read_ntcTEMP,
	EEPROM_Write,
	EEPROM_Read,
};


typedef enum{
	ADCV = 0,
	ADOW,
	CVST,
	ADOL,
	ADAX,
	ADAXD,
	AXOW,
	AXST,
	ADSTAT,
	ADSTATD,
	STATST,
	ADCVAX,
	ADCVSC
}MT9805_CmdType;


typedef struct {
    void (*func)(void);
    unsigned short lc;
    unsigned long dl;
}t_EVENTION;

extern t_EVENTION e_SAMPLE;
extern t_EVENTION e_Test_Debug;
extern enum  MT9805_WorkMode List_work;
extern MT9805DataPackageTypeDef MT9805DataBase[MAX_MT9805_CHIP_SIZE];
extern uint8_t CMD_BYTE1;
extern uint8_t CMD_BYTE2;
extern uint8_t ad_pCMD[2];

uint8_t ValidIspChipSize(void);
void MT9805_GlobeVaribleInit(void);
void MT9805_Init(void);
void Get_Volt(void);
void Get_Temp(void);
void testfunc(void);

#endif