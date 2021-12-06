/*
 * elster.c
 *
 *  Created on: Nov 30, 2021
 *      Author: lenovo
 */

#include "elster.h"
#include "event_driven.h"
#include "sim_command.h"
#include "stddef.h"
#include "queue_p.h"
#include "usart.h"

extern sQueue_Struct_TypeDef QueueMeterCmd;
extern sQueue_Struct_TypeDef QueueMeterMessage;
/* */
static uint8_t a_elster_hand1[] = {0x2F,0x3F,0x21,0x0D,0x0A};
static uint8_t a_elster_hand2[] = {0x06,0x30,0x35,0x31,0x0D,0x0A};
static uint8_t a_elster_response[] = {0x0D, 0x0A};
static uint8_t a_elster_hand_block[] = {M_ELSTER_CMD_HAND_1, M_ELSTER_CMD_HAND_2};
/* */
static uint8_t fevent_meter_cmd_send_handler(uint8_t event);
static uint8_t fevent_meter_cmd_send_OK_handler(uint8_t event);
static uint8_t fevent_meter_cmd_send_Timeout_handler(uint8_t event);
static uint8_t fevent_meter_uart_receive_handler(uint8_t event);
static uint8_t fevent_meter_read_message_handler(uint8_t event);
/* */
static uint8_t meter_pack_message(uint8_t *buff);
static uint8_t meter_cmd_callback_success(uint8_t *buff);
static uint8_t meter_cmd_callback_failure(uint8_t *buff);
/* */
static uint8_t M_GetMeterCmdToQueue(uint8_t Type);

sEvent_struct s_event_meter_handler[] =
{
	{ EVENT_METER_CMD_SEND, 		0, 0, 1000,    			fevent_meter_cmd_send_handler },
	{ EVENT_METER_CMD_SEND_OK, 		0, 0, 1000, 			fevent_meter_cmd_send_OK_handler },
	{ EVENT_METER_CMD_SEND_TIMEOUT, 0, 0, 1000, 			fevent_meter_cmd_send_Timeout_handler },
	{ EVENT_METER_UART_RECEIVE, 	0, 0, 10, 				fevent_meter_uart_receive_handler },
	{ EVENT_METER_READ_MESSAGE, 	0, 0, 30000, 			fevent_meter_read_message_handler },
};

sCommand_Meter_Struct	cmdGelexBlock[] =
{
	{ M_ELSTER_CMD_HAND_1, 		a_elster_hand1, meter_pack_message, a_elster_response, meter_cmd_callback_success, meter_cmd_callback_failure},
	{ M_ELSTER_CMD_HAND_2, 		a_elster_hand2, meter_pack_message, a_elster_response, meter_cmd_callback_success, meter_cmd_callback_failure},
	{ M_ELSTER_CMD_PASSWORD_1, 	a_elster_hand2, meter_pack_message, a_elster_response, meter_cmd_callback_success, meter_cmd_callback_failure},
	{ M_ELSTER_CMD_PASSWORD_2, 	a_elster_hand2, meter_pack_message, a_elster_response, meter_cmd_callback_success, meter_cmd_callback_failure},
	{ M_ELSTER_CMD_READ_ID, 	a_elster_hand2, meter_pack_message, a_elster_response, meter_cmd_callback_success, meter_cmd_callback_failure},

	//...
	{ M_ELSTER_CMD_END, 		NULL, 			meter_pack_message, NULL, 				meter_cmd_callback_success, meter_cmd_callback_failure},
};

void MeterTask(void)
{
	uint8_t i = 0;

	for (i = 0; i < EVENT_METER_END; i++)
	{
		if (s_event_meter_handler[i].e_status == 1)
		{
			if((s_event_meter_handler[i].e_systick == 0)||(HAL_GetTick() - s_event_meter_handler[i].e_systick  >=  s_event_meter_handler[i].e_period))
			{
				s_event_meter_handler[i].e_systick = HAL_GetTick();
				s_event_meter_handler[i].e_function_handler(i);
			}
		}
	}
}

