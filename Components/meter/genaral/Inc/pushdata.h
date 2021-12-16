
#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"
#include "stdint.h"
#include "stdbool.h"
#include "sim900.h"
#include "Init_All_Meter.h"

#ifndef __PUSHDATA_H
#define __PUSHDATA_H 


#define SOH				0x01
#define STX				0x02 
#define ETX				0x03
#define EOT				0x04
#define ACK				0x06

   
#define DAYS_IN_LEAP_YEAR                        ( ( uint32_t )  366U )
#define DAYS_IN_YEAR                             ( ( uint32_t )  365U )
#define SECONDS_IN_1DAY                          ( ( uint32_t )86400U )
#define SECONDS_IN_1HOUR                         ( ( uint32_t ) 3600U )
#define SECONDS_IN_1MINUTE                       ( ( uint32_t )   60U )
#define MINUTES_IN_1HOUR                         ( ( uint32_t )   60U )
#define HOURS_IN_1DAY                            ( ( uint32_t )   24U )

#define  DAYS_IN_MONTH_CORRECTION_NORM           ((uint32_t) 0x99AAA0 )
#define  DAYS_IN_MONTH_CORRECTION_LEAP           ((uint32_t) 0x445550 )

#define DIVC( X, N )                                ( ( ( X ) + ( N ) -1 ) / ( N ) )
   
   

extern truct_String        Str_MeterType_u8[11];


extern truct_String         Str_OB_EN_REGISTER; 
extern  truct_String         Str_OB_INTAN;
extern  truct_String         Str_OB_CHOT;
extern  truct_String         Str_OB_EVENT;
extern  truct_String         Str_OB_LPF ; 
extern  truct_String         Str_OB_RESPONSE_AT; 
extern  truct_String         Str_OB_INFOR_METER;

extern  truct_String         Str_OB_IDENT_METER; 
extern  truct_String         Str_Connect;
extern  truct_String         Str_DisConnect;   

extern  truct_String         Str_AlarmPowModerm;
extern  truct_String         Str_PowOn_Moderm;
extern  truct_String         Str_PowOFF_Moderm;

extern  truct_String         Str_AlarmPowLine;
extern  truct_String         Str_PowUp_Line;
extern  truct_String         Str_PowDown_Line;

/*-----------------Cac mã nhan dang dai luong Giá tri tuc thoi----------------------*/
extern  truct_String         Str_Ob_VolA;
extern  truct_String         Str_Ob_VolB;
extern  truct_String         Str_Ob_VolC;

extern  truct_String         Str_Ob_CurA;
extern  truct_String         Str_Ob_CurB;
extern  truct_String         Str_Ob_CurC;
extern  truct_String         Str_Ob_CurNeu;

extern  truct_String         Str_Ob_PhRotation;  
extern  truct_String         Str_Ob_PhAnglePhA;
extern  truct_String         Str_Ob_PhAnglePhB;
extern  truct_String         Str_Ob_PhAnglePhC;

extern  truct_String         Str_Ob_Freq;

extern  truct_String         Str_Ob_PoFac;     
extern  truct_String         Str_Ob_PoFacA;
extern  truct_String         Str_Ob_PoFacB;
extern  truct_String         Str_Ob_PoFacC;
	
extern  truct_String         Str_Ob_AcPowTo;
extern  truct_String         Str_Ob_AcPowA;
extern  truct_String         Str_Ob_AcPowB;
extern  truct_String         Str_Ob_AcPowC;

extern  truct_String         Str_Ob_RePowTo;
extern  truct_String         Str_Ob_RePowA;
extern  truct_String         Str_Ob_RePowB;
extern  truct_String         Str_Ob_RePowC;

extern  truct_String         Str_Ob_ApprTo;
extern  truct_String         Str_Ob_ApprA;
extern  truct_String         Str_Ob_ApprB;
extern  truct_String         Str_Ob_ApprC;

extern  truct_String         Str_Ob_Q1;
extern  truct_String         Str_Ob_Q2;
extern  truct_String         Str_Ob_Q3;
extern  truct_String         Str_Ob_Q4;
extern  truct_String         Str_Ob_Appr;

extern  truct_String         Str_Ob_Tu;
extern  truct_String         Str_Ob_Ti;
extern  truct_String         He_So_Nhan;
/*-----------------Cac mã nhan dang dai luong thanh ghi dien nang-----------------------*/
//apprent				2D	

extern  truct_String         Str_Ob_En_ImportWh;
extern  truct_String         Str_Ob_En_ExportWh;
extern  truct_String         Str_Ob_En_ImportVar;
extern  truct_String         Str_Ob_En_ExportVar;

