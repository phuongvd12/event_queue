/*
 * Read_Meter.h
 *
 *  Created on: Dec 15, 2015
 *      Author: Administrator
 */

#include "usart.h"
#include "variable.h"
#include "myuart.h"
#include "sim900.h"

#include "GELEX_Init_Meter.h"
#include "Init_All_Meter.h"

#ifndef _GELEX_READ_METER_
#define _GELEX_READ_METER_ _GELEX_READ_METER_H_

//Fix scale cua dai luong tuc thoi
#define GELEX_SCALE_VOLTAGE   	       		0       //V
#define GELEX_SCALE_CURRENT   	       		0       //A

#define GELEX_SCALE_FREQ      	       		0
#define GELEX_SCALE_ANGLE_PHASE	       		0
#define GELEX_SCALE_POW_FACTOR	       		0

#define GELEX_SCALE_ACTIVE_POW	       		3       //wh.
#define GELEX_SCALE_REACTIVE_POW	       	3
#define GELEX_SCALE_APPRENT_POW	       		3

#define GELEX_SCALE_Q_INTAN   	       		3       //var
//Fix scale cua dai luong thanh ghi dien nang
#define GELEX_SCALE_TOTAL_ENERGY       	   	3  //kwh.  kvar
//Fix scale cua dai luong thanh ghi biêu giá
#define GELEX_SCALE_TARRIFF          	   	    3  //kwh.  kvar
//Fix scale cua dai luong thanh ghi công suat cuc dai
#define GELEX_SCALE_MAX_DEMAND       	   	    3  //kwh.  kvar
//Fix scale cua dai luong TuTi
#define GELEX_SCALE_TU_TI           	   	    0  //kwh.  kvar
#define GELEX_SCALE_TUM_TIM           	   	    10  //kwh.  kvar
//Fix scale cua dai luong loadprf
#define GELEX_SCALE_HE_SO_NHAN                0     


//========================= Define =============================
#define METER_RS485_ADDR 0x21
#define READ_DELAY 10

// ======================== Array ==============================
extern uint8_t	GELEX_MInfo_Table[60][11];
extern uint8_t 	GELEX_GetScale_Table[79][11];
extern uint8_t	GELEX_OBIS_Table[111][7];

//-----------------Message template---------------------
extern uint8_t 	GELEX_SNRM[9];
extern uint8_t 	GELEX_AARQ[74];
extern uint8_t	GELEX_Get_Period_Template[27];
extern uint8_t	GELEX_Get_OBIS_Template[27];
extern uint8_t	GELEX_Get_OBIS_Template_Special[27];
extern uint8_t	GELEX_Get_Block_Template[21];
extern uint8_t	GELEX_Get_DATA_Template[78];

extern uint8_t	GELEX_NextFrame[9];
extern uint8_t	GELEX_Read_END[9];

extern uint8_t	GELEX_MInfo_MTemp[27];

extern uint8_t	GELEX_MEvent_Mess_OBIS[6];
extern uint8_t	GELEX_MTamper_Mess_OBIS[6];
extern uint8_t	GELEX_MAlarm_Mess_OBIS[6];
extern uint8_t	GELEX_MEvent_StartTime[8];
extern uint8_t	GELEX_MEvent_StopTime[8];

extern uint8_t	GELEX_MBilling_Mess_OBIS[6];
extern uint8_t	GELEX_MBilling_Mess_OBIS_MDTime[6];
extern uint8_t	GELEX_MBilling_StartTime[8];
extern uint8_t	GELEX_MBilling_StopTime[8];
extern uint8_t	GELEX_MBilling_OBIS_List[128];

extern uint8_t 	GELEX_MLProfile_Mess_OBIS_Channel1[6];
extern uint8_t 	GELEX_MLProfile_Mess_OBIS_Channel2[6];
extern uint8_t 	GELEX_MLProfile_OBIS_List[128];

extern uint8_t	GELEX_MLProfile_StartTime[8];
extern uint8_t	GELEX_MLProfile_StopTime[8];

//-----------------Mess Code table
extern uint8_t	GELEX_MeterInfoMessIDTable[61];
extern uint8_t	GELEX_GetScale_Code_Table[80];
extern uint8_t	GELEX_MeterTuTiMessIDTable[5];
/*
 * 				FUNCTION
 */

