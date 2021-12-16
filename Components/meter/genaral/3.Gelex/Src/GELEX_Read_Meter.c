#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"
#include "variable.h"
#include "usart.h"
//#include "S25FL216K.h"
#include "onchipflash.h"
#include "t_mqtt.h"
#include "gpio.h"
#include "pushdata.h"
#include "math.h"

#include "GELEX_Init_Meter.h"
#include "GELEX_Read_Meter.h"
#include "at_commands.h"

#include "myDefine.h"

#define UNDER_COMP	0
#define OVER_COMP	1

uint8_t		                Billing_Num_Record_ui8=0;
uint8_t                     GE_Fl_Alow_GetTimeLpf = 0;

void GELEX_Init_Function (uint8_t type)
{
    eMeter_20._f_Read_ID            = GELEX_Get_Meter_ID;
    eMeter_20._f_Check_Reset_Meter  = GELEX_CheckResetReadMeter;
  
    eMeter_20._f_Connect_Meter      = GELEX_Connect_meter_Gelex;
    eMeter_20._f_Read_TSVH          = GELEX_Read_TSVH;
    eMeter_20._f_Read_Bill          = GELEX_Read_Bill;
    eMeter_20._f_Read_Event         = GELEX_Read_Event;  
    eMeter_20._f_Read_Lpf           = GELEX_Read_Lpf;
    eMeter_20._f_Read_InforMeter    = GELEX_Read_Infor;
    eMeter_20._f_Get_UartData       = GetUART2Data;
    eMeter_20._f_Check_Meter        = GELEX_Check_Meter;
    eMeter_20._f_Test1Cmd_Respond   = GELEX_Send1Cmd_Test;
}

uint8_t GELEX_Send1Cmd_Test (void)
{
	Init_UART2();
    RS485_SEND;
    osDelay(READ_DELAY);
    HAL_UART_Transmit(&UART_METER,&GELEX_SNRM[0],9,1000);
    RS485_RECIEVE;
    if (osSemaphoreWait(bsUART2PendingMessHandle,2000) == osOK) 
    {
        RS485_SEND;
        osDelay(READ_DELAY);
        HAL_UART_Transmit(&UART_METER,&GELEX_Read_END[0],9,1000);
        RS485_RECIEVE;
    }
    if(UART1_Control.fRecei_Respond == 1) 
        return 1;
    
    return 0;
}

uint8_t GELEX_Read_TSVH (uint32_t Type)
{
    uint8_t         Temp_BBC = 0;
    truct_String    Str_Data_Write = {&Buff_Temp1[0], 0};
    
    Init_Meter_TuTi_Struct();
    if(GELEX_Read_Info_Scale(&Get_Meter_TuTi,&GELEX_TuTi_ExtractDataFunc,&GELEX_TuTi_SendData,&GELEX_Fill_MTuTi_Mess,GELEX_MeterTuTiMessIDTable) != 1) 
    {   
        Init_Meter_TuTi_Struct();
        if(GELEX_Read_Info_Scale(&Get_Meter_TuTi,&GELEX_TuTi_ExtractDataFunc,&GELEX_TuTi_SendData,&GELEX_Fill_MTuTi_Mess,GELEX_MeterTuTiMessIDTable) != 1) 
        {
            Init_Meter_TuTi_Struct();
            if(GELEX_Read_Info_Scale(&Get_Meter_TuTi,&GELEX_TuTi_ExtractDataFunc,&GELEX_TuTi_SendData,&GELEX_Fill_MTuTi_Mess,GELEX_MeterTuTiMessIDTable) != 1) 
                return 0;
        }
    }
    
    Init_Meter_Info_Struct();
    if(GELEX_Read_Info_Scale(&Get_Meter_Info,&GELEX_MI_ExtractDataFunc,&GELEX_MI_SendData,&GELEX_Fill_MInfo_Mess,GELEX_MeterInfoMessIDTable) != 1)
    {
        osDelay(5000);
        Init_Meter_Info_Struct();
        if(GELEX_Read_Info_Scale(&Get_Meter_Info,&GELEX_MI_ExtractDataFunc,&GELEX_MI_SendData,&GELEX_Fill_MInfo_Mess,GELEX_MeterInfoMessIDTable) != 1)
        {
            osDelay(5000);
            Init_Meter_Info_Struct();
            if(GELEX_Read_Info_Scale(&Get_Meter_Info,&GELEX_MI_ExtractDataFunc,&GELEX_MI_SendData,&GELEX_Fill_MInfo_Mess,GELEX_MeterInfoMessIDTable) != 1) 
              return 0;
        }
    }
    sDCU.Flag_ReadEventSVH = 1;
    GELEX_Read_Event(30);     //doc event TSVH
    sDCU.Flag_ReadEventSVH = 0;
    //
    Add_TuTI_toPayload(&Get_Meter_Info);
    //
    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
    Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) Get_Meter_Info.Numqty, 0);
    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')'; 
    
    Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.PosNumqty, &Str_Data_Write);
    //ETX
    *(Get_Meter_Info.Str_Payload.Data_a8 + Get_Meter_Info.Str_Payload.Length_u16++) = ETX; 
    //BBC
    Temp_BBC = BBC_Cacul(Get_Meter_Info.Str_Payload.Data_a8 + 1,Get_Meter_Info.Str_Payload.Length_u16 - 1);
    *(Get_Meter_Info.Str_Payload.Data_a8 + Get_Meter_Info.Str_Payload.Length_u16++) = Temp_BBC;
    
    Push_TSVH_toQueue(Type);
    return 1;
}


uint8_t GELEX_Read_Bill (void)
{    
    sDCU.FlagHave_BillMes = 0;   //
    
    Init_Meter_Billing_Struct();
    Init_MD_Bill();
    
    Init_Meter_TuTi_Struct();
    if(GELEX_Read_Info_Scale(&Get_Meter_TuTi,&GELEX_TuTi_ExtractDataFunc,&GELEX_TuTi_SendData,&GELEX_Fill_MTuTi_Mess,GELEX_MeterTuTiMessIDTable) != 1) 
    {   
        Init_Meter_TuTi_Struct();
        if(GELEX_Read_Info_Scale(&Get_Meter_TuTi,&GELEX_TuTi_ExtractDataFunc,&GELEX_TuTi_SendData,&GELEX_Fill_MTuTi_Mess,GELEX_MeterTuTiMessIDTable) != 1) 
        {
            Init_Meter_TuTi_Struct();
            if(GELEX_Read_Info_Scale(&Get_Meter_TuTi,&GELEX_TuTi_ExtractDataFunc,&GELEX_TuTi_SendData,&GELEX_Fill_MTuTi_Mess,GELEX_MeterTuTiMessIDTable) != 1) 
                return 0;
        }
    }
    
    Queue_Meter_Billing.Mess_Status_ui8 = 4;
    //Init OBIS list
    Get_Meter_Billing.OBIS_Pointer_ui8 = 0;
    Get_Meter_Billing.Num_OBIS_ui8 = 0;
    Get_Meter_Billing.Total_OBIS_ui8 = 0;
    Get_Meter_Billing.Last_Data_Buff_Pointer_ui16 = 10;
    Get_Meter_Billing.Reading_Block_ui8 = 0;
    //Read OBIS from GELEX
    GELEX_Insert_GetOBIS_Mess_OBIS(GELEX_MBilling_Mess_OBIS);
    if (GELEX_Read(&Get_Meter_Billing,GELEX_Get_OBIS_Template,1,&GELEX_MBillingGetRawData,&GELEX_MBillingExtractOBISList) != 1) return 0;
    osDelay(READ_DELAY);
    GELEX_Insert_GetBLOCK_Mess_OBIS(1);
    if (GELEX_Read(&Get_Meter_Billing,GELEX_Get_Block_Template,2,&GELEX_MBillingGetRawData,&GELEX_MBillingExtractOBISList) != 1) return 0;
    osDelay(READ_DELAY);
    GELEX_Insert_GetBLOCK_Mess_OBIS(2);
    if (GELEX_Read(&Get_Meter_Billing,GELEX_Get_Block_Template,2,&GELEX_MBillingGetRawData,&GELEX_MBillingExtractOBISList) != 1) return 0;
    osDelay(READ_DELAY);
    //Read billing data
    GELEX_MBillingInsertReadTime();
    GELEX_Prepare_Read_Date(GELEX_MBilling_StartTime,GELEX_MBilling_StopTime);
    GELEX_Insert_GetDATA_Mess_OBIS(GELEX_MBilling_Mess_OBIS);
    if (GELEX_Read(&Get_Meter_Billing,GELEX_Get_DATA_Template,4,&GELEX_MBillingGetRawData,&GELEX_MBillingExtractData) != 1) return 0;
    osDelay(READ_DELAY);
    
    //Read Maxdemand time
    Get_Meter_Billing.OBIS_Pointer_ui8 = 0;
    Get_Meter_Billing.Num_OBIS_ui8 = 0;
    Get_Meter_Billing.Total_OBIS_ui8 = 0;
    
    GELEX_Insert_GetOBIS_Mess_OBIS(GELEX_MBilling_Mess_OBIS_MDTime);
    if (GELEX_Read(&Get_Meter_Billing,GELEX_Get_OBIS_Template,1,&GELEX_MBillingGetRawData,&GELEX_MBillingExtractOBISList) != 1) return 0;
    osDelay(READ_DELAY);
    GELEX_Insert_GetDATA_Mess_OBIS(GELEX_MBilling_Mess_OBIS_MDTime);
    if (GELEX_Read(&Get_Meter_Billing,GELEX_Get_DATA_Template,4,&GELEX_MBillingGetRawData,&GELEX_MBillingExtractData) != 1) return 0;
    osDelay(READ_DELAY);

    while ((Get_Meter_Billing.Num_Block_ui8>0) && (sInformation.Flag_Stop_ReadBill == 0))
    {
        Init_MD_Bill();
        
        Get_Meter_Billing.Num_Block_ui8--;
        Get_Meter_Billing.Reading_Block_ui8++;
        
        Get_Meter_Billing.OBIS_Pointer_ui8 = 0;
        Get_Meter_Billing.Num_OBIS_ui8 = 0;
        Get_Meter_Billing.Total_OBIS_ui8 = 0;
        Get_Meter_Billing.Last_Data_Buff_Pointer_ui16 = 10;
        Get_Meter_Billing.Flag_Start_Pack = 0;
        
        Reset_Buff(&Get_Meter_Billing.Str_Payload); //khong init lai ca ca Get Bill
//        Reset_Buff(&Get_Meter_TuTi.Str_Payload);
        
        //Read OBIS from GELEX
        GELEX_Insert_GetOBIS_Mess_OBIS(GELEX_MBilling_Mess_OBIS);
        if (GELEX_Read(&Get_Meter_Billing,GELEX_Get_OBIS_Template,1,&GELEX_MBillingGetRawData,&GELEX_MBillingExtractOBISList) != 1) break;
        osDelay(READ_DELAY);
        GELEX_Insert_GetBLOCK_Mess_OBIS(1);
        if (GELEX_Read(&Get_Meter_Billing,GELEX_Get_Block_Template,2,&GELEX_MBillingGetRawData,&GELEX_MBillingExtractOBISList) != 1) break;
        osDelay(READ_DELAY);
        GELEX_Insert_GetBLOCK_Mess_OBIS(2);
        if (GELEX_Read(&Get_Meter_Billing,GELEX_Get_Block_Template,2,&GELEX_MBillingGetRawData,&GELEX_MBillingExtractOBISList) != 1) break;
        osDelay(READ_DELAY);
        //Read billing data
        GELEX_MBillingInsertReadTime();

        GELEX_Prepare_Read_Date(GELEX_MBilling_StartTime,GELEX_MBilling_StopTime);
        GELEX_Insert_GetDATA_Mess_OBIS(GELEX_MBilling_Mess_OBIS);
        if (GELEX_Read(&Get_Meter_Billing,GELEX_Get_DATA_Template,4,&GELEX_MBillingGetRawData_Dummy,&GELEX_MBillingExtractData_Dummy) != 1) break;
        osDelay(READ_DELAY);
        Billing_Num_Record_ui8 = Get_Meter_Billing.Reading_Block_ui8;
        while (Billing_Num_Record_ui8>1)
        {
            Billing_Num_Record_ui8--;
            GELEX_Insert_GetBLOCK_Mess_OBIS(Get_Meter_Billing.Reading_Block_ui8-Billing_Num_Record_ui8);
            if (GELEX_Read(&Get_Meter_Billing,GELEX_Get_Block_Template,2,&GELEX_MBillingGetRawData_Dummy,&GELEX_MBillingExtractData_Dummy) != 1) break;
            osDelay(READ_DELAY);
        }
        GELEX_Insert_GetBLOCK_Mess_OBIS(Get_Meter_Billing.Reading_Block_ui8);
        if (GELEX_Read(&Get_Meter_Billing,GELEX_Get_Block_Template,2,&GELEX_MBillingGetRawData,&GELEX_MBillingExtractData) != 1) break;
        osDelay(READ_DELAY);
        
        //Read Maxdemand time
        Get_Meter_Billing.OBIS_Pointer_ui8 = 0;
        Get_Meter_Billing.Num_OBIS_ui8 = 0;
        Get_Meter_Billing.Total_OBIS_ui8 = 0;
        
        GELEX_Insert_GetOBIS_Mess_OBIS(GELEX_MBilling_Mess_OBIS_MDTime);
        if (GELEX_Read(&Get_Meter_Billing,GELEX_Get_OBIS_Template,1,&GELEX_MBillingGetRawData,&GELEX_MBillingExtractOBISList) != 1) break;
        osDelay(READ_DELAY);
        GELEX_Insert_GetDATA_Mess_OBIS(GELEX_MBilling_Mess_OBIS_MDTime);
        if (GELEX_Read(&Get_Meter_Billing,GELEX_Get_DATA_Template,4,&GELEX_MBillingGetRawData_Dummy,&GELEX_MBillingExtractData_Dummy) != 1) break;
        osDelay(READ_DELAY);
        Billing_Num_Record_ui8 = Get_Meter_Billing.Reading_Block_ui8;
        while (Billing_Num_Record_ui8>1)
        {
            Billing_Num_Record_ui8--;
            GELEX_Insert_GetBLOCK_Mess_OBIS(Get_Meter_Billing.Reading_Block_ui8-Billing_Num_Record_ui8);
            if (GELEX_Read(&Get_Meter_Billing,GELEX_Get_Block_Template,2,&GELEX_MBillingGetRawData_Dummy,&GELEX_MBillingExtractData_Dummy) != 1) break;
            osDelay(READ_DELAY);
        }
        GELEX_Insert_GetBLOCK_Mess_OBIS(Get_Meter_Billing.Reading_Block_ui8);
        if (GELEX_Read(&Get_Meter_Billing,GELEX_Get_Block_Template,2,&GELEX_MBillingGetRawData,&GELEX_MBillingExtractData) != 1) break;
        osDelay(READ_DELAY);
    }
    
    if(sDCU.FlagHave_BillMes == 0)
    {
        Packet_Empty_MessHistorical();
    }
    sInformation.Flag_Request_Bill = 0;  //vi o day co 2 lan tinh lai thoi gian
    sInformation.Flag_Stop_ReadBill = 0;
    return 1;
}


