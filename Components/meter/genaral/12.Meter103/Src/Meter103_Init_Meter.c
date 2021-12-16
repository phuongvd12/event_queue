/*
 * Init_Meter.c
 *
 *  Created on: Dec 15, 2015
 *      Author: Administrator
 */

#include "variable.h"
#include "Meter103_Init_Meter.h"
#include "Meter103_Read_Meter.h"
#include "t_mqtt.h"
#include "pushdata.h"

/*======================== Init Structs ======================*/


uint8_t	MET103_GELEX_MANUFAC[3] = {'G', 'L', 'X'};
uint8_t	MET103_HH_MANUFAC[3] = {'H', 'H', 'M'};
uint8_t	MET103_VSE_MANUFAC[3] = {'V', 'S', 'E'};

uint8_t	MET103_Handshake[5] = {0x2F,0x3F,0x21,0x0D,0x0A};  
uint8_t	MET103_REQUEST_BAUD[6] = {0x06,0x30,0x30,0x31,0x0D,0x0A};  
//01 50 31 02 28 30 30 30 30 30 30 30 30 29 03 61 	<SOH>P1<STX>(00000000)<ETX><BCC>
uint8_t	MET103_SEND_PASS[16] = {0x01, 0x50, 0x31, 0x02, 0x28, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x29, 0x03, 0x61}; 
                                //0    1     2     3     4     5     6     7     8     9     10    11    12    13    14    15    
uint8_t METER103_PASS_GEL[8] ={"00000000"};      //vi tri tu byte thu 5
uint8_t METER103_PASS_HHM[8] ={"M_KH_DOC"};
uint8_t METER103_PASS_VSE[8] ={"11111111"};
uint8_t METER103_PASS_WRITE_HH[8] ={"12345678"};

uint8_t METER103_MeterInfoMessIDTable[60] = { 1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
                                             21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 
                                             41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 0,  0xFF};


uint8_t METER103_MeterTuTi_IDTable[3]     = {0, 1, 0xFF};
uint8_t METER103_MeterBillMessIDTable[30] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 ,20, 21, 22, 23, 24, 0xFF};
uint8_t METER103_MeterEvenMessIDTable[30] = {0, 2, 4, 6, 8, 10, 0xFF};
uint8_t METER103_MeterLPFMessIDTable[3]   = {0, 1, 0xFF};

uint8_t METER103_LPF_StartTime[5];
uint8_t METER103_LPF_StopTime[5];

