#include "variable.h"
#include "t_mqtt.h"

#include "pushdata.h"
#include "Init_All_Meter.h"

#include "LANDIS_Init_Meter.h"
#include "LANDIS_Read_Meter.h"
#include "at_commands.h"

#include "myDefine.h"

#define UNDER_COMP	0
#define OVER_COMP	1


uint32_t    LANDISStimelpfSecond = 0;
uint8_t     LAND_Fl_Alow_GetTimeLpf = 0;

void LANDIS_Init_Function (uint8_t type)
{
    eMeter_20._f_Read_ID            = LANDIS_Get_Meter_ID;
    eMeter_20._f_Check_Reset_Meter  = LANDIS_CheckResetReadMeter;
  
    eMeter_20._f_Connect_Meter      = LANDIS_Connect_meter_Landis;
    eMeter_20._f_Read_TSVH          = LANDIS_Read_TSVH;
    eMeter_20._f_Read_Bill          = LANDIS_Read_Bill;
    eMeter_20._f_Read_Event         = LANDIS_Read_Event;  
    eMeter_20._f_Read_Lpf           = LANDIS_Read_Lpf;
    eMeter_20._f_Read_InforMeter    = LANDIS_Read_Infor;
    eMeter_20._f_Get_UartData       = GetUART2Data;
    eMeter_20._f_Check_Meter        = LANDIS_Check_Meter;
    eMeter_20._f_Test1Cmd_Respond   = LANDIS_Send1Cmd_Test;
}

uint8_t LANDIS_Send1Cmd_Test (void)
{
    Init_UART2();
    if(Meter_Type == 1)
        if(Meter_Type == 1) RS485_SEND;
    osDelay(LANDIS_Meter_Delay_ms);
    HAL_UART_Transmit(&UART_METER,(uint8_t*)&LANDIS_SNRM[0],34,2000);
    if(Meter_Type == 1)
        if(Meter_Type == 1) RS485_RECIEVE;
    
    osSemaphoreWait(bsUART2PendingMessHandle,2000);
    
    if(UART1_Control.fRecei_Respond == 1) return 1;
    
    return 0;
}