uint8_t GELEX_Read_Event (uint32_t ForMin)
{
    uint32_t            TempSec = 0;
    ST_TIME_FORMAT   sTimeTemp;
    uint16_t        Total_record  = 0;

    if(ForMin != 30) 
    {
        Init_Meter_TuTi_Struct();
        if(GELEX_Read_Info_Scale(&Get_Meter_TuTi,&GELEX_TuTi_ExtractDataFunc,&GELEX_TuTi_SendData,&GELEX_Fill_MTuTi_Mess,GELEX_MeterTuTiMessIDTable) != 1) return 0;
    }
	
    Queue_Meter_Event.Mess_Status_ui8 = 4;
    TempSec = HW_RTC_GetCalendarValue_Second(sRTC, 0);
    Epoch_to_date_time(&sTimeTemp, TempSec - ForMin *60 *4*5, 0);
    
    Insert_Read_Time(GELEX_MEvent_StartTime,sTimeTemp.year,sTimeTemp.month,sTimeTemp.date,sTimeTemp.hour,sTimeTemp.min,sTimeTemp.sec);
    Insert_Read_Time(GELEX_MEvent_StopTime,sRTC.year,sRTC.month,sRTC.date,sRTC.hour,sRTC.min,sRTC.sec);
    GELEX_Prepare_Read_Date(GELEX_MEvent_StartTime,GELEX_MEvent_StopTime);
    //Tamper
    Init_Meter_Event_Struct();
    Get_Meter_Event.Reading_Block_ui8 = 0;
    Get_Meter_Event.Num_Record_ui16 = 0;
    Get_Meter_Event.Received_Record_ui16 = 0;
    osDelay(READ_DELAY);
    GELEX_Insert_GetDATA_Mess_OBIS(GELEX_MTamper_Mess_OBIS);
    if (GELEX_Read(&Get_Meter_Event,GELEX_Get_DATA_Template,4,&GELEX_MEventGetRawData,&MTamperExtractData) != 1) return 0;
    Total_record = Get_Meter_Event.Num_Record_ui16;
    while (Get_Meter_Event.Received_Record_ui16 < Total_record)
    {
        Get_Meter_Event.Reading_Block_ui8++;
        osDelay(READ_DELAY);
        GELEX_Insert_GetBLOCK_Mess_OBIS(Get_Meter_Event.Reading_Block_ui8);
        if (GELEX_Read(&Get_Meter_Event,GELEX_Get_Block_Template,2,&GELEX_MEventGetRawData,&MTamperExtractData) != 1) break;
    }

    //Alarm
    Init_Meter_Event_Struct();
    Get_Meter_Event.Reading_Block_ui8 = 0;	
    Get_Meter_Event.Num_Record_ui16 = 0;
    Get_Meter_Event.Received_Record_ui16 = 0;				
    osDelay(READ_DELAY);
    GELEX_Insert_GetDATA_Mess_OBIS(GELEX_MAlarm_Mess_OBIS);
    if (GELEX_Read(&Get_Meter_Event,GELEX_Get_DATA_Template,4,&GELEX_MEventGetRawData,&MAlarmExtractData) != 1)	return 0;
    Total_record = Get_Meter_Event.Num_Record_ui16;
    while (Get_Meter_Event.Received_Record_ui16 < Total_record)
    {
        Get_Meter_Event.Reading_Block_ui8++;
        osDelay(READ_DELAY);
        GELEX_Insert_GetBLOCK_Mess_OBIS(Get_Meter_Event.Reading_Block_ui8);
        if (GELEX_Read(&Get_Meter_Event,GELEX_Get_Block_Template,2,&GELEX_MEventGetRawData,&MAlarmExtractData) != 1)	break;
    }

    //Event
    Init_Meter_Event_Struct();
    Get_Meter_Event.Reading_Block_ui8 = 0;
    Get_Meter_Event.Num_Record_ui16 = 0;
    Get_Meter_Event.Received_Record_ui16 = 0;			
    osDelay(READ_DELAY);
    GELEX_Prepare_Read_Date(GELEX_MEvent_StartTime,GELEX_MEvent_StopTime);
    GELEX_Insert_GetDATA_Mess_OBIS(GELEX_MEvent_Mess_OBIS);
    if (GELEX_Read(&Get_Meter_Event,GELEX_Get_DATA_Template,4,&GELEX_MEventGetRawData,&GELEX_MEventExtractData) != 1)	return 0;
    Total_record = Get_Meter_Event.Num_Record_ui16;
    while (Get_Meter_Event.Received_Record_ui16 < Total_record)
    {
        Get_Meter_Event.Reading_Block_ui8++;
        osDelay(READ_DELAY);
        GELEX_Insert_GetBLOCK_Mess_OBIS(Get_Meter_Event.Reading_Block_ui8);
        if (GELEX_Read(&Get_Meter_Event,GELEX_Get_Block_Template,2,&GELEX_MEventGetRawData,&GELEX_MEventExtractData) != 1)	break;
    }
    return 1;
}

uint8_t GELEX_Read_Lpf (void)
{
    uint16_t        TempYear=0;
    sDCU.FlagHave_ProfMess = 0;
    
    GE_Fl_Alow_GetTimeLpf = 0; 
    Init_Meter_TuTi_Struct();
    if(GELEX_Read_Info_Scale(&Get_Meter_TuTi,&GELEX_TuTi_ExtractDataFunc,&GELEX_TuTi_SendData,&GELEX_Fill_MTuTi_Mess,GELEX_MeterTuTiMessIDTable) != 1) 
    {   
        Init_Meter_TuTi_Struct();
        if(GELEX_Read_Info_Scale(&Get_Meter_TuTi,&GELEX_TuTi_ExtractDataFunc,&GELEX_TuTi_SendData,&GELEX_Fill_MTuTi_Mess,GELEX_MeterTuTiMessIDTable) != 1) 
        {
            Init_Meter_TuTi_Struct();
            if(GELEX_Read_Info_Scale(&Get_Meter_TuTi,&GELEX_TuTi_ExtractDataFunc,&GELEX_TuTi_SendData,&GELEX_Fill_MTuTi_Mess,GELEX_MeterTuTiMessIDTable) != 1) 
                return 0;
        }
    }
    
    Init_Meter_LProf_Struct();    
    if(sInformation.Flag_Request_lpf == 1)
    {
        //Day
        GELEX_MLProfile_StartTime[3] = sInformation.StartTime_GetLpf.date;
        GELEX_MLProfile_StopTime[3] = sInformation.EndTime_GetLpf.date;
        //Month
        GELEX_MLProfile_StartTime[2] = sInformation.StartTime_GetLpf.month;
        GELEX_MLProfile_StopTime[2] = sInformation.EndTime_GetLpf.month;
        //Year
        TempYear = sInformation.StartTime_GetLpf.year;
        TempYear = TempYear + 2000;
        GELEX_MLProfile_StartTime[0] = (uint8_t)((TempYear>>8)&0x00FF);
        GELEX_MLProfile_StartTime[1] = (uint8_t)(TempYear&0x00FF);
        
        TempYear = sInformation.EndTime_GetLpf.year;
        TempYear = TempYear + 2000;
        GELEX_MLProfile_StopTime[0] = (uint8_t)((TempYear>>8)&0x00FF);
        GELEX_MLProfile_StopTime[1] = (uint8_t)(TempYear&0x00FF);	
        sInformation.Flag_Request_lpf = 0;
    }else
    {
        //Day
        GELEX_MLProfile_StartTime[3] = Read_Meter_LProf_Day[0];
        GELEX_MLProfile_StopTime[3] = Read_Meter_LProf_Day[0];
        //Month
        GELEX_MLProfile_StartTime[2] = Read_Meter_LProf_Day[1];
        GELEX_MLProfile_StopTime[2] = Read_Meter_LProf_Day[1];
        //Year
        TempYear = Read_Meter_LProf_Day[2];
        TempYear = TempYear + 2000;
        GELEX_MLProfile_StartTime[0] = (uint8_t)((TempYear>>8)&0x00FF);
        GELEX_MLProfile_StartTime[1] = (uint8_t)(TempYear&0x00FF);
        
        GELEX_MLProfile_StopTime[0] = (uint8_t)((TempYear>>8)&0x00FF);
        GELEX_MLProfile_StopTime[1] = (uint8_t)(TempYear&0x00FF);	
    }
    
    //Read channel 1=====================================================================================
    //Init OBIS list
    Get_Meter_LProf.OBIS_Pointer_ui8 = 0;
    Get_Meter_LProf.Num_OBIS_ui8 = 0;
    Get_Meter_LProf.Total_OBIS_ui8 = 0;	
    Get_Meter_LProf.Reading_Block_ui8 = 0;
    Get_Meter_LProf.Flag_ui32 = 1;
    //Read Period
    GELEX_Insert_GetPeriod_Mess_OBIS(GELEX_MLProfile_Mess_OBIS_Channel1);
    GELEX_READ_1REG(GELEX_Get_Period_Template, &GELEX_MLProfileGetPeriod);
    //Read OBIS from GELEX
    GELEX_Insert_GetOBIS_Mess_OBIS(GELEX_MLProfile_Mess_OBIS_Channel1);
    if (GELEX_Read(&Get_Meter_LProf,GELEX_Get_OBIS_Template,1,&GELEX_MLProfileGetRawData,&GELEX_MLProfileExtractOBISList) != 1) return 0;
    osDelay(READ_DELAY);
    //Read Data
    GELEX_Prepare_Read_Date(GELEX_MLProfile_StartTime,GELEX_MLProfile_StopTime);
    GELEX_Insert_GetDATA_Mess_OBIS(GELEX_MLProfile_Mess_OBIS_Channel1);
    if (GELEX_Read(&Get_Meter_LProf,GELEX_Get_DATA_Template,4,&GELEX_MLProfileGetRawData,&GELEX_MLProfileExtractData) != 1) return 0;
    
    if (((Get_Meter_LProf.Num_Record_ui16%3) == 0) && (Get_Meter_LProf.Num_Record_ui16 >3))
        Get_Meter_LProf.Num_Block_ui8 = Get_Meter_LProf.Num_Record_ui16/3 -1;
    else
        Get_Meter_LProf.Num_Block_ui8 = Get_Meter_LProf.Num_Record_ui16/3;
    
    Get_Meter_LProf.Reading_Block_ui8 = 1;
    while ((Get_Meter_LProf.Reading_Block_ui8 <= Get_Meter_LProf.Num_Block_ui8) &&(sInformation.Flag_Stop_ReadLpf == 0))
    {	
        osDelay(READ_DELAY);
        GELEX_Insert_GetBLOCK_Mess_OBIS(Get_Meter_LProf.Reading_Block_ui8);
        if (GELEX_Read(&Get_Meter_LProf,GELEX_Get_Block_Template,2,&GELEX_MLProfileGetRawData,&GELEX_MLProfileExtractData) != 1) break;
        Get_Meter_LProf.Reading_Block_ui8++;
    };
    osDelay(READ_DELAY);
    
    //Read channel 2=====================================================================================
    //Init OBIS list
    Init_Meter_LProf_Struct();  
    
    Get_Meter_LProf.OBIS_Pointer_ui8 = 0;
    Get_Meter_LProf.Num_OBIS_ui8 = 0;
    Get_Meter_LProf.Total_OBIS_ui8 = 0;
    Get_Meter_LProf.Reading_Block_ui8 = 0;
    Get_Meter_LProf.Flag_ui32 = 2;
    //Read Period
    GELEX_Insert_GetPeriod_Mess_OBIS(GELEX_MLProfile_Mess_OBIS_Channel2);
    GELEX_READ_1REG(GELEX_Get_Period_Template, &GELEX_MLProfileGetPeriod);
    //Read OBIS
    GELEX_Insert_GetOBIS_Mess_OBIS(GELEX_MLProfile_Mess_OBIS_Channel2);
    if (GELEX_Read(&Get_Meter_LProf,GELEX_Get_OBIS_Template,1,&GELEX_MLProfileGetRawData,&GELEX_MLProfileExtractOBISList) != 1) return 0;
    osDelay(READ_DELAY);
    //Read Data
    GELEX_Prepare_Read_Date(GELEX_MLProfile_StartTime,GELEX_MLProfile_StopTime);
    GELEX_Insert_GetDATA_Mess_OBIS(GELEX_MLProfile_Mess_OBIS_Channel2);
    if (GELEX_Read(&Get_Meter_LProf,GELEX_Get_DATA_Template,4,&GELEX_MLProfileGetRawData,&GELEX_MLProfileExtractData) != 1) return 0;
    
    if (((Get_Meter_LProf.Num_Record_ui16%3) == 0)&& (Get_Meter_LProf.Num_Record_ui16 >3))
        Get_Meter_LProf.Num_Block_ui8 = Get_Meter_LProf.Num_Record_ui16/3 -1;
    else
        Get_Meter_LProf.Num_Block_ui8 = Get_Meter_LProf.Num_Record_ui16/3;	
    Get_Meter_LProf.Reading_Block_ui8 = 1;
    while ((Get_Meter_LProf.Reading_Block_ui8 <= Get_Meter_LProf.Num_Block_ui8) &&(sInformation.Flag_Stop_ReadLpf == 0))
    {	
        osDelay(READ_DELAY);
        GELEX_Insert_GetBLOCK_Mess_OBIS(Get_Meter_LProf.Reading_Block_ui8);
        if (GELEX_Read(&Get_Meter_LProf,GELEX_Get_Block_Template,2,&GELEX_MLProfileGetRawData,&GELEX_MLProfileExtractData) != 1) break;
        Get_Meter_LProf.Reading_Block_ui8++;
    };
    
    if(sDCU.FlagHave_ProfMess == 0)
    {
        _fSend_Empty_Lpf();
    }
    sInformation.Flag_Stop_ReadLpf = 0;
    return 1;
}

uint8_t GELEX_Read_Infor (void)
{
    Init_Meter_TuTi_Struct();
    if(GELEX_Read_Info_Scale(&Get_Meter_TuTi,&GELEX_TuTi_ExtractDataFunc,&GELEX_TuTi_SendData,&GELEX_Fill_MTuTi_Mess,GELEX_MeterTuTiMessIDTable) == 1)
    {
        Pack_PushData_103_Infor_Meter();
        return 1;
    }
    return 0;
}

uint8_t GELEX_Connect_meter_Gelex(void)
{
	uint8_t ReadIDRetry = 1;

	GELEX_Fill_Meter_Addr();
	
	while (ReadIDRetry>0)
	{
        osDelay(500);
        if (osMutexWait(mtFlashMeterHandle,1000) == osOK)
		{
			Read_Meter_ID_Success = GELEX_Get_Meter_ID_Scale();
            
            if (osMutexRelease(mtFlashMeterHandle) != osOK)
                osMutexRelease(mtFlashMeterHandle);
            
            if (Read_Meter_ID_Success == 1)
                break;
            ReadIDRetry--;
		}
	}
	
	if (Read_Meter_ID_Success == 0)
	{//Reinit uart2
		UART_METER.Instance = UART__METER;
		UART_METER.Init.BaudRate = UART2_BAUR_2;
		UART_METER.Init.WordLength = UART2_DATALENGTH;
		UART_METER.Init.StopBits = UART_STOPBITS_1;
		UART_METER.Init.Parity = UART2_PARITY1;
		UART_METER.Init.Mode = UART_MODE_TX_RX;
		UART_METER.Init.HwFlowCtl = UART_HWCONTROL_NONE;
		UART_METER.Init.OverSampling = UART_OVERSAMPLING_16;
		UART_METER.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE ;
		UART_METER.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
		HAL_UART_Init(&UART_METER);
		Init_UART2();
		__HAL_UART_ENABLE_IT(&UART_METER, UART_IT_RXNE);
		osDelay(1000);
		
		ReadIDRetry = 1;
		
		while (ReadIDRetry>0)
		{
            osDelay(1000);
			if (osMutexWait(mtFlashMeterHandle,1000) == osOK)
			{
				Read_Meter_ID_Success = GELEX_Get_Meter_ID_Scale();
                
				if (osMutexRelease(mtFlashMeterHandle) != osOK)
					osMutexRelease(mtFlashMeterHandle);
                
                if (Read_Meter_ID_Success == 1)
                    break;
                ReadIDRetry--;
			}
		}
	}
    if (Read_Meter_ID_Success == 1)
        return 1;
    else return 0;
}


/*
 * 			FUNCTION
 */


void GELEX_Fill_Meter_Addr (void)
{
	//GELEX_SNRM
	GELEX_Cal_Header_CheckSum(GELEX_SNRM,1,5);
	
	//GELEX_AARQ
	GELEX_Cal_Header_CheckSum(GELEX_AARQ,1,5);
	GELEX_Cal_Full_CheckSum(GELEX_AARQ,1,70);
	
	//GELEX_MInfo_MTemp
	GELEX_MInfo_MTemp[4] = METER_RS485_ADDR;
	
	//GELEX_NextFrame
	GELEX_NextFrame[4] = METER_RS485_ADDR;
	
	//GELEX_Read_END
	GELEX_Cal_Header_CheckSum(GELEX_Read_END,1,5);
	
	//GELEX_Get_OBIS_Template
	GELEX_Cal_Header_CheckSum(GELEX_Get_OBIS_Template,1,5);
    
    //GELEX_Get_Period_Template
	GELEX_Cal_Header_CheckSum(GELEX_Get_Period_Template,1,5);
	
	//GELEX_Get_Block_Template
	GELEX_Cal_Header_CheckSum(GELEX_Get_Block_Template,1,5);
	
	//GELEX_Get_DATA_Template
	GELEX_Cal_Header_CheckSum(GELEX_Get_DATA_Template,1,5);
}

void GELEX_Cal_Header_CheckSum (uint8_t *MessTemp,uint8_t Start_Pos,uint8_t End_Pos)
{
	uint16_t Temp_CheckSum=0;
	
	*(MessTemp+4) = METER_RS485_ADDR;
	Temp_CheckSum = CountFCS16(MessTemp,Start_Pos,End_Pos);
	*(MessTemp+6) = (uint8_t)((Temp_CheckSum>>8)&0x00FF);
	*(MessTemp+7) = (uint8_t)(Temp_CheckSum&0x00FF);	
}
void GELEX_Cal_Full_CheckSum (uint8_t *MessTemp,uint8_t Start_Pos,uint8_t End_Pos)
{
	uint16_t Temp_CheckSum=0;
	
	Temp_CheckSum = CountFCS16(MessTemp,Start_Pos,End_Pos);
	*(MessTemp+End_Pos+1) = (uint8_t)((Temp_CheckSum>>8)&0x00FF);
	*(MessTemp+End_Pos+2) = (uint8_t)(Temp_CheckSum&0x00FF);	
}

