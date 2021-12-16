/*
 * flash_ext.c
 *
 *  Created on: Dec 14, 2021
 *      Author: lenovo
 */
#include "flash_ext.h"
#include "spi.h"

extern sQueue_Struct_TypeDef QueueFlashtoRead;
extern sQueue_Struct_TypeDef QueueFlashStep;

static uint8_t fEvent_Flash_Send_BYTE_handler(uint8_t event);
static uint8_t fEvent_Flash_Write_buff_handler(uint8_t event);
static uint8_t fEvent_Flash_Read_buff_handler(uint8_t event);
static uint8_t fEvent_Flash_Init_handler(uint8_t event);

static uint8_t Flash_cmd_callback_failure(void);
static uint8_t Flash_cmd_read_status_callback_success(void);
static uint8_t Flash_cmd_check_write_enable_callback_success(void);
static uint8_t Flash_cmd_check_Erase_enable_callback_success(void);
static uint8_t Flash_cmd_check_write_END_callback_success(void);
static uint8_t Flash_cmd_check_Erase_END_callback_success(void);
static uint8_t Flash_cmd_erase_sector_callback_success(void);
static uint8_t Flash_cmd_write_data_callback_success(void);
static uint8_t Flash_cmd_read_data_callback_success(void);

static uint8_t Flash_cmd_active_cs_callback_success(void);
static uint8_t Flash_cmd_Deactive_cs_callback_success(void);
static uint8_t Flash_cmd_Enable_callback_success(void);


static uint8_t Flash_WriteMessage(uint8_t message);
static uint8_t Flash_ReadMessage(uint8_t message);

static uint8_t Flash_PushStepToQueue(uint8_t flash_step);
static uint8_t Flash_PushBlockToQueue(uint8_t *block_flash_step, uint8_t size);
static uint8_t Flash_GetStepFromQueue(uint8_t Type);

uint8_t Flash_event_disable(uint8_t event_flash);
uint8_t Flash_event_enable(uint8_t event_flash);
uint8_t Flash_event_active(uint8_t event_flash);

uint8_t 	flash_cmd = 0;
uint8_t 	num_message = 0;
uint32_t 	w_flash_address;
uint32_t 	r_flash_address;
uint32_t 	e_flash_address;

uint8_t 	aFlash_buff_read[100];
uint8_t 	aFlash_buff_write[13]= {'*', '*', '*', 'S', 'v', '_', '0', '1', '*', '*', '*','\r','\n'};

/*static uint8_t aFlash_Enable_BlockCmd[] = 				{FLASH_CMD_ACTIVE_CS, FLASH_CMD_ENABLE, FLASH_CMD_DEACTIVE_CS};
static uint8_t aFlash_CheckWrite_Enable_BlockCmd[] = 	{FLASH_CMD_ACTIVE_CS, FLASH_CMD_READ_STATUS, FLASH_CMD_CHECK_WRITE_ENABLE, FLASH_CMD_DEACTIVE_CS};
static uint8_t aFlash_CheckWrite_End_BlockCmd[] = 		{FLASH_CMD_ACTIVE_CS, FLASH_CMD_READ_STATUS, FLASH_CMD_CHECK_WRITE_END, FLASH_CMD_DEACTIVE_CS};*/

static uint8_t aFlash_Erase_Sector_BlockCmd[] =
{
	FLASH_CMD_ACTIVE_CS, FLASH_CMD_ENABLE, FLASH_CMD_DEACTIVE_CS,	\
	FLASH_CMD_ACTIVE_CS, FLASH_CMD_READ_STATUS, FLASH_CMD_CHECK_ERASE_ENABLE, FLASH_CMD_DEACTIVE_CS,	\
	FLASH_CMD_ACTIVE_CS, FLASH_CMD_ERASE_SECTOR, FLASH_CMD_DEACTIVE_CS, \
	FLASH_CMD_ACTIVE_CS, FLASH_CMD_READ_STATUS, FLASH_CMD_CHECK_ERASE_END, FLASH_CMD_DEACTIVE_CS
};
static uint8_t aFlash_Write_Data_BlockCmd[] =
{
	FLASH_CMD_ACTIVE_CS, FLASH_CMD_ENABLE, FLASH_CMD_DEACTIVE_CS,	\
	FLASH_CMD_ACTIVE_CS, FLASH_CMD_READ_STATUS, FLASH_CMD_CHECK_WRITE_ENABLE, FLASH_CMD_DEACTIVE_CS,	\
	FLASH_CMD_ACTIVE_CS, FLASH_CMD_WRITE_DATA, FLASH_CMD_DEACTIVE_CS, \
	FLASH_CMD_ACTIVE_CS, FLASH_CMD_READ_STATUS, FLASH_CMD_CHECK_WRITE_END, FLASH_CMD_DEACTIVE_CS
};
static uint8_t aFlash_Read_Data_BlockCmd[] =
{
//	FLASH_CMD_ACTIVE_CS, FLASH_CMD_READ_STATUS, FLASH_CMD_CHECK_WRITE_END, FLASH_CMD_DEACTIVE_CS,
	FLASH_CMD_ACTIVE_CS, FLASH_CMD_READ_DATA, FLASH_CMD_DEACTIVE_CS
};

