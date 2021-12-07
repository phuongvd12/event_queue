/*
 * elster.c
 *
 *  Created on: Nov 30, 2021
 *      Author: lenovo
 */
#include "stddef.h"
#include "queue_p.h"
#include "usart.h"
#include "string.h"
#include "myDefine.h"

#define METER_CMD_RETRY		3
#define METER_CMD_FREQ		2000
#define METER_CMD_TIMEOUT	10000
extern sQueue_Struct_TypeDef QueueMeterCmd;
extern sQueue_Struct_TypeDef QueueMeterMessage;
uint8_t uartMeterBuffReceive[300] = {0};
uint8_t meter_rx_buff[1] = {0};
sData sUartMeter;
static uint8_t numRetry;
//
const static uint8_t a_elster_hand1[] = {0x2F,0x3F,0x21,0x0D,0x0A};
const static uint8_t a_elster_hand2[] = {0x06,0x30,0x35,0x31,0x0D,0x0A};
const static uint8_t a_elster_hand3[] = {0x81,0x50,0xB2,0x82,0x28,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xA9,0x03,0x00};
const static uint8_t a_elster_get_id[]= {0x81,0xD2,0xB1,0x82,0xB7,0x39,0xB8,0x30,0x30,0xB1,0x28,0xB1,0x30,0xA9,0x03,0x65};
const static uint8_t a_elster_dummy1[] = 			{0x81,0xD2,0xB1,0x82,0x36,0x30,0x35,0x30,0x30,0xB1,0x28,0x30,0xB4,0xA9,0x03,0x65};
const static uint8_t a_elster_dummy2[] = 			{0x81,0xD2,0xB1,0x82,0x36,0x30,0x36,0x30,0x30,0xB1,0x28,0xB1,0xC3,0xA9,0x03,0x90};
const static uint8_t a_elster_voltage[] =			{0x81,0xD7,0xB1,0x82,0x36,0x30,0x35,0x30,0x30,0xB1,0x28,0xB1,0x42,0xB2,0x42,0xB4,0x42,0x30,0x30,0xA9,0x03,0x11};//VoltagePhaseA,B,C
const static uint8_t a_elster_current[] = 		{0x81,0xD7,0xB1,0x82,0x36,0x30,0x35,0x30,0x30,0xB1,0x28,0xB1,0x41,0xB2,0x41,0xB4,0x41,0x30,0x30,0xA9,0x03,0x12};//CurrentPhaseA,B,C
const static uint8_t a_elster_frequency[] = 		{0x81,0xD7,0xB1,0x82,0x36,0x30,0x35,0x30,0x30,0xB1,0x28,0xB1,0xB8,0xB2,0xB8,0xB4,0xB8,0x30,0x30,0xA9,0x03,0xEB};//FrequencyPhaseA,B,C
const static uint8_t a_elster_phaseAngle[] = 		{0x81,0xD7,0xB1,0x82,0x36,0x30,0x35,0x30,0x30,0xB1,0x28,0xB1,0x39,0xB2,0x39,0xB4,0x39,0x30,0x30,0xA9,0x03,0x6A};//PhaseAnglePhaseA,B,C
const static uint8_t a_elster_activePower[] = 	{0x81,0xD7,0xB1,0x82,0x36,0x30,0x35,0x30,0x30,0xB1,0x28,0xB1,0xC3,0xB2,0xC3,0xB4,0xC3,0x30,0xC3,0xA9,0x03,0x63};//ActivePowerPhaseA,B,C,Total
const static uint8_t a_elster_reactivePower[] = 	{0x81,0xD7,0xB1,0x82,0x36,0x30,0x35,0x30,0x30,0xB1,0x28,0xB1,0x44,0xB2,0x44,0xB4,0x44,0x30,0x44,0xA9,0x03,0x63};//ReactivePowerPhaseA,B,C,Total
const static uint8_t a_elster_apprentPower[] = 	{0x81,0xD7,0xB1,0x82,0x36,0x30,0x35,0x30,0x30,0xB1,0x28,0xB1,0xC5,0xB2,0xC5,0xB4,0xC5,0x30,0xC5,0xA9,0x03,0x63};//ApparentPowerPhaseA,B,C,Total
const static uint8_t a_elster_powerFactor[] = 	{0x81,0xD7,0xB1,0x82,0x36,0x30,0x35,0x30,0x30,0xB1,0x28,0xB1,0x33,0xB2,0x33,0xB4,0x33,0x30,0x33,0xA9,0x03,0x63};//PowerFactorPhaseA,B,C,Total
const static uint8_t a_elster_phaseRotation[] = 	{0x81,0xD7,0xB1,0x82,0x36,0x30,0x35,0x30,0x30,0xB1,0x28,0x30,0xB7,0x30,0x30,0x30,0x30,0x30,0x30,0xA9,0x03,0x63};//PhaseRotation
const static uint8_t a_elster_ImExport_Q[] = 		{0x81,0xD2,0xB1,0x82,0x35,0x30,0xB7,0x30,0x30,0xB1,0x28,0xB4,0x30,0xA9,0x03,0xE4};//Cumulative,0xTotal:,0xImportWh,ExportWh,Q1,Q2,Q3,Q4,Vah
const static uint8_t a_elster_Enegry_Pus_Sub[] = 	{0x81,0xD2,0xB1,0x82,0x35,0x30,0xB8,0x30,0x30,0xB1,0x28,0xB4,0x30,0xA9,0x03,0xEB};//EnergyPlusArate1-3,EnergySubArate1-3
const static uint8_t a_elster_Max_Demand[] = 		{0x81,0xD2,0xB1,0x82,0x35,0xB1,0x30,0x30,0x30,0xB1,0x28,0xB4,0x30,0xA9,0x03,0xE2};//MaxDemandPlusARate13-Time
const static uint8_t a_elster_Tu_TuM[] = 			{0x81,0xD2,0xB1,0x82,0x36,0xB1,0xB4,0x30,0x30,0xB1,0x28,0x30,0xB7,0xA9,0x03,0x66};//Tu-TuM
const static uint8_t a_elster_Ti_TiM[] = 			{0x81,0xD2,0xB1,0x82,0x36,0xB1,0x36,0x30,0x30,0xB1,0x28,0x30,0x36,0xA9,0x03,0x65};//Ti-TiM
const static uint8_t a_elster_Meter_Time[] = 		{0x81,0xD2,0xB1,0x82,0xB8,0x36,0xB1,0x30,0x30,0xB1,0x28,0x30,0xB7,0xA9,0x03,0x6A}; //MeterTime
const static uint8_t a_elster_end[5] =  {0x81,0x42,0x30,0x03,0x71};
//
static uint8_t a_elster_response[] = {0x0D, 0x0A};
static uint8_t a_elster_cmd_buff[30] = {0}; // max 22
/* block command */
static uint8_t a_MeterStepBlockGetID[] = {
		M_ELSTER_CMD_HAND_1, M_ELSTER_CMD_HAND_2, M_ELSTER_CMD_HAND_3,
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
};
/* */
static uint8_t fevent_meter_cmd_send_handler(uint8_t event);
static uint8_t fevent_meter_cmd_send_OK_handler(uint8_t event);
static uint8_t fevent_meter_cmd_send_Timeout_handler(uint8_t event);
static uint8_t fevent_meter_uart_receive_handler(uint8_t event);
static uint8_t fevent_meter_read_message_handler(uint8_t event);
/* */
static uint8_t meter_pack_message(const uint8_t *in_buff, uint8_t *out_put, uint8_t length);
static uint8_t meter_cmd_callback_success(uint8_t *buff);
static uint8_t meter_cmd_callback_failure(uint8_t *buff);
/* */
static uint8_t M_GetMeterCmdFromQueue(uint8_t Type);
static uint8_t M_ClearMeterStepQueue(void);
static uint8_t M_PushMeterCmdToQueue(uint8_t cmd);

