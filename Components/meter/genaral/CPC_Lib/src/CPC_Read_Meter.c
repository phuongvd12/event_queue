
#include "math.h"

#include "CPC_Init_Meter.h"
#include "CPC_Read_Meter.h"


#include "variable.h"
#include "sim900.h"
#include "t_mqtt.h"
#include "pushdata.h"
#include "myuart.h"
#include "at_commands.h"

#include "myDefine.h"


uint8_t 							aTempBuff_Cmd[256] = {0};   //buff chua byte gui lenh
uint8_t 							Buff_ACK[] = {'/', 'X', 'X', 'X'};   //Bat dau cua ACK 1 tu cong to ra
truct_String						Str_ACK ={(uint8_t*) &Buff_ACK[0], 4};
uint8_t 							Buff_RECEI_KEY[] = {SOH, 'P', '0', STX};   //Bat dau cua ACK 1 tu cong to ra
truct_String						Str_RECEI_KEY ={(uint8_t*) &Buff_RECEI_KEY[0], 4};

Meter_Var_Struct					Met_Var;

/*----------------Function-----------------*/
//Function send cmd theo chuan IEC62056_21
void CPC_IEC62056_21_Command (uint8_t Kind_Cmd, uint8_t* BuffSend, uint16_t lengthSend)
{
	uint8_t i=0;
	uint8_t length = 0;
	uint8_t Temp_BBC = 0;
	
	switch(Kind_Cmd)
	{
		case CMD_HANDSHAKE:
			for(i = 0; i< sizeof(CPC_Handshake1); i++)
				aTempBuff_Cmd[length++] = CPC_Handshake1[i];
			break;
		case CMD_ACK: 
			for(i = 0; i< sizeof(CPC_Handshake2); i++)
				aTempBuff_Cmd[length++] = CPC_Handshake2[i];
			break;
		case CMD_SEND_PASS: 
			aTempBuff_Cmd[length++] = SOH;
			aTempBuff_Cmd[length++] = 'P';
			aTempBuff_Cmd[length++] = '2';
			aTempBuff_Cmd[length++] = STX;
			for(i = 0; i< lengthSend; i++)
			{	
				aTempBuff_Cmd[length++] = *(BuffSend + i);
			}
			aTempBuff_Cmd[length++] = ETX;
			//them BBC vao
			Temp_BBC = BBC_Cacul(&aTempBuff_Cmd[1], (length-1));    //lenh nay BBC bang Xor tu 'P' cho den ETX
			aTempBuff_Cmd[length++] = Temp_BBC;
			break;  
		case CMD_READ: 
			aTempBuff_Cmd[length++] = SOH;
			aTempBuff_Cmd[length++] = 'R';
			aTempBuff_Cmd[length++] = '1';
			aTempBuff_Cmd[length++] = STX;
			for(i = 0; i< lengthSend; i++)
			{	
				aTempBuff_Cmd[length++] = *(BuffSend + i);
			}
			aTempBuff_Cmd[length++] = ETX;
			//them BBC vao
			Temp_BBC = BBC_Cacul(&aTempBuff_Cmd[1], (length-1));    //lenh nay BBC bang Xor tu 'P' cho den ETX
			aTempBuff_Cmd[length++] = Temp_BBC;
			break; 
		case CMD_WRITE: 
			aTempBuff_Cmd[length++] = SOH;
			aTempBuff_Cmd[length++] = 'W';
			aTempBuff_Cmd[length++] = '1';
			aTempBuff_Cmd[length++] = STX;
			for(i = 0; i< lengthSend; i++)
			{	
				aTempBuff_Cmd[length++] = *(BuffSend + i);
			}
			aTempBuff_Cmd[length++] = ETX;
			//them BBC vao
			Temp_BBC = BBC_Cacul(&aTempBuff_Cmd[1], (length-1));    //lenh nay BBC bang Xor tu 'P' cho den ETX
			aTempBuff_Cmd[length++] = Temp_BBC;
			break; 
			
		case CMD_LOGOUT:   
			// SOH B0 STX () ETX BCC
			aTempBuff_Cmd[length++] = SOH;
			aTempBuff_Cmd[length++] = 'B';
			aTempBuff_Cmd[length++] = '0';
			aTempBuff_Cmd[length++] = STX;
			aTempBuff_Cmd[length++] = '(';
			aTempBuff_Cmd[length++] = ')';
			aTempBuff_Cmd[length++] = ETX;
			//them BBC vao
			Temp_BBC = BBC_Cacul(&aTempBuff_Cmd[1], (length-1));    //lenh nay BBC bang Xor tu 'P' cho den ETX
			aTempBuff_Cmd[length++] = Temp_BBC;
			
		default: 
			break;
	}
    Init_UART2();
	HAL_UART_Transmit(&UART_METER,&aTempBuff_Cmd[0], length, 1000);
}



void CPC_Init_Function (uint8_t type)
{
    eMeter_20._f_Read_ID            = CPC_Get_Meter_ID;
    eMeter_20._f_Check_Reset_Meter  = CPC_CheckResetReadMeter;
  
    eMeter_20._f_Connect_Meter      = CPC_Connect_Metter_Handle;
    eMeter_20._f_Read_TSVH          = CPC_Read_TSVH;
    eMeter_20._f_Read_Bill          = CPC_Read_Bill;
    eMeter_20._f_Read_Event         = CPC_Read_Event;  
    eMeter_20._f_Read_Lpf           = CPC_Read_Lpf;
    eMeter_20._f_Read_InforMeter    = CPC_Read_Infor;
    eMeter_20._f_Get_UartData       = CPC_GetUART1Data;
    eMeter_20._f_Check_Meter        = CPC_Check_Meter;
    eMeter_20._f_Test1Cmd_Respond   = CPC_Send1Cmd_Test;
}

uint8_t CPC_Send1Cmd_Test (void)
{
    UART1_Control.Mode_ui8 = 1;

    Met_Var.Step_HandShake = CHECK_ACK_1;
    CPC_IEC62056_21_Command (CMD_HANDSHAKE, NULL, 0);
    osSemaphoreWait(bsUART2PendingMessHandle,2000);
    if(UART1_Control.fRecei_Respond == 1) 
    {
        _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nTEST 1 Command OK", 19, 1000);
        return 1;
    }
  
    return 0;
}


uint8_t CPC_Read_TSVH (uint32_t Type)
{        
    //Doc Tu TI truoc
    Get_Meter_Info.fTypeMessRead = Type;
    
    Init_Meter_TuTi_Struct();   
    if(CPC_Read(&Get_Meter_TuTi,&CPC_FunExtract_TuTi,&CPC_TuTi_SendData,&CPC_Send_TuTi_Mess,&CPC_MeterTuTi_IDTable[0]) != 1) 
    {
        Init_Meter_TuTi_Struct();
        if(CPC_Read(&Get_Meter_TuTi,&CPC_FunExtract_TuTi,&CPC_TuTi_SendData,&CPC_Send_TuTi_Mess,&CPC_MeterTuTi_IDTable[0]) != 1) 
        {
            Init_Meter_TuTi_Struct();
            if(CPC_Read(&Get_Meter_TuTi,&CPC_FunExtract_TuTi,&CPC_TuTi_SendData,&CPC_Send_TuTi_Mess,&CPC_MeterTuTi_IDTable[0]) != 1) 
              return 0;
        }
    }
    //Doc data cac thanh ghi
    Init_Meter_Info_Struct();
    if(CPC_Read(&Get_Meter_Info,&CPC_MI_ExtractDataFunc,&CPC_MI_SendData,&CPC_Send_MInfo_Mess,&CPC_MeterInfoMessIDTable[0]) != 1)
    {
        Init_Meter_Info_Struct();
        if(CPC_Read(&Get_Meter_Info,&CPC_MI_ExtractDataFunc,&CPC_MI_SendData,&CPC_Send_MInfo_Mess,&CPC_MeterInfoMessIDTable[0]) != 1)
        {
            Init_Meter_Info_Struct();
            if(CPC_Read(&Get_Meter_Info,&CPC_MI_ExtractDataFunc,&CPC_MI_SendData,&CPC_Send_MInfo_Mess,&CPC_MeterInfoMessIDTable[0]) != 1) 
              return 0;
        }
    }
     
    return 1;
}


