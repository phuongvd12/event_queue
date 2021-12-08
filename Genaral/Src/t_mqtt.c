
#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"

#include "variable.h"
#include "t_mqtt.h"
#include "usart.h"
#include "myuart.h"
#include "usart.h"
#include "sim900.h"
#include "onchipflash.h"
#include "S25FL216K.h"
#include "pushdata.h"


uint8_t aServer_MQTT[DCU_ID_LENGTH];
uint8_t aServer_MQTT_RQ[DCU_ID_LENGTH];
uint8_t aPort_MQTT_RQ[PASSWORD_LENGTH];
uint8_t aPort_MQTT[PASSWORD_LENGTH];
uint8_t aBuff_Alarm_Power[MAX_LENGTH_ALARM_POW];

extern xQueueHandle qMeter_SIM900Handle;
extern xQueueHandle qFlash_SIM900Handle;
extern xQueueHandle qSIM_SIM_UpdateHandle;

const struct_MARK_TYPE_Message_SEND sMark_MessageSend_Type[] = {
		{DATA_temp				,NULL				            ,{NULL}},
// Ban tin can phai co dia chi Flash
        {DATA_PRE_OPERA			,_mDATA_GET_FROM_QUEUE	        ,{NULL}},
		{DATA_OPERATION			,_mDATA_GET_FROM_QUEUE	        ,{NULL}},
        {DATA_LOAD_PROFILE		,_mDATA_GET_FROM_QUEUE          ,{NULL}},
        {DATA_PRE_HISTORICAL	,_mDATA_GET_FROM_QUEUE	        ,{NULL}}, 
		{DATA_HISTORICAL		,_mDATA_GET_FROM_QUEUE	        ,{NULL}},
        
		{DATA_EVEN_METER		,_mDATA_GET_FROM_QUEUE	        ,{NULL}},
		{DATA_INTANTANIOUS		,_mDATA_GET_FROM_QUEUE          ,{NULL}},	
        
        {DATA_METER_INFOR		,_mDATA_GET_FROM_QUEUE          ,{NULL}},	
		{DATA_LOG_DCU		    ,_mDATA_LOG_DCU                 ,{NULL}},  
        
        {DATA_RESPOND_AT		,_mDATA_AT_RESPOND              ,{NULL}},
        {DATA_TUTI		        ,NULL                           ,{NULL}},
		
        // Ban in ko can co dia chi Flash ban sang
        {NOTIF_CONNECT_METER		,_mDATA_NOTIF_CONNECT_MET           ,{NULL}},
        {NOTIF_DISCONNECT_METER		,_mDATA_NOTIF_DISCONNECT_MET        ,{NULL}},
        
        {ALARM_POWER_ON_MODERM		,_mDATA_ALARM_P_ON_MOD              ,{NULL}},
        {ALARM_POWER_OFF_MODERM		,_mDATA_ALARM_P_OFF_MOD             ,{NULL}},
        {ALARM_POWER_UP_METER		,_mDATA_ALARM_P_UP_MET              ,{NULL}},
        {ALARM_POWER_DOWN_METER		,_mDATA_ALARM_P_DOW_MET             ,{NULL}},
        //
        {UPDATE_FIRM_OK		        ,_mUPDATE_FIRM_SUCCES           ,{NULL}},
        {UPDATE_FIRM_FAIL		    ,_mUPDATE_FIRM_FAIL             ,{NULL}},   
};






//=============================================================
/*
 * 			CONTROL DATA SEND
*/

