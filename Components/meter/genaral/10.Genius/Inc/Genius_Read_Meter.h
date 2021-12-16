/*
 * Read_Meter.h
 *
 *  Created on: Dec 15, 2015
 *      Author: Administrator
 */
#include "variable.h"
#include "myuart.h"
#include "sim900.h"
#include "t_mqtt.h"

#include "GENIUS_Init_Meter.h"
#include "Init_All_Meter.h"

#ifndef _GENIUS_READ_METER_H_
#define _GENIUS_READ_METER_H_

//Fix scale cua dai luong tuc thoi
#define GENIUS_SCALE_VOLTAGE   	       		2       //V
#define GENIUS_SCALE_CURRENT   	       		2       //A

#define GENIUS_SCALE_FREQ      	       		2
#define GENIUS_SCALE_ANGLE_PHASE	       	2
#define GENIUS_SCALE_POW_FACTOR	       		2

#define GENIUS_SCALE_ACTIVE_POW	       		3       //wh.
#define GENIUS_SCALE_REACTIVE_POW	       	3
#define GENIUS_SCALE_APPRENT_POW	       	3

#define GENIUS_SCALE_Q_INTAN   	       		3       //var
//Fix scale cua dai luong thanh ghi dien nang
#define GENIUS_SCALE_TOTAL_ENERGY       	   	3  //kwh.  kvar
//Fix scale cua dai luong thanh ghi biêu giá
#define GENIUS_SCALE_TARRIFF          	   	    3  //kwh.  kvar
//Fix scale cua dai luong thanh ghi công suat cuc dai
#define GENIUS_SCALE_MAX_DEMAND       	   	    3  //kwh.  kvar
//Fix scale cua dai luong TuTi
#define GENIUS_SCALE_TU_TI           	   	    0  //kwh.  kvar
//Fix scale cua dai luong loadprf
#define GENIUS_SCALE_HE_SO_NHAN                0     

#define GENIUS_SCALE_LPF                       3      //so o lpf la float. don vi theo nhu trong phan mem. Don vi nho nhat
#define GENIUS_SCALE_BILL                      3 
   

#define REG_ID_METER  					0xF002
#define	REG_TIME_METER 			 		0xF03D

typedef enum
{
	GE_STEP_FIRST,
	GE_CHECK_ACK_1,
	GE_SEND_LOGIN,
	GE_CHECK_ACK_2,
	GE_STEP_END,
} GE_Step_HandShake;




typedef struct
{
	ST_TIME_FORMAT		Meter_Time;
    uint8_t             Count_Error;
}Genius_Var_Struct;

typedef struct _genius_regIDTagMapping_t 
{
	uint32_t regID;
	uint8_t tag;
} genius_regIDTagMapping_t;

extern Genius_Var_Struct              GeniusVar;
extern cmd_recordInfo_t 			  recordBIll_Inf;
/*
 * 				FUNCTION
 */

void        GENIUS_Init_Function (uint8_t type);
uint8_t     GENIUS_Read_TSVH (uint32_t Type);
uint8_t     GENIUS_Read_Bill (void);
uint8_t     GENIUS_Read_Event (uint32_t TemValue);
uint8_t     GENIUS_Read_Lpf (void);
uint8_t     GENIUS_Read_Infor (void);


//-------------------------UART function--------------------
uint8_t     Genius_Handshake_Handle (void);
//uint8_t     Genius_Detect_Meter(void);
uint8_t     Genius_GetUART1Data(void);
uint8_t     Genius_Get_Meter_ID (uint32_t TempValue);
uint8_t     Genius_Check_Sum (truct_String* Str);
void        Convert_Genius_Respond (truct_String* Str);
uint8_t     Genius_Get_Meter_Time  (uint32_t RegID, ST_TIME_FORMAT* sTime);
uint8_t     Genius_Connect_Meter_Handle(void);
//
uint8_t     Genius_Extract_DataRaw (uint16_t Pos);
uint8_t	    Genius_ReadInfor_1 (const uint32_t* Reg, uint8_t length,Meter_Comm_Struct* Get_Meter);
uint8_t     Genius_ExtractMinfor_data (void);
//
uint8_t     Genius_Minfor_value(uint16_t PosStart, uint8_t length, uint8_t type, truct_String oBIS, truct_String unit, uint8_t ScaleNum, uint8_t fEnablescale);
uint8_t     Genius_CheckResetReadMeter(uint32_t Timeout);
uint8_t     Genius_Check_Meter(void);
uint8_t     Test_Read_1reg (uint32_t RegID, uint8_t* Buff);

//read lpf
uint8_t 			Read_Record_Infor (void);
uint8_t 			onSuccessGetRecordInfo (truct_String* Str, uint16_t Pos);
uint8_t 			onSuccessGetChannelInfo (truct_String* Str, uint16_t Pos);
uint8_t 			onSuccessReadProfile (truct_String* Str, uint16_t Pos);
uint8_t 			getTagByChannelName(char* channelName); 
uint8_t 			Genius_Read_Loadprofile (void) ;
int64_t             packLoadProfileImportExportData(uint8_t* data, float scaleFactor, uint8_t Scale) ;
int32_t             packLoadProfileVoltageCurrentData(uint8_t* data, float scaleFactor, uint8_t Scale) ;
//billing
void            Genius_SetTime_GetBill (void);
uint8_t         Read_Step_Billing (uint8_t Step);
uint8_t         Genius_Read_Billing (void);

uint8_t         onSuccessReadBill (truct_String* Str, uint16_t Pos);
uint8_t         onSuccessGetChannelBill_Inf (truct_String* Str, uint16_t Pos);
uint8_t         onSuccessGetRecordBill_Inf (truct_String* Str, uint16_t Pos);


uint8_t         genius_isResetBillingTimeInRange(uint32_t timeStamp, ST_TIME_FORMAT *p_timeMeter);
uint8_t         genius_getTag(uint32_t regIDFind);
int8_t          Genius_Check_Row_Obis103 (struct_Obis_Genius* structObis, uint8_t ObisHex);

//event
uint8_t         Genius_Read_event (void);
void            Genius_GetData_Event (Meter_Comm_Struct* Get_Met, uint16_t PosStart, uint8_t length, uint8_t type, truct_String oBIS);
uint8_t         Genius_Extract_Data_Event (Meter_Comm_Struct* Get_Met);
//tuti
void            Genius_MTuTi_value(uint16_t Pos, uint8_t length, uint8_t type, truct_String oBIS);
void            Genius_ExtractMTuTi_data (void);
uint8_t         Genius_Read_TuTi (void);
uint8_t         Genius_Read_event_TSVH (void);
uint8_t         GENIUS_Send1Cmd_Test (void);


#endif /* 2_START_INC_READ_METER_H_ */