uint8_t LANDIS_Read_TSVH (uint32_t Type)
{
    uint8_t         Temp_BBC = 0;
    truct_String    Str_Data_Write = {&Buff_Temp1[0], 0};
    
    Init_Meter_TuTi_Struct();
    if(LANDIS_Read(&Get_Meter_TuTi,&LANDIS_MTuTi_ExtractDataFunc,&Landis_TuTi_SendData,&LANDIS_Fill_MTuTi_Mess,LANDIS_MeterTuTiMessIDTable) != 1)
    {
        Init_Meter_TuTi_Struct();
        if(LANDIS_Read(&Get_Meter_TuTi,&LANDIS_MTuTi_ExtractDataFunc,&Landis_TuTi_SendData,&LANDIS_Fill_MTuTi_Mess,LANDIS_MeterTuTiMessIDTable) != 1)
        {
            Init_Meter_TuTi_Struct();
            if(LANDIS_Read(&Get_Meter_TuTi,&LANDIS_MTuTi_ExtractDataFunc,&Landis_TuTi_SendData,&LANDIS_Fill_MTuTi_Mess,LANDIS_MeterTuTiMessIDTable) != 1) 
              return 0;
        }
    }
    
    Init_Meter_Info_Struct();
    if(LANDIS_Read(&Get_Meter_Info,&LANDIS_MI_ExtractDataFunc,&LANDIS_MI_SendData,&LANDIS_Fill_MInfo_Mess,LANDIS_MeterInfoMessIDTable) != 1)
    {
        Init_Meter_Info_Struct();
        if(LANDIS_Read(&Get_Meter_Info,&LANDIS_MI_ExtractDataFunc,&LANDIS_MI_SendData,&LANDIS_Fill_MInfo_Mess,LANDIS_MeterInfoMessIDTable) != 1)
        {
            Init_Meter_Info_Struct();
            if(LANDIS_Read(&Get_Meter_Info,&LANDIS_MI_ExtractDataFunc,&LANDIS_MI_SendData,&LANDIS_Fill_MInfo_Mess,LANDIS_MeterInfoMessIDTable) != 1) return 0;
        }
    }
    
    Init_Meter_Event_Struct();
    Get_Meter_Event.Flag_Start_Pack = 1;  //khong cho dong goi header. chi dong goi data roi copy vao TSVH
    LANDIS_Read(&Get_Meter_Event,&LANDIS_ME_ExtractDataFunc,&LANDIS_ME_SendData_TSVH,&LANDIS_Fill_MEvent_Mess,LANDIS_MEvent_Code_Table);
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

uint8_t LANDIS_Read_Bill (void)
{
    uint16_t        TempYear=0;
    
    sDCU.FlagHave_BillMes = 0;   //
    
    Init_Meter_TuTi_Struct();
    if(LANDIS_Read(&Get_Meter_TuTi,&LANDIS_MTuTi_ExtractDataFunc,&Landis_TuTi_SendData,&LANDIS_Fill_MTuTi_Mess,LANDIS_MeterTuTiMessIDTable) != 1) 
        return 0;
    Init_Meter_Billing_Struct();
    //Read OBIS from LANDIS
    LANDIS_Insert_GetOBIS_Mess_OBIS(LANDIS_MBilling_Mess_OBIS);  // sn cua obis "0.0.98.1.0.126" - 64 00
    if (LANDIS_Read(&Get_Meter_Billing,&LANDIS_MBillingExtractOBISList,&OBIS_SendData,&Fill_frameOBIS_His,LANDIS_MHis_Code_Obis_Table) != 1) 
        return 0;
    //Read billing data
    LANDIS_MBillingInsertReadTime();
    LANDIS_Prepare_Read_Date(LANDIS_MBilling_StartTime,LANDIS_MBilling_StopTime); 
    LANDIS_Insert_GetDATA_Mess_OBIS(LANDIS_MBilling_Mess_OBIS);
    //Init OBIS list
    Get_Meter_Billing.OBIS_Pointer_ui8 = 0;
    Get_Meter_Billing.Num_OBIS_ui8 = 0;
    Get_Meter_Billing.Received_Record_ui16 = 0;
    Get_Meter_Billing.Num_Record_ui16 = 0;
    do{
        Get_Meter_Billing.Data_Buff_Pointer_ui16 = 10;
        Get_Meter_Billing.Flag_Start_Pack = 0;
        for (uint8_t i = 0; i < sizeof(LANDIS_MHis_Code_Data_Table); i++)
            LANDIS_MHis_Code_Data_Table[i] = i;
        LANDIS_MHis_Code_Data_Table[sizeof(LANDIS_MHis_Code_Data_Table)-1] = 0xFF;
        
        if (LANDIS_Read(&Get_Meter_Billing,&LANDIS_MBillingExtractData,&LANDIS_His_Send_Data,&Fill_frameData_His,LANDIS_MHis_Code_Data_Table) != 1) break;                   
        Get_Meter_Billing.Received_Record_ui16++; 
        
        if(Get_Meter_Billing.Num_Record_ui16>0) Get_Meter_Billing.Num_Record_ui16--;
        if(sInformation.Flag_Request_Bill == 1)   
        {
            if(Check_sTimeBill_withStarttime(sInformation.StartTime_GetBill, &sInformation.EndTime_GetBill, &StrSTime_Bill) == 0 ) break;
            LANDIS_MBillingInsertReadTime();
            LANDIS_Prepare_Read_Date(LANDIS_MBilling_StartTime,LANDIS_MBilling_StopTime);
            LANDIS_Insert_GetDATA_Mess_OBIS(LANDIS_MBilling_Mess_OBIS);
        }else       
        {
            if(Check_sTimeBill_withStarttime(sInformation.StartTime_GetBill, &sInformation.EndTime_GetBill, &StrSTime_Bill) == 0 ) break;
            TempYear = 2000 + sRTC.year;
            LANDIS_MBilling_StopTime[0]  = (uint8_t)((TempYear>>8)&0x00FF);
            LANDIS_MBilling_StopTime[1]  = (uint8_t)(TempYear&0x00FF);
            LANDIS_MBilling_StopTime[2]  = sInformation.EndTime_GetBill.month;		
            LANDIS_MBilling_StopTime[3] = sInformation.EndTime_GetBill.date;
            LANDIS_Prepare_Read_Date(LANDIS_MBilling_StartTime,LANDIS_MBilling_StopTime);
            LANDIS_Insert_GetDATA_Mess_OBIS(LANDIS_MBilling_Mess_OBIS);
        }
    }while((Get_Meter_Billing.Num_Record_ui16>0) && (sInformation.Flag_Stop_ReadBill == 0));
    //
    if(sDCU.FlagHave_BillMes == 0)
    {
        Packet_Empty_MessHistorical();
    }
    sInformation.Flag_Request_Bill = 0;  //reset request
    sInformation.Flag_Stop_ReadBill = 0;
    return 1;
}

uint8_t LANDIS_Read_Event (uint32_t TemValue)
{
    Init_Meter_TuTi_Struct();
    if(LANDIS_Read(&Get_Meter_TuTi,&LANDIS_MTuTi_ExtractDataFunc,&Landis_TuTi_SendData,&LANDIS_Fill_MTuTi_Mess,LANDIS_MeterTuTiMessIDTable) != 1) 
        return 0;
    
    Init_Meter_Event_Struct();
    return LANDIS_Read(&Get_Meter_Event,&LANDIS_ME_ExtractDataFunc,&LANDIS_ME_SendData,&LANDIS_Fill_MEvent_Mess,LANDIS_MEvent_Code_Table);
}

uint8_t LANDIS_Read_Lpf (void)
{
    uint8_t         DiffDate = 0;
    
    PeriodLpf_Min = 30;   //gia tri ban dau la30. sau do se lay thoi gian truoc tru thoi gian sau.
    LANDISStimelpfSecond = 0;
    sDCU.FlagHave_ProfMess = 0; 
     
    Init_Meter_LProf_Struct();   
    Init_Meter_TuTi_Struct();
    if(LANDIS_Read(&Get_Meter_TuTi,&LANDIS_MTuTi_ExtractDataFunc,&Landis_TuTi_SendData,&LANDIS_Fill_MTuTi_Mess,LANDIS_MeterTuTiMessIDTable) != 1) 
      return 0;
    MLoadpfInsertReadTime();
    /*--------------------------------------------------------------*/
    //Read OBIS from LANDIS
    LANDIS_Insert_GetOBIS_Mess_OBIS(LANDIS_MLprofile_OBIS_List);
    LANDIS_Read(&Get_Meter_LProf,&LANDIS_MLProfileExtractOBISList,&OBIS_SendData,&Fill_frameOBIS_LoPro,LANDIS_MLprofile_Code_Obis_Table);
    osDelay(5);
    // Init variable
    LANDIS_Prepare_Read_Date(LANDIS_MLProfile_StartTime,LANDIS_MLProfile_StopTime);
    LANDIS_Insert_GetDATA_Mess_OBIS(LANDIS_MLprofile_OBIS_List);
    //Read Data
    Get_Meter_LProf.Num_OBIS_ui8 = 0;
    Get_Meter_LProf.OBIS_Pointer_ui8 = 0;
    Get_Meter_LProf.Num_Record_ui16 = 0;
    do{
        Get_Meter_LProf.Flag_Start_Pack = 0;
        Get_Meter_LProf.Data_Buff_Pointer_ui16 = 10;
        for (uint8_t i = 0; i < sizeof(LANDIS_MLprofile_Code_Data_Table); i++)
            LANDIS_MLprofile_Code_Data_Table[i] = i;
        LANDIS_MLprofile_Code_Data_Table[sizeof(LANDIS_MLprofile_Code_Data_Table)-2] = 0xFF;
        
        if(LANDIS_Read(&Get_Meter_LProf,&LANDIS_MLProfileExtractData,&LoPro_Send_Data,&Fill_frameData_Load,LANDIS_MLprofile_Code_Data_Table) != 1) break;
        
        // Init variable
        if(sInformation.Flag_Request_lpf == 1)
        {
            //kiem tra
            if(Check_2_sTime_By_date(sInformation.StartTime_GetLpf,sInformation.EndTime_GetLpf, &DiffDate) == 1)
            {
                MLoadpfInsertReadTime();
                LANDIS_Prepare_Read_Date(LANDIS_MLProfile_StartTime,LANDIS_MLProfile_StopTime);
                LANDIS_Insert_GetDATA_Mess_OBIS(LANDIS_MLprofile_OBIS_List);
            }else 
              break;
        }else 
          break;  
    }while (sInformation.Flag_Stop_ReadLpf == 0);
    
    if(sDCU.FlagHave_ProfMess == 0)
    {
        _fSend_Empty_Lpf();
    }
    
    sInformation.Flag_Request_lpf = 0; 
    sInformation.Flag_Stop_ReadLpf = 0;
    return 1;
}

uint8_t LANDIS_Read_Infor (void)
{
    Init_Meter_TuTi_Struct();
    if(LANDIS_Read(&Get_Meter_TuTi,&LANDIS_MTuTi_ExtractDataFunc,&Landis_TuTi_SendData,&LANDIS_Fill_MTuTi_Mess,LANDIS_MeterTuTiMessIDTable) == 1)
    {
        Pack_PushData_103_Infor_Meter();
        return 1;
    }
    return 0;
}



uint8_t LANDIS_Connect_meter_Landis(void) 
{
    uint8_t NumRetryReadMeter = 0;
    Meter_Type = 1;
    UART_METER.Init.BaudRate = UART2_BAUR_2;
    
	while(NumRetryReadMeter<4)
	{
        osDelay(500);
        RS485_SEND;    //Dua ve che do 232. De moi lan chuyen sang check uart 485 thi se check 232 (disable chan RXDE)
		if (osMutexWait(mtFlashMeterHandle,1000) == osOK)
		{
			if (LANDIS_Read(&Get_Meter_ShortName,&LANDIS_Get_ShortName,&OBIS_SendData,&Fill_Nextframe,LANDIS_GetSN_Code_Table) == 1)
            {
                LANDIS_Read(&Get_Meter_Scale,&LANDIS_MScale_ExtractDataFunc,&LANDIS_MS_SendData,&LANDIS_Fill_Scale_Mess,LANDIS_GetScale_Code_Table);
                Read_Meter_ID_Success = LANDIS_Get_Meter_ID(0);
                if(Read_Meter_ID_Success == 1)
                {
                    if (osMutexRelease(mtFlashMeterHandle) != osOK)
                        osMutexRelease(mtFlashMeterHandle);
                    return 1;
                }
            }
			NumRetryReadMeter++;	
			Meter_Type--;  // RS232 -> RS485
			switch(NumRetryReadMeter)
			{
				case 2:
					Meter_Type = 1;
					UART_METER.Init.BaudRate = UART2_BAUR_1;
					break;
				default:
					break;
			}
            
			//Reinit baudrate uart2
			UART_METER.Instance = UART__METER;
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
			
			if (osMutexRelease(mtFlashMeterHandle) != osOK)
                osMutexRelease(mtFlashMeterHandle);
		}
	}
    if(Read_Meter_ID_Success == 1) 
      return 1;
    return 0;
}

/*
 * 			FUNCTION
 */


void LANDIS_Cal_Header_CheckSum (uint8_t *MessTemp,uint8_t Start_Pos,uint8_t End_Pos)
{

	uint16_t Temp_CheckSum=0;
	*(MessTemp+5) += 0x20;
	Temp_CheckSum = CountFCS16(MessTemp,Start_Pos,End_Pos);
	*(MessTemp+6) = (uint8_t)((Temp_CheckSum>>8)&0x00FF);
	*(MessTemp+7) = (uint8_t)(Temp_CheckSum&0x00FF);
}
void LANDIS_Cal_Full_CheckSum (uint8_t *MessTemp,uint8_t Start_Pos,uint8_t End_Pos)
{
	uint16_t Temp_CheckSum=0;
	
	Temp_CheckSum = CountFCS16(MessTemp,Start_Pos,End_Pos);
	*(MessTemp+End_Pos+1) = (uint8_t)((Temp_CheckSum>>8)&0x00FF);
	*(MessTemp+End_Pos+2) = (uint8_t)(Temp_CheckSum&0x00FF);	
}

void LANDIS_Insert_GetOBIS_Mess_OBIS (uint8_t *Mess_OBIS)
{
	uint16_t TempSN = 0;
	uint16_t TempCheckSum = 0;
	// ID Frame
	LANDIS_MInfo_MTemp[5] = 0x32;
	//Calculate Header checksum
	TempCheckSum = CountFCS16(&LANDIS_MInfo_MTemp[0],1,5);
	//Insert Header checksum
	LANDIS_MInfo_MTemp[6] = (uint8_t)((TempCheckSum>>8)&0x00FF);
	LANDIS_MInfo_MTemp[7] = (uint8_t)(TempCheckSum&0x00FF);
	LANDIS_MInfo_MTemp[14] = *(Mess_OBIS);
	LANDIS_MInfo_MTemp[15] = *(Mess_OBIS+1);
	TempSN = (TempSN + LANDIS_MInfo_MTemp[14])<<8;
	TempSN = TempSN +  LANDIS_MInfo_MTemp[15] + 0x10;  // +0x10 - Get Obis
	LANDIS_MInfo_MTemp[14] = (uint8_t)(TempSN>>8);
	LANDIS_MInfo_MTemp[15] = (uint8_t)TempSN;
	LANDIS_Cal_Full_CheckSum(LANDIS_MInfo_MTemp,1,15);
}

void LANDIS_Insert_GetDATA_Mess_OBIS (uint8_t *Mess_OBIS)
{
	uint16_t TempSN = 0;
	LANDIS_Get_DATA_Template[14] = *(Mess_OBIS);
	LANDIS_Get_DATA_Template[15] = *(Mess_OBIS+1);
	TempSN = (TempSN + LANDIS_Get_DATA_Template[14])<<8;
	TempSN = TempSN +  LANDIS_Get_DATA_Template[15] + 0x08;  // +0x08 - Get Value
	LANDIS_Get_DATA_Template[14] = (uint8_t)(TempSN>>8);
	LANDIS_Get_DATA_Template[15] = (uint8_t)TempSN;
	LANDIS_Cal_Full_CheckSum(LANDIS_Get_DATA_Template,1,66);
}

void LANDIS_Mess_Checksum(void)
{
	uint16_t	GenCheckSum=0,MesCheckSum=0;
	
	GenCheckSum = CountFCS16(UART1_Receive_Buff,1,UART1_Control.Mess_Length_ui16-3);
	MesCheckSum = UART1_Receive_Buff[UART1_Control.Mess_Length_ui16-2];
	MesCheckSum = (MesCheckSum << 8) + UART1_Receive_Buff[UART1_Control.Mess_Length_ui16-1];
	
	if (GenCheckSum == MesCheckSum)
		UART1_Control.Mess_Status_ui8 = 2;
	else
		UART1_Control.Mess_Status_ui8 = 4;
}
                                                                                                           
	                                                                                                             
uint8_t LANDIS_Read_Info_Scale (Meter_Comm_Struct *Meter_Comm,void (*FuncExtractRawData)(void),void (*FuncDataHandle)(void),void (*FuncFillMess)(void),uint8_t *CodeTable)
{
	uint8_t mReVal = 0;
	// Init
	Meter_Comm->Step_ui8 = 1;
	Meter_Comm->Mess_Step_ui8 = 0;
	Meter_Comm->Reading_ui8 = 1;
	//Start read
	Meter_Comm->ID_Frame_ui8 = 0x32;  // Init Frame ID - moi lan doc cong to deu nhay vao ham nay
	if(Meter_Type == 1) RS485_SEND;
	while(Meter_Comm->Reading_ui8 == 1) {
		switch(Meter_Comm->Step_ui8) {  // int Step_ui8 = 1
			case 0:  // Nhay vao check data
				Meter_Comm->Total_Mess_Sent_ui32++;
				//Wait for respond message from Meter in 2000ms: checksum - extract raw data - save to buffer - goto next message
				if (osSemaphoreWait(bsUART2PendingMessHandle,5000) == osOK) {
					//Checksum
					LANDIS_Mess_Checksum();
					//0x00-no mess			0x01-not check		0x02-checked-ok			0x04-check-fail
					if (UART1_Control.Mess_Status_ui8 == 2) {  // ok
						Meter_Comm->Success_Read_Mess_ui32++;
						//Extract raw data
						if (Meter_Comm->Mess_Step_ui8 > 1) // xu li data
							FuncExtractRawData();  // ham xu li data nhan dc
						//Goto next message
						Meter_Comm->Mess_Step_ui8++; // tang de doc thong so tiep
						Meter_Comm->Error_ui8 = 0x00;
						Meter_Comm->Step_ui8 = 1;  // di toi gui lenh tiep theo
						//If all mess sent -> finish
						if (Meter_Comm->Mess_Step_ui8 > 1){
							if ((*(CodeTable+Meter_Comm->Mess_Step_ui8-2)) == 0xFF) {
								//Push mess or check alarm
								FuncDataHandle(); // xu li du lieu
								mReVal = 1;       // Tra ve ket qua doc thanh cong cong to
								Meter_Comm->Step_ui8 = 2;
							}
						}
                        //Reset UART2 status
						Init_UART2();
					} else {
						Meter_Comm->Error_Wrong_Mess_Format_ui32++;
						//Reset UART
						Init_UART2();
						//If Meter respond wrong, jump out of reading cycle
						Meter_Comm->Step_ui8 = 2;
					}
				} else {//If Meter no respond after 2000ms: wait 2000ms - resend the message (3 times) 
					Meter_Comm->Error_Meter_Norespond_ui32++;
					//Reinit uart2
                    HAL_UART_Init(&UART_METER);
					Init_UART2();
					__HAL_UART_ENABLE_IT(&UART_METER, UART_IT_RXNE);
					osDelay(1000);
					Meter_Comm->Step_ui8 = 2;
				}
				break;
			case 1:
				if (Meter_Comm->Mess_Step_ui8 == 0) {  // Step 0 - ban tin SNRM
					if(Meter_Type == 1) RS485_SEND;
					HAL_UART_Transmit(&UART_METER,(uint8_t*)&LANDIS_SNRM[0],34,1000);
					if(Meter_Type == 1) RS485_RECIEVE;
				} else if (Meter_Comm->Mess_Step_ui8 == 1) {  // Step 1 - ban tin AARQ
					if(Meter_Type == 1) RS485_SEND;
					osDelay(3);
					HAL_UART_Transmit(&UART_METER,(uint8_t*)&LANDIS_AARQ[0],48,1000);
					if(Meter_Type == 1) RS485_RECIEVE;
				} else {  				// Ban tin doc cac thong so
					if(Meter_Type == 1) RS485_SEND;
					osDelay(3);					
					FuncFillMess();  	// dien day command de gui xuong cong to
					HAL_UART_Transmit(&UART_METER,&LANDIS_MInfo_MTemp[0],19,1000);
					if(Meter_Type == 1) RS485_RECIEVE;
				}
				Meter_Comm->Step_ui8 = 0; // Quay ve buoc 0 de check data
				break;
			case 2:
				//Reset UART2 status
				Init_UART2();
				Meter_Comm->Reading_ui8 = 0;
				//Reset pointer
				Meter_Comm->Data_Buff_Pointer_ui16 = 10;
				break;	
			default:
				break;				
		}
	}
	return mReVal;
}

void LANDIS_Fill_MInfo_Mess (void)
{
	uint8_t 	j=0;
	uint16_t	TempCheckSum = 0;
	uint16_t 	TempSN = 0;
	//Insert ID Frame
	if(Get_Meter_Info.Mess_Step_ui8==0)
		Get_Meter_Info.ID_Frame_ui8 = 0x32;
	LANDIS_MInfo_MTemp[5] = Get_Meter_Info.ID_Frame_ui8;
	//Calculate Header checksum
	TempCheckSum = CountFCS16(&LANDIS_MInfo_MTemp[0],1,5);
	//Insert Header checksum
	LANDIS_MInfo_MTemp[6] = (uint8_t)((TempCheckSum>>8)&0x00FF);
	LANDIS_MInfo_MTemp[7] = (uint8_t)(TempCheckSum&0x00FF);
	//Insert SN
	do
	{
        j = LANDIS_MeterInfoMessIDTable[Get_Meter_Info.Mess_Step_ui8];
        LANDIS_MInfo_MTemp[14] = LANDIS_Info_SN_Table[j][1];
        LANDIS_MInfo_MTemp[15] = LANDIS_Info_SN_Table[j][2];
        TempSN = LANDIS_MInfo_MTemp[14] + LANDIS_MInfo_MTemp[15];
        Get_Meter_Info.Mess_Step_ui8++;
	}
	while(TempSN == 0x00);
    
	Get_Meter_Info.Mess_Step_ui8--;
	TempSN  = LANDIS_MInfo_MTemp[14]<<8;
	TempSN  = TempSN + LANDIS_MInfo_MTemp[15];
	if(( j > 30)&&(j < 36))
	TempSN = TempSN + 0x20;  // +0x20 - Get Time Maxdemand
	else
	TempSN = TempSN + 0x08;  // +0x08 - Get Value
	LANDIS_MInfo_MTemp[14] = (uint8_t)(TempSN>>8);
	LANDIS_MInfo_MTemp[15] = (uint8_t)TempSN;
	//Calculate Total checksum
	TempCheckSum = CountFCS16(&LANDIS_MInfo_MTemp[0],1,15);
	//Insert Total checksum
	LANDIS_MInfo_MTemp[16] = (uint8_t)((TempCheckSum>>8)&0x00FF);
	LANDIS_MInfo_MTemp[17] = (uint8_t)(TempCheckSum&0x00FF);
	//Calculate next ID Frame
	Get_Meter_Info.ID_Frame_ui8 += 0x22;
	if (Get_Meter_Info.ID_Frame_ui8 == 0x20)
		Get_Meter_Info.ID_Frame_ui8 = 0x10;
	// Send
	if(Meter_Type == 1) RS485_SEND;
	osDelay(LANDIS_Meter_Delay_ms);
	HAL_UART_Transmit(&UART_METER,&LANDIS_MInfo_MTemp[0],19,1000);
	if(Meter_Type == 1) RS485_RECIEVE;
	
}
    
    
void LANDIS_Fill_MTuTi_Mess (void)
{
	uint8_t 	j=0;
	uint16_t	TempCheckSum = 0;
	uint16_t 	TempSN = 0;
	//Insert ID Frame
	if(Get_Meter_TuTi.Mess_Step_ui8==0)
		Get_Meter_TuTi.ID_Frame_ui8 = 0x32;
	LANDIS_MInfo_MTemp[5] = Get_Meter_TuTi.ID_Frame_ui8;
	//Calculate Header checksum
	TempCheckSum = CountFCS16(&LANDIS_MInfo_MTemp[0],1,5);
	//Insert Header checksum
	LANDIS_MInfo_MTemp[6] = (uint8_t)((TempCheckSum>>8)&0x00FF);
	LANDIS_MInfo_MTemp[7] = (uint8_t)(TempCheckSum&0x00FF);
	//Insert SN
	do
	{
        j = LANDIS_MeterTuTiMessIDTable[Get_Meter_TuTi.Mess_Step_ui8];
        LANDIS_MInfo_MTemp[14] = LANDIS_Info_SN_Table[j][1];
        LANDIS_MInfo_MTemp[15] = LANDIS_Info_SN_Table[j][2];
        TempSN = LANDIS_MInfo_MTemp[14] + LANDIS_MInfo_MTemp[15];
        Get_Meter_TuTi.Mess_Step_ui8++;
	}
	while(TempSN == 0x00);
    
	Get_Meter_TuTi.Mess_Step_ui8--;
	TempSN  = LANDIS_MInfo_MTemp[14]<<8;
	TempSN  = TempSN + LANDIS_MInfo_MTemp[15];
	if(( j > 30)&&(j < 36))
	TempSN = TempSN + 0x20;  // +0x20 - Get Time Maxdemand
	else
	TempSN = TempSN + 0x08;  // +0x08 - Get Value
	LANDIS_MInfo_MTemp[14] = (uint8_t)(TempSN>>8);
	LANDIS_MInfo_MTemp[15] = (uint8_t)TempSN;
	//Calculate Total checksum
	TempCheckSum = CountFCS16(&LANDIS_MInfo_MTemp[0],1,15);
	//Insert Total checksum
	LANDIS_MInfo_MTemp[16] = (uint8_t)((TempCheckSum>>8)&0x00FF);
	LANDIS_MInfo_MTemp[17] = (uint8_t)(TempCheckSum&0x00FF);
	//Calculate next ID Frame
	Get_Meter_TuTi.ID_Frame_ui8 += 0x22;
	if (Get_Meter_TuTi.ID_Frame_ui8 == 0x20)
		Get_Meter_TuTi.ID_Frame_ui8 = 0x10;
	// Send
	if(Meter_Type == 1) RS485_SEND;
	osDelay(LANDIS_Meter_Delay_ms);
	HAL_UART_Transmit(&UART_METER,&LANDIS_MInfo_MTemp[0],19,1000);
	if(Meter_Type == 1) RS485_RECIEVE;
	
}
void LANDIS_Fill_MEvent_Mess (void)
{
	uint8_t 	j=0;
	uint16_t	TempCheckSum = 0;
	uint16_t 	TempSN = 0;
	//Insert ID Frame
	if(Get_Meter_Event.Mess_Step_ui8==0)
		Get_Meter_Event.ID_Frame_ui8 = 0x32;
	LANDIS_MInfo_MTemp[5] = Get_Meter_Event.ID_Frame_ui8;
	//Calculate Header checksum
	TempCheckSum = CountFCS16(&LANDIS_MInfo_MTemp[0],1,5);
	//Insert Header checksum
	LANDIS_MInfo_MTemp[6] = (uint8_t)((TempCheckSum>>8)&0x00FF);
	LANDIS_MInfo_MTemp[7] = (uint8_t)(TempCheckSum&0x00FF);
	//Insert Class ID + OBIS + Data Type
	j = LANDIS_MEvent_Code_Table[Get_Meter_Event.Mess_Step_ui8];
	LANDIS_MInfo_MTemp[14] = LANDIS_Event_SN_Table[j][1];
	LANDIS_MInfo_MTemp[15] = LANDIS_Event_SN_Table[j][2];
	TempSN = LANDIS_MInfo_MTemp[14]<<8;
	TempSN = TempSN +  LANDIS_MInfo_MTemp[15];
	TempSN = TempSN + 0x08;  // +0x08 - Get Value
	LANDIS_MInfo_MTemp[14] = (uint8_t)(TempSN>>8);
	LANDIS_MInfo_MTemp[15] = (uint8_t)TempSN;
	//Calculate Total checksum
	TempCheckSum = CountFCS16(&LANDIS_MInfo_MTemp[0],1,15);
	//Insert Total checksum
	LANDIS_MInfo_MTemp[16] = (uint8_t)((TempCheckSum>>8)&0x00FF);
	LANDIS_MInfo_MTemp[17] = (uint8_t)(TempCheckSum&0x00FF);
	
	//Calculate next ID Frame
	Get_Meter_Event.ID_Frame_ui8 += 0x22;
	if (Get_Meter_Event.ID_Frame_ui8 == 0x20)
		Get_Meter_Event.ID_Frame_ui8 = 0x10;
	// Send
	if(Meter_Type == 1) RS485_SEND;
	osDelay(LANDIS_Meter_Delay_ms);
	HAL_UART_Transmit(&UART_METER,&LANDIS_MInfo_MTemp[0],19,1000);
	if(Meter_Type == 1) RS485_RECIEVE;
}


void LANDIS_MI_ExtractDataFunc(void)  // cach cat byte cua Meter Infor
{
	uint8_t Data_type=0,i=0,j=0;
	
	Data_type = UART1_Receive_Buff[14];
	j = LANDIS_MeterInfoMessIDTable[Get_Meter_Info.Mess_Step_ui8];  // tim lenh dang doc
	switch (Data_type)
	{
		case 0x05:
			Meter_TempBuff[Get_Meter_Info.Data_Buff_Pointer_ui16++] = LANDIS_Info_SN_Table[j][0];  // gan obis tuong ung
			Meter_TempBuff[Get_Meter_Info.Data_Buff_Pointer_ui16++] = 0x04; // Length
			for (i=0;i<4;i++)
				Meter_TempBuff[Get_Meter_Info.Data_Buff_Pointer_ui16++] = UART1_Receive_Buff[15+i];
			break;
		case 0x09:
			Meter_TempBuff[Get_Meter_Info.Data_Buff_Pointer_ui16++] = LANDIS_Info_SN_Table[j][0];
			Meter_TempBuff[Get_Meter_Info.Data_Buff_Pointer_ui16++] = UART1_Receive_Buff[15];
			for (i=0;i<UART1_Receive_Buff[15];i++)
				Meter_TempBuff[Get_Meter_Info.Data_Buff_Pointer_ui16++] = UART1_Receive_Buff[16+i];			
			break;
		case 0x12:
			Meter_TempBuff[Get_Meter_Info.Data_Buff_Pointer_ui16++] = LANDIS_Info_SN_Table[j][0];
			Meter_TempBuff[Get_Meter_Info.Data_Buff_Pointer_ui16++] = 0x02;
			for (i=0;i<2;i++)
				Meter_TempBuff[Get_Meter_Info.Data_Buff_Pointer_ui16++] = UART1_Receive_Buff[15+i];			
			break;
		case 0x14:
			Meter_TempBuff[Get_Meter_Info.Data_Buff_Pointer_ui16++] = LANDIS_Info_SN_Table[j][0];
			Meter_TempBuff[Get_Meter_Info.Data_Buff_Pointer_ui16++] = 0x08;
			for (i=0;i<8;i++)
				Meter_TempBuff[Get_Meter_Info.Data_Buff_Pointer_ui16++] = UART1_Receive_Buff[15+i];			
			break;
		default:
			break;
	}
}



void LANDIS_MTuTi_ExtractDataFunc(void)  // cach cat byte cua Meter Infor
{
	uint8_t Data_type=0,i=0,j=0;
	
	Data_type = UART1_Receive_Buff[14];
	j = LANDIS_MeterTuTiMessIDTable[Get_Meter_TuTi.Mess_Step_ui8];  // tim lenh dang doc
	switch (Data_type)
	{
		case 0x05:
			Meter_TempBuff[Get_Meter_TuTi.Data_Buff_Pointer_ui16++] = LANDIS_Info_SN_Table[j][0];  // gan obis tuong ung
			Meter_TempBuff[Get_Meter_TuTi.Data_Buff_Pointer_ui16++] = 0x04; // Length
			for (i=0;i<4;i++)
				Meter_TempBuff[Get_Meter_TuTi.Data_Buff_Pointer_ui16++] = UART1_Receive_Buff[15+i];
			break;
		case 0x09:
			Meter_TempBuff[Get_Meter_TuTi.Data_Buff_Pointer_ui16++] = LANDIS_Info_SN_Table[j][0];
			Meter_TempBuff[Get_Meter_TuTi.Data_Buff_Pointer_ui16++] = UART1_Receive_Buff[15];
			for (i=0;i<UART1_Receive_Buff[15];i++)
				Meter_TempBuff[Get_Meter_TuTi.Data_Buff_Pointer_ui16++] = UART1_Receive_Buff[16+i];			
			break;
		case 0x12:
			Meter_TempBuff[Get_Meter_TuTi.Data_Buff_Pointer_ui16++] = LANDIS_Info_SN_Table[j][0];
			Meter_TempBuff[Get_Meter_TuTi.Data_Buff_Pointer_ui16++] = 0x02;
			for (i=0;i<2;i++)
				Meter_TempBuff[Get_Meter_TuTi.Data_Buff_Pointer_ui16++] = UART1_Receive_Buff[15+i];			
			break;
		case 0x14:
			Meter_TempBuff[Get_Meter_TuTi.Data_Buff_Pointer_ui16++] = LANDIS_Info_SN_Table[j][0];
			Meter_TempBuff[Get_Meter_TuTi.Data_Buff_Pointer_ui16++] = 0x08;
			for (i=0;i<8;i++)
				Meter_TempBuff[Get_Meter_TuTi.Data_Buff_Pointer_ui16++] = UART1_Receive_Buff[15+i];			
			break;
		default:
			break;
	}
}


void LANDIS_ME_ExtractDataFunc(void)  // cach cat byte cua Meter Event
{
	uint8_t Data_type=0,i=0,j=0;
    uint8_t ObisHex = 0;
    uint8_t length = 0;
	uint32_t Temp_u32 = 0;
    int8_t Row = 0;
    uint8_t Buff[20];
    ST_TIME_FORMAT  sTime_temp;
    uint16_t        Tempyear = 0;
    
	Data_type = UART1_Receive_Buff[14];
	j = LANDIS_MEvent_Code_Table[Get_Meter_Event.Mess_Step_ui8];  // tim lenh dang doc
    
    if(Get_Meter_Event.Flag_Start_Pack == 0)
    {
        Reset_Buff(&Get_Meter_Event.Str_Payload);
        Header_event_103(&Get_Meter_Event.Str_Payload, 1);
        Get_Meter_Event.Flag_Start_Pack = 1;
    }
    
	switch (Data_type)
	{
		case 0x06:
			ObisHex= LANDIS_Event_SN_Table[j][0];  // gan obis tuong ung
			length = 0x04; // Length
			for (i=0;i<4;i++)
				Temp_u32 = Temp_u32 << 8 | UART1_Receive_Buff[15+i];
			break;
		case 0x09:
			ObisHex = LANDIS_Event_SN_Table[j][0];
			length = UART1_Receive_Buff[15];
			for (i=0;i<length;i++)
				Buff[i] = UART1_Receive_Buff[16+i];			
			break;
		default:
			break;
	}
    
     Row = Landis_Check_Obis_event(ObisHex);
    if(Row >= 0)
    {
        Copy_String_toTaget(&Get_Meter_Event.Str_Payload, Get_Meter_Event.Pos_Obis_Inbuff, Obis_landis_Event[Row].Str_Obis);
        Get_Meter_Event.Pos_Obis_Inbuff += Obis_landis_Event[Row].Str_Obis->Length_u16;
        Get_Meter_Event.Pos_Data_Inbuff = Get_Meter_Event.Str_Payload.Length_u16;
        Get_Meter_Event.Numqty++;
        //ghi data vao
        switch(length)
        {
            case 4:
                *(Get_Meter_Event.Str_Payload.Data_a8 + Get_Meter_Event.Str_Payload.Length_u16++) = '('; 
                Pack_HEXData_Frame_Uint64(&Get_Meter_Event.Str_Payload, (uint64_t) Temp_u32, 0);
                *(Get_Meter_Event.Str_Payload.Data_a8 + Get_Meter_Event.Str_Payload.Length_u16++) = ')'; 
                Get_Meter_Event.Pos_Data_Inbuff = Get_Meter_Event.Str_Payload.Length_u16;
                break;
            case 12:
                Tempyear         = Buff[0] << 8 | Buff[1];
                sTime_temp.year  = Tempyear%100;
                sTime_temp.month = Buff[2];
                sTime_temp.date  = Buff[3];
                sTime_temp.day   = Buff[4];
                sTime_temp.hour  = Buff[5];
                sTime_temp.min   = Buff[6];
                sTime_temp.sec   = Buff[7];
                *(Get_Meter_Event.Str_Payload.Data_a8 + Get_Meter_Event.Str_Payload.Length_u16++) = '('; 
                Copy_String_STime(&Get_Meter_Event.Str_Payload,sTime_temp);
                *(Get_Meter_Event.Str_Payload.Data_a8 + Get_Meter_Event.Str_Payload.Length_u16++) = ')'; 
                Get_Meter_Event.Pos_Data_Inbuff = Get_Meter_Event.Str_Payload.Length_u16;
                break;
            default: 
                break;
        }
    }
    
            
}


void LANDIS_MI_SendData(void) 
{
    Convert_Opera_ToPack103_TSVH();
}


void Convert_Opera_ToPack103_TSVH(void) 
{
	uint32_t	i=10;
	uint8_t     ObisHex = 0;
    uint8_t     Length = 0;
    uint16_t     j = 0; //dem trong byte data
    uint32_t    Temp_data_32;
    uint16_t    Temp_data_16;
    uint64_t    Temp_data_64;
    uint64_t    ScaleRead = 1;
    uint64_t        Div = 0;
    int8_t          RowObis103 = 0;
    uint8_t         BuffNum[30];
    truct_String    Str_Data_Write={&BuffNum[0], 0};
    uint8_t         BuffTemp[12];
    ST_TIME_FORMAT  sTime_temp;
    
	//dong goi lai theo 103 o day
    if(Get_Meter_Info.Flag_Start_Pack == 0)
    {
        Reset_Buff(&Get_Meter_Info.Str_Payload);
        //wrire header TSVH vao
        Write_Header_TSVH_Push103();
        Get_Meter_Info.Flag_Start_Pack = 1;
    }  
    //bat dau cat data tu form cu sang form 103
    while(i <Get_Meter_Info.Data_Buff_Pointer_ui16)
    {
        Temp_data_16 = 0;
        Temp_data_32 = 0;
        Temp_data_64 = 0;
        Reset_Buff(&Str_Data_Write);
        
        //Get Obis Hex
        ObisHex = Meter_TempBuff[i++];
        Length = Meter_TempBuff[i++];
        switch(Length)
        {
            case 2:   //0x12
                for(j = 0; j < Length; j++)
                    Temp_data_16 = Temp_data_16 <<8 | Meter_TempBuff[i++];
                break;
            case 4:  //0x05: 05 l� int32_t
                for(j = 0; j < Length; j++)
                    Temp_data_32 = Temp_data_32 <<8 | Meter_TempBuff[i++];
                break;
            case 8:  
                for(j = 0; j < Length; j++)
                    Temp_data_64 = Temp_data_64 <<8 | Meter_TempBuff[i++];
                break;
            case 0x0C:   //gia tri nang luong. va ca stime cua (meter va maxdemand)   //0x14 0x09
                for(j = 0; j < Length; j++)
                    BuffTemp[j] = Meter_TempBuff[i++];
                
                Temp_data_16     = (BuffTemp[0] << 8) | (BuffTemp[1]);
                sTime_temp.year  = Temp_data_16%100;
                sTime_temp.month = BuffTemp[2];
                sTime_temp.date  = BuffTemp[3];
                sTime_temp.day   = BuffTemp[4];
                sTime_temp.hour  = BuffTemp[5];
                sTime_temp.min   = BuffTemp[6];
                sTime_temp.sec   = BuffTemp[7];
                break;
            default: 
                i += Length;
                break;
        }
        
        //kiem tra obis xem co can lay khong. Neu co dong goi sang 1 buffer khac
        RowObis103 = LANDIS_Check_Row_Obis103(&Obis_Landis_Ins[0], ObisHex);
        if(RowObis103 >=0)
        {
            if(Obis_Landis_Ins[RowObis103].Str_Obis != NULL)
            {
                Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Obis_Inbuff, Obis_Landis_Ins[RowObis103].Str_Obis);
                Get_Meter_Info.Pos_Obis_Inbuff += Obis_Landis_Ins[RowObis103].Str_Obis->Length_u16;
                Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
                Get_Meter_Info.Numqty++;
                //do cac dai luong nang luong va power lay unit la kw. chu k phai don vi nho nhat nen scale tach 2 loai
                if(((ObisHex >= 0x02) && (ObisHex <= 0x09)) || ((ObisHex >= 0x1B) && (ObisHex <= 0x1E)))
                    ScaleRead = ConvertScaleMeter_toDec (Obis_Landis_Ins[RowObis103].Scale_ReadMeter + Obis_Landis_Ins[RowObis103].scale, &Div);
                else ScaleRead = ConvertScaleMeter_toDec (Obis_Landis_Ins[RowObis103].Scale_ReadMeter, &Div);  //de nhu the nay la ve gia tri thuc. voi don vi nho nhat. TI nua scale theo unit can

                switch(Length)
                {
                    case 2:  //giai m� HEx -> Dec c� dau
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                        Pack_HEXData_Frame(&Str_Data_Write, (int64_t) (Convert_uint16_2int16 (Temp_data_16) * ScaleRead /Div) , Obis_Landis_Ins[RowObis103].scale);
                        if(Obis_Landis_Ins[RowObis103].Str_Unit != NULL)
                        {
                            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                            Copy_String_2(&Str_Data_Write, Obis_Landis_Ins[RowObis103].Str_Unit);  //don vi
                        }
                        if((ObisHex == 0x41) || (ObisHex == 0x42))
                        {
                            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '/';
                            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '1';
                        }
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                        break;
                    case 4:  //
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                        Pack_HEXData_Frame(&Str_Data_Write, ((int64_t) Convert_uint_2int(Temp_data_32) * ScaleRead / Div), Obis_Landis_Ins[RowObis103].scale);
                        if(Obis_Landis_Ins[RowObis103].Str_Unit != NULL)
                        {
                            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                            Copy_String_2(&Str_Data_Write, Obis_Landis_Ins[RowObis103].Str_Unit);  //don vi
                        }
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';  
                        break;
                    case 8:   //gia tri nang luong. 
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                        Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) (Temp_data_64 * ScaleRead / Div), Obis_Landis_Ins[RowObis103].scale);
                        if(Obis_Landis_Ins[RowObis103].Str_Unit != NULL)
                        {
                            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                            Copy_String_2(&Str_Data_Write, Obis_Landis_Ins[RowObis103].Str_Unit);  //don vi
                        }
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                        break;
                    default: 
                        break;
                }
                Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Data_Inbuff, &Str_Data_Write);
                Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
            }else if((ObisHex == 0x32) || (ObisHex == 0x36) || (ObisHex == 0x3A))
            {
                *(Get_Meter_Info.Str_Payload.Data_a8 + Get_Meter_Info.Str_Payload.Length_u16++) = '(';
                Copy_String_STime(&Get_Meter_Info.Str_Payload,sTime_temp);
                *(Get_Meter_Info.Str_Payload.Data_a8 + Get_Meter_Info.Str_Payload.Length_u16++) = ')';
                Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
            }
        }else if(ObisHex == 0x01)
        {
            //ghi he so nhan vao
//            Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Obis_Inbuff,  &He_So_Nhan);
//            Get_Meter_Info.Pos_Obis_Inbuff += He_So_Nhan.Length_u16;
//            Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
//            Get_Meter_Info.Numqty++;
//            Reset_Buff(&Str_Data_Write);
//            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
//            Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) sDCU.He_So_Nhan, LANDIS_SCALE_HE_SO_NHAN); 
//            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
//            Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Data_Inbuff, &Str_Data_Write);
//            Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
            
            Reset_Buff(&Str_Data_Write);
            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
            Copy_String_STime(&Str_Data_Write,sTime_temp);
            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
//            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
//            Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t)  Get_Meter_Info.Numqty, 0);
//            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
            Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.PosNumqty, &Str_Data_Write);
            Get_Meter_Info.PosNumqty += Str_Data_Write.Length_u16;
            Get_Meter_Info.Pos_Obis_Inbuff += Str_Data_Write.Length_u16;
            Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
            
            break;
        }
    }
    
