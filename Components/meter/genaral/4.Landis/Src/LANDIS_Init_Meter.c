/*
 * Init_Meter.c
 *
 *  Created on: Dec 15, 2015
 *      Author: Administrator
 */

#include "variable.h"
#include "LANDIS_Init_Meter.h"
#include "LANDIS_Read_Meter.h"
#include "t_mqtt.h"
#include "pushdata.h"

/*======================== Init Variables ======================*/
Meter_Comm_Struct	    Get_Meter_ShortName;  

uint8_t 	ID_Frame_ui8					= 0x32;
uint16_t    OBIS_Order = 0;

// ======================== Array ==============================
uint8_t LANDIS_SNRM[34] = {0x7E,0xA0,0x20,0x03,0x21,0x93,0x7D,0xD9,0x81,0x80,0x14,0x05,0x02,0x07,0xD0,0x06,0x02,0x07,0xD0,0x07,
						  0x04,0x00,0x00,0x00,0x01,0x08,0x04,0x00,0x00,0x00,0x01,0x3A,0xF2,0x7E};
uint8_t LANDIS_AARQ[48] = {0x7E,0xA0,0x2E,0x03,0x21,0x10,0xAC,0xC1,0xE6,0xE6,0x00,0x60,0x20,0x80,0x02,0x07,0x80,0xA1,0x09,0x06,
						  0x07,0x60,0x85,0x74,0x05,0x08,0x01,0x02,0xBE,0x0F,0x04,0x0D,0x01,0x00,0x00,0x00,0x06,0x5F,0x04,0x00,
						  0x18,0x1A,0x20,0x00,0x00,0x7B,0xD0,0x7E};

uint8_t LANDIS_GET_SHORTNAME[19] = {0x7E,0xA0,0x11,0x03,0x21,0x32,0xB7,0x3D,0xE6,0xE6,0x00,0x05,0x01,0x02,0xFA,0x08,0x18,0x9E,0x7E};
	
uint8_t	LANDIS_NextFrame[9] = {0x7E,0xA0,0x07,0x03,0x21,0x00,0x00,0x00,0x7E};
uint8_t	LANDIS_GetSN_Code_Table[300] = {0};
uint8_t	LANDIS_GetScale_Code_Table[57] = {1 ,2 ,3 , 4 ,5 ,6 ,7 ,8 ,9 ,10 ,11 ,12 ,13 ,14 ,15 ,16 ,17 ,18 ,19 ,20 ,21 ,22 ,23 ,24 ,25 ,26 ,27 ,28 ,29 ,30 ,31 ,32 ,33 ,34 ,35 ,36 ,37 ,38 ,39 ,40 ,41 ,42 ,43 ,44 ,45 ,46 ,47 ,48 ,49 ,50 , 51, 52 ,53 ,54,55,0xFF, 56};
// 7E A0 11 03 21 @@ Y1 Y1  E6 E6 00 05 01 02 XX XX Y2 Y2 7E 
uint8_t	LANDIS_MInfo_MTemp[19] = {0x7E,0xA0,0x11,0x03,0x21		//Byte0-4		4 byte Fix-don't care 1Byte: Addr HN-0x25	SG-0x21
								,0x00							//Byte5			ID Frame
								,0x00,0x00						//Byte6-7		Header check sum (calculate 1-5)
								,0xE6,0xE6,0x00,0x05,0x01,0x02	//Byte8-13		Fix-don't care
								,0x00,0x00						//Byte14-15		Short Name
								,0x00,0x00,0x7E};				//Byte16-18		Total check sum (calculate 1-15)
uint8_t	LANDIS_MeterInfoMessIDTable[56] = {3 ,4 ,5 ,6 ,7 ,8 ,9 ,10 ,11 ,12 ,13 ,14 ,15 ,16 ,17 ,18 ,19 
									,20 ,21 ,22 ,23 ,24 ,25 ,26 ,31 ,27 ,32, 28 ,33, 29 ,34,30 ,35 ,55, 0xFF};  // 1, 2 ,    55: bo tu ti
