#include "BatConfig.h"
#include "BQ769x2_protocol.h"

BatPackBasicInfo packInfo = {
    .cellIndex = {Cell1Voltage, Cell2Voltage, Cell3Voltage, Cell4Voltage, Cell5Voltage, Cell6Voltage, Cell15Voltage, Cell16Voltage},
		.thermistorPinIndex = {ALERTPinConfig, CFETOFFPinConfig, TS3Config, HDQPinConfig},
		.thermistorReadCommond = {CFETOFFTemperature, ALERTTemperature, TS3Temperature, HDQTemperature},
		.CUVvol = 0x31,//2479 mV CUV Threshold is this value multiplied by 50.6mV
		.COVvol = 0x4B,//3800 mV COV Threshold is this value multiplied by 50.6mV;
};
