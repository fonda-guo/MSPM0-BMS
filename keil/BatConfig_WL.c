#include "BatConfig.h"
#include "BQ769x2_protocol.h"

//This file is made for WL battery!

//no cur
const uint16_t NoCur_chg_VolTable[8]    = {2800, 3222, 3279, 3303, 3308, 3334, 3340, 3400};
const uint8_t  NoCur_chg_SOCRange[8]    = {0, 10, 20, 30, 50, 60, 90, 100};
const uint8_t  NoCur_chg_SOCErrRange[8] = {10, 10, 10, 20, 20, 20, 20, 10};

const uint16_t NoCur_dchg_VolTable[8]    = {2800, 3180, 3228, 3270, 3289, 3295, 3330, 3400};
const uint8_t  NoCur_dchg_SOCRange[8]    = {0, 10, 20, 30, 40, 60, 90, 100};
const uint8_t  NoCur_dchg_SOCErrRange[8] = {10, 10, 10, 10, 20, 20, 20, 10};
//---------------------------------------------------------------------------
//SOC Calibration Data	
//---------------------------------------------------------------------------	
const uint16_t Cali_T_tbl[CALI_T_NUM] = {0, 150, 250, 350, 450};  // temperature * 10

//SOC*100
//soc tbl
const uint16_t Cali_chg_SOC_tbl[CALI_POINT_NUM]  = {500, 2000, 6000, 9700};
const uint16_t Cali_dchg_SOC_tbl[CALI_POINT_NUM] = {500, 2000, 6500, 9900};
//soc error tbl
const uint16_t Cali_delt_chg_SOC_region[CALI_POINT_NUM]  = {100, 300, 700, 150};
const uint16_t Cali_delt_dchg_SOC_region[CALI_POINT_NUM] = {100, 300, 700, 100};
//test
//const uint16_t Cali_delt_chg_SOC_region[CALI_POINT_NUM]  = {10, 10, 10, 10};
//const uint16_t Cali_delt_dchg_SOC_region[CALI_POINT_NUM] = {10, 10, 10, 10};

//k tbl * 10
const uint16_t Cali_K_dchg_Higher_SOC[CALI_POINT_NUM]  = {200, 50, 30, 15};
const uint16_t Cali_K_dchg_Lower_SOC[CALI_POINT_NUM]   = {1, 2, 3, 8};

const uint16_t Cali_K_chg_Higher_SOC[CALI_POINT_NUM]  = {8, 3, 2, 1};
const uint16_t Cali_K_chg_Lower_SOC[CALI_POINT_NUM]   = {15, 30, 50, 200};

//Temperature 0 Celsius
const uint16_t Cali_T0_vol_tbl[CALI_CUR_NUM][CALI_POINT_NUM] ={{3390, 3439, 3447, 3541}, {3351, 3402, 3425, 3531}, {3287, 3353, 3389, 3495},  //chg  0.5C  0.3C  0.1C
                                                               {2996, 3163, 3240, 3274}, {2887, 3098, 3188, 3211}, {2881, 3074, 3158, 3166}}; //dchg 0.1C  0.3C  0.5C

//Temperature 15 Celsius
const uint16_t Cali_T15_vol_tbl[CALI_CUR_NUM][CALI_POINT_NUM] ={{3297, 3379, 3433, 3554}, {3273, 3344, 3401, 3463}, {3235, 3310, 3368, 3407},  //chg  0.5C  0.3C  0.1C
                                                                {3125, 3208, 3274, 3329}, {3077, 3167, 3236, 3294}, {3041, 3133, 3203, 3257}}; //dchg 0.1C  0.3C  0.5C

//Temperature 25 Celsius
const uint16_t Cali_T25_vol_tbl[CALI_CUR_NUM][CALI_POINT_NUM] ={{3275, 3355, 3412, 3477}, {3244, 3326, 3386, 3429}, {3215, 3299, 3358, 3390},  //chg  0.5C  0.3C  0.1C
                                                                {3135, 3217, 3287, 3339}, {3108, 3189, 3257, 3315}, {3079, 3162, 3231, 3289}}; //dchg 0.1C  0.3C  0.5C
																															
//Temperature 35 Celsius
const uint16_t Cali_T35_vol_tbl[CALI_CUR_NUM][CALI_POINT_NUM] ={{3254, 3343, 3406, 3450}, {3234, 3319, 3381, 3417}, {3207, 3295, 3354, 3382},  //chg  0.5C  0.3C  0.1C
                                                                {3135, 3219, 3292, 3345}, {3114, 3195, 3263, 3323}, {3089, 3169, 3238, 3300}}; //dchg 0.1C  0.3C  0.5C																															 
																															 
//Temperature 45 Celsius
const uint16_t Cali_T45_vol_tbl[CALI_CUR_NUM][CALI_POINT_NUM] ={{3251, 3341, 3405, 3443}, {3228, 3316, 3377, 3409}, {3202, 3292, 3352, 3377},  //chg  0.5C  0.3C  0.1C
                                                                {3134, 3219, 3296, 3348}, {3114, 3196, 3267, 3327}, {3089, 3168, 3239, 3302}}; //dchg 0.1C  0.3C  0.5C

//Put all of the temperature voltage data into one table																																
const uint16_t (*Cali_T_Vol_tbl[CALI_T_NUM])[CALI_POINT_NUM]	 = {Cali_T0_vol_tbl, Cali_T15_vol_tbl, Cali_T25_vol_tbl, Cali_T35_vol_tbl, Cali_T45_vol_tbl};																															
																															 
//---------------------------------------------------------------------------	
BatPackBasicInfo packInfo = {
    .cellIndex             = {Cell1Voltage, Cell2Voltage, Cell3Voltage, Cell4Voltage, Cell5Voltage, Cell6Voltage, Cell7Voltage, Cell16Voltage},
		.thermistorPinIndex    = {DDSGPinConfig, DCHGPinConfig, DFETOFFPinConfig, ALERTPinConfig, TS3Config, TS1Config, CFETOFFPinConfig},
		.notUsedPinIndex       = {TS2Config, HDQPinConfig},//will be set to 0x0000
		.thermistorReadCommond = {DDSGTemperature, DCHGTemperature, DFETOFFTemperature, ALERTTemperature, TS3Temperature, TS1Temperature, CFETOFFTemperature},
		.Tmap_ther2bat         = {ts1,ts1,ts2,ts2,ts3,ts3,ts4,ts4},
		.CUVvol = 0x27,//1973 mV CUV Threshold is this value multiplied by 50.6mV;
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
		
		//default data, may not initialize in the function to save some time
		.Sleep_Current        = 20,
		.Sleep_VoltageTime    = 5,
		.Sleep_WakeComparatorCurrent = 500,
		.Sleep_HysteresisTime = 10,
		.Sleep_ChargerVoltageThreshold = 2000,
		.Sleep_ChargerPACK_TOSDelta = 200,
};

NoCurVolTable nocur_voltbl = {
    .chg_len = sizeof(NoCur_chg_VolTable)/sizeof(uint16_t),
	  .chg_vol_table = NoCur_chg_VolTable,
	  .chg_soc_table = NoCur_chg_SOCRange,
	  .chg_soc_error_range = NoCur_chg_SOCErrRange,
		
		.dchg_len = sizeof(NoCur_dchg_VolTable)/sizeof(uint16_t),
	  .dchg_vol_table = NoCur_dchg_VolTable,
	  .dchg_soc_table = NoCur_dchg_SOCRange,
	  .dchg_soc_error_range = NoCur_dchg_SOCErrRange,
};