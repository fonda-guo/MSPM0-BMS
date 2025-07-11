#include "BatConfig.h"
#include "BQ769x2_protocol.h"
//This file is made for RPLJ battery 
//no cur
const uint16_t NoCur_chg_VolTable[14]    = {2800, 3181, 3224, 3247, 3277, 3294, 3303, 3304, 3306, 3308, 3311, 3320, 3340, 3406};
const uint8_t  NoCur_chg_SOCRange[14]    = {0, 5, 10, 15, 20, 25, 30, 40, 45, 50, 55, 60, 80, 100};
const uint8_t  NoCur_chg_SOCErrRange[14] = {10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 20, 20, 20};

const uint16_t NoCur_dchg_VolTable[14]    = {2800, 3205, 3248, 3282, 3291, 3293, 3305, 3330, 3331, 3339, 3390};
const uint8_t  NoCur_dchg_SOCRange[14]    = {0, 10, 20, 30, 40, 50, 60, 70, 80, 95, 100};
const uint8_t  NoCur_dchg_SOCErrRange[14] = {10, 10, 10, 10, 10, 10, 10, 10, 15, 15, 10};
//---------------------------------------------------------------------------
//SOC Calibration Data	
//---------------------------------------------------------------------------	
const uint16_t Cali_T_tbl[CALI_T_NUM] = {0, 150, 250, 350, 450};  // temperature * 10

//SOC*100
//soc tbl
const uint16_t Cali_chg_SOC_tbl[CALI_POINT_NUM]  = {500, 2000, 6200, 9700};
const uint16_t Cali_dchg_SOC_tbl[CALI_POINT_NUM] = {500, 2000, 6400, 9900};
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
const uint16_t Cali_T0_vol_tbl[CALI_CUR_NUM][CALI_POINT_NUM] ={{3390, 3438, 3463, 3580}, {3319, 3388, 3428, 3580}, {3271, 3345, 3389, 3472},  //chg  0.5C  0.3C  0.1C
                                                               {3070, 3180, 3248, 3304}, {2942, 3119, 3199, 3253}, {2926, 3085, 3162, 3207}}; //dchg 0.1C  0.3C  0.5C

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