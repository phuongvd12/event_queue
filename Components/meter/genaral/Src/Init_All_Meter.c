#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"

#include "variable.h"
#include "sim900.h"
#include "t_mqtt.h"
#include "Init_All_Meter.h"
#include "usart.h"
#include "myuart.h"
#include "pushdata.h"

#include "LANDIS_Init_Meter.h"
#include "LANDIS_Read_Meter.h"
#include "GELEX_Init_Meter.h"
#include "GELEX_Read_Meter.h"
#include "ELSTER_Init_Meter.h"
#include "ELSTER_Read_Meter.h"
#include "CPC_Init_Meter.h"
#include "CPC_Read_Meter.h"
//#include "STAR_Init_Meter.h"
//#include "STAR_Read_Meter.h"
#include "Genius_Init_Meter.h"
#include "Genius_Read_Meter.h"
#include "START_Init_Meter.h"
#include "START_Read_Meter.h"
#include "METER103_Init_Meter.h"
#include "METER103_Read_Meter.h"

#include "myDefine.h"
//----------------------------------------------------------------------------------------------------------
const uint16_t 	fcstab[256] = {  0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad,
		0x6536, 0x74bf, 0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e,
		0xf8f7, 0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
		0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876, 0x2102,
		0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd, 0xad4a, 0xbcc3,
		0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5, 0x3183, 0x200a, 0x1291,
		0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c, 0xbdcb, 0xac42, 0x9ed9, 0x8f50,
		0xfbef, 0xea66, 0xd8fd, 0xc974, 0x4204, 0x538d, 0x6116, 0x709f, 0x0420,
		0x15a9, 0x2732, 0x36bb, 0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1,
		0xab7a, 0xbaf3, 0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3,
		0x263a, 0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
		0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9, 0xef4e,
		0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1, 0x7387, 0x620e,
		0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738, 0xffcf, 0xee46, 0xdcdd,
		0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70, 0x8408, 0x9581, 0xa71a, 0xb693,
		0xc22c, 0xd3a5, 0xe13e, 0xf0b7, 0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64,
		0x5fed, 0x6d76, 0x7cff, 0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324,
		0xf1bf, 0xe036, 0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7,
		0x6c7e, 0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
		0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd, 0xb58b,
		0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134, 0x39c3, 0x284a,
		0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c, 0xc60c, 0xd785, 0xe51e,
		0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3, 0x4a44, 0x5bcd, 0x6956, 0x78df,
		0x0c60, 0x1de9, 0x2f72, 0x3efb, 0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9,
		0x8120, 0xb3bb, 0xa232, 0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68,
		0x3ff3, 0x2e7a, 0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238,
		0x93b1, 0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
		0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330, 0x7bc7,
		0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78 };


Meter_Comm_Struct				    Get_Meter_Scale;
Meter_Comm_Struct					Get_Meter_Alert;

Meter_Comm_Struct					Get_Meter_Info;
Meter_Comm_Struct					Get_Meter_Billing;
Meter_Comm_Struct					Get_Meter_Event;
Meter_Comm_Struct					Get_Meter_LProf;
Meter_Comm_Struct				    Get_Meter_TuTi;

Meter_ReadForAlert_Struct	        Meter_ReadForAlert;
Meter_AlertSetting_Struct		    Meter_AlertSetting, temp_Meter_AlertSetting;

struct_Func_ReadMeter               eMeter_20 = {NULL , NULL , NULL , NULL , NULL ,  NULL, NULL, NULL, NULL, NULL, NULL };

/*
    METER_TYPE_ELSTER, 
    METER_TYPE_CPC,
    METER_TYPE_GELEX,
    METER_TYPE_LANDIS, 
    
    METER_TYPE_GENIUS, 
    METER_TYPE_STAR, 
    METER_TYPE_UNKNOWN
  */
