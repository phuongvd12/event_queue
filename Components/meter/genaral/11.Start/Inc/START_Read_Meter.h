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

#include "START_Init_Meter.h"
#include "Init_All_Meter.h"

#ifndef _START_READ_METER_H_
#define _START_READ_METER_H_


//Fix scale cua dai luong tuc thoi
#define START_SCALE_VOLTAGE   	       		2       //V
#define START_SCALE_CURRENT   	       		2       //A

#define START_SCALE_FREQ      	       		2
#define START_SCALE_ANGLE_PHASE	       		2
#define START_SCALE_POW_FACTOR	       		3

#define START_SCALE_ACTIVE_POW	       		4       //wh.
#define START_SCALE_REACTIVE_POW	       	4
#define START_SCALE_APPRENT_POW	       		4

#define START_SCALE_Q_INTAN   	       		2       //var
//Fix scale cua dai luong thanh ghi dien nang
#define START_SCALE_TOTAL_ENERGY       	   	2  //kwh.  kvar
//Fix scale cua dai luong thanh ghi biêu giá
#define START_SCALE_TARRIFF          	   	    2  //kwh.  kvarh
//Fix scale cua dai luong thanh ghi công suat cuc dai
#define START_SCALE_MAX_DEMAND       	   	    4  //kwh.  kvarh
//Fix scale cua dai luong TuTi
#define START_SCALE_TU_TI           	   	    0  //kwh.  kvar
#define START_SCALE_TUM_TIM           	   	    10  //kwh.  kvar
//Fix scale cua dai luong loadprf
#define START_SCALE_HE_SO_NHAN                  0     
#define START_SCALE_LPF                         2 

//-------------------------TASK--------------------

void        START_Init_Function (uint8_t type);
uint8_t     START_Read_TSVH (uint32_t Type);
uint8_t     START_Read_Bill (void);
uint8_t     START_Read_Event (uint32_t ForMin);
uint8_t     START_Read_Lpf (void);
uint8_t     START_Read_Infor (void);

//-------------------------UART function--------------------
uint8_t     START_GetUART2Data(void);
void        UART2_Mess_Checksum (void);
uint8_t     START_Connect_Meter(void);

//-------------------------Meter function--------------------
uint8_t     START_Get_Meter_ID (uint32_t Tempvalue);
uint8_t     Meter_Comm_Func (Meter_Comm_Struct *Meter_Comm, uint8_t mess_type, uint8_t *ptr_code_table, void (*ExtractDataFunc)(void), void (*DataHandle)(void));
void        START_Fill_Mess (uint8_t message_code, uint8_t message_type);

void        RMI_Extract_Data (void);
void        RMI_Push_Mess (void);
void        RMI_Push_Mess_Instant (void);

void        RMB_Extract_Data (void);
void        RMB_Push_Mess (void);

void        RME_Extract_Data (void);
void        RME_Push_Mess (void);

void        RM_Load_Prof_Func (void);
uint8_t     UInt8_To_Hex (uint8_t mNum);
void        RMLP_Extract_First_Adress(void);
void        START_RMLP_Extract_Data (void);
void        START_RMLP_Push_Mess (void);
//
uint8_t     START_CheckResetReadMeter(uint32_t Timeout);
uint8_t     START_Check_Meter(void);
void        Extract_One_Element_Meter_Info (uint8_t length, uint8_t buff_pos, truct_String *oBIS, truct_String *unit, uint8_t ScaleNum );

void        Extract_TuTi_Meter_Info (Meter_Comm_Struct* GetMet, uint8_t type, uint8_t length, uint8_t buff_pos, truct_String *oBIS, truct_String *unit, uint8_t ScaleNum );
void        START_Extract_MAXDEMAND_Time (uint8_t length, uint8_t buff_pos, Struct_Maxdemand_Value *Struct_MaxD);
void        START_Extract_MAXDEMAND_Value (uint8_t length, uint8_t buff_pos, Struct_Maxdemand_Value *Struct_MaxD);
void        RMTuTi_Extract_Data (void);
void        RMI_Push_Mess_TuTi (void);
uint8_t     START_Extract_Bill_Time (uint8_t length, uint8_t buff_pos, Meter_Comm_Struct* GetMet);

void        Pack_MD_Value_Time (uint8_t Type, Meter_Comm_Struct* Get_Meter, Struct_Maxdemand_Value* Struct_MD, uint8_t FirstobisHex, struct_Obis_START* StructObis,  uint8_t Max_Obis);
int8_t      Check_Row_Obis103 (struct_Obis_START* structObis, uint8_t ObisHex, uint8_t MaxObis);

void        RME_Push_Mess_2 (void);
void        RME_Push_Mess_TSVH (void);
void        ReadEvent_TSVH(void);
void        Cacul_ID_Table(uint8_t* buff, uint8_t LastBill);
uint8_t     START_Send1Cmd_Test (void);


#endif /* 2_START_INC_READ_METER_H_ */



