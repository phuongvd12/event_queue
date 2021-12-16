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

#include "LANDIS_Init_Meter.h"
#include "Init_All_Meter.h"

#ifndef _LANDIS_READ_METER_
#define _LANDIS_READ_METER_ _LANDIS_READ_METER_H_

//========================= Define =============================
//Fix scale cua dai luong tuc thoi
#define LANDIS_SCALE_VOLTAGE   	       		2       //V
#define LANDIS_SCALE_CURRENT   	       		2       //A

#define LANDIS_SCALE_FREQ      	       		2
#define LANDIS_SCALE_ANGLE_PHASE	        2
#define LANDIS_SCALE_POW_FACTOR	       		2

#define LANDIS_SCALE_ACTIVE_POW	       		    3       //wh.
#define LANDIS_SCALE_REACTIVE_POW	       		3
#define LANDIS_SCALE_APPRENT_POW	       		3

#define LANDIS_SCALE_Q_INTAN   	       		    3       //kvar
//Fix scale cua dai luong thanh ghi dien nang
#define LANDIS_SCALE_TOTAL_ENERGY       	   	3  //kwh.  kvar
//Fix scale cua dai luong thanh ghi biêu giá
#define LANDIS_SCALE_TARRIFF          	   	    3  //kwh.  kvar
//Fix scale cua dai luong thanh ghi công suat cuc dai
#define LANDIS_SCALE_MAX_DEMAND       	   	    3  //kwh.  kvar
//Fix scale cua dai luong TuTi
#define LANDIS_SCALE_TU_TI           	   	    0  //kwh.  kvar
//Fix scale cua dai luong loadprf
#define LANDIS_SCALE_HE_SO_NHAN                 0     


// ======================== Array ==============================
extern uint8_t	LANDIS_OBIS_Table[56][7];
extern uint8_t 	LANDIS_MEvent_Table[6][7];
//-----------------Message template---------------------
extern uint8_t 	LANDIS_SNRM[34];
extern uint8_t 	LANDIS_AARQ[48];
extern uint8_t	LANDIS_Read_END[9];

extern uint8_t  LANDIS_GET_SHORTNAME[19];
extern uint8_t	LANDIS_NextFrame[9];
extern uint8_t	LANDIS_GetSN_Code_Table[300];
extern uint8_t	LANDIS_MeterInfoMessIDTable[56];
extern uint8_t  LANDIS_MeterTuTiMessIDTable[3];

extern uint8_t	LANDIS_Get_DATA_Template[70];

extern uint8_t	LANDIS_MInfo_MTemp[19];
extern uint8_t  LANDIS_Info_SN_Table[56][3];
extern uint8_t  LANDIS_Event_SN_Table[6][3];
extern uint8_t  LANDIS_Profile_Historical_SN_Table[2][3];

extern uint8_t	LANDIS_MBilling_Mess_OBIS[2];
extern uint8_t	LANDIS_MLprofile_OBIS_List[2];
extern uint8_t	LANDIS_MBilling_StartTime[8];
extern uint8_t	LANDIS_MBilling_StopTime[8];
extern uint8_t	LANDIS_MLProfile_StartTime[8];
extern uint8_t	LANDIS_MLProfile_StopTime[8];
extern uint8_t  LANDIS_OBIS_Order[27];
extern uint8_t	LANDIS_MBilling_OBIS_List[128];
extern uint8_t 	LANDIS_MLProfile_OBIS_List[128];

//-----------------Mess Code table
extern uint8_t	LANDIS_MEvent_Code_Table[7];
extern uint8_t	LANDIS_GetScale_Code_Table[57];
extern uint8_t	LANDIS_MHis_Code_Data_Table[20];
extern uint8_t	LANDIS_MHis_Code_Obis_Table[20];
extern uint8_t	LANDIS_MLprofile_Code_Obis_Table[20];
extern uint8_t	LANDIS_MLprofile_Code_Data_Table[200];  //40. co cong to test 1p 1 record. request 1 h nó dc 60 record

//-------------------------Meter function--------------------
void LANDIS_Init_Function (uint8_t type);
uint8_t LANDIS_Read_TSVH (uint32_t Type);
uint8_t LANDIS_Read_Bill (void);
uint8_t LANDIS_Read_Event (uint32_t TemValue);
uint8_t LANDIS_Read_Lpf (void);
uint8_t LANDIS_Read_Infor (void);

