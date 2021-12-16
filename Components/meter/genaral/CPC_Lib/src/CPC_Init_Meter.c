/*
 * Init_Meter.c
 *
 *  Created on: Dec 15, 2015
 *      Author: Administrator
 */


#include "CPC_Init_Meter.h"
#include "CPC_Read_Meter.h"


#include "variable.h"
#include "sim900.h"
#include "t_mqtt.h"
#include "pushdata.h"



uint8_t	CPC_KEY_ENCRYPT[16];

uint8_t	CPC_Handshake1[5] = {0x2F,0x3F,0x21,0x0D,0x0A};  
uint8_t	CPC_Handshake2[6] = {0x06,0x30,0x35,0x31,0x0D,0x0A};

uint8_t password_1[10] = {'(', 'M','_','K','H','_','D','O','C', ')'};				// M_KH_DOC
uint8_t password_2[10] = {'(', 'M','K','_','M','U','C','_','2', ')'};				// MK_MUC_2
uint8_t password_3[10] = {'(', 'M','K','_','M','U','C','_','3', ')'};				// FEDC0003

//
uint8_t CPC_MeterTuTi_IDTable[2] ={2, 0xFF};
uint8_t CPC_MeterInfoMessIDTable[CPC_MAX_OBIS_TSVH] ={3, 4, 5, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 40, 41, 0, 0xFF};
                                                   // 0  1  2  3  4  5  6   7   8   9   10  11  12  13  14  15  16  17  18  19  20  21  22
uint8_t CPC_MeterBillMessIDTable[CPC_MAX_OBIS_BILL] ={0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xFF};
uint8_t CPC_MeterEvenMessIDTable[CPC_MAX_OBIS_EVENT] ={21, 22, 23, 24, 25, 26, 30, 31, 32, 33, 34, 35, 36, 40, 41, 43, 45, 48, 0xFF};
                                                  //    0  1  2  3  4  5  6  7  8  9  10   11  12  13  14  15  16  17  18  19  20  21  22  23   24
uint8_t CPC_MeterLpfMessIDTable[CPC_MAX_OBIS_LPF] ={0, 1, 2, 0, 0xFF};


