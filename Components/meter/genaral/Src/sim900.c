/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"

#include "variable.h"
#include "myuart.h"
#include "usart.h"
#include "gpio.h"
#include "sim900.h"
#include "t_mqtt.h"
#include "at_commands.h"
#include "onchipflash.h"
#include "pushdata.h"
#include "S25FL216K.h"


/*
 * 		VARIABLES
 */
uint8_t     aDCU_ID[DCU_ID_LENGTH];
uint8_t     aDCU_SERI[DCU_ID_LENGTH];
uint8_t     aSIM_ID[SIM_ID_LENGTH] 	= {"SV0123122488"};
uint8_t     aMeter_ID[METER_LENGTH];

uint8_t             Buff_IPModule[20];
uint8_t             Buff_IMEI_Module[20];
uint8_t             Buff_Recei_Ping[500];

uint8_t             Buff_Cmd[50];
uint8_t             BuffRecei[20];
truct_String        strIP_Dienluc = {(uint8_t*) "172", 3};

//Variale Flag avoid execute 2 time in 1 function
uint8_t     CountFirst = 0;
uint8_t     aDataWriteFLash[8];

/*!
 * Number of days in each month on a normal year
 */
uint8_t     DaysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
uint8_t     DaysInMonthLeapYear[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

uint8_t		Read_Meter_ID_Success 			= 0;
uint8_t		Read_Meter_ID_Change			= 0;



/*--------------------------Function---------------------------*/
void Init_DCU_METER_ID(void)
{
	uint8_t temp=0;
	uint8_t var;
    uint8_t Buff_temp[50];
    uint8_t Length = 0;
	//DCU ID
	temp = *(__IO uint8_t*)ADDR_DCUID;  
	if (temp != 0xFF)
	{
		sDCU.sDCU_id.Length_u16 = *(__IO uint8_t*)(ADDR_DCUID+1);
        if(sDCU.sDCU_id.Length_u16 > DCU_ID_LENGTH)
            sDCU.sDCU_id.Length_u16 = DCU_ID_LENGTH;     
		OnchipFlashReadData(ADDR_DCUID+2,aDCU_ID,sDCU.sDCU_id.Length_u16);
		sDCU.sDCU_id.Data_a8 = aDCU_ID;
        sDCU.Init_ID_Flash = 1;
	}
	else
	{
        aDCU_ID[0] 	= '1'; aDCU_ID[1] 	= '2'; aDCU_ID[2] 	= '3'; aDCU_ID[3] 	= '4'; aDCU_ID[4] = '5'; aDCU_ID[5] = '6';
		aDCU_ID[6] 	= '7'; aDCU_ID[7] 	= '0'; aDCU_ID[8] 	= '6'; aDCU_ID[9] 	= '9'; aDCU_ID[10] = '2'; aDCU_ID[11] = '5';
        
		sDCU.sDCU_id.Length_u16 = 12;   
		sDCU.sDCU_id.Data_a8 = aDCU_ID;
        sDCU.Init_ID_Flash = 0;  //Flag de chut nua lay ID bang IMEI sim luu vao flash
        Save_DCU_ID();
	}
	
    //
    temp = *(__IO uint8_t*)ADDR_DEV_SERIAL;   //Dev Seri. Bay gio seri va ID la khác nhau
	if (temp != 0xFF)
	{
		sDCU.sDCU_Seri.Length_u16 = *(__IO uint8_t*)(ADDR_DEV_SERIAL+1);
        if(sDCU.sDCU_Seri.Length_u16 > DCU_ID_LENGTH)
            sDCU.sDCU_Seri.Length_u16 = DCU_ID_LENGTH;   
		OnchipFlashReadData(ADDR_DEV_SERIAL+2,aDCU_SERI,sDCU.sDCU_Seri.Length_u16);
		sDCU.sDCU_Seri.Data_a8 = aDCU_SERI;
	}
	else
	{
		aDCU_SERI[0] 	= '1'; aDCU_SERI[1] 	= '2'; aDCU_SERI[2] 	= '3'; aDCU_SERI[3] 	= '4'; aDCU_SERI[4] = '5'; aDCU_SERI[5] = '6';
		aDCU_SERI[6] 	= '7'; aDCU_SERI[7] 	= '0'; aDCU_SERI[8] 	= '6'; aDCU_SERI[9] 	= '9'; aDCU_SERI[10] = '2'; aDCU_SERI[11] = '5';
		sDCU.sDCU_Seri.Length_u16 = 12;
		sDCU.sDCU_Seri.Data_a8 = aDCU_SERI;
	}
    
    sDCU.sMeter_id_now.Length_u16 = 0;
    sDCU.sMeter_id_now.Data_a8 = aMeter_ID;  //k lay lai gia tri serial cu

    temp = *(__IO uint8_t*)ADDR_DUTY_HEARTB;    
	if (temp != 0xFF)
	{
        sDCU.Freq_Send_Heartb_u32 = 0;
        Length = *(__IO uint8_t*)(ADDR_DUTY_HEARTB+1);
        if(Length < 7)
        {
            OnchipFlashReadData(ADDR_DUTY_HEARTB+2,&Buff_temp[0],Length);
            for(var = 0; var < Length; var++)
                sDCU.Freq_Send_Heartb_u32 = sDCU.Freq_Send_Heartb_u32 <<8 | Buff_temp[var];
            sDCU.Freq_Send_Heartb_u32 = sDCU.Freq_Send_Heartb_u32*1000;
        }else sDCU.Freq_Send_Heartb_u32 = 120000; 
    }else
    {
        sDCU.Freq_Send_Heartb_u32 = 120000;    
        
        Buff_temp[0] = ((sDCU.Freq_Send_Heartb_u32/1000) >>24) & 0xFF;
        Buff_temp[1] = ((sDCU.Freq_Send_Heartb_u32/1000) >>16) & 0xFF;
        Buff_temp[2] = ((sDCU.Freq_Send_Heartb_u32/1000) >>8) & 0xFF;
        Buff_temp[3] = (sDCU.Freq_Send_Heartb_u32/1000) & 0xFF;
        
        Save_Array(ADDR_DUTY_HEARTB, &Buff_temp[0], 4);
    }
    
    temp = *(__IO uint8_t*)ADDR_RADIO_FRE;  
	if (temp != 0xFF)
	{
        temp = *(__IO uint8_t*)(ADDR_RADIO_FRE+2);
        if(temp < 5)
          sSim900_status.Radio_4G_GSM = temp;
    }else
    {
        temp = 3;  //Auto
        sSim900_status.Radio_4G_GSM = temp;
        Save_Array(ADDR_RADIO_FRE, &temp, 1);
    }
    //Last Meter tyoe
    temp = *(__IO uint8_t*)ADDR_METER_TYPE;
	if (temp != 0xFF)
	{
        temp = *(__IO uint8_t*)(ADDR_METER_TYPE+2);
        if(temp < METER_TYPE_UNKNOWN) 
            sDCU.LastMeterType =  temp;
    }else
    {
        sDCU.LastMeterType = 0;
    }
    //
    temp = *(__IO uint8_t*)ADDR_PASS_MODERM;   
	if (temp != 0xFF)
	{
		sInformation.Password_Moderm.Length_u16 = *(__IO uint8_t*)(ADDR_PASS_MODERM+1);
        if(sInformation.Password_Moderm.Length_u16 >= BUFF_LENGTH_SETUP) 
            sInformation.Password_Moderm.Length_u16 = BUFF_LENGTH_SETUP;   //do doan nay them sau. Neu chip chua xoa flash se tran buff
		OnchipFlashReadData(ADDR_PASS_MODERM+2,PasswordModerm,sInformation.Password_Moderm.Length_u16);
		sInformation.Password_Moderm.Data_a8 = PasswordModerm;
	}
	else
	{
		sInformation.Password_Moderm.Length_u16 = 6;
		sInformation.Password_Moderm.Data_a8 = PasswordModerm;
        Save_PassModerm();
	} 
    //Khong luu mode connect nen set mac dinh la 1
    sDCU.Connect_Mode = '1';
}



void function_retry(void)
{
	if (((sSim900_status.Call_Ready_u8 == 0) || (sSim900_status.Sim_inserted_u8 == 0)) &&
			(sSim900_status.Step_Control_u8 > GSM_SOFT_RESET)) {
		sSim900_status.Step_Control_u8 = GSM_HARD_RESET; //GSM_SOFT_RESET
	}
	
	if (sSim900_status.Count_Startup_GPRS >= TIME_RETRY)
		sSim900_status.Step_Control_u8 = GSM_HARD_RESET; //GSM_SOFT_RESET

	if (sSim900_status.Count_Init_GPRS >= TIME_RETRY)
		sSim900_status.Step_Control_u8 = GSM_HARD_RESET; //GSM_SOFT_RESET

	if (sSim900_status.Count_Soft_Reset > TIME_RETRY) 
    {
        sSim900_status.Step_Control_u8 = GSM_HARD_RESET;
    }
	if (sSim900_status.Count_Hard_Reset > TIME_RESET_MCU) 
    {
        // Check dieu khien Reset MCU
		Check_Reset_MCU(300000);
	}
	if (sSim900_status.Step_Control_u8 != GSM_ATCOMMAND)	
    {
        sSim900_status.Status_Connect_u8 = 0;    //Neu khong o buoc 10 thi cho den ket noi HES off
	}
    
    if(sSim900_status.Status_Connect_u8 == 1)   //Co ket noi den Hes
    {
        sSim900_status.Count_Startup_GPRS = 0;
		sSim900_status.Count_Init_GPRS = 0;
		sSim900_status.Count_Soft_Reset = 0;
        sSim900_status.Count_Hard_Reset = 0;
    }
    
    if((sSim900_status.Falg_SetNewRadio == 1) && (sSim900_status.Call_Ready_u8 == 1))  //khi module sim duoc khoi dong 1 lan thi moi cho setmode radio
    {
        sSim900_status.Step_Control_u8 = GSM_CONFIG_RADIO;
    }
    
    if(sDCU.ReadPin_DetectCardSim == 1)
    {
        sSim900_status.Step_Control_u8 = GSM_HARD_RESET;
        sSim900_status.Call_Ready_u8 = 0;
        sSim900_status.Sim_inserted_u8 = 0;
        sDCU.ReadPin_DetectCardSim = 0;
    }
    
    if(sInformation.fUpdateFirmware == 1)
    {
        if((sSim900_status.Step_Control_u8 == GSM_ATCOMMAND) && (sSim900_status.CheckConnectATCommand == AT_CMD_WAIT)) 
        {
            if ((sDCU.Mode_Connect_Now !=  sDCU.Mode_Connect_Future)
                && (sDCU.Mode_Connect_Future != 0)&& (Check_Sector_Table_Empty() == TRUE)){   //check update Firmware
                sSim900_status.Step_Control_u8 = GSM_CLOSING;	
                sInformation.fUpdateFirmware  = 0;
            }
        }else   //neu o 1 buoc bat ki nao khac
        {
            if ((sDCU.Mode_Connect_Now !=  sDCU.Mode_Connect_Future) && (sDCU.Mode_Connect_Future != 0)) 
            {
                sDCU.Mode_Connect_Now 			= sDCU.Mode_Connect_Future;
                sSim900_status.Count_Hard_Reset = TIME_RESET_MCU - 1;     //Set bien count này chi can 2 lan nua thi se reset chip: 1 lan update va 1 lan khoi dong lai
                sDCU.Mode_Connect_Future 		= 0;
                sSim900_status.Step_Control_u8 = GSM_HARD_RESET;
                sInformation.fUpdateFirmware = 0;
            }
        }
    }
}

void Check_Reset_MCU(uint32_t TimeOut)
{
	sSim900_status.Timeout_SIMReset = RT_Count_Systick_u32;
	sSim900_status.Call_Ready_u8 = 0;
	sSim900_status.Sim_inserted_u8 = 0;
	SIM_PWKEY_OFF1; // Off PWKEY
	SIM_PW_OFF1;  // OFF Sim800C
	while(1)
	{
		osDelay(10000);
		if(Check_Time_Out(sSim900_status.Timeout_SIMReset, TimeOut) == 1) // 20p
		{
            _fPackStringToLog ((uint8_t*) "Reset Chip\r\n", 12);
			while(1)
			{
				osDelay(1000);
				if (((sRTC.min < 57) && (sRTC.min > 34)) || ((sRTC.min < 27) && (sRTC.min >= 5)))
				{
                    _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "Reset Chip\r\n", 12, 1000);
					__disable_irq();
					NVIC_SystemReset(); // Reset MCU
				}
			}
		}
	}
}


/*======================== Init Structs ======================*/
void init_Sim900(void)
{	
	sSim900_status.SMS_index = 0;

	sSim900_status.Count_Hard_Reset = 0;
	sSim900_status.Count_Soft_Reset 	= 0;
	sSim900_status.Timeout_SIMReset = RT_Count_Systick_u32;
	sSim900_status.Count_Init_GPRS		= 0;
	
	sSim900_status.No_Process_i16 		= _RES_CALL_READY;
	sSim900_status.No_ResProcess_i16 	= _RES_CALL_READY;
	
    sSim900_status.Status_Connect_u8 = 0;
    sSim900_status.ServerChange_u8 = 0;
    sSim900_status.systick_CheckConnectATcommand = 1;
    sSim900_status.Cmd_Uart_u8 = 0;
    sSim900_status.TimeoutCancelCmdUart_u32 = 0;
    sSim900_status.Request_Meter = 0;  

	// Step RUN
	sSim900_status.Step_Control_u8 = CHECK_STEP_MEAS(GSM_HARD_RESET,GSM_MEAS_RSSI);
	sDCU.Keep_Check_UART_i16 = UART_WAIT_LONG;	// delay feedback SIM 250ms
}

void Set_default_Sim900(void)
{
	sSim900_status.Count_Init_GPRS = 0;
	sSim900_status.Count_Startup_GPRS = 0;
	
	sSim900_status.Ber_c8 = 0;
	sSim900_status.RSSI_c8 = 0;

	sSim900_status.Call_Ready_u8 = 0;
	sSim900_status.Sim_inserted_u8 = 0;
	
	sSim900_status.No_Process_i16 = _RES_SIM_LOST;						// ko co SIM
    sSim900_status.Mode_Tranfer_u8 = 0;
    
    sSim900_status.TimeoutSendHeartBeat_u32 = 0;
}



