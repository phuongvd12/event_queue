/*
 * event_driven.c
 *
 *  Created on: Nov 26, 2021
 *      Author: lenovo
 */

#include "main_app.h"
#include "stm32l4xx_hal.h"
#include "gpio.h"
#include "iwdg.h"

#include "myDefine.h"

static sData sUartDebug;
uint8_t uartDebugBuff[100] = {0};
/* */
static void CheckEventQueue(void);
static uint8_t fEvent_blink_led1_handler(uint8_t event);
static uint8_t fEvent_blink_led2_handler(uint8_t event);
static uint8_t fEvent_blink_led3_handler(uint8_t event);
static uint8_t fEvent_print_debug_handler(uint8_t event);
static uint8_t fEvent_collect_handler(uint8_t event);

sEvent_struct s_event_sub_handler[] =
{
	{ EVENT_SUB_BLINK_LED1, 		1, 0, 1000, 	fEvent_blink_led1_handler},
	{ EVENT_SUB_BLINK_LED2, 		1, 1, 20000, 	fEvent_blink_led2_handler},
	{ EVENT_SUB_BLINK_LED3, 		0, 0, 1000, 	fEvent_blink_led3_handler},
	{ EVENT_SUB_PRINT_DEBUG, 		0, 0, 10, 		fEvent_print_debug_handler},
	{ EVENT_SUB_COLLECT_DATA, 		1, 0, 180000, 	fEvent_collect_handler},
};

void Event_Init(void)
{
	sUartDebug.data = &uartDebugBuff[0];
	sUartDebug.length = 0;
}

void EventTask(void)
{
	CheckEventQueue();
}

static void CheckEventQueue(void)
{
	uint8_t i = 0;

	for (i = 0; i < EVENT_SUB_END; i++)
	{
		if (s_event_sub_handler[i].e_status == 1)
		{
			if((s_event_sub_handler[i].e_systick == 0)||(HAL_GetTick() - s_event_sub_handler[i].e_systick  >=  s_event_sub_handler[i].e_period))
			{
				s_event_sub_handler[i].e_systick = HAL_GetTick();
				s_event_sub_handler[i].e_function_handler(i);
			}
		}
	}
}

uint8_t fevent_active(sEvent_struct *event_struct, uint8_t event_name)
{
	// check input data
	event_struct[event_name].e_status = 1;
	event_struct[event_name].e_systick = 0;

	// push to queue
	return 1;
}

uint8_t fevent_enable(sEvent_struct *event_struct, uint8_t event_name)
{
	// check input data
	event_struct[event_name].e_status = 1;
	event_struct[event_name].e_systick = HAL_GetTick();

	// push to queue
	return 1;
}

uint8_t fevent_disable(sEvent_struct *event_struct, uint8_t event_name)
{
	// check input data
	event_struct[event_name].e_status = 0;

	// reject cauz status = 0 (don't need status variable)
	return 1;
}


/* handler function */
static uint8_t fEvent_blink_led1_handler(uint8_t event)
{
	HAL_GPIO_TogglePin(PB12_GPIO_Port, PB12_Pin);

	return 1;
}

static uint8_t fEvent_blink_led2_handler(uint8_t event)
{
	HAL_GPIO_TogglePin(PB13_GPIO_Port, PB13_Pin);
	Meter_PushToQueueToSend(M_MESSAGE_OPEATION);
//	HAL_IWDG_Refresh(&hiwdg);
	return 1;
}

static uint8_t fEvent_blink_led3_handler(uint8_t event)
{
	HAL_GPIO_TogglePin(PB14_GPIO_Port, PB14_Pin);

	return 1;
}
static uint8_t fEvent_print_debug_handler(uint8_t event)
{
	fSend_String_to_Uart(&uart_debug, (char *)sUartDebug.data);
	Sub_event_disable(event);
	return 1;
}

static uint8_t fEvent_collect_handler(uint8_t event)
{
	Meter_PushToQueueToRead(M_MESSAGE_OPEATION);
	return 1;
}

uint8_t Sub_event_disable(uint8_t event_sub)
{
	if (event_sub >= EVENT_SUB_END)
		return 0;
	fevent_disable(s_event_sub_handler, event_sub);

	return 1;
}

uint8_t Sub_event_enable(uint8_t event_sub)
{
	if (event_sub >= EVENT_SUB_END)
		return 0;
	fevent_enable(s_event_sub_handler, event_sub);

	return 1;
}

uint8_t Sub_event_active(uint8_t event_sub)
{
	if (event_sub >= EVENT_SUB_END)
		return 0;
	fevent_active(s_event_sub_handler, event_sub);

	return 1;
}