Struct_Infor_Meter      ListMeterInfor[] =    
{
    { METER_TYPE_GELEX,  { UART2_BAUR_1, UART2_DATALENGTH,  UART2_STOP_BIT, UART2_PARITY1 },   ReInit_Uart_Meter ,  GELEX_Init_Function}, /*Gelex 2400*/
    { METER_TYPE_103,    { UART2_BAUR_3, UART2_DATALENGTH, UART2_STOP_BIT, UART2_PARITY2 },   ReInit_Uart_Meter ,  METER103_Init_Function}, /*103 2400*/
    { METER_GELEX_103,   { UART2_BAUR_3, UART2_DATALENGTH, UART2_STOP_BIT, UART2_PARITY2 },   ReInit_Uart_Meter ,  METER103_Init_Function}, /*Gelex 2400*/
    { METER_HHM_103,     { UART2_BAUR_3, UART2_DATALENGTH, UART2_STOP_BIT, UART2_PARITY2 },   ReInit_Uart_Meter ,  METER103_Init_Function}, /*HHM 2400*/
    { METER_VSE_103,     { UART2_BAUR_3, UART2_DATALENGTH, UART2_STOP_BIT, UART2_PARITY2 },   ReInit_Uart_Meter ,  METER103_Init_Function}, /*VSE 2400*/
    
    { METER_TYPE_LANDIS, { UART2_BAUR_2, UART2_DATALENGTH,  UART2_STOP_BIT, UART2_PARITY1 },   ReInit_Uart_Meter ,  LANDIS_Init_Function},/*Landis & Gyr*/
    { METER_TYPE_ELSTER, { UART2_BAUR_1, UART2_DATALENGTH,  UART2_STOP_BIT, UART2_PARITY2 },   ReInit_Uart_Meter ,  ELSTER_Init_Function}, /*Elster 9600 even*/
    { METER_TYPE_START,  { 1200,         UART2_DATALENGTH2, UART2_STOP_BIT, UART2_PARITY2 },   ReInit_Uart_Meter ,  START_Init_Function},/*DTS27 Star cu*/
    { METER_TYPE_GENIUS, { UART2_BAUR_1, UART2_DATALENGTH,  UART2_STOP_BIT, UART2_PARITY1 },   ReInit_Uart_Meter ,  GENIUS_Init_Function}, /*Genius 9600*/
    { METER_TYPE_CPC,    { UART2_BAUR_1, UART2_DATALENGTH,  UART2_STOP_BIT, UART2_PARITY1 },   ReInit_Uart_Meter ,  CPC_Init_Function},
//    { METER_TYPE_STAR,   { 1200,         UART2_DATALENGTH2,UART2_STOP_BIT, UART2_PARITY2 },   ReInit_Uart_Meter ,  STAR_Init_Function},/*DTS Star*/
    { METER_TYPE_UNKNOWN },
};


truct_String        StrNull = {NULL};
uint8_t             Buff_Temp_Uart[100];
truct_String        StrUartTemp = {&Buff_Temp_Uart[0], 0};
uint8_t 	        Meter_Type = 0;  // 0 la RS232, 1 la RS485


/*---------------------Function---------------------------------*/