//Obis cua TSVH va Event
uint8_t    CPC_TSVH_ObisCode[CPC_MAX_OBIS_TSVH][11] =     
{
        //Type Data                             //Data set
    {'1', '(', '0', '0', '0', '0', '0', '0', '0', '0', ')'},   //11 byte: 1(00000000) stime                 //0
    {'2', '(', '0', '0', '0', '0', '0', '0', '0', '0', ')'},   //11 byte: 2(00000000) out station           //1
    {'3', '(', '0', '0', '0', '0', '0', '0', '0', '0', ')'},   //11 byte: 3(00000000) Tuti                  //2
    //4 goi inta (3pha có 3 goi: 0, 1, 2;   1 pha co 2 goi: 3, 2)
    {'<', '(', '0', '0', '0', '0', '0', '0', '0', '0', ')'},   //11 byte: <(00000000) Set_Intan             //3
    {'<', '(', '0', '1', '0', '0', '0', '0', '0', '0', ')'},   //11 byte: <(01000000) Set_Intan             //4
    {'<', '(', '0', '2', '0', '0', '0', '0', '0', '0', ')'},   //11 byte: <(02000000) Set_Intan             //5
    {'<', '(', '0', '3', '0', '0', '0', '0', '0', '0', ')'},   //11 byte: <(03000000) Set_Intan             //6
    //2 goi Energy
    {'6', '(', '0', '0', '0', '1', '0', '0', '0', '0', ')'},   //11 byte: 6(00010000) ReadEnergy_Intan      //7
    //4 goi Tariff moi goi 8 thanh ghi tuong ung 8 tariff
    {'G', '(', '0', '0', '0', '1', '0', '0', '0', '0', ')'},   //11 byte: G(00010000) ReadTariff            //8
    {'G', '(', '0', '1', '0', '1', '0', '0', '0', '0', ')'},   //11 byte: G(01010000) ReadTariff            //9 
    {'G', '(', '0', '2', '0', '1', '0', '0', '0', '0', ')'},   //11 byte: G(02010000) ReadTariff            //10 
    {'G', '(', '0', '3', '0', '1', '0', '0', '0', '0', ')'},   //11 byte: G(03010000) ReadTariff            //11 
    //8 goi maxdemand
    {'P', '(', '0', '0', '0', '1', '0', '0', '0', '0', ')'},   //11 byte: P(00010000) ReadMD_INTAN          //12
    {'P', '(', '0', '1', '0', '1', '0', '0', '0', '0', ')'},   //11 byte: P(01010000) ReadMD_INTAN          //13
    {'P', '(', '0', '2', '0', '1', '0', '0', '0', '0', ')'},   //11 byte: P(02010000) ReadMD_INTAN          //14
    {'P', '(', '0', '3', '0', '1', '0', '0', '0', '0', ')'},   //11 byte: P(03010000) ReadMD_INTAN          //15
    {'P', '(', '0', '4', '0', '1', '0', '0', '0', '0', ')'},   //11 byte: P(04010000) ReadMD_INTAN          //16
    {'P', '(', '0', '5', '0', '1', '0', '0', '0', '0', ')'},   //11 byte: P(05010000) ReadMD_INTAN          //17
    {'P', '(', '0', '6', '0', '1', '0', '0', '0', '0', ')'},   //11 byte: P(06010000) ReadMD_INTAN          //18
    {'P', '(', '0', '7', '0', '1', '0', '0', '0', '0', ')'},   //11 byte: P(07010000) ReadMD_INTAN          //19
    
    //28 goi Event: TSVH: 21 22 23 40 41.
    {'U', '(', '0', '0', '0', '0', '0', '0', '0', '0', ')'},   //11 byte: U(0000000)  Event                 //20	
    {'U', '(', '0', '0', '0', '1', '0', '0', '0', '0', ')'},   //11 byte: U(0000000)  Event                 //21
    {'U', '(', '0', '0', '0', '2', '0', '0', '0', '0', ')'},   //11 byte: U(0000000)  Event                 //22	
    {'U', '(', '0', '0', '0', '3', '0', '0', '0', '0', ')'},   //11 byte: U(0000000)  Event                 //23	
    {'U', '(', '0', '0', '0', '4', '0', '0', '0', '0', ')'},   //11 byte: U(0000000)  Event                 //24	
    {'U', '(', '0', '0', '0', '5', '0', '0', '0', '0', ')'},   //11 byte: U(0000000)  Event                 //25	
    {'U', '(', '0', '0', '0', '6', '0', '0', '0', '0', ')'},   //11 byte: U(0000000)  Event                 //26	
    {'U', '(', '0', '0', '0', '7', '0', '0', '0', '0', ')'},   //11 byte: U(0000000)  Event                 //27	
    {'U', '(', '0', '0', '0', '8', '0', '0', '0', '0', ')'},   //11 byte: U(0000000)  Event                 //28	
    {'U', '(', '0', '0', '0', '9', '0', '0', '0', '0', ')'},   //11 byte: U(0000000)  Event                 //29	
    {'U', '(', '0', '0', '0', 'A', '0', '0', '0', '0', ')'},   //11 byte: U(0000000)  Event                 //30	
    {'U', '(', '0', '0', '0', 'B', '0', '0', '0', '0', ')'},   //11 byte: U(0000000)  Event                 //31	
    {'U', '(', '0', '0', '0', 'C', '0', '0', '0', '0', ')'},   //11 byte: U(0000000)  Event                 //32	
    {'U', '(', '0', '0', '0', 'D', '0', '0', '0', '0', ')'},   //11 byte: U(0000000)  Event                 //33	
    {'U', '(', '0', '0', '0', 'E', '0', '0', '0', '0', ')'},   //11 byte: U(0000000)  Event                 //34	
    {'U', '(', '0', '0', '0', 'F', '0', '0', '0', '0', ')'},   //11 byte: U(0000000)  Event                 //35	
    {'U', '(', '0', '0', '1', '0', '0', '0', '0', '0', ')'},   //11 byte: U(0000000)  Event                 //36	
    {'U', '(', '0', '0', '1', '1', '0', '0', '0', '0', ')'},   //11 byte: U(0000000)  Event                 //37	
    {'U', '(', '0', '0', '1', '2', '0', '0', '0', '0', ')'},   //11 byte: U(0000000)  Event                 //38	
    {'U', '(', '0', '0', '1', '3', '0', '0', '0', '0', ')'},   //11 byte: U(0000000)  Event                 //39	
    {'U', '(', '0', '0', '1', '4', '0', '0', '0', '0', ')'},   //11 byte: U(0000000)  Event                 //40	
    {'U', '(', '0', '0', '1', '5', '0', '0', '0', '0', ')'},   //11 byte: U(0000000)  Event                 //41	
    {'U', '(', '0', '0', '1', '6', '0', '0', '0', '0', ')'},   //11 byte: U(0000000)  Event                 //42	
    {'U', '(', '0', '0', '1', '7', '0', '0', '0', '0', ')'},   //11 byte: U(0000000)  Event                 //43	
    {'U', '(', '0', '0', '1', '8', '0', '0', '0', '0', ')'},   //11 byte: U(0000000)  Event                 //44	
    {'U', '(', '0', '0', '1', '9', '0', '0', '0', '0', ')'},   //11 byte: U(0000000)  Event                 //45	
    {'U', '(', '0', '0', '1', 'A', '0', '0', '0', '0', ')'},   //11 byte: U(0000000)  Event                 //46	
    {'U', '(', '0', '0', '1', 'B', '0', '0', '0', '0', ')'},   //11 byte: U(0000000)  Event                 //47	
    {'U', '(', '0', '0', '1', 'C', '0', '0', '0', '0', ')'},   //11 byte: U(0000000)  Event                 //48	
};

        
        
