#include "variable.h"
#include "t_mqtt.h"
#include "Elster_Init_Meter.h"
#include "Elster_Read_Meter.h"
#include "pushdata.h"
#include "at_commands.h"

#define UNDER_COMP	0
#define OVER_COMP	1

uint8_t	 			meterPeriod_Mins=0;
uint32_t			meterTimeStamp=0;
ST_TIME_FORMAT 		meterLPTime,*ptrMeterLPTime;


//khia b�o meter type
          

void ELSTER_Init_Function (uint8_t type)
{
    eMeter_20._f_Read_ID            = ELSTER_Get_Meter_ID;
    eMeter_20._f_Check_Reset_Meter  = ELSTER_CheckResetReadMeter;
  
    eMeter_20._f_Connect_Meter      = ELSTER_Connect_Metter_Handle;
    eMeter_20._f_Read_TSVH          = ELSTER_Read_TSVH;
    eMeter_20._f_Read_Bill          = ELSTER_Read_Bill;
    eMeter_20._f_Read_Event         = ELSTER_Read_Event;  
    eMeter_20._f_Read_Lpf           = ELSTER_Read_Lpf;
    eMeter_20._f_Read_InforMeter    = ELSTER_Read_Infor;
    eMeter_20._f_Get_UartData       = ELSTER_GetUART2Data;
    eMeter_20._f_Check_Meter        = ELSTER_Check_Meter;
    eMeter_20._f_Test1Cmd_Respond   = ELSTER_Send1Cmd_Test;
}

uint8_t ELSTER_Send1Cmd_Test (void)
{
	Init_Meter_Handshake();
	Init_UART2();
	UART1_Control.Mode_ui8 = 1;

    Meter_Handshake.Step_ui8++; //check respond lenh dau tien
//    osDelay(ELSTER_Meter_Delay_ms);
    HAL_UART_Transmit(&UART_METER,&ELSTER_Handshake1[0],5,1000);
//    if (osSemaphoreWait(bsUART2PendingMessHandle,2000) == osOK)
//    {
//        osDelay(ELSTER_Meter_Delay_ms);
//        HAL_UART_Transmit(&UART_METER,&ELSTER_END[0],5,1000);
//    }
    
    if(UART1_Control.fRecei_Respond == 1) return 1;
    
    return 0;
}


uint8_t ELSTER_Read_TSVH (uint32_t Type)
{
    uint8_t         Temp_BBC = 0;
    truct_String    Str_Data_Write = {&Buff_Temp1[0], 0};
    
    Init_Meter_TuTi_Struct();
    if(ELSTER_Read(&Get_Meter_TuTi,&ELSTER_TuTi_ExtractDataFunc,&ELSTER_TuTi_SendData,&ELSTER_Send_MTuTi_Mess,&ELSTER_MeterTuTiIDTable[0]) != 1) 
    {
        Init_Meter_TuTi_Struct();
        if(ELSTER_Read(&Get_Meter_TuTi,&ELSTER_TuTi_ExtractDataFunc,&ELSTER_TuTi_SendData,&ELSTER_Send_MTuTi_Mess,&ELSTER_MeterTuTiIDTable[0]) != 1) 
        {
            Init_Meter_TuTi_Struct();
            if(ELSTER_Read(&Get_Meter_TuTi,&ELSTER_TuTi_ExtractDataFunc,&ELSTER_TuTi_SendData,&ELSTER_Send_MTuTi_Mess,&ELSTER_MeterTuTiIDTable[0]) != 1) 
              return 0;
        }
    }
    
    Init_Meter_Info_Struct();
    if(ELSTER_Read(&Get_Meter_Info,&ELSTER_MI_ExtractDataFunc,&ELSTER_MI_SendData,&ELSTER_Send_MInfo_Mess,&ELSTER_MeterInfoMessIDTable[0]) != 1)
    {
        Init_Meter_Info_Struct();
        if(ELSTER_Read(&Get_Meter_Info,&ELSTER_MI_ExtractDataFunc,&ELSTER_MI_SendData,&ELSTER_Send_MInfo_Mess,&ELSTER_MeterInfoMessIDTable[0]) != 1)
        {
            Init_Meter_Info_Struct();
            if(ELSTER_Read(&Get_Meter_Info,&ELSTER_MI_ExtractDataFunc,&ELSTER_MI_SendData,&ELSTER_Send_MInfo_Mess,&ELSTER_MeterInfoMessIDTable[0]) != 1) 
              return 0;
        }
    }
    
    Init_Meter_Event_Struct();
    Get_Meter_Event.Flag_Start_Pack = 1;  //khong cho dong goi header. chi dong goi data roi copy vao TSVH
    ELSTER_Read(&Get_Meter_Event,&ELSTER_ME_ExtractDataFunc,&ELSTER_ME_SendData_TSVH,&Send_MEvent_Mess,&ELSTER_MeterEventMessIDTable[0]);//doc event loi thi van cu gui TSVH.
    
    //them tuti
    Add_TuTI_toPayload(&Get_Meter_Info);
    
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
    Get_Meter_Info.Data_Buff_Pointer_ui16 = 10;	
    
    return 1;
}


uint8_t ELSTER_Read_Bill (void)
{    
    sDCU.FlagHave_BillMes = 0;
    Init_Meter_TuTi_Struct();
    if(ELSTER_Read(&Get_Meter_TuTi,&ELSTER_TuTi_ExtractDataFunc,&ELSTER_TuTi_SendData,&ELSTER_Send_MTuTi_Mess,&ELSTER_MeterTuTiIDTable[0]) != 1) return 0;
    //
    if(sInformation.Flag_Request_Bill == 1)
    { 
        Bill_NumDay = sInformation.IndexStartBill;
        if(Bill_NumDay > MAX_RECORD_BILL) Bill_NumDay = MAX_RECORD_BILL;
    }else 
    {
        Bill_NumDay = 1;
        Copy_STime_fromsTime(&sInformation.EndTime_GetBill, sRTC.year, sRTC.month, sRTC.date, sRTC.hour, sRTC.min, sRTC.sec);
    }
    Init_Meter_Billing_Struct();
    while ((Get_Meter_Billing.IndexRead < Bill_NumDay) && (sInformation.Flag_Stop_ReadBill == 0))
    {
        Get_Meter_Billing.Flag_Start_Pack = 0;
        Get_Meter_Billing.Str_Payload.Data_a8 = &MeterBillingDataBuff[0];
        Get_Meter_Billing.Data_Buff_Pointer_ui16 = 10;
        Get_Meter_Billing.Step_ui8 = 1;
        Get_Meter_Billing.Mess_Step_ui8 = 0;

        for(uint16_t i=0; i<sizeof(ELSTER_MeterBillingMessIDTable)-1; i++)
            ELSTER_MeterBillingMessIDTable[i] = Get_Meter_Billing.IndexRead *14 + i + 1;  //bat dau tu so 1

        ELSTER_MeterBillingMessIDTable[14] = 0xFF;
        
        ELSTER_Read(&Get_Meter_Billing,&MB_ExtractDataFunc,&MB_SendData,&Send_MBilling_Mess,&ELSTER_MeterBillingMessIDTable[0]);
        
        Get_Meter_Billing.IndexRead++;
    }
    if(sDCU.FlagHave_BillMes == 0)
    {
        Packet_Empty_MessHistorical();
    }
    sInformation.Flag_Stop_ReadBill = 0;
    sInformation.Flag_Request_Bill = 0;
    return 1;
}

uint8_t ELSTER_Read_Event (uint32_t TemValue)
{
    Init_Meter_Event_Struct();
    Init_Meter_TuTi_Struct();
    if(ELSTER_Read(&Get_Meter_TuTi,&ELSTER_TuTi_ExtractDataFunc,&ELSTER_TuTi_SendData,&ELSTER_Send_MTuTi_Mess,&ELSTER_MeterTuTiIDTable[0]) == 1)
        return ELSTER_Read(&Get_Meter_Event,&ELSTER_ME_ExtractDataFunc,&ELSTER_ME_SendData,&Send_MEvent_Mess,&ELSTER_MeterEventMessIDTable[0]);
    return 0;
}
uint8_t ELSTER_Read_Lpf (void)
{   
    sDCU.FlagHave_ProfMess = 0;
    
    ptrMeterLPTime = &meterLPTime;   //Iit con tro stime lpf
    
    if(sInformation.Flag_Request_lpf == 1)   //request tu sever xuong
    { 
        RMLP_NumDay = sInformation.IndexStartLpf;
        if(RMLP_NumDay > MAX_RECORD_LPF) 
            RMLP_NumDay = 1;
        sInformation.Flag_Request_lpf = 0;
    }else 
    {
        RMLP_NumDay = 2;
        Copy_STime_fromsTime(&sInformation.EndTime_GetLpf, sRTC.year, sRTC.month, sRTC.date, sRTC.hour, sRTC.min, sRTC.sec);
    }
    ToTal_PackLpf = 2;
    Init_Meter_TuTi_Struct();   //chi vao day moi Init lai Tu Ti
    if(ELSTER_Read(&Get_Meter_TuTi,&ELSTER_TuTi_ExtractDataFunc,&ELSTER_TuTi_SendData,&ELSTER_Send_MTuTi_Mess,&ELSTER_MeterTuTiIDTable[0]) != 1) return 0;
      
    Init_Meter_LProf_Struct();
    Reset_Buff(&StrUartTemp);
    while((Get_Meter_LProf.IndexRead < ToTal_PackLpf) && (sInformation.Flag_Stop_ReadLpf == 0))  //BANG THOI LA DA PHAI THOAT ROI
    {
        Get_Meter_LProf.Data_Buff_Pointer_ui16 = 10;
        Get_Meter_LProf.Step_ui8 = 1;
        Get_Meter_LProf.Mess_Step_ui8 = 0;
        
        // Reset Buff ID load profile
        for(uint16_t i=2; i<sizeof(MeterLProfMessIDTable)-1; i++)   //if(MeterLProfMessIDTable[i] == ToTal_PackLpf) 
            MeterLProfMessIDTable[i] = Get_Meter_LProf.IndexRead + i -1;   //bat dau tu so 1

        MeterLProfMessIDTable[14] = 0xFF;
        
        if(ELSTER_Read(&Get_Meter_LProf,&RMLP_Extract_Data,&MLProf_SendData,&Send_MLoadProfile_Mess,&MeterLProfMessIDTable[0]) == 0) break;
    }
    
    if(sDCU.FlagHave_ProfMess == 0)
    {
        _fSend_Empty_Lpf();
    }
    
    sInformation.Flag_Stop_ReadLpf = 0;
    sInformation.Flag_Request_lpf = 0;
    return 1;
}
uint8_t ELSTER_Read_Infor (void)
{
    Init_Meter_TuTi_Struct();
    if(ELSTER_Read(&Get_Meter_TuTi,&ELSTER_TuTi_ExtractDataFunc,&ELSTER_TuTi_SendData,&ELSTER_Send_MTuTi_Mess,&ELSTER_MeterTuTiIDTable[0]) == 1)
    {
        Pack_PushData_103_Infor_Meter();
        return 1;
    }
    return 0;
}



uint8_t ELSTER_Connect_Metter_Handle(void)
{
	uint16_t i=0;
	uint8_t ReadIDRetry = 1;            // qua 1 lan la bi loi khi sang mat khau moi - chi doc 1 lan
    
    while (ReadIDRetry>0)
    {
        osDelay(500);
//        if (osMutexWait(mtFlashMeterHandle,1000) == osOK)
//		{
            Read_Meter_ID_Success = ELSTER_Get_Meter_ID(0);
            if (Read_Meter_ID_Success == 1)
            {
//                if (osMutexRelease(mtFlashMeterHandle) != osOK)
//                osMutexRelease(mtFlashMeterHandle);
                break;
            }
            else
                osDelay(2000);
            
            if (osMutexRelease(mtFlashMeterHandle) != osOK)
                osMutexRelease(mtFlashMeterHandle);
            
            ReadIDRetry--;
//        }
    }
    if (Read_Meter_ID_Success == 0)   //Reinit uart1  /**** Check lai o day xem con loi khong ***/
    {
        UART_METER.Instance = UART__METER;
        UART_METER.Init.BaudRate = UART2_BAUR_1;
        UART_METER.Init.WordLength = UART2_DATALENGTH;
        UART_METER.Init.StopBits = UART_STOPBITS_1;
        UART_METER.Init.Parity = UART_PARITY_EVEN;
        UART_METER.Init.Mode = UART_MODE_TX_RX;
        UART_METER.Init.HwFlowCtl = UART_HWCONTROL_NONE;
        UART_METER.Init.OverSampling = UART_OVERSAMPLING_16;
        UART_METER.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE ;
        UART_METER.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
        HAL_UART_Init(&UART_METER);
        Init_UART2();
        __HAL_UART_ENABLE_IT(&UART_METER, UART_IT_RXNE);

        ReadIDRetry = 1;
        for(i=0;i<8;i++)
            password[i] = password_backup[i];
        
        while (ReadIDRetry>0)
        {
            osDelay(500);
//            if (osMutexWait(mtFlashMeterHandle,1000) == osOK)
//            {
                Read_Meter_ID_Success = ELSTER_Get_Meter_ID(0);
                
//                if (osMutexRelease(mtFlashMeterHandle) != osOK)
//                    osMutexRelease(mtFlashMeterHandle);
                
                if (Read_Meter_ID_Success == 1)
                    break;
                else
                    osDelay(1000);
                ReadIDRetry--;
//            }
        }
    }
    if (Read_Meter_ID_Success == 1) return 1;  
    else return 0;
}