void Fun_SimControl (void)
{   
    uint32_t    Land_Mark_Ping = 1;
    uint8_t     ResultTemp = 0;
    
    Fill_Table();
    FUNC_RETRY;
    
    switch (sSim900_status.Step_Control_u8)
    {
        case GSM_HARD_RESET:
            if (Hard_Reset() == TRUE) {
                Check_mode(&sSim900_status.Step_Control_u8,GSM_HTTP_TRANSFER, GSM_FTP_TRANSFER, GSM_STATE_STARTUP);
            } else  {
                sSim900_status.Systick_LandMarkReset_u32	= RT_Count_Systick_u32;
                sSim900_status.Step_Control_u8 				= GSM_SHUT_DOWN;
            }              
            sSim900_status.Count_Soft_Reset = 0;
            sSim900_status.Count_Hard_Reset++;
            break;
        case GSM_SOFT_RESET:
            FUNC_WHILE;
            Step_Config(&sSim900_status.No_Process_i16,_AT_TCP_OUT_DATAMODE,_AT_TCP_OUT_DATAMODE);    
            if (Soft_Reset() == TRUE) 
            {				
                Check_mode(&sSim900_status.Step_Control_u8,GSM_HTTP_TRANSFER, GSM_FTP_TRANSFER, GSM_STATE_STARTUP); 
            }else 
                sSim900_status.Step_Control_u8 = GSM_HARD_RESET;

            sSim900_status.Count_Soft_Reset++;
            break;
        case GSM_STATE_STARTUP:
            if (Check_Tranparent_Mode(sSim900_status.Step_Control_u8) == TRUE)  //sau lenh nay vao GSM_CLOSING
              break;
            
            sSim900_status.Count_Startup_GPRS++;
            sSim900_status.Status_Connect_u8 = 0;
            
            if(Step_Config(&sSim900_status.No_Process_i16,_AT_CHECK_ATTACH,_AT_CHECK_ATTACH) != 1) 
                if(Step_Config(&sSim900_status.No_Process_i16,_AT_CHECK_ATTACH,_AT_CHECK_ATTACH) != 1)
                    if(Step_Config(&sSim900_status.No_Process_i16,_AT_CHECK_ATTACH,_AT_CHECK_ATTACH) != 1)
                    {
                        sSim900_status.Step_Control_u8 = GSM_HARD_RESET;
                        break;
                    }
            sDCU.Keep_Check_UART_i16 = UART_WAIT_NORMAL;
            Step_Config(&sSim900_status.No_Process_i16,_AT_CHECK_RSSI,_AT_CHECK_RSSI);
            Step_Config(&sSim900_status.No_Process_i16,_AT_SYN_TIME_ZONE,_AT_SYN_TIME_ZONE); 
            Step_Config(&sSim900_status.No_Process_i16,_AT_GET_RTC,_AT_GET_RTC); 
            
            if(Step_Config(&sSim900_status.No_Process_i16,_AT_CHECK_SIM,_AT_CHECK_SIM)== 1)
            {
                sSim900_status.Step_Control_u8 = GSM_ATCOMMAND;
                sSim900_status.CheckConnectATCommand = AT_CONNECT_SERVICE;
                sSim900_status.ServerChange_u8 = 1;                 
            }else 
                sSim900_status.Step_Control_u8 = GSM_HARD_RESET;
            break;
        case GSM_FTP_TRANSFER:	
            if(Step_Config(&sSim900_status.No_Process_i16,_AT_CHECK_ATTACH,_AT_CHECK_ATTACH) != 1) 
                if(Step_Config(&sSim900_status.No_Process_i16,_AT_CHECK_ATTACH,_AT_CHECK_ATTACH) != 1)
                    if(Step_Config(&sSim900_status.No_Process_i16,_AT_CHECK_ATTACH,_AT_CHECK_ATTACH) != 1)
                    {
                        sSim900_status.Step_Control_u8 = GSM_HARD_RESET;
                        break;
                    }
            Step_Config(&sSim900_status.No_Process_i16,_AT_CHECK_RSSI,_AT_CHECK_RSSI);
               
            sSim900_status.factiveAPN = 0;  //Reset lai flag check active APN
            Load_Receipt(&sDCU.sReceipt, sRec_INIT_APN);  
            Do_Receipt(&sDCU.sReceipt,&sSim900_status.No_Process_i16,&sSim900_status.Step_Control_u8,GSM_HTTP_TRANSFER,GSM_HTTP_TRANSFER);
              
            if(sSim900_status.factiveAPN == 0)  //chua duoc gan APN thi phai active trong truong hop 3G
            {
                Step_Config(&sSim900_status.No_Process_i16,_AT_ACTIVE_APN,_AT_ACTIVE_APN); 
                Step_Config(&sSim900_status.No_Process_i16,_AT_GET_IP_APN,_AT_GET_IP_APN); 
            }
            
            Load_Receipt(&sDCU.sReceipt, sRec_FTP_Init);   
            if(Do_Receipt(&sDCU.sReceipt,&sSim900_status.No_Process_i16,&sSim900_status.Step_Control_u8,GSM_FTP_TRANSFER_DOWNLOAD,GSM_FTP_TRANSFER_DONE) == 1)
            {
                if ((sSim900_status.Count_Soft_Reset >= TIME_RETRY) || (sFTP.FirmWare_Length == 0)) 
                { 
                    sSim900_status.Step_Control_u8 	= GSM_FTP_TRANSFER_DONE;
                    sFTP.Receiver_u8 				= SEND_UPDATE_FIRMWARE_FAIL_SETUP;
                }
//                Erase_Firmware(ADDR_UPDATE_PROGRAM,MAX_SIZE_FIRM);
            }
            else
            {
                // Thong bao loi -> Reset MCU
                sFTP.Receiver_u8 				= SEND_UPDATE_FIRMWARE_FAIL_SETUP;
                sSim900_status.Step_Control_u8 	= GSM_FTP_TRANSFER_DONE;
            }     
            
            break;
        case GSM_FTP_TRANSFER_DOWNLOAD:
            Set_default_FTP();
            Flash_S25FL_Erase_Sector(pos_FirmWrite_u32);               //Xoa sector dau tien cua vi tri luu firm
			while (sFTP.Receiver_u8 == SEND_UPDATE_FIRMWARE)  
            {
                Load_Receipt(&sDCU.sReceipt, sRec_FTP_ReadData_2);  
                if(Do_Receipt(&sDCU.sReceipt,&sSim900_status.No_Process_i16,&sSim900_status.Step_Control_u8,GSM_FTP_TRANSFER_DOWNLOAD,GSM_FTP_TRANSFER_DOWNLOAD) != TRUE)
                {
                    sFTP.Update_success = FALSE;
                    sFTP.Receiver_u8 = SEND_UPDATE_FIRMWARE_FAIL_TIME;
                }                
                if(Check_Time_Out(sFTP.Systick_LandMarkFTP_u32, 480000) == 1)
                {
                    sFTP.Update_success = FALSE;
                    sFTP.Receiver_u8 = SEND_UPDATE_FIRMWARE_FAIL_TIME;
                } 
            }            
//            osSemaphoreRelease(sem_FTP_DataHandle);               //cach cu
            sSim900_status.Step_Control_u8 = GSM_FTP_TRANSFER_DONE;
            break;
        case GSM_HTTP_TRANSFER:	
            if(Step_Config(&sSim900_status.No_Process_i16,_AT_CHECK_ATTACH,_AT_CHECK_ATTACH) != 1) 
                if(Step_Config(&sSim900_status.No_Process_i16,_AT_CHECK_ATTACH,_AT_CHECK_ATTACH) != 1)
                    if(Step_Config(&sSim900_status.No_Process_i16,_AT_CHECK_ATTACH,_AT_CHECK_ATTACH) != 1)
                    {
                        sSim900_status.Step_Control_u8 = GSM_HARD_RESET;
                        break;
                    }
            Step_Config(&sSim900_status.No_Process_i16,_AT_CHECK_RSSI,_AT_CHECK_RSSI);  
            
            sSim900_status.factiveAPN = 0;  //Reset lai flag check active APN
            Load_Receipt(&sDCU.sReceipt, sRec_INIT_APN);  
            Do_Receipt(&sDCU.sReceipt,&sSim900_status.No_Process_i16,&sSim900_status.Step_Control_u8,GSM_HTTP_TRANSFER,GSM_HTTP_TRANSFER);
              
            if(sSim900_status.factiveAPN == 0)  //chua duoc gan APN thi phai active trong truong hop 3G
            {
                Step_Config(&sSim900_status.No_Process_i16,_AT_ACTIVE_APN,_AT_ACTIVE_APN); 
                Step_Config(&sSim900_status.No_Process_i16,_AT_GET_IP_APN,_AT_GET_IP_APN); 
            }
              
            Load_Receipt(&sDCU.sReceipt, sRec_HTTP_INIT);   
            if(Do_Receipt(&sDCU.sReceipt,&sSim900_status.No_Process_i16,&sSim900_status.Step_Control_u8,GSM_HTTP_TRANSFER_DOWNLOAD,GSM_FTP_TRANSFER_DONE) == 1)
            {
                if ((sSim900_status.Count_Soft_Reset >= TIME_RETRY) || (sFTP.FirmWare_Length == 0))  // not Connect FTP server
                { 
                    sSim900_status.Step_Control_u8 	= GSM_FTP_TRANSFER_DONE;
                    sFTP.Receiver_u8 				= SEND_UPDATE_FIRMWARE_FAIL_SETUP;
                    break;
                }
//                Erase_Firmware(ADDR_UPDATE_PROGRAM, MAX_SIZE_FIRM);
            }
            else
            {
                sFTP.Receiver_u8 				= SEND_UPDATE_FIRMWARE_FAIL_SETUP;
                sSim900_status.Step_Control_u8 	= GSM_FTP_TRANSFER_DONE;  
            }
            
            break;
        case GSM_HTTP_TRANSFER_DOWNLOAD:
            Set_default_FTP();
            Flash_S25FL_Erase_Sector(pos_FirmWrite_u32);               //Xoa sector dau tien cua vi tri luu firm
			while (sFTP.Receiver_u8 == SEND_UPDATE_FIRMWARE)  
            {
                Load_Receipt(&sDCU.sReceipt, sRec_HTTP_ReadData);  
                if(Do_Receipt(&sDCU.sReceipt,&sSim900_status.No_Process_i16,&sSim900_status.Step_Control_u8,GSM_HTTP_TRANSFER_DOWNLOAD,GSM_HTTP_TRANSFER_DOWNLOAD) != TRUE)
                {
                    sFTP.Update_success = FALSE;
                    sFTP.Receiver_u8 = SEND_UPDATE_FIRMWARE_FAIL_TIME;
                }                
                if(Check_Time_Out(sFTP.Systick_LandMarkFTP_u32, 480000) == 1)
                {
                    sFTP.Update_success = FALSE;
                    sFTP.Receiver_u8 = SEND_UPDATE_FIRMWARE_FAIL_TIME;
                } 
                osDelay(20);
            }
            Step_Config(&sSim900_status.No_Process_i16,_AT_HTTP_CLOSE,_AT_HTTP_CLOSE);
            sSim900_status.Step_Control_u8 = GSM_FTP_TRANSFER_DONE;
            break;
        case GSM_FTP_TRANSFER_DONE:
            // Neu loi -> reset MCU. thanh cong cung reset MCU
            if(sFTP.Receiver_u8 == SEND_UPDATE_FIRMWARE_OK)        
                sTCP.aNeed_Send[UPDATE_FIRM_OK] = 1;  
            else 
                sTCP.aNeed_Send[UPDATE_FIRM_FAIL] = 1; //Them ban tin bao fail de reset chip. Vi truoc do dã cho dung thuc thi task ReadMeter và Flash
            sSim900_status.Step_Control_u8 = GSM_HARD_RESET;
            sDCU.Mode_Connect_Now = MODE_CONNECT_DATA;
            break;
        case GSM_CLOSING:	// Reset SIM900
            if ((sDCU.Mode_Connect_Now != sDCU.Mode_Connect_Future) && (sDCU.Mode_Connect_Future != 0))   //Check xem có phai updateFirm k
            {
                sDCU.Mode_Connect_Now 			= sDCU.Mode_Connect_Future;
                sSim900_status.Count_Soft_Reset = 0;	// reset Count
                sDCU.Mode_Connect_Future 		= 0;
                sSim900_status.Count_Hard_Reset = TIME_RESET_MCU - 1;     //Set bien count này chi can 2 lan nua thi se reset chip
            }
            Step_Config(&sSim900_status.No_Process_i16,_AT_TCP_OUT_DATAMODE,_AT_TCP_OUT_DATAMODE);
            Step_Config(&sSim900_status.No_Process_i16,_AT_TCP_CLOSE,_AT_TCP_CLOSE);
            Step_Config(&sSim900_status.No_Process_i16,_AT_TCP_NETCLOSE,_AT_TCP_NETCLOSE);
            sSim900_status.Step_Control_u8 	= GSM_HARD_RESET; //GSM_SOFT_RESET
            break;
        case GSM_SHUT_DOWN:
            SIM_PW_OFF1;
            SIM_PWKEY_OFF1;; // Off chan PWKEY
            osDelay(2000);
            sSim900_status.Call_Ready_u8 = 0;
            sSim900_status.Sim_inserted_u8 = 0;			
            if (Cal_Time_s(sSim900_status.Systick_LandMarkReset_u32,RT_Count_Systick_u32) > 120) { // off trong 2 phút
                sSim900_status.Step_Control_u8 = GSM_HARD_RESET;
            }
            break;
        case GSM_MEAS_RSSI:
            Reset_GPIO();
            FUNC_WHILE;
            break;
        /* Dung cho giao tiep voi tap lenh at qua cong DB15 */
        case GSM_ATCOMMAND:
            switch(sSim900_status.CheckConnectATCommand)
            {
                case AT_CMD_WAIT: // Cho thuc hien lenh at tiep theo hoac cho trang thai
                    if(sSim900_status.Cmd_Uart_u8 == 1)
                    {
                        if(Check_Time_Out_2(UART1_Control.Systick_Last_Recerver_u32, 66) == 1) // Co data phan hoi tu cong to thi gui den Server/ baudarate 300 -> (1000/300)*10 = 33ms
                        {
                            sSim900_status.Cmd_Uart_u8 = 0;
                            UART1_Control.Systick_Last_Recerver_u32 = 0;
                            //copy chuoi thu duoc
                            if(UART1_Control.Mess_Length_ui16 < 100)
                                DCU_Respond(PortConfig, UART1_Control.UART1_Str_Recei.Data_a8, UART1_Control.Mess_Length_ui16);
                            HAL_GPIO_WritePin(LED_RED_port,LED_RED_pin,LED_OFF);
                        }
                        else{
                            if(Check_Time_Out_2(sSim900_status.TimeoutCancelCmdUart_u32, 2000) == 1) // ko co phan hoi tu cong to
                            {
                                sSim900_status.TimeoutCancelCmdUart_u32 = 0;
                                sSim900_status.Cmd_Uart_u8 = 0;
                                sSim900_status.Request_Meter = 0;
                                DCU_Respond(PortConfig,(uint8_t *)"ERROR",5);
                            }
                        }
                    }
                    
                    if(sSim900_status.FlagSet_NewAPN == 1)
                    {
                        if(sSim900_status.Status_Connect_u8 == 0)
                        {
                            Str_Cmd_AT.Length_u16 = 0;   //copy chuoi hoi apn
                            for(uint8_t i=0;i< (CheckList_AT[_ACCOUNT].sTempReceiver.Length_u16 - 1);i++) 
                                *(Str_Cmd_AT.Data_a8 + Str_Cmd_AT.Length_u16++) = *(CheckList_AT[_ACCOUNT].sTempReceiver.Data_a8+i);
                            DCU_Respond(PortConfig,(uint8_t *)"Incorrect phone number, username or password",44);
                        }
                        sSim900_status.FlagSet_NewAPN = 0;
                    }
                
                    if(sSim900_status.Status_Connect_u8 == 1)
                    {
                        _f_Send_Meter_Data ();
                        if(Check_Time_Out(sSim900_status.TimeoutSendHeartBeat_u32, sDCU.Freq_Send_Heartb_u32) == 1)
                        {
                            GetHandshakeData();
                            _f_TCP_SEND_SERVER(sHandsake.Data_a8,sHandsake.Length_u16, 0xFF, TIME_DELAY_TCP);
                        }
                        //Cuoi ngay thoat che do data mode de láy stime tu tram
                        if((sRTC.hour == 12)&& (sRTC.min == sTCP.Min_Allow_Send_DATA_u8)&&(sSim900_status.Flag_Get_sTime == 0))
                        {
                            _f_TCP_SEND_SERVER(sHandsake.Data_a8,sHandsake.Length_u16, 0xFF, TIME_DELAY_TCP);
                            osDelay(1000);
                            Step_Config(&sSim900_status.No_Process_i16,_AT_TCP_OUT_DATAMODE,_AT_TCP_OUT_DATAMODE);
                            Step_Config(&sSim900_status.No_Process_i16,_AT_GET_RTC,_AT_GET_RTC);
                            Step_Config(&sSim900_status.No_Process_i16,_AT_TCP_IN_DATAMODE,_AT_TCP_IN_DATAMODE);
                            
                            sSim900_status.Flag_Get_sTime = 1;
                        }else if((sRTC.hour != 12)&& (sRTC.min != sTCP.Min_Allow_Send_DATA_u8)) 
                            sSim900_status.Flag_Get_sTime = 0;
                    }
                    else
                    {
                        // reconnect neu o che do online lien tuc
                        sSim900_status.Step_Control_u8 = GSM_HARD_RESET;
                        sSim900_status.Sim_inserted_u8 = 0;
                        sSim900_status.Call_Ready_u8 = 0;
                    }
                    
                    break;
                case AT_SEND_DATA_UART: // Gui du lieu qua cong Uart doc cong to.  Them che do tranfer: doi phan hoi ve roi gui lai qua TCP luon k?
                    if(sSim900_status.Cmd_Uart_u8 == 0)
                    {
                        sSim900_status.Cmd_Uart_u8 = 1;
                        ATSendDataLength_u8 = 0;
                        sSim900_status.TimeoutCancelCmdUart_u32 = RT_Count_Systick_u32;
                        Init_UART2();
                        SendDatatoSerial(aATBuffReceData, ATReceDataLength_u8);
                        HAL_GPIO_WritePin(LED_RED_port,LED_RED_pin,LED_ON);
                    }
                    
                    if(sSim900_status.Init_Uart_u8 == 1)
                    {
//                        __HAL_UART_DISABLE_IT(&UART_METER, UART_IT_RXNE);
                        UART_METER.Instance = UART__METER;
                        switch(sSim900_status.Baudrate_Uart_u8)
                        {
                            case '0':
                                UART_METER.Init.BaudRate = 300;
                                break;
                            case '1':
                                UART_METER.Init.BaudRate = 600;
                                break;
                            case '2':
                                UART_METER.Init.BaudRate = 1200;
                                break;
                            case '3':
                                UART_METER.Init.BaudRate = 2400;
                                break;
                            case '4':
                                UART_METER.Init.BaudRate = 4800;
                                break;
                            case '5':
                                UART_METER.Init.BaudRate = 9600;
                                break;
                            case '6':
                                UART_METER.Init.BaudRate = 19200;
                                break;		
                            default:
                                break;
                        }
                        HAL_UART_Init(&UART_METER);
//                        __HAL_UART_ENABLE_IT(&UART_METER, UART_IT_RXNE);
//                        osDelay(2);
                        sSim900_status.Init_Uart_u8 = 0;
                    }
                    sSim900_status.CheckConnectATCommand = AT_CMD_WAIT;
                    break;
                case AT_CONNECT_SERVICE: // Ket noi vao dia chi Server dang dc cau hinh
                    if(sSim900_status.ServerChange_u8 == 1) // truong hop dia chi IP, Port khac, connect lai
                    {
                        sSim900_status.ServerChange_u8 = 0;
                        Load_Receipt(&sDCU.sReceipt,sRec_TCP_DATA_Init);
                        if (Do_Receipt(&sDCU.sReceipt,&sSim900_status.No_Process_i16,&sSim900_status.Step_Control_u8,GSM_ATCOMMAND,GSM_ATCOMMAND) != TRUE)
                        {
//                            DCU_Respond(1,(uint8_t*)"Fail",4);
                            sSim900_status.CheckConnectATCommand = AT_CONNECT_SERVICE;
                            sSim900_status.Step_Control_u8 = GSM_HARD_RESET;
                            sSim900_status.Status_Connect_u8 = 0;
                            break;
                        }
                    }
                    sSim900_status.Status_Connect_u8 = 1;
                    osDelay(1000);
                    GetHandshakeData();
                    _f_TCP_SEND_SERVER(sHandsake.Data_a8,sHandsake.Length_u16, 0xFF, TIME_DELAY_TCP);
                    sSim900_status.CheckConnectATCommand = AT_CMD_WAIT;
                    break;
                case AT_DISCONNECT_SERVICE:  //ngat luôn dich v?
                    _f_TCP_SEND_SERVER(&aATBuffSendData[0],ATSendDataLength_u8, 0xFF, TIME_DELAY_TCP);
                    osDelay(1000);
                    sTCP.aNeed_Send[DATA_RESPOND_AT] = 0;
                    Step_Config(&sSim900_status.No_Process_i16,_AT_TCP_OUT_DATAMODE,_AT_TCP_OUT_DATAMODE);
                    sSim900_status.ServerChange_u8 = 1;
                    if(Step_Config(&sSim900_status.No_Process_i16,_AT_TCP_CLOSE,_AT_TCP_CLOSE) == TRUE)
                    {
                        Step_Config(&sSim900_status.No_Process_i16,_AT_TCP_NETCLOSE,_AT_TCP_NETCLOSE);
                        sSim900_status.Status_Connect_u8 = 0;
                        sSim900_status.CheckConnectATCommand = AT_CMD_WAIT;
                    }
                    else
                    {
                        sSim900_status.CheckConnectATCommand = AT_CMD_WAIT;
                        Step_Config(&sSim900_status.No_Process_i16,_AT_TCP_IN_DATAMODE,_AT_TCP_IN_DATAMODE);
                    }
                    break;
                case AT_PING_TCP:
                    //cau hinh thoat data mode
                    _f_TCP_SEND_SERVER(sHandsake.Data_a8,sHandsake.Length_u16, 0xFF, TIME_DELAY_TCP);
                    osDelay(1000);
                    Step_Config(&sSim900_status.No_Process_i16,_AT_TCP_OUT_DATAMODE,_AT_TCP_OUT_DATAMODE);
                    //câu hinh lenh
                    Reset_Buff(&sDCU.StrPing_Recei);  //xoa buff nhan 
                    sDCU.Fl_Finish_Ping = 0;
                    Load_Receipt(&sDCU.sReceipt,sRec_TCP_Send_Ping);
                    Do_Receipt(&sDCU.sReceipt,&sSim900_status.No_Process_i16,&sSim900_status.Step_Control_u8,GSM_ATCOMMAND,GSM_ATCOMMAND);
                    //Doi receive ACK
                    Land_Mark_Ping = RT_Count_Systick_u32;
                    while(sDCU.Fl_Finish_Ping == 0)
                    {   
                        if(Check_Time_Out(Land_Mark_Ping, 20000) == 1)
                            break;
                        osDelay(50);
                    }
                    //chuyen lai sang data mode
                    Step_Config(&sSim900_status.No_Process_i16,_AT_TCP_IN_DATAMODE,_AT_TCP_IN_DATAMODE);
                    DCU_Respond(PortConfig, sDCU.StrPing_Recei.Data_a8, sDCU.StrPing_Recei.Length_u16);
                    sSim900_status.CheckConnectATCommand = AT_CMD_WAIT;
                    break;
                case AT_CONNECT_TCP: // Gui lenh ket noi den dia chi Server nhan dc
                    if(sSim900_status.ServerChange_u8 == 1) // truong hop dia chi IP, Port khac, connect lai
                    {
                        sSim900_status.ServerChange_u8 = 0;
                        sDCU.Flag_ConnectNewServer = 1;
                        
                        Load_Receipt(&sDCU.sReceipt,sRec_AT_CONNECT);
                        if (Do_Receipt(&sDCU.sReceipt,&sSim900_status.No_Process_i16,&sSim900_status.Step_Control_u8,GSM_ATCOMMAND,GSM_ATCOMMAND)!= TRUE)
                        {
//                            DCU_Respond(1,(uint8_t*)"Fail",4);
                            sSim900_status.Step_Control_u8 = GSM_HARD_RESET;
                            sDCU.Flag_ConnectNewServer = 0;
                            break;
                        }
                    }
                    sDCU.Flag_ConnectNewServer = 0;
                    
                    GetHandshakeData();
                    _f_TCP_SEND_SERVER(sHandsake.Data_a8,sHandsake.Length_u16, 0xFF, TIME_DELAY_TCP);
                    //
                    sSim900_status.Status_Connect_u8 = 1;
                    sSim900_status.CheckConnectATCommand = AT_CMD_WAIT;
                    break;
                case AT_SEND_DATA_TCP: // Gui data toi dia chi Server nhan dc, ket noi xong moi gui data  
                    if(sSim900_status.ServerChange_u8 == 1) // truong hop dia chi IP, Port khac, connect lai
                    {
                        sDCU.Flag_ConnectNewServer = 1;
                        sSim900_status.ServerChange_u8 = 0;
                        Load_Receipt(&sDCU.sReceipt,sRec_AT_CONNECT);
                        if (Do_Receipt(&sDCU.sReceipt,&sSim900_status.No_Process_i16,&sSim900_status.Step_Control_u8,GSM_ATCOMMAND,GSM_ATCOMMAND) != TRUE)
                        {
//                            DCU_Respond(1, (uint8_t*)"Fail",4); 
                            sSim900_status.Step_Control_u8 = GSM_HARD_RESET;
                            sDCU.Flag_ConnectNewServer = 0;
                            break;
                        }
                    }
                    sDCU.Flag_ConnectNewServer = 0;
                    _f_TCP_SEND_SERVER(&aATBuffSendData[0],ATSendDataLength_u8, 0xFF, TIME_DELAY_TCP);   
                    sSim900_status.CheckConnectATCommand = AT_CMD_WAIT;
                    break;
                case AT_DISCONNECT_TCP: // Gui lenh ngat ket noi den dia chi Server nhan dc
                    if(Check_Time_Out(sSim900_status.TimeoutSendHeartBeat_u32, 1000) == 0)
                        osDelay(1000);
                    _f_TCP_SEND_SERVER(&aATBuffSendData[0],ATSendDataLength_u8, 0xFF, TIME_DELAY_TCP);
                    sTCP.aNeed_Send[DATA_RESPOND_AT] = 0;
                    sSim900_status.ServerChange_u8 = 0;
//                    if(sSim900_status.ServerChange_u8 == 1)
//                    {
//                        // Khac server
//                        sSim900_status.ServerChange_u8 = 0;
//                        sSim900_status.CheckConnectATCommand = AT_CMD_WAIT;
//                    }else
//                    {
//                        Step_Config(&sSim900_status.No_Process_i16,_AT_TCP_OUT_DATAMODE,_AT_TCP_OUT_DATAMODE);
//                        if(Step_Config(&sSim900_status.No_Process_i16,_AT_TCP_CLOSE,_AT_TCP_CLOSE) == TRUE)
//                        {
                            sSim900_status.Status_Connect_u8 = 0;
                            osDelay(30000);
                            sSim900_status.CheckConnectATCommand = AT_CONNECT_SERVICE;
//                        }else
//                        {
//                            DCU_Respond(1, (uint8_t*)"Fail",4);
//                            sSim900_status.CheckConnectATCommand = AT_CMD_WAIT;
//                            Step_Config(&sSim900_status.No_Process_i16,_AT_TCP_IN_DATAMODE,_AT_TCP_IN_DATAMODE);
//                        }
//                    }
                    break;
                case AT_CHECK_CONNECT:
                    if(Step_Config(&sSim900_status.No_Process_i16,_AT_CHECK_TCP_STATUS,_AT_CHECK_TCP_STATUS) == TRUE)
                        sSim900_status.CheckConnectATCommand = AT_CMD_WAIT;
                    else
                        sSim900_status.CheckConnectATCommand = AT_CONNECT_SERVICE;
                    break;
                default:    
                  break;
            }
            break;
        case GSM_CONFIG_RADIO: 
            //Check de thoat che do data mode
            if ((sSim900_status.FRequestAT_ByServer == 1) && (sSim900_status.Mode_Tranfer_u8 == 1)) 
            {
//                GetHandshakeData();
//                _f_TCP_SEND_SERVER(sHandsake.Data_a8,sHandsake.Length_u16, 0xFF, TIME_DELAY_TCP);
                DCU_Respond_Direct(PortConfig,(uint8_t *)"OK",2);  //cu bao OK da. vi khi chuyen mode co the bi mat ket noi
                osDelay(1000);
                Step_Config(&sSim900_status.No_Process_i16,_AT_TCP_OUT_DATAMODE,_AT_TCP_OUT_DATAMODE);
            }
            //
            sSim900_status.Falg_SetNewRadio = 0;
            ResultTemp = 0;            
            switch(sSim900_status.Radio_ConfigAT)
            {
                case 0:       
                    ResultTemp = Step_Config(&sSim900_status.No_Process_i16,_AT_ACCESS_RA_2G,_AT_ACCESS_RA_2G);
                    if (sSim900_status.FRequestAT_ByServer == 1)
                        Step_Config(&sSim900_status.No_Process_i16,_AT_TCP_IN_DATAMODE,_AT_TCP_IN_DATAMODE);  
                    
                    if( ResultTemp == 1)
                    {
                        sSim900_status.Radio_4G_GSM = sSim900_status.Radio_ConfigAT;
                        Save_Array(ADDR_RADIO_FRE, &sSim900_status.Radio_ConfigAT, 1);
                        DCU_Respond_Direct(PortConfig,(uint8_t *)"OK",2);                
                    }else DCU_Respond_Direct(PortConfig,(uint8_t *)"ERROR",5);
                    break;
                case 1: 
                    ResultTemp = Step_Config(&sSim900_status.No_Process_i16,_AT_ACCESS_RA_3G,_AT_ACCESS_RA_3G);
                    if (sSim900_status.FRequestAT_ByServer == 1)
                        Step_Config(&sSim900_status.No_Process_i16,_AT_TCP_IN_DATAMODE,_AT_TCP_IN_DATAMODE);
                    if(ResultTemp == 1)
                    {
                        sSim900_status.Radio_4G_GSM = sSim900_status.Radio_ConfigAT;
                        Save_Array(ADDR_RADIO_FRE, &sSim900_status.Radio_ConfigAT, 1);
                        DCU_Respond_Direct(PortConfig,(uint8_t *)"OK",2);
                    }else DCU_Respond_Direct(PortConfig,(uint8_t *)"ERROR",5);
                    break;
                case 2: 
                    ResultTemp = Step_Config(&sSim900_status.No_Process_i16,_AT_ACCESS_RA_4G,_AT_ACCESS_RA_4G);
                    if (sSim900_status.FRequestAT_ByServer == 1)
                        Step_Config(&sSim900_status.No_Process_i16,_AT_TCP_IN_DATAMODE,_AT_TCP_IN_DATAMODE);
                    if(ResultTemp == 1)
                    {
                        sSim900_status.Radio_4G_GSM = sSim900_status.Radio_ConfigAT;
                        Save_Array(ADDR_RADIO_FRE, &sSim900_status.Radio_ConfigAT, 1);
                        DCU_Respond_Direct(PortConfig,(uint8_t *)"OK",2);
                    }else DCU_Respond_Direct(PortConfig,(uint8_t *)"ERROR",5);
                    break;
                case 3: 
                    ResultTemp = Step_Config(&sSim900_status.No_Process_i16,_AT_ACCESS_RA_AUTO,_AT_ACCESS_RA_AUTO);
                    if (sSim900_status.FRequestAT_ByServer == 1)
                        Step_Config(&sSim900_status.No_Process_i16,_AT_TCP_IN_DATAMODE,_AT_TCP_IN_DATAMODE);
                    if(ResultTemp == 1)
                    {
                        sSim900_status.Radio_4G_GSM = sSim900_status.Radio_ConfigAT;
                        Save_Array(ADDR_RADIO_FRE, &sSim900_status.Radio_ConfigAT, 1);
                        DCU_Respond_Direct(PortConfig,(uint8_t *)"OK",2);
                    }else DCU_Respond_Direct(PortConfig,(uint8_t *)"ERROR",5);
                    break;
                default:
                    DCU_Respond_Direct(PortConfig,(uint8_t *)"ERROR",5);
                    break;
            }
            
            if (sSim900_status.FRequestAT_ByServer == 1)
            {
                Step_Config(&sSim900_status.No_Process_i16,_AT_TCP_OUT_DATAMODE,_AT_TCP_OUT_DATAMODE);
                Step_Config(&sSim900_status.No_Process_i16,_AT_TCP_CLOSE,_AT_TCP_CLOSE);
                Step_Config(&sSim900_status.No_Process_i16,_AT_TCP_NETCLOSE,_AT_TCP_NETCLOSE);
            }
            sSim900_status.FRequestAT_ByServer  = 0;
            sSim900_status.Step_Control_u8      = GSM_HARD_RESET;
            break;
        default:
            break;
    }
    
    if(sSim900_status.fRequest_AT_cmd == 1)
    {
        Request_AT_Command(sSim900_status.sAT_CMD_Request, sSim900_status.StrCheck_AT_Req);
        sSim900_status.fRequest_AT_cmd = 0; 
    }
}



