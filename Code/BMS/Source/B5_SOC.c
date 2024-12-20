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
 * @date           2024/10/8
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
// Private variables
//---------------------------------------------------------------------------
//leave for EEPROM write
uint32_t BoxSOCEEBuffer                = 0;
uint32_t BoxCoulombTotalEEBuffer       = 0;
uint16_t BoxTemperatureEEBuffer        = 0;
uint32_t SingleBatCoulombTotalEEBuffer = 0;
uint32_t SingleBatSOCEEReadBuffer[BAT_NUM]   = {0};
uint32_t SingleBatSOCEEWriteBuffer[BAT_NUM]  = {0};
uint16_t SingleBatTempEEWriteBuffer[BAT_NUM] = {0};
uint32_t test1 = 2; 
uint16_t test2 = 2;
uint16_t test3 = 2;
uint16_t test4 = 2;
uint16_t writeEEcnt = 0;
//dC is important
int32_t dC = 0;


//const GRP6_ID BatCommPoint[BAT_NUM] = { BCU_RSVD11, BCU_RSVD12, BCU_RSVD13, BCU_RSVD14, BCU_RSVD15,
//                                        BCU_RSVD24, BCU_RSVD25, BCU_RSVD26, BCU_RSVD27, BCU_RSVD32,
//                                        BCU_RSVD33, BCU_RSVD34, BCU_RSVD35, BCU_RSVD42, BCU_RSVD43,
//                                        BCU_RSVD44, BCU_RSVD45, BCU_RSVD46};