//	//ETX
//    *(Get_Meter_Info.Str_Payload.Data_a8 + Get_Meter_Info.Str_Payload.Length_u16++) = ETX; 
//    //BBC
//    Temp_BBC = BBC_Cacul(Get_Meter_Info.Str_Payload.Data_a8 + 1,Get_Meter_Info.Str_Payload.Length_u16 - 1);
//    *(Get_Meter_Info.Str_Payload.Data_a8 + Get_Meter_Info.Str_Payload.Length_u16++) = Temp_BBC;
}

void LANDIS_MI_SendData_Inst(void)
{
    Convert_Opera_ToPack103_TSVH();
//    Push_TSVH_toQueue(DATA_INTANTANIOUS);
}
void LANDIS_ME_SendData(void)
{
    uint8_t         Buff[20];
    truct_String    Str_Data_Write = {&Buff[0], 0};
	uint8_t         Temp_BBC = 0;
    	
	Add_TuTI_toPayload(&Get_Meter_Event);
    
    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
    Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) Get_Meter_Event.Numqty, 0);
    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')'; 
    
    Copy_String_toTaget(&Get_Meter_Event.Str_Payload, Get_Meter_Event.PosNumqty, &Str_Data_Write);
    //
    //ETX
    *(Get_Meter_Event.Str_Payload.Data_a8 + Get_Meter_Event.Str_Payload.Length_u16++) = ETX; 
    //BBC
    Temp_BBC = BBC_Cacul(Get_Meter_Event.Str_Payload.Data_a8 + 1,Get_Meter_Event.Str_Payload.Length_u16 - 1);
    *(Get_Meter_Event.Str_Payload.Data_a8 + Get_Meter_Event.Str_Payload.Length_u16++) = Temp_BBC;            
    
    Get_Meter_Event.Flag_Start_Pack = 0;   //cho phep dong goi head tiep
    //gui thang sang queue Sim de gui luon
    
    Push_Even_toQueue(0);
}


void LANDIS_ME_SendData_TSVH(void)
{
    truct_String    StrTemp;
    
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
}

void LANDIS_MScale_ExtractDataFunc(void)  // process function Meter ID and Scale
{
	uint8_t Data_type=0,j=0;
    int8_t RowObis103 = 0;
    
	j = LANDIS_GetScale_Code_Table[Get_Meter_Scale.Mess_Step_ui8];  // lay Scale o Step doc
	Data_type = UART1_Receive_Buff[14];  // Xac dinh Type data
	switch (Data_type)
	{
		case 0x02:// lay Scale
			if (UART1_Receive_Buff[16] == 0x0F)
			{
                RowObis103 = LANDIS_Check_Row_Obis103(&Obis_Landis_Ins[0], LANDIS_Info_SN_Table[j][0]);
                if(RowObis103 >=0)
                    Obis_Landis_Ins[RowObis103].Scale_ReadMeter = UART1_Receive_Buff[17];
			}
			break;
		default:
			break;
	}
}
void LANDIS_MS_SendData(void)
{
	
}

void LANDIS_Fill_Scale_Mess (void)  // 
{
	uint8_t 	j=0;
	uint16_t	TempCheckSum = 0;
	uint16_t 	TempSN = 0;
	//Insert ID Frame
	if(Get_Meter_Scale.Mess_Step_ui8==0)
		Get_Meter_Scale.ID_Frame_ui8 = 0x32;
	LANDIS_MInfo_MTemp[5] = Get_Meter_Scale.ID_Frame_ui8;
	//Calculate Header checksum
	TempCheckSum = CountFCS16(&LANDIS_MInfo_MTemp[0],1,5);
	//Insert Header checksum
	LANDIS_MInfo_MTemp[6] = (uint8_t)((TempCheckSum>>8)&0x00FF);
	LANDIS_MInfo_MTemp[7] = (uint8_t)(TempCheckSum&0x00FF);
	do
	{
        j = LANDIS_GetScale_Code_Table[Get_Meter_Scale.Mess_Step_ui8];
        LANDIS_MInfo_MTemp[14] = LANDIS_Info_SN_Table[j][1];
        LANDIS_MInfo_MTemp[15] = LANDIS_Info_SN_Table[j][2];
        TempSN = LANDIS_MInfo_MTemp[14]+LANDIS_MInfo_MTemp[15];
        Get_Meter_Scale.Mess_Step_ui8++;
	}
	while((TempSN == 0)&&(LANDIS_GetScale_Code_Table[Get_Meter_Scale.Mess_Step_ui8]!= 0xFF));
	Get_Meter_Scale.Mess_Step_ui8--;
	TempSN = LANDIS_MInfo_MTemp[14]<<8;
	TempSN = TempSN + LANDIS_MInfo_MTemp[15];
	TempSN = TempSN + 0x10; // +0x10 - Get Scale
	LANDIS_MInfo_MTemp[14] = (uint8_t)(TempSN>>8);
	LANDIS_MInfo_MTemp[15] = (uint8_t)TempSN;
	//Calculate Total checksum
	TempCheckSum = CountFCS16(&LANDIS_MInfo_MTemp[0],1,15);
	//Insert Total checksum
	LANDIS_MInfo_MTemp[16] = (uint8_t)((TempCheckSum>>8)&0x00FF);
	LANDIS_MInfo_MTemp[17] = (uint8_t)(TempCheckSum&0x00FF);
	//Calculate next ID Frame
	Get_Meter_Scale.ID_Frame_ui8 += 0x22;
	if (Get_Meter_Scale.ID_Frame_ui8 == 0x20)
		Get_Meter_Scale.ID_Frame_ui8 = 0x10;
	// Send
	if(Meter_Type == 1) RS485_SEND;
	osDelay(LANDIS_Meter_Delay_ms);
	HAL_UART_Transmit(&UART_METER,&LANDIS_MInfo_MTemp[0],19,1000);
	if(Meter_Type == 1) RS485_RECIEVE;
}


