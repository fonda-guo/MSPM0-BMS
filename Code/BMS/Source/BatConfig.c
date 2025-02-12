#include "BatConfig.h"
#include "BQ769x2_protocol.h"

//no cur
const uint16_t NoCur_VolTable[17] = {2850, 3113, 3218, 3234, 3261, 3279, 3292, 3302, 3303, 3304, 3306, 3308, 3332, 3338, 3340, 3350, 3400};
const uint8_t NoCur_SOCRange[17] = {0, 5, 10, 15, 20, 25, 30, 40, 45, 50, 55, 60, 65, 70, 85, 95, 100};
const uint8_t NoCur_SOCErrRange[17] = {10, 10, 10, 10, 10, 10, 15, 10, 10, 10, 10, 10, 10, 15, 20, 10, 10};

//little cur

//large cur
	
//---------------------------------------------------------------------------
//SOC Calibration Data	
//---------------------------------------------------------------------------	
const uint8_t Cali_T_tbl[CALI_T_NUM] = {0, 15, 25, 35, 45};

//SOC*100
const uint16_t Cali_chg_SOC_tbl[CALI_POINT_NUM]  = {500, 2000, 6000, 9700};
const uint16_t Cali_dchg_SOC_tbl[CALI_POINT_NUM] = {500, 2000, 6500, 9900};

const uint16_t Cali_delt_chg_SOC_region[CALI_POINT_NUM]  = {100, 300, 700, 150};
const uint16_t Cali_delt_dchg_SOC_region[CALI_POINT_NUM] = {100, 300, 700, 100};

//Temperature 0 Celsius
const uint16_t Cali_T0_vol_tbl[CALI_CUR_NUM][CALI_POINT_NUM] ={{3390, 3439, 3447, 3541}, {3351, 3402, 3425, 3531}, {3287, 3353, 3389, 3495},  //chg  0.5C  0.3C  0.1C
                                                               {2996, 3163, 3240, 3274}, {2887, 3098, 3188, 3211}, {2881, 3074, 3158, 3166}}; //dchg 0.1C  0.3C  0.5C

//Temperature 15 Celsius
const uint16_t Cali_T15_vol_tbl[CALI_CUR_NUM][CALI_POINT_NUM] ={{3287, 3371, 3412, 3494}, {3282, 3347, 3392, 3467}, {3246, 3313, 3361, 3419},  //chg  0.5C  0.3C  0.1C
                                                                {3107, 3206, 3276, 3307}, {3048, 3166, 3239, 3265}, {3019, 3141, 3213, 3236}}; //dchg 0.1C  0.3C  0.5C

//Temperature 25 Celsius
const uint16_t Cali_T25_vol_tbl[CALI_CUR_NUM][CALI_POINT_NUM] ={{3262, 3352, 3400, 3467}, {3258, 3331, 3381, 3440}, {3225, 3304, 3355, 3399},  //chg  0.5C  0.3C  0.1C
                                                                {3124, 3217, 3288, 3330}, {3085, 3187, 3255, 3287}, {3058, 3163, 3232, 3260}}; //dchg 0.1C  0.3C  0.5C
																															
//Temperature 35 Celsius
const uint16_t Cali_T35_vol_tbl[CALI_CUR_NUM][CALI_POINT_NUM] ={{3229, 3334, 3389, 3444}, {3226, 3316, 3369, 3416}, {3201, 3294, 3347, 3382},  //chg  0.5C  0.3C  0.1C
                                                                {3127, 3222, 3298, 3346}, {3107, 3200, 3271, 3313}, {3088, 3179, 3249, 3283}}; //dchg 0.1C  0.3C  0.5C																															 
																															 
//Temperature 45 Celsius
const uint16_t Cali_T45_vol_tbl[CALI_CUR_NUM][CALI_POINT_NUM] ={{3217, 3325, 3383, 3425}, {3215, 3308, 3364, 3401}, {3193, 3290, 3344, 3374},  //chg  0.5C  0.3C  0.1C
                                                                {3129, 3224, 3303, 3356}, {3114, 3205, 3279, 3328}, {3098, 3187, 3259, 3305}}; //dchg 0.1C  0.3C  0.5C

//Put all of the temperature voltage data into one table																																
const uint16_t (*Cali_T_Vol_tbl[CALI_T_NUM])[CALI_POINT_NUM]	 = {Cali_T0_vol_tbl, Cali_T15_vol_tbl, Cali_T25_vol_tbl, Cali_T35_vol_tbl, Cali_T45_vol_tbl};																															
																															 
//---------------------------------------------------------------------------	
BatPackBasicInfo packInfo = {
    .cellIndex = {Cell1Voltage, Cell2Voltage, Cell3Voltage, Cell4Voltage, Cell5Voltage, Cell6Voltage, Cell15Voltage, Cell16Voltage},
		.thermistorPinIndex = {ALERTPinConfig, CFETOFFPinConfig, TS3Config, HDQPinConfig},
		.notUsedPinIndex = {TS1Config, TS2Config, DFETOFFPinConfig, DCHGPinConfig, DDSGPinConfig},
		.thermistorReadCommond = {CFETOFFTemperature, ALERTTemperature, TS3Temperature, HDQTemperature},
		.CUVvol = 0x31,//2479 mV CUV Threshold is this value multiplied by 50.6mV
		.COVvol = 0x4B,//3800 mV COV Threshold is this value multiplied by 50.6mV;
		
		//CB
		//Settings:Current Thresholds:Dsg Current Threshold is 100 by default
		//Settings:Current Thresholds:Chg Current Threshold is 50  by default
		.CB_config            = 0x13, //0b10011  no host-controlled mode, only chg & rlx
		.CB_MinTemp           = -15,
		.CB_MaxTemp           = 60,
		.CB_MaxIntTemp        = 70,
		.CB_Interval          = 6,
		.CB_MaxCells          = 4,
		.CB_MinCellVol_Chg    = 3000, //may change later
		.CB_MinDelta_Chg      = 40,
		.CB_StopDelta_Chg     = 20,
		.CB_MinCellVol_Rlx    = 3000, //may change later
		.CB_MinDelta_Rlx      = 40,
		.CB_StopDelta_Rlx     = 20,
		
		.CB_SOCLowThreshold   = 30,
		.CB_VolLowThreshold   = 3000,
		.CB_VolStartThreshold = 3200,
		.CB_VolUpperThreshold = 3650,
};

NoCurVolTable nocur_voltbl = {
    .len = sizeof(NoCur_VolTable)/sizeof(uint16_t),
	  .vol_table = NoCur_VolTable,
	  .soc_table = NoCur_SOCRange,
	  .soc_error_range = NoCur_SOCErrRange,
};