uint8_t    CPC_LASTBILL_ObisCode[CPC_MAX_OBIS_BILL][11] = 
{
    //10 goi lastbill byte thu 4 la Index.    
    {'T', '(', '0', '1', '0', '0', '0', '0', '0', '0', ')'},   //11 byte: T(01000000) LastBill              //0
    {'T', '(', '0', '1', '0', '0', '0', '1', '0', '0', ')'},   //11 byte: T(01000100) LastBill              //1
    {'T', '(', '0', '1', '0', '0', '0', '2', '0', '0', ')'},   //11 byte: T(01000200) LastBill              //2
    {'T', '(', '0', '1', '0', '0', '0', '3', '0', '0', ')'},   //11 byte: T(01000300) LastBill              //3
    {'T', '(', '0', '1', '0', '0', '0', '4', '0', '0', ')'},   //11 byte: T(01000400) LastBill              //4
    {'T', '(', '0', '1', '0', '0', '0', '5', '0', '0', ')'},   //11 byte: T(01000500) LastBill              //5
    {'T', '(', '0', '1', '0', '0', '0', '6', '0', '0', ')'},   //11 byte: T(01000600) LastBill              //6
    {'T', '(', '0', '1', '0', '0', '0', '7', '0', '0', ')'},   //11 byte: T(01000700) LastBill              //7
    {'T', '(', '0', '1', '0', '0', '0', '8', '0', '0', ')'},   //11 byte: T(01000800) LastBill              //8
    {'T', '(', '0', '1', '0', '0', '0', '9', '0', '0', ')'},   //11 byte: T(01000900) LastBill              //9
};