void Init_DCU(void)
{
	uint8_t var;
    uint16_t Length_ID = 0;
    
	// Load RTC
	Get_RTC();

	sDCU.Mode_Connect_Now 	=	MODE_CONNECT_DATA; 
	sDCU.Mode_Connect_Future 	= 0;

    // Init
	init_Sim900();
	Init_DCU_METER_ID();
	Init_Port_MQTT();

	for (var = DATA_PRE_OPERA; var <= DATA_METER_INFOR; ++var)  
    {
		sStatus_Meter.Flag[var] = 1;
        sTCP.aCountRetryMess[var] = 0;   //Init lai CountRetry Send mess cua cac ban tin 
    }
        
    //2020
    sDCU.StrPing_Recei.Data_a8 = &Buff_Recei_Ping[0];
    sDCU.StrPing_Recei.Length_u16 = 0;
    sDCU.Fl_Finish_Ping = 0;
    //He so nhan mac dinh bang 1
    sDCU.He_So_Nhan = 1;
    //dat con trol ban tin heartbeat
    sHandsake.Data_a8 = aBuffHansdshake; // ban tin bat tay voi Server
    
    // Time Allow send Publish Message
    Length_ID = sDCU.sDCU_id.Length_u16-1;
	sTCP.Compare_Allow_Send_DATA_u16 = (sDCU.sDCU_id.Data_a8[Length_ID] - 0x30 + (sDCU.sDCU_id.Data_a8[Length_ID-1] - 0x30)*10 + (sDCU.sDCU_id.Data_a8[Length_ID-2] - 0x30) * 100);
    
    sTCP.Compare_Allow_Send_LPF_u16  = sTCP.Compare_Allow_Send_DATA_u16 / 5 * 60;       //Max 200 phut * 60 (s)
    sTCP.Compare_Allow_Send_DATA_u16 = sTCP.Compare_Allow_Send_DATA_u16 / 2;            //Max 500 s
    
    sTCP.Min_Allow_Send_DATA_u8 = sTCP.Compare_Allow_Send_DATA_u16/60;              // min for read His 

	sTCP.Landmark_Allow_Send_DATA_u32 = 0x7FFFFFFF;
    sTCP.Landmark_Allow_Send_LPF_u32 = 0x7FFFFFFF;
}

void Init_Port_MQTT(void)
{
	uint8_t     temp=0;
    uint16_t    i = 0;
    uint8_t tempBuff[32];
	//MQTT server info
	temp = *(__IO uint8_t*)ADDR_MQTTSERVER; 
	if (temp != 0xFF)
	{
		sInformation.sServer_MQTT.Length_u16 = *(__IO uint8_t*)(ADDR_MQTTSERVER+1);
        if(sInformation.sServer_MQTT.Length_u16 > DCU_ID_LENGTH)
            sInformation.sServer_MQTT.Length_u16 = DCU_ID_LENGTH;
		OnchipFlashReadData(ADDR_MQTTSERVER+2,aServer_MQTT,sInformation.sServer_MQTT.Length_u16);
		sInformation.sServer_MQTT.Data_a8 = aServer_MQTT;

		sInformation.sPort_MQTT.Length_u16 = *(__IO uint8_t*)(ADDR_MQTTSERVER+2+sInformation.sServer_MQTT.Length_u16);
        if(sInformation.sPort_MQTT.Length_u16 > PASSWORD_LENGTH)
            sInformation.sPort_MQTT.Length_u16 = PASSWORD_LENGTH;
        
		OnchipFlashReadData(ADDR_MQTTSERVER+3+sInformation.sServer_MQTT.Length_u16,aPort_MQTT,sInformation.sPort_MQTT.Length_u16);  //29/5/21Loi day sua  sInformation.sServer_MQTT.Length_u16  
		sInformation.sPort_MQTT.Data_a8 = aPort_MQTT;
	}
	else	//124.158.5.155:1883
	{
        aServer_MQTT[0] = '1'; aServer_MQTT[1] = '7'; aServer_MQTT[2] = '2'; aServer_MQTT[3] = '.';
        aServer_MQTT[4] = '1'; aServer_MQTT[5] = '6'; aServer_MQTT[6] = '.'; aServer_MQTT[7] = '1';
        aServer_MQTT[8] = '3'; aServer_MQTT[9] = '.';
        aServer_MQTT[10] = '1'; aServer_MQTT[11] = '3';                                                  //172.16.13.13
        sInformation.sServer_MQTT.Length_u16 = 12;
        sInformation.sServer_MQTT.Data_a8 = aServer_MQTT;

        aPort_MQTT[0] = '1'; aPort_MQTT[1] = '3'; aPort_MQTT[2] = '3'; aPort_MQTT[3] = '0';             //1330
        sInformation.sPort_MQTT.Length_u16 = 4;
        sInformation.sPort_MQTT.Data_a8 = aPort_MQTT;
        
        tempBuff[0] = 2;
        tempBuff[1] = sInformation.sServer_MQTT.Length_u16;
        for (i=0;i<sInformation.sServer_MQTT.Length_u16;i++)
            tempBuff[i+2] = *(sInformation.sServer_MQTT.Data_a8+i);
        tempBuff[sInformation.sServer_MQTT.Length_u16+2] = sInformation.sPort_MQTT.Length_u16;
        for (i=0;i<sInformation.sPort_MQTT.Length_u16;i++)
            tempBuff[sInformation.sServer_MQTT.Length_u16+i+3] = *(sInformation.sPort_MQTT.Data_a8+i);
        
        OnchipFlashPageErase(ADDR_MQTTSERVER);
        OnchipFlashWriteData(ADDR_MQTTSERVER,tempBuff,32);
	} 

    //init buff rq
    sInformation.sServer_MQTT_Request.Length_u16 = 0;
    sInformation.sServer_MQTT_Request.Data_a8 = aServer_MQTT_RQ;
    sInformation.sPort_MQTT_Request.Length_u16 = 0;
    sInformation.sPort_MQTT_Request.Data_a8 = aPort_MQTT_RQ;
}