//---------------------FUNCTION------------------------

void ELSTER_Mess_Checksum(void)
{
	uint8_t	GenCheckSum=0;
	uint16_t	i=0;
	
	for (i=1;i<UART1_Control.Mess_Length_ui16;i++)
		GenCheckSum = GenCheckSum ^ UART1_Receive_Buff[i];
	
	if (GenCheckSum == UART1_Receive_Buff[UART1_Control.Mess_Length_ui16])
		UART1_Control.Mess_Status_ui8 = 2;
	else
		UART1_Control.Mess_Status_ui8 = 4;
}


uint8_t ELSTER_GetUART2Data(void)
{
	uint8_t	temp_recieve = 0;

	switch (UART1_Control.Mode_ui8)
	{
		case 0:
			if (UART1_Control.Mess_Pending_ui8 == 0)
			{
				temp_recieve = UART_METER.Instance->RDR & 0x00FF;
				UART1_Receive_Buff[UART1_Control.Mess_Length_ui16] = temp_recieve;
				
				if (temp_recieve == 0x03)
				{
					UART1_Control.Mess_Pending_ui8 = 2;
					UART1_Control.Mess_Length_ui16++;
				}
				else if (temp_recieve == 0x06)
				{
					UART1_Control.Mess_Length_ui16++;
					UART1_Control.Mess_Pending_ui8 = 1;
					UART1_Control.Mess_Status_ui8 = 1;
//					osSemaphoreRelease(bsUART2PendingMessHandle);
				}
				else
				{
					UART1_Control.Mess_Length_ui16++;
					if (UART1_Control.Mess_Length_ui16 > 254)	
						UART1_Control.Mess_Length_ui16 = 0;
				}
			}
			else if (UART1_Control.Mess_Pending_ui8 == 2)
			{
				temp_recieve = UART_METER.Instance->RDR & 0x00FF;
				UART1_Receive_Buff[UART1_Control.Mess_Length_ui16] = temp_recieve;
				UART1_Control.Mess_Pending_ui8 = 1;
				UART1_Control.Mess_Status_ui8 = 1;
				osSemaphoreRelease(bsUART2PendingMessHandle);
			}
			else
				temp_recieve = UART_METER.Instance->RDR & 0x00FF;
            UART1_Control.fRecei_Respond = 1;
			break;
		case 1:
			if (UART1_Control.Mess_Pending_ui8 != 1)
			{
				temp_recieve = UART_METER.Instance->RDR & 0x00FF;
				UART1_Receive_Buff[UART1_Control.Mess_Length_ui16] = temp_recieve;
				UART1_Control.Mess_Length_ui16++;
				
				switch (Meter_Handshake.Step_ui8)
				{
					case 1:
						if (temp_recieve == 0x0A)
						{
							UART1_Control.Mess_Pending_ui8 = 1;
							UART1_Control.Mess_Status_ui8 = 1;
							osSemaphoreRelease(bsUART2PendingMessHandle);							
						}
						break;
					case 3:
						if (UART1_Control.Mess_Pending_ui8 == 2)
						{
							UART1_Control.Mess_Pending_ui8 = 1;
							UART1_Control.Mess_Status_ui8 = 1;
							osSemaphoreRelease(bsUART2PendingMessHandle);						
						}				
						if (temp_recieve == 0x03)
						{
							UART1_Control.Mess_Pending_ui8 = 2;
						}
						break;
					case 5:
						if (temp_recieve == 0x06)
						{
							UART1_Control.Mess_Pending_ui8 = 1;
							UART1_Control.Mess_Status_ui8 = 1;
							osSemaphoreRelease(bsUART2PendingMessHandle);
						}							
						break;
					default:
						break;
				}
			}
			else
				temp_recieve = UART_METER.Instance->RDR & 0x00FF;
            UART1_Control.fRecei_Respond = 1;
			break;
		default:
			break;
	}
    return 1;
}

uint8_t ELSTER_Handshake_Handle (void)
{
	uint8_t i=0,mReVal=0;
	uint8_t	temp_checksum = 0;

	Init_Meter_Handshake();
	Init_UART2();
	UART1_Control.Mode_ui8 = 1;
	Meter_Handshake.Reading_ui8 = 1;
    
	while (Meter_Handshake.Reading_ui8 == 1)
	{
		switch (Meter_Handshake.Step_ui8)
		{
			case 0:	//Send Handshake 1
				Meter_Handshake.Step_ui8++;
				osDelay(ELSTER_Meter_Delay_ms);
				HAL_UART_Transmit(&UART_METER,&ELSTER_Handshake1[0],5,1000);
				break;
			case 1:
//				if (osSemaphoreWait(bsUART2PendingMessHandle,5000) == osOK)
//				{
					if ((UART1_Receive_Buff[0]==0xAF)&&(UART1_Receive_Buff[21]==0x8D))
						Meter_Handshake.Step_ui8++;
					else
					{
						Meter_Handshake.Step_ui8 = 6;
						Meter_Handshake.Error_ui8 = 1;
					}
//				}
//				else
//				{
//					Meter_Handshake.Step_ui8 = 6;
//					Meter_Handshake.Error_ui8 = 2;
//				}
				break;
			case 2:	//Send Handshake 2
                Init_UART2();
				Meter_Handshake.Step_ui8++;
				osDelay(ELSTER_Meter_Delay_ms);
				HAL_UART_Transmit(&UART_METER,&ELSTER_Handshake2[0],6,1000);	
				break;
			case 3:
//				if (osSemaphoreWait(bsUART2PendingMessHandle,5000) == osOK)
//				{
					UART1_Control.Mess_Length_ui16--;
					ELSTER_Mess_Checksum();
					if (UART1_Control.Mess_Status_ui8 == 2)
						Meter_Handshake.Step_ui8++;
					else
					{
						Meter_Handshake.Step_ui8 = 6;
						Meter_Handshake.Error_ui8 = 3;								
					}
//				}
//				else
//				{
//					Meter_Handshake.Step_ui8 = 6;
//					Meter_Handshake.Error_ui8 = 4;
//				}
				break;
			case 4:	//Encrypt pass
				//Encrypt pass
				for (i=0;i<16;i++)
					ELSTER_Hs_inbuff[i] = UART1_Receive_Buff[i+5];						
				Encryptpass(ELSTER_Hs_inbuff,ELSTER_Hs_outbuff);
				//Create message to send
				for (i=0;i<16;i++)
					ELSTER_Handshake3[i+5] = ELSTER_Hs_outbuff[i];
				for (i=1;i<23;i++)
					temp_checksum = temp_checksum ^ ELSTER_Handshake3[i];
				ELSTER_Handshake3[23] = temp_checksum;
			
                Init_UART2();   //sua code lan truoc o co nay
				//Send message
				Meter_Handshake.Step_ui8++;
				osDelay(ELSTER_Meter_Delay_ms);
				HAL_UART_Transmit(&UART_METER,&ELSTER_Handshake3[0],24,1000);
				break;
			case 5:
				if (osSemaphoreWait(bsUART2PendingMessHandle,5000) == osOK)
					Meter_Handshake.Step_ui8++;
				else
				{
					Meter_Handshake.Step_ui8 = 6;
					Meter_Handshake.Error_ui8 = 5;					
				}
				break;
			case 6:
				Init_UART2();
                UART1_Control.Mode_ui8 = 0;
				Meter_Handshake.Reading_ui8 = 0;
				if (Meter_Handshake.Error_ui8 != 0)
				{
					osDelay(ELSTER_Meter_Delay_ms);
					HAL_UART_Transmit(&UART_METER,&ELSTER_END[0],5,1000);
					mReVal = 0;
				}
				else
					mReVal = 1;
				break;
			default:
				break;		
		}
	}
	return mReVal;
}

void Encryptpass(uint8_t *in, uint8_t *out)
{
	uint8_t i = 0;
	uint8_t inbuffer[8];
	uint8_t outbuffer[8];
	uint8_t password[8] = {0x4D,0x5F,0x4B,0x48,0x5F,0x44,0x4F,0x43};	
	///////////////////////////////////////////////////////////
	for ( i = 0; i < 16; i++)
	{
		switch(in[i])
		{
			case 0x30:
				in[i] = 0x00;
				break;
			case 0xB1:
				in[i] = 0x01;
				break;
			case 0xB2:
				in[i] = 0x02;
				break;
			case 0x33:
				in[i] = 0x03;
				break;
			case 0xB4:
				in[i] = 0x04;
				break;
			case 0x35:
				in[i] = 0x05;
				break;
			case 0x36:
				in[i] = 0x06;
				break;
			case 0xB7:
				in[i] = 0x07;
				break;
			case 0xB8:
				in[i] = 0x08;
				break;
			case 0x39:
				in[i] = 0x09;
				break;
			case 0x41:
				in[i] = 0x0A;
				break;
			case 0x42:
				in[i] = 0x0B;
				break;
			case 0xC3:
				in[i] = 0x0C;
				break;
			case 0x44:
				in[i] = 0x0D;
				break;
			case 0xC5:
				in[i] = 0x0E;
				break;
			case 0xC6:
				in[i] = 0x0F;
				break;
			default:
				break;
		}
	}
	///////////////////// conver 16 byte to 8 byte
	for ( i = 0; i < 8; i++)
	{
		inbuffer[i] = (in[i*2] << 4) | in[i*2 + 1];
		inbuffer[i] = inbuffer[i] ^ password[i];
	}
	///////////////////////////////////////////////////////////
	outbuffer[0] = (uint8_t)(inbuffer[0]+inbuffer[7]);
	for ( i = 1; i < 8; i++)
		outbuffer[i] = (uint8_t)(outbuffer[i - 1]+inbuffer[i]);
	////////////////////// conver 8 byte to 16 byte pass
	for ( i = 0; i < 8; i++)
	{
		out[i*2] = (outbuffer[i] >> 4)&0x0f;
		out[i*2 + 1] = outbuffer[i]&0x0f;
	}
	///////////////////////////////////////////////////////////
	for ( i = 0; i < 16; i++)
	{
		switch(out[i])
		{
			case 0x00:
				out[i] = 0x30;
				break;
			case 0x01:
				out[i] = 0xB1;
				break;
			case 0x02:
				out[i] = 0xB2;
				break;
			case 0x03:
				out[i] = 0x33;
				break;
			case 0x04:
				out[i] = 0xB4;
				break;
			case 0x05:
				out[i] = 0x35;
				break;
			case 0x06:
				out[i] = 0x36;
				break;
			case 0x07:
				out[i] = 0xB7;
				break;
			case 0x08:
				out[i] = 0xB8;
				break;
			case 0x09:
				out[i] = 0x39;
				break;
			case 0x0A:
				out[i] = 0x41;
				break;
			case 0x0B:
				out[i] = 0x42;
				break;
			case 0x0C:
				out[i] = 0xC3;
				break;
			case 0x0D:
				out[i] = 0x44;
				break;
			case 0x0E:
				out[i] = 0xC5;
				break;
			case 0x0F:
				out[i] = 0xC6;
				break;
			default:
				break;
		}
	}
}

void ELSTER_Decode_byte(uint8_t *mBuff,uint8_t startAddr,uint8_t Length)
{
	uint8_t i = 0;
	
	for (i=0;i<Length;i++)
	{
		switch(*(mBuff+startAddr+i))
		{
			case 0x30:
				*(mBuff+startAddr+i) = 0x00;
				break;
			case 0xB1:
				*(mBuff+startAddr+i) = 0x01;
				break;
			case 0xB2:
				*(mBuff+startAddr+i) = 0x02;
				break;
			case 0x33:
				*(mBuff+startAddr+i) = 0x03;
				break;
			case 0xB4:
				*(mBuff+startAddr+i) = 0x04;
				break;
			case 0x35:
				*(mBuff+startAddr+i) = 0x05;
				break;
			case 0x36:
				*(mBuff+startAddr+i) = 0x06;
				break;
			case 0xB7:
				*(mBuff+startAddr+i) = 0x07;
				break;
			case 0xB8:
				*(mBuff+startAddr+i) = 0x08;
				break;
			case 0x39:
				*(mBuff+startAddr+i) = 0x09;
				break;
			case 0x41:
				*(mBuff+startAddr+i) = 0x0A;
				break;
			case 0x42:
				*(mBuff+startAddr+i) = 0x0B;
				break;
			case 0xC3:
				*(mBuff+startAddr+i) = 0x0C;
				break;
			case 0x44:
				*(mBuff+startAddr+i) = 0x0D;
				break;
			case 0xC5:
				*(mBuff+startAddr+i) = 0x0E;
				break;
			case 0xC6:
				*(mBuff+startAddr+i) = 0x0F;
				break;
			default:
				break;
		}		
	}
}

