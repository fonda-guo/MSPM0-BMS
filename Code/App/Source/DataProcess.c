#include "DataProcess.h"
uint8_t testBuffer[10] = {0};
int16_t test = 0;
void Driver_I2cInit(void)
{
    DL_I2C_reset(I2C_0_INST);
    DL_I2C_enablePower(I2C_0_INST);
    SYSCFG_DL_I2C_0_init();
}
void BQDataGet(){
	  DL_I2C_flushControllerRXFIFO(I2C_0_INST);
	
    for(int i = 0; i < 16; i++){
        PcPointBuffer[i] = BQ769x2_ReadVoltage(Cell1Voltage + 2*i);
    }
		
		PcPointBuffer[stackVol] = BQ769x2_ReadVoltage(StackVoltage);
	  PcPointBuffer[LDpinVol] = BQ769x2_ReadVoltage(LDPinVoltage);
		BQ769x2_ReadAllTemperatures();
		for(int i = 0; i < 4; i++){
		    PcPointBuffer[ts1 + i] = (uint16_t)(Temperature[i] * 10);//float occupies 4 bytes
		}
		BQ769x2_ReadCurrent();
		memcpy(&PcPointBuffer[current],&Pack_Current,2);
		//PcPointBuffer[current] = (uint16_t)Pack_Current;
		test = sizeof(Pack_Current);
}