void Fill_Table (void)
{
    Meter_Flash_Queue_Struct *sSend_Update;
    
    if (uxQueueSpacesAvailable(qSIM_SIM_UpdateHandle) == 10) 
	{
        if (xQueueReceive(qFlash_SIM900Handle, &sFlash_Update, 0) == pdTRUE) 
        {
            xQueueSend(qSIM_SIM_UpdateHandle,(void *)&sFlash_Update,100);
        }
        //
        if (xQueueReceive(qMeter_SIM900Handle, &sFlash_Update, 0) == pdTRUE) 
		{
            switch (sFlash_Update->Mess_Type_ui8) {
				case DATA_METER_INFOR:	// Request Meter Infor
					xQueueSend(qSIM_SIM_UpdateHandle,(void *)&sFlash_Update,100);
					break;
				case DATA_INTANTANIOUS:	// Intantanious
					xQueueSend(qSIM_SIM_UpdateHandle,(void *)&sFlash_Update,100);
					break;
                case DATA_EVEN_METER:	// 
					xQueueSend(qSIM_SIM_UpdateHandle,(void *)&sFlash_Update,100);
					break;
				default:
					break;
			} 
        }
    }
    //
    while (xQueueReceive(qSIM_SIM_UpdateHandle, &(sSend_Update), 0) == pdTRUE)
		Update_Table(sSend_Update);
}


void Update_Table(Meter_Flash_Queue_Struct* SendUpdate)
{
    sTCP.aNeed_Send[SendUpdate->Mess_Type_ui8] = 1;
    
    if (SendUpdate->Mess_Type_ui8 <= DATA_LOG_DCU)
        sTCP.paNeed_Send[SendUpdate->Mess_Type_ui8] = SendUpdate;
}


/*
 * return 1 : Neu toan bo bang o Muc uu tien Priority rong
 *        0 : Neu co request Send
 */
uint8_t Check_Sector_Table_Empty(void)
{
	int8_t  var = 0;
	for (var = 0; var < TOTAL_SLOT; ++var) 
    {
		if (sTCP.aNeed_Send[var] != 0) return 0;
	}
	return 1;
}


/*
 *  return 1 - TRUE : if data correct
 */
uint8_t Check_update_Time(ST_TIME_FORMAT *sRTC_temp)
{
	if (sRTC_temp->sec > 60) return 0;
	if (sRTC_temp->min > 60) return 0;
	if (sRTC_temp->hour > 23) return 0;
	if (sRTC_temp->day > 8)  return 0;
	if (sRTC_temp->date > 31) return 0;
	if (sRTC_temp->month > 12) return 0;
	if (sRTC_temp->year > 99) return 0;
	return 1;
}



                           
uint8_t _fConvertHexToStringDec (uint32_t Hex, uint8_t *Buff)
{
	uint8_t 	PhanDu = 0;
	uint32_t 	PhanNguyen = Hex;
	uint8_t 	aTemp[15];
	uint8_t 	Count = 0, i = 0;
	
	if(PhanNguyen == 0)
	{
		*(Buff) = 0x30;
		return 1;
	}
	
	while (PhanNguyen > 0)
	{
		PhanDu	   = PhanNguyen % 10;
		PhanNguyen = PhanNguyen / 10;
		aTemp[Count++] = PhanDu + 0x30;
	}
	//Dao nguoc lai
	for (i = 0; i< Count; i++)
		*(Buff + i) = aTemp[Count - 1 - i];
	
	return Count;
}

