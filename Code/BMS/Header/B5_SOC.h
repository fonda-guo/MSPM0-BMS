/****************************************************************/
/**
 * @file           B5_SOC.c
 *----------------------------------------------------------------
 * @brief
 *----------------------------------------------------------------
 * @note This file aims to calculate the SOC of B5(C7) DCDC.    
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
#ifndef B5_SOC_H_
#define B5_SOC_H_

#ifdef EXPORT_B5_SOC
  #define EXTERN
#else
  #define EXTERN extern
#endif
//---------------------------------------------------------------------------
// Included header
//---------------------------------------------------------------------------
#include <stdint.h>
#include "BatConfig.h"
#include "PC_protocol.h"
//---------------------------------------------------------------------------
// Constants and macros
//---------------------------------------------------------------------------
//basic parameter
#define MAX_BAT_CAP          115200000  //320Ah 320*10*3600*10(*100ms)
#define NOMINAL_BAT_CAP      100800000  //280Ah 280*10*3600*10(*100ms)
#define BAT_CAP_NINTY_PER    90720000   //NOMINAL_BAT_CAP*0.9
#define BAT_CAP_QUARTER      25200000   //NOMINAL_BAT_CAP*0.25
#define MAX_SOC              10000      //100*100
#define MAX_SOH              10000      //100*100
#define VOL_LOWER_BOUND      2900       //2.8V
#define VOL_UPPER_BOUND      3580       //3.4V

//status define macro
#define START_UP_PROCESS           0x0001
#define REACH_BAT_LOWER_BOUND      0x0002
#define REACH_BAT_UPPER_BOUND      0x0004
#define CYCLE_CALIB_ENABLE         0x0008
#define CYCLE_WRITE_EE             (REACH_BAT_LOWER_BOUND|\
                                    REACH_BAT_UPPER_BOUND)
//Flash address(use EE in the formal version)
#define E2_COULOMBTOTAL_ADDR            (0x00006000 + 0x400)                 //Next sector to prevent from being erased
#define E2_BOXSOCCAL_ADDR               (E2_COULOMBTOTAL_ADDR + 8)
#define E2_SINGLEBATCOULOMBTOTAL_ADDR   (E2_BOXSOCCAL_ADDR + 8)
#define E2_SINGLE_BAT_SOC_ADDR          (E2_SINGLEBATCOULOMBTOTAL_ADDR + 8)//8*8

//Check and protect
#define BOX_COULOMBTOTAL_FAULT       0x0001
#define BAT_COULOMBTOTAL_FAULT       0x0002
#define CUR_FAULT                    0x0004
#define LOW_SOH                      0x0008
#define VOL_IMBALANCE                0x0010
#define SOC_IMBALANCE                0x0020

//---------------------------------------------------------------------------
// Typedefs, structs and enums
//---------------------------------------------------------------------------
typedef struct{
    uint16_t MaxBatVol;
    uint16_t MinBatVol;

    uint16_t MaxBatTemp;
    uint16_t MinBatTemp;

    uint32_t MaxBatSOC;
    uint32_t MinBatSOC;

    uint32_t SingleBatCoulombTotal;
    uint32_t DataCheck;
}BOXINFO;

typedef struct{
    uint16_t status;
    uint16_t temperature;           //10-70
    uint16_t BoxSOCShow;            //0~10000 stands for 0~1
    uint16_t BoxSOHShow;            //0~10000 stands for 0~1
    uint32_t BoxCoulombCounter;     //0 ~ MAX_BAT_CAP
    uint32_t BoxCoulombTotal;       //0 ~ MAX_BAT_CAP
    uint32_t BoxSOCCal;             //0 ~ MAX_BAT_CAP
    int32_t  BoxCoulombCounterCali;
}BOXBMS;

typedef struct{
    uint16_t BatVol;
    uint16_t BatTemp;
    uint32_t BatCoulombSOC;          //0 ~ MAX_BAT_CAP
}BATBMS;

typedef struct{
    uint16_t VolCheckIndex;
}TOBEDONE;

//---------------------------------------------------------------------------
// Public variables
//---------------------------------------------------------------------------
EXTERN BOXINFO  boxInfo;
EXTERN BOXBMS   boxBMS;
EXTERN BATBMS   batBMS[CELL_NUM];
EXTERN uint32_t BMSFaultBit;

//EXTERN uint16_t  CalibVolTable[1] = {3150};
//EXTERN uint16_t  CaliSOCTable[1]  = {1200};
//---------------------------------------------------------------------------
// Public functions prototypes
//---------------------------------------------------------------------------
//Structure functions
EXTERN void BMSInit(void);
EXTERN void BMSTask(void);
EXTERN void BMSStartUp(void);
EXTERN void BMSBasicDataGet(void);
EXTERN void BMSCalTask(void);
EXTERN void BMSSingleBatVolCheck(void);
EXTERN void BMSCoulombTotalUpdate(void);
EXTERN void BMSWriteEETask(void);

//Detail functions
EXTERN uint32_t BatSOCVolInitEstimate(uint16_t vol, uint16_t temperature, uint32_t coulombSOC);
EXTERN void DataCheck(void);
EXTERN void TemperatureCali(void);
EXTERN void SingleBatSOCupdate(void);
EXTERN void BoxCoulombCount(void);
EXTERN void BoxSOCUpdate(void);

EXTERN void SingleBatSOCCal(uint16_t batIndex);
EXTERN void SingleBatSOCCoulombClear(void);
EXTERN void SingleBatSOCCoulombFull(void);

#undef EXTERN
#endif
