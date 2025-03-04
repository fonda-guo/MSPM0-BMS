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
 * @par Project    G2
 * @par Compiler   ARM
 * @par Processor  STMH5
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
    boxBMS.BoxCoulombCounter       = NOMINAL_BAT_CAP;
    boxBMS.BoxCoulombTotal         = NOMINAL_BAT_CAP;
    boxBMS.BoxSOCCal               = NOMINAL_BAT_CAP;
    boxBMS.BoxCoulombCounterCali   = 0;
    
    boxInfo.MaxBatVol              = 0;
    boxInfo.MinBatVol              = 0;
    boxInfo.MaxBatTemp             = 0;
    boxInfo.MaxBatSOC              = 0;
    boxInfo.MinBatSOC              = 0;
    boxInfo.SingleBatCoulombTotal  = NOMINAL_BAT_CAP;    
    boxInfo.DataCheck              = 0;

    memset(batBMS,0,sizeof(batBMS));
	  
	  memset(&cellR,0,sizeof(cellR));
    cellR.statusR = INIT_R_CAL;

    memset(&cellBalance,0,sizeof(cellBalance));
		cellBalance.statusCB = CB_ON;
	
	  memset(&chgdchgStatus,0,sizeof(chgdchgStatus));
		
		boxSOH.absCoulombCnter = 0;
		boxSOH.alreadyLost = 0;
		boxSOH.totalLifeCap = TOTAL_LIFE_CAP;
    dC = 0;
}
/***************************************************************************************/
/*
 * Function              BMSTask
 * @param[in]            none
 * @return               none
 * \brief                BMS function runs every 500ms
 *///
/***************************************************************************************/
void BMSTask(void){
    //Start up process(run only once): read eeprom, check & initialize import value
    BMSStartUp();
    //Get voltage & temperature
    BMSBasicDataGet();
    //Calculation task every 500 ms
    BMSCalTask();
    //Check & write EE after cycle
    BMSCoulombTotalFinalUpdate();  
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
		//Init
		boxInfo.MaxBatSOC = 0;
		boxInfo.MinBatSOC = boxInfo.SingleBatCoulombTotal;

    //Bat E2 Initial
    uint16_t index = 0;
    for(index = 0; index < CELL_NUM; index++){
        batBMS[index].BatVol   = PcPointBuffer[cell1Vol + index];
        batBMS[index].BatTemp  = PcPointBuffer[packInfo.Tmap_ther2bat[index]]; //ts1-> cell1,2;ts2->cell3,4
			  Read32Flash(&SingleBatSOCEEReadBuffer[index], E2_SINGLE_BAT_SOC_ADDR + index*8);
        batBMS[index].BatSOCCal = BatSOCVolInitEstimate(batBMS[index].BatVol,batBMS[index].BatTemp,SingleBatSOCEEReadBuffer[index]);
			  boxInfo.MaxBatSOC = (boxInfo.MaxBatSOC > batBMS[index].BatSOCCal) ? boxInfo.MaxBatSOC : batBMS[index].BatSOCCal;
			  boxInfo.MinBatSOC = (boxInfo.MinBatSOC < batBMS[index].BatSOCCal) ? boxInfo.MinBatSOC : batBMS[index].BatSOCCal;
    }
		
    //BOX E2 Initial
	  Read32Flash(&boxBMS.BoxCoulombTotal, E2_COULOMBTOTAL_ADDR);
	  Read32Flash(&boxBMS.BoxSOCCal, E2_BOXSOCCAL_ADDR);
    //check if total capcity is resonable
    if(boxBMS.BoxCoulombTotal > MAX_BAT_CAP || boxBMS.BoxCoulombTotal < BAT_CAP_QUARTER){
        boxBMS.BoxCoulombTotal = BAT_CAP_NINTY_PER;
    }
    //BOX SOC initial value
		BoxSOCInitEstimate();
    memset(SingleBatSOCEEReadBuffer,0,sizeof(SingleBatSOCEEReadBuffer));
		
		//SOHInit
		BoxSOHInit();
		
		HysteresisInit();
		
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
	  boxInfo.MaxBatTemp = -2000;
	  boxInfo.MinBatTemp =  2000;
	  boxInfo.MaxBatSOC = 0;
		boxInfo.MinBatSOC = boxInfo.SingleBatCoulombTotal;
    for(index = 0; index < CELL_NUM; index++){
        batBMS[index].BatVol   = PcPointBuffer[cell1Vol + index];
        batBMS[index].BatTemp  = PcPointBuffer[packInfo.Tmap_ther2bat[index]];
        
			  boxInfo.MaxBatTemp = (boxInfo.MaxBatTemp > batBMS[index].BatTemp) ? boxInfo.MaxBatTemp : batBMS[index].BatTemp;
	      boxInfo.MinBatTemp = (boxInfo.MinBatTemp < batBMS[index].BatTemp) ? boxInfo.MinBatTemp : batBMS[index].BatTemp;
			
			  boxInfo.MaxBatSOC = (boxInfo.MaxBatSOC > batBMS[index].BatSOCCal) ? boxInfo.MaxBatSOC : batBMS[index].BatSOCCal;
			  boxInfo.MinBatSOC = (boxInfo.MinBatSOC < batBMS[index].BatSOCCal) ? boxInfo.MinBatSOC : batBMS[index].BatSOCCal;

        boxBMS.temperature += batBMS[index].BatTemp;
    }
    boxBMS.temperature /= CELL_NUM;
		boxInfo.MaxBatVol = PcPointBuffer[maxcellvol];
    boxInfo.MinBatVol = PcPointBuffer[mincellvol];
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
    int16_t I = PcPointBuffer[current];//uint16->int16 10 times

	  if(I <= NO_CURRENT_THRESHOLD && I >= -NO_CURRENT_THRESHOLD){
		    chgdchgStatus.status = NO_CUR_STATUS;
		}else if(I < 0){
		    chgdchgStatus.status = DCHG_STATUS;
		}else{
		    chgdchgStatus.status = CHG_STATUS;
		}
		
    //every 500ms
    //I = (int16_t)((float)I / 25 * 10);//25 is the shunt ratio,10 times of current
	
    dC = I * -5;//5 stands for 500ms

    DataCheck();
	
	  ResistanceCal(I);

    TemperatureCali();
		
    //pure counter
    BoxCoulombCount();
	
	  BMSCoulombTotalRealTimeUpdate();
		
		SingleBatSOCupdate(I);
		
		BoxSOCCalVolCaliCal();

    BoxSOCShowUpdate();
		
		CellBalanceTask();
		
		SOHTask();
		
		HysteresisTask();
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
 * @param[in]            Do this calibration when reaching the upper/lower voltage
 * @return               none
 * \brief                
 *///
/***************************************************************************************/
void BMSCoulombTotalFinalUpdate(void){
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
			  boxBMS.BoxSOCCal = 0;
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
        Write32Flash(&boxBMS.BoxCoulombTotal, E2_COULOMBTOTAL_ADDR);
		}else if(writeEEcnt == 129){
		    Write32Flash(&boxInfo.SingleBatCoulombTotal, E2_SINGLEBATCOULOMBTOTAL_ADDR);
		}else if(writeEEcnt == 130){
		    Write32Flash(&boxSOH.absCoulombCnter, E2_COULOMB_CNTER_SOH_ADDR);
		}else if(writeEEcnt == 131){
			  Write32Flash(&boxSOH.alreadyLost, E2_ALREADY_LOST_SOH_ADDR);
		}else if(writeEEcnt == 132){
		    Write32Flash(&chgdchgStatus.hysteresisStatus, E2_HYSTERESIS_ADDR);
		}else if(writeEEcnt > 132){
			  writeEEcnt = 0;
		}
}
//To be done
uint32_t BatSOCVolInitEstimate(uint16_t vol, uint16_t temperature, uint32_t coulombSOC){
	int16_t I = PcPointBuffer[current];
  int16_t abs_I = abs_value(I);
	if(abs_I < 5){
	     uint32_t coulombSOCEst = BatSOCVolEst_NoCur(vol, coulombSOC);
		   if(coulombSOCEst > MAX_BAT_CAP)   return BAT_CAP_QUARTER;
		   return coulombSOCEst;
	}else if(abs_I < 20){
	
	}else{
	
	}
	
	
	//if(coulombSOC > MAX_BAT_CAP)   coulombSOC = BAT_CAP_QUARTER;
	
	return BAT_CAP_QUARTER;
}

