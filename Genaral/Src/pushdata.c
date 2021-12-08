#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"
#include "pushdata.h"
#include "sim900.h"
#include "t_mqtt.h"

#include "variable.h"
#include "myuart.h"




uint16_t days[4][12] =
{
    {   0,  31,  60,  91, 121, 152, 182, 213, 244, 274, 305, 335},
    { 366, 397, 425, 456, 486, 517, 547, 578, 609, 639, 670, 700},
    { 731, 762, 790, 821, 851, 882, 912, 943, 974,1004,1035,1065},
    {1096,1127,1155,1186,1216,1247,1277,1308,1339,1369,1400,1430},
};


truct_String        Str_MeterType_u8[11]=
{
    {(uint8_t*)"GELEX", 5},
    {NULL},
    {(uint8_t*)"GELEX", 5},
    {(uint8_t*)"HUUHONG", 7},
    {(uint8_t*)"VINASINO", 8},
    {(uint8_t*)"LANDIS", 6},
    {(uint8_t*)"ELSTER", 6},
    {(uint8_t*)"SHENZHEN", 8},
    {(uint8_t*)"GENIUS", 6},
    {(uint8_t*)"CPC", 3},
//  {(uint8_t*)"STAR", 4},
    {(uint8_t*)"UNKNOWN", 7},
};


/*
  +     98.1.0: m� nhan dang du lieu thanh ghi dien nang
  +     97.1.0: m� nhan dang du lieu thanh ghi th�ng so tuc thoi
  +     98.1.0*1: m� nhan dang du lieu chot th�ng truoc lien ke
*/

truct_String         Str_OB_EN_REGISTER      = {(uint8_t*)"98.1.0", 6};
truct_String         Str_OB_INTAN            = {(uint8_t*)"97.1.0", 6};
truct_String         Str_OB_CHOT             = {(uint8_t*)"98.1.0*1",8}; 
truct_String         Str_OB_EVENT            = {(uint8_t*)"96.1.0", 6};
truct_String         Str_OB_LPF              = {(uint8_t*)"99.1.0", 6};
truct_String         Str_OB_RESPONSE_AT      = {(uint8_t*)"RESPONSE", 8}; 
truct_String         Str_OB_INFOR_METER      = {(uint8_t*)"94.1.1", 6}; 

truct_String         Str_OB_IDENT_METER      = {(uint8_t*)"1.0.95.1.2", 10};
truct_String         Str_Connect             = {(uint8_t*)"connect:", 8};
truct_String         Str_DisConnect          = {(uint8_t*)"disconnect:", 11};

truct_String         Str_AlarmPowModerm      = {(uint8_t*)"1.0.95.1.1", 10};
truct_String         Str_PowOn_Moderm        = {(uint8_t*)"poweron:", 8};
truct_String         Str_PowOFF_Moderm       = {(uint8_t*)"poweroff:", 9};

truct_String         Str_AlarmPowLine        = {(uint8_t*)"1.0.95.1.3", 10};
truct_String         Str_PowUp_Line          = {(uint8_t*)"powerup:", 8};
truct_String         Str_PowDown_Line        = {(uint8_t*)"powerdown:", 10};


/*-----------------Cac m� nhan dang dai luong Gi� tri tuc thoi----------------------*/
// Dien ap A, B C, 	    32.7.0	52.7.0  72.7.0		12.7.0

truct_String         Str_Ob_VolA ={(uint8_t*)"(32.7.0)", 8};
truct_String         Str_Ob_VolB ={(uint8_t*)"(52.7.0)", 8};
truct_String         Str_Ob_VolC ={(uint8_t*)"(72.7.0)", 8};

//dong dien A, B , C		05, 06, 07			   31.7.0	51.7.0  71.7.0		11.7.0
truct_String         Str_Ob_CurA ={(uint8_t*)"(31.7.0)", 8};
truct_String         Str_Ob_CurB ={(uint8_t*)"(51.7.0)", 8};
truct_String         Str_Ob_CurC ={(uint8_t*)"(71.7.0)", 8};   
truct_String         Str_Ob_CurNeu ={(uint8_t*)"(91.7.0)", 8};

//G�c lech A, B, C, trung b�nh.						0C, 0D, 0E, 43
//truct_String         Str_Ob_PhRotation ={(uint8_t*)"(13.7.0)", 8};    //trung binh
truct_String         Str_Ob_PhAnglePhA ={(uint8_t*)"(81.7.4)", 8};
truct_String         Str_Ob_PhAnglePhB ={(uint8_t*)"(81.7.15)", 9};
truct_String         Str_Ob_PhAnglePhC ={(uint8_t*)"(81.7.26)", 9};

// He so c�ng suat A, B, C, trung b�nh.				1C, 1D, 1E, 1B		   33.7.0	53.7.0  73.7.0		13.7.0
truct_String         Str_Ob_PoFac   ={(uint8_t*)"(13.7.0)", 8};     //trung binh
truct_String         Str_Ob_PoFacA  ={(uint8_t*)"(33.7.0)", 8};
truct_String         Str_Ob_PoFacB  ={(uint8_t*)"(53.7.0)", 8};
truct_String         Str_Ob_PoFacC  ={(uint8_t*)"(73.7.0)", 8};

truct_String         Str_Ob_Freq ={(uint8_t*)"(14.7.0)", 8};

//+ C�ng suat t�c dung pha A, B, C tong.			0F, 10, 11, 12		   21.7.0	41.7.0  61.7.0  1.7.0	
truct_String         Str_Ob_AcPowTo ={(uint8_t*)"(1.7.0)", 7};
truct_String         Str_Ob_AcPowA  ={(uint8_t*)"(21.7.0)", 8};
truct_String         Str_Ob_AcPowB  ={(uint8_t*)"(41.7.0)", 8};
truct_String         Str_Ob_AcPowC  ={(uint8_t*)"(61.7.0)", 8};

//C�ng suat phan kh�ng pha A, B, C, tong.			13, 14, 15 , 16		   23.7.0	43.7.0	63.7.0	3.7.0
truct_String         Str_Ob_RePowTo ={(uint8_t*)"(3.7.0)", 7};
truct_String         Str_Ob_RePowA  ={(uint8_t*)"(23.7.0)", 8};
truct_String         Str_Ob_RePowB  ={(uint8_t*)"(43.7.0)", 8};
truct_String         Str_Ob_RePowC  ={(uint8_t*)"(63.7.0)", 8};

// C�ng suat bieu kien pha A, B, C, Tong.			17, 18, 19, 1A		   29.7.0	49.7.0	69.7.0  9.7.0
truct_String         Str_Ob_ApprTo  ={(uint8_t*)"(9.7.0)", 7};
truct_String         Str_Ob_ApprA   ={(uint8_t*)"(29.7.0)", 8};
truct_String         Str_Ob_ApprB   ={(uint8_t*)"(49.7.0)", 8};
truct_String         Str_Ob_ApprC   ={(uint8_t*)"(69.7.0)", 8};

/*-----------------Cac m� nhan dang dai luong Tu TI he so nhan----------------------*/
//							Ti: 0.4.2   Tu: 0.4.3		He so nhan (110/11)(105/5)(1.0)<	
truct_String         Str_Ob_Tu      ={(uint8_t*)"(0.4.3)", 7};
truct_String         Str_Ob_Ti      ={(uint8_t*)"(0.4.2)", 7};
truct_String         He_So_Nhan     ={(uint8_t*)"(96.99.9)", 9};

/*-----------------Cac m� nhan dang dai luong thanh ghi dien nang-----------------------*/
//apprent				2D	                    Theo: IEC

truct_String         Str_Ob_En_ImportWh  ={(uint8_t*)"(1.8.0)", 7};
truct_String         Str_Ob_En_ExportWh  ={(uint8_t*)"(2.8.0)", 7};
truct_String         Str_Ob_En_ImportVar ={(uint8_t*)"(3.8.0)", 7};
truct_String         Str_Ob_En_ExportVar ={(uint8_t*)"(4.8.0)", 7};

truct_String         Str_Ob_En_Q1 ={(uint8_t*)"(5.8.0)", 7};
truct_String         Str_Ob_En_Q2 ={(uint8_t*)"(6.8.0)", 7};
truct_String         Str_Ob_En_Q3 ={(uint8_t*)"(7.8.0)", 7};
truct_String         Str_Ob_En_Q4 ={(uint8_t*)"(8.8.0)", 7};

truct_String         Str_Ob_En_Appr ={(uint8_t*)"(9.8.0)", 7};


/*-----------------Cac m� nhan dang dai luong bieu gi�-----------------------*/
//EnergyPlusArate1	    1F				1.8.1       							nang luong tac dung huu cong bieu 1 2 3
truct_String         Str_Ob_AcPlus_Rate1 ={(uint8_t*)"(1.8.1)", 7};
//EnergyPlusArate2	    20				1.8.2  																chieu giao	
truct_String         Str_Ob_AcPlus_Rate2 ={(uint8_t*)"(1.8.2)", 7};
//EnergyPlusArate3	    21				1.8.3 
truct_String         Str_Ob_AcPlus_Rate3 ={(uint8_t*)"(1.8.3)", 7};
//EnergyPlusArate4	    22				1.8.4 
truct_String         Str_Ob_AcPlus_Rate4 ={(uint8_t*)"(1.8.4)", 7};

//EnergySubArate1		23				2.8.1											nang luong vo cong bieu 1 2 3	
truct_String         Str_Ob_AcSub_Rate1 ={(uint8_t*)"(2.8.1)", 7};
//EnergySubArate2		24				2.8.2															chieu nhan
truct_String         Str_Ob_AcSub_Rate2 ={(uint8_t*)"(2.8.2)", 7};
//EnergySubArate3		25				2.8.3
truct_String         Str_Ob_AcSub_Rate3 ={(uint8_t*)"(2.8.3)", 7};
//EnergySubArate4		26				2.8.4
truct_String         Str_Ob_AcSub_Rate4 ={(uint8_t*)"(2.8.4)", 7};

//ReactiveEnergyPlusArate1	49			3.8.1											nang luong phan khang chi?u giao 123
truct_String         Str_Ob_RePlus_Rate1 ={(uint8_t*)"(3.8.1)", 7};
//ReactiveEnergyPlusArate2	4A			3.8.2
truct_String         Str_Ob_RePlus_Rate2 ={(uint8_t*)"(3.8.2)", 7};
//ReactiveEnergyPlusArate3	4B			3.8.3	
truct_String         Str_Ob_RePlus_Rate3 ={(uint8_t*)"(3.8.3)", 7};
//ReactiveEnergyPlusArate3	4C			3.8.4	
truct_String         Str_Ob_RePlus_Rate4 ={(uint8_t*)"(3.8.4)", 7};

