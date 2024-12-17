#include "FaultProtect.h"
double res = 0;

void FaultProtect_init(){
}

void FaultDetect(){
  if(I2C_dead == 1){
		  PcPointBuffer[fault] = I2C_dead;
	    BQ769x2_Init();
	}else{
		  PcPointBuffer[fault] = 0;
	}
	BQ769x2_ReadAlarmStatus();
	BQ769x2_ReadSafetyStatus();
	BQ769x2_ReadFETStatus();
	PcPointBuffer[alarmBits] = AlarmBits;
	PcPointBuffer[safetyStatusA] = value_SafetyStatusA;
	PcPointBuffer[safetyStatusB] = value_SafetyStatusB;
	PcPointBuffer[safetyStatusC] = value_SafetyStatusC;
	PcPointBuffer[fet_Status] = FET_Status;
	
}