void DataCheck(void){
    
}
/***************************************************************************************/
/*
 * Function              
 * @param[in]            current
 * @return               none
 * \brief                calculation equivalent resistance
 *///
/***************************************************************************************/
void ResistanceCal(int16_t Current){
	  if(boxInfo.MinBatVol < START_VOL_LOWER|| boxInfo.MaxBatVol > START_VOL_UPPER) return;
	
	  //staus init
	  if(cellR.statusR == INIT_R_CAL){
			cellR.cnt = 0;
			cellR.CurrentLastTime = Current;
			for(int i = 0; i < CELL_NUM; i++){
				  cellR.voltage[i] = batBMS[i].BatVol;
				}
			cellR.statusR = READY_R_CAL;
			return;
		}
		//status ready, most time it goes here
		if(cellR.statusR == READY_R_CAL){
			int16_t dcur = abs_value(Current - cellR.CurrentLastTime);
			if(dcur >= START_CURRENT_THRESHOLD){
				cellR.abs_dcur = dcur;
			  cellR.CurrentLastTime = Current;
				cellR.statusR = R_WAITING;
			}else{
			  //no step current still ready state
				cellR.CurrentLastTime = Current;
				for(int i = 0; i < CELL_NUM; i++){
				  cellR.voltage[i] = batBMS[i].BatVol;
				}
			}
			return;
		}
		//status waiting
		if(cellR.statusR == R_WAITING){
		    if(abs_value(Current - cellR.CurrentLastTime) <= CURRENT_ERROR_RANGE){
					  cellR.cnt++;
				    if(cellR.cnt == DELAY_CNT){
						    for(int i = 0; i < CELL_NUM; i++){
									 if(cellR.abs_dcur < START_CURRENT_THRESHOLD) cellR.statusR = INIT_R_CAL;
									
								   float r = (float)(abs_value(batBMS[i].BatVol - cellR.voltage[i])) * 10000 / cellR.abs_dcur;//micro-omega *1000*10
									 cellR.resis_cell[i] = (uint16_t)r;
									 //test
									 //PcPointBuffer[resis_cell1 + i] = cellR.resis_cell[i];
								}
								cellR.statusR = INIT_R_CAL;
						}
				}else{
				   cellR.statusR = INIT_R_CAL;
				}
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
void TemperatureCali(void){
    
}
/***************************************************************************************/
/*
 * Function              SingleBatSOCupdate
 * @param[in]            I(current A*10)
 * @return               none
 * \brief                
 *///
/***************************************************************************************/
void SingleBatSOCupdate(int16_t I){
    uint8_t batIndex = 0;
    //clear buffer
    memset(SingleBatSOCEEWriteBuffer, 0, sizeof(SingleBatSOCEEWriteBuffer));
    for(batIndex = 0; batIndex < CELL_NUM; batIndex++){
        SingleBatSOCCalVolCali(batIndex, I);
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
 * Function              BoxSOCShowUpdate
 * @param[in]            none
 * @return               none
 * \brief                smooth box soc change
 *///
/***************************************************************************************/
void BoxSOCShowUpdate(void){
	   uint16_t BoxSOC   = (uint16_t)(((float)boxBMS.BoxSOCCal/boxBMS.BoxCoulombTotal) * 10000);
	   int16_t  SOCerror = BoxSOC - boxBMS.BoxSOCShow;
	   if(abs_value(SOCerror) <= CALI_BOX_SHOW_MAX_STEP){
		   boxBMS.BoxSOCShow = BoxSOC;
		 }else{
		   if(BoxSOC > boxBMS.BoxSOCShow){
				 boxBMS.BoxSOCShow += CALI_BOX_SHOW_MAX_STEP;
			 }else{
				 if(boxBMS.BoxSOCShow >= CALI_BOX_SHOW_MAX_STEP){
				   boxBMS.BoxSOCShow -= CALI_BOX_SHOW_MAX_STEP;
				 }else{
				   boxBMS.BoxSOCShow = 0;
				 }   
			 }
		 }
		 boxBMS.BoxSOCShow = (boxBMS.BoxSOCShow > MAX_SOC) ? MAX_SOC : boxBMS.BoxSOCShow;
		 PcPointBuffer[SOC_box_show] = boxBMS.BoxSOCShow;
}
/***************************************************************************************/
/*
 * Function              
 * @param[in]            Do this calibration periodically
 * @return               none
 * \brief                
 *///
/***************************************************************************************/
void BMSCoulombTotalRealTimeUpdate(void){
    if(boxBMS.BoxCoulombCounter == 0){
			  if(boxBMS.BoxCoulombCounterCali >= 50) boxBMS.status |= BOXCAP_REAL_TIME_UPDATE; // >50 means >1A, allow some noise
		    boxBMS.BoxCoulombTotal += boxBMS.BoxCoulombCounterCali;
				boxInfo.SingleBatCoulombTotal += boxBMS.BoxCoulombCounterCali;
			  boxBMS.BoxCoulombCounterCali = 0;
			  return;
		}
		
		if(boxBMS.BoxCoulombCounter == boxBMS.BoxCoulombTotal){
			  if(boxBMS.BoxCoulombCounterCali >= 50) boxBMS.status |= BOXCAP_REAL_TIME_UPDATE; // >50 means >1A, allow some noise
		    boxBMS.BoxCoulombTotal += boxBMS.BoxCoulombCounterCali;
			  boxBMS.BoxCoulombCounter += boxBMS.BoxCoulombCounterCali;
			  boxInfo.SingleBatCoulombTotal += boxBMS.BoxCoulombCounterCali;
			  boxBMS.BoxCoulombCounterCali = 0;
			  return;
		}
		
		boxBMS.status &= (~BOXCAP_REAL_TIME_UPDATE);
    return;
}

//used for now, a more accurate calibration method is needed later 
void SingleBatSOCCoulombClear(void){
    uint8_t batIndex = 0;
    for(batIndex = 0; batIndex < CELL_NUM; batIndex++){
        batBMS[batIndex].BatSOCCal = 0;
    }
}
//used for now, a more accurate calibration method is needed later 
void SingleBatSOCCoulombFull(void){
    uint8_t batIndex = 0;
    for(batIndex = 0; batIndex < CELL_NUM; batIndex++){
        batBMS[batIndex].BatSOCCal = boxInfo.SingleBatCoulombTotal;
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
 * Function              Write32Flash
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

int16_t abs_value(int16_t error){
	return error < 0 ? (-error): (error);
}


uint32_t BatSOCVolEst_NoCur(uint16_t vol, uint32_t coulombSOC){
	  uint8_t chg = (chgdchgStatus.hysteresisStatus == CHG_VALUE);
	  if(chgdchgStatus.hysteresisStatus > CHG_VALUE || chgdchgStatus.hysteresisStatus < DCHG_VALUE){
		    chg = 1;
		}
		
		int16_t SOC_vol_estimate = 0;
		uint16_t EESOC = (uint16_t)(((float)coulombSOC/boxInfo.SingleBatCoulombTotal) * 100);
		//charge state before check
		uint8_t   len       = nocur_voltbl.chg_len;
		const uint16_t* vol_table = nocur_voltbl.chg_vol_table;
		const uint8_t*  soc_table = nocur_voltbl.chg_soc_table;
		const uint8_t*  soc_error_range = nocur_voltbl.chg_soc_error_range;
	  if(chg != 1){
			  //discharge state before check 
				len       = nocur_voltbl.dchg_len;
			  vol_table = nocur_voltbl.dchg_vol_table;
				soc_table = nocur_voltbl.dchg_soc_table;
	      soc_error_range = nocur_voltbl.dchg_soc_error_range;
		}
	  
	  uint8_t index = BiSearch(vol_table, len, vol);
		if(index == 0 || vol == vol_table[len - 1]){
				SOC_vol_estimate = soc_table[index];
		}else{
			if(vol - vol_table[index] <= vol_table[index + 1] - vol){
					SOC_vol_estimate = soc_table[index];
			}else{
					SOC_vol_estimate = soc_table[index + 1];
			}				
		}
	  if(abs_value(SOC_vol_estimate - EESOC) >= soc_error_range[index]){    
			if(index < len - 1){
			    float ratio = ((float)soc_table[index + 1] - soc_table[index])/(vol_table[index + 1] - vol_table[index]);
				  SOC_vol_estimate = ratio * (vol - vol_table[index]) + soc_table[index];
			}
			SOC_vol_estimate = (SOC_vol_estimate < 0)     ? 0   : SOC_vol_estimate; //In case bat vol is less than the least voltage in the  tbl
      SOC_vol_estimate = (SOC_vol_estimate > 100)   ? 100 : SOC_vol_estimate;
			uint32_t new_couombSOC = (uint32_t)((float)boxInfo.SingleBatCoulombTotal / 100 * SOC_vol_estimate);
			return new_couombSOC;
		}else{
			return coulombSOC;
		}
}

void BoxSOCInitEstimate(void){
    uint16_t MaxBatSOC = (uint16_t)(((float)boxInfo.MaxBatSOC/boxInfo.SingleBatCoulombTotal) * 100);
	  uint16_t MinBatSOC = (uint16_t)(((float)boxInfo.MinBatSOC/boxInfo.SingleBatCoulombTotal) * 100);
    uint16_t BoxSOC    = (uint16_t)(((float)boxBMS.BoxSOCCal/boxBMS.BoxCoulombTotal) * 100);
    if(BoxSOC <= MaxBatSOC && BoxSOC >= MinBatSOC){
			  boxBMS.BoxCoulombCounter = boxBMS.BoxSOCCal;
			  boxBMS.BoxSOCShow = (uint16_t)(((float)boxBMS.BoxSOCCal/boxBMS.BoxCoulombTotal) * 10000);
			  return;
		}
    uint16_t d = MaxBatSOC - MinBatSOC;
		if(d == 100){
		  boxBMS.BoxSOCCal = 0;
			boxBMS.BoxCoulombCounter = 0;
			boxBMS.BoxSOCShow = 0;
			return;
		}
		BoxSOC = MinBatSOC + MinBatSOC * (float)d /(100 - d);
    boxBMS.BoxSOCCal = (uint32_t)((float)boxBMS.BoxCoulombTotal / 100 * BoxSOC);
		boxBMS.BoxCoulombCounter = boxBMS.BoxSOCCal;
		boxBMS.BoxSOCShow = (uint16_t)(((float)boxBMS.BoxSOCCal/boxBMS.BoxCoulombTotal) * 10000);
		return;
}
/***************************************************************************************/
/*
 * Function              CellBalanceTask
 * @param[in]            void
 * @return               none
 * \brief                to be done
 *///
/***************************************************************************************/
void CellBalanceTask(void){
	// DONT USE I2C in the interrupt!!!!!
	if(dC < NO_CURRENT_THRESHOLD*5 && dC > -NO_CURRENT_THRESHOLD*5){
		//if not open, open up, else continue
		if(cellBalance.statusCB == CB_OFF) cellBalance.statusCB |= CB_TURN_ON;
	   
	}else{
		//chg or dischg
		if(cellBalance.cnt == 0){
		    for(int i = 0; i < CELL_NUM; i++){
				    cellBalance.VolLastTime[i] = batBMS[i].BatVol;
				}
				//check if condition is suitable for balance
			  if(PcPointBuffer[SOC_box] < packInfo.CB_SOCLowThreshold*100){
					//should close
					if((cellBalance.statusCB & CB_ON)){
					   cellBalance.statusCB |= CB_TURN_OFF;
					}					
				}else{
				  if(PcPointBuffer[maxcellvol] < packInfo.CB_VolStartThreshold){
					  //should close
					  if((cellBalance.statusCB & CB_ON)){
					    cellBalance.statusCB |= CB_TURN_OFF;
					  }
					}else{
						//should open
						if(!(cellBalance.statusCB & CB_ON) && !(cellBalance.statusCB & CB_FORBIDDEN)){
					    cellBalance.statusCB |= CB_TURN_ON;
					  }
					}
				}
		}

		cellBalance.cnt++;
		cellBalance.dCSum += dC;
		//Calculate dVdC
		if(cellBalance.cnt == 10){
		  cellBalance.cnt = 0;
			uint32_t abs_dCSum = cellBalance.dCSum < 0 ? (-cellBalance.dCSum) : cellBalance.dCSum;
			if(abs_dCSum >= 50 && abs_dCSum <= 200000){
			  for(int i = 0; i < CELL_NUM; i++){
					float dV = 0;
					if(cellBalance.dCSum > 0){//>0 means discharge
						batBMS[i].BatVol > cellBalance.VolLastTime[i] ? (dV = 0) : (dV = cellBalance.VolLastTime[i] - batBMS[i].BatVol);
					}else{
					  batBMS[i].BatVol < cellBalance.VolLastTime[i] ? (dV = 0) : (dV = batBMS[i].BatVol - cellBalance.VolLastTime[i]);
					}
				  cellBalance.dVdC[i] = (uint16_t)(dV * 1000000 / abs_dCSum);
					PcPointBuffer[dVdC1 + i] = cellBalance.dVdC[i];
				}	
			}else{
			   for(int i = 0; i < CELL_NUM; i++){
				  cellBalance.dVdC[i] = 1;
					PcPointBuffer[dVdC1 + i] = cellBalance.dVdC[i];
				 }
			}
			cellBalance.dCSum = 0;		
	  }
  }
}

uint8_t BiSearch(const uint16_t* arr, uint8_t len, uint16_t value){
	  if(value < arr[0]) return 0;
	  uint8_t low = 0;
	  uint8_t high = len - 1;
	  uint8_t mid = 0;
	  while(low <= high){
		  mid = (high + low)/2;
			if(arr[mid] > value){
			    high = mid - 1;
			}else{
			    low = mid + 1;
			}
		}
    return high;
}
// default R > L
uint16_t VolInterpolation(uint16_t RefLvalue, uint16_t RefRvalue, uint16_t RefValue, uint16_t Lvol, uint16_t Rvol){
    if(RefLvalue == RefRvalue || Lvol == Rvol) return Lvol;
	  return ((int16_t)(Rvol - Lvol)*(RefValue - RefLvalue)/(RefRvalue - RefLvalue)) + Lvol;
}
/***************************************************************************************/
/*Calibration PART
 *///
/***************************************************************************************/


/***************************************************************************************/
/*
 * Function              SingleBatSOCCalVolCali
 * @param[in]            none
 * @return               none
 * \brief         
 *///
/***************************************************************************************/
void SingleBatSOCCalVolCali(uint8_t batIndex, int16_t I){
    uint16_t k_cali = CALI_K_WITHOUT_CALI;
    if(checkVolCaliConditon(batIndex,I)){
		//calibrate is enable			  
			  uint16_t Vol_tbl_now[CALI_POINT_NUM] = {0};
				uint16_t Vol_tbl_LTemp[CALI_POINT_NUM] = {0};
				uint16_t Vol_tbl_RTemp[CALI_POINT_NUM] = {0};
				
		    uint8_t Temp_left_index = BiSearch(Cali_T_tbl, CALI_T_NUM, batBMS[batIndex].BatTemp);
				//get vol table
				if(Temp_left_index == CALI_T_NUM - 1){
				    InterpolVolBasedOnCur(Vol_tbl_now, Temp_left_index, I);
				}else{
					  //get nearest 2 temperature vol tables
				    InterpolVolBasedOnCur(Vol_tbl_LTemp, Temp_left_index, I);
					  InterpolVolBasedOnCur(Vol_tbl_RTemp, Temp_left_index + 1, I);
					  for(int i = 0; i < CALI_POINT_NUM; i++){
						    Vol_tbl_now[i] = VolInterpolation(Cali_T_tbl[Temp_left_index], Cali_T_tbl[Temp_left_index+1], batBMS[batIndex].BatTemp, Vol_tbl_LTemp[i], Vol_tbl_RTemp[i]);
						}
				}
				k_cali = CaliK(batIndex, Vol_tbl_now, I);
				//maybe dont calibrate logic will be added in k_cali later
				if(k_cali != CALI_K_WITHOUT_CALI) boxBMS.BatSOCCaliStatus |= (1 << batIndex);
		}else{
		//do not calibrate
			  boxBMS.BatSOCCaliStatus &= (~(1 << batIndex));
			  k_cali = CALI_K_WITHOUT_CALI;
		}
		SingleBatSOCCalVolCaliCal(batIndex, k_cali);
    return;
}
/***************************************************************************************/
/*
 * Function              SingleBatSOCCalVolCaliCal
 * @param[in]            battery index, k_cali
 * @return               void
 * \brief                
 *///
/***************************************************************************************/
void SingleBatSOCCalVolCaliCal(uint8_t batIndex, uint16_t k_cali){
	int32_t cali_dC = (int32_t)((float)k_cali * dC / 10);
	//cap real time update compensate
	int32_t capUpdataCompensate = 0;
	if((boxBMS.status & BOXCAP_REAL_TIME_UPDATE) != 0){
	    capUpdataCompensate = dC;
	}
	
  if(cali_dC > 0){
        //Discahrge
        if(batBMS[batIndex].BatSOCCal > cali_dC){
            batBMS[batIndex].BatSOCCal -= cali_dC;
        }else{
            //Still remain, single bat needs no calibration
            batBMS[batIndex].BatSOCCal = 0;
        }
    }else{
        //Charge
        if(batBMS[batIndex].BatSOCCal < (boxInfo.SingleBatCoulombTotal + cali_dC + capUpdataCompensate)){
            batBMS[batIndex].BatSOCCal -= (cali_dC + capUpdataCompensate);
        }else{
            //Still space left
            batBMS[batIndex].BatSOCCal = boxInfo.SingleBatCoulombTotal;
        }
    }

    if(boxInfo.SingleBatCoulombTotal == 0){
        PcPointBuffer[BatCommPoint[batIndex]] = 1234;
    }else{
        PcPointBuffer[BatCommPoint[batIndex]] = (uint16_t)(((float)batBMS[batIndex].BatSOCCal/boxInfo.SingleBatCoulombTotal) * 10000);
    }
    
    SingleBatSOCEEWriteBuffer[batIndex]  = batBMS[batIndex].BatSOCCal;
    SingleBatTempEEWriteBuffer[batIndex] = batBMS[batIndex].BatTemp;
}
/***************************************************************************************/
/*
 * Function              BoxSOCCalVolCaliCal
 * @param[in]            void
 * @return               void
 * \brief                
 *///
/***************************************************************************************/
void BoxSOCCalVolCaliCal(void){
	  int32_t  dCali = 0;
	  PcPointBuffer[Cali_bits] = boxBMS.BatSOCCaliStatus;
	  if(boxBMS.BatSOCCaliStatus == 0){
			//do not cali
		  dCali = 0;
		}else{
	    uint16_t MaxBatSOC = (uint16_t)(((float)boxInfo.MaxBatSOC/boxInfo.SingleBatCoulombTotal) * 10000);
	    uint16_t MinBatSOC = (uint16_t)(((float)boxInfo.MinBatSOC/boxInfo.SingleBatCoulombTotal) * 10000);
      uint16_t BoxSOC    = (uint16_t)(((float)boxBMS.BoxSOCCal/boxBMS.BoxCoulombTotal) * 10000);
			uint16_t BoxSOCBasedOnBat = 0;
      uint16_t d = MaxBatSOC - MinBatSOC;
		  if(d == 10000){
			  BoxSOC = 0;
		  }else{
		    BoxSOCBasedOnBat = MinBatSOC + MinBatSOC * (float)d /(10000 - d);
		  }
			if(BoxSOC > BoxSOCBasedOnBat){
				if(dC > 0){
				//Discahrge
					int32_t dSOCCoulomb = (BoxSOC - BoxSOCBasedOnBat) * (boxInfo.SingleBatCoulombTotal / 10000);
					dCali = (CALI_BOX_MAX_STEP > dSOCCoulomb) ? dSOCCoulomb : CALI_BOX_MAX_STEP;
				}else{
				//Charge
					dCali = -0.95f * dC; //charge less (different sign)
				}
			}else{
			  if(dC > 0){
				//Discahrge
					dCali = -0.95f * dC; //discharge less (different sign)
				}else{
				//Charge
					int32_t dSOCCoulomb = (BoxSOC - BoxSOCBasedOnBat) * (boxInfo.SingleBatCoulombTotal / 10000);
					dCali = (-CALI_BOX_MAX_STEP < dSOCCoulomb) ? dSOCCoulomb : -CALI_BOX_MAX_STEP;					
				}
			}
			boxBMS.BatSOCCaliStatus = 0;
		}
		
			
    if(dC > 0){
            //Discahrge
            if(boxBMS.BoxSOCCal > (dC + dCali)){
                boxBMS.BoxSOCCal = boxBMS.BoxSOCCal - dC - dCali;
            }else{
                //Still remain 
                boxBMS.BoxSOCCal = 0;
            }
    }else{
            //Charge
            if(boxBMS.BoxSOCCal < (boxBMS.BoxCoulombTotal + dC + dCali)){
                boxBMS.BoxSOCCal = boxBMS.BoxSOCCal - dC - dCali;
            }else{
                //Still space left
                boxBMS.BoxSOCCal = boxBMS.BoxCoulombTotal;
            }
    }
    if(boxBMS.BoxCoulombTotal == 0){
		    PcPointBuffer[SOC_box_cal] = 1234;
    }else{
        PcPointBuffer[SOC_box_cal] = (uint16_t)(((float)boxBMS.BoxSOCCal/boxBMS.BoxCoulombTotal) * 10000);
    }
		BoxSOCEEBuffer = boxBMS.BoxSOCCal;
}
/***************************************************************************************/
/*
 * Function              checkVolCaliConditon
 * @param[in]            battery index, current
 * @return               cali or not
 * \brief                to be done
 *///
/***************************************************************************************/
bool checkVolCaliConditon(uint8_t batIndex, int16_t I){
	  //T
    if(batBMS[batIndex].BatTemp > 500 || batBMS[batIndex].BatTemp < 0) return false;
	  //I
	  if(I >= 0){
		    if(I > TEN_TIMES_06C || I < TEN_TIMES_10A){
				    return false;
				}
		}else{
		   if(I < -TEN_TIMES_06C || I > -TEN_TIMES_10A){
				    return false;
				}
		} 
	  return true;
}
/***************************************************************************************/
/*
 * Function              InterpolVolBasedOnCur
 * @param[in]            Result array, Temperature table index, current
 * @return               a vol table
 * \brief                now it is done by interpolate the nearest two voltages under different current
 *                       The data scale is small now, so if-else is a good choice, maybe bisearch is more suitable later 
 *///
/***************************************************************************************/
void InterpolVolBasedOnCur(uint16_t* resultTbl, uint8_t tempIndex, int16_t I){
    if(I > 0){
			//charge
			if(I > TEN_TIMES_05C){
			    for(int i = 0; i < CALI_POINT_NUM; i++){
					    resultTbl[i] = Cali_T_Vol_tbl[tempIndex][0][i] + (uint16_t)((float)(I - TEN_TIMES_05C) * DEFAULT_R / 100);
					}
			}else if(I < TEN_TIMES_01C){
			    for(int i = 0; i < CALI_POINT_NUM; i++){
					    resultTbl[i] = Cali_T_Vol_tbl[tempIndex][2][i] - (uint16_t)((float)(TEN_TIMES_01C - I) * DEFAULT_R / 100);
					}
			}else if(I > TEN_TIMES_03C){
			    for(int i = 0; i < CALI_POINT_NUM; i++){
					    resultTbl[i] = VolInterpolation(TEN_TIMES_03C,TEN_TIMES_05C,I,Cali_T_Vol_tbl[tempIndex][1][i],Cali_T_Vol_tbl[tempIndex][0][i]);
					}
			}else{
			    for(int i = 0; i < CALI_POINT_NUM; i++){
					    resultTbl[i] = VolInterpolation(TEN_TIMES_01C,TEN_TIMES_03C,I,Cali_T_Vol_tbl[tempIndex][2][i],Cali_T_Vol_tbl[tempIndex][1][i]);
					}
			} 
		
		}else{
			uint16_t absI = abs_value(I);
		  if(absI > TEN_TIMES_05C){
			    for(int i = 0; i < CALI_POINT_NUM; i++){
					    resultTbl[i] = Cali_T_Vol_tbl[tempIndex][5][i] - (uint16_t)((float)(absI - TEN_TIMES_05C) * DEFAULT_R / 100);
					}
			}else if(absI < TEN_TIMES_01C){
			    for(int i = 0; i < CALI_POINT_NUM; i++){
					    resultTbl[i] = Cali_T_Vol_tbl[tempIndex][3][i] + (uint16_t)((float)(TEN_TIMES_01C - absI) * DEFAULT_R / 100);
					}
			}else if(absI > TEN_TIMES_03C){
			    for(int i = 0; i < CALI_POINT_NUM; i++){
					    resultTbl[i] = VolInterpolation(TEN_TIMES_03C, TEN_TIMES_05C, absI, Cali_T_Vol_tbl[tempIndex][4][i], Cali_T_Vol_tbl[tempIndex][5][i]);
					}
			}else{
			    for(int i = 0; i < CALI_POINT_NUM; i++){
					    resultTbl[i] = VolInterpolation(TEN_TIMES_01C, TEN_TIMES_03C, absI, Cali_T_Vol_tbl[tempIndex][3][i], Cali_T_Vol_tbl[tempIndex][4][i]);
					}
			}
		}
}
/***************************************************************************************/
/*
 * Function              CaliK
 * @param[in]            batIndex, Vol_tbl_now, current
 * @return               CaliK
 * \brief                bisearch the nearst voltage & get k(10 or other value)
 *///
/***************************************************************************************/
uint16_t CaliK(uint8_t batIndex, uint16_t* Vol_tbl_now, int16_t I){
	//Calibration of SOC edge slow down, superior to the voltage calibration
	if(checkSOCEdgeSlowDownConditon(batIndex, Vol_tbl_now, I)){
	  return CALI_K_EDGE_SLOW_DOWN;
	}
	
	//voltage calibration, range check & k choose
	uint8_t volIndex = BiSearch(Vol_tbl_now, CALI_POINT_NUM, batBMS[batIndex].BatVol);
	uint8_t delt_leftVolPoint[2] = {0};
	CaliVolRangeCalculate(batIndex, volIndex, delt_leftVolPoint, I);
	//check if voltage in the left point voltage range
	if(batBMS[batIndex].BatVol <= Vol_tbl_now[volIndex] - delt_leftVolPoint[0] || batBMS[batIndex].BatVol >= Vol_tbl_now[volIndex] + delt_leftVolPoint[1]){
	  if(volIndex == CALI_POINT_NUM - 1) return CALI_K_WITHOUT_CALI;
		//right point exists, so check it
		uint8_t delt_rightVolPoint[2] = {0};
		CaliVolRangeCalculate(batIndex, volIndex, delt_rightVolPoint, I);
		if(batBMS[batIndex].BatVol <= Vol_tbl_now[volIndex + 1] - delt_rightVolPoint[0] || batBMS[batIndex].BatVol >= Vol_tbl_now[volIndex + 1] + delt_rightVolPoint[1]){
		  return CALI_K_WITHOUT_CALI;	
		}
		return CaliKChooseBasedOnSOC(batIndex, volIndex+1, I);
	}
	
  return CaliKChooseBasedOnSOC(batIndex, volIndex, I);
}
/***************************************************************************************/
/*
 * Function              checkSOCEdgeSlowDownConditon
 * @param[in]            batIndex, * Vol_tbl_now, current
 * @return               whether cali soc should slow down 
 * \brief                
 *///
/***************************************************************************************/
bool checkSOCEdgeSlowDownConditon(uint8_t batIndex, uint16_t* Vol_tbl_now, int16_t I){
    uint16_t batSOC  = (uint16_t)(((float)batBMS[batIndex].BatSOCCal/boxInfo.SingleBatCoulombTotal) * 10000);
	  bool isSOCInDchgLowestRange = (batSOC > 0 && batSOC < (Cali_dchg_SOC_tbl[0] + Cali_delt_dchg_SOC_region[0]));
	  bool isSOCInChgHighestRange = (batSOC > (Cali_chg_SOC_tbl[CALI_POINT_NUM - 1] - Cali_delt_chg_SOC_region[CALI_POINT_NUM - 1]) && batSOC < MAX_SOC);
		bool isVolDchgHigher = batBMS[batIndex].BatVol > (Vol_tbl_now[0] + 5);  //5mV
		bool isVolChgLower   = batBMS[batIndex].BatVol < (Vol_tbl_now[CALI_POINT_NUM - 1] - 5);  //5mV
	  if(I > 0){
			//chg
			if(isSOCInChgHighestRange && isVolChgLower){
			  return true;
			}
		}else{
			//dchg
		  if(isSOCInDchgLowestRange && isVolDchgHigher){
			  return true;
			}
		}
		
		return false;
}
/***************************************************************************************/
/*
 * Function              CaliKChooseBasedOnSOC
 * @param[in]            batIndex, volIndex, current
 * @return               CaliK
 * \brief                get k around the volIndex voltage based on SOC(10 or other value)
 *///
/***************************************************************************************/
uint16_t CaliKChooseBasedOnSOC(uint8_t batIndex, uint8_t volIndex, int16_t I){
	//Check SOC error
	uint16_t batSOC  = (uint16_t)(((float)batBMS[batIndex].BatSOCCal/boxInfo.SingleBatCoulombTotal) * 10000);
	uint16_t standardSOC = 0;
	int16_t SOCErrorRange = 0;
	if(I > 0){
		standardSOC = Cali_chg_SOC_tbl[volIndex];
		SOCErrorRange = Cali_delt_chg_SOC_region[volIndex];
	}else{
	  standardSOC = Cali_dchg_SOC_tbl[volIndex];
		SOCErrorRange = Cali_delt_dchg_SOC_region[volIndex];
	}
	//do not minus two uint, it has to be a signed int to be compared with standard value
	int16_t SOC_error = batSOC - standardSOC;
	
	if(SOC_error > SOCErrorRange){
	//Higher SOC
		if(I > 0){
			//charge
		  return Cali_K_chg_Higher_SOC[volIndex];
		}else{
			//discharge
		  return Cali_K_dchg_Higher_SOC[volIndex];
		}
	}else if(SOC_error < -SOCErrorRange){
	//Lower SOC
		if(I > 0){
			//charge
		  return Cali_K_chg_Lower_SOC[volIndex];
		}else{
			//discharge
		  return Cali_K_dchg_Lower_SOC[volIndex];
		}
	}
	//SOC is in a acceptable region
	return CALI_K_WITHOUT_CALI;
}

/***************************************************************************************/
/*
 * Function              CaliVolRangeCalculate
 * @param[in]            batIndex, volIndex, delt_Vol, I
 * @return               void(left value & right value in delt_Vol
 * \brief                voltage range
 *///
/***************************************************************************************/
void CaliVolRangeCalculate(uint8_t batIndex, uint8_t volIndex, uint8_t* delt_Vol, int16_t I){
    delt_Vol[0] = 0;
	  delt_Vol[1] = 0;
	  uint8_t T0_VolRange_tbl[CALI_POINT_NUM] = {5, 0, 0, 5};
		
	  if(batBMS[batIndex].BatTemp > 300){
		  delt_Vol[0] = 5;
			delt_Vol[1] = 5;
		}else if(batBMS[batIndex].BatTemp > 200){
			//T25
		  if(I >= -TEN_TIMES_03C){
			  delt_Vol[0] = 5;
			  delt_Vol[1] = 5;
			}else{
			  delt_Vol[0] = 3;
			  delt_Vol[1] = 5;
			}
		
		}else if(batBMS[batIndex].BatTemp > 100){
			//T15
		  if(I >= -TEN_TIMES_01C){
			  delt_Vol[0] = 5;
			  delt_Vol[1] = 5;
			}else{
			  delt_Vol[0] = 2;
			  delt_Vol[1] = 5;
			}
		}else{
		  //T0
		  if(I >= TEN_TIMES_03C || I <= -TEN_TIMES_03C){
			  delt_Vol[0] = T0_VolRange_tbl[volIndex];
			  delt_Vol[1] = T0_VolRange_tbl[volIndex];
			}else{
			  delt_Vol[0] = 3;
			  delt_Vol[1] = 3;
			}
		}
}
/***************************************************************************************/
/*SOH PART
 *///
/***************************************************************************************/
void BoxSOHInit(void){
  Read32Flash(&boxSOH.absCoulombCnter, E2_COULOMB_CNTER_SOH_ADDR);
	if(boxSOH.absCoulombCnter > CNTER_100AH){
	  boxSOH.absCoulombCnter = 0;
	}
	
	Read32Flash(&boxSOH.alreadyLost, E2_ALREADY_LOST_SOH_ADDR);
	if(boxSOH.alreadyLost > TOTAL_LIFE_CAP/10 && boxSOH.alreadyLost != 0xffffffff){
	  boxSOH.alreadyLost = TOTAL_LIFE_CAP/10;
	}else if(boxSOH.alreadyLost == 0xffffffff){
	  boxSOH.alreadyLost = 0;
	}
}

void SOHTask(void){
	uint16_t soh = 0;
	uint32_t numerator = 0;
	uint32_t denominator = TOTAL_LIFE_CAP;
	if(dC < 0){
	  boxSOH.absCoulombCnter -= dC;
	}else{
	  boxSOH.absCoulombCnter += dC;
	}
	
	if(boxSOH.absCoulombCnter >= CNTER_100AH){
	  boxSOH.absCoulombCnter = 0;
		boxSOH.alreadyLost++;
	}
	numerator = TOTAL_LIFE_CAP - boxSOH.alreadyLost * LEAST_SOH_K;//70%
	soh = (uint16_t)(((float)numerator/denominator) * 10000);
  if(soh > 9950){
		PcPointBuffer[SOH_box] = 10000;
	}else if(soh < 7000){
	  PcPointBuffer[SOH_box] = 7000;
	}else{
	  PcPointBuffer[SOH_box] = soh;
	}
}
/***************************************************************************************/
/* chg&dchg hysteresis PART
 *///
/***************************************************************************************/
void HysteresisInit(void){
  Read32Flash(&chgdchgStatus.hysteresisStatus, E2_HYSTERESIS_ADDR);
	if(chgdchgStatus.hysteresisStatus < DCHG_VALUE || chgdchgStatus.hysteresisStatus > CHG_VALUE){
	  chgdchgStatus.hysteresisStatus = CHG_VALUE;
		chgdchgStatus.hysteresisCnter = HYSTERESIS_CNTER;
	}else if(chgdchgStatus.hysteresisStatus >= DCHG_VALUE && chgdchgStatus.hysteresisStatus < 15){
	  chgdchgStatus.hysteresisStatus = DCHG_VALUE;
		chgdchgStatus.hysteresisCnter = 0;
	}else{
	 chgdchgStatus.hysteresisStatus = CHG_VALUE;
	 chgdchgStatus.hysteresisCnter = HYSTERESIS_CNTER;
	}
}

void HysteresisTask(void){
    if(dC > 0){
            //Discahrge
			      uint32_t delt_dC = dC * HYSTERESIS_K;
            if(chgdchgStatus.hysteresisCnter > delt_dC){
                chgdchgStatus.hysteresisCnter -= delt_dC;
            }else{
                //Still remain 
                chgdchgStatus.hysteresisCnter = 0;
            }
    }else{
            //Charge
            if(chgdchgStatus.hysteresisCnter < (HYSTERESIS_CNTER + dC)){
                chgdchgStatus.hysteresisCnter -= dC;
            }else{
                //Still space left
                chgdchgStatus.hysteresisCnter = HYSTERESIS_CNTER;
            }
    }
		
		chgdchgStatus.hysteresisStatus = (10*chgdchgStatus.hysteresisCnter)/HYSTERESIS_CNTER + 10;
		PcPointBuffer[resis_cell1] = chgdchgStatus.hysteresisStatus;
}