void            GELEX_Init_Function (uint8_t type);
uint8_t         GELEX_Read_TSVH (uint32_t Type);
uint8_t         GELEX_Read_Bill (void);
uint8_t         GELEX_Read_Event (uint32_t ForMin);
uint8_t         GELEX_Read_Lpf (void);
uint8_t         GELEX_Read_Infor (void);
//-------------------------UART function--------------------
uint8_t         GELEX_Read (Meter_Comm_Struct *Meter_Comm, uint8_t *First_Mess_Addr, uint8_t Mess_Type,void (*FuncExtractRawData)(uint8_t MType), uint8_t (*DataHandle)(void));
uint8_t         GELEX_Read_Info_Scale (Meter_Comm_Struct *Meter_Comm,uint8_t (*FuncExtractRawData)(void),void (*FuncDataHandle)(void),void (*FuncFillMess)(void),uint8_t *CodeTable);

void            GELEX_Fill_Meter_Addr (void);
void            GELEX_Mess_Checksum(void);
void            GELEX_Cal_Header_CheckSum (uint8_t *MessTemp,uint8_t Start_Pos,uint8_t End_Pos);
void            GELEX_Cal_Full_CheckSum (uint8_t *MessTemp,uint8_t Start_Pos,uint8_t End_Pos);
void            GELEX_Insert_GetOBIS_Mess_OBIS (uint8_t *Mess_OBIS);
void            GELEX_Insert_GetOBIS_Mess_OBIS_Special (uint8_t *Mess_OBIS);
void            GELEX_Insert_GetDATA_Mess_OBIS (uint8_t *Mess_OBIS);
void            GELEX_Insert_GetBLOCK_Mess_OBIS (uint8_t Block_no);
uint8_t         GELEX_CheckOBISInTable(uint16_t buff_start_pos, uint8_t table_end_pos);

uint8_t         GELEX_Get_Meter_ID_Scale (void);
uint8_t         GELEX_Get_Meter_ID (uint32_t Temp);
uint8_t         GELEX_MI_ExtractDataFunc(void);
void            GELEX_MI_SendData(void);
void            GELEX_MI_SendData_Inst(void);
void            GELEX_Fill_MInfo_Mess (void);

uint8_t         GELEX_MS_ExtractDataFunc(void);
void            GELEX_MS_SendData(void);
void            GELEX_Fill_Scale_Mess (void);

void            GELEX_MEventGetRawData (uint8_t Mess_Type);
uint8_t         GELEX_MEventExtractData (void);
uint8_t         MTamperExtractData (void);
uint8_t         MAlarmExtractData (void);
uint8_t         MEventOBISFilter (uint8_t Received_OBIS);
uint8_t         MEventSendData (void);

void            GELEX_MBillingGetRawData(uint8_t Mess_Type);
void            GELEX_MBillingGetRawData_Dummy(uint8_t Mess_Type);
uint8_t         GELEX_MBillingExtractOBISList(void);
uint8_t         GELEX_MBillingExtractData(void);
uint8_t         GELEX_MBillingExtractData_Dummy(void);
void            GELEX_MBillingInsertReadTime (void);

void            GELEX_MLProfileGetPeriod(void);
void            GELEX_MLProfileGetRawData(uint8_t Mess_Type);
uint8_t         GELEX_MLProfileExtractOBISList(void);
uint8_t         GELEX_MLProfileExtractData(void);
uint8_t         GELEX_CheckResetReadMeter(uint32_t Timeout);
void            GELEX_Prepare_Read_Date(uint8_t *Start, uint8_t *Stop);

//2020
void            GELEX_TuTi_SendData(void);
uint8_t         GELEX_TuTi_ExtractDataFunc (void);
uint8_t         GELEX_Connect_meter_Gelex(void);
void            GELEX_Fill_MTuTi_Mess (void);
void            Pack_MD_Gelex (Struct_Maxdemand_Value* Struct_MD, uint8_t FirstRate);
void            GELEX_Insert_GetPeriod_Mess_OBIS (uint8_t *Mess_OBIS);
uint8_t         GELEX_READ_1REG (uint8_t *Mess_Addr,void (*FuncExtractRawData)(void));
uint8_t         GELEX_Pack_event_103Pushdata (void);
uint8_t         GELEX_Check_Meter(void);
int8_t          GELEX_Check_Row_Obis103 (struct_Obis_Scale* structObis, uint8_t ObisHex);
uint8_t         GELEX_PackEvent_TSVH(void);
uint8_t         GELEX_Send1Cmd_Test (void);
uint8_t         GELEX_ConvertScaleMeter_toDec (uint8_t ScaleMeter, uint8_t* Div);

#endif /* 2_START_INC_READ_METER_H_ */