uint8_t Request_AT_Command (truct_String StrSend, truct_String Str_Recei)
{
    uint32_t LandMark_Time = 0;
      
    if((sSim900_status.Step_Control_u8 == GSM_ATCOMMAND) && (sSim900_status.Status_Connect_u8 == 1))
    {
        _f_TCP_SEND_SERVER(sHandsake.Data_a8,sHandsake.Length_u16, 0xFF, TIME_DELAY_TCP);
        osDelay(1000);
        Step_Config(&sSim900_status.No_Process_i16,_AT_TCP_OUT_DATAMODE,_AT_TCP_OUT_DATAMODE);
    }

    sDCU.fRecei_StrAT_OK = 1; 
    Reset_Buff(&sDCU.StrPing_Recei);  //xoa buff nhan 
    
    HAL_UART_Transmit(&UART_SIM, StrSend.Data_a8, StrSend.Length_u16,1000);

    LandMark_Time = RT_Count_Systick_u32;
    while(sDCU.fRecei_StrAT_OK == 1)
    {   
        if(Check_Time_Out(LandMark_Time, 10000) == 1)
            break;
        osDelay(50);
    }
    sDCU.fRecei_StrAT_OK = 0;

    if((sSim900_status.Step_Control_u8 == GSM_ATCOMMAND) && (sSim900_status.Status_Connect_u8 == 1))
        Step_Config(&sSim900_status.No_Process_i16,_AT_TCP_IN_DATAMODE,_AT_TCP_IN_DATAMODE);
            
    if(sDCU.StrPing_Recei.Length_u16 != 0)
        DCU_Respond(PortConfig, sDCU.StrPing_Recei.Data_a8, sDCU.StrPing_Recei.Length_u16);
    else 
        DCU_Respond(PortConfig,(uint8_t *)"FAIL",4);
    
    return 1;
}


/*
Ham khoi tao SIM900 = 1 : Khoi tao thanh cong
										= 0 : Khong khoi tao dc
	Wait SIM900 tra ve : "Call ready" => Khoi tao xong ham
	Dat che do SMS : Co tin nhan -> tra ve ngay
*/
void Reset_GPIO(void)
{	
	 SIM_PW_OFF1;  // Sim800C
     SIM_PWKEY_OFF1;
	 osDelay(2000); 
	 SIM_PW_ON1; // Sim 800C
	 osDelay(1000); 
	 SIM_PWKEY_ON1;
	 while(1)
	 {
		osDelay(5000);
		break;
	 }
	// Check chan trang thai de dung chan PWKEY hop li
	 SIM_PWKEY_OFF1;
     osDelay(5000);
}

void Check_mode(uint8_t *Step_Control,uint8_t Step_HTTP, uint8_t Step_FTP, uint8_t Step_TCP)
{
//	Init_Header_MQTT();
	switch (sDCU.Mode_Connect_Now) {
		case MODE_CONNECT_DATA:
			*Step_Control = Step_TCP;
			break;
		case MODE_CONNECT_FTP:
			*Step_Control = Step_FTP;
            sSim900_status.Count_Hard_Reset = 0;
			break;
        case MODE_CONNECT_HTTP:
			*Step_Control = Step_HTTP;
            sSim900_status.Count_Hard_Reset = 0;
			break;
		default:
			*Step_Control = Step_TCP;
			break;
	}
}
uint8_t Hard_Reset(void)
{
	uint8_t  Result = TRUE;
	Set_default_Sim900();
	Reset_GPIO();	
	sSim900_status.Systick_LandMarkReset_u32 = RT_Count_Systick_u32;
    sDCU.Keep_Check_UART_i16 = UART_WAIT_LONG;
	// 3 lenh nay luon co moi khi ngat nguon sim
    osDelay(10000);
	Step_Config(&sSim900_status.No_Process_i16,_AT_DISPLAY_CMD,_AT_DISPLAY_CMD); // nen luon co de check URC
    Step_Config(&sSim900_status.No_Process_i16,_AT_DISPLAY_CMD,_AT_DISPLAY_CMD);
    Step_Config(&sSim900_status.No_Process_i16,_AT_MAN_LOG,_AT_MAN_LOG);
    
    switch(sSim900_status.Radio_4G_GSM)
    {
        case 0:     //2G
            Step_Config(&sSim900_status.No_Process_i16,_AT_ACCESS_RA_2G,_AT_ACCESS_RA_2G);      
            break;
        case 1:   //3G
            Step_Config(&sSim900_status.No_Process_i16,_AT_ACCESS_RA_3G,_AT_ACCESS_RA_3G);
            break;
        case 2:   //4G
            Step_Config(&sSim900_status.No_Process_i16,_AT_ACCESS_RA_4G,_AT_ACCESS_RA_4G);
            break;
        case 3:   //Auto
            Step_Config(&sSim900_status.No_Process_i16,_AT_ACCESS_RA_AUTO,_AT_ACCESS_RA_AUTO);
            break;
        default:
            break;
    }   
    HAL_UART_Transmit(&UART_SIM, (uint8_t*) "AT+CEREG=0\r", 11, 1000);
    osDelay(2000);
	Step_Config(&sSim900_status.No_Process_i16,_AT_BAUD_RATE,_AT_BAUD_RATE);            // _AT_BAUD_RATE- Lenh thiet lap lai Baudrate
    Step_Config(&sSim900_status.No_Process_i16,_AT_SIM_ID,_AT_SIM_ID); 
	Step_Config(&sSim900_status.No_Process_i16,_AT_CHECK_RSSI,_AT_CHECK_RSSI);
	Step_Config(&sSim900_status.No_Process_i16,_AT_GET_IMEI,_AT_GET_IMEI);
    Result = Step_Config(&sSim900_status.No_Process_i16,_AT_CHECK_SIM,_AT_CHECK_SIM);   //
    
	return Result;
}

uint8_t Soft_Reset(void)
{
    uint8_t  Result = TRUE;
	Set_default_Sim900();
    
//    Step_Config(&sSim900_status.No_Process_i16,_AT_RESET_MODULE,_AT_RESET_MODULE);    //Comment day de update dc firm tu dien luc
//    osDelay(25000);
	Step_Config(&sSim900_status.No_Process_i16,_AT_DISPLAY_CMD,_AT_DISPLAY_CMD); 
    Step_Config(&sSim900_status.No_Process_i16,_AT_DISPLAY_CMD,_AT_DISPLAY_CMD);
    Step_Config(&sSim900_status.No_Process_i16,_AT_MAN_LOG,_AT_MAN_LOG);
	Step_Config(&sSim900_status.No_Process_i16,_AT_BAUD_RATE,_AT_BAUD_RATE); // _AT_BAUD_RATE- Lenh thiet lap lai Baudrate
    
    switch(sSim900_status.Radio_4G_GSM)
    {
        case 0:     //2G
            Step_Config(&sSim900_status.No_Process_i16,_AT_ACCESS_RA_2G,_AT_ACCESS_RA_2G);      
            break;
        case 1:   //3G
            Step_Config(&sSim900_status.No_Process_i16,_AT_ACCESS_RA_3G,_AT_ACCESS_RA_3G);
            break;
        case 2:   //4G
            Step_Config(&sSim900_status.No_Process_i16,_AT_ACCESS_RA_4G,_AT_ACCESS_RA_4G);
            break;
        case 3:   //Auto
            Step_Config(&sSim900_status.No_Process_i16,_AT_ACCESS_RA_AUTO,_AT_ACCESS_RA_AUTO);
            break;
        default:
            break;
    }
    osDelay(5000);
	Result = Step_Config(&sSim900_status.No_Process_i16,_AT_CHECK_SIM,_AT_CHECK_SIM);
	Step_Config(&sSim900_status.No_Process_i16,_AT_SIM_ID,_AT_SIM_ID);
    
    Step_Config(&sSim900_status.No_Process_i16,_AT_CHECK_RSSI,_AT_CHECK_RSSI);
    return Result;
}
/*
 * 	switch TranParent_Mode -> NonTranparent Mode
 */
uint8_t Check_Tranparent_Mode(uint8_t GSM_Step)
{
	if (sSim900_status.Mode_Tranfer_u8 == 1) 
    {
		sSim900_status.Step_Control_u8 = GSM_CLOSING;
		return 1;
	}
	return 0;
}

void Load_Receipt(truct_String *sReceipt,uint8_t *Array_Receipt)
{
	uint16_t Length = 0;
	sReceipt->Data_a8 = Array_Receipt;
	do {
		Length++;
	} while (*(Array_Receipt + Length) != 0);
	sReceipt->Length_u16 = Length;
}