extern  truct_String         Str_Ob_En_Q1;
extern  truct_String         Str_Ob_En_Q2;
extern  truct_String         Str_Ob_En_Q3;
extern  truct_String         Str_Ob_En_Q4;

extern  truct_String         Str_Ob_En_Appr;


/*-----------------Cac mã nhan dang dai luong bieu giá-----------------------*/
extern  truct_String         Str_Ob_AcPlus_Rate1;
extern  truct_String         Str_Ob_AcPlus_Rate2;
extern  truct_String         Str_Ob_AcPlus_Rate3;
extern  truct_String         Str_Ob_AcPlus_Rate4;

extern  truct_String         Str_Ob_AcSub_Rate1;
extern  truct_String         Str_Ob_AcSub_Rate2;
extern  truct_String         Str_Ob_AcSub_Rate3;
extern  truct_String         Str_Ob_AcSub_Rate4;

extern  truct_String         Str_Ob_RePlus_Rate1;
extern  truct_String         Str_Ob_RePlus_Rate2;
extern  truct_String         Str_Ob_RePlus_Rate3;
extern  truct_String         Str_Ob_RePlus_Rate4
;
extern  truct_String         Str_Ob_ReSub_Rate1;
extern  truct_String         Str_Ob_ReSub_Rate2;
extern  truct_String         Str_Ob_ReSub_Rate3;
extern  truct_String         Str_Ob_ReSub_Rate4;

extern  truct_String         Str_Ob_MaxDe;
extern  truct_String         Str_Ob_MaxDeRate1;
extern  truct_String         Str_Ob_MaxDeRate2;
extern  truct_String         Str_Ob_MaxDeRate3;
extern  truct_String         Str_Ob_MaxDeRate4;

extern  truct_String         Str_Ob_MaxDeSub;
extern  truct_String         Str_Ob_MaxDeSub_Rate1;
extern  truct_String         Str_Ob_MaxDeSub_Rate2;
extern  truct_String         Str_Ob_MaxDeSub_Rate3;
extern  truct_String         Str_Ob_MaxDeSub_Rate4;

/*-----------------Cac mã nhan dang dai luong lpf------------------------*/
extern  truct_String        Str_Ob_lpf[18];
extern  truct_String        Str_Ob_lpf_type2[4];
extern  truct_String        Unit_Lpf_type2[4];

/*-----------------Khai bao cac don vi co cho cac dai luong-----------------------*/
//dai luong tuc thoi
extern  truct_String         Unit_Voltage;
extern  truct_String         Unit_Current;
extern  truct_String         Unit_Freq;

extern  truct_String         Unit_Active_Intan;
extern  truct_String         Unit_Reactive_Intan;
extern  truct_String         Unit_Q_Intan ;
extern  truct_String         Unit_Apprent_Intan ;

extern  truct_String         Unit_Active_Intan_Kw;
extern  truct_String         Unit_Reactive_Intan_Kvar ;


//Ðai luong thanh ghi nang luong va bieu gia, cs cuc dai
extern  truct_String         Unit_Active_EnTotal;
extern  truct_String         Unit_Reactive_EnTotal;
extern  truct_String         Unit_Q_EnTotal;
extern  truct_String         Unit_Apprent_EnTotal;
extern  truct_String         Unit_MAXDEMAND;
extern  truct_String         Unit_Energy_Min;
extern  truct_String         Unit_En_Reactive_Min;
extern  truct_String         Unit_Lpf[18];

//Ðai luong chot tháng
extern  truct_String         Str_Ob_AcImTotal_Chot;
extern  truct_String         Str_Ob_AcExTotal_Chot;
extern  truct_String         Str_Ob_ReImTotal_Chot;
extern  truct_String         Str_Ob_ReExTotal_Chot;

extern  truct_String         Str_Ob_Q1_Chot;
extern  truct_String         Str_Ob_Q2_Chot;
extern  truct_String         Str_Ob_Q3_Chot;
extern  truct_String         Str_Ob_Q4_Chot;

extern  truct_String         Str_Ob_Appr_Chot;

//Thanh ghi bieu gia chot
extern  truct_String         Str_Ob_AcPlus_Rate1_chot;													
extern  truct_String         Str_Ob_AcPlus_Rate2_chot;
extern  truct_String         Str_Ob_AcPlus_Rate3_chot;
extern  truct_String         Str_Ob_AcPlus_Rate4_chot;
extern  truct_String         Str_Ob_AcSub_Rate1_chot;
extern  truct_String         Str_Ob_AcSub_Rate2_chot;
extern  truct_String         Str_Ob_AcSub_Rate3_chot;	
extern  truct_String         Str_Ob_AcSub_Rate4_chot;	