//ReactiveEnergySubArate1	4D				4.8.1										nang luong phan khang chi?u giao 123
truct_String         Str_Ob_ReSub_Rate1 ={(uint8_t*)"(4.7.1)", 7};
//ReactiveEnergySubArate2	4E				4.8.2
truct_String         Str_Ob_ReSub_Rate2 ={(uint8_t*)"(4.7.2)", 7};
//ReactiveEnergySubArate3	4F				4.8.3
truct_String         Str_Ob_ReSub_Rate3 ={(uint8_t*)"(4.7.3)", 7};
//ReactiveEnergySubArate3	50				4.8.4
truct_String         Str_Ob_ReSub_Rate4 ={(uint8_t*)"(4.7.4)", 7};
      

/*-----------------Cac m� nhan dang dai luong c�ng su�t cuc dai-----------------------*/
//MaxDemandPlusArate	2E						1.6.0: 		 t�ng		    C�ng suat t�c dung chieu giao	
truct_String         Str_Ob_MaxDe      ={(uint8_t*)"(1.6.0)", 7};
//MaxDemandPlusArate1	31						1.6.1		 Bi�u 1 2 3
truct_String         Str_Ob_MaxDeRate1 ={(uint8_t*)"(1.6.1)", 7};
//MaxDemandPlusArate2	35						1.6.2
truct_String         Str_Ob_MaxDeRate2 ={(uint8_t*)"(1.6.2)", 7};
//MaxDemandPlusArate3	39						1.6.3
truct_String         Str_Ob_MaxDeRate3 ={(uint8_t*)"(1.6.3)", 7};
//MaxDemandPlusArate4	3D						1.6.4
truct_String         Str_Ob_MaxDeRate4 ={(uint8_t*)"(1.6.4)", 7};

//MaxDemandSubArate	2E						    2.6.0: 		 t�ng		    C�ng suat t�c dung chieu giao	
truct_String         Str_Ob_MaxDeSub      ={(uint8_t*)"(2.6.0)", 7};
//MaxDemandsubArate1	32						2.6.1		 Bi�u 1 2 3
truct_String         Str_Ob_MaxDeSub_Rate1 ={(uint8_t*)"(2.6.1)", 7};
//MaxDemandSubArate2	36						2.6.2
truct_String         Str_Ob_MaxDeSub_Rate2 ={(uint8_t*)"(2.6.2)", 7};
//MaxDemandSubArate3	3A						2.6.3
truct_String         Str_Ob_MaxDeSub_Rate3 ={(uint8_t*)"(2.6.3)", 7};
//MaxDemandSubArate4	3E						2.6.4
truct_String         Str_Ob_MaxDeSub_Rate4 ={(uint8_t*)"(2.6.4)", 7};


/*-----------------Cac m� nhan dang dai luong chot thang-----------------------*/
//thanh ghi dien nang tong chot
truct_String         Str_Ob_AcImTotal_Chot ={(uint8_t*)"(1.8.0*1)", 9};
truct_String         Str_Ob_AcExTotal_Chot ={(uint8_t*)"(2.8.0*1)", 9};
truct_String         Str_Ob_ReImTotal_Chot ={(uint8_t*)"(3.8.0*1)", 9};
truct_String         Str_Ob_ReExTotal_Chot ={(uint8_t*)"(4.8.0*1)", 9};

truct_String         Str_Ob_Q1_Chot        ={(uint8_t*)"(5.8.0*1)", 9};
truct_String         Str_Ob_Q2_Chot        ={(uint8_t*)"(6.8.0*1)", 9};
truct_String         Str_Ob_Q3_Chot        ={(uint8_t*)"(7.8.0*1)", 9};
truct_String         Str_Ob_Q4_Chot        ={(uint8_t*)"(8.8.0*1)", 9};

truct_String         Str_Ob_Appr_Chot      ={(uint8_t*)"(9.8.0*1)", 9};

//Thanh ghi bieu gia chot
truct_String         Str_Ob_AcPlus_Rate1_chot    ={(uint8_t*)"(1.8.1*1)", 9};														
truct_String         Str_Ob_AcPlus_Rate2_chot    ={(uint8_t*)"(1.8.2*1)", 9};
truct_String         Str_Ob_AcPlus_Rate3_chot    ={(uint8_t*)"(1.8.3*1)", 9};
truct_String         Str_Ob_AcPlus_Rate4_chot    ={(uint8_t*)"(1.8.4*1)", 9};
truct_String         Str_Ob_AcSub_Rate1_chot     ={(uint8_t*)"(2.8.1*1)", 9};
truct_String         Str_Ob_AcSub_Rate2_chot     ={(uint8_t*)"(2.8.2*1)", 9};
truct_String         Str_Ob_AcSub_Rate3_chot     ={(uint8_t*)"(2.8.3*1)", 9};	
truct_String         Str_Ob_AcSub_Rate4_chot     ={(uint8_t*)"(2.8.4*1)", 9};
truct_String         Str_Ob_RePlus_Rate1_chot    ={(uint8_t*)"(3.8.1*1)", 9};
truct_String         Str_Ob_RePlus_Rate2_chot    ={(uint8_t*)"(3.8.2*1)", 9};
truct_String         Str_Ob_RePlus_Rate3_chot    ={(uint8_t*)"(3.8.3*1)", 9};
truct_String         Str_Ob_RePlus_Rate4_chot    ={(uint8_t*)"(3.8.4*1)", 9};
truct_String         Str_Ob_ReSub_Rate1_chot     ={(uint8_t*)"(4.7.1*1)", 9};
truct_String         Str_Ob_ReSub_Rate2_chot     ={(uint8_t*)"(4.7.2*1)", 9};
truct_String         Str_Ob_ReSub_Rate3_chot     ={(uint8_t*)"(4.7.3*1)", 9};
truct_String         Str_Ob_ReSub_Rate4_chot     ={(uint8_t*)"(4.7.4*1)", 9};  

/*-----------------Cac m� nhan dang dai luong c�ng su�t cuc dai-----------------------*/
truct_String         Str_Ob_MaxDe_Chot          ={(uint8_t*)"(1.6.0*1)", 9};
truct_String         Str_Ob_MaxDeRate1_Chot     ={(uint8_t*)"(1.6.1*1)", 9};
truct_String         Str_Ob_MaxDeRate2_Chot     ={(uint8_t*)"(1.6.2*1)", 9};
truct_String         Str_Ob_MaxDeRate3_Chot     ={(uint8_t*)"(1.6.3*1)", 9};
truct_String         Str_Ob_MaxDeRate4_Chot     ={(uint8_t*)"(1.6.4*1)", 9};

truct_String         Str_Ob_MaxDe2_Chot          ={(uint8_t*)"(2.6.0*1)", 9};
truct_String         Str_Ob_MaxDe2Rate1_Chot     ={(uint8_t*)"(2.6.1*1)", 9};
truct_String         Str_Ob_MaxDe2Rate2_Chot     ={(uint8_t*)"(2.6.2*1)", 9};
truct_String         Str_Ob_MaxDe2Rate3_Chot     ={(uint8_t*)"(2.6.3*1)", 9};
truct_String         Str_Ob_MaxDe2Rate4_Chot     ={(uint8_t*)"(2.6.4*1)", 9};
/*-----------------Cac m� nhan dang dai luong lpf-----------------------*/

truct_String        Str_Ob_lpf[18]=
{
    {(uint8_t*)"(1.5.0)", 7},    //Import W  //trong file m� ta obis txt.
    {(uint8_t*)"(2.5.0)", 7},
    {(uint8_t*)"(3.5.0)", 7},
    {(uint8_t*)"(4.5.0)", 7},
    
    {(uint8_t*)"(5.7.0)", 7},
    {(uint8_t*)"(6.7.0)", 7},
    {(uint8_t*)"(7.7.0)", 7},
    {(uint8_t*)"(8.7.0)", 7},
    
    {(uint8_t*)"(32.7.0)", 8},
    {(uint8_t*)"(52.7.0)", 8},
    {(uint8_t*)"(72.7.0)", 8},
    
    {(uint8_t*)"(31.7.0)", 8},
    {(uint8_t*)"(51.7.0)", 8},
    {(uint8_t*)"(71.7.0)", 8},
    
    {(uint8_t*)"(33.7.0)", 8},
    {(uint8_t*)"(53.7.0)", 8},
    {(uint8_t*)"(73.7.0)", 8},
    
    {(uint8_t*)"(13.7.0)", 8}, 
};

truct_String        Str_Ob_lpf_type2[4]=
{
    {(uint8_t*)"(1.4.0)", 7},    //Import W  //trong file m� ta obis txt.
    {(uint8_t*)"(2.4.0)", 7},
    {(uint8_t*)"(3.4.0)", 7},
    {(uint8_t*)"(4.4.0)", 7},
};

/*
Dts27(shenzhen) ; va elster: 1.4.0   1.40

cpc genius gelex (huu hong vinasino) landis : 1.5.0 ;   2.5.0;
*/

truct_String        Unit_Lpf_type2[4]=
{
    {(uint8_t*)"kW", 2},
    {(uint8_t*)"kW", 2},
    {(uint8_t*)"kVar", 4},
    {(uint8_t*)"kVar", 4},
};
/*-----------------Cac m� nhan dang dai luong event-----------------------*/

//Khai bao lai 1 list obis event cua dien luc
truct_String    Ev_ChangeTime_Count   = {(uint8_t*)"(C.51.15)", 9};   //count time change
truct_String    Ev_ChangeTime_Startt  = {(uint8_t*)"(C.51.16)", 9};   //Start time change
    
truct_String    Ev_PhaseAfail_Count  = {(uint8_t*)"(C.51.83)", 9};   //pha a fail start count
truct_String    Ev_PhaseBfail_Count  = {(uint8_t*)"(C.51.87)", 9};   //pha b fail start count
truct_String    Ev_PhaseCfail_Count  = {(uint8_t*)"(C.51.91)", 9};   //pha c fail start count    
    