void ELSTER_Encode_byte(uint8_t *mBuff,uint8_t startAddr,uint8_t Length)
{
	uint8_t i = 0;
	
	for (i=0;i<Length;i++)
	{
		switch(*(mBuff+startAddr+i))
		{
			case 0x00:
				*(mBuff+startAddr+i) = 0x30;
				break;
			case 0x01:
				*(mBuff+startAddr+i) = 0xB1;
				break;
			case 0x02:
				*(mBuff+startAddr+i) = 0xB2;
				break;
			case 0x03:
				*(mBuff+startAddr+i) = 0x33;
				break;
			case 0x04:
				*(mBuff+startAddr+i) = 0xB4;
				break;
			case 0x05:
				*(mBuff+startAddr+i) = 0x35;
				break;
			case 0x06:
				*(mBuff+startAddr+i) = 0x36;
				break;
			case 0x07:
				*(mBuff+startAddr+i) = 0xB7;
				break;
			case 0x08:
				*(mBuff+startAddr+i) = 0xB8;
				break;
			case 0x09:
				*(mBuff+startAddr+i) = 0x39;
				break;
			case 0x0A:
				*(mBuff+startAddr+i) = 0x41;
				break;
			case 0x0B:
				*(mBuff+startAddr+i) = 0x42;
				break;
			case 0x0C:
				*(mBuff+startAddr+i) = 0xC3;
				break;
			case 0x0D:
				*(mBuff+startAddr+i) = 0x44;
				break;
			case 0x0E:
				*(mBuff+startAddr+i) = 0xC5;
				break;
			case 0x0F:
				*(mBuff+startAddr+i) = 0xC6;
				break;
			default:
				break;
		}		
	}	
}

uint8_t ELSTER_Read (Meter_Comm_Struct *Meter_Comm,void (*FuncExtractRawData)(void),void (*FuncDataHandle)(void),void (*FuncSendMess)(void),uint8_t *CodeTable)
{
	uint8_t mReVal = 0;
	
	//Start read
	Meter_Comm->Reading_ui8 = 1;
	ELSTER_Handshake_Handle();
    UART1_Control.Mode_ui8 = 0;
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
					if (UART1_Control.Mess_Length_ui16 == 1)
						UART1_Control.Mess_Status_ui8 = 2;
					else
						ELSTER_Mess_Checksum();
					
					if (UART1_Control.Mess_Status_ui8 == 2) 
                    {
						Meter_Comm->Success_Read_Mess_ui32++;
						Meter_Comm->Step_ui8 = 1;
						//Extract raw data
						FuncExtractRawData();
						//Goto next message
						Meter_Comm->Mess_Step_ui8++;
						Meter_Comm->Error_ui8 = 0x00;
						//If all mess sent -> finish
						if (Meter_Comm->Mess_Step_ui8 > 1)
                        {
							if ((*(CodeTable+Meter_Comm->Mess_Step_ui8)) == 0xFF) 
							{
								//Push mess or check alarm
								FuncDataHandle();
								mReVal = 1;
								Meter_Comm->Step_ui8 = 2;
							}
						}
                        //Reset UART2 status
						Init_UART2();
					} else 
                    {
						Meter_Comm->Error_Wrong_Mess_Format_ui32++;
						//Reset UART
						Init_UART2();
						//If Meter respond wrong, jump out of reading cycle
						Meter_Comm->Step_ui8 = 2;
					}
				} else 
                {//If Meter no respond after 2000ms: wait 2000ms - resend the message (3 times) 
					Meter_Comm->Error_Meter_Norespond_ui32++;
					Init_UART2();
					//If Meter norespond, jump out of reading cycle
					Meter_Comm->Step_ui8 = 2;
				}
				break;
			case 1:
				osDelay(ELSTER_Meter_Delay_ms);					
				FuncSendMess();
				Meter_Comm->Step_ui8 = 0;
				break;
			case 2:
				osDelay(ELSTER_Meter_Delay_ms);
				HAL_UART_Transmit(&UART_METER,&ELSTER_END[0],5,1000);
				//Reset UART2 status
				Init_UART2();
				//Jump out of reading cycle, wait for next cycle (after 30mins)
				Meter_Comm->Reading_ui8 = 0;
				Meter_Comm->Step_ui8 = 1;
				Meter_Comm->Mess_Step_ui8 = 0;
				//Reset pointer
				Meter_Comm->Data_Buff_Pointer_ui16 = 10;
				break;					
			default:
				break;				
		}
	}
	return mReVal;	
}

// 
//		Meter ID
//
uint8_t ELSTER_Get_Meter_ID (uint32_t TempValue)
{
	uint8_t GetMeterIDSuccess = 0;
	uint8_t GetMeterIDRetry = 3;
	uint8_t	i=0,j=0;
	uint8_t Temp_MeterID[8];
	
	while (GetMeterIDRetry--)
	{
		if (ELSTER_Handshake_Handle() == 1)
		{
			osDelay(500);
			HAL_UART_Transmit(&UART_METER,&ELSTER_Get_MID[0],16,1000);

			if (osSemaphoreWait(bsUART2PendingMessHandle,5000) == osOK)
			{
				//Check sum
				ELSTER_Mess_Checksum();
				
				if (UART1_Control.Mess_Status_ui8 == 2)
				{
					ELSTER_Decode_byte(UART1_Receive_Buff,19,16);
                    
					for (i=0;i<16;i++)
					{
						Temp_MeterID[j] = UART1_Receive_Buff[19+i]+0x30;
						j++;
						i++;
					}
					if (sDCU.sMeter_id_now.Length_u16 != 8)
						Read_Meter_ID_Change = 1;
					for (i=0;i<8;i++)
					{
						if ((*(sDCU.sMeter_id_now.Data_a8+i)) != Temp_MeterID[i])
						{
							Read_Meter_ID_Change = 1;
							break;
						}
					}

					if (Read_Meter_ID_Change == 1)
					{
						sDCU.sMeter_id_now.Length_u16 = 8;
						for (i = 0; i < METER_LENGTH; i++)
							aMeter_ID[i] =  0;
						for (i=0;i<8;i++)
							aMeter_ID[i] =  Temp_MeterID[i];
//						Save_Meter_ID();
					}
					osDelay(500);
					HAL_UART_Transmit(&UART_METER,&ELSTER_END[0],5,1000);
					Init_UART2();
					GetMeterIDSuccess = 1;
					break;
				}
				else
				{
					osDelay(500);
					HAL_UART_Transmit(&UART_METER,&ELSTER_END[0],5,1000);
					Init_UART2();
				}
			}
			else
			{
				osDelay(500);
				HAL_UART_Transmit(&UART_METER,&ELSTER_END[0],5,1000);
				Init_UART2();
			}
		}
		else
		{
			osDelay(500);
			HAL_UART_Transmit(&UART_METER,&ELSTER_END[0],5,1000);
			Init_UART2();			
		}
        osDelay(5000);
	}
	
	return GetMeterIDSuccess;
}

// 
// 		Meter info function
//

void ELSTER_Send_MInfo_Mess (void)
{
	uint8_t i=0,temp=0;
	uint8_t mMess[22];
	
	temp = ELSTER_MeterInfoMessIDTable[Get_Meter_Info.Mess_Step_ui8];
	for (i=0;i<ELSTER_MInfo_Table[temp][22];i++)
		mMess[i] = ELSTER_MInfo_Table[temp][i];
	HAL_UART_Transmit(&UART_METER,mMess,ELSTER_MInfo_Table[temp][22],1000);
}

void ELSTER_MI_ExtractDataFunc (void)
{
	if ((ELSTER_MeterInfoMessIDTable[Get_Meter_Info.Mess_Step_ui8]==1)||(ELSTER_MeterInfoMessIDTable[Get_Meter_Info.Mess_Step_ui8]>10))
	{
		ELSTER_Decode_byte(UART1_Receive_Buff,2,UART1_Control.Mess_Length_ui16-4);   //chuyen sang 4 bit sau la gi� tri HEX.
		switch (Get_Meter_Info.Mess_Step_ui8)
		{
			case 5://VoltagePhaseA,B,C
				MI_DecodeData_NoSpin( 2,14,Str_Ob_VolA, Unit_Voltage, ELSTER_SCALE_VOLTAGE);      //sua thanh 6
				MI_DecodeData_NoSpin(16,14,Str_Ob_VolB, Unit_Voltage, ELSTER_SCALE_VOLTAGE);
				MI_DecodeData_NoSpin(30,14,Str_Ob_VolC, Unit_Voltage, ELSTER_SCALE_VOLTAGE);
				break;
			case 11://CurrentPhaseA,B,C
				MI_DecodeData_NoSpin( 2,14,Str_Ob_CurA, Unit_Current, ELSTER_SCALE_CURRENT);
				MI_DecodeData_NoSpin(16,14,Str_Ob_CurB, Unit_Current, ELSTER_SCALE_CURRENT);
				MI_DecodeData_NoSpin(30,14,Str_Ob_CurC, Unit_Current, ELSTER_SCALE_CURRENT);
				break;
			case 17://FrequencyPhaseA,B,C
//				MI_DecodeData_NoSpin( 2,14,Str_Ob_Freq, Unit_Freq, ELSTER_SCALE_FREQ);
//				MI_DecodeData_NoSpin(16,14,Str_Ob_Freq, Unit_Freq, ELSTER_SCALE_FREQ);
				MI_DecodeData_NoSpin(30,14,Str_Ob_Freq, Unit_Freq, ELSTER_SCALE_FREQ);
				break;
			case 23://PhaseAnglePhaseA,B,C
//				MI_DecodeData_NoSpin( 2,14,Str_Ob_PhAnglePhB, unit, ELSTER_SCALE_ANGLE_PHASE);
//				MI_DecodeData_NoSpin(16,14,Str_Ob_PhAnglePhB,);
//				MI_DecodeData_NoSpin(30,14,Str_Ob_PhAnglePhC, );				
				break;
			case 29://ActivePowerPhaseA,B,C,Total
				MI_DecodeData_NoSpin( 2,14,Str_Ob_AcPowA, Unit_Active_Intan_Kw, ELSTER_SCALE_ACTIVE_POW);       
				MI_DecodeData_NoSpin(16,14,Str_Ob_AcPowB, Unit_Active_Intan_Kw, ELSTER_SCALE_ACTIVE_POW); 
				MI_DecodeData_NoSpin(30,14,Str_Ob_AcPowC, Unit_Active_Intan_Kw, ELSTER_SCALE_ACTIVE_POW);
				MI_DecodeData_NoSpin(44,14,Str_Ob_AcPowTo, Unit_Active_Intan_Kw, ELSTER_SCALE_ACTIVE_POW);			
				break;
			case 35://ReactivePowerPhaseA,B,C,Total
				MI_DecodeData_NoSpin( 2,14,Str_Ob_RePowA, Unit_Reactive_Intan_Kvar, ELSTER_SCALE_REACTIVE_POW);     
				MI_DecodeData_NoSpin(16,14,Str_Ob_RePowB, Unit_Reactive_Intan_Kvar, ELSTER_SCALE_REACTIVE_POW);
				MI_DecodeData_NoSpin(30,14,Str_Ob_RePowC, Unit_Reactive_Intan_Kvar, ELSTER_SCALE_REACTIVE_POW);
				MI_DecodeData_NoSpin(44,14,Str_Ob_RePowTo, Unit_Reactive_Intan_Kvar, ELSTER_SCALE_REACTIVE_POW);
				break;
			case 41://ApparentPowerPhaseA,B,C,Total
//				MI_DecodeData_NoSpin( 2,14, obis, unit, SCALE_APPRENT_POW);
//				MI_DecodeData_NoSpin(16,14,0x18);
//				MI_DecodeData_NoSpin(30,14,0x19);
//				MI_DecodeData_NoSpin(44,14,0x1A);					
				break;
			case 47://PowerFactorPhaseA,B,C,Total
                MI_DecodeData_NoSpin( 2,14,Str_Ob_PoFacA,  StrNull, ELSTER_SCALE_POW_FACTOR);
				MI_DecodeData_NoSpin(16,14,Str_Ob_PoFacB, StrNull, ELSTER_SCALE_POW_FACTOR);
				MI_DecodeData_NoSpin(30,14,Str_Ob_PoFacC, StrNull, ELSTER_SCALE_POW_FACTOR);
				MI_DecodeData_NoSpin(44,14,Str_Ob_PoFac, StrNull, ELSTER_SCALE_POW_FACTOR);					
				break;
			case 53://PhaseRotation
//				MI_DecodeData_NoSpin( 2,14,0x43);
				break;
			case 54://Cumulative,0xTotal:,0xImportWh,ExportWh,Q1,Q2,Q3,Q4,Vah
				MI_DecodeData_Spin( 2,16,Str_Ob_En_ImportWh, Unit_Active_EnTotal, ELSTER_SCALE_TOTAL_ENERGY);    
				MI_DecodeData_Spin(18,16,Str_Ob_En_ExportWh, Unit_Active_EnTotal, ELSTER_SCALE_TOTAL_ENERGY);      
				MI_DecodeData_Spin(34,16, StrNull, StrNull, 0);   //them Q Reactive  
				MI_DecodeData_Spin(50,16,Str_Ob_En_ImportVar, Unit_Reactive_EnTotal, ELSTER_SCALE_TOTAL_ENERGY);  
				MI_DecodeData_Spin(66,16, StrNull, StrNull, 0);
				MI_DecodeData_Spin(82,16,Str_Ob_En_ExportVar, Unit_Reactive_EnTotal, ELSTER_SCALE_TOTAL_ENERGY);
//				MI_DecodeData_Spin(98,16,0x2D);
				break;
			case 55://EnergyPlusArate1-3,EnergySubArate1-3
				MI_DecodeData_Spin( 2,16,Str_Ob_AcPlus_Rate1, Unit_Active_EnTotal, ELSTER_SCALE_TARRIFF);    
				MI_DecodeData_Spin(18,16,Str_Ob_AcPlus_Rate2, Unit_Active_EnTotal, ELSTER_SCALE_TARRIFF);
				MI_DecodeData_Spin(34,16,Str_Ob_AcPlus_Rate3, Unit_Active_EnTotal, ELSTER_SCALE_TARRIFF);
				MI_DecodeData_Spin(50,16,Str_Ob_AcSub_Rate1, Unit_Active_EnTotal, ELSTER_SCALE_TARRIFF);   
				MI_DecodeData_Spin(66,16,Str_Ob_AcSub_Rate2, Unit_Active_EnTotal, ELSTER_SCALE_TARRIFF);
				MI_DecodeData_Spin(82,16,Str_Ob_AcSub_Rate3, Unit_Active_EnTotal, ELSTER_SCALE_TARRIFF);
				break;
			case 56://MaxDemandPlusARate13-Time
                if(Check_sTime_MD(&UART1_Receive_Buff[0], 2, 8) == 1)
                {
                    MI_DecodeData_Spin(12,14,Str_Ob_MaxDeRate1, Unit_MAXDEMAND, ELSTER_SCALE_MAX_DEMAND);  //value
                    MI_DecodeData_TimeBySecond( 2, 8);  //time     
                }
                if(Check_sTime_MD(&UART1_Receive_Buff[0], 26, 8) == 1)
                {
                    MI_DecodeData_Spin(36,14,Str_Ob_MaxDeRate2, Unit_MAXDEMAND, ELSTER_SCALE_MAX_DEMAND);
                    MI_DecodeData_TimeBySecond(26, 8);
                }
                if(Check_sTime_MD(&UART1_Receive_Buff[0], 50, 8) == 1)
                {
                    MI_DecodeData_Spin(60,14,Str_Ob_MaxDeRate3, Unit_MAXDEMAND, ELSTER_SCALE_MAX_DEMAND);	
                    MI_DecodeData_TimeBySecond(50, 8);   //giai ma lai time
                }
				break;
			case 57://Tu-TuM
//				MI_DecodeData_Spin( 2, 8,Str_Ob_Tu, StrNull, ELSTER_SCALE_TU_TI); // ( 4, 6,0x42)   
//				MI_DecodeData_Spin(10, 6,Str_Ob_Tu, StrNull, ELSTER_SCALE_TU_TI);	// (12, 4,0x65)			
				break;
			case 58://Ti-TiM
//				MI_DecodeData_Spin( 2, 8,Str_Ob_Ti, StrNull, ELSTER_SCALE_TU_TI);  // 4 6     
//				MI_DecodeData_Spin(10, 4,Str_Ob_Ti, StrNull, ELSTER_SCALE_TU_TI);	 // 12 2				
				break;
			case 59://MeterTime
				MI_DecodeData_DateTime( 2,14,0x01);
				break;
			default:
				break;			
		}
	}
}