uint8_t Step_Type = 0;
uint8_t Scan_DetectMeter (void)
{
    Step_Type = sDCU.LastMeterType;  //lay metertype cuoi cung ra check truoc
    uint32_t LandMark_Check = RT_Count_Systick_u32;
    
    while(1)
    {
        __HAL_UART_DISABLE_IT(&UART_METER, UART_IT_RXNE);
        switch(Step_Type)
        {
            case METER_TYPE_GELEX:
                RS485_SEND;
                Meter_Type = 1;
                ListMeterInfor[METER_TYPE_GELEX].InitUartMeter(METER_TYPE_GELEX);
                __HAL_UART_ENABLE_IT(&UART_METER, UART_IT_RXNE);
                ListMeterInfor[METER_TYPE_GELEX].initFunctional(METER_TYPE_GELEX);
                if(GELEX_Connect_meter_Gelex() == 1) 
                    return Step_Type;
                Step_Type = METER_TYPE_103;
                break;    
            case METER_TYPE_103:
                RS485_SEND;
                Meter_Type = 1;
                ListMeterInfor[METER_TYPE_103].InitUartMeter(METER_TYPE_103);
                __HAL_UART_ENABLE_IT(&UART_METER, UART_IT_RXNE);
                ListMeterInfor[METER_TYPE_103].initFunctional(METER_TYPE_103);
                if(METER103_Connect_Meter() == 1) 
                {
                    if(sMet103Var.Mettype103 == 1)
                       Step_Type = METER_GELEX_103; 
                    else if(sMet103Var.Mettype103 == 2)
                       Step_Type = METER_HHM_103;
                    else if(sMet103Var.Mettype103 == 3)
                       Step_Type = METER_VSE_103;
                    else 
                        break;
                    
                   return Step_Type;
                }
                Step_Type = METER_TYPE_LANDIS;
                break;
            case METER_GELEX_103:
                Step_Type = METER_TYPE_103;
                break;
             case METER_HHM_103:
                Step_Type = METER_TYPE_103;
                break;  
            case METER_VSE_103:
                Step_Type = METER_TYPE_103;
                break; 
             case METER_TYPE_LANDIS: 
                RS485_SEND;
                Meter_Type = 0;
                ListMeterInfor[METER_TYPE_LANDIS].InitUartMeter(METER_TYPE_LANDIS);
                __HAL_UART_ENABLE_IT(&UART_METER, UART_IT_RXNE);
                ListMeterInfor[METER_TYPE_LANDIS].initFunctional(METER_TYPE_LANDIS);
                if(LANDIS_Connect_meter_Landis() == 1) return Step_Type;
                Step_Type = METER_TYPE_ELSTER;
                break;    
            case METER_TYPE_ELSTER: 
                //gui lenh bat tay
                RS485_SEND;
                Meter_Type = 0;
                ListMeterInfor[METER_TYPE_ELSTER].InitUartMeter(METER_TYPE_ELSTER);
                __HAL_UART_ENABLE_IT(&UART_METER, UART_IT_RXNE);
                ListMeterInfor[METER_TYPE_ELSTER].initFunctional(METER_TYPE_ELSTER);
                if(ELSTER_Connect_Metter_Handle() == 1) return Step_Type;
                Step_Type = METER_TYPE_GENIUS;
                break;       
            case METER_TYPE_GENIUS:
                RS485_SEND;
                Meter_Type = 0;
                ListMeterInfor[METER_TYPE_GENIUS].InitUartMeter(METER_TYPE_GENIUS);
                __HAL_UART_ENABLE_IT(&UART_METER, UART_IT_RXNE);
                ListMeterInfor[METER_TYPE_GENIUS].initFunctional(METER_TYPE_GENIUS);
                if(Genius_Connect_Meter_Handle() == 1) 
                  return Step_Type;
                Step_Type = METER_TYPE_START;
                break;
            case METER_TYPE_START: 
                RS485_SEND;
                //dua chan CTS ve muc 0
                Meter_Type = 0;
                ListMeterInfor[METER_TYPE_START].InitUartMeter(METER_TYPE_START);
                __HAL_UART_ENABLE_IT(&UART_METER, UART_IT_RXNE);
                ListMeterInfor[METER_TYPE_START].initFunctional(METER_TYPE_START);
                if(START_Connect_Meter() == 1) return Step_Type;
                Step_Type = METER_TYPE_CPC;
                break;
            case METER_TYPE_CPC: 
                RS485_SEND;
                Meter_Type = 0;
                ListMeterInfor[METER_TYPE_CPC].InitUartMeter(METER_TYPE_CPC);
                __HAL_UART_ENABLE_IT(&UART_METER, UART_IT_RXNE);
                ListMeterInfor[METER_TYPE_CPC].initFunctional(METER_TYPE_CPC);
                if(CPC_Connect_Metter_Handle() == 1) 
                    return Step_Type;
                Step_Type = METER_TYPE_GELEX;
                break; 
            default: 
                Step_Type = METER_TYPE_GELEX;
                break;
        }
        Get_Meter_Info.Error_Meter_Norespond_ui32 = 0;
        osDelay(500);
        if(Check_Time_Out(LandMark_Check, 1800000) == 1)  //1800000
        {
            _fPackStringToLog ((uint8_t*) "Khong detect duoc meter\r\n", 25);
            Reset_Chip();
        }
    }
}