uint8_t LANDIS_MeterTuTiMessIDTable[3] = {2, 1,0xFF};  //Tu tru?c Ti sau
uint8_t	LANDIS_MEvent_Code_Table[7] = {0, 1, 2, 3, 4, 5, 0xFF};
uint8_t	LANDIS_MHis_Code_Obis_Table[20] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 0xFF};
	
uint8_t	LANDIS_MHis_Code_Data_Table[20] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 0xFF};
	

//  Array get SN
uint8_t LANDIS_Info_SN_Table[56][3] = {0};
uint8_t LANDIS_Event_SN_Table[6][3] = {0};
uint8_t	LANDIS_MAlert_Scale[15];
//
uint8_t	LANDIS_MBilling_Mess_OBIS[2] = {0x64,0x00};  // sau nay phai tu lay tu collecting object
uint8_t	LANDIS_MBilling_OBIS_List[128];  // Bang chua cac obis cua ban tin His
// Dung cho ban tin His va Load
uint8_t	LANDIS_Get_DATA_Template[70] = {0x7E,0xA0,0x44,0x03,0x21,0x32,0xF6,0x86,0xE6,0xE6,0x00,0x05,0x01,0x04,
									  0x00,0x00,0x01,0x02,0x04,0x02,0x04,0x12,0x00,0x08,0x09,0x06,0x00,0x00,
									  0x01,0x00,0x00,0xFF,0x0F,0x02,0x12,0x00,0x00,0x09,0x0C,0x07,0xDD,0x09,0x11,
									  0xFF,0x00,0x00,0x00,0x00,0x80,0x00,0xFF,0x09,0x0C,0x07,0xDD,0x09,0x11,0xFF,
                                      0x17,0x3B,0x00,0x00,0x80,0x00,0xFF,0x01,0x00,0x00,0x00,0x7E};
// Time read Billing
uint8_t	LANDIS_MBilling_StartTime[8] = {0x07,0xDF,0x0C,0x01,0xFF,0x00,0x00,0x00};
uint8_t	LANDIS_MBilling_StopTime[8] = {0x07,0xE0,0x01,0x01,0xFF,0x00,0x00,0x00};									  

uint8_t	LANDIS_MLprofile_OBIS_List[2] = {0x62,0x70};  // sau nay phai tu lay tu collecting object
uint8_t	LANDIS_MLprofile_Code_Obis_Table[20] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 0xFF};									  
uint8_t	LANDIS_MLprofile_Code_Data_Table[200] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
												20 ,21 ,22 ,23 ,24 ,25 ,26 ,27 ,28 ,29 ,30 ,31 ,32 ,33 ,34 ,35 ,36 ,37 ,38 ,0xFF};									  
uint8_t LANDIS_MLProfile_OBIS_List[128];


uint8_t LANDIS_Profile_Historical_SN_Table[2][3] = {0};

/*
7E A0 44 03 21 @@ Y1 Y1 E6 E6 00 05 01 04 XX XX  01 02 04 02 04 12 00 08 09 06 00 00 01 00 00 FF 0F 02 12 00 00 09 0C 07 DD 09 11 FF 00 00 00 00 80 00 FF 09 0C 07 DD 09 11 FF 17 3B 00 00 80 00 FF 01 00 Y2 Y2 7E
*/