uint8_t LANDIS_Read_His_Load(Meter_Comm_Struct *Meter_Comm, uint8_t *First_Mess_Addr, uint8_t Mess_Type,void (*FuncExtractRawData)(uint8_t MType), uint8_t (*DataHandle)(void))
{
	uint8_t 	mReVal = 0;
	uint16_t	mTemp_CheckSum=0;
	
	//Start read
	Meter_Comm->Reading_ui8 = 1;
	Meter_Comm->ID_Frame_ui8 = 0x51;
	if(Meter_Type == 1) RS485_SEND;
	while(Meter_Comm->Reading_ui8 == 1) {
		switch(Meter_Comm->Step_ui8) {
			case 0:
				Meter_Comm->Total_Mess_Sent_ui32++;
				//Wait for respond message from Meter in 2000ms: checksum - extract raw data - save to buffer - goto next message
				if (osSemaphoreWait(bsUART2PendingMessHandle,5000) == osOK) {
					//Checksum
					LANDIS_Mess_Checksum();
					
					if (UART1_Control.Mess_Status_ui8 == 2) {
						Meter_Comm->Success_Read_Mess_ui32++;
						//Extract raw data
						if (Meter_Comm->Mess_Step_ui8 > 1)
							FuncExtractRawData(Mess_Type);  // nhom data
						//Goto next message
						Meter_Comm->Mess_Step_ui8++;
						Meter_Comm->Error_ui8 = 0x00;
						Meter_Comm->Step_ui8 = 1;
						//If all mess sent -> finish
						if ((UART1_Receive_Buff[1] == 0xA0)&&(Meter_Comm->Mess_Step_ui8>2)) {
							//Send END message
							if(Meter_Type == 1) RS485_SEND;
							//osDelay(3);
							//HAL_UART_Transmit(&UART_METER,&LANDIS_Read_END[0],9,1000);
							if(Meter_Type == 1) RS485_RECIEVE;
							//Extract OBIS list
							mReVal = DataHandle();  // Ham xu li data
							Meter_Comm->Step_ui8 = 3;
						}
                        //Reset UART2 status
						Init_UART2();
					} else {
						Meter_Comm->Error_Wrong_Mess_Format_ui32++;
						//Reset UART
						Init_UART2();
						Meter_Comm->Step_ui8 = 2;
					}
				} else {//If Meter no respond after 2000ms: wait 2000ms - resend the message (3 times) 
					Meter_Comm->Error_Meter_Norespond_ui32++;
                    HAL_UART_Init(&UART_METER);
					Init_UART2();
					__HAL_UART_ENABLE_IT(&UART_METER, UART_IT_RXNE);
					osDelay(1000);
					//Reset UART
					Init_UART2();
					Meter_Comm->Step_ui8 = 2;
				}
				break;
			case 1:
				if (Meter_Comm->Mess_Step_ui8 == 0) 
				{
					if(Meter_Type == 1) RS485_SEND;
					HAL_UART_Transmit(&UART_METER,(uint8_t*)&LANDIS_SNRM[0],34,1000);
					if(Meter_Type == 1) RS485_RECIEVE;
				}
				else if (Meter_Comm->Mess_Step_ui8 == 1) 
				{
					if(Meter_Type == 1) RS485_SEND;
					osDelay(3);
					HAL_UART_Transmit(&UART_METER,(uint8_t*)&LANDIS_AARQ[0],48,1000);
					if(Meter_Type == 1) RS485_RECIEVE;
				} 
				else if (Meter_Comm->Mess_Step_ui8 == 2) 
				{
					if(Meter_Type == 1) RS485_SEND;
					osDelay(3);
					switch (Mess_Type)
					{
						case 1:
							HAL_UART_Transmit(&UART_METER,First_Mess_Addr,19,1000); // _ Obis/SN length  = sizeof(First Message)
							break;
						case 2: 
							HAL_UART_Transmit(&UART_METER,First_Mess_Addr,19,1000); // Get SN, First_Message_Addr
							break;
						case 3:
							HAL_UART_Transmit(&UART_METER,First_Mess_Addr,21,1000); // _
							break;
						case 4:
							HAL_UART_Transmit(&UART_METER,First_Mess_Addr,70,1000); // _ Time - Gui kem thoi gian de lay data
						default:
							break;
					}
					
					if(Meter_Type == 1) RS485_RECIEVE;
				}
				else   // gui Nextframe
				{
					if(Meter_Type == 1) RS485_SEND;
					LANDIS_NextFrame[5] = Meter_Comm->ID_Frame_ui8;
					mTemp_CheckSum = CountFCS16(LANDIS_NextFrame,1,5);
					LANDIS_NextFrame[6] = (uint8_t)((mTemp_CheckSum>>8)&0x00FF);
					LANDIS_NextFrame[7] = (uint8_t)(mTemp_CheckSum&0x00FF);
					Meter_Comm->ID_Frame_ui8 += 0x20;
					osDelay(3);
					HAL_UART_Transmit(&UART_METER,&LANDIS_NextFrame[0],9,1000);
					if(Meter_Type == 1) RS485_RECIEVE;
					
				}
				Meter_Comm->Step_ui8 = 0;  // quay ve check data
				break;
			case 2:
				//Reset UART2 status
				Init_UART2();
				//Jump out of reading cycle, wait for next cycle (after 30mins)
				Meter_Comm->Reading_ui8 = 0;
				Meter_Comm->Step_ui8 = 1;
				Meter_Comm->Mess_Step_ui8 = 0;
				//Reset pointer
				Meter_Comm->Data_Buff_Pointer_ui16 = 10;
				break;
			case 3:
				Meter_Comm->Total_Mess_Sent_ui32++;
				if (osSemaphoreWait(bsUART2PendingMessHandle,5000) == osOK) 
				{
					//Checksum
					LANDIS_Mess_Checksum();
					
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

uint8_t LANDIS_Read_LProfile(Meter_Comm_Struct *Meter_Comm, uint8_t *First_Mess_Addr, uint8_t Mess_Type,void (*FuncExtractRawData)(uint8_t MType), uint8_t (*DataHandle)(void))
{
	uint8_t 	mReVal = 0;
	uint16_t	mTemp_CheckSum=0;
	
	//Start read
	Meter_Comm->Reading_ui8 = 1;
	Meter_Comm->ID_Frame_ui8 = 0x51;
	if(Meter_Type == 1) RS485_SEND;
	while(Meter_Comm->Reading_ui8 == 1) {
		switch(Meter_Comm->Step_ui8) {
			case 0:
				Meter_Comm->Total_Mess_Sent_ui32++;
				//Wait for respond message from Meter in 2000ms: checksum - extract raw data - save to buffer - goto next message
				if (osSemaphoreWait(bsUART2PendingMessHandle,5000) == osOK) {
					//Checksum
					LANDIS_Mess_Checksum();
					
					if (UART1_Control.Mess_Status_ui8 == 2) {
						Meter_Comm->Success_Read_Mess_ui32++;
						//Extract raw data
						if (Meter_Comm->Mess_Step_ui8 > 1)
							FuncExtractRawData(Mess_Type);  // nhom data
						//Goto next message
						Meter_Comm->Mess_Step_ui8++;
						Meter_Comm->Error_ui8 = 0x00;
						Meter_Comm->Step_ui8 = 1;
						//If all mess sent -> finish			
						if ((UART1_Receive_Buff[1] == 0xA0)&&(Meter_Comm->Mess_Step_ui8>2)) {
							//Send END message
							if(Meter_Type == 1) RS485_SEND;
							//osDelay(3);
							//HAL_UART_Transmit(&UART_METER,&LANDIS_Read_END[0],9,1000);
							if(Meter_Type == 1) RS485_RECIEVE;
							//Extract OBIS list
							mReVal = DataHandle();  // Ham xu li data
							Meter_Comm->Step_ui8 = 3;
						}
						else
							if((Get_Meter_LProf.Reading_Block_ui8%4 == 0)&&(Meter_Comm->Mess_Step_ui8>2))
								mReVal = DataHandle();  // Ham xu li data
                        //Reset UART2 status
						Init_UART2();
					} else {
						Meter_Comm->Error_Wrong_Mess_Format_ui32++;
						//Reset UART
						Init_UART2();
						Meter_Comm->Step_ui8 = 2;
					}
				} else {//If Meter no respond after 2000ms: wait 2000ms - resend the message (3 times) 
					Meter_Comm->Error_Meter_Norespond_ui32++;
					//Reinit uart2
                    HAL_UART_Init(&UART_METER);
					Init_UART2();
					__HAL_UART_ENABLE_IT(&UART_METER, UART_IT_RXNE);
					osDelay(1000);
					Meter_Comm->Step_ui8 = 2;
				}
				break;
			case 1:
				if (Meter_Comm->Mess_Step_ui8 == 0) 
				{
					if(Meter_Type == 1) RS485_SEND;
					HAL_UART_Transmit(&UART_METER,(uint8_t*)&LANDIS_SNRM[0],34,1000);
					if(Meter_Type == 1) RS485_RECIEVE;
				}
				else if (Meter_Comm->Mess_Step_ui8 == 1) 
				{
					if(Meter_Type == 1) RS485_SEND;
					osDelay(3);
					HAL_UART_Transmit(&UART_METER,(uint8_t*)&LANDIS_AARQ[0],48,1000);
					if(Meter_Type == 1) RS485_RECIEVE;
				} 
				else if (Meter_Comm->Mess_Step_ui8 == 2) 
				{
					if(Meter_Type == 1) RS485_SEND;
					osDelay(3);
					Get_Meter_LProf.Reading_Block_ui8++;  // check block
					switch (Mess_Type)
					{
						case 1:
							HAL_UART_Transmit(&UART_METER,First_Mess_Addr,19,1000); // _ Obis/SN length  = sizeof(First Message)
							break;
						case 2: 
							HAL_UART_Transmit(&UART_METER,First_Mess_Addr,19,1000); // Get SN, First_Message_Addr
							break;
						case 3:
							HAL_UART_Transmit(&UART_METER,First_Mess_Addr,21,1000); // _
							break;
						case 4:
							HAL_UART_Transmit(&UART_METER,First_Mess_Addr,70,1000); // _ Time - Gui kem thoi gian de lay data
						default:
							break;
					}
					
					if(Meter_Type == 1) RS485_RECIEVE;
				}
				else   // gui Nextframe
				{
					if(Meter_Type == 1) RS485_SEND;
					LANDIS_NextFrame[5] = Meter_Comm->ID_Frame_ui8;
					mTemp_CheckSum = CountFCS16(LANDIS_NextFrame,1,5);
					LANDIS_NextFrame[6] = (uint8_t)((mTemp_CheckSum>>8)&0x00FF);
					LANDIS_NextFrame[7] = (uint8_t)(mTemp_CheckSum&0x00FF);
					Meter_Comm->ID_Frame_ui8 += 0x20;
					osDelay(3);
					HAL_UART_Transmit(&UART_METER,&LANDIS_NextFrame[0],9,1000);
					if(Meter_Type == 1) RS485_RECIEVE;
					Get_Meter_LProf.Reading_Block_ui8++;  // check block
				}
				Meter_Comm->Step_ui8 = 0;  // quay ve check data
				break;
			case 2:
				//Reset UART2 status
				Init_UART2();
				//Jump out of reading cycle, wait for next cycle (after 30mins)
				Meter_Comm->Reading_ui8 = 0;
				Meter_Comm->Step_ui8 = 1;
				Meter_Comm->Mess_Step_ui8 = 0;
				//Reset pointer
				Meter_Comm->Data_Buff_Pointer_ui16 = 10;
				break;
			case 3:
				Meter_Comm->Total_Mess_Sent_ui32++;
				if (osSemaphoreWait(bsUART2PendingMessHandle,5000) == osOK) 
				{
					//Checksum
					LANDIS_Mess_Checksum();
					
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

uint8_t LANDIS_CheckOBISInTable(uint16_t buff_start_pos, uint8_t table_end_pos)
{
	uint8_t row=0,column=0,matched_obis=0xFF,matched_count=0;
	
	for (row=0;row<table_end_pos;row++)
	{
		for (column=0;column<6;column++)
		{
			if (Meter_TempBuff[buff_start_pos+column] == LANDIS_OBIS_Table[row][column])
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
	
	if (matched_obis == 0xFF) // ko tim thay - obis chua dc dinh nghia
		return 0xFF;
	else
		return matched_obis;
}

void LANDIS_Prepare_Read_Date(uint8_t *Start, uint8_t *Stop)
{
	uint8_t i=0;
	
	for (i=0;i<8;i++)
		LANDIS_Get_DATA_Template[i+39] = *(Start+i); // 47
	
	for (i=0;i<8;i++)
		LANDIS_Get_DATA_Template[i+53] = *(Stop+i); // 
}
//                                                                                                                
	                                                                                                             
void LANDIS_MBillingGetRawData(uint8_t Mess_Type)  // ham xu li ghep data de lay data cua ban tin Historical
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
				if (UART1_Receive_Buff[16] == 0x02)
				{
					data_pos = 16;  // Xac dinh vi tri data
					for (i=0;i<UART1_Control.Mess_Length_ui16-data_pos-2;i++)
						Meter_TempBuff[Get_Meter_Billing.Data_Buff_Pointer_ui16++] = UART1_Receive_Buff[i+data_pos];
				}
				else  // ko xay ra vs Landis
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
			Meter_TempBuff[Get_Meter_Billing.Data_Buff_Pointer_ui16++] = UART1_Receive_Buff[i+8];  // Start byte 8
}
// Them return cho ham
void LANDIS_MBillingExtractOBISList(void)   // ham xu li cat tung byte obis cua ban tin Historical
{
	uint16_t 	OBISList_Length = 0,i=0;
	uint8_t		Data_OBIS = 0,OBIS_Row=0;
	uint8_t 	Start_index = 0;
	// Ghep data cua tung ban tin
    if (Get_Meter_Billing.Mess_Step_ui8 == 0) Start_index = 14;
    else Start_index = 8;
    for (i=0;i<UART1_Control.Mess_Length_ui16-Start_index-2;i++)
        Meter_TempBuff[Get_Meter_Billing.Data_Buff_Pointer_ui16++] = UART1_Receive_Buff[i+Start_index];  // Start byte 8
	// Xu li toan bo chuoi da ta nhan duoc
	if (UART1_Receive_Buff[1] == 0xA0){
		i = 10;
		LANDIS_MHis_Code_Obis_Table[Get_Meter_Billing.Mess_Step_ui8+1] = 0xFF;  // Out
		OBISList_Length = Get_Meter_Billing.Data_Buff_Pointer_ui16;
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
					if (Meter_TempBuff[i+1] == 0x06) // Lay Obis
					{
						OBIS_Row = LANDIS_CheckOBISInTable(i+2,56);  // Quet bang obis tu dau den cuoi
						if (OBIS_Row != 0xFF)  // lay obis tuong ung
							LANDIS_MBilling_OBIS_List[Get_Meter_Billing.OBIS_Pointer_ui8++] = LANDIS_OBIS_Table[OBIS_Row][6];
						else
							LANDIS_MBilling_OBIS_List[Get_Meter_Billing.OBIS_Pointer_ui8++] = 0xFF;  // chua dinh nghia
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
			if (Get_Meter_Billing.Error_ui8 == 0x10) break;  // Check them dk so obis nhan dc phai bang so obis thong bao -> loi
//				return 0;
		};
//		if (Get_Meter_Billing.Num_OBIS_ui8 == Get_Meter_Billing.Total_OBIS_ui8)
//			return 1;
//		else
//			return 0;
//		return 1;
	}
}

void LANDIS_MBillingExtractData(void)
{
	uint16_t 	Data_Length = 0,i=10,j=0;
	uint8_t		Data_OBIS = 0,Temp_length=0;
	uint32_t	Start_index = 0;
	// Ghep data cua tung ban tin
	if (Get_Meter_Billing.Mess_Step_ui8 == 0) Start_index = 14;
        else Start_index = 8;
	for (i=0;i<UART1_Control.Mess_Length_ui16-Start_index-2;i++)
		Meter_TempBuff[Get_Meter_Billing.Data_Buff_Pointer_ui16++] = UART1_Receive_Buff[i+Start_index];  // Start byte 8	
    
	if (UART1_Receive_Buff[1] == 0xA0)
    {
		LANDIS_MHis_Code_Data_Table[Get_Meter_Billing.Mess_Step_ui8+1] = 0xFF;  // Out
		
		Get_Meter_Billing.Num_OBIS_ui8 = 0;
		Get_Meter_Billing.OBIS_Pointer_ui8 = 0;
		Data_Length = Get_Meter_Billing.Data_Buff_Pointer_ui16;
		Get_Meter_Billing.Data_Buff_Pointer_ui16 = 10;  // Reset point
        i = 10;
        while (i<Data_Length)
        {
            Data_OBIS = Meter_TempBuff[i];
            switch (Data_OBIS)
            {
                case 0x01:
                    // So record dc luu
                    Get_Meter_Billing.Num_Record_ui16 = Meter_TempBuff[i+1];
                    i += 2;
                    break;
                case 0x02:
                    Get_Meter_Billing.Total_OBIS_ui8 = Meter_TempBuff[i+1];
                    i += 2;
                    break;
                case 0x05:
                    if(LANDIS_MBilling_OBIS_List[Get_Meter_Billing.Num_OBIS_ui8] == 0xFF)
                        i+=5;
                    else
                    {
                        MeterBillingDataBuff_2[Get_Meter_Billing.Data_Buff_Pointer_ui16++] = LANDIS_MBilling_OBIS_List[Get_Meter_Billing.Num_OBIS_ui8];
                        MeterBillingDataBuff_2[Get_Meter_Billing.Data_Buff_Pointer_ui16++] = 0x04;
                        i++;
                        for (j=0;j<4;j++)
                            MeterBillingDataBuff_2[Get_Meter_Billing.Data_Buff_Pointer_ui16++] = Meter_TempBuff[i++];
                    }
                    Get_Meter_Billing.Num_OBIS_ui8++;
                    break;
                case 0x06:
                    if(LANDIS_MBilling_OBIS_List[Get_Meter_Billing.Num_OBIS_ui8] == 0xFF)
                    {
                        i+=5;
                        Get_Meter_Billing.OBIS_Pointer_ui8++;
                    }
                    else
                    {
                        MeterBillingDataBuff_2[Get_Meter_Billing.Data_Buff_Pointer_ui16++] = LANDIS_MBilling_OBIS_List[Get_Meter_Billing.Num_OBIS_ui8];
                        MeterBillingDataBuff_2[Get_Meter_Billing.Data_Buff_Pointer_ui16++] = 0x04;
                        i++;
                        for (j=0;j<4;j++)
                            MeterBillingDataBuff_2[Get_Meter_Billing.Data_Buff_Pointer_ui16++] = Meter_TempBuff[i++];
                    }
                    Get_Meter_Billing.Num_OBIS_ui8++;
                    break;
                case 0x09:
                    if(LANDIS_MBilling_OBIS_List[Get_Meter_Billing.Num_OBIS_ui8] == 0xFF)
                    {
                        i+= Meter_TempBuff[i+1]+2;
                        Get_Meter_Billing.OBIS_Pointer_ui8++;
                    }
                    else
                    {
                        // insert fields Maxdemand time	
                        if((LANDIS_MBilling_OBIS_List[Get_Meter_Billing.Num_OBIS_ui8] == 0x2E)||(LANDIS_MBilling_OBIS_List[Get_Meter_Billing.Num_OBIS_ui8] == 0x31)
                        ||(LANDIS_MBilling_OBIS_List[Get_Meter_Billing.Num_OBIS_ui8] == 0x35)||(LANDIS_MBilling_OBIS_List[Get_Meter_Billing.Num_OBIS_ui8] == 0x39)
                        ||(LANDIS_MBilling_OBIS_List[Get_Meter_Billing.Num_OBIS_ui8] == 0x3D))
                            MeterBillingDataBuff_2[Get_Meter_Billing.Data_Buff_Pointer_ui16++] = LANDIS_MBilling_OBIS_List[Get_Meter_Billing.Num_OBIS_ui8]+1;
                        else
                            MeterBillingDataBuff_2[Get_Meter_Billing.Data_Buff_Pointer_ui16++] = LANDIS_MBilling_OBIS_List[Get_Meter_Billing.Num_OBIS_ui8];
                        i++;
                        Temp_length= Meter_TempBuff[i++];
                        MeterBillingDataBuff_2[Get_Meter_Billing.Data_Buff_Pointer_ui16++] = Temp_length;
                        for (j=0;j<Temp_length;j++)
                            MeterBillingDataBuff_2[Get_Meter_Billing.Data_Buff_Pointer_ui16++] = Meter_TempBuff[i++];
                    }
                    Get_Meter_Billing.Num_OBIS_ui8++;
                    break;
                case 0x14:
                    if(LANDIS_MBilling_OBIS_List[Get_Meter_Billing.Num_OBIS_ui8] == 0xFF)
                    {
                        i+= 9;
                        Get_Meter_Billing.OBIS_Pointer_ui8++;
                    }
                    else
                    {
                        MeterBillingDataBuff_2[Get_Meter_Billing.Data_Buff_Pointer_ui16++] = LANDIS_MBilling_OBIS_List[Get_Meter_Billing.Num_OBIS_ui8];
                        MeterBillingDataBuff_2[Get_Meter_Billing.Data_Buff_Pointer_ui16++] = 0x08;
                        i++;
                        for (j=0;j<8;j++)
                            MeterBillingDataBuff_2[Get_Meter_Billing.Data_Buff_Pointer_ui16++] = Meter_TempBuff[i++];				
                    }
                    Get_Meter_Billing.Num_OBIS_ui8++;
                    break;
                default:
                    Get_Meter_Billing.Error_ui8 = 0x10;
                    break;
            }
            if(Get_Meter_Billing.Num_OBIS_ui8 == Get_Meter_Billing.Total_OBIS_ui8)
            {
                Convert_His_To103Pack();
                Push_Bill_toQueue(DATA_HISTORICAL);
                break;
            }
            if (Get_Meter_Billing.Error_ui8 == 0x10) 
                break;
        };
        Get_Meter_Billing.OBIS_Pointer_ui8 = 0;
        Get_Meter_Billing.Num_OBIS_ui8 = 0;
    }
}
//                                                                                                                


void LANDIS_MLProfileExtractOBISList(void)
{
	uint16_t 	OBISList_Length = 0,i=10, Start_index = 0;
	uint8_t		Data_OBIS = 0,OBIS_Row=0;

	// Ghep data cua tung ban tin
	if (Get_Meter_LProf.Mess_Step_ui8 == 0) Start_index = 14;
	else Start_index = 8;
	for (i=0;i<UART1_Control.Mess_Length_ui16-Start_index-2;i++)
		Meter_TempBuff[Get_Meter_LProf.Data_Buff_Pointer_ui16++] = UART1_Receive_Buff[i+Start_index];  // Start byte 8
	if (UART1_Receive_Buff[1] == 0xA0){
		i = 10;
		LANDIS_MLprofile_Code_Obis_Table[Get_Meter_LProf.Mess_Step_ui8+1] = 0xFF;  // Out
		OBISList_Length = Get_Meter_LProf.Data_Buff_Pointer_ui16;
	//	
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
						OBIS_Row = LANDIS_CheckOBISInTable(i+2,56);
						if (OBIS_Row != 0xFF)
							LANDIS_MLProfile_OBIS_List[Get_Meter_LProf.OBIS_Pointer_ui8++] = LANDIS_OBIS_Table[OBIS_Row][6];
						else
							LANDIS_MLProfile_OBIS_List[Get_Meter_LProf.OBIS_Pointer_ui8++] = 0xFF;
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
            if (Get_Meter_LProf.Error_ui8 == 0x10) break; // Check them dk so obis lay dc phai bang so obis thong bao
		};
	}
}
void LANDIS_MLProfileExtractData(void)
{
	uint16_t 	Data_Length = 0,i=10,j=0;
	uint8_t		Data_OBIS = 0, Start_index = 0;
    
    uint8_t     ObisHex = 0;
    uint8_t     Length = 0;
    uint32_t    Temp_data_32;
    uint16_t    Temp_data_16;
    uint64_t    Temp_data_64;
    int8_t          RowObis103 = 0;
    uint8_t         BuffNum[30];
    truct_String    Str_Data_Write={&BuffNum[0], 0};
    uint8_t         BuffTemp[12];
    ST_TIME_FORMAT  sTime_temp;
    uint8_t         sign = 0;
    uint64_t        ScaleRead = 1;
    uint64_t        Div = 0;
    uint32_t        sTimeSecond = 0;
    
	//Extract data - Insert data to buffer
	if (Get_Meter_LProf.Mess_Step_ui8 == 0) 
        Start_index = 14;
    else Start_index = 8;
    //
    if(Get_Meter_LProf.Data_Buff_Pointer_ui16 < (MAX_LENGTH_BUFF_TEMP - 500))   //vi co cong to test no dang de 1p luu 1 lan. hoac cai nhieu thong so.nen doc mat nhieu byte de luu.
        for (i=0;i<UART1_Control.Mess_Length_ui16-Start_index-2;i++)
            Meter_TempBuff[Get_Meter_LProf.Data_Buff_Pointer_ui16++] = UART1_Receive_Buff[i+Start_index];  // Start byte 8	
    
	if ((UART1_Receive_Buff[1] == 0xA0) || (Get_Meter_LProf.Mess_Step_ui8 >= (sizeof(LANDIS_MLprofile_Code_Data_Table) - 2)))   //step = 198 la dung
    {
		Data_Length = Get_Meter_LProf.Data_Buff_Pointer_ui16;
		LANDIS_MLprofile_Code_Data_Table[Get_Meter_LProf.Mess_Step_ui8+1] = 0xFF;  // break. dung lai o day
		Get_Meter_LProf.Data_Buff_Pointer_ui16 = 10; // reset point
        i = 10;
		while (i<Data_Length)
		{
            sign = 0;
            Temp_data_16 = 0;
            Temp_data_32 = 0;
            Temp_data_64 = 0;
            ObisHex = 0;
            Reset_Buff(&Str_Data_Write);//dong goi lai theo 103 o day
            
            if(Get_Meter_LProf.Flag_Start_Pack == 0)
            {
                Reset_Buff(&Get_Meter_LProf.Str_Payload);
                //wrire header TSVH vao
                Pack_Header_lpf_Pushdata103();
                Get_Meter_LProf.Flag_Start_Pack = 1;
            }
    
			Data_OBIS = Meter_TempBuff[i];
			switch (Data_OBIS)
			{
				case 0x01:
                    if(Meter_TempBuff[i+1] == 0x81)
                    {
                        Get_Meter_LProf.Num_Record_ui16 = Meter_TempBuff[i+2]; // xac dinh so ban ghi Load profile
                        i++;
                    }else Get_Meter_LProf.Num_Record_ui16 = Meter_TempBuff[i+1];
					i += 2;
					break;
				case 0x02:
					Get_Meter_LProf.Total_OBIS_ui8 = Meter_TempBuff[i+1];
					i += 2;
					break;
				case 0x05: //so co dau
					if(LANDIS_MLProfile_OBIS_List[Get_Meter_LProf.OBIS_Pointer_ui8]==0xFF)
					{
						i+=5;
						Get_Meter_LProf.OBIS_Pointer_ui8++;
					}
					else
					{    
                        sign =1;
						ObisHex = LANDIS_MLProfile_OBIS_List[Get_Meter_LProf.OBIS_Pointer_ui8++];
						Length = 0x04;
						i++;
						for (j=0;j<4;j++)
							Temp_data_32 = Temp_data_32 <<8 | Meter_TempBuff[i++];
					}
					Get_Meter_LProf.Num_OBIS_ui8++;
					break;
				case 0x06:  //so khong dau
					if(LANDIS_MLProfile_OBIS_List[Get_Meter_LProf.OBIS_Pointer_ui8]==0xFF)
					{
						i+=5;
						Get_Meter_LProf.OBIS_Pointer_ui8++;
					}
					else
					{
						ObisHex = LANDIS_MLProfile_OBIS_List[Get_Meter_LProf.OBIS_Pointer_ui8++];
						Length = 0x04;
						i++;
						for (j=0;j<4;j++)
							Temp_data_32 = Temp_data_32 <<8 | Meter_TempBuff[i++];
					}
					Get_Meter_LProf.Num_OBIS_ui8++;
					break;
				case 0x09:
					if(LANDIS_MLProfile_OBIS_List[Get_Meter_LProf.OBIS_Pointer_ui8]==0xFF)
					{
						i+=2+Meter_TempBuff[i+1];
						Get_Meter_LProf.OBIS_Pointer_ui8++;
					}
					else
					{	
						ObisHex = LANDIS_MLProfile_OBIS_List[Get_Meter_LProf.OBIS_Pointer_ui8++];
						i++;
						Length = Meter_TempBuff[i++];
						for (j=0;j<Length;j++)
                            BuffTemp[j] = Meter_TempBuff[i++];
                        
                        Temp_data_16     = (BuffTemp[0] << 8) | (BuffTemp[1]);
                        sTime_temp.year  = Temp_data_16%100;
                        sTime_temp.month = BuffTemp[2];
                        sTime_temp.date  = BuffTemp[3];
                        sTime_temp.day   = BuffTemp[4];
                        sTime_temp.hour  = BuffTemp[5];
                        sTime_temp.min   = BuffTemp[6];
                        sTime_temp.sec   = BuffTemp[7];
					}
					Get_Meter_LProf.Num_OBIS_ui8++;
					break;
				case 0x14:
					if(LANDIS_MLProfile_OBIS_List[Get_Meter_LProf.OBIS_Pointer_ui8]==0xFF)
					{
						i+=9;
						Get_Meter_LProf.OBIS_Pointer_ui8++;
					}
					else
					{
						ObisHex = LANDIS_MLProfile_OBIS_List[Get_Meter_LProf.OBIS_Pointer_ui8++];
						Length = 0x08;
						i++;
						for (j=0;j<8;j++)
							Temp_data_64 = Temp_data_64 <<8 | Meter_TempBuff[i++];
					}
					Get_Meter_LProf.Num_OBIS_ui8++;
					break;
				default:
                    i += Length;
					Get_Meter_LProf.Error_ui8 = 0x10;
					break;
			}
			if(Get_Meter_LProf.Error_ui8 == 0x10)
				break;
            
            //convert lai so lieu
            //kiem tra obis xem co can lay khong. Neu co dong goi sang 1 buffer khac
            RowObis103 = LANDIS_Check_Row_Obis103(&Obis_Landis_Ins[0], ObisHex);
            if((RowObis103 >=0) && (RowObis103 < (MAX_OBIS_INTAN - 2)))
            {
                if(Obis_Landis_Ins[RowObis103].StrObis_Lpf != NULL)
                {
                    Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Obis_Inbuff, Obis_Landis_Ins[RowObis103].StrObis_Lpf);
                    Get_Meter_LProf.Pos_Obis_Inbuff += Obis_Landis_Ins[RowObis103].StrObis_Lpf->Length_u16;
                    Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
                    Get_Meter_LProf.Numqty++;
                    
                    if(((ObisHex >= 0x02) && (ObisHex <= 0x09)) || ((ObisHex >= 0x1B) && (ObisHex <= 0x1E)))  //dien ap dong dien, tan so, pf thi k scale unit nen lay gia tri nho nhat
                        ScaleRead = ConvertScaleMeter_toDec (Obis_Landis_Ins[RowObis103].Scale_ReadMeter + Obis_Landis_Ins[RowObis103].scale, &Div);
                    else ScaleRead = ConvertScaleMeter_toDec (Obis_Landis_Ins[RowObis103].Scale_ReadMeter, &Div);  //de nhu the nay la ve gia tri thuc. voi don vi nho nhat. TI nua scale theo unit can

                    switch(Length)
                    {
                        case 4:  //
                            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                            if(sign == 0)
                                Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) (Temp_data_32*ScaleRead / Div), Obis_Landis_Ins[RowObis103].scale);  //fix length 1000
                            else Pack_HEXData_Frame(&Str_Data_Write, (int64_t) Convert_uint_2int (Temp_data_32* ScaleRead / Div), Obis_Landis_Ins[RowObis103].scale);  //fix length 1000

                            if(Obis_Landis_Ins[RowObis103].Str_Unit != NULL)
                            {
                                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                                Copy_String_2(&Str_Data_Write, Obis_Landis_Ins[RowObis103].Str_Unit);  //don vi
                            }
                            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';  
                            break;
                        case 8:   //gia tri nang luong. 
                            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                            Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) (Temp_data_64*ScaleRead / Div), Obis_Landis_Ins[RowObis103].scale);
                            if(Obis_Landis_Ins[RowObis103].Str_Unit != NULL)
                            {
                                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                                Copy_String_2(&Str_Data_Write, Obis_Landis_Ins[RowObis103].Str_Unit);  //don vi
                            }
                            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                            break;
                        default: 
                            break;
                    }
                    Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Data_Inbuff, &Str_Data_Write);
                    Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
                }
            }else if(ObisHex == 0x01)
            {
                //tinh lai thoi gian period
                if(LANDISStimelpfSecond == 0)
                {
                    PeriodLpf_Min = 30;
                    sTimeSecond = HW_RTC_GetCalendarValue_Second(sTime_temp, 1);
                    LANDISStimelpfSecond = sTimeSecond;
                }
                else 
                {
                    sTimeSecond = HW_RTC_GetCalendarValue_Second(sTime_temp, 1);
                    PeriodLpf_Min = (sTimeSecond - LANDISStimelpfSecond)/60;
                    LANDISStimelpfSecond = sTimeSecond;
                }
                    
                Reset_Buff(&StrSTime_Bill);
                //luu vao 1 buff rieng de sau do moi dong goi sau cung
                *(StrSTime_Bill.Data_a8 + StrSTime_Bill.Length_u16++) = '('; 
                Copy_String_STime(&StrSTime_Bill,sTime_temp);
                *(StrSTime_Bill.Data_a8 + StrSTime_Bill.Length_u16++) = ')';
            }
        
        
			if(Get_Meter_LProf.Num_OBIS_ui8>=Get_Meter_LProf.Total_OBIS_ui8) // Send Data
			{
				Get_Meter_LProf.OBIS_Pointer_ui8 = 0;
				Get_Meter_LProf.Num_OBIS_ui8 = 0;
                //Them Tu TI
                Add_TuTI_toPayload(&Get_Meter_LProf);
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
                
                Send_MessLpf_toQueue();
                Get_Meter_LProf.Flag_Start_Pack = 0;
                LAND_Fl_Alow_GetTimeLpf = 0;
			}
//            else
//            {
//                LAND_Fl_Alow_GetTimeLpf = 1;
//                *(Get_Meter_LProf.Str_Payload.Data_a8 + Get_Meter_LProf.Str_Payload.Length_u16++) = 0x0D;
//                *(Get_Meter_LProf.Str_Payload.Data_a8 + Get_Meter_LProf.Str_Payload.Length_u16++) = 0x0A;
//                Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
//            }
		}
	}
}

                                                                                                         
	                                                                                                             
void LANDIS_MEventGetRawData (uint8_t Mess_Type)
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
					data_pos = 12;
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



void LANDIS_Get_ShortName(void)  // Extra data Get Obis
{
	int i, j;
	uint8_t Temp[6];
	if (UART1_Receive_Buff[1] == 0xA0)  // chua het data
		LANDIS_GetSN_Code_Table[Get_Meter_ShortName.Mess_Step_ui8+1] = 0xFF;  // Out
	for (i = 0; i < UART1_Receive_Buff[2]; )
    {  // lay data den het length nhan dc
		if (UART1_Receive_Buff[i] == 0x02 && UART1_Receive_Buff[i+1] == 0x04 && UART1_Receive_Buff[i+2] == 0x10 )
        {
			if (UART1_Receive_Buff[i+6] == 0x00)
            {  // co dinh nghia SN
				for (j = 0; j < 6; j++)
					Temp[j] = UART1_Receive_Buff[i+j+12];  // lay Obis
				for (j = 0; j < 56; j++)
                {  // kiem tra Obis vua nhan dc
					if (LANDIS_Info_SN_Table[j][0] == 0 && LANDIS_Info_SN_Table[j][1] == 0)
                    {  // truong hop chua lay SN
						if (Compare_Array(Temp,(uint8_t*)LANDIS_OBIS_Table[j],6) == 1)
                        {  // so sanh 2 mang Obis
							LANDIS_Info_SN_Table[j][0] = LANDIS_OBIS_Table[j][6];  // lay Obis
							LANDIS_Info_SN_Table[j][1] = UART1_Receive_Buff[i+3];   // Lay SN
							LANDIS_Info_SN_Table[j][2] = UART1_Receive_Buff[i+4]; 	// Lay SN
							if((j > 25)&&(j<31)) // Them Obis cho cac thong so Maxdemand
							{
								LANDIS_Info_SN_Table[j+5][0] = LANDIS_OBIS_Table[j+5][6];
								LANDIS_Info_SN_Table[j+5][1] = UART1_Receive_Buff[i+3];  
								LANDIS_Info_SN_Table[j+5][2] = UART1_Receive_Buff[i+4];
							}
							break;
						}
					}
				}
				for (j = 0; j < 6; j++){
					if (LANDIS_Event_SN_Table[j][0] == 0 && LANDIS_Event_SN_Table[j][1] == 0){
						if (Compare_Array(Temp,(uint8_t*)LANDIS_MEvent_Table[j],6) == 1){
							LANDIS_Event_SN_Table[j][0] = LANDIS_MEvent_Table[j][6];
							LANDIS_Event_SN_Table[j][1] = UART1_Receive_Buff[i+3];
							LANDIS_Event_SN_Table[j][2] = UART1_Receive_Buff[i+4];
							break;
						}
					}
				}
				for (j = 0; j < 2; j++){
					if (LANDIS_Profile_Historical_SN_Table[j][0] == 0 && LANDIS_Profile_Historical_SN_Table[j][1] == 0){
						if (Compare_Array(Temp,(uint8_t*)LANDIS_OBIS_Table[j],6) == 1){
							LANDIS_Profile_Historical_SN_Table[j][0] = UART1_Receive_Buff[i+3];
							LANDIS_Profile_Historical_SN_Table[j][1] = UART1_Receive_Buff[i+4];
							break;
						}
					}
				}
			}
			i = i + 18;
		} else i++;
	}
	Init_UART2();
}

uint8_t Compare_Array(uint8_t *Array1, uint8_t *Array2, uint8_t length)
{
	uint8_t i, result = 1;
	for (i = 0; i < length; i++)
		if (Array1[i] != Array2[i]){
			result = 0;
			break;
		}
	return result;	
}


void MGetShortNameData(uint8_t Mess_Type)  // ham xu li ghep data de lay ShortName
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
				if (UART1_Receive_Buff[16] == 0x01)
				{
					data_pos = 16;  // Xac dinh so Obis
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
			Meter_TempBuff[Get_Meter_Billing.Data_Buff_Pointer_ui16++] = UART1_Receive_Buff[i+8];  // Start byte 8
}


uint8_t LANDIS_Get_Meter_ID(uint32_t ValueTemp)
{
	uint8_t ID_Length = 0,index = 0, Temp=0;
	uint8_t Temp_MeterID[METER_LENGTH] = {0};
	uint16_t TempCheckSum = 0, TempSN=0;
    
	if(LANDIS_HANDSHAKE()==1)
	{
		LANDIS_MInfo_MTemp[5] = 0x32;//Insert ID Frame
		TempCheckSum = CountFCS16(&LANDIS_MInfo_MTemp[0],1,5);
		LANDIS_MInfo_MTemp[6] = (uint8_t)((TempCheckSum>>8)&0x00FF);
		LANDIS_MInfo_MTemp[7] = (uint8_t)(TempCheckSum&0x00FF);
		LANDIS_MInfo_MTemp[14] = LANDIS_Info_SN_Table[0][1];// Get index SN ID Meter
		LANDIS_MInfo_MTemp[15] = LANDIS_Info_SN_Table[0][2];
		TempSN = LANDIS_MInfo_MTemp[14]<<8;
		TempSN = TempSN + LANDIS_MInfo_MTemp[15];
		TempSN = TempSN + 0x08; // Get value
		LANDIS_MInfo_MTemp[14] = (uint8_t)(TempSN>>8);
		LANDIS_MInfo_MTemp[15] = (uint8_t)TempSN;
		//Calculate Total checksum
		TempCheckSum = CountFCS16(&LANDIS_MInfo_MTemp[0],1,15);
		//Insert Total checksum
		LANDIS_MInfo_MTemp[16] = (uint8_t)((TempCheckSum>>8)&0x00FF);
		LANDIS_MInfo_MTemp[17] = (uint8_t)(TempCheckSum&0x00FF);
		// Send
		if(Meter_Type == 1) RS485_SEND;
		osDelay(LANDIS_Meter_Delay_ms);
		HAL_UART_Transmit(&UART_METER,&LANDIS_MInfo_MTemp[0],19,1000);
		if(Meter_Type == 1) RS485_RECIEVE;
		// Check Data
		if(osSemaphoreWait(bsUART2PendingMessHandle,5000) == osOK){
			if(UART1_Receive_Buff[14]==0x09){  // Xac dinh Type data
				for (index=0;index<UART1_Receive_Buff[15];index++){
					if((UART1_Receive_Buff[19+index]<0x30)||(UART1_Receive_Buff[19+index]>0x39)) // Bo qua 3 byte Type Meter
						break;
					Temp_MeterID[index] = UART1_Receive_Buff[19+index];
					ID_Length++;
				}
				if (sDCU.sMeter_id_now.Length_u16 != ID_Length)
					Read_Meter_ID_Change = 1;
				for (index=0;index<ID_Length;index++)
				{
					if ((*(sDCU.sMeter_id_now.Data_a8+index)) != Temp_MeterID[index])
					{
						Read_Meter_ID_Change = 1;
						break;
					}
				}
				if (Read_Meter_ID_Change == 1)
				{
					sDCU.sMeter_id_now.Length_u16 = ID_Length;
					for (index=0;index<ID_Length;index++)
						aMeter_ID[index] =  Temp_MeterID[index];
//					Save_Meter_ID();
				}
				Temp = 1;
			}
		}
	}
	return Temp;
}

void Fill_General_Mess (uint8_t option)  // Load profile
{
	uint16_t	TempCheckSum = 0;
	
	//Insert ID Frame
	LANDIS_MInfo_MTemp[5] = ID_Frame_ui8;
	//Calculate Header checksum
	TempCheckSum = CountFCS16(&LANDIS_MInfo_MTemp[0],1,5);
	//Insert Header checksum
	LANDIS_MInfo_MTemp[6] = (uint8_t)((TempCheckSum>>8)&0x00FF);
	LANDIS_MInfo_MTemp[7] = (uint8_t)(TempCheckSum&0x00FF);
	//Insert Class ID + OBIS + Data Type
	TempCheckSum = LANDIS_Profile_Historical_SN_Table[option][0];
	TempCheckSum = (TempCheckSum << 8) + LANDIS_Profile_Historical_SN_Table[option][1] + 0x10;
	LANDIS_MInfo_MTemp[14] = (uint8_t)((TempCheckSum>>8)&0x00FF);
	LANDIS_MInfo_MTemp[15] = (uint8_t)(TempCheckSum&0x00FF);
	//Calculate Total checksum
	TempCheckSum = CountFCS16(&LANDIS_MInfo_MTemp[0],1,15);
	//Insert Total checksum
	LANDIS_MInfo_MTemp[16] = (uint8_t)((TempCheckSum>>8)&0x00FF);
	LANDIS_MInfo_MTemp[17] = (uint8_t)(TempCheckSum&0x00FF);
	//Calculate next ID Frame
	ID_Frame_ui8 += 0x22;
	if (ID_Frame_ui8 == 0x20)
		ID_Frame_ui8 = 0x10;
}

uint8_t LANDIS_Read(Meter_Comm_Struct *Meter_Comm,void (*FuncExtractRawData)(void),void (*FuncDataHandle)(void),void (*FuncFillMess)(void),uint8_t *CodeTable)
{
	uint8_t mReVal = 0;
	// Init
	Meter_Comm->Step_ui8 = 1;
	Meter_Comm->Mess_Step_ui8 = 0;
	Meter_Comm->Reading_ui8 = 1;
	Meter_Comm->Error_ui8 = 0;
	//Start read
	if(LANDIS_HANDSHAKE()==1)
    {
		while(Meter_Comm->Reading_ui8 == 1) 
        { //
			switch(Meter_Comm->Step_ui8) 
            {
				case 0:
					Meter_Comm->Total_Mess_Sent_ui32++;
					if (osSemaphoreWait(bsUART2PendingMessHandle,5000) == osOK) 
                    {
						LANDIS_Mess_Checksum();
						if (UART1_Control.Mess_Status_ui8 == 2) 
                        {
							Meter_Comm->Success_Read_Mess_ui32++;
							FuncExtractRawData();  // xu li data nhan dc
							Meter_Comm->Mess_Step_ui8++; // Next message
							if (*(CodeTable+Meter_Comm->Mess_Step_ui8) == 0xFF)  // Tao Code Table cho ban tin Coll Obis
                            {
								mReVal = 1;
								FuncDataHandle();// Send Data
								Meter_Comm->Step_ui8 = 2;  // End
							}
							else {
								Meter_Comm->Step_ui8 = 1;  // Send next message
							}
						} else {//Meter respond wrong
							Meter_Comm->Error_Wrong_Mess_Format_ui32++;
							Meter_Comm->Step_ui8 = 2;
						}
					} else {//Meter no respond 
						Meter_Comm->Error_Meter_Norespond_ui32++;
						Meter_Comm->Step_ui8 = 2;
                        HAL_UART_Init(&UART_METER);
						Init_UART2();
						__HAL_UART_ENABLE_IT(&UART_METER, UART_IT_RXNE);
						osDelay(1000);
					}
					break;
				case 1:
					Init_UART2();
					FuncFillMess();
					Meter_Comm->Step_ui8 = 0;
					break;
				case 2:
					Meter_Comm->Reading_ui8 = 0;
					// Vs Landis 485 thi co End, Landis 232 thi ko
					if(Meter_Type == 1)
					{	
						RS485_SEND;
						osDelay(LANDIS_Meter_Delay_ms); // End message
						HAL_UART_Transmit(&UART_METER,(uint8_t*)&LANDIS_Read_END[0],9,1000);
						RS485_RECIEVE;
					}
					Meter_Comm->Data_Buff_Pointer_ui16 = 10; //Reset pointer
					osDelay(5000);
					break;			
				default:
					break;
			}
		}
	}
	return mReVal;
}


uint8_t LANDIS_Get_OBIS(Meter_Comm_Struct *Meter_Comm)
{
	int i, j;
	uint8_t Temp[6], next_frame = 0;
	if (UART1_Receive_Buff[1] == 0xA8)
		next_frame = 1;
	else
		next_frame = 0;
	for (i = 0; i < UART1_Receive_Buff[2]; ){
		if (UART1_Receive_Buff[i] == 0x02 && UART1_Receive_Buff[i+1] == 0x04 && UART1_Receive_Buff[i+2] == 0x12 ){
			for (j = 0; j < 6; j++)
				Temp[j] = UART1_Receive_Buff[i+j+7];
			
			for (j = 0; j < 56; j++){
				if (Compare_Array(Temp,(uint8_t*)LANDIS_OBIS_Table[j],6) == 1){
					LANDIS_OBIS_Order[Meter_Comm->Num_OBIS_ui8] = LANDIS_OBIS_Table[j][6];
					break;
				}
			}
			Meter_Comm->Num_OBIS_ui8 ++;
			i = i + 18;
		} else i++;
	}
	return next_frame;
}

uint8_t LANDIS_Extract_Data(Meter_Comm_Struct *Meter_Comm,void (*FuncDataHandle)(void),uint8_t *Buff, uint8_t option, uint8_t *count, uint8_t No_Obis)
{
	int i, j;
	uint8_t Temp, next_frame = 0;
	if (UART1_Receive_Buff[1] == 0xA8)
		next_frame = 1;
	else
		next_frame = 0;
	if (option == 0) Temp = 16;
	else Temp = 8;
	for (i = Temp; i < UART1_Receive_Buff[2] - 3;){
		switch(UART1_Receive_Buff[i]){
			case 0x02:
				OBIS_Order = Meter_Comm->Data_Buff_Pointer_ui16;
				Meter_Comm->Data_Buff_Pointer_ui16++;
				j = 2;
				break;
			case 0x09:
				if (LANDIS_OBIS_Order[*count] != 0){
					Buff[Meter_Comm->Data_Buff_Pointer_ui16++] = LANDIS_OBIS_Order[*count];
					Buff[Meter_Comm->Data_Buff_Pointer_ui16++] = 0x08;
					for (j = 0; j < 8; j++)
						Buff[Meter_Comm->Data_Buff_Pointer_ui16++] = UART1_Receive_Buff[i+2+j];
				}
				j = 14;
				(*count)++;
				break;
			case 0x14:
				if (LANDIS_OBIS_Order[*count] != 0){
					Buff[Meter_Comm->Data_Buff_Pointer_ui16++] = LANDIS_OBIS_Order[*count];
					Buff[Meter_Comm->Data_Buff_Pointer_ui16++] = 0x08;
					for (j = 0; j < 8; j++)
						Buff[Meter_Comm->Data_Buff_Pointer_ui16++] = UART1_Receive_Buff[i+1+j];
				}
				j = 9;
				(*count)++;
				break;
			case 0x05:
				if (LANDIS_OBIS_Order[*count] != 0){
					Buff[Meter_Comm->Data_Buff_Pointer_ui16++] = LANDIS_OBIS_Order[*count];
					Buff[Meter_Comm->Data_Buff_Pointer_ui16++] = 0x04;
					for (j = 0; j < 4; j++)
						Buff[Meter_Comm->Data_Buff_Pointer_ui16++] = UART1_Receive_Buff[i+1+j];
				}
				j = 5;
				(*count)++;
				break;
			case 0x06:
				if (LANDIS_OBIS_Order[*count] != 0){
					Buff[Meter_Comm->Data_Buff_Pointer_ui16++] = LANDIS_OBIS_Order[*count];
					Buff[Meter_Comm->Data_Buff_Pointer_ui16++] = 0x04;
					for (j = 0; j < 4; j++)
						Buff[Meter_Comm->Data_Buff_Pointer_ui16++] = UART1_Receive_Buff[i+1+j];
				}
				j = 5;
				(*count)++;
				break;
			default: 
				break;
		}
		if (*count >= Meter_Comm->Num_OBIS_ui8) {
			*count = 0;
			Buff[OBIS_Order] = (uint8_t)(Meter_Comm->Data_Buff_Pointer_ui16 - OBIS_Order - 1);
			Meter_Comm->Num_Block_ui8 ++;
			if (Meter_Comm->Num_Block_ui8 >= 14){
				FuncDataHandle();
				Meter_Comm->Num_Block_ui8 = 0;
			}
		}
		i = i + j;
	}
	return next_frame;
}
void LANDIS_MS_ExtractDataSN(void)
{
//	uint8_t j=0;
//	
//	j = LANDIS_GetScale_Code_Table[Get_Meter_Scale.Mess_Step_ui8-5];
//	if (UART1_Receive_Buff[14] == 0x02 && UART1_Receive_Buff[16] == 0x0F)  // chua chinh xac - bo
//	{
//		MeterScaleDataBuff[Get_Meter_Scale.Data_Buff_Pointer_ui16++] = LANDIS_OBIS_Table[j][6];
//		MeterScaleDataBuff[Get_Meter_Scale.Data_Buff_Pointer_ui16++] = 0x01;
//		MeterScaleDataBuff[Get_Meter_Scale.Data_Buff_Pointer_ui16++] = UART1_Receive_Buff[17];
//	}
}

uint8_t LANDIS_HANDSHAKE(void)
{
	Get_Meter_Info.Error_Meter_Norespond_ui32 = 0;
	uint8_t status =0, retry =0;
    uint8_t step_handshake=0;
	// Init handshake
	while(retry<3)
    {
		switch(step_handshake)
		{
			case 0:
                step_handshake++;
				Init_UART2();
				if(Meter_Type == 1)
					if(Meter_Type == 1) 
                      RS485_SEND;
				osDelay(LANDIS_Meter_Delay_ms);
				HAL_UART_Transmit(&UART_METER,(uint8_t*)&LANDIS_SNRM[0],34,2000);
				if(Meter_Type == 1)
					if(Meter_Type == 1) 
                        RS485_RECIEVE;
                break;
			case 1:
				if(osSemaphoreWait(bsUART2PendingMessHandle,3000) == osOK)
                {
					if(UART1_Receive_Buff[5] == 0x73)
						step_handshake++;
					else
						step_handshake = 4;
					Init_UART2();
				}
				else
                {
                    step_handshake = 4;
                    Get_Meter_Info.Error_Meter_Norespond_ui32++; // Bao WarDCU
                }
				break;
			case 2:
				Init_UART2();
				if(Meter_Type == 1)
					if(Meter_Type == 1) RS485_SEND;
				osDelay(LANDIS_Meter_Delay_ms);
				HAL_UART_Transmit(&UART_METER,(uint8_t*)&LANDIS_AARQ[0],48,2000);
				if(Meter_Type == 1)
					if(Meter_Type == 1) RS485_RECIEVE;
				step_handshake++;
                break;
			case 3:
				if(osSemaphoreWait(bsUART2PendingMessHandle,3000) == osOK)
                {
					if(UART1_Receive_Buff[5] == 0x30)
						status = 1;
					Init_UART2();
					step_handshake++;
				}
				else
				{
					step_handshake = 4;
					Get_Meter_Info.Error_Meter_Norespond_ui32++;	// Bao WarDCU
				}
				break;
			case 4:
				if(status==1)
                {
					retry=3;  // out;
					break;
				}else
                {
                    retry++;
                    step_handshake = 0;
				}
				break;
		}
	}
	return status;
}

// Collecting OBIS Message
void Fill_Nextframe(void)
{
//	LANDIS_GetSN_Code_Table[299] = 0xFF;  // Thua???
	uint16_t Temp_CheckSum=0;
	if(Meter_Type == 1) RS485_SEND;
	osDelay(LANDIS_Meter_Delay_ms);
	if (Get_Meter_ShortName.Mess_Step_ui8 == 0)
    {
		HAL_UART_Transmit(&UART_METER,(uint8_t*)&LANDIS_GET_SHORTNAME[0],19,1000);
		LANDIS_NextFrame[5] = 0x31;
	}
	else
	{
		LANDIS_NextFrame[5] += 0x20;
		Temp_CheckSum = CountFCS16(&LANDIS_NextFrame[0],1,5);
		LANDIS_NextFrame[6] = (uint8_t)((Temp_CheckSum>>8)&0x00FF);
		LANDIS_NextFrame[7] = (uint8_t)(Temp_CheckSum&0x00FF);
		HAL_UART_Transmit(&UART_METER,&LANDIS_NextFrame[0],9,1000);
	}
	if(Meter_Type == 1) RS485_RECIEVE;
}
// Send Obis data - Not Send
void OBIS_SendData(void)
{
	//
}
void Fill_frameOBIS_His(void)
{
	uint16_t Temp_CheckSum=0;
	if(Meter_Type == 1) RS485_SEND;
	osDelay(LANDIS_Meter_Delay_ms);
	if (Get_Meter_Billing.Mess_Step_ui8 == 0){
		HAL_UART_Transmit(&UART_METER,LANDIS_MInfo_MTemp,19,1000);
		LANDIS_NextFrame[5] = 0x31;
	}
	else
	{
		LANDIS_NextFrame[5] += 0x20;
		Temp_CheckSum = CountFCS16(&LANDIS_NextFrame[0],1,5);
		LANDIS_NextFrame[6] = (uint8_t)((Temp_CheckSum>>8)&0x00FF);
		LANDIS_NextFrame[7] = (uint8_t)(Temp_CheckSum&0x00FF);
		HAL_UART_Transmit(&UART_METER,&LANDIS_NextFrame[0],9,1000);
	}
	if(Meter_Type == 1) RS485_RECIEVE;
}
void  Fill_frameData_His(void)
{
	uint16_t Temp_CheckSum=0;
	if(Meter_Type == 1) RS485_SEND;
	osDelay(LANDIS_Meter_Delay_ms);
	if (Get_Meter_Billing.Mess_Step_ui8 == 0)
    {
		HAL_UART_Transmit(&UART_METER,LANDIS_Get_DATA_Template,70,1000);
		LANDIS_NextFrame[5] = 0x31;
	}
	else
	{
		LANDIS_NextFrame[5] += 0x20;
		Temp_CheckSum = CountFCS16(&LANDIS_NextFrame[0],1,5);
		LANDIS_NextFrame[6] = (uint8_t)((Temp_CheckSum>>8)&0x00FF);
		LANDIS_NextFrame[7] = (uint8_t)(Temp_CheckSum&0x00FF);
		HAL_UART_Transmit(&UART_METER,&LANDIS_NextFrame[0],9,1000);
	}
	if(Meter_Type == 1) RS485_RECIEVE;
}



void LANDIS_His_Send_Data(void)
{    
    
}



//fix scale 1000.
void Convert_His_To103Pack (void)   
{
    uint32_t	i=10;
	uint8_t     ObisHex = 0;
    uint8_t     Length = 0;
    uint8_t     j = 0; //dem trong byte data
    uint32_t    Temp_data_32;
    uint16_t    Temp_data_16;
    uint64_t    Temp_data_64;
    int8_t          RowObis103 = 0;
    uint8_t         BuffNum[30];
    truct_String    Str_Data_Write={&BuffNum[0], 0};
    uint8_t         BuffTemp[12];
    ST_TIME_FORMAT  sTime_temp;
    uint8_t         Temp_BBC = 0;
    uint64_t        ScaleRead = 1;
    uint64_t        Div = 0;
    
	//dong goi lai theo 103 o day
    if(Get_Meter_Billing.Flag_Start_Pack == 0)
    {
        Reset_Buff(&Get_Meter_Billing.Str_Payload);
        //wrire header TSVH vao
        Write_Header_His_Push103();
        Get_Meter_Billing.Flag_Start_Pack = 1;
        Init_MD_Bill();
    }  
    //bat dau cat data tu form cu sang form 103
    while(i <Get_Meter_Billing.Data_Buff_Pointer_ui16)
    {
        Temp_data_16 = 0;
        Temp_data_32 = 0;
        Temp_data_64 = 0;
        Reset_Buff(&Str_Data_Write);
        
        //Get Obis Hex
        ObisHex = MeterBillingDataBuff_2[i++];
        Length = MeterBillingDataBuff_2[i++];
        switch(Length)
        {
            case 4:  //0x05, 0x06: 
                for(j = 0; j < Length; j++)
                    Temp_data_32 = Temp_data_32 <<8 | MeterBillingDataBuff_2[i++];
                break;
            case 8:  //0x14
                for(j = 0; j < Length; j++)
                    Temp_data_64 = Temp_data_64 << 8 | MeterBillingDataBuff_2[i++];
                break;
            case 0x0C:   //gia tri nang luong. va ca stime cua (meter va maxdemand)   //0x14 0x09
                for(j = 0; j < Length; j++)
                    BuffTemp[j] = MeterBillingDataBuff_2[i++];
                
                Temp_data_16     = (BuffTemp[0] << 8) | (BuffTemp[1]);
                sTime_temp.year  = Temp_data_16%100;
                sTime_temp.month = BuffTemp[2];
                sTime_temp.date  = BuffTemp[3];
                sTime_temp.day   = BuffTemp[4];
                sTime_temp.hour  = BuffTemp[5];
                sTime_temp.min   = BuffTemp[6];
                sTime_temp.sec   = BuffTemp[7];
                break;
            default: 
                i += Length;
                break;
        }
        
        //kiem tra obis xem co can lay khong. Neu co dong goi sang 1 buffer khac
        RowObis103 = LANDIS_Check_Row_Obis103(&Obis_Landis_Ins[0], ObisHex);
        if((RowObis103 >=0) && (RowObis103 < (MAX_OBIS_INTAN - 2)))
        {
            if(Obis_Landis_Ins[RowObis103].StrObis_Bill != NULL)
            {
                if(((ObisHex >= 0x02) && (ObisHex <= 0x09)) || ((ObisHex >= 0x1B) && (ObisHex <= 0x1E)))  //dien ap dong dien, tan so, pf thi k scale unit nen lay gia tri nho nhat
                    ScaleRead = ConvertScaleMeter_toDec (Obis_Landis_Ins[RowObis103].Scale_ReadMeter + Obis_Landis_Ins[RowObis103].scale, &Div);
                else ScaleRead = ConvertScaleMeter_toDec (Obis_Landis_Ins[RowObis103].Scale_ReadMeter, &Div);  //de nhu the nay la ve gia tri thuc. voi don vi nho nhat. TI nua scale theo unit can

                switch(Length)
                {
                    case 4:  //ph�n biet so co dau va khong dau theo obis hex
                        if((ObisHex == 0x31) || (ObisHex == 0x35) || (ObisHex == 0x39))   //value maxdemand
                            MD_Plus_Bill.Value_u32[MD_Plus_Bill.NumValue_MD++] = Temp_data_32;
                        else
                        {
                            Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Obis_Inbuff, Obis_Landis_Ins[RowObis103].StrObis_Bill);
                            Get_Meter_Billing.Pos_Obis_Inbuff += Obis_Landis_Ins[RowObis103].StrObis_Bill->Length_u16;
                            Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
                            Get_Meter_Billing.Numqty++;
                
                            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                            Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) (Temp_data_32 * ScaleRead / Div), Obis_Landis_Ins[RowObis103].scale);
                            if(Obis_Landis_Ins[RowObis103].Str_Unit != NULL)
                            {
                                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                                Copy_String_2(&Str_Data_Write, Obis_Landis_Ins[RowObis103].Str_Unit);  //don vi
                            }
                            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';  
                            
                            Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Data_Inbuff, &Str_Data_Write);
                            Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
                        }
                        break;
                    case 8:   //gia tri nang luong. 
                        if((ObisHex == 0x31) || (ObisHex == 0x35) || (ObisHex == 0x39))   //value maxdemand
                            MD_Plus_Bill.Value_u32[MD_Plus_Bill.NumValue_MD++] = Temp_data_64;
                        else
                        {
                            Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Obis_Inbuff, Obis_Landis_Ins[RowObis103].StrObis_Bill);
                            Get_Meter_Billing.Pos_Obis_Inbuff += Obis_Landis_Ins[RowObis103].StrObis_Bill->Length_u16;
                            Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
                            Get_Meter_Billing.Numqty++;
                            
                            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                            Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) (Temp_data_64 * ScaleRead / Div), Obis_Landis_Ins[RowObis103].scale);
                            if(Obis_Landis_Ins[RowObis103].Str_Unit != NULL)
                            {
                                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                                Copy_String_2(&Str_Data_Write, Obis_Landis_Ins[RowObis103].Str_Unit);  //don vi
                            }
                            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                            
                            Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Data_Inbuff, &Str_Data_Write);
                            Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
                        }
                        break;
                    default: 
                        break;
                }
            }else if((ObisHex == 0x32) || (ObisHex == 0x36) || (ObisHex == 0x3A))
            {
                MD_Plus_Bill.sTime[MD_Plus_Bill.NumTariff].year   = sTime_temp.year;
                MD_Plus_Bill.sTime[MD_Plus_Bill.NumTariff].month  = sTime_temp.month;
                MD_Plus_Bill.sTime[MD_Plus_Bill.NumTariff].date   = sTime_temp.date;
                MD_Plus_Bill.sTime[MD_Plus_Bill.NumTariff].day    = sTime_temp.day;
                MD_Plus_Bill.sTime[MD_Plus_Bill.NumTariff].hour   = sTime_temp.hour;
                MD_Plus_Bill.sTime[MD_Plus_Bill.NumTariff].min    = sTime_temp.min;
                MD_Plus_Bill.sTime[MD_Plus_Bill.NumTariff].sec    = sTime_temp.sec;
                MD_Plus_Bill.NumTariff++;
            }      
        }else if(ObisHex == 0x01)
        {
            Reset_Buff(&StrSTime_Bill);
            //luu vao 1 buff rieng de sau do moi dong goi sau cung
            *(StrSTime_Bill.Data_a8 + StrSTime_Bill.Length_u16++) = '('; 
            Copy_String_STime(&StrSTime_Bill,sTime_temp);
            *(StrSTime_Bill.Data_a8 + StrSTime_Bill.Length_u16++) = ')';
        }
    }
    
    Pack_MD_Landis(&MD_Plus_Bill, 0x31);      
    
	Add_TuTI_toPayload(&Get_Meter_Billing);
    //lay het data moi ghep stime Bill vao
    Reset_Buff(&Str_Data_Write);
    Copy_String_2(&Str_Data_Write, &StrSTime_Bill);  //don vi cua bieu gi�

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
}


