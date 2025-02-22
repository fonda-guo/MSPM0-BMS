#include "DataProcess.h"
uint8_t testBuffer[10] = {0};
void Driver_I2cInit(void)
{
    DL_I2C_reset(I2C_0_INST);
    DL_I2C_enablePower(I2C_0_INST);
    SYSCFG_DL_I2C_0_init();
}

void CellBalanceNormalTask(){
    //Cell Balance
		PcPointBuffer[CB_bits] = CellBalanceStatusGet();
		if(PcPointBuffer[CB_bits]) cellBalance.statusCB |= CB_ON;
		
		//CB check forbide bit
	  if(PcPointBuffer[controlBits] & CB_STATUS_BIT){
	    cellBalance.statusCB |= CB_FORBIDDEN;
	  }else{
	    cellBalance.statusCB &= (~CB_FORBIDDEN);
	  }
		
	  //turn off
	  if(cellBalance.statusCB & CB_ON){
			//forbide or turn off
	    if((cellBalance.statusCB & CB_FORBIDDEN) || (cellBalance.statusCB & CB_TURN_OFF)){
			  CloseCellBalance();
		    cellBalance.statusCB &= (~CB_ON);
			}
	  }else{
			//not forbide and turn on
			if(!(cellBalance.statusCB & CB_FORBIDDEN) && (cellBalance.statusCB & CB_TURN_ON)){
			  OpenCellBalance(&packInfo);
		    cellBalance.statusCB |= CB_ON;
			}
		}
		//clear action bit
		cellBalance.statusCB &= (~CB_TURN_ON);
		cellBalance.statusCB &= (~CB_TURN_OFF);

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
		    PcPointBuffer[ts1 + i] = (int16_t)(Temperature[i] * 10);//float occupies 4 bytes
		}
		//current
		BQ769x2_ReadCurrent();
		//memcpy(&PcPointBuffer[current],&Pack_Current,2);
    CellBalanceNormalTask();
}