void _fPrintNumber (uint32_t Num)
{
    uint8_t aTemp[10];
    
    aTemp[0] = 0x0D;
    aTemp[1] = 0x0A;
    aTemp[2] = (Num / 10000) %10 + 0x30;
    aTemp[3] = (Num / 1000) %10 + 0x30;
    aTemp[4] = (Num / 100) %10 + 0x30;
    aTemp[5] = (Num / 10) %10 + 0x30;
    aTemp[6] = Num % 10 + 0x30;
    
    _fPrint_Via_Debug(&UART_SERIAL, &aTemp[0], 7, 1000);
}
uint32_t    LandMark_Test1 = 0;
uint32_t    Duty_2Packet = 0;
uint32_t    MinDuty = 0;
uint32_t    MaxDuty = 0;

void _mDATA_GET_FROM_QUEUE(int Kind_Send)     
{
    uint8_t          RetrySend = MAX_RETRY_SEND_TCP;
            
    if (Kind_Send == DATA_PRE_OPERA)
        LandMark_Test1 = RT_Count_Systick_u32;
    else if (Kind_Send == DATA_OPERATION)
    {
        Duty_2Packet = Cal_Time(LandMark_Test1, RT_Count_Systick_u32);
        if (Duty_2Packet < MinDuty)
            MinDuty = Duty_2Packet;
        if (Duty_2Packet > MaxDuty)
            MaxDuty = Duty_2Packet;
        //Print ra man hinh
        _fPrintNumber(MinDuty);
        _fPrintNumber(MaxDuty);
        LandMark_Test1 = RT_Count_Systick_u32;
    }

	sDCU.sPayload.Data_a8 = sTCP.paNeed_Send[Kind_Send]->str_Flash.Data_a8;
	sDCU.sPayload.Length_u16 = sTCP.paNeed_Send[Kind_Send]->str_Flash.Length_u16;
    
    if (sTCP.paNeed_Send[Kind_Send]->WaitACK == NONE_ACK)   //Neu khong doi ACK
       RetrySend = 0xFF;
    else if (sTCP.paNeed_Send[Kind_Send]->WaitACK == ONCE_ACK)   //Chi retry 1 lan
       RetrySend = 1;
    
    _f_TCP_SEND_SERVER(sDCU.sPayload.Data_a8, sDCU.sPayload.Length_u16, RetrySend, TIME_DELAY_TCP);
}

void _mDATA_LOG_DCU(int Kind_Send)           
{
    if(Check_Time_Out(sSim900_status.TimeoutSendHeartBeat_u32, 1000) == 0)
        osDelay(1000);
	sDCU.sPayload.Data_a8 = sTCP.paNeed_Send[Kind_Send]->str_Flash.Data_a8;
	sDCU.sPayload.Length_u16 = sTCP.paNeed_Send[Kind_Send]->str_Flash.Length_u16;
    _f_TCP_SEND_SERVER(sDCU.sPayload.Data_a8, sDCU.sPayload.Length_u16, 0xFF, TIME_DELAY_TCP);
}

void _mDATA_AT_RESPOND(int Kind_Send)           
{
    //Delay tu ban tin heartbeat xong
    if(Check_Time_Out(sSim900_status.TimeoutSendHeartBeat_u32, 1000) == 0)
       osDelay(1000);
    _f_TCP_SEND_SERVER(&aATBuffSendData[0],ATSendDataLength_u8, 0xFF, TIME_DELAY_TCP);
}

 

void _mDATA_NOTIF_CONNECT_MET(int Kind_Send)
{
    if(Check_Time_Out(sSim900_status.TimeoutSendHeartBeat_u32, 1000) == 0)
        osDelay(1000);
    //Dong goi ban tin ALarm Power On Moderm o day
	sDCU.sPayload.Data_a8 = &aBuff_Alarm_Power[0];
	sDCU.sPayload.Length_u16 = 0;
      
    Pack_AlarmPower(&sDCU.sPayload, &Str_OB_IDENT_METER, &sDCU.sMeter_id_now, sDCU.MeterType, sRTC, Str_Connect);
      
    _f_TCP_SEND_SERVER(sDCU.sPayload.Data_a8, sDCU.sPayload.Length_u16, 0xFF, TIME_DELAY_TCP);  //k cho ACK
}
     