void ReInit_Uart_Meter(uint8_t type)
{
    UART_METER.Instance = UART__METER;
    UART_METER.Init.BaudRate = ListMeterInfor[type].uartInfo.BaudRate;
    UART_METER.Init.WordLength = ListMeterInfor[type].uartInfo.WordLength;
    UART_METER.Init.StopBits = ListMeterInfor[type].uartInfo.StopBits;
    UART_METER.Init.Parity = ListMeterInfor[type].uartInfo.Parity;
    UART_METER.Init.Mode = UART_MODE_TX_RX;
    UART_METER.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    UART_METER.Init.OverSampling = UART_OVERSAMPLING_16;
    UART_METER.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    UART_METER.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&UART_METER) != HAL_OK)
    {
        Error_Handler();
    }
}

void Init_UART2 (void)   //Khong duoc init lai UART1_Control.Mode_ui8
{
    uint16_t  i = 0;
    
    for(i = 0; i < UART1_Control.Mess_Length_ui16;i++)
        UART1_Receive_Buff[i] = 0;
    
    UART1_Control.UART1_Str_Recei.Data_a8 = &UART1_Receive_Buff[0];
    Reset_Buff(&UART1_Control.UART1_Str_Recei);
    UART1_Control.UART1_Str_Recei.Length_u16 = 0;
	UART1_Control.Mess_Length_ui16 = 0;
	UART1_Control.Mess_Pending_ui8 = 0;
	UART1_Control.Mess_Status_ui8 = 0;
    UART1_Control.Flag_Have_0x02 = 0;
    UART1_Control.Systick_Last_Recerver_u32 = 0;
    UART1_Control.fRecei_Respond = 0;
    UART1_Control.fETX_EOT      = 0;
}



void Init_MD_Bill(void)
{
    MD_Plus_Bill.NumTariff = 0;
    MD_Plus_Bill.NumValue_MD = 0;
    
    MD_Sub_Bill.NumTariff = 0;
    MD_Sub_Bill.NumValue_MD = 0; 
}

void Init_Meter_Struct(Meter_Comm_Struct* Meter_Struct, uint8_t* Buff1, uint8_t* Buff2)
{
    Meter_Struct->Step_ui8 = 1;
	Meter_Struct->Mess_Step_ui8 = 0;
	Meter_Struct->ID_Frame_ui8 = 0;
	Meter_Struct->Reading_ui8 = 0;
	Meter_Struct->Error_ui8 = 0;
    Meter_Struct->Total_OBIS_ui8 = 0;
    Meter_Struct->Num_OBIS_ui8 = 0;
    Meter_Struct->OBIS_Pointer_ui8 = 0;
    Meter_Struct->Num_Block_ui8 = 0;
    Meter_Struct->Reading_Block_ui8 = 0;
    Meter_Struct->Handle_Server_Request_ui8 = 0;
    Meter_Struct->Flag_ui32 = 0;
    Meter_Struct->Collecting_Object_ui8 = 0;
    Meter_Struct->Num_Record_ui16 = 0;
    Meter_Struct->Received_Record_ui16 = 0;
	Meter_Struct->Data_Buff_Pointer_ui16 = 10;
    Meter_Struct->Last_Data_Buff_Pointer_ui16 = 10;
    Meter_Struct->Time_Base_ui32 = 0;
	Meter_Struct->Total_Mess_Sent_ui32 = 0;
	Meter_Struct->Success_Read_Mess_ui32 = 0;
	Meter_Struct->Error_Meter_Norespond_ui32 = 0;
	Meter_Struct->Error_Wrong_Mess_Format_ui32 = 0;
    
    Meter_Struct->Flag_Start_Pack = 0;
    Meter_Struct->Pos_Data_Inbuff = 0;
    Meter_Struct->Pos_Obis_Inbuff = 0;
    Meter_Struct->Str_Payload.Data_a8 = Buff1;
    Reset_Buff(&Meter_Struct->Str_Payload);
    
    Meter_Struct->Str_Payload_2.Data_a8 = Buff2;
    Reset_Buff(&Meter_Struct->Str_Payload_2);
    
    Meter_Struct->PosNumqty = 0;
    Meter_Struct->Numqty = 0;
    
    Meter_Struct->Flag_ui8 = 0;
    Meter_Struct->IndexRead = 0;
    
    Meter_Struct->Mess_Count_ui8 = 0;
    Meter_Struct->Mess_To_Send_ui8 = 0;
    Meter_Struct->First_Mess_Ok_ui8 = 0;
}

