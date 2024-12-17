#include "myMCUCommBSW.h"



//*****************************************************************************
// UART control
//*****************************************************************************

//Use this after Reset
void BSW_UartInit(void)
{
    DL_UART_Main_reset(UART_0_INST);
    DL_UART_Main_enablePower(UART_0_INST);
    delay_cycles(POWER_STARTUP_DELAY);
    SYSCFG_DL_UART_0_init();
}

void BSW_UartReset(void)
{
    //Disable UART
    DL_UART_Main_reset(UART_0_INST);
    DL_UART_Main_disablePower(UART_0_INST);
}

void BSW_UartTxU16Data(uint16_t u16Data)
{
    while (DL_UART_isBusy(UART_0_INST))
        ;
    DL_UART_Main_transmitData(UART_0_INST, (u16Data & 0xFF00) >> 8);
    while (DL_UART_isBusy(UART_0_INST))
        ;
    DL_UART_Main_transmitData(UART_0_INST, u16Data & 0xFF);
    while (DL_UART_isBusy(UART_0_INST))
        ;
}

void BSW_UartTxU32Data(uint32_t u32Data)
{
    BSW_UartTxU16Data((u32Data & 0xFFFF0000) >> 16);
    BSW_UartTxU16Data(u32Data & 0xFFFF);
}

uint16_t BSW_UartRxU16Data(void)
{
    uint16_t u16RxData;

    u16RxData = 0;
    while (DL_UART_getRawInterruptStatus(
               UART_0_INST, DL_UART_INTERRUPT_RX) == 0)
        ;
    DL_UART_clearInterruptStatus(UART_0_INST, DL_UART_INTERRUPT_RX);
    u16RxData += (DL_UART_Main_receiveData(UART_0_INST) << 8);

    while (DL_UART_getRawInterruptStatus(
               UART_0_INST, DL_UART_INTERRUPT_RX) == 0)
        ;
    DL_UART_clearInterruptStatus(UART_0_INST, DL_UART_INTERRUPT_RX);
    u16RxData += DL_UART_Main_receiveData(UART_0_INST);

    return u16RxData;
}

uint32_t BSW_UartRxU32Data(void)
{
    uint32_t u32RxData;
    uint16_t u16RxData;

    u32RxData = (BSW_UartRxU16Data() << 16);
    u16RxData = BSW_UartRxU16Data();

    return u32RxData + u16RxData;
}

//*****************************************************************************
// I2C control
//*****************************************************************************
void BSW_I2cInit(void)
{
    DL_I2C_reset(I2C_0_INST);
    DL_I2C_enablePower(I2C_0_INST);
    SYSCFG_DL_I2C_0_init();
}

void BSW_I2cReset(void)
{
    DL_I2C_reset(I2C_0_INST);
    DL_I2C_disablePower(I2C_0_INST);
}

void delayMS(uint16_t ms)
// Sets the delay in microseconds.
{
    char i;
    for (i = 0; i < ms; i++) delay_cycles(32000);
}

void delayUS(uint16_t us)
// Sets the delay in useconds.
{
    char i;
    for (i = 0; i < us; i++) delay_cycles(32);
}