void MI_DecodeData_NoSpin (uint8_t startPos, uint8_t dataLength, truct_String oBIS, truct_String unit, uint8_t ScaleNum )
{
	uint16_t i=0;
    uint8_t Sign = 0;
    uint8_t Scale = 0;
    uint8_t Buff_data[20];
    uint8_t BuffPayload[30];
    uint8_t Flag_ZeroFirst = 0;
    truct_String Str_Data_Write = {&Buff_data[0], 0};
    truct_String StrAdd_To_payload = {&BuffPayload[0], 0};
   
	for (i=0;i<dataLength;i++)
		MeterRawData[i] = UART1_Receive_Buff[startPos+i];
    
    if(Get_Meter_Info.Flag_Start_Pack == 0)
    {
        Reset_Buff(&Get_Meter_Info.Str_Payload);
        //wrire header TSVH vao
        Write_Header_TSVH_Push103();
        Get_Meter_Info.Flag_Start_Pack = 1;
    }   
    
    
    if(MeterRawData[0] == 8) Sign = 1; //dau am
    else Sign = 0;  //duong
    
    Scale = ((MeterRawData[0] & 0x07) << 4) | MeterRawData[1];
    
    if(Scale > 4) return;    //neu scale > 4: du lieu bi loi
    
    //ghi obis va data vao
    Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Obis_Inbuff, &oBIS);
    Get_Meter_Info.Pos_Obis_Inbuff += oBIS.Length_u16;
    Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
    
    if(oBIS.Length_u16 != 0) Get_Meter_Info.Numqty ++;
    //ghi data vao
    
    if((MeterRawData[2] == 0x0F) && (MeterRawData[3] == 0x0F)) 
    {
      *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '0';
    }else
    {
        for (i=2;i<dataLength;i++)
        {
            if(Flag_ZeroFirst == 0)
            {
                if(MeterRawData[i] == 0) 
                  continue;
                else Flag_ZeroFirst = 1;
            }

            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = MeterRawData[i] + 0x30; 
        }
        if(Flag_ZeroFirst == 0) *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '0';   //neu gia tri deu la so 0 thi ghi 1 so 0 vao Buff
    }
    //scale trong value. 1 tuong ung voi 10, 2: 100... chi max la 4.
    //Cac dai luong Vol, cur, PA, PR PAP moi tinh den scale nay
    if((Get_Meter_Info.Mess_Step_ui8 == 5) || (Get_Meter_Info.Mess_Step_ui8 == 11) || 
        (Get_Meter_Info.Mess_Step_ui8 == 29) || (Get_Meter_Info.Mess_Step_ui8 == 35) || (Get_Meter_Info.Mess_Step_ui8 == 41))  
    for(i = 0; i < Scale; i++)
         *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '0';
    
    //them dau '.' vao vi tri tuong ung voi scale
    AddPoint_to_StringDec(&Str_Data_Write, ScaleNum);
    
    //copy ca chuoi du lieu bao gom ca  ().
    *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '(';
    if((Sign == 1) && ((Buff_data[0] != 0) || (Str_Data_Write.Length_u16 != 1)))  
      *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '-';
    for(i = 0; i < Str_Data_Write.Length_u16; i++)
      *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = *(Str_Data_Write.Data_a8 + i) ;
    if(unit.Length_u16 != 0)
        *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '*';
    Copy_String_2(&StrAdd_To_payload, &unit);  //don vi
    *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = ')';
    
    
    Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Data_Inbuff, &StrAdd_To_payload);
    Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
}

uint8_t Check_sTime_MD (uint8_t* Buff, uint16_t pos, uint8_t length)
{
    uint16_t i = 0;
    
    for(i = 0; i <length; i++)
      if(*(Buff + pos + i) != 0) return 1;
    
    return 0;
}
void MI_DecodeData_TimeBySecond (uint8_t startPos, uint8_t dataLength)
{
    int8_t i = 0;
    uint8_t j = 0;
    uint32_t            meterTimeStamp = 0;
    ST_TIME_FORMAT 		sTimeConvert;
    uint8_t BuffPayload[30];
    truct_String StrAdd_To_payload = {&BuffPayload[0], 0};
    
    uint8_t   aTempData[16] = {0};
    
	for (i=0;i<dataLength;i++)
		MeterRawData[i] = UART1_Receive_Buff[startPos+i];
	
    for (i=dataLength-1; i>-1;i-=2)
	{
		aTempData[dataLength-1-i] =  MeterRawData[i-1];
		aTempData[dataLength-i] =  MeterRawData[i];
	}
    
    for (i=0;i<dataLength;i++)
	{
		MeterRawData[j] = (aTempData[i]<<4)|(aTempData[i+1]);
		i++;
		j++;
	}
    
    for (j=0;j<(dataLength/2);j++)
        meterTimeStamp = (meterTimeStamp << 8) | MeterRawData[j];
    Epoch_to_date_time(&sTimeConvert,meterTimeStamp, 2);
    
    *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '('; 
    Copy_String_STime(&StrAdd_To_payload,sTimeConvert);
    *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = ')';
    
    
    Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Data_Inbuff, &StrAdd_To_payload);
    Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
}

uint64_t Q1_Data = 0;
uint64_t Q3_Data = 0;
void MI_DecodeData_Spin (uint8_t startPos, uint8_t dataLength, truct_String oBIS, truct_String unit, uint8_t ScaleNum)
{
	int16_t i=0;
    uint8_t Buff_data[20];
    uint8_t BuffPayload[30];
    uint8_t Flag_ZeroFirst = 0;
    truct_String Str_Data_Write = {&Buff_data[0], 0};
    uint8_t Flag_have_TuTi = 0;
    truct_String StrAdd_To_payload = {&BuffPayload[0], 0};
    uint8_t   aTempData[16] = {0};
    uint64_t 	Temp_data = 0;
    
	for (i=0;i<dataLength;i++)
		MeterRawData[i] = UART1_Receive_Buff[startPos+i];

    
	
    for (i=dataLength-1; i>-1;i-=2)
	{
		aTempData[dataLength-1-i] =  MeterRawData[i-1];
		aTempData[dataLength-i] =  MeterRawData[i];
	}

    if(Get_Meter_Info.Flag_Start_Pack == 0)
    {
        Reset_Buff(&Get_Meter_Info.Str_Payload);
        //wrire header TSVH vao
        Write_Header_TSVH_Push103();
        Get_Meter_Info.Flag_Start_Pack = 1;
    }   
       
    if(Get_Meter_Info.Mess_Step_ui8 != 54)
    {
        Flag_have_TuTi = Compare_String(oBIS, Str_Ob_Tu);  
        Flag_have_TuTi |= Compare_String(oBIS, Str_Ob_Ti);
        
        //ghi obis va data vao
        if((Flag_have_TuTi != 1) || (startPos != 10)) //Khac Tum Tim moi cho ghi obis
        {
            Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Obis_Inbuff, &oBIS);
            Get_Meter_Info.Pos_Obis_Inbuff += oBIS.Length_u16;
            Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
            
            if(oBIS.Length_u16 != 0) Get_Meter_Info.Numqty ++;
        }
        //bo qua sign va scale
    
        for (i=0;i<dataLength;i++)
        {
            if(Flag_ZeroFirst == 0)
            {
                if(aTempData[i] == 0) 
                  continue;
                else Flag_ZeroFirst = 1;
            }

            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = aTempData[i] + 0x30; 
        }
        if(Flag_ZeroFirst == 0) *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '0';   //neu gia tri deu la so 0 thi ghi 1 so 0 vao Buff
        
        //them dau '.' vao vi tri tuong ung voi scale
        AddPoint_to_StringDec(&Str_Data_Write, ScaleNum);
        
        //copy ca chuoi du lieu bao gom ca  ().
        if((Flag_have_TuTi == 1) && (startPos == 10))  //Neu la Tum va Tim   
            *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '/';
        else
            *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '(';
        
        for(i = 0; i < Str_Data_Write.Length_u16; i++)
          *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = *(Str_Data_Write.Data_a8 + i) ;
        if(unit.Length_u16 != 0)
            *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '*';
        Copy_String_2(&StrAdd_To_payload, &unit);  //don vi
        
        if((Flag_have_TuTi != 1) || (startPos != 2)) //neu khac Tu Ti thi c� ')'. C�n neu la Tu va Ti thi k co ')'
            *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = ')';
        
        Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Data_Inbuff, &StrAdd_To_payload);
        Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
    }else
    {
        //ghep data thanh Hex.
        for (i=2;i<dataLength;i++)
        {
            Temp_data = Temp_data*10 + (aTempData[i] & 0x0F);
        }
        //c�ng lai
        if((startPos == 34)||(startPos == 66))
        {
            if(startPos == 34)
                Q1_Data = Temp_data;
            if(startPos == 66)
                Q3_Data = Temp_data;
        }
        else
        {
            Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Obis_Inbuff, &oBIS);
            Get_Meter_Info.Pos_Obis_Inbuff += oBIS.Length_u16;
            Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
            
            if(oBIS.Length_u16 != 0) Get_Meter_Info.Numqty ++;
            
            if(startPos == 50)
                Temp_data += Q1_Data;
            if(startPos == 82)
                Temp_data += Q3_Data;
            //chuyen data thanh chuoi Dec string va co scale
            Pack_HEXData_Frame_Uint64(&Str_Data_Write, Temp_data, ScaleNum);   
                
             //copy ca chuoi du lieu bao gom ca  ().
            *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '(';
            
            for(i = 0; i < Str_Data_Write.Length_u16; i++)
              *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = *(Str_Data_Write.Data_a8 + i) ;
            if(unit.Length_u16 != 0)
                *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '*';
            Copy_String_2(&StrAdd_To_payload, &unit);  //don vi
            *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = ')';
            
            Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Data_Inbuff, &StrAdd_To_payload);
            Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
        }
    }
}