void _mDATA_NOTIF_DISCONNECT_MET(int Kind_Send)
{
    if(Check_Time_Out(sSim900_status.TimeoutSendHeartBeat_u32, 1000) == 0)
        osDelay(1000);
    //Dong goi ban tin ALarm Power On Moderm o day
	sDCU.sPayload.Data_a8 = &aBuff_Alarm_Power[0];
	sDCU.sPayload.Length_u16 = 0;
      
    Pack_AlarmPower(&sDCU.sPayload, &Str_OB_IDENT_METER, &sDCU.sMeter_id_now, sDCU.MeterType, sRTC, Str_DisConnect);
      
    _f_TCP_SEND_SERVER(sDCU.sPayload.Data_a8, sDCU.sPayload.Length_u16, 0xFF, TIME_DELAY_TCP);  //k cho ACK
}
     

void _mDATA_ALARM_P_ON_MOD(int Kind_Send)
{
    if(Check_Time_Out(sSim900_status.TimeoutSendHeartBeat_u32, 1000) == 0)
        osDelay(1000);
    //Dong goi ban tin ALarm Power On Moderm o day
	sDCU.sPayload.Data_a8 = &aBuff_Alarm_Power[0];
	sDCU.sPayload.Length_u16 = 0;
      
    Pack_AlarmPower(&sDCU.sPayload, &Str_AlarmPowModerm, &sDCU.sMeter_id_now, sDCU.MeterType, sRTC, Str_PowOn_Moderm);
      
    _f_TCP_SEND_SERVER(sDCU.sPayload.Data_a8, sDCU.sPayload.Length_u16, 0xFF, TIME_DELAY_TCP);  //k cho ACK
}
     

void _mDATA_ALARM_P_OFF_MOD(int Kind_Send)
{
    if(Check_Time_Out(sSim900_status.TimeoutSendHeartBeat_u32, 1000) == 0)
        osDelay(1000);
	sDCU.sPayload.Data_a8 = &aBuff_Alarm_Power[0];
	sDCU.sPayload.Length_u16 = 0;
      
    Pack_AlarmPower(&sDCU.sPayload, &Str_AlarmPowModerm, &sDCU.sMeter_id_now, sDCU.MeterType, sRTC, Str_PowOFF_Moderm);
    
    _f_TCP_SEND_SERVER(sDCU.sPayload.Data_a8, sDCU.sPayload.Length_u16, 0xFF, TIME_DELAY_TCP);
    
}


void _mDATA_ALARM_P_UP_MET(int Kind_Send)
{
    if(Check_Time_Out(sSim900_status.TimeoutSendHeartBeat_u32, 1000) == 0)
        osDelay(1000);
	sDCU.sPayload.Data_a8 = &aBuff_Alarm_Power[0];
	sDCU.sPayload.Length_u16 = 0;
      
    Pack_AlarmPower(&sDCU.sPayload, &Str_AlarmPowLine, &sDCU.sMeter_id_now, sDCU.MeterType, sRTC, Str_PowUp_Line);
    
    _f_TCP_SEND_SERVER(sDCU.sPayload.Data_a8, sDCU.sPayload.Length_u16, 0xFF, TIME_DELAY_TCP);
}


void _mDATA_ALARM_P_DOW_MET(int Kind_Send)
{
    if(Check_Time_Out(sSim900_status.TimeoutSendHeartBeat_u32, 1000) == 0)
        osDelay(1000);
	sDCU.sPayload.Data_a8 = &aBuff_Alarm_Power[0];
	sDCU.sPayload.Length_u16 = 0;
      
    Pack_AlarmPower(&sDCU.sPayload, &Str_AlarmPowLine, &sDCU.sMeter_id_now, sDCU.MeterType, sRTC, Str_PowDown_Line);
    
    _f_TCP_SEND_SERVER(sDCU.sPayload.Data_a8, sDCU.sPayload.Length_u16, 0xFF, TIME_DELAY_TCP);
}



