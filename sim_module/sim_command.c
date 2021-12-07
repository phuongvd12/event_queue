/*
 * sim_command.c
 *
 *  Created on: Nov 26, 2021
 *      Author: lenovo
 */
#include "string.h"
#include "queue_p.h"
#include "myDefine.h"

#define SIM_CMD_RETRY	3
#define SIM_CMD_FREQ	500
#define SIM_CMD_TIMEOUT	10000
/* define static variable */
uint8_t uartSimBuffReceive[100] = {0};
uint8_t sim_rx_buff[1] = {0};
sData sUartSim;
static uint8_t numRetry;
//
static uint8_t aSimStepBlockConnect[] = {
	SIM_CMD_AT, SIM_CMD_ECHO, SIM_CMD_MAN_LOG, SIM_CMD_ACCESS_RA_AUTO, SIM_CMD_CEREG, SIM_CMD_BAUD_RATE, SIM_CMD_GET_ID, \
	SIM_CMD_CHECK_RSSI, SMI_CMD_GET_IMEI, SIM_CMD_CHECK_SIM, SIM_CMD_CHECK_ATTACH, SIM_CMD_SYN_TIME_ZONE, SIM_CMD_GET_RTC,\
	SIM_CMD_TCP_COFI_CONTEXT_2, SIM_CMD_APN_AUTHEN_1, SIM_CMD_TCP_CLOSE, SIM_CMD_TCP_TRANS_SETUP, SIM_CMD_TCP_TRANS,\
	SIM_CMD_TCP_NETOPEN, SIM_CMD_TCP_CONNECT, SIM_CMD_TCP_SEND_MESSAGE
};
/* */
extern sQueue_Struct_TypeDef QueueSimStep;
/* define static functions */
static uint8_t at_callback_success(uint8_t *uart_string);
static uint8_t at_callback_failure(uint8_t *uart_string);
static uint8_t at_connect_TCP_callback_success(uint8_t *uart_string);
static uint8_t at_send_message_callback_success(uint8_t *uart_string);
/* event handler function */
static uint8_t fevent_sim_at_send_handler(uint8_t event);
static uint8_t fevent_sim_at_send_ok_handler(uint8_t event);
static uint8_t fevent_sim_at_send_timeout_handler(uint8_t event);
static uint8_t fevent_sim_uart_receive_handler(uint8_t event);
static uint8_t fevent_sim_turn_on_handler(uint8_t event);
sEvent_struct s_event_sim_handler[] =
{
	{ EVENT_SIM_AT_SEND, 			0, 0, SIM_CMD_FREQ,    	fevent_sim_at_send_handler },
	{ EVENT_SIM_AT_SEND_OK, 		0, 0, 1000, 			fevent_sim_at_send_ok_handler },
	{ EVENT_SIM_AT_SEND_TIMEOUT, 	0, 0, 1000, 			fevent_sim_at_send_timeout_handler },
	{ EVENT_SIM_UART_RECEIVE, 		0, 0, 10, 				fevent_sim_uart_receive_handler },
	{ EVENT_SIM_TURN_ON, 			0, 0, 10, 				fevent_sim_turn_on_handler },
//	{ EVENT_SIM_SEND_MESSAGE, 		0, 0, 10, 				fevent_sim_turn_on_handler },
};