void MI_DecodeData_DateTime (uint8_t startPos, uint8_t dataLength, uint8_t oBIS)
{
	uint8_t i=0,j=0;
	ST_TIME_FORMAT					sTime_Ins;
    uint8_t Buff_data[20];
    truct_String Str_Data_Write = {&Buff_data[0], 0};
    
	for (i=0;i<dataLength;i++)
	{
		MeterRawData[j] = (UART1_Receive_Buff[startPos+i]<<4) | (UART1_Receive_Buff[startPos+i+1]);
		i++;
		j++;
	}
	
    sTime_Ins.year  = ConvertHex_to_Dec (MeterRawData[6]); 		 // year
    sTime_Ins.month = ConvertHex_to_Dec (MeterRawData[4]&0x1F);  // month
	sTime_Ins.date  = ConvertHex_to_Dec (MeterRawData[3]&0x3F);  // date
	
	sTime_Ins.hour  = ConvertHex_to_Dec (MeterRawData[2]);       // hour
	sTime_Ins.min   = ConvertHex_to_Dec (MeterRawData[1]); 		 // min 
	sTime_Ins.sec   = ConvertHex_to_Dec (MeterRawData[0]);	     // sec 
//    //Ghi them data he so nhan vao
//    Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Obis_Inbuff, &He_So_Nhan);
//    Get_Meter_Info.Pos_Obis_Inbuff += He_So_Nhan.Length_u16;
//    Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
//    Get_Meter_Info.Numqty++;
//    //he so nhan	
//    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
//    Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) sDCU.He_So_Nhan, ELSTER_SCALE_HE_SO_NHAN); 
//    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
//    
//    Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Data_Inbuff, &Str_Data_Write);
    
    Str_Data_Write.Length_u16 = 0;
    
    //ghi stime vao vi tri Pos numqty. Sau do ghi numqty.
    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
    Copy_String_STime(&Str_Data_Write,sTime_Ins);
    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')'; 
    
//    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
//    Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) Get_Meter_Info.Numqty, 0);
//    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')'; 
//    
	Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.PosNumqty, &Str_Data_Write);
    Get_Meter_Info.PosNumqty += Str_Data_Write.Length_u16;
    Get_Meter_Info.Pos_Obis_Inbuff += Str_Data_Write.Length_u16;
    Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
}
void ELSTER_MI_SendData(void)
{
	                           
}
void ELSTER_MI_SendData_Inst(void)
{   
    	
}
// 
// 		Meter billing function
//

uint8_t Convert_Char_Elster (uint8_t Num)
{
    uint8_t Result = 0;
    
    switch(Num)
    {
        case 0x00:
            Result = 0x30;
            break;
        case 0x01:
            Result = 0xB1;
            break;
        case 0x02:
            Result = 0xB2;
            break;
        case 0x03:
            Result = 0x33;
            break;
        case 0x04:
            Result = 0xB4;
            break;
        case 0x05:
            Result = 0x35;
            break;
        case 0x06:
            Result = 0x36;
            break;
        case 0x07:
            Result = 0xB7;
            break;
        case 0x08:
            Result = 0xB8;
            break;
        case 0x09:
            Result = 0x39;
            break;
        case 0x0A:
            Result = 0x41;
            break;
        case 0x0B:
            Result = 0x42;
            break;
        case 0x0C:
            Result = 0xC3;
            break;
        case 0x0D:
            Result = 0x44;
            break;
        case 0x0E:
            Result = 0xC5;
            break;
        case 0x0F:
            Result = 0xC6;
            break;
        default:
            break;
    }
    return Result;
}


void Send_MBilling_Mess (void)
{
	uint8_t i=0,temp=0,temp_checksum=0;
    
	temp = ELSTER_MeterBillingMessIDTable[Get_Meter_Billing.Mess_Step_ui8];
	
    if((temp % 14) != 0)
    {
        ELSTER_MBilling_Table[0] = Convert_Char_Elster((temp>>4) & 0x0F);
        ELSTER_MBilling_Table[1] = Convert_Char_Elster(temp &0x0F);
        ELSTER_MBilling_Table[3] = 0xB4;
        ELSTER_MBilling_Table[4] = 0x30;
    }else
    {   
        ELSTER_MBilling_Table[0] = Convert_Char_Elster((temp>>4) & 0x0F);
        ELSTER_MBilling_Table[1] = Convert_Char_Elster(temp &0x0F);
        ELSTER_MBilling_Table[3] = 0xB1;
        ELSTER_MBilling_Table[4] = 0xC6;
    }
    
	for (i=0;i<5;i++)
		ELSTER_MeterBillingTemplate[i+8] = ELSTER_MBilling_Table[i];
	for (i=1;i<15;i++)
		temp_checksum = temp_checksum ^ ELSTER_MeterBillingTemplate[i];
	
	ELSTER_MeterBillingTemplate[15] = temp_checksum;
	
	HAL_UART_Transmit(&UART_METER,ELSTER_MeterBillingTemplate,16,1000);
}
void MB_ExtractDataFunc (void)
{
	uint16_t i=0;
	
	ELSTER_Decode_byte(UART1_Receive_Buff,2,UART1_Control.Mess_Length_ui16-4);
	for (i=2;i<UART1_Control.Mess_Length_ui16-2;i++)
		Meter_TempBuff[Get_Meter_Billing.Data_Buff_Pointer_ui16++] = UART1_Receive_Buff[i];
}

uint8_t MB_DecodeData_TimeBySecond (uint16_t startPos, uint8_t dataLength)
{
    int8_t i = 0;
    uint8_t j = 0;
    uint32_t            meterTimeStamp = 0;
    ST_TIME_FORMAT 		sTimeConvert;
    uint8_t BuffPayload[30];
    truct_String StrAdd_To_payload = {&BuffPayload[0], 0};
    uint32_t            diffTimeStampStart = 0;
    uint8_t         diffmonth = 0;
    
    uint8_t   aTempData[16] = {0};
    
	for (i=0;i<dataLength;i++)
		MeterRawData[i] = Meter_TempBuff[startPos+i];
	
    for (i=dataLength-1; i>-1;i-=2)
	{
		aTempData[dataLength-1-i] =  MeterRawData[i-1];
		aTempData[dataLength-i] =  MeterRawData[i];
	}
    
    for (i=0;i<dataLength;i++)
	{
		MeterRawData[j] = (aTempData[i]<<4)|(aTempData[i+1]);
		i++;
		j++;
	}
    
    for (j=0;j<(dataLength/2);j++)
        meterTimeStamp = (meterTimeStamp << 8)| MeterRawData[j];
    Epoch_to_date_time(&sTimeConvert,meterTimeStamp, 2);
    
    *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '('; 
    Copy_String_STime(&StrAdd_To_payload,sTimeConvert);
    *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = ')';
    
    if(startPos != 1718)
    {
        Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Data_Inbuff, &StrAdd_To_payload);
        Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
    }else    //neu la stime cua Billing. ghi stime va numqty
    {
        if(sInformation.Flag_Request_Bill == 1)
        {
            //kiem tra xem Bill da qua end time khi request chua. thi dung lai
            if(Check_DiffTime(sInformation.EndTime_GetBill, sTimeConvert, &diffTimeStampStart) == 0)
            {
                //tinh lai index.
                //neu nho hon start thi doc tiep. lon hon stop thi dung doc
                diffmonth = (diffTimeStampStart/86400)/31;   //cu lay thang lon nhat 31 ngay. Neu no cach xa hon n thang thi tang index len (n-1). de ti thoat ra ngoai tang 1 nua
                if(diffmonth >= 2)
                {
                    Get_Meter_Billing.IndexRead += diffmonth -1;
                    Bill_NumDay += diffmonth -1;
                }
                return 0;
            }
            if(Check_DiffTime(sTimeConvert, sInformation.StartTime_GetBill, &diffTimeStampStart) == 0)
            {
                sInformation.Flag_Stop_ReadBill = 1;   //do doc nguoc tro xuong. 
                return 0; 
            }
            Bill_NumDay++;  //tang dan Numbill. cho den khi thoi gian qua thi dung lai
        }
        
        *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '('; 
        Pack_HEXData_Frame_Uint64(&StrAdd_To_payload, (uint64_t) Get_Meter_Billing.Numqty, 0);
        *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = ')'; 
        
        Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.PosNumqty, &StrAdd_To_payload);
        
        Get_Meter_Billing.Pos_Obis_Inbuff += StrAdd_To_payload.Length_u16;
        Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
    }
    return 1;
}

uint64_t Q1_Chot_Data = 0;
uint64_t Q3_Chot_Data = 0;
void MB_DecodeData_Spin (uint16_t startPos, uint8_t dataLength, truct_String oBIS, truct_String unit, uint8_t ScaleNum)
{
	int8_t i=0;
    uint8_t Buff_data[20];
    uint8_t BuffPayload[30];
    truct_String StrAdd_To_payload = {&BuffPayload[0], 0};
    uint8_t Flag_ZeroFirst = 0;
    truct_String Str_Data_Write = {&Buff_data[0], 0};
    uint8_t   aTempData[16] = {0};
    uint64_t 	Temp_data = 0;
    
	for (i=0;i<dataLength;i++)
		MeterRawData[i] = Meter_TempBuff[startPos+i];
	
    //dao lai cac byte
    for (i=dataLength-1; i>-1;i-=2)
	{
		aTempData[dataLength-1-i] =  MeterRawData[i-1];
		aTempData[dataLength-i] =  MeterRawData[i];
	}
    
    if(Get_Meter_Billing.Flag_Start_Pack == 0)
    {
        Reset_Buff(&Get_Meter_Billing.Str_Payload);
        Write_Header_His_Push103();
        Get_Meter_Billing.Flag_Start_Pack = 1;
    }
    if((startPos != 42) && (startPos != 58) && (startPos != 74) && (startPos != 90))
    {
        //ghi obis vao
        Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Obis_Inbuff, &oBIS);
        Get_Meter_Billing.Pos_Obis_Inbuff += oBIS.Length_u16;
        Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
        
        if(oBIS.Length_u16 != 0) Get_Meter_Billing.Numqty ++;
        //ghi data vao
        
        for (i=0;i<dataLength;i++)
        {
            if(Flag_ZeroFirst == 0)
            {
                if(aTempData[i] == 0) 
                  continue;
                else Flag_ZeroFirst = 1;
            }

            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = aTempData[i] + 0x30; 
        }
        if(Flag_ZeroFirst == 0) *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '0';   //neu gia tri deu la so 0 thi ghi 1 so 0 vao Buff
        
        //them dau '.' vao vi tri tuong ung voi scale
        AddPoint_to_StringDec(&Str_Data_Write, ScaleNum);

        
        //copy ca chuoi du lieu bao gom ca  ().
        *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '(';
        
        for(i = 0; i < Str_Data_Write.Length_u16; i++)
          *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = *(Str_Data_Write.Data_a8 + i) ;
        if(unit.Length_u16 != 0)
            *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '*';
        Copy_String_2(&StrAdd_To_payload, &unit);  //don vi

        *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = ')';
        
        Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Data_Inbuff, &StrAdd_To_payload);
        Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
    }else
    {
        //ghep data thanh Hex.
        for (i=0;i<dataLength;i++)
        {
            Temp_data = Temp_data*10 + (aTempData[i] & 0x0F);
        }
        //c�ng lai
        if((startPos == 42)||(startPos == 74))
        {
            if(startPos == 42)
                Q1_Chot_Data = Temp_data;  
            if(startPos == 74)
                Q3_Chot_Data = Temp_data;
        }
        else
        {
            Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Obis_Inbuff, &oBIS);
            Get_Meter_Billing.Pos_Obis_Inbuff += oBIS.Length_u16;
            Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
            
            if(oBIS.Length_u16 != 0) Get_Meter_Billing.Numqty ++;
            
            if(startPos == 58)
                Temp_data += Q1_Chot_Data;
            if(startPos == 90)
                Temp_data += Q3_Chot_Data;
            //chuyen data thanh chuoi Dec string va co scale
            Pack_HEXData_Frame_Uint64(&Str_Data_Write, Temp_data, ScaleNum);   
                
             //copy ca chuoi du lieu bao gom ca  ().
            *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '(';
            
            for(i = 0; i < Str_Data_Write.Length_u16; i++)
              *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = *(Str_Data_Write.Data_a8 + i) ;
            if(unit.Length_u16 != 0)
                *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '*';
            Copy_String_2(&StrAdd_To_payload, &unit);  //don vi
            *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = ')';
            
            Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Data_Inbuff, &StrAdd_To_payload);
            Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
        }
    }
}

//void MB_DecodeBeginTime (uint16_t startPos, uint8_t dataLength, uint8_t oBIS)
//{
//	int8_t i=0;
//	uint32_t BeginTimeStamp=0;
//	uint8_t Temp0=0;
//	ST_TIME_FORMAT BeginTime;
//	
//	for (i=0;i<dataLength;i++)
//		MeterRawData[i] = Meter_TempBuff[startPos+i];
//	
//	for (i=dataLength-2;i>-1;i--)
//	{
//		Temp0 = (MeterRawData[i]<<4)|(MeterRawData[i+1]);
//		BeginTimeStamp = BeginTimeStamp|(Temp0<<(4*i));
//		i--;
//	}
//	Epoch_to_date_time(&BeginTime,BeginTimeStamp, 2);
//	MeterBillingDataBuff[Get_Meter_Billing.Data_Buff_Pointer_ui16++] = oBIS;
//	MeterBillingDataBuff[Get_Meter_Billing.Data_Buff_Pointer_ui16++] = 0x06;
//	MeterBillingDataBuff[Get_Meter_Billing.Data_Buff_Pointer_ui16++] = BeginTime.year;
//	MeterBillingDataBuff[Get_Meter_Billing.Data_Buff_Pointer_ui16++] = BeginTime.month;
//	MeterBillingDataBuff[Get_Meter_Billing.Data_Buff_Pointer_ui16++] = BeginTime.date;
//	MeterBillingDataBuff[Get_Meter_Billing.Data_Buff_Pointer_ui16++] = BeginTime.hour;
//	MeterBillingDataBuff[Get_Meter_Billing.Data_Buff_Pointer_ui16++] = BeginTime.min;
//	MeterBillingDataBuff[Get_Meter_Billing.Data_Buff_Pointer_ui16++] = BeginTime.sec;
//}