void Init_Meter_Info_Struct (void)
{
    Init_Meter_Struct(&Get_Meter_Info, &MeterInfoDataBuff[0], NULL);
}


void Init_Meter_Scale_Struct (void)
{
	Get_Meter_Scale.Step_ui8 = 1;
	Get_Meter_Scale.Mess_Step_ui8 = 0;
	Get_Meter_Scale.ID_Frame_ui8 = 0;
	Get_Meter_Scale.Reading_ui8 = 0;
	Get_Meter_Scale.Flag_ui32 = 0;
	Get_Meter_Scale.Error_ui8 = 0;
	Get_Meter_Scale.Data_Buff_Pointer_ui16 = 10;
	Get_Meter_Scale.Total_Mess_Sent_ui32 = 0;
	Get_Meter_Scale.Success_Read_Mess_ui32 = 0;
	Get_Meter_Scale.Error_Meter_Norespond_ui32 = 0;
	Get_Meter_Scale.Error_Wrong_Mess_Format_ui32 = 0;
}

void Init_Meter_Billing_Struct (void)
{
	Init_Meter_Struct(&Get_Meter_Billing, &MeterBillingDataBuff[0], &MeterBillingDataBuff_2[0]);
}


void Init_Meter_Event_Struct (void)
{
	Init_Meter_Struct(&Get_Meter_Event, &MeterEventDataBuff[0], NULL);
}


void Init_Meter_LProf_Struct (void)
{
    Init_Meter_Struct(&Get_Meter_LProf, &MeterLProfDataBuff[0], NULL); 
}


void Init_Meter_TuTi_Struct (void)
{
	Init_Meter_Struct(&Get_Meter_TuTi, &MeterTuTiDataBuff[0], NULL);     
}

//

uint16_t CountFCS16(uint8_t *buff, uint16_t index, uint16_t count)
{
	uint16_t FCS16 = 0xFFFF;
	uint16_t pos;
	for (pos = index; pos < (index + count); ++pos)
	{
		FCS16 = (uint16_t)(((FCS16 >> 8) ^ fcstab[(FCS16 ^ buff[pos]) & 0xFF]) & 0xFFFF);
	}
	FCS16 = ~FCS16;
	FCS16 = ((FCS16 >> 8) & 0xFF) | (FCS16 << 8);
	return (uint16_t)FCS16;
}

uint8_t getObjectCount(uint8_t *buff, uint8_t index) 
{
	uint8_t cnt = (*(buff+index)) & 0xFF;
	
	index += 1;
	if (cnt > 0x80)
		index = index + cnt +1 - 0x81;
	
	return index;
}


void Insert_Read_Time (uint8_t *aTime, uint8_t Year, uint8_t Month, uint8_t Day, uint8_t Hour, uint8_t Min, uint8_t Sec)
{
	uint16_t Full_Year=0;
	
	Full_Year = Year+2000;
	*(aTime) 	= (uint8_t)((Full_Year>>8)&0x00FF);
	*(aTime+1) 	= (uint8_t)(Full_Year&0x00FF);
	*(aTime+2)	= Month;
	*(aTime+3)	= Day;
	*(aTime+5)	= Hour;
	*(aTime+6)	= Min;
	*(aTime+7)	= Sec;
}
  