int Display_Process, Display_Step, Display_run_Funtion = 0;
uint8_t	Do_Receipt(truct_String *sReceipt, int *No_Process, uint8_t *Flag_Success, uint8_t Flag_Next, uint8_t Flag_Return)
{
	uint32_t Wait_Time_u32 = TIME_DELAY_MQTT;
	uint8_t Step = 0;
	uint8_t Retry = 0;
    sSim900_status.fERROR_AT_Cmd = 0;      //Reset Flag Error
    
	while (Step < sReceipt->Length_u16) 
    {
		if (sSim900_status.Step_Control_u8 == GSM_SOFT_RESET) 
        {
			*Flag_Success = GSM_HARD_RESET; //
			return 0;
		}
		if ((sSim900_status.Step_Control_u8 != GSM_HTTP_TRANSFER_DOWNLOAD) && (sSim900_status.Step_Control_u8 != GSM_FTP_TRANSFER_DOWNLOAD)) 
            osDelay(50);        
          
		*No_Process = *(sReceipt->Data_a8 + Step);
        if (*No_Process == _AT_HTTP_CLOSE)
        {
            osDelay(10000);
        }
        
		sSim900_status.No_ResProcess_i16 = *No_Process;
		sSim900_status.Systick_LandMarkReset_u32 = RT_Count_Systick_u32;
		Display_Process = *No_Process;
		Display_Step = Step;
		if (CheckList_GSM[*No_Process].sSender.Length_u16 != NULL)
			HAL_UART_Transmit(&UART_SIM,CheckList_GSM[*No_Process].sSender.Data_a8,CheckList_GSM[*No_Process].sSender.Length_u16,1000);
		if (*No_Process == _RESET_SIM900) 
            break;		
//		osDelay(50);
		if (CheckList_GSM[*No_Process].sTempReceiver.Length_u16 == NULL) 
        { 
            // Step don't need Feedback
			if (CheckList_GSM[*No_Process].CallBack != NULL)
				CheckList_GSM[*No_Process].CallBack(NULL);
			Step++;
//			*No_Process = *(sReceipt->Data_a8 + Step); // Khong co dong lenh nay, se chay lai 2 lan CallBack -> do trinh bien dich compile code ASM
			Display_run_Funtion++;
			Retry = 0;
		} else 
        { 
            // Check Feedback of Sender
            Wait_Time_u32 = TIME_DELAY_MQTT;
			while (sSim900_status.No_ResProcess_i16 != _CORRECT_RESPONDING) 
            {
				osDelay(20);
				if (Check_Time_Out(sSim900_status.Systick_LandMarkReset_u32,Wait_Time_u32) == TRUE) {
					Retry++;
					break;
				}
                if(sSim900_status.fERROR_AT_Cmd == 1)
                    return 0;
				if (sSim900_status.No_ResProcess_i16 == _AT_TCP_OUT_DATAMODE) {
					sSim900_status.Mode_Tranfer_u8 = 0;
					sSim900_status.No_ResProcess_i16 = _CORRECT_RESPONDING;
					osDelay(1000); // 15_07
				}
			}
			if (sSim900_status.No_ResProcess_i16 == _CORRECT_RESPONDING) {
				Step++;
				Retry = 0;
			} else 
            {
				if (Retry > TIME_RETRY) 
                {
                    *Flag_Success = Flag_Return;
                    return FALSE;
				}
			}
		}
	}
	*Flag_Success = Flag_Next;
	return TRUE;
}


uint8_t State_Init_GPRS(void)
{
	return FALSE;
}

/*
 * Param :
 * 		- No_Begin_Process : vi tri dau tien cua chuoi gui di
 * 		- No_End_Process : vi tri cuoi cua chuoi gui gi
 * 		- *No_Process : tra ve vi tri Cua QUA TRINH ket noi mang
 * return 	: TRUE : - If it run Correct
 * 			: FALSE : - if it stuck
 */
int8_t Step_Config(int *No_Process, int Process_Begin, int Process_End)
{
	*No_Process = Process_Begin;
    
    sSim900_status.fERROR_AT_Cmd = 0;
	while (*No_Process <= Process_End)
	{
        if (*No_Process == _AT_TCP_OUT_DATAMODE) 
		{
            SIM_DTR_OFF1; 
            osDelay(1000);
		}
		osDelay(50);  //osDelay(1000);
        sSim900_status.No_ResProcess_i16 = *No_Process;
		HAL_UART_Transmit(&UART_SIM,CheckList_GSM[*No_Process].sSender.Data_a8,CheckList_GSM[*No_Process].sSender.Length_u16,1000);
        SIM_DTR_ON1;
        if (*No_Process == _AT_TCP_OUT_DATAMODE) 
            osDelay(1000);
		if (*No_Process == _RESET_SIM900) break;
		
		sSim900_status.Systick_LandMarkReset_u32 = RT_Count_Systick_u32;
		osDelay(10);
		while (sSim900_status.No_ResProcess_i16 != _CORRECT_RESPONDING) 
        {
			if (Check_Time_Out(sSim900_status.Systick_LandMarkReset_u32,TIME_DELAY_LONG) == TRUE) {
				break;
			}
            if(sSim900_status.fERROR_AT_Cmd == 1)
                return 0;
            //
			if ((sSim900_status.No_ResProcess_i16 == _AT_TCP_OUT_DATAMODE) &&
					(sSim900_status.Mode_Tranfer_u8 == 0)) {
				sSim900_status.No_ResProcess_i16 = _CORRECT_RESPONDING;
				osDelay(1000); // 15_07		
			}
            osDelay(50);
		}
		if (sSim900_status.No_ResProcess_i16 == _CORRECT_RESPONDING) (*No_Process)++;
		else {
			return FALSE;			
		}
	}
	return TRUE;
}


//---------------------------------------------------------
/*	Phan loai chuoi nhan qua SIM900
 *  return 	= -1 		:	If don't FIND Responding
 *  		= 0-0xFFFF 	:	No_Responding
 */
int Classify_Response(truct_String *Str_Final, int Process_Begin, int Process_End)
{
	int Result = -1;
	int Count_Step_Respond;
	int Pos_Str = -1;
    
	for (Count_Step_Respond = Process_Begin; Count_Step_Respond <= Process_End; ++Count_Step_Respond) {
		Pos_Str = Find_String_V2((truct_String*) &CheckList_GSM[Count_Step_Respond].sTempReceiver,Str_Final);
		if ((Pos_Str >= 0) && (CheckList_GSM[Count_Step_Respond].CallBack != NULL))
		{
			Result = Count_Step_Respond;
			CheckList_GSM[Count_Step_Respond].CallBack (&UartSIM_Control.str_Receiv);
		}
	}
	return Result;
}

/*
 Ham nay chi chay khi nhan du chuoi qua UART3 & DCU chay che do APP_FULL
return : enum of Step Write
*/

uint16_t Check_Responding(uint16_t No_Responding_wait)
{	
	int Pos_Str;
    int Pos_StrPing;
    uint16_t j = 0;
    truct_String    Str_Recei_Ping = {(uint8_t*) "+MPING:", 7};

    Pos_StrPing = Find_String_V2(&Str_Recei_Ping,&UartSIM_Control.str_Receiv);
    if (Pos_StrPing >= 0)
    {
        for(j = Pos_StrPing; j<UartSIM_Control.str_Receiv.Length_u16; j++)
        {
            *(sDCU.StrPing_Recei.Data_a8 + sDCU.StrPing_Recei.Length_u16++) = *(UartSIM_Control.str_Receiv.Data_a8 + j);
            
            if(sDCU.StrPing_Recei.Length_u16 >=499)
              sDCU.StrPing_Recei.Length_u16 = 0;
        }
        if(Check_Finish_Ping(&UartSIM_Control.str_Receiv) == 1)
            sDCU.Fl_Finish_Ping = 1;
    }
    //check request tai cho
    if(sDCU.fRecei_StrAT_OK == 1)
    {
        Pos_StrPing = Find_String_V2(&sSim900_status.StrCheck_AT_Req,&UartSIM_Control.str_Receiv);
        if (Pos_StrPing >= 0)
        {
            for(j = Pos_StrPing; j<UartSIM_Control.str_Receiv.Length_u16; j++)
            {
                *(sDCU.StrPing_Recei.Data_a8 + sDCU.StrPing_Recei.Length_u16++) = *(UartSIM_Control.str_Receiv.Data_a8 + j);
                
                if(sDCU.StrPing_Recei.Length_u16 >=499)
                  sDCU.StrPing_Recei.Length_u16 = 0;
            }
            sDCU.fRecei_StrAT_OK = 0;   //flag cho lay tra ve cua 1 lenh AT bat ki
            Reset_UART(&UartSIM_Control);
            return 1;
        }
    }   
    //
    //Check xem có URC nào khong. URC la dieu kien quan trong nhat. phai dat truoc vi sau do nó bi convert het sang chu thuong
    if ((sSim900_status.Step_Control_u8 != GSM_HTTP_TRANSFER_DOWNLOAD) && (sSim900_status.Step_Control_u8 != GSM_FTP_TRANSFER_DOWNLOAD)) 
        Classify_Response(&UartSIM_Control.str_Receiv,_RESET_SIM900,_RES_ERROR);
    
	Pos_Str = Find_String_V2((truct_String*) &CheckList_GSM[No_Responding_wait].sTempReceiver,&UartSIM_Control.str_Receiv);
	if (Pos_Str >= 0)
	{
		if (CheckList_GSM[No_Responding_wait].CallBack != NULL) 
			CheckList_GSM[No_Responding_wait].CallBack (&UartSIM_Control.str_Receiv);
        
        sSim900_status.No_ResProcess_i16 = _CORRECT_RESPONDING;   //Neu de tren update firmware kem.
	}
    // check At command from Server vao day no convert thanh chu thuong het roi
    if(sSim900_status.Status_Connect_u8 == 1)
        CheckATCommandServer();             

    Reset_UART(&UartSIM_Control);
	return 1;
}

/*
 * FUNCTION to control GSM
 *
 */
void _fRESET_SIM900(truct_String *str_Receiv)
{
//	Reset_GPIO();
	sSim900_status.Step_Control_u8 = GSM_HARD_RESET;
}

void _fRES_SIM_LOST(truct_String *str_Receiv)
{
	sSim900_status.Call_Ready_u8 	= 0;
	sSim900_status.Sim_inserted_u8 	= 0;
	sSim900_status.GPRS_u8	 	= 0;
	sSim900_status.No_ResProcess_i16 = _RES_SIM_LOST;
    sSim900_status.fERROR_AT_Cmd = 1;
	// Ghi lai su kien mat SIM -> Co kiem tra neu Buff_Log_DCU >= 256  -> ghi flash
	// Flash cung co 2 bo dem Buff_log_DCU1 va Buff_log_DCU2; co Buff_Log_DCU_sel : xac dinh ghi du lieu vao thanh nao.
}

void _fCHECK_ACK_MESS(truct_String *str_Receiv)
{
    sTCP.KindFB = _AT_TCP_SEND;
}



void _fRES_CLOSED(truct_String *str_Receiv)
{
	if((sSim900_status.CheckConnectATCommand !=  AT_DISCONNECT_SERVICE)&&(sSim900_status.CheckConnectATCommand !=  AT_DISCONNECT_TCP)) 
    {
        sSim900_status.Step_Control_u8      = GSM_CLOSING;
        sSim900_status.Status_Connect_u8    = 0;
        sSim900_status.fERROR_AT_Cmd = 1;
	}
}

void _fRES_CALL_READY(truct_String *str_Receiv)
{
	sSim900_status.Call_Ready_u8 		= 1;
	sSim900_status.Sim_inserted_u8 		= 1;
	sSim900_status.No_ResProcess_i16 	= _RES_CALL_READY;
	// Ghi lai su kien co song GSM
}

void _fRES_ALREADY_CONNECT(truct_String *str_Receiv)
{
	sSim900_status.No_ResProcess_i16 	= _CORRECT_RESPONDING;
	Step_Config(&sSim900_status.No_Process_i16,_AT_TCP_IN_DATAMODE,_AT_TCP_IN_DATAMODE);
	// Ghi lai su kien co ket noi GPRS
}

void _fAT_CHECK_SIM(truct_String *str_Receiv)
{
	sSim900_status.Sim_inserted_u8 	= 1;
	sSim900_status.Call_Ready_u8 	= 1;
	sSim900_status.No_ResProcess_i16 = _CORRECT_RESPONDING;
	// ghi su kien co SIM vao LOG
}

void _fAT_CHECK_RSSI(truct_String *str_Receiv)
{
	int Pos_Str = -1;
	sSim900_status.No_ResProcess_i16 = _CORRECT_RESPONDING;
	/* ghi lai RSSI va BER */
	Pos_Str = Find_String_V2((truct_String*) &CheckList_GSM[_AT_CHECK_RSSI].sTempReceiver,str_Receiv);
	if(Pos_Str >= 0) {
		sSim900_status.RSSI_c8 = *(str_Receiv->Data_a8 + Pos_Str + 5) - 0x30;
		sSim900_status.Ber_c8 = *(str_Receiv->Data_a8 + Pos_Str + 7) - 0x30;
		if(*(str_Receiv->Data_a8 + Pos_Str + 7) == 0x2C) {
			sSim900_status.RSSI_c8 = ((*(str_Receiv->Data_a8 + Pos_Str + 5))<<4) + \
															 ((*(str_Receiv->Data_a8 + Pos_Str + 6)) & 0x0F);
			sSim900_status.Ber_c8 = *(str_Receiv->Data_a8 + Pos_Str + 8) - 0x30;  // ber 0->7
		}
	}
	/* convert signal strength */
	switch(sSim900_status.RSSI_c8) {
		case 0x00:
			sSim900_status.RSSI_c8 = 113;
			break;
		case 0x01:
			sSim900_status.RSSI_c8 = 111;	
			break;
		case 0x31:
			sSim900_status.RSSI_c8 = 51;
			break;
        case 0x99:
			sSim900_status.RSSI_c8 = 0;
			break;
		default: //2 den 30/ 2 per step: 0x19 ->19  sau do - 2 = 17
			sSim900_status.RSSI_c8 = 113 - (((sSim900_status.RSSI_c8)/16)*10 + sSim900_status.RSSI_c8 %16)*2;
			break;
	}
}


void _fAT_CHECK_CREG(truct_String *str_Receiv)
{
	// Ghi lai su kien co ban tin CREG
	sSim900_status.No_ResProcess_i16 	= _CORRECT_RESPONDING;
}

void _fAT_DELAY(truct_String *str_Receiv)
{
	// Ghi lai su kien co ban tin CREG
	osDelay(1000); // Delay wait to search Cell ID off GSM
}

void _fAT_CHECK_IP(truct_String *str_Receiv)
{
	sSim900_status.GPRS_u8 = INIT_CONNECT;	
	sSim900_status.No_ResProcess_i16 	= _CORRECT_RESPONDING;	
}


void _fAT_SIM_ID(truct_String *str_Receiv)
{
	truct_String	sRes;
	uint16_t 		Pos,var;
	uint8_t			aRes_Data[SIM_ID_LENGTH];
	sRes.Data_a8 	= aRes_Data;
	sRes.Length_u16 = 0;
	for (Pos = 1; Pos < str_Receiv->Length_u16; ++Pos) 
    {
		if ((str_Receiv->Data_a8[Pos-1] == '8') && (str_Receiv->Data_a8[Pos] == '9')) break;
	}
	for (var = Pos-1; var < str_Receiv->Length_u16; var++) {
		if ((str_Receiv->Data_a8[var] < '0') || (str_Receiv->Data_a8[var] > '9')) break;
	}
	sRes.Data_a8 = &str_Receiv->Data_a8[Pos-1];
	sRes.Length_u16 = var - Pos + 1;
	if(sRes.Length_u16 > SIM_ID_LENGTH) sRes.Length_u16 = SIM_ID_LENGTH;
	if ((Find_String_V2(&sDCU.sSIM_id,&sRes) != 0)||(sRes.Length_u16 != sDCU.sSIM_id.Length_u16)) { // co SIM moi
		for (var = 0; var < sRes.Length_u16; ++var) {
			sDCU.sSIM_id.Data_a8[var] = sRes.Data_a8[var];
		}
		sDCU.sSIM_id.Length_u16 = sRes.Length_u16;
//		Save_SIM_ID();
	}
}


void _fRESET_COUNT_FIRST(truct_String *str_Receiv)
{
	CountFirst = 0;
}

void _fTRANPARENT_MODE1(truct_String *str_Receiv)
{
	if (CountFirst == 0) {
	/*
	 * Check voi IAR
	 * co bi chay lai 2 lan ham nay ko -> thong qua bien " CountFirst"
	 */
		CountFirst++;
        if(sDCU.Flag_ConnectNewServer == 1)
            HAL_UART_Transmit(&UART_SIM,sInformation.sServer_MQTT_Request.Data_a8,sInformation.sServer_MQTT_Request.Length_u16,1000);
        else
            HAL_UART_Transmit(&UART_SIM,sInformation.sServer_MQTT.Data_a8,sInformation.sServer_MQTT.Length_u16,1000);
	}
}

void _fTRANPARENT_MODE2(truct_String *str_Receiv)
{
    if(sDCU.Flag_ConnectNewServer == 1)
        HAL_UART_Transmit(&UART_SIM,sInformation.sPort_MQTT_Request.Data_a8,sInformation.sPort_MQTT_Request.Length_u16,1000);
    else
        HAL_UART_Transmit(&UART_SIM,sInformation.sPort_MQTT.Data_a8,sInformation.sPort_MQTT.Length_u16,1000);
}

void _fOPEN_TCP(truct_String *str_Receiv)
{
//    //Phan biet voi CONNECT FAIL
//	if (str_Receiv->Length_u16 <= (CheckList_GSM[_AT_TCP_CONNECT2].sTempReceiver.Length_u16 + 4))
//	{ // except "Connect fail"
//		sSim900_status.No_ResProcess_i16 	= _CORRECT_RESPONDING;
//	}
    int Posfix = 0;
    
    truct_String StConnectOk = {(uint8_t*)"CONNECT \r\n",10};
    
    Posfix = Find_String_V2((truct_String*) &StConnectOk, &UartSIM_Control.str_Receiv);
	if (Posfix >= 0)
	{
        //neu thanh cong check tiep
        sSim900_status.No_ResProcess_i16 = _CORRECT_RESPONDING;
    }else
    {
        sSim900_status.fERROR_AT_Cmd = 1;   //bao loi de thoat
    }
}



void _fPING_TCP(truct_String *str_Receiv)
{
    if(CountFirst == 0)
    {
        CountFirst++;
        HAL_UART_Transmit(&UART_SIM,sInformation.Ping_IP.Data_a8,sInformation.Ping_IP.Length_u16,100);
    }
}

