/*
 * Init_Meter.c
 *
 *  Created on: Dec 15, 2015
 *      Author: Administrator
 */

#include "variable.h"
#include "START_Init_Meter.h"
#include "START_Read_Meter.h"
#include "t_mqtt.h"

#include "pushdata.h"

/*======================== Init Structs ======================*/

void Init_IDTable_Readevent (void)
{
	uint8_t i=0;

	for (i=0;i<20;i++)
		MeterEventMessIDTable[i] = i;

	MeterEventMessIDTable[20] = 0xFF;
}

uint8_t		START_MeterInfoMessIDTable[36] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,28,29,30,31,32,33,34,35,27,0xFF};
uint8_t		START_MeterTiTuIDTable[2] = {27, 0xFF};
uint8_t		MeterBillingMessIDTableFirst[10] = {1,2,36,37,38,39,40,41,0xFF,0xFF};
uint8_t		MeterBillingMessIDTable[10] = {1,2,36,37,38,39,40,41,0xFF,0xFF};

uint8_t		Read_Meter_ID_Message[17] 		= {0xFE,0xFE,0xFE,0x68,0x99,0x99,0x99,0x99,0x99,0x99,0x68,0x01,0x02,0x65,0xF3,0xC1,0x16};
uint8_t		Read_Meter_Info_Template[17] 	= {0xFE,0xFE,0xFE,0x68,0x56,0x49,0x71,0x10,0x14,0x00,0x68,0x01,0x02,0x00,0x00,0x00,0x16};
uint8_t		Read_Meter_LProf_Template1[20] 	= {0xFE,0xFE,0xFE,0x68,0x56,0x49,0x71,0x10,0x14,0x00,0x68,0x01,0x05,0x43,0x06,0x00,0x00,0x00,0x00,0x16};
uint8_t		Read_Meter_LProf_Template2[19] 	= {0xFE,0xFE,0xFE,0x68,0x56,0x49,0x71,0x10,0x14,0x00,0x68,0x01,0x04,0x43,0x06,0x00,0x00,0x00,0x16};
uint8_t		Read_Meter_LProf_StartCode[2]	=	{0,0};