void Pack_MD_Landis (Struct_Maxdemand_Value* Struct_MD, uint8_t FirstobisHex)  //chuyen sang hex
{
    uint16_t        i = 0;
    uint8_t         Num_Bieu = 0;
    uint32_t        Temp_data_32 = 0;
    uint8_t         TempObis = 0;
    uint8_t         BuffNum[30];
    truct_String    Str_Data_Write={&BuffNum[0], 0};
    int8_t          Row = 0;
    uint64_t        ScaleRead = 1;
    uint64_t        Div = 0;  
    
    if(Struct_MD->NumTariff > Struct_MD->NumValue_MD) 
        Num_Bieu = Struct_MD->NumValue_MD;
    else Num_Bieu = Struct_MD->NumTariff;
    
    if(Num_Bieu > 3) Num_Bieu = 3;
    Row = LANDIS_Check_Row_Obis103(&Obis_Landis_Ins[0],FirstobisHex);
    
    for(i = 0; i < Num_Bieu; i++)
    {
        Reset_Buff(&Str_Data_Write);
        TempObis = Row + i*4;
        Temp_data_32 = Struct_MD->Value_u32[i];
        
        if(Obis_Landis_Ins[TempObis].StrObis_Bill != NULL)
        {
            Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Obis_Inbuff, Obis_Landis_Ins[TempObis].StrObis_Bill);  
            Get_Meter_Billing.Pos_Obis_Inbuff += Obis_Landis_Ins[TempObis].StrObis_Bill->Length_u16;
            Get_Meter_Billing.Numqty++;
            Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
            ScaleRead = ConvertScaleMeter_toDec (Obis_Landis_Ins[TempObis].Scale_ReadMeter + Obis_Landis_Ins[TempObis].scale, &Div);
            
            //o d�y l� so khong dau
            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
            Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) (Temp_data_32 * ScaleRead / Div), Obis_Landis_Ins[TempObis].scale);
            if(Obis_Landis_Ins[TempObis].Str_Unit != NULL)
            {
                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                Copy_String_2(&Str_Data_Write, Obis_Landis_Ins[TempObis].Str_Unit);  //don vi bieu
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
}