uint32_t Nomalize_Read_Value (uint16_t RawVal, uint8_t ReadScale, uint16_t ServerSettingScale)
{
	uint32_t mReVal=0;
	
	switch (ReadScale)
	{
		case 0xFF:
			mReVal = (RawVal*ServerSettingScale)/10;
			break;
		case 0xFE:
			mReVal = (RawVal*ServerSettingScale)/100;
			break;
		case 0xFD:
			mReVal = (RawVal*ServerSettingScale)/1000;
			break;
		case 0xFC:
			mReVal = (RawVal*ServerSettingScale)/10000;
			break;
		case 0x01:
			mReVal = (RawVal*ServerSettingScale*10);
			break;
		case 0x00:
			mReVal = (RawVal*ServerSettingScale);
			break;
		default:
			break;
	}
	
	return mReVal;
}


uint64_t ConvertScaleMeter_toDec (uint8_t ScaleMeter, uint64_t* Div)
{
    uint8_t  i = 0;
    uint64_t Result = 1;
    
    switch (ScaleMeter)
    {
        case 0xFF:
            Result = 1;
            *Div = 10;
            break;
        case 0xFE:
            Result = 1;
            *Div = 100;
            break;
        case 0xFD:
            Result = 1;
            *Div = 1000;
            break;
        case 0xFC:
            Result = 1;
            *Div = 10000;
            break;
        default:
            if(ScaleMeter < 7)
                for(i = 0; i< ScaleMeter; i++)
                    Result = Result * 10;
            *Div = 1;
            break;
    }
    return Result;
}



uint8_t GetUART2Data(void)
{
	uint8_t	temp_recieve = 0;
	
	if (UART1_Control.Mess_Pending_ui8 == 0)  // ???
	{
		temp_recieve = UART_METER.Instance->RDR & 0x00FF; 
		UART1_Receive_Buff[UART1_Control.Mess_Length_ui16] = temp_recieve; // get Data
		
		if ((temp_recieve == 0x7E)&&(UART1_Control.Mess_Length_ui16 == (UART1_Receive_Buff[2]+ 1))) // check de lay het data
		{
			UART1_Control.Mess_Pending_ui8 = 1;  // Thua
			UART1_Control.Mess_Status_ui8 = 1;  // Thua
			// Check them truong hop tra ve dung format nhung la ban tin thong bao khong co data -> thoat (xem nhu ko doc dc)
			osSemaphoreRelease(bsUART2PendingMessHandle);
		}
		else
		{
			UART1_Control.Mess_Length_ui16++;
			if (UART1_Control.Mess_Length_ui16 > MAX_LENGTH_BUFF_NHAN)		
				UART1_Control.Mess_Length_ui16 = 0;
		}
	}
	else
		temp_recieve = UART_METER.Instance->RDR & 0x00FF;
    
    UART1_Control.fRecei_Respond = 1;
    return 1;
}



uint8_t _fWaitSaveInFlash (uint8_t* FlagWait)
{
    uint32_t    LandMark_Checktimeout = 0;
    uint8_t     Result = 1;
    
    if (sDCU.Mutex_ReadMeter_Status_u8 == _MUTEX_WAITED)
        if (osMutexRelease(mtFlashMeterHandle) != osOK)  //chi gay treo khi th flash dang wait.   
             osMutexRelease(mtFlashMeterHandle);
    
    LandMark_Checktimeout = RT_Count_Systick_u32;
    while (*(FlagWait) == 0)    //Gui qua task flash de luu. se set status = 1
    {
        if(Check_Time_Out(LandMark_Checktimeout, 60000) == 1)  
        {
            Result = 0;
            break;
        }
        osDelay(50);
    }

    if (osMutexWait(mtFlashMeterHandle, 20000) != osOK)
    {
        sDCU.Mutex_ReadMeter_Status_u8 = _MUTEX_RELEASED;
        Result = 0;
    } else
        sDCU.Mutex_ReadMeter_Status_u8 = _MUTEX_WAITED;
    
    return Result;
}



