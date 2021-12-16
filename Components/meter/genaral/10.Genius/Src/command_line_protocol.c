#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"

#include "crc_utility.h"
#include "command_line_protocol.h"
#include "string.h"


uint8_t cmd_username2[6] = {'E','D','M','I'};
uint8_t cmd_password2[8] = {'I','M','D','E','I','M','D','E'};

uint8_t cmd_username[6] = {'M','K','H','D','O','C'};
uint8_t cmd_password[8] = {'M','K','H','D','O','C'};



truct_String    StrUser = {&cmd_username[0], 6};
truct_String    StrPass = {&cmd_password[0], 6};

uint8_t cmd_generateCommand(uint8_t cmdType, const uint32_t regId,const uint32_t* Buff_Reg, uint8_t legnth_reg, uint32_t StartRecord, uint16_t Numrec, uint16_t recordSize) 
{
	uint16_t    crc;
	uint16_t    i = 0;
	uint32_t    regIDOneErrorCode = 0x0000FFF0;
    truct_String    Str_Send = {&BuffRecord[0], 0};    
    
	//Them lenh vao
	switch (cmdType)  
	{
		case MIF_COMMANDTYPE_CONNECT:
			Add_Byte_2Buff(&Str_Send, CMD_ESC);
			Add_Byte_2Buff(&Str_Send, CMD_STX);
			Add_Byte_2Buff(&Str_Send, CMD_ETX);
			break;
		case MIF_COMMANDTYPE_LOGON:
			Add_Byte_2Buff(&Str_Send, CMD_STX);
		
			crc = CalculateCharacterCRC16(0,CMD_STX);
		
			Add_Byte_2Buff(&Str_Send, CMD_LOGON);
			Add_Array_2Buff(&Str_Send, StrUser.Data_a8, StrUser.Length_u16);
		
			Add_Byte_2Buff(&Str_Send, CMD_COMMA);  //day phay
			Add_Array_2Buff(&Str_Send, StrPass.Data_a8, StrPass.Length_u16);
			Add_Byte_2Buff(&Str_Send, CMD_NULL);
			//tinh crc
			for (i=1; i<Str_Send.Length_u16; i++) 
				crc = CalculateCharacterCRC16(crc,*(Str_Send.Data_a8 + i));
			//them crc
			Add_2byte_2Buff(&Str_Send, (uint16_t) crc); 
			//them ki tu ket thuc
			Add_Byte_2Buff(&Str_Send, CMD_ETX);   //khong co ki tu dac biet khong can tim
			break;
		case MIF_COMMANDTYPE_LOGOUT:
			Add_Byte_2Buff(&Str_Send, CMD_STX);
			crc = CalculateCharacterCRC16(0,CMD_STX);
			Add_Byte_2Buff(&Str_Send, CMD_LOGOFF);
//			Add_Byte_2Buff(&Str_Send, CMD_NULL);
			for (i=1; i<Str_Send.Length_u16; i++) 
				crc = CalculateCharacterCRC16(crc,*(Str_Send.Data_a8 + i));
			//them crc
			Add_2byte_2Buff(&Str_Send, (uint16_t) crc); 
			//them ki tu ket thuc
			Add_Byte_2Buff(&Str_Send, CMD_ETX);   //khong co ki tu dac biet khong can tim
			break;
		case MIF_COMMANDTYPE_INFO:
			Add_Byte_2Buff(&Str_Send, CMD_STX);
			crc = CalculateCharacterCRC16(0,CMD_STX);
			Add_Byte_2Buff(&Str_Send, CMD_INFO);  // CMD_READ
			//them Reg_ID
            Add_Reg_2Buff (&Str_Send, regId);
			//tinh crc
			for (i=1; i<Str_Send.Length_u16; i++) 
				crc = CalculateCharacterCRC16(crc,*(Str_Send.Data_a8 + i));
			//them crc
			Add_2byte_2Buff(&Str_Send, (uint16_t) crc); 
			//them ki tu ket thuc
			Add_Byte_2Buff(&Str_Send, CMD_ETX);  
			//tim ki tu dac biet va them
			replaceControlCharacter(&Str_Send);
			break;
		case MIF_COMMANDTYPE_READ: 
			Add_Byte_2Buff(&Str_Send, CMD_STX);
			crc = CalculateCharacterCRC16(0,CMD_STX);
			Add_Byte_2Buff(&Str_Send, CMD_MULTIPLE);  // CMD_READ
			//them Reg_ID
			 Add_Reg_2Buff (&Str_Send, regId);
			//tinh crc
			for (i=1; i<Str_Send.Length_u16; i++) 
			{
				crc = CalculateCharacterCRC16(crc,*(Str_Send.Data_a8 + i));
			}
//			Add_Byte_2Buff(&Str_Send, CMD_DOU_READ); 
			//them crc
			Add_2byte_2Buff(&Str_Send, (uint16_t) crc); 
			//them ki tu ket thuc
			Add_Byte_2Buff(&Str_Send, CMD_ETX);  
			//tim ki tu dac biet va them
			replaceControlCharacter(&Str_Send);
			break;
		case MIF_COMMANDTYPE_MULTIPLE:
			Add_Byte_2Buff(&Str_Send, CMD_STX);
			crc = CalculateCharacterCRC16(0,CMD_STX);
			Add_Byte_2Buff(&Str_Send, CMD_MULTIPLE);
			
			Add_Reg_2Buff (&Str_Send,regIDOneErrorCode);
		
			for(i = 0; i < legnth_reg; i++)
			{
				Add_Reg_2Buff (&Str_Send, (uint32_t) *(Buff_Reg + i));
			}
			
			//tinh crc
			for (i=1; i<Str_Send.Length_u16; i++) 
			{
				crc = CalculateCharacterCRC16(crc,*(Str_Send.Data_a8 + i));
			}
			//them crc
			Add_2byte_2Buff(&Str_Send, (uint16_t) crc); 
			//them ki tu ket thuc
			Add_Byte_2Buff(&Str_Send, CMD_ETX);  
			//tim ki tu dac biet va them
			replaceControlCharacter(&Str_Send);
			
			break;
		
		case MIF_COMMANDTYPE_FILE_READ:
			Add_Byte_2Buff(&Str_Send, CMD_STX);
			crc = CalculateCharacterCRC16(0,CMD_STX);
		
			Add_2byte_2Buff(&Str_Send, (uint16_t) CMD_FILE_READ);
			//Register ID
			Add_Reg_2Buff (&Str_Send,regId);
			//Start record
			Add_Reg_2Buff (&Str_Send, StartRecord);
			//NoRecord
			Add_2byte_2Buff(&Str_Send, Numrec);          //2 byte
			//Read offset
			Add_2byte_2Buff(&Str_Send, 0); //2 byte
			//Record size
			Add_2byte_2Buff(&Str_Send, (uint16_t) recordSize);   //2 byte
			//tinh crc
			for (i=1; i<Str_Send.Length_u16; i++) 
			{
				crc = CalculateCharacterCRC16(crc,*(Str_Send.Data_a8 + i));
			}
			//them crc
			Add_2byte_2Buff(&Str_Send, (uint16_t) crc); 
			//them ki tu ket thuc
			Add_Byte_2Buff(&Str_Send, CMD_ETX);  
			//tim ki tu dac biet va them
			replaceControlCharacter(&Str_Send);

			break;
		case MIF_COMMANDTYPE_FILE_INFO:
			Add_Byte_2Buff(&Str_Send, CMD_STX);
			crc = CalculateCharacterCRC16(0,CMD_STX);
		
			Add_2byte_2Buff(&Str_Send, (uint16_t) CMD_FILE_INFO);
		
			Add_Reg_2Buff (&Str_Send,regId);

			//tinh crc
			for (i=1; i<Str_Send.Length_u16; i++) 
			{
				crc = CalculateCharacterCRC16(crc,*(Str_Send.Data_a8 + i));
			}
			
			//tinh crc
			for (i=1; i<Str_Send.Length_u16; i++) 
			{
				crc = CalculateCharacterCRC16(crc,*(Str_Send.Data_a8 + i));
			}
			//them crc
			Add_2byte_2Buff(&Str_Send, (uint16_t) crc); 
			//them ki tu ket thuc
			Add_Byte_2Buff(&Str_Send, CMD_ETX);  
			//tim ki tu dac biet va them
			replaceControlCharacter(&Str_Send);
			break;
		default:
			
			break;
	}
	//Send via 232
    HAL_UART_Transmit(&UART_METER,  Str_Send.Data_a8, Str_Send.Length_u16, 1000);

	return 1;
}