uint8_t	LANDIS_Pro_MTemp[70] = {0x7E,0xA0,0x44,0x03,0x21		//Byte0-4		4 byte Fix-don't care 1Byte: Addr HN-0x25	SG-0x21
								,0x32							//Byte5			ID Frame
								,0x00,0x00						//Byte6-7		Header check sum (calculate 1-5)
								,0xE6,0xE6,0x00,0x05,0x01,0x04	//Byte8-13		Fix-don't care
								,0x00,0x00						//Byte14-15		Short Name
								,0x01,0x02,0x04,0x02,0x04,0x12,0x00,0x08,0x09,0x06,0x00,0x00,0x01,0x00,0x00,0xFF,0x0F,0x02,0x12,0x00,0x00,0x09,0x0C  //fix
								,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00 //start time
								,0x00,0x80,0x00,0xFF,0x09,0x0C  //fix
								,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00 //end time
								,0x00,0x80,0x00,0xFF,0x01,0x00	//fix
								,0x00,0x00,0x7E};				//Byte24-26		Total check sum (calculate 1-15)

uint8_t	LANDIS_MLProfile_StartTime[8] = {0x07,0xE0,0x01,0x09,0xFF,0x00,0x00,0x00};
uint8_t	LANDIS_MLProfile_StopTime[8] = {0x07,0xE0,0x01,0x09,0xFF,0x17,0x3B,0x00};
uint8_t LANDIS_OBIS_Order[27] = {0};  // 27???

uint8_t	LANDIS_Read_END[9] = {0x7E,0xA0,0x07,0x03,0x21,0x53,0x03,0xC7,0x7E};