uint8_t    CPC_LPF_ObisCode[CPC_MAX_OBIS_LPF][11] = 
{
    //Lpf
    {0x5C,'(', '0', '0', '0', '0', '0', '0', '0', '0', ')'},  //11 byte: \(0000000) ReadConfigChart        //0    
    {']', '(', '0', '1', '0', '0', '0', '0', '0', '0', ')'},   //11 byte: ](0100000) Data_Set_TotalPack[11] //1	    
    {'^', '(', '0', '1', '0', '0', '0', '1', '0', '0', ')'},   //11 byte: ](0100100) Data_Set_lpf_1[11] 	//2
};


truct_String*  CPC_Obis103_TSVH[MAX_OBIS103_TSVH]=
{
    &Str_Ob_VolA,
    &Str_Ob_VolB,
    &Str_Ob_VolC,
    
    &Str_Ob_CurA,
    &Str_Ob_CurB,
    &Str_Ob_CurC,
    
    &Str_Ob_Freq,
    &Str_Ob_PoFacA,
    &Str_Ob_PoFacB,
    &Str_Ob_PoFacC,  //10
    
    &Str_Ob_AcPowA,
    &Str_Ob_AcPowB,
    &Str_Ob_AcPowC,
    &Str_Ob_AcPowTo,
    
    &Str_Ob_RePowA,
    &Str_Ob_RePowB,
    &Str_Ob_RePowC,
    &Str_Ob_RePowTo,   //18
    
    &Str_Ob_ApprA,
    &Str_Ob_ApprB,
    &Str_Ob_ApprC,
    &Str_Ob_ApprTo,    //22
    
    &Str_Ob_En_ImportWh,
    &Str_Ob_En_ExportWh,
    &Str_Ob_En_ImportVar,
    &Str_Ob_En_ExportVar,  //26
};


truct_String*  CPC_OBIS_BIEU[MAX_OBIS_BIEU]=
{
    //Pos Tariff TSVH
    &Str_Ob_AcPlus_Rate1,
    &Str_Ob_AcPlus_Rate2,
    &Str_Ob_AcPlus_Rate3,
    
    &Str_Ob_AcSub_Rate1,
    &Str_Ob_AcSub_Rate2,
    &Str_Ob_AcSub_Rate3,
    
    &Str_Ob_MaxDeRate1,
    &Str_Ob_MaxDeRate2,
    &Str_Ob_MaxDeRate3,
    
    &Str_Ob_MaxDeSub_Rate1,
    &Str_Ob_MaxDeSub_Rate2,
    &Str_Ob_MaxDeSub_Rate3,
    //Pos Tariff Bill 
    &Str_Ob_AcPlus_Rate1_chot,  //12
    &Str_Ob_AcPlus_Rate2_chot,
    &Str_Ob_AcPlus_Rate3_chot,
    
    &Str_Ob_AcSub_Rate1_chot,
    &Str_Ob_AcSub_Rate2_chot,
    &Str_Ob_AcSub_Rate3_chot,
    
    &Str_Ob_MaxDeRate1_Chot,
    &Str_Ob_MaxDeRate2_Chot,
    &Str_Ob_MaxDeRate3_Chot,
    
    &Str_Ob_MaxDe2Rate1_Chot,
    &Str_Ob_MaxDe2Rate2_Chot,
    &Str_Ob_MaxDe2Rate3_Chot,
};



