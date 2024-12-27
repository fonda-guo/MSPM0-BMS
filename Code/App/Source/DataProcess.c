#include "DataProcess.h"
uint8_t testBuffer[10] = {0};
void Driver_I2cInit(void)
{
    DL_I2C_reset(I2C_0_INST);
    DL_I2C_enablePower(I2C_0_INST);
    SYSCFG_DL_I2C_0_init();
}
void BQDataGet(){
	  //clear buffer
	  DL_I2C_flushControllerRXFIFO(I2C_0_INST);
	  
	  PcPointBuffer[cell7Vol] = BQ769x2_ReadVoltage(Cell15Voltage);
		PcPointBuffer[maxcellvol] = PcPointBuffer[cell7Vol];
		PcPointBuffer[mincellvol] = PcPointBuffer[cell7Vol];
    for(int i = 0; i < 6; i++){
        PcPointBuffer[i] = BQ769x2_ReadVoltage(Cell1Voltage + 2*i);
			  PcPointBuffer[maxcellvol] = (PcPointBuffer[maxcellvol] > PcPointBuffer[i]) ? PcPointBuffer[maxcellvol] : PcPointBuffer[i];
			  PcPointBuffer[mincellvol] = (PcPointBuffer[mincellvol] < PcPointBuffer[i]) ? PcPointBuffer[mincellvol] : PcPointBuffer[i];
    }
		PcPointBuffer[cell8Vol] = BQ769x2_ReadVoltage(Cell16Voltage);
		PcPointBuffer[maxcellvol] = (PcPointBuffer[maxcellvol] > PcPointBuffer[cell8Vol]) ? PcPointBuffer[maxcellvol] : PcPointBuffer[cell8Vol];
		PcPointBuffer[mincellvol] = (PcPointBuffer[mincellvol] < PcPointBuffer[cell8Vol]) ? PcPointBuffer[mincellvol] : PcPointBuffer[cell8Vol];
		
		PcPointBuffer[stackVol] = BQ769x2_ReadVoltage(StackVoltage);
	  PcPointBuffer[LDpinVol] = BQ769x2_ReadVoltage(LDPinVoltage);
		BQ769x2_ReadAllTemperatures();
		for(int i = 0; i < 4; i++){
		    PcPointBuffer[ts1 + i] = (uint16_t)(Temperature[i] * 10);//float occupies 4 bytes
		}
		//current
		BQ769x2_ReadCurrent();
		memcpy(&PcPointBuffer[current],&Pack_Current,2);
}


