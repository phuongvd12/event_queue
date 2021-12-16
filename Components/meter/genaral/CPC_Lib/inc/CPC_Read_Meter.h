/*
 * Read_Meter.h
 *
 *  Created on: Dec 15, 2015
 *      Author: Administrator
 */

/*----------------Fix don vi cho cac dai luong CPC -----------------*/
/*
Don vi cho Các thanh ghi ddien nang tong:
- Ðai luong imp watt: Wh
- Ðai luong Exp watt: 
- Ðai luong imp Var: 
- Ðai luong Exp Var: 

- Ðai luong imp Q1: 
- Ðai luong Exp Q2: 
- Ðai luong imp Q3: 
- Ðai luong Exp Q4: 

- Ðai luong imp Apprent: 





* dai luong thanh ghi tong, chot, bieu gia , maxdemand: Khong dau
* dai luong tuc thoi (ngoai tru goc lech pha), lpf: có dau


*/  
   
#include "CPC_Init_Meter.h"
#include "variable.h"
#include "sim900.h"

#include "Init_All_Meter.h"


#ifndef _CPC_READ_METER_H_
#define _CPC_READ_METER_H_



#define TIME_DELAY_ERR					3000
//Fix scale cua dai luong tuc thoi
#define CPC_SCALE_VOLTAGE   	       		2       //V
#define CPC_SCALE_CURRENT   	       		3       //A

#define CPC_SCALE_FREQ      	       		2
#define CPC_SCALE_ANGLE_PHASE	       		3
#define CPC_SCALE_POW_FACTOR	       		4

#define CPC_SCALE_ACTIVE_POW	       		2       //wh.
#define CPC_SCALE_REACTIVE_POW	       		2
#define CPC_SCALE_APPRENT_POW	       		2

#define CPC_SCALE_Q_INTAN   	       		5       //var
//Fix scale cua dai luong thanh ghi dien nang
#define CPC_SCALE_TOTAL_ENERGY       	   	4  //kwh.  kvar
//Fix scale cua dai luong thanh ghi biêu giá
#define CPC_SCALE_TARRIFF          	   	    4  //kwh.  kvar
//Fix scale cua dai luong thanh ghi công suat cuc dai
#define CPC_SCALE_MAX_DEMAND       	   	    5  //kwh.  kvar
//Fix scale cua dai luong TuTi
#define CPC_SCALE_TU_TI           	   	    0  //kwh.  kvar
//Fix scale cua dai luong loadprf
#define CPC_SCALE_HE_SO_NHAN                0     

#define CPC_SCALE_LPF                       2      //so o lpf la float. don vi theo nhu trong phan mem. Don vi nho nhat

/*
 * 			Variable
 */
   
   
typedef enum
{
	CMD_HANDSHAKE,
	CMD_ACK,
	CMD_SEND_PASS,
	CMD_READ,
	CMD_WRITE,
	CMD_LOGOUT,
	
}struct_Step_Cmd;
 

typedef enum
{
	SEND_FIRST_CHAR,
	CHECK_ACK_1,
	
	SEND_BAURATE,
	RECEI_ENCRYPT_KEY,
	
	SEND_PASSWORD,
	CHECK_ACK_2,
	STEP_END,
} Step_HandShake;



typedef struct
{
	uint8_t		 					Status;
    uint8_t		 					Status_Before;
	uint32_t 						UTC_Time;
	uint16_t						Period;
	uint32_t 						Cofig;
	uint8_t							Crc;
    ST_TIME_FORMAT		            sTimeRTC;
}StructHeadData;

typedef struct
{
	uint8_t 				Source;
	uint32_t 				Value;    //data co dau
}Struct_ValueData;


typedef struct
{
	uint8_t		 					Error;
	Struct_ValueData				Data[18];   //4*n        .Tôi da 18 dai luong
	uint8_t							Crc;
	uint32_t						Stime;
    ST_TIME_FORMAT		            sTimeRTC;
}StructRecordValue;



typedef struct
{
	StructHeadData 					Header;
	StructRecordValue				Record;
}StrucDataLoadpf;


typedef struct
{
	uint32_t 						Config_Source;
	uint16_t						Period;
	uint16_t 						TotalPack;
	uint8_t 						NumChannel;
	StrucDataLoadpf					DataLpf;
    uint16_t 						IndexPack;
    uint32_t						StimeStart;
    uint16_t 						IndexDay;
}StructLoadpf;



typedef struct
{
    uint32_t                        MeterID;
	uint8_t 						Step_HandShake;
    uint8_t                         Count_Error;
	
	ST_TIME_FORMAT					STimeIntan;
    uint32_t                        sTime_s;
	StructLoadpf					Loadpf;
    uint16_t                        Pos_Eventlpf;
    //
    uint8_t                         SttLastBill;
    ST_TIME_FORMAT		            sTimeLastBill;   
}Meter_Var_Struct;