void LANDIS_MBillingInsertReadTime(void)
{
	uint16_t TempYear=0;
    if(sInformation.Flag_Request_Bill == 1)
    {
        TempYear = 2000 + sInformation.StartTime_GetBill.year;
          
        LANDIS_MBilling_StartTime[0] = (uint8_t)((TempYear>>8)&0x00FF);
        LANDIS_MBilling_StartTime[1] = (uint8_t)(TempYear&0x00FF);
        LANDIS_MBilling_StartTime[2] = sInformation.StartTime_GetBill.month;
        LANDIS_MBilling_StartTime[3] = sInformation.StartTime_GetBill.date;
        
        TempYear = 2000 + sInformation.EndTime_GetBill.year;
        LANDIS_MBilling_StopTime[0]  = (uint8_t)((TempYear>>8)&0x00FF);
        LANDIS_MBilling_StopTime[1]  = (uint8_t)(TempYear&0x00FF);
        LANDIS_MBilling_StopTime[2]  = sInformation.EndTime_GetBill.month;
        LANDIS_MBilling_StopTime[3] = sInformation.EndTime_GetBill.date;
    }else
    {
        if (sRTC.month == 1)
        {
            TempYear = 1999 + sRTC.year;
            LANDIS_MBilling_StartTime[0] = (uint8_t)((TempYear>>8)&0x00FF);
            LANDIS_MBilling_StartTime[1] = (uint8_t)(TempYear&0x00FF);
            LANDIS_MBilling_StartTime[2] = 12;
            LANDIS_MBilling_StartTime[3] = sRTC.date;
            TempYear += 1;
            LANDIS_MBilling_StopTime[0]  = (uint8_t)((TempYear>>8)&0x00FF);
            LANDIS_MBilling_StopTime[1]  = (uint8_t)(TempYear&0x00FF);
            LANDIS_MBilling_StopTime[2]  = sRTC.month;
            LANDIS_MBilling_StopTime[3] = sRTC.date;
            
            sInformation.StartTime_GetBill.year = (1999 + sRTC.year)%100;
            sInformation.StartTime_GetBill.month = 12;
            sInformation.StartTime_GetBill.date = sRTC.date;
            Copy_STime_fromsTime(&sInformation.EndTime_GetBill,sRTC.year, sRTC.month, sRTC.date, sRTC. hour, sRTC.min, sRTC.sec);
        }
        else
        {
            TempYear = 2000 + sRTC.year;
            LANDIS_MBilling_StartTime[0] = (uint8_t)((TempYear>>8)&0x00FF);
            LANDIS_MBilling_StartTime[1] = (uint8_t)(TempYear&0x00FF);
            LANDIS_MBilling_StartTime[2] = sRTC.month-1;
            LANDIS_MBilling_StartTime[3] = sRTC.date;
            LANDIS_MBilling_StopTime[0]  = (uint8_t)((TempYear>>8)&0x00FF);
            LANDIS_MBilling_StopTime[1]  = (uint8_t)(TempYear&0x00FF);
            LANDIS_MBilling_StopTime[2]  = sRTC.month;		
            LANDIS_MBilling_StopTime[3]  = sRTC.date;
            
            sInformation.StartTime_GetBill.year  = sRTC.year;
            sInformation.StartTime_GetBill.month = sRTC.month-1;
            sInformation.StartTime_GetBill.date  = sRTC.date;
            Copy_STime_fromsTime(&sInformation.EndTime_GetBill,sRTC.year, sRTC.month, sRTC.date, sRTC. hour, sRTC.min, sRTC.sec);
        }
    }
}