const sCommand_Sim_Struct aSimStep[] =
{
	{	SIM_CMD_AT,				"AT\r\n", 		 	"OK", 			at_callback_success,   at_callback_failure	},
	{	SIM_CMD_ECHO,			"ATE1\r\n", 	 	"OK", 			at_callback_success,   at_callback_failure	},
	{   SIM_CMD_MAN_LOG,    	"AT+MLOGK=5\r", 	"OK",			at_callback_success,   at_callback_failure	},
	{	SIM_CMD_ACCESS_RA_AUTO, "AT+CNMP=2\r",		"OK",			at_callback_success,   at_callback_failure	},
	{	SIM_CMD_CEREG, 			"AT+CEREG=0\r",		"OK",			at_callback_success,   at_callback_failure	},
	{   SIM_CMD_BAUD_RATE, 		"AT+IPR=115200\r",	"OK",			at_callback_success,   at_callback_failure	},
	{	SIM_CMD_GET_ID,			"AT+CICCID\r\n", 	"OK", 			at_callback_success,   at_callback_failure	},
	{	SIM_CMD_CHECK_RSSI,		"AT+CSQ\r\n", 	 	"OK", 		    at_callback_success,   at_callback_failure	},
	{   SMI_CMD_GET_IMEI, 		"AT+CGSN\r",		"OK", 		    at_callback_success,   at_callback_failure	},
	{	SIM_CMD_CHECK_SIM,		"AT+CPIN?\r\n",  	"OK", 			at_callback_success,   at_callback_failure	},
	{	SIM_CMD_CHECK_ATTACH,	"AT+CGATT?\r",		"+CGATT: 1",	at_callback_success,   at_callback_failure	},
	{	SIM_CMD_SYN_TIME_ZONE, 	"AT+MTZ=1\r",		"OK",			at_callback_success,   at_callback_failure	},
	{	SIM_CMD_GET_RTC, 		"AT+CCLK?\r",		"+CCLK:",		at_callback_success,   at_callback_failure	},

	{	SIM_CMD_TCP_COFI_CONTEXT_2,"AT+CGDCONT=1,\"IP\",\"V3G2057\"\r",	"OK",	at_callback_success,   at_callback_failure	},// AT+CGDCONT=1,"IP","V3G2057"  - "AT+CGDCONT=1,\"IP\",\"",19}
	{	SIM_CMD_APN_AUTHEN_1,	"AT$QCPDPP=1,0\r",	"OK",					at_callback_success,   at_callback_failure	},// AT$QCPDPP=1,0 - AT$QCPDPP=1,",12}

	{	SIM_CMD_TCP_CLOSE,	 	"AT+CIPCLOSE=0\r",  "OK",					at_callback_success,   at_callback_failure	},

	{	SIM_CMD_TCP_TRANS_SETUP,"AT+MCIPCFGPL=0,1,0,0,0\r","OK", 			at_callback_success,   at_callback_failure	},

	{	SIM_CMD_TCP_TRANS,		"AT+CIPMODE=2\r",	"OK",		 			at_callback_success,   at_callback_failure	},
	{	SIM_CMD_TCP_NETOPEN,	"AT+NETOPEN\r",		"+NETOPEN:SUCCESS",		at_callback_success,   at_callback_failure	},

	{	SIM_CMD_TCP_CONNECT,	"AT+CIPOPEN=0,\"TCP\",\"172.16.13.13\",1330,0\r", "CONNECT \r\n", 		at_connect_TCP_callback_success,   at_callback_failure	},

	{	SIM_CMD_TCP_SEND_MESSAGE,"#SV1_9_6#113456700005##172.18.9.68#100%+CSQ: -69+UNKNOWN#", "ACK", 	at_send_message_callback_success,   at_callback_failure	},

	// ...
	{	SIM_CMD_END, 			NULL,		NULL,	at_callback_success,   at_callback_failure},
};

const sCommand_Sim_Struct aSimUrc[] =
{
	{SIM_URC_RESET_SIM900,		NULL, 	"NORMAL POWER DOWN", 		at_callback_success,   at_callback_failure	},// OK
	{SIM_URC_ALREADY_CONNECT, 	NULL, 	"ALREADY CONNECT",   		at_callback_success,   at_callback_failure	},
	{SIM_URC_SIM_LOST, 			NULL, 	"SIM CRASH",		 		at_callback_success,   at_callback_failure	},
	{SIM_URC_SIM_REMOVE, 		NULL, 	"SIM REMOVED",		 		at_callback_success,   at_callback_failure	},
	{SIM_URC_CLOSED,			NULL, 	"+SERVER DISCONNECTED", 	at_callback_success,   at_callback_failure	},
	{SIM_URC_PDP_DEACT, 		NULL,	"+NETWORK DISCONNECTED",    at_callback_success,   at_callback_failure	},   //+NETWORK DISCONNECTED:0
	{SIM_URC_CALL_READY, 		NULL,	"Call Ready",				at_callback_success,   at_callback_failure	},
	{SIM_URC_ERROR, 			NULL,	"ERROR",					at_callback_success,   at_callback_failure	},
};