void MB_SendData(void)
{
	uint8_t     Temp_BBC = 0;
    
	Get_Meter_Billing.Data_Buff_Pointer_ui16 = 10;
	MB_DecodeData_Spin(  10,16,Str_Ob_AcImTotal_Chot, Unit_Active_EnTotal, ELSTER_SCALE_TOTAL_ENERGY);  //Import Kwh  
	MB_DecodeData_Spin(  26,16,Str_Ob_AcExTotal_Chot, Unit_Active_EnTotal, ELSTER_SCALE_TOTAL_ENERGY);
    
	MB_DecodeData_Spin(  42,16,StrNull, StrNull, 0);   //Them Q1+Q3 v� Q2+Q4  
	MB_DecodeData_Spin(  58,16,Str_Ob_ReImTotal_Chot, Unit_Reactive_EnTotal, ELSTER_SCALE_TOTAL_ENERGY);  
	MB_DecodeData_Spin(  74,16,StrNull, StrNull, 0);
	MB_DecodeData_Spin(  90,16,Str_Ob_ReExTotal_Chot, Unit_Reactive_EnTotal, ELSTER_SCALE_TOTAL_ENERGY);
    
//	MB_DecodeData_Spin( 106,16,0x2D);
	MB_DecodeData_Spin( 170,16,Str_Ob_AcPlus_Rate1_chot, Unit_Active_EnTotal, ELSTER_SCALE_TARRIFF);   //bieu 1 import        
	MB_DecodeData_Spin( 186,16,Str_Ob_AcPlus_Rate2_chot, Unit_Active_EnTotal, ELSTER_SCALE_TARRIFF);
	MB_DecodeData_Spin( 202,16,Str_Ob_AcPlus_Rate3_chot, Unit_Active_EnTotal, ELSTER_SCALE_TARRIFF);
    
	MB_DecodeData_Spin( 218,16,Str_Ob_AcSub_Rate1_chot, Unit_Active_EnTotal, ELSTER_SCALE_TARRIFF);
	MB_DecodeData_Spin( 234,16,Str_Ob_AcSub_Rate2_chot, Unit_Active_EnTotal, ELSTER_SCALE_TARRIFF);
	MB_DecodeData_Spin( 250,16,Str_Ob_AcSub_Rate3_chot, Unit_Active_EnTotal, ELSTER_SCALE_TARRIFF);
    
	if(Check_sTime_MD(&UART1_Receive_Buff[0], 914, 8) == 1)
    {
        MB_DecodeData_Spin( 922,16,Str_Ob_MaxDeRate1_Chot, Unit_MAXDEMAND, ELSTER_SCALE_MAX_DEMAND);  //value
        MB_DecodeData_TimeBySecond( 914, 8);   //time   
    }
	
    if(Check_sTime_MD(&UART1_Receive_Buff[0], 1010, 8) == 1)
    {
        MB_DecodeData_Spin(1018,16,Str_Ob_MaxDeRate2_Chot, Unit_MAXDEMAND, ELSTER_SCALE_MAX_DEMAND);
        MB_DecodeData_TimeBySecond(1010, 8);
    }
    
    if(Check_sTime_MD(&UART1_Receive_Buff[0], 1034, 8) == 1)
    {
        MB_DecodeData_Spin(1042,16,Str_Ob_MaxDeRate3_Chot, Unit_MAXDEMAND, ELSTER_SCALE_MAX_DEMAND);
        MB_DecodeData_TimeBySecond(1034, 8);
    }
        
    //ghi Tu Ti vao
    Add_TuTI_toPayload(&Get_Meter_Billing);
        
//	MB_DecodeData_Spin(1706, 4,0x5D);
//	MB_DecodeData_Spin(1710, 8,0x5B);
    
    //ghi time 
	if(MB_DecodeData_TimeBySecond(1718, 8) == 1)   //numqty o trong nay roi
	{
        //ETX
        *(Get_Meter_Billing.Str_Payload.Data_a8 + Get_Meter_Billing.Str_Payload.Length_u16++) = ETX; 
        //BBC
        Temp_BBC = BBC_Cacul(Get_Meter_Billing.Str_Payload.Data_a8 + 1,Get_Meter_Billing.Str_Payload.Length_u16 - 1);
        *(Get_Meter_Billing.Str_Payload.Data_a8 + Get_Meter_Billing.Str_Payload.Length_u16++) = Temp_BBC;
        
        //Push data to queue
        Push_Bill_toQueue(DATA_HISTORICAL);
    }
}
// 
// 		Meter event function
//
void Send_MEvent_Mess (void)
{
	uint8_t i=0,temp=0;
	uint8_t mMess[22];
	
	temp = ELSTER_MeterEventMessIDTable[Get_Meter_Event.Mess_Step_ui8];
	for (i=0;i<16;i++)
		mMess[i] = ELSTER_MEvent_Table[temp][i];
	HAL_UART_Transmit(&UART_METER,mMess,16,1000);
}


void ELSTER_ME_ExtractDataFunc (void)
{
	ELSTER_Decode_byte(UART1_Receive_Buff,2,UART1_Control.Mess_Length_ui16-4);
	switch(Get_Meter_Event.Mess_Step_ui8)
	{
//		case 0://ProgrammingLog
//			ME_DecodeData_Spin( 2, 4,0x17);
//			ME_DecodeData_Spin( 6, 8,0x18);
//			ME_DecodeData_Spin(14, 8,0x18);
//			ME_DecodeData_Spin(22, 8,0x18);
//			ME_DecodeData_Spin(30, 8,0x18);
//			ME_DecodeData_Spin(38, 8,0x18);
//			break;
//		case 1://PasswordChange
//			ME_DecodeData_Spin( 2, 4,0x29);
//			ME_DecodeData_Spin( 6, 8,0x2A);
//			ME_DecodeData_Spin(14, 8,0x2A);
//			ME_DecodeData_Spin(22, 8,0x2A);
//			ME_DecodeData_Spin(30, 8,0x2A);
//			ME_DecodeData_Spin(38, 8,0x2A);
//			break;
//		case 2://VoltageImbalance
//			ME_DecodeData_Spin( 2, 4,0x13);
//			ME_DecodeData_Spin( 6, 8,0x14);
//		
//			ME_DecodeData_Spin(14, 8,0x15);
//			ME_DecodeData_Spin(22, 8,0x15);
//			ME_DecodeData_Spin(30, 8,0x15);
//			ME_DecodeData_Spin(38, 8,0x15);	
//			ME_DecodeData_Spin(46, 8,0x15);
//		
//			ME_DecodeData_Spin(54, 8,0x16);
//			ME_DecodeData_Spin(62, 8,0x16);
//			ME_DecodeData_Spin(70, 8,0x16);	
//			ME_DecodeData_Spin(78, 8,0x16);
//			ME_DecodeData_Spin(86, 8,0x16);	
//			break;
//		case 3://PowerFailure
//			ME_DecodeData_Spin( 2, 4,0x2D);
//			ME_DecodeData_Spin( 6, 8,0x04);
//		
//			ME_DecodeData_Spin(14, 8,0x06);
//			ME_DecodeData_Spin(22, 8,0x06);
//			ME_DecodeData_Spin(30, 8,0x06);
//			ME_DecodeData_Spin(38, 8,0x06);	
//			ME_DecodeData_Spin(46, 8,0x06);
//		
//			ME_DecodeData_Spin(54, 8,0x05);
//			ME_DecodeData_Spin(62, 8,0x05);
//			ME_DecodeData_Spin(70, 8,0x05);	
//			ME_DecodeData_Spin(78, 8,0x05);
//			ME_DecodeData_Spin(86, 8,0x05);
//			break;
		case 4://Time/DateChange
            if(Get_Meter_Event.Flag_Start_Pack == 0)
            {
                Get_Meter_Event.Flag_Start_Pack = 1;
                Reset_Buff(&Get_Meter_Event.Str_Payload);
                Header_event_103(&Get_Meter_Event.Str_Payload, 1);
            }
			ME_DecodeData_Spin( 2, 4,0x19);
//			ME_DecodeData_Spin( 6, 8,0x1A);
//		
			ME_DecodeData_Spin(14, 8,0x1B);
//			ME_DecodeData_Spin(22, 8,0x1B);
//			ME_DecodeData_Spin(30, 8,0x1B);
//			ME_DecodeData_Spin(38, 8,0x1B);	
//			ME_DecodeData_Spin(46, 8,0x1B);
		
//			ME_DecodeData_Spin(54, 8,0x1C);
//			ME_DecodeData_Spin(62, 8,0x1C);
//			ME_DecodeData_Spin(70, 8,0x1C);	
//			ME_DecodeData_Spin(78, 8,0x1C);
//			ME_DecodeData_Spin(86, 8,0x1C);			
			break;
		case 5://PhaseFailure
            if(Get_Meter_Event.Flag_Start_Pack == 0)
            {
                Get_Meter_Event.Flag_Start_Pack = 1;
                Reset_Buff(&Get_Meter_Event.Str_Payload);
                Header_event_103(&Get_Meter_Event.Str_Payload, 1);
            }
			ME_DecodeMessType2(0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12);		
			break;
		case 6://ReverseRun
            if(Get_Meter_Event.Flag_Start_Pack == 0)
            {
                Get_Meter_Event.Flag_Start_Pack = 1;
                Reset_Buff(&Get_Meter_Event.Str_Payload);
                Header_event_103(&Get_Meter_Event.Str_Payload, 1);
            }
//			ME_DecodeMessType2(0x1D,0x1E,0x1F,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28);
			break;
//		case 7://ReadInstEvent1
//			ME_DecodeMessType2(0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B);
//			break;
//		case 8://ReadInstEvent2
//			ME_DecodeMessType2(0x3C,0x3D,0x3E,0x3F,0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47);
//			break;
//		case 9://ReadInstEvent3
//			ME_DecodeMessType2(0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,0x50,0x51,0x52,0x53);
//			break;
//		case 10://ReadInstEvent4
//			ME_DecodeMessType2(0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F);
//			break;
//		case 11://ReadInstEvent5
//			ME_DecodeMessType2(0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B);
//			break;
		default:
			break;		
	}
}
void ME_DecodeData_Spin (uint16_t startPos, uint8_t dataLength, uint8_t oBIS)
{
	int8_t i = 0;
    uint8_t j = 0;
    uint32_t            meterTimeStamp = 0;
    ST_TIME_FORMAT 		sTimeConvert;
    uint8_t BuffPayload[30];
    truct_String StrAdd_To_payload = {&BuffPayload[0], 0};
    uint8_t   aTempData[16] = {0};
    int8_t    Row = 0;
    
    
	for (i=0;i<dataLength;i++)
		MeterRawData[i] = UART1_Receive_Buff[startPos+i];
	
    for (i=dataLength-1; i>-1;i-=2)
	{
		aTempData[dataLength-1-i] =  MeterRawData[i-1];
		aTempData[dataLength-i] =  MeterRawData[i];
	}
    
    for (i=0;i<dataLength;i++)
	{
		MeterRawData[j] = (aTempData[i]<<4)|(aTempData[i+1]);
		i++;
		j++;
	}
    
    for (j=0;j<(dataLength/2);j++)
        meterTimeStamp = (meterTimeStamp << 8) | MeterRawData[j];
    
    Row = Get_Row_InlistObis(oBIS);
  
    if(Row >= 0)
    {
        Copy_String_toTaget(&Get_Meter_Event.Str_Payload, Get_Meter_Event.Pos_Obis_Inbuff, Elster_Ob_Event[Row].ObisString);
        Get_Meter_Event.Pos_Obis_Inbuff += Elster_Ob_Event[Row].ObisString->Length_u16;
        Get_Meter_Event.Pos_Data_Inbuff = Get_Meter_Event.Str_Payload.Length_u16;
        Get_Meter_Event.Numqty ++;
    
        if(dataLength == 8)
        {
            Epoch_to_date_time(&sTimeConvert,meterTimeStamp, 2);
            
            *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '('; 
            Copy_String_STime(&StrAdd_To_payload,sTimeConvert);
            *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = ')'; 
        }else
        {
            *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '('; 
            Pack_HEXData_Frame_Uint64(&StrAdd_To_payload, (uint64_t) meterTimeStamp, 0);
            *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = ')'; 
        }
        
        Copy_String_toTaget(&Get_Meter_Event.Str_Payload, Get_Meter_Event.Pos_Data_Inbuff, &StrAdd_To_payload);
        Get_Meter_Event.Pos_Data_Inbuff = Get_Meter_Event.Str_Payload.Length_u16;
    }
}

int8_t Get_Row_InlistObis (uint8_t Obis_Hex)
{
    uint8_t i = 0;
    
    for(i =0; i < MAX_OBIS_EVENT; i++)   //them 
      if(Obis_Hex == Elster_Ob_Event[i].ObisHex) return i;
    
    return -1;
}

