
#ifndef COMMAND_LINE_PROTOCOL_H_
#define COMMAND_LINE_PROTOCOL_H_

#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"
#include "variable.h"

#define CMD_STX         0x02
#define CMD_ETX         0x03
#define CMD_XON         0x11
#define CMD_XOFF        0x13
#define CMD_DLE         0x10
#define CMD_ESC         0x1B
#define CMD_LOGON       0x4C
#define CMD_LOGOFF      0x58
#define CMD_READ        0x52
#define CMD_DOU_READ    0x44
#define CMD_COMMA       0x2C
#define CMD_NULL        0x00
#define CMD_ACK         0x06
#define CMD_CAN         0x18
#define CMD_INFO        0x4F
#define CMD_MULTIPLE    0x4D
#define CMD_FILE_READ   0x4652
#define CMD_FILE_INFO   0x4649

#define CMD_ADDING_POSITION     6
#define CMD_COMMAND_POSITION    1
#define CMD_START_DATA_POSITION 4
#define CMD_START_DATA_POSITION_CMD_MULTIPLE    6
#define CMD_START_DATA_POSITION_CMD_FILE_READ    17
#define CMD_START_DATA_POSITION_CMD_FILE_INFO    7

#define CMD_USERNAME_LENGTH     20 
#define CMD_PASSWORD_LENGTH     20
#define CMD_READ_UNUSED_LENGTH              7
#define CMD_MULTIPLE_UNUSED_LENGTH          9
#define CMD_FILE_READ_UNUSED_LENGTH         20
#define CMD_FLOAT_LENGTH        4

#define CMD_MAX_NUMBER_RESEND   2



typedef enum _mif_commandType_e 
{
	MIF_COMMANDTYPE_CONNECT = 0,
	MIF_COMMANDTYPE_LOGON = 1,
	MIF_COMMANDTYPE_LOGOUT = 2,
	MIF_COMMANDTYPE_INFO = 3,
	MIF_COMMANDTYPE_READ = 4,
	MIF_COMMANDTYPE_WRITE = 5,
	MIF_COMMANDTYPE_MULTIPLE = 6,
	MIF_COMMANDTYPE_FILE_READ = 7,
	MIF_COMMANDTYPE_EXTEND = 0x08,
	MIF_COMMANDTYPE_FILE_INFO = 0x09,
	MIF_COMMANDTYPE_MAX = 0xFF
} mif_commandType_e;


extern uint8_t cmd_username2[6];
extern uint8_t cmd_password2[8];
extern uint8_t cmd_username[6];
extern uint8_t cmd_password[8];

extern truct_String    StrUser; 
extern truct_String    StrPass;

//
void        Add_Byte_2Buff (truct_String* Str, uint8_t Byte);
void        Add_Array_2Buff (truct_String* Str, uint8_t* Array, uint16_t length_Ar);
void        Add_2byte_2Buff (truct_String* Str, uint16_t Reg_ID);
void        replaceControlCharacter(truct_String* arr) ;
void        Add_Reg_2Buff (truct_String* Str, uint32_t Reg_ID);

uint8_t     cmd_generateCommand(uint8_t cmdType,const uint32_t regId,const uint32_t* Buff_Reg, uint8_t legnth_reg, uint32_t StartRecord, uint16_t Numrec, uint16_t recordSize) ;


#endif /* COMMAND_LINE_PROTOCOL_H_ */
