/*
 * myDefine.h
 *
 *  Created on: Dec 7, 2021
 *      Author: lenovo
 */

#ifndef MYDEFINE_H_
#define MYDEFINE_H_

#include <Uart_handler.h>
#include "stdint.h"
#include "sim_command.h"
#include "meter_test.h"
#include "event_driven.h"
#include "queue_p.h"
#include "S25FL.h"

#include "Init_All_Meter.h"
#include "ELSTER_Read_Meter.h"
#include "Elster_Init_Meter.h"
#include "variable.h"

#define RS485_DISABLE HAL_GPIO_WritePin(NET485IO_GPIO_Port, NET485IO_Pin, GPIO_PIN_SET);
/* */
#define osDelay(x)					myOsDelay(x)
#define osOK	1
#define osFAIL	0

#define osSemaphoreWait(x, y)		fmySemaphoreWait(x,y)
#define osSemaphoreRelease(x) 	 	fmySemaphoreRelease(x)
#define bsUART2PendingMessHandle	EVENT_METER_CMD_SEND_OK

#define osMutexWait(x, y)			fmyMutexWait(x, y)
#define osMutexRelease(x)			fmyMutexRelease(x)
#define mtFlashMeterHandle			EVENT_METER_CMD_SEND_OK

#define	_fPackStringToLog(x, y)		HAL_UART_Transmit(&uart_debug, (uint8_t *)x, y, 1000);

typedef struct
{
	uint8_t *data;
	uint16_t length;
}sData;


//
extern uint8_t		Read_Meter_ID_Success;
extern uint8_t		Read_Meter_ID_Change;
extern uint8_t      aMeter_ID[METER_LENGTH];
//
extern sData sUartSim;
extern sData sUartMeter;
extern sEvent_struct s_event_sim_handler[];
extern sEvent_struct s_event_sub_handler[];

uint8_t myOsDelay(uint16_t timeout);
uint8_t fmySemaphoreWait(uint8_t event, uint16_t timeout);
uint8_t fmySemaphoreRelease(uint8_t event);
uint8_t fmyMutexWait(uint8_t event, uint16_t timeout);
uint8_t fmyMutexRelease(uint8_t event);

void Check_Meter(void);
void Send_Data_Meter(UART_HandleTypeDef *huart, uint8_t *data, uint16_t length, uint32_t Timeout);
#endif /* MYDEFINE_H_ */