void ME_DecodeEventOder (uint8_t startA,uint8_t startB,uint8_t startC,uint8_t endA,uint8_t endB,uint8_t endC)
{
	uint8_t temp=0,i=0,j=0;
	
	for (i=0;i<10;i++)
	{
		temp = (UART1_Receive_Buff[i+118]<<4)|(UART1_Receive_Buff[i+119]);
		switch (temp)
		{
			case 1:
				ME_ObisOderStart[j] = startA;
				ME_ObisOderEnd[j] = endA;
				break;
			case 2:
				ME_ObisOderStart[j] = startB;
				ME_ObisOderEnd[j] = endC;
				break;
			case 3:
				ME_ObisOderStart[j] = startC;
				ME_ObisOderEnd[j] = endC;
				break;
			default:
				break;
		}
		i++;
		j++;
	}
}

void ME_DecodeMessType2 (uint8_t countA,uint8_t countB,uint8_t countC,uint8_t timeA,uint8_t timeB,uint8_t timeC,
									uint8_t startA,uint8_t startB,uint8_t startC,uint8_t endA,uint8_t endB,uint8_t endC)
{
	ME_DecodeEventOder(startA,startB,startC,endA,endB,endC);
	
	ME_DecodeData_Spin( 2, 4,countA);
	ME_DecodeData_Spin( 6, 4,countB);
	ME_DecodeData_Spin(10, 4,countC);

//	ME_DecodeData_Spin(14, 8,timeA);
//	ME_DecodeData_Spin(22, 8,timeB);
//	ME_DecodeData_Spin(30, 8,timeC);

	ME_DecodeData_Spin(38, 8,ME_ObisOderStart[0]);	
//	ME_DecodeData_Spin(46, 8,ME_ObisOderStart[1]);
//	ME_DecodeData_Spin(54, 8,ME_ObisOderStart[2]);
//	ME_DecodeData_Spin(62, 8,ME_ObisOderStart[3]);
//	ME_DecodeData_Spin(70, 8,ME_ObisOderStart[4]);	

	ME_DecodeData_Spin(78, 8,ME_ObisOderEnd[0]);
//	ME_DecodeData_Spin(86, 8,ME_ObisOderEnd[1]);		
//	ME_DecodeData_Spin(94, 8,ME_ObisOderEnd[2]);
//	ME_DecodeData_Spin(102, 8,ME_ObisOderEnd[3]);
//	ME_DecodeData_Spin(110, 8,ME_ObisOderEnd[4]);		
}
void ELSTER_ME_SendData (void)
{
    uint8_t         Temp_BBC = 0;
    uint8_t         Buff[20];
    truct_String    Str_Data_Write = {&Buff[0], 0};
    
	Add_TuTI_toPayload(&Get_Meter_Event);
            
    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
    Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) Get_Meter_Event.Numqty, 0);
    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')'; 
    
    Copy_String_toTaget(&Get_Meter_Event.Str_Payload, Get_Meter_Event.PosNumqty, &Str_Data_Write);
            
    *(Get_Meter_Event.Str_Payload.Data_a8 + Get_Meter_Event.Str_Payload.Length_u16++) = ETX;
    Temp_BBC = BBC_Cacul(Get_Meter_Event.Str_Payload.Data_a8 + 1,Get_Meter_Event.Str_Payload.Length_u16 - 1);
    *(Get_Meter_Event.Str_Payload.Data_a8 + Get_Meter_Event.Str_Payload.Length_u16++) = Temp_BBC;
    
    Get_Meter_Event.Flag_Start_Pack = 0; 
    Push_Even_toQueue(0);
}

void ELSTER_ME_SendData_TSVH (void)
{
    truct_String    StrTemp;
    
    //copy data tu Event sang buff tsvh
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
  
    Get_Meter_Event.Flag_Start_Pack = 0; //reset flag
}

void Send_MLoadProfile_Mess (void)
{
	uint8_t tempArr[4]={0x00,0x00,0x00,0x00};
	uint8_t i=0,tempCS=0;
	
	switch (Get_Meter_LProf.Mess_Step_ui8)
	{
		case 0:
			if (RMLP_NumDay > MAX_RECORD_LPF) RMLP_NumDay = 1;
			tempArr[0] = (RMLP_NumDay&0x0F)>>4;
			tempArr[1] = (RMLP_NumDay&0x0F);
			tempArr[2] = ((RMLP_NumDay>>8)&0x0F)>>4;
			tempArr[3] = ((RMLP_NumDay>>8)&0x0F);
			ELSTER_Encode_byte(tempArr,0,4);
			for (i=0;i<4;i++)
				RMLP_First_Mess[i+11] = tempArr[i];
			for (i=1;i<17;i++)
				tempCS = tempCS^RMLP_First_Mess[i];
			RMLP_First_Mess[17] = tempCS;
			
			HAL_UART_Transmit(&UART_METER,RMLP_First_Mess,18,1000);
			break;
		case 1:
			HAL_UART_Transmit(&UART_METER,RMLP_Second_Mess,16,1000);
			break;
		default:
            Get_Meter_LProf.IndexRead++;
			tempArr[0] = (Get_Meter_LProf.IndexRead >>12)&0x0F;;
			tempArr[1] = (Get_Meter_LProf.IndexRead >>8)&0x0F;
			tempArr[2] = (Get_Meter_LProf.IndexRead>>4)&0x0F;
			tempArr[3] = (Get_Meter_LProf.IndexRead)&0x0F;
            
			ELSTER_Encode_byte(tempArr,0,4);
			for (i=0;i<4;i++)
				RMLP_GetData_Mess[i+6] = tempArr[i];
			for (i=1;i<15;i++)
				tempCS = tempCS^RMLP_GetData_Mess[i];
			RMLP_GetData_Mess[15] = tempCS;
              
			HAL_UART_Transmit(&UART_METER,RMLP_GetData_Mess,16,1000);
			break;
	}
}

void RMLP_Extract_Data(void)
{
	uint16_t i=0;
	uint16_t temp=0;
	
	switch (Get_Meter_LProf.Mess_Step_ui8)
	{
		case 0:
			break;
		case 1:
            ELSTER_Decode_byte(UART1_Receive_Buff,2,UART1_Control.Mess_Length_ui16-4);
			for (i=4;i<=UART1_Control.Mess_Length_ui16-4;i++) // i = 2;
			{
				temp = (UART1_Receive_Buff[i]<<4)|(UART1_Receive_Buff[i+1]);
				i++;
			}
            if(ToTal_PackLpf < MAX_RECORD_LPF)
                ToTal_PackLpf = temp;
            else ToTal_PackLpf = 2;
            //kiem tra xem Index Pack doc ra da lon hon total chua
            for(i = 0; i<14; i++)
            {
                if(MeterLProfMessIDTable[i] == ToTal_PackLpf) 
                {
                  MeterLProfMessIDTable[i + 1] = 0xFF;
                }
            }
			break;
		default:
			ELSTER_Decode_byte(UART1_Receive_Buff,2,UART1_Control.Mess_Length_ui16-4);
			for (i=2;i<UART1_Control.Mess_Length_ui16-2;i++)
			{
				temp = (UART1_Receive_Buff[i]<<4)|(UART1_Receive_Buff[i+1]);
				Meter_TempBuff[Get_Meter_LProf.Data_Buff_Pointer_ui16++] = temp;
				i++;
			}
			break;
	}
}

void Pack_String_Dec (truct_String* Str_taget, truct_String* str_source, uint8_t length_Source, uint8_t scale)
{
    uint16_t i = 0;
    uint8_t Flag_ZeroFirst = 0;
    uint8_t Buff_data[20];
    truct_String Str_Data_Write = {&Buff_data[0], 0};
    
    for (i=0;i<length_Source;i++)
    {
        if(Flag_ZeroFirst == 0)
        {
            if(*(str_source->Data_a8+i) == '0') 
              continue;
            else Flag_ZeroFirst = 1;
        }

        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = *(str_source->Data_a8+i);
    }
    if(Flag_ZeroFirst == 0) *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '0';
    
    AddPoint_to_StringDec(&Str_Data_Write, scale);
    
    Copy_String_2(Str_taget, &Str_Data_Write);
}


