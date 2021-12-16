/*
 * event_driven.h
 *
 *  Created on: Nov 26, 2021
 *      Author: lenovo
 */

#ifndef EVENT_DRIVEN_H_
#define EVENT_DRIVEN_H_

#include "stdint.h"


typedef enum
{
	EVENT_SUB_BLINK_LED1 = 0,
	EVENT_SUB_BLINK_LED2,
	EVENT_SUB_BLINK_LED3,
	EVENT_SUB_PRINT_DEBUG,
	EVENT_SUB_COLLECT_DATA,
	EVENT_SUB_END, // don't use
}event_Sub_TypeDef;

typedef uint8_t (*fEventHandler) (uint8_t);

typedef struct
{
	uint8_t 		e_name;
	uint8_t			e_status;
	uint32_t 		e_systick;
	uint32_t 		e_period;
	fEventHandler 	e_function_handler;
}sEvent_struct;


void Event_Init(void);
void EventTask(void);

uint8_t fevent_active(sEvent_struct *event_struct, uint8_t event_name);
uint8_t fevent_enable(sEvent_struct *event_struct, uint8_t event_name);
uint8_t fevent_disable(sEvent_struct *event_struct, uint8_t event_name);

uint8_t Sub_event_disable(uint8_t event_sub);
uint8_t Sub_event_enable(uint8_t event_sub);
uint8_t Sub_event_active(uint8_t event_sub);
#endif /* EVENT_DRIVEN_H_ */
