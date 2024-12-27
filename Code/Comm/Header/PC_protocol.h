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
	stackVol,
	LDpinVol,
	alarmBits,
	safetyStatusA,
	safetyStatusB,
	safetyStatusC,
	fet_Status,
	ts1,                //temperature
	ts2,                //temperature
	ts3,
	ts4,
	current,
	test_cnter,
	debug_register1,
	debug_register2,
	SOC_box,
	SOC_cell1,
	SOC_cell2,
	SOC_cell3,
	SOC_cell4,
	SOC_cell5,
	SOC_cell6,
	SOC_cell7,
	SOC_cell8,
	SOH_box,
	maxcellvol,
	mincellvol,
	//add before here
	fault,
  debug_addr1,
	debug_addr2,
	u16_pc_buffer_num,
}PC_point;


extern volatile uint8_t RecBuffer[8];
extern uint8_t SendBuffer[8];
extern uint16_t PcPointBuffer[u16_pc_buffer_num];
extern bool UART_Fault;

void PC_SendData(PC_point pcpoint);
void PC_protocol_init();
void PC_sendDataPoll();
void PC_ProcessData();
void UART_FaultReset();
#endif