int8_t Check_Obis_event (struct_Obis_event* StructObisEvent, uint8_t ID_Event, uint8_t MaxLength) 
{
    uint16_t i = 0;
    
    for(i = 0; i< MaxLength; i++)
      if(ID_Event == StructObisEvent[i].ID_event)
        return i;
    
    return -1;
}

uint8_t ConvertHex_to_Dec(uint8_t Hex)
{
    return (((Hex >> 4)&0x0F)*10 + (Hex & 0x0F));
}


void Func_Scan_NewMeter (void)
{
    Read_Meter_ID_Success = 0;
    sDCU.MeterType = METER_TYPE_UNKNOWN;
    sDCU.sMeter_id_now.Length_u16 = 0;
    
    //check Meter type bang chan HandShake. return qua bien sDCU.MeterType
    sDCU.MeterType = Scan_DetectMeter();
    
//check Meter type bang chan GPIO de init. return qua bien sDCU.MeterType
//    sDCU.MeterType = METER_TYPE_LANDIS; 
//    sDCU.MeterType = METER_TYPE_GELEX; 
//    sDCU.MeterType = METER_TYPE_ELSTER; 
//    sDCU.MeterType = METER_TYPE_CPC;  //check metertype. sau do check serial
//    sDCU.MeterType = METER_TYPE_GENIUS; 
//    sDCU.MeterType = METER_TYPE_START; 
    
//    __HAL_UART_DISABLE_IT(&UART_METER, UART_IT_RXNE);
//    ListMeterInfor[sDCU.MeterType].InitUartMeter(sDCU.MeterType);
//    __HAL_UART_ENABLE_IT(&UART_METER, UART_IT_RXNE);

    
    //Init lai Uart cho c�ng to.//Gui ban tin connect cong to
    if(sDCU.MeterType != 0xFF)
    {
/*
        sSim900_status.TimeoutSendHeartBeat_u32 = 0; //gui hearbeat ngay
        _f_Send_Alarm_To_Queue(CONNECT_METER);
        Save_Array(ADDR_METER_TYPE, &sDCU.MeterType, 1); //luu lai metertype cuoi cung
*/
        ListMeterInfor[sDCU.MeterType].initFunctional(sDCU.MeterType);
//        eMeter_20._f_Connect_Meter();    //comment day khi ch?y
    }
}



void Packet_Empty_MessHistorical (void)
{
    truct_String    Str_Data_Write={&Buff_Temp1[0], 0};
    uint8_t         Temp_BBC = 0;
    
    Reset_Buff(&Get_Meter_Billing.Str_Payload);
    Write_Header_His_Push103();
    Add_TuTI_toPayload(&Get_Meter_Billing);
    //lay het data moi ghep stime Bill vao
    Reset_Buff(&Str_Data_Write);
    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
    //de trong neu k c� data
    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')'; 
    
    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
    Pack_HEXData_Frame_Uint64(&Str_Data_Write, 3, 0);
    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')'; 
    
    Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.PosNumqty, &Str_Data_Write);
    //
    //ETX
    *(Get_Meter_Billing.Str_Payload.Data_a8 + Get_Meter_Billing.Str_Payload.Length_u16++) = ETX; 
    //BBC
    Temp_BBC = BBC_Cacul(Get_Meter_Billing.Str_Payload.Data_a8 + 1,Get_Meter_Billing.Str_Payload.Length_u16 - 1);
    *(Get_Meter_Billing.Str_Payload.Data_a8 + Get_Meter_Billing.Str_Payload.Length_u16++) = Temp_BBC;
    
    Push_Bill_toQueue(DATA_HISTORICAL);
}

uint8_t _fSetStimeMeter_FromDCU (ST_TIME_FORMAT sRTC)
{
    return METER103_SetStimeToMeter(sRTC);
}