void MLoadpfInsertReadTime(void)
{
    uint16_t        TempYear=0;
    uint32_t        Diffsec = 0;
    uint32_t        SecondStart = 0;
    
    if(sInformation.Flag_Request_lpf == 1)
    {
       //time start
        LANDIS_MLProfile_StartTime[7] = sInformation.StartTime_GetLpf.sec;
        LANDIS_MLProfile_StartTime[6] = sInformation.StartTime_GetLpf.min;
        LANDIS_MLProfile_StartTime[5] = sInformation.StartTime_GetLpf.hour;
        LANDIS_MLProfile_StartTime[4] =((HW_RTC_GetCalendarValue_Second(sInformation.StartTime_GetLpf, 1)/SECONDS_IN_1DAY) + 6)%7 + 1;   //week day
        LANDIS_MLProfile_StartTime[3] = sInformation.StartTime_GetLpf.date;
        LANDIS_MLProfile_StartTime[2] = sInformation.StartTime_GetLpf.month;
        //Year
        TempYear = sInformation.StartTime_GetLpf.year;
        TempYear = TempYear + 2000;
        LANDIS_MLProfile_StartTime[0] = (uint8_t)((TempYear>>8)&0x00FF);
        LANDIS_MLProfile_StartTime[1] = (uint8_t)(TempYear&0x00FF);
        
        Check_DiffTime(sInformation.StartTime_GetLpf, sInformation.EndTime_GetLpf, &Diffsec);
        if(Diffsec > 86400)
        {
            LANDIS_MLProfile_StopTime[7] = 0;       //sInformation.StartTime_GetLpf.sec;
            LANDIS_MLProfile_StopTime[6] = 59;      //sInformation.StartTime_GetLpf.min;
            LANDIS_MLProfile_StopTime[5] = 23;      //sInformation.StartTime_GetLpf.hour;
            LANDIS_MLProfile_StopTime[4] =((HW_RTC_GetCalendarValue_Second(sInformation.StartTime_GetLpf, 1)/SECONDS_IN_1DAY) + 6)%7 + 1;   //week day
            LANDIS_MLProfile_StopTime[3] = sInformation.StartTime_GetLpf.date;
            LANDIS_MLProfile_StopTime[2] = sInformation.StartTime_GetLpf.month;
            TempYear = sInformation.StartTime_GetLpf.year;
            TempYear = TempYear + 2000;
            LANDIS_MLProfile_StopTime[0] = (uint8_t)((TempYear>>8)&0x00FF);
            LANDIS_MLProfile_StopTime[1] = (uint8_t)(TempYear&0x00FF);	
            
            //tang start time l�n 1 ngay
            SecondStart = HW_RTC_GetCalendarValue_Second(sInformation.StartTime_GetLpf, 0);
            SecondStart = SecondStart - SecondStart%86400;      //dua gio phut giay ve 0
            Epoch_to_date_time(&sInformation.StartTime_GetLpf, SecondStart + 86400, 0); //cong them 1 ngay
            
        }else     
        {
            LANDIS_MLProfile_StopTime[7] = sInformation.EndTime_GetLpf.sec;
            LANDIS_MLProfile_StopTime[6] = sInformation.EndTime_GetLpf.min;
            LANDIS_MLProfile_StopTime[5] = sInformation.EndTime_GetLpf.hour;
            LANDIS_MLProfile_StopTime[4] =((HW_RTC_GetCalendarValue_Second(sInformation.EndTime_GetLpf, 1)/SECONDS_IN_1DAY) + 6)%7 + 1;   //week day
            LANDIS_MLProfile_StopTime[3] = sInformation.EndTime_GetLpf.date;
            LANDIS_MLProfile_StopTime[2] = sInformation.EndTime_GetLpf.month;
            TempYear = sInformation.EndTime_GetLpf.year;
            TempYear = TempYear + 2000;
            LANDIS_MLProfile_StopTime[0] = (uint8_t)((TempYear>>8)&0x00FF);
            LANDIS_MLProfile_StopTime[1] = (uint8_t)(TempYear&0x00FF);
            //tang start time bang endtime
            sInformation.StartTime_GetLpf.sec = sInformation.EndTime_GetLpf.sec;
            sInformation.StartTime_GetLpf.min = sInformation.EndTime_GetLpf.min;
            sInformation.StartTime_GetLpf.hour = sInformation.EndTime_GetLpf.hour;
            sInformation.StartTime_GetLpf.date = sInformation.EndTime_GetLpf.date;
            sInformation.StartTime_GetLpf.month = sInformation.EndTime_GetLpf.month;
            sInformation.StartTime_GetLpf.year = sInformation.EndTime_GetLpf.year;
        }
    }else
    {            
        LANDIS_MLProfile_StartTime[7] = 0;
        LANDIS_MLProfile_StartTime[6] = 0;
        LANDIS_MLProfile_StartTime[5] = 0;
        LANDIS_MLProfile_StartTime[4] = Read_Meter_LProf_Day[3];   //week day
        //Insert read day received from server
        LANDIS_MLProfile_StartTime[3] = Read_Meter_LProf_Day[0];
        LANDIS_MLProfile_StartTime[2] = Read_Meter_LProf_Day[1];
        //Year
        TempYear = Read_Meter_LProf_Day[2];
        TempYear = TempYear + 2000;
        LANDIS_MLProfile_StartTime[0] = (uint8_t)((TempYear>>8)&0x00FF);
        LANDIS_MLProfile_StartTime[1] = (uint8_t)(TempYear&0x00FF);
               
        LANDIS_MLProfile_StopTime[7] = 0;
        LANDIS_MLProfile_StopTime[6] = 59;
        LANDIS_MLProfile_StopTime[5] = 23;
        LANDIS_MLProfile_StopTime[4] = Read_Meter_LProf_Day[3];   //week day
        LANDIS_MLProfile_StopTime[3] = Read_Meter_LProf_Day[0];
        LANDIS_MLProfile_StopTime[2] = Read_Meter_LProf_Day[1];
        TempYear = Read_Meter_LProf_Day[2];
        TempYear = TempYear + 2000;
        LANDIS_MLProfile_StopTime[0] = (uint8_t)((TempYear>>8)&0x00FF);
        LANDIS_MLProfile_StopTime[1] = (uint8_t)(TempYear&0x00FF);	
    }
}