void GELEX_Insert_GetOBIS_Mess_OBIS (uint8_t *Mess_OBIS)
{
	uint8_t i=0;
	
	for (i=0;i<6;i++)
		GELEX_Get_OBIS_Template[16+i] = *(Mess_OBIS+i);
	
	GELEX_Cal_Full_CheckSum(GELEX_Get_OBIS_Template,1,23);
}

void GELEX_Insert_GetPeriod_Mess_OBIS (uint8_t *Mess_OBIS)
{
	uint8_t i=0;
	
	for (i=0;i<6;i++)
		GELEX_Get_Period_Template[16+i] = *(Mess_OBIS+i);
	
	GELEX_Cal_Full_CheckSum(GELEX_Get_Period_Template,1,23);
}

void GELEX_Insert_GetOBIS_Mess_OBIS_Special (uint8_t *Mess_OBIS)
{
	uint8_t i=0;
	
	for (i=0;i<6;i++)
		GELEX_Get_OBIS_Template_Special[16+i] = *(Mess_OBIS+i);
	
	GELEX_Cal_Full_CheckSum(GELEX_Get_OBIS_Template_Special,1,23);
}

void GELEX_Insert_GetDATA_Mess_OBIS (uint8_t *Mess_OBIS)
{
	uint8_t i=0;
	
	for (i=0;i<6;i++)
		GELEX_Get_DATA_Template[16+i] = *(Mess_OBIS+i);
	
	GELEX_Cal_Full_CheckSum(GELEX_Get_DATA_Template,1,74);
}

void GELEX_Insert_GetBLOCK_Mess_OBIS (uint8_t Block_no)
{
	GELEX_Get_Block_Template[17] = Block_no;
	
	GELEX_Cal_Full_CheckSum(GELEX_Get_Block_Template,1,17);
}

void GELEX_Mess_Checksum(void)
{
	uint16_t	GenCheckSum=0,MesCheckSum=0;
	
	GenCheckSum = CountFCS16(UART1_Receive_Buff,1,UART1_Control.Mess_Length_ui16-3);
	MesCheckSum = UART1_Receive_Buff[UART1_Control.Mess_Length_ui16-2];
	MesCheckSum = (MesCheckSum << 8) + UART1_Receive_Buff[UART1_Control.Mess_Length_ui16-1];
	
	if (GenCheckSum == MesCheckSum)
	{
		UART1_Control.Mess_Status_ui8 = 2;
	}
	else
		UART1_Control.Mess_Status_ui8 = 4;
}


//                                                                                                                
uint8_t GELEX_Get_Meter_ID_Scale (void)
{
	uint8_t mReVal=0;
	Init_Meter_Scale_Struct();
    
	mReVal = GELEX_Read_Info_Scale(&Get_Meter_Scale,&GELEX_MS_ExtractDataFunc,&GELEX_MS_SendData,&GELEX_Fill_Scale_Mess,GELEX_GetScale_Code_Table);
	
	return mReVal;
}


uint8_t GELEX_Read_Info_Scale (Meter_Comm_Struct *Meter_Comm,uint8_t (*FuncExtractRawData)(void),void (*FuncDataHandle)(void),void (*FuncFillMess)(void),uint8_t *CodeTable)
{
	uint8_t mReVal = 0;
	
	//Start read
	Meter_Comm->Reading_ui8 = 1;
	Meter_Comm->ID_Frame_ui8 = 0x32;

	while(Meter_Comm->Reading_ui8 == 1) 
    {
		switch(Meter_Comm->Step_ui8) {
			case 0:
				Meter_Comm->Total_Mess_Sent_ui32++;
				//Wait for respond message from Meter in 2000ms: checksum - extract raw data - save to buffer - goto next message
				if (osSemaphoreWait(bsUART2PendingMessHandle,5000) == osOK) {
					//Checksum
					GELEX_Mess_Checksum();
					if(Meter_Comm->Mess_Step_ui8 == 0)
						if(UART1_Receive_Buff[5] != 0x73) // SNRM
							UART1_Control.Mess_Status_ui8 = 4;
					if(Meter_Comm->Mess_Step_ui8 == 1)
						if(UART1_Receive_Buff[5] != 0x30) // AARQ
							UART1_Control.Mess_Status_ui8 = 4;
					if (UART1_Control.Mess_Status_ui8 == 2) {
						Meter_Comm->Success_Read_Mess_ui32++;
						//Extract raw data
						if (Meter_Comm->Mess_Step_ui8 > 1)
						{
							if(FuncExtractRawData() == 0){
								Meter_Comm->Step_ui8 = 2; // break
								Meter_Comm->Mess_Step_ui8 = 0;
							}
							else
								Meter_Comm->Step_ui8 = 1;
						}
						else
							Meter_Comm->Step_ui8 = 1;
						//Goto next message
						Meter_Comm->Mess_Step_ui8++;
						Meter_Comm->Error_ui8 = 0x00;
						Meter_Comm->Error_Wrong_Mess_Format_ui32 = 0x00;
						Meter_Comm->Error_Meter_Norespond_ui32 = 0x00;
						//If all mess sent -> finish
						if (Meter_Comm->Mess_Step_ui8 > 1){
							if ((*(CodeTable+Meter_Comm->Mess_Step_ui8-2)) == 0xFF) {
								//Push mess or check alarm
								FuncDataHandle();
								mReVal = 1;
								Meter_Comm->Step_ui8 = 2;
							}
						}
                        Init_UART2();
					} else {
						Meter_Comm->Error_Wrong_Mess_Format_ui32++;
						//If Meter respond wrong, jump out of reading cycle
						Meter_Comm->Step_ui8 = 2;
					}
				} else {//If Meter no respond after 2000ms: wait 2000ms - resend the message (3 times) 
					Meter_Comm->Error_Meter_Norespond_ui32++;
					//If Meter norespond, jump out of reading cycle
					Meter_Comm->Step_ui8 = 2;
				}
				break;
			case 1:
				Init_UART2();
				if (Meter_Comm->Mess_Step_ui8 == 0) {
					RS485_SEND;
					osDelay(READ_DELAY);
					HAL_UART_Transmit(&UART_METER,&GELEX_SNRM[0],9,1000);
                    osDelay(READ_DELAY);
					RS485_RECIEVE;
				} else if (Meter_Comm->Mess_Step_ui8 == 1) {
					RS485_SEND;
					osDelay(READ_DELAY);
					HAL_UART_Transmit(&UART_METER,&GELEX_AARQ[0],74,1000);
					RS485_RECIEVE;
				} else {
					RS485_SEND;
					osDelay(READ_DELAY);					
					FuncFillMess();
					HAL_UART_Transmit(&UART_METER,&GELEX_MInfo_MTemp[0],27,1000);
					RS485_RECIEVE;
				}
				Meter_Comm->Step_ui8 = 0;
				break;
			case 2:
				//Reset UART2 status
				UART1_Control.Mess_Pending_ui8 = 1; // Not wait semaphore, not check
				RS485_SEND;
				osDelay(READ_DELAY);
				HAL_UART_Transmit(&UART_METER,&GELEX_Read_END[0],9,1000);
				RS485_RECIEVE;
				//Jump out of reading cycle, wait for next cycle (after 30mins)
				Meter_Comm->Step_ui8 = 1;
				Meter_Comm->Mess_Step_ui8 = 0;
				//Reset pointer
				Meter_Comm->Data_Buff_Pointer_ui16 = 10;
				Meter_Comm->Reading_ui8 = 0;
				osDelay(5000);
				break;					
			default:
				break;				
		}
	}
	return mReVal;
}


void GELEX_Fill_MInfo_Mess (void)
{
	uint8_t 	i = 0,j=0;
	uint16_t	TempCheckSum = 0;
	
	//Insert ID Frame
	GELEX_MInfo_MTemp[5] = Get_Meter_Info.ID_Frame_ui8;
	//Calculate Header checksum
	TempCheckSum = CountFCS16(&GELEX_MInfo_MTemp[0],1,5);
	//Insert Header checksum
	GELEX_MInfo_MTemp[6] = (uint8_t)((TempCheckSum>>8)&0x00FF);
	GELEX_MInfo_MTemp[7] = (uint8_t)(TempCheckSum&0x00FF);
	//Insert Class ID + OBIS + Data Type
	j = GELEX_MeterInfoMessIDTable[Get_Meter_Info.Mess_Step_ui8-2]; 
	for (i=0;i<10;i++)
		GELEX_MInfo_MTemp[i+14] = GELEX_MInfo_Table[j][i+1];
	//Calculate Total checksum
	TempCheckSum = CountFCS16(&GELEX_MInfo_MTemp[0],1,23);
	//Insert Total checksum
	GELEX_MInfo_MTemp[24] = (uint8_t)((TempCheckSum>>8)&0x00FF);
	GELEX_MInfo_MTemp[25] = (uint8_t)(TempCheckSum&0x00FF);
	
	//Calculate next ID Frame
	Get_Meter_Info.ID_Frame_ui8 += 0x22;
	if (Get_Meter_Info.ID_Frame_ui8 == 0x20)
		Get_Meter_Info.ID_Frame_ui8 = 0x10;
}

void GELEX_Fill_MTuTi_Mess (void)
{
	uint8_t 	i = 0,j=0;
	uint16_t	TempCheckSum = 0;
	
	//Insert ID Frame
	GELEX_MInfo_MTemp[5] = Get_Meter_TuTi.ID_Frame_ui8;
	//Calculate Header checksum
	TempCheckSum = CountFCS16(&GELEX_MInfo_MTemp[0],1,5);
	//Insert Header checksum
	GELEX_MInfo_MTemp[6] = (uint8_t)((TempCheckSum>>8)&0x00FF);
	GELEX_MInfo_MTemp[7] = (uint8_t)(TempCheckSum&0x00FF);
	//Insert Class ID + OBIS + Data Type
	j = GELEX_MeterTuTiMessIDTable[Get_Meter_TuTi.Mess_Step_ui8-2]; 
	for (i=0;i<10;i++)
		GELEX_MInfo_MTemp[i+14] = GELEX_MInfo_Table[j][i+1];
	//Calculate Total checksum
	TempCheckSum = CountFCS16(&GELEX_MInfo_MTemp[0],1,23);
	//Insert Total checksum
	GELEX_MInfo_MTemp[24] = (uint8_t)((TempCheckSum>>8)&0x00FF);
	GELEX_MInfo_MTemp[25] = (uint8_t)(TempCheckSum&0x00FF);
	
	//Calculate next ID Frame
	Get_Meter_TuTi.ID_Frame_ui8 += 0x22;
	if (Get_Meter_TuTi.ID_Frame_ui8 == 0x20)
		Get_Meter_TuTi.ID_Frame_ui8 = 0x10;
}
int8_t GELEX_Check_Row_Obis103 (struct_Obis_Scale* structObis, uint8_t ObisHex)
{
    uint8_t  i = 0;
    
    for(i = 0; i < GELEX_MAX_OBIS_INTAN; i++)
      if(structObis[i].ObisHex == ObisHex) return i; 
    
    return -1;
}

uint8_t GELEX_MI_ExtractDataFunc(void)
{
	uint8_t     Data_type=0,i=0,j=0;
	uint8_t     Return = 1;
    uint32_t    Temp_data_32 = 0;
    uint16_t    Temp_data_16 = 0;
    uint8_t         BuffNum[30];
    truct_String    Str_Data_Write={&BuffNum[0], 0};
    uint8_t         ScaleRead = 1;
    ST_TIME_FORMAT  sTime_temp;
    uint16_t        Tempyear = 0;
    uint8_t         Div = 0;
    int8_t          RowObis103 = 0;
    uint8_t         ObisHex = 0;
    uint64_t        Mul = 1;
    
    if(Get_Meter_Info.Flag_Start_Pack == 0)
    {
        Reset_Buff(&Get_Meter_Info.Str_Payload);
        //wrire header TSVH vao
        Write_Header_TSVH_Push103();
        Get_Meter_Info.Flag_Start_Pack = 1;
    }  
    
	Data_type = UART1_Receive_Buff[15];
	j = GELEX_MeterInfoMessIDTable[Get_Meter_Info.Mess_Step_ui8-2];
    
	switch (Data_type)
	{
		case 0x06:     //06:  uint 32 ( 4 byte Data)  d?i gi� tr? t? hex -> decimal
            ObisHex = GELEX_MInfo_Table[j][0];
            RowObis103 = GELEX_Check_Row_Obis103(&Obis_Gelex_Ins[0], ObisHex);
            if(RowObis103 >=0)
            {
                if(Obis_Gelex_Ins[RowObis103].Str_Obis != NULL)
                {
                    //ghi obis vao
                    Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Obis_Inbuff, Obis_Gelex_Ins[RowObis103].Str_Obis);
                    Get_Meter_Info.Pos_Obis_Inbuff += Obis_Gelex_Ins[RowObis103].Str_Obis->Length_u16;
                    Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
                    Get_Meter_Info.Numqty++;
                    //ghi data
                    for (i=0;i<4;i++)
                        Temp_data_32 = (Temp_data_32 << 8) | UART1_Receive_Buff[16+i];
                    ScaleRead = GELEX_ConvertScaleMeter_toDec (Obis_Gelex_Ins[RowObis103].Scale_ReadMeter, &Div);
                    for(j = 0; j < ScaleRead; j++)
                        Mul = Mul *10;
                    //o d�y l� so khong dau
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                    Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) (Temp_data_32 * Mul), Div + Obis_Gelex_Ins[RowObis103].scale); 
                    
                    if(Obis_Gelex_Ins[RowObis103].Str_Unit != NULL)
                    {
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                        Copy_String_2(&Str_Data_Write, Obis_Gelex_Ins[RowObis103].Str_Unit);  //don vi
                    }
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                    
                    Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Data_Inbuff, &Str_Data_Write);
                    Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
                }
            }
			
			break;
		case 0x09:   //thoi gian. sTime, Maxdemand: chi cat 8 byte d�u tien: 2 byte nam/ thang/ ngay/week/gio /phut/ giay
            ObisHex = GELEX_MInfo_Table[j][0];
			if((ObisHex == 0x01)&&(UART1_Receive_Buff[16] != 0x0C)) 
				Return = 0;
            if(UART1_Receive_Buff[16] >= 0x08)
            {
                //chia ra 2 truong hop: 1: la stime meter. 2: stime cua MaxDemand
                Tempyear         = (UART1_Receive_Buff[17] << 8) | (UART1_Receive_Buff[18]);
                sTime_temp.year  = Tempyear%100;
                sTime_temp.month = UART1_Receive_Buff[19];
                sTime_temp.date  = UART1_Receive_Buff[20];
                sTime_temp.day   = UART1_Receive_Buff[21];
                sTime_temp.hour  = UART1_Receive_Buff[22];
                sTime_temp.min   = UART1_Receive_Buff[23];
                sTime_temp.sec   = UART1_Receive_Buff[24];
                
                if(ObisHex== 0x01) //stime meter
                {
//                    Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Obis_Inbuff,  &He_So_Nhan);
//                    Get_Meter_Info.Pos_Obis_Inbuff += He_So_Nhan.Length_u16;
//                    Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
//                    Get_Meter_Info.Numqty++;
//                    Reset_Buff(&Str_Data_Write);
//                    
//                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
//                    Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) sDCU.He_So_Nhan, GELEX_SCALE_HE_SO_NHAN); 
//                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
//                    
//                    Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Data_Inbuff, &Str_Data_Write);
//                    Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
            
                    Reset_Buff(&Str_Data_Write);
                    //ghi stime vao vi tri Pos numqty. Sau do ghi numqty.
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
                    Copy_String_STime(&Str_Data_Write,sTime_temp);
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')'; 
                    
//                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
//                    Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) Get_Meter_Info.Numqty, 0);  //comment day de dong goi tiep event
//                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')'; 
//                    
                    Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.PosNumqty, &Str_Data_Write);  
                    Get_Meter_Info.PosNumqty += Str_Data_Write.Length_u16;
                    Get_Meter_Info.Pos_Obis_Inbuff += Str_Data_Write.Length_u16;
                    Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
                }else if ((ObisHex == 0x32) || (ObisHex == 0x36) || (ObisHex == 0x3A))
                {
                    //chi dong doi data time cua maxdend
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
                    Copy_String_STime(&Str_Data_Write,sTime_temp);
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                    Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Data_Inbuff, &Str_Data_Write);
                    Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
                }
            }		
			break;
		case 0x10:   //dai luong khong dau
		case 0x12:   //12: uint 16 (2 byte) doi gi� tri tu hex -> decimal: trong file word chia 10: int, 12 uint.: ddien ap pf, tan so 
            ObisHex = GELEX_MInfo_Table[j][0];
            RowObis103 = GELEX_Check_Row_Obis103(&Obis_Gelex_Ins[0], ObisHex);
            if(RowObis103 >=0)
            {
                if(Obis_Gelex_Ins[RowObis103].Str_Obis != NULL)
                {
                    //ghi obis vao
                    Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Obis_Inbuff, Obis_Gelex_Ins[RowObis103].Str_Obis);
                    Get_Meter_Info.Pos_Obis_Inbuff += Obis_Gelex_Ins[RowObis103].Str_Obis->Length_u16;
                    Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
                    Get_Meter_Info.Numqty++;
                    for (i=0;i<2;i++)
                        Temp_data_16 = (Temp_data_16 << 8) | UART1_Receive_Buff[16+i];
                    
                    ScaleRead = GELEX_ConvertScaleMeter_toDec (Obis_Gelex_Ins[RowObis103].Scale_ReadMeter, &Div);
                    for(j = 0; j < ScaleRead; j++)
                        Mul = Mul *10;
                    //data
                    if((ObisHex >= 0x0F) && (ObisHex <= 0x19))   //co dau
                    {
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                        Pack_HEXData_Frame(&Str_Data_Write, (int64_t) (Convert_uint16_2int16 (Temp_data_16) *Mul) ,Div + Obis_Gelex_Ins[RowObis103].scale);
                        if(Obis_Gelex_Ins[RowObis103].Str_Unit != NULL)
                        {
                            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                            Copy_String_2(&Str_Data_Write, Obis_Gelex_Ins[RowObis103].Str_Unit);  //don vi
                        }
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                            
                        Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Data_Inbuff, &Str_Data_Write);
                        Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
                    }else //khong dau
                    {
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                        Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t)(Temp_data_16 *Mul) ,Div + Obis_Gelex_Ins[RowObis103].scale);
                        if(Obis_Gelex_Ins[RowObis103].Str_Unit != NULL)
                        {
                            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                            Copy_String_2(&Str_Data_Write, Obis_Gelex_Ins[RowObis103].Str_Unit);  //don vi
                        }
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                        
                        Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Data_Inbuff, &Str_Data_Write);
                        Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
                    }
                }
            }else
            {
                //chi lay Tim va Tum: chi ghi /value  vao vi tri posdata - 1
                if((ObisHex == 0x64) || (ObisHex == 0x65))
                {
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++)= '/';
                    for (i=0;i<2;i++)
                        Temp_data_16 = (Temp_data_16 << 8) | UART1_Receive_Buff[16+i];
                         
                    Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) (Temp_data_16/GELEX_SCALE_TUM_TIM), 0); 
                    Copy_String_toTaget(&Get_Meter_Info.Str_Payload, (Get_Meter_Info.Pos_Data_Inbuff - 1), &Str_Data_Write);   //ghi vao pos - 1
                    Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
                }
            }
            
			break;
		case 0x17:   //17 : Float 32 (4 byte) � c�ch chuy?n xem t?i 
            //ghi obis vao
            ObisHex = GELEX_MInfo_Table[j][0];
            RowObis103 = GELEX_Check_Row_Obis103(&Obis_Gelex_Ins[0], ObisHex);
            if(RowObis103 >=0)
            {
                if(Obis_Gelex_Ins[RowObis103].Str_Obis != NULL)
                {
                    Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Obis_Inbuff, Obis_Gelex_Ins[RowObis103].Str_Obis);
                    Get_Meter_Info.Pos_Obis_Inbuff += Obis_Gelex_Ins[RowObis103].Str_Obis->Length_u16;
                    Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
                    Get_Meter_Info.Numqty++;
                    //ghi data
                    for (i=0;i<4;i++)
                        Temp_data_32 = (Temp_data_32 << 8) | UART1_Receive_Buff[19-i];
                    
                    ScaleRead = GELEX_ConvertScaleMeter_toDec (Obis_Gelex_Ins[RowObis103].Scale_ReadMeter, &Div);
                    for(j = 0; j < ScaleRead; j++)
                        Mul = Mul *10;
                    
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                    Pack_HEXData_Frame(&Str_Data_Write, (int64_t)(Convert_float_2int(Temp_data_32, 0) * Mul), Div + Obis_Gelex_Ins[RowObis103].scale);
                    if(Obis_Gelex_Ins[RowObis103].Str_Unit != NULL)
                    {
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                        Copy_String_2(&Str_Data_Write, Obis_Gelex_Ins[RowObis103].Str_Unit);  //don vi
                    }
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                        
                    Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Data_Inbuff, &Str_Data_Write);
                    Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
                }
            }
			break;
		default:
			Return = 0;
			break;
	}
	return Return;
}