sEvent_struct s_event_meter_handler[] =
{
	{ EVENT_METER_CMD_SEND, 		0, 0, METER_CMD_FREQ,   fevent_meter_cmd_send_handler },
	{ EVENT_METER_CMD_SEND_OK, 		0, 0, 1000, 			fevent_meter_cmd_send_OK_handler },
	{ EVENT_METER_CMD_SEND_TIMEOUT, 0, 0, 1000, 			fevent_meter_cmd_send_Timeout_handler },
	{ EVENT_METER_UART_RECEIVE, 	0, 0, 10, 				fevent_meter_uart_receive_handler },
	{ EVENT_METER_READ_MESSAGE, 	0, 0, 30000, 			fevent_meter_read_message_handler },
};

const sCommand_Meter_Struct	aElsterStep[] =
{
	{ M_ELSTER_CMD_HAND_1, 		a_elster_hand1, sizeof(a_elster_hand1), 	meter_pack_message, a_elster_response, meter_cmd_callback_success, meter_cmd_callback_failure},
	{ M_ELSTER_CMD_HAND_2, 		a_elster_hand2, sizeof(a_elster_hand2),  	meter_pack_message, a_elster_response, meter_cmd_callback_success, meter_cmd_callback_failure},
	{ M_ELSTER_CMD_HAND_3, 		a_elster_hand3, sizeof(a_elster_hand3), 	meter_pack_message, a_elster_response, meter_cmd_callback_success, meter_cmd_callback_failure},
	{ M_ELSTER_CMD_READ_ID, 	a_elster_get_id, sizeof(a_elster_get_id), 	meter_pack_message, a_elster_response, meter_cmd_callback_success, meter_cmd_callback_failure},
	{ M_ELSTER_CMD_DUMMY1, 		a_elster_dummy1, sizeof(a_elster_dummy1),	meter_pack_message, a_elster_response, meter_cmd_callback_success, meter_cmd_callback_failure},
	{ M_ELSTER_CMD_DUMMY2, 		a_elster_dummy2, sizeof(a_elster_dummy2),	meter_pack_message, a_elster_response, meter_cmd_callback_success, meter_cmd_callback_failure},
	{ M_ELSTER_CMD_VOLTAGE, 	a_elster_voltage, sizeof(a_elster_voltage),	meter_pack_message, a_elster_response, meter_cmd_callback_success, meter_cmd_callback_failure},
	{ M_ELSTER_CMD_CURRENT, 	a_elster_current, sizeof(a_elster_current),	meter_pack_message, a_elster_response, meter_cmd_callback_success, meter_cmd_callback_failure},
	{ M_ELSTER_CMD_FREQUENCY, 	a_elster_frequency,sizeof(a_elster_frequency),	meter_pack_message, a_elster_response, meter_cmd_callback_success, meter_cmd_callback_failure},
	{ M_ELSTER_CMD_PHASE_ANGLE, 	a_elster_phaseAngle, sizeof(a_elster_phaseAngle),	meter_pack_message, a_elster_response, meter_cmd_callback_success, meter_cmd_callback_failure},
	{ M_ELSTER_CMD_ACTIVE_POWER, 	a_elster_activePower,sizeof(a_elster_activePower),	meter_pack_message, a_elster_response, meter_cmd_callback_success, meter_cmd_callback_failure},
	{ M_ELSTER_CMD_RE_ACTIVE_POWER, a_elster_reactivePower,	sizeof(a_elster_reactivePower), meter_pack_message, a_elster_response, meter_cmd_callback_success, meter_cmd_callback_failure},
	{ M_ELSTER_CMD_APPRENT_POWER, 	a_elster_apprentPower, sizeof(a_elster_apprentPower),	meter_pack_message, a_elster_response, meter_cmd_callback_success, meter_cmd_callback_failure},
	{ M_ELSTER_CMD_POWER_FACTOR, 	a_elster_powerFactor, sizeof(a_elster_powerFactor),		meter_pack_message, a_elster_response, meter_cmd_callback_success, meter_cmd_callback_failure},
	{ M_ELSTER_CMD_PHASE_ROTATION, 	a_elster_phaseRotation, sizeof(a_elster_phaseRotation),	meter_pack_message, a_elster_response, meter_cmd_callback_success, meter_cmd_callback_failure},
	{ M_ELSTER_CMD_IM_EXPORT_Q, 	a_elster_ImExport_Q, sizeof(a_elster_ImExport_Q),		meter_pack_message, a_elster_response, meter_cmd_callback_success, meter_cmd_callback_failure},
	{ M_ELSTER_CMD_ENEGRY_PLUS_SUB, 	a_elster_Enegry_Pus_Sub, sizeof(a_elster_Enegry_Pus_Sub), meter_pack_message, a_elster_response, meter_cmd_callback_success, meter_cmd_callback_failure},
	{ M_ELSTER_CMD_MAX_DEMAND, 	a_elster_Max_Demand, sizeof(a_elster_Max_Demand),	meter_pack_message, a_elster_response, meter_cmd_callback_success, meter_cmd_callback_failure},
	{ M_ELSTER_CMD_TU_TUM, 		a_elster_Tu_TuM, sizeof(a_elster_Tu_TuM),			meter_pack_message, a_elster_response, meter_cmd_callback_success, meter_cmd_callback_failure},
	{ M_ELSTER_CMD_TI_TIM, 		a_elster_Ti_TiM, sizeof(a_elster_Ti_TiM),			meter_pack_message, a_elster_response, meter_cmd_callback_success, meter_cmd_callback_failure},
	{ M_ELSTER_CMD_METER_TIME, 	a_elster_Meter_Time, sizeof(a_elster_Meter_Time), 	meter_pack_message, a_elster_response, meter_cmd_callback_success, meter_cmd_callback_failure},


	{ M_ELSTER_CMD_DISCONNECT, 	a_elster_end, 	sizeof(a_elster_end), meter_pack_message, a_elster_response, meter_cmd_callback_success, meter_cmd_callback_failure},

	//...
	{ M_ELSTER_CMD_END, 		NULL, 			0, meter_pack_message, NULL, 				meter_cmd_callback_success, meter_cmd_callback_failure},
};

