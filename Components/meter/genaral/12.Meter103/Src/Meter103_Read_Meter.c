
#include "variable.h"
#include "t_mqtt.h"
#include "sim900.h"
#include "Meter103_Init_Meter.h"
#include "Meter103_Read_Meter.h"

#include "at_commands.h"
#include "pushdata.h"

#include "myDefine.h"

#define UNDER_COMP	0
#define OVER_COMP	1


/*------------------Var va struct--------------------*/
uint8_t 							aCmd103[256] = {0};         //buff chua byte gui lenh
Struct_Var_Meter103                 sMet103Var;
uint8_t                             aStimeBilling[20];
truct_String                        strStimeBill = {&aStimeBilling[0], 0};

/*--------------------Function------------------------*/
void METER103_Init_Function (uint8_t type)
{
    eMeter_20._f_Read_ID            = METER103_Get_Meter_ID;
    eMeter_20._f_Check_Reset_Meter  = METER103_CheckResetReadMeter;
  
    eMeter_20._f_Connect_Meter      = METER103_Connect_Meter;
    eMeter_20._f_Read_TSVH          = METER103_Read_TSVH;
    eMeter_20._f_Read_Bill          = METER103_Read_Bill;
    eMeter_20._f_Read_Event         = METER103_Read_Event;  
    eMeter_20._f_Read_Lpf           = METER103_Read_Lpf;
    eMeter_20._f_Read_InforMeter    = METER103_Read_Infor;
    eMeter_20._f_Get_UartData       = METER103_GetUART2Data;
    eMeter_20._f_Check_Meter        = METER103_Check_Meter;
    eMeter_20._f_Test1Cmd_Respond   = METER103_Send1Cmd_Test;
}

uint32_t Count_TestCMd1s = 0;

uint8_t METER103_Send1Cmd_Test (void)
{
    UART1_Control.Mode_ui8 = 1;
    uint8_t Result = 0;
    
//    ListMeterInfor[METER_TYPE_103].InitUartMeter(METER_TYPE_103);
//
//    METER103_IEC62056_21_Command (CMD_HANDSHAKE_103, NULL, 0);
//    
//    osSemaphoreWait(bsUART2PendingMessHandle, TIME_WAIT_SEM);
//    
//    if(UART1_Control.fRecei_Respond == 1) 
//    {
//        _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nTEST 1 Command OK\r\n", 21, 1000);
//        Result = 1;
//    }
    if(sMet103Var.Count_Error == 0)
        Result = 1;   
//    
//    Count_TestCMd1s++;
//    if(Count_TestCMd1s >= 2)  //2p ms vao. do phia ngoai 1p
//    {
//        Count_TestCMd1s = 0;
//        if(METER103_Handshake(0) == 1) 
//        {
//            _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nTEST 1 Command OK", 19, 1000);
//            Result = 1;
//            METER103_IEC62056_21_Command (CMD_LOGOUT_103, NULL, 0);
//        }
//    }else 
//        return 5;
    
    return Result;
}

uint8_t METER103_Read_TSVH (uint32_t Type)
{
    //Doc Tu TI truoc
    Get_Meter_Info.fTypeMessRead = Type;
    Init_Meter_TuTi_Struct();   
    if(METER103_Read(&Get_Meter_TuTi,&METER103_TuTi_ExtractDataFunc,&METER103_TuTi_SendData,&METER103_Send_TuTi_Mess,&METER103_MeterTuTi_IDTable[0]) != 1) 
    {
        Init_Meter_TuTi_Struct();
        if(METER103_Read(&Get_Meter_TuTi,&METER103_TuTi_ExtractDataFunc,&METER103_TuTi_SendData,&METER103_Send_TuTi_Mess,&METER103_MeterTuTi_IDTable[0]) != 1) 
          return 0;
    }
    //Doc data cac thanh ghi
    Init_Meter_Info_Struct();
    if(METER103_Read(&Get_Meter_Info,&METER103_MI_ExtractDataFunc,&METER103_MI_SendData,&METER103_Send_MInfo_Mess,&METER103_MeterInfoMessIDTable[0]) != 1)
    {
        Init_Meter_Info_Struct();
        if(METER103_Read(&Get_Meter_Info,&METER103_MI_ExtractDataFunc,&METER103_MI_SendData,&METER103_Send_MInfo_Mess,&METER103_MeterInfoMessIDTable[0]) != 1)
        {
            Init_Meter_Info_Struct();
            if(METER103_Read(&Get_Meter_Info,&METER103_MI_ExtractDataFunc,&METER103_MI_SendData,&METER103_Send_MInfo_Mess,&METER103_MeterInfoMessIDTable[0]) != 1) 
              return 0;
        }
    }
     
    return 1;
}

uint8_t METER103_Read_Bill (void)
{    
    sDCU.FlagHave_BillMes = 0;
    Init_Meter_TuTi_Struct();   
    if(METER103_Read(&Get_Meter_TuTi,&METER103_TuTi_ExtractDataFunc,&METER103_TuTi_SendData,&METER103_Send_TuTi_Mess,&METER103_MeterTuTi_IDTable[0]) != 1) 
        return 0;
    
    METER103_MBillInsertTimeReq();
    sMet103Var.SttLastBill = 0;
    while ((sMet103Var.SttLastBill < 12) &&(sInformation.Flag_Stop_ReadBill == 0))
    {
        sMet103Var.SttLastBill++;  //tang vi tri doc lastbill
        Init_Meter_Billing_Struct();
        METER103_Read(&Get_Meter_Billing,&METER103_MBill_ExtractDataFunc,&METER103_MBill_SendData,&METER103_Send_MBill_Mess,&METER103_MeterBillMessIDTable[0]);
        //k request chi doc cai moi nhat
        if(sInformation.Flag_Request_Bill == 0)  
            break;                      
    }
    //
    if(sDCU.FlagHave_BillMes == 0)
    {
        Packet_Empty_MessHistorical();
    }
    
    sInformation.Flag_Stop_ReadBill = 0;
    sInformation.Flag_Request_Bill = 0;
    return 1;
}

uint8_t METER103_Read_Event (uint32_t ForMin)
{
    //Doc Tu TI truoc
    Init_Meter_TuTi_Struct();   
    if(METER103_Read(&Get_Meter_TuTi,&METER103_TuTi_ExtractDataFunc,&METER103_TuTi_SendData,&METER103_Send_TuTi_Mess,&METER103_MeterTuTi_IDTable[0]) != 1) 
        return 0;
    //Doc data cac thanh ghi   
    Init_Meter_Event_Struct();   //Sua lai 9_8_21> truoc la infor
    if(METER103_Read(&Get_Meter_Event,&METER103_MEven_ExtractDataFunc,&METER103_MEven_SendData,&METER103_Send_MEven_Mess,&METER103_MeterEvenMessIDTable[0]) != 1)
    {
        Init_Meter_Event_Struct();
        if(METER103_Read(&Get_Meter_Event,&METER103_MEven_ExtractDataFunc,&METER103_MEven_SendData,&METER103_Send_MEven_Mess,&METER103_MeterEvenMessIDTable[0]) != 1)
        {
            Init_Meter_Event_Struct();
            if(METER103_Read(&Get_Meter_Event,&METER103_MEven_ExtractDataFunc,&METER103_MEven_SendData,&METER103_Send_MEven_Mess,&METER103_MeterEvenMessIDTable[0]) != 1) 
              return 0;
        }
    }
    
    return 1;
}

uint8_t METER103_Read_Lpf (void)
{
    uint8_t Result = 0;
    
    sDCU.FlagHave_ProfMess = 0;
    //Doc Tu TI truoc
    Init_Meter_TuTi_Struct();   
    if(METER103_Read(&Get_Meter_TuTi,&METER103_TuTi_ExtractDataFunc,&METER103_TuTi_SendData,&METER103_Send_TuTi_Mess,&METER103_MeterTuTi_IDTable[0]) != 1) 
        return 0;

    METER103_GET_TIME_ReadLPF();
    Init_Meter_LProf_Struct();
    Result = METER103_Read_Record(&Get_Meter_LProf,&METER103_MLpf_ExtractDataFunc,&METER103_MLpf_SendData,&METER103_Send_MLpf_Mess,&METER103_MeterLPFMessIDTable[0]);
     
    if(sDCU.FlagHave_ProfMess == 0)
        _fSend_Empty_Lpf();
    else
        _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nREAD LPF OK", 13, 1000);
      
    sInformation.Flag_Stop_ReadLpf = 0;
    sInformation.Flag_Request_lpf = 0;
    return Result;
}

uint8_t METER103_Read_Infor (void)
{
    Init_Meter_TuTi_Struct();   
    if(METER103_Read(&Get_Meter_TuTi,&METER103_TuTi_ExtractDataFunc,&METER103_TuTi_SendData,&METER103_Send_TuTi_Mess,&METER103_MeterTuTi_IDTable[0]) == 1) 
    {
        Pack_PushData_103_Infor_Meter();
        return 1;
    }
    return 0;
}
 
// Ban tin M�u:   <STX>0.0.C.1.0(19044152)<ETX><BCC>
uint8_t METER103_Get_Meter_ID (uint32_t Tempvalue)
{
    uint8_t         GetMeterIDRetry = 1, Result = 0;
	truct_String    ObisIDMeter = {(uint8_t*) "0.0.C.1.0", 9}; 
    
    uint16_t        i = 0, PosID = 0;
    uint8_t         ByteTemp = 0, LenID = 0;
    uint8_t         aMetID_Temp[METER_LENGTH];
    uint8_t         fMarkFirstZero = 0;
    
	while (GetMeterIDRetry--)
	{
		if (METER103_Handshake(0) == 1)
		{
			UART1_Control.Mode_ui8 = 0;    //chuyen uart sang nhan data
			//Send Read ID
			METER103_IEC62056_21_Command (CMD_READ_REGIS_103, ObisIDMeter.Data_a8, ObisIDMeter.Length_u16);
            
			if (osSemaphoreWait(bsUART2PendingMessHandle, TIME_WAIT_SEM) == osOK) 
            {
                Result = METER_CheckBBC_OBIS_Recei(ObisIDMeter);   //Neu tra ve 2: cat data, tra ve 1: khong cat data return 1. Neu tra ve 0 thi return 0
                if(Result == 2)
                {
                    PosID = 2 + ObisIDMeter.Length_u16;
                    //cat ID meter o day
                    ByteTemp = *(UART1_Control.UART1_Str_Recei.Data_a8 + PosID++);
                    while ((ByteTemp <= 0x39) && (ByteTemp >= 0x30))
                    {
                        if((ByteTemp != 0x30) || (fMarkFirstZero == 1))    //loai nhung so 0 o phia truoc
                        {
                            fMarkFirstZero = 1;
                            aMetID_Temp[LenID++] = ByteTemp;
                        }

                        ByteTemp = *(UART1_Control.UART1_Str_Recei.Data_a8 + PosID++);
                        if(LenID >= METER_LENGTH)
                          break;
                    }
                    _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nMeter ID: ", 12, 1000);
                    _fPrint_Via_Debug(&UART_SERIAL, &aMetID_Temp[0], LenID, 1000);
                    //Bo qua cac so 0 o dau tien
                    
                    //Check xem co phai thau doi ID meter khong
                    if (sDCU.sMeter_id_now.Length_u16 != LenID)
                        Read_Meter_ID_Change = 1;
                    else
                    {
                        for (i=0; i<LenID ;i++)
                        {
                            if ((*(sDCU.sMeter_id_now.Data_a8+i)) != aMetID_Temp[i])
                            {
                                Read_Meter_ID_Change = 1;
                                _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nChange Meter ID", 17, 1000);
                                break;
                            }
                        }
                    }
                
                    if(Read_Meter_ID_Change == 1)
                    {
                        Reset_Buff(&sDCU.sMeter_id_now);
                        for (i=0; i<LenID ;i++)
                            *(sDCU.sMeter_id_now.Data_a8+i) = aMetID_Temp[i];
                        sDCU.sMeter_id_now.Length_u16 = LenID;
                    }
                    _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nRead ID OK", 12, 1000);
                    METER103_IEC62056_21_Command (CMD_LOGOUT_103, NULL, 0);
                    osDelay(3000);
                    return 1;
                }
            }
            METER103_IEC62056_21_Command (CMD_LOGOUT_103, NULL, 0);
            osDelay(3000);
        }
    }
            
	return 0;
}