void _mUPDATE_FIRM_SUCCES(int Kind_Send)
{
    //copy Updating firmware.txt successfully 
    uint8_t Buff_Send[50];
    truct_String   StrRespond = {&Buff_Send[0], 0};
    truct_String   Str1 = {(uint8_t*) "Updating ", 9};
    truct_String   Str2 = {(uint8_t*) " successfully", 13};
    
    Copy_String_2(&StrRespond, &Str1);
    Copy_String_2(&StrRespond, &sInformation.Str_URL_Firm_Name);
    Copy_String_2(&StrRespond, &Str2);

    sDCU.sPayload.Data_a8 = &aBuff_Alarm_Power[0];
    sDCU.sPayload.Length_u16 = 0;
    sDCU.sPayload.Length_u16 = _fPacket_RespondAT(sDCU.sPayload.Data_a8, CheckList_AT[_QUERY_UP_FIRM_HTTP].sTempReceiver.Data_a8, 
                                                  CheckList_AT[_QUERY_UP_FIRM_HTTP].sTempReceiver.Length_u16, StrRespond.Data_a8, StrRespond.Length_u16); 
    
    HAL_UART_Transmit(&UART_SERIAL, sDCU.sPayload.Data_a8, sDCU.sPayload.Length_u16, 1000);
    
    _f_TCP_SEND_SERVER(sDCU.sPayload.Data_a8, sDCU.sPayload.Length_u16, 0xFF, TIME_DELAY_TCP);
    osDelay(2000);
    Reset_Chip ();
}

void _mUPDATE_FIRM_FAIL(int Kind_Send)
{
    //copy Updating firmware.txt successfully 
    uint8_t Buff_Send[50];
    truct_String   StrRespond = {&Buff_Send[0], 0};
    truct_String   Str1 = {(uint8_t*) "Updating ", 9};
    truct_String   Str2 = {(uint8_t*) " failed", 7};
    
    Copy_String_2(&StrRespond, &Str1);
    Copy_String_2(&StrRespond, &sInformation.Str_URL_Firm_Name);
    Copy_String_2(&StrRespond, &Str2);
    
    sDCU.sPayload.Data_a8 = &aBuff_Alarm_Power[0];
    sDCU.sPayload.Length_u16 = 0;
    sDCU.sPayload.Length_u16 = _fPacket_RespondAT(sDCU.sPayload.Data_a8, CheckList_AT[_QUERY_UP_FIRM_HTTP].sTempReceiver.Data_a8, 
                                                  CheckList_AT[_QUERY_UP_FIRM_HTTP].sTempReceiver.Length_u16, StrRespond.Data_a8, StrRespond.Length_u16); 
    
    HAL_UART_Transmit(&UART_SERIAL, sDCU.sPayload.Data_a8, sDCU.sPayload.Length_u16, 1000);
    _f_TCP_SEND_SERVER(sDCU.sPayload.Data_a8, sDCU.sPayload.Length_u16, 0xFF, TIME_DELAY_TCP);
    
    osDelay(2000);
    Reset_Chip ();
}

//Function kiem tra queue ve sim. Kiem tra xem có ban tin can gui di ? Neu co thi gui di va cho ACK
void _f_Send_Meter_Data (void)
{
    _fSent_MetData_Important();
    _fSent_MetData_Delay();
    _fSent_LPF_DELAY();
}


void _fSent_MetData_Important (void)
{
    uint8_t varC = 0;
    
    for (varC = DATA_EVEN_METER; varC < TOTAL_SLOT; ++varC)
    {
        if (sTCP.aNeed_Send[varC] != 0) 
        {
            if (sMark_MessageSend_Type[varC].CallBack != NULL) 
            {
                sTCP.SendOk_u8 = FALSE;
                sMark_MessageSend_Type[varC].CallBack(varC);
            }
            if (sTCP.SendOk_u8 == TRUE) 
            {
                if (varC == DATA_EVEN_METER)            // event sem
                    osSemaphoreRelease(bsMLProfMessSentHandle);
                
                sTCP.SendOk_u8 = FALSE;
                sTCP.aNeed_Send[varC] = 0;
                
                if (sTCP.paNeed_Send[varC]->Mess_Status_ui8 == 0) 
                  sTCP.paNeed_Send[varC]->Mess_Status_ui8 = 1; // Thong bao day len Server Thanh Cong FB cho Flash
                
                sTCP.paNeed_Send[varC] = NULL;
                
                sSim900_status.TimeoutSendHeartBeat_u32 = RT_Count_Systick_u32;
            }
        }
    }
}