sEvent_struct s_event_flash_handler[] =
{
	{ EVENT_FLASH_SEND_BYTE, 	0, 0, FLASH_CMD_FREQ,   fEvent_Flash_Send_BYTE_handler},
	{ EVENT_FLASH_WRITE_BUFF, 	1, 0, 500,  			fEvent_Flash_Write_buff_handler},
	{ EVENT_FLASH_READ_BUFF, 	1, 0, 500,  			fEvent_Flash_Read_buff_handler},
	{ EVENT_FLASH_INIT, 		0, 0, 1000, 			fEvent_Flash_Init_handler},
};

sFlash_CMD_struct	aFlashCmd[] =
{
	{ FLASH_CMD_READ_STATUS,  		S25FL_READ_STATUS, 	SKIP, 	   	Flash_cmd_read_status_callback_success, Flash_cmd_callback_failure},
	{ FLASH_CMD_CHECK_WRITE_ENABLE, DUMMY_BYTE, 		WEL_Flag, 	Flash_cmd_check_write_enable_callback_success, Flash_cmd_callback_failure},
	{ FLASH_CMD_CHECK_ERASE_ENABLE, DUMMY_BYTE, 		WEL_Flag, 	Flash_cmd_check_Erase_enable_callback_success, Flash_cmd_callback_failure},

	{ FLASH_CMD_CHECK_WRITE_END, 	DUMMY_BYTE, 		WIP_Flag, 	Flash_cmd_check_write_END_callback_success, Flash_cmd_callback_failure},
	{ FLASH_CMD_CHECK_ERASE_END, 	DUMMY_BYTE, 		WIP_Flag, 	Flash_cmd_check_Erase_END_callback_success, Flash_cmd_callback_failure},

	{ FLASH_CMD_ERASE_SECTOR, 		S25FL_SECTOR_ERASE, SKIP, 		Flash_cmd_erase_sector_callback_success, Flash_cmd_callback_failure},
	{ FLASH_CMD_WRITE_DATA, 		S25FL_WRITE, 		SKIP, 		Flash_cmd_write_data_callback_success, Flash_cmd_callback_failure},
	{ FLASH_CMD_READ_DATA, 			S25FL_READ, 		SKIP, 		Flash_cmd_read_data_callback_success, Flash_cmd_callback_failure},

	{ FLASH_CMD_ACTIVE_CS, 	  		SKIP, 		 	 	SKIP, 	    Flash_cmd_active_cs_callback_success, Flash_cmd_callback_failure},
	{ FLASH_CMD_DEACTIVE_CS,  		SKIP, 	 	     	SKIP, 	    Flash_cmd_Deactive_cs_callback_success, Flash_cmd_callback_failure},
	{ FLASH_CMD_ENABLE,  	  		S25FL_WREN, 	 	SKIP, 	    Flash_cmd_Enable_callback_success, Flash_cmd_callback_failure},
};

uint8_t FlashInit(void)
{
	Flash_S25FL_ChipSelect(HIGH);

	return 1;
}

void FlashTask(void)
{
	uint8_t i = 0;

	for (i = 0; i < EVENT_FLASH_END; i++)
	{
		if (s_event_flash_handler[i].e_status == 1)
		{
			if((s_event_flash_handler[i].e_systick == 0)||(HAL_GetTick() - s_event_flash_handler[i].e_systick  >=  s_event_flash_handler[i].e_period))
			{
				s_event_flash_handler[i].e_systick = HAL_GetTick();
				s_event_flash_handler[i].e_function_handler(i);
			}
		}
	}
}

static uint8_t Flash_cmd_callback_failure(void)
{

	Flash_S25FL_ChipSelect(HIGH);
	s_event_flash_handler[EVENT_FLASH_SEND_BYTE].e_period = FLASH_CMD_TIMEOUT;

	fSend_String_to_Uart(&uart_debug, "\r\n flash cmd failure \r\n");

	return 1;
}

static uint8_t Flash_cmd_read_status_callback_success(void)
{
	fSend_String_to_Uart(&uart_debug, "\r\n Read status OK \r\n");

	return 1;
}

static uint8_t Flash_cmd_check_write_enable_callback_success(void)
{
	fSend_String_to_Uart(&uart_debug, "\r\n Write Enable OK \r\n");

	return 1;
}