uint8_t METER103_CheckObisMeter (truct_String* Buff, uint16_t Pos, uint8_t* Obis, uint8_t LenObis)
{
    uint16_t i = 0;
    
    if(Buff->Length_u16 < (Pos + LenObis))
        return 0;
    for(i = 0; i < LenObis; i++)
      if(*(Obis + i) != *(Buff->Data_a8 + Pos + i))
        return 0;
    
    return 1;
}

uint8_t METER103_Connect_Meter(void)
{
	int8_t ReadIDRetry = 2;

	while (ReadIDRetry>0)
	{
        osDelay(500);
		if (osMutexWait(mtFlashMeterHandle,1000) == osOK)
		{
			Read_Meter_ID_Success = METER103_Get_Meter_ID(0);
            
			if (osMutexRelease(mtFlashMeterHandle) != osOK)
				osMutexRelease(mtFlashMeterHandle);
            
            if (Read_Meter_ID_Success == 1)
			break;
		
            ReadIDRetry--;
		}
	}
    if (Read_Meter_ID_Success == 1)
        return 1;
    else return 0;
}


uint8_t METER103_CheckResetReadMeter(uint32_t Timeout)
{
	// Reset neu doc sai cong to
    if ((sDCU.Status_Meter_u8 == 0) &&
            (Check_Time_Out(sDCU.LandMark_Count_Reset_Find_Meter,600000) == TRUE)) // 10p
    {
        osDelay(Timeout); // 5p
        Read_Meter_ID_Success = METER103_Get_Meter_ID(0);
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


uint8_t METER103_Check_Meter(void)
{
    uint8_t Check_Found = 0;
    
    // Check Error of Meter ID
	if (Read_Meter_ID_Success == 1) 
	{ 
        Check_Found = Get_Meter_Info.Error_Meter_Norespond_ui32 | Get_Meter_Billing.Error_Meter_Norespond_ui32 | Get_Meter_Event.Error_Meter_Norespond_ui32 |
					Get_Meter_LProf.Error_Meter_Norespond_ui32 | Get_Meter_Alert.Error_Meter_Norespond_ui32 ;
		// Not found Meter
		if ((Check_Found >= 2) || (sMet103Var.Count_Error >= 2))
		{ // Mat ket noi cong to - 5 lan doc loi
			sDCU.Status_Meter_u8 = 3; // Meter no Response
			Get_Meter_Info.Error_Meter_Norespond_ui32 		= 0;
			Get_Meter_Billing.Error_Meter_Norespond_ui32 	= 0;
			Get_Meter_Event.Error_Meter_Norespond_ui32  	= 0;
			Get_Meter_LProf.Error_Meter_Norespond_ui32 		= 0;
			Get_Meter_Alert.Error_Meter_Norespond_ui32		= 0;
            sMet103Var.Count_Error = 0;
			// Doc lai Cong to
			Read_Meter_ID_Success = METER103_Get_Meter_ID(0);
        }
	} 
    return 1;
}  

uint8_t METER103_GetUART2Data(void)
{
    uint8_t	temp_recieve = 0;

	switch (UART1_Control.Mode_ui8)
	{
		case 0: // data message
			if (UART1_Control.Mess_Pending_ui8 != 1)
			{
				temp_recieve = (uint8_t)(UART_METER.Instance->RDR&0x7F);
				
				if((temp_recieve == 0x02) || (UART1_Control.Flag_Have_0x02 != 0))    //bat dau ghi vai buff tu 0x02
				{
					UART1_Control.Flag_Have_0x02 = 1;  //DA NHAN DUOC KI TU BAT DAU LENH
					
					*(UART1_Control.UART1_Str_Recei.Data_a8 + UART1_Control.UART1_Str_Recei.Length_u16) = temp_recieve;
					UART1_Control.UART1_Str_Recei.Length_u16++;
                    
					if (UART1_Control.Mess_Pending_ui8 == 2)  //nhan BCC
					{
						UART1_Control.Mess_Pending_ui8 = 1;
						osSemaphoreRelease(bsUART2PendingMessHandle);
					}	
					//
					if (temp_recieve == ETX)// Ki tu ket thuc ETX hoac EOT
					{
						UART1_Control.Mess_Pending_ui8 = 2;
                        UART1_Control.fETX_EOT = 1;
					}else if(temp_recieve == EOT)
                    {
                        UART1_Control.Mess_Pending_ui8 = 2;
                        UART1_Control.fETX_EOT = 2;
                    }
		
					if (UART1_Control.UART1_Str_Recei.Length_u16 > MAX_LENGTH_BUFF_NHAN)	
							UART1_Control.UART1_Str_Recei.Length_u16 = 0;
				}
			}else
				temp_recieve = (uint8_t)(UART_METER.Instance->RDR&0x7F);
            
			UART1_Control.fRecei_Respond = 1;
			break;
		case 1: // handshake message
			if (UART1_Control.Mess_Pending_ui8 != 1)
			{
				temp_recieve = (uint8_t)(UART_METER.Instance->RDR&0x7F);
				
				*(UART1_Control.UART1_Str_Recei.Data_a8 + UART1_Control.UART1_Str_Recei.Length_u16) = temp_recieve;
			    UART1_Control.UART1_Str_Recei.Length_u16++;    //bien chung
				
				switch (sMet103Var.Step_HandShake)    
				{ 
					case _GET_MANUF:
						if (temp_recieve == 0x0A) // handshake 1
						{
							UART1_Control.Mess_Pending_ui8 = 1;
							osSemaphoreRelease(bsUART2PendingMessHandle);
						}
						break;
					case _CHECK_ACK:
						if (UART1_Control.Mess_Pending_ui8 == 2)  //nhan BCC
						{
							UART1_Control.Mess_Pending_ui8 = 1;
							osSemaphoreRelease(bsUART2PendingMessHandle);
						}	
                        //
						if (temp_recieve == ETX)// Ki tu ket thuc ETX hoac EOT
                        {
                            UART1_Control.Mess_Pending_ui8 = 2;
                            UART1_Control.fETX_EOT = 1;
                        }else if(temp_recieve == EOT)
                        {
                            UART1_Control.Mess_Pending_ui8 = 2;
                            UART1_Control.fETX_EOT = 2;
                        }
						break;
					case _CHECK_ACK_2:
						if (temp_recieve == ACK) //  - ACK
						{
							UART1_Control.Mess_Pending_ui8 = 1;
							osSemaphoreRelease(bsUART2PendingMessHandle);
						}	

						break;
                    case _STEP_END:
                        if (UART1_Control.Mess_Pending_ui8 == 2)  //nhan BCC
						{
							UART1_Control.Mess_Pending_ui8 = 1;
							osSemaphoreRelease(bsUART2PendingMessHandle);
						}
						if (temp_recieve == ETX)// Ki tu ket thuc ETX hoac EOT
                        {
                            UART1_Control.Mess_Pending_ui8 = 2;
                            UART1_Control.fETX_EOT = 1;
                        }else if(temp_recieve == EOT)
                        {
                            UART1_Control.Mess_Pending_ui8 = 2;
                            UART1_Control.fETX_EOT = 2;
                        }
                        break;
					default:
						break;
				}
			}
			else
				temp_recieve = (uint8_t)(UART_METER.Instance->RDR&0x7F);
            UART1_Control.fRecei_Respond = 1;
			break;
		default:
			break;
	}
    return 1;
}


/*
    ModeRW:
    R v� w can 2 pass khac nhau nen handshake se truyen vao mode
    R: 0
    W: 1
*/
uint8_t METER103_Handshake (uint8_t ModeRW)
{
    uint16_t            i = 0;
	uint8_t             Count_Step_End = 0;
    uint8_t             Reading = 0, TemU8 = 0;
    uint8_t             aManuFac[3]={0, 0, 0};    //3 ki tu   
    truct_String        strGelex = {(uint8_t*) "GELEX", 5};
//    truct_String        strHHM = {(uint8_t*) "HHM", 3};
    
    UART1_Control.Mode_ui8 = 1;
    sMet103Var.Step_HandShake = _HANDSHAKE;
  
	while (Reading == 0)  
	{
		switch (sMet103Var.Step_HandShake)        
		{
			case _HANDSHAKE:	 
                // Init lai uart sang 300
                __HAL_UART_DISABLE_IT(&UART_METER, UART_IT_RXNE);
                ListMeterInfor[METER_TYPE_103].InitUartMeter(METER_TYPE_103);
                __HAL_UART_ENABLE_IT(&UART_METER, UART_IT_RXNE);
				_fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nHandshake 103: ", 17, 1000);
                sMet103Var.Delay485 = TIM_ACTIVE485;
				sMet103Var.Step_HandShake++;
				METER103_IEC62056_21_Command (CMD_HANDSHAKE_103, NULL, 0);
				break;
			case _GET_MANUF:   //check ACK 1 tu cong to ra
				if (osSemaphoreWait(bsUART2PendingMessHandle, TIME_WAIT_SEM) == osOK) 
                {
                    _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "OK", 2, 1000);
                    sMet103Var.Step_HandShake++;
					//Check chuoi nhan ve
					if(UART1_Control.UART1_Str_Recei.Length_u16 < 5) 
                        return 0;
                    for(i = 0; i < UART1_Control.UART1_Str_Recei.Length_u16; i++)
                        if(*(UART1_Control.UART1_Str_Recei.Data_a8 + i) == '/')   //Tim den ki tu bat dau chuoi nhan Ki tu nha san xua
                            break; 
                    i++;
                    aManuFac[0] = *(UART1_Control.UART1_Str_Recei.Data_a8 + i++);
                    aManuFac[1] = *(UART1_Control.UART1_Str_Recei.Data_a8 + i++);
                    aManuFac[2] = *(UART1_Control.UART1_Str_Recei.Data_a8 + i++);
                    TemU8 = *(UART1_Control.UART1_Str_Recei.Data_a8 + i++);
                    if((TemU8 <= 0x36) && (TemU8 >= 0x30))
                        sMet103Var.BaudRec = TemU8;    //Thong so baud request
                    else 
                    {
                        sMet103Var.Step_HandShake = _STEP_END;
                        break;
                    }
                    i++; //'\'
                    sMet103Var.CharSpecMan = *(UART1_Control.UART1_Str_Recei.Data_a8 + i++);   //ki tu dac bi�t nha sx
                    //check nha san xuat
					if((aManuFac[0] == MET103_GELEX_MANUFAC[0]) && (aManuFac[1] == MET103_GELEX_MANUFAC[1]) && 
                       (aManuFac[2] == MET103_GELEX_MANUFAC[2])) {   //  
                        sMet103Var.Mettype103 = 1;  //Gelex
					}else if ((aManuFac[0] == MET103_HH_MANUFAC[0]) && (aManuFac[1] == MET103_HH_MANUFAC[1]) && 
                              (aManuFac[2] == MET103_HH_MANUFAC[2])) {
                        sMet103Var.Mettype103 = 2;  //Huu hong
                    }else if ((aManuFac[0] == MET103_VSE_MANUFAC[0]) && (aManuFac[1] == MET103_VSE_MANUFAC[1]) && 
                              (aManuFac[2] == MET103_VSE_MANUFAC[2])) {
                        sMet103Var.Mettype103 = 3;          //Vinasino moi
                    }else 
                        sMet103Var.Step_HandShake = _STEP_END;
				} else   //bat tay k dc thi thu log o toc do 4800
                {
                    _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "Fail", 4, 1000);
                    if(Count_Step_End == 1)   //lan dau tien cau hinh ve 4800 de logout. lan sau thi de nguyen 300
                    {
                        _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nReInit Uart Baud 4800", 23, 1000);
                        METER103_Init_UART_GET_DATA(METER_TYPE_103, 4800);
                    }else
                    {
                        _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nReInit Uart Baud 300", 22, 1000);
                        ListMeterInfor[METER_TYPE_103].InitUartMeter(METER_TYPE_103);
                    }
                    sMet103Var.Step_HandShake = _STEP_END;	
                }
				break;
			case _REQUETBAUD:	
                Count_Step_End++;  //den buoc nay ma fail thi chi thu lai 1 lan thoi
				sMet103Var.Step_HandShake++;
				//send ACK + Baurate
                _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nReInit Uart Baud Meter", 24, 1000);
				METER103_IEC62056_21_Command (CMD_REQ_BAUD_103, NULL, 0); 
                
                __HAL_UART_DISABLE_IT(&UART_METER, UART_IT_RXNE);
                METER103_Init_UART_GET_DATA(METER_TYPE_103, 0xFFFF);                 //cau hinh sang Baudrate moi de nhan du lieu
                __HAL_UART_ENABLE_IT(&UART_METER, UART_IT_RXNE);
                
                sMet103Var.Delay485 = TIM_ACTIVE485_4K8;
				break;
			case _CHECK_ACK: 
                if (osSemaphoreWait(bsUART2PendingMessHandle, TIME_WAIT_SEM) == osOK) 
                {
                    sMet103Var.Step_HandShake ++;
                    sMet103Var.Step_HandShake += ModeRW;   //Neu write se nhay xuong buoc write
					//Check chuoi nhan ve
					if(UART1_Control.UART1_Str_Recei.Length_u16 < 5) 
                      return 0;
                    // Hinh nhu day la key m� h�a thi phai
					if(sMet103Var.Mettype103 == 1)
                    {
                        if(Find_String_V2(&strGelex, &UART1_Control.UART1_Str_Recei) < 0)
                            sMet103Var.Step_HandShake = _STEP_END;
                    }
//                    else if(sMet103Var.Mettype103 == 2)
//                    {
//                        if(Find_String_V2(&strHHM, &UART1_Control.UART1_Str_Recei) < 0)
//                            sMet103Var.Step_HandShake = _STEP_END;
//                    }
				} else 
                {
                    _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nRequest Baud Fail", 19, 1000);
//                    ListMeterInfor[METER_TYPE_103].InitUartMeter(METER_TYPE_103);  //cau hinh tro lai 300 de out
                    sMet103Var.Step_HandShake = _STEP_END;
                }
				break;
			case _SENDPASS_READ:	
                _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nSend Pass", 11, 1000);
				sMet103Var.Step_HandShake = _CHECK_ACK_2;    
				METER103_IEC62056_21_Command (CMD_PASS_READ_103, NULL, 0);
				break;
            case _SENDPASS_WRITE:	
                _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nSend Pass", 11, 1000);
				sMet103Var.Step_HandShake = _CHECK_ACK_2;
				METER103_IEC62056_21_Command (CMD_PASS_WRITE_103, NULL, 0);
				break;
			case _CHECK_ACK_2:
				if (osSemaphoreWait(bsUART2PendingMessHandle, TIME_WAIT_SEM) == osOK) 
                {
                    _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nHandshake OK", 14, 1000);
                    sMet103Var.Count_Error = 0;
					Reading = 1;
                    UART1_Control.Mode_ui8 = 0;   //chuyen sang che do doc du lieu
				}else    //Logout o toc do 300 hay 4800 ?
                {
                    _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nReInit Uart Baud 300 CHECKACK2", 32, 1000);
                    ListMeterInfor[METER_TYPE_103].InitUartMeter(METER_TYPE_103);
                    sMet103Var.Step_HandShake = _STEP_END;
                }
				break;
			case _STEP_END:
                Count_Step_End++;
                _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nLog Out", 9, 1000);
				METER103_IEC62056_21_Command (CMD_LOGOUT_103, NULL, 0);
                osDelay(5000);
                
				if(Count_Step_End <= 1)
					sMet103Var.Step_HandShake = _HANDSHAKE;
				else 
                {
                    sMet103Var.Count_Error++;
					return 0;
                }
				break;				
			default:
                    return 0;
				break;		
		}
	}
	return 1;
}


