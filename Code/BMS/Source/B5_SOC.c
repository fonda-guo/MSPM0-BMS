/****************************************************************/
/**
 * @file           B5_SOC.c
 *----------------------------------------------------------------
 * @brief
 *----------------------------------------------------------------
 * @note This file aims to calculate the SOC of G2 Battery.
 *       Some basic notices are attention-needed.
 *       Charge capcity is taken as 280Ah
 *       Calculation is executed every 500ms
 *       
 *----------------------------------------------------------------
 * @author         Fonda.Guo
 * @date           2024/12/20
 * @version        0.0
 *----------------------------------------------------------------
 * @par Project    B5
 * @par Compiler   TI v20.2.4.LTS
 * @par Processor  TMS320F280025
 *----------------------------------------------------------------
 * @copyright Copyright (c) 2021 Star Charge (Hangzhou).
 *----------------------------------------------------------------
 * @par History
 * <table>
 * <tr><th>Date <th>Version <th>Author    <th>Change List
 * <tr><td>2024/10/8 <td>0.0 <td>Fonda <td>first created
 * </table>
 *
 */
/****************************************************************/

//---------------------------------------------------------------------------
// Included header
//---------------------------------------------------------------------------
#define EXPORT_B5_SOC
#include "B5_SOC.h"
#undef EXPORT_B5_SOC

#include <string.h>
//---------------------------------------------------------------------------
// Private functions
//---------------------------------------------------------------------------
void Read32Flash(uint32_t* des, uint32_t addr);
void Write32Flash(uint32_t* datapointer, uint32_t addr);
//---------------------------------------------------------------------------
// Private variables
//---------------------------------------------------------------------------
//leave for EEPROM write
uint32_t BoxSOCEEBuffer                = 0;
uint32_t BoxCoulombTotalEEBuffer       = 0;
uint16_t BoxTemperatureEEBuffer        = 0;
uint32_t SingleBatCoulombTotalEEBuffer = 0;
uint32_t SingleBatSOCEEReadBuffer[CELL_NUM]   = {0};
uint32_t SingleBatSOCEEWriteBuffer[CELL_NUM]  = {0};
uint16_t SingleBatTempEEWriteBuffer[CELL_NUM] = {0};
//write EE interval
volatile uint16_t writeEEcnt = 0;
//dC is important
int32_t dC = 0;


const PC_point BatCommPoint[CELL_NUM] = { SOC_cell1, SOC_cell2, SOC_cell3, SOC_cell4,
                                          SOC_cell5, SOC_cell6, SOC_cell7, SOC_cell8};

void BMSInit(void){
    boxBMS.status                  = (CYCLE_CALIB_ENABLE | START_UP_PROCESS);
    boxBMS.BoxSOCShow              = MAX_SOC;
    boxBMS.BoxSOHShow              = MAX_SOH;
    boxBMS.BoxCoulombCounter       = MAX_BAT_CAP;
    boxBMS.BoxCoulombTotal         = MAX_BAT_CAP;
    boxBMS.BoxSOCCal               = MAX_BAT_CAP;
    boxBMS.BoxCoulombCounterCali   = 0;
    
    boxInfo.MaxBatVol              = 0;
    boxInfo.MinBatVol              = 0;
    boxInfo.MaxBatTemp             = 0;
    boxInfo.MaxBatSOC              = 0;
    boxInfo.MinBatSOC              = 0;
    boxInfo.SingleBatCoulombTotal  = MAX_BAT_CAP;    
    boxInfo.DataCheck              = 0;

    memset(batBMS,0,sizeof(batBMS));

    dC = 0;
}
/***************************************************************************************/
/*
 * Function              BMSTask
 * @param[in]            none
 * @return               none
 * \brief                BMS function runs every 3.75ms
 *///
/***************************************************************************************/
void BMSTask(void){
    //Start up process(run only once): read eeprom, check & initialize import value
    BMSStartUp();
    //Get voltage & temperature
    BMSBasicDataGet();
    //Calculation task every 300 ms
    BMSCalTask();
    //Check & write EE after cycle
    BMSCoulombTotalUpdate();  
    //except CoulombTotal, INFO needs to be stored in EE will execute here
    BMSWriteEETask();
}
/***************************************************************************************/
/*
 * Function              BMSStartUp
 * @param[in]            none
 * @return               none
 * \brief                Start up process(run only once):
                           read eeprom, check & initialize import value
 *///
