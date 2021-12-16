/*
 * mater_test.h
 *
 *  Created on: Dec 13, 2021
 *      Author: lenovo
 */

#ifndef METER_TEST_H_
#define METER_TEST_H_

#include "stdint.h"
#include "myUart.h"

#define uart_meter	huart4//uart_debug//huart4

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
	M_ELSTER_CMD_HAND_3,
	M_ELSTER_CMD_READ_ID,
	M_ELSTER_CMD_DUMMY1,
	M_ELSTER_CMD_DUMMY2,
	M_ELSTER_CMD_VOLTAGE,
	M_ELSTER_CMD_CURRENT,
	M_ELSTER_CMD_FREQUENCY,
	M_ELSTER_CMD_PHASE_ANGLE,
	M_ELSTER_CMD_ACTIVE_POWER,
	M_ELSTER_CMD_RE_ACTIVE_POWER,
	M_ELSTER_CMD_APPRENT_POWER,
	M_ELSTER_CMD_POWER_FACTOR,
	M_ELSTER_CMD_PHASE_ROTATION,
	M_ELSTER_CMD_IM_EXPORT_Q,
	M_ELSTER_CMD_ENEGRY_PLUS_SUB,
	M_ELSTER_CMD_MAX_DEMAND,
	M_ELSTER_CMD_TU_TUM,
	M_ELSTER_CMD_TI_TIM,
	M_ELSTER_CMD_METER_TIME,

	M_ELSTER_CMD_DISCONNECT,
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

typedef uint8_t (*fp_meter_cmd_pack) (const uint8_t *in_buff, uint8_t *out_buff, uint8_t length);

typedef struct
{
	uint8_t m_cmd_name;
	const uint8_t	*m_cmd_buff;
	uint8_t	m_cmd_length;
	fp_meter_cmd_pack	pack_data;
	uint8_t *m_cmd_response;
	fp_meter_cmd_callback	callback_success;
	fp_meter_cmd_callback	callback_failure;
}sCommand_Meter_Struct;

void MeterInit(void);
void MeterTask(void);
uint8_t Meter_PushToQueueToSend(uint8_t message);
uint8_t Meter_GetFromQueueToSend(uint8_t Type);
uint8_t Meter_PushToQueueToRead(uint8_t message);
uint8_t Meter_GetFromQueueToRead(uint8_t Type);

#endif /* METER_TEST_H_ */