void METER103_IEC62056_21_Command (uint8_t Kind_Cmd, uint8_t* BuffSend, uint16_t lengthSend)
{
	uint16_t i=0;
	uint8_t length = 0;
	uint8_t Temp_BBC = 0;
	
	switch(Kind_Cmd)
	{
		case CMD_HANDSHAKE_103:
            aCmd103[length++]=0xFF;
            aCmd103[length++]=0xFF;
            aCmd103[length++]=0xFF;
			for(i = 0; i< sizeof(MET103_Handshake); i++)
			{	
				aCmd103[length++] = MET103_Handshake[i];
			}
			break;
		case CMD_REQ_BAUD_103: 
            //Thay the byte baud trong ban tin bat tay nhan ve
            MET103_REQUEST_BAUD[2] = sMet103Var.BaudRec;
            //
            aCmd103[length++]=0xFF;
            aCmd103[length++]=0xFF;
            aCmd103[length++]=0xFF;
			for(i = 0; i< sizeof(MET103_REQUEST_BAUD); i++)
			{	
				aCmd103[length++] = MET103_REQUEST_BAUD[i];
			}
			break;
		case CMD_PASS_READ_103: 
			for(i = 0; i< sizeof(MET103_SEND_PASS); i++)
				aCmd103[length++] = MET103_SEND_PASS[i];
            //Thay Pass v�o
            if(sMet103Var.Mettype103 == 1)  //GELEX
                for(i = 0; i < 8; i++)
                  aCmd103[5+i] = METER103_PASS_GEL[i]; 
            else if(sMet103Var.Mettype103 == 2)  //HHM
                for(i = 0; i < 8; i++)
                  aCmd103[5+i] = METER103_PASS_HHM[i]; 
            else if(sMet103Var.Mettype103 == 3)  //VSE
                for(i = 0; i < 8; i++)
                  aCmd103[5+i] = METER103_PASS_VSE[i]; 
            
            //Tinh bbc thay vao byte thu 16
            Temp_BBC = BBC_Cacul(&aCmd103[1], 14);
            aCmd103[15] = Temp_BBC;
			break;  
        case CMD_PASS_WRITE_103: 
			for(i = 0; i< sizeof(MET103_SEND_PASS); i++)
				aCmd103[length++] = MET103_SEND_PASS[i];
            aCmd103[2] = 0x33;    //P3
            //Thay Pass v�o
            if(sMet103Var.Mettype103 == 1)  //GELEX
                for(i = 0; i < 8; i++)
                  aCmd103[5+i] = METER103_PASS_GEL[i]; 
            else if(sMet103Var.Mettype103 == 2)  //HHM
                for(i = 0; i < 8; i++)
                  aCmd103[5+i] = METER103_PASS_WRITE_HH[i]; 
            else if(sMet103Var.Mettype103 == 3)  //VSE
                for(i = 0; i < 8; i++)
                  aCmd103[5+i] = METER103_PASS_VSE[i]; 
            
            //Tinh bbc thay vao byte thu 16
            Temp_BBC = BBC_Cacul(&aCmd103[1], 14);
            aCmd103[15] = Temp_BBC;
			break; 
        case CMD_ACK_103: 
            aCmd103[length++] = 0x06;
			break;  
		case CMD_READ_REGIS_103:        //01 52 31 02 28 29 03 63 	<SOH>R1<STX> Obis ()<ETX><BCC>
			aCmd103[length++] = SOH;
			aCmd103[length++] = 'R';
			aCmd103[length++] = '1';
			aCmd103[length++] = STX;
			for(i = 0; i< lengthSend; i++)    //them Obis can doc
			{	
				aCmd103[length++] = *(BuffSend + i);
			}
            aCmd103[length++] = '(';
            aCmd103[length++] = ')';
			aCmd103[length++] = ETX;
			//them BBC vao
			Temp_BBC = BBC_Cacul(&aCmd103[1], (length-1));    //lenh nay BBC bang Xor tu 'P' cho den ETX
			aCmd103[length++] = Temp_BBC;
			break; 
        case CMD_READ_RECORD1_103:   //Doc lpf.
            aCmd103[length++] = SOH;
            aCmd103[length++] = 'R';
			aCmd103[length++] = '5';
            aCmd103[length++] = STX;
            for(i = 0; i< lengthSend; i++)    //them Obis can doc
			{	
				aCmd103[length++] = *(BuffSend + i);
			}
            aCmd103[length++] = ETX;
			//them BBC vao
			Temp_BBC = BBC_Cacul(&aCmd103[1], (length-1));    //lenh nay BBC bang Xor tu 'P' cho den ETX
			aCmd103[length++] = Temp_BBC;
			break;      
		case CMD_WRITE_REGIS_103: 
			aCmd103[length++] = SOH;
			aCmd103[length++] = 'W';
			aCmd103[length++] = '1';
			aCmd103[length++] = STX;
			for(i = 0; i< lengthSend; i++)   //them obis can ghi
				aCmd103[length++] = *(BuffSend + i);
			aCmd103[length++] = ETX;
			//them BBC vao
			Temp_BBC = BBC_Cacul(&aCmd103[1], (length-1));    //lenh nay BBC bang Xor tu 'P' cho den ETX
			aCmd103[length++] = Temp_BBC;
			break; 
		case CMD_LOGOUT_103:    
			// SOH B0 STX ETX BCC
			aCmd103[length++] = SOH;
			aCmd103[length++] = 'B';
			aCmd103[length++] = '0';
			aCmd103[length++] = ETX;
			//them BBC vao
			Temp_BBC = BBC_Cacul(&aCmd103[1], (length-1));    //lenh nay BBC bang Xor tu 'P' cho den ETX
			aCmd103[length++] = Temp_BBC;
            break;
		default: 
			break;
	}
    RS485_SEND;
    osDelay(sMet103Var.Delay485);
    Init_UART2();
    HAL_UART_Transmit(&UART_METER,&aCmd103[0], length, 1000);
    RS485_RECIEVE;
}




