#ifndef PC_PROTOCOL_H_
#define PC_PROTOCOL_H_
#include "myMCUCommBSW.h"
#include "UART_comm.h"
#define MCU_ADDR 7

typedef enum{
  cell1Vol,
	cell2Vol,
	cell3Vol,
	cell4Vol,
	cell5Vol,
	cell6Vol,
	cell7Vol,
	cell8Vol,
	cell9Vol,
	cell10Vol,
	cell11Vol,
	cell12Vol,
	cell13Vol,
	cell14Vol,
	cell15Vol,
	cell16Vol,
	stackVol,
	LDpinVol,
	alarmBits,
	safetyStatusA,
	safetyStatusB,
	safetyStatusC,
	fet_Status,
	ts1,
	ts2,
	ts3,
	ts4,
	current,
	test_cnter,
	//add before here
	fault,
	u16_pc_buffer_num,
}PC_point;


extern volatile uint8_t RecBuffer[8];
extern uint8_t SendBuffer[8];
extern uint16_t PcPointBuffer[u16_pc_buffer_num];
extern bool UART_Fault;

void PC_RecData();
void PC_SendData(PC_point pcpoint);
void PC_protocol_init();
void PC_sendDataPoll();
void PC_ProcessData();
void UART_FaultReset();
#endif