void GELEX_MI_SendData(void)
{
	
	
}

void GELEX_MI_SendData_Inst(void)
{

}

                                                                                                             

uint8_t GELEX_MS_ExtractDataFunc(void)
{
	uint8_t Data_type=0,i=0,j=0;
	uint8_t Temp_MeterID[10];
    int8_t  Row = 0;
    
	j = GELEX_GetScale_Code_Table[Get_Meter_Scale.Mess_Step_ui8-2];
	Data_type = UART1_Receive_Buff[15];
    
	switch (Data_type)
	{
		case 0x09:
			for (i=0;i<UART1_Receive_Buff[16];i++)
				Temp_MeterID[i] = UART1_Receive_Buff[17+i];

			if (sDCU.sMeter_id_now.Length_u16 != 8)
				sDCU.Status_Meter_u8 = 1;
			for (i=0;i<8;i++)
			{
				if ((*(sDCU.sMeter_id_now.Data_a8+i)) != Temp_MeterID[i])
				{
					sDCU.Status_Meter_u8 = 1;
					break;
				}
			}	
			if (sDCU.Status_Meter_u8 == 1)
			{
				sDCU.sMeter_id_now.Length_u16 = 8;
				for (i = 0; i < METER_LENGTH; i++)
					aMeter_ID[i] =  0;
				for (i=0;i<8;i++)
					aMeter_ID[i] =  Temp_MeterID[i];

//				Save_Meter_ID();
			}			
			break;
		case 0x02:
			if (UART1_Receive_Buff[17] == 0x0F)
			{
                Row = GELEX_Check_Row_Obis103(&Obis_Gelex_Ins[0], GELEX_GetScale_Table[j][0]);
                if(Row >= 0)
                    Obis_Gelex_Ins[Row].Scale_ReadMeter = UART1_Receive_Buff[18];	
			}
			break;
		default:
			break;
	}
	return 1;
}
void GELEX_MS_SendData(void)
{
	
}


void GELEX_Fill_Scale_Mess (void)
{
	uint8_t 	i = 0,j=0;
	uint16_t	TempCheckSum = 0;
	
	//Insert ID Frame
	GELEX_MInfo_MTemp[5] = Get_Meter_Scale.ID_Frame_ui8;
	//Calculate Header checksum
	TempCheckSum = CountFCS16(&GELEX_MInfo_MTemp[0],1,5);
	//Insert Header checksum
	GELEX_MInfo_MTemp[6] = (uint8_t)((TempCheckSum>>8)&0x00FF);
	GELEX_MInfo_MTemp[7] = (uint8_t)(TempCheckSum&0x00FF);
	//Insert Class ID + OBIS + Data Type
	j = GELEX_GetScale_Code_Table[Get_Meter_Scale.Mess_Step_ui8-2];
	for (i=0;i<10;i++)
		GELEX_MInfo_MTemp[i+14] = GELEX_GetScale_Table[j][i+1];
	//Calculate Total checksum
	TempCheckSum = CountFCS16(&GELEX_MInfo_MTemp[0],1,23);
	//Insert Total checksum
	GELEX_MInfo_MTemp[24] = (uint8_t)((TempCheckSum>>8)&0x00FF);
	GELEX_MInfo_MTemp[25] = (uint8_t)(TempCheckSum&0x00FF);
	
	//Calculate next ID Frame
	Get_Meter_Scale.ID_Frame_ui8 += 0x22;
	if (Get_Meter_Scale.ID_Frame_ui8 == 0x20)
		Get_Meter_Scale.ID_Frame_ui8 = 0x10;	
}
//                                                                                                                
                                                                                                       
//
uint8_t GELEX_READ_1REG (uint8_t *Mess_Addr,void (*FuncExtractRawData)(void))
{
    uint8_t mReVal = 0;
    uint8_t Flag_Reading = 0;
    uint8_t StepRead = 0;
    uint8_t Mess_Step = 0;
    
    while(Flag_Reading == 0)
    {
        switch(StepRead) 
        {
            case 0:
                Init_UART2();
                switch(Mess_Step)
                {
                    case 0:
                        RS485_SEND;
                        HAL_UART_Transmit(&UART_METER,&GELEX_SNRM[0],9,1000);
                        RS485_RECIEVE;
                        break;
                    case 1:
                        RS485_SEND;
                        osDelay(3);
                        HAL_UART_Transmit(&UART_METER,&GELEX_AARQ[0],74,1000);
                        RS485_RECIEVE;
                        break;
					case 2:
                        RS485_SEND;
                        osDelay(3);
                        HAL_UART_Transmit(&UART_METER,Mess_Addr,27,1000);
                        RS485_RECIEVE;
                        break;
                    default: 
                        break;
                }
                StepRead = 1;
                break;
            case 1:
                //Wait for respond message from Meter in 2000ms: checksum - extract raw data - save to buffer - goto next message
                if (osSemaphoreWait(bsUART2PendingMessHandle,5000) == osOK) 
                {
                    GELEX_Mess_Checksum();
                    
                    if(Mess_Step == 0)
						if(UART1_Receive_Buff[5] != 0x73) // SNRM
							UART1_Control.Mess_Status_ui8 = 4;
					if(Mess_Step == 1)
						if(UART1_Receive_Buff[5] != 0x30) // AARQ
							UART1_Control.Mess_Status_ui8 = 4;
                    
                    if (UART1_Control.Mess_Status_ui8 == 2) 
                    {
                        Mess_Step++;
                        StepRead = 0;
                        if(Mess_Step > 2)
                        {
                            RS485_SEND;
                            osDelay(3);
                            HAL_UART_Transmit(&UART_METER,&GELEX_Read_END[0],9,1000);
                            RS485_RECIEVE;
                            FuncExtractRawData();
                            StepRead = 3;
                        }
                    } else {
                        Init_UART2();
                        StepRead = 2;
                    }
                } else 
                {
                    Init_UART2();
                    StepRead = 2;
                }
              break;
            case 2:
				Init_UART2();
                Flag_Reading = 1;
                Get_Meter_LProf.Data_Buff_Pointer_ui16 = 10;
              break;
            case 3:  
                if (osSemaphoreWait(bsUART2PendingMessHandle,10000) == osOK) 
					GELEX_Mess_Checksum();
                StepRead = 2;
                break;
            default:
              break;
        } 
    }
    return mReVal;
}

uint8_t GELEX_Read(Meter_Comm_Struct *Meter_Comm, uint8_t *First_Mess_Addr, uint8_t Mess_Type,void (*FuncExtractRawData)(uint8_t MType), uint8_t (*DataHandle)(void))
{
	uint8_t 	mReVal = 0;
	uint16_t	mTemp_CheckSum=0;
	
	//Start read
	Meter_Comm->Reading_ui8 = 1;
	Meter_Comm->ID_Frame_ui8 = 0x51;
   
	while(Meter_Comm->Reading_ui8 == 1) 
    {
		switch(Meter_Comm->Step_ui8) 
        {
			case 0:
				Meter_Comm->Total_Mess_Sent_ui32++;
				//Wait for respond message from Meter in 2000ms: checksum - extract raw data - save to buffer - goto next message
				if (osSemaphoreWait(bsUART2PendingMessHandle,5000) == osOK) 
                {
					//Checksum
					GELEX_Mess_Checksum();
					
					if(Meter_Comm->Mess_Step_ui8 == 0)
						if(UART1_Receive_Buff[5] != 0x73) // SNRM
							UART1_Control.Mess_Status_ui8 = 4;
					if(Meter_Comm->Mess_Step_ui8 == 1)
						if(UART1_Receive_Buff[5] != 0x30) // AARQ
							UART1_Control.Mess_Status_ui8 = 4;
						
					if (UART1_Control.Mess_Status_ui8 == 2) 
                    {
						Meter_Comm->Success_Read_Mess_ui32++;
						//Extract raw data
						if (Meter_Comm->Mess_Step_ui8 > 1)
							FuncExtractRawData(Mess_Type);
						//Goto next message
						Meter_Comm->Mess_Step_ui8++;
						Meter_Comm->Error_ui8 = 0x00;
						Meter_Comm->Error_Wrong_Mess_Format_ui32 = 0x00;
						Meter_Comm->Error_Meter_Norespond_ui32 = 0x00;
						Meter_Comm->Step_ui8 = 1;
						//If all mess sent -> finish
						if ((UART1_Receive_Buff[1] == 0xA0)&&(Meter_Comm->Mess_Step_ui8>2)) {
                            Init_UART2();
							//Send END message
							RS485_SEND;
							osDelay(3);
							HAL_UART_Transmit(&UART_METER,&GELEX_Read_END[0],9,1000);
							RS485_RECIEVE;
							//Extract OBIS list
							mReVal = DataHandle();
							Meter_Comm->Step_ui8 = 3;
						}
					} else {
						Meter_Comm->Error_Wrong_Mess_Format_ui32++;
						//Reset UART
						Init_UART2();
						Meter_Comm->Step_ui8 = 2;
					}
				} else {//If Meter no respond after 2000ms: wait 2000ms - resend the message (3 times) 
					Meter_Comm->Error_Meter_Norespond_ui32++;
					//Reset UART
                    RS485_SEND;
                    osDelay(3);
					HAL_UART_Transmit(&UART_METER,&GELEX_Read_END[0],9,1000);
					RS485_RECIEVE;
                    
					Init_UART2();
					Meter_Comm->Step_ui8 = 2;
				}
				break;
			case 1:
				Init_UART2();
				if (Meter_Comm->Mess_Step_ui8 == 0) 
				{
					RS485_SEND;
                    osDelay(3);
					HAL_UART_Transmit(&UART_METER,&GELEX_SNRM[0],9,1000);
					RS485_RECIEVE;
				}
				else if (Meter_Comm->Mess_Step_ui8 == 1) 
				{
					RS485_SEND;
					osDelay(3);
					HAL_UART_Transmit(&UART_METER,&GELEX_AARQ[0],74,1000);
					RS485_RECIEVE;
				} 
				else if (Meter_Comm->Mess_Step_ui8 == 2) 
				{
					RS485_SEND;
					osDelay(3);
					switch (Mess_Type)
					{
						case 1:
							HAL_UART_Transmit(&UART_METER,First_Mess_Addr,27,1000);
							break;
						case 2:
						case 3:
							HAL_UART_Transmit(&UART_METER,First_Mess_Addr,21,1000);
							break;
						case 4:
							HAL_UART_Transmit(&UART_METER,First_Mess_Addr,78,1000);
						default:
							break;
					}
					
					RS485_RECIEVE;
				}
				else 
				{
					RS485_SEND;
					GELEX_NextFrame[5] = Meter_Comm->ID_Frame_ui8;
					mTemp_CheckSum = CountFCS16(GELEX_NextFrame,1,5);
					GELEX_NextFrame[6] = (uint8_t)((mTemp_CheckSum>>8)&0x00FF);
					GELEX_NextFrame[7] = (uint8_t)(mTemp_CheckSum&0x00FF);
					Meter_Comm->ID_Frame_ui8 += 0x20;
					osDelay(3);
					HAL_UART_Transmit(&UART_METER,&GELEX_NextFrame[0],9,1000);
					RS485_RECIEVE;
					
				}
				Meter_Comm->Step_ui8 = 0;
				break;
			case 2:
				//Reset UART2 status
				Init_UART2();
				//Jump out of reading cycle, wait for next cycle (after 30mins)
				Meter_Comm->Reading_ui8 = 0;
				Meter_Comm->Step_ui8 = 1;
				Meter_Comm->Mess_Step_ui8 = 0;
				//Reset pointer
				Meter_Comm->Data_Buff_Pointer_ui16 = 10;   //reset leng nhan o day
				break;
			case 3:
				Meter_Comm->Total_Mess_Sent_ui32++;
				if (osSemaphoreWait(bsUART2PendingMessHandle,5000) == osOK) 
				{
					//Checksum
					GELEX_Mess_Checksum();
					
					if (UART1_Control.Mess_Status_ui8 == 2)
						Meter_Comm->Success_Read_Mess_ui32++;
				}
				Meter_Comm->Step_ui8 = 2;
				break;
			default:
				break;				
		}
	}
	return mReVal;	
}

uint8_t CheckOBISInTable(uint16_t buff_start_pos, uint8_t table_end_pos)
{
	uint8_t row=0,column=0,matched_obis=0xFF,matched_count=0;
	
	for (row=0;row<table_end_pos;row++)
	{
		for (column=0;column<6;column++)
		{
			if (Meter_TempBuff[buff_start_pos+column] == GELEX_OBIS_Table[row][column])
				matched_count++;
			else
				break;
		}
		if (matched_count == 6)
		{
			matched_obis = row;
			break;
		}
		else
			matched_count = 0;
	}
	
	if (matched_obis == 0xFF)
		return 0xFF;
	else
		return matched_obis;
}