uint8_t 	LANDIS_OBIS_Table[56][7] = {
//  -----------OBIS Code----------|OBIS|
//     0    1    2    3    4    5    6  
	{0x00,0x00,0x2A,0x00,0x00,0xFF,0x00},        //ID meter									
	{0x01,0x01,0x00,0x04,0x02,0xFF,0x41},       //Ti										
	{0x01,0x01,0x00,0x04,0x03,0xFF,0x42},       //Tu										
	{0x01,0x01,0x01,0x08,0x00,0xFF,0x27},       //ImportWh(EnergyPlusATotal)		//3								
	{0x01,0x01,0x01,0x08,0x01,0xFF,0x1F},       //EnergyPlusArate1										
	{0x01,0x01,0x01,0x08,0x02,0xFF,0x20},       //EnergyPlusArate2										
	{0x01,0x01,0x01,0x08,0x03,0xFF,0x21},       //EnergyPlusArate3										
	{0x01,0x01,0x02,0x08,0x00,0xFF,0x28},       //ExportWh(EnergySubATotal)										
	{0x01,0x01,0x02,0x08,0x01,0xFF,0x23},       //EnergySubArate1										
	{0x01,0x01,0x02,0x08,0x02,0xFF,0x24},       //EnergySubArate2										
	{0x01,0x01,0x02,0x08,0x03,0xFF,0x25},       //EnergySubArate3				//10						
	{0x01,0x01,0x03,0x08,0x00,0xFF,0x29},       //Q1(ReactiveenergyPlusATotal)										
	{0x01,0x01,0x04,0x08,0x00,0xFF,0x2B},       //Q3(ReactiveenergySubATotal)										
	{0x01,0x01,0x0D,0x07,0x00,0xFF,0x1B},       //PowerFactorPhaseTotal										
	{0x01,0x01,0x21,0x07,0x00,0xFF,0x1C},       //PowerFactorPhaseA										
	{0x01,0x01,0x35,0x07,0x00,0xFF,0x1D},       //PowerFactorPhaseB										
	{0x01,0x01,0x49,0x07,0x00,0xFF,0x1E},       //PowerFactorPhaseC										
	{0x01,0x01,0x0E,0x07,0x00,0xFF,0x09},       //Frequency										
	{0x01,0x01,0x20,0x07,0x00,0xFF,0x02},       //VoltagePhaseA										
	{0x01,0x01,0x34,0x07,0x00,0xFF,0x03},       //VoltagePhaseB										
	{0x01,0x01,0x48,0x07,0x00,0xFF,0x04},       //VoltagePhaseC			            //20							
	{0x01,0x01,0x1F,0x07,0x00,0xFF,0x05},       //CurrentPhaseA										
	{0x01,0x01,0x33,0x07,0x00,0xFF,0x06},       //CurrentPhaseB										
	{0x01,0x01,0x47,0x07,0x00,0xFF,0x07},       //CurrentPhaseC										
	{0x01,0x01,0x83,0x07,0x00,0xFF,0x16},       //ReactivePowerPhaseTotal										
	{0x01,0x01,0x10,0x07,0x00,0xFF,0x12},       //ActivePowerPhaseTotal										
	{0x01,0x01,0x01,0x06,0x00,0xFF,0x2E},       //MaxDemandPlusArate		        //26								
	{0x01,0x01,0x01,0x06,0x01,0xFF,0x31},       //MaxDemandPlusArate1										
	{0x01,0x01,0x01,0x06,0x02,0xFF,0x35},       //MaxDemandPlusArate2										
	{0x01,0x01,0x01,0x06,0x03,0xFF,0x39},       //MaxDemandPlusArate3										
	{0x01,0x01,0x01,0x06,0x04,0xFF,0x3D},       //MaxDemandPlusArate4	            //30
	{0x01,0x01,0x01,0x06,0x00,0xFF,0x2F},       //MaxDemandPlusArate_Time										
	{0x01,0x01,0x01,0x06,0x01,0xFF,0x32},       //MaxDemandPlusArate1_Time										
	{0x01,0x01,0x01,0x06,0x02,0xFF,0x36},       //MaxDemandPlusArate2_Time										
	{0x01,0x01,0x01,0x06,0x03,0xFF,0x3A},       //MaxDemandPlusArate3_Time										
	{0x01,0x01,0x01,0x06,0x04,0xFF,0x3E},       //MaxDemandPlusArate4_Time          //35
	{0x01,0x01,0x02,0x06,0x00,0xFF,0x30},       //MaxDemandSubArate										
	{0x01,0x01,0x02,0x06,0x01,0xFF,0x33},       //MaxDemandSubArate1										
	{0x01,0x01,0x02,0x06,0x02,0xFF,0x37},       //MaxDemandSubArate2										
	{0x01,0x01,0x02,0x06,0x03,0xFF,0x3B},       //MaxDemandSubArate3										
	{0x01,0x01,0x02,0x06,0x04,0xFF,0x3F},       //MaxDemandSubArate4				//40	    
	{0x01,0x01,0x09,0x08,0x00,0xFF,0x2D},       //Energy +VA										
	{0x01,0x01,0x51,0x07,0x04,0xFF,0x0C},       //Angle I(L1) to U(L1)										
	{0x01,0x01,0x51,0x07,0x05,0xFF,0x0D},       //Angle I(L2) to U(L1)										
	{0x01,0x01,0x51,0x07,0x06,0xFF,0x0E},       //Angle I(L3) to U(L1)										
	{0x01,0x01,0x01,0x02,0x00,0xFF,0x66},       //Cumulative maximum demand +A										
	{0x01,0x01,0x01,0x02,0x01,0xFF,0x67},       //Cumulative maximum demand +A rate 1										
	{0x01,0x01,0x01,0x02,0x02,0xFF,0x68},       //Cumulative maximum demand +A rate 2										
	{0x01,0x01,0x01,0x02,0x03,0xFF,0x69},       //Cumulative maximum demand +A rate 3										
	{0x01,0x01,0x01,0x02,0x04,0xFF,0x6A},       //Cumulative maximum demand +A rate 4										
	{0x01,0x01,0x02,0x02,0x00,0xFF,0x6B},       //Cumulative maximum demand -A		        //50								
	{0x01,0x01,0x02,0x02,0x01,0xFF,0x6C},       //Cumulative maximum demand -A rate 1										
	{0x01,0x01,0x02,0x02,0x02,0xFF,0x6D},       //Cumulative maximum demand -A rate 2										
	{0x01,0x01,0x02,0x02,0x03,0xFF,0x6E},       //Cumulative maximum demand -A rate 3										
	{0x01,0x01,0x02,0x02,0x04,0xFF,0x6F},        //Cumulative maximum demand -A rate 4
	{0x00,0x00,0x01,0x00,0x00,0xFF,0x01}         //MeterTime	                            //55
};
// Obis cua ban tin event LANDIS_MInfo_Table