uint8_t     LANDIS_HANDSHAKE(void);
// Collecting OBIS
uint8_t     LANDIS_Read(Meter_Comm_Struct *Meter_Comm,void (*FuncExtractRawData)(void),void (*FuncDataHandle)(void),void (*FuncFillMess)(void),uint8_t *CodeTable);
void        LANDIS_Get_ShortName(void);
void        OBIS_SendData(void);
void        Fill_Nextframe(void);
void        Fill_frameOBIS_His(void);
void        Fill_frameData_His(void);

void        Fill_frameOBIS_LoPro(void);
void        LoPro_Send_Data(void);
void        Fill_frameData_Load(void);
// 
uint8_t     LANDIS_Read_His_Load (Meter_Comm_Struct *Meter_Comm, uint8_t *First_Mess_Addr, uint8_t Mess_Type,void (*FuncExtractRawData)(uint8_t MType), uint8_t (*DataHandle)(void));
uint8_t     LANDIS_Read_Info_Scale (Meter_Comm_Struct *Meter_Comm,void (*FuncExtractRawData)(void),void (*FuncDataHandle)(void),void (*FuncFillMess)(void),uint8_t *CodeTable);
uint8_t     LANDIS_Extract_Data(Meter_Comm_Struct *Meter_Comm,void (*FuncDataHandle)(void),uint8_t *Buff, uint8_t option, uint8_t *count, uint8_t No_Obis);

void        LANDIS_Mess_Checksum(void);
void        LANDIS_Cal_Full_CheckSum (uint8_t *MessTemp,uint8_t Start_Pos,uint8_t End_Pos);
void        LANDIS_Insert_GetOBIS_Mess_OBIS (uint8_t *Mess_OBIS);
void        LANDIS_Insert_GetDATA_Mess_OBIS (uint8_t *Mess_OBIS);


void        LANDIS_MI_ExtractDataFunc(void);
void        LANDIS_ME_ExtractDataFunc(void);
void        LANDIS_MI_SendData(void);
void        LANDIS_MI_SendData_Inst(void);
void        LANDIS_ME_SendData(void);

void        LANDIS_Fill_MInfo_Mess (void);
void        LANDIS_Fill_MEvent_Mess (void);

void        LANDIS_MScale_ExtractDataFunc(void);
void        LANDIS_MS_ExtractDataSN(void);
void        LANDIS_MS_SendData(void);
void        LANDIS_Fill_Scale_Mess (void);

uint8_t     Compare_Array(uint8_t *Array1, uint8_t *Array2, uint8_t length);

uint8_t     LANDIS_Get_Meter_ID(uint32_t ValueTemp);

void        LANDIS_MBillingGetRawData(uint8_t Mess_Type);  //chua dung
void        LANDIS_MBillingExtractOBISList(void);  //chua dung
void        LANDIS_MBillingExtractData(void);
void        LANDIS_His_Send_Data(void);

void        LANDIS_MLProfileExtractOBISList(void);
void        LANDIS_MLProfileExtractData(void);

uint8_t     LANDIS_CheckResetReadMeter(uint32_t Timeout);
uint8_t     LANDIS_Connect_meter_Landis(void) ;

void        Convert_Opera_ToPack103_TSVH(void);
void        Convert_His_To103Pack (void);  
void        Pack_MD_Landis (Struct_Maxdemand_Value* Struct_MD, uint8_t FirstobisHex);
void        Landis_TuTi_SendData(void); 
uint8_t     Check_sTimeBill_withStarttime (ST_TIME_FORMAT sTimeStart, ST_TIME_FORMAT* sTimeStop, truct_String* sTimeJustGet);
void        LANDIS_Fill_MTuTi_Mess (void);
void        LANDIS_MTuTi_ExtractDataFunc(void) ;
void        MLoadpfInsertReadTime(void);
uint8_t     Check_2_sTime_By_date (ST_TIME_FORMAT sTime1, ST_TIME_FORMAT sTime2, uint8_t* DifDate);
uint32_t    ConvertStime_toDate (ST_TIME_FORMAT sTime);
void        Convert_Date_toStime(uint32_t date, ST_TIME_FORMAT* sTime);
int8_t      Landis_Check_Obis_event (uint8_t ID_Event);

void        LANDIS_Prepare_Read_Date (uint8_t *Start, uint8_t *Stop);
uint8_t     LANDIS_CheckOBISInTable(uint16_t buff_start_pos, uint8_t table_end_pos);
void        LANDIS_MBillingInsertReadTime(void);
int8_t      LANDIS_Check_Row_Obis103 (struct_Obis_Scale* structObis, uint8_t ObisHex);
uint8_t     LANDIS_Check_Meter(void);
void        LANDIS_ME_SendData_TSVH(void);
uint8_t     LANDIS_Send1Cmd_Test (void);






#endif /* 2_START_INC_READ_METER_H_ */