void BMSInit(void){
    boxBMS.status                  = (CYCLE_CALIB_ENABLE | START_UP_PROCESS);
    boxBMS.cnt                     = 0;
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

    //group6_addr = (uint16_t *)Comm_GetGrpStart(6);
    //group7_addr = (uint16_t *)Comm_GetGrpStart(7);

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
    //Check if battery voltage reaches the threshold
    BMSSingleBatVolCheck();
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
    
    // uint32_t BoxSOCEE = 0;
    //BOX E2 Initial
    //Lib_vReceiveEEPROMData(E2_COULOMBTOTAL_ADDR, 2, &boxBMS.BoxCoulombTotal, 2);
    //Lib_vReceiveEEPROMData(E2_BOXSOCCAL_ADDR, 2, &boxBMS.BoxSOCCal, 2);
    //check if total capcity is resonable
    if(boxBMS.BoxCoulombTotal > MAX_BAT_CAP || boxBMS.BoxCoulombTotal < BAT_CAP_QUARTER){
        boxBMS.BoxCoulombTotal = BAT_CAP_NINTY_PER;
    }
    //initial value
    boxBMS.BoxCoulombCounter = boxBMS.BoxSOCCal;

    //BOX Info Initial
    //Lib_vReceiveEEPROMData(E2_SINGLEBATCOULOMBTOTAL_ADDR, 2, &boxInfo.SingleBatCoulombTotal, 2);
    
    if(boxInfo.SingleBatCoulombTotal > MAX_BAT_CAP || boxInfo.SingleBatCoulombTotal < BAT_CAP_QUARTER){
        boxInfo.SingleBatCoulombTotal = BAT_CAP_NINTY_PER;
    }

    //Bat E2 Initial
    //Lib_vReceiveEEPROMData(E2_SINGLE_BAT_SOC_ADDR, 2*BAT_NUM, SingleBatSOCEEReadBuffer, 2*BAT_NUM);
    
    uint16_t index = 0;
    for(index = 0; index < BAT_NUM; index++){
        //batBMS[index].BatVol        = group7_addr[CELL_VOLT_START + index];
        //batBMS[index].BatTemp       = group7_addr[CELL_TEMP_START + index];
        batBMS[index].BatCoulombSOC = BatSOCVolInitEstimate(batBMS[index].BatVol,batBMS[index].BatTemp,SingleBatSOCEEReadBuffer[index]);
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
    for(index = 0; index < BAT_NUM; index++){
        //batBMS[index].BatVol   = group7_addr[CELL_VOLT_START + index];
        //batBMS[index].BatTemp  = group7_addr[CELL_TEMP_START + index];
        
        //boxInfo.MaxBatVol = GET_IMAX(boxInfo.MaxBatVol, batBMS[index].BatVol);
        //boxInfo.MinBatVol = GET_IMIN(boxInfo.MinBatVol, batBMS[index].BatVol);

        //boxInfo.MaxBatTemp = GET_IMAX(boxInfo.MaxBatVol, batBMS[index].BatTemp);
        //boxInfo.MinBatTemp = GET_IMIN(boxInfo.MaxBatVol, batBMS[index].BatTemp);

        boxBMS.temperature += batBMS[index].BatTemp;
    }
        boxBMS.temperature /= BAT_NUM;
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
    boxBMS.cnt++;
    int32_t I = 0;
    if(boxBMS.cnt < CNT_PERIOD){
        return;
    }else{
        //every 300ms
        boxBMS.cnt = 0;
        //I = (int32_t)((mg_sSpFlt.Filt_gain[I_BOT1_CALI] + mg_sSpFlt.Filt_gain[I_BOT2_CALI]) * 10);//10 times of current
        dC = I * 300;

        DataCheck();

        TemperatureCali();

        //VolCali();

        SingleBatSOCupdate();

        BoxCoulombCount();

        BoxSOCUpdate();
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
void BMSSingleBatVolCheck(void){
    float f32Bat_MaxSingleVolt = 3.3f;
    float f32Bat_MinSingleVolt = 3.3f;
    //f32Bat_MaxSingleVolt = ((group6_addr[CELL_VOLT_MAX] == 0) ? f32Bat_MaxSingleVolt  : (group6_addr[CELL_VOLT_MAX] * 0.001f));//scale 0.001V
    //f32Bat_MinSingleVolt = ((group6_addr[CELL_VOLT_MIN] == 0) ? f32Bat_MinSingleVolt  : (group6_addr[CELL_VOLT_MIN] * 0.001f));//scale 0.001V

    if(f32Bat_MaxSingleVolt >= 3.55f){
        boxBMS.status |= REACH_BAT_UPPER_BOUND;
        boxBMS.BoxCoulombCounterCali -= (boxBMS.BoxCoulombTotal - boxBMS.BoxCoulombCounter);
    }

    if(f32Bat_MinSingleVolt <= 3.0f){
        boxBMS.status |= REACH_BAT_LOWER_BOUND;
        boxBMS.BoxCoulombCounterCali -= boxBMS.BoxCoulombCounter;
        boxBMS.BoxCoulombCounter = 0;
        SingleBatSOCCoulombClear();
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
 
    boxBMS.BoxCoulombTotal += boxBMS.BoxCoulombCounterCali;
    boxInfo.SingleBatCoulombTotal = boxBMS.BoxCoulombTotal;//to be done
    if((boxBMS.status & REACH_BAT_UPPER_BOUND) != 0){
        boxBMS.BoxCoulombCounter = boxBMS.BoxCoulombTotal; //make sure SOC is full, do this after calibrating the SOH
        SingleBatSOCCoulombFull();
        boxBMS.BoxSOCCal = boxBMS.BoxCoulombTotal;
    } 
    
    //if(Lib_vTransmitEEPROMData(&BoxCoulombTotalEEBuffer,&boxBMS.BoxCoulombTotal,2,E2_COULOMBTOTAL_ADDR)){
    //    boxBMS.status &= (~CYCLE_WRITE_EE);//only success write can clear the bit, or do this task again
    //}
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
    writeEEcnt++;
	  //have a change
//    if(writeEEcnt == 15){
//        test1 = Lib_vTransmitEEPROMData(SingleBatSOCEEWriteBuffer, SingleBatSOCEEWriteBuffer, 2*BAT_NUM, E2_SINGLE_BAT_SOC_ADDR);
//    }else if(writeEEcnt == 30){
//        test2 = Lib_vTransmitEEPROMData(SingleBatTempEEWriteBuffer, SingleBatTempEEWriteBuffer, BAT_NUM, E2_SINGLE_BAT_TEMP_ADDR);
//    }else if(writeEEcnt == 45){
//        test3 = Lib_vTransmitEEPROMData(&BoxSOCEEBuffer, &boxBMS.BoxSOCCal, 2, E2_BOXSOCCAL_ADDR);
//    }else if(writeEEcnt == 60){
//        test4 = Lib_vTransmitEEPROMData(&BoxTemperatureEEBuffer, &boxBMS.temperature, 1, E2_BOXTEMP_ADDR);
//        writeEEcnt = 0;
//    }
}
//To be done
uint32_t BatSOCVolInitEstimate(uint16_t vol, uint16_t temperature, uint32_t coulombSOC){
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
    for(batIndex = 0; batIndex < BAT_NUM; batIndex++){
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
        //boxBMS.BoxCoulombCounter = (boxBMS.BoxCoulombCounter > I * 300) ? (boxBMS.BoxCoulombCounter - I * 300) : 0;
        if(batBMS[batIndex].BatCoulombSOC > dC){
            batBMS[batIndex].BatCoulombSOC -= dC;
        }else{
            //Still remain, single bat needs no calibration
            batBMS[batIndex].BatCoulombSOC = 0;
        }
    }else{
        //Charge
        //boxBMS.BoxCoulombCounter = (boxBMS.BoxCoulombCounter > (boxBMS.BoxCoulombTotal + I * 300)) ? boxBMS.BoxCoulombTotal : (boxBMS.BoxCoulombCounter - I * 300);
        if(batBMS[batIndex].BatCoulombSOC < (boxInfo.SingleBatCoulombTotal + dC)){
            batBMS[batIndex].BatCoulombSOC -= dC;
        }else{
            //Still space left
            batBMS[batIndex].BatCoulombSOC = boxInfo.SingleBatCoulombTotal;
        }
    }

    if(boxInfo.SingleBatCoulombTotal == 0){
        //group6_addr[BatCommPoint[batIndex]] = 1234;
    }else{
        //group6_addr[BatCommPoint[batIndex]] = (uint16_t)(((float32_t)batBMS[batIndex].BatCoulombSOC/boxInfo.SingleBatCoulombTotal) * 10000);
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
            //boxBMS.BoxCoulombCounter = (boxBMS.BoxCoulombCounter > I * 300) ? (boxBMS.BoxCoulombCounter - I * 300) : 0;
            if(boxBMS.BoxCoulombCounter > dC){
                boxBMS.BoxCoulombCounter -= dC;
            }else{
                //Still remain 
                boxBMS.BoxCoulombCounterCali += (dC - boxBMS.BoxCoulombCounter);
                boxBMS.BoxCoulombCounter = 0;
            }
    }else{
            //Charge
            //boxBMS.BoxCoulombCounter = (boxBMS.BoxCoulombCounter > (boxBMS.BoxCoulombTotal + I * 300)) ? boxBMS.BoxCoulombTotal : (boxBMS.BoxCoulombCounter - I * 300);
            if(boxBMS.BoxCoulombCounter < (boxBMS.BoxCoulombTotal + dC)){
                boxBMS.BoxCoulombCounter -= dC;
            }else{
                //Still space left
                boxBMS.BoxCoulombCounterCali += (boxBMS.BoxCoulombCounter - dC - boxBMS.BoxCoulombTotal);
                boxBMS.BoxCoulombCounter = boxBMS.BoxCoulombTotal;
            }
    }
    if(boxBMS.BoxCoulombTotal == 0){
        //group6_addr[BCU_RSVD2] = 1234;
    }else{
        //group6_addr[BCU_RSVD2] = (uint16_t)(((float32_t)boxBMS.BoxCoulombCounter/boxBMS.BoxCoulombTotal) * 10000);
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
}
//used for now, a more accurate calibration method is needed later 
void SingleBatSOCCoulombClear(void){
    uint16_t batIndex = 0;
    for(batIndex = 0; batIndex < BAT_NUM; batIndex++){
        batBMS[batIndex].BatCoulombSOC = 0;
    }
}
//used for now, a more accurate calibration method is needed later 
void SingleBatSOCCoulombFull(void){
    uint16_t batIndex = 0;
    for(batIndex = 0; batIndex < BAT_NUM; batIndex++){
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