void MeterInit(void)
{
	sUartMeter.data = &uartMeterBuffReceive[0];
	sUartMeter.length = 0;

	RS485_DISABLE;
	HAL_UART_Receive_IT(&uart_meter, meter_rx_buff, 1);
}

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

	if (numRetry < METER_CMD_RETRY)
	{
		cmd = M_GetMeterCmdFromQueue(0);
		if (cmd >= M_ELSTER_CMD_END)
		{
			fevent_disable(s_event_meter_handler, event);
			fPushSimStepToQueue(SIM_CMD_TCP_SEND_MESSAGE);
		}
		else
		{
			numRetry++;
			// pack data
			aElsterStep[cmd].pack_data(aElsterStep[cmd].m_cmd_buff, a_elster_cmd_buff, aElsterStep[cmd].m_cmd_length);
			HAL_UART_Transmit(&uart_meter, a_elster_cmd_buff, aElsterStep[cmd].m_cmd_length, 1000);
			s_event_meter_handler[event].e_period = METER_CMD_TIMEOUT;
		}
	}
	else
	{
		numRetry = 0;
		fevent_active(s_event_meter_handler, EVENT_METER_CMD_SEND_TIMEOUT);
	}

	return 1;
}
static uint8_t fevent_meter_cmd_send_OK_handler(uint8_t event)
{
	numRetry = 0;
	fevent_disable(s_event_meter_handler, event);

	s_event_meter_handler[EVENT_METER_CMD_SEND].e_period = METER_CMD_FREQ;
	s_event_meter_handler[EVENT_METER_CMD_SEND].e_systick = HAL_GetTick();

	M_GetMeterCmdFromQueue(1);

	return 1;
}

