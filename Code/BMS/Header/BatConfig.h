#ifndef BAT_CONFIG_H_
#define BAT_CONFIG_H_
//This file is to contain some basic bat configuration
//To simplify and standardlize some process
#include <stdint.h>

#define  CELL_NUM         (8)
#define  THERMISTOR_NUM   (4)


typedef const struct {
	uint8_t cellIndex[CELL_NUM];
	uint16_t thermistorPinIndex[THERMISTOR_NUM];
	uint8_t thermistorReadCommond[THERMISTOR_NUM];
	uint8_t CUVvol;
	uint8_t COVvol;
} BatPackBasicInfo;

extern BatPackBasicInfo packInfo;
#endif