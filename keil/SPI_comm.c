#include"SPI_comm.h"
#define SPI_TIMEOUT 50
uint8_t test1 = 0;

uint8_t SPI_WriteReadByte(uint8_t TxData)
{
	uint16_t i, retry = 0;

//	while(DL_SPI_isTXFIFOEmpty(SPI_0_INST))
//	{
//		retry++;
//		if(retry > SPI_TIMEOUT)
//				return 0;
//	}
	DL_SPI_transmitData8(SPI_0_INST, TxData);
  for(i=0;i<50;i++);

	retry = 0;
	while (DL_SPI_isRXFIFOEmpty(SPI_0_INST))
	{
		retry++;
		if(retry > SPI_TIMEOUT)
				return 0;
	}
	test1 = DL_SPI_receiveDataBlocking8(SPI_0_INST);
	return  test1;
}

void SPI_CS_LOW(void){
    DL_GPIO_clearPins(GPIO_CS_PORT, GPIO_CS_PIN_CS_PIN);
}

void SPI_CS_HIGH(void){
    DL_GPIO_setPins(GPIO_CS_PORT, GPIO_CS_PIN_CS_PIN);
}
