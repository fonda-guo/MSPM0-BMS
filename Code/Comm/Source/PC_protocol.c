#include "PC_protocol.h"

volatile uint8_t RecBuffer[8] = {0};
uint8_t SendBuffer[8] = {0};
uint16_t PcPointBuffer[u16_pc_buffer_num] = {0};
uint8_t TrashCan[8] = {0};
bool UART_Fault = false;

void DebugRegiter(){
    uint32_t *pointer = (uint32_t *)((PcPointBuffer[debug_addr1] << 16) + PcPointBuffer[debug_addr2]);
	  PcPointBuffer[debug_register1] = *pointer >> 16;
	  PcPointBuffer[debug_register2] = *pointer & 0xFFFF;
}

void PC_ProcessData(){
    if((RecBuffer[0] != MCU_ADDR) || (RecBuffer[7] != '\n')){
			UART_Fault = true;
			return;
		} 
		uint16_t pcpoint = RecBuffer[2];
		pcpoint = (pcpoint << 8) + RecBuffer[3];
		if((pcpoint < 0)||(pcpoint >= u16_pc_buffer_num)){
			return;
		}
		if(RecBuffer[1] == 0){
		  //read
			PC_SendData(pcpoint);
			UART_Fault = false;
			memset((void*)RecBuffer,0,8);
		}else if(RecBuffer[1] == 0x1){
		  //write
			UART_Fault = false;
			uint16_t data = RecBuffer[4];
			data = (data << 8) + RecBuffer[5];
 			PcPointBuffer[pcpoint] = data;
			//debug address
			if(pcpoint == debug_addr1 || pcpoint == debug_addr2){
			    DebugRegiter();
			}
			memset((void*)RecBuffer,0,8);
		}else{
      UART_Fault = true;
			return;
		}
}

void PC_SendData(PC_point pcpoint){
    SendBuffer[0] = MCU_ADDR;
	  SendBuffer[1] = 0x11;
	  SendBuffer[2] = (pcpoint & 0xFF00) >> 8;
    SendBuffer[3] = pcpoint & 0xFF;
    SendBuffer[4] = (PcPointBuffer[pcpoint]	& 0xFF00) >> 8;
	  SendBuffer[5] = PcPointBuffer[pcpoint] & 0xFF;
	  SendBuffer[6] = 0x00;
	  SendBuffer[7] = '\n';
    UART_WriteArray(SendBuffer,8);	
}

void PC_sendDataPoll(){
  for(int i = 0; i < u16_pc_buffer_num; i++){
	  PC_SendData(i);
		delayMS(1);
	}
}

void PC_protocol_init(){
	//test
   PcPointBuffer[6] = 123;
}

void UART_FaultReset(){
      memset((void*)RecBuffer,0,8);
			
			DL_DMA_disableChannel(DMA, DMA_CH0_CHAN_ID);
			DL_DMA_setSrcAddr(DMA, DMA_CH0_CHAN_ID, (uint32_t)(&UART_0_INST->RXDATA));
      DL_DMA_setDestAddr(DMA, DMA_CH0_CHAN_ID, (uint32_t) &RecBuffer[0]);
      DL_DMA_setTransferSize(DMA, DMA_CH0_CHAN_ID, 8);
	
			//DL_UART_drainRXFIFO(UART_0_INST,TrashCan,8);
			DL_UART_disableFIFOs(UART_0_INST);

			DL_DMA_enableChannel(DMA, DMA_CH0_CHAN_ID);
			DL_UART_enableFIFOs(UART_0_INST);
}