void Fill_frameOBIS_LoPro(void)
{
	uint16_t Temp_CheckSum=0;
	if(Meter_Type == 1) RS485_SEND;
	osDelay(LANDIS_Meter_Delay_ms);
	if (Get_Meter_LProf.Mess_Step_ui8 == 0){
		HAL_UART_Transmit(&UART_METER,LANDIS_MInfo_MTemp,19,1000);
		LANDIS_NextFrame[5] = 0x31;
	}
	else
	{
		LANDIS_NextFrame[5] += 0x20;
		Temp_CheckSum = CountFCS16(&LANDIS_NextFrame[0],1,5);
		LANDIS_NextFrame[6] = (uint8_t)((Temp_CheckSum>>8)&0x00FF);
		LANDIS_NextFrame[7] = (uint8_t)(Temp_CheckSum&0x00FF);
		HAL_UART_Transmit(&UART_METER,&LANDIS_NextFrame[0],9,1000);
	}
	if(Meter_Type == 1) RS485_RECIEVE;
}
void LoPro_Send_Data(void)
{

}

void Fill_frameData_Load(void)
{
	uint16_t Temp_CheckSum=0;
	if(Meter_Type == 1) RS485_SEND;
	osDelay(LANDIS_Meter_Delay_ms);
	if (Get_Meter_LProf.Mess_Step_ui8 == 0){
		HAL_UART_Transmit(&UART_METER,LANDIS_Get_DATA_Template,70,1000);
		LANDIS_NextFrame[5] = 0x31;
	}
	else
	{
		LANDIS_NextFrame[5] += 0x20;
		Temp_CheckSum = CountFCS16(&LANDIS_NextFrame[0],1,5);
		LANDIS_NextFrame[6] = (uint8_t)((Temp_CheckSum>>8)&0x00FF);
		LANDIS_NextFrame[7] = (uint8_t)(Temp_CheckSum&0x00FF);
		HAL_UART_Transmit(&UART_METER,&LANDIS_NextFrame[0],9,1000);
	}
	if(Meter_Type == 1) RS485_RECIEVE;
}
//
uint8_t LANDIS_CheckResetReadMeter(uint32_t Timeout)
{
	// Reset neu doc sai cong to
    if ((sDCU.Status_Meter_u8 == 0) &&
            (Check_Time_Out(sDCU.LandMark_Count_Reset_Find_Meter,600000) == TRUE)) // 10p
    {
        osDelay(Timeout); // 5p
        Read_Meter_ID_Success = LANDIS_Get_Meter_ID(0);
        if(Read_Meter_ID_Success != 1) // Reset MCU
        {
            _fPackStringToLog ((uint8_t*) "Check reset meter va reset Chip\r\n", 33);
            osDelay(1000);
            __disable_irq();
            NVIC_SystemReset(); // Reset MCU	
        }
        else
            sDCU.LandMark_Count_Reset_Find_Meter = RT_Count_Systick_u32;
    }
    return 1;
}




void Landis_TuTi_SendData(void) 
{
    uint32_t	i=10;
	uint8_t     ObisHex = 0;
    uint8_t     Length = 0;
    uint8_t     j = 0; //dem trong byte data
    uint16_t    Temp_data_16;

    //bat dau cat data tu form cu sang form 103
    while(i < Get_Meter_TuTi.Data_Buff_Pointer_ui16)
    {
        Temp_data_16 = 0;
        //Get Obis Hex
        ObisHex = Meter_TempBuff[i++];
        Length = Meter_TempBuff[i++];
        switch(Length)
        {
            case 2:   //0x12
                for(j = 0; j < Length; j++)
                    Temp_data_16 = Temp_data_16 <<8 | Meter_TempBuff[i++];
                break;
            default: 
                i += Length;
                break;
        }

        if((ObisHex == 0x41) || (ObisHex == 0x42))
        {
            *(Get_Meter_TuTi.Str_Payload.Data_a8 + Get_Meter_TuTi.Str_Payload.Length_u16++) = '(';
            Pack_HEXData_Frame_Uint64(&Get_Meter_TuTi.Str_Payload, (uint64_t)Temp_data_16, 0);

            *(Get_Meter_TuTi.Str_Payload.Data_a8 + Get_Meter_TuTi.Str_Payload.Length_u16++) = '/';
            *(Get_Meter_TuTi.Str_Payload.Data_a8 + Get_Meter_TuTi.Str_Payload.Length_u16++) = '1';
            *(Get_Meter_TuTi.Str_Payload.Data_a8 + Get_Meter_TuTi.Str_Payload.Length_u16++) = ')';
        }
    }
    
    *(Get_Meter_TuTi.Str_Payload.Data_a8 + Get_Meter_TuTi.Str_Payload.Length_u16++) = '(';
    Pack_HEXData_Frame_Uint64(&Get_Meter_TuTi.Str_Payload, (uint64_t) sDCU.He_So_Nhan, LANDIS_SCALE_HE_SO_NHAN); 
    *(Get_Meter_TuTi.Str_Payload.Data_a8 + Get_Meter_TuTi.Str_Payload.Length_u16++) = ')';
}


uint8_t Check_2_sTime_By_date (ST_TIME_FORMAT sTime1, ST_TIME_FORMAT sTime2, uint8_t* DifDate)
{
    uint32_t date1 = 0;
    uint32_t date2 = 0;
    date2 =  ConvertStime_toDate(sTime2);
    //convert thoi gian
    date1 = ConvertStime_toDate(sTime1);
    
    if(date2 > date1)
    {
        *DifDate = date2 - date1;
        return 1;
    }
    else return 0;
}


uint8_t Check_sTimeBill_withStarttime (ST_TIME_FORMAT sTimeStart, ST_TIME_FORMAT* sTimeStop, truct_String* sTimeJustGet)
{
    ST_TIME_FORMAT  sTime_temp;
    
    uint32_t date1 = 0;
    uint32_t date2 = 0;
    
    //convert thoi gian
    sTime_temp.year  = (*(sTimeJustGet->Data_a8+ 1) - 0x30) * 10 + (*(sTimeJustGet->Data_a8 + 2) - 0x30);
    sTime_temp.month = (*(sTimeJustGet->Data_a8+3) - 0x30) * 10 + (*(sTimeJustGet->Data_a8 + 4) - 0x30);
    sTime_temp.date  = (*(sTimeJustGet->Data_a8 + 5) - 0x30) * 10 + (*(sTimeJustGet->Data_a8 + 6)- 0x30);  
    
    date2 =  ConvertStime_toDate(sTime_temp);
    //convert thoi gian
    date1 = ConvertStime_toDate(sTimeStart);

    if((date2 >= date1) && (sTime_temp.year <= (sTimeStart.year+1)))
    {
        date2 -= 1;
        //convert lai gia tri just get -1 ngay. thanh gia tri stime stop
        Convert_Date_toStime(date2, sTimeStop);
        return 1;
    }
    return 0;
}

uint32_t ConvertStime_toDate (ST_TIME_FORMAT sTime)
{
    uint32_t date = 0;
    uint32_t correction;
    

    date = DIVC( (DAYS_IN_YEAR*3 + DAYS_IN_LEAP_YEAR)* sTime.year , 4);
    correction = ( (sTime.year % 4) == 0 ) ? DAYS_IN_MONTH_CORRECTION_LEAP : DAYS_IN_MONTH_CORRECTION_NORM ;
    date +=( DIVC( (sTime.month-1)*(30+31) ,2 ) - (((correction>> ((sTime.month-1)*2) )&0x3)));
    date += (sTime.date -1);
    
    return date;
}

void Convert_Date_toStime(uint32_t date, ST_TIME_FORMAT* sTime)
{
    unsigned int years;
	unsigned int year;
	unsigned int month;
    
    years = date/(365*4+1)*4; date %= 365*4+1;

    for (year=3; year>0; year--)
    {
        if (date >= days[year][0])
            break;
    }

    for (month=11; month>0; month--)
    {
        if (date >= days[year][month])
            break;
    }

    sTime->year  = years+year;
    sTime->month = month+1;
    sTime->date  = date-days[year][month]+1;
}

int8_t Landis_Check_Obis_event (uint8_t ID_Event)
{
    uint8_t i = 0;
    
    for(i = 0; i< LANDIS_MAX_EVENT; i++)
      if(ID_Event == Obis_landis_Event[i].ID_event)
        return i;
    
    return -1;
}


//   
int8_t LANDIS_Check_Row_Obis103 (struct_Obis_Scale* structObis, uint8_t ObisHex)
{
    uint8_t  i = 0;
    
    for(i = 0; i < MAX_OBIS_INTAN; i++)
      if(structObis[i].ObisHex == ObisHex) return i; 
    
    return -1;
}

uint8_t LANDIS_Check_Meter(void)
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
			Read_Meter_ID_Success = LANDIS_Get_Meter_ID(0);
        }
	} 
    return 1;
}  



