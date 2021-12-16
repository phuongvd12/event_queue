/*
 * S25FL.c
 *
 *  Created on: Dec 15, 2021
 *      Author: lenovo
 */

#include "S25FL.h"
#include "spi.h"
#include "gpio.h"


void Flash_S25FL_ChipSelect(uint8_t State)
{
	uint16_t	i = 0;

	for (i = 0; i<1000; i++);

	if (State == LOW)
		HAL_GPIO_WritePin(SPI_NSS_GPIO_Port, SPI_NSS_Pin, GPIO_PIN_RESET);
	else
		HAL_GPIO_WritePin(SPI_NSS_GPIO_Port, SPI_NSS_Pin, GPIO_PIN_SET);

	for (i = 0; i<1000; i++);
}

uint8_t Flash_S25FL_Send_Byte(uint8_t byte)
{
	uint8_t retVal=0;

	HAL_SPI_TransmitReceive(&hspi1,&byte,&retVal,1,1000);

	return retVal;
}