Struct_Event_Obis        CPC_Str_OEvent103[29]=
{
    { {NULL},                        {NULL},                        {NULL},     {NULL},                      }, //Mat can bang pha
    
    { &Ev_PhaseAfail_Count,     &Ev_PhaseAfail_Startt,    {NULL},    &Ev_PhaseAfail_Stopt   },   //Mat ap pha a
    { &Ev_PhaseBfail_Count,     &Ev_PhaseBfail_Startt,    {NULL},    &Ev_PhaseBfail_Stopt   },   //Mat ap pha b
    { &Ev_PhaseCfail_Count,     &Ev_PhaseCfail_Startt,    {NULL},    &Ev_PhaseCfail_Stopt   },   //Mat ap pha C
    
    { &Ev_PhaseAreve_Count,     &Ev_PhaseAreve_Startt,    {NULL},    &Ev_PhaseAreve_Stopt   },   //Nguoc chieu cong suat pha A
    { &Ev_PhaseBreve_Count,     &Ev_PhaseBreve_Startt,    {NULL},    &Ev_PhaseBreve_Stopt   },   //Nguoc chieu cong suat pha B
    { &Ev_PhaseCreve_Count,     &Ev_PhaseCreve_Startt,    {NULL},    &Ev_PhaseCreve_Stopt   },   //Nguoc chieu cong suat pha C  //6
    
    { &Ev_PhaseALow_Count,      &Ev_PhaseALow_Startt,     {NULL},    &Ev_PhaseALow_Stopt   },   //Dien ap thap pha A
    { &Ev_PhaseBLow_Count,      &Ev_PhaseBLow_Startt,     {NULL},    &Ev_PhaseBLow_Stopt   },    //Dien ap thap pha B
    { &Ev_PhaseCLow_Count,      &Ev_PhaseCLow_Startt,     {NULL},    &Ev_PhaseCLow_Stopt   },   //Dien ap thap pha C   //9
    
    { &Ev_PhaseAOver_Count,     &Ev_PhaseAOver_Startt,    {NULL},    &Ev_PhaseAOver_Stopt   },   //Qua ap pha A
    { &Ev_PhaseBOver_Count,     &Ev_PhaseBOver_Startt,    {NULL},    &Ev_PhaseBOver_Stopt   },   //Qua ap pha B
    { &Ev_PhaseCOver_Count,     &Ev_PhaseCOver_Startt,    {NULL},    &Ev_PhaseCOver_Stopt   },   //Qua ap pha C
    
    { &Ev_PhaseAOverCur_Count,     &Ev_PhaseAOverCur_Startt,     {NULL},    &Ev_PhaseAOverCur_Stop   },    //qua dong pha A
    { &Ev_PhaseBOverCur_Count,     &Ev_PhaseBOverCur_Startt,     {NULL},    &Ev_PhaseBOverCur_Stop   },    
    { &Ev_PhaseCOverCur_Count,     &Ev_PhaseCOverCur_Startt,    {NULL},    &Ev_PhaseCOverCur_Stop   },      //15
    
    { &Ev_ReverseABC_Count,     &Ev_ReverseABC_Startt,    {NULL},    &Ev_ReverseABC_Stop   },   //Nguoc thu tu pha A-C-B
    
    { {NULL},                   {NULL},                         {NULL},                         {NULL}   },                      //---
    { {NULL},                   {NULL},                         {NULL},                         {NULL}   },                      //---
    { {NULL},                   {NULL},                         {NULL},                         {NULL}   },                      //---
    
    { &Ev_PowerDow_Count,       &Ev_PowerDow_Startt,            {NULL},                         {NULL}   },             //Mat dien    //20
    { &Ev_ProcessTime_Count,    &Ev_ProcessTime_Startt,         {NULL},                         {NULL}   },             //lap trinh thoi gian  //21
    { {NULL},                   {NULL},                         {NULL},                         {NULL}   },             //Tran thanh ghi dien nang
    { &Ev_ProcessPara_Count,    &Ev_ProcessPara_Startt,         {NULL},                         {NULL}   },             //Lap trinh thong so
    { {NULL},                   {NULL},                         {NULL},                         {NULL}   },             //Loi bo nho
    { &Ev_ClearData_Count,      &Ev_ClearData_Startt,           {NULL},                         {NULL}   },             //Xoa du lieu
    { {NULL},                   {NULL},                         {NULL},                         {NULL}   },             //Thay doi mat khau
    { {NULL},                   {NULL},                         {NULL},                         {NULL}   },             //Mat can bang dien ap
    { &Ev_LowRTC_Count,         &Ev_LowRTC_Startt,              {NULL},         &Ev_LowRTC_Stop   }, //Het Pin RTC
};