truct_String    Ev_PhaseAfail_Startt = {(uint8_t*)"(C.51.84)", 9};    //pha a fail start time 0x0D,0x0E,0x0F,0x10,0x11,0x12
truct_String    Ev_PhaseAfail_Stopt  = {(uint8_t*)"(C.51.86)", 9};    //pha a fail stop time
truct_String    Ev_PhaseBfail_Startt = {(uint8_t*)"(C.51.88)", 9};    //pha b fail start time
truct_String    Ev_PhaseBfail_Stopt  = {(uint8_t*)"(C.51.90)", 9};     //pha b fail stop time
truct_String    Ev_PhaseCfail_Startt = {(uint8_t*)"(C.51.92)", 9};     //pha c fail start time
truct_String    Ev_PhaseCfail_Stopt  = {(uint8_t*)"(C.51.94)", 9};     //pha c fail stop time
    
truct_String    Ev_PhaseAreve_Count  = {(uint8_t*)"(C.52.13)", 9};     //pha a reverse start count 
truct_String    Ev_PhaseBreve_Count  = {(uint8_t*)"(C.52.17)", 9};    //pha b reverse start count
truct_String    Ev_PhaseCreve_Count  = {(uint8_t*)"(C.52.21)", 9};    //pha c reverse start count
    
truct_String    Ev_PhaseAreve_Startt = {(uint8_t*)"(C.52.14)", 9};    //pha a reverse start time 
truct_String    Ev_PhaseAreve_Stopt  = {(uint8_t*)"(C.52.16)", 9};    //pha a reverse stop time
truct_String    Ev_PhaseBreve_Startt = {(uint8_t*)"(C.52.18)", 9};     //pha b reverse start time    
truct_String    Ev_PhaseBreve_Stopt  = {(uint8_t*)"(C.52.20)", 9};     //pha b reverse stop time
truct_String    Ev_PhaseCreve_Startt = {(uint8_t*)"(C.52.22)", 9};     //pha c reverse start time
truct_String    Ev_PhaseCreve_Stopt  = {(uint8_t*)"(C.52.24)", 9};     //pha c reverse stop time

//
truct_String    Ev_PhaseALow_Count = {(uint8_t*)"(C.51.59)", 9};  //Dien ap thap pha A
truct_String    Ev_PhaseBLow_Count = {(uint8_t*)"(C.51.63)", 9};  //Dien ap thap pha B
truct_String    Ev_PhaseCLow_Count = {(uint8_t*)"(C.51.67)", 9};  //Dien ap thap pha C 

truct_String    Ev_PhaseALow_Startt = {(uint8_t*)"(C.51.60)", 9};    //Dien ap thap pha A start time 
truct_String    Ev_PhaseALow_Stopt  = {(uint8_t*)"(C.51.62)", 9};    //Dien ap thap pha A stop time
truct_String    Ev_PhaseBLow_Startt = {(uint8_t*)"(C.51.64)", 9};     //Dien ap thap pha B start time    
truct_String    Ev_PhaseBLow_Stopt  = {(uint8_t*)"(C.51.66)", 9} ;     //Dien ap thap pha B stop time
truct_String    Ev_PhaseCLow_Startt = {(uint8_t*)"(C.51.68)", 9};     //Dien ap thap pha C start time
truct_String    Ev_PhaseCLow_Stopt  = {(uint8_t*)"(C.51.70)", 9};     //Dien ap thap pha C stop time


truct_String    Ev_PhaseAOver_Count = {(uint8_t*)"(C.51.71)", 9};  //qu� ap pha A
truct_String    Ev_PhaseBOver_Count = {(uint8_t*)"(C.51.75)", 9};  //qu� ap pha B
truct_String    Ev_PhaseCOver_Count = {(uint8_t*)"(C.51.79)", 9};  //qu� ap pha C 

truct_String    Ev_PhaseAOver_Startt = {(uint8_t*)"(C.51.72)", 9};    //qu� ap start time 
truct_String    Ev_PhaseAOver_Stopt  = {(uint8_t*)"(C.51.74)", 9};    //qu� ap pha A stop time
truct_String    Ev_PhaseBOver_Startt = {(uint8_t*)"(C.51.76)", 9};     //qu� ap pha B start time    
truct_String    Ev_PhaseBOver_Stopt  = {(uint8_t*)"(C.51.78)", 9} ;     //qu� ap pha B stop time
truct_String    Ev_PhaseCOver_Startt = {(uint8_t*)"(C.51.80)", 9};     //qu� ap pha C start time
truct_String    Ev_PhaseCOver_Stopt  = {(uint8_t*)"(C.51.82)", 9};     //qu� ap pha C stop time

//
truct_String    Ev_PhaseAOverCur_Count =  {(uint8_t*)"(C.52.1)", 8};  //Qu� d�ng phase A Count
truct_String    Ev_PhaseBOverCur_Count =  {(uint8_t*)"(C.52.5)", 8};  //Qu� d�ng phase B Count
truct_String    Ev_PhaseCOverCur_Count =  {(uint8_t*)"(C.52.9)", 8};  //Qu� d�ng phase C Count

truct_String    Ev_PhaseAOverCur_Startt =  {(uint8_t*)"(C.52.2)", 8};  //Qu� d�ng phase A Time start   //over curent
truct_String    Ev_PhaseBOverCur_Startt =  {(uint8_t*)"(C.52.6)", 8};  //Qu� d�ng phase B Time start 
truct_String    Ev_PhaseCOverCur_Startt =  {(uint8_t*)"(C.52.10)", 9};  //Qu� d�ng phase C Time start 

truct_String    Ev_PhaseAOverCur_Stop =  {(uint8_t*)"(C.52.4)", 8};  //Qu� d�ng phase A Time stop 
truct_String    Ev_PhaseBOverCur_Stop =  {(uint8_t*)"(C.52.8)", 8};  //Qu� d�ng phase B Time stop 
truct_String    Ev_PhaseCOverCur_Stop =  {(uint8_t*)"(C.52.12)", 9};  //Qu� d�ng phase C Time stop 
//
truct_String    Ev_ReverseABC_Count   =  {(uint8_t*)"(C.52.25)", 9};  //nguoc thu tu pha A-C B count
truct_String    Ev_ReverseABC_Startt  =  {(uint8_t*)"(C.52.26)", 9};  //nguoc thu tu pha A-C B start time
truct_String    Ev_ReverseABC_Stop    =  {(uint8_t*)"(C.52.28)", 9};  //nguoc thu tu pha A-C B stop time


truct_String    Ev_PowerDow_Count    =  {(uint8_t*)"(C.7.0)", 7};  //mat dien count
truct_String    Ev_PowerDow_Startt   =  {(uint8_t*)"(C.7.10)", 8};  //mat dien start time
truct_String    Ev_PowerOn_Startt    =  {(uint8_t*)"(C.51.14)",9};    //Power On

truct_String    Ev_ProcessTime_Count    =  {(uint8_t*)"(C.51.15)", 9};  //lap trinh thoi gian count
truct_String    Ev_ProcessTime_Startt   =  {(uint8_t*)"(C.51.16)", 9};  //lap trinh thoi gian time start

truct_String    Ev_ProcessPara_Count    =  {(uint8_t*)"(C.2.0)", 7};  //lap trinh thong so count
truct_String    Ev_ProcessPara_Startt   =  {(uint8_t*)"(C.2.1)", 7};  //lap trinh thong so time start

truct_String    Ev_ClearData_Count    =  {(uint8_t*)"(C.51.25)", 9};  //x�a du lieu count
truct_String    Ev_ClearData_Startt   =  {(uint8_t*)"(C.51.26)", 9};  //xoa du lieu start time

truct_String    Ev_LowRTC_Count     =  {(uint8_t*)"(C.52.33)", 9};  //het pin rtc count
truct_String    Ev_LowRTC_Startt    =  {(uint8_t*)"(C.52.34)", 9};  //het pin rtcstart time
truct_String    Ev_LowRTC_Stop      =  {(uint8_t*)"(C.52.36)", 9}; //het pin rtc stop time

truct_String    Ev_MagField_Startt  =  {(uint8_t*)"(C.51.6)",8};      //Strong magnetic fields start
truct_String    Ev_RePowFlow_Startt =  {(uint8_t*)"(C.51.8)",8};      //Reverse Power Start 


truct_String    Ev_OpenCover_Startt  = {(uint8_t*)"(C.51.4)",8};      //Meter cover open
truct_String    Ev_CloseCover_Startt = {(uint8_t*)"(C.51.24)",9};     //Meter cover close                    
truct_String    Ev_OpenTer_Startt    = {(uint8_t*)"(C.51.2)",8};      //Terminal cover open
truct_String    Ev_CloseTer_Startt   = {(uint8_t*)"(C.51.22)",9};      //Terminal cover close    

    
    
    
/*-----------------Cac m� nhan dang thong tin Moderm-----------------------*/
truct_String         ModermIP             ={(uint8_t*)"(99.0.0.1)", 10};
truct_String         ModermFirmVer        ={(uint8_t*)"(99.0.2.1)", 10};
truct_String         ModermCSQ            ={(uint8_t*)"(99.0.4.1)", 10};


/*-----------------Khai bao cac don vi co cho cac dai luong-----------------------*/
//dai luong tuc thoi
truct_String         Unit_Voltage           ={(uint8_t*)"V", 1};
truct_String         Unit_Current           ={(uint8_t*)"A", 1};
truct_String         Unit_Freq              ={(uint8_t*)"Hz", 2};

truct_String         Unit_Active_Intan      ={(uint8_t*)"W", 1};
truct_String         Unit_Reactive_Intan    ={(uint8_t*)"Var", 3};
truct_String         Unit_Q_Intan           ={(uint8_t*)"Var", 3};
truct_String         Unit_Apprent_Intan     ={(uint8_t*)"VA", 2};

truct_String         Unit_Active_Intan_Kw      ={(uint8_t*)"kW", 2};
truct_String         Unit_Reactive_Intan_Kvar    ={(uint8_t*)"kVar", 4};


//�ai luong thanh ghi nang luong va bieu gia, cs cuc dai
truct_String         Unit_Active_EnTotal    ={(uint8_t*)"kWh", 3};
truct_String         Unit_Reactive_EnTotal  ={(uint8_t*)"kVarh", 5};
truct_String         Unit_Q_EnTotal         ={(uint8_t*)"kVarh", 5};
truct_String         Unit_Apprent_EnTotal   ={(uint8_t*)"kVAh", 4};