void SimInit(void)
{
	sUartSim.data = &uartSimBuffReceive[0];
	sUartSim.length = 0;

	HAL_UART_Receive_IT(&uart_sim, sim_rx_buff, 1);
	fevent_active(s_event_sim_handler, EVENT_SIM_TURN_ON);
}

void SimTask(void)
{
	uint8_t i = 0;

	for (i = 0; i < EVENT_SIM_END; i++)
	{
		if (s_event_sim_handler[i].e_status == 1)
		{
			if((s_event_sim_handler[i].e_systick == 0)||(HAL_GetTick() - s_event_sim_handler[i].e_systick  >=  s_event_sim_handler[i].e_period))
			{
				s_event_sim_handler[i].e_systick = HAL_GetTick();
				s_event_sim_handler[i].e_function_handler(i);
			}
		}
	}
}

uint8_t SimON(void)
{
	static uint8_t OnStep = 0;

	switch(OnStep)
	{
	case 0:
		fevent_disable(s_event_sim_handler, EVENT_SIM_AT_SEND);
		SIM_PW_OFF1;
		SIM_PWKEY_OFF1;
		s_event_sim_handler[EVENT_SIM_TURN_ON].e_period = 2000;
		break;
	case 1:
		SIM_PW_ON1;
		s_event_sim_handler[EVENT_SIM_TURN_ON].e_period = 2000;
		break;
	case 2:
		SIM_PWKEY_ON1;
		s_event_sim_handler[EVENT_SIM_TURN_ON].e_period = 1000;
		break;
	case 3:
		SIM_PWKEY_OFF1;
		s_event_sim_handler[EVENT_SIM_TURN_ON].e_period = 5000;
		break;
	case 4:
		s_event_sim_handler[EVENT_SIM_TURN_ON].e_period = 10000;
		break;
	default:
		fevent_disable(s_event_sim_handler, EVENT_SIM_TURN_ON);
		OnStep = 0;
		return 1;
		break;
	}
	OnStep++;

	return 0;
}

/* */
static uint8_t fevent_sim_at_send_handler(uint8_t event)
{
	uint8_t sim_step = 0;

	if (numRetry < SIM_CMD_RETRY)
	{
		sim_step = fGetSimStepFromQueue(0); // clear from queue later
		if (sim_step >= SIM_CMD_END)
		{
			fevent_disable(s_event_sim_handler, event);
		}
		else
		{
			numRetry++;
			fSend_String_to_UartSim(&uart_sim, aSimStep[sim_step].at_string);
			s_event_sim_handler[event].e_period = SIM_CMD_TIMEOUT;
		}
	}
	else
	{
		numRetry = 0;
		fevent_active(s_event_sim_handler, EVENT_SIM_AT_SEND_TIMEOUT);
	}

	return 1;
}

static uint8_t fevent_sim_at_send_ok_handler(uint8_t event)
{
	numRetry = 0;
	fevent_disable(s_event_sim_handler, event);

	s_event_sim_handler[EVENT_SIM_AT_SEND].e_period = SIM_CMD_FREQ;
	s_event_sim_handler[EVENT_SIM_AT_SEND].e_systick = HAL_GetTick();

	fGetSimStepFromQueue(1); // clear AT from queue to complete

	return 1;
}