void METER103_Init_UART_GET_DATA (uint8_t type, uint16_t Fixbaud)
{
    if(Fixbaud == 0xFFFF)
    {
        switch(sMet103Var.BaudRec - 0x30)
        {
            case 0:
                UART_METER.Init.BaudRate = 300;
                break;
            case 1:
                UART_METER.Init.BaudRate = 600;
                break;
            case 2:
                UART_METER.Init.BaudRate = 1200;
                break;
            case 3:
                UART_METER.Init.BaudRate = 2400;
                break;
            case 4:
                UART_METER.Init.BaudRate = 4800;
                break;
            case 5:
                UART_METER.Init.BaudRate = 9600;
                break;
            case 6:
                UART_METER.Init.BaudRate = 19200;
                break;
        }
    }else UART_METER.Init.BaudRate = Fixbaud;
    
    UART_METER.Instance = UART__METER;
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


//Viet chuong trinh doc TuTi
//1.0.0.4.3(1*V)(1.0*V)
uint8_t METER103_TuTi_ExtractDataFunc (void)
{
    truct_String     strObis = {NULL, 0};
    uint8_t          RowCheck = 0;
    uint16_t         PosFind = 0;
    uint8_t          Result = 0;
     
    RowCheck = METER103_MeterTuTi_IDTable[Get_Meter_TuTi.Mess_Step_ui8];   //Lay hang cua obis trong struct de check data
    if(RowCheck >= MAX_OBIS_TUTI)   //qu� so dong t�i da cua list obis
        return 0;
    strObis.Data_a8 = METER103_TUTI_ObisCode[RowCheck].Data_a8;
    strObis.Length_u16 = METER103_TUTI_ObisCode[RowCheck].Length_u16;
      
    Result = METER_CheckBBC_OBIS_Recei(strObis);   //Neu tra ve 2: cat data, tra ve 1 hoac 0: cho Tu Ti ve mac dinh return 1
    if(Result == 2)
    {
        Result = 1;
        PosFind = 1 + strObis.Length_u16;
        //Cat ban tin data
        METER103_Decode_TuTi(PosFind);
    }else 
    {
        *(Get_Meter_TuTi.Str_Payload.Data_a8 + Get_Meter_TuTi.Str_Payload.Length_u16++) = '(';
        *(Get_Meter_TuTi.Str_Payload.Data_a8 + Get_Meter_TuTi.Str_Payload.Length_u16++) = '1';
        *(Get_Meter_TuTi.Str_Payload.Data_a8 + Get_Meter_TuTi.Str_Payload.Length_u16++) = '/';
        *(Get_Meter_TuTi.Str_Payload.Data_a8 + Get_Meter_TuTi.Str_Payload.Length_u16++) = '1';
        *(Get_Meter_TuTi.Str_Payload.Data_a8 + Get_Meter_TuTi.Str_Payload.Length_u16++) = ')';
    }

    return 1;
}

/*
    - Func check BBC va Obis
    - Tra ve 0 Neu sai BBC hoac sai obis
    - Tra ve 1 neu co check dung va khong co data
    - Tra ve 2 neu check dung va c� data
*/

uint8_t METER_CheckBBC_OBIS_Recei (truct_String ObisCheck)
{
    uint16_t         PosFind = 0;
    uint8_t          aTemp[2] = {0};
    
    //Check BBC
    if(Check_BBC(&UART1_Control.UART1_Str_Recei) == 1)  
    {
        if(METER103_CheckObisMeter( &UART1_Control.UART1_Str_Recei, 1, ObisCheck.Data_a8, ObisCheck.Length_u16) == 1)  //check obis code meter
        {
            PosFind = 1 + ObisCheck.Length_u16;   //tro vao ki tu tiep theo sau obis " thuong la (
            //Check xem la ban tin r�ng, hay ban tin loi, hay ban tin data. chi cat tiep ban tin data con lai return 0;
            aTemp[0] = * (UART1_Control.UART1_Str_Recei.Data_a8 + PosFind);
            aTemp[1] = * (UART1_Control.UART1_Str_Recei.Data_a8 + PosFind + 1);   //Posfind tro den vi tri  (
            //
            if((aTemp[0] == '(') && (aTemp[1] == ')'))
            {
                _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nNo data in Register", 21, 1000);
                _fPrint_Via_Debug(&UART_SERIAL, UART1_Control.UART1_Str_Recei.Data_a8, UART1_Control.UART1_Str_Recei.Length_u16, 1000);
                return 1;
            }
            if((aTemp[0] == '(') && ((aTemp[1] > '9') || (aTemp[1] < '0')))   //ban tin loi se tiep theo ( la chu
            {
                _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nDoc thanh ghi loi", 19, 1000);
                _fPrint_Via_Debug(&UART_SERIAL, UART1_Control.UART1_Str_Recei.Data_a8, UART1_Control.UART1_Str_Recei.Length_u16, 1000);
                return 1;
            }
            
            return 2;
        }else{
            _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nMeter103 Read Sai Obis", 24, 1000);
            _fPrint_Via_Debug(&UART_SERIAL, UART1_Control.UART1_Str_Recei.Data_a8, UART1_Control.UART1_Str_Recei.Length_u16, 1000);
        }
    }else{
        _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nMeter103 Read Sai BBC", 23, 1000);
        _fPrint_Via_Debug(&UART_SERIAL, UART1_Control.UART1_Str_Recei.Data_a8, UART1_Control.UART1_Str_Recei.Length_u16, 1000);
    }
    
    return 0;
}

void METER103_Decode_TuTi (uint16_t PosStart)   //bat buoc doc Tu truoc
{
    uint16_t        Pos = PosStart;    
    uint8_t         aDataTuTi[MAX_LENGTH_BYTE_RECEI_METER] = {0};
    truct_String    strData = {&aDataTuTi[0], 0};
    uint8_t         TemHex = 0;    
    //1.0.0.4.3(1*V)(1.0*V)
    while (*(UART1_Control.UART1_Str_Recei.Data_a8 + Pos) != '*')
    {
        *(strData.Data_a8 + strData.Length_u16++) = *(UART1_Control.UART1_Str_Recei.Data_a8 + Pos);
        if(strData.Length_u16 >= MAX_LENGTH_BYTE_RECEI_METER)    //Data se khong qua MAX_LENGTH
            return;
        Pos++;
    }
    //
    *(strData.Data_a8 + strData.Length_u16++) = '/';
    //1.0.0.4.3(1*V)(1.0*V)
    do
    {
        Pos++;
        if(Pos == (UART1_Control.UART1_Str_Recei.Length_u16 - 1))
          return;
        TemHex = *(UART1_Control.UART1_Str_Recei.Data_a8 + Pos);
    }while((TemHex < 0x30) || (TemHex > 0x39));    //Check den vi tri co so tiep theo
    //Cat tiep Tum hoac tim
    while (*(UART1_Control.UART1_Str_Recei.Data_a8 + Pos) != '*')
    {
        *(strData.Data_a8 + strData.Length_u16++) = *(UART1_Control.UART1_Str_Recei.Data_a8 + Pos);
        if(strData.Length_u16 >= MAX_LENGTH_BYTE_RECEI_METER)    //Data se khong qua MAX_LENGTH
            return;
        Pos++;
    }
    if(Get_Meter_TuTi.Flag_Start_Pack ==0)
    {
        Reset_Buff(&Get_Meter_TuTi.Str_Payload);
        Get_Meter_TuTi.Flag_Start_Pack = 1;
    }
    *(strData.Data_a8 + strData.Length_u16++) = ')';
    //Ghi obis va data vao buff tuti. Sau do them he so nhan o ngoai
    Copy_String_2(&Get_Meter_TuTi.Str_Payload , &strData);
}


void METER103_TuTi_SendData(void)
{
    *(Get_Meter_TuTi.Str_Payload.Data_a8 + Get_Meter_TuTi.Str_Payload.Length_u16++) = '(';
    Pack_HEXData_Frame_Uint64(&Get_Meter_TuTi.Str_Payload, (uint64_t) sDCU.He_So_Nhan, 0); 
    *(Get_Meter_TuTi.Str_Payload.Data_a8 + Get_Meter_TuTi.Str_Payload.Length_u16++) = ')';
    _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nREAD TuTi OK", 14, 1000);
}

void METER103_Send_TuTi_Mess (void)
{
    uint8_t temp=0;
	
	temp = METER103_MeterTuTi_IDTable[Get_Meter_TuTi.Mess_Step_ui8];

    METER103_IEC62056_21_Command (CMD_READ_REGIS_103, METER103_TUTI_ObisCode[temp].Data_a8, METER103_TUTI_ObisCode[temp].Length_u16); 
}


//Viet chuong trinh doc TSVH v� INTAN
uint8_t METER103_Read (Meter_Comm_Struct *Meter_Comm, uint8_t (*FuncExtractRawData)(void),void (*FuncDataHandle)(void),void (*FuncSendMess)(void),uint8_t *CodeTable)
{
	uint8_t mReVal = 0;
    
    if(METER103_Handshake(0) != 1) 
        return 0;
    
    Meter_Comm->Reading_ui8 = 1;
    UART1_Control.Mode_ui8 = 0;
    
	while(Meter_Comm->Reading_ui8 == 1) 
	{
		switch(Meter_Comm->Step_ui8) 
		{
			case 0:
				Meter_Comm->Total_Mess_Sent_ui32++;
				//Wait for respond message from Meter in 2000ms: checksum - extract raw data - save to buffer - goto next message
				if (osSemaphoreWait(bsUART2PendingMessHandle, TIME_WAIT_SEM) == osOK) 
                {
                    Meter_Comm->Success_Read_Mess_ui32++;
                    Meter_Comm->Step_ui8 = 1;
                    if(UART1_Control.fETX_EOT != 2)  //Doi voi Even neu tra ra EOT thi khong xu ly data. m� doc tiep cho den khi lay data cuoi
                    {
                        //Extract raw data
                        if(FuncExtractRawData() == 1)   //Check v� cat data
                        {
                            //Goto next message
                            Meter_Comm->Mess_Step_ui8++;
                            Meter_Comm->Error_ui8 = 0;   
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
                        }else
                            Meter_Comm->Step_ui8 = 2;
                    }  //Doc lai obis luc nay cho den khi nhan duoc ETX thi thoi
				} else 
                {
                    _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nNo Respond Meter", 18, 1000);
                    //If Meter no respond after 2000ms: wait 2000ms - resend the message (3 times) 
					Meter_Comm->Error_Meter_Norespond_ui32++;   //thay bien dem loi nay bang bien khac de check retry
					Meter_Comm->Step_ui8 = 2;
				}
				break;
			case 1:				
				FuncSendMess();
				Meter_Comm->Step_ui8 = 0;
				break;
			case 2:
                METER103_IEC62056_21_Command (CMD_LOGOUT_103, NULL, 0);
                osDelay(3000);
                //Jump out of reading cycle, wait for next cycle (after 30mins)
				Meter_Comm->Reading_ui8 = 0;
				Meter_Comm->Step_ui8 = 1;
				Meter_Comm->Mess_Step_ui8 = 0;
				break;					
			default:
				break;				
		}
	}
	return mReVal;	
}



uint8_t METER103_MI_ExtractDataFunc (void)
{
    truct_String     strObis = {NULL, 0};
    uint8_t          RowCheck = 0, Result = 0;
    uint16_t         PosFind = 0;
    uint8_t          TypeDecode = 0;
    uint8_t          aData[MAX_LENGTH_BYTE_RECEI_METER] = {0};
    truct_String     strData = {&aData[0], 0};
    
    RowCheck = METER103_MeterInfoMessIDTable[Get_Meter_Info.Mess_Step_ui8];   //Lay hang cua obis trong struct de check data
    if(RowCheck >= METER103_MAX_OBIS_TSVH)   //qu� so dong t�i da cua list obis
        return 0;
    strObis.Data_a8 = METER103_TSVH_ObisCode[RowCheck].Data_a8;
    strObis.Length_u16 = METER103_TSVH_ObisCode[RowCheck].Length_u16;
      
    Result = METER_CheckBBC_OBIS_Recei(strObis);   //Neu tra ve 2: cat data, tra ve 1: khong cat data return 1. Neu tra ve 0 thi return 0
    if(Result == 2)
    {
        Result = 1;
        PosFind = 1 + strObis.Length_u16;
        //Cat ban tin data
        TypeDecode = METER103_Minfor_TypeDecode(RowCheck);
        //
        switch (TypeDecode)  //cat them dung obis cua hang do
        {
            case 0://Meter time: chi lay meter time vao buff
                if(METER103_DecodeData_1Obis1Data (0, PosFind, &strData) == 1)
                    METER103_PacketData_TSVH(0, strObis, strData);
                break;
            case 1://Du lieu chi 1obis va data. chi can cat vao dua vao buff : thanh ghi
                if(METER103_DecodeData_1Obis1Data (1, PosFind, &strData) == 1)
                    METER103_PacketData_TSVH(1, strObis, strData);
                break;
            case 2://Du lieu chi 1obis va data v� time: Maxdemand
                if(METER103_DecodeData_1Obis1Data (2, PosFind, &strData) == 1)
                    METER103_PacketData_TSVH(2, strObis, strData);
                break;
            case 3://Du lieu can cat ra thanh 2 obis va 2 data
                METER103_DecodeData_Event_TSVH (RowCheck, PosFind, &Get_Meter_Info);
                break;
            default:
                return 0;
        }
    }
    
    return Result;
}

uint8_t METER103_Minfor_TypeDecode (uint8_t row)
{
    uint8_t  result = 0;
    
    if(row == 0)
        result = 0;
    else if ((row >= TSVH_POS_REGIS_INTAN) && (row < TSVH_POS_MAXDEMAND))
        result = 1;
    else if ((row >= TSVH_POS_MAXDEMAND) && (row < TSVH_POS_EVEN_2OBIS))
        result = 2;
    else if ((row >= TSVH_POS_EVEN_2OBIS) && (row < TSVH_POS_EVEN_END))
        result = 3;
    else 
        result = 0xFF;
    
    return result;
}
 
/*
     Ham cat du lieu bao gom 1 obis va 1 data. Chi can cat va dua vao buff chinh Minfor
     Tham so dua vao:   - Obis cua data theo quy dinh DCU 103EVN
                        - Vi tri dau tien cua data sau  '('
                        - Type: 0 Chi lay data (metertime)  1: lay obis  +data ;   2: Maxdemand: 1 obis, 1data + time

    C� 2 TH:        -Thanh ghi bt
                    -MAXDEMAND : Obis + Data +TIME
*/

uint8_t METER103_DecodeData_1Obis1Data (uint8_t type, uint16_t PosStart, truct_String* aData)
{
    uint16_t        lengdata = 0, i = 0;
    uint16_t        Pos = PosStart;
    truct_String    strData = {NULL, 0};
    uint8_t         TotalCheck = 0;
    
    if(type == 2) 
        TotalCheck = 2;
    else TotalCheck = 1;
    //Cat data. Dem xem length data la bao nhieu. Ti nua tro con tro vao start va length =  length tinh duoc
    for(i = 0; i < TotalCheck; i++)
        do 
        {
            lengdata++;
            if(lengdata >= MAX_LENGTH_BYTE_RECEI_METER)    //Data se khong qua MAX_LENGTH
                return 0;
        }while (*(UART1_Control.UART1_Str_Recei.Data_a8 + Pos++) != ')');
                 
    //Tro data
    strData.Data_a8     = UART1_Control.UART1_Str_Recei.Data_a8 + PosStart;
    strData.Length_u16  = lengdata;
    
    for(i = 0; i < strData.Length_u16; i++)
        *(aData->Data_a8 + aData->Length_u16++) = *(strData.Data_a8 + i);
    
    return 1;
}

void METER103_PacketData_TSVH (uint8_t type, truct_String Obis, truct_String strData)
{
    uint8_t         aObisTemp[20] = {0};
    uint16_t        i= 0;
    truct_String    strObis = {&aObisTemp[0], 0};
    
    //Ghi obis va data vao buff chinh theo kieu dong goi 103
    if(Get_Meter_Info.Flag_Start_Pack == 0)
    {
        Reset_Buff(&Get_Meter_Info.Str_Payload);
        //wrire header TSVH vao
        Write_Header_TSVH_Push103();
        Get_Meter_Info.Flag_Start_Pack = 1;
    }  
    if(type != 0)
    {
        //Them  () vao obis g�c.
        *(strObis.Data_a8 + strObis.Length_u16++) = '(';
        for(i = 4; i < Obis.Length_u16; i++)          //bo qua 0.0. v� 1.0.
            *(strObis.Data_a8 + strObis.Length_u16++) = *(Obis.Data_a8 + i);
        *(strObis.Data_a8 + strObis.Length_u16++) = ')';
        ////ghi obis va data vao
        Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Obis_Inbuff, &strObis);
        Get_Meter_Info.Pos_Obis_Inbuff += strObis.Length_u16;
        Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
        //Tang tong so qty len
        Get_Meter_Info.Numqty ++;
        //ghi data vao
        Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Data_Inbuff, &strData);
        Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
    }else   //Ghi thoi gian vao va ket thuc ban tin TSVH
    {
        Get_Meter_Info.Flag_ui8 = 1;   //muon flag nay de bao doc thanh cong
        //them tuti
        Add_TuTI_toPayload(&Get_Meter_Info);
        //Ghi stime. Nhung bo 2byte sec di: (210519151733)   bo 33 di
        for(i = 0; i < (strData.Length_u16 - 3); i++)   
            *(strObis.Data_a8 + strObis.Length_u16++) = *(strData.Data_a8 + i);
        *(strObis.Data_a8 + strObis.Length_u16++) = ')';
        //
        Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.PosNumqty, &strObis);
        Get_Meter_Info.PosNumqty += strObis.Length_u16;
        //ghi ca qty vao
        Reset_Buff(&strObis);
        *(strObis.Data_a8 + strObis.Length_u16++) = '('; 
        Pack_HEXData_Frame_Uint64(&strObis, (uint64_t) Get_Meter_Info.Numqty, 0);
        *(strObis.Data_a8 + strObis.Length_u16++) = ')'; 
        
        Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.PosNumqty, &strObis);  //lay tam struct string nay gan tam
    }
}