truct_String         Unit_MAXDEMAND         ={(uint8_t*)"kW", 2};
truct_String         Unit_Energy_Min        ={(uint8_t*)"Wh", 2};
truct_String         Unit_En_Reactive_Min   ={(uint8_t*)"Varh", 4};

truct_String        Unit_Lpf[18]=
{
    {(uint8_t*)"W", 1},
    {(uint8_t*)"W", 1},
    {(uint8_t*)"Var", 3},
    {(uint8_t*)"Var", 3},
    
    {(uint8_t*)"Var", 3},
    {(uint8_t*)"Var", 3},
    {(uint8_t*)"Var", 3},
    {(uint8_t*)"Var", 3},
    
    {(uint8_t*)"V", 1},  //8
    {(uint8_t*)"V", 1},
    {(uint8_t*)"V", 1},
    
    {(uint8_t*)"A", 1},
    {(uint8_t*)"A", 1},
    {(uint8_t*)"A", 1},   //13
    
    {(uint8_t*)"", 0},
    {(uint8_t*)"", 0},
    {(uint8_t*)"", 0},
    
    {(uint8_t*)"", 0},
};





/*-----------------Funcion-----------------------*/



int32_t Convert_float_2int (uint32_t Float_Point_IEEE, uint8_t scale)
{
	int 		scaleVal = 1;
	float 		floatTmp = 0;
	uint8_t		i = 0;

	//Get scale value
	for (i = 0; i < scale; i++) 
	{
		scaleVal *= 10;
	}
	
	floatTmp = (float) Convert_FloatPoint_2Float(Float_Point_IEEE);

	return (int32_t) (floatTmp * scaleVal);
}


float Convert_FloatPoint_2Float (uint32_t Float_Point)
{
	return *((float*)(&Float_Point));
}

int32_t Convert_uint_2int (uint32_t Num)
{
	return *((int32_t*)(&Num));
}

int16_t Convert_uint16_2int16 (uint16_t Num)
{
	return *((int16_t*)(&Num));
}


int64_t Convert_uint64_2int64 (uint64_t Num)
{
	return *((int64_t*)(&Num));
}
//t�nh BBC bang cach xor c�c byte.

uint8_t BBC_Cacul (uint8_t* Buff, uint16_t length)
{
	uint16_t i = 0;
	uint8_t Result = 0;
	
	for(i = 0; i < length; i++)
		Result ^= *(Buff + i);
	return Result;
}
 

uint8_t Check_BBC (truct_String* Str)
{
	uint8_t BBC_Value;
	
	BBC_Value = BBC_Cacul((Str->Data_a8 + 1), (Str->Length_u16 - 2));
	
	if(BBC_Value == *(Str->Data_a8 + Str->Length_u16 - 1))
		 return 1;
	else 
        return 0;
}

void Pack_HEXData_Frame_Uint64 (truct_String* Payload, uint64_t Data, uint8_t Scale)
{
    uint16_t i = 0;
    uint16_t j = 0;
    uint8_t Count = 0;
    uint8_t  PDu = 0;
    uint8_t Temp = 0;    
    uint8_t BufStrDec[30] = {0};
    
    truct_String StrDecData = {&BufStrDec[0], 0};
    
    if(Data == 0)
    {
        *(Payload->Data_a8 + Payload->Length_u16) = '0';
        Payload->Length_u16++;
        return;
    }
    
    while (Data != 0)
    {
        PDu = (uint8_t) (Data%10);
        Data = (Data/10);
        *(StrDecData.Data_a8 + StrDecData.Length_u16++) = PDu + 0x30;
    }
    //�ao nguoc lai
    for(i = 0; i <(StrDecData.Length_u16/2); i++)
    {
        Temp = *(StrDecData.Data_a8 + StrDecData.Length_u16 - i - 1);
        *(StrDecData.Data_a8 + StrDecData.Length_u16 - i - 1) = *(StrDecData.Data_a8 + i);
        *(StrDecData.Data_a8 + i) = Temp;
    }
    
    if(StrDecData.Length_u16 >= (Scale + 1))
    {
        if(Scale != 0)
        {
            //chuyen cac chu so dich ra sau 1 de lay 1 vi tri cho
            for(i = 0; i < Scale; i++)
            {
                *(StrDecData.Data_a8 + StrDecData.Length_u16 - i) = *(StrDecData.Data_a8 + StrDecData.Length_u16 - i - 1);
            }
        
            *(StrDecData.Data_a8 + StrDecData.Length_u16 - i) = '.';                                                           
            //them dau cham vao vi tri da chon
            StrDecData.Length_u16++;
            //neu sau dau. c� cac so 0 bi thua thi bo di
            for(j = (StrDecData.Length_u16 - 1); j >= (StrDecData.Length_u16 - 1 - Scale); j--)
            {
              if(*(StrDecData.Data_a8 + j) == '0')
              {
                  Count++;  
              }else if(*(StrDecData.Data_a8 + j) == '.')
              {
                  Count++;
                  break;
              }
              else break;
            }
            
            StrDecData.Length_u16 -= Count;
        } 
    }else
    {
        if(Scale!= 0)
        {
            //chuyen cac chu so dich ra sau: (scale - strDecLength) + 2.vi tri
            for(i = 0; i < StrDecData.Length_u16; i++)
            {
                *(StrDecData.Data_a8 + Scale + 1 - i) = *(StrDecData.Data_a8 + StrDecData.Length_u16 - i - 1);
            }
        
            //ghi them 0. vao
            *(StrDecData.Data_a8 + 0) = '0';
            *(StrDecData.Data_a8 + 1) = '.';
            for(i = 0; i < (Scale - StrDecData.Length_u16); i++)
                *(StrDecData.Data_a8 + 2 + i) = '0';
                
            StrDecData.Length_u16 = Scale + 2;
            //neu sau dau. c� cac so 0 bi thua thi bo di
            for(j = (StrDecData.Length_u16 - 1); j >= 1 ; j--)
              if(*(StrDecData.Data_a8 + j) == '0')
              {
                  Count++;  
              }else if(*(StrDecData.Data_a8 + j) == '.')
              {
                  Count++;
                  break;
              }
              else  break;
            
            StrDecData.Length_u16 -= Count;
        }
    }
    
    Copy_String_2(Payload, &StrDecData);
}

//Test lai function
void Pack_HEXData_Frame (truct_String* Payload, int64_t Data, uint8_t Scale)
{
    uint8_t i = 0;
    uint8_t BufStrDec[30] = {0};
    uint8_t Sign = 0;
    
    truct_String StrDecData = {&BufStrDec[0], 0};
    
    if(Data == 0)
    {
        *(Payload->Data_a8 + Payload->Length_u16) = '0';
        Payload->Length_u16++;
        return;
    }
    
    if(Data < 0)
    {
        Sign = 1;
        Data = 0 - Data;
    }
    
    Pack_HEXData_Frame_Uint64(&StrDecData, (uint64_t)Data, Scale);
    
    if(*(StrDecData.Data_a8 + StrDecData.Length_u16) != '0')
    {
        if(Sign == 1)
        {
            for(i = 0; i < StrDecData.Length_u16; i++)
              *(StrDecData.Data_a8 + StrDecData.Length_u16 - i) =  *(StrDecData.Data_a8 + StrDecData.Length_u16 - i - 1);
            
            *(StrDecData.Data_a8) = '-';
            StrDecData.Length_u16 ++;
        }
    }
        //copy ca chuoi vao
    Copy_String_2(Payload, &StrDecData);
}
/*
    -function chung:
        + khac nhau moc thoi gian danh cho tung loai cong to
    - type: 1: CPC
            2: ELster
            3: Genius
*/

void Epoch_to_date_time(ST_TIME_FORMAT* date_time, uint32_t meterTS, uint8_t type)
{
	unsigned int years;
	unsigned int year;
	unsigned int month;
	uint32_t	 epoch;
	
    switch(type)
    {
        case 1:
            epoch = meterTS + 378691200;   //from 2000. Do CPC moc thoi gian tu 1.1.2012. n�n cong them cho ve nam 2000
            break;
        case 2:
            epoch = meterTS - 946684800;   //from 2000.moc tu 1976
            break; 
        case 3:
            epoch = meterTS - 126230400;     //1/1/1996 - 1/1/2000: 1461 ngay
            break;     
        default: 
            epoch = meterTS;
          break;
    }
	
    date_time->sec = epoch%60; epoch /= 60;
    date_time->min = epoch%60; epoch /= 60;
    date_time->hour= epoch%24; epoch /= 24;

    years = epoch/(365*4+1)*4; epoch %= 365*4+1;

    for (year=3; year>0; year--)
    {
        if (epoch >= days[year][0])
            break;
    }

    for (month=11; month>0; month--)
    {
        if (epoch >= days[year][month])
            break;
    }

    date_time->year  = years+year;
    date_time->month = month+1;
    date_time->date  = epoch-days[year][month]+1;
}



/*
    -function chung:
        + khac nhau moc thoi gian danh cho tung loai cong to
    - type: 1: CPC
            3: Genius
*/


uint32_t HW_RTC_GetCalendarValue_Second( ST_TIME_FORMAT sTimeRTC, uint8_t type)
{
  uint32_t correction;
  uint32_t seconds;
  
 //anh l�y bat d�u tu d�y
  /* calculte amount of elapsed days since 01/01/2000 */
  seconds= DIVC( (DAYS_IN_YEAR*3 + DAYS_IN_LEAP_YEAR)* sTimeRTC.year , 4);

  correction = ( (sTimeRTC.year % 4) == 0 ) ? DAYS_IN_MONTH_CORRECTION_LEAP : DAYS_IN_MONTH_CORRECTION_NORM ;
 
  seconds +=( DIVC( (sTimeRTC.month-1)*(30+31) ,2 ) - (((correction>> ((sTimeRTC.month-1)*2) )&0x3)));

  seconds += (sTimeRTC.date -1);
  
  /* convert from days to seconds */
  seconds *= SECONDS_IN_1DAY; 

  seconds += ( ( uint32_t )sTimeRTC.sec + 
             ( ( uint32_t )sTimeRTC.min * SECONDS_IN_1MINUTE ) +
             ( ( uint32_t )sTimeRTC.hour * SECONDS_IN_1HOUR ) ) ;

    switch(type)
    {
        case 1:
            seconds -= 378691200;   //Do moc CPC 2012
            break; 
        case 3:
            seconds += 126230400;
            break;     
        default: 
          break;
    }
    
    return seconds;   
}


