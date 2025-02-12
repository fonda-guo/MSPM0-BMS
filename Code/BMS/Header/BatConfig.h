#ifndef BAT_CONFIG_H_
#define BAT_CONFIG_H_
//This file is to contain some basic bat configuration
//To simplify and standardlize some process
#include <stdint.h>

#define  CELL_NUM         (8)
#define  THERMISTOR_NUM   (4)
#define  NOT_USED_PIN_NUM (5)

#define  CALI_POINT_NUM   (4)
#define  CALI_T_NUM       (5)
#define  CALI_CUR_NUM     (6) //chg/dchg 0.1C 0.3C 0.5C

typedef const struct {
	//Basic
	uint8_t cellIndex[CELL_NUM];
	uint16_t thermistorPinIndex[THERMISTOR_NUM];
	uint16_t notUsedPinIndex[NOT_USED_PIN_NUM];
	uint8_t thermistorReadCommond[THERMISTOR_NUM];
	uint8_t CUVvol;
	uint8_t COVvol;
	//CB
	uint8_t  CB_config;
	int8_t   CB_MinTemp;
  int8_t   CB_MaxTemp;
	int8_t   CB_MaxIntTemp;
	uint8_t  CB_Interval;
	uint8_t  CB_MaxCells;
	uint16_t CB_MinCellVol_Chg;
	uint8_t  CB_MinDelta_Chg;
	uint8_t  CB_StopDelta_Chg;
	uint16_t CB_MinCellVol_Rlx;
	uint8_t  CB_MinDelta_Rlx;
	uint8_t  CB_StopDelta_Rlx;
	
	
	uint8_t  CB_SOCLowThreshold;
	uint16_t CB_VolLowThreshold;
	uint16_t CB_VolStartThreshold;
	uint16_t CB_VolUpperThreshold;
	
	
} BatPackBasicInfo;

typedef struct {
  const uint8_t  len;
	const uint16_t* vol_table;
	const uint8_t*  soc_table;
	const uint8_t*  soc_error_range;
} NoCurVolTable;


extern NoCurVolTable      nocur_voltbl;
extern BatPackBasicInfo   packInfo;
#endif