void GELEX_Prepare_Read_Date(uint8_t *Start, uint8_t *Stop)
{
	uint8_t i=0;
	
	for (i=0;i<8;i++)
		GELEX_Get_DATA_Template[i+47] = *(Start+i);
	
	for (i=0;i<8;i++)
		GELEX_Get_DATA_Template[i+61] = *(Stop+i);
}
//                                                                                                                
uint8_t test1=0;	                                                                                                             
void GELEX_MBillingGetRawData(uint8_t Mess_Type)
{
	uint16_t i=0;
	uint8_t data_pos;
	
	if (Get_Meter_Billing.Mess_Step_ui8 == 2)
	{
		switch (Mess_Type)
		{
			case 1:
			case 2:
			case 4:
				if (UART1_Receive_Buff[12] == 0x01)
				{
					data_pos = getObjectCount(UART1_Receive_Buff,13);
					for (i=0;i<UART1_Control.Mess_Length_ui16-data_pos-2;i++)
						Meter_TempBuff[Get_Meter_Billing.Data_Buff_Pointer_ui16++] = UART1_Receive_Buff[i+data_pos]; 
				}
				else
				{	
					data_pos = getObjectCount(UART1_Receive_Buff,20);
					for (i=0;i<UART1_Control.Mess_Length_ui16-data_pos-2;i++)
						Meter_TempBuff[Get_Meter_Billing.Data_Buff_Pointer_ui16++] = UART1_Receive_Buff[i+data_pos];
				}
				break;
			case 3:
				break;
			default:
				break;
		}
	}
	else
		for (i=0;i<UART1_Control.Mess_Length_ui16-10;i++)
			Meter_TempBuff[Get_Meter_Billing.Data_Buff_Pointer_ui16++] = UART1_Receive_Buff[i+8];
}

uint8_t GELEX_MBillingExtractOBISList(void)
{
	uint16_t 	OBISList_Length = 0,i=10;
	uint8_t		Data_OBIS = 0,OBIS_Row=0;
	
	OBISList_Length = Get_Meter_Billing.Data_Buff_Pointer_ui16 - 2;
	
	while (i<OBISList_Length)
	{
		Data_OBIS = Meter_TempBuff[i];
		
		switch (Data_OBIS)
		{
			case 0x01:
				Get_Meter_Billing.Total_OBIS_ui8 = Meter_TempBuff[i+1];
				i += 2;
				break;
			case 0x02:
				i += 2;
				break;
			case 0x09:
				if (Meter_TempBuff[i+1] == 0x06)  
				{
					OBIS_Row = CheckOBISInTable(i+2,71);
					if ((OBIS_Row != 0xFF) && (OBIS_Row < 37)) //tu Import Wh total den time begin
                        if((GELEX_OBIS_Table[OBIS_Row][6] <= 0x45) || (GELEX_OBIS_Table[OBIS_Row][6] == 0x64) || (GELEX_OBIS_Table[OBIS_Row][6] == 0x65))  
                            GELEX_MBilling_OBIS_List[Get_Meter_Billing.OBIS_Pointer_ui8++] = GELEX_OBIS_Table[OBIS_Row][6];
                        else GELEX_MBilling_OBIS_List[Get_Meter_Billing.OBIS_Pointer_ui8++] = 0xFF;
					else
						GELEX_MBilling_OBIS_List[Get_Meter_Billing.OBIS_Pointer_ui8++] = 0xFF;
					i += 8;
					Get_Meter_Billing.Num_OBIS_ui8++;
				}
				else
					Get_Meter_Billing.Error_ui8 = 0x10;
				break;
			case 0x0F:
				i += 2;
				break;
			case 0x12:
				i += 3;
				break;
			default:
				Get_Meter_Billing.Error_ui8 = 0x10;
				break;
		}
		if (Get_Meter_Billing.Error_ui8 == 0x10)
			return 0;
	};

	return 1;
}


uint8_t GELEX_MBillingExtractData(void)
{
	uint16_t 	Data_Length = 0,i=10,j=0;
	uint8_t		Data_OBIS = 0,Temp_length=0;

    uint8_t         BuffNum[20];
    truct_String    Str_Data_Write={&BuffNum[0], 0};
    uint8_t         ScaleRead = 0;
    ST_TIME_FORMAT  sTime_temp;
    uint16_t        Tempyear = 0;
    uint16_t        Temp_data_16 = 0;
    uint32_t        Temp_data_32 = 0;
    uint8_t         TempObis = 0;
    uint8_t         Buf_TempStime[8];
    uint8_t         Temp_BBC = 0;
    uint8_t         Div = 0;
    int8_t          RowObis103 = 0;
    uint32_t        diffTimeStampStart;
    uint64_t        Mul = 1;
    
	//Extract data - Insert data to buffer
	Get_Meter_Billing.OBIS_Pointer_ui8 = 0;
	Data_Length = Get_Meter_Billing.Data_Buff_Pointer_ui16;
	
    
	while (i<Data_Length)
	{
        Temp_data_16 = 0;
        Temp_data_32 = 0;
        Div = 0;
        Mul = 1;
        ScaleRead = 0;
        
        Reset_Buff(&Str_Data_Write);
        
        if(Get_Meter_Billing.Flag_Start_Pack == 0)
        {
            Reset_Buff(&Get_Meter_Billing.Str_Payload);
            Write_Header_His_Push103();
            Get_Meter_Billing.Flag_Start_Pack = 1;
//            Reset_Buff(&Get_Meter_TuTi.Str_Payload);
        }
        
		Data_OBIS = Meter_TempBuff[i];
		
		switch (Data_OBIS)
		{
			case 0x01:
				Get_Meter_Billing.Num_Block_ui8 = Meter_TempBuff[i+1];  //Meter_TempBuff[i+1]-1;
				i += 2;
				break;
			case 0x02:
				Get_Meter_Billing.Total_OBIS_ui8 = Meter_TempBuff[i+1];
				i += 2;
				break;
			case 0x06:   //maxdemand value
				TempObis = GELEX_MBilling_OBIS_List[Get_Meter_Billing.OBIS_Pointer_ui8++];
				i++;
				for (j=0;j<4;j++)
					Temp_data_32 = (Temp_data_32 << 8) | Meter_TempBuff[i++];
                
                if(TempObis != 0xFF)
                {
                    RowObis103 = GELEX_Check_Row_Obis103(&Obis_Gelex_Ins[0], TempObis);
                    if(RowObis103 >=0)
                    {
                        if(Obis_Gelex_Ins[RowObis103].StrObis_Bill != NULL)
                        {
                            if((TempObis == 0x31) || (TempObis == 0x35) || (TempObis == 0x39))   //value maxdemand
                            {
                                MD_Plus_Bill.Value_u32[MD_Plus_Bill.NumValue_MD++] = Temp_data_32;
                                //lay MAXDEMAND SUB
                            }else if((TempObis == 0x33) || (TempObis == 0x37) || (TempObis == 0x3B))
                            {
                                MD_Sub_Bill.Value_u32[MD_Sub_Bill.NumValue_MD++] = Temp_data_32;
                            }else
                            {
                                if((TempObis == 0x41) || (TempObis == 0x42))  //bo qua. lay phia ngoai
                                {
//                                    ScaleRead = GELEX_ConvertScaleMeter_toDec (Obis_Gelex_Ins[RowObis103].Scale_ReadMeter, &Div);
//                                    for(j = 0; j < ScaleRead; j++)
//                                        Mul = Mul *10;
//                                    //o d�y l� so khong dau
//                                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
//                                    
//                                    Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) (Temp_data_32 * Mul), Div + Obis_Gelex_Ins[RowObis103].scale); 
//                                    
//                                    if(Obis_Gelex_Ins[RowObis103].Str_Unit!= NULL)
//                                    {
//                                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
//                                        Copy_String_2(&Str_Data_Write, Obis_Gelex_Ins[RowObis103].Str_Unit);  //don vi
//                                    }
//                                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
//                                    
//                                    Copy_String_2(&Get_Meter_TuTi.Str_Payload , &Str_Data_Write);
                                }else
                                {
                                    //ghi obis vao
                                    Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Obis_Inbuff, Obis_Gelex_Ins[RowObis103].StrObis_Bill);
                                    Get_Meter_Billing.Pos_Obis_Inbuff += Obis_Gelex_Ins[RowObis103].StrObis_Bill->Length_u16;
                                    Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
                                    Get_Meter_Billing.Numqty++;
                                    //ghi data

                                    ScaleRead = GELEX_ConvertScaleMeter_toDec (Obis_Gelex_Ins[RowObis103].Scale_ReadMeter, &Div);
                                    for(j = 0; j < ScaleRead; j++)
                                        Mul = Mul *10;
                                    //o d�y l� so khong dau
                                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                                    Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) (Temp_data_32 * Mul), Div+ Obis_Gelex_Ins[RowObis103].scale); 
                                    if(Obis_Gelex_Ins[RowObis103].Str_Unit != NULL)
                                    {
                                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                                        Copy_String_2(&Str_Data_Write, Obis_Gelex_Ins[RowObis103].Str_Unit);  //don vi bieu
                                    }
                                    
                                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                                    
                                    Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Data_Inbuff, &Str_Data_Write);
                                    Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
                                }
                            }
                        }
                    }
                }
                
				break;
			case 0x09:   //time begin bill v� stime cua MaxDemand
				TempObis = GELEX_MBilling_OBIS_List[Get_Meter_Billing.OBIS_Pointer_ui8++];
				i++;
				Temp_length= Meter_TempBuff[i++];
				for (j=0;j<Temp_length;j++)
                {
                    if(j < 8)   //chi lay 8 byte dau
                        Buf_TempStime[j] = Meter_TempBuff[i++];
                    else i++;
                }
                
                Tempyear         = (Buf_TempStime[0] << 8) | (Buf_TempStime[1]);
                sTime_temp.year  = Tempyear%100;
                sTime_temp.month = Buf_TempStime[2];
                sTime_temp.date  = Buf_TempStime[3];
                sTime_temp.day   = Buf_TempStime[4];
                sTime_temp.hour  = Buf_TempStime[5];
                sTime_temp.min   = Buf_TempStime[6];
                sTime_temp.sec   = Buf_TempStime[7];
                        
                if(TempObis != 0xFF)
                {
                    if(TempObis == 0x01)  //stime begin
                    {   
                        Reset_Buff(&StrSTime_Bill);
                        //luu vao 1 buff rieng de sau do moi dong goi sau cung
                        *(StrSTime_Bill.Data_a8 + StrSTime_Bill.Length_u16++) = '('; 
                        Copy_String_STime(&StrSTime_Bill,sTime_temp);
                        *(StrSTime_Bill.Data_a8 + StrSTime_Bill.Length_u16++) = ')';
                    }else if((TempObis == 0x32) || (TempObis == 0x36) || (TempObis == 0x3A))  //stime maxdemand
                    {   
                        //luu lai gia tri stime bieu cua maxdemand
                        MD_Plus_Bill.sTime[MD_Plus_Bill.NumTariff].year   = sTime_temp.year;
                        MD_Plus_Bill.sTime[MD_Plus_Bill.NumTariff].month  = sTime_temp.month;
                        MD_Plus_Bill.sTime[MD_Plus_Bill.NumTariff].date   = sTime_temp.date;
                        MD_Plus_Bill.sTime[MD_Plus_Bill.NumTariff].day    = sTime_temp.day;
                        MD_Plus_Bill.sTime[MD_Plus_Bill.NumTariff].hour   = sTime_temp.hour;
                        MD_Plus_Bill.sTime[MD_Plus_Bill.NumTariff].min    = sTime_temp.min;
                        MD_Plus_Bill.sTime[MD_Plus_Bill.NumTariff].sec    = sTime_temp.sec;
                        MD_Plus_Bill.NumTariff++;
                    }else if((TempObis == 0x34) || (TempObis == 0x38) || (TempObis == 0x3C))
                    {
                        //luu lai gia tri stime bieu cua maxdemand
                        MD_Sub_Bill.sTime[MD_Sub_Bill.NumTariff].year   = sTime_temp.year;
                        MD_Sub_Bill.sTime[MD_Sub_Bill.NumTariff].month  = sTime_temp.month;
                        MD_Sub_Bill.sTime[MD_Sub_Bill.NumTariff].date   = sTime_temp.date;
                        MD_Sub_Bill.sTime[MD_Sub_Bill.NumTariff].day    = sTime_temp.day;
                        MD_Sub_Bill.sTime[MD_Sub_Bill.NumTariff].hour   = sTime_temp.hour;
                        MD_Sub_Bill.sTime[MD_Sub_Bill.NumTariff].min    = sTime_temp.min;
                        MD_Sub_Bill.sTime[MD_Sub_Bill.NumTariff].sec    = sTime_temp.sec;
                        MD_Sub_Bill.NumTariff++;
                    }
                }
				break;
			case 0x0F:
                Get_Meter_Billing.OBIS_Pointer_ui8++;
				i++;
                i++;
				break;
			case 0x12:
                TempObis = GELEX_MBilling_OBIS_List[Get_Meter_Billing.OBIS_Pointer_ui8++];
                i++;
				for (j=0;j<2;j++)
					Temp_data_16 = (Temp_data_16 << 8) | Meter_TempBuff[i++];	
                
                if(TempObis != 0xFF)
                {
                    RowObis103 = GELEX_Check_Row_Obis103(&Obis_Gelex_Ins[0], TempObis);
                    if(RowObis103 >=0)
                    {
                        if((Obis_Gelex_Ins[RowObis103].StrObis_Bill != NULL)&& (TempObis <= 0x45))
                        {
                            Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Obis_Inbuff, Obis_Gelex_Ins[RowObis103].StrObis_Bill);
                            Get_Meter_Billing.Pos_Obis_Inbuff += Obis_Gelex_Ins[RowObis103].StrObis_Bill->Length_u16;
                            Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
                            Get_Meter_Billing.Numqty++;
                            
                            ScaleRead = GELEX_ConvertScaleMeter_toDec (Obis_Gelex_Ins[RowObis103].Scale_ReadMeter, &Div);
                            for(j = 0; j < ScaleRead; j++)
                                Mul = Mul *10;
                            //data
                            if((TempObis >= 0x0F) && (TempObis <= 0x19))   //co dau
                            {
                                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                                Pack_HEXData_Frame(&Str_Data_Write, (int64_t) (Convert_uint16_2int16 (Temp_data_16) * Mul) ,Div + Obis_Gelex_Ins[TempObis].scale);
                                if(Obis_Gelex_Ins[RowObis103].Str_Unit != NULL)
                                {
                                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                                    Copy_String_2(&Str_Data_Write, Obis_Gelex_Ins[RowObis103].Str_Unit);  //don vi
                                }
                                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                                    
                                Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Data_Inbuff, &Str_Data_Write);
                                Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
                            }else //khong dau
                            {
                                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                                Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t)(Temp_data_16 * Mul), Div + Obis_Gelex_Ins[RowObis103].scale);
                                if(Obis_Gelex_Ins[RowObis103].Str_Unit!= NULL)
                                {
                                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                                    Copy_String_2(&Str_Data_Write, Obis_Gelex_Ins[RowObis103].Str_Unit);  //don vi
                                }
                                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                                
                                Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Data_Inbuff, &Str_Data_Write);
                                Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
                            }
                        }
                    }
                }
            
				break;
			case 0x17:
                TempObis = GELEX_MBilling_OBIS_List[Get_Meter_Billing.OBIS_Pointer_ui8++];
                i++;
                for (j=0;j<4;j++)
                        Temp_data_32 = (Temp_data_32 << 8) | Meter_TempBuff[i+3-j];
                
                if(TempObis != 0xFF)
                {
                    RowObis103 = GELEX_Check_Row_Obis103(&Obis_Gelex_Ins[0], TempObis);
                    if(RowObis103 >=0)
                    {
                        if(Obis_Gelex_Ins[RowObis103].StrObis_Bill != NULL)
                        {
                            //ghi obis vao
                            Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Obis_Inbuff, Obis_Gelex_Ins[RowObis103].StrObis_Bill);
                            Get_Meter_Billing.Pos_Obis_Inbuff += Obis_Gelex_Ins[RowObis103].StrObis_Bill->Length_u16;
                            Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
                            Get_Meter_Billing.Numqty++;
                            //ghi data
                            ScaleRead = GELEX_ConvertScaleMeter_toDec (Obis_Gelex_Ins[RowObis103].Scale_ReadMeter, &Div);
                            for(j = 0; j < ScaleRead; j++)
                                Mul = Mul *10;
                            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                            Pack_HEXData_Frame(&Str_Data_Write, (int64_t)(Convert_float_2int(Temp_data_32, 0) * Mul), Div + Obis_Gelex_Ins[RowObis103].scale);
                            if(Obis_Gelex_Ins[RowObis103].Str_Unit!= NULL)
                            {
                                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                                Copy_String_2(&Str_Data_Write, Obis_Gelex_Ins[RowObis103].Str_Unit);  //don vi cua bieu gi�
                            }
                            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                                
                            Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Data_Inbuff, &Str_Data_Write);
                            Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
                        }
                    }
                }
                i += 4;
				break;
			default:
				Get_Meter_Billing.Error_ui8 = 0x10;
				break;
		}
		if (Get_Meter_Billing.Error_ui8 == 0x10)
			return 0;
	};
	
	if (Get_Meter_Billing.Last_Data_Buff_Pointer_ui16 != 10)
	{
        Pack_MD_Gelex(&MD_Plus_Bill, 0x31);   
        Pack_MD_Gelex(&MD_Sub_Bill, 0x33);
        
        Add_TuTI_toPayload(&Get_Meter_Billing);
        //lay het data moi ghep stime Bill vao
        Reset_Buff(&Str_Data_Write);
        
        if(Get_Meter_Billing.Numqty > 3)  //co data moi can check time
        {
            Copy_String_2(&Str_Data_Write, &StrSTime_Bill);  //don vi cua bieu gi�
            ConvertStringsTime_tosTime(StrSTime_Bill.Data_a8 + 1,&sTime_temp);
            if(sInformation.Flag_Request_Bill == 1)
            {
                //so sanh xem nam trong khoang start time va stop time nua k. de dung lai hoac bo qua
                if(Check_DiffTime(sTime_temp, sInformation.StartTime_GetBill, &diffTimeStampStart) == 0)
                {
                    Get_Meter_Billing.Last_Data_Buff_Pointer_ui16 = Get_Meter_Billing.Str_Payload.Length_u16 + 10;  //cho giong code cu
                    Get_Meter_Billing.Data_Buff_Pointer_ui16 = 10;
                    return 1; //return 1 de chay doc tiep cac record khac 
                }
                if(Check_DiffTime(sInformation.EndTime_GetBill, sTime_temp, &diffTimeStampStart) == 0)
                {
                    Get_Meter_Billing.Last_Data_Buff_Pointer_ui16 = Get_Meter_Billing.Str_Payload.Length_u16 + 10;  //cho giong code cu
                    Get_Meter_Billing.Data_Buff_Pointer_ui16 = 10;
                    sInformation.Flag_Stop_ReadBill = 1;  //dung flag nay de dung vi�c doc. vi qua thoi gian stop time. kiem tra lai 
                    return 1;
                }
            }
        }
        else 
        {
            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
//            Copy_String_STime(&Str_Data_Write, sInformation.StartTime_GetBill);   //khong co data bo trong stime start
            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')'; 
        }
    
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
        Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) Get_Meter_Billing.Numqty, 0);
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
	
	//Reset Billing struct for next reading cycle
	Get_Meter_Billing.Last_Data_Buff_Pointer_ui16 = Get_Meter_Billing.Str_Payload.Length_u16 + 10;  //cho giong code cu
	Get_Meter_Billing.Data_Buff_Pointer_ui16 = 10;
	
	return 1;
}

