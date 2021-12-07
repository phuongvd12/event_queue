/*
 * sim_command.h
 *
 *  Created on: Nov 26, 2021
 *      Author: lenovo
 */

#ifndef SIM_COMMAND_H_
#define SIM_COMMAND_H_

#include "usart.h"

#define SIM_PW_PORT1   ON_OFF_SIM_GPIO_Port
#define SIM_PW_PIN1    ON_OFF_SIM_Pin
#define SIM_PW_OFF1    HAL_GPIO_WritePin(SIM_PW_PORT1,SIM_PW_PIN1,GPIO_PIN_RESET)
#define SIM_PW_ON1     HAL_GPIO_WritePin(SIM_PW_PORT1,SIM_PW_PIN1,GPIO_PIN_SET)

#define SIM_PWKEY_PORT1  PWRKEY_GPIO_Port
#define SIM_PWKEY_PIN1   PWRKEY_Pin
#define SIM_PWKEY_ON1    HAL_GPIO_WritePin(SIM_PWKEY_PORT1,SIM_PWKEY_PIN1,GPIO_PIN_SET)
#define SIM_PWKEY_OFF1   HAL_GPIO_WritePin(SIM_PWKEY_PORT1,SIM_PWKEY_PIN1,GPIO_PIN_RESET)

typedef enum
{
	SIM_CMD_AT,
	SIM_CMD_ECHO,
	SIM_CMD_MAN_LOG,
	SIM_CMD_ACCESS_RA_AUTO,
	SIM_CMD_CEREG,
	SIM_CMD_BAUD_RATE,
	SIM_CMD_GET_ID,
	SIM_CMD_CHECK_RSSI,
	SMI_CMD_GET_IMEI,
	SIM_CMD_CHECK_SIM,
	SIM_CMD_CHECK_ATTACH,
	SIM_CMD_SYN_TIME_ZONE,
	SIM_CMD_GET_RTC,
	SIM_CMD_TCP_COFI_CONTEXT_2,
	SIM_CMD_APN_AUTHEN_1,
	SIM_CMD_TCP_CLOSE,
	SIM_CMD_TCP_TRANS_SETUP,
	SIM_CMD_TCP_TRANS,
	SIM_CMD_TCP_NETOPEN,
	SIM_CMD_TCP_CONNECT,
	SIM_CMD_TCP_SEND_MESSAGE,
	SIM_CMD_END, // don't use
}AT_SIM_TypeDef;

typedef enum
{
	SIM_URC_RESET_SIM900 = 0,
	SIM_URC_ALREADY_CONNECT,
	SIM_URC_SIM_LOST,
	SIM_URC_SIM_REMOVE,
	SIM_URC_CLOSED,
	SIM_URC_PDP_DEACT,
	SIM_URC_CALL_READY,
	SIM_URC_ERROR,
	SIM_URC_END,
}URC_SIM_TypeDef;

typedef enum
{
	EVENT_SIM_AT_SEND = 0,
	EVENT_SIM_AT_SEND_OK,
	EVENT_SIM_AT_SEND_TIMEOUT,
	EVENT_SIM_UART_RECEIVE,
	EVENT_SIM_TURN_ON,
	EVENT_SIM_END, // don't use
}Event_sim_TypeDef;

typedef uint8_t (*fp_at_callback) (uint8_t *string);

typedef struct
{
	uint8_t at_name;
	char	*at_string;
	char 	*at_response;
	fp_at_callback	callback_success;
	fp_at_callback	callback_failure;
}sCommand_Sim_Struct; // it is not a declaration, it is a new type of data

/* external */
extern const sCommand_Sim_Struct aSimStep[];

/* define function */
void SimInit(void);
void SimTask(void);
uint8_t fPushSimStepToQueue(uint8_t sim_step);
uint8_t fPushBlockSimStepToQueue(uint8_t *block_sim_step, uint8_t size);
uint8_t fClearSimStepQueue(void);
uint8_t fGetSimStepFromQueue(uint8_t Type);
void fSend_String_to_UartSim(UART_HandleTypeDef *huart, char *string);

uint8_t Sim_Check_Response(uint8_t sim_step);
#endif /* SIM_COMMAND_H_ */