/*
     Ham cat du lieu bao gom 1 obis va 2 data. Sau do phai tach ra 2 obis va 2 data
     Tham so dua vao:   - Obis1
                        - Obis2
                        - Start Data 1
    Neu nhu tham so nao k co data thi se khong co :data2 m� chi co obis (data1).
*/
void METER103_DecodeData_2Obis2Data (truct_String Obis1, truct_String Obis2, uint16_t PosStart1, Meter_Comm_Struct* sGet_Meter_Data)
{
    uint8_t         lengdata1 = 0, lengdata2 = 0;
    uint16_t        i = 0;
    uint8_t         Check = 0;
    uint16_t        Pos1 = PosStart1;
    uint16_t        PosStart2 = PosStart1;
    uint8_t         aData1[15]= {0};
    uint8_t         aData2[15]= {0};
    truct_String    strData1 = {&aData1[0], 0};
    truct_String    strData2 = {&aData2[0], 0};
    uint8_t         aObisTemp[20] = {0};
    truct_String    strObis = {&aObisTemp[0], 0};
    
    //Cat data. Dem xem length data la bao nhieu. Ti nua tro con tro vao start va length =  length tinh duoc
    //Con Gelex se sap x�p moi dan o ph�a sau. Trong khi con huu hong thi sap xep moi nhat o phia truoc
    do
    {
        do //bat dau tu (
        {
            if(*(UART1_Control.UART1_Str_Recei.Data_a8 + Pos1) == ':')   //ket thuc data1 
            {
                Check = 1;  //check data2
                PosStart2 = Pos1 + 1;  //vi tri bat dau obis thu 2
            }else
            {
                if(Check == 0) //check data1
                    lengdata1++;
                else lengdata2++;

                if((lengdata1 >= 15) || (lengdata2 >= 15))    //Data se khong qua MAX_LENGTH
                    return;
            }
        }while (*(UART1_Control.UART1_Str_Recei.Data_a8 + Pos1++) != ')');
        //Check xem co phai data cuoi c�ng chua
        if((*(UART1_Control.UART1_Str_Recei.Data_a8 + Pos1 + 2) != '(') || (sDCU.MeterType == METER_HHM_103) || (sDCU.MeterType == METER_VSE_103))  //Neu con dadtaa tiep thi vi tri +2 se la ( . Hoac day la cong to huu hong
            break;
        else
        {
            Pos1 += 2;  //tro den vi tri (;
            //Reset cac vi tri cat data va length de cat lai 
            lengdata1 = 0;
            lengdata2 = 0;
            Check = 0;
            PosStart1 = Pos1;
            PosStart2 = PosStart1;
        }
    }while (Pos1 < UART1_Control.UART1_Str_Recei.Length_u16);
    
    //Tro data
    if(lengdata2 != 0)   //co data 2: 
    {
        for(i = 0; i< lengdata1; i++)
            *(strData1.Data_a8 + strData1.Length_u16++) = *(UART1_Control.UART1_Str_Recei.Data_a8 + i + PosStart1);
        // them ')' Vao cuoi data 1
        *(strData1.Data_a8 + strData1.Length_u16++) = ')';
        //them'(' v�o dau data2
        *(strData2.Data_a8 + strData2.Length_u16++) = '(';
        for(i = 0; i< lengdata2; i++)
            *(strData2.Data_a8 + strData2.Length_u16++) = *(UART1_Control.UART1_Str_Recei.Data_a8 + i + PosStart2);
    }else //chi co data 1. thi toan bi length la du data 1
    {
        strData1.Data_a8     = UART1_Control.UART1_Str_Recei.Data_a8 + PosStart1;
        strData1.Length_u16  = lengdata1;
    }
    //
    //Ghi obis va data vao buff chinh theo kieu dong goi 103
    if(sGet_Meter_Data->Flag_Start_Pack == 0)
    {
        Reset_Buff(&sGet_Meter_Data->Str_Payload);
        //wrire header TSVH vao
        if(sGet_Meter_Data == &Get_Meter_Info)           //khong biet co so sanh dia chi nay duoc k
            Write_Header_TSVH_Push103();
        else if (sGet_Meter_Data == &Get_Meter_Event)
            Header_event_103(&Get_Meter_Event.Str_Payload, 1);
        sGet_Meter_Data->Flag_Start_Pack = 1;
    }  
    //Them  () vao obis g�c.
    *(strObis.Data_a8 + strObis.Length_u16++) = '(';    
    for(i = 4; i < Obis1.Length_u16; i++)          //bo qua 0.0. v� 1.0.
        *(strObis.Data_a8 + strObis.Length_u16++) = *(Obis1.Data_a8 + i);
    *(strObis.Data_a8 + strObis.Length_u16++) = ')';
    //ghi obis va data vao
    Copy_String_toTaget(&sGet_Meter_Data->Str_Payload, sGet_Meter_Data->Pos_Obis_Inbuff, &strObis);
    sGet_Meter_Data->Pos_Obis_Inbuff += strObis.Length_u16;
    sGet_Meter_Data->Pos_Data_Inbuff = sGet_Meter_Data->Str_Payload.Length_u16;
    //Tang tong so qty len
    sGet_Meter_Data->Numqty ++;
    //ghi data vao
    Copy_String_toTaget(&sGet_Meter_Data->Str_Payload, sGet_Meter_Data->Pos_Data_Inbuff, &strData1);
    sGet_Meter_Data->Pos_Data_Inbuff = sGet_Meter_Data->Str_Payload.Length_u16;
    //
    if((lengdata2 != 0) &&(Obis2.Length_u16 != 0))  //neu k muon lay obis thu 2 thi length obis = 0 la duoc
    {
        //Them  () vao obis g�c.
        Reset_Buff(&strObis);
        *(strObis.Data_a8 + strObis.Length_u16++) = '(';
        for(i = 4; i < Obis2.Length_u16; i++)          //bo qua 0.0. v� 1.0.
            *(strObis.Data_a8 + strObis.Length_u16++) = *(Obis2.Data_a8 + i);
        *(strObis.Data_a8 + strObis.Length_u16++) = ')';
        ////ghi obis va data vao
        Copy_String_toTaget(&sGet_Meter_Data->Str_Payload, sGet_Meter_Data->Pos_Obis_Inbuff, &strObis);
        sGet_Meter_Data->Pos_Obis_Inbuff += strObis.Length_u16;
        sGet_Meter_Data->Pos_Data_Inbuff = sGet_Meter_Data->Str_Payload.Length_u16;
        //Tang tong so qty len
        sGet_Meter_Data->Numqty ++;
        //ghi data vao
        Copy_String_toTaget(&sGet_Meter_Data->Str_Payload, sGet_Meter_Data->Pos_Data_Inbuff, &strData2);
        sGet_Meter_Data->Pos_Data_Inbuff = sGet_Meter_Data->Str_Payload.Length_u16;
    }
}


void METER103_DecodeData_Event_TSVH (uint8_t Row, uint16_t PosStart1, Meter_Comm_Struct* sGet_Meter_Data)
{
   uint8_t PosObis = 0 ;
    
    if(Row < TSVH_POS_EVEN_2OBIS) 
      return;
    
    PosObis = Row - TSVH_POS_EVEN_2OBIS;
  
    METER103_DecodeData_2Obis2Data(METER103_EVENT_ObisCode[PosObis*2], METER103_EVENT_ObisCode[PosObis*2 + 1], PosStart1, sGet_Meter_Data);
}


void METER103_MI_SendData(void)
{
    uint8_t         Temp_BBC = 0;
     
    if(Get_Meter_Info.Flag_ui8 == 1)
    {
        _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nREAD TSVH OK", 14, 1000);
        *(Get_Meter_Info.Str_Payload.Data_a8 + Get_Meter_Info.Str_Payload.Length_u16++) = ETX; 
        //BBC
        Temp_BBC = BBC_Cacul(Get_Meter_Info.Str_Payload.Data_a8 + 1,Get_Meter_Info.Str_Payload.Length_u16 - 1);
        *(Get_Meter_Info.Str_Payload.Data_a8 + Get_Meter_Info.Str_Payload.Length_u16++) = Temp_BBC;
        
        Push_TSVH_toQueue(Get_Meter_Info.fTypeMessRead);     
    }
}


void METER103_Send_MInfo_Mess (void)
{
	uint8_t temp=0;
	
	temp = METER103_MeterInfoMessIDTable[Get_Meter_Info.Mess_Step_ui8];

    if(UART1_Control.fETX_EOT != 2)
        METER103_IEC62056_21_Command (CMD_READ_REGIS_103, METER103_TSVH_ObisCode[temp].Data_a8, METER103_TSVH_ObisCode[temp].Length_u16);  
    else 
        METER103_IEC62056_21_Command (CMD_ACK_103, NULL, 0);   //Neu EOT thi gui tiep ACK
}



