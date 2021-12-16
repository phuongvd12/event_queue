/*
 * flash_ext.h
 *
 *  Created on: Dec 14, 2021
 *      Author: lenovo
 */

#ifndef FLASH_EXT_FLASH_EXT_H_
#define FLASH_EXT_FLASH_EXT_H_

#include "myDefine.h"
#include "event_driven.h"

#define FLASH_CMD_TIMEOUT 1000
#define FLASH_CMD_FREQ 	  5

typedef enum
{
	EVENT_FLASH_SEND_BYTE = 0,
	EVENT_FLASH_WRITE_BUFF,
	EVENT_FLASH_READ_BUFF,
	EVENT_FLASH_INIT,
	EVENT_FLASH_END, // don't use
}Event_flash_TypeDef;

typedef enum
{

	FLASH_CMD_READ_STATUS = 0,
	FLASH_CMD_CHECK_WRITE_ENABLE,
	FLASH_CMD_CHECK_ERASE_ENABLE,

	FLASH_CMD_CHECK_WRITE_END,
	FLASH_CMD_CHECK_ERASE_END,

	FLASH_CMD_ERASE_SECTOR,
	FLASH_CMD_WRITE_DATA,
	FLASH_CMD_READ_DATA,

	FLASH_CMD_ACTIVE_CS,
	FLASH_CMD_DEACTIVE_CS,

	FLASH_CMD_ENABLE,

	FLASH_CMD_END, // don't use
}Cmd_flash_TypeDef;

typedef uint8_t (*fp_flash_cmd_callback) (void);

typedef struct
{
	uint8_t cmd_name;
	uint8_t cmd_byte;
	uint8_t cmd_check;
	fp_flash_cmd_callback callback_success;
	fp_flash_cmd_callback callback_failure;
}sFlash_CMD_struct;

uint8_t FlashInit(void);
void FlashTask(void);

uint8_t Flash_GetMessageFromQueueToRead(uint8_t Type);
uint8_t Flash_PushMessageToQueueToRead(uint8_t message);
#endif /* FLASH_EXT_FLASH_EXT_H_ */
