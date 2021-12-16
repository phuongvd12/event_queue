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

#include "Meter103_Init_Meter.h"
#include "Init_All_Meter.h"

#ifndef _METER_103_READ_METER_H_
#define _METER_103_READ_METER_H_


#define TIME_NO_RESPOND					3000
#define TIME_WAIT_SEM					8000             //Con gelex103 timeout < 3s. Con Huu hong timeout khoang 5.5s.
#define TIM_ACTIVE485                   500              //thoi gian giua cac lenh la (20ms) 200ms < Tr <=1500
#define TIM_ACTIVE485_4K8               20               //o toc do 4800

#define MAX_LENGTH_BYTE_RECEI_METER     40               //Data cua Maxdemand dai (0.000*kW)(0000000000)
 
#define TSVH_POS_REGIS_INTAN             1
#define TSVH_POS_MAXDEMAND              40
#define TSVH_POS_EVEN_2OBIS             48
#define TSVH_POS_EVEN_END               54  
   
#define BILL_POS_MAXDEMAND              17
#define BILL_POS_EVEN_END               25

#define EVEN_POS_2OBIS_2DATA            12
   
//Khai bao struct  
typedef enum
{
	CMD_HANDSHAKE_103,
	CMD_REQ_BAUD_103,
	CMD_PASS_READ_103,
    CMD_PASS_WRITE_103,
    CMD_ACK_103,
	CMD_READ_REGIS_103,
    CMD_READ_RECORD1_103,
	CMD_WRITE_REGIS_103,
	CMD_LOGOUT_103,
}Struct_Cmd_Meter103;
 

typedef enum
{
	_HANDSHAKE,
    _GET_MANUF,
	_REQUETBAUD,
    _CHECK_ACK,
	_SENDPASS_READ,
    _SENDPASS_WRITE,
	_CHECK_ACK_2,
	_STEP_END,
} Step_HandShake_Meter103;


typedef struct
{
    uint8_t     Count_Error;  
    uint8_t     BaudRec;            //baud lay ra tu ban tin bat tay GLX4\4
    uint8_t     CharSpecMan;
    uint8_t     Mettype103;
    uint8_t     Step_HandShake;
    uint8_t     SttLastBill;
    uint16_t    Delay485;
}Struct_Var_Meter103;


extern  Struct_Var_Meter103     sMet103Var;



//-------------------------Function--------------------
//-------------------------Task--------------------
void        METER103_Init_Function (uint8_t type);
uint8_t     METER103_Read_TSVH (uint32_t Type);
uint8_t     METER103_Read_Bill (void);
uint8_t     METER103_Read_Event (uint32_t ForMin);
uint8_t     METER103_Read_Lpf (void);
uint8_t     METER103_Read_Infor (void);
uint8_t     METER103_Send1Cmd_Test (void);
//-------------------------UART function--------------------
uint8_t     METER103_GetUART2Data(void);
uint8_t     METER103_Connect_Meter(void);

//-------------------------Meter function--------------------
uint8_t     METER103_Get_Meter_ID (uint32_t Tempvalue);
uint8_t     METER103_CheckResetReadMeter(uint32_t Timeout);
uint8_t     METER103_Check_Meter(void);


//Function_ Request Data Meter
void        METER103_IEC62056_21_Command (uint8_t Kind_Cmd, uint8_t* BuffSend, uint16_t lengthSend);
uint8_t     METER103_Handshake (uint8_t ModeRW);
uint8_t     METER103_CheckObisMeter (truct_String* Buff, uint16_t Pos, uint8_t* Obis, uint8_t LenObis);
void        METER103_Init_UART_GET_DATA (uint8_t type, uint16_t Fixbaud);

//Function doc TuTi
uint8_t     METER103_TuTi_ExtractDataFunc (void);
uint8_t     METER_CheckBBC_OBIS_Recei (truct_String ObisCheck);
void        METER103_Decode_TuTi (uint16_t PosStart); 
void        METER103_TuTi_SendData(void);
void        METER103_Send_TuTi_Mess (void);
//Function doc TSVH và INTAN
uint8_t     METER103_Minfor_TypeDecode (uint8_t row);
uint8_t     METER103_DecodeData_1Obis1Data (uint8_t type, uint16_t PosStart,truct_String* aData);
void        METER103_PacketData_TSVH (uint8_t type, truct_String Obis, truct_String strData);
void        METER103_DecodeData_2Obis2Data (truct_String Obis1, truct_String Obis2, uint16_t PosStart1, Meter_Comm_Struct* sGet_Meter_Data);
void        METER103_DecodeData_Event_TSVH (uint8_t Row, uint16_t PosStart1, Meter_Comm_Struct* sGet_Meter_Data);
uint8_t     METER103_Read (Meter_Comm_Struct *Meter_Comm, uint8_t (*FuncExtractRawData)(void),void (*FuncDataHandle)(void),void (*FuncSendMess)(void),uint8_t *CodeTable);

uint8_t     METER103_MI_ExtractDataFunc (void);
uint8_t     METER103_PacketData_Bill (uint8_t type, truct_String Obis, truct_String strData);
void        METER103_MI_SendData(void);
void        METER103_Send_MInfo_Mess (void);
//Function doc BILL
uint8_t     METER103_MBill_ExtractDataFunc (void);
void        METER103_MBill_SendData(void);
void        METER103_Send_MBill_Mess (void);
uint8_t     METER103_ConvertStrtime_toStime (uint8_t* Buff, uint8_t length, ST_TIME_FORMAT* Stime);
void        METER103_MBillInsertTimeReq (void);
//Function doc EVEN
uint8_t     METER103_MEven_ExtractDataFunc (void);
void        METER103_MEven_SendData(void);
void        METER103_Send_MEven_Mess (void);
uint8_t     METER103_MEven_TypeDecode (uint8_t row);
void        METER103_PacketData_EVEN (uint8_t type, truct_String Obis, truct_String strData);
//Function doc LPF
uint8_t     METER103_Read_Record (Meter_Comm_Struct *Meter_Comm, uint8_t (*FuncExtractRawData)(void),void (*FuncDataHandle)(void),void (*FuncSendMess)(void),uint8_t *CodeTable);
void        METER103_Send_MLpf_Mess (void);
void        METER103_MLpf_SendData(void);
uint8_t     METER103_MLpf_ExtractDataFunc (void);

void        METER103_GET_TIME_ReadLPF (void);
uint8_t     METER103_Decode_LPF(uint16_t PosStart1);

//Function write vao cong to
uint8_t     METER103_SetStimeToMeter (ST_TIME_FORMAT sRTC);
uint8_t     METER103_WriteToMeter (truct_String *Obis, uint8_t *Data, uint8_t Length);  

#endif /* 2_START_INC_READ_METER_H_ */