extern  truct_String         Str_Ob_RePlus_Rate1_chot ;
extern  truct_String         Str_Ob_RePlus_Rate2_chot;
extern  truct_String         Str_Ob_RePlus_Rate3_chot;
extern  truct_String         Str_Ob_RePlus_Rate4_chot;

extern  truct_String         Str_Ob_ReSub_Rate1_chot;
extern  truct_String         Str_Ob_ReSub_Rate2_chot;
extern  truct_String         Str_Ob_ReSub_Rate3_chot;
extern  truct_String         Str_Ob_ReSub_Rate4_chot;
      

/*-----------------Cac mã nhan dang dai luong công suât cuc dai-----------------------*/
extern  truct_String         Str_Ob_MaxDe_Chot;
extern  truct_String         Str_Ob_MaxDeRate1_Chot;
extern  truct_String         Str_Ob_MaxDeRate2_Chot;
extern  truct_String         Str_Ob_MaxDeRate3_Chot;
extern  truct_String         Str_Ob_MaxDeRate4_Chot;

extern  truct_String         Str_Ob_MaxDe2_Chot;
extern  truct_String         Str_Ob_MaxDe2Rate1_Chot;
extern  truct_String         Str_Ob_MaxDe2Rate2_Chot;
extern  truct_String         Str_Ob_MaxDe2Rate3_Chot;
extern  truct_String         Str_Ob_MaxDe2Rate4_Chot;

extern  truct_String         Str_PowDown_Line1;

/*-----------------Cac mã nhan dang thong tin Moderm-----------------------*/
extern  truct_String         ModermIP;
extern  truct_String         ModermFirmVer;
extern  truct_String         ModermCSQ;
extern  uint16_t days[4][12];


extern truct_String    Ev_ChangeTime_Count ;
extern truct_String    Ev_ChangeTime_Startt; 
    
extern truct_String    Ev_PhaseAfail_Count; 
extern truct_String    Ev_PhaseBfail_Count;  
extern truct_String    Ev_PhaseCfail_Count;    
    
extern truct_String    Ev_PhaseAfail_Startt; 
extern truct_String    Ev_PhaseAfail_Stopt ; 
extern truct_String    Ev_PhaseBfail_Startt; 
extern truct_String    Ev_PhaseBfail_Stopt;  
extern truct_String    Ev_PhaseCfail_Startt; 
extern truct_String    Ev_PhaseCfail_Stopt ;
    
extern truct_String    Ev_PhaseAreve_Count;  
extern truct_String    Ev_PhaseBreve_Count; 
extern truct_String    Ev_PhaseCreve_Count;  
    
extern truct_String    Ev_PhaseAreve_Startt;
extern truct_String    Ev_PhaseAreve_Stopt; 
extern truct_String    Ev_PhaseBreve_Startt;   
extern truct_String    Ev_PhaseBreve_Stopt;  
extern truct_String    Ev_PhaseCreve_Startt; 
extern truct_String    Ev_PhaseCreve_Stopt;
//
extern truct_String    Ev_PhaseALow_Count ;
extern truct_String    Ev_PhaseBLow_Count ;
extern truct_String    Ev_PhaseCLow_Count ;

extern truct_String    Ev_PhaseALow_Startt;
extern truct_String    Ev_PhaseALow_Stopt ;
extern truct_String    Ev_PhaseBLow_Startt ;  
extern truct_String    Ev_PhaseBLow_Stopt;
extern truct_String    Ev_PhaseCLow_Startt;
extern truct_String    Ev_PhaseCLow_Stopt;


extern truct_String    Ev_PhaseAOver_Count;
extern truct_String    Ev_PhaseBOver_Count;
extern truct_String    Ev_PhaseCOver_Count;

extern truct_String    Ev_PhaseAOver_Startt;
extern truct_String    Ev_PhaseAOver_Stopt;
extern truct_String    Ev_PhaseBOver_Startt;   
extern truct_String    Ev_PhaseBOver_Stopt;
extern truct_String    Ev_PhaseCOver_Startt;
extern truct_String    Ev_PhaseCOver_Stopt;
//
extern truct_String    Ev_PhaseAOverCur_Count ;
extern truct_String    Ev_PhaseBOverCur_Count ;
extern truct_String    Ev_PhaseCOverCur_Count;

