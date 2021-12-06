/*
 * elster.h
 *
 *  Created on: Nov 30, 2021
 *      Author: lenovo
 */

#ifndef ELSTER_H_
#define ELSTER_H_

#include "stdint.h"

#define uart_meter	huart1

typedef enum
{
	EVENT_METER_CMD_SEND = 0,
	EVENT_METER_CMD_SEND_OK,
	EVENT_METER_CMD_SEND_TIMEOUT,
	EVENT_METER_UART_RECEIVE,
	EVENT_METER_READ_MESSAGE,
	EVENT_METER_END,
}Event_Meter_TypeDef;

typedef enum
{
	M_ELSTER_CMD_HAND_1 = 0,
	M_ELSTER_CMD_HAND_2,
	M_ELSTER_CMD_PASSWORD_1,
	M_ELSTER_CMD_PASSWORD_2,
	M_ELSTER_CMD_READ_ID,
	M_ELSTER_CMD_END, // don't use
}M_Gelex_TypeDef;

typedef enum
{
	M_MESSAGE_OPEATION = 0,
	M_MESSAGE_EVENT,
	M_MESSAGE_HISTORICAL,
	M_MESSAGE_LOAD_PROFILE,
	M_MESSAGE_END,
}M_Message_TypeDef;

typedef uint8_t (*fp_meter_cmd_callback) (uint8_t *buff);

typedef struct
{
	uint8_t m_cmd_name;
	uint8_t	*m_cmd_buff;
	fp_meter_cmd_callback	packet;
	uint8_t *m_cmd_response;
	fp_meter_cmd_callback	callback_success;
	fp_meter_cmd_callback	callback_failure;
}sCommand_Meter_Struct;

void MeterTask(void);
uint8_t M_PushMeterMessageToQueue(uint8_t meter_message);


#endif /* ELSTER_H_ */