uint8_t 	LANDIS_MEvent_Table[6][7] = {
//  -----------OBIS Code----------|OBIS|
//     0    1    2    3    4    5    6	
	{0x00,0x00,0x60,0x02,0x00,0xFF,0x17},       //Number of configuration program changes
	{0x00,0x00,0x60,0x07,0x01,0xFF,0x01},       //Number of power failures objects in phase L1
	{0x00,0x00,0x60,0x07,0x02,0xFF,0x02},       //Number of power failures objects in phase L2
	{0x00,0x00,0x60,0x07,0x03,0xFF,0x03},       //Number of power failures objects in phase L3
	{0x00,0x00,0x60,0x07,0x00,0xFF,0x04},       //Number of power failures objects in all three phases
	{0x00,0x00,0x60,0x02,0x01,0xFF,0x18}        //Date of last configuration program change
};



struct_Obis_Scale       Obis_Landis_Ins[MAX_OBIS_INTAN]=          
{
    {0x02,    &Str_Ob_VolA,           LANDIS_SCALE_VOLTAGE,             &Unit_Voltage,       0,      {NULL},              &Str_Ob_VolA },         //vol a             2
    {0x03,    &Str_Ob_VolB,           LANDIS_SCALE_VOLTAGE,             &Unit_Voltage,       0,      {NULL},              &Str_Ob_VolB },         //vol b             3
    {0x04,    &Str_Ob_VolC,           LANDIS_SCALE_VOLTAGE,             &Unit_Voltage,       0,      {NULL},              &Str_Ob_VolC },          //vol c             4
    {0x05,    &Str_Ob_CurA,           LANDIS_SCALE_CURRENT,             &Unit_Current,       0,      {NULL},              &Str_Ob_CurA },         //current a        5
    {0x06,    &Str_Ob_CurB,           LANDIS_SCALE_CURRENT,             &Unit_Current,       0,      {NULL},              &Str_Ob_CurB },         //current b        6
    {0x07,    &Str_Ob_CurC,           LANDIS_SCALE_CURRENT,             &Unit_Current,       0,      {NULL},              &Str_Ob_CurC },         //current c        7
    
    {0x09,    &Str_Ob_Freq,           LANDIS_SCALE_FREQ,                &Unit_Freq,          0,      {NULL},              &Str_Ob_Freq },            //freq      9

    {0x0F,    &Str_Ob_AcPowA,         LANDIS_SCALE_ACTIVE_POW,           &Unit_Active_Intan_Kw, 0,      {NULL},              {NULL} },      //active pow pha a 0F   
    {0x10,    &Str_Ob_AcPowB,         LANDIS_SCALE_ACTIVE_POW,           &Unit_Active_Intan_Kw, 0,      {NULL},              {NULL} },        //active pow pha b 10
    {0x11,    &Str_Ob_AcPowC,         LANDIS_SCALE_ACTIVE_POW,           &Unit_Active_Intan_Kw, 0,      {NULL},              {NULL}  },        //active pow pha c 11
    {0x12,    &Str_Ob_AcPowTo,        LANDIS_SCALE_ACTIVE_POW,           &Unit_Active_Intan_Kw, 0,      {NULL},              {NULL} },        //active pow pha total 12

    {0x13,    &Str_Ob_RePowA,         LANDIS_SCALE_REACTIVE_POW,         &Unit_Reactive_Intan_Kvar,       0,      {NULL},              {NULL}},    //Reactive pow pha a 13
    {0x14,    &Str_Ob_RePowB,         LANDIS_SCALE_REACTIVE_POW,         &Unit_Reactive_Intan_Kvar,       0,      {NULL},              {NULL}},    //Reactive pow pha b 14
    {0x15,    &Str_Ob_RePowC,         LANDIS_SCALE_REACTIVE_POW,         &Unit_Reactive_Intan_Kvar,       0,      {NULL},              {NULL}},    //Reactive pow pha c 15
    {0x16,    &Str_Ob_RePowTo,        LANDIS_SCALE_REACTIVE_POW,         &Unit_Reactive_Intan_Kvar,       0,      {NULL},              {NULL}},    //Reactive pow pha total 16
    
    {0x1B,    &Str_Ob_PoFac,           LANDIS_SCALE_POW_FACTOR,          {NULL},                     0,      {NULL},              &Str_Ob_PoFac  },     //Pow factor total 1B
    {0x1C,    &Str_Ob_PoFacA,          LANDIS_SCALE_POW_FACTOR,          {NULL},                     0,      {NULL},              &Str_Ob_PoFacA  },    //Pow factor pha a 1C
    {0x1D,    &Str_Ob_PoFacB,          LANDIS_SCALE_POW_FACTOR,          {NULL},                     0,      {NULL},              &Str_Ob_PoFacB  },      //Pow factor pha b 1D
    {0x1E,    &Str_Ob_PoFacC,          LANDIS_SCALE_POW_FACTOR,          {NULL},                     0,      {NULL},              &Str_Ob_PoFacC  },      //Pow factor pha c 1E
    
    
    {0x1F,    &Str_Ob_AcPlus_Rate1,   LANDIS_SCALE_TARRIFF,              &Unit_Active_EnTotal,           0,      &Str_Ob_AcPlus_Rate1_chot,          &Str_Ob_AcPlus_Rate1},          //nang luong plus bieu 1 1F
    {0x20,    &Str_Ob_AcPlus_Rate2,   LANDIS_SCALE_TARRIFF,              &Unit_Active_EnTotal,           0,      &Str_Ob_AcPlus_Rate2_chot,          &Str_Ob_AcPlus_Rate2},          //nang luong plus bieu 2 20
    {0x21,    &Str_Ob_AcPlus_Rate3,   LANDIS_SCALE_TARRIFF,              &Unit_Active_EnTotal,           0,      &Str_Ob_AcPlus_Rate3_chot,          &Str_Ob_AcPlus_Rate3},          //nang luong plus bieu 3 21

    {0x23,    &Str_Ob_AcSub_Rate1,    LANDIS_SCALE_TARRIFF,              &Unit_Active_EnTotal,           0,      &Str_Ob_AcSub_Rate1_chot,           &Str_Ob_AcSub_Rate1},         //nang luong Sub bieu 1 23
    {0x24,    &Str_Ob_AcSub_Rate2,    LANDIS_SCALE_TARRIFF,              &Unit_Active_EnTotal,           0,      &Str_Ob_AcSub_Rate2_chot,           &Str_Ob_AcSub_Rate2},        //nang luong Sub bieu 2 24
    {0x25,    &Str_Ob_AcSub_Rate3,    LANDIS_SCALE_TARRIFF,              &Unit_Active_EnTotal,           0,      &Str_Ob_AcSub_Rate3_chot,           &Str_Ob_AcSub_Rate3},         //nang luong Sub bieu 3 25

    {0x27,    &Str_Ob_En_ImportWh,    LANDIS_SCALE_TOTAL_ENERGY,         &Unit_Active_EnTotal,           0,      &Str_Ob_AcImTotal_Chot,             &Str_Ob_En_ImportWh},    //Total Plus Wh      27
    {0x28,    &Str_Ob_En_ExportWh,    LANDIS_SCALE_TOTAL_ENERGY,         &Unit_Active_EnTotal,           0,      &Str_Ob_AcExTotal_Chot,             &Str_Ob_En_ExportWh},    //Total Sub Wh      28
    {0x29,    &Str_Ob_En_ImportVar,   LANDIS_SCALE_TOTAL_ENERGY,         &Unit_Reactive_EnTotal,      0,      &Str_Ob_ReImTotal_Chot,             &Str_Ob_En_ImportVar},    //Q1        29
    {0x2B,    &Str_Ob_En_ExportVar,   LANDIS_SCALE_TOTAL_ENERGY,         &Unit_Reactive_EnTotal,      0,      &Str_Ob_ReExTotal_Chot,             &Str_Ob_En_ExportVar},        //Q3        2B

    {0x31,    &Str_Ob_MaxDeRate1,      LANDIS_SCALE_MAX_DEMAND,         &Unit_Active_Intan_Kw,       0,      &Str_Ob_MaxDeRate1_Chot,                {NULL}},                   //Maxdemand plus value bieu 1 31
    {0x32,    {NULL},                  0,                               {NULL},                      0,      {NULL},              {NULL} },                                     //Maxdemand plus time bieu 1 32
    {0x33,    {NULL},                  0,                               &Unit_Active_Intan_Kw,       0,      {NULL},              {NULL} },                   //Maxdemand sub value bieu 1 33
    {0x34,    {NULL},                  0,                               {NULL},                      0,      {NULL},              {NULL}  },
    
    {0x35,    &Str_Ob_MaxDeRate2,      LANDIS_SCALE_MAX_DEMAND,         &Unit_Active_Intan_Kw,       0,      &Str_Ob_MaxDeRate2_Chot,                {NULL}},          //Maxdemand plus value bieu 2 35
    {0x36,    {NULL},                  0,                               {NULL},                      0,      {NULL},              {NULL}  },                            //Maxdemand plus time bieu 2 36
    {0x37,    {NULL},                  0,                               &Unit_Active_Intan_Kw,       0,      {NULL},              {NULL}  },          //Maxdemand sub value bieu 2 37
    {0x38,    {NULL},                  0,                               {NULL},                      0,      {NULL},              {NULL}  },
    
    {0x39,    &Str_Ob_MaxDeRate3,      LANDIS_SCALE_MAX_DEMAND,         &Unit_Active_Intan_Kw,       0,      &Str_Ob_MaxDeRate3_Chot,                {NULL}},           //Maxdemand plus value bieu 3 39
    {0x3A,    {NULL},                  0,                               {NULL},                      0,      {NULL},              {NULL}  },                             //Maxdemand plus time bieu 3 3A
    {0x3B,    {NULL},                  0,                               &Unit_Active_Intan_Kw,       0,      {NULL},              {NULL}  },          //Maxdemand sub value bieu 3 3B
    {0x3C,    {NULL},                  0,                               {NULL},                      0,      {NULL},              {NULL}  },
    
    {0x41,    &Str_Ob_Ti,              LANDIS_SCALE_TU_TI,              {NULL},                      0,      &Str_Ob_Ti,          {NULL}  },                      //Ti     41
    {0x42,    &Str_Ob_Tu,              LANDIS_SCALE_TU_TI,              {NULL},                      0,      &Str_Ob_Tu,          {NULL}  },                     //Tu  42
 };

struct_Obis_event       Obis_landis_Event[LANDIS_MAX_EVENT]= 
{
    {0x17,          &Ev_ProcessPara_Count},       //so laan lap trinh thong so
    {0x18,          &Ev_ProcessPara_Startt},       //thoi gian cuoi cung lap trinh thong so 
    
    {0x01,          &Ev_PhaseAfail_Count},     //mat ap pha A count == Powfail A
    {0x02,          &Ev_PhaseBfail_Count},     //mat ap pha B count == Powfail B
    {0x03,          &Ev_PhaseCfail_Count},     //mat ap pha B count == Powfail C
};