static uint8_t fevent_sim_at_send_timeout_handler(uint8_t event)
{
	uint8_t sim_step = 0;

	fevent_disable(s_event_sim_handler, event);
	sim_step = fGetSimStepFromQueue(0);

	if (sim_step < SIM_CMD_TCP_TRANS) {
		fClearSimStepQueue();
		sim_step = SIM_CMD_AT;	// check this point again
		fPushSimStepToQueue(sim_step);
	}
	else
	{
		fevent_active(s_event_sim_handler, EVENT_SIM_TURN_ON);
	}

	return 1;
}

static uint8_t fevent_sim_uart_receive_handler(uint8_t event)
{
	uint8_t sim_step = 0;

	sim_step = fGetSimStepFromQueue(0);
	Sim_Check_Response(sim_step);

	return 1;
}

static uint8_t fevent_sim_turn_on_handler(uint8_t event)
{
	if (SimON() == 1)
	{
		fClearSimStepQueue();
		fPushBlockSimStepToQueue(aSimStepBlockConnect, sizeof(aSimStepBlockConnect));
	}

	return 1;
}

uint8_t fPushSimStepToQueue(uint8_t sim_step)
{
	if (sim_step >= SIM_CMD_END)
		return 0;

	if (Q_push_data_to_queue(&QueueSimStep, sim_step) == 0)
		return 0;

	if (s_event_sim_handler[EVENT_SIM_AT_SEND].e_status == 0)
		fevent_active(s_event_sim_handler, EVENT_SIM_AT_SEND);

	return 1;
}

uint8_t fPushBlockSimStepToQueue(uint8_t *block_sim_step, uint8_t size)
{
	uint8_t i = 0;

	for (i = 0; i < size; i++)
	{
		if (fPushSimStepToQueue(block_sim_step[i]) == 0)
			return 0;
	}

	return 1;
}

uint8_t fClearSimStepQueue(void)
{
	if (Q_clear_queue(&QueueSimStep) == 0)
		return 0;

	return 1;
}
/*
 * Type = 0 - don't clear queue
 * */
uint8_t fGetSimStepFromQueue(uint8_t Type)
{
	uint8_t sim_step = 0;

	if (Q_get_number_items(&QueueSimStep) == 0)
		return SIM_CMD_END;
	sim_step = Q_get_data_from_queue(&QueueSimStep, Type);

	return sim_step;
}

void fSend_String_to_UartSim(UART_HandleTypeDef *huart, char *string) // alternative by DMA or IT tranfer
{
	HAL_UART_Transmit(huart, (uint8_t *)string, strlen(string), 1000);
}


static uint8_t at_callback_success(uint8_t *uart_string)
{

	return 1;
}

static uint8_t at_callback_failure(uint8_t *uart_string)
{

	return 1;
}

static uint8_t at_send_message_callback_success(uint8_t *uart_string)
{

	return 1;
}

static uint8_t at_connect_TCP_callback_success(uint8_t *uart_string)
{
	s_event_sub_handler[EVENT_SUB_BLINK_LED1].e_period = 200;
	fevent_active(s_event_sub_handler, EVENT_SUB_BLINK_LED1);

	return 1;
}

uint8_t Sim_Check_Response(uint8_t sim_step) // alternative by ring buffer
{
	char *p = NULL;

	if (sim_step < SIM_CMD_END)
	{
		p = strstr((char*)sUartSim.data, aSimStep[sim_step].at_response);
		if (p != NULL)
		{
			fevent_active(s_event_sim_handler, EVENT_SIM_AT_SEND_OK);
			aSimStep[sim_step].callback_success(sUartSim.data);
		}
	}
	memcpy(uartDebugBuff, uartSimBuffReceive, sizeof(uartSimBuffReceive));
	memset(&uartSimBuffReceive[0], 0, sizeof(uartSimBuffReceive));
	sUartSim.length = 0;
	fevent_active(s_event_sub_handler, EVENT_SUB_PRINT_DEBUG);
	fevent_disable(s_event_sim_handler, EVENT_SIM_UART_RECEIVE);

	return 1;
}