//Dong goi ban tin k�t noi/ ngat ket noi voi cong to
/*
    <STX>1.0.95.1.2(IMEI)(MeterID)(MeterType)(connect:YYMMDDhhmm) <ETX><BCC>
    <STX>1.0.95.1.2(IMEI)(MeterID)(MeterType)(disconnect:YYMMDDhhmm)
    <ETX><BCC> 
    *Phan 1.0. can xem lai
*/
uint8_t Identi_Meter (truct_String* Payload, truct_String* sMeter_ID, uint8_t Meter_Type, ST_TIME_FORMAT sTime, truct_String Str_Connect)
{
    uint8_t Temp_BBC = 0;
    
    //Reset Buff
    Reset_Buff(Payload);
    
    Payload->Length_u16 = 0;
    
    *(Payload->Data_a8 + Payload->Length_u16++) = 0x02;
    //Obis Code
    Copy_String_2(Payload, &Str_OB_IDENT_METER);
    //DCUID
    
    *(Payload->Data_a8 + Payload->Length_u16++) = '(';
    Copy_String_2(Payload, &sDCU.sDCU_id); 
    *(Payload->Data_a8 + Payload->Length_u16++) = ')';
    
    //ID Meter
    *(Payload->Data_a8 + Payload->Length_u16++) = '(';  
    Copy_String_2(Payload, sMeter_ID); 
    *(Payload->Data_a8 + Payload->Length_u16++) = ')';
    
    //Meter type
    *(Payload->Data_a8 + Payload->Length_u16++) = '(';
    Copy_String_2(Payload, &Str_MeterType_u8[Meter_Type]);  
    *(Payload->Data_a8 + Payload->Length_u16++) = ')';
    //Payload 
    
    *(Payload->Data_a8 + Payload->Length_u16++) = '(';
    //String connect: /disconnect:
    Copy_String_2(Payload, &Str_Connect); 
    //Timlabel
    Copy_String_STime(Payload,sTime);
    *(Payload->Data_a8 + Payload->Length_u16++) = ')';
    
    //ETX
    *(Payload->Data_a8 + Payload->Length_u16++) = 0x03; 
    //BBC
    Temp_BBC = BBC_Cacul(Payload->Data_a8 + 1,Payload->Length_u16 - 1);
    *(Payload->Data_a8 + Payload->Length_u16++) = Temp_BBC;
    
    return 1;
}
/*
    Du lieu su kien modem: Kh�ng y�u cau HES x�c nhan �ACK�
    Mat/c� nguon modem
    <STX>1.0.95.1.1(IMEI)(MeterID)(MeterType)(poweroff:YYMMDDhhmm) <ETX><BCC>
    <STX>1.0.95.1.1(IMEI)(MeterID)(MeterType)(poweron:YYMMDDhhmm) <ETX><BCC>
    Canh b�o c� dien
    <STX>1.0.95.1.3(IMEI)(MeterID)(MeterType)(powerup:YYMMDDhhmm) <ETX><BCC>
    Canh b�o c�p dien
    <STX>1.0.95.1.3(IMEI)(MeterID)(MeterType)(powerdown:YYMMDDhhmm)
    <ETX><BCC>
*/

void Pack_AlarmPower (truct_String* Payload, truct_String* Str_ObCode, truct_String* sMeter_ID, uint8_t Meter_Type, ST_TIME_FORMAT sTime, truct_String Str_Connect)
{
    uint8_t Temp_BBC = 0;
    
    //Reset Buff
    Reset_Buff(Payload);
    
    Payload->Length_u16 = 0;
    
    *(Payload->Data_a8 + Payload->Length_u16++) = 0x02;
    //Obis Code
    Copy_String_2(Payload, Str_ObCode);
    //DCUID
    *(Payload->Data_a8 + Payload->Length_u16++) = '(';
    Copy_String_2(Payload, &sDCU.sDCU_id); 
    *(Payload->Data_a8 + Payload->Length_u16++) = ')';
    
    //ID Meter
    *(Payload->Data_a8 + Payload->Length_u16++) = '(';
    Copy_String_2(Payload, sMeter_ID);  
    *(Payload->Data_a8 + Payload->Length_u16++) = ')';
    
    //Meter type
    *(Payload->Data_a8 + Payload->Length_u16++) = '(';
    Copy_String_2(Payload, &Str_MeterType_u8[Meter_Type]);  
    *(Payload->Data_a8 + Payload->Length_u16++) = ')';
    //Payload 
    
    *(Payload->Data_a8 + Payload->Length_u16++) = '(';
    //String connect: /disconnect:
    Copy_String_2(Payload, &Str_Connect); 
    //Timlabel
    Copy_String_STime(Payload,sTime);
    *(Payload->Data_a8 + Payload->Length_u16++) = ')';
    
    //ETX
    *(Payload->Data_a8 + Payload->Length_u16++) = 0x03; 
    //BBC
    Temp_BBC = BBC_Cacul(Payload->Data_a8 + 1,Payload->Length_u16 - 1);
    *(Payload->Data_a8 + Payload->Length_u16++) = Temp_BBC;
}

//Dong goi ban tin thong tin Moderm
void Infor_Moderm (truct_String* Payload, truct_String* Str_ObCode, truct_String* sMeter_ID, uint8_t Meter_Type, ST_TIME_FORMAT sTime, truct_String* FirmVer, int8_t	RSSI_c8, uint16_t Hesonhan)
{
    uint8_t Temp_BBC = 0;
    
    //Reset Buff
    Reset_Buff(Payload);
    
    Payload->Length_u16 = 0;
    
    *(Payload->Data_a8 + Payload->Length_u16++) = 0x02;
    //Obis Code
    Copy_String_2(Payload, Str_ObCode);
    //DCUID
    *(Payload->Data_a8 + Payload->Length_u16++) = '(';
    Copy_String_2(Payload, &sDCU.sDCU_id); 
    *(Payload->Data_a8 + Payload->Length_u16++) = ')';
    
    //ID Meter
    *(Payload->Data_a8 + Payload->Length_u16++) = '(';
    Copy_String_2(Payload, sMeter_ID); 
    *(Payload->Data_a8 + Payload->Length_u16++) = ')';
    
    //Meter type
    *(Payload->Data_a8 + Payload->Length_u16++) = '(';
    Copy_String_2(Payload, &Str_MeterType_u8[Meter_Type]);  
    *(Payload->Data_a8 + Payload->Length_u16++) = ')';
    
    //Timlabel
    *(Payload->Data_a8 + Payload->Length_u16++) = '(';
    Copy_String_STime(Payload,sTime);
    *(Payload->Data_a8 + Payload->Length_u16++) = ')';
    
    //numqty
    *(Payload->Data_a8 + Payload->Length_u16++) = '(';
    *(Payload->Data_a8 + Payload->Length_u16++) = '4';
    *(Payload->Data_a8 + Payload->Length_u16++) = ')';
    
    //Obis dai luong
    Copy_String_2(Payload, &ModermIP); 
    Copy_String_2(Payload, &ModermFirmVer); 
    Copy_String_2(Payload, &ModermCSQ);
    Copy_String_2(Payload, &He_So_Nhan); 
    
    *(Payload->Data_a8 + Payload->Length_u16++) = 0x0D;
    *(Payload->Data_a8 + Payload->Length_u16++) = 0x0A;
    
    //data
    *(Payload->Data_a8 + Payload->Length_u16++) = '(';
    Copy_String_2(Payload, &ModermIP); 
    *(Payload->Data_a8 + Payload->Length_u16++) = ')';
        
    *(Payload->Data_a8 + Payload->Length_u16++) = '(';
    Copy_String_2(Payload, FirmVer); 
    *(Payload->Data_a8 + Payload->Length_u16++) = ')';
        
    *(Payload->Data_a8 + Payload->Length_u16++) = '(';
    Pack_HEXData_Frame(Payload, (int64_t) RSSI_c8, 0);  //csq
    *(Payload->Data_a8 + Payload->Length_u16++) = ')';
        
    *(Payload->Data_a8 + Payload->Length_u16++) = '(';
    Pack_HEXData_Frame(Payload, (int64_t) Hesonhan, 0); 
    *(Payload->Data_a8 + Payload->Length_u16++) = ')';
        
    
    //ETX
    *(Payload->Data_a8 + Payload->Length_u16++) = 0x03; 
    //BBC
    Temp_BBC = BBC_Cacul(Payload->Data_a8 + 1,Payload->Length_u16 - 1);
    *(Payload->Data_a8 + Payload->Length_u16++) = Temp_BBC;
}






uint16_t Find_Idex_By_sTime (ST_TIME_FORMAT sTimeFind, ST_TIME_FORMAT sTime_Real)
{
    uint32_t Date_Real; 
    uint32_t Date_Find;
    
    Date_Real = HW_RTC_GetCalendarValue_Second(sTime_Real, 0) / SECONDS_IN_1DAY;
    Date_Find = HW_RTC_GetCalendarValue_Second(sTimeFind, 0) / SECONDS_IN_1DAY;
    
    if(Date_Find > Date_Real) 
      return 0;
    else 
      return (Date_Real - Date_Find + 1);   //CPC Index tai thoi diem hien tai la 1. truoc 1 ngay la 2
}



//2020 sua them dong goi push data 103EVN HES

void Write_Header_TSVH_Push103 (void)
{
    //STX
    *(Get_Meter_Info.Str_Payload.Data_a8 + Get_Meter_Info.Str_Payload.Length_u16++) = STX;
    //obiscode Intan
    Copy_String_2(&Get_Meter_Info.Str_Payload, &Str_OB_INTAN);

    //DCUID
    *(Get_Meter_Info.Str_Payload.Data_a8 + Get_Meter_Info.Str_Payload.Length_u16++) = '(';
    Copy_String_2(&Get_Meter_Info.Str_Payload, &sDCU.sDCU_id); 
    *(Get_Meter_Info.Str_Payload.Data_a8 + Get_Meter_Info.Str_Payload.Length_u16++) = ')';
    
    //ID Meter
    *(Get_Meter_Info.Str_Payload.Data_a8 + Get_Meter_Info.Str_Payload.Length_u16++) = '(';
    Copy_String_2(&Get_Meter_Info.Str_Payload, &sDCU.sMeter_id_now);  
    *(Get_Meter_Info.Str_Payload.Data_a8 + Get_Meter_Info.Str_Payload.Length_u16++) = ')';

    //Meter type
    *(Get_Meter_Info.Str_Payload.Data_a8 + Get_Meter_Info.Str_Payload.Length_u16++) = '(';
    Copy_String_2(&Get_Meter_Info.Str_Payload, &Str_MeterType_u8[sDCU.MeterType]);  
    *(Get_Meter_Info.Str_Payload.Data_a8 + Get_Meter_Info.Str_Payload.Length_u16++) = ')';
    
    //Timlabel
    //numqty
    Get_Meter_Info.PosNumqty = Get_Meter_Info.Str_Payload.Length_u16;
    //Obis
    Get_Meter_Info.Pos_Obis_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
    
    *(Get_Meter_Info.Str_Payload.Data_a8 + Get_Meter_Info.Str_Payload.Length_u16++) = 0x0D;
    *(Get_Meter_Info.Str_Payload.Data_a8 + Get_Meter_Info.Str_Payload.Length_u16++) = 0x0A;
    //Data
    Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
    Get_Meter_Info.Numqty = 0;
}