void Pack_MD_Gelex (Struct_Maxdemand_Value* Struct_MD, uint8_t FirstRate)
{
    uint16_t        i = 0, j = 0;
    uint8_t         Num_Bieu = 0;
    uint32_t        Temp_data_32 = 0;
    uint8_t         TempObis = 0;
    uint8_t         BuffNum[30];
    truct_String    Str_Data_Write={&BuffNum[0], 0};
    uint8_t         ScaleRead = 0;
    uint8_t         Div = 0;
    int8_t          Row = 0;
    uint64_t        Mul = 1;
    
    if(Struct_MD->NumTariff > Struct_MD->NumValue_MD) 
        Num_Bieu = Struct_MD->NumValue_MD;
    else Num_Bieu = Struct_MD->NumTariff;
    
    if(Num_Bieu > 3) Num_Bieu = 3;
    Row = GELEX_Check_Row_Obis103(&Obis_Gelex_Ins[0],FirstRate);
    
    for(i = 0; i < Num_Bieu; i++)
    {
        Reset_Buff(&Str_Data_Write);
        TempObis = Row + i*4;
        Temp_data_32 = Struct_MD->Value_u32[i];
        
        Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Obis_Inbuff, Obis_Gelex_Ins[TempObis].StrObis_Bill);  
        Get_Meter_Billing.Pos_Obis_Inbuff += Obis_Gelex_Ins[TempObis].StrObis_Bill->Length_u16;
        Get_Meter_Billing.Numqty++;
        Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
        //don vi
        ScaleRead = GELEX_ConvertScaleMeter_toDec (Obis_Gelex_Ins[TempObis].Scale_ReadMeter, &Div);
        for(j = 0; j < ScaleRead; j++)
            Mul = Mul *10;
        //o d�y l� so khong dau
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';

        Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) (Temp_data_32 * Mul), Div + Obis_Gelex_Ins[TempObis].scale); 
        if(Obis_Gelex_Ins[TempObis].Str_Unit != NULL)
        {
            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
            Copy_String_2(&Str_Data_Write, Obis_Gelex_Ins[TempObis].Str_Unit);  //don vi bieu
        }

        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';

        Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Data_Inbuff, &Str_Data_Write);
        Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
         //ghi stime vao
        *(Get_Meter_Billing.Str_Payload.Data_a8 + Get_Meter_Billing.Str_Payload.Length_u16++) = '(';
        Copy_String_STime(&Get_Meter_Billing.Str_Payload, Struct_MD->sTime[i]);
        *(Get_Meter_Billing.Str_Payload.Data_a8 + Get_Meter_Billing.Str_Payload.Length_u16++) = ')';
        Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
    }
}


void GELEX_MBillingInsertReadTime (void)
{
	uint16_t TempYear=0;
	
    if(sInformation.Flag_Request_Bill == 1)
    {
        TempYear = 2000 + sInformation.StartTime_GetBill.year;
          
        GELEX_MBilling_StartTime[0] = (uint8_t)((TempYear>>8)&0x00FF);
        GELEX_MBilling_StartTime[1] = (uint8_t)(TempYear&0x00FF);
        GELEX_MBilling_StartTime[2] = sInformation.StartTime_GetBill.month;
        GELEX_MBilling_StartTime[3] = sInformation.StartTime_GetBill.date;
        
        TempYear = 2000 + sInformation.EndTime_GetBill.year;
        GELEX_MBilling_StopTime[0]  = (uint8_t)((TempYear>>8)&0x00FF);
        GELEX_MBilling_StopTime[1]  = (uint8_t)(TempYear&0x00FF);
        GELEX_MBilling_StopTime[2]  = sInformation.EndTime_GetBill.month;
        GELEX_MBilling_StopTime[3] = sInformation.EndTime_GetBill.date;
    }else
    {
        if (sRTC.month == 1)
        {
            TempYear = 1999 + sRTC.year;
            GELEX_MBilling_StartTime[0] = (uint8_t)((TempYear>>8)&0x00FF);
            GELEX_MBilling_StartTime[1] = (uint8_t)(TempYear&0x00FF);
            GELEX_MBilling_StartTime[2] = 12;
            GELEX_MBilling_StartTime[3] = sRTC.date;
            TempYear += 1;
            GELEX_MBilling_StopTime[0]  = (uint8_t)((TempYear>>8)&0x00FF);
            GELEX_MBilling_StopTime[1]  = (uint8_t)(TempYear&0x00FF);
            GELEX_MBilling_StopTime[2]  = sRTC.month;
            GELEX_MBilling_StopTime[3] = sRTC.date;
            Copy_STime_fromsTime(&sInformation.StartTime_GetBill,(uint8_t) (sRTC.year + 99) %100, 12, sRTC.date, sRTC. hour, sRTC.min, sRTC.sec);
            Copy_STime_fromsTime(&sInformation.EndTime_GetBill,sRTC.year, sRTC.month, sRTC.date, sRTC. hour, sRTC.min, sRTC.sec);
        }
        else
        {
            TempYear = 2000 + sRTC.year;
            GELEX_MBilling_StartTime[0] = (uint8_t)((TempYear>>8)&0x00FF);
            GELEX_MBilling_StartTime[1] = (uint8_t)(TempYear&0x00FF);
            GELEX_MBilling_StartTime[2] = sRTC.month-1;
            GELEX_MBilling_StartTime[3] = sRTC.date;
            GELEX_MBilling_StopTime[0]  = (uint8_t)((TempYear>>8)&0x00FF);
            GELEX_MBilling_StopTime[1]  = (uint8_t)(TempYear&0x00FF);
            GELEX_MBilling_StopTime[2]  = sRTC.month;
            GELEX_MBilling_StopTime[3]  = sRTC.date;
            Copy_STime_fromsTime(&sInformation.StartTime_GetBill, sRTC.year,  sRTC.month -1, sRTC.date, sRTC. hour, sRTC.min, sRTC.sec);
            Copy_STime_fromsTime(&sInformation.EndTime_GetBill,sRTC.year, sRTC.month, sRTC.date, sRTC. hour, sRTC.min, sRTC.sec);
        }
    }
}

uint8_t GELEX_MBillingExtractData_Dummy(void)
{
	return 1;
}
void GELEX_MBillingGetRawData_Dummy(uint8_t Mess_Type)
{
	
}
//                                                                                                                
void GELEX_MLProfileGetPeriod(void)
{
    uint8_t  i = 0;
    
    if ((UART1_Receive_Buff[12] == 0x01) && (UART1_Receive_Buff[15] == 0x06))
    {   
        PeriodLpf_Min = 0;
        for(i = 0; i<4; i++)
            PeriodLpf_Min = (PeriodLpf_Min << 8) | UART1_Receive_Buff[16+i]; 
    }
}

void GELEX_MLProfileGetRawData(uint8_t Mess_Type)
{
	uint16_t i=0;
	uint8_t data_pos;
	
	if (Get_Meter_LProf.Mess_Step_ui8 == 2)
	{
		switch (Mess_Type)
		{
			case 1:
			case 4:
			case 2:
				if (UART1_Receive_Buff[12] == 0x01)
				{
					data_pos = getObjectCount(UART1_Receive_Buff,13);
					for (i=0;i<UART1_Control.Mess_Length_ui16-data_pos-2;i++)
						Meter_TempBuff[Get_Meter_LProf.Data_Buff_Pointer_ui16++] = UART1_Receive_Buff[i+data_pos];
				}
				else
				{	
					data_pos = getObjectCount(UART1_Receive_Buff,20);
					for (i=0;i<UART1_Control.Mess_Length_ui16-data_pos-2;i++)
						Meter_TempBuff[Get_Meter_LProf.Data_Buff_Pointer_ui16++] = UART1_Receive_Buff[i+data_pos];
				}
				break;
			case 3:
				break;
			default:
				break;
		}
	}
	else
		for (i=0;i<UART1_Control.Mess_Length_ui16-10;i++)
			Meter_TempBuff[Get_Meter_LProf.Data_Buff_Pointer_ui16++] = UART1_Receive_Buff[i+8];	
}

uint8_t GELEX_MLProfileExtractOBISList(void)
{
	uint16_t 	OBISList_Length = 0,i=10;
	uint8_t		Data_OBIS = 0,OBIS_Row=0;
	
	OBISList_Length = Get_Meter_LProf.Data_Buff_Pointer_ui16 - 2;
	
	while (i<OBISList_Length)
	{
		Data_OBIS = Meter_TempBuff[i];
		
		switch (Data_OBIS)
		{
			case 0x01:
				Get_Meter_LProf.Total_OBIS_ui8 = Meter_TempBuff[i+1];
				i += 2;
				break;
			case 0x02:
				i += 2;
				break;
			case 0x09:
				if (Meter_TempBuff[i+1] == 0x06)
				{
					OBIS_Row = CheckOBISInTable(i+2,111);
					if ((OBIS_Row != 0xFF) && ((OBIS_Row >= 5) || (OBIS_Row == 0)))
//                        if(GELEX_OBIS_Table[OBIS_Row][6] <= 0x45)
                            GELEX_MLProfile_OBIS_List[Get_Meter_LProf.OBIS_Pointer_ui8++] = GELEX_OBIS_Table[OBIS_Row][6];
//                        else GELEX_MLProfile_OBIS_List[Get_Meter_LProf.OBIS_Pointer_ui8++] = 0xFF;
					else
						GELEX_MLProfile_OBIS_List[Get_Meter_LProf.OBIS_Pointer_ui8++] = 0xFF;
					i += 8;
					Get_Meter_LProf.Num_OBIS_ui8++;
				}
				else
					Get_Meter_LProf.Error_ui8 = 0x10;
				break;
			case 0x0F:
				i += 2;
				break;
			case 0x12:
				i += 3;
				break;
			default:
				Get_Meter_LProf.Error_ui8 = 0x10;
				break;
		}
		if (Get_Meter_LProf.Error_ui8 == 0x10)
			return 0;
	};
//	if (Get_Meter_LProf.Num_OBIS_ui8 == Get_Meter_LProf.Total_OBIS_ui8)
//		return 1;
//	else
//		return 0;
	return 1;
}