truct_String    METER103_TSVH_ObisCode[METER103_MAX_OBIS_TSVH] = {
	{(uint8_t*) "0.0.0.9.4", 9},       //MeterTime                                  0     
    
	{(uint8_t*) "1.0.1.8.0", 9},       //PositiveActiveEnergyTotal                  1
	{(uint8_t*) "1.0.1.8.1", 9},       //PositiveActiveEnergyTariff1                2
	{(uint8_t*) "1.0.1.8.2", 9},       //PositiveActiveEnergyTariff2                3
	{(uint8_t*) "1.0.1.8.3", 9},       //PositiveActiveEnergyTariff3                4

	{(uint8_t*) "1.0.2.8.0", 9},       //NegativeActiveEnergyTotal                  5
	{(uint8_t*) "1.0.2.8.1", 9},       //NegativeActiveEnergyTariff1                6      
	{(uint8_t*) "1.0.2.8.2", 9},       //NegativeActiveEnergyTariff2                7
	{(uint8_t*) "1.0.2.8.3", 9},       //NegativeActiveEnergyTariff3                8
    
	{(uint8_t*) "1.0.3.8.0", 9},       //PositiveReactiveEnergyTotal                9
	{(uint8_t*) "1.0.3.8.1", 9},       //PositiveReactiveEnergyTariff1              10
	{(uint8_t*) "1.0.3.8.2", 9},       //PositiveReactiveEnergyTariff2              11     
	{(uint8_t*) "1.0.3.8.3", 9},       //PositiveReactiveEnergyTariff3              12
    
	{(uint8_t*) "1.0.4.8.0", 9},       //NegativeReactiveEnergyTotal                13
	{(uint8_t*) "1.0.4.8.1", 9},       //NegativeReactiveEnergyTariff1              14
	{(uint8_t*) "1.0.4.8.2", 9},       //NegativeReactiveEnergyTariff2              15
	{(uint8_t*) "1.0.4.8.3", 9},       //NegativeReactiveEnergyTariff3              16      
    
    {(uint8_t*) "1.0.1.7.0", 9},        //InstantPowerTotal                         17
	{(uint8_t*) "1.0.21.7.0", 10},       //InstantPowerPhaseA                       18
	{(uint8_t*) "1.0.41.7.0", 10},       //InstantPowerPhaseB                       19
	{(uint8_t*) "1.0.61.7.0", 10},       //InstantPowerPhaseC                       20
    
	{(uint8_t*) "1.0.3.7.0", 9},        //ReactivePowerTotal                        21
	{(uint8_t*) "1.0.23.7.0", 10},       //ReactivePowerPhaseA                      22    
	{(uint8_t*) "1.0.43.7.0", 10},       //ReactivePowerPhaseB                      23
	{(uint8_t*) "1.0.63.7.0", 10},       //ReactivePowerPhaseC                      24
    
	{(uint8_t*) "1.0.9.7.0", 9},        //RealApparentPowerTotal                    25
	{(uint8_t*) "1.0.29.7.0", 10},       //ApparentPowerPhaseA                      26
	{(uint8_t*) "1.0.49.7.0", 10},       //ApparentPowerPhaseB                      27
	{(uint8_t*) "1.0.69.7.0", 10},       //ApparentPowerPhaseC                      28
    
    {(uint8_t*) "1.0.32.7.0", 10},       //InstantVoltagePhaseA		                29
	{(uint8_t*) "1.0.52.7.0", 10},       //InstantVoltagePhaseB		                30
	{(uint8_t*) "1.0.72.7.0", 10},       //InstantVoltagePhaseC		                31    
    
	{(uint8_t*) "1.0.31.7.0", 10},       //InstantCurrentPhaseA 		            32
	{(uint8_t*) "1.0.51.7.0", 10},       //InstantCurrentPhaseB		                33
	{(uint8_t*) "1.0.71.7.0", 10},       //InstantCurrentPhaseC		                34
    
	{(uint8_t*) "1.0.13.7.0", 10},       //TotalPowerFactor			                35              
	{(uint8_t*) "1.0.33.7.0", 10},       //PowerFactorL1				            36
	{(uint8_t*) "1.0.53.7.0", 10},       //PowerFactorL2				            37
	{(uint8_t*) "1.0.73.7.0", 10},       //PowerFactorL3				            38
	{(uint8_t*) "1.0.14.7.0", 10},       //Frequencies				                39
    
    {(uint8_t*) "1.0.1.6.0", 9},         //Maxdemand A+ Total                       40
    {(uint8_t*) "1.0.1.6.1", 9},         //Maxdemand A+ 1                           41
    {(uint8_t*) "1.0.1.6.2", 9},         //Maxdemand A+ 2                           42
    {(uint8_t*) "1.0.1.6.3", 9},         //Maxdemand A+ 3                           43
    
    {(uint8_t*) "1.0.2.6.0", 9},         //Maxdemand A- Total                       44            
    {(uint8_t*) "1.0.2.6.1", 9},         //Maxdemand A- 1                           45        
    {(uint8_t*) "1.0.2.6.2", 9},         //Maxdemand A- 2                           46
    {(uint8_t*) "1.0.2.6.3", 9},         //Maxdemand A- 3                           47
    
    //Event TSVH (C.7.10)(C.51.84)(C.51.86)(C.51.88)(C.51.90)(C.51.92)(C.51.94)(C.51.8)(C.51.15)(C.51.16)
    {(uint8_t*) "0.0.C.7.10", 10},     //Powdown start time va Powup time (10 va C.51.14) //48
    {(uint8_t*) "0.0.C.51.84", 11},   //FAIL Phase A start và stop time (84  và 86)      //49    
    {(uint8_t*) "0.0.C.51.88", 11},   //FAIL Phase B start và stop time (88  và 90)      //50    
    {(uint8_t*) "0.0.C.51.92", 11},   //FAIL Phase C start và stop time (92  và 94)      //51    
    //co 2 thoi gian nhung chi lay 1 obis
    {(uint8_t*) "0.0.C.51.8", 10},     //Event reverse power flow - timestamp           //52
    {(uint8_t*) "0.0.C.51.16", 11},    //Event RTC (Real Time Clock) set - timestamp     //53  0.0.C.51.16
};




truct_String    METER103_TUTI_ObisCode[MAX_OBIS_TUTI] = 
{
    {(uint8_t*) "1.0.0.4.3", 9},       //Tu                                                                   
    {(uint8_t*) "1.0.0.4.2", 9},       //Ti   
};