void _fGET_IMEI(truct_String *str_Receiv)
{
    uint16_t i = 0;
    
    for(i = 0; i< str_Receiv->Length_u16; i++)
      if((*(str_Receiv->Data_a8 + i) >= 0x30) && (*(str_Receiv->Data_a8 + i) <= 0x39)) 
        break;
    
//    if(sDCU.Init_ID_Flash == 0) //luu ID vao Flash neu nhu trong Flash chua luu ID
//    {
//        if(i < str_Receiv->Length_u16)
//        {
//            //Reset Buf IMEI
//            Reset_Buff(&sDCU.sDCU_id);
//            while((*(str_Receiv->Data_a8 + i) >= 0x30) && (*(str_Receiv->Data_a8 + i) <= 0x39)) 
//            {
//                *(sDCU.sDCU_id.Data_a8 + sDCU.sDCU_id.Length_u16++) = *(str_Receiv->Data_a8 + i);
//                i++;
//                if(sDCU.sDCU_id.Length_u16 >=20) break;
//            }
//            //
//            Save_DCU_ID();
//            sDCU.Init_ID_Flash = 1;
//        }
//    }
}


void _fCHECK_APN_ACTIVE(truct_String *str_Receiv)
{
    //Kiem tra dau IP cua dien luc
    if(Find_String_V2(&strIP_Dienluc,&UartSIM_Control.str_Receiv) >= 0)
    {
        sSim900_status.factiveAPN = 1;
    }else
        sSim900_status.factiveAPN = 0;
}



void _fGet_IP_Module(truct_String *str_Receiv)
{
    uint16_t i = 0;
    
    sDCU.Str_IP_Module.Data_a8 = &Buff_IPModule[0];
    Reset_Buff(&sDCU.Str_IP_Module);
    
    for(i = 0; i< str_Receiv->Length_u16; i++)
      if((*(str_Receiv->Data_a8 + i)) == ',')
        break;
     
    i++;
       
    if(i >= str_Receiv->Length_u16) return;
    
	while(*(str_Receiv->Data_a8 + i) != 0x0D)
    {
        *(sDCU.Str_IP_Module.Data_a8 + sDCU.Str_IP_Module.Length_u16++) = *(str_Receiv->Data_a8 + i);
        i++;
         if(sDCU.Str_IP_Module.Length_u16 >=20) 
           break;
    }
    //luu IP vao Flash  
}



void _fPING_TEST(truct_String *str_Receiv)
{
    if(CountFirst == 0) 
    {
        CountFirst++;
        HAL_UART_Transmit(&UART_SIM,sInformation.Str_URL_Firm_Add.Data_a8,sInformation.Str_URL_Firm_Add.Length_u16,100);
    }
}



void _f_COFI_CONTEXT_2(truct_String *str_Receiv)
{
    if(CountFirst == 0)
    {
        CountFirst++;
        HAL_UART_Transmit(&UART_SIM,sInformation.APN.Data_a8,sInformation.APN.Length_u16,1000);
    }
}



void _f_APN_AUTHEN_1(truct_String *str_Receiv)
{
    if(CountFirst == 0)
    {
        CountFirst++;
        if((sInformation.APN_Username.Length_u16 == 0) && (sInformation.APN_Password.Length_u16 == 0))
        {
            HAL_UART_Transmit(&UART_SIM, (uint8_t*) "0", 1, 1000);
        }else
        {
            HAL_UART_Transmit(&UART_SIM, (uint8_t*) "3,\"", 3, 1000);
            HAL_UART_Transmit(&UART_SIM,sInformation.APN_Username.Data_a8,sInformation.APN_Username.Length_u16,1000);
            HAL_UART_Transmit(&UART_SIM, (uint8_t*) "\",\"", 3, 1000);
            HAL_UART_Transmit(&UART_SIM,sInformation.APN_Password.Data_a8,sInformation.APN_Password.Length_u16,1000);
            HAL_UART_Transmit(&UART_SIM, (uint8_t*) "\"", 1, 1000);
        }
    }
}


void _fTRANPARENT_MODE(truct_String *str_Receiv)
{
    sSim900_status.Mode_Tranfer_u8 = 1;
    sSim900_status.No_ResProcess_i16 	= _CORRECT_RESPONDING;
}

void _fNON_TRANPARENT_MODE(truct_String *str_Receiv)
{
	sSim900_status.Mode_Tranfer_u8 = 0;
	sSim900_status.No_ResProcess_i16 	= _CORRECT_RESPONDING;
}

void _f_GET_CLOCK(truct_String *str_Receiv)
{
	int             Pos_Fix = 0;
	uint16_t        Temp_Get = 0;
    uint16_t        Temp_year = 0;
    ST_TIME_FORMAT  sRTC_Get;
    uint8_t         count = 0;
    uint8_t         aRTCtemp[8];
	
	Pos_Fix = Find_String_V2((truct_String*) &CheckList_GSM[sSim900_status.No_Process_i16].sTempReceiver,str_Receiv);
    
    if(str_Receiv->Length_u16 > (Pos_Fix + CheckList_GSM[sSim900_status.No_Process_i16].sTempReceiver.Length_u16 + 14))
	{
        Pos_Fix += CheckList_GSM[sSim900_status.No_Process_i16].sTempReceiver.Length_u16 + 1;   //dau khoang trong va nhay ++ de tro vao year
        
        //tim vi tri bat dau so decima
        Temp_year = *(str_Receiv->Data_a8 + Pos_Fix);
        while ((0x30 > Temp_year) || (Temp_year > 0x39))
        {
            Pos_Fix++;
            Temp_year = *(str_Receiv->Data_a8 + Pos_Fix);
        }
        
        //bat dau vi tri co so: year month date hour min sec
        while(count < 6)
        {
            Temp_Get  = 0;
            Temp_year = *(str_Receiv->Data_a8 + Pos_Fix);
            while ((0x30 <=  Temp_year) && (Temp_year <= 0x39))
            {
                Temp_Get = Temp_Get* 10 + Temp_year - 0x30;
                Pos_Fix++;
                Temp_year = *(str_Receiv->Data_a8 + Pos_Fix);
            }
                    
            aRTCtemp[count++] = Temp_Get%100;               //nam chi lay 2 chu so sau
            
            if(Pos_Fix >= str_Receiv->Length_u16) 
                return;
            
            Pos_Fix++; //dau /
        }
        sRTC_Get.year   = aRTCtemp[0];
        sRTC_Get.month  = aRTCtemp[1];
        sRTC_Get.date   = aRTCtemp[2];
        sRTC_Get.hour   = aRTCtemp[3];   // + 7;  //GMT = 7
        sRTC_Get.min    = aRTCtemp[4];
        sRTC_Get.sec    = aRTCtemp[5];
        if(sRTC_Get.year < 20) 
                return;

        Convert_Time_GMT(&sRTC_Get, 0);  //GMT 7
        //Convert lai day. 1/1/2012 la chu nhat. Thu 2 - cn: 2-8
        sRTC_Get.day = ((HW_RTC_GetCalendarValue_Second(sRTC_Get, 1)/SECONDS_IN_1DAY) + 6)%7 + 1;
        Set_RTC_2(sRTC_Get);
        sDCU.Flag_Get_sTime_OK = 1;
    }
	  
}



void _fRES_ERROR(truct_String *str_Receiv)
{
    if(sSim900_status.Step_Control_u8 == GSM_HARD_RESET)
    {
        sSim900_status.Step_Control_u8 = GSM_HARD_RESET;
        sSim900_status.fERROR_AT_Cmd = 1;
        return;
    }
    
    if ((sSim900_status.No_Process_i16 != _AT_HTTP_REQUEST_GET) && (sSim900_status.No_Process_i16 != _AT_HTTP_READ_2)) 
    {
        sSim900_status.Step_Control_u8 = GSM_SOFT_RESET;
        sSim900_status.fERROR_AT_Cmd = 1;
    } 
}

/*
 * 			FTP - DATA
 */
void _fFTP_GET_DONE(truct_String *str_Receiv)
{
	sSim900_status.No_ResProcess_i16 	= _CORRECT_RESPONDING;
}



void _fFTP_SENDIP_2(truct_String *str_Receiv)
{
	if(CountFirst == 0) 
	{
		HAL_UART_Transmit(&UART_SIM,sInformation.Str_URL_Firm_Add.Data_a8,sInformation.Str_URL_Firm_Add.Length_u16,1000);
		CountFirst++;
	}
}

void _fSystem_Del_File(truct_String *str_Receiv)
{
    HAL_UART_Transmit(&UART_SIM, (uint8_t *)&firmFileName[1], (SIZE_OF_FIRMWARE - 1),100);
}

void _fCFTP_GET_FILE1(truct_String *str_Receiv)
{
    if (CountFirst == 0)
    {
        CountFirst++;
        
        HAL_UART_Transmit(&UART_SIM, sInformation.Str_URL_Firm_Path.Data_a8,sInformation.Str_URL_Firm_Path.Length_u16,100);
        if(sInformation.Str_URL_Firm_Path.Length_u16 != 0) HAL_UART_Transmit(&UART_SIM, (uint8_t*) "/",1,100);
        HAL_UART_Transmit(&UART_SIM, sInformation.Str_URL_Firm_Name.Data_a8, sInformation.Str_URL_Firm_Name.Length_u16,100); 
    }
}

void _fCFTP_GET_FILE2(truct_String *str_Receiv)
{
   HAL_UART_Transmit(&UART_SIM, (uint8_t *) &firmFileName[0], SIZE_OF_FIRMWARE, 100); /* Save data */
}

void _fCFTP_GET_FILE(truct_String *str_Receiv)
{
    int16_t index =0;
    int16_t Pos_Str = -1;
    uint8_t statusM = 0;
    uint8_t dataMemory[8] = {0x00};
    uint8_t count = 0;
     Pos_Str = Find_String_V2((truct_String*) &CheckList_GSM[_AT_FTP_GET_FILE].sTempReceiver,str_Receiv);
    if(Pos_Str >= 0)
    {
        for(index = Pos_Str; index < str_Receiv->Length_u16; index++)
        {
            if(*(str_Receiv->Data_a8 + index) == ',')
            {
                statusM = 1;
            }
            if(*(str_Receiv->Data_a8 + (index + 1)) == '\r')
            {
                statusM = 0;
                break;
            }
            if(statusM == 1)
            {
                dataMemory[count] = *(str_Receiv->Data_a8 + (index + 1));
                count++;
            }
            if(count == 8)
            {
                break;
            }
        }
        sFTP.FirmWare_Length = converStringToDec(dataMemory, count);
    }

}



void _fFTP_READ_1(truct_String *str_Receiv)
{
    uint8_t aNumbyte[10];
    uint8_t aLengread[10];
    truct_String    strPos      = {&aNumbyte[0], 0};
    truct_String    strlenread  = {&aLengread[0], 0};
    //chuyen offset ra string de truyen vao sim
    Pack_HEXData_Frame_Uint64(&strPos, sFTP.Count_Data_u32, 0);
    
    if(sFTP.FirmWare_Length >= (sFTP.Count_Data_u32 + 1024))
    {
        Pack_HEXData_Frame_Uint64(&strlenread, 1024, 0);
    }else Pack_HEXData_Frame_Uint64(&strlenread, (sFTP.FirmWare_Length - sFTP.Count_Data_u32), 0);
      
    if(CountFirst == 0)
    {
        CountFirst++;
        HAL_UART_Transmit(&UART_SIM,(uint8_t*) &firmFileName[0], SIZE_OF_FIRMWARE,100);
        HAL_UART_Transmit(&UART_SIM,(uint8_t*) "\",", 2,100);
        HAL_UART_Transmit(&UART_SIM,(uint8_t*) strPos.Data_a8, strPos.Length_u16,100);
        HAL_UART_Transmit(&UART_SIM,(uint8_t*) ",", 1,100);
        HAL_UART_Transmit(&UART_SIM,(uint8_t*) strlenread.Data_a8, strlenread.Length_u16,100);
    }
}

void _fFTP_READ_2(truct_String *str_Receiv)
{
    int             Pos = 0, Pos2 = 0, PosHeadSV = 0;
    truct_String    strHeader      = {(uint8_t*)"+CFTPRDFILE:",12};
    truct_String    strStartHttp   = {(uint8_t*)"\r\n",2};
    truct_String    strFix;
    uint16_t        i = 0;
    uint8_t         aNumbyte[10];
    uint8_t         lengthnum = 0;
    uint16_t        NumbyteRecei = 0;    
    truct_String    StrHeaderSVFirm = {(uint8_t*)"ModemFirmware:SV",16};
    
    Pos = Find_String_V2((truct_String*) &strHeader, &UartSIM_Control.str_Receiv);
	if (Pos >= 0)
	{
        for(i = Pos; i < UartSIM_Control.str_Receiv.Length_u16; i++)
          if(*(UartSIM_Control.str_Receiv.Data_a8 + i) == ',')
          {
              i++;
              while ((*(UartSIM_Control.str_Receiv.Data_a8 + i) >= 0x30) && (*(UartSIM_Control.str_Receiv.Data_a8 + i) <= 0x39))
              {
                   aNumbyte[lengthnum++] = *(UartSIM_Control.str_Receiv.Data_a8 + i) - 0x30;
                   i++;
              } 
              break;
          }
        //convert ra so byte nhan dc
        for(i = 0; i < lengthnum; i++)
            NumbyteRecei = NumbyteRecei *10 + aNumbyte[i];
        
        if(NumbyteRecei == 0) 
        {
            sFTP.Update_success = FALSE;
            sFTP.Receiver_u8 = SEND_UPDATE_FIRMWARE_FAIL_LENGTH;
            return;
        }
        
        strFix.Data_a8 = UartSIM_Control.str_Receiv.Data_a8 + Pos;
        strFix.Length_u16 = UartSIM_Control.str_Receiv.Length_u16 - Pos;
        
        Pos2 = Find_String_V2((truct_String*)&strStartHttp, &strFix);  //check vi tri bat dau cua toan bo data ftp
        if (Pos2 >= 0)
        {   
            Pos2  += strStartHttp.Length_u16;
            //
            strFix.Data_a8 = UartSIM_Control.str_Receiv.Data_a8 + Pos + Pos2;
            if(NumbyteRecei > (UartSIM_Control.str_Receiv.Length_u16 - Pos - Pos2 - 19))  //ket thuc +CFTPRDFILE:SUCCESS 19 +2+ chuoi so
            {
                sFTP.Update_success = FALSE;
                sFTP.Receiver_u8 = SEND_UPDATE_FIRMWARE_FAIL_LENGTH;
                return;
            }
            strFix.Length_u16 = NumbyteRecei;
            //
            sFTP.Count_Data_u32 += NumbyteRecei;                          // tong so data ma ban tin nhan doc ra cua http
            sFTP.Offset = sFTP.Count_Data_u32;
            //Neu la Pack dau tien, Check 32 byte header xem có chu: ModemFirmware:SV1.1,0x0800C000
            if(sFTP.Count_PacketFirm_ExFlash == 0)
            {
                PosHeadSV = Find_String_V2((truct_String*)&StrHeaderSVFirm, &strFix);  
                if(PosHeadSV >= 0)
                  sFTP.HeaderSV_OK = 1;
                //Lay byte cuoi cung ra: byte 32 cua Packdau tien. Sau header 31 byte
                sFTP.LastCrcFile = *(strFix.Data_a8 + 31);  
            }
               
            if(sFTP.HeaderSV_OK == 0)    //neu khong co header SV code thi return luon
            {
                sFTP.Update_success = FALSE;
                sFTP.Receiver_u8    = SEND_UPDATE_FIRMWARE_FAIL_FLASH;
                return;
            }
            //viet lai chuong trinh update firm luu o flash ngoai
            //Luu data firm vao flash ngoai. Sau khi ghi vao con doc ra de check xem ghi co dung k.
            if (Flash_S25FL_Log_Message(strFix.Data_a8, strFix.Length_u16, 9) != 1)   //Neu luu bi loi
            {
                sFTP.Update_success = FALSE;
                sFTP.Receiver_u8    = SEND_UPDATE_FIRMWARE_FAIL_FLASH;
                return;
            }
            
            //kiem tra xem nhan du byte chua de bao ok
            if(sFTP.Count_Data_u32 == sFTP.FirmWare_Length)             
            {
                //Tinh crc cua tat ca cac CRC
                for(i = 0; i < sFTP.Count_PacketFirm_ExFlash; i++)
                    sFTP.LasCrcCal += aCrC_Firmware[i];
                //Ghi buff crc vao vi tri exflash
                sFTP.Count_PacketFirm_ExFlash = RePacket_Message( &aCrC_Firmware[0], sFTP.Count_PacketFirm_ExFlash);
                if (Flash_S25FL_Log_Message(&aCrC_Firmware[0], sFTP.Count_PacketFirm_ExFlash, 10) != 1)   //Neu luu bi loi
                {
                    sFTP.Update_success = FALSE;
                    sFTP.Receiver_u8    = SEND_UPDATE_FIRMWARE_FAIL_FLASH;
                    return;
                }
                //kiem tra CRC o cuoi cung file .Bin cua Firm: bang cach tinh lai crc cua tat ca cac CRC phia tren. Roi so sanh voi byte cuoi cung
                if(sFTP.LastCrcFile == sFTP.LasCrcCal)
                {
                    sFTP.Update_success = TRUE;
                    sFTP.Receiver_u8 = SEND_UPDATE_FIRMWARE_OK;
                    Erase_Firmware(ADDR_FLAG_HAVE_NEW_FW, 1);
                    //ghi Flag update va Size firm vao Inflash
                    HAL_FLASH_Unlock();
                    osDelay(10);
                    HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,ADDR_FLAG_HAVE_NEW_FW,0xAA);
                    HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,ADDR_FLAG_HAVE_NEW_FW+0x08, sFTP.Count_Data_u32);
                    osDelay(10);
                    HAL_FLASH_Lock();
                }else
                {
                    sFTP.Update_success = FALSE;
                    sFTP.Receiver_u8 = SEND_UPDATE_FIRMWARE_FAIL_LENGTH;
                }
            }
            //Bao fail neu Count  > Length thuc te cua firm
            if (sFTP.Count_Data_u32 > sFTP.FirmWare_Length)
            {
                sFTP.Update_success = FALSE;
                sFTP.Receiver_u8 = SEND_UPDATE_FIRMWARE_FAIL_LENGTH;
            } 
        }else
        {
            sFTP.Update_success = FALSE;
			sFTP.Receiver_u8 = SEND_UPDATE_FIRMWARE_FAIL_TIME;
        }
    }else
    {
        sFTP.Update_success = FALSE;
        sFTP.Receiver_u8 = SEND_UPDATE_FIRMWARE_FAIL_SETUP;
    }
}