//Viet chuong trinh doc BILL

uint8_t METER103_MBill_ExtractDataFunc (void)
{
    uint8_t         aObis[15] = {0};
    truct_String    strObis = {&aObis[0], 0};
    uint8_t         RowCheck = 0, Result = 0;
    uint16_t        i = 0;    
    uint16_t        PosFind = 0;
    uint8_t         aData[MAX_LENGTH_BYTE_RECEI_METER] = {0};
    truct_String    strData = {&aData[0], 0};
    
    
    RowCheck = METER103_MeterBillMessIDTable[Get_Meter_Billing.Mess_Step_ui8];   //Lay hang cua obis trong struct de check data
    if(RowCheck >= MAX_OBIS_BILL)   //qu� so dong t�i da cua list obis
        return 0;
    for(i = 0; i < (METER103_BILL_ObisCode[RowCheck].Length_u16 - 1); i++)    //bo 1 byte cuoi di
        *(strObis.Data_a8 + strObis.Length_u16++) = *(METER103_BILL_ObisCode[RowCheck].Data_a8 + i);
    //thay 1 byte cuoi v� c� the phai them 1 byte neu lastbill >9
    if(sMet103Var.SttLastBill <= 9)
        *(strObis.Data_a8 + strObis.Length_u16++) = sMet103Var.SttLastBill + 0x30;  //them byte cuoi cung
    else
    {
        *(strObis.Data_a8 + strObis.Length_u16++) = (sMet103Var.SttLastBill /10) + 0x30;
        *(strObis.Data_a8 + strObis.Length_u16++) = (sMet103Var.SttLastBill %10) + 0x30;
    }

    Result = METER_CheckBBC_OBIS_Recei(strObis);   //Neu tra ve 2: cat data, tra ve 1: khong cat data return 1. Neu tra ve 0 thi return 0
    if(Result == 2)
    {
        Result = 1;
        PosFind = 1 + strObis.Length_u16;
        //Cat ban tin data
        if(RowCheck == 0)
        {
            if(METER103_DecodeData_1Obis1Data (0, PosFind, &strData) == 1)    //Neu thoi gian khong nam trong khoang can thiet thi bao fail luon
               return METER103_PacketData_Bill(0, strObis, strData);
            else 
                return 0;
        }else if (RowCheck < BILL_POS_MAXDEMAND)
        {
            if(METER103_DecodeData_1Obis1Data (1, PosFind, &strData) == 1)
                METER103_PacketData_Bill(1, strObis, strData);
        }else 
        {
            if(METER103_DecodeData_1Obis1Data (2, PosFind, &strData) == 1)
                METER103_PacketData_Bill(2, strObis, strData);
        }
    }   
 
    return Result;
}



uint8_t METER103_PacketData_Bill (uint8_t type, truct_String Obis, truct_String strData)
{
    uint8_t         aObisTemp[20] = {0};
    uint16_t        i = 0;
    truct_String    strObis = {&aObisTemp[0], 0};
    
    //Ghi obis va data vao buff chinh theo kieu dong goi 103
    if(Get_Meter_Billing.Flag_Start_Pack == 0)
    {
        Reset_Buff(&Get_Meter_Billing.Str_Payload);
        Write_Header_His_Push103();
        Get_Meter_Billing.Flag_Start_Pack = 1;
    }
    
    if(type != 0)
    {
        //Them  () vao obis g�c.
        *(strObis.Data_a8 + strObis.Length_u16++) = '(';
        for(i = 4; i < (Obis.Length_u16 - 2); i++)          //bo qua 0.0. hoac 1.0 va  .1, .2 chuyen thanh *1
            *(strObis.Data_a8 + strObis.Length_u16++) = *(Obis.Data_a8 + i);
        *(strObis.Data_a8 + strObis.Length_u16++) = '*';
        *(strObis.Data_a8 + strObis.Length_u16++) = '1';
        *(strObis.Data_a8 + strObis.Length_u16++) = ')';
        ////ghi obis va data vao
        Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Obis_Inbuff, &strObis);
        Get_Meter_Billing.Pos_Obis_Inbuff += strObis.Length_u16;
        Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
        //Tang tong so qty len
        Get_Meter_Billing.Numqty ++;
        //ghi data vao
        Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Data_Inbuff, &strData);
        Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
    }else   //Ghi thoi gian vao va ket thuc ban tin TSVH
    {
        //Check stime xem nam trong khoang request khong
        ST_TIME_FORMAT  sTime = {0, 0, 0, 1, 1, 1, 0, 0};
        uint32_t        DiffValue;
        
        METER103_ConvertStrtime_toStime(strData.Data_a8 + 1, strData.Length_u16 - 2, &sTime);
        if(sInformation.Flag_Request_Bill == 1)
        {
            //so sanh xem nam trong khoang start time va stop time nua k. de dung lai hoac bo qua
            if(Check_DiffTime (sTime, sInformation.EndTime_GetBill,&DiffValue) == 1)  //neu thoi gian hien tai lon hon EndTime_GetBill. thi bo qua
            {
                _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nThoi gian Bill > EndTime", 26, 1000);
                return 0; //return 0 de chay doc tiep cac record khac 
            }
            //
            if(Check_DiffTime (sInformation.StartTime_GetBill, sTime, &DiffValue) == 1) //Neu StartTime_GetBill lon hon thoi gian hien tai doc ra thi dung viec doc
            {
                _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nThoi gian Bill < StartTime", 28, 1000);
                sInformation.Flag_Stop_ReadBill = 1;  //dung flag nay de dung vi�c doc.
                return 0;
            }
        }     
        //
        Get_Meter_Billing.Flag_ui8 = 1;   //muon flag nay de bao doc thanh cong
        //Ghi stime
        Reset_Buff(&strStimeBill);
        Copy_String_2(&strStimeBill, &strData);
    }
    return 1;
}


void METER103_MBillInsertTimeReq (void)
{	
    if(sInformation.Flag_Request_Bill == 0)   //khong co request. them stimestart va stime stop de so sanh
    {
        if (sRTC.month == 1){
            Copy_STime_fromsTime(&sInformation.StartTime_GetBill,(uint8_t) (sRTC.year + 99) %100, 12, sRTC.date, sRTC. hour, sRTC.min, sRTC.sec);
            Copy_STime_fromsTime(&sInformation.EndTime_GetBill,sRTC.year, sRTC.month, sRTC.date, sRTC. hour, sRTC.min, sRTC.sec);
        }else{
            Copy_STime_fromsTime(&sInformation.StartTime_GetBill, sRTC.year,  sRTC.month -1, sRTC.date, sRTC. hour, sRTC.min, sRTC.sec);
            Copy_STime_fromsTime(&sInformation.EndTime_GetBill,sRTC.year, sRTC.month, sRTC.date, sRTC. hour, sRTC.min, sRTC.sec);
        }
    } //co request thi da co thoi gian start va stop roi
}



uint8_t METER103_ConvertStrtime_toStime (uint8_t* Buff, uint8_t length, ST_TIME_FORMAT* Stime)
{
    if((length != 10) && (length != 12))
        return 0;
    
    //convert thoi gian
    Stime->year  = (*(Buff) - 0x30) * 10      + (*(Buff + 1) - 0x30);
    Stime->month = (*(Buff + 2)  - 0x30) * 10 + (*(Buff + 3)  - 0x30);
    Stime->date  = (*(Buff + 4)  - 0x30) * 10 + (*(Buff + 5) - 0x30); 
    
    Stime->hour  = (*(Buff + 6)  - 0x30) * 10 + (*(Buff + 7)  - 0x30);
    Stime->min   = (*(Buff + 8)  - 0x30) * 10 + (*(Buff + 9)  - 0x30);
    if (length == 12)
        Stime->min   = (*(Buff + 10)  - 0x30) * 10 + (*(Buff + 11)  - 0x30);
    else 
        Stime->sec   = 0;
    
    return 1;
}

void METER103_MBill_SendData(void)
{
    uint8_t         Temp_BBC = 0;
    uint8_t         aTempData[25] = {0};
    truct_String    aNumqty = {&aTempData[0], 0};
      
    if(Get_Meter_Billing.Flag_ui8 == 1)
    {
        _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nREAD BILL OK", 18, 1000);
        
        //them tuti
        Add_TuTI_toPayload(&Get_Meter_Billing);
        //Them thoi gian Bill
        if(Get_Meter_Billing.Numqty > 3)
            Copy_String_2(&aNumqty, &strStimeBill);
        else  //De trong thoi gian
        {
            *(aNumqty.Data_a8 + aNumqty.Length_u16++) = '(';
            *(aNumqty.Data_a8 + aNumqty.Length_u16++) = ')'; 
        }
        //ghi ca qty vao
        *(aNumqty.Data_a8 + aNumqty.Length_u16++) = '('; 
        Pack_HEXData_Frame_Uint64(&aNumqty, (uint64_t) Get_Meter_Billing.Numqty, 0);
        *(aNumqty.Data_a8 + aNumqty.Length_u16++) = ')'; 
        
        Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.PosNumqty, &aNumqty);  //lay tam struct string nay gan tam
        
        *(Get_Meter_Billing.Str_Payload.Data_a8 + Get_Meter_Billing.Str_Payload.Length_u16++) = ETX; 
        //BBC
        Temp_BBC = BBC_Cacul(Get_Meter_Billing.Str_Payload.Data_a8 + 1,Get_Meter_Billing.Str_Payload.Length_u16 - 1);
        *(Get_Meter_Billing.Str_Payload.Data_a8 + Get_Meter_Billing.Str_Payload.Length_u16++) = Temp_BBC;
        
        Push_Bill_toQueue(DATA_HISTORICAL);   
    }
}


void METER103_Send_MBill_Mess (void)
{
	uint8_t     temp=0, leng = 0;
    uint16_t    i = 0;
    uint8_t     aObisTemp[20] = {0};
	
	temp = METER103_MeterBillMessIDTable[Get_Meter_Billing.Mess_Step_ui8];
    
    for(i = 0; i < (METER103_BILL_ObisCode[temp].Length_u16 - 1); i++)    //bo 1 byte cuoi di
        aObisTemp[leng++] = *(METER103_BILL_ObisCode[temp].Data_a8 + i);
    //thay 1 byte cuoi v� c� the phai them 1 byte neu lastbill >9
    if(sMet103Var.SttLastBill <= 9)
        aObisTemp[leng++] = sMet103Var.SttLastBill + 0x30;  //them byte cuoi cung
    else
    {
        aObisTemp[leng++] = (sMet103Var.SttLastBill /10) + 0x30;
        aObisTemp[leng++] = (sMet103Var.SttLastBill %10) + 0x30;
    }

    METER103_IEC62056_21_Command (CMD_READ_REGIS_103, &aObisTemp[0], leng);  //gui lenh doc obis thanh ghi
}

//Viet chuong trinh doc EVEN
/*
    - Gui lenh doc 1 obis. co data tra ve  -> 
        Cat data theo obis -> Neu EVEN thi khong luu
    - Neu co ETX thi dung lai doc tiep sang obis khac.
    - Neu Co EOT thi gui ACK ->
        cat data theo obis -> dong goi vao buff chinh
*/


uint8_t METER103_MEven_ExtractDataFunc (void)
{
    truct_String     strObis = {NULL, 0};
    uint8_t          RowCheck = 0, Result = 0;
    uint16_t         PosFind = 0;
    uint8_t          TypeDecode = 0;
    
    RowCheck = METER103_MeterEvenMessIDTable[Get_Meter_Event.Mess_Step_ui8];   //Lay hang cua obis trong struct de check data
    if(RowCheck >= MAX_OBIS_EVEN)   //qu� so dong t�i da cua list obis
        return 0;
    strObis.Data_a8 = METER103_EVENT_ObisCode[RowCheck].Data_a8;
    strObis.Length_u16 = METER103_EVENT_ObisCode[RowCheck].Length_u16;
      
    Result = METER_CheckBBC_OBIS_Recei(strObis);   //Neu tra ve 2: cat data, tra ve 1: khong cat data return 1. Neu tra ve 0 thi return 0
    if(Result == 2)
    {
        Result = 1;
        PosFind = 1 + strObis.Length_u16;
        //Cat ban tin data
        TypeDecode = METER103_MEven_TypeDecode(RowCheck);
        //
        switch (TypeDecode)  //cat them dung obis cua hang do
        {
            case 0://Du lieu can cat ra thanh 2 obis va 2 data
                METER103_DecodeData_2Obis2Data(METER103_EVENT_ObisCode[RowCheck], METER103_EVENT_ObisCode[RowCheck + 1], PosFind, &Get_Meter_Event);
                break;
            default:
                break;
        }
    }
    
    return Result;
}


void METER103_PacketData_EVEN (uint8_t type, truct_String Obis, truct_String strData)
{
    uint8_t         aObisTemp[20] = {0};
    uint16_t        i= 0;
    truct_String    strObis = {&aObisTemp[0], 0};
    
    //Ghi obis va data vao buff chinh theo kieu dong goi 103
    if(Get_Meter_Event.Flag_Start_Pack == 0)
    {
        Reset_Buff(&Get_Meter_Event.Str_Payload);
        Header_event_103(&Get_Meter_Event.Str_Payload, 1);
        Get_Meter_Event.Flag_Start_Pack = 1;
    }  
    if(type != 0)
    {
        //Them  () vao obis g�c.
        *(strObis.Data_a8 + strObis.Length_u16++) = '(';
        for(i = 4; i < Obis.Length_u16; i++)          //bo qua 0.0. v� 1.0.
            *(strObis.Data_a8 + strObis.Length_u16++) = *(Obis.Data_a8 + i);
        *(strObis.Data_a8 + strObis.Length_u16++) = ')';
        ////ghi obis va data vao
        Copy_String_toTaget(&Get_Meter_Event.Str_Payload, Get_Meter_Event.Pos_Obis_Inbuff, &strObis);
        Get_Meter_Event.Pos_Obis_Inbuff += strObis.Length_u16;
        Get_Meter_Event.Pos_Data_Inbuff = Get_Meter_Event.Str_Payload.Length_u16;
        //Tang tong so qty len
        Get_Meter_Event.Numqty ++;
        //ghi data vao
        Copy_String_toTaget(&Get_Meter_Event.Str_Payload, Get_Meter_Event.Pos_Data_Inbuff, &strData);
        Get_Meter_Event.Pos_Data_Inbuff = Get_Meter_Event.Str_Payload.Length_u16;
    }
}

uint8_t METER103_MEven_TypeDecode (uint8_t row)
{
    uint8_t  result = 0;
    
    if(row < EVEN_POS_2OBIS_2DATA)
        result = 0;
    else 
        result = 0xFF;
    
    return result;
}


void METER103_MEven_SendData(void)
{
    uint8_t         Temp_BBC = 0;
    uint8_t         aTempData[10] = {0};
    truct_String    aNumqty = {&aTempData[0], 0};
    
    _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nREAD Even OK", 14, 1000);
    
    //them tuti
    Add_TuTI_toPayload(&Get_Meter_Event);
    
    //ghi ca qty vao
    *(aNumqty.Data_a8 + aNumqty.Length_u16++) = '('; 
    Pack_HEXData_Frame_Uint64(&aNumqty, (uint64_t) Get_Meter_Event.Numqty, 0);
    *(aNumqty.Data_a8 + aNumqty.Length_u16++) = ')'; 
    
    Copy_String_toTaget(&Get_Meter_Event.Str_Payload, Get_Meter_Event.PosNumqty, &aNumqty);  //lay tam struct string nay gan tam
    
    *(Get_Meter_Event.Str_Payload.Data_a8 + Get_Meter_Event.Str_Payload.Length_u16++) = ETX; 
    //BBC
    Temp_BBC = BBC_Cacul(Get_Meter_Event.Str_Payload.Data_a8 + 1,Get_Meter_Event.Str_Payload.Length_u16 - 1);
    *(Get_Meter_Event.Str_Payload.Data_a8 + Get_Meter_Event.Str_Payload.Length_u16++) = Temp_BBC;
    
    Push_Even_toQueue(0);
}

void METER103_Send_MEven_Mess (void)
{
    uint8_t temp=0;
	
	temp = METER103_MeterEvenMessIDTable[Get_Meter_Event.Mess_Step_ui8];

    if(UART1_Control.fETX_EOT != 2)
        METER103_IEC62056_21_Command (CMD_READ_REGIS_103, METER103_EVENT_ObisCode[temp].Data_a8, METER103_EVENT_ObisCode[temp].Length_u16);
    else METER103_IEC62056_21_Command (CMD_ACK_103, NULL, 0);   //Neu EOT thi gui tiep ACK
}


//Viet chuong trinh doc lpf
/*
    -
    - Gui lenh doc 1 obis lpf va kem theo thoi gian request doc. 
        co data tra ve  -> Cat ban tin lpf -> send to queue flash
    - Neu co ETX thi dung lai khong doc nua.
    - Neu Co EOT thi gui ACK   
        Co data tra ve -> Chek xem nam trong khoang thoi gian request k?  -> den khi ngoai khoang roi thi dung lai.
*/

uint8_t METER103_Read_Record (Meter_Comm_Struct *Meter_Comm, uint8_t (*FuncExtractRawData)(void),void (*FuncDataHandle)(void),void (*FuncSendMess)(void),uint8_t *CodeTable)
{
	uint8_t mReVal = 0;
    uint8_t CheckData = 0;
    
    if(METER103_Handshake(0) != 1) 
    {
        return 0;
    }
   
    Meter_Comm->Reading_ui8 = 1;
    UART1_Control.Mode_ui8 = 0;
    
	while(Meter_Comm->Reading_ui8 == 1) 
	{
		switch(Meter_Comm->Step_ui8) 
		{
			case 0:
				Meter_Comm->Total_Mess_Sent_ui32++;
				//Wait for respond message from Meter in 2000ms: checksum - extract raw data - save to buffer - goto next message
				if (osSemaphoreWait(bsUART2PendingMessHandle, TIME_WAIT_SEM) == osOK) 
                {
                    _fPrint_Via_Debug(&UART_SERIAL, UART1_Control.UART1_Str_Recei.Data_a8, UART1_Control.UART1_Str_Recei.Length_u16, 1000);
                    Meter_Comm->Success_Read_Mess_ui32++;
                    Meter_Comm->Step_ui8 = 1;
                    //Check v� cat data: 0: sai BBC hoac obis thi van doc tiep. 1 dung bbc, obis va cat data ok.  2 dung obisbbc, qua thoi gian dung lai
                    CheckData = FuncExtractRawData();
                    if(CheckData== 2)   
                    {
                        mReVal = 1;
                        Meter_Comm->Mess_Step_ui8++;   //doc tiep lpf2
                    }else if(CheckData == 1)   //co dadtaa thi push len queue
                    {
                        FuncDataHandle();  //Push mess 
                        mReVal = 1;
                    }
                    //Goto next message
                    if(UART1_Control.fETX_EOT != 2)    //ETX thi ket thuc khoi data. Chuyen sang obis khac
                    {
                        Meter_Comm->Mess_Step_ui8++;    //Doc tiep obis kh�c. Lpf2
                    }
                    //Goto next message
                    
                    Meter_Comm->Error_ui8 = 0;   
                    //If all mess sent -> finish
                    if ((*(CodeTable+Meter_Comm->Mess_Step_ui8)) == 0xFF) 
                    {
                        Meter_Comm->Step_ui8 = 2;  //Ket thuc
                    }
				} else 
                {//If Meter no respond after 2000ms: wait 2000ms - resend the message (3 times) 
					Meter_Comm->Error_Meter_Norespond_ui32++;   //thay bien dem loi nay bang bien khac de check retry
					Meter_Comm->Step_ui8 = 2;
				}
				break;
			case 1:	
                Get_Meter_LProf.Flag_Start_Pack = 0;      //Init lai Buff o day mo lan doc 1 ban ghi
				FuncSendMess();
				Meter_Comm->Step_ui8 = 0;
                if(sInformation.Flag_Stop_ReadLpf == 1)  //Neu co flag bao stop thi k doc nua
                {
                    Meter_Comm->Step_ui8 = 2;
                }
				break;
			case 2:
                METER103_IEC62056_21_Command (CMD_LOGOUT_103, NULL, 0);
                //Jump out of reading cycle, wait for next cycle (after 30mins)
                osDelay(3000);
				Meter_Comm->Reading_ui8 = 0;
				Meter_Comm->Step_ui8 = 1;
				Meter_Comm->Mess_Step_ui8 = 0;
				break;					
			default:
				break;				
		}
	}
	return mReVal;	
}



uint8_t METER103_MLpf_ExtractDataFunc (void)
{
    truct_String     strObis = {NULL, 0};
    uint8_t          RowCheck = 0, Result = 0;
    uint16_t         PosFind = 0;
      
    RowCheck = METER103_MeterLPFMessIDTable[Get_Meter_LProf.Mess_Step_ui8];   //Lay hang cua obis trong struct de check data
    if(RowCheck >= MAX_OBIS_LPF)   //qu� so dong t�i da cua list obis
        return 0;
    strObis.Data_a8 = METER103_LPF_ObisCode[RowCheck].Data_a8;
    strObis.Length_u16 = METER103_LPF_ObisCode[RowCheck].Length_u16;
      
    Result = METER_CheckBBC_OBIS_Recei(strObis);   //Neu tra ve 2: cat data, tra ve 1: khong cat data return 1. Neu tra ve 0 thi return 0
    if(Result == 2)
    {
        PosFind = 1 + strObis.Length_u16;
        //Cat ban tin data
        Result = METER103_Decode_LPF (PosFind);   //Return 2: qu� thoi gian end. return 1 neu OK. return 0 Neu cat data bi sai
    }else 
        Result = 0;   //Neu k c� data cung tra ve 0. de doc tiep
    
    return Result;
}

//ban tin mau de decode
//<STX>99.1.0(14)(1.8.0)(2.8.0)(3.8.0)(4.8.0)(32.7.0)(52.7.0)(72.7.0)(31.7.0)(51.7.0)(71.7.0)(13.7.0)(33.7.0)(53.7.0)(73.7.0)<CR><LF>
//(2105140100)(2105140130)(0018)(0.000*kWh)(0.000*kWh)(0.000*kvarh)(0.000*kvarh)(231.07*V)(15.19*V)(15.12*V)(0.00*A)(0.00*A)(0.00*A)(1.000)(1.000)(1.000)(1.000)<CR><LF><EOT><BCC>

/*
    - �au tien can cat so numqty = 14 ra va cong them 3 cai la Tu Ti hsn
    - cat data den 0D 0A, ghi vao buff chinh phan obis.
    - Cat tiep gia tri timestart (2105140100). De rieng ra de check thoi gian va de rieng ra ti dong goi
    - Tinh period bang stime end - stime start
    - Cat tiep even (0018). ti nua ghep voi nunqty de ghi vao
    - Cat het data con lai den 0D 0A. ghi vao buf chinh
    - Them TuTi vao.
    - Gui sang queue flash
*/

ST_TIME_FORMAT  sTimeEnd = {0, 0, 0, 1, 1, 1, 0, 0};
ST_TIME_FORMAT  sTimeReadStart = {0, 0, 0, 1, 1, 1, 0, 0}; 
ST_TIME_FORMAT  sTimeReadEnd = {0, 0, 0, 1, 1, 1, 0, 0}; 
    
uint8_t METER103_Decode_LPF(uint16_t PosStart1)
{
    uint16_t        Pos = 0, Period = 0;
    uint8_t         TemHex = 0;
    uint8_t         Numqty = 0;
    truct_String    sListObis = {NULL, 0};
    truct_String    sStartTime = {NULL, 0};
    truct_String    sStopTime = {NULL, 0};
    truct_String    sListData = {NULL, 0};
    truct_String    sEvenLpf = {NULL, 0};
    uint32_t        DiffValue = 0;
    uint8_t         aTemp[30] = {0};
    truct_String    strTemp = {&aTemp[0], 0};
    
    //Sau khi check obis. se tro den vi tri (14). Cat ra de them 3 qty cua Tu Ti va hsn
    for(Pos = PosStart1; Pos <UART1_Control.UART1_Str_Recei.Length_u16; Pos++)
    {
        TemHex = *(UART1_Control.UART1_Str_Recei.Data_a8 +Pos);   //Data la chu so thi dung lai
        if((TemHex >= '0') &&(TemHex <= '9'))
            break;
    }
    while((TemHex >= '0') && (TemHex <= '9'))
    {
        Numqty = Numqty*10 + (TemHex - 0x30);
        Pos++;
        TemHex = *(UART1_Control.UART1_Str_Recei.Data_a8 +Pos);
        if(Pos >= UART1_Control.UART1_Str_Recei.Length_u16) 
            return 0;
    }
    //Den day se tro de dau ')'
    Pos++;
    sListObis.Data_a8 = UART1_Control.UART1_Str_Recei.Data_a8 +Pos;   //Tro listobis vao vi tri bat dau cua cac obis
    while (*(UART1_Control.UART1_Str_Recei.Data_a8 + Pos) != 0x0D)
    {
        if(Pos >= UART1_Control.UART1_Str_Recei.Length_u16) 
            return 0;
        Pos++;
        sListObis.Length_u16 ++;
    }
    //cat start time lpf
    if((*(UART1_Control.UART1_Str_Recei.Data_a8 + Pos + 1) != 0x0A) && (*(UART1_Control.UART1_Str_Recei.Data_a8 + Pos + 2) != '('))
        return 0;
    Pos += 2;  //bo qua 0x0D va 0x0A
    sStartTime.Data_a8      = UART1_Control.UART1_Str_Recei.Data_a8 +Pos;  //Start time lpf
    sStartTime.Length_u16   = 12;
    Pos += 12;
    //Check luon startime doc ra duoc voi End time Request
    METER103_ConvertStrtime_toStime(sStartTime.Data_a8 + 1, 10, &sTimeReadStart);
    Copy_STime_fromsTime(&sTimeEnd, METER103_LPF_StopTime[0], METER103_LPF_StopTime[1], METER103_LPF_StopTime[2], 
                         METER103_LPF_StopTime[3], METER103_LPF_StopTime[4], 0);  
    //So sanh
    if(Check_DiffTime (sTimeEnd, sTimeReadStart, &DiffValue) == 0)  //Neu thoi gian Endtime nho hon Read thi dung lai
    {
        return 2; //return 2 de ket thuc qua trinh doc lpf
    }
    //cat time stop lpf de tinh period
    sStopTime.Data_a8      = UART1_Control.UART1_Str_Recei.Data_a8 +Pos;  //Start time lpf
    sStopTime.Length_u16   = 12;
    Pos += 12;
    METER103_ConvertStrtime_toStime(sStopTime.Data_a8 + 1, 10, &sTimeReadEnd);
    if(Check_DiffTime (sTimeReadStart, sTimeReadEnd, &DiffValue) == 0)  //end > start
    {
        Period = DiffValue/60;  //tinh ra phut
    }
    //
    sEvenLpf.Data_a8 = UART1_Control.UART1_Str_Recei.Data_a8 +Pos; 
    sEvenLpf.Length_u16 = 6;
    Pos += 6;
    //
    sListData.Data_a8 = UART1_Control.UART1_Str_Recei.Data_a8 +Pos;    //tro vao bat dau data
    while (*(UART1_Control.UART1_Str_Recei.Data_a8 + Pos) != 0x0D)
    {
        if(Pos >= UART1_Control.UART1_Str_Recei.Length_u16) 
            return 0;
        Pos++;
        sListData.Length_u16 ++;
    }
    if(Numqty == 0)   //khong co gia tri nao trong lpf
      return 0;
    // bat dau dong goi tat ca vao buff chinh
    if(Get_Meter_LProf.Flag_Start_Pack == 0)
    {
        Reset_Buff(&Get_Meter_LProf.Str_Payload);
        Pack_Header_lpf_Pushdata103();
        Get_Meter_LProf.Flag_Start_Pack = 1;
    }
    //ghi list obis v�o
    Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Obis_Inbuff, &sListObis);
    Get_Meter_LProf.Pos_Obis_Inbuff += sListObis.Length_u16;
    Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
    //ghi list data v�o
    Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Data_Inbuff, &sListData);
    Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
    //ghi TuTi vao
    Add_TuTI_toPayload(&Get_Meter_LProf);
    //Ghi stime ,event, preiod, numqty
    Copy_String_2(&strTemp, &sStartTime);  //12 byte
    Copy_String_2(&strTemp, &sEvenLpf);   //6 byte
    //
    *(strTemp.Data_a8 + strTemp.Length_u16++) = '('; 
    Pack_HEXData_Frame_Uint64(&strTemp, (uint64_t)(Period) ,0);    //4byte
    *(strTemp.Data_a8 + strTemp.Length_u16++) = ')'; 
    //num chanel
    *(strTemp.Data_a8 + strTemp.Length_u16++) = '('; 
    Pack_HEXData_Frame_Uint64(&strTemp, (uint64_t) (Numqty + 3), 0);   //4byte   //cong 3 obis cua tu ti hsn
    *(strTemp.Data_a8 + strTemp.Length_u16++) = ')'; 
    
    Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.PosNumqty, &strTemp);
     
    return 1;
}


void METER103_MLpf_SendData(void)
{
    Send_MessLpf_toQueue();   
}

void METER103_Send_MLpf_Mess (void)
{
    uint8_t     temp=0;
    uint16_t    i = 0;
	uint8_t     aObisTimeRequest[40] = {0};
    uint8_t     leng = 0;
    
	temp = METER103_MeterLPFMessIDTable[Get_Meter_LProf.Mess_Step_ui8];
    //Ghi obis can doc vao
    for(i = 0; i < METER103_LPF_ObisCode[temp].Length_u16;i++)
        aObisTimeRequest[leng++] = *(METER103_LPF_ObisCode[temp].Data_a8 + i);
    aObisTimeRequest[leng++] = '(';
    aObisTimeRequest[leng++] = ')';
    //ghi khoang thoi gian can doc vao
    aObisTimeRequest[leng++] = '(';
    for(i = 0; i < 5; i++)
    {
        aObisTimeRequest[leng++] = (METER103_LPF_StartTime[i]/10) %10 + 0x30;
        aObisTimeRequest[leng++] = (METER103_LPF_StartTime[i] %10) + 0x30;
    }
    aObisTimeRequest[leng++] = ')';
    aObisTimeRequest[leng++] = '(';
    for(i = 0; i < 5; i++)
    {
        aObisTimeRequest[leng++] = (METER103_LPF_StopTime[i]/10) %10 + 0x30;
        aObisTimeRequest[leng++] = (METER103_LPF_StopTime[i] %10) + 0x30;
    }
    aObisTimeRequest[leng++] = ')';
    //gui sang cong to
    if(UART1_Control.fETX_EOT != 2)
        METER103_IEC62056_21_Command (CMD_READ_RECORD1_103, &aObisTimeRequest[0], leng);
    else 
        METER103_IEC62056_21_Command (CMD_ACK_103, NULL, 0);   //Neu EOT thi gui tiep ACK
}


void METER103_GET_TIME_ReadLPF (void)
{
    if(sInformation.Flag_Request_lpf == 1)
    {
        //START TIME
        METER103_LPF_StartTime[0]   = sInformation.StartTime_GetLpf.year;  //year
        METER103_LPF_StartTime[1]   = sInformation.StartTime_GetLpf.month;  //month
        METER103_LPF_StartTime[2]   = sInformation.StartTime_GetLpf.date;  //Day
        METER103_LPF_StartTime[3]   = sInformation.StartTime_GetLpf.hour;  //hour
        METER103_LPF_StartTime[4]   = sInformation.StartTime_GetLpf.min;  //Min
        
        METER103_LPF_StopTime[0]    = sInformation.EndTime_GetLpf.year; //year
        METER103_LPF_StopTime[1]    = sInformation.EndTime_GetLpf.month;
        METER103_LPF_StopTime[2]    = sInformation.EndTime_GetLpf.date;
        METER103_LPF_StopTime[3]    = sInformation.EndTime_GetLpf.hour;  //hour
        METER103_LPF_StopTime[4]    = sInformation.EndTime_GetLpf.min;  //Min
    }else
    {
        //START TIME. cua gelex khac voi (Huu hong + vinasino): Endtime phai vuot qua 23:59
        METER103_LPF_StartTime[0]   = Read_Meter_LProf_Day[2];  //year
        METER103_LPF_StartTime[1]   = Read_Meter_LProf_Day[1];  //month
        METER103_LPF_StartTime[2]   = Read_Meter_LProf_Day[0];  //date
        METER103_LPF_StartTime[3]   = 0;  //hour
        METER103_LPF_StartTime[4]   = 0;  //Min
        
        if(sMet103Var.Mettype103 == 1)
        {
            METER103_LPF_StopTime[0]    = Read_Meter_LProf_Day[2]; //year
            METER103_LPF_StopTime[1]    = Read_Meter_LProf_Day[1]; //month
            METER103_LPF_StopTime[2]    = Read_Meter_LProf_Day[0]; //date
            METER103_LPF_StopTime[3]    = 23;  //Hour
            METER103_LPF_StopTime[4]    = 59;  //Min
        }else
        {
            ST_TIME_FORMAT      sTimeTemp;
            uint32_t            SecondVal = 0;
            //tang 1 ngay tu thoi gian 0h01min00sec.
            Copy_STime_fromsTime(&sTimeTemp, Read_Meter_LProf_Day[2], Read_Meter_LProf_Day[1], Read_Meter_LProf_Day[0], 0, 1, 0);
            SecondVal = HW_RTC_GetCalendarValue_Second(sTimeTemp, 1);
            SecondVal += 86400;     //Cong them 1 ngay
            Epoch_to_date_time(&sTimeTemp, SecondVal, 1);
            
            METER103_LPF_StopTime[0]    = sTimeTemp.year; //year
            METER103_LPF_StopTime[1]    = sTimeTemp.month; //month
            METER103_LPF_StopTime[2]    = sTimeTemp.date; //date
            METER103_LPF_StopTime[3]    = 0;  //Hour
            METER103_LPF_StopTime[4]    = 1;  //Min
        }
    }
}


//Function Write v�o cong to

uint8_t METER103_SetStimeToMeter (ST_TIME_FORMAT sRTC)
{
    uint8_t         aTemp[20] = {0};
    truct_String    StrTime = {&aTemp[0], 0};
    
    *(StrTime.Data_a8 + StrTime.Length_u16++) = '(';
    Copy_String_STime(&StrTime, sRTC);
    *(StrTime.Data_a8 +StrTime.Length_u16++) = (uint8_t) ((sRTC.sec/10)%10 + 0x30);
    *(StrTime.Data_a8 +StrTime.Length_u16++) = (uint8_t) (sRTC.sec%10 + 0x30);
    *(StrTime.Data_a8 + StrTime.Length_u16++) = ')';
    
    return METER103_WriteToMeter(&METER103_TSVH_ObisCode[0], StrTime.Data_a8, StrTime.Length_u16);
}
//<SOH>W1<STX>0.0.0.9.4(191106221017)<ETX><BCC>          pass 99999999
//return ACK

uint8_t METER103_WriteToMeter (truct_String *Obis, uint8_t *Data, uint8_t Length)
{
    uint8_t         aTempDataTotal[40] = {0};
    uint8_t         Len = 0;
    uint16_t        i = 0;
    
    if(METER103_Handshake(1) == 0)
        return 0;
    //Ghep obis va data lai chung
    for(i = 0; i < Obis->Length_u16; i++)
        aTempDataTotal[Len++] = *(Obis->Data_a8 + i);
    for(i = 0; i < Length; i++)
        aTempDataTotal[Len++] = *(Data + i);
    //Ghi vao c�ng to
    METER103_IEC62056_21_Command (CMD_WRITE_REGIS_103, &aTempDataTotal[0], Len);  //tra ve 0x06 hoac 0x18
         
    if (osSemaphoreWait(bsUART2PendingMessHandle, TIME_WAIT_SEM) == osOK)   
       if(UART1_Receive_Buff[0] == ACK)
           return 1;
       
    return 0;
}