uint8_t GELEX_MLProfileExtractData(void)   
{
	uint16_t 	    Data_Length = 0,i=10,j=0;
	uint8_t		    Data_OBIS = 0,Temp_length=0;
    uint8_t         BuffNum[30];
    ST_TIME_FORMAT  sTime_temp;
    uint16_t        Tempyear = 0;
    uint16_t        Temp_data_16 = 0;
    uint32_t        Temp_data_32 = 0;
    uint8_t         TempObis = 0;
    uint8_t         Buf_TempStime[8];
    uint8_t         Div = 0;
    int8_t          RowObis103 = 0;
    uint8_t         ScaleRead = 0;
    uint64_t        Mul = 1;
    truct_String    Str_Data_Write={&BuffNum[0], 0};
    
    if(Get_Meter_LProf.Flag_Start_Pack == 0)
    {
        Reset_Buff(&Get_Meter_LProf.Str_Payload);
        Pack_Header_lpf_Pushdata103();
        Get_Meter_LProf.Flag_Start_Pack = 1;
    }
    
	//Extract data - Insert data to buffer
	Get_Meter_LProf.OBIS_Pointer_ui8 = 0;
	Data_Length = Get_Meter_LProf.Data_Buff_Pointer_ui16;
	Get_Meter_LProf.Data_Buff_Pointer_ui16 = 10;
	
	while (i<Data_Length)
	{
        if(Get_Meter_LProf.Flag_Start_Pack == 0)
        {
            Reset_Buff(&Get_Meter_LProf.Str_Payload);
            Pack_Header_lpf_Pushdata103();
            Get_Meter_LProf.Flag_Start_Pack = 1;
        }
        
        Temp_data_16 = 0;
        Temp_data_32 = 0;
        ScaleRead = 0;
        Div = 0;
        Mul = 1;
        Reset_Buff(&Str_Data_Write);

		Data_OBIS = Meter_TempBuff[i];
		switch (Data_OBIS)
		{
			case 0x01:
                if(Meter_TempBuff[i+1] == 0x81)
                {
                    Get_Meter_LProf.Num_Record_ui16 = Meter_TempBuff[i+2];
                    i++;
                }else Get_Meter_LProf.Num_Record_ui16 = Meter_TempBuff[i+1];
				i += 2;
				break;
			case 0x02:
                if ((Get_Meter_LProf.Flag_ui32 == 1) || (Get_Meter_LProf.Flag_ui32 == 2))
                    Get_Meter_LProf.Data_Buff_Pointer_ui16 += 2;
				i += 2;
				break;
			case 0x06:
                TempObis = GELEX_MLProfile_OBIS_List[Get_Meter_LProf.OBIS_Pointer_ui8++];
				i++;
				for (j=0;j<4;j++)
					Temp_data_32 = (Temp_data_32 << 8) | Meter_TempBuff[i++];
                
                if(TempObis != 0xFF)
                {
                    RowObis103 = GELEX_Check_Row_Obis103(&Obis_Gelex_Ins[0], TempObis);
                    if(RowObis103 >=0)
                    {
                        if(Obis_Gelex_Ins[RowObis103].StrObis_Lpf != NULL) 
                        {
                            //ghi obis vao
                            Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Obis_Inbuff, Obis_Gelex_Ins[RowObis103].StrObis_Lpf);
                            Get_Meter_LProf.Pos_Obis_Inbuff += Obis_Gelex_Ins[RowObis103].StrObis_Lpf->Length_u16;
                            Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
                            Get_Meter_LProf.Numqty++;
                            //ghi data
                            ScaleRead = GELEX_ConvertScaleMeter_toDec (Obis_Gelex_Ins[RowObis103].Scale_ReadMeter, &Div);
                            for(j = 0; j < ScaleRead; j++)
                                Mul = Mul *10;
                            //o d�y l� so khong dau
                            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                            
                            Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) (Temp_data_32 * Mul), Div + Obis_Gelex_Ins[RowObis103].scale); 
                            if(Obis_Gelex_Ins[RowObis103].Str_Unit != NULL)
                            {
                                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                                Copy_String_2(&Str_Data_Write, Obis_Gelex_Ins[RowObis103].Str_Unit);  //don vi bieu
                            }
                            
                            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                            
                            Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Data_Inbuff, &Str_Data_Write);
                            Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
                        }
                    }
                }
				break;
			case 0x09:
                TempObis = GELEX_MLProfile_OBIS_List[Get_Meter_LProf.OBIS_Pointer_ui8++];
				i++;
				Temp_length= Meter_TempBuff[i++];
				for (j=0;j<Temp_length;j++)
                {
                    if(j < 8)   //chi lay 8 byte dau
                        Buf_TempStime[j] = Meter_TempBuff[i++];
                    else i++;
                }
                
                Tempyear         = (Buf_TempStime[0] << 8) | (Buf_TempStime[1]);
                sTime_temp.year  = Tempyear%100;
                sTime_temp.month = Buf_TempStime[2];
                sTime_temp.date  = Buf_TempStime[3];
                sTime_temp.day   = Buf_TempStime[4];
                sTime_temp.hour  = Buf_TempStime[5];
                sTime_temp.min   = Buf_TempStime[6];
                sTime_temp.sec   = Buf_TempStime[7];
                        
                if(TempObis != 0xFF)
                {
                    if(TempObis == 0x01) //&& (GE_Fl_Alow_GetTimeLpf == 0))   //stime begin
                    {   
                        Reset_Buff(&StrSTime_Bill);
                        //luu vao 1 buff rieng de sau do moi dong goi sau cung
                        *(StrSTime_Bill.Data_a8 + StrSTime_Bill.Length_u16++) = '('; 
                        Copy_String_STime(&StrSTime_Bill,sTime_temp);
                        *(StrSTime_Bill.Data_a8 + StrSTime_Bill.Length_u16++) = ')';
                    }
                }
				break;
			case 0x0F:
                Get_Meter_LProf.OBIS_Pointer_ui8++;
				i++;
                i++;
				break;
			case 0x12:
                TempObis = GELEX_MLProfile_OBIS_List[Get_Meter_LProf.OBIS_Pointer_ui8++];
				i++;
 
				for (j=0;j<2;j++)
					Temp_data_16 = (Temp_data_16 << 8) | Meter_TempBuff[i++];	
                
                if(TempObis != 0xFF)
                {
                    RowObis103 = GELEX_Check_Row_Obis103(&Obis_Gelex_Ins[0], TempObis);
                    if(RowObis103 >=0)
                    {
                        if(Obis_Gelex_Ins[RowObis103].StrObis_Lpf != NULL)
                        {
                            Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Obis_Inbuff, Obis_Gelex_Ins[RowObis103].StrObis_Lpf);
                            Get_Meter_LProf.Pos_Obis_Inbuff += Obis_Gelex_Ins[RowObis103].StrObis_Lpf->Length_u16;
                            Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
                            Get_Meter_LProf.Numqty++;
                            
                            ScaleRead = GELEX_ConvertScaleMeter_toDec (Obis_Gelex_Ins[RowObis103].Scale_ReadMeter, &Div);
                            for(j = 0; j < ScaleRead; j++)
                                Mul = Mul *10;
                            //data
                            if((TempObis >= 0x0F) && (TempObis <= 0x19))   //co dau
                            { 
                                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                                Pack_HEXData_Frame(&Str_Data_Write, (int64_t) (Convert_uint16_2int16 (Temp_data_16) * Mul) , Div + Obis_Gelex_Ins[RowObis103].scale);
                                if(Obis_Gelex_Ins[RowObis103].Str_Unit != NULL)
                                {
                                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                                    Copy_String_2(&Str_Data_Write, Obis_Gelex_Ins[RowObis103].Str_Unit);  //don vi
                                }
                                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                                    
                                Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Data_Inbuff, &Str_Data_Write);
                                Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
                            }else //khong dau
                            {
                                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                                Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t)(Temp_data_16 *Mul) , Div + Obis_Gelex_Ins[RowObis103].scale);
                                if(Obis_Gelex_Ins[RowObis103].Str_Unit!= NULL)
                                {
                                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                                    Copy_String_2(&Str_Data_Write, Obis_Gelex_Ins[RowObis103].Str_Unit);  //don vi
                                }
                                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                                
                                Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Data_Inbuff, &Str_Data_Write);
                                Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
                            }		
                        }
                    }
                }
				break;
			case 0x17:
                TempObis = GELEX_MLProfile_OBIS_List[Get_Meter_LProf.OBIS_Pointer_ui8++];
                for (j=4;j>0;j--)
                        Temp_data_32 = (Temp_data_32 << 8) | Meter_TempBuff[i+j];	
                
                if(TempObis != 0xFF)
                {
                    RowObis103 = GELEX_Check_Row_Obis103(&Obis_Gelex_Ins[0], TempObis);
                    if(RowObis103 >=0)
                    {
                        if(Obis_Gelex_Ins[RowObis103].StrObis_Lpf != NULL)
                        {
                            //ghi obis vao
                            Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Obis_Inbuff, Obis_Gelex_Ins[RowObis103].StrObis_Lpf);
                            Get_Meter_LProf.Pos_Obis_Inbuff += Obis_Gelex_Ins[RowObis103].StrObis_Lpf->Length_u16;
                            Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
                            Get_Meter_LProf.Numqty++;
                            //ghi data
                            ScaleRead = GELEX_ConvertScaleMeter_toDec (Obis_Gelex_Ins[RowObis103].Scale_ReadMeter, &Div);
                            for(j = 0; j < ScaleRead; j++)
                                Mul = Mul *10;
                            
                            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                            Pack_HEXData_Frame(&Str_Data_Write, (int64_t)(Convert_float_2int(Temp_data_32, 0) * Mul), Div + Obis_Gelex_Ins[RowObis103].scale);
                            if(Obis_Gelex_Ins[RowObis103].Str_Unit!= NULL)
                            {
                                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                                Copy_String_2(&Str_Data_Write, Obis_Gelex_Ins[RowObis103].Str_Unit);  //don vi cua bieu gi�
                            }
                            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                                
                            Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Data_Inbuff, &Str_Data_Write);
                            Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
                        }
                    }
                }
                i+=5;
				break;
			default:
				Get_Meter_LProf.Error_ui8 = 0x10;
				break;
		}
		if (Get_Meter_LProf.Error_ui8 == 0x10)
			return 0;
        
		if (Get_Meter_LProf.OBIS_Pointer_ui8 >= Get_Meter_LProf.Total_OBIS_ui8)
        {
//            if((Get_Meter_LProf.Str_Payload.Length_u16 > 1000) || (Get_Meter_LProf.Reading_Block_ui8 == Get_Meter_LProf.Num_Block_ui8))
//            {
                Add_TuTI_toPayload(&Get_Meter_LProf);
                 //lay het data moi ghep stime lpf vao
                Reset_Buff(&Str_Data_Write);
                Copy_String_2(&Str_Data_Write, &StrSTime_Bill);  //time start lpf
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
            
                if(Get_Meter_LProf.Numqty >3)
                    if(Send_MessLpf_toQueue() == 0) return 0;
                
                Get_Meter_LProf.Flag_Start_Pack = 0;
                Get_Meter_LProf.OBIS_Pointer_ui8 = 0;
//                GE_Fl_Alow_GetTimeLpf = 0;
//            }
//            else
//            {
//                GE_Fl_Alow_GetTimeLpf = 1;
//                *(Get_Meter_LProf.Str_Payload.Data_a8 + Get_Meter_LProf.Str_Payload.Length_u16++) = 0x0D;
//                *(Get_Meter_LProf.Str_Payload.Data_a8 + Get_Meter_LProf.Str_Payload.Length_u16++) = 0x0A;
//                Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
//            }
        }	
	}
	
	if (Get_Meter_LProf.Num_Record_ui16 == 0)
		return 0;
//	
//	if (((Get_Meter_LProf.Reading_Block_ui8%2)==1)||(Get_Meter_LProf.Reading_Block_ui8==Get_Meter_LProf.Num_Block_ui8))
//	{
//		Send_MessLpf_toQueue();
//        Get_Meter_LProf.Flag_Start_Pack = 0;
//	}

    Get_Meter_LProf.Data_Buff_Pointer_ui16 = 10;
	return 1;
}


                                                                                                         
void GELEX_MEventGetRawData (uint8_t Mess_Type)
{
	uint16_t i=0;
	uint8_t data_pos;
	
	if (Get_Meter_Event.Mess_Step_ui8 == 2)
	{
		switch (Mess_Type)
		{
			case 1:
			case 2:
			case 4:
				if (UART1_Receive_Buff[12] == 0x01)
				{
					data_pos = getObjectCount(UART1_Receive_Buff,13);
					for (i=0;i<UART1_Control.Mess_Length_ui16-data_pos-2;i++)
						Meter_TempBuff[Get_Meter_Event.Data_Buff_Pointer_ui16++] = UART1_Receive_Buff[i+data_pos];
				}
				else
				{	
					data_pos = getObjectCount(UART1_Receive_Buff,20);
					for (i=0;i<UART1_Control.Mess_Length_ui16-data_pos-2;i++)
						Meter_TempBuff[Get_Meter_Event.Data_Buff_Pointer_ui16++] = UART1_Receive_Buff[i+data_pos];
				}
				break;
			case 3:
				break;
			default:
				break;
		}
	}
	else
		for (i=0;i<UART1_Control.Mess_Length_ui16-10;i++)
			Meter_TempBuff[Get_Meter_Event.Data_Buff_Pointer_ui16++] = UART1_Receive_Buff[i+8];	
}

uint8_t GELEX_MEventExtractData (void)
{
	uint16_t 	Data_Length = 0,i=10,j=0;
	uint8_t		Data_OBIS = 0;
	uint8_t		Temp_OBIS_Data[10];
	
	Data_Length = Get_Meter_Event.Data_Buff_Pointer_ui16;
	Get_Meter_Event.Data_Buff_Pointer_ui16 = Get_Meter_Event.Last_Data_Buff_Pointer_ui16;
	
	while (i<Data_Length)
	{
		Data_OBIS = Meter_TempBuff[i];
		
		switch (Data_OBIS)
		{
			case 0x01:
                if(Meter_TempBuff[i+1] == 0x81)
                {
                    Get_Meter_Event.Num_Record_ui16 = Meter_TempBuff[i+2];
                    i++;
                }else Get_Meter_Event.Num_Record_ui16 = Meter_TempBuff[i+1];
				i += 2;
				break;
			case 0x02:
				i += 2;
				break;
			case 0x12: // add
				i += 3;
				break;
			case 0x09:
				for (j=0;j<8;j++)
					Temp_OBIS_Data[j+2] = Meter_TempBuff[i+j+2];
				i += 14;
				break;
			case 0x11:
				if (MEventOBISFilter(Meter_TempBuff[i+1]) == 1)
				{
					Temp_OBIS_Data[0] = Meter_TempBuff[i+1];
					Temp_OBIS_Data[1] = 0x08;
					for (j=0;j<10;j++)
						MeterEventDataBuff_2[Get_Meter_Event.Data_Buff_Pointer_ui16++] = Temp_OBIS_Data[j];
				}
				Get_Meter_Event.Received_Record_ui16++;
				i += 2;
				break;
			default:
				Get_Meter_Event.Error_ui8 = 0x10;
				break;
		}
		if (Get_Meter_Event.Error_ui8 == 0x10)
			return 0;
	};
	
//	if (Get_Meter_Event.Num_Record_ui16 >0)//de k co data thi gui len rong
		if (MEventSendData() == 0)
			return 0;

	return 1;
}

uint8_t MTamperExtractData (void)
{
	uint16_t 	Data_Length = 0,i=10,j=0;
	uint8_t		Data_OBIS = 0;
	uint8_t		Temp_OBIS_Data[10];
	
	Data_Length = Get_Meter_Event.Data_Buff_Pointer_ui16;
	Get_Meter_Event.Data_Buff_Pointer_ui16 = Get_Meter_Event.Last_Data_Buff_Pointer_ui16;
	
	while (i<Data_Length)
	{
		Data_OBIS = Meter_TempBuff[i];
		
		switch (Data_OBIS)
		{
			case 0x01:
                 if(Meter_TempBuff[i+1] == 0x81)
                {
                    Get_Meter_Event.Num_Record_ui16 = Meter_TempBuff[i+2];
                    i++;
                }else Get_Meter_Event.Num_Record_ui16 = Meter_TempBuff[i+1];
				i += 2;
				break;
			case 0x02:
				i += 2;
				break;
			case 0x12: // add
				i += 3;
				break;
			case 0x09:
				for (j=0;j<8;j++)
					Temp_OBIS_Data[j+2] = Meter_TempBuff[i+j+2];
				i += 14;
				break;
			case 0x11:
				if (MEventOBISFilter(Meter_TempBuff[i+1]) == 1)
				{
					Temp_OBIS_Data[0] = Meter_TempBuff[i+1];
					Temp_OBIS_Data[1] = 0x08;
					for (j=0;j<10;j++)
						MeterEventDataBuff_2[Get_Meter_Event.Data_Buff_Pointer_ui16++] = Temp_OBIS_Data[j];
				}
				Get_Meter_Event.Received_Record_ui16++;
				i += 7;  //test roi
				break;
			default:
				Get_Meter_Event.Error_ui8 = 0x10;
				break;
		}
		if (Get_Meter_Event.Error_ui8 == 0x10)
			return 0;
	};

	if (Get_Meter_Event.Num_Record_ui16 >0)
		if (MEventSendData() == 0)
			return 0;

	return 1;
}

uint8_t MAlarmExtractData (void)
{
	uint16_t 	Data_Length = 0,i=10,j=0;
	uint8_t		Data_OBIS = 0;
	uint8_t		Temp_OBIS_Data[10];
	
	Data_Length = Get_Meter_Event.Data_Buff_Pointer_ui16;
	Get_Meter_Event.Data_Buff_Pointer_ui16 = Get_Meter_Event.Last_Data_Buff_Pointer_ui16;
	
	while (i<Data_Length)
	{
		Data_OBIS = Meter_TempBuff[i];
		
		switch (Data_OBIS)
		{
			case 0x01:
                 if(Meter_TempBuff[i+1] == 0x81)
                {
                    Get_Meter_Event.Num_Record_ui16 = Meter_TempBuff[i+2];
                    i++;
                }else Get_Meter_Event.Num_Record_ui16 = Meter_TempBuff[i+1];
				i += 2;
				break;
			case 0x02:
				i += 2;
				break;
			case 0x12: // add
				i += 3;
				break;
			case 0x09:
				for (j=0;j<8;j++)
					Temp_OBIS_Data[j+2] = Meter_TempBuff[i+j+2];
				i += 14;
				break;
			case 0x11:
				if (MEventOBISFilter(Meter_TempBuff[i+1]) == 1)
				{				
					Temp_OBIS_Data[0] = Meter_TempBuff[i+1];  
					Temp_OBIS_Data[1] = 0x08;
					for (j=0;j<10;j++)
						MeterEventDataBuff_2[Get_Meter_Event.Data_Buff_Pointer_ui16++] = Temp_OBIS_Data[j];
				}
				Get_Meter_Event.Received_Record_ui16++;
				i += 2 ;   //+=8
 				break;
			default:
				Get_Meter_Event.Error_ui8 = 0x10;
				break;
		}
		if (Get_Meter_Event.Error_ui8 == 0x10)
			return 0;
	};

	if (Get_Meter_Event.Num_Record_ui16 >0)
		if (MEventSendData() == 0)
			return 0;

	return 1;
}





