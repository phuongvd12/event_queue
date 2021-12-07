/*
 * myDefine.h
 *
 *  Created on: Dec 7, 2021
 *      Author: lenovo
 */

#ifndef MYDEFINE_H_
#define MYDEFINE_H_

#include "stdint.h"
#include "elster.h"
#include "sim_command.h"
#include "event_driven.h"
#include "myUart.h"

#define RS485_DISABLE HAL_GPIO_WritePin(NET485IO_GPIO_Port, NET485IO_Pin, GPIO_PIN_SET);

typedef struct
{
	uint8_t *data;
	uint16_t length;
}sData;

extern sData sUartSim;
extern sData sUartMeter;
extern sEvent_struct s_event_sim_handler[];
extern sEvent_struct s_event_meter_handler[];
extern sEvent_struct s_event_sub_handler[];
#endif /* MYDEFINE_H_ */