void Write_Header_His_Push103 (void)
{
    //STX
    *(Get_Meter_Billing.Str_Payload.Data_a8 + Get_Meter_Billing.Str_Payload.Length_u16++) = STX;
    //obiscode Intan
    Copy_String_2(&Get_Meter_Billing.Str_Payload, &Str_OB_CHOT);

    //DCUID
    *(Get_Meter_Billing.Str_Payload.Data_a8 + Get_Meter_Billing.Str_Payload.Length_u16++) = '(';
    Copy_String_2(&Get_Meter_Billing.Str_Payload, &sDCU.sDCU_id); 
    *(Get_Meter_Billing.Str_Payload.Data_a8 + Get_Meter_Billing.Str_Payload.Length_u16++) = ')';
    
    //ID Meter
    *(Get_Meter_Billing.Str_Payload.Data_a8 + Get_Meter_Billing.Str_Payload.Length_u16++) = '(';
    Copy_String_2(&Get_Meter_Billing.Str_Payload, &sDCU.sMeter_id_now);  
    *(Get_Meter_Billing.Str_Payload.Data_a8 + Get_Meter_Billing.Str_Payload.Length_u16++) = ')';

    //Meter type
    *(Get_Meter_Billing.Str_Payload.Data_a8 + Get_Meter_Billing.Str_Payload.Length_u16++) = '(';
    Copy_String_2(&Get_Meter_Billing.Str_Payload, &Str_MeterType_u8[sDCU.MeterType]);  
    *(Get_Meter_Billing.Str_Payload.Data_a8 + Get_Meter_Billing.Str_Payload.Length_u16++) = ')';
    
    //Timlabel
    //numqty
    Get_Meter_Billing.PosNumqty = Get_Meter_Billing.Str_Payload.Length_u16;
    //Obis
    Get_Meter_Billing.Pos_Obis_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
    
    *(Get_Meter_Billing.Str_Payload.Data_a8 + Get_Meter_Billing.Str_Payload.Length_u16++) = 0x0D;
    *(Get_Meter_Billing.Str_Payload.Data_a8 + Get_Meter_Billing.Str_Payload.Length_u16++) = 0x0A;
    //Data
    Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
    Get_Meter_Billing.Numqty = 0;
}





void Pack_Header_lpf_Pushdata103 (void)
{
    //STX
    *(Get_Meter_LProf.Str_Payload.Data_a8 + Get_Meter_LProf.Str_Payload.Length_u16++) = STX;
    //obiscode Intan
    Copy_String_2(&Get_Meter_LProf.Str_Payload, &Str_OB_LPF);

    //DCUID
    *(Get_Meter_LProf.Str_Payload.Data_a8 + Get_Meter_LProf.Str_Payload.Length_u16++) = '(';
    Copy_String_2(&Get_Meter_LProf.Str_Payload, &sDCU.sDCU_id); 
    *(Get_Meter_LProf.Str_Payload.Data_a8 + Get_Meter_LProf.Str_Payload.Length_u16++) = ')';
    
    //ID Meter
    *(Get_Meter_LProf.Str_Payload.Data_a8 + Get_Meter_LProf.Str_Payload.Length_u16++) = '(';
    Copy_String_2(&Get_Meter_LProf.Str_Payload, &sDCU.sMeter_id_now);  
    *(Get_Meter_LProf.Str_Payload.Data_a8 + Get_Meter_LProf.Str_Payload.Length_u16++) = ')';

    //Meter type
    *(Get_Meter_LProf.Str_Payload.Data_a8 + Get_Meter_LProf.Str_Payload.Length_u16++) = '(';
    Copy_String_2(&Get_Meter_LProf.Str_Payload, &Str_MeterType_u8[sDCU.MeterType]);  
    *(Get_Meter_LProf.Str_Payload.Data_a8 + Get_Meter_LProf.Str_Payload.Length_u16++) = ')';
    
    //Timlabel
    //event
    //preiod
    //numqty
    Get_Meter_LProf.PosNumqty = Get_Meter_LProf.Str_Payload.Length_u16;
    //Obis
    Get_Meter_LProf.Pos_Obis_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
    
    *(Get_Meter_LProf.Str_Payload.Data_a8 + Get_Meter_LProf.Str_Payload.Length_u16++) = 0x0D;
    *(Get_Meter_LProf.Str_Payload.Data_a8 + Get_Meter_LProf.Str_Payload.Length_u16++) = 0x0A;
    //Data
    Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
    
    Get_Meter_LProf.Numqty = 0;
}



void Pack_PushData_103_Infor_Meter (void)
{ 
//    uint8_t Temp_BBC = 0;
//
//    Str_Infor_Meter.Data_a8 = &InfoMeterBuff[0];
//    Reset_Buff(&Str_Infor_Meter);
//
//    //STX
//    *(Str_Infor_Meter.Data_a8 + Str_Infor_Meter.Length_u16++) = STX;
//    //obiscode Intan
//    Copy_String_2(&Str_Infor_Meter, &Str_OB_INFOR_METER);
//
//    //DCUID
//    *(Str_Infor_Meter.Data_a8 + Str_Infor_Meter.Length_u16++) = '(';
//    Copy_String_2(&Str_Infor_Meter, &sDCU.sDCU_id);
//    *(Str_Infor_Meter.Data_a8 + Str_Infor_Meter.Length_u16++) = ')';
//
//    //ID Meter
//    *(Str_Infor_Meter.Data_a8 + Str_Infor_Meter.Length_u16++) = '(';
//    Copy_String_2(&Str_Infor_Meter, &sDCU.sMeter_id_now);
//    *(Str_Infor_Meter.Data_a8 + Str_Infor_Meter.Length_u16++) = ')';
//
//    //Meter type
//    *(Str_Infor_Meter.Data_a8 + Str_Infor_Meter.Length_u16++) = '(';
//    Copy_String_2(&Str_Infor_Meter, &Str_MeterType_u8[sDCU.MeterType]);
//    *(Str_Infor_Meter.Data_a8 + Str_Infor_Meter.Length_u16++) = ')';
//
//    //Timlabel
//    *(Str_Infor_Meter.Data_a8 + Str_Infor_Meter.Length_u16++) = '(';
//    Copy_String_STime(&Str_Infor_Meter,sRTC);
//    *(Str_Infor_Meter.Data_a8 + Str_Infor_Meter.Length_u16++) = ')';
//    //numqty
//    *(Str_Infor_Meter.Data_a8 + Str_Infor_Meter.Length_u16++) = '(';
//    *(Str_Infor_Meter.Data_a8 + Str_Infor_Meter.Length_u16++) = '6';
//    *(Str_Infor_Meter.Data_a8 + Str_Infor_Meter.Length_u16++) = ')';
//
//    Copy_String_2(&Str_Infor_Meter, &ModermIP);
//    Copy_String_2(&Str_Infor_Meter, &ModermFirmVer);
//    Copy_String_2(&Str_Infor_Meter, &ModermCSQ);
//    Copy_String_2(&Str_Infor_Meter, &Str_Ob_Tu);
//    Copy_String_2(&Str_Infor_Meter, &Str_Ob_Ti);
//    Copy_String_2(&Str_Infor_Meter, &He_So_Nhan);
//
//    *(Str_Infor_Meter.Data_a8 + Str_Infor_Meter.Length_u16++) = 0x0D;
//    *(Str_Infor_Meter.Data_a8 + Str_Infor_Meter.Length_u16++) = 0x0A;
//
//    *(Str_Infor_Meter.Data_a8 + Str_Infor_Meter.Length_u16++) = '(';
//    Copy_String_2(&Str_Infor_Meter, &sDCU.Str_IP_Module);
//    *(Str_Infor_Meter.Data_a8 + Str_Infor_Meter.Length_u16++) = ')';
//
//    *(Str_Infor_Meter.Data_a8 + Str_Infor_Meter.Length_u16++) = '(';
//    Copy_String_2(&Str_Infor_Meter, &sFirmware_Version);
//    *(Str_Infor_Meter.Data_a8 + Str_Infor_Meter.Length_u16++) = ')';
//
//
//    *(Str_Infor_Meter.Data_a8 + Str_Infor_Meter.Length_u16++) = '(';
//    *(Str_Infor_Meter.Data_a8 + Str_Infor_Meter.Length_u16++) = (sSim900_status.RSSI_c8>>4) + 0x30;;
//    *(Str_Infor_Meter.Data_a8 + Str_Infor_Meter.Length_u16++) = (sSim900_status.RSSI_c8&0x0F) + 0x30;;
//    *(Str_Infor_Meter.Data_a8 + Str_Infor_Meter.Length_u16++) = ')';
//
//    Copy_String_2(&Str_Infor_Meter, &Get_Meter_TuTi.Str_Payload);
//
//    //ETX
//    *(Str_Infor_Meter.Data_a8 + Str_Infor_Meter.Length_u16++) = ETX;
//    //BBC
//    Temp_BBC = BBC_Cacul(Str_Infor_Meter.Data_a8 + 1,Str_Infor_Meter.Length_u16 - 1);
//    *(Str_Infor_Meter.Data_a8 + Str_Infor_Meter.Length_u16++) = Temp_BBC;
//
//    //Push data to queue
//    Queue_Meter_AddInfor.Mess_Direct_ui8 = 1;
//    Queue_Meter_AddInfor.str_Flash.Length_u16 = Str_Infor_Meter.Length_u16;
//    Queue_Meter_AddInfor.Mess_Status_ui8 = 0;
//    Queue_Meter_AddInfor.Mess_Type_ui8 = DATA_METER_INFOR;
//    Queue_Meter_AddInfor.str_Flash.Data_a8 = &InfoMeterBuff[0];
//
//    xQueueSend(qMeter_SIM900Handle,(void *)&ptrQueue_Met_Addinfor, 1000);
    
//    _fPrint_Via_Debug(&UART_SERIAL, &InfoMeterBuff[0], Str_Infor_Meter.Length_u16, 1000);
//    _fPrint_Via_Debug(&UART_SERIAL,(uint8_t*) "\r\n", 2,1000);
}