static uint8_t fevent_meter_cmd_send_Timeout_handler(uint8_t event)
{
	uint8_t meter_step = 0;

	fevent_disable(s_event_meter_handler, event);
	meter_step = M_GetMeterCmdFromQueue(0);

//	if (meter_step < M_ELSTER_CMD_END)
//	{
		M_ClearMeterStepQueue();
		meter_step = M_ELSTER_CMD_HAND_1;
		M_PushMeterCmdToQueue(meter_step);
//	}

	return 1;
}

static uint8_t Meter_Check_Response(uint8_t meter_step)
{
	char *p = NULL;

	if (meter_step < M_ELSTER_CMD_END)
	{
		p = strstr((char*)sUartMeter.data, (char*)aElsterStep[meter_step].m_cmd_response);
		if (p != NULL)
		{
			fevent_active(s_event_meter_handler, EVENT_METER_CMD_SEND_OK);
			aElsterStep[meter_step].callback_success(sUartMeter.data);
		}
	}
	memcpy(uartDebugBuff, uartMeterBuffReceive, sizeof(uartMeterBuffReceive));
	memset(&uartMeterBuffReceive[0], 0, sizeof(uartMeterBuffReceive));
	sUartMeter.length = 0;
	fevent_active(s_event_sub_handler, EVENT_SUB_PRINT_DEBUG);
	fevent_disable(s_event_meter_handler, EVENT_METER_UART_RECEIVE);


	return 1;
}

static uint8_t fevent_meter_uart_receive_handler(uint8_t event)
{
	uint8_t meter_step = 0;

	meter_step = M_GetMeterCmdFromQueue(0);
	Meter_Check_Response(meter_step);

	return 1;
}

static uint8_t M_ClearMeterStepQueue(void)
{
	if (Q_clear_queue(&QueueMeterCmd) == 0)
		return 0;

	return 1;
}
/* QueueMeterCmd */
static uint8_t M_GetMeterCmdFromQueue(uint8_t Type)
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
		M_PushMeterCmdBlockToQueue(a_MeterStepBlockGetID, sizeof(a_MeterStepBlockGetID));
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

uint8_t Copy_buff(const uint8_t *in_buff, uint8_t *out_buff, uint8_t length)
{
	uint8_t i = 0;

	for (i = 0; i < length; i++)
	{
		out_buff[i] = in_buff[i]&0x7F; // 7 bit data and even parity
	}

	return i;
}

static uint8_t meter_pack_message(const uint8_t *in_buff, uint8_t *out_buff, uint8_t length)
{
	Copy_buff(in_buff, out_buff, length);

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
