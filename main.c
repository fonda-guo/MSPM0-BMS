#include "ti_msp_dl_config.h"
#include "PC_protocol.h"
#include "BQ769x2_protocol.h"
#include "DataProcess.h"
#include "FaultProtect.h"


int main(void)
{
    SYSCFG_DL_init();
	
	  /* Configure DMA source, destination and size */
    DL_DMA_setSrcAddr(DMA, DMA_CH0_CHAN_ID, (uint32_t)(&UART_0_INST->RXDATA));
    DL_DMA_setDestAddr(DMA, DMA_CH0_CHAN_ID, (uint32_t) &RecBuffer[0]);
    DL_DMA_setTransferSize(DMA, DMA_CH0_CHAN_ID, 8);//when uart DMA is enabled, single mode must be chosen and the length is RX buffer length
    DL_DMA_enableChannel(DMA, DMA_CH0_CHAN_ID);
	  /* Enable NVIC*/
	  NVIC_EnableIRQ(TIMER_INST_INT_IRQN);
	  NVIC_EnableIRQ(UART_0_INST_INT_IRQN);
	  NVIC_EnableIRQ(I2C_0_INST_INT_IRQN);
	
	  BQ769x2_Init();
	
    delayMS(1);
	  CommandSubcommands(FET_ENABLE); // Enable the CHG and DSG FETs
	  delayMS(1);
	  CommandSubcommands(SLEEP_DISABLE); // Sleep mode is enabled by default. For this example, Sleep is disabled to 
									   // demonstrate full-speed measurements in Normal mode.
    PC_protocol_init();
    
    FaultProtect_init();

    DL_TimerG_startCounter(TIMER_INST);
    while (1) {
			
			DL_GPIO_togglePins(GPIO_LED_PORT,GPIO_LED_USER_LED_2B_PIN);
			
			BQDataGet();
			
			FaultDetect();
			
			if(UART_Fault){
				  UART_FaultReset();//dont put this in interrupt
			}
		}
}

void UART_0_INST_IRQHandler(void)
{
    switch (DL_UART_Main_getPendingInterrupt(UART_0_INST)) {
        case DL_UART_MAIN_IIDX_DMA_DONE_RX:
				     PC_ProcessData();
             break;
        default:
             break;
    }
}

void TIMER_INST_IRQHandler(void)
{
    switch (DL_TimerA_getPendingInterrupt(TIMER_INST)) {
        case DL_TIMER_IIDX_ZERO:
				     if(PcPointBuffer[test_cnter] == 65535) PcPointBuffer[test_cnter] = 0;
				     PcPointBuffer[test_cnter]++;
             break;
        default:
             break;
  }
}

void I2C_0_INST_IRQHandler(void)
{
    switch (DL_I2C_getPendingInterrupt(I2C_0_INST)) {
        case DL_I2C_IIDX_CONTROLLER_NACK:
            I2C_dead = 1;
						break;
        default:
            break;
    }
}