void Check_Meter(void)
{
    if((eMeter_20._f_Check_Meter != NULL) && (sDCU.MeterType != METER_TYPE_UNKNOWN))
        eMeter_20._f_Check_Meter();
    else sDCU.Status_Meter_u8 = 0;	// khong co Dong ho
}
 

void Add_TuTI_toPayload (Meter_Comm_Struct* Struct_Get)
{
    //Them Tu TI
    Copy_String_toTaget(&Struct_Get->Str_Payload, Struct_Get->Pos_Obis_Inbuff, &Str_Ob_Tu);  
    Struct_Get->Pos_Obis_Inbuff += Str_Ob_Tu.Length_u16;
    Struct_Get->Numqty++;
    Copy_String_toTaget(&Struct_Get->Str_Payload, Struct_Get->Pos_Obis_Inbuff, &Str_Ob_Ti);  
    Struct_Get->Pos_Obis_Inbuff += Str_Ob_Ti.Length_u16;
    Struct_Get->Numqty++;
    Copy_String_toTaget(&Struct_Get->Str_Payload, Struct_Get->Pos_Obis_Inbuff, &He_So_Nhan);  
    Struct_Get->Pos_Obis_Inbuff += He_So_Nhan.Length_u16;
    Struct_Get->Numqty++;
    Struct_Get->Pos_Data_Inbuff = Struct_Get->Str_Payload.Length_u16;
    
    Copy_String_toTaget(&Struct_Get->Str_Payload, Struct_Get->Pos_Data_Inbuff, &Get_Meter_TuTi.Str_Payload); 
    Struct_Get->Pos_Data_Inbuff = Struct_Get->Str_Payload.Length_u16;
}

uint8_t Send_MessLpf_toQueue (void)
{
//    uint8_t         Temp_BBC = 0;
//    uint32_t	    temp=0;
//    uint16_t        i = 0;
//
//    sDCU.FlagHave_ProfMess = 1;
//    //ETX
//    *(Get_Meter_LProf.Str_Payload.Data_a8 + Get_Meter_LProf.Str_Payload.Length_u16++) = ETX;
//    //BBC
//    Temp_BBC = BBC_Cacul(Get_Meter_LProf.Str_Payload.Data_a8 + 1,Get_Meter_LProf.Str_Payload.Length_u16 - 1);
//    *(Get_Meter_LProf.Str_Payload.Data_a8 + Get_Meter_LProf.Str_Payload.Length_u16++) = Temp_BBC;
//
//    //Them length trong phan luu flash
//    for(i = 0; i < Get_Meter_LProf.Str_Payload.Length_u16; i++)
//        *(Get_Meter_LProf.Str_Payload.Data_a8 + Get_Meter_LProf.Str_Payload.Length_u16 + 1 - i) = *(Get_Meter_LProf.Str_Payload.Data_a8 + Get_Meter_LProf.Str_Payload.Length_u16 - i - 1);
//
//    Get_Meter_LProf.Str_Payload.Length_u16 += 2;
//    *(Get_Meter_LProf.Str_Payload.Data_a8)     = (uint8_t) ((Get_Meter_LProf.Str_Payload.Length_u16 + 1) >> 8) ;
//    *(Get_Meter_LProf.Str_Payload.Data_a8 + 1) = (uint8_t) (Get_Meter_LProf.Str_Payload.Length_u16 + 1);
//
//
//    //Generate checksum byte
//    for (i=0;i<Get_Meter_LProf.Str_Payload.Length_u16;i++)
//        temp += MeterLProfDataBuff[i];
//
//    temp = temp & 0x000000FF;
//    *(Get_Meter_LProf.Str_Payload.Data_a8 + Get_Meter_LProf.Str_Payload.Length_u16++) = (uint8_t)temp;
//
//    Queue_Meter_LProf.Mess_Direct_ui8 = 1;
//    Queue_Meter_LProf.str_Flash.Length_u16 = Get_Meter_LProf.Str_Payload.Length_u16;
//    Queue_Meter_LProf.Mess_Status_ui8 = 0;
//    Queue_Meter_LProf.Mess_Type_ui8 = DATA_LOAD_PROFILE;
//    Queue_Meter_LProf.str_Flash.Data_a8 = &MeterLProfDataBuff[0];
//
//    xQueueSend(qMeter_FlashHandle,(void *)&ptrQueue_Meter_LProf,1000);
//
////    _fPrint_Via_Debug(&UART_SERIAL, &MeterLProfDataBuff[0], Queue_Meter_LProf.str_Flash.Length_u16, 1000);
////    _fPrint_Via_Debug(&UART_SERIAL,(uint8_t*) "\r\n", 2,1000);
//    if(_fWaitSaveInFlash(&Queue_Meter_LProf.Mess_Status_ui8) == 0)
//      return 0;
//
    return 1;
}


void Push_TSVH_toQueue (uint8_t Type)
{
    uint32_t	temp=0;
    uint16_t    i = 0;
    uint16_t    Length = 0;
    
    //Push data to queue
    if(Get_Meter_Info.Str_Payload.Length_u16 > MAX_BYTE_IN_PACKET)
    {
        Length = Get_Meter_Info.Str_Payload.Length_u16 - MAX_BYTE_IN_PACKET; //chia thanh 2 ban tin MAX_BYTE_IN_PACKET byte va con lai
        //Dich ra sau 2 byte
        for(i = 0; i < Get_Meter_Info.Str_Payload.Length_u16; i++)
            *(Get_Meter_Info.Str_Payload.Data_a8 + Get_Meter_Info.Str_Payload.Length_u16 + 1 - i) = *(Get_Meter_Info.Str_Payload.Data_a8 + Get_Meter_Info.Str_Payload.Length_u16 - i - 1);
        
        Get_Meter_Info.Str_Payload.Length_u16 += 2;
        *(Get_Meter_Info.Str_Payload.Data_a8)     = (uint8_t) ((MAX_BYTE_IN_PACKET + 3) >> 8) ;
        *(Get_Meter_Info.Str_Payload.Data_a8 + 1) = (uint8_t) (MAX_BYTE_IN_PACKET + 3) ;
        
        //Generate checksum byte
        for (i = 0; i< (MAX_BYTE_IN_PACKET + 2); i++)
            temp += MeterInfoDataBuff[i];
        temp = temp & 0x000000FF;
        
        //Dich ra sau 3 byte tu vi tri 1202
        for(i = 0; i < (Get_Meter_Info.Str_Payload.Length_u16 - (MAX_BYTE_IN_PACKET + 2)); i++)
            *(Get_Meter_Info.Str_Payload.Data_a8 + Get_Meter_Info.Str_Payload.Length_u16 + 2 - i) = *(Get_Meter_Info.Str_Payload.Data_a8 + Get_Meter_Info.Str_Payload.Length_u16 - i - 1);
        
        Get_Meter_Info.Str_Payload.Length_u16 += 3;
        Length += 2;
        *(Get_Meter_Info.Str_Payload.Data_a8 + MAX_BYTE_IN_PACKET + 2) = (uint8_t)temp;                    //crc cua MAX_BYTE_IN_PACKET byte dau
        *(Get_Meter_Info.Str_Payload.Data_a8 + MAX_BYTE_IN_PACKET + 3) = (uint8_t) ((Length + 1) >> 8) ;  //length cua con lai
        *(Get_Meter_Info.Str_Payload.Data_a8 + MAX_BYTE_IN_PACKET + 4) = (uint8_t) (Length + 1) ;
        
        temp = 0;
        //Generate checksum byte
        for (i = (MAX_BYTE_IN_PACKET + 3); i<Get_Meter_Info.Str_Payload.Length_u16; i++)
            temp += MeterInfoDataBuff[i];
        temp = temp & 0x000000FF;
        
        *(Get_Meter_Info.Str_Payload.Data_a8 + Get_Meter_Info.Str_Payload.Length_u16++) = (uint8_t)temp;  //crc cua ban tin 2
        
        _fSendTSVH_ToQueue(Type, &MeterInfoDataBuff[0], MAX_BYTE_IN_PACKET + 3);
        _fSendTSVH_ToQueue(Type, &MeterInfoDataBuff[MAX_BYTE_IN_PACKET + 3], Length + 1);
    }else
    {
        //Dich ra sau 2 byte
        for(i = 0; i < Get_Meter_Info.Str_Payload.Length_u16; i++)
            *(Get_Meter_Info.Str_Payload.Data_a8 + Get_Meter_Info.Str_Payload.Length_u16 + 1 - i) = *(Get_Meter_Info.Str_Payload.Data_a8 + Get_Meter_Info.Str_Payload.Length_u16 - i - 1);
        
        Get_Meter_Info.Str_Payload.Length_u16 += 2;
        *(Get_Meter_Info.Str_Payload.Data_a8)     = (uint8_t) ((Get_Meter_Info.Str_Payload.Length_u16 + 1) >> 8) ;
        *(Get_Meter_Info.Str_Payload.Data_a8 + 1) = (uint8_t) (Get_Meter_Info.Str_Payload.Length_u16 + 1) ;
        
        //Generate checksum byte
        for (i=0;i<Get_Meter_Info.Str_Payload.Length_u16;i++)
            temp += MeterInfoDataBuff[i];
        temp = temp & 0x000000FF;
        
        *(Get_Meter_Info.Str_Payload.Data_a8 + Get_Meter_Info.Str_Payload.Length_u16++) = (uint8_t)temp;  
      
        _fSendTSVH_ToQueue(Type, Get_Meter_Info.Str_Payload.Data_a8, Get_Meter_Info.Str_Payload.Length_u16);
    }    
}