void _fHTTP_SETURL_1(truct_String *str_Receiv)
{
    if(CountFirst == 0) 
	{
        CountFirst++;
		HAL_UART_Transmit(&UART_SIM,sInformation.Str_URL_HTTP.Data_a8,sInformation.Str_URL_HTTP.Length_u16,1000);
        HAL_UART_Transmit(&UART_SIM, (uint8_t*) ",", 1,1000);
        HAL_UART_Transmit(&UART_SIM,sInformation.Str_URL_Firm_Port.Data_a8,sInformation.Str_URL_Firm_Port.Length_u16,1000);
	}
}

void _fHTTP_SETURL_2(truct_String *str_Receiv)
{
  
}


void _fHTTP_FIRM_LENGTH(truct_String *str_Receiv)
{
    int16_t index =0;
    int16_t Pos_Str = -1;
    uint8_t statusM = 0;
    uint8_t dataMemory[8] = {0x00};
    uint8_t count = 0;
    Pos_Str = Find_String_V2((truct_String*) &CheckList_GSM[_AT_HTTP_LENGTH].sTempReceiver,str_Receiv);
    if(Pos_Str >= 0)
    {
        for(index = Pos_Str; index < str_Receiv->Length_u16; index++)
        {
            if(*(str_Receiv->Data_a8 + index) == ',')
            {
                statusM = 1;
            }
            if(*(str_Receiv->Data_a8 + (index + 1)) == '\r')
            {
                statusM = 0;
                break;
            }
            if(statusM == 1)
            {
                dataMemory[count] = *(str_Receiv->Data_a8 + (index + 1));
                count++;
            }
            if(count == 8)
            {
                break;
            }
        }
        sFTP.FirmWare_Length = converStringToDec(dataMemory, count);
    }      
}

void _fHTTP_READ_1(truct_String *str_Receiv)
{
    uint8_t         aOffset[10];
    truct_String    strOffset = {&aOffset[0], 0};
    //chuyen offset ra string de truyen vao sim
    Pack_HEXData_Frame_Uint64(&strOffset, sFTP.Offset, 0);

    if(CountFirst == 0) 
	{
        CountFirst++;
		HAL_UART_Transmit(&UART_SIM, strOffset.Data_a8, strOffset.Length_u16,1000);
	}
}



void _fHTTP_READ_2(truct_String *str_Receiv)
{
    int             Pos = 0, Pos2 = 0, PosHeadSV = 0;
    truct_String    strHeader      = {(uint8_t*)"$HTTPREAD:",10};
    truct_String    strStartHttp   = {(uint8_t*)"\r\n",2};
    truct_String    strFix;
    uint16_t        i = 0;
    uint8_t         aNumbyte[10];
    uint8_t         lengthnum = 0;
    uint16_t        NumbyteRecei = 0;
    truct_String    StrHeaderSVFirm = {(uint8_t*)"ModemFirmware:",14};
    
    
    Pos = Find_String_V2((truct_String*) &strHeader, &UartSIM_Control.str_Receiv);
	if (Pos >= 0)
	{
        for(i = Pos; i < UartSIM_Control.str_Receiv.Length_u16; i++)
          if(*(UartSIM_Control.str_Receiv.Data_a8 + i) == ',')
          {
              i++;
              while ((*(UartSIM_Control.str_Receiv.Data_a8 + i) >= 0x30) && (*(UartSIM_Control.str_Receiv.Data_a8 + i) <= 0x39))
              {
                   aNumbyte[lengthnum++] = *(UartSIM_Control.str_Receiv.Data_a8 + i) - 0x30;
                   i++;
              } 
              break;
          }
        //convert ra so byte nhan dc
        for(i = 0; i < lengthnum; i++)
            NumbyteRecei = NumbyteRecei *10 + aNumbyte[i];
        
        if(NumbyteRecei == 0) 
        {
            sFTP.Update_success = FALSE;
            sFTP.Receiver_u8 = SEND_UPDATE_FIRMWARE_FAIL_LENGTH;
            return;
        }
        
        strFix.Data_a8 = UartSIM_Control.str_Receiv.Data_a8 + Pos;
        strFix.Length_u16 = UartSIM_Control.str_Receiv.Length_u16 - Pos;
        
        Pos2 = Find_String_V2((truct_String*)&strStartHttp, &strFix);  //check vi tri bat dau cua toan bo data http
        if (Pos2 >= 0)
        {   
            Pos2  += strStartHttp.Length_u16;
            //
            strFix.Data_a8 = UartSIM_Control.str_Receiv.Data_a8 + Pos + Pos2;
            if(NumbyteRecei > (UartSIM_Control.str_Receiv.Length_u16 - Pos - Pos2 - 6))
            {
                sFTP.Update_success = FALSE;
                sFTP.Receiver_u8 = SEND_UPDATE_FIRMWARE_FAIL_LENGTH;
                return;
            }
            strFix.Length_u16 = NumbyteRecei;
            //
            sFTP.Count_Data_u32 += NumbyteRecei;                          // tong so data ma ban tin nhan doc ra cua http
            sFTP.Offset = sFTP.Count_Data_u32;          
            //Neu la Pack dau tien, Check 32 byte header xem có chu: ModemFirmware:SV1.1,0x0800C000
            if(sFTP.Count_PacketFirm_ExFlash == 0)
            {
                PosHeadSV = Find_String_V2((truct_String*)&StrHeaderSVFirm, &strFix);  
                if(PosHeadSV >= 0)
                    sFTP.HeaderSV_OK = 1;
                //Lay byte cuoi cung ra: byte 32 cua Packdau tien. Sau header 31 byte
                sFTP.LastCrcFile = *(strFix.Data_a8 + 31);  
            }
               
            if(sFTP.HeaderSV_OK == 0)    //neu khong co header SV code thi return luon
            {
                sFTP.Update_success = FALSE;
                sFTP.Receiver_u8    = SEND_UPDATE_FIRMWARE_FAIL_FLASH;
                return;
            }
            //viet lai chuong trinh update firm luu o flash ngoai
            //Luu data firm vao flash ngoai. Sau khi ghi vao con doc ra de check xem ghi co dung k.
            if (Flash_S25FL_Log_Message(strFix.Data_a8, strFix.Length_u16, 9) != 1)   //Neu luu bi loi
            {
                sFTP.Update_success = FALSE;
                sFTP.Receiver_u8    = SEND_UPDATE_FIRMWARE_FAIL_FLASH;
                return;
            }
            
            //kiem tra xem nhan du byte chua de bao ok
            if(sFTP.Count_Data_u32 == sFTP.FirmWare_Length)             
            {
                //Tinh crc cua tat ca cac CRC
                for(i = 0; i < sFTP.Count_PacketFirm_ExFlash; i++)
                    sFTP.LasCrcCal += aCrC_Firmware[i];
                //Ghi buff crc vao vi tri exflash
                sFTP.Count_PacketFirm_ExFlash = RePacket_Message( &aCrC_Firmware[0], sFTP.Count_PacketFirm_ExFlash);
                if (Flash_S25FL_Log_Message(&aCrC_Firmware[0], sFTP.Count_PacketFirm_ExFlash, 10) != 1)   //Neu luu bi loi
                {
                    sFTP.Update_success = FALSE;
                    sFTP.Receiver_u8    = SEND_UPDATE_FIRMWARE_FAIL_FLASH;
                    return;
                }
                //kiem tra CRC o cuoi cung file .Bin cua Firm: bang cach tinh lai crc cua tat ca cac CRC phia tren. Roi so sanh voi byte cuoi cung
                if(sFTP.LastCrcFile == sFTP.LasCrcCal)
                {
                    sFTP.Update_success = TRUE;
                    sFTP.Receiver_u8 = SEND_UPDATE_FIRMWARE_OK;
                    Erase_Firmware(ADDR_FLAG_HAVE_NEW_FW, 1);
                    //ghi Flag update va Size firm vao Inflash
                    HAL_FLASH_Unlock();
                    osDelay(10);
                    HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,ADDR_FLAG_HAVE_NEW_FW,0xAA);
                    HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,ADDR_FLAG_HAVE_NEW_FW+0x08, sFTP.Count_Data_u32);
                    osDelay(10);
                    HAL_FLASH_Lock();
                }else
                {
                    sFTP.Update_success = FALSE;
                    sFTP.Receiver_u8 = SEND_UPDATE_FIRMWARE_FAIL_LENGTH;
                }
            }
            //Bao fail neu Count  > Length thuc te cua firm
            if (sFTP.Count_Data_u32 > sFTP.FirmWare_Length)
            {
                sFTP.Update_success = FALSE;
                sFTP.Receiver_u8 = SEND_UPDATE_FIRMWARE_FAIL_LENGTH;
            }
        }else
        {
            sFTP.Update_success = FALSE;
			sFTP.Receiver_u8 = SEND_UPDATE_FIRMWARE_FAIL_TIME;
        }
    }else
    {
        sFTP.Update_success = FALSE;
        sFTP.Receiver_u8 = SEND_UPDATE_FIRMWARE_FAIL_SETUP;
    }
}


/*
 * Led Do : Nhay khi co Meter ID va Cong to co Phan hoi lai Lenh
 * Led vang : nhay khi co ket noi MQTT len server
 * LED Xanh : co ket noi den SIM
 */
void Togle_LED(void)
{
    if ((sDCU.Status_Meter_u8 == 1) || (sDCU.Status_Meter_u8 == 2))
        HAL_GPIO_TogglePin(LED_RED_port,LED_RED_pin);          // Doc Cong to bt
    else if (((sDCU.Status_Meter_u8 == 0) || (sDCU.Status_Meter_u8 == 3)) && (Check_HardPin_DectectMeter() == 0))  //k có cap
        HAL_GPIO_WritePin(LED_RED_port,LED_RED_pin,LED_OFF);    // Ko co Cong to
    else 
        HAL_GPIO_WritePin(LED_RED_port,LED_RED_pin,LED_ON);     // Doc sai cong to

// Thong bao co SIM
    if (sSim900_status.Sim_inserted_u8 == 1) 
        HAL_GPIO_TogglePin(LED_WHITE_port,LED_WHITE_pin);
    else  HAL_GPIO_WritePin(LED_WHITE_port,LED_WHITE_pin,LED_OFF); // Ko co Sim

    if(sSim900_status.Status_Connect_u8 == 1)
      HAL_GPIO_TogglePin(LED_BLUE_port,LED_BLUE_pin);
    else
      HAL_GPIO_WritePin(LED_BLUE_port,LED_BLUE_pin,LED_OFF);
}


void _fControlLed(void)
{
    if((sSim900_status.Step_Control_u8 != GSM_FTP_TRANSFER_DOWNLOAD) && (sSim900_status.Step_Control_u8 != GSM_HTTP_TRANSFER_DOWNLOAD))  
    {
        if(sDCU.Power_Status_u8 == 1)  //Mat dien
        {
//            if (Check_Time_Out(sDCU.LandMark_ControlLed_Status,100) == 1) 
//            {
//                HAL_GPIO_TogglePin(LED_YELLOW_port,LED_YELLOW_pin);
//                sDCU.LandMark_ControlLed_Status = RT_Count_Systick_u32;
//            }
            
            if (Check_Time_Out(sDCU.LandMark_ControlLed_Status,50) == 1) 
            {
                HAL_GPIO_WritePin(LED_YELLOW_port,LED_YELLOW_pin, GPIO_PIN_SET);
                if (Check_Time_Out(sDCU.LandMark_ControlLed_Status,1950) == 1)
                {
                    HAL_GPIO_WritePin(LED_YELLOW_port,LED_YELLOW_pin, GPIO_PIN_RESET);
                    sDCU.LandMark_ControlLed_Status = RT_Count_Systick_u32;
                }
            }
        }else
        {
            if (Check_Time_Out(sDCU.LandMark_ControlLed_Status,500) == 1) 
            {
                HAL_GPIO_TogglePin(LED_YELLOW_port,LED_YELLOW_pin);
                sDCU.LandMark_ControlLed_Status = RT_Count_Systick_u32;
            }
        }
        //
        if (Check_Time_Out(sDCU.LandMark_ControlLed,500) == 1) 
        {
            Togle_LED();
            sDCU.LandMark_ControlLed = RT_Count_Systick_u32;
        }
    }else
    {
        if(sDCU.Power_Status_u8 == 1)  //Mat dien
        {
            if (Check_Time_Out(sDCU.LandMark_ControlLed_Status,50) == 1) 
            {
                HAL_GPIO_WritePin(LED_YELLOW_port,LED_YELLOW_pin, GPIO_PIN_SET);
                if (Check_Time_Out(sDCU.LandMark_ControlLed_Status,1950) == 1)
                {
                    HAL_GPIO_WritePin(LED_YELLOW_port,LED_YELLOW_pin, GPIO_PIN_RESET);
                    sDCU.LandMark_ControlLed_Status = RT_Count_Systick_u32;
                }
            }
        }else
        {
            if (Check_Time_Out(sDCU.LandMark_ControlLed_Status,500) == 1) 
            {
                HAL_GPIO_TogglePin(LED_YELLOW_port,LED_YELLOW_pin);
                sDCU.LandMark_ControlLed_Status = RT_Count_Systick_u32;
            }
        }
            
        if (Check_Time_Out(sDCU.LandMark_ControlLed,500) == 1) 
        {
            if ((sDCU.Status_Meter_u8 == 1) || (sDCU.Status_Meter_u8 == 2))
                HAL_GPIO_TogglePin(LED_RED_port,LED_RED_pin);          // Doc Cong to bt
            else if (((sDCU.Status_Meter_u8 == 0) || (sDCU.Status_Meter_u8 == 3)) && (Check_HardPin_DectectMeter() == 0))  //k có cap
                HAL_GPIO_WritePin(LED_RED_port,LED_RED_pin,LED_OFF);    // Ko co Cong to
            else 
                HAL_GPIO_WritePin(LED_RED_port,LED_RED_pin,LED_ON);     // Doc sai cong to
            
            sDCU.LandMark_ControlLed = RT_Count_Systick_u32;
        }
        
        if (Check_Time_Out(sDCU.LandMark_ControlLed_2,100) == 1) 
        {
            HAL_GPIO_TogglePin(LED_WHITE_port,LED_WHITE_pin);
            HAL_GPIO_TogglePin(LED_BLUE_port,LED_BLUE_pin);
            
            sDCU.LandMark_ControlLed_2 = RT_Count_Systick_u32;
        }
    }
}

void _f_TCP_SEND_SERVER (uint8_t* data, uint16_t length, uint8_t Max_Resend, uint32_t Max_Time_Delay)
{
    if(SendDataTCP_IP(data, length, Max_Resend, Max_Time_Delay) == 1)
    {
        sTCP.SendOk_u8 = 1;
        sSim900_status.TimeoutSendHeartBeat_u32 = RT_Count_Systick_u32;
    }else
    {
        _fPackStringToLog ((uint8_t*) "Send Data Fail\r\n", 16);
        //Gui buff uart gan nhat
        if(sLogDCU.LogUart.Length_u16 > 100)
            sLogDCU.LogUart.Length_u16 = 100;
        _fPackStringToLog (sLogDCU.LogUart.Data_a8, sLogDCU.LogUart.Length_u16);
        //
        sTCP.SendOk_u8 = 0;
        sSim900_status.Step_Control_u8 = GSM_CLOSING;   //server test thi phai de sendok == 0. comment Step = GSM_CLOSING
    }
}

uint8_t SendDataTCP_IP(uint8_t* data, uint16_t length, uint8_t Max_Resend, uint32_t Max_Time_Delay)
{
    uint8_t Result = FALSE;
    
    if(length >= MAX_LENGTH_DATA_TCP)
        length = MAX_LENGTH_DATA_TCP;

    sSim900_status.No_Process_i16 = _AT_TCP_SEND;
    sTCP.Count_Send_u8 = 0;
    sTCP.KindFB = 0;
      
    while (sTCP.Count_Send_u8 < Max_Resend)
    {
        sTCP.Count_Send_u8++;
        _fPrint_Via_Debug(&UART_SERIAL, data, length,2000);
        osDelay(5);
        HAL_UART_Transmit(&UART_SIM, data, length,2000);
        sTCP.Landmark_Send_Mess = RT_Count_Systick_u32;
        
        while ((sTCP.KindFB != _AT_TCP_SEND) && (Max_Resend != 0xFF))
		{ 
			if (Check_Time_Out(sTCP.Landmark_Send_Mess, Max_Time_Delay)) 
              break;
			osDelay(50);
		}
        
		if ((sTCP.KindFB == _AT_TCP_SEND) || (Max_Resend == 0xFF)) 
			return TRUE;
    }
    
    return Result;
}

void GetHandshakeData(void)
{
        uint16_t i = 0;

        sHandsake.Data_a8 = &aBuffHansdshake[0];
        Reset_Buff(&sHandsake);
        sHandsake.Length_u16=0;
        
        //HCM HES
        aBuffHansdshake[0] = '#'; // Start
        sHandsake.Length_u16++;
        //firm ware version
        for(i=0;i<sFirmware_Version.Length_u16;i++) 
            aBuffHansdshake[sHandsake.Length_u16++] = *(sFirmware_Version.Data_a8+i);

        aBuffHansdshake[sHandsake.Length_u16++] = '#';
        // IMEI Moderm
        for(i=0;i<sDCU.sDCU_id.Length_u16;i++) 
            aBuffHansdshake[sHandsake.Length_u16++] = *(sDCU.sDCU_id.Data_a8+i);
        aBuffHansdshake[sHandsake.Length_u16++] = '#';
        // Meter ID
        for(i=0;i<sDCU.sMeter_id_now.Length_u16;i++) 
            aBuffHansdshake[sHandsake.Length_u16++] = *(sDCU.sMeter_id_now.Data_a8+i);
    
        aBuffHansdshake[sHandsake.Length_u16++] = '#';
        //SIM IP
        for(i=0;i<sDCU.Str_IP_Module.Length_u16;i++) 
            aBuffHansdshake[sHandsake.Length_u16++] = *(sDCU.Str_IP_Module.Data_a8+i);

        aBuffHansdshake[sHandsake.Length_u16++] = '#';
        //batery level
        Pack_HEXData_Frame_Uint64(&sHandsake, (uint64_t) sDCU.BatLevel_Per, 0);
        
        aBuffHansdshake[sHandsake.Length_u16++] = '%';
        //CSQ
        aBuffHansdshake[sHandsake.Length_u16++] = '+'; 
        aBuffHansdshake[sHandsake.Length_u16++] = 'C';
        aBuffHansdshake[sHandsake.Length_u16++] = 'S';
        aBuffHansdshake[sHandsake.Length_u16++] = 'Q';
        aBuffHansdshake[sHandsake.Length_u16++] = ':';
        aBuffHansdshake[sHandsake.Length_u16++] = ' ';
        aBuffHansdshake[sHandsake.Length_u16++] = '-';
        if(sSim900_status.RSSI_c8 >= 100)
            aBuffHansdshake[sHandsake.Length_u16++] = (sSim900_status.RSSI_c8/100) + 0x30;
        aBuffHansdshake[sHandsake.Length_u16++] = ((sSim900_status.RSSI_c8 / 10) %10) + 0x30;
        aBuffHansdshake[sHandsake.Length_u16++] = (sSim900_status.RSSI_c8 %10) + 0x30;
        
        //Meter type
        aBuffHansdshake[sHandsake.Length_u16++] = '+'; 
        Copy_String_2(&sHandsake, &Str_MeterType_u8[sDCU.MeterType]);  
        
        aBuffHansdshake[sHandsake.Length_u16++] = '#';// Stop
        
//        //EVN
//        aBuffHansdshake[0] = '#'; // Start
//        sHandsake.Length_u16++;
//        //firm ware version
//        for(i=0;i<sFirmware_Version.Length_u16;i++) 
//            aBuffHansdshake[sHandsake.Length_u16++] = *(sFirmware_Version.Data_a8+i);
//
//        aBuffHansdshake[sHandsake.Length_u16++] = '#';
//        // IMEI Moderm
//        for(i=0;i<sDCU.sDCU_id.Length_u16;i++) 
//            aBuffHansdshake[sHandsake.Length_u16++] = *(sDCU.sDCU_id.Data_a8+i);
//        //CSQ
//        aBuffHansdshake[sHandsake.Length_u16++] = '+'; 
//        aBuffHansdshake[sHandsake.Length_u16++] = 'C';
//        aBuffHansdshake[sHandsake.Length_u16++] = 'S';
//        aBuffHansdshake[sHandsake.Length_u16++] = 'Q';
//        aBuffHansdshake[sHandsake.Length_u16++] = ':';
//        aBuffHansdshake[sHandsake.Length_u16++] = ' ';
//        aBuffHansdshake[sHandsake.Length_u16++] = (sSim900_status.RSSI_c8>>4) + 0x30;;
//        aBuffHansdshake[sHandsake.Length_u16++] = (sSim900_status.RSSI_c8&0x0F) + 0x30;;
//        
//        //Meter type
//        aBuffHansdshake[sHandsake.Length_u16++] = '+'; 
//        Copy_String_2(&sHandsake, &Str_MeterType_u8[sDCU.MeterType]);  
//        aBuffHansdshake[sHandsake.Length_u16++] = '#';// Stop
}