/***************************************************************************************/
void BMSStartUp(void){
    if((boxBMS.status&START_UP_PROCESS) == 0) return;
    //Single Battery done first, box will use these infomation
    //BOX Info Initial
    Read32Flash(&boxInfo.SingleBatCoulombTotal, E2_SINGLEBATCOULOMBTOTAL_ADDR);
    if(boxInfo.SingleBatCoulombTotal > MAX_BAT_CAP || boxInfo.SingleBatCoulombTotal < BAT_CAP_QUARTER){
        boxInfo.SingleBatCoulombTotal = BAT_CAP_NINTY_PER;
    }

    //Bat E2 Initial
    uint16_t index = 0;
    for(index = 0; index < CELL_NUM; index++){
        batBMS[index].BatVol   = PcPointBuffer[cell1Vol + index];
        batBMS[index].BatTemp  = PcPointBuffer[ts1 + index/2]; //ts1-> cell1,2;ts2->cell3,4
			  Read32Flash(&SingleBatSOCEEReadBuffer[index], E2_SINGLE_BAT_SOC_ADDR + index*8);
        batBMS[index].BatCoulombSOC = BatSOCVolInitEstimate(batBMS[index].BatVol,batBMS[index].BatTemp,SingleBatSOCEEReadBuffer[index]);
    }
		
    //BOX E2 Initial
	  Read32Flash(&boxBMS.BoxCoulombTotal, E2_COULOMBTOTAL_ADDR);
	  Read32Flash(&boxBMS.BoxSOCCal, E2_BOXSOCCAL_ADDR);
    //check if total capcity is resonable
    if(boxBMS.BoxCoulombTotal > MAX_BAT_CAP || boxBMS.BoxCoulombTotal < BAT_CAP_QUARTER){
        boxBMS.BoxCoulombTotal = BAT_CAP_NINTY_PER;
    }
    //initial value
		if(boxBMS.BoxSOCCal > MAX_BAT_CAP){
			  boxBMS.BoxCoulombCounter = BAT_CAP_QUARTER;
    }else{
		    boxBMS.BoxCoulombCounter = boxBMS.BoxSOCCal;
		}
    memset(SingleBatSOCEEReadBuffer,0,sizeof(SingleBatSOCEEReadBuffer));
    boxBMS.status &= (~START_UP_PROCESS);
    return;
}
/***************************************************************************************/
/*
 * Function              
 * @param[in]            none
 * @return               none
 * \brief                
 *///
/***************************************************************************************/
void BMSBasicDataGet(void){
    uint16_t index = 0;
    for(index = 0; index < CELL_NUM; index++){
        batBMS[index].BatVol   = PcPointBuffer[cell1Vol + index];
        batBMS[index].BatTemp  = PcPointBuffer[ts1 + index/2];
        
        boxInfo.MaxBatVol = PcPointBuffer[maxcellvol];
        boxInfo.MinBatVol = PcPointBuffer[mincellvol];

        boxBMS.temperature += batBMS[index].BatTemp;
    }
        boxBMS.temperature /= CELL_NUM;
    return;
}
/***************************************************************************************/
/*
 * Function              
 * @param[in]            none
 * @return               none
 * \brief                
 *///
/***************************************************************************************/
void BMSCalTask(void){
    int16_t I = PcPointBuffer[current];//uint16->int16
    //every 500ms
    I = (int16_t)((float)I / 25 * 10);//25 is the shunt ratio,10 times of current
	
    dC = I * -5;//5 stands for 500ms

    DataCheck();

    TemperatureCali();

    //VolCali();

    SingleBatSOCupdate();

    BoxCoulombCount();

    BoxSOCUpdate();
}
/***************************************************************************************/
/*
 * Function              
 * @param[in]            none
 * @return               none
 * \brief                
 *///
/***************************************************************************************/
void BMSSingleBatVolCheck(void){

    if(PcPointBuffer[maxcellvol] >= VOL_UPPER_BOUND){
        boxBMS.status |= REACH_BAT_UPPER_BOUND;
    }

    if(PcPointBuffer[mincellvol] <= VOL_LOWER_BOUND && PcPointBuffer[mincellvol] != 0){
        boxBMS.status |= REACH_BAT_LOWER_BOUND;
    }
    return;
}

/***************************************************************************************/
/*
 * Function              
 * @param[in]            none
 * @return               none
 * \brief                
 *///