uint8_t		Type_Meter = 0;  // Meter direct(0) an indirect(1)
uint8_t	Read_Meter_Info_Code[START_MAX_REGIS_INFOR][4] =
{	    {0x65,0xF3,0x5F,0xFF},	//onSuccessReadAddress									0
		{0x43,0xF3,0x3D,0x00},	//Read Date from meter									1
		{0x44,0xF3,0x3E,0x01},	//Read Time from meter									2		
		{0x43,0xC3,0x0D,0x28},	//Positive active energy (A+) total [kWh]				3
		{0x44,0xC3,0x0E,0x1F},	//Positive active energy (A+) Tariff 1 [kWh]			4
		{0x45,0xC3,0x0F,0x20},	//Positive active energy (A+) Tariff 2 [kWh]			5
		{0x46,0xC3,0x10,0x21},	//Positive active energy (A+) Tariff 3 [kWh]			6
		{0x47,0xC3,0x11,0x22},	//Positive active energy (A+) Tariff 4 [kWh]			7
		{0x53,0xC3,0x1D,0x27},	//Negative active energy (A-) total [kWh]				8
		{0x54,0xC3,0x1E,0x23},	//Negative active energy (A-) Tariff 1 [kWh]			9
		{0x55,0xC3,0x1F,0x24},	//Negative active energy (A-) Tariff 2 [kWh]			10
		{0x56,0xC3,0x20,0x25},	//Negative active energy (A-) Tariff 3 [kWh]			11
		{0x57,0xC3,0x21,0x26},	//Negative active energy (A-) Tariff 4 [kWh]			12
		{0x43,0xC4,0x0E,0x29},	//Positive reactive energy (Q+) total [kvarh]			13
		{0x53,0xC4,0x1E,0x2B},	//Negative reactive energy (Q-) total [kvarh]			14
		{0x43,0xE3,0x2D,0x2F},	//Maximum demand (Time)									15  //doi obis
		{0x43,0xD3,0x1D,0x2E},	//Maximum demand (Value)								16   //doi obis
		{0x92,0xE9,0x82,0x1B},	//Power factor											17
		{0x72,0xE9,0x62,0x12},	//Instant power											18
		{0x82,0xE9,0x72,0x16},	//Reactive power										19
		{0x97,0xE9,0x87,0x09},	//Freq													20
		{0x44,0xE9,0x34,0x02},	//Instant Voltage Phase A								21
		{0x45,0xE9,0x35,0x03},	//Instant Voltage Phase B								22
		{0x46,0xE9,0x36,0x04},	//Instant Voltage Phase C								23
		{0x54,0xE9,0x44,0x05},	//Instant current Phase A								24
		{0x55,0xE9,0x45,0x06},	//Instant current Phase B								25
		{0x56,0xE9,0x46,0x07},	//Instant current Phase C								26
		{0x4F,0xF4,0x4A,0x42},	//Ti-Tu	  												27   //doi obis
		{0x44,0xC4,0x0F,0x49},	//ReactiveEnergyPlusARate1								28
		{0x45,0xC4,0x10,0x4A},	//ReactiveEnergyPlusARate2								29
		{0x46,0xC4,0x11,0x4B},	//ReactiveEnergyPlusARate3								30
		{0x47,0xC4,0x12,0x4C},	//ReactiveEnergyPlusARate4								31
		{0x54,0xC4,0x1F,0x4D},	//ReactiveEnergySubARate1								32
		{0x55,0xC4,0x20,0x4E},	//ReactiveEnergySubARate2								33
		{0x56,0xC4,0x21,0x4F},	//ReactiveEnergySubARate3								34
		{0x57,0xC4,0x22,0x50},	//ReactiveEnergySubARate4	-------------------------------------------------------------35
        //Bill last 1 month
		{0x52,0xCB,0x24,0x10},	//ActivePower											36
		{0x43,0xEB,0x35,0x14},	//Max Demand Active Energy Time							37
		{0x43,0xDB,0x25,0x13},	//Max Demand Active Power								38
		{0x52,0xCC,0x25,0x11},	//ReactivePower											39
		{0x43,0xEC,0x36,0x26},	//Max Demand reactive Energy Time						40
		{0x43,0xDC,0x26,0x25},	//Max Demand reactive Power								41
        //Bill last 2 month
        {0x62,0xCB,0x24,0x10},	//ActivePower											42
		{0x53,0xEB,0x35,0x14},	//Max Demand Active Energy Time							43
		{0x53,0xDB,0x25,0x13},	//Max Demand Active Power								44
		{0x62,0xCC,0x25,0x11},	//ReactivePower											45
		{0x53,0xEC,0x36,0x26},	//Max Demand reactive Energy Time						46
		{0x53,0xDC,0x26,0x25},	//Max Demand reactive Power								47
        //Bill last 3 month
        {0x72,0xCB,0x24,0x10},	//ActivePower											48
		{0x63,0xEB,0x35,0x14},	//Max Demand Active Energy Time							49
		{0x63,0xDB,0x25,0x13},	//Max Demand Active Power								50
		{0x72,0xCC,0x25,0x11},	//ReactivePower											51
		{0x63,0xEC,0x36,0x26},	//Max Demand reactive Energy Time						52
		{0x63,0xDC,0x26,0x25},	//Max Demand reactive Power								53
        //Bill last 4 month
        {0x82,0xCB,0x24,0x10},	//ActivePower											54
		{0x73,0xEB,0x35,0x14},	//Max Demand Active Energy Time							55
		{0x73,0xDB,0x25,0x13},	//Max Demand Active Power								56
		{0x82,0xCC,0x25,0x11},	//ReactivePower											57
		{0x73,0xEC,0x36,0x26},	//Max Demand reactive Energy Time						58
		{0x73,0xDC,0x26,0x25},	//Max Demand reactive Power								59
        //Bill last 5 month
        {0x92,0xCB,0x24,0x10},	//ActivePower											60
		{0x83,0xEB,0x35,0x14},	//Max Demand Active Energy Time							61
		{0x83,0xDB,0x25,0x13},	//Max Demand Active Power								62
		{0x92,0xCC,0x25,0x11},	//ReactivePower											63
		{0x83,0xEC,0x36,0x26},	//Max Demand reactive Energy Time						64
		{0x83,0xDC,0x26,0x25},	//Max Demand reactive Power								65
        //Bill last 6 month
        {0xA2,0xCB,0x24,0x10},	//ActivePower											66
		{0x93,0xEB,0x35,0x14},	//Max Demand Active Energy Time							67
		{0x93,0xDB,0x25,0x13},	//Max Demand Active Power								68
		{0xA2,0xCC,0x25,0x11},	//ReactivePower											69
		{0x93,0xEC,0x36,0x26},	//Max Demand reactive Energy Time						70
		{0x93,0xDC,0x26,0x25},	//Max Demand reactive Power								71
        
        //Bill last 7 month
        {0xB2,0xCB,0x24,0x10},	//ActivePower											72
		{0xA3,0xEB,0x35,0x14},	//Max Demand Active Energy Time							73
		{0xA3,0xDB,0x25,0x13},	//Max Demand Active Power								74
		{0xB2,0xCC,0x25,0x11},	//ReactivePower											75
		{0xA3,0xEC,0x36,0x26},	//Max Demand reactive Energy Time						76
		{0xA3,0xDC,0x26,0x25},	//Max Demand reactive Power								77
        
        //Bill last 8 month
        {0xC2,0xCB,0x24,0x10},	//ActivePower											78
		{0xB3,0xEB,0x35,0x14},	//Max Demand Active Energy Time							79
		{0xB3,0xDB,0x25,0x13},	//Max Demand Active Power								80
		{0xC2,0xCC,0x25,0x11},	//ReactivePower											81
		{0xB3,0xEC,0x36,0x26},	//Max Demand reactive Energy Time						82
		{0xB3,0xDC,0x26,0x25},	//Max Demand reactive Power								83
        
        //Bill last 9 month
        {0xD2,0xCB,0x24,0x10},	//ActivePower											84
		{0xC3,0xEB,0x35,0x14},	//Max Demand Active Energy Time							85
		{0xC3,0xDB,0x25,0x13},	//Max Demand Active Power								86
		{0xD2,0xCC,0x25,0x11},	//ReactivePower											87
		{0xC3,0xEC,0x36,0x26},	//Max Demand reactive Energy Time						88
		{0xC3,0xDC,0x26,0x25},	//Max Demand reactive Power								89
        
        //Bill last 10 month
        {0xE2,0xCB,0x24,0x10},	//ActivePower											90
		{0xD3,0xEB,0x35,0x14},	//Max Demand Active Energy Time							91
		{0xD3,0xDB,0x25,0x13},	//Max Demand Active Power								92
		{0xE2,0xCC,0x25,0x11},	//ReactivePower											93
		{0xD3,0xEC,0x36,0x26},	//Max Demand reactive Energy Time						94
		{0xD3,0xDC,0x26,0x25},	//Max Demand reactive Power								95
        
        //Bill last 11 month
        {0xF2,0xCB,0x24,0x10},	//ActivePower											96
		{0xD3,0xEB,0x35,0x14},	//Max Demand Active Energy Time							97
		{0xD3,0xDB,0x25,0x13},	//Max Demand Active Power								98
		{0xF2,0xCC,0x25,0x11},	//ReactivePower											99
		{0xD3,0xEC,0x36,0x26},	//Max Demand reactive Energy Time						100
		{0xD3,0xDC,0x26,0x25},	//Max Demand reactive Power								101
        
        //Bill last 12 month
        {0x02,0xCB,0x24,0x10},	//ActivePower											102
		{0xF3,0xEB,0x35,0x14},	//Max Demand Active Energy Time							103
		{0x43,0xDB,0x25,0x13},	//Max Demand Active Power								104
		{0x02,0xCC,0x25,0x11},	//ReactivePower											105
		{0xF3,0xEC,0x36,0x26},	//Max Demand reactive Energy Time						106
		{0xF3,0xDC,0x26,0x25},	//Max Demand reactive Power								107
  };