extern truct_String    Ev_PhaseAOverCur_Startt ; 
extern truct_String    Ev_PhaseBOverCur_Startt ;
extern truct_String    Ev_PhaseCOverCur_Startt ;

extern truct_String    Ev_PhaseAOverCur_Stop ;
extern truct_String    Ev_PhaseBOverCur_Stop ;
extern truct_String    Ev_PhaseCOverCur_Stop;

extern truct_String    Ev_ReverseABC_Count;
extern truct_String    Ev_ReverseABC_Startt;
extern truct_String    Ev_ReverseABC_Stop ;


extern truct_String    Ev_PowerDow_Count;
extern truct_String    Ev_PowerDow_Startt ;
extern truct_String    Ev_PowerOn_Startt;

extern truct_String    Ev_ProcessTime_Count;
extern truct_String    Ev_ProcessTime_Startt ;

extern truct_String    Ev_ProcessPara_Count ;
extern truct_String    Ev_ProcessPara_Startt ;

extern truct_String    Ev_ClearData_Count ;
extern truct_String    Ev_ClearData_Startt;

extern truct_String    Ev_LowRTC_Count;
extern truct_String    Ev_LowRTC_Startt;
extern truct_String    Ev_LowRTC_Stop;

extern truct_String    Ev_MagField_Startt;
extern truct_String    Ev_RePowFlow_Startt;

extern truct_String    Ev_CloseCover_Startt;
extern truct_String    Ev_OpenCover_Startt;

extern truct_String    Ev_CloseTer_Startt;
extern truct_String    Ev_OpenTer_Startt;


/*-----------------Funcion-----------------------*/
float               Convert_FloatPoint_2Float (uint32_t Float_Point);
int32_t             Convert_float_2int (uint32_t Float_Point_IEEE, uint8_t scale);
int32_t             Convert_uint_2int (uint32_t Num);
int16_t             Convert_uint16_2int16 (uint16_t Num);
int64_t             Convert_uint64_2int64 (uint64_t Num);
uint8_t 			BBC_Cacul (uint8_t* Buff, uint16_t length);
uint8_t 			Check_BBC (truct_String* Str);
void                Pack_HEXData_Frame (truct_String* Payload, int64_t Data, uint8_t Scale);

void                Epoch_to_date_time(ST_TIME_FORMAT* date_time,uint32_t meterTS, uint8_t type);
uint32_t            HW_RTC_GetCalendarValue_Second( ST_TIME_FORMAT sTimeRTC, uint8_t type);

//Dong goi cac ban tin
uint8_t             Identi_Meter (truct_String* Payload, truct_String* sMeter_ID, uint8_t Meter_Type, ST_TIME_FORMAT sTime, truct_String Str_Connect);
void                Pack_AlarmPower (truct_String* Payload, truct_String* Str_ObCode, truct_String* sMeter_ID, uint8_t Meter_Type, ST_TIME_FORMAT sTime, truct_String Str_Connect);
void                Infor_Moderm (truct_String* Payload, truct_String* Str_ObCode,truct_String* sMeter_ID, uint8_t Meter_Type, ST_TIME_FORMAT sTime, truct_String* FirmVer, int8_t	RSSI_c8, uint16_t Hesonhan);
void                Pack_HEXData_Frame_Uint64 (truct_String* Payload, uint64_t Data, uint8_t Scale);

uint16_t            Find_Idex_By_sTime (ST_TIME_FORMAT sTimeFind, ST_TIME_FORMAT sTime_Real);
void                Write_Header_His_Push103 (void);
void                Write_Header_TSVH_Push103 (void);
void                Pack_Header_lpf_Pushdata103 (void);


uint8_t             Send_MessLpf_toQueue (void);
void                Add_TuTI_toPayload (Meter_Comm_Struct* Struct_Get);

void                Pack_PushData_103_Infor_Meter (void);
void                Push_Bill_toQueue (uint8_t Type);
void                Push_TSVH_toQueue (uint8_t Type);
uint8_t             Push_Even_toQueue (uint8_t Type);  

void                Header_event_103 (truct_String* Payload, uint8_t Type) ; 
void                Check_Meter(void);
void                _fSend_Empty_Lpf (void);
uint8_t             _fSendTSVH_ToQueue(uint8_t Type, uint8_t *aData, uint16_t length);
uint8_t             _fSendBill_ToQueue(uint8_t Type, uint8_t *aData, uint16_t length);

#endif /* __VARIABLE_H */