void MLProf_SendData (void)
{
	uint16_t 	Data_Length = 0,i=10,j=0;
	uint8_t		Data_OBIS = 0;
    uint8_t     Buff_data[20];
    uint16_t    m = 0;
    uint32_t    diffTimeStampStart = 0;
    uint8_t 	number_parameter = 0;
    uint8_t 	parameter_index[4] ={0};
    uint8_t     scale_ID = 0;
    
    truct_String Str_Data_Write = {&Buff_data[0], 0};
    truct_String StrAdd_To_payload = {&BuffRecord[0], 0};
    
    
    //copy phan du cua lan truoc vao
    //dich cac byte ra phia sau: StrUartTemp.Length_u16 byte
    for(m = (Get_Meter_LProf.Data_Buff_Pointer_ui16 - 1); m>=10; m--)
        Meter_TempBuff[m + StrUartTemp.Length_u16] = Meter_TempBuff[m];
    for(m = 0; m < StrUartTemp.Length_u16; m++)
         Meter_TempBuff[m + 10] = *(StrUartTemp.Data_a8 + m);
    //cong them Length Uart them phan du
    Get_Meter_LProf.Data_Buff_Pointer_ui16 += StrUartTemp.Length_u16;
    StrUartTemp.Length_u16 = 0;
    
	Data_Length = Get_Meter_LProf.Data_Buff_Pointer_ui16;
	Get_Meter_LProf.Data_Buff_Pointer_ui16 = 10;
	Get_Meter_LProf.Flag_ui8 = 0;

	while (i<Data_Length)
	{
        Reset_Buff(&StrAdd_To_payload);
        Reset_Buff(&Str_Data_Write);
        
		Data_OBIS = Meter_TempBuff[i];
		switch (Data_OBIS)
		{
			case 0xE4:
                meterPeriod_Mins = Cal_LPPeriod(Meter_TempBuff[i+7]);
                
                number_parameter = 0;
                parameter_index[0] = 0; parameter_index[1] = 0; parameter_index[2] = 0; parameter_index[3] = 0;
				if(Meter_TempBuff[i+6]&0x01) // Import W   //kiem tra lai byte so 5 6 coi co nguoc k
                {
					number_parameter++;
                    parameter_index[0] = number_parameter;
                }
				if(Meter_TempBuff[i+6]&0x02) // Export W
                {
					number_parameter++;
                    parameter_index[1] = number_parameter;
                }
				if(Meter_TempBuff[i+5]&0x01) // Import var
                {
					number_parameter++;
                    parameter_index[2] = number_parameter;
                }
				if(Meter_TempBuff[i+5]&0x02) // Export var
                {
					number_parameter++;
                    parameter_index[3] = number_parameter;
                }
				if(Meter_TempBuff[i+6]&0x40) // Total Va
					number_parameter++;
                
                if (((Meter_TempBuff[i+8]) == 0xE6) || ((Meter_TempBuff[i+8]) == 0xE5))
				{
					i += 8;
					break;
				}else
                {
                    meterTimeStamp = 0;
                    for (j=0;j<4;j++)
                        meterTimeStamp = meterTimeStamp|(Meter_TempBuff[i+j+1]<<(j*8));
                    Epoch_to_date_time(ptrMeterLPTime,meterTimeStamp, 2);
                    if ((meterLPTime.min%meterPeriod_Mins) == 0)
                        i += 8;
                    else
                    {
                        Round_MLPTime();
//                        i += 21;      //4 dai luong 8+ 3*4
//                        i += 15;   //2 dai luong: 8 + 3*2 + 1
                        i += number_parameter*3+9;
                    }
                }
				break;
			case 0xE5://Co dien
				if ((Meter_TempBuff[i+5]) == 0xE6)
				{
					i += 5;
					break;
				}
				else
				{
					meterTimeStamp = 0;
					for (j=0;j<4;j++)
						meterTimeStamp = meterTimeStamp|(Meter_TempBuff[i+j+1]<<(j*8));
					Epoch_to_date_time(ptrMeterLPTime,meterTimeStamp, 2);
					if (((meterLPTime.min%meterPeriod_Mins)==0)&&(meterLPTime.sec==0))
						i += 5;
					else
					{
						Round_MLPTime();
                        i += number_parameter*3+6;
					}
				}
				break;
			case 0xE6://Mat dien
				if ((Meter_TempBuff[i+5]) == 0xE5)
					i += 5;
				else
                    i += number_parameter*3+6;
				break;
			case 0xFF:
				Get_Meter_LProf.Flag_ui8 = 2;
				break;
			default:
                
                if(Get_Meter_LProf.Flag_Start_Pack == 0)
                {
                    //kiem tra time start xem ngoai khoang request chua thi dung lai.
                    if(Check_DiffTime(sInformation.EndTime_GetLpf, meterLPTime, &diffTimeStampStart) == 0)
                    {
                        sInformation.Flag_Stop_ReadLpf = 1; //ket thuc
                        return;
                    }
                    Reset_Buff(&Get_Meter_LProf.Str_Payload);
                    Pack_Header_lpf_Pushdata103();
                    Get_Meter_LProf.Flag_Start_Pack = 1;
                }
                //cat va ghi data vao. Data d�n day duoi dang str DEC r�i nhung bi ghep thanh tung cap: 12 34 56. Bo qua bit dau va scale    
                if(parameter_index[0] != 0)
                {
                    Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Obis_Inbuff, &Str_Ob_lpf_type2[0]);
                    Get_Meter_LProf.Pos_Obis_Inbuff += Str_Ob_lpf_type2[0].Length_u16;
                    Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
                    Get_Meter_LProf.Numqty++;
                            
                    Reset_Buff(&Str_Data_Write);
                    *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '(';
                    for (j=0;j<3;j++)
                    {
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ((Meter_TempBuff[i+(parameter_index[0]-1)*3+1+j] >> 4) & 0x0F) +0x30;
                        if(j != 2)
                            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = (Meter_TempBuff[i+(parameter_index[0]-1)*3+1+j] & 0x0F) +0x30;
                        else scale_ID = (Meter_TempBuff[i+(parameter_index[0]-1)*3+1+j] & 0x0F);
                    }
                    for(j = 0; j<scale_ID; j++)
                      *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '0';
                    
                    Pack_String_Dec(&StrAdd_To_payload, &Str_Data_Write, (5 + scale_ID), ELSTER_SCALE_LPF);
                    *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '*';
                    Copy_String_2(&StrAdd_To_payload, &Unit_Lpf_type2[0]);  //don vi
                    *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = ')';
                }
                
                //
                if(parameter_index[1] != 0)
                {
                    Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Obis_Inbuff, &Str_Ob_lpf_type2[1]);
                    Get_Meter_LProf.Pos_Obis_Inbuff += Str_Ob_lpf_type2[1].Length_u16;
                    Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
                    Get_Meter_LProf.Numqty++;
                    
                    Reset_Buff(&Str_Data_Write);
                    *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '(';
                    for (j=0;j<3;j++)
                    {
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ((Meter_TempBuff[i+(parameter_index[1]-1)*3+1+j] >> 4) & 0x0F) +0x30;
                        if(j != 2)
                            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = (Meter_TempBuff[i+(parameter_index[1]-1)*3+1+j] & 0x0F) +0x30;
                        else scale_ID = Meter_TempBuff[i+(parameter_index[1]-1)*3+1+j] & 0x0F;
                    }
                    for(j = 0; j<scale_ID; j++)
                      *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '0';
                    
                    Pack_String_Dec(&StrAdd_To_payload, &Str_Data_Write, 5 + scale_ID, ELSTER_SCALE_LPF);
                    *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '*';
                    Copy_String_2(&StrAdd_To_payload, &Unit_Lpf_type2[1]);  //don vi
                    *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = ')';
                }
                
                if(parameter_index[2] != 0)
                {
                    Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Obis_Inbuff, &Str_Ob_lpf_type2[2]);
                    Get_Meter_LProf.Pos_Obis_Inbuff += Str_Ob_lpf_type2[2].Length_u16;
                    Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
                    Get_Meter_LProf.Numqty++;
                    
                    Reset_Buff(&Str_Data_Write);
                    *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '(';
                    for (j=0;j<3;j++)
                    {
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ((Meter_TempBuff[i+(parameter_index[2]-1)*3+1+j] >> 4) & 0x0F) +0x30;  
                        if(j != 2)
                            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = (Meter_TempBuff[i+(parameter_index[2]-1)*3+1+j] & 0x0F) +0x30;
                        else scale_ID = Meter_TempBuff[i+(parameter_index[2]-1)*3+1+j] & 0x0F;
                    }
                    for(j = 0; j<scale_ID; j++)
                      *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '0';
                    
                    Pack_String_Dec(&StrAdd_To_payload, &Str_Data_Write, 5 + scale_ID, ELSTER_SCALE_LPF);
                    *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '*';
                    Copy_String_2(&StrAdd_To_payload, &Unit_Lpf_type2[2]);  //don vi
                    *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = ')';
                }
                if(parameter_index[3] != 0)
                {
                    Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Obis_Inbuff, &Str_Ob_lpf_type2[3]);
                    Get_Meter_LProf.Pos_Obis_Inbuff += Str_Ob_lpf_type2[3].Length_u16;
                    Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
                    Get_Meter_LProf.Numqty++;
                    
                    Reset_Buff(&Str_Data_Write);
                    *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '(';
                    for (j=0;j<3;j++)
                    {
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ((Meter_TempBuff[i+(parameter_index[3]-1)*3+1+j] >> 4) & 0x0F) +0x30;
                        if(j != 2)
                            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = (Meter_TempBuff[i+(parameter_index[3]-1)*3+1+j] & 0x0F) +0x30;
                        else scale_ID = Meter_TempBuff[i+(parameter_index[3]-1)*3+1+j] & 0x0F;
                    }
                    for(j = 0; j<scale_ID; j++)
                      *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '0';
                    
                    Pack_String_Dec(&StrAdd_To_payload, &Str_Data_Write, 5 + scale_ID, ELSTER_SCALE_LPF);
                    *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '*';
                    Copy_String_2(&StrAdd_To_payload, &Unit_Lpf_type2[3]);  //don vi
                    *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = ')';
                }
                
                Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Data_Inbuff, &StrAdd_To_payload);
                Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
                
                //Them data TuTi
                Add_TuTI_toPayload(&Get_Meter_LProf);
                //num chanel
                 Reset_Buff(&StrAdd_To_payload);
                 *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '('; 
                Copy_String_STime(&StrAdd_To_payload, meterLPTime); 
                *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = ')'; 
                //event
                Copy_String_2(&StrAdd_To_payload, &Str_event_Temp);
                //period
                *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '('; 
                Pack_HEXData_Frame_Uint64(&StrAdd_To_payload, meterPeriod_Mins , 0);
                *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = ')'; 
                //num chanel
                *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '('; 
                Pack_HEXData_Frame_Uint64(&StrAdd_To_payload, (uint64_t) Get_Meter_LProf.Numqty, 0);  
                *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = ')'; 
                
                Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.PosNumqty, &StrAdd_To_payload);
                
				i += number_parameter*3+1; // 5 tham so 5*3+1
				//Calculate time for next record
				meterTimeStamp += meterPeriod_Mins*60;
				Epoch_to_date_time(ptrMeterLPTime,meterTimeStamp, 2);
                
                //neu qua 1000 byte th� gui queue sang task luu. Luu xong moi dong goi tiep
//                if((Get_Meter_LProf.Str_Payload.Length_u16 >= MAX_LENGTH_DATA_TCP) || (i >= Data_Length) || 
//                   (Meter_TempBuff[i] == 0xE4) ||(Meter_TempBuff[i] == 0xE5) || (Meter_TempBuff[i] == 0xE6) || (Meter_TempBuff[i] == 0xFF))
//                {
                    Send_MessLpf_toQueue();
                    Get_Meter_LProf.Flag_Start_Pack = 0;    //dong goi header moi
                    if(Meter_TempBuff[i] == 0xFF) return;
//                }
                
//                if((i > (Data_Length - 13)) && (i < Data_Length))  //neu lon hon 13 byte th� s? kh�ng c� data nua
                if((i > (Data_Length - 6)) && (i < Data_Length))
                {
                    Reset_Buff(&StrUartTemp);
                    //Copy phan du sang 1 buff khac de. 
                    for(uint16_t a = i; a < Data_Length; a++)
                        *(StrUartTemp.Data_a8 + StrUartTemp.Length_u16++) = Meter_TempBuff[a];
                    return;
                }
				break;
		}
        if (Get_Meter_LProf.Flag_ui8 == 2)
		{
			Get_Meter_LProf.Flag_ui8 = 0;
			break;
		}
	}	
}




uint8_t Cal_LPPeriod (uint8_t mPer)
{
	uint8_t periodIdx=0,mReVal=0;
	
	periodIdx = mPer & 0x0F;
	mReVal = aLoadprofileDataPeriod[periodIdx];
	
	return mReVal;
}

void Round_MLPTime(void)
{
	uint8_t spareMins=0,spareSec=0;
	
	spareSec = meterTimeStamp%60;
	spareMins = meterLPTime.min%meterPeriod_Mins;
	meterTimeStamp -= spareSec;
	meterTimeStamp += (meterPeriod_Mins-spareMins)*60;
	Epoch_to_date_time(ptrMeterLPTime,meterTimeStamp, 2);
}
//
uint8_t ELSTER_CheckResetReadMeter(uint32_t Timeout)
{
	// Reset neu doc sai cong to
    if ((sDCU.Status_Meter_u8 == 0) &&
            (Check_Time_Out(sDCU.LandMark_Count_Reset_Find_Meter,600000) == TRUE)) // 10p
    {
        osDelay(Timeout); // 5p
        Read_Meter_ID_Success = ELSTER_Get_Meter_ID(0);
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


//2020 sua them dong goi push data 103EVN HES


void ELSTER_TuTi_ExtractDataFunc (void)
{
    if ((ELSTER_MeterTuTiIDTable[Get_Meter_TuTi.Mess_Step_ui8]==1)||(ELSTER_MeterTuTiIDTable[Get_Meter_TuTi.Mess_Step_ui8]>10))
	{
        ELSTER_Decode_byte(UART1_Receive_Buff,2,UART1_Control.Mess_Length_ui16-4);   //chuyen sang 4 bit sau la gi� tri HEX.
        switch (Get_Meter_TuTi.Mess_Step_ui8)
        {
            case 0://Tu-TuM
                TuTi_DecodeData_Spin( 2, 8, ELSTER_SCALE_TU_TI);    // ( 4, 6,0x42)   
                TuTi_DecodeData_Spin(10, 6, ELSTER_SCALE_TU_TI);	// (12, 4,0x65)			
                break;
            case 1://Ti-TiM
                TuTi_DecodeData_Spin( 2, 8, ELSTER_SCALE_TU_TI);    // 4 6     
                TuTi_DecodeData_Spin(10, 4, ELSTER_SCALE_TU_TI);	 // 12 2	
                break;
            default: 
              break;
        }
    }
}

void ELSTER_TuTi_SendData(void)
{
    *(Get_Meter_TuTi.Str_Payload.Data_a8 + Get_Meter_TuTi.Str_Payload.Length_u16++) = '(';
    Pack_HEXData_Frame_Uint64(&Get_Meter_TuTi.Str_Payload, (uint64_t) sDCU.He_So_Nhan, ELSTER_SCALE_HE_SO_NHAN); 
    *(Get_Meter_TuTi.Str_Payload.Data_a8 + Get_Meter_TuTi.Str_Payload.Length_u16++) = ')';
}

void ELSTER_Send_MTuTi_Mess (void)
{
	uint8_t i=0,temp=0;
	uint8_t mMess[22];
	
	temp = ELSTER_MeterTuTiIDTable[Get_Meter_TuTi.Mess_Step_ui8];
	for (i=0;i<ELSTER_MInfo_Table[temp][22];i++)
		mMess[i] = ELSTER_MInfo_Table[temp][i];
	HAL_UART_Transmit(&UART_METER,mMess,ELSTER_MInfo_Table[temp][22],1000);
}


void TuTi_DecodeData_Spin (uint8_t startPos, uint8_t dataLength, uint8_t ScaleNum)
{
	int8_t i=0;
    uint8_t Buff_data[20];
    uint8_t BuffPayload[30];
    uint8_t Flag_ZeroFirst = 0;
    truct_String Str_Data_Write = {&Buff_data[0], 0};
    truct_String StrAdd_To_payload = {&BuffPayload[0], 0};
    uint8_t   aTempData[16] = {0};
    
	for (i=0;i<dataLength;i++)
		MeterRawData[i] = UART1_Receive_Buff[startPos+i];
	
    for (i=dataLength-1; i>-1;i-=2)
	{
		aTempData[dataLength-1-i] =  MeterRawData[i-1];
		aTempData[dataLength-i] =  MeterRawData[i];
	}
    
    if(Get_Meter_TuTi.Flag_Start_Pack ==0)
    {
        Reset_Buff(&Get_Meter_TuTi.Str_Payload);
        Get_Meter_TuTi.Flag_Start_Pack = 1;
    }
      
    for (i=0;i<dataLength;i++)
    {
        if(Flag_ZeroFirst == 0)
        {
            if(aTempData[i] == 0) 
              continue;
            else Flag_ZeroFirst = 1;
        }

        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = aTempData[i] + 0x30; 
    }
    if(Flag_ZeroFirst == 0) *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '0';   //neu gia tri deu la so 0 thi ghi 1 so 0 vao Buff
    
    //them dau '.' vao vi tri tuong ung voi scale
    AddPoint_to_StringDec(&Str_Data_Write, ScaleNum);
    
    //copy ca chuoi du lieu bao gom ca  ().
    if(startPos == 10)  //Neu la Tum va Tim   
        *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '/';
    else
        *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '(';
    
    for(i = 0; i < Str_Data_Write.Length_u16; i++)
      *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = *(Str_Data_Write.Data_a8 + i) ;
    
    if(startPos != 2) //neu khac Tu Ti thi c� ')'. C�n neu la Tu va Ti thi k co ')'
        *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = ')';
    
    Copy_String_2(&Get_Meter_TuTi.Str_Payload , &StrAdd_To_payload);
}





uint8_t ELSTER_Check_Meter(void)
{
    uint8_t Check_Found = 0;
    
    // Check bang phan mem. Check Error of Meter ID
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
			Read_Meter_ID_Success = ELSTER_Get_Meter_ID(0);
        }
	} 
    return 1;
}  




