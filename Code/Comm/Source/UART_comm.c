#include "UART_comm.h"

uint8_t UART_WriteArray(uint8_t *array, uint8_t len){
  uint8_t index = 0;
	while(index < len){
	  index += DL_UART_Main_fillTXFIFO(UART_0_INST,array + index,len - index);
		while (DL_UART_isBusy(UART_0_INST))
        ;
	}
	return 1;
}