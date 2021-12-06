/*
 * event_driven.c
 *
 *  Created on: Nov 26, 2021
 *      Author: lenovo
 */


#include "event_driven.h"
#include "sim_command.h"
#include "elster.h"
#include "main_app.h"
#include "stm32l4xx_hal.h"
#include "gpio.h"
#include "iwdg.h"

static sData sUartDebug;
uint8_t uartDebugBuff[100] = {0};
/* */
uint8_t aSimStepBlockLed2[] = { SIM_CMD_CHECK_RSSI, SMI_CMD_GET_IMEI,SIM_CMD_BAUD_RATE};
uint8_t aSimStepBlockLed3[] = { SIM_CMD_CEREG, SIM_CMD_GET_RTC,SIM_CMD_APN_AUTHEN_1};
/* */
static void CheckEventQueue(void);
static uint8_t f_event_blink_led1_handler(uint8_t event);
static uint8_t f_event_blink_led2_handler(uint8_t event);
static uint8_t f_event_blink_led3_handler(uint8_t event);
static uint8_t f_event_print_debug_handler(uint8_t event);
static uint8_t f_event_collect_handler(uint8_t event);
sEvent_struct s_event_sub_handler[] =
{
	{ EVENT_SUB_BLINK_LED1, 		1, 0, 1000, 	f_event_blink_led1_handler},
	{ EVENT_SUB_BLINK_LED2, 		1, 0, 1000, 	f_event_blink_led2_handler},
	{ EVENT_SUB_BLINK_LED3, 		0, 0, 1000, 	f_event_blink_led3_handler},
	{ EVENT_SUB_PRINT_DEBUG, 		0, 0, 10, 		f_event_print_debug_handler},
	{ EVENT_SUB_COLLECT_DATA, 		1, 0, 30000, 	f_event_collect_handler},
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
static uint8_t f_event_blink_led1_handler(uint8_t event)
{
	HAL_GPIO_TogglePin(PB12_GPIO_Port, PB12_Pin);

	return 1;
}

static uint8_t f_event_blink_led2_handler(uint8_t event)
{
	HAL_GPIO_TogglePin(PB13_GPIO_Port, PB13_Pin);
	HAL_IWDG_Refresh(&hiwdg);
	return 1;
}

static uint8_t f_event_blink_led3_handler(uint8_t event)
{
	HAL_GPIO_TogglePin(PB14_GPIO_Port, PB14_Pin);

	return 1;
}
static uint8_t f_event_print_debug_handler(uint8_t event)
{
	fSend_String_to_UartSim(&uart_debug, (char *)sUartDebug.data);
	fevent_disable(s_event_sub_handler, event);
	return 1;
}

static uint8_t f_event_collect_handler(uint8_t event)
{
	M_PushMeterMessageToQueue(M_MESSAGE_OPEATION);

	return 1;
}