/*
 - doc het tat ca 10 thoi diem roi so sanh de tim duoc thoi diem moi nhat
*/

uint8_t		MeterEventMessIDTable[25];

uint8_t	Read_Meter_Event_Code[START_MAX_REGIS_EVENT][5] = 
{
        {0x44,0xE6,0x31,0x01,0x00},		//So lan mat nguon phase A                  //0         //them byte thu 5 lua chon loai: 0: count   1: time 1     //cu 1 loai gom thanh 1 block   
		{0x43,0x16,0x60,0x02,0x01},		//Thoi diem mat nguon phase A               //1
		{0x43,0x18,0x62,0x03,0x01},		//Thoi diem co nguon phase A                //2
		
        {0x45,0xE6,0x32,0x04,0x00},		//So lan mat nguon phase B                  //3         //block 1
		{0x53,0x16,0x70,0x05,0x01},		//Thoi diem mat nguon phase B               //4
		{0x53,0x18,0x72,0x06,0x01},		//Thoi diem co nguon phase B                //5
		
        {0x46,0xE6,0x33,0x07,0x00},		//So lan mat nguon phase C                  //6         //2
		{0x63,0x16,0x80,0x08,0x01},		//Thoi diem mat nguon phase C               //7
		{0x63,0x18,0x82,0x09,0x01},		//Thoi diem co nguon phase C                //8
		
        {0xA4,0xE6,0x91,0x0A,0x00},		//So lan reverse run phase A                //9        //3
		{0x83,0x16,0xA0,0x0B,0x01},		//Thoi gian bat dau reverse run phase A     //10
		{0x83,0x18,0xA2,0x0C,0x01},		//Thoi gian ket thuc reverse run phase A    //11
		
        {0xA5,0xE6,0x92,0x0D,0x00},		//So lan reverse run phase B                //12        //4
		{0x93,0x16,0xB0,0x0E,0x01},		//Thoi gian bat dau reverse run phase B     //13
		{0x93,0x18,0xB2,0x0F,0x01},		//Thoi gian ket thuc reverse run phase B    //14
		
        {0xA6,0xE6,0x53,0x10,0x00},		//So lan reverse run phase C                //15        //5
		{0xA3,0x16,0xC0,0x11,0x01},		//Thoi gian bat dau reverse run phase C     //16
		{0xA3,0x18,0xC2,0x12,0x01},		//Thoi gian ket thuc reverse run phase C	//17
        //moi them
        {0x64,0xE6,0x91,0x13,0x00},		//So lan over vol phase A                //18           //6
		{0x93,0x15,0xA0,0x14,0x01},		//Thoi gian bat dau over vol phase A     //19
		{0xC3,0x15,0xA2,0x15,0x01},		//Thoi gian ket thuc over vol phase A    //20
		
        {0x65,0xE6,0x92,0x16,0x00},		//So lan over vol phase B                //21           //7
		{0xA3,0x15,0xB0,0x17,0x01},		//Thoi gian bat dau over vol phase B     //22
		{0xD3,0x18,0xB2,0x18,0x01},		//Thoi gian ket thuc over vol phase B    //23
		
        {0x66,0xE6,0x53,0x19,0x00},		//So lan over vol phase C                //24            //8
		{0xB3,0x15,0xC0,0x1A,0x01},		//Thoi gian bat dau over vol phase C     //25
		{0xE3,0x18,0xC2,0x1B,0x01},		//Thoi gian ket thuc over vol phase C	//26
        //current
        {0x84,0xE6,0x91,0x1C,0x00},		//So lan over Curent phase A                //27        //9
		{0xB3,0x18,0xA0,0x1D,0x01},		//Thoi gian bat dau over Curent phase A     //28
		{0xE3,0x18,0xA2,0x1E,0x01},		//Thoi gian ket thuc over Curent phase A    //29
		
        {0x85,0xE6,0x92,0x1F,0x00},		//So lan over Curent phase B                //30        //10
		{0xC3,0x18,0xB0,0x20,0x01},		//Thoi gian bat dau over Curent phase B     //31
		{0xF3,0x18,0xB2,0x21,0x01},		//Thoi gian ket thuc over Curent phase B    //32
		
        {0x86,0xE6,0x53,0x22,0x00},		//So lan over Curent phase C                //33        //11
		{0xD3,0x18,0xC0,0x23,0x01},		//Thoi gian bat dau over Curent phase C     //34
		{0x03,0x18,0xC2,0x24,0x01},		//Thoi gian ket thuc over Curent phase C	//35
         
        {0x43,0x15,0x5F,0x25,0x01},		//Thoi gian cau hinh cong to lan cuoi       //36
        {0x73,0x15,0x39,0x26,0x01},	    //Powoff                                    //37 
        {0x83,0x15,0x49,0x27,0x01},	    //PowOn                                     //38      //chi co time
};