uint8_t _fSendTSVH_ToQueue(uint8_t Type, uint8_t *aData, uint16_t length)
{
//    Queue_Meter_Info.Mess_Direct_ui8 = 1;
//    Queue_Meter_Info.Mess_Status_ui8 = 0;
//    Queue_Meter_Info.Mess_Type_ui8 = Type;
//    if(Type == DATA_INTANTANIOUS)
//    {
//        Queue_Meter_Info.str_Flash.Length_u16 = length - 3;
//        Queue_Meter_Info.str_Flash.Data_a8 = aData + 2;
//        xQueueSend(qMeter_SIM900Handle,(void *)&ptrQueue_Meter_Info, 1000);
//    }else
//    {
//        Queue_Meter_Info.str_Flash.Length_u16 = length;
//        Queue_Meter_Info.str_Flash.Data_a8 = aData;
//        xQueueSend(qMeter_FlashHandle,(void *)&ptrQueue_Meter_Info, 1000);
//    }
//        _fPrint_Via_Debug(&UART_SERIAL, aData, length, 1000);
//        _fPrint_Via_Debug(&UART_SERIAL,(uint8_t*) "\r\n", 2,1000);
//    return _fWaitSaveInFlash(&Queue_Meter_Info.Mess_Status_ui8);
	return 1;
}



void Push_Bill_toQueue (uint8_t Type)   
{
    uint32_t	temp=0;
    uint16_t    i = 0;
    uint16_t    Length = 0;

    sDCU.FlagHave_BillMes = 1;
    //Push data to queue
    if(Get_Meter_Billing.Str_Payload.Length_u16 > MAX_BYTE_IN_PACKET)
    {
        Length = Get_Meter_Billing.Str_Payload.Length_u16 - MAX_BYTE_IN_PACKET; //chia thanh 2 ban tin 1000 byte va con lai
        //Dich ra sau 2 byte
        for(i = 0; i < Get_Meter_Billing.Str_Payload.Length_u16; i++)
            *(Get_Meter_Billing.Str_Payload.Data_a8 + Get_Meter_Billing.Str_Payload.Length_u16 + 1 - i) = *(Get_Meter_Billing.Str_Payload.Data_a8 + Get_Meter_Billing.Str_Payload.Length_u16 - i - 1);
        
        Get_Meter_Billing.Str_Payload.Length_u16 += 2;
        *(Get_Meter_Billing.Str_Payload.Data_a8)     = (uint8_t) ((MAX_BYTE_IN_PACKET + 3) >> 8) ;
        *(Get_Meter_Billing.Str_Payload.Data_a8 + 1) = (uint8_t) (MAX_BYTE_IN_PACKET + 3) ;
        
        //Generate checksum byte
        for (i = 0; i < (MAX_BYTE_IN_PACKET + 2); i++)
            temp += MeterBillingDataBuff[i];
        temp = temp & 0x000000FF;
        
        //Dich ra sau 3 byte tu vi tri 1002
        for(i = 0; i < (Get_Meter_Billing.Str_Payload.Length_u16 - (MAX_BYTE_IN_PACKET + 2)); i++)
            *(Get_Meter_Billing.Str_Payload.Data_a8 + Get_Meter_Billing.Str_Payload.Length_u16 + 2 - i) = *(Get_Meter_Billing.Str_Payload.Data_a8 + Get_Meter_Billing.Str_Payload.Length_u16 - i - 1);
        
        Get_Meter_Billing.Str_Payload.Length_u16 += 3;
        Length += 2;
        *(Get_Meter_Billing.Str_Payload.Data_a8 + MAX_BYTE_IN_PACKET + 2) = (uint8_t)temp;   //crc cua 1000 byte dau
        *(Get_Meter_Billing.Str_Payload.Data_a8 + MAX_BYTE_IN_PACKET + 3) = (uint8_t) ((Length + 1) >> 8) ;  //length cua con lai
        *(Get_Meter_Billing.Str_Payload.Data_a8 + MAX_BYTE_IN_PACKET + 4) = (uint8_t) (Length + 1) ;
        temp = 0;
        //Generate checksum byte
        for (i = (MAX_BYTE_IN_PACKET + 3); i < Get_Meter_Billing.Str_Payload.Length_u16; i++)
            temp += MeterBillingDataBuff[i];
        temp = temp & 0x000000FF;
        
        *(Get_Meter_Billing.Str_Payload.Data_a8 + Get_Meter_Billing.Str_Payload.Length_u16++) = (uint8_t)temp;  //crc cua ban tin 2
        
        _fSendBill_ToQueue(Type, &MeterBillingDataBuff[0], MAX_BYTE_IN_PACKET + 3);
        _fSendBill_ToQueue(Type, &MeterBillingDataBuff[MAX_BYTE_IN_PACKET + 3], Length + 1);
    }else
    {
        //Dich ra sau 2 byte
        for(i = 0; i < Get_Meter_Billing.Str_Payload.Length_u16; i++)
            *(Get_Meter_Billing.Str_Payload.Data_a8 + Get_Meter_Billing.Str_Payload.Length_u16 + 1 - i) = *(Get_Meter_Billing.Str_Payload.Data_a8 + Get_Meter_Billing.Str_Payload.Length_u16 - i - 1);
        
        Get_Meter_Billing.Str_Payload.Length_u16 += 2;
        *(Get_Meter_Billing.Str_Payload.Data_a8)     = (uint8_t) ((Get_Meter_Billing.Str_Payload.Length_u16 + 1) >> 8) ;
        *(Get_Meter_Billing.Str_Payload.Data_a8 + 1) = (uint8_t) (Get_Meter_Billing.Str_Payload.Length_u16 + 1) ;
        
        //Generate checksum byte
        for (i=0;i<Get_Meter_Billing.Str_Payload.Length_u16;i++)
            temp += MeterBillingDataBuff[i];
        temp = temp & 0x000000FF;
        
        *(Get_Meter_Billing.Str_Payload.Data_a8 + Get_Meter_Billing.Str_Payload.Length_u16++) = (uint8_t)temp;  
        
        _fSendBill_ToQueue(Type, Get_Meter_Billing.Str_Payload.Data_a8, Get_Meter_Billing.Str_Payload.Length_u16);
    }    
}


uint8_t _fSendBill_ToQueue(uint8_t Type, uint8_t *aData, uint16_t length)
{
//    Queue_Meter_Billing.Mess_Direct_ui8 = 1;
//    Queue_Meter_Billing.Mess_Status_ui8 = 0;
//    Queue_Meter_Billing.Mess_Type_ui8 = Type;
//    Queue_Meter_Billing.str_Flash.Length_u16 = length;
//    Queue_Meter_Billing.str_Flash.Data_a8 = aData;
//    xQueueSend(qMeter_FlashHandle,(void *)&ptrQueue_Meter_Billing,1000);

//    _fPrint_Via_Debug(&UART_SERIAL, aData, length, 1000);
//    _fPrint_Via_Debug(&UART_SERIAL,(uint8_t*) "\r\n", 2,1000);
    
    return _fWaitSaveInFlash(&Queue_Meter_Billing.Mess_Status_ui8);
}

uint8_t Push_Even_toQueue (uint8_t Type)  
{
//    Queue_Meter_Event.Mess_Direct_ui8 = 1;
//    Queue_Meter_Event.Mess_Status_ui8 = 0;
//    Queue_Meter_Event.Mess_Type_ui8 = DATA_EVEN_METER;
//    Queue_Meter_Event.str_Flash.Length_u16 = Get_Meter_Event.Str_Payload.Length_u16;
//    Queue_Meter_Event.str_Flash.Data_a8 = &MeterEventDataBuff[0];
//
//    xQueueSend(qMeter_SIM900Handle,(void *)&ptrQueue_Meter_Event,1000);
//
//    _fPrint_Via_Debug(&UART_SERIAL, &MeterEventDataBuff[0], Get_Meter_Event.Str_Payload.Length_u16, 5000);
//
//    if (osSemaphoreWait(bsMLProfMessSentHandle,20000) != osOK)
//        return 0;
//    else return 1;
}


void Header_event_103 (truct_String* Payload,uint8_t type)  
{    
    //STX
    *(Payload->Data_a8 + Payload->Length_u16++) = STX;
    //obiscode Intan
    Copy_String_2(Payload, &Str_OB_EVENT);
        
    //DCUID
    *(Payload->Data_a8 + Payload->Length_u16++) = '(';
    Copy_String_2(Payload, &sDCU.sDCU_id); 
    *(Payload->Data_a8 + Payload->Length_u16++) = ')';
    
    //ID Meter
    *(Payload->Data_a8 + Payload->Length_u16++) = '(';
    Copy_String_2(Payload, &sDCU.sMeter_id_now); 
    *(Payload->Data_a8 + Payload->Length_u16++) = ')';
    
    //Meter type
    *(Payload->Data_a8 + Payload->Length_u16++) = '(';
    Copy_String_2(Payload, &Str_MeterType_u8[sDCU.MeterType]);  
    *(Payload->Data_a8 + Payload->Length_u16++) = ')';
    
    //Timlabel
    *(Payload->Data_a8 + Payload->Length_u16++) = '('; 
    if(type == 1)  
        Copy_String_STime(Payload, sRTC);
    *(Payload->Data_a8 + Payload->Length_u16++) = ')';
    
    Get_Meter_Event.PosNumqty = Payload->Length_u16;
    Get_Meter_Event.Pos_Obis_Inbuff = Payload->Length_u16;
    
    *(Payload->Data_a8 + Payload->Length_u16++) = 0x0D; 
    *(Payload->Data_a8 + Payload->Length_u16++) = 0x0A;
    Get_Meter_Event.Pos_Data_Inbuff = Payload->Length_u16;
    Get_Meter_Event.Numqty = 0;
}




void _fSend_Empty_Lpf (void)
{
    truct_String    Str_Data_Write={&Buff_Temp1[0], 0};
    
    //dong goi tin rong gui len
    Init_Meter_LProf_Struct(); 
    //wrire header TSVH vao
    Pack_Header_lpf_Pushdata103();
    
    Add_TuTI_toPayload(&Get_Meter_LProf);
    Reset_Buff(&Str_Data_Write);
    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')'; 
    //event
    Copy_String_2(&Str_Data_Write, &Str_event_Temp);
    //period
    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
    Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t)(PeriodLpf_Min) ,0);
    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')'; 
    //num chanel
    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
    Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) Get_Meter_LProf.Numqty, 0);
    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')'; 
    
    Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.PosNumqty, &Str_Data_Write);
    
    Send_MessLpf_toQueue();
}