static uint8_t fevent_meter_cmd_send_handler(uint8_t event)
{
	uint8_t cmd = 0;

	cmd = M_GetMeterCmdToQueue(1); // (0)
	if (cmd >= M_ELSTER_CMD_END)
	{
		fevent_disable(s_event_meter_handler, event);
		fPushSimStepToQueue(SIM_CMD_TCP_SEND_MESSAGE);
	}
	else
	{
		HAL_UART_Transmit(&uart_meter, cmdGelexBlock[cmd].m_cmd_buff, 5, 1000); // fix
	}

	return 1;
}
static uint8_t fevent_meter_cmd_send_OK_handler(uint8_t event)
{

	return 1;
}
static uint8_t fevent_meter_cmd_send_Timeout_handler(uint8_t event)
{

	return 1;
}
static uint8_t fevent_meter_uart_receive_handler(uint8_t event)
{

	return 1;
}

/* QueueMeterCmd */
static uint8_t M_GetMeterCmdToQueue(uint8_t Type)
{
	uint8_t cmd = 0;

	if (Q_get_number_items(&QueueMeterCmd) == 0)
		return M_ELSTER_CMD_END;

	cmd = Q_get_data_from_queue(&QueueMeterCmd, Type);

	return cmd;
}

static uint8_t M_PushMeterCmdToQueue(uint8_t cmd)
{
	if (cmd >= M_ELSTER_CMD_END)
		return 0;

	if (Q_push_data_to_queue(&QueueMeterCmd, cmd) == 0)
		return 0;

	if (s_event_meter_handler[EVENT_METER_CMD_SEND].e_status == 0)
		fevent_active(s_event_meter_handler, EVENT_METER_CMD_SEND);

	return 1;
}

uint8_t M_PushMeterCmdBlockToQueue(uint8_t *block_cmd_step, uint8_t size)
{
	uint8_t i = 0;

	for (i = 0; i < size; i++)
	{
		if (M_PushMeterCmdToQueue(block_cmd_step[i]) == 0)
			return 0;
	}

	return 1;
}

/* QueueMeterMessage */
uint8_t M_PushMeterMessageToQueue(uint8_t meter_message)
{
	if (Q_push_data_to_queue(&QueueMeterMessage, meter_message) == 0)
		return 0;

	fevent_active(s_event_meter_handler, EVENT_METER_READ_MESSAGE);

	return 1;
}

static uint8_t M_GetMeterMessageFromQueue(uint8_t Type)
{
	uint8_t message = 0;

	if (Q_get_number_items(&QueueMeterMessage) == 0)
		return M_MESSAGE_END;

	message = Q_get_data_from_queue(&QueueMeterMessage, Type);

	return message;
}

static uint8_t fevent_meter_read_message_handler(uint8_t event)
{
	uint8_t message = 0;


	message = M_GetMeterMessageFromQueue(1);
	switch (message)
	{
	case M_MESSAGE_OPEATION:
		M_PushMeterCmdBlockToQueue(a_elster_hand_block, sizeof(a_elster_hand_block));
		break;
	case M_MESSAGE_LOAD_PROFILE:
		break;
	case M_MESSAGE_HISTORICAL:
		break;
	case M_MESSAGE_END:
		fevent_disable(s_event_meter_handler, event);
		break;
	default:
		break;
	}

	return 1;
}


/* */
static uint8_t meter_pack_message(uint8_t *buff)
{
	if (buff == NULL)
		return 0;

	return 1;
}

static uint8_t meter_cmd_callback_success(uint8_t *buff)
{
	if (buff == NULL)
		return 0;

	return 1;
}
static uint8_t meter_cmd_callback_failure(uint8_t *buff)
{
	if (buff == NULL)
		return 0;

	return 1;
}