uint8_t GELEX_Pack_event_103Pushdata (void)
{
    uint16_t        i = 10;
    int8_t          Temp_obis = 0;
    ST_TIME_FORMAT  sTime_temp;
    uint16_t        Tempyear = 0;
    uint8_t         Buff[20];
    truct_String    Str_Data_Write = {&Buff[0], 0};
	uint8_t         Temp_BBC = 0;
    
    if(Get_Meter_Event.Data_Buff_Pointer_ui16 == 10)
    {
        Reset_Buff(&Get_Meter_Event.Str_Payload);
        Header_event_103(&Get_Meter_Event.Str_Payload, 0);    //header k c� stime. de trong stime
        //them tuti
        Add_TuTI_toPayload(&Get_Meter_Event);
            
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
        Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) Get_Meter_Event.Numqty, 0);
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')'; 
        
        Copy_String_toTaget(&Get_Meter_Event.Str_Payload, Get_Meter_Event.PosNumqty, &Str_Data_Write);
        Get_Meter_Event.PosNumqty += Str_Data_Write.Length_u16;
        Get_Meter_Event.Pos_Obis_Inbuff += Str_Data_Write.Length_u16;
        Get_Meter_Event.Pos_Data_Inbuff = Get_Meter_Event.Str_Payload.Length_u16;
        //
        //ETX
        *(Get_Meter_Event.Str_Payload.Data_a8 + Get_Meter_Event.Str_Payload.Length_u16++) = ETX; 
        //BBC
        Temp_BBC = BBC_Cacul(Get_Meter_Event.Str_Payload.Data_a8 + 1,Get_Meter_Event.Str_Payload.Length_u16 - 1);
        *(Get_Meter_Event.Str_Payload.Data_a8 + Get_Meter_Event.Str_Payload.Length_u16++) = Temp_BBC; 
        
        return Push_Even_toQueue(0);
    }
    
    while (i < Get_Meter_Event.Data_Buff_Pointer_ui16)
    {
        if(Get_Meter_Event.Flag_Start_Pack == 0)
        {
            Reset_Buff(&Get_Meter_Event.Str_Payload);
            Header_event_103(&Get_Meter_Event.Str_Payload, 1);
            Get_Meter_Event.Flag_Start_Pack = 1;
        }
        
        Temp_obis = Check_Obis_event(&Obis_Gelex_Event[0], MeterEventDataBuff_2[i], GELEX_MAX_EVENT);
        if((Temp_obis) > 0)
        {
            i += 2;
            Copy_String_toTaget(&Get_Meter_Event.Str_Payload, Get_Meter_Event.Pos_Obis_Inbuff, Obis_Gelex_Event[Temp_obis].Str_Obis);
            Get_Meter_Event.Pos_Obis_Inbuff += Obis_Gelex_Event[Temp_obis].Str_Obis->Length_u16;
            Get_Meter_Event.Pos_Data_Inbuff = Get_Meter_Event.Str_Payload.Length_u16;
            Get_Meter_Event.Numqty++;
            //pack time su kien
            Tempyear         = (MeterEventDataBuff_2[i] << 8) | (MeterEventDataBuff_2[i+1]);
            sTime_temp.year  = Tempyear%100;
            sTime_temp.month = MeterEventDataBuff_2[i+2];
            sTime_temp.date  = MeterEventDataBuff_2[i+3];
            sTime_temp.day   = MeterEventDataBuff_2[i+4];
            sTime_temp.hour  = MeterEventDataBuff_2[i+5];
            sTime_temp.min   = MeterEventDataBuff_2[i+6];
            sTime_temp.sec   = MeterEventDataBuff_2[i+7];
            //
            *(Get_Meter_Event.Str_Payload.Data_a8 + Get_Meter_Event.Str_Payload.Length_u16++) = '('; 
            Copy_String_STime(&Get_Meter_Event.Str_Payload,sTime_temp);
            *(Get_Meter_Event.Str_Payload.Data_a8 + Get_Meter_Event.Str_Payload.Length_u16++) = ')'; 
            Get_Meter_Event.Pos_Data_Inbuff = Get_Meter_Event.Str_Payload.Length_u16;
            i += 8;
        }else i+=10;
        
        if((Get_Meter_Event.Str_Payload.Length_u16 >=1000) || (i >= Get_Meter_Event.Data_Buff_Pointer_ui16))
        {
            Add_TuTI_toPayload(&Get_Meter_Event);
            
            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
            Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) Get_Meter_Event.Numqty, 0);
            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')'; 
            
            Copy_String_toTaget(&Get_Meter_Event.Str_Payload, Get_Meter_Event.PosNumqty, &Str_Data_Write);
            Get_Meter_Event.PosNumqty += Str_Data_Write.Length_u16;
            Get_Meter_Event.Pos_Obis_Inbuff += Str_Data_Write.Length_u16;
            Get_Meter_Event.Pos_Data_Inbuff = Get_Meter_Event.Str_Payload.Length_u16;
            //
            //ETX
            *(Get_Meter_Event.Str_Payload.Data_a8 + Get_Meter_Event.Str_Payload.Length_u16++) = ETX; 
            //BBC
            Temp_BBC = BBC_Cacul(Get_Meter_Event.Str_Payload.Data_a8 + 1,Get_Meter_Event.Str_Payload.Length_u16 - 1);
            *(Get_Meter_Event.Str_Payload.Data_a8 + Get_Meter_Event.Str_Payload.Length_u16++) = Temp_BBC;            
            
            Get_Meter_Event.Flag_Start_Pack = 0;   //cho phep dong goi head tiep
            //gui thang sang queue Sim de gui luon
            
            if(Push_Even_toQueue(0) == 0)
              return 0;
        }
    }
    
    return 1;
}



uint8_t GELEX_PackEvent_TSVH(void)
{
    uint16_t        i = 10;
    int8_t          Temp_obis = 0;
    ST_TIME_FORMAT  sTime_temp;
    uint16_t        Tempyear = 0;
    truct_String    StrTemp;
    
    if(Get_Meter_Event.Data_Buff_Pointer_ui16 == 10) return 0;
    
    while (i < Get_Meter_Event.Data_Buff_Pointer_ui16)
    {        
        Temp_obis = Check_Obis_event(&Obis_Gelex_Event[0], MeterEventDataBuff_2[i], GELEX_MAX_EVENT);
        if((Temp_obis == 2) || (Temp_obis == 12) || (Temp_obis == 13) || (Temp_obis == 14) || ((Temp_obis >= 21) && (Temp_obis <= 26)))   //row dung voi c�c thong so can lay
        {
            i += 2;
            Copy_String_toTaget(&Get_Meter_Event.Str_Payload, Get_Meter_Event.Pos_Obis_Inbuff, Obis_Gelex_Event[Temp_obis].Str_Obis);
            Get_Meter_Event.Pos_Obis_Inbuff += Obis_Gelex_Event[Temp_obis].Str_Obis->Length_u16;
            Get_Meter_Event.Pos_Data_Inbuff = Get_Meter_Event.Str_Payload.Length_u16;
            Get_Meter_Event.Numqty++;
            //pack time su kien
            Tempyear         = (MeterEventDataBuff_2[i] << 8) | (MeterEventDataBuff_2[i+1]);
            sTime_temp.year  = Tempyear%100;
            sTime_temp.month = MeterEventDataBuff_2[i+2];
            sTime_temp.date  = MeterEventDataBuff_2[i+3];
            sTime_temp.day   = MeterEventDataBuff_2[i+4];
            sTime_temp.hour  = MeterEventDataBuff_2[i+5];
            sTime_temp.min   = MeterEventDataBuff_2[i+6];
            sTime_temp.sec   = MeterEventDataBuff_2[i+7];
            //
            *(Get_Meter_Event.Str_Payload.Data_a8 + Get_Meter_Event.Str_Payload.Length_u16++) = '('; 
            Copy_String_STime(&Get_Meter_Event.Str_Payload,sTime_temp);
            *(Get_Meter_Event.Str_Payload.Data_a8 + Get_Meter_Event.Str_Payload.Length_u16++) = ')'; 
            Get_Meter_Event.Pos_Data_Inbuff = Get_Meter_Event.Str_Payload.Length_u16;
            i += 8;
        }else i+=10;
    }
    //cat vao buff TSVH luon
    StrTemp.Data_a8 = &MeterEventDataBuff[0];
    StrTemp.Length_u16 = Get_Meter_Event.Pos_Obis_Inbuff;
    Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Obis_Inbuff, &StrTemp);  //ghi obis vao
    Get_Meter_Info.Pos_Obis_Inbuff += StrTemp.Length_u16;
    Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
    
    StrTemp.Data_a8 = &MeterEventDataBuff[Get_Meter_Event.Pos_Obis_Inbuff];
    StrTemp.Length_u16 = Get_Meter_Event.Pos_Data_Inbuff - Get_Meter_Event.Pos_Obis_Inbuff;
    Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Data_Inbuff, &StrTemp);  //ghi data vao
    Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
    
    Get_Meter_Info.Numqty += Get_Meter_Event.Numqty;
    return 1;
}



uint8_t MEventSendData (void)
{
	if ((((Get_Meter_Event.Reading_Block_ui8+1)%5)==0)||(Get_Meter_Event.Received_Record_ui16==Get_Meter_Event.Num_Record_ui16))
	{
		//Send to queue func convert Old Pack to new Pack
        if(sDCU.Flag_ReadEventSVH == 1)
            GELEX_PackEvent_TSVH();
        else
            GELEX_Pack_event_103Pushdata();
		        
		Get_Meter_Event.Last_Data_Buff_Pointer_ui16 = 10;
		Get_Meter_Event.Data_Buff_Pointer_ui16 = 10;
	}
	else
	{
		Get_Meter_Event.Last_Data_Buff_Pointer_ui16 = Get_Meter_Event.Data_Buff_Pointer_ui16;
		Get_Meter_Event.Data_Buff_Pointer_ui16 = 10;	
	}
	return 1;
}

uint8_t MEventOBISFilter (uint8_t Received_OBIS)
{
	uint8_t mReVal=0,i=0;
	
	for (i=0;i<GELEX_MAX_EVENT;i++)
		if (Received_OBIS == Obis_Gelex_Event[i].ID_event)
		{
			mReVal = 1; 
			break;
		}
	return mReVal;
}

uint8_t GELEX_Get_Meter_ID (uint32_t Temp)
{
	uint8_t status =0, retry =0, step_handshake=0;
	// Init handshake
	while(retry<1){
		switch(step_handshake)
		{
			case 0:
				Init_UART2();
				RS485_SEND;
				osDelay(READ_DELAY);
				HAL_UART_Transmit(&UART_METER,&GELEX_SNRM[0],9,1000);
				RS485_RECIEVE;
				step_handshake++;
			break;
			case 1:
				if(osSemaphoreWait(bsUART2PendingMessHandle,3000) == osOK){    //if(UART1_Receive_Buff[5] != 0x30) 
                    if(UART1_Receive_Buff[5] == 0x73) step_handshake++;
                    else step_handshake = 4;
					Init_UART2();
				}
				else
                {
                    step_handshake = 4;
                }
				break;
			case 2:
				Init_UART2();
				RS485_SEND;
				osDelay(READ_DELAY);
				HAL_UART_Transmit(&UART_METER,&GELEX_AARQ[0],74,1000);
				RS485_RECIEVE;
				step_handshake++;
			break;
			case 3:
				if(osSemaphoreWait(bsUART2PendingMessHandle,3000) == osOK){
                   if(UART1_Receive_Buff[5] == 0x30) // AARQ
                   {
                     step_handshake++;
                     status = 1;
                   }else step_handshake = 4;   
					Init_UART2();
				}
				else
				{
					step_handshake = 4;
					Get_Meter_Info.Error_Meter_Norespond_ui32++;	// Bao WarDCU
				}
				break;
			case 4:
				UART1_Control.Mess_Pending_ui8 = 1; // Not wait semaphore, not check
				RS485_SEND;
				osDelay(READ_DELAY);
				HAL_UART_Transmit(&UART_METER,&GELEX_Read_END[0],9,1000);
				RS485_RECIEVE;
				if(status==1)
				{
					retry=3;  // out;
					break;
				}
				else
				{
					retry++;
					step_handshake = 0;
				}
				osDelay(2000);
				break;
		}
	}
	return status;
}
//
uint8_t GELEX_CheckResetReadMeter(uint32_t Timeout)
{
	// Reset neu doc sai cong to
    if ((sDCU.Status_Meter_u8 == 0) &&
            (Check_Time_Out(sDCU.LandMark_Count_Reset_Find_Meter,600000) == TRUE)) // 10p
    {
        Read_Meter_ID_Success = GELEX_Get_Meter_ID(0); 
        if(Read_Meter_ID_Success != 1) // Reset MCU
        {
            _fPackStringToLog ((uint8_t*) "Check reset meter va reset Chip\r\n", 33);
            osDelay(1000);
            __disable_irq();
            NVIC_SystemReset(); // Reset MCU	
        } else
            sDCU.LandMark_Count_Reset_Find_Meter = RT_Count_Systick_u32;
    }
    return 1;
}






uint8_t GELEX_TuTi_ExtractDataFunc (void)
{
    uint8_t     Data_type=0,i=0,j=0;
    uint32_t    Temp_data_32 = 0;
    uint16_t    Temp_data_16 = 0;
    uint8_t     BuffNum[30] = {0};
    truct_String    Str_Data_Write={&BuffNum[0], 0};
//    uint8_t         ScaleRead = 1;
    int8_t          RowObis103 = 0;
//    uint8_t         Div = 0;
//    uint64_t        Mul = 1;
    
	Data_type = UART1_Receive_Buff[15];
	j = GELEX_MeterTuTiMessIDTable[Get_Meter_TuTi.Mess_Step_ui8-2];
    
    switch (Data_type)
	{
		case 0x06:     //06:  uint 32 ( 4 byte Data)  d?i gi� tr? t? hex -> decimal
            RowObis103 = GELEX_Check_Row_Obis103(&Obis_Gelex_Ins[0], GELEX_MInfo_Table[j][0]);
            if(RowObis103 >=0)
            {
                if(Obis_Gelex_Ins[RowObis103].Str_Obis != NULL)
                {     
                    //ghi data
                    for (i=0;i<4;i++)
                        Temp_data_32 = (Temp_data_32 << 8) | UART1_Receive_Buff[16+i];
//                    ScaleRead = GELEX_ConvertScaleMeter_toDec (Obis_Gelex_Ins[RowObis103].Scale_ReadMeter, &Div);
//                    for(j = 0; j < ScaleRead; j++)
//                        Mul = Mul *10;
                    
                    //o d�y l� so khong dau
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
//                    Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) (Temp_data_32 * Mul), Div + Obis_Gelex_Ins[RowObis103].scale);    //Theo tai lieu la k chia scale read
                    Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) (Temp_data_32), 0); 
                    if(Obis_Gelex_Ins[RowObis103].Str_Unit!= NULL)
                    {
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                        Copy_String_2(&Str_Data_Write, Obis_Gelex_Ins[RowObis103].Str_Unit);  //don vi
                    }
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                    
                    Copy_String_2(&Get_Meter_TuTi.Str_Payload , &Str_Data_Write);
                }
            }
			break;
        case 0x12: 
            if(GELEX_MInfo_Table[j][0] > 0x45)
            {
                //chi lay Tim va Tum: chi ghi /value  vao vi tri posdata - 1
                if((GELEX_MInfo_Table[j][0] == 0x64) || (GELEX_MInfo_Table[j][0] == 0x65))
                {
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++)= '/';
                    for (i=0;i<2;i++)
                        Temp_data_16 = (Temp_data_16 << 8) | UART1_Receive_Buff[16+i];
                         
                    Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) (Temp_data_16/GELEX_SCALE_TUM_TIM), 0); 
                    
                    Get_Meter_TuTi.Str_Payload.Length_u16 --;
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++)= ')';                    
                    Copy_String_2(&Get_Meter_TuTi.Str_Payload , &Str_Data_Write);
                }
            }
            break;
        default: 
            break;
    }
    
    return 1;
}
		
void GELEX_TuTi_SendData(void)
{
    *(Get_Meter_TuTi.Str_Payload.Data_a8 + Get_Meter_TuTi.Str_Payload.Length_u16++) = '(';
    Pack_HEXData_Frame_Uint64(&Get_Meter_TuTi.Str_Payload, (uint64_t) sDCU.He_So_Nhan, GELEX_SCALE_HE_SO_NHAN); 
    *(Get_Meter_TuTi.Str_Payload.Data_a8 + Get_Meter_TuTi.Str_Payload.Length_u16++) = ')';
}



uint8_t GELEX_Check_Meter(void)
{
    uint8_t Check_Found = 0;
    
    // Check Error of Meter ID
	if (Read_Meter_ID_Success == 1) 
	{ 
        Check_Found = Get_Meter_Info.Error_Meter_Norespond_ui32 | Get_Meter_Billing.Error_Meter_Norespond_ui32 | Get_Meter_Event.Error_Meter_Norespond_ui32 |
					Get_Meter_LProf.Error_Meter_Norespond_ui32 | Get_Meter_Alert.Error_Meter_Norespond_ui32;
		// Not found Meter
		if (Check_Found >= 2) 
		{ // Mat ket noi cong to - 5 lan doc loi
			sDCU.Status_Meter_u8 = 3; // Meter no Response
			Get_Meter_Info.Error_Meter_Norespond_ui32 		= 0;
			Get_Meter_Billing.Error_Meter_Norespond_ui32 	= 0;
			Get_Meter_Event.Error_Meter_Norespond_ui32  	= 0;
			Get_Meter_LProf.Error_Meter_Norespond_ui32 		= 0;
			Get_Meter_Alert.Error_Meter_Norespond_ui32		= 0;
			// Doc lai Cong to
			Read_Meter_ID_Success = GELEX_Get_Meter_ID(0);
        }
	} 
    return 1;
}  



uint8_t GELEX_ConvertScaleMeter_toDec (uint8_t ScaleMeter, uint8_t* Div)
{
    uint8_t     Result = 0;
    
    switch (ScaleMeter)
    {
        case 0xFF:
            Result = 0;
            *Div = 1;
            break;
        case 0xFE:
            Result = 0;
            *Div = 2;
            break;
        case 0xFD:
            Result = 0;
            *Div = 3;
            break;
        case 0xFC:
            Result = 0;
            *Div = 4;
            break;
        default:
            if(ScaleMeter < 7)
                Result = ScaleMeter;
            *Div = 0;
            break;
    }
    return Result;
}