static uint8_t Flash_cmd_check_Erase_enable_callback_success(void)
{
	fSend_String_to_Uart(&uart_debug, "\r\n Erase Enable OK \r\n");

	return 1;
}

static uint8_t Flash_cmd_check_write_END_callback_success(void)
{
	uint8_t message = 0;

	fSend_String_to_Uart(&uart_debug, "\r\n Write End OK \r\n");

	message = Meter_GetFromQueueToSend(1);
	if (message < M_MESSAGE_END)
		Flash_PushMessageToQueueToRead(message);

	Flash_event_enable(EVENT_FLASH_WRITE_BUFF);
	num_message += 1;

	return 1;
}

static uint8_t Flash_cmd_check_Erase_END_callback_success(void)
{
	fSend_String_to_Uart(&uart_debug, "\r\n Erase END OK \r\n");

	return 1;
}

static uint8_t Flash_cmd_erase_sector_callback_success(void)
{
	fSend_String_to_Uart(&uart_debug, "\r\n Erase Sector OK \r\n");

	/* Send SectorAddr high nibble address byte */
	Flash_S25FL_Send_Byte((w_flash_address & 0xFF0000) >> 16);
	  /* Send SectorAddr medium nibble address byte */
	Flash_S25FL_Send_Byte((w_flash_address & 0xFF00) >> 8);
	  /* Send SectorAddr low nibble address byte */
	Flash_S25FL_Send_Byte(w_flash_address & 0xFF);

	return 1;
}

static uint8_t Flash_cmd_write_data_callback_success(void)
{
	uint8_t i = 0;

	/* Send SectorAddr high nibble address byte */
	Flash_S25FL_Send_Byte((w_flash_address & 0xFF0000) >> 16);
	  /* Send SectorAddr medium nibble address byte */
	Flash_S25FL_Send_Byte((w_flash_address & 0xFF00) >> 8);
	  /* Send SectorAddr low nibble address byte */
	Flash_S25FL_Send_Byte(w_flash_address & 0xFF);

	aFlash_buff_write[6] = num_message/10 + 0x30;
	aFlash_buff_write[7] = num_message%10 + 0x30;

	for (i = 0; i < sizeof(aFlash_buff_write); i++)
		Flash_S25FL_Send_Byte(aFlash_buff_write[i]);

	fSend_String_to_Uart(&uart_debug, "\r\n Write Data OK \r\n");
	fSend_String_to_Uart(&uart_debug, (char*) aFlash_buff_write);

	return 1;
}

static uint8_t Flash_cmd_read_data_callback_success(void)
{
	uint8_t message = 0;
	uint8_t i = 0;

	/* Send SectorAddr high nibble address byte */
	Flash_S25FL_Send_Byte((r_flash_address & 0xFF0000) >> 16);
	  /* Send SectorAddr medium nibble address byte */
	Flash_S25FL_Send_Byte((r_flash_address & 0xFF00) >> 8);
	  /* Send SectorAddr low nibble address byte */
	Flash_S25FL_Send_Byte(r_flash_address & 0xFF);

	for (i = 0; i < sizeof(aFlash_buff_write); i++)
		aFlash_buff_read[i] =  Flash_S25FL_Send_Byte(DUMMY_BYTE);

	fSend_String_to_Uart(&uart_debug, "\r\n Read Data OK \r\n");
	fSend_String_to_Uart(&uart_debug, (char*)aFlash_buff_read);

	message = Flash_GetMessageFromQueueToRead(1);
	Flash_event_enable(EVENT_FLASH_READ_BUFF);

	Sim_PushToQueueToSend(message);

	return 1;
}

static uint8_t Flash_cmd_active_cs_callback_success(void)
{
	Flash_S25FL_ChipSelect(LOW);

	return 1;
}

static uint8_t Flash_cmd_Deactive_cs_callback_success(void)
{
	Flash_S25FL_ChipSelect(HIGH);

	return 1;
}

static uint8_t Flash_cmd_Enable_callback_success(void)
{
	fSend_String_to_Uart(&uart_debug, "\r\n Enable cmd Send OK \r\n");

	return 1;
}

