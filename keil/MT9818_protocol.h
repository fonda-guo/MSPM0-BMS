#include "I2C_comm.h"

#define	SYS_STAT_REG			0x00
#define	SYS_CTRL1_REG			0x04
#define	SYS_CTRL2_REG			0x05
#define	PROTECT1_REG			0x06
#define	PROTECT2_REG			0x07
#define	PROTECT3_REG			0x08
#define	OV_TRIP_REG				0x09
#define	UV_TRIP_REG				0x0a
#define VC1_ADDRESS				0x0c
#define CELLBAL1					0x01
#define CELLBAL2					0x02
#define CELLBAL3					0x03

#define OVP_VALUE						3600					//mv
#define UVP_VALUE						3000//1900		//mv
#define RSNS					    1

typedef enum
{
	OV_DELAY_ONE_SECOND = 1,
	OV_DELAY_TWO_SECOND = 2,
	OV_DELAY_FOUR_SECOND = 4,
	OV_DELAY_EIGHT_SECOND = 8
}OV_DELAY;

typedef enum
{
	UV_DELAY_ONE_SECOND = 1,
	UV_DELAY_FOUR_SECOND = 4,
	UV_DELAY_EIGHT_SECOND = 8,
	UV_DELAY_SIXTEEN_SECOND = 16
}UV_DELAY;


typedef enum
{
	OCD_DELAY_8MS = 0,
	OCD_DELAY_20MS = 1,
	OCD_DELAY_40MS = 2,
	OCD_DELAY_80MS = 3,
	OCD_DELAY_160MS = 4,
	OCD_DELAY_320MS = 5,
	OCD_DELAY_640MS = 6,
	OCD_DELAY_1280MS = 7
}OCD_DELAY;


typedef enum
{
	SCD_DELAY_70US = 0,
	SCD_DELAY_100US = 1,
	SCD_DELAY_200US = 2,
	SCD_DELAY_400US = 3
}SCD_DELAY;

typedef enum
{
	#if RSNS
	SCD_THRESH_50MV = 0,
	SCD_THRESH_70MV = 1,
	SCD_THRESH_90MV = 2,
	SCD_THRESH_110MV = 3,
	SCD_THRESH_130MV = 4,
	SCD_THRESH_150MV = 5,
	SCD_THRESH_170MV = 6,
	SCD_THRESH_190MV = 7
	#else
	SCD_THRESH_25MV = 0,
	SCD_THRESH_35MV = 1,
	SCD_THRESH_45MV = 2,
	SCD_THRESH_55MV = 3,
	SCD_THRESH_65MV = 4,
	SCD_THRESH_75MV = 5,
	SCD_THRESH_85MV = 6,
	SCD_THRESH_95MV = 7
	#endif
}SCD_THRESH;

typedef enum
{
	#if RSNS
	OCD_THRESH_20MV = 0,
	OCD_THRESH_25MV = 1,
	OCD_THRESH_30MV = 2,
	OCD_THRESH_35MV = 3,
	OCD_THRESH_40MV = 4,
	OCD_THRESH_45MV = 5,
	OCD_THRESH_50MV = 6,
	OCD_THRESH_55MV = 7,
	OCD_THRESH_60MV = 8,
	OCD_THRESH_65MV = 9,
	OCD_THRESH_70MV = 10,
	OCD_THRESH_75MV = 11,
	OCD_THRESH_80MV = 12,
	OCD_THRESH_85MV = 13,
	OCD_THRESH_90MV = 14,
	OCD_THRESH_95MV = 15
	#else
	OCD_THRESH_10MV = 0,
	OCD_THRESH_12P5MV = 1,
	OCD_THRESH_15MV = 2,
	OCD_THRESH_17P5MV = 3,
	OCD_THRESH_20MV = 4,
	OCD_THRESH_22P5MV = 5,
	OCD_THRESH_25MV = 6,
	OCD_THRESH_27P5MV = 7,
	OCD_THRESH_30MV = 8,
	OCD_THRESH_32P5MV = 9,
	OCD_THRESH_35MV = 10,
	OCD_THRESH_37P5MV = 11,
	OCD_THRESH_40MV = 12,
	OCD_THRESH_42P5MV = 13,
	OCD_THRESH_45MV = 14,
	OCD_THRESH_47P5MV = 15
	#endif
}OCD_THRESH;


typedef struct
{
	uint16_t Volt[18];		
	uint16_t Temp[4];			//offset -40
	uint32_t TotalVolt;		    //mv
//	int16_t adc_current;	    //adc?
//	int16_t adc_current_cal;	//adc???
	float Current;
	float current_adc;
	float current_filter[10];
	int8_t current_filter_count;
}MT9818_Data_Struct;

void MT9818_Parameter_Init(void);
void Get_All_Vol(void);
float Get_BAT_ADC_GAIN(void);
float Get_BAT_ADC_GAIN1(void);
float Get_BAT_ADC_GAIN2(void);
float Get_BAT_ADC_GAIN3(void);