struct_Obis_START       Obis_Start_Ins[START_MAX_OBIS_INTAN]=          
{
    {0x02,    &Str_Ob_VolA,           START_SCALE_VOLTAGE,             &Unit_Voltage,              {NULL}  },         //vol a             2
    {0x03,    &Str_Ob_VolB,           START_SCALE_VOLTAGE,             &Unit_Voltage,              {NULL}  },         //vol b             3
    {0x04,    &Str_Ob_VolC,           START_SCALE_VOLTAGE,             &Unit_Voltage,              {NULL}  },          //vol c             4
    
    {0x05,    &Str_Ob_CurA,           START_SCALE_CURRENT,             &Unit_Current,              {NULL}  },         //current a        5
    {0x06,    &Str_Ob_CurB,           START_SCALE_CURRENT,             &Unit_Current,              {NULL}  },         //current b        6
    {0x07,    &Str_Ob_CurC,           START_SCALE_CURRENT,             &Unit_Current,              {NULL}  },         //current c        7
    
    {0x09,    &Str_Ob_Freq,           START_SCALE_FREQ,                &Unit_Freq,                 {NULL}  },            //freq      9
   
    {0x0F,    &Str_Ob_AcPowA,         START_SCALE_ACTIVE_POW,           &Unit_Reactive_Intan_Kvar,        {NULL}  },      //active pow pha a 0F   
    {0x10,    &Str_Ob_AcPowB,         START_SCALE_ACTIVE_POW,           &Unit_Reactive_Intan_Kvar,        {NULL}  },        //active pow pha b 10
    {0x11,    &Str_Ob_AcPowC,         START_SCALE_ACTIVE_POW,           &Unit_Reactive_Intan_Kvar,        {NULL}  },        //active pow pha c 11
    {0x12,    &Str_Ob_AcPowTo,        START_SCALE_ACTIVE_POW,           &Unit_Reactive_Intan_Kvar,        {NULL}  },        //active pow pha total 12
    
    {0x12,    &Str_Ob_RePowA,         START_SCALE_REACTIVE_POW,         &Unit_Reactive_Intan_Kvar,      {NULL}  },    //Reactive pow pha a 13
    {0x13,    &Str_Ob_RePowB,         START_SCALE_REACTIVE_POW,         &Unit_Reactive_Intan_Kvar,      {NULL}  },    //Reactive pow pha b 14
    {0x14,    &Str_Ob_RePowC,         START_SCALE_REACTIVE_POW,         &Unit_Reactive_Intan_Kvar,      {NULL}  },    //Reactive pow pha c 15
    {0x15,    &Str_Ob_RePowTo,        START_SCALE_REACTIVE_POW,         &Unit_Reactive_Intan_Kvar,      {NULL}  },    //Reactive pow pha total 16
    
    {0x1B,    &Str_Ob_PoFac,           START_SCALE_POW_FACTOR,          {NULL},                    {NULL}  },     //Pow factor total 1B
    {0x1C,    &Str_Ob_PoFacA,          START_SCALE_POW_FACTOR,          {NULL},                    {NULL}  },    //Pow factor pha a 1C
    {0x1D,    &Str_Ob_PoFacB,          START_SCALE_POW_FACTOR,          {NULL},                    {NULL}  },      //Pow factor pha b 1D
    {0x1E,    &Str_Ob_PoFacC,          START_SCALE_POW_FACTOR,          {NULL},                    {NULL}  },      //Pow factor pha c 1E
    
    
    {0x1F,    &Str_Ob_AcPlus_Rate1,   START_SCALE_TARRIFF,              &Unit_Active_EnTotal,          &Str_Ob_AcPlus_Rate1_chot   },          //nang luong plus bieu 1 1F
    {0x20,    &Str_Ob_AcPlus_Rate2,   START_SCALE_TARRIFF,              &Unit_Active_EnTotal,          &Str_Ob_AcPlus_Rate2_chot   },          //nang luong plus bieu 2 20
    {0x21,    &Str_Ob_AcPlus_Rate3,   START_SCALE_TARRIFF,              &Unit_Active_EnTotal,          &Str_Ob_AcPlus_Rate3_chot   },          //nang luong plus bieu 3 21
    {0x22,    &Str_Ob_AcPlus_Rate4,   START_SCALE_TARRIFF,              &Unit_Active_EnTotal,          &Str_Ob_AcPlus_Rate4_chot   }, 
    
    {0x23,    &Str_Ob_AcSub_Rate1,    START_SCALE_TARRIFF,              &Unit_Active_EnTotal,          &Str_Ob_AcSub_Rate1_chot    },         //nang luong Sub bieu 1 23
    {0x24,    &Str_Ob_AcSub_Rate2,    START_SCALE_TARRIFF,              &Unit_Active_EnTotal,          &Str_Ob_AcSub_Rate2_chot    },        //nang luong Sub bieu 2 24
    {0x25,    &Str_Ob_AcSub_Rate3,    START_SCALE_TARRIFF,              &Unit_Active_EnTotal,          &Str_Ob_AcSub_Rate3_chot    },         //nang luong Sub bieu 3 25
    {0x26,    &Str_Ob_AcSub_Rate4,    START_SCALE_TARRIFF,              &Unit_Active_EnTotal,          &Str_Ob_AcSub_Rate4_chot    }, 
    
    {0x28,    &Str_Ob_En_ImportWh,    START_SCALE_TOTAL_ENERGY,         &Unit_Active_EnTotal,          &Str_Ob_AcImTotal_Chot  },    //Total Plus Wh      27   //con nay bi doi va elster
    {0x27,    &Str_Ob_En_ExportWh,    START_SCALE_TOTAL_ENERGY,         &Unit_Active_EnTotal,          &Str_Ob_AcExTotal_Chot  },    //Total Sub Wh      28
    {0x29,    &Str_Ob_En_ImportVar,   START_SCALE_TOTAL_ENERGY,         &Unit_Reactive_EnTotal,        &Str_Ob_ReImTotal_Chot  },    //Q1        29
    {0x2B,    &Str_Ob_En_ExportVar,   START_SCALE_TOTAL_ENERGY,         &Unit_Reactive_EnTotal,        &Str_Ob_ReExTotal_Chot  },    //Q3        2B

    {0x2E,    &Str_Ob_MaxDe,           START_SCALE_MAX_DEMAND,   &Unit_Reactive_Intan_Kvar,               &Str_Ob_MaxDe_Chot    },           //Maxdemand plus value bieu 1 31
    {0x2F,    {NULL},                  0,                        {NULL},                           {NULL}    },                        //Maxdemand plus time bieu 1 32
    {0x30,    &Str_Ob_MaxDeSub,        START_SCALE_MAX_DEMAND,   &Unit_Reactive_Intan_Kvar,               &Str_Ob_MaxDe2_Chot },          //Maxdemand sub value bieu 1 33
    {0x31,    {NULL},                  0,                        {NULL},                           {NULL}    },                        //Maxdemand sub time bieu 1 34
    
    
    {0x32,    &Str_Ob_MaxDeRate1,      START_SCALE_MAX_DEMAND,   &Unit_Reactive_Intan_Kvar,               &Str_Ob_MaxDeRate1_Chot   },      //Maxdemand plus value bieu 1 31
    {0x33,    {NULL},                  0,                        {NULL},                           {NULL}    },                        //Maxdemand plus time bieu 1 32
    {0x34,    &Str_Ob_MaxDeSub_Rate1,   START_SCALE_MAX_DEMAND,   &Unit_Reactive_Intan_Kvar,              &Str_Ob_MaxDe2Rate1_Chot },    //Maxdemand sub value bieu 1 33
    {0x35,    {NULL},                  0,                        {NULL},                           {NULL}    },                        //Maxdemand sub time bieu 1 34
    
    {0x36,    &Str_Ob_MaxDeRate2,      START_SCALE_MAX_DEMAND,   &Unit_Reactive_Intan_Kvar,               &Str_Ob_MaxDeRate2_Chot   },      //Maxdemand plus value bieu 2 35
    {0x37,    {NULL},                  0,                        {NULL},                           {NULL}    },                       //Maxdemand plus time bieu 2 36
    {0x38,    &Str_Ob_MaxDeSub_Rate2,  START_SCALE_MAX_DEMAND,   &Unit_Reactive_Intan_Kvar,               &Str_Ob_MaxDe2Rate2_Chot  },    //Maxdemand sub value bieu 2 37
    {0x39,    {NULL},                  0,                        {NULL},                           {NULL}    },                        //Maxdemand sub time bieu 2 38
    
    {0x3A,    &Str_Ob_MaxDeRate3,      START_SCALE_MAX_DEMAND,   &Unit_Reactive_Intan_Kvar,               &Str_Ob_MaxDeRate3_Chot   },      //Maxdemand plus value bieu 3 39
    {0x3B,    {NULL},                  0,                        {NULL},                           {NULL}    },                       //Maxdemand plus time bieu 3 3A
    {0x3C,    &Str_Ob_MaxDeSub_Rate3,  START_SCALE_MAX_DEMAND,   &Unit_Reactive_Intan_Kvar,               &Str_Ob_MaxDe2Rate3_Chot  },    //Maxdemand sub value bieu 3 3B
    {0x3D,    {NULL},                  0,                        {NULL},                           {NULL}    },                        //Maxdemand sub time bieu 3 3C
    
    {0x3E,    &Str_Ob_MaxDeRate4,      START_SCALE_MAX_DEMAND,   &Unit_Reactive_Intan_Kvar,               &Str_Ob_MaxDeRate4_Chot   },      //Maxdemand plus value bieu 4 39
    {0x3F,    {NULL},                  0,                        {NULL},                           {NULL}    },                       //Maxdemand plus time bieu 4 3A
    {0x40,    &Str_Ob_MaxDeSub_Rate4,  START_SCALE_MAX_DEMAND,   &Unit_Reactive_Intan_Kvar,               &Str_Ob_MaxDe2Rate4_Chot  },    //Maxdemand sub value bieu 4 3B
    {0x41,    {NULL},                  0,                        {NULL},                           {NULL}    },                       //Maxdemand sub time bieu 4 3C
  
    {0x42,    &Str_Ob_Ti,              START_SCALE_TU_TI,        {NULL},                            &Str_Ob_Ti    },                      //Ti     41
    {0x43,    &Str_Ob_Tu,              START_SCALE_TU_TI,        {NULL},                            &Str_Ob_Tu    },                   //Tu  42
};



