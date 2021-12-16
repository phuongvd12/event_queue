/*
 * myDefine.c
 *
 *  Created on: Dec 8, 2021
 *      Author: lenovo
 */


#include "myDefine.h"

#include "string.h"
#include "usart.h"

static volatile uint8_t mySemaphoreFlag = 0;
uint8_t		Read_Meter_ID_Success = 0;
uint8_t		Read_Meter_ID_Change  = 0;
uint8_t     aMeter_ID[METER_LENGTH];

uint8_t myOsDelay(uint16_t timeout)
{
	uint32_t my_tick = HAL_GetTick();

	do
	{
		EventTask();
		SimTask();
		FlashTask();
		if (HAL_GetTick() - my_tick >= timeout)
			return osFAIL;
	}while(1);

	HAL_Delay(10);
	return osOK;
}

uint8_t fmySemaphoreWait(uint8_t event, uint16_t timeout)
{
	uint32_t my_tick = HAL_GetTick();
	mySemaphoreFlag = 1;

	do
	{
		EventTask();
		SimTask();
		FlashTask();
		if (HAL_GetTick() - my_tick >= timeout)
			return osFAIL;
	}while(mySemaphoreFlag);

	HAL_Delay(10);
	return osOK;
}

uint8_t fmySemaphoreRelease(uint8_t event) // be executed in ISR
{
	mySemaphoreFlag = 0;

	return osOK;
}

uint8_t fmyMutexWait(uint8_t event, uint16_t timeout)
{

	return osOK;
}

uint8_t fmyMutexRelease(uint8_t event)
{

	return osOK;
}

void Send_Data_Meter(UART_HandleTypeDef *huart, uint8_t *data, uint16_t length, uint32_t Timeout)
{
	uint8_t i=0;
	uint8_t aTempBuff[256] = {0};

	for(i=0; i<length; i++)
	{
		aTempBuff[i] = *(data+i);//(*(data+i)&0x7F);
	}
	HAL_UART_Transmit(huart,&aTempBuff[0],length,Timeout);
	HAL_UART_Transmit(&uart_debug,&aTempBuff[0],length,Timeout);
}