/***************************************************************************************/
void BMSCoulombTotalUpdate(void){
    if((boxBMS.status & CYCLE_CALIB_ENABLE) == 0) return;
    if((boxBMS.status & CYCLE_WRITE_EE) == 0)     return;
 
	  if((boxBMS.status & REACH_BAT_UPPER_BOUND) != 0){
		    boxBMS.BoxCoulombCounterCali -= (boxBMS.BoxCoulombTotal - boxBMS.BoxCoulombCounter);
			  boxBMS.BoxCoulombTotal += boxBMS.BoxCoulombCounterCali;
			  boxInfo.SingleBatCoulombTotal = boxBMS.BoxCoulombTotal;//to be done
			  boxBMS.BoxCoulombCounter = boxBMS.BoxCoulombTotal; //make sure SOC is full, do this after calibrating the SOH
			  SingleBatSOCCoulombFull();
			  boxBMS.BoxSOCCal = boxBMS.BoxCoulombTotal;
		}
	  if((boxBMS.status & REACH_BAT_LOWER_BOUND) != 0){
			  boxBMS.BoxCoulombCounterCali -= boxBMS.BoxCoulombCounter;
        boxBMS.BoxCoulombCounter = 0;
        SingleBatSOCCoulombClear();
			  boxBMS.BoxCoulombTotal += boxBMS.BoxCoulombCounterCali;
        boxInfo.SingleBatCoulombTotal = boxBMS.BoxCoulombTotal;//to be done
		}
    boxBMS.status &= (~CYCLE_WRITE_EE);//only success write can clear the bit, or do this task again
    boxBMS.BoxCoulombCounterCali = 0;
    return;
}
/***************************************************************************************/
/*
 * Function              
 * @param[in]            none
 * @return               none
 * \brief                EE write task will take a long time, make sure every write is seperated by at least 5ms
 *///
/***************************************************************************************/
void BMSWriteEETask(void){
	  //storage SOC 1 min = 120 * 0.5s 
    writeEEcnt++;
	  if(writeEEcnt == 119){
			  //erase this sector before writing into it
			  DL_FlashCTL_unprotectSector(FLASHCTL, E2_COULOMBTOTAL_ADDR, DL_FLASHCTL_REGION_SELECT_MAIN);
	      DL_FlashCTL_eraseMemoryFromRAM(FLASHCTL, E2_COULOMBTOTAL_ADDR, DL_FLASHCTL_COMMAND_SIZE_SECTOR);
			  Write32Flash(&BoxSOCEEBuffer, E2_BOXSOCCAL_ADDR);
		}else if(writeEEcnt >= 120 && writeEEcnt <= 127){
		    Write32Flash(&SingleBatSOCEEWriteBuffer[writeEEcnt - 120], E2_SINGLE_BAT_SOC_ADDR + 8 * (writeEEcnt - 120));
		}else if(writeEEcnt == 128){			   
        Write32Flash(&boxBMS.BoxCoulombTotal,E2_COULOMBTOTAL_ADDR);
		}else if(writeEEcnt > 128){
		    writeEEcnt = 0;
		}
}
//To be done
uint32_t BatSOCVolInitEstimate(uint16_t vol, uint16_t temperature, uint32_t coulombSOC){
  if(coulombSOC > MAX_BAT_CAP)   coulombSOC = BAT_CAP_QUARTER;
	
	return coulombSOC;
}

void DataCheck(void){
    
}
void VolCali(void){

}
/***************************************************************************************/
/*
 * Function              
 * @param[in]            none
 * @return               none
 * \brief                
 *///
/***************************************************************************************/
void TemperatureCali(void){
    
}
/***************************************************************************************/
/*
 * Function              
 * @param[in]            none
 * @return               none
 * \brief                
 *///
/***************************************************************************************/
void SingleBatSOCupdate(void){
    uint16_t batIndex = 0;
    //clear buffer
    memset(SingleBatSOCEEWriteBuffer, 0, sizeof(SingleBatSOCEEWriteBuffer));
    for(batIndex = 0; batIndex < CELL_NUM; batIndex++){
        SingleBatSOCCal(batIndex);
    }
    return;
}
/***************************************************************************************/
/*
 * Function              
 * @param[in]            none
 * @return               none
 * \brief                
 *///
