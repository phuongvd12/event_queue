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

#include "Elster_Init_Meter.h"
#include "Init_All_Meter.h"

#ifndef _ELSTER_READ_METER_H_
#define _ELSTER_READ_METER_H_

//Fix scale cua dai luong tuc thoi
#define ELSTER_SCALE_VOLTAGE   	       		4       //V  
#define ELSTER_SCALE_CURRENT   	       		4       //A

#define ELSTER_SCALE_FREQ      	       		4
#define ELSTER_SCALE_ANGLE_PHASE	       	4
#define ELSTER_SCALE_POW_FACTOR	       		4

#define ELSTER_SCALE_ACTIVE_POW	       		4       //wh.
#define ELSTER_SCALE_REACTIVE_POW	       	4
#define ELSTER_SCALE_APPRENT_POW	       	4

#define ELSTER_SCALE_Q_INTAN   	       		4       //var
//Fix scale cua dai luong thanh ghi dien nang
#define ELSTER_SCALE_TOTAL_ENERGY       	   	6  //kwh.  kvar
//Fix scale cua dai luong thanh ghi biêu giá
#define ELSTER_SCALE_TARRIFF          	   	    6  //kwh.  kvar
//Fix scale cua dai luong thanh ghi công suat cuc dai
#define ELSTER_SCALE_MAX_DEMAND       	   	    6  //kwh.  kvar
//Fix scale cua dai luong TuTi
#define ELSTER_SCALE_TU_TI           	   	    0  //kwh.  kvar
//Fix scale cua dai luong loadprf
#define ELSTER_SCALE_HE_SO_NHAN                0     

#define ELSTER_SCALE_LPF                       6      //don vi theo nhu trong phan mem. don vi la kw

   
   
   
   

typedef struct 
{
    uint8_t             ObisHex;
    truct_String*       ObisString;
}Struct_ObisEvent;


extern Struct_ObisEvent  Elster_Ob_Event[];
/*
 * 			Variable
 */
extern uint8_t	 		meterPeriod_Mins;
extern uint32_t	 		meterTimeStamp;
extern ST_TIME_FORMAT 	meterLPTime,*ptrMeterLPTime;

/*
 * 				FUNCTION
 */

void           ELSTER_Init_Function (uint8_t type);
uint8_t        ELSTER_Read_TSVH (uint32_t Type);
uint8_t        ELSTER_Read_Bill (void);
uint8_t        ELSTER_Read_Event (uint32_t TemValue);
uint8_t        ELSTER_Read_Lpf (void);
uint8_t        ELSTER_Read_Infor (void);


//-------------------------UART function--------------------
uint8_t     ELSTER_GetUART2Data(void);
void        ELSTER_Mess_Checksum (void);
void        ELSTER_Decode_byte(uint8_t *mBuff,uint8_t startAddr,uint8_t Length);
void        ELSTER_Encode_byte(uint8_t *mBuff,uint8_t startAddr,uint8_t Length);

//-------------------------Meter function--------------------
uint8_t     ELSTER_Handshake_Handle (void);
void        Encryptpass(uint8_t *in, uint8_t *out);
uint8_t     ELSTER_Get_Meter_ID (uint32_t TempValue);
uint8_t     ELSTER_Read (Meter_Comm_Struct *Meter_Comm,void (*FuncExtractRawData)(void),void (*FuncDataHandle)(void),void (*FuncSendMess)(void),uint8_t *CodeTable);

void        ELSTER_MI_ExtractDataFunc (void);
void        ELSTER_Send_MInfo_Mess (void);

//2020
void        MI_DecodeData_NoSpin (uint8_t startPos, uint8_t dataLength, truct_String oBIS, truct_String unit, uint8_t ScaleNum);
void        MI_DecodeData_Spin (uint8_t startPos, uint8_t dataLength, truct_String oBIS, truct_String unit, uint8_t ScaleNum);

void        MI_DecodeData_DateTime (uint8_t startPos, uint8_t dataLength, uint8_t oBIS);
void        ELSTER_MI_SendData(void);
void        ELSTER_MI_SendData_Inst(void);

void        Send_MBilling_Mess (void);
void        MB_ExtractDataFunc (void);
void        MB_DecodeData_Spin (uint16_t startPos, uint8_t dataLength, truct_String oBIS, truct_String unit, uint8_t ScaleNum);
//void MB_DecodeBeginTime (uint16_t startPos, uint8_t dataLength, uint8_t oBIS);
void        MB_SendData(void);

void        Send_MEvent_Mess (void);
void        ELSTER_ME_ExtractDataFunc (void);
void        ME_DecodeData_Spin (uint16_t startPos, uint8_t dataLength, uint8_t oBIS);
void        ME_DecodeEventOder (uint8_t startA,uint8_t startB,uint8_t startC,uint8_t endA,uint8_t endB,uint8_t endC);
void        ME_DecodeMessType2 (uint8_t countA,uint8_t countB,uint8_t countC,uint8_t timeA,uint8_t timeB,uint8_t timeC,
									uint8_t startA,uint8_t startB,uint8_t startC,uint8_t endA,uint8_t endB,uint8_t endC);
void        ELSTER_ME_SendData (void);

void        Send_MLoadProfile_Mess (void);
void        RMLP_Extract_Data(void);
void        MLProf_SendData (void);
uint8_t     Cal_LPPeriod (uint8_t mPer);
void        Round_MLPTime(void);
//
uint8_t     ELSTER_CheckResetReadMeter(uint32_t Timeout);

//2020
uint8_t     ELSTER_Connect_Metter_Handle(void);
//function get tu ti
void        ELSTER_TuTi_ExtractDataFunc (void);
void        ELSTER_TuTi_SendData(void);
void        ELSTER_Send_MTuTi_Mess (void);
void        TuTi_DecodeData_Spin (uint8_t startPos, uint8_t dataLength, uint8_t ScaleNum);
void        MI_DecodeData_TimeBySecond (uint8_t startPos, uint8_t dataLength);
uint8_t     MB_DecodeData_TimeBySecond (uint16_t startPos, uint8_t dataLength);

uint8_t     Check_sTime_MD (uint8_t* Buff, uint16_t pos, uint8_t length);
void        Pack_String_Dec (truct_String* Str_taget, truct_String* str_source, uint8_t length_Source, uint8_t scale);
int8_t      Get_Row_InlistObis (uint8_t Obis_Hex);

uint8_t     Convert_Char_Elster (uint8_t Num);
uint8_t     ELSTER_Check_Meter(void);
void        ELSTER_ME_SendData_TSVH (void);
uint8_t     ELSTER_Send1Cmd_Test (void);


#endif /* 2_START_INC_READ_METER_H_ */