extern Meter_Var_Struct				Met_Var;



//-----------------------Function---------------------
void                CPC_Init_Function (uint8_t type);
uint8_t             CPC_Read_TSVH (uint32_t Type);
uint8_t             CPC_Read_Bill (void);
uint8_t             CPC_Read_Event (uint32_t TemValue);
uint8_t             CPC_Read_Lpf (void);
uint8_t             CPC_Read_Infor (void);

void 				CPC_IEC62056_21_Command (uint8_t Kind_Cmd, uint8_t* BuffSend, uint16_t lengthSend);
//function connect
uint8_t 			CPC_Get_Meter_ID (uint32_t TempValue);
uint8_t				CPC_Connect_Metter_Handle(void);
uint8_t 			CPC_Handshake_Handle (void);
uint8_t 			CPC_GetUART1Data(void);

uint8_t 			ConvertHexStringtoHex (uint8_t HexString);	
uint8_t 			Convert2ByteHexStringto_1Hex (uint8_t* Buff);
uint8_t 			ConvertHextoAscii (uint8_t Hex);

//Function debug
uint16_t 			ConvertHextoHexstring (uint8_t* BuffHex, uint16_t length, uint8_t* BuffHexString);
void 				PrintInteger4byte(uint32_t value);
void 				PrintChar1byte(uint8_t value);
void 				Print8byte(uint64_t value);

//Dong goi theo Push data
uint8_t             CPC_CheckResetReadMeter(uint32_t Timeout);
uint8_t             CPC_Check_Meter(void);
uint8_t             CPC_Send1Cmd_Test (void);

//Viet lai ctrinh cho cpc
//Function chung 
void                CPC_Decode_Tariff (Meter_Comm_Struct* Get_Meter, uint8_t IndexPack);
void                CPC_Decode_MAXDemand_TSVH (Meter_Comm_Struct* Get_Meter, uint8_t IndexPack);
void                CPC_DecodeEnergyTotal(Meter_Comm_Struct* Get_Meter, uint8_t IndexPack);
void                CPC_Pack_Tariff_Demand_2(Meter_Comm_Struct* Get_Meter, uint8_t* Data_Tariff, uint8_t* Data_Maxdemand, uint8_t type);
void                CPC_Decode_Event(Meter_Comm_Struct* Get_Meter, uint8_t Pack);
uint8_t             CPC_Pack_Event (Meter_Comm_Struct* Get_Meter, uint16_t Pos);
//TuTi
uint8_t             CPC_Read (Meter_Comm_Struct *Meter_Comm, uint8_t (*FuncExtractRawData)(void),void (*FuncDataHandle)(void),void (*FuncSendMess)(void),uint8_t *CodeTable);
uint8_t             CPC_FunExtract_TuTi (void);   
void                CPC_TuTi_SendData(void);
void                CPC_Send_TuTi_Mess (void);
//TSVH
void                CPC_Decode_Intan (uint8_t type);
uint8_t             CPC_PackData_ToMinfor (void);
void                CPC_Decode_STime(uint8_t IndexPack);
uint8_t             CPC_MI_ExtractDataFunc (void);
void                CPC_MI_SendData(void);
void                CPC_Send_MInfo_Mess (void);
//Bill trong cpc
void                CPC_MBillInsertTimeReq (void);
uint8_t             CPC_MBill_ExtractDataFunc (void);
void                CPC_MBill_SendData(void);
void                CPC_Send_MBill_Mess (void);
uint8_t             CPC_Decode_StimeBill (void);
void                CPC_Pack_Lastbill_103 (void); 
//Event
void                CPC_Send_EVENT_Mess (void);
void                CPC_EVENT_SendData(void);
uint8_t             CPC_MEven_ExtractDataFunc (void);
void                CPC_PackEvent_103 (void);
//Lpf
void                CPC_Pack_1Mess_lpf_1(uint8_t Pack);
uint8_t             CPC_Lpf_ExtractDataFunc (void);
void                CPC_Lpf_SendData(void);
void                CPC_Send_Lpf_Mess (void);
void                CPC_Get_ConfigChart (uint8_t pack);
void                CPC_Decode_LpfConfig (uint32_t ValueConfig);
void                CPC_Get_ToTalPack_inday (uint8_t pack);
void                CPC_Pack_Header_lpf_Pushdata103 (truct_String* Payload, ST_TIME_FORMAT sTime, uint32_t Event);
void                Get_Event_Lpf(truct_String* Str, uint8_t header_status, uint8_t Error_record);
void                CPC_Decode_MAXDemand_LASTBILL (Meter_Comm_Struct* Get_Meter, uint8_t IndexPack);

#endif /* 2_START_INC_READ_METER_H_ */

