struct_Obis_event       Obis_Start_Event[START_MAX_EVENT]= 
{
    {0x01,       &Ev_PhaseAfail_Count},     //VT Failure of phase A count     //0
    {0x02,       &Ev_PhaseAfail_Startt},     //VT Failure of phase A start
    {0x03,       &Ev_PhaseAfail_Stopt},     //VT Failure of phase A end
    
    {0x04,       &Ev_PhaseBfail_Count},     //VT Failure of phase B count 
    {0x05,       &Ev_PhaseBfail_Startt},     //VT Failure of phase B start
    {0x06,       &Ev_PhaseBfail_Stopt},     //VT Failure of phase B end
    
    {0x07,       &Ev_PhaseCfail_Count},    //VT Failure of phase c count 
    {0x08,       &Ev_PhaseCfail_Startt},     //VT Failure of phase C start
    {0x09,       &Ev_PhaseCfail_Stopt},     //VT Failure of phase C end       //8

    {0x0A,       &Ev_PhaseAreve_Count},    //reverse of phase A count 
    {0x0B,       &Ev_PhaseAreve_Startt},    //reverse of phase A start
    {0x0C,       &Ev_PhaseAreve_Stopt},    //reverse of phase A end
    
    {0x0D,       &Ev_PhaseBreve_Count},    //reverse of phase B count 
    {0x0E,       &Ev_PhaseBreve_Startt},    //reverse of phase B start
    {0x0F,       &Ev_PhaseBreve_Stopt},    //reverse of phase B end
    
    {0x10,       &Ev_PhaseCreve_Count},    //reverse of phase C count 
    {0x11,       &Ev_PhaseCreve_Startt},    //reverse of phase C start
    {0x12,       &Ev_PhaseCreve_Stopt},    //reverse of phase C end    //19
    
    {0x13,       &Ev_PhaseAOver_Count},     //Over Vol of phase A count
    {0x14,       &Ev_PhaseAOver_Startt},     //Over Vol of phase A start
    {0x15,       &Ev_PhaseAOver_Stopt},     //Over Vol of phase A end
    
    {0x16,       &Ev_PhaseBOver_Count},    //Over Vol of phase B count
    {0x17,       &Ev_PhaseBOver_Startt},     //Over Vol of phase B start
    {0x18,       &Ev_PhaseBOver_Stopt},     //Over Vol of phase B end
    
    {0x19,       &Ev_PhaseCOver_Count},     //Over Vol of phase Ccount
    {0x1A,       &Ev_PhaseCOver_Startt},     //Over Vol of phase C start
    {0x1B,       &Ev_PhaseCOver_Stopt},     //Over Vol of phase C end        //11
    
    {0x1C,       &Ev_PhaseAOverCur_Count},     //Over current of phase A count
    {0x1D,       &Ev_PhaseAOverCur_Startt},     //Over current of phase A start
    {0x1E,       &Ev_PhaseAOverCur_Stop},     //Over current of phase A end
    
    {0x1F,       &Ev_PhaseBOverCur_Count},     //Over current of phase B count
    {0x20,       &Ev_PhaseBOverCur_Startt},     //Over current of phase B start
    {0x21,       &Ev_PhaseBOverCur_Stop},     //Over current of phase B end
    
    {0x22,       &Ev_PhaseCOverCur_Count},     //Over current of phase C count
    {0x23,       &Ev_PhaseCOverCur_Startt},     //Over current of phase C start
    {0x24,       &Ev_PhaseCOverCur_Stop},     //Over current of phase C end        //11
             
    {0x25,       &Ev_ProcessPara_Startt},      //thay doi cau hinh cong to  stime   //10
    
    {0x26,       &Ev_PowerDow_Startt},     //Power Off
    {0x27,       &Ev_PowerOn_Startt},    //Power On
    
};

