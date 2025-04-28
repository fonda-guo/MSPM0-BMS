#include "I2C_comm.h"

/* Data sent to the Target */
uint8_t gTxPacket[I2C_TX_MAX_PACKET_SIZE] = {0x3E, 0x01, 0x00, 0x40, 0x04,
    0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
/* Counters for TX length and bytes sent */
uint32_t gTxLen, gTxCount;

/* Data received from Target */
uint8_t gRxPacket[I2C_RX_MAX_PACKET_SIZE];
/* Counters for TX length and bytes sent */
uint32_t gRxLen, gRxCount;

I2cControllerStatus_t gI2cControllerStatus;
uint8_t I2C_dead = 0;
//************Array copy **********************
void CopyArray(uint8_t *source, uint8_t *dest, uint8_t count)
{
    uint8_t copyIndex = 0;
    for (copyIndex = 0; copyIndex < count; copyIndex++) {
        dest[copyIndex] = source[copyIndex];
    }
}

//************I2C write register **********************
void I2C_WriteReg(uint8_t reg_addr, uint8_t *reg_data, uint8_t count)
{
    uint8_t I2Ctxbuff[8] = {0x00};

    I2Ctxbuff[0] = reg_addr;
    uint8_t i, j = 1;

    for (i = 0; i < count; i++) {
        I2Ctxbuff[j] = reg_data[i];
        j++;
    }

    //    DL_I2C_flushControllerTXFIFO(I2C_0_INST);
    DL_I2C_fillControllerTXFIFO(I2C_0_INST, &I2Ctxbuff[0], count + 1);

    /* Wait for I2C to be Idle */
    while (!(DL_I2C_getControllerStatus(I2C_0_INST) &
             DL_I2C_CONTROLLER_STATUS_IDLE))
        ;

    DL_I2C_startControllerTransfer(I2C_0_INST, I2C_TARGET_ADDRESS,
        DL_I2C_CONTROLLER_DIRECTION_TX, count + 1);

    while (DL_I2C_getControllerStatus(I2C_0_INST) &
           DL_I2C_CONTROLLER_STATUS_BUSY_BUS)
        ;
    /* Wait for I2C to be Idle */
    while (!(DL_I2C_getControllerStatus(I2C_0_INST) &
             DL_I2C_CONTROLLER_STATUS_IDLE))
        ;

    //Avoid BQ769x2 to stretch the SCLK too long and generate a timeout interrupt at 400kHz because of low power mode
    // if(DL_I2C_getRawInterruptStatus(I2C_0_INST,DL_I2C_INTERRUPT_CONTROLLER_CLOCK_TIMEOUT))
    // {
    //     DL_I2C_flushControllerTXFIFO(I2C_0_INST);
    //     DL_I2C_clearInterruptStatus(I2C_0_INST,DL_I2C_INTERRUPT_CONTROLLER_CLOCK_TIMEOUT);
    //     I2C_WriteReg(reg_addr, reg_data, count);
    // }
    DL_I2C_flushControllerTXFIFO(I2C_0_INST);
}

//************I2C read register **********************
void I2C_ReadReg(uint8_t reg_addr, volatile uint8_t *reg_data, uint8_t count)
{
	  memset((void*)reg_data,0,count);
	  uint8_t noresponse = 0;
    DL_I2C_fillControllerTXFIFO(I2C_0_INST, &reg_addr, count);

    /* Wait for I2C to be Idle */
    while (!(DL_I2C_getControllerStatus(I2C_0_INST) &
             DL_I2C_CONTROLLER_STATUS_IDLE))
        ;

    DL_I2C_startControllerTransfer(
        I2C_0_INST, I2C_TARGET_ADDRESS, DL_I2C_CONTROLLER_DIRECTION_TX, 1);

    while (DL_I2C_getControllerStatus(I2C_0_INST) &
           DL_I2C_CONTROLLER_STATUS_BUSY_BUS)
        ;
    /* Wait for I2C to be Idle */
    while (!(DL_I2C_getControllerStatus(I2C_0_INST) &
             DL_I2C_CONTROLLER_STATUS_IDLE))
        ;

    DL_I2C_flushControllerTXFIFO(I2C_0_INST);

    /* Send a read request to Target */
    DL_I2C_startControllerTransfer(
        I2C_0_INST, I2C_TARGET_ADDRESS, DL_I2C_CONTROLLER_DIRECTION_RX, count);
		
    for (uint8_t i = 0; i < count; i++) {
        while (DL_I2C_isControllerRXFIFOEmpty(I2C_0_INST) && (noresponse <= 40)){
					delayUS(80);
					noresponse++;
				}
				if(noresponse > 40){
				  I2C_dead = 1;
					noresponse = 0;
					break;
				}else{
					noresponse = 0;
					I2C_dead = 0;
          reg_data[i] = DL_I2C_receiveControllerData(I2C_0_INST);
				}
    }
}

//no interrupt is used for now
//void I2C_0_INST_IRQHandler(void)
//{
//    switch (DL_I2C_getPendingInterrupt(I2C_0_INST)) {
//        case DL_I2C_IIDX_CONTROLLER_RX_DONE:
//            gI2cControllerStatus = I2C_STATUS_RX_COMPLETE;
//            break;
//        case DL_I2C_IIDX_CONTROLLER_TX_DONE:
//            DL_I2C_disableInterrupt(
//                I2C_0_INST, DL_I2C_INTERRUPT_CONTROLLER_TXFIFO_TRIGGER);
//            gI2cControllerStatus = I2C_STATUS_TX_COMPLETE;
//            break;
//        case DL_I2C_IIDX_CONTROLLER_RXFIFO_TRIGGER:
//            gI2cControllerStatus = I2C_STATUS_RX_INPROGRESS;
//            /* Receive all bytes from target */
//            while (DL_I2C_isControllerRXFIFOEmpty(I2C_0_INST) != true) {
//                if (gRxCount < gRxLen) {
//                    gRxPacket[gRxCount++] =
//                        DL_I2C_receiveControllerData(I2C_0_INST);
//                } else {
//                    /* Ignore and remove from FIFO if the buffer is full */
//                    DL_I2C_receiveControllerData(I2C_0_INST);
//                }
//            }
//            break;
//        case DL_I2C_IIDX_CONTROLLER_TXFIFO_TRIGGER:
//            gI2cControllerStatus = I2C_STATUS_TX_INPROGRESS;
//            /* Fill TX FIFO with next bytes to send */
//            if (gTxCount < gTxLen) {
//                gTxCount += DL_I2C_fillControllerTXFIFO(
//                    I2C_0_INST, &gTxPacket[gTxCount], gTxLen - gTxCount);
//            }
//            break;
//            /* Not used for this example */
//        case DL_I2C_IIDX_CONTROLLER_ARBITRATION_LOST:
//        case DL_I2C_IIDX_CONTROLLER_NACK:
//            if ((gI2cControllerStatus == I2C_STATUS_RX_STARTED) ||
//                (gI2cControllerStatus == I2C_STATUS_TX_STARTED)) {
//                /* NACK interrupt if I2C Target is disconnected */
//                gI2cControllerStatus = I2C_STATUS_ERROR;
//            }
//        case DL_I2C_IIDX_CONTROLLER_RXFIFO_FULL:
//        case DL_I2C_IIDX_CONTROLLER_TXFIFO_EMPTY:
//        case DL_I2C_IIDX_CONTROLLER_START:
//        case DL_I2C_IIDX_CONTROLLER_STOP:
//        case DL_I2C_IIDX_CONTROLLER_EVENT1_DMA_DONE:
//        case DL_I2C_IIDX_CONTROLLER_EVENT2_DMA_DONE:
//        default:
//            break;
//    }
//}


//***************************************************************************//
//
//            Functions of MT9818
//
//***************************************************************************//
uint8_t MT9818_CRC8(uint8_t *ptr, uint8_t len,uint8_t key) 
{
	uint8_t i;
	uint8_t crc=0;
	while(len--!=0)
	{
		for(i=0x80; i!=0; i/=2)
		{
			if((crc & 0x80) != 0)
			{
				crc *= 2;
				crc ^= key;
			}
			else
				crc *= 2;

			if((*ptr & i)!=0)
				crc ^= key;
		}
		ptr++;
	}
	return(crc);
}



void I2C_WriteBreq_MsgSBS(uint8_t RegAdd, uint8_t data)  
{
	uint8_t crc=0;
	uint8_t buff[4];
	
	buff[0]=(I2C_TARGET_ADDRESS<<1);
	buff[1]=RegAdd;
	buff[2]=data;
	crc = MT9818_CRC8(buff,3,7);
	buff[3]=crc;
	
	DL_I2C_fillControllerTXFIFO(I2C_0_INST, &buff[1], 3);
	
	/* Wait for I2C to be Idle */
    while (!(DL_I2C_getControllerStatus(I2C_0_INST) &
             DL_I2C_CONTROLLER_STATUS_IDLE))
        ;

    DL_I2C_startControllerTransfer(I2C_0_INST, I2C_TARGET_ADDRESS,
        DL_I2C_CONTROLLER_DIRECTION_TX, 3);

    while (DL_I2C_getControllerStatus(I2C_0_INST) &
           DL_I2C_CONTROLLER_STATUS_BUSY_BUS)
        ;
    /* Wait for I2C to be Idle */
    while (!(DL_I2C_getControllerStatus(I2C_0_INST) &
             DL_I2C_CONTROLLER_STATUS_IDLE))
        ;
    DL_I2C_flushControllerTXFIFO(I2C_0_INST);
}


uint8_t I2C_GetRead_MsgSBS(uint8_t RegAdd) 
{
	uint8_t rec_data = 0,rec_crc = 0;
	uint8_t reg_data[2];
	uint8_t crc_buffer[2];
	uint8_t crc_result = 0;
	uint8_t noresponse = 0;
	static uint8_t error_count = 0;
	
	DL_I2C_fillControllerTXFIFO(I2C_0_INST, &RegAdd, 1);

    /* Wait for I2C to be Idle */
    while (!(DL_I2C_getControllerStatus(I2C_0_INST) &
             DL_I2C_CONTROLLER_STATUS_IDLE))
        ;

    DL_I2C_startControllerTransfer(
        I2C_0_INST, I2C_TARGET_ADDRESS, DL_I2C_CONTROLLER_DIRECTION_TX, 1);

    while (DL_I2C_getControllerStatus(I2C_0_INST) &
           DL_I2C_CONTROLLER_STATUS_BUSY_BUS)
        ;
    /* Wait for I2C to be Idle */
    while (!(DL_I2C_getControllerStatus(I2C_0_INST) &
             DL_I2C_CONTROLLER_STATUS_IDLE))
        ;

    DL_I2C_flushControllerTXFIFO(I2C_0_INST);

    /* Send a read request to Target */
    DL_I2C_startControllerTransfer(
        I2C_0_INST, I2C_TARGET_ADDRESS, DL_I2C_CONTROLLER_DIRECTION_RX, 2);

		for (uint8_t i = 0; i < 2; i++) {
        while (DL_I2C_isControllerRXFIFOEmpty(I2C_0_INST) && (noresponse <= 40)){
					delayUS(80);
					noresponse++;
				}
				if(noresponse > 40){
				  I2C_dead = 1;
					noresponse = 0;
					break;
				}else{
					noresponse = 0;
					I2C_dead = 0;
          reg_data[i] = DL_I2C_receiveControllerData(I2C_0_INST);
				}
    }

	rec_data = reg_data[0];
	rec_crc = reg_data[1];

	
	crc_buffer[0] = I2C_TARGET_ADDRESS;
	crc_buffer[1] = rec_data;
	crc_result = MT9818_CRC8(crc_buffer,2,7);	
	if (crc_result != rec_crc)
	{
		error_count++;
		if (error_count >= 10)
		{
			error_count = 0;
		}
	}	
	
	return rec_data;
}