void Add_Byte_2Buff (truct_String* Str, uint8_t Byte)
{
	*(Str->Data_a8 + Str->Length_u16) =  Byte;
	Str->Length_u16 ++;
}

void Add_2byte_2Buff (truct_String* Str, uint16_t Reg_ID)
{
	*(Str->Data_a8 + Str->Length_u16) =  (uint8_t) (Reg_ID>>8);
	Str->Length_u16 ++;
	*(Str->Data_a8 + Str->Length_u16) =  (uint8_t) Reg_ID;
	Str->Length_u16 ++;
}



void Add_Array_2Buff (truct_String* Str, uint8_t* Array, uint16_t length_Ar)
{
	uint16_t i = 0;
	
	for(i = 0; i < length_Ar; i++)
	{
		*(Str->Data_a8 + Str->Length_u16) =  *(Array + i);
		Str->Length_u16++;
	}
}


void Add_Reg_2Buff (truct_String* Str, uint32_t Reg_ID)
{
	*(Str->Data_a8 + Str->Length_u16) =  (uint8_t) (Reg_ID>>24);
	Str->Length_u16 ++;
	*(Str->Data_a8 + Str->Length_u16) =  (uint8_t) (Reg_ID>>16);
	Str->Length_u16 ++;
	*(Str->Data_a8 + Str->Length_u16) =  (uint8_t) (Reg_ID>>8);
	Str->Length_u16 ++;
	*(Str->Data_a8 + Str->Length_u16) =  (uint8_t) Reg_ID;
	Str->Length_u16 ++;
}

static uint8_t isControlCharacter(uint8_t character) 
{
	switch (character) 
	{
		case CMD_STX: /* fall through */
		case CMD_ETX:
			case CMD_XON:
			case CMD_XOFF:
			case CMD_DLE:
			return 1;
		default:
			return 0;
	}
}

void replaceControlCharacter(truct_String* arr) 
{
	uint16_t  i = 1;
	uint16_t  j = 0;

	while (i < (arr->Length_u16 - 1))
    {
		if (isControlCharacter(*(arr->Data_a8 + i)) == 1) 
		{
			j = 0;
            
            //day mang ra phia sau them 1 o
            for(j = arr->Length_u16; j >= (i + 2) ; j--)
              *(arr->Data_a8 + j) = *(arr->Data_a8 + j - 1);
			
             //them 0x10 vao
             *(arr->Data_a8 + i + 1) = 0x40 + *(arr->Data_a8 + i);
             *(arr->Data_a8 + i)  = 0x10;
             //tang phan tu mang len 1
             arr->Length_u16++;
             //tang chi sô quet vong for i lên 2
             i = i+1;
		}
		i++;
	}
}