//Obis cua Lastbill, các bill khac chi can thay byte cuoi .1 thanh ,2  ... .12
truct_String    METER103_BILL_ObisCode[MAX_OBIS_BILL] = 
{
    {(uint8_t*) "1.0.0.1.3.1", 11},       //LastBill MeterTime                              //0
    
	{(uint8_t*) "1.0.1.8.0.1", 11},       //LastBill PositiveActiveEnergyTotal              //1    
	{(uint8_t*) "1.0.1.8.1.1", 11},       //LastBill PositiveActiveEnergyTariff1            //2
	{(uint8_t*) "1.0.1.8.2.1", 11},       //LastBill PositiveActiveEnergyTariff2            //3
	{(uint8_t*) "1.0.1.8.3.1", 11},       //LastBill PositiveActiveEnergyTariff3            //4

	{(uint8_t*) "1.0.2.8.0.1", 11},       //LastBill NegativeActiveEnergyTotal              //5
	{(uint8_t*) "1.0.2.8.1.1", 11},       //LastBill NegativeActiveEnergyTariff1            //6      
	{(uint8_t*) "1.0.2.8.2.1", 11},       //LastBill NegativeActiveEnergyTariff2            //7
	{(uint8_t*) "1.0.2.8.3.1", 11},       //LastBill NegativeActiveEnergyTariff3            //8
    
	{(uint8_t*) "1.0.3.8.0.1", 11},       //LastBill PositiveReactiveEnergyTotal            //9
	{(uint8_t*) "1.0.3.8.1.1", 11},       //LastBill PositiveReactiveEnergyTariff1          //10    
	{(uint8_t*) "1.0.3.8.2.1", 11},       //LastBill PositiveReactiveEnergyTariff2          //11    
	{(uint8_t*) "1.0.3.8.3.1", 11},       //LastBill PositiveReactiveEnergyTariff3          //12
    
	{(uint8_t*) "1.0.4.8.0.1", 11},       //LastBill NegativeReactiveEnergyTotal            //13
	{(uint8_t*) "1.0.4.8.1.1", 11},       //LastBill NegativeReactiveEnergyTariff1          //14
	{(uint8_t*) "1.0.4.8.2.1", 11},       //LastBill NegativeReactiveEnergyTariff2          //15    
	{(uint8_t*) "1.0.4.8.3.1", 11},       //LastBill NegativeReactiveEnergyTariff3          //16
    
    {(uint8_t*) "1.0.1.6.0.1", 11},         //LastBill Maxdemand A+ Total                   //17
    {(uint8_t*) "1.0.1.6.1.1", 11},         //LastBill Maxdemand A+ 1                       //18
    {(uint8_t*) "1.0.1.6.2.1", 11},         //LastBill Maxdemand A+ 2                       //19
    {(uint8_t*) "1.0.1.6.3.1", 11},         //LastBill Maxdemand A+ 3                       //20
    
    {(uint8_t*) "1.0.2.6.0.1", 11},         //LastBill Maxdemand A- Total                   //21         
    {(uint8_t*) "1.0.2.6.1.1", 11},         //LastBill Maxdemand A- 1                       //22        
    {(uint8_t*) "1.0.2.6.2.1", 11},         //LastBill Maxdemand A- 2                       //23
    {(uint8_t*) "1.0.2.6.3.1", 11},         //LastBill Maxdemand A- 3                       //24
};



truct_String    METER103_EVENT_ObisCode[MAX_OBIS_EVEN] = 
{
    //Event TSVH (C.7.10)(C.51.84)(C.51.86)(C.51.88)(C.51.90)(C.51.92)(C.51.94)(C.51.8)(C.51.15)(C.51.16)
    {(uint8_t*) "0.0.C.7.10", 10},     //Powdown start time va Powup time (10 va C.51.14) //0
    {(uint8_t*) "0.0.C.51.14", 11},                                                       //1
    {(uint8_t*) "0.0.C.51.84", 11},   //FAIL Phase A start và stop time (84  và 86)       //2    
    {(uint8_t*) "0.0.C.51.86", 11},                                                       //3    
    {(uint8_t*) "0.0.C.51.88", 11},   //FAIL Phase B start và stop time (88  và 90)       //4   
    {(uint8_t*) "0.0.C.51.90", 11},                                                       //5   
    {(uint8_t*) "0.0.C.51.92", 11},   //FAIL Phase C start và stop time (92  và 94)       //6  
    {(uint8_t*) "0.0.C.51.94", 11},                                                       //7
    //Event khac
    //Co 2 thoi gian nhung chi lay 1 thoi gian va 1 obis
    {(uint8_t*) "0.0.C.51.8", 10},      //Event reverse power flow - timestamp            //8
    {(uint8_t*) "", 0},                 //Event reverse power flow - timestamp            //9
    {(uint8_t*) "0.0.C.51.16", 11},     //Event RTC (Real Time Clock) set - timestamp     //10
    {(uint8_t*) "", 0},                 //Event reverse power flow - timestamp            //11
};


truct_String    METER103_LPF_ObisCode[MAX_OBIS_LPF] = 
{
    {(uint8_t*) "1.0.99.1.0", 10},     //LPF1
    {(uint8_t*) "1.0.99.2.0", 10},     //LPF2
};


/*======================== Function ======================*/