void SendDatatoSerial(uint8_t *data, uint8_t length)
{
      if(Meter_Type == 1)
      {
        RS485_SEND;
        HAL_Delay(10);    
      }
      HAL_UART_Transmit(&UART_METER,data,length,1000);
      if(Meter_Type == 1)
        RS485_RECIEVE;
}

uint8_t CheckTimeAutoConnect(void)
{
    uint16_t NumberSecond = 0;
    if(sRTC.hour>=sInformation.DisconnectTime.Hours){
      if(sRTC.min>=sInformation.DisconnectTime.Minutes){
        if(sRTC.sec >=sInformation.DisconnectTime.Seconds){
          return 0; // ngat ket noi
        }
      }
    }
    else
    {
        if(sRTC.hour>=sInformation.ConnectTime.Hours){
          if(sRTC.min>=sInformation.ConnectTime.Minutes){
            if(sRTC.sec >=sInformation.ConnectTime.Seconds){
                NumberSecond = (sInformation.ConnectTime.Hours+sInformation.ConnectPeriod.Hours)*3600+\
                                (sInformation.ConnectTime.Minutes+sInformation.ConnectPeriod.Minutes)*60+\
                                  sInformation.ConnectTime.Seconds+sInformation.ConnectPeriod.Seconds;
                if(sRTC.hour*3600+sRTC.min*60+ sRTC.sec < NumberSecond)
                    return 1; // thuc hien ket noi
            }
          }
        }
    }
    return 0; // ngat ket noi
}
void InitATCommandBuffData(void)
{
    sInformation.Device_Name.Data_a8=Device_Name_Buff;
	sInformation.Device_IP.Data_a8=Device_IP;
    sInformation.Ping_IP.Data_a8=Ping_IP; 
	sInformation.UART_Config.Data_a8=UART_Config;
	sInformation.APN.Data_a8=APN;
	sInformation.APN_Dial_String.Data_a8=APN_Dial_String;
	sInformation.APN_Username.Data_a8=APN_Username;
	sInformation.APN_Password.Data_a8=APN_Password;
}
   

/*
 * 		REQUEST METER
 */
void Request_Meter(void)
{
	Flag_Request_Queue_struct   qFlag_Request;
	uint8_t                     var;
	uint8_t	                    Check_Billing[8];
    uint32_t                    SecondVal = 0; 
    ST_TIME_FORMAT              sTimeTemp;
	
	if ((sDCU.Mode_Connect_Now != MODE_CONNECT_FTP) && (sDCU.Status_Meter_u8 != 0) && (sDCU.Mode_Connect_Now != MODE_CONNECT_HTTP)) 
	{
	/*
	 * 	Meter Message
	 */
		if (sRTC.sec <= 10) 
		{ 
			if ((sRTC.hour == 0) && (sRTC.min == (15 + sTCP.Min_Allow_Send_DATA_u8)))    //cu 1 ngay doc 1 lan
			{
                sInformation.IndexStartBill = 1;
                sInformation.IndexEndBill = 1;
				sStatus_Meter.Flag[DATA_HISTORICAL] = 0; // Historical
			}
            
			if ((sRTC.hour > 0) && (sRTC.min == (15 + sTCP.Min_Allow_Send_DATA_u8)))
			{
				Flash_S25FL_BufferRead(Check_Billing, ADDR_SENT_BILLING_MARK+(sRTC.month-1)*S25FL_PAGE_SIZE,8);
				if(Check_Billing[0] == 0)
                {
                    sInformation.IndexStartBill = 1;
                    sInformation.IndexEndBill = 1;
					sStatus_Meter.Flag[DATA_HISTORICAL] = 0; // Historical
                }
			}
			//lpf			
			if ((sRTC.hour == 0) && (sRTC.min == 32)) 
            {
                sInformation.IndexStartLpf = 2;   //luc 0h. doc record ngay truoc
                sInformation.IndexEndLpf = 2;
 
                if((Read_Meter_LProf_Day[0] == 0)||(Read_Meter_LProf_Day[1] == 0)||(Read_Meter_LProf_Day[2] == 0))
				{
                    SecondVal = HW_RTC_GetCalendarValue_Second(sRTC, 1);
                    SecondVal -= 86400;     //tru di 1 ngay
                    Epoch_to_date_time(&sTimeTemp, SecondVal, 1);
                      
					Read_Meter_LProf_Day[0] = sTimeTemp.date;
					Read_Meter_LProf_Day[1] = sTimeTemp.month;
					Read_Meter_LProf_Day[2] = sTimeTemp.year;
                    
                    Read_Meter_LProf_Day[3] = sTimeTemp.day;
                    Read_Meter_LProf_Day[4] = sTimeTemp.hour;
                    Read_Meter_LProf_Day[5] = sTimeTemp.min;
                    Read_Meter_LProf_Day[6] = sTimeTemp.sec;
				}
                
                sStatus_Meter.Flag[DATA_LOAD_PROFILE] = 0;
                sTCP.Landmark_Allow_Send_LPF_u32 = RT_Count_Systick_u32;   //Reset lai moc thoi gian gui lpf
			}
   
//            if ((sRTC.min %10) == 0)
			if ((sRTC.min == 0) || (sRTC.min == 30)) 
            {
				sStatus_Meter.Flag[DATA_OPERATION] = 0;
                sTCP.Landmark_Allow_Send_DATA_u32 = RT_Count_Systick_u32;       // reset Landmark Send data delay
                if (sLogDCU.IsLogParamImport == 0)
                {
                    sLogDCU.IsLogParamImport = 1;
                    _fLogDCU_ParaImportant ();
                }
			}
            
			if ((sRTC.min == 59) && ((sRTC.hour == 23) || (sRTC.hour == 5) || (sRTC.hour == 11) || (sRTC.hour == 17)))
			{
				// Get Time for Load profile , chua xet truong hop mat dien trong khoang nay
				Read_Meter_LProf_Day[0] = sRTC.date;
				Read_Meter_LProf_Day[1] = sRTC.month;
				Read_Meter_LProf_Day[2] = sRTC.year;
                
                Read_Meter_LProf_Day[3] = sRTC.day;
                Read_Meter_LProf_Day[4] = sRTC.hour;
                Read_Meter_LProf_Day[5] = sRTC.min;
                Read_Meter_LProf_Day[6] = sRTC.sec;
			}
		}
        
		// Call function to read Meter
		if ((sRTC.sec >= 11) && (sRTC.sec <= 21))  
		{
            sLogDCU.IsLogParamImport = 0;  //Cho phep log Param import vao chu ki sau
			for (var = DATA_OPERATION; var <= DATA_METER_INFOR; ++var) 
			{
				if (sStatus_Meter.Flag[var] == 0) 
				{ // send queue
					sStatus_Meter.Flag[var] = 3;
					sStatus_Meter.Landmark_Flag[var] = RT_Count_Systick_u32;
					qFlag_Request.Mess_Status_ui8 	= 0;
					qFlag_Request.Mess_add_u32 		= 0;
					qFlag_Request.Mess_Type_u8 		= var;
					xQueueSend(qSIM_MeterHandle,&qFlag_Request,100);					
				}
				if ((sStatus_Meter.Flag[var] == 3)
						&& (Cal_Time_s(sStatus_Meter.Landmark_Flag[var], RT_Count_Systick_u32) >= 100)) // resend queue
				{
					sStatus_Meter.Flag[var] = 0;    // Moi Code Read_Meter.c Deu phai add = 1 khi chay xong
				}
			}
		}
	}
}



//bat dau tu hour
void Convert_Time_GMT (ST_TIME_FORMAT* sRTC_Check, uint8_t GMT)
{      
    sRTC_Check->hour = sRTC_Check->hour + GMT;
      
    if((sRTC_Check->hour >= 24) && (sRTC_Check->hour < 48))
    {
       sRTC_Check->hour = sRTC_Check->hour%24;
       //công ngay them 1
       if((sRTC_Check->year %4) == 0)
       {
             sRTC_Check->date += 1;
             if(sRTC_Check->date > DaysInMonthLeapYear[sRTC_Check->month - 1])
             {
                sRTC_Check->date = 1;
                sRTC_Check->month += 1;
                //
                if(sRTC_Check->month > 12)
                {
                    sRTC_Check->month = 1;
                    sRTC_Check->year  = (sRTC_Check->year + 1) %100;
                }
             }   
       }else
       {
             sRTC_Check->date += 1;
             if(sRTC_Check->date > DaysInMonth[sRTC_Check->month - 1])
             {
                sRTC_Check->date = 1;
                sRTC_Check->month += 1;
                //
                if(sRTC_Check->month > 12)
                {
                    sRTC_Check->month = 1;
                    sRTC_Check->year  = (sRTC_Check->year + 1) %100;
                }
             } 
       }
    }  
}


uint16_t	MInfo_Mess_Length=0;
uint8_t     aTempPacket1[MInfoBuffLength];
uint16_t	Length_Packet1 = 0;

uint8_t _f_Check_NewMessInfor_Inflash (void)
{
  uint8_t		Read_B_Buff[2] = {0};
    uint8_t     Result = 0;
    uint32_t    PosMessToCheck = 0;
    uint16_t    i = 0;
    
    //Send Meter info mess to SIM900   
    if(Manage_Flash.Error_ui8 == 0)
    {
        switch (Manage_Flash.Step_ui8)
        {
            case 0:
                //Check corresponding position in buffer A
                if ((sSim900_status.Step_Control_u8 == GSM_ATCOMMAND) && (sSim900_status.CheckConnectATCommand == AT_CMD_WAIT)) 
                {
                    // Tao thoi gian tre de doc Flash
                    if (Check_Time_Out(Manage_Flash.TimeOut_Check_Flash, DUTY_CHECK_FLASH) == 1) // 5s
                    {
                        Manage_Flash.TimeOut_Check_Flash= RT_Count_Systick_u32;
                        
                        Flash_S25FL_BufferRead(&Read_B_Buff[0], pos_MeterInfor_Sent_u32, 2);
                        if (Read_B_Buff[0] == 0xFF) // Chua gui ban tin
                        {
                            //Check corresponding position in buffer A
                            Flash_S25FL_BufferRead(&ReadMeterInfoFromA[0], pos_MeterInfor_Sent_u32 - ADDR_OFFSET_METER_INFO, MInfoBuffLength);
                            if ((ReadMeterInfoFromA[0] != 0xFF)||(ReadMeterInfoFromA[1] != 0xFF))
                            {
                                MInfo_Mess_Length = (uint16_t) ReadMeterInfoFromA[0];
                                MInfo_Mess_Length = (uint16_t) (MInfo_Mess_Length<<8)|ReadMeterInfoFromA[1];
                                
                                if (MInfo_Mess_Length > MInfoBuffLength)
                                    MInfo_Mess_Length = MInfoBuffLength;
                                //If have data to send
                                if (_fCheckCrcPacket(&ReadMeterInfoFromA[0], MInfo_Mess_Length) == 1)
                                {          
                                    Queue_Flash_MInfo.WaitACK           = NOMAL_ACK;   //Neu ban tin ghep thi co waitACK = 2. de retry 1 lan
                                    //Check xem có phai ban tin cat ra khong/ Check kis tu ETX o byte thu 1199. Vi dang cat 1200 byte
                                    if ((MInfo_Mess_Length == (MAX_BYTE_IN_PACKET + 3)) && (ReadMeterInfoFromA[MInfo_Mess_Length-3] != 0x03))   //ETX BBC CRC
                                    {          
                                        //Copy buff nhan duoc sang buff temp. De send sang queue
                                        for (i = 0; i < MInfo_Mess_Length; i++)
                                            aTempPacket1[i] = ReadMeterInfoFromA[i];
                                        
                                        Length_Packet1 = MInfo_Mess_Length;
                                        //Doc tiep ban tin phan 2 cua cat. check xem dung k. Neu dung thi gui ca 2. sai thi se luu ca 2 vao phan B
                                        PosMessToCheck = _fJumToNextMessInFlash(pos_MeterInfor_Sent_u32, METER_LOG_MESSAGE_SIZE, ADDR_METER_B_STOP, ADDR_METER_B_START);
                                        Flash_S25FL_BufferRead(&ReadMeterInfoFromA[0], PosMessToCheck - ADDR_OFFSET_METER_INFO, MInfoBuffLength);
                                        if ((ReadMeterInfoFromA[0] != 0xFF)||(ReadMeterInfoFromA[1] != 0xFF))
                                        {
                                            MInfo_Mess_Length = (uint16_t)ReadMeterInfoFromA[0];
                                            MInfo_Mess_Length = (uint16_t) (MInfo_Mess_Length<<8)|ReadMeterInfoFromA[1]; 
                                            if(MInfo_Mess_Length > MInfoBuffLength)
                                                MInfo_Mess_Length = MInfoBuffLength;
                                            //Kiem tra goi tin 2 co dung data khong va crc khong
                                            if (_fCheckCrcPacket(&ReadMeterInfoFromA[0], MInfo_Mess_Length) != 1)
                                            {
                                                _fLog_PacketTSVH_ToSectorB(1); 
                                                return 1;  
                                            }
                                            //Gui ban tin phan 1 sang queue truoc. Khong doi ACK 
                                            Queue_Flash_MInfo_2.WaitACK           = NONE_ACK;  
                                            Queue_Flash_MInfo_2.Mess_Direct_ui8   = 1;
                                            Queue_Flash_MInfo_2.str_Flash.Length_u16 = Length_Packet1 - 3;     //tru di 2 byte length va 1 byte crc
                                            Queue_Flash_MInfo_2.Mess_Status_ui8   = 0;
                                            Queue_Flash_MInfo_2.Mess_Type_ui8     = DATA_PRE_OPERA;
                                            Queue_Flash_MInfo_2.str_Flash.Data_a8 = &aTempPacket1[2];  //
                                            
                                            xQueueSend(qFlash_SIM900Handle,(void *)&ptrQueue_Flash_MInfo_2, 100);
                                            Result  = 1;  
                                            sDCU.TypeMessTSVH = 1;
                                            Queue_Flash_MInfo.WaitACK           = ONCE_ACK; 
                                        } else
                                        {
                                            _fLog_PacketTSVH_ToSectorB(1); 
                                            return 1; 
                                        }
                                    } else if(ReadMeterInfoFromA[MInfo_Mess_Length-3] != 0x03)
                                    {
                                        Queue_Flash_MInfo.WaitACK           = NONE_ACK;
                                        sDCU.TypeMessTSVH = 2;
                                    } else
                                        sDCU.TypeMessTSVH = 2;  //Danh dau loai ban tin TSVH
                                    
                                    //khong phai ban tin cat ra thi se gui ban tin do sang queue thoi
                                    Queue_Flash_MInfo.Mess_Direct_ui8   = 1;
                                    Queue_Flash_MInfo.str_Flash.Length_u16 = MInfo_Mess_Length - 3;   //tru di 2 byte length va 1 byte crc
                                    Queue_Flash_MInfo.Mess_Status_ui8   = 0;
                                    Queue_Flash_MInfo.Mess_Type_ui8     = DATA_OPERATION;
                                    Queue_Flash_MInfo.str_Flash.Data_a8 = &ReadMeterInfoFromA[2];
                                    
                                    xQueueSend(qFlash_SIM900Handle,(void *)&ptrQueue_Flash_MInfo, 100);
                                    // Xac nhan thoi gian day data vao Queue
                                    Manage_Flash.Step_ui8 = 1;
                                    Manage_Flash.Time_outFB_MQTT = RT_Count_Systick_u32; // Reset Time FB MQTT
                                } else
                                    _fLog_PacketTSVH_ToSectorB(2); 
                            }
                        }
                    }
                }	
                break;
            case 1:
                if ((Queue_Flash_MInfo.Mess_Status_ui8 == 1) || (sTCP.aCountRetryMess[DATA_OPERATION] >= MAX_RETRY_SEND_1MESS))   //gia tri bien này lay thong qua con tro:sFlash_Update và sMQTT.paNeed_Send[var] = sFlash_Update;
                {
                    //Save mess into flash
                    if (Manage_Flash.BuffA_Change_Sector_ui8 == 0) // ???
                        _fLog_PacketTSVH_ToSectorB(sDCU.TypeMessTSVH);
                    
                    Manage_Flash.BuffA_Change_Sector_ui8 = 0;
                    Manage_Flash.Step_ui8 = 0;
                    sTCP.aCountRetryMess[DATA_OPERATION] = 0;
                    break;
                }
                // Neu qua thoi gian khong co FB tu Server -> day lai data vao Queue do co the Queue bi loi
                if (Check_Time_Out(Manage_Flash.Time_outFB_MQTT,120000) == TRUE )
                    Manage_Flash.Step_ui8 = 0; //  Quay lai day data vo Queue
                break;
            default:
                Manage_Flash.Step_ui8 = 0;
                break;
        }
    }
    
    return Result;
}

