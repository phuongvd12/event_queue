/*
 * Init_Meter.c
 *
 *  Created on: Dec 15, 2015
 *      Author: Administrator
 */

#include "variable.h"
#include "GENIUS_Init_Meter.h"
#include "GENIUS_Read_Meter.h"
#include "t_mqtt.h"
#include "sim900.h"
#include "pushdata.h"

//=======================================Constant & Template===========================================

const uint32_t   Genius_IDRegisIntan_Table[50] = 
{
    0x0000E000,       //InstantVoltagePhaseA		//0
	0x0000E001,       //InstantVoltagePhaseB		
	0x0000E002,       //InstantVoltagePhaseC		
	0x0000E010,       //InstantCurrentPhaseA 		
	0x0000E011,       //InstantCurrentPhaseB		
	0x0000E012,       //InstantCurrentPhaseC	    //5

	0x0000E030,       //InstantPowerPhaseA
	0x0000E031,       //InstantPowerPhaseB
	0x0000E032,       //InstantPowerPhaseC	
    0x0000E033,       //InstantPowerTotal             //9
    
    0x0000E040,       //ReactivePowerPhaseA
	0x0000E041,       //ReactivePowerPhaseB
	0x0000E042,       //IReactivePowerPhaseC	
    0x0000E043,       //ReactivePowerTotal              //13

    0x0000E060,       //Freq 
    0x0000E026,       //powfactor                       //4byte  dang o: float 4 byte.  //15
    
    0x00000069,         //Chanel 1 total. ImportWh total         //dang 8 byte dang double 8 byte
    0x00000060,         //rate 1
    0x00000061,
    0x00000062,                                             //19
    
    0x00000169,         //Chanel 2 total. EXportWh total
    0x00000160,         //rate 1
    0x00000161,
    0x00000162,                                                 //23
    
    0x00000269,         //Chanel 3 total. ImportVar total
    0x00000369,         //Chanel 4 total. EXportVar total      //25
    //maxdemand
    0x00001100,         //value                             //float 4 byte
    0x00008100,         //time
    
    0x00001101,                                             //time 6 byte
    0x00008101,
    
    0x00001102,
    0x00008102,                                             
    
    0x0000F700,       //Ti_1         //32                                                     
	0x0000F702,       //Ti_2
	0x0000F701,       //Tu_1
	0x0000F703,       //Tu_2                              dang  F: float 4 byte    //35
    0x0000F03D,         //36
};      


struct_Obis_Genius       Obis_Genius[MAX_OBIS_GENIUS]=          
{
    {0x02,            &Unit_Voltage,             {NULL},              &Str_Ob_VolA },         //vol a             2
    {0x03,            &Unit_Voltage,             {NULL},              &Str_Ob_VolB },         //vol b             3
    {0x04,            &Unit_Voltage,             {NULL},              &Str_Ob_VolC },          //vol c             4
    {0x05,            &Unit_Current,             {NULL},              &Str_Ob_CurA },         //current a        5
    {0x06,            &Unit_Current,             {NULL},              &Str_Ob_CurB },         //current b        6
    {0x07,            &Unit_Current,             {NULL},              &Str_Ob_CurC },         //current c        7

    
    {0x1F,            &Unit_Active_EnTotal,          &Str_Ob_AcPlus_Rate1_chot,          &Str_Ob_AcPlus_Rate1},          //nang luong plus bieu 1 1F
    {0x20,            &Unit_Active_EnTotal,          &Str_Ob_AcPlus_Rate2_chot,          &Str_Ob_AcPlus_Rate2},          //nang luong plus bieu 2 20
    {0x21,            &Unit_Active_EnTotal,          &Str_Ob_AcPlus_Rate3_chot,          &Str_Ob_AcPlus_Rate3},          //nang luong plus bieu 3 21

    {0x23,            &Unit_Active_EnTotal,          &Str_Ob_AcSub_Rate1_chot,           &Str_Ob_AcSub_Rate1},         //nang luong Sub bieu 1 23
    {0x24,            &Unit_Active_EnTotal,          &Str_Ob_AcSub_Rate2_chot,           &Str_Ob_AcSub_Rate2},        //nang luong Sub bieu 2 24
    {0x25,            &Unit_Active_EnTotal,          &Str_Ob_AcSub_Rate3_chot,           &Str_Ob_AcSub_Rate3},         //nang luong Sub bieu 3 25

    {0x27,            &Unit_Active_EnTotal,          &Str_Ob_AcImTotal_Chot,             &Str_Ob_En_ImportWh},    //Total Plus Wh      27
    {0x28,            &Unit_Active_EnTotal,          &Str_Ob_AcExTotal_Chot,             &Str_Ob_En_ExportWh},    //Total Sub Wh      28
    {0x29,            &Unit_Reactive_EnTotal,        &Str_Ob_ReImTotal_Chot,             &Str_Ob_En_ImportVar},    //Q1        29
    {0x2B,            &Unit_Reactive_EnTotal,        &Str_Ob_ReExTotal_Chot,             &Str_Ob_En_ExportVar},        //Q3        2B

    {0x31,            &Unit_MAXDEMAND,               &Str_Ob_MaxDeRate1_Chot,            {NULL}},                   //Maxdemand plus value bieu 1 31
    {0x33,            &Unit_MAXDEMAND,               &Str_Ob_MaxDe2Rate1_Chot,           {NULL}},                   //Maxdemand sub value bieu 1 33
   
    {0x35,            &Unit_MAXDEMAND,               &Str_Ob_MaxDeRate2_Chot,            {NULL}},          //Maxdemand plus value bieu 2 35
    {0x37,            &Unit_MAXDEMAND,               &Str_Ob_MaxDe2Rate2_Chot,           {NULL}},          //Maxdemand sub value bieu 2 37
   
    {0x39,            &Unit_MAXDEMAND,               &Str_Ob_MaxDeRate3_Chot,            {NULL}},           //Maxdemand plus value bieu 3 39
    {0x3B,            &Unit_MAXDEMAND,               &Str_Ob_MaxDe2Rate3_Chot,           {NULL}},          //Maxdemand sub value bieu 3 3B
 };



const uint32_t   Genius_IDRegisEvent[4] = 
{
    0x0000F092,   //num power up
    0x0000F093,    //last time power up
    0x0000FC20,    //time of last power loss
};

truct_String  StrobisEvent[3] =
{
    {(uint8_t*)"(C.51.13)",9},      //count pow up
    {(uint8_t*)"(C.51.14)",9},     // time power on
    {(uint8_t*)"(C.7.10)",8},      // time Power off 
};