/***************************************************************************************/
void SingleBatSOCCal(uint16_t batIndex){
    if(dC > 0){
        //Discahrge
        if(batBMS[batIndex].BatCoulombSOC > dC){
            batBMS[batIndex].BatCoulombSOC -= dC;
        }else{
            //Still remain, single bat needs no calibration
            batBMS[batIndex].BatCoulombSOC = 0;
        }
    }else{
        //Charge
        if(batBMS[batIndex].BatCoulombSOC < (boxInfo.SingleBatCoulombTotal + dC)){
            batBMS[batIndex].BatCoulombSOC -= dC;
        }else{
            //Still space left
            batBMS[batIndex].BatCoulombSOC = boxInfo.SingleBatCoulombTotal;
        }
    }

    if(boxInfo.SingleBatCoulombTotal == 0){
        PcPointBuffer[BatCommPoint[batIndex]] = 1234;
    }else{
        PcPointBuffer[BatCommPoint[batIndex]] = (uint16_t)(((float)batBMS[batIndex].BatCoulombSOC/boxInfo.SingleBatCoulombTotal) * 10000);
    }
    
    SingleBatSOCEEWriteBuffer[batIndex]  = batBMS[batIndex].BatCoulombSOC;
    SingleBatTempEEWriteBuffer[batIndex] = batBMS[batIndex].BatTemp;
    
    return;
}

/***************************************************************************************/
/*
 * Function              
 * @param[in]            none
 * @return               none
 * \brief                
 *///
/***************************************************************************************/
void BoxCoulombCount(void){
    if(dC > 0){
            //Discahrge
            if(boxBMS.BoxCoulombCounter > dC){
                boxBMS.BoxCoulombCounter -= dC;
            }else{
                //Still remain 
                boxBMS.BoxCoulombCounterCali += (dC - boxBMS.BoxCoulombCounter);
                boxBMS.BoxCoulombCounter = 0;
            }
    }else{
            //Charge
            if(boxBMS.BoxCoulombCounter < (boxBMS.BoxCoulombTotal + dC)){
                boxBMS.BoxCoulombCounter -= dC;
            }else{
                //Still space left
                boxBMS.BoxCoulombCounterCali += (boxBMS.BoxCoulombCounter - dC - boxBMS.BoxCoulombTotal);
                boxBMS.BoxCoulombCounter = boxBMS.BoxCoulombTotal;
            }
    }
    if(boxBMS.BoxCoulombTotal == 0){
		    PcPointBuffer[SOC_box] = 1234;
    }else{
        PcPointBuffer[SOC_box] = (uint16_t)(((float)boxBMS.BoxCoulombCounter/boxBMS.BoxCoulombTotal) * 10000);
    }
}

/***************************************************************************************/
/*
 * Function              
 * @param[in]            none
 * @return               none
 * \brief                
 *///
/***************************************************************************************/
void BoxSOCUpdate(void){
    boxBMS.BoxSOCCal = boxBMS.BoxCoulombCounter;
	  BoxSOCEEBuffer = boxBMS.BoxSOCCal;
}
//used for now, a more accurate calibration method is needed later 
void SingleBatSOCCoulombClear(void){
    uint16_t batIndex = 0;
    for(batIndex = 0; batIndex < CELL_NUM; batIndex++){
        batBMS[batIndex].BatCoulombSOC = 0;
    }
}
//used for now, a more accurate calibration method is needed later 
void SingleBatSOCCoulombFull(void){
    uint16_t batIndex = 0;
    for(batIndex = 0; batIndex < CELL_NUM; batIndex++){
        batBMS[batIndex].BatCoulombSOC = boxInfo.SingleBatCoulombTotal;
    }
}

/***************************************************************************************/
/*
 * Function              Read32Flash
 * @param[in]            destination varaible and address in flash
 * @return               none
 * \brief                
 *///
/***************************************************************************************/
void Read32Flash(uint32_t* des, uint32_t addr){
    uint32_t *p = 0;
	  p = (void *)addr;
	  *des = *p;
}
/***************************************************************************************/
/*
 * Function              Read32Flash
 * @param[in]            destination varaible and address in flash
 * @return               none
 * \brief                
 *///
/***************************************************************************************/
void Write32Flash(uint32_t* datapointer, uint32_t addr){
	  //if you erase, the whole sector will be empty
    //DL_FlashCTL_unprotectSector(FLASHCTL, addr, DL_FLASHCTL_REGION_SELECT_MAIN);
	  //DL_FlashCTL_eraseMemoryFromRAM(FLASHCTL, addr, DL_FLASHCTL_COMMAND_SIZE_SECTOR);
	  DL_FlashCTL_unprotectSector(FLASHCTL, addr, DL_FLASHCTL_REGION_SELECT_MAIN);
    DL_FlashCTL_programMemory32WithECCGenerated(FLASHCTL, addr, datapointer);
}