void _fSent_MetData_Delay (void)
{
    uint8_t varC = 0;
    uint8_t	Check_Billing[8];  
    
    if (Cal_Time_s(sTCP.Landmark_Allow_Send_DATA_u32, RT_Count_Systick_u32) >= sTCP.Compare_Allow_Send_DATA_u16)
    {
        for (varC = DATA_PRE_OPERA; varC < DATA_EVEN_METER; ++varC)
        {
            if (varC == DATA_LOAD_PROFILE)   //bo qua lpf
                continue;
            
            if (sTCP.aNeed_Send[varC] != 0) 
            {
                if (sMark_MessageSend_Type[varC].CallBack != NULL) 
                {
                    sTCP.SendOk_u8 = FALSE;
                    sMark_MessageSend_Type[varC].CallBack(varC);
                }
                //
                if (sTCP.SendOk_u8 == TRUE) 
                {
                    //Ban tin chot
                    if (varC == DATA_HISTORICAL)           // Ban tin chot
                    {
                        //Danh dau co da gui vao Flash ngoai
                        Check_Billing[0] = 1; Check_Billing[1] = 0; Check_Billing[2] = 0; Check_Billing[3] = 0;
                        Check_Billing[4] = 0; Check_Billing[5] = 0; Check_Billing[6] = 0; Check_Billing[7] = 0;
                        Flash_S25FL_BufferWrite(Check_Billing,ADDR_SENT_BILLING_MARK+(sRTC.month-1)*S25FL_PAGE_SIZE,8);
                    }
                    //
                    sTCP.SendOk_u8 = FALSE;

                    if (sTCP.paNeed_Send[varC]->Mess_Status_ui8 == 0) 
                        sTCP.paNeed_Send[varC]->Mess_Status_ui8 = 1;        // Thong bao day len Server Thanh Cong FB cho Flash

                    sSim900_status.TimeoutSendHeartBeat_u32 = RT_Count_Systick_u32;
                } else   
                {
                    sTCP.aCountRetryMess[varC]++;     //trang count Retry len
                    //Neu fail trong truong hop data bi cat thi can clear needsend dê day lai queue
                    switch (varC)
                    {
                        case DATA_OPERATION:
                            Manage_Flash.Time_outFB_MQTT = Cal_Time(120000, RT_Count_Systick_u32);   //Tru di de het time out de flash dau lai queue
                            break;
                        case DATA_HISTORICAL: 
                            Manage_Flash_Bill.Time_outFB_MQTT = Cal_Time(120000, RT_Count_Systick_u32); 
                            break;
                        default:
                            break;
                    }
                }
                // Xoa danh dau aNeed_Send. 
                sTCP.aNeed_Send[varC] = 0;
                sTCP.paNeed_Send[varC] = NULL;
            }
        }
    }
}

//Tach ban tin loadprofile delay trong khoang 2 tieng. chu k phai 8 phut nhu cua TSVH



void _fSent_LPF_DELAY(void)
{    
    if (Cal_Time_s(sTCP.Landmark_Allow_Send_LPF_u32, RT_Count_Systick_u32) >= sTCP.Compare_Allow_Send_LPF_u16)
    {
        if (sTCP.aNeed_Send[DATA_LOAD_PROFILE] != 0) 
        {
            if (sMark_MessageSend_Type[DATA_LOAD_PROFILE].CallBack != NULL) 
            {
                sTCP.SendOk_u8 = FALSE;
                sMark_MessageSend_Type[DATA_LOAD_PROFILE].CallBack(DATA_LOAD_PROFILE);
            }
            //
            if (sTCP.SendOk_u8 == TRUE) 
            {
                sTCP.SendOk_u8 = FALSE;
  
                if (sTCP.paNeed_Send[DATA_LOAD_PROFILE]->Mess_Status_ui8 == 0) 
                    sTCP.paNeed_Send[DATA_LOAD_PROFILE]->Mess_Status_ui8 = 1;        // Thong bao day len Server Thanh Cong FB cho Flash
                
                sSim900_status.TimeoutSendHeartBeat_u32 = RT_Count_Systick_u32;
            }else
            {
                sTCP.aCountRetryMess[DATA_LOAD_PROFILE]++;
                Manage_Flash_Lpf.Time_outFB_MQTT = Cal_Time(120000, RT_Count_Systick_u32); 
            }

            sTCP.aNeed_Send[DATA_LOAD_PROFILE] = 0;
            sTCP.paNeed_Send[DATA_LOAD_PROFILE] = NULL;
        }
    }
}

