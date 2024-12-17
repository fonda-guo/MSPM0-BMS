#ifndef MY_MCU_COMM_BSW_H_
#define MY_MCU_COMM_BSW_H_

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <ti/driverlib/m0p/dl_core.h>
#include <ti/iqmath/include/IQmathLib.h>
#include "ti_msp_dl_config.h"
#include <stdint.h>
#include <string.h>

void BSW_I2cInit(void);
void BSW_I2cReset(void);
void BSW_UartTxU16Data(uint16_t u16Data);
void BSW_UartTxU32Data(uint32_t u32Data);
void BSW_UartInit(void);
void BSW_UartReset(void);
void BSW_TimerInit(void);
void delayMS(uint16_t ms);
void delayUS(uint16_t us);

#endif