uint8_t _fCheckCrcPacket (uint8_t *Buff, uint16_t Length)
{
    uint8_t     Result = 0;
    uint8_t		ChecksumByte=0;
    uint16_t    i = 0;
    
    //Kiem tra goi tin 2 co dung data khong va crc khong
    ChecksumByte = 0;
    for (i = 0; i < Length-1; i++)
        ChecksumByte += Buff[i];
    
    if (ChecksumByte == Buff[Length-1])
        Result = 1;
    
    return Result;
}


void _fLog_PacketTSVH_ToSectorB (uint8_t Type)
{
    if(Type == 1)     //luu gói 1 khi cat ra vao
    {
        Queue_Flash_MInfo.str_Flash.Length_u16 = Length_Packet1;
        Queue_Flash_MInfo.str_Flash.Data_a8 = &aTempPacket1[0];
        if(Flash_S25FL_Log_Message(ptrQueue_Flash_MInfo->str_Flash.Data_a8, ptrQueue_Flash_MInfo->str_Flash.Length_u16, 2) != 1)
            Manage_Flash.Error_ui8 = 1;
    }
    
    Queue_Flash_MInfo.str_Flash.Length_u16 = MInfo_Mess_Length;
    Queue_Flash_MInfo.str_Flash.Data_a8 = &ReadMeterInfoFromA[0];
    if(Flash_S25FL_Log_Message(ptrQueue_Flash_MInfo->str_Flash.Data_a8, ptrQueue_Flash_MInfo->str_Flash.Length_u16, 2) != 1)
        Manage_Flash.Error_ui8 = 1;
}

 

uint16_t	MBill_Mess_Length=0;
uint8_t     aTempDataBillCut1[MBillingBuffFLashLength];
uint16_t	LenTempPack1 = 0;
uint8_t     TypeBillCut = 0;

void _f_Check_NewMessBill_Inflash (void)
{
    uint8_t		Read_B_Buff[2] = {0};
    uint32_t    PosMessToCheck = 0;
    uint16_t	i=0;
    
    //Send Meter Lpf mess to SIM900
    if(Manage_Flash.Error_ui8 == 0)
    {
        switch (Manage_Flash_Bill.Step_ui8)
        {
            case 0:
                //Check corresponding position in buffer A
                if((sSim900_status.Step_Control_u8 == GSM_ATCOMMAND) && (sSim900_status.CheckConnectATCommand == AT_CMD_WAIT)) 
                {
                    // Tao thoi gian tre de doc Flash
                    if(Check_Time_Out(Manage_Flash_Bill.TimeOut_Check_Flash, DUTY_CHECK_FLASH) == 1) // 20s
                    {
                        Manage_Flash_Bill.TimeOut_Check_Flash= RT_Count_Systick_u32;
                        Flash_S25FL_BufferRead(&Read_B_Buff[0], pos_MeterBill_Sent_u32, 2);
                        if (Read_B_Buff[0] == 0xFF) // Chua gui ban tin
                        {
                            //Check corresponding position in buffer A
                            Flash_S25FL_BufferRead(&ReadMeterBillFromA[0], pos_MeterBill_Sent_u32 - ADDR_OFFSET_METER_BILL, MBillingBuffFLashLength);

                            //If have data to send
                            if ((ReadMeterBillFromA[0] != 0xFF)||(ReadMeterBillFromA[1] != 0xFF))
                            {
                                MBill_Mess_Length = (uint16_t)ReadMeterBillFromA[0];
                                MBill_Mess_Length = (MBill_Mess_Length<<8)|(uint16_t)ReadMeterBillFromA[1];
    
                                if(MBill_Mess_Length > MBillingBuffFLashLength)
                                    MBill_Mess_Length = MBillingBuffFLashLength;
                                
                                //If have data to send
                                if(_fCheckCrcPacket(&ReadMeterBillFromA[0], MBill_Mess_Length) == 1)
                                {          
                                    Queue_Flash_MBill.WaitACK           = NOMAL_ACK;   //Neu ban tin ghep thi co waitACK = 2. de retry 1 lan
                                    //Check xem có phai ban tin cat ra khong/ Check kis tu ETX o byte thu 1199. Vi dang cat 1200 byte
                                    if((MBill_Mess_Length == (MAX_BYTE_IN_PACKET + 3)) && (ReadMeterBillFromA [MBill_Mess_Length-3] != 0x03))   //ETX BBC CRC
                                    {          
                                        //Copy buff nhan duoc sang buff temp. De send sang queue
                                        for(i = 0; i < MBill_Mess_Length; i++)
                                            aTempDataBillCut1[i] = ReadMeterBillFromA[i];
                                        
                                        LenTempPack1 = MBill_Mess_Length;
                                        //Doc tiep ban tin phan 2 cua cat. check xem dung k. Neu dung thi gui ca 2. sai thi se luu ca 2 vao phan B  
                                        PosMessToCheck = _fJumToNextMessInFlash(pos_MeterBill_Sent_u32, METER_LOG_MESSAGE_SIZE, ADDR_BILLING_A_STOP, ADDR_BILLING_A_START);
                                        Flash_S25FL_BufferRead(&ReadMeterBillFromA[0], PosMessToCheck - ADDR_OFFSET_METER_BILL, MBillingBuffFLashLength);
                                        if ((ReadMeterBillFromA[0] != 0xFF)||(ReadMeterBillFromA[1] != 0xFF))
                                        {
                                            MBill_Mess_Length = (uint16_t)ReadMeterBillFromA[0];
                                            MBill_Mess_Length = (uint16_t) (MBill_Mess_Length<<8)|ReadMeterBillFromA[1]; 
                                            if(MBill_Mess_Length > MBillingBuffFLashLength)
                                                MBill_Mess_Length = MBillingBuffFLashLength;
                                            //Kiem tra goi tin 2 co dung data khong va crc khong
                                            if(_fCheckCrcPacket(&ReadMeterBillFromA[0], MBill_Mess_Length) != 1)
                                            {
                                                _fLog_PacketBill_ToSectorB(1);   
                                                return;  
                                            }
                                            //Gui ban tin phan 1 sang queue truoc. Khong doi ACK 
                                            Queue_Flash_MBill_2.WaitACK           = NONE_ACK;  
                                            Queue_Flash_MBill_2.Mess_Direct_ui8   = 1;
                                            Queue_Flash_MBill_2.str_Flash.Length_u16 = LenTempPack1 - 3;   
                                            Queue_Flash_MBill_2.Mess_Status_ui8   = 0;
                                            Queue_Flash_MBill_2.Mess_Type_ui8     = DATA_PRE_HISTORICAL;
                                            Queue_Flash_MBill_2.str_Flash.Data_a8 = &aTempDataBillCut1[2];  //
                                            
                                            xQueueSend(qFlash_SIM900Handle,(void *)&ptrQueue_Flash_MBill_2, 100);
                                            
                                            Queue_Flash_MBill.WaitACK           = ONCE_ACK;   //ban tin thu 2 cua ban tin bi cat ra chi cho retry 1 lan
                                            TypeBillCut = 1;
                                        }else
                                        {
                                            _fLog_PacketBill_ToSectorB(1); 
                                            return ; 
                                        }
                                    }else if (ReadMeterBillFromA [MBill_Mess_Length-3] != 0x03)
                                    {
                                        Queue_Flash_MBill.WaitACK           = NONE_ACK; 
                                        TypeBillCut = 2;
                                    }else
                                        TypeBillCut = 2;
                                    
                                    //khong phai ban tin cat ra thi se gui ban tin do sang queue thoi
                                    Queue_Flash_MBill.Mess_Direct_ui8   = 1;
                                    Queue_Flash_MBill.str_Flash.Length_u16 = MBill_Mess_Length - 3;   //
                                    Queue_Flash_MBill.Mess_Status_ui8   = 0;
                                    Queue_Flash_MBill.Mess_Type_ui8     = DATA_HISTORICAL;
                                    Queue_Flash_MBill.str_Flash.Data_a8 = &ReadMeterBillFromA[2];
                                    
                                    xQueueSend(qFlash_SIM900Handle,(void *)&ptrQueue_Flash_MBill, 100);
                                    // Xac nhan thoi gian day data vao Queue
                                    Manage_Flash_Bill.Step_ui8 = 1;
                                    Manage_Flash_Bill.Time_outFB_MQTT = RT_Count_Systick_u32; // Reset Time FB MQTT
                                } else
                                    _fLog_PacketBill_ToSectorB(2); 
                            }
                        }
                    } 
                }	
                break;
            case 1:
                if ((Queue_Flash_MBill.Mess_Status_ui8 == 1) || (sTCP.aCountRetryMess[DATA_HISTORICAL] >= MAX_RETRY_SEND_1MESS))   //gia tri bien này lay thong qua con tro:sFlash_Update và sMQTT.paNeed_Send[var] = sFlash_Update;
                {
                    //Save mess into flash TypeBillCut
                    if (Manage_Flash_Bill.BuffA_Change_Sector_ui8 == 0) 
                        _fLog_PacketBill_ToSectorB (TypeBillCut);
                    
                    Manage_Flash_Bill.BuffA_Change_Sector_ui8 = 0;
                    Manage_Flash_Bill.Step_ui8 = 0;
                    sTCP.aCountRetryMess[DATA_HISTORICAL] = 0;
                    break;
                }
                // Neu qua thoi gian khong co FB tu Server -> day lai data vao Queue do co the Queue bi loi
                if (Check_Time_Out(Manage_Flash_Bill.Time_outFB_MQTT,120000) == TRUE )
                    Manage_Flash_Bill.Step_ui8 = 0; //  Quay lai day data vo Queue
                break;
            default:
                Manage_Flash_Bill.Step_ui8 = 0;
                break;
        }
    }
}


void _fLog_PacketBill_ToSectorB (uint8_t Type)
{
    if(Type == 1)     //luu gói 1 khi cat ra vao
    {
        Queue_Flash_MBill.str_Flash.Length_u16 = LenTempPack1;  
        Queue_Flash_MBill.str_Flash.Data_a8 = &aTempDataBillCut1[0];
        if(Flash_S25FL_Log_Message(ptrQueue_Flash_MBill->str_Flash.Data_a8, ptrQueue_Flash_MBill->str_Flash.Length_u16, 6) != 1)
            Manage_Flash.Error_ui8 = 1;   //De struc manage flash de neu loi flash thi lay bien nay chung
    }
    
    Queue_Flash_MBill.str_Flash.Length_u16 = MBill_Mess_Length;
    Queue_Flash_MBill.str_Flash.Data_a8 = &ReadMeterBillFromA[0];
    if(Flash_S25FL_Log_Message(ptrQueue_Flash_MBill->str_Flash.Data_a8, ptrQueue_Flash_MBill->str_Flash.Length_u16, 6) != 1)
        Manage_Flash.Error_ui8 = 1;
}



uint16_t	MLpf_Mess_Length=0;
void _f_Check_NewMessLpf_Inflash (void)
{
    uint8_t		Read_B_Buff[2] = {0};
    uint8_t		ChecksumByte=0;
    uint16_t    i=0;
    
    //Send Meter Lpf mess to SIM900
    if(Manage_Flash.Error_ui8 == 0)
    {
        switch (Manage_Flash_Lpf.Step_ui8)
        {
            case 0:
                //Check corresponding position in buffer A
                if((sSim900_status.Step_Control_u8 == GSM_ATCOMMAND) && (sSim900_status.CheckConnectATCommand == AT_CMD_WAIT)) 
                {
                    // Tao thoi gian tre de doc Flash
                    if(Check_Time_Out(Manage_Flash_Lpf.TimeOut_Check_Flash, DUTY_CHECK_FLASH) == 1) // 20s moi check flash 1 lan
                    {
                        Manage_Flash_Lpf.TimeOut_Check_Flash= RT_Count_Systick_u32;
                        Flash_S25FL_BufferRead(&Read_B_Buff[0], pos_MeterLPF_Sent_u32, 2);
                        if (Read_B_Buff[0] == 0xFF) // Chua gui ban tin
                        {
                            //Check corresponding position in buffer A
                            Flash_S25FL_BufferRead(&ReadMeterLpfFromA[0], pos_MeterLPF_Sent_u32 - ADDR_OFFSET_METER_LPF, MLProfBuffLength);

                            //If have data to send
                            if ((ReadMeterLpfFromA[0] != 0xFF)||(ReadMeterLpfFromA[1] != 0xFF))
                            {
                                MLpf_Mess_Length = (uint16_t)ReadMeterLpfFromA[0];
                                MLpf_Mess_Length = (uint16_t)(MLpf_Mess_Length<<8) | ReadMeterLpfFromA[1];

                                if(MLpf_Mess_Length > MLProfBuffLength)
                                    MLpf_Mess_Length = MLProfBuffLength;
                                ChecksumByte = 0;
                                for (i=0;i<MLpf_Mess_Length-1;i++)
                                    ChecksumByte += ReadMeterLpfFromA[i];

                                if (ChecksumByte == ReadMeterLpfFromA[MLpf_Mess_Length-1])
                                {
                                    Queue_Flash_MLpf.Mess_Direct_ui8   = 1;
                                    Queue_Flash_MLpf.str_Flash.Length_u16 = MLpf_Mess_Length - 3;
                                    Queue_Flash_MLpf.Mess_Status_ui8   = 0;
                                    Queue_Flash_MLpf.Mess_Type_ui8     = DATA_LOAD_PROFILE;
                                    Queue_Flash_MLpf.str_Flash.Data_a8 = &ReadMeterLpfFromA[2];
                                    Queue_Flash_MLpf.WaitACK           =  NOMAL_ACK;
                                    
                                    xQueueSend(qFlash_SIM900Handle,(void *)&ptrQueue_Flash_MLpf, 100); 
                                    // Xac nhan thoi gian day data vao Queue
                                    Manage_Flash_Lpf.Step_ui8 = 1;
                                    Manage_Flash_Lpf.Time_outFB_MQTT = RT_Count_Systick_u32; // Reset Time FB MQTT
                                }
                                else
                                {
                                    //Save mess into flash
                                    Queue_Flash_MLpf.str_Flash.Length_u16 = MLpf_Mess_Length;
                                    Queue_Flash_MLpf.str_Flash.Data_a8 = &ReadMeterLpfFromA[0];
                                    if(Flash_S25FL_Log_Message(ptrQueue_Flash_MLpf->str_Flash.Data_a8, ptrQueue_Flash_MLpf->str_Flash.Length_u16, 8) != 1)
                                        Manage_Flash.Error_ui8 = 1;
                                }
                            }
                        }
                    }
                }	
                break;
            case 1:
                if ((Queue_Flash_MLpf.Mess_Status_ui8 == 1) || (sTCP.aCountRetryMess[DATA_LOAD_PROFILE] >= MAX_RETRY_SEND_1MESS))   //gia tri bien này lay thong qua con tro:sFlash_Update và sMQTT.paNeed_Send[var] = sFlash_Update;
                {
                    //Save mess into flash
                    if (Manage_Flash_Lpf.BuffA_Change_Sector_ui8 == 0) 
                    {
                        Queue_Flash_MLpf.str_Flash.Length_u16 = MLpf_Mess_Length;
                        Queue_Flash_MLpf.str_Flash.Data_a8 = &ReadMeterLpfFromA[0];
                        
                        if(Flash_S25FL_Log_Message(ptrQueue_Flash_MLpf->str_Flash.Data_a8, ptrQueue_Flash_MLpf->str_Flash.Length_u16, 8) != 1)
                            Manage_Flash.Error_ui8 = 1;
                    }
                    Manage_Flash_Lpf.BuffA_Change_Sector_ui8 = 0;
                    //Back to step 0 to send next mess
                    Manage_Flash_Lpf.Step_ui8 = 0;
                    sTCP.aCountRetryMess[DATA_LOAD_PROFILE] = 0;
                    break;
                }
                // Neu qua thoi gian khong co FB tu Server -> day lai data vao Queue do co the Queue bi loi
                if (Check_Time_Out(Manage_Flash_Lpf.Time_outFB_MQTT, 120000) == TRUE )
                    Manage_Flash_Lpf.Step_ui8 = 0; //  Quay lai day data vo Queue
                break;
            default:
                Manage_Flash_Lpf.Step_ui8 = 0;
                break;
        }
    }
}



//Co alarm-> check loai alarm -> gui MessType Sang queue sim
Meter_Flash_Queue_Struct	Queue_Alarm_Power,*ptrQueue_Alarm;

uint8_t _f_Send_Alarm_To_Queue (uint8_t Type)
{
    ptrQueue_Alarm = &Queue_Alarm_Power;
      
    switch (Type)     
    {
        case CONNECT_METER:  
            Queue_Alarm_Power.Mess_Type_ui8 = NOTIF_CONNECT_METER;  
            xQueueSend(qSIM_SIM_UpdateHandle,(void *)&ptrQueue_Alarm,100);
            break;
        case DISCONECT_METER:
            Queue_Alarm_Power.Mess_Type_ui8 = NOTIF_DISCONNECT_METER;  
            xQueueSend(qSIM_SIM_UpdateHandle,(void *)&ptrQueue_Alarm,100);
            break;
        case POW_ON_MODERM:
            Queue_Alarm_Power.Mess_Type_ui8 = ALARM_POWER_ON_MODERM;  
            xQueueSend(qSIM_SIM_UpdateHandle,(void *)&ptrQueue_Alarm,100);
            break;
        case POW_OFF_MODERM:
            Queue_Alarm_Power.Mess_Type_ui8 = ALARM_POWER_OFF_MODERM;  
            xQueueSend(qSIM_SIM_UpdateHandle,(void *)&ptrQueue_Alarm,100);
            break;
        case POW_UP_METER:
            Queue_Alarm_Power.Mess_Type_ui8 = ALARM_POWER_UP_METER;    
            xQueueSend(qSIM_SIM_UpdateHandle,(void *)&ptrQueue_Alarm,100);
            break;
        case POW_DOWN_METER:
            Queue_Alarm_Power.Mess_Type_ui8 = ALARM_POWER_DOWN_METER;  
            xQueueSend(qSIM_SIM_UpdateHandle,(void *)&ptrQueue_Alarm,100);
            break;
        default:
            break;
    }
    
    return 1;
}


uint8_t Check_Finish_Ping (truct_String* StrBuff)  //sua lai
{
    int Pos;
    
    truct_String StrEnd = {(uint8_t*) "+MPING:3", 8};
      
    Pos = Find_String_V2(&StrEnd,StrBuff);
    if (Pos >= 0)
        return 1;
    return 0;
}


void _f_TCP_SEND_SERVER_DIRECT (uint8_t* data, uint16_t length)
{
    HAL_UART_Transmit(&UART_SIM, data, length, 2000);
    osDelay(5);
    sSim900_status.TimeoutSendHeartBeat_u32 = RT_Count_Systick_u32;
}




