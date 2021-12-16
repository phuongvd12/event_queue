/*
 * Init_Meter.h
 *
 *  Created on: Dec 15, 2015
 *      Author: Administrator
 */

#include "variable.h"
#include "Init_All_Meter.h"

#ifndef _START_INIT_METER_H_
#define _START_INIT_METER_H_

#define                     START_MAX_REGIS_INFOR       108
#define                     START_MAX_REGIS_EVENT       40

#define                     START_MAX_EVENT             40
#define                     START_MAX_OBIS_INTAN        53
   
   

typedef struct 
{
    uint8_t           ObisHex;
    truct_String*     Str_Obis;
    uint8_t                 scale;
    truct_String*     Str_Unit;
    truct_String*     StrObis_Bill;
}struct_Obis_START;


/*======================== Constant ======================*/
extern uint8_t	Read_Meter_ID_Message[17];
extern uint8_t	Read_Meter_Info_Template[17];
extern uint8_t	Read_Meter_LProf_Template1[20];
extern uint8_t	Read_Meter_LProf_Template2[19];
extern uint8_t	Read_Meter_LProf_StartCode[2];

extern uint8_t		START_MeterTiTuIDTable[2];
extern uint8_t		START_MeterInfoMessIDTable[36];
extern uint8_t	    MeterBillingMessIDTableFirst[10];
extern uint8_t		MeterBillingMessIDTable[10];
extern uint8_t		MeterEventMessIDTable[25];

extern uint8_t	Read_Meter_Info_Code[START_MAX_REGIS_INFOR][4];
extern uint8_t	Read_Meter_Event_Code[START_MAX_REGIS_EVENT][5];

extern uint8_t		            Type_Meter;
extern struct_Obis_START        Obis_Start_Ins[START_MAX_OBIS_INTAN];
extern struct_Obis_event        Obis_Start_Event[START_MAX_EVENT];
//
void    Init_IDTable_Readevent (void);


#endif /* _START_INC_INIT_METER_H_ */



