#ifndef SPI_COMM_H_
#define SPI_COMM_H_

#include <stdbool.h>
#include <stdint.h>
#include "myMCUCommBSW.h"
#include "ti_msp_dl_config.h"

uint8_t SPI_WriteReadByte(uint8_t TxData);
void SPI_CS_LOW(void);
void SPI_CS_HIGH(void);
#endif