uint8_t CPC_Read_Bill (void)
{
    sDCU.FlagHave_BillMes = 0;
    Init_Meter_TuTi_Struct();   
    if(CPC_Read(&Get_Meter_TuTi,&CPC_FunExtract_TuTi,&CPC_TuTi_SendData,&CPC_Send_TuTi_Mess,&CPC_MeterTuTi_IDTable[0]) != 1) 
        return 0;
      
    CPC_MBillInsertTimeReq();
    Met_Var.SttLastBill = 0;
    while ((Met_Var.SttLastBill < 12) &&(sInformation.Flag_Stop_ReadBill == 0))
    {
        Met_Var.SttLastBill++;  //tang vi tri doc lastbill
        Init_Meter_Billing_Struct();
        CPC_Read(&Get_Meter_Billing, &CPC_MBill_ExtractDataFunc,&CPC_MBill_SendData,&CPC_Send_MBill_Mess,&CPC_MeterBillMessIDTable[0]);
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
uint8_t CPC_Read_Event (uint32_t TemValue)
{
    //Doc Tu TI truoc
    Init_Meter_TuTi_Struct();   
    if(CPC_Read(&Get_Meter_TuTi,&CPC_FunExtract_TuTi,&CPC_TuTi_SendData,&CPC_Send_TuTi_Mess,&CPC_MeterTuTi_IDTable[0]) != 1) 
        return 0;
    
    //Doc data cac thanh ghi   
    Init_Meter_Event_Struct();
    if(CPC_Read(&Get_Meter_Event,&CPC_MEven_ExtractDataFunc,&CPC_EVENT_SendData,&CPC_Send_EVENT_Mess,&CPC_MeterEvenMessIDTable[0]) != 1)
    {
        Init_Meter_Event_Struct();
        if(CPC_Read(&Get_Meter_Event,&CPC_MEven_ExtractDataFunc,&CPC_EVENT_SendData,&CPC_Send_EVENT_Mess,&CPC_MeterEvenMessIDTable[0]) != 1)
        {
            Init_Meter_Event_Struct();
            if(CPC_Read(&Get_Meter_Event,&CPC_MEven_ExtractDataFunc,&CPC_EVENT_SendData,&CPC_Send_EVENT_Mess,&CPC_MeterEvenMessIDTable[0]) != 1) 
              return 0;
        }
    }  
   
    return 1;
}
uint8_t CPC_Read_Lpf (void)
{
    uint8_t result = 0;
    
    sDCU.FlagHave_ProfMess = 0;
    Init_Meter_TuTi_Struct();   
    if(CPC_Read(&Get_Meter_TuTi,&CPC_FunExtract_TuTi,&CPC_TuTi_SendData,&CPC_Send_TuTi_Mess,&CPC_MeterTuTi_IDTable[0]) != 1) 
        return 0;

    if(sInformation.Flag_Request_lpf == 0)  //K co request thi doc indexday = 2;
    {  
        sInformation.IndexStartLpf = 2;
        sInformation.IndexEndLpf  = 2;
    }
      
    Met_Var.Loadpf.IndexDay = sInformation.IndexStartLpf;
    while ((Met_Var.Loadpf.IndexDay >= sInformation.IndexEndLpf) && (sInformation.Flag_Stop_ReadLpf == 0))
    {
        Init_Meter_LProf_Struct();
        Reset_Buff(&StrUartTemp);   //Reset buff luu phan du
        result = CPC_Read(&Get_Meter_LProf,&CPC_Lpf_ExtractDataFunc,&CPC_Lpf_SendData,&CPC_Send_Lpf_Mess,&CPC_MeterLpfMessIDTable[0]);
        Met_Var.Loadpf.IndexDay--;  //Giam start di 1
        if(sInformation.Flag_Request_lpf == 0)
          break;
    }

    if(sDCU.FlagHave_ProfMess == 0)
    {
        _fSend_Empty_Lpf();
    } 
    
    sInformation.Flag_Stop_ReadLpf = 0;
    sInformation.Flag_Request_lpf = 0;
    
    return result;
}


uint8_t CPC_Read_Infor (void)
{
   Init_Meter_TuTi_Struct();   
    if(CPC_Read(&Get_Meter_TuTi,&CPC_FunExtract_TuTi,&CPC_TuTi_SendData,&CPC_Send_TuTi_Mess,&CPC_MeterTuTi_IDTable[0]) == 1) 
    {
        Pack_PushData_103_Infor_Meter();
        return 1;
    }
    return 0;
}


 
uint8_t CPC_Connect_Metter_Handle(void)
{
	uint8_t ReadIDRetry = 2; // qua 1 lan la bi loi khi sang mat khau moi - chi doc 1 lan
	
	__HAL_UART_ENABLE_IT(&UART_METER, UART_IT_RXNE);
	
	while (ReadIDRetry>0)
	{
        osDelay(500);
        if (osMutexWait(mtFlashMeterHandle,1000) == osOK)
		{
            Read_Meter_ID_Success = CPC_Get_Meter_ID(0);
            
            if (osMutexRelease(mtFlashMeterHandle) != osOK)
                osMutexRelease(mtFlashMeterHandle);
            
            if (Read_Meter_ID_Success == 1)
                break;
            else
                osDelay(2000);
            ReadIDRetry--;
        }
	}
	
    if(Read_Meter_ID_Success == 1) return 1;
    else return 0;
}


uint8_t CPC_Get_Meter_ID (uint32_t TempValue)
{
	uint8_t GetMeterIDRetry = 3;
	uint8_t Data_Set_ID[11] = {'0', '(', '0', '0', '0', '0', '0', '0', '0', '0', ')'}; // 11byte: 0(00000000)
    uint16_t i = 0;
    uint16_t length = 0;
    uint8_t ByteTemp = 0;
    uint8_t Buff_ID_Temp[METER_LENGTH];
    
	while (GetMeterIDRetry--)
	{
		if (CPC_Handshake_Handle() == 1)
		{
			UART1_Control.Mode_ui8 = 0;    //chuyen uart sang nhan data
			//Send Read ID
			CPC_IEC62056_21_Command (CMD_READ, &Data_Set_ID[0], 11);
            
			if (osSemaphoreWait(bsUART2PendingMessHandle,5000) == osOK) 
            {
				//Check BBC
				if(Check_BBC(&UART1_Control.UART1_Str_Recei) == 1)
				{
                    Met_Var.MeterID = 0;
					//cat ID meter o day
                    if(*(UART1_Control.UART1_Str_Recei.Data_a8 + 1) == Data_Set_ID[0])  //
                    {
                        for(i = 0; i < 4; i++)
                        {
                            ByteTemp = Convert2ByteHexStringto_1Hex(UART1_Control.UART1_Str_Recei.Data_a8 + length + 3);
                            length += 2;
                            Met_Var.MeterID |= (ByteTemp << (8*i));
                        }     
                        //Convert sang Struct string va luu vao FLash
                        sDCU.sMeter_id_Read.Data_a8 = &Buff_ID_Temp[0];
                        Reset_Buff(&sDCU.sMeter_id_Read);
                        Pack_HEXData_Frame(&sDCU.sMeter_id_Read,  Met_Var.MeterID, 0); 
                        
                        if (sDCU.sMeter_id_now.Length_u16 != sDCU.sMeter_id_Read.Length_u16)
                            Read_Meter_ID_Change = 1;
                        else
                        {
                            for (i=0; i<sDCU.sMeter_id_Read.Length_u16 ;i++)
                            {
                                if ((*(sDCU.sMeter_id_now.Data_a8+i)) != (*(sDCU.sMeter_id_Read.Data_a8+i)))
                                {
                                    Read_Meter_ID_Change = 1;
                                    break;
                                }
                            }
                        }
                    
                        if(Read_Meter_ID_Change == 1)
                        {
                            Reset_Buff(&sDCU.sMeter_id_now);
                            Pack_HEXData_Frame(&sDCU.sMeter_id_now,  Met_Var.MeterID, 0); 
//                            Save_Meter_ID();
                        }
                        _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nGet ID OK", 11, 1000);
                        CPC_IEC62056_21_Command (CMD_LOGOUT, NULL, 0);
                        osDelay(TIME_DELAY_ERR);
                        return 1;
                    }
				}
                CPC_IEC62056_21_Command (CMD_LOGOUT, NULL, 0);
                osDelay(TIME_DELAY_ERR);
			}
		}
	}
	return 0;
}

uint8_t CPC_Handshake_Handle (void)
{
    uint8_t     i = 0;
    int	        PosFind = 0;
	uint8_t     Count_Step_End = 0, Reading_ui8 = 0;;
    
    UART1_Control.Mode_ui8 = 1;
	Met_Var.Step_HandShake = STEP_END;   	   
	
	
	while (Reading_ui8 == 0)     
	{
		switch (Met_Var.Step_HandShake)
		{
			case SEND_FIRST_CHAR:	//Send first Char  
//				_fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nSend First Char\r\n", 19, 1000);
				Met_Var.Step_HandShake++;
				//Send First Char cmd
				CPC_IEC62056_21_Command (CMD_HANDSHAKE, NULL, 0);
				break;
			case CHECK_ACK_1:   //check ACK 1 tu cong to ra
				if (osSemaphoreWait(bsUART2PendingMessHandle,5000) == osOK) 
                {
                    Met_Var.Step_HandShake++;
					//Check chuoi nhan ve
					if(UART1_Control.UART1_Str_Recei.Length_u16 < 5) return 0;
					
					if(Find_String_V2(&Str_ACK, &UART1_Control.UART1_Str_Recei) < 0)
                        Met_Var.Step_HandShake = STEP_END;
				} else 
                    Met_Var.Step_HandShake = STEP_END;	
				break;
			case SEND_BAURATE:	
				Met_Var.Step_HandShake++;
				//send ACK + Baurate
				CPC_IEC62056_21_Command (CMD_ACK, NULL, 0);
				break;
			case RECEI_ENCRYPT_KEY:   //Mat khau c�p 3 can phai lay Key de m� hoa Pass:16 byte DC 34 61 91 0E 9E 70 B6 E7 8F AB 3A D3 30 3A C2
				if (osSemaphoreWait(bsUART2PendingMessHandle,5000) == osOK) 
                {
                    Met_Var.Step_HandShake++;
					PosFind = Find_String_V2(&Str_RECEI_KEY, &UART1_Control.UART1_Str_Recei);
					if( PosFind >= 0)
					{
						//lay key
						for(i = 0; i < 16; i++)
						{
							CPC_KEY_ENCRYPT[i] = *(UART1_Control.UART1_Str_Recei.Data_a8 + PosFind + Str_RECEI_KEY.Length_u16 + 1 + i);
							//ma hoa Pass n�u la Pass cap 3
						}
					}else 
						Met_Var.Step_HandShake = STEP_END;
				} else 
                    Met_Var.Step_HandShake = STEP_END;  
				break;
			case SEND_PASSWORD:	//Send first Char
				Met_Var.Step_HandShake++;
				//Send truc tiep mat khau cap 2
				CPC_IEC62056_21_Command (CMD_SEND_PASS, (uint8_t*)&password_1[0], sizeof(password_1));
				break;
			case CHECK_ACK_2:
				if (osSemaphoreWait(bsUART2PendingMessHandle,5000) == osOK) 
                {
                    Met_Var.Count_Error = 0;
					Reading_ui8 = 1;
				}else 
                    Met_Var.Step_HandShake = STEP_END;
				break;
			case STEP_END:
				CPC_IEC62056_21_Command (CMD_LOGOUT, NULL, 0);
				Count_Step_End++;
				if(Count_Step_End <= 1)
					Met_Var.Step_HandShake = SEND_FIRST_CHAR;
				else
                {
                    Met_Var.Count_Error++;
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

//Nhan uart trong ngat. Co Phai thay cac ham delay bang cac hang doi queue. de check moi chuan?

uint8_t CPC_GetUART1Data(void)
{
	uint8_t	temp_recieve = 0;

	switch (UART1_Control.Mode_ui8)
	{
		case 0: // data message
			if (UART1_Control.Mess_Pending_ui8 != 1)
			{
				temp_recieve = (uint8_t)(UART_METER.Instance->RDR&0xFF);
				
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
					if (temp_recieve == ETX) // Ki tu ket thuc
					{
						UART1_Control.Mess_Pending_ui8 = 2;
					}
		
					if (UART1_Control.UART1_Str_Recei.Length_u16 > MAX_LENGTH_BUFF_NHAN)	
							UART1_Control.UART1_Str_Recei.Length_u16 = 0;
				}
			}else
				temp_recieve = (uint8_t)(UART_METER.Instance->RDR&0xFF);
            
			UART1_Control.fRecei_Respond = 1;
			break;
		case 1: // handshake message
			if (UART1_Control.Mess_Pending_ui8 != 1)
			{
				temp_recieve = (uint8_t)(UART_METER.Instance->RDR&0xFF);
				
				*(UART1_Control.UART1_Str_Recei.Data_a8 + UART1_Control.UART1_Str_Recei.Length_u16) = temp_recieve;
			    UART1_Control.UART1_Str_Recei.Length_u16++;    //bien chung
				
				switch (Met_Var.Step_HandShake)    
				{ 
					case CHECK_ACK_1:
						if (temp_recieve == 0x0A) // handshake 1
						{
							UART1_Control.Mess_Pending_ui8 = 1;
							osSemaphoreRelease(bsUART2PendingMessHandle);
						}
						break;
					case RECEI_ENCRYPT_KEY:
						if (UART1_Control.Mess_Pending_ui8 == 2)  //nhan BCC
						{
							UART1_Control.Mess_Pending_ui8 = 1;
							osSemaphoreRelease(bsUART2PendingMessHandle);
						}	
						if (temp_recieve == ETX) // Ki tu ket thuc
						{
							UART1_Control.Mess_Pending_ui8 = 2;
						}
						break;
					case CHECK_ACK_2:
						if (temp_recieve == ACK) //  - ACK
						{
							UART1_Control.Mess_Pending_ui8 = 1;
							osSemaphoreRelease(bsUART2PendingMessHandle);
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



uint8_t CPC_CheckResetReadMeter(uint32_t Timeout)
{
	// Reset neu doc sai cong to
    if ((sDCU.Status_Meter_u8 == 0) && (Check_Time_Out(sDCU.LandMark_Count_Reset_Find_Meter,600000) == TRUE)) // 10p
    {
        osDelay(Timeout); // 5p
        Read_Meter_ID_Success = CPC_Get_Meter_ID(0);
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


uint8_t CPC_Check_Meter(void)
{
	if (Read_Meter_ID_Success == 1) 
	{ 
        if(Met_Var.Count_Error >=2)
        {
            Read_Meter_ID_Success = CPC_Get_Meter_ID(0);
        }
	}
    return 1;
} 


uint8_t ConvertHexStringtoHex (uint8_t HexString)
{
	if(HexString > 0x39) 
        return(HexString - 0x37);
	else 
        return(HexString - 0x30);
}

uint8_t Convert2ByteHexStringto_1Hex (uint8_t* Buff)
{
	uint8_t ByteTemp = 0;
	
	ByteTemp = (ConvertHexStringtoHex (*Buff)) << 4;
	ByteTemp |= ConvertHexStringtoHex (*(Buff + 1));
	
	return ByteTemp;
}


uint8_t ConvertHextoAscii (uint8_t Hex)
{
	if(Hex > 0x09) 
        return (Hex + 0x37);
	else 
        return (Hex + 0x30);
}

//Ham de debug
uint16_t ConvertHextoHexstring (uint8_t* BuffHex, uint16_t length, uint8_t* BuffHexString)
{
	uint8_t temp = 0;
	uint16_t i = 0;
	uint16_t LengthResult = 0;
	
	for(i = 0; i < length; i++)
	{
		temp = (*(BuffHex +i) & 0xF0) >> 4;
		*(BuffHexString + 2*i) = ConvertHextoAscii(temp);
		temp = *(BuffHex +i) & 0x0F;
		*(BuffHexString + 2*i + 1) = ConvertHextoAscii(temp);
		LengthResult += 2;
	}
	return LengthResult;
}

void PrintInteger4byte(uint32_t value)
{
//	uint8_t buff[4];
//	uint8_t LengthPrint = 0;
//	uint8_t Buff_Print[8];
//	
//	buff[0] = (uint8_t)(value >> 24);
//	buff[1] = (uint8_t)(value >> 16);
//	buff[2] = (uint8_t)(value >> 8);
//	buff[3] = (uint8_t)(value);
//	
//	LengthPrint = (uint8_t) ConvertHextoHexstring(&buff[0], 4, &Buff_Print[0]);
	
//	_fPrint_Via_Debug(&UART_SERIAL, &Buff_Print[0], LengthPrint, 1000);
//	_fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "  \r\n", 4, 1000);
}

void PrintChar1byte(uint8_t value)
{
//	uint8_t Buff_Print[2];
	
//	ConvertHextoHexstring(&value, 1, &Buff_Print[0]);
	
//	_fPrint_Via_Debug(&UART_SERIAL, &Buff_Print[0], 2, 1000);
//	_fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "  \r\n", 4, 1000);
}

void Print8byte(uint64_t value)
{
//	uint8_t buff[8];
//	uint8_t LengthPrint = 0;
//	uint8_t Buff_Print[16];
	
//	buff[1] = (uint8_t)(value >> 48);
//	buff[2] = (uint8_t)(value >> 40);
//	buff[3] = (uint8_t)(value >> 32);
//	buff[4] = (uint8_t)(value >> 24);
//	buff[0] = (uint8_t)(value >> 56);
//	buff[5] = (uint8_t)(value >> 16);
//	buff[6] = (uint8_t)(value >> 8);
//	buff[7] = (uint8_t)(value);
	
//	LengthPrint = (uint8_t) ConvertHextoHexstring(&buff[0], 8, &Buff_Print[0]);
	
//	_fPrint_Via_Debug(&UART_SERIAL, &Buff_Print[0], LengthPrint, 1000);
//	_fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "  \r\n", 4, 1000);
}


//Viet lai doc cong to CPC

uint8_t CPC_Read (Meter_Comm_Struct *Meter_Comm, uint8_t (*FuncExtractRawData)(void),void (*FuncDataHandle)(void),void (*FuncSendMess)(void),uint8_t *CodeTable)
{
	uint8_t mReVal = 0;
    
    if (CPC_Handshake_Handle() != 1)
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
				if (osSemaphoreWait(bsUART2PendingMessHandle, CPC_TIME_WAIT_SEM) == osOK) 
                {
                    Meter_Comm->Success_Read_Mess_ui32++;
                    Meter_Comm->Step_ui8 = 1;
                    
                    if(Check_BBC(&UART1_Control.UART1_Str_Recei) == 1)
                    {
                        //Extract raw data
                        if(FuncExtractRawData() == 1)   //Check v� cat data
                        {
                            //Goto next message
                            Meter_Comm->Mess_Step_ui8++;
                            Meter_Comm->Error_ui8 = 0;   
                            //If all mess sent -> finish
                            if (Meter_Comm->Mess_Step_ui8 > 0)
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
                    }else
                    {
                        _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nFun ReadMeter Sai CRC\r\n", 25, 1000);
                        _fPrint_Via_Debug(&UART_SERIAL, UART1_Control.UART1_Str_Recei.Data_a8, UART1_Control.UART1_Str_Recei.Length_u16, 1000);
                    }
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
                CPC_IEC62056_21_Command (CMD_LOGOUT, NULL, 0);
                osDelay(TIME_DELAY_ERR);
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

			


//TuTi
uint8_t CPC_FunExtract_TuTi (void)   
{
	uint16_t length = 0;
	uint8_t i = 0;
	uint8_t ByteTemp = 0, temp;
	
	uint32_t Tu_Primary = 0;
	uint32_t Ti_Primary = 0;
	uint32_t Ti_Second = 0;
	uint32_t Tu_Second = 0;
	
    temp = CPC_MeterTuTi_IDTable[Get_Meter_TuTi.Mess_Step_ui8];
    
    //check byte thu 2. xem dung nhu byte dau tien cua data set khong?
    if(*(UART1_Control.UART1_Str_Recei.Data_a8 + 1) == CPC_TSVH_ObisCode[temp][0])  //
    {
        //Tu Primary  ConvertHexStringtoHex
        for(i = 0; i < 4; i++)
        {
            ByteTemp = Convert2ByteHexStringto_1Hex(UART1_Control.UART1_Str_Recei.Data_a8 + length + 3);
            length += 2;
            Tu_Primary |= (ByteTemp << (8*i));
        }
        
        //Tu Second
        Tu_Second = Convert2ByteHexStringto_1Hex(UART1_Control.UART1_Str_Recei.Data_a8 + length + 3);
        length += 2;
        
        //Ti Primary
        for(i = 0; i < 4; i++)
        {
            ByteTemp = Convert2ByteHexStringto_1Hex(UART1_Control.UART1_Str_Recei.Data_a8 + length + 3);
            length += 2;
            Ti_Primary |= (ByteTemp << (8*i));
        }
        //Ti Second
        Ti_Second = Convert2ByteHexStringto_1Hex(UART1_Control.UART1_Str_Recei.Data_a8 + length + 3);
        
        //dong goi vao Buff TuTi
        //TU Ti He so nhan		
        *(Get_Meter_TuTi.Str_Payload.Data_a8 + Get_Meter_TuTi.Str_Payload.Length_u16++) = '(';
        Pack_HEXData_Frame_Uint64(&Get_Meter_TuTi.Str_Payload, (uint64_t) Tu_Primary, CPC_SCALE_TU_TI);
        *(Get_Meter_TuTi.Str_Payload.Data_a8 + Get_Meter_TuTi.Str_Payload.Length_u16++) = '/';
        Pack_HEXData_Frame_Uint64(&Get_Meter_TuTi.Str_Payload, (uint64_t) Tu_Second, CPC_SCALE_TU_TI); 
        *(Get_Meter_TuTi.Str_Payload.Data_a8 + Get_Meter_TuTi.Str_Payload.Length_u16++) = ')';
        
        *(Get_Meter_TuTi.Str_Payload.Data_a8 + Get_Meter_TuTi.Str_Payload.Length_u16++) = '(';
        Pack_HEXData_Frame_Uint64(&Get_Meter_TuTi.Str_Payload, (uint64_t) Ti_Primary, CPC_SCALE_TU_TI); 
        *(Get_Meter_TuTi.Str_Payload.Data_a8 + Get_Meter_TuTi.Str_Payload.Length_u16++) = '/';
        Pack_HEXData_Frame_Uint64(&Get_Meter_TuTi.Str_Payload, (uint64_t) Ti_Second, CPC_SCALE_TU_TI); 
        *(Get_Meter_TuTi.Str_Payload.Data_a8 + Get_Meter_TuTi.Str_Payload.Length_u16++) = ')';
        
        return 1;
    }
    
    return 0;
}


void CPC_TuTi_SendData(void)
{
    *(Get_Meter_TuTi.Str_Payload.Data_a8 + Get_Meter_TuTi.Str_Payload.Length_u16++) = '(';
    Pack_HEXData_Frame_Uint64(&Get_Meter_TuTi.Str_Payload, (uint64_t) sDCU.He_So_Nhan, CPC_SCALE_HE_SO_NHAN); 
    *(Get_Meter_TuTi.Str_Payload.Data_a8 + Get_Meter_TuTi.Str_Payload.Length_u16++) = ')';
    _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nREAD TuTi OK", 14, 1000);
}

void CPC_Send_TuTi_Mess (void)
{
    uint8_t temp=0;
	
	temp = CPC_MeterTuTi_IDTable[Get_Meter_TuTi.Mess_Step_ui8];
    
    CPC_IEC62056_21_Command (CMD_READ, &CPC_TSVH_ObisCode[temp][0], 11);
}

//Function doc TSVH
uint8_t CPC_MI_ExtractDataFunc (void)
{
    uint8_t temp =0;
    
    
    temp = CPC_MeterInfoMessIDTable[Get_Meter_Info.Mess_Step_ui8];
    
    //check byte thu 2. xem dung nhu byte dau tien cua data set khong?
    if(*(UART1_Control.UART1_Str_Recei.Data_a8 + 1) == CPC_TSVH_ObisCode[temp][0])  //
    {
        switch(Get_Meter_Info.Mess_Step_ui8)
        {
            case 0:
            case 1:
            case 2: CPC_Decode_Intan(Get_Meter_Info.Mess_Step_ui8);  break; 
            case 3: CPC_DecodeEnergyTotal(&Get_Meter_Info, Get_Meter_Info.Mess_Step_ui8); break; 
            case 4:  //4 goi taiff
            case 5:
            case 6:
            case 7: CPC_Decode_Tariff (&Get_Meter_Info, Get_Meter_Info.Mess_Step_ui8 - 4); break;
            case 8: //8 goi Maxdemand
            case 9:  
            case 10:  
            case 11:
            case 12:
            case 13: 
            case 14:
            case 15: CPC_Decode_MAXDemand_TSVH (&Get_Meter_Info, Get_Meter_Info.Mess_Step_ui8 - 8); break;
            case 16: //Cac goi tin cua event TSVH
            case 17:
            case 18:
            case 19:
            case 20: CPC_Decode_Event(&Get_Meter_Info, Get_Meter_Info.Mess_Step_ui8 - 16);  break;
            case 21: CPC_Decode_STime(Get_Meter_Info.Mess_Step_ui8); break;         
            default:
                return 0;
        }
        
        return 1;
    }
    
    return 0;
}



void CPC_Decode_Intan (uint8_t type)
{
	uint16_t length = 0;
	uint8_t j = 0;

	switch(type)
	{
		case 0:
			for(j = 0; j < 54; j++) //54 byte goi 1
			{
                Meter_TempBuff[Get_Meter_Info.Data_Buff_Pointer_ui16++] = Convert2ByteHexStringto_1Hex(UART1_Control.UART1_Str_Recei.Data_a8 + length + 3);
                length += 2;
			}
			break;
		case 1:
			for(j = 0; j < 96; j++) //96 byte goi 2
            {
                Meter_TempBuff[Get_Meter_Info.Data_Buff_Pointer_ui16++] = Convert2ByteHexStringto_1Hex(UART1_Control.UART1_Str_Recei.Data_a8 + length + 3);
                length += 2;
            }
			break;
		case 2:
            for(j = 0; j < 32; j++) //32 goi 3
            {
                Meter_TempBuff[Get_Meter_Info.Data_Buff_Pointer_ui16++] = Convert2ByteHexStringto_1Hex(UART1_Control.UART1_Str_Recei.Data_a8 + length + 3);
                length += 2;
            }
			break;
		default:
			break;
	}
	
}


void CPC_DecodeEnergyTotal(Meter_Comm_Struct* Get_Meter, uint8_t IndexPack)
{
	uint16_t length = 0;
	uint8_t i = 0;
    uint8_t count = 0;
	//l�y lan luot cac gia tri: moi gia tri 8byte
    for(count = 0; count < 9; count++)  //9*8 = 72 byte
        for(i = 0; i < 8; i++)
        {
            Meter_TempBuff[Get_Meter->Data_Buff_Pointer_ui16++] = Convert2ByteHexStringto_1Hex(UART1_Control.UART1_Str_Recei.Data_a8 + length + 3);
            length += 2;
        }
    _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nGet EN Total OK", 17, 1000);
}



void CPC_Decode_Tariff (Meter_Comm_Struct* Get_Meter, uint8_t IndexPack)
{
    uint16_t length = 0;
	uint8_t i = 0;
    
    for(i = 0; i < 72; i++)   //8 bieu*9 = 72.   4 g�i
	{
        Meter_TempBuff[Get_Meter->Data_Buff_Pointer_ui16++] =  Convert2ByteHexStringto_1Hex(UART1_Control.UART1_Str_Recei.Data_a8 + length + 3);
        length += 2;
	}
	_fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nGet Tariff OK", 15, 1000);
}


void CPC_Decode_MAXDemand_TSVH (Meter_Comm_Struct* Get_Meter, uint8_t IndexPack)
{
    uint16_t length = 0;
	uint8_t i = 0;
    
    for(i = 0; i < 37; i++) //1 goi = 1+(8 +4) *3 = 37 byte
	{
        Meter_TempBuff[Get_Meter->Data_Buff_Pointer_ui16++] =  Convert2ByteHexStringto_1Hex(UART1_Control.UART1_Str_Recei.Data_a8 + length + 3);
        length += 2;
	}
    
	_fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nGet MaxDemand OK", 18, 1000);
}


void CPC_Decode_MAXDemand_LASTBILL (Meter_Comm_Struct* Get_Meter, uint8_t IndexPack)
{
    uint16_t length = 0;
	uint8_t i = 0;
    
    for(i = 0; i < 74; i++) //1 goi = 1+(8 +4) *3 = 37 byte. 2 source
	{
        Meter_TempBuff[Get_Meter->Data_Buff_Pointer_ui16++] =  Convert2ByteHexStringto_1Hex(UART1_Control.UART1_Str_Recei.Data_a8 + length + 3);
        length += 2;
	}
    
	_fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nGet MaxDemand OK", 18, 1000);
}





void CPC_MI_SendData(void)
{
    uint8_t         Temp_BBC = 0;
     
    if(Get_Meter_Info.Flag_ui8 == 1)
    {
        CPC_PackData_ToMinfor();   //Dong goi
        
        _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nREAD TSVH OK", 14, 1000);
        *(Get_Meter_Info.Str_Payload.Data_a8 + Get_Meter_Info.Str_Payload.Length_u16++) = ETX; 
        //BBC
        Temp_BBC = BBC_Cacul(Get_Meter_Info.Str_Payload.Data_a8 + 1,Get_Meter_Info.Str_Payload.Length_u16 - 1);
        *(Get_Meter_Info.Str_Payload.Data_a8 + Get_Meter_Info.Str_Payload.Length_u16++) = Temp_BBC;
       
        Push_TSVH_toQueue(Get_Meter_Info.fTypeMessRead);     
    }
}

/*
    Moi dataset co the co truyen nhieu lan va chi thay doi 1 byte nao do. Neu nhu copy het dataset vao 1 mang thi mang do se dai
*/

void CPC_Send_MInfo_Mess (void)
{
    uint8_t temp=0;
	
	temp = CPC_MeterInfoMessIDTable[Get_Meter_Info.Mess_Step_ui8];
    
    CPC_IEC62056_21_Command (CMD_READ, &CPC_TSVH_ObisCode[temp][0], 11);
}



void CPC_Decode_STime(uint8_t IndexPack)
{
	//check length nhan:
	Met_Var.STimeIntan.sec 		= Convert2ByteHexStringto_1Hex(UART1_Control.UART1_Str_Recei.Data_a8 + 3);
	Met_Var.STimeIntan.min 		= Convert2ByteHexStringto_1Hex(UART1_Control.UART1_Str_Recei.Data_a8 + 5);
	Met_Var.STimeIntan.hour 	= Convert2ByteHexStringto_1Hex(UART1_Control.UART1_Str_Recei.Data_a8 + 7);
	Met_Var.STimeIntan.date 	= Convert2ByteHexStringto_1Hex(UART1_Control.UART1_Str_Recei.Data_a8 + 9);
	Met_Var.STimeIntan.month 	= Convert2ByteHexStringto_1Hex(UART1_Control.UART1_Str_Recei.Data_a8 + 11);
	Met_Var.STimeIntan.year 	= Convert2ByteHexStringto_1Hex(UART1_Control.UART1_Str_Recei.Data_a8 + 13);
	
    Met_Var.sTime_s = HW_RTC_GetCalendarValue_Second(Met_Var.STimeIntan, 1);
    Get_Meter_Info.Flag_ui8 = 1;   //b�o co stime decode duoc
}

uint8_t CPC_PackData_ToMinfor (void)
{ 
    uint16_t    i = 0;
    truct_String    Str_Data_Write={&BuffRecord[0], 0};   //buff nay 200 byte khai bao chung
    uint32_t    Tem_u32 = 0;
    uint16_t    Tem_u16 = 0;
    uint64_t    Tem_u64 = 0;
    uint16_t    j = 0;
    uint16_t    count = 0;
    
    Write_Header_TSVH_Push103();

    //0 1 2.(dien ap A B C) 3 4 5 (dong dien A B C) 6 7 8.(tan so A B C)   
    //9 10 11 12(Goc lech A B C trung binh) .
    // 13, 14, 15 ,16  (he so cong suat A B C trung binh)
    
    //54 byte goi1. 196 byte goi 2 .32 goi 3
    for(j = 0; j < MAX_OBIS103_TSVH; j++)
    {
        Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Obis_Inbuff, CPC_Obis103_TSVH[j]);
        Get_Meter_Info.Pos_Obis_Inbuff += CPC_Obis103_TSVH[j]->Length_u16;
    }
    Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
    Get_Meter_Info.Numqty += MAX_OBIS103_TSVH;   //So obis da khai bao
    
    //data
    i = 10;  //ddau tien cua buff temp
    Reset_Buff(&Str_Data_Write);
    //Dien ap
    for(count = 0; count<3; count++)  //3 gia tri dien ap
    {
        Tem_u32 = 0;
        for(j = 0; j<4; j++)
            Tem_u32 |= Meter_TempBuff[i++] << (8*j);
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
        Pack_HEXData_Frame(&Str_Data_Write, Convert_uint_2int(Tem_u32), CPC_SCALE_VOLTAGE);
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
        Copy_String_2(&Str_Data_Write, &Unit_Voltage);  //don vi
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
    }
    //Dong dien
    for(count = 0; count<3; count++)  //3 gia tri dong dien
    {
        Tem_u32 = 0;
        for(j = 0; j<4; j++)
            Tem_u32 |= Meter_TempBuff[i++] << (8*j);
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
        Pack_HEXData_Frame(&Str_Data_Write, Convert_uint_2int(Tem_u32), CPC_SCALE_CURRENT);
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
        Copy_String_2(&Str_Data_Write, &Unit_Current);  //don vi
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
    }  
    //Freq
    Tem_u16 = 0;
    for(j = 0; j<2; j++)
        Tem_u16 |= Meter_TempBuff[i++] << (8*j);
    
    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
    Pack_HEXData_Frame(&Str_Data_Write, Convert_uint16_2int16((uint16_t)Tem_u16), CPC_SCALE_FREQ);
    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
    Copy_String_2(&Str_Data_Write, &Unit_Freq);    //don vi
    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
    //
    i += 4;
    i += 16;  //16 byte goc lech pha
    //he so cong suat
    for(count = 0; count<3; count++)  //3 gia tri Powfactor bo cai cuoi
    {
        Tem_u16 = 0;
        for(j = 0; j<2; j++)
            Tem_u16 |= Meter_TempBuff[i++] << (8*j);
        
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
        Pack_HEXData_Frame(&Str_Data_Write, Convert_uint16_2int16(Tem_u16), CPC_SCALE_POW_FACTOR);
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
    }
    
    i += 2;
    //goi 2:  
    //+ C�ng suat t�c dug pha A, B, C tong:  21.7.0	41.7.0  61.7.0  1.7.0
    for(count = 0; count <4; count++)  //4 gia tri active power
    {
        Tem_u64 = 0;
        for(j = 0; j<8; j++)
            Tem_u64 |= ((uint64_t) Meter_TempBuff[i++]) << (8*j);
        
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
        Pack_HEXData_Frame(&Str_Data_Write, Convert_uint64_2int64(Tem_u64), CPC_SCALE_ACTIVE_POW); 
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
        Copy_String_2(&Str_Data_Write, &Unit_Active_Intan);    //don vi
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
    }
    
    //C�ng suat phan kh�ng pha A, B, C, tong.  
    //data
    for(count = 0; count <4; count++)  //4 gia tri active power
    {
        Tem_u64 = 0;
        for(j = 0; j<8; j++)
            Tem_u64 |= ((uint64_t) Meter_TempBuff[i++]) << (8*j);
        
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
        Pack_HEXData_Frame(&Str_Data_Write, Convert_uint64_2int64(Tem_u64), CPC_SCALE_REACTIVE_POW); 
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
        Copy_String_2(&Str_Data_Write, &Unit_Reactive_Intan);    //don vi
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
    }
    //appr
    for(count = 0; count <4; count++)  //4 gia tri active power
    {
        Tem_u64 = 0;
        for(j = 0; j<8; j++)
            Tem_u64 |= ((uint64_t) Meter_TempBuff[i++]) << (8*j);
        
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
        Pack_HEXData_Frame(&Str_Data_Write, Convert_uint64_2int64(Tem_u64), CPC_SCALE_APPRENT_POW); 
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
        Copy_String_2(&Str_Data_Write, &Unit_Apprent_Intan);    //don vi
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
    }
    //goi 3: bo qua
    i = 192;  //54 byte goi 1. 96 byte goi 2 .32 goi 3. bat dau sang thanh ghi tong 72 byte
    //nang luong Importwh tong bieu 1 2 3  
    for(count = 0; count < 4; count++)
    {
        Tem_u64 = 0;
        for(j = 0; j < 8; j++)
            Tem_u64 |= ((uint64_t) Meter_TempBuff[i++]) << (8*j);
        //ghi data vao
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
        Pack_HEXData_Frame_Uint64(&Str_Data_Write, Tem_u64, CPC_SCALE_TOTAL_ENERGY);
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
        if(count < 2) 
            Copy_String_2(&Str_Data_Write, &Unit_Active_EnTotal); 
        else 
            Copy_String_2(&Str_Data_Write, &Unit_Reactive_EnTotal); 
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';

    }
    
    Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Data_Inbuff, &Str_Data_Write);
    Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
    
    i += 40;  //bo qua 5 dai luong tiep the0
    //tariff
    CPC_Pack_Tariff_Demand_2(&Get_Meter_Info, &Meter_TempBuff[i], &Meter_TempBuff[i+288], DATA_OPERATION);  //maxdemand tu vi tri i+288. Maxdemand 8*37 byte
    i = i + 288 + 296; 
    //Event. .
    CPC_Pack_Event(&Get_Meter_Info, i);
    
    //dong goi Tu Ti va stime
    Reset_Buff(&Str_Data_Write);
    //them tu ti va ket thuc goi tin
    Add_TuTI_toPayload(&Get_Meter_Info);
    
    //Timlabel
    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
    Copy_String_STime(&Str_Data_Write,Met_Var.STimeIntan);
    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
    
    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
    Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) Get_Meter_Info.Numqty, 0);
    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')'; 
    
    Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.PosNumqty, &Str_Data_Write);
    
    return 1;
}




void CPC_Pack_Tariff_Demand_2(Meter_Comm_Struct* Get_Meter, uint8_t* Data_Tariff, uint8_t* Data_Maxdemand, uint8_t type)
{
    uint8_t     Count = 0;
    uint16_t    i = 0;
    uint8_t     j = 0;
    truct_String    Str_Data_Write={&BuffRecord[0], 0}; 
    uint8_t         Soure = 0;
    uint64_t        Second = 0;
    ST_TIME_FORMAT  STimeMaxD;
    uint64_t        TempData = 0;
    
    uint8_t         IndexTariff_Plus = 0;
    uint8_t         IndexTariff_Sub = 0;
    uint8_t         PosOffset = POS_OBIS_TARIFF_TSVH;  //vi tri obis cua TSVh
    
    if(type == DATA_HISTORICAL)
        PosOffset = POS_OBIS_TARIFF_BILL;
    
    //lay source
    for(Count = 0; Count < 32; Count++)  //32 biue gia
    {
        Reset_Buff(&Str_Data_Write);
        Soure = Data_Tariff[i++] ;
        TempData = 0;
        for(j = 0; j < 8; j++)
            TempData |=  ((uint64_t) Data_Tariff[i++]) << (8*j);
        
        switch (Soure)
        {
            case 0x01:  //Active Energy plus
                if(IndexTariff_Plus < 3)
                {
                    Copy_String_toTaget(&Get_Meter->Str_Payload, Get_Meter->Pos_Obis_Inbuff, CPC_OBIS_BIEU[IndexTariff_Plus + PosOffset]);
                    Get_Meter->Pos_Obis_Inbuff += CPC_OBIS_BIEU[IndexTariff_Plus + PosOffset]->Length_u16;  //tang dan bieu
                    Get_Meter->Pos_Data_Inbuff = Get_Meter->Str_Payload.Length_u16;
                    Get_Meter->Numqty ++;
                    IndexTariff_Plus++;
                    //DATA
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                    Pack_HEXData_Frame_Uint64(&Str_Data_Write, TempData, CPC_SCALE_TOTAL_ENERGY);
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                    Copy_String_2(&Str_Data_Write, &Unit_Active_EnTotal); 
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                    
                    Copy_String_toTaget(&Get_Meter->Str_Payload, Get_Meter->Pos_Data_Inbuff, &Str_Data_Write);
                    Get_Meter->Pos_Data_Inbuff = Get_Meter->Str_Payload.Length_u16;
                }
                break;
            case 0x02:  //Active Energy Sub
                if(IndexTariff_Sub < 3)
                {
                    Copy_String_toTaget(&Get_Meter->Str_Payload, Get_Meter->Pos_Obis_Inbuff, CPC_OBIS_BIEU[IndexTariff_Sub + PosOffset + 3]);
                    Get_Meter->Pos_Obis_Inbuff += CPC_OBIS_BIEU[IndexTariff_Sub + PosOffset + 3]->Length_u16;
                    Get_Meter->Pos_Data_Inbuff = Get_Meter->Str_Payload.Length_u16;
                    Get_Meter->Numqty ++;
                    IndexTariff_Sub++;
                    //DATA
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                    Pack_HEXData_Frame_Uint64(&Str_Data_Write, TempData, CPC_SCALE_TOTAL_ENERGY);
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                    Copy_String_2(&Str_Data_Write, &Unit_Active_EnTotal); 
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                    
                    Copy_String_toTaget(&Get_Meter->Str_Payload, Get_Meter->Pos_Data_Inbuff, &Str_Data_Write);
                    Get_Meter->Pos_Data_Inbuff = Get_Meter->Str_Payload.Length_u16;
                }
                break;
            default:
                break;
        }
    }
    //Dong goi maxdemand
    IndexTariff_Plus = 0;   
    IndexTariff_Sub = 0;
    i = 0;
    for(Count = 0; Count < 8; Count++)  //8 biue gia: time va value chi lay cai dau tien
    {
        Reset_Buff(&Str_Data_Write);
        Soure = Data_Maxdemand[i++] ;
        TempData = 0;
        for(j = 0; j < 8; j++)
            TempData |=  ((uint64_t) Data_Maxdemand[i++]) << (8*j);
        Second = 0;
        for(j = 0; j < 4; j++)
            Second |=  Data_Maxdemand[i++] << (8*j);
        
        switch (Soure)
        {
            case 0x01:  //Active Energy plus
                if(IndexTariff_Plus < 3)
                {
                    Copy_String_toTaget(&Get_Meter->Str_Payload, Get_Meter->Pos_Obis_Inbuff, CPC_OBIS_BIEU[IndexTariff_Plus + PosOffset + 6]);
                    Get_Meter->Pos_Obis_Inbuff += CPC_OBIS_BIEU[IndexTariff_Plus + PosOffset + 6]->Length_u16;  //tang dan bieu
                    Get_Meter->Pos_Data_Inbuff = Get_Meter->Str_Payload.Length_u16;
                    Get_Meter->Numqty ++;
                    IndexTariff_Plus++;
                    //data
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                    Pack_HEXData_Frame_Uint64(&Str_Data_Write, TempData, CPC_SCALE_MAX_DEMAND);  
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                    Copy_String_2(&Str_Data_Write, &Unit_MAXDEMAND);
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                    //convert time(uint32) sang stime RTC.
                    Epoch_to_date_time(&STimeMaxD, Second, 1);
                    
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
                    Copy_String_STime(&Str_Data_Write,STimeMaxD);
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                    
                    Copy_String_toTaget(&Get_Meter->Str_Payload, Get_Meter->Pos_Data_Inbuff, &Str_Data_Write);
                    Get_Meter->Pos_Data_Inbuff = Get_Meter->Str_Payload.Length_u16;
                }
                break;
            case 0x02:  //Active Energy Sub 
                if(IndexTariff_Sub < 3)
                {
                    Copy_String_toTaget(&Get_Meter->Str_Payload, Get_Meter->Pos_Obis_Inbuff, CPC_OBIS_BIEU[IndexTariff_Sub + PosOffset + 9]);
                    Get_Meter->Pos_Obis_Inbuff += CPC_OBIS_BIEU[IndexTariff_Sub + PosOffset + 9]->Length_u16;
                    Get_Meter->Pos_Data_Inbuff = Get_Meter->Str_Payload.Length_u16;
                    Get_Meter->Numqty ++;
                    IndexTariff_Sub++;
     
                    //data
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                    Pack_HEXData_Frame_Uint64(&Str_Data_Write, TempData, CPC_SCALE_MAX_DEMAND);  
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                    Copy_String_2(&Str_Data_Write, &Unit_MAXDEMAND);
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                    //convert time(uint32) sang stime RTC.
                    Epoch_to_date_time(&STimeMaxD, Second, 1);
                    
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
                    Copy_String_STime(&Str_Data_Write,STimeMaxD);
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                    
                    Copy_String_toTaget(&Get_Meter->Str_Payload, Get_Meter->Pos_Data_Inbuff, &Str_Data_Write);
                    Get_Meter->Pos_Data_Inbuff = Get_Meter->Str_Payload.Length_u16;
                }
                break;
            default:
                break;
        }
        
        i += 24; //bo qua 2 value va time tiep theo
    }
}
    


void CPC_Decode_Event (Meter_Comm_Struct* Get_Meter, uint8_t Pack)
{
	uint16_t length = 0;
	uint8_t i = 0;

    for(i = 0; i < 13; i++) //1 byte source + 4 byte total + 4byte starttime + 4 byte stoptime = 13 byte
    {
        Meter_TempBuff[Get_Meter->Data_Buff_Pointer_ui16++] = Convert2ByteHexStringto_1Hex(UART1_Control.UART1_Str_Recei.Data_a8 + length + 3);
        length += 2;
    }
}



//chi lay cac event TSVH
uint8_t CPC_Pack_Event (Meter_Comm_Struct* Get_Meter, uint16_t Pos)
{
    uint16_t        i = Pos;
    uint8_t         Soure_temp = 0;
    uint32_t        Total = 0;
    uint32_t        TimeStart = 0;
    uint32_t        TimeStop = 0;
    uint8_t         j = 0;
    ST_TIME_FORMAT		TimeStopRTC;
    ST_TIME_FORMAT		TimeStartRTC;
        
    while(i < Get_Meter->Data_Buff_Pointer_ui16)
    {
    	//lay source
        Soure_temp = Meter_TempBuff[i++];
        //total 
        Total = 0;
        for(j = 0; j < 4; j++)
            Total |=  Meter_TempBuff[i++] << (8*j);
        
        if(Total == 0xFFFFFFFF) 
            Total = 0;
        //Starttime
        TimeStart = 0;
        for(j = 0; j < 4; j++)
             TimeStart |=  Meter_TempBuff[i++] << (8*j);
        //Stoptime
        TimeStop = 0;
        for(j = 0; j < 4; j++)
             TimeStop |=  Meter_TempBuff[i++] << (8*j);
        
        if(TimeStart == 0xFFFFFFFF) 
            TimeStart = 0;
        if(TimeStop == 0xFFFFFFFF) 
            TimeStop = 0;
        //Convert sang RTC luon
        Epoch_to_date_time(&TimeStartRTC,  TimeStart, 1);
        Epoch_to_date_time(&TimeStopRTC,  TimeStop, 1);

        if(CPC_Str_OEvent103[Soure_temp].StartCountOB != NULL)
        {
            //ghi obis count vao
            Copy_String_toTaget(&Get_Meter->Str_Payload, Get_Meter->Pos_Obis_Inbuff, CPC_Str_OEvent103[Soure_temp].StartCountOB);
            Get_Meter->Pos_Obis_Inbuff += CPC_Str_OEvent103[Soure_temp].StartCountOB->Length_u16;
            Get_Meter->Pos_Data_Inbuff = Get_Meter->Str_Payload.Length_u16;
            Get_Meter->Numqty++;
            //data
            *(Get_Meter->Str_Payload.Data_a8 + Get_Meter->Str_Payload.Length_u16++) = '('; 
            Pack_HEXData_Frame_Uint64(&Get_Meter->Str_Payload, (uint64_t) Total, 0); 
            *(Get_Meter->Str_Payload.Data_a8 + Get_Meter->Str_Payload.Length_u16++) = ')';
            Get_Meter->Pos_Data_Inbuff = Get_Meter->Str_Payload.Length_u16;
        }
        //time start
        if(CPC_Str_OEvent103[Soure_temp].StartTimeOB != NULL)
        {
            //ghi obis count vao
            Copy_String_toTaget(&Get_Meter->Str_Payload, Get_Meter->Pos_Obis_Inbuff, CPC_Str_OEvent103[Soure_temp].StartTimeOB);
            Get_Meter->Pos_Obis_Inbuff += CPC_Str_OEvent103[Soure_temp].StartTimeOB->Length_u16;
            Get_Meter->Pos_Data_Inbuff = Get_Meter->Str_Payload.Length_u16;
            Get_Meter->Numqty++;
            //data
            *(Get_Meter->Str_Payload.Data_a8 + Get_Meter->Str_Payload.Length_u16++) = '(';
            Copy_String_STime(&Get_Meter->Str_Payload, TimeStartRTC);
            *(Get_Meter->Str_Payload.Data_a8 + Get_Meter->Str_Payload.Length_u16++) = ')';
            Get_Meter->Pos_Data_Inbuff = Get_Meter->Str_Payload.Length_u16;
        }
            
        //time stop
        if(CPC_Str_OEvent103[Soure_temp].StopTimeOB != NULL)
        {
            //ghi obis count vao
            Copy_String_toTaget(&Get_Meter->Str_Payload, Get_Meter->Pos_Obis_Inbuff, CPC_Str_OEvent103[Soure_temp].StopTimeOB);
            Get_Meter->Pos_Obis_Inbuff += CPC_Str_OEvent103[Soure_temp].StopTimeOB->Length_u16;
            Get_Meter->Pos_Data_Inbuff = Get_Meter->Str_Payload.Length_u16;
            Get_Meter->Numqty++;
            //data
            *(Get_Meter->Str_Payload.Data_a8 + Get_Meter->Str_Payload.Length_u16++) = '(';
            Copy_String_STime(&Get_Meter->Str_Payload, TimeStopRTC);
            *(Get_Meter->Str_Payload.Data_a8 + Get_Meter->Str_Payload.Length_u16++) = ')';
            Get_Meter->Pos_Data_Inbuff = Get_Meter->Str_Payload.Length_u16;
        } 
    }
    return 1;
}


//Doc bill CPC

void CPC_MBillInsertTimeReq (void)
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




uint8_t CPC_MBill_ExtractDataFunc (void)
{
	uint8_t     i = 0, temp =0;
	uint16_t    length = 0;
	
    temp = CPC_MeterBillMessIDTable[Get_Meter_Billing.Mess_Step_ui8];
    
    //check byte thu 2. xem dung nhu byte dau tien cua data set khong? 
    if(*(UART1_Control.UART1_Str_Recei.Data_a8 + 1) == CPC_LASTBILL_ObisCode[temp][0])  //
    {
        switch(Get_Meter_Billing.Mess_Step_ui8)
        {
            case 0: 
                if(CPC_Decode_StimeBill() == 0)
                    return 0;
                break;
            case 1:
                //l�y lan luot cac gia tri: 12 dai luong. moi gia tri 8byte
                for(i = 0; i < 96; i++)
                {
                    Meter_TempBuff[Get_Meter_Billing.Data_Buff_Pointer_ui16++] = Convert2ByteHexStringto_1Hex(UART1_Control.UART1_Str_Recei.Data_a8 + length + 3);
                    length += 2;
                }
                break;
            case 2:
            case 3:
            case 4:
            case 5: CPC_Decode_Tariff (&Get_Meter_Billing, Get_Meter_Billing.Mess_Step_ui8 - 2); break;
            case 6:
            case 7:
            case 8: 
            case 9: CPC_Decode_MAXDemand_LASTBILL (&Get_Meter_Billing, Get_Meter_Billing.Mess_Step_ui8 - 6); break;
                break;
            default:
                break;
        }
        return 1;
    }
    
    return 0;
}

uint8_t CPC_Decode_StimeBill (void)
{
    
    uint32_t        DiffValue = 0;
    uint32_t        sTime = 0;
    uint16_t        i = 0, length = 0;
    
    //4byte time UTC. don vi gi�y.
    for(i = 0; i < 4; i++)
    {
        sTime |=  (Convert2ByteHexStringto_1Hex(UART1_Control.UART1_Str_Recei.Data_a8 + length + 3)) << (8*i);
        length += 2;
    }
    Epoch_to_date_time(&Met_Var.sTimeLastBill, sTime, 1);
     
    // Check stime xem nam trong khoang request khong
    if(sInformation.Flag_Request_Bill == 1)
    {
        //so sanh xem nam trong khoang start time va stop time nua k. de dung lai hoac bo qua
        if(Check_DiffTime (Met_Var.sTimeLastBill, sInformation.EndTime_GetBill,&DiffValue) == 1)  //neu thoi gian hien tai lon hon EndTime_GetBill. thi bo qua
        {
            _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nThoi gian Bill > EndTime", 26, 1000);
            return 0; //return 0 de chay doc tiep cac record khac 
        }
        //
        if(Check_DiffTime (sInformation.StartTime_GetBill, Met_Var.sTimeLastBill, &DiffValue) == 1) //Neu StartTime_GetBill lon hon thoi gian hien tai doc ra thi dung viec doc
        {
            _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nThoi gian Bill < StartTime", 28, 1000);
            sInformation.Flag_Stop_ReadBill = 1;  //dung flag nay de dung vi�c doc.
            return 0;
        }
    }     
    //
    Get_Meter_Billing.Flag_ui8 = 1;   //muon flag nay de bao doc thanh cong
    
    return 1;
}
    

void CPC_In1Pack_TariffLastBill(uint8_t Pack)
{
	uint16_t length = 0;
	uint8_t i = 0;

	for(i = 0; i < 72; i++)   //(1+8) *8) = 
	{
		Meter_TempBuff[Get_Meter_Billing.Data_Buff_Pointer_ui16++] = Convert2ByteHexStringto_1Hex(UART1_Control.UART1_Str_Recei.Data_a8 + length + 3);
		length += 2;
	}
}


void CPC_MBill_SendData(void)
{
    uint8_t         Temp_BBC = 0;
    uint8_t         aTempData[25];
    truct_String    aNumqty = {&aTempData[0], 0};
      
    if(Get_Meter_Billing.Flag_ui8 == 1)
    {
        _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nREAD BILL OK", 18, 1000);
        
        CPC_Pack_Lastbill_103();  //dong goi
        
        //them tuti
        Add_TuTI_toPayload(&Get_Meter_Billing);
        //Them thoi gian Bill
        *(aNumqty.Data_a8 + aNumqty.Length_u16++) = '(';
        if(Get_Meter_Billing.Numqty > 3)
            Copy_String_STime(&aNumqty, Met_Var.sTimeLastBill);
        *(aNumqty.Data_a8 + aNumqty.Length_u16++) = ')'; 
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

void CPC_Send_MBill_Mess (void)
{
    uint8_t     temp=0, i = 0;
    uint8_t     aObisTemp[11];
    
	temp = CPC_MeterBillMessIDTable[Get_Meter_Billing.Mess_Step_ui8];
    
    for(i = 0; i < 11; i++)    //bo 1 byte cuoi di
        aObisTemp[i] = CPC_LASTBILL_ObisCode[temp][i];
    
    //thay 2 byte 4 5 SttBill
    temp = ConvertHextoAscii((Met_Var.SttLastBill &0xF0) >> 4);
    aObisTemp[4] = temp;    //khac nhau byte thu 4
    temp = ConvertHextoAscii(Met_Var.SttLastBill &0x0F);
    aObisTemp[5] = temp;    //khac nhau byte thu 5
    
    CPC_IEC62056_21_Command (CMD_READ, &aObisTemp[0], 11);
}


//D�ng goi ban tin chot
void CPC_Pack_Lastbill_103 (void) 
{ 
    uint16_t        i = 10;
    uint64_t        TempData = 0;
    uint8_t         j = 0;
    uint8_t         Count = 0;
    truct_String    Str_Data_Write={&BuffRecord[0], 0}; 
    
    Write_Header_His_Push103();
    //them c�c gia tri thanh ghi dien nang tong vao  
    Reset_Buff(&Str_Data_Write);
    Copy_String_2(&Str_Data_Write, &Str_Ob_AcImTotal_Chot); 
    Copy_String_2(&Str_Data_Write, &Str_Ob_AcExTotal_Chot);
    Copy_String_2(&Str_Data_Write, &Str_Ob_ReImTotal_Chot);        
    Copy_String_2(&Str_Data_Write, &Str_Ob_ReExTotal_Chot);
    
    Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Obis_Inbuff, &Str_Data_Write);
    Get_Meter_Billing.Pos_Obis_Inbuff += Str_Data_Write.Length_u16;
    Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
    Get_Meter_Billing.Numqty += 4;
    //ghi data thanh ghi tong
    i = 10;
    Reset_Buff(&Str_Data_Write);
    for(Count = 0; Count < 4; Count++)
    {
        TempData = 0;
        for(j = 0; j < 8; j++)
            TempData |= ((uint64_t)Meter_TempBuff[i++]) << (8*j);
        //ghi data vao
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
        Pack_HEXData_Frame_Uint64(&Str_Data_Write, TempData, CPC_SCALE_TOTAL_ENERGY);
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
        if(Count < 2) Copy_String_2(&Str_Data_Write, &Unit_Active_EnTotal); 
        else Copy_String_2(&Str_Data_Write, &Unit_Reactive_EnTotal); 
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';

    }
    
    Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Data_Inbuff, &Str_Data_Write);
    Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
    //bieu gia
    
    i = 106; //bat dau vi tri cua bieu gia.do buff luu bat dau tu 10
    CPC_Pack_Tariff_Demand_2(&Get_Meter_Billing, &Meter_TempBuff[i], &Meter_TempBuff[i+288], DATA_HISTORICAL);
}



//Event
void CPC_Send_EVENT_Mess (void)
{
    uint8_t temp=0;
	
	temp = CPC_MeterEvenMessIDTable[Get_Meter_Event.Mess_Step_ui8];
    
    CPC_IEC62056_21_Command (CMD_READ, &CPC_TSVH_ObisCode[temp][0], 11);
}


void CPC_EVENT_SendData(void)
{
    uint8_t         Temp_BBC = 0;
    uint8_t         aTempData[10];
    truct_String    aNumqty = {&aTempData[0], 0};
    
    _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nREAD Even OK", 14, 1000);
    
    CPC_PackEvent_103();
    
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


uint8_t CPC_MEven_ExtractDataFunc (void)
{
    uint8_t         temp = 0;
    
    temp = CPC_MeterEvenMessIDTable[Get_Meter_Event.Mess_Step_ui8];
    
    //check byte thu 2. xem dung nhu byte dau tien cua data set khong? 
    if(*(UART1_Control.UART1_Str_Recei.Data_a8 + 1) == CPC_TSVH_ObisCode[temp][0])  
    {
        if(Get_Meter_Event.Mess_Step_ui8 <= 22)
            CPC_Decode_Event(&Get_Meter_Event, Get_Meter_Event.Mess_Step_ui8);       
        return 1;
    }

    return 0;
}

void CPC_PackEvent_103 (void)
{
    //header cho ban tin event
    Header_event_103(&Get_Meter_Event.Str_Payload, 1);
    CPC_Pack_Event(&Get_Meter_Event, 10);
}


//lpf
void CPC_Send_Lpf_Mess (void)
{
    uint8_t     Temp=0, i = 0;
	uint8_t     aObis[11];
    uint8_t     Index_U8 = 0;
    
	Temp = CPC_MeterLpfMessIDTable[Get_Meter_LProf.Mess_Step_ui8];
    
    switch(Get_Meter_LProf.Mess_Step_ui8)
    {
        case 0:
            for(i = 0; i < 11; i++)
                aObis[i] = CPC_LPF_ObisCode[Temp][i];
            
            Met_Var.Loadpf.IndexPack     = 1;
            break;
        case 1:
            for(i = 0; i < 11; i++)
                aObis[i] = CPC_LPF_ObisCode[Temp][i];
            
            Index_U8 = (uint8_t) Met_Var.Loadpf.IndexDay;   //lay byte thap truoc
            //Set Ngay doc Lpf v�o total Pack
            Temp = ConvertHextoAscii((Index_U8 &0xF0) >> 4);
            aObis[2] = Temp;    
            Temp = ConvertHextoAscii(Index_U8 &0x0F);
            aObis[3] = Temp;    
            
            Index_U8 = (uint8_t) (Met_Var.Loadpf.IndexDay >> 8);     //lay byte cao
            Temp = ConvertHextoAscii((Index_U8 &0xF0) >> 4);
            aObis[4] = Temp;    
            Temp = ConvertHextoAscii(Index_U8 &0x0F);
            aObis[5] = Temp;    
            
            Met_Var.Loadpf.IndexPack     = 1; //reset chi so
            break;
        case 2:
            if(Met_Var.Loadpf.IndexPack <= Met_Var.Loadpf.TotalPack)
            {
                for(i = 0; i < 11; i++)
                    aObis[i] = CPC_LPF_ObisCode[Temp][i];
                //
                Index_U8 = (uint8_t) Met_Var.Loadpf.IndexDay;    //lay byte thap truoc
                //Set Ngay doc Lpf v�o total Pack
                Temp = ConvertHextoAscii((Index_U8 &0xF0) >> 4);
                aObis[2] = Temp;    
                Temp = ConvertHextoAscii(Index_U8 &0x0F);
                aObis[3] = Temp;    
                
                Index_U8 = (uint8_t) (Met_Var.Loadpf.IndexDay >> 8);     //lay byte cao
                Temp = ConvertHextoAscii((Index_U8 &0xF0) >> 4);
                aObis[4] = Temp;    
                Temp = ConvertHextoAscii(Index_U8 &0x0F);
                aObis[5] = Temp;   
                
                //thay 2 byte 6 7 bang Met_Var.Loadpf.IndexPack
                Temp = ConvertHextoAscii((Met_Var.Loadpf.IndexPack &0xF0) >> 4);
                aObis[6] = Temp;    //khac nhau byte thu 6
                Temp = ConvertHextoAscii(Met_Var.Loadpf.IndexPack &0x0F);
                aObis[7] =  Temp;    //khac nhau byte thu 7
                
                Met_Var.Loadpf.IndexPack++;
            }else{
                for(i = 0; i < 11; i++)
                    aObis[i] = CPC_LPF_ObisCode[CPC_MeterLpfMessIDTable[3]][i];
                Get_Meter_LProf.Mess_Step_ui8++;    //cong len va gui ban tin cau h�nh. Sau do se khong dong goi ban tin nay. va check ket thuc
            }
            break;
        default: 
            break;
    }
           
    CPC_IEC62056_21_Command (CMD_READ, &aObis[0], 11);
}


void CPC_Lpf_SendData(void)
{
    
}


uint8_t CPC_Lpf_ExtractDataFunc (void)
{
    uint8_t         temp = 0;
    
    temp = CPC_MeterLpfMessIDTable[Get_Meter_LProf.Mess_Step_ui8];
    
    //check byte thu 2. xem dung nhu byte dau tien cua data set khong? 
    if(*(UART1_Control.UART1_Str_Recei.Data_a8 + 1) == CPC_LPF_ObisCode[temp][0])  
    {
        switch(Get_Meter_LProf.Mess_Step_ui8)
        {
            case 0:
                CPC_Get_ConfigChart(Get_Meter_LProf.Mess_Step_ui8);
                break;
            case 1:
                CPC_Get_ToTalPack_inday(Get_Meter_LProf.Mess_Step_ui8);
                break;
            case 2:
                CPC_Pack_1Mess_lpf_1(Get_Meter_LProf.Mess_Step_ui8);
                Get_Meter_LProf.Mess_Step_ui8--; //de lan tiep theo lai chay vao day
                break;
            default:
                _fPrint_Via_Debug(&UART_SERIAL, UART1_Control.UART1_Str_Recei.Data_a8, UART1_Control.UART1_Str_Recei.Length_u16, 1000);
                break;
        }

        return 1;
    }

    return 0;
}



void CPC_Get_ConfigChart (uint8_t pack)
{
	uint16_t length = 0;
	uint16_t i = 0;
	
    _fPrint_Via_Debug(&UART_SERIAL, UART1_Control.UART1_Str_Recei.Data_a8, UART1_Control.UART1_Str_Recei.Length_u16, 1000);
    
	//4 byte cofi va 2 byte chu ki tich phan
	Met_Var.Loadpf.Config_Source = 0;
	
	for(i = 0; i < 4; i++)
	{
		Met_Var.Loadpf.Config_Source |=  (Convert2ByteHexStringto_1Hex(UART1_Control.UART1_Str_Recei.Data_a8 + length + 3)) << (8*i);
		length += 2;
	}
	
	for(i = 0; i < 2; i++)
	{
		Met_Var.Loadpf.Period |=  (Convert2ByteHexStringto_1Hex(UART1_Control.UART1_Str_Recei.Data_a8 + length + 3)) << (8*i);
		length += 2;
	}
	//Giai ma cac dai luong va period luon
	CPC_Decode_LpfConfig(Met_Var.Loadpf.Config_Source);
}

void CPC_Decode_LpfConfig (uint32_t ValueConfig)
{
	uint8_t i = 0;
	uint8_t j = 0;
	
    Met_Var.Loadpf.NumChannel = 0;
	for(i = 0; i <18; i++)
	{
		if((uint8_t)((ValueConfig >>i) & 0x00000001) == 1)
		{
			for(j = 0; j < 24; j++)
				Met_Var.Loadpf.DataLpf.Record.Data[Met_Var.Loadpf.NumChannel].Source = i;
			
			Met_Var.Loadpf.NumChannel++;
		}
	}
}


void CPC_Get_ToTalPack_inday (uint8_t pack)
{
	uint16_t length = 0;
	uint16_t i = 0;
	//2byte
	Met_Var.Loadpf.TotalPack = 0;
    _fPrint_Via_Debug(&UART_SERIAL, UART1_Control.UART1_Str_Recei.Data_a8, UART1_Control.UART1_Str_Recei.Length_u16, 1000);
    
	for(i = 0; i < 2; i++)
	{
		Met_Var.Loadpf.TotalPack |=  (Convert2ByteHexStringto_1Hex(UART1_Control.UART1_Str_Recei.Data_a8 + length + 3)) << (8*i);
		length += 2;
	}
}


void CPC_Pack_1Mess_lpf_1(uint8_t Pack)
{
	uint16_t        length = 0, i = 0, j = 0;
    uint8_t         Buff_temp[20];
    truct_String    Str_eventlpf = {&Buff_temp[0], 0};
    uint8_t         Status = 0;
  
    _fPrint_Via_Debug(&UART_SERIAL, UART1_Control.UART1_Str_Recei.Data_a8, UART1_Control.UART1_Str_Recei.Length_u16, 1000);
    //Neu nhu co byte du o truoc, chen vao dau data buff uart. Nhung sau 3 ki tu dau
    for(j = (UART1_Control.UART1_Str_Recei.Length_u16 - 1); j>=3; j--)    //dich cac byte ra phia sau: StrUartTemp.Length_u16 byte
        *(UART1_Control.UART1_Str_Recei.Data_a8 + j + StrUartTemp.Length_u16) = *(UART1_Control.UART1_Str_Recei.Data_a8 + j);
    for(i = 0; i < StrUartTemp.Length_u16; i++)   //chuyen cac byte vao dau data
         *(UART1_Control.UART1_Str_Recei.Data_a8 + i + 3) = *(StrUartTemp.Data_a8 + i);
    //cong them Length Uart them phan du
    UART1_Control.UART1_Str_Recei.Length_u16 += StrUartTemp.Length_u16;
    
    length += 3;  //3 byte dau tien 
	//lay file header: 1byte status
    while(length < (UART1_Control.UART1_Str_Recei.Length_u16 - 3 - (2+8*Met_Var.Loadpf.NumChannel)))   
    {
        Met_Var.Loadpf.DataLpf.Header.Status = Convert2ByteHexStringto_1Hex(UART1_Control.UART1_Str_Recei.Data_a8 + length);
        length += 2;
        Status = Met_Var.Loadpf.DataLpf.Header.Status;
        //Reset data
        for(i = 0; i < Met_Var.Loadpf.NumChannel; i++)
            Met_Var.Loadpf.DataLpf.Record.Data[i].Value = 0;
    
        switch(Met_Var.Loadpf.DataLpf.Header.Status)
        {
            case 0xF0:  //new date
            case 0xF1:
            case 0xF2:
            case 0xF3:
            case 0xF4:
            case 0xF5:
            case 0xF6:        
            case 0xF7:
                //4byte UTC
                Met_Var.Loadpf.DataLpf.Header.UTC_Time = 0;
                for(j = 0; j < 4; j++)
                {
                    Met_Var.Loadpf.DataLpf.Header.UTC_Time |=  (Convert2ByteHexStringto_1Hex(UART1_Control.UART1_Str_Recei.Data_a8 + length)) << (8*j);
                    length += 2;
                }
                //2byte Period
                Met_Var.Loadpf.DataLpf.Header.Period = 0;
                for(j = 0; j < 2; j++)
                {
                    Met_Var.Loadpf.DataLpf.Header.Period |=  (Convert2ByteHexStringto_1Hex(UART1_Control.UART1_Str_Recei.Data_a8 + length)) << (8*j);
                    length += 2;
                }
                //4byte Config
                Met_Var.Loadpf.DataLpf.Header.Cofig = 0;
                for(j = 0; j < 4; j++)
                {
                    Met_Var.Loadpf.DataLpf.Header.Cofig |=  (Convert2ByteHexStringto_1Hex(UART1_Control.UART1_Str_Recei.Data_a8 + length)) << (8*j);
                    length += 2;
                }
                //1byte Crc
                Met_Var.Loadpf.DataLpf.Header.Crc = Convert2ByteHexStringto_1Hex(UART1_Control.UART1_Str_Recei.Data_a8 + length);
                length += 2;
                if((Status == 0xF2) || (Status == 0xF4) || (Status == 0xF5) || (Status == 0xF7))  //neu la F2, F4 F5 F7 thi can lay thoi gian tron. cua khung du lieu phia truoc
                    Met_Var.Loadpf.DataLpf.Record.Stime = (Met_Var.Loadpf.DataLpf.Header.UTC_Time/Met_Var.Loadpf.DataLpf.Header.Period) *Met_Var.Loadpf.DataLpf.Header.Period;
                else  Met_Var.Loadpf.DataLpf.Record.Stime =  Met_Var.Loadpf.DataLpf.Header.UTC_Time; 
                
                break;
            case 0xFF:
                return;
            default:
                //neu vao day thi tiep cai data theo status cu
                length -= 2;   //tru de lay lai ERROR
                Met_Var.Loadpf.DataLpf.Header.Status = 0;
                
                if(Get_Meter_LProf.Flag_Start_Pack == 0)
                {
                    Met_Var.Loadpf.DataLpf.Header.Status_Before = 0;
                    Epoch_to_date_time(&Met_Var.Loadpf.DataLpf.Record.sTimeRTC, Met_Var.Loadpf.DataLpf.Record.Stime, 1);   //stime cong them truoc de dong goi
                    Reset_Buff(&Get_Meter_LProf.Str_Payload);
                    CPC_Pack_Header_lpf_Pushdata103(&Get_Meter_LProf.Str_Payload, Met_Var.Loadpf.DataLpf.Record.sTimeRTC, 0);
                    Get_Meter_LProf.Flag_Start_Pack = 1;
                }
                Met_Var.Loadpf.DataLpf.Record.Stime = (Met_Var.Loadpf.DataLpf.Record.Stime/Met_Var.Loadpf.DataLpf.Header.Period) *Met_Var.Loadpf.DataLpf.Header.Period + Met_Var.Loadpf.DataLpf.Header.Period;
                Met_Var.Loadpf.DataLpf.Record.Error = Convert2ByteHexStringto_1Hex(UART1_Control.UART1_Str_Recei.Data_a8 + length);
                length += 2;
                for(i = 0; i < Met_Var.Loadpf.NumChannel; i++)
                {
                    Met_Var.Loadpf.DataLpf.Record.Data[i].Value = 0;
                    for(j = 0; j < 4; j++)
                    {
                        Met_Var.Loadpf.DataLpf.Record.Data[i].Value |=  (Convert2ByteHexStringto_1Hex(UART1_Control.UART1_Str_Recei.Data_a8 + length)) << (8*j);
                        length += 2;
                    }
                }
                Met_Var.Loadpf.DataLpf.Record.Crc = Convert2ByteHexStringto_1Hex(UART1_Control.UART1_Str_Recei.Data_a8 + length);
                length += 2;

                //Dong goi data theo 103
                *(Get_Meter_LProf.Str_Payload.Data_a8 + Get_Meter_LProf.Str_Payload.Length_u16++) = 0x0D;
                *(Get_Meter_LProf.Str_Payload.Data_a8 + Get_Meter_LProf.Str_Payload.Length_u16++) = 0x0A;
                for(i = 0; i < Met_Var.Loadpf.NumChannel; i++)
                {
                    *(Get_Meter_LProf.Str_Payload.Data_a8 + Get_Meter_LProf.Str_Payload.Length_u16++) = '(';
                    Pack_HEXData_Frame(&Get_Meter_LProf.Str_Payload, (int64_t)Convert_float_2int(Met_Var.Loadpf.DataLpf.Record.Data[i].Value, CPC_SCALE_LPF), CPC_SCALE_LPF);
                    *(Get_Meter_LProf.Str_Payload.Data_a8 + Get_Meter_LProf.Str_Payload.Length_u16++) = '*';
                    Copy_String_2(&Get_Meter_LProf.Str_Payload, &Unit_Lpf[Met_Var.Loadpf.DataLpf.Record.Data[i].Source]);    //don vi
                    *(Get_Meter_LProf.Str_Payload.Data_a8 + Get_Meter_LProf.Str_Payload.Length_u16++) = ')';
                }
                //Tu.Ti HSN	
                Copy_String_2(&Get_Meter_LProf.Str_Payload, &Get_Meter_TuTi.Str_Payload); 
                //dung 1 byte status va 1 byte ERROR de phan tich event
                Get_Event_Lpf (&Str_eventlpf, Met_Var.Loadpf.DataLpf.Header.Status_Before,  Met_Var.Loadpf.DataLpf.Record.Error);
                Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Met_Var.Pos_Eventlpf, &Str_eventlpf); 
                Send_MessLpf_toQueue();
                Get_Meter_LProf.Flag_Start_Pack = 0;
                break;  
        }
    }
    
    Reset_Buff(&StrUartTemp);
    //Copy phan du sang 1 buff khac de. Phan du se bang: TongLength_Uart -  (Length da cat + 3byte check dau) -  3byte cuoi.
    for(i = 0; i < (UART1_Control.UART1_Str_Recei.Length_u16 - length - 3); i++)
        *(StrUartTemp.Data_a8 + StrUartTemp.Length_u16++) = *(UART1_Control.UART1_Str_Recei.Data_a8 + length + i);
}


/*<STX>OBIScode(IDmeter)(starttime)(event)(period)(numchanel)(ch1)[(ch2)�]<CR><LF>(ch1value*unit1)[(ch2value*unit2)�]<ETX><BCC>
?	OBIScode: 99.1.0
*/

void CPC_Pack_Header_lpf_Pushdata103 (truct_String* Payload, ST_TIME_FORMAT sTime, uint32_t Event)
{
     uint16_t i = 0;
     
    //STX
    *(Payload->Data_a8 + Payload->Length_u16++) = STX;
    //obiscode Intan
    Copy_String_2(Payload, &Str_OB_LPF);
    
    //DCUID
    *(Payload->Data_a8 + Payload->Length_u16++) = '(';
    Copy_String_2(Payload, &sDCU.sDCU_id); 
    *(Payload->Data_a8 + Payload->Length_u16++) = ')';
    
    //ID Meter
    *(Payload->Data_a8 + Payload->Length_u16++) = '(';
    Copy_String_2(Payload, &sDCU.sMeter_id_now);   
    *(Payload->Data_a8 + Payload->Length_u16++) = ')';
    
    //Meter type
    *(Payload->Data_a8 + Payload->Length_u16++) = '(';
    Copy_String_2(Payload, &Str_MeterType_u8[sDCU.MeterType]);  
    *(Payload->Data_a8 + Payload->Length_u16++) = ')';
    
    //Start time
    *(Payload->Data_a8 + Payload->Length_u16++) = '('; 
    Copy_String_STime(Payload, sTime);
    *(Payload->Data_a8 + Payload->Length_u16++) = ')';
    //event (0000)
    Met_Var.Pos_Eventlpf = Payload->Length_u16;  //luu vi tri lai
   
    //period
    *(Payload->Data_a8 + Payload->Length_u16++) = '('; 
    Pack_HEXData_Frame(Payload,  (Met_Var.Loadpf.DataLpf.Header.Period/60), 0);
    *(Payload->Data_a8 + Payload->Length_u16++) = ')';
    //num channel
    *(Payload->Data_a8 + Payload->Length_u16++) = '('; 
    Pack_HEXData_Frame(Payload,  (Met_Var.Loadpf.NumChannel + 3), 0);
    *(Payload->Data_a8 + Payload->Length_u16++) = ')';
     //Lan luot cac obis cua channel
    for(i = 0; i<Met_Var.Loadpf.NumChannel; i++)
    {
        Copy_String_2(Payload, &Str_Ob_lpf[Met_Var.Loadpf.DataLpf.Record.Data[i].Source]);
    }
    //Tu Ti He so nhan
    Copy_String_2(Payload, &Str_Ob_Tu); 
    Copy_String_2(Payload, &Str_Ob_Ti);  
    Copy_String_2(Payload, &He_So_Nhan);
}


/*	    //thu tu tu bit cao xuong bit thap
     Loi phan cung       Pin yeu               �oi ti so VT-CT	  �oi mat khau	
     Cau h�nh c�ng to	 Thay doi thoi gian	    Cau h�nh moi      Ng�y moi
     Mat dien	         Sai thu tu pha         Qu� d�ng         Qu� �p    	 
     Thap �p	        Nguoc d�ng dien	        Loi pha	         Mat c�n bang pha 	
*/

void Get_Event_Lpf(truct_String* Str, uint8_t header_status, uint8_t Error_record)
{
    uint16_t event = 0;
//    uint8_t test = 0x30;
    uint16_t temp = 0;
    Reset_Buff(Str);

    if((Error_record&0x01) == 1)            event += 0x0001;  //mat can bang pha
    if(((Error_record >> 1) & 0x01) == 1)   event |= 0x0007; //mat ap pha a
    if(((Error_record >> 2) & 0x01) == 1)   event |= 0x0007; //mat ap pha b
    if(((Error_record >> 3) & 0x01) == 1)   event |= 0x0007; //mat ap pha c
    
    if(((Error_record >> 4) & 0x01) == 1)   event |= 0x000B; //nguoc cuc tinh pha a
    if(((Error_record >> 5) & 0x01) == 1)   event |= 0x000B; //nguoc cuc tinh pha b
    if(((Error_record >> 6) & 0x01) == 1)   event |= 0x000B; //nguoc cuc tinh pha c
    
    if(header_status == 0xF0) event|= 0x0E00;     //new date
    if(header_status == 0xF1) event|= 0x0D00;     //new config
    if(header_status == 0xF2) event|= 0x0070;     //mat dien
//    if(header_status == 0xF3) event|= 0x80;       //co dien    
    if(header_status == 0xF4) event|= 0x0B00;    // thay doi thoi gian
    if(header_status == 0xF5) event|= 0x0700;     //new config cong to
    if(header_status == 0xF6) event|= 0x0700;         //new pass
    if(header_status == 0xF7) event|= 0xD000;         // new tu ti
    
    *(Str->Data_a8 + Str->Length_u16++) = '(';
    for(uint8_t i = 0; i < 4; i++)
    {
        temp = (event >> (12- 4*i) & 0x000F);
        if(temp <=9) temp += 0x30;
        else temp += 0x37; 
        *(Str->Data_a8 + Str->Length_u16++) = temp;
    }
    *(Str->Data_a8 + Str->Length_u16++) = ')';
}





