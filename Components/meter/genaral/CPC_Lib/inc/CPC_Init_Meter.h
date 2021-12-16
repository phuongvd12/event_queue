/*
 * Init_Meter.h
 *
 *  Created on: Dec 15, 2015
 *      Author: Administrator
 */
#include "variable.h"
#include "sim900.h"

#ifndef _START_INC_INIT_METER_H_
#define _START_INC_INIT_METER_H_

/*======================== Define ======================*/
#define IMPORT_ACTIVE_POWER		    0x01
#define IMPORT_ACTIVE_POWER		    0x01
#define IMPORT_ACTIVE_POWER		    0x01

#define CPC_MAX_OBIS_TSVH           50
#define CPC_MAX_OBIS_BILL           12
#define CPC_MAX_OBIS_EVENT          30
#define CPC_MAX_OBIS_LPF            5

#define MAX_OBIS103_TSVH            26
#define MAX_OBIS_BIEU               24
#define POS_OBIS_TARIFF_TSVH         0
#define POS_OBIS_TARIFF_BILL        12

#define CPC_TIME_WAIT_SEM           4000


/*======================== Extern variable ======================*/

/*-----------------Cac mã nhan dang dai luong event-----------------------*/
typedef struct
{
    truct_String*    StartCountOB;
    truct_String*    StartTimeOB;
    truct_String*    StopCountOB;
    truct_String*    StopTimeOB;
}Struct_Event_Obis;


extern uint8_t	    CPC_KEY_ENCRYPT[16];
extern uint8_t		CPC_Handshake1[5];
extern uint8_t		CPC_Handshake2[6];

extern uint8_t 		        password_1[10];
extern uint8_t 		        password_2[10];
extern uint8_t 		        password_3[10];
//
extern uint8_t              CPC_MeterTuTi_IDTable[2];
extern uint8_t              CPC_MeterInfoMessIDTable[CPC_MAX_OBIS_TSVH];
extern uint8_t              CPC_MeterBillMessIDTable[CPC_MAX_OBIS_BILL];
extern uint8_t              CPC_MeterEvenMessIDTable[CPC_MAX_OBIS_EVENT];
extern uint8_t              CPC_MeterLpfMessIDTable[CPC_MAX_OBIS_LPF];

//
extern uint8_t              CPC_TSVH_ObisCode[CPC_MAX_OBIS_TSVH][11];
extern uint8_t              CPC_LASTBILL_ObisCode[CPC_MAX_OBIS_BILL][11];
extern uint8_t              CPC_LPF_ObisCode[CPC_MAX_OBIS_LPF][11];
//
extern truct_String*        CPC_OBIS_BIEU[MAX_OBIS_BIEU];
extern truct_String*        CPC_Obis103_TSVH[MAX_OBIS103_TSVH];
extern Struct_Event_Obis    CPC_Str_OEvent103[29];
//


#endif /* _START_INC_INIT_METER_H_ */