static uint8_t fEvent_Flash_Send_BYTE_handler(uint8_t event)
{
	uint8_t FLASH_Status = 0;
	uint8_t Flag_Check = 0;

	flash_cmd = Flash_GetStepFromQueue(0);

	s_event_flash_handler[event].e_period = FLASH_CMD_FREQ;

	if (flash_cmd >= FLASH_CMD_END) return 0;

	if (aFlashCmd[flash_cmd].cmd_byte == SKIP) {
		aFlashCmd[flash_cmd].callback_success();
		Flash_GetStepFromQueue(1); // clear
		return 1;
	}

	if (flash_cmd == FLASH_CMD_CHECK_WRITE_ENABLE)
		Flag_Check = aFlashCmd[FLASH_CMD_CHECK_WRITE_ENABLE].cmd_check;

	FLASH_Status = Flash_S25FL_Send_Byte(aFlashCmd[flash_cmd].cmd_byte);
	if ((aFlashCmd[flash_cmd].cmd_check == SKIP)||((FLASH_Status & aFlashCmd[flash_cmd].cmd_check) == Flag_Check)) // &
	{
		aFlashCmd[flash_cmd].callback_success();
		Flash_GetStepFromQueue(1); // clear
	}
	else
	{
		aFlashCmd[flash_cmd].callback_failure();
		return 0;
	}

	return 1;
}

static uint8_t fEvent_Flash_Write_buff_handler(uint8_t event)
{
	uint8_t message = 0;

	message = Meter_GetFromQueueToSend(0);
	if (message >= M_MESSAGE_END) return 0;

	// calculator address to write message
	w_flash_address = FLASH_S25FL_BASE + num_message*S25FL_PAGE_SIZE;

	Flash_WriteMessage(message);

	Flash_event_disable(event);

	return 1;
}

static uint8_t fEvent_Flash_Read_buff_handler(uint8_t event)
{
	uint8_t message = 0;

	message = Flash_GetMessageFromQueueToRead(0);
	if (message >= M_MESSAGE_END) return 0;

	// calculator address to read message
	if (num_message > 0)
	{
		r_flash_address = w_flash_address;
		Flash_ReadMessage(message);
		Flash_event_disable(event);
	}

	return 1;
}

static uint8_t fEvent_Flash_Init_handler(uint8_t event)
{


	Flash_event_disable(event);
	return 1;
}

uint8_t Flash_WriteMessage(uint8_t message)
{
	if ((w_flash_address/S25FL_PAGE_SIZE)%16 == 0)
		Flash_PushBlockToQueue(aFlash_Erase_Sector_BlockCmd, sizeof(aFlash_Erase_Sector_BlockCmd));

	Flash_PushBlockToQueue(aFlash_Write_Data_BlockCmd, sizeof(aFlash_Write_Data_BlockCmd));

	return 1;
}

uint8_t Flash_ReadMessage(uint8_t message)
{
	Flash_PushBlockToQueue(aFlash_Read_Data_BlockCmd, sizeof(aFlash_Read_Data_BlockCmd));


	return 1;
}

static uint8_t Flash_PushStepToQueue(uint8_t flash_step)
{
	if (flash_step >= FLASH_CMD_END)
		return 0;

	if (Q_push_data_to_queue(&QueueFlashStep, flash_step) == 0)
		return 0;

	if (s_event_flash_handler[EVENT_FLASH_SEND_BYTE].e_status == 0)
		Flash_event_active(EVENT_FLASH_SEND_BYTE);

	return 1;
}

static uint8_t Flash_PushBlockToQueue(uint8_t *block_flash_step, uint8_t size)
{
	uint8_t i = 0;

	for (i = 0; i < size; i++)
	{
		if (Flash_PushStepToQueue(block_flash_step[i]) == 0)
			return 0;
	}

	return 1;
}

static uint8_t Flash_GetStepFromQueue(uint8_t Type)
{
	uint8_t flash_step = 0;

	if (Q_get_number_items(&QueueFlashStep) == 0)
		return FLASH_CMD_END;
	flash_step = Q_get_data_from_queue(&QueueFlashStep, Type);

	return flash_step;
}

uint8_t Flash_GetMessageFromQueueToRead(uint8_t Type)
{
	uint8_t message = 0;

	if (Q_get_number_items(&QueueFlashtoRead) == 0)
		return M_MESSAGE_END;

	message = Q_get_data_from_queue(&QueueFlashtoRead, Type);

	return message;
}

uint8_t Flash_PushMessageToQueueToRead(uint8_t message)
{
	if (Q_push_data_to_queue(&QueueFlashtoRead, message) == 0)
		return 0;

	return 1;
}

uint8_t Flash_event_disable(uint8_t event_flash)
{
	if (event_flash >= EVENT_FLASH_END)
		return 0;
	fevent_disable(s_event_flash_handler, event_flash);

	return 1;
}

uint8_t Flash_event_enable(uint8_t event_flash)
{
	if (event_flash >= EVENT_FLASH_END)
		return 0;
	fevent_enable(s_event_flash_handler, event_flash);

	return 1;
}

uint8_t Flash_event_active(uint8_t event_flash)
{
	if (event_flash >= EVENT_FLASH_END)
		return 0;
	fevent_active(s_event_flash_handler, event_flash);

	return 1;
}
