/*
 * meter_test.c
 *
 *  Created on: Dec 13, 2021
 *      Author: lenovo
 */


#include "meter_test.h"
#include "stddef.h"
#include "queue_p.h"
#include "usart.h"
#include "string.h"

#include "myDefine.h"

extern sQueue_Struct_TypeDef QueueMetertoSend;
extern sQueue_Struct_TypeDef QueueMetertoRead;
/* */

void MeterInit(void)
{
	ELSTER_Init_Function(0);
	RS485_DISABLE;
	__HAL_UART_ENABLE_IT(&UART_METER, UART_IT_RXNE);
}

uint8_t Fl_Scan_Meter = 0;
void MeterTask(void)
{
	if (Fl_Scan_Meter == 0)
	{
		osDelay(4000);            //neu disconnect thi no se bao dung meter type
		Func_Scan_NewMeter();
		Fl_Scan_Meter = 1;
	}

	Check_Meter();

	if (Meter_GetFromQueueToRead(1) < M_MESSAGE_END)
	{
		s_event_sub_handler[EVENT_SUB_BLINK_LED3].e_period = 200;
		Sub_event_active(EVENT_SUB_BLINK_LED3);
		if (eMeter_20._f_Read_ID(0) == 1)
		{
            if (eMeter_20._f_Read_TSVH(DATA_OPERATION) == 1)
            {
            	Meter_PushToQueueToSend(M_MESSAGE_OPEATION);
				s_event_sub_handler[EVENT_SUB_BLINK_LED3].e_period = 1000;
            } else {
            	Sub_event_disable(EVENT_SUB_BLINK_LED3);
            }
		} else {
			Sub_event_disable(EVENT_SUB_BLINK_LED3);
		}
	}
}


/* QueueMetertoSend */
uint8_t Meter_PushToQueueToSend(uint8_t message)
{
	if (Q_push_data_to_queue(&QueueMetertoSend, message) == 0)
		return 0;

	return 1;
}

uint8_t Meter_GetFromQueueToSend(uint8_t Type)
{
	uint8_t message = 0;

	if (Q_get_number_items(&QueueMetertoSend) == 0)
		return M_MESSAGE_END;

	message = Q_get_data_from_queue(&QueueMetertoSend, Type);

	return message;
}

/* Read Message */
uint8_t Meter_PushToQueueToRead(uint8_t message)
{
	if (Q_push_data_to_queue(&QueueMetertoRead, message) == 0)
		return 0;

	return 1;
}

uint8_t Meter_GetFromQueueToRead(uint8_t Type)
{
	uint8_t message = 0;

	if (Q_get_number_items(&QueueMetertoRead) == 0)
		return M_MESSAGE_END;

	message = Q_get_data_from_queue(&QueueMetertoRead, Type);

	return message;
}
