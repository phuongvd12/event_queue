/*
 * Init_Meter.h
 *
 *  Created on: Dec 15, 2015
 *      Author: Administrator
 */

#include "variable.h"
#include "Init_All_Meter.h"

#ifndef _GENIUS_INIT_METER_H_
#define _GENIUS_INIT_METER_H_

/*======================== Structs ======================*/


#define NUMBER_REG_RECORD_INFO_LP					6
#define NUMBER_REG_CHANNEL_INFO_LP					8
#define MAX_LENGHT_ALL_REG_INFOR					200  //100

#define MAX_OBIS_GENIUS                             22

///////
extern const uint32_t   Genius_IDRegisIntan_Table[50];



typedef enum
{
	LP1_GET_RECORD_INFOR,
	LP1_GET_CHANNEL_INFOR,
	LP1_READ_DATA,
    LP2_GET_RECORD_INFOR,
	LP2_GET_CHANNEL_INFOR,
	LP2_READ_DATA,
	END_LP1,
} Step_Read_Lp1;

typedef enum
{
	Bill_GET_RECORD_INFOR,
	Bill_GET_CHANNEL_INFOR,
	Bill_READ_DATA,
} Step_Read_Bill;




typedef struct  
{
	uint8_t modeLoadSurvey;
	uint8_t noChannel;

	uint32_t channelSize;
	uint32_t Number_Entries;

	uint32_t startRecordIndex;  //vi tribat dau
    uint16_t currentInd;      //idex read
    uint16_t NumRecordRead;     //so record
    uint8_t  NumReadone;

	uint32_t recordOffset;
	uint32_t recordSize;
    
    uint32_t Interval;
    ST_TIME_FORMAT  sTimeStart;
    uint16_t ToTalRead;
} cmd_recordInfo_t;


typedef struct  
{
	uint32_t 	regID;
	uint32_t 	size;
	uint32_t    offset;
	float 		scale;
	uint8_t		tag;
} cmd_channelInfo_t;


typedef uint8_t (*_functionCallback)(truct_String *str_Receiv, uint16_t Pos);
typedef struct  
{
	uint8_t 			Step_Cmd;
	uint8_t 			cmdType;
	
	const uint32_t*	    Buff_List_Reg;
	uint8_t				Number_Reg;
	_functionCallback 	CallBack;
	truct_String 		Str_Recei;
	uint16_t 			Length_Data;
} meterInterfaceInfo_t;



typedef struct 
{
    uint8_t                 ObisHex;
    truct_String*     Str_Unit;
    truct_String*     StrObis_Bill;
    truct_String*     StrObis_Lpf;
}struct_Obis_Genius;


extern const meterInterfaceInfo_t 				genius_loadProfileList[];
extern cmd_channelInfo_t 						Lp1_Channel_Infor[20];
extern cmd_channelInfo_t 						Bill_Channel_Infor[20];

extern struct_Obis_Genius       Obis_Genius[MAX_OBIS_GENIUS];
extern const uint32_t           Genius_IDRegisEvent[4];
extern truct_String             StrobisEvent[3];

#endif /* _START_INC_INIT_METER_H_ */


