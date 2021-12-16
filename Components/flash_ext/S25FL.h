/*
 * S25FL.h
 *
 *  Created on: Dec 15, 2021
 *      Author: lenovo
 */

#ifndef FLASH_EXT_S25FL_H_
#define FLASH_EXT_S25FL_H_

#include "myDefine.h"

#define SKIP				0x00

#define LOW    	 			0x00  /* Chip Select line low */
#define HIGH    			0x01  /* Chip Select line high */

#define S25FL_READ          0x03
#define S25FL_WRITE         0x02
#define S25FL_READ_STATUS   0x05
#define S25FL_WREN          0x06
#define S25FL_SECTOR_ERASE  0x20

#define WIP_Flag   		    0x03 // 0x01 vs 0x02 /* Write In Progress (WIP) flag */
#define WEL_Flag   		    0x02 /* Chua co check Flag WEL */
#define DUMMY_BYTE 		    0xA5

/* Define Size */
#define S25FL_PAGE_SIZE 	256
#define FLASH_S25FL_BASE 	0x000000

void Flash_S25FL_ChipSelect(uint8_t State);
uint8_t Flash_S25FL_Send_Byte(uint8_t byte);

#endif /* FLASH_EXT_S25FL_H_ */
