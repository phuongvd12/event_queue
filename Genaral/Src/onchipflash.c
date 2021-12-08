#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"

#include "variable.h"
#include "onchipflash.h"
#include "myuart.h"
#include "t_mqtt.h"


extern HAL_StatusTypeDef    FLASH_WaitForLastOperation(uint32_t Timeout);
extern void                 FLASH_PageErase(uint32_t Page, uint32_t Banks);
/*
 *	So luong bien truyen vao Phai la Boi cua 8
 */
HAL_StatusTypeDef	OnchipFlashWriteData (uint32_t andress, uint8_t	*data_address, uint32_t data_length)
{
	uint8_t		*temp_data_address;
	uint16_t	i=0;
	uint64_t	writeval=0,writetime=0;
	uint32_t	temp_write_address;
	HAL_StatusTypeDef status = HAL_ERROR;
	
	temp_data_address = data_address;
	temp_write_address = andress;
	
	if ((data_length%8) == 0)
	{
		//Calculate number of word to write
		writetime = data_length/8;
		
		//Unlock flash
		HAL_FLASH_Unlock();  //
		//Wait for last operation to be completed
        FLASH_WaitForLastOperation(1000);
		//Change bytes order then write
		for (i=0;i<writetime;i++)
		{
			writeval = *(__IO uint64_t*)(temp_data_address);
			
			status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,temp_write_address,writeval);
			
			if (status == HAL_ERROR)
				break;
			
			temp_write_address = temp_write_address + 8;
			temp_data_address = temp_data_address + 8;
		}
		
		//Lock flash
		HAL_FLASH_Lock();
	}
		
	return status;
}

HAL_StatusTypeDef	OnchipFlashCopy (uint32_t sourceandress, uint32_t destinationandress, uint32_t data_length)
{
	uint32_t	i=0,writetime=0;
	uint64_t	writeval=0;
	uint32_t	temp_sourceandress,temp_destinationandress;
	HAL_StatusTypeDef status = HAL_ERROR;
	
	temp_sourceandress = sourceandress;
	temp_destinationandress = destinationandress;
	
	if ((data_length%8) == 0)
	{
		//Calculate number of word to write
		writetime = data_length/8;
		
		//Unlock flash
		HAL_FLASH_Unlock();
		//Wait for last operation to be completed
        FLASH_WaitForLastOperation(1000);
		//Change bytes order then write
		for (i=0;i<writetime;i++)
		{
			writeval = *(__IO uint64_t*)(temp_sourceandress);
			
			status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,temp_destinationandress,writeval);
			
			if (status == HAL_ERROR)
				break;
			
			temp_sourceandress = temp_sourceandress + 8;
			temp_destinationandress = temp_destinationandress + 8;
		}
		
		//Lock flash
		HAL_FLASH_Lock();
	}
		
	return status;
}

HAL_StatusTypeDef OnchipFlashPageErase (uint32_t pageaddress)
{
	HAL_StatusTypeDef status = HAL_ERROR;
	
	//Unlock flash
	status = HAL_FLASH_Unlock();
    
	//Wait for last operation to be completed
    FLASH_WaitForLastOperation(1000);
	//If the previous operation is completed, proceed to erase the page
	if (status == HAL_OK)
		FLASH_PageErase((pageaddress & 0x00FFFFFF)/2048, FLASH_BANK_1);
	
	//Wait for last operation to be completed
	status = FLASH_WaitForLastOperation(1000);
	
	//Disable the PER Bit
	CLEAR_BIT(FLASH->CR, FLASH_CR_PER);
	
	HAL_FLASH_Lock();
	
	return status;
}

void Erase_Firmware(uint32_t Add_Flash_update,uint32_t Total_page)
{
	uint32_t var;
	for (var = 0; var < Total_page; var++)
		OnchipFlashPageErase(Add_Flash_update+var*0x800);
}

void OnchipFlashReadData (uint32_t address,uint8_t *destination, uint16_t length_inByte)
{
	uint16_t i=0;
	uint32_t temp_address;
	uint8_t *temp_destination;

	temp_address = address;
	temp_destination = destination;

	for (i=0;i<length_inByte;i++)
	{
		*temp_destination = *(__IO uint8_t*)temp_address;
		temp_destination++;
		temp_address++;
	}
}

void Save_DCU_ID (void)
{
	uint16_t i=0;
	uint8_t temp_arr[40];

	OnchipFlashPageErase(ADDR_DCUID);
	temp_arr[0] = 1;
	temp_arr[1] = sDCU.sDCU_id.Length_u16;
	for (i=0;i<sDCU.sDCU_id.Length_u16;i++)
		temp_arr[i+2] = sDCU.sDCU_id.Data_a8[i];

	OnchipFlashWriteData(ADDR_DCUID,temp_arr,40);
}

void _fSave_Uart_Config (void)
{
    uint16_t i=0;
	uint8_t temp_arr[16];
    
    if(sInformation.UART_Config.Length_u16 == 9)
    {
        temp_arr[0] = 1;
        temp_arr[1] = sInformation.UART_Config.Length_u16;
        
        for(i=0;i<sInformation.UART_Config.Length_u16;i++)
        {
            temp_arr[i+2] = *(sInformation.UART_Config.Data_a8+i);
        }
        
        OnchipFlashPageErase(ADDR_UART_CONFIG);
        OnchipFlashWriteData(ADDR_UART_CONFIG, temp_arr, 16);
    }
}
        
        
void Save_PassModerm (void)
{
	uint16_t i=0;
	uint8_t temp_arr[40];

	OnchipFlashPageErase(ADDR_PASS_MODERM);
	temp_arr[0] = 1;
	temp_arr[1] = sInformation.Password_Moderm.Length_u16;
	for (i=0;i<sInformation.Password_Moderm.Length_u16;i++)
		temp_arr[i+2] = sInformation.Password_Moderm.Data_a8[i];

	OnchipFlashWriteData(ADDR_PASS_MODERM,temp_arr,40);
}


void Save_Array(uint32_t ADD, uint8_t* Buff, uint16_t length)
{
    uint16_t i = 0;
    uint8_t Buff_Temp[40];
    
    OnchipFlashPageErase(ADD);
    Buff_Temp[0] = 1;
    Buff_Temp[1] = length;
    for(i = 0; i < length; i++)
      Buff_Temp[i+2] = *(Buff + i);

    OnchipFlashWriteData(ADD,&Buff_Temp[0],40);
    
}

//======================================================================
//				FTP
//======================================================================
void Set_default_FTP(void)
{
	sFTP.Receiver_u8 					= SEND_UPDATE_FIRMWARE;
	sFTP.Count_Data_u32 				= 0;
	sFTP.Pending_u8 					= 0;
	sFTP.Systick_LandMarkFTP_u32 		= RT_Count_Systick_u32;
	sFTP.Systick_LandMarkFTP_new_u32 	= RT_Count_Systick_u32;
	sFTP.Min_Period_Recei_u32 			= 0xFFFFFFF;
	sFTP.Max_Period_Recei_u32 			= 0;
	sFTP.Add_buff						= ADDR_UPDATE_PROGRAM;
	sFTP.Update_success					= FALSE;
    //
    sFTP.IndexTemp                      = 0;
    sFTP.Offset                         = 0;
    sFTP.Count_PacketFirm_ExFlash       = 0;
    sFTP.LastCrcFile                    = 0;
    sFTP.LasCrcCal                      = 0;
    sFTP.HeaderSV_OK                    = 0;
    pos_FirmWrite_u32                   = ADDR_BASE_FIRM_DCU;
}


void _fFTP_Receiv(void)
{	
    truct_String StrUART_Temp;
    truct_String Begin_Get_Firm ={(uint8_t *) "+MFSREAD: DATA:", 15};  //+MFSREAD: DATA: 10,
    truct_String Begin_Get_Firm_2 ={(uint8_t *) ", ", 2};
    int PosFind = 0;
    
    sFTP.Systick_LandMarkFTP_new_u32 = RT_Count_Systick_u32;
        
    Reset_UART(&UartSIM_Control);
    HAL_UART_Transmit(&UART_SIM,(uint8_t*)"AT+MFSREAD=\"",12,1000);
    HAL_UART_Transmit(&UART_SIM,(uint8_t*) &firmFileName[0], SIZE_OF_FIRMWARE,100);
    HAL_UART_Transmit(&UART_SIM,(uint8_t*)"\",0,0\r",6,1000);
      
	while (sFTP.Receiver_u8 == SEND_UPDATE_FIRMWARE)  
    {
        PosFind = Find_String_V2((truct_String*)&Begin_Get_Firm, &UartSIM_Control.str_Receiv);
        if ( PosFind >= 0)  //CONNECT 54472 (\r\n)
        {
            StrUART_Temp.Data_a8 = UartSIM_Control.str_Receiv.Data_a8 + PosFind;
            StrUART_Temp.Length_u16 = UartSIM_Control.str_Receiv.Length_u16 - PosFind;
            if (Find_String_V2((truct_String*)&Begin_Get_Firm_2, &StrUART_Temp) >= 0)
            {
                sFTP.Count_Data_u32 = 0;
                sFTP.Receiver_u8 = SEND_UPDATE_FIRMWARE_PENDING;
                Reset_UART(&UartSIM_Control);
            }
        }
		if (Check_Time_Out(sFTP.Systick_LandMarkFTP_new_u32,TIME_OUT_FTP) == TRUE)
		{
			sFTP.Receiver_u8 = SEND_UPDATE_FIRMWARE_FAIL_SETUP;
            sTCP.aNeed_Send[UPDATE_FIRM_FAIL] = 1;
		}
	}
}


void _fFTP_Pending(void)
{
    uint8_t BuffTemp[50];
	int32_t Cal_Period = 0;
	uint8_t Count = 0;
	uint8_t	aWrite_flash[5] = {0,0,0,0,0};
    uint8_t Temp = 0;
    uint8_t Flag_Alow_Get = 0;
    uint8_t Check_end = 0;
      
	while (sFTP.Receiver_u8 == SEND_UPDATE_FIRMWARE_PENDING) 
    {
		if (sFTP.Pending_u8 == 1) 
        {	
			sFTP.Count_Data_u32++;	
			Cal_Period = sFTP.Systick_LandMarkFTP_new_u32 - sFTP.Systick_LandMarkFTP_u32;
			if ((Cal_Period > 0) && (sFTP.Min_Period_Recei_u32 > Cal_Period)) 	sFTP.Min_Period_Recei_u32 = Cal_Period;
			if ((Cal_Period < 20) && (sFTP.Max_Period_Recei_u32 < Cal_Period)) 	sFTP.Max_Period_Recei_u32 = Cal_Period;

			sFTP.Pending_u8 = 0;
			sFTP.Systick_LandMarkFTP_u32 = sFTP.Systick_LandMarkFTP_new_u32;
            if(((sFTP.Count_Data_u32 %1024) == 1) && (Check_end == 0) && (sFTP.Count_Data_u32 != 1))
            {
                Check_end = 0;
                Temp ++;   //luu vao buff tai vi tr 1
                BuffTemp[Temp] =  sFTP.Data_buff_u8;
                if((BuffTemp[Temp -1] == 0x2C) && (BuffTemp[Temp] == 0x20))   //", "
                    Check_end = 1;
                sFTP.Count_Data_u32 --;
                Flag_Alow_Get = 0;
            }else 
            {
                Temp = 0;
                Check_end = 0;
                Flag_Alow_Get = 1;
            }
            //
            if(Flag_Alow_Get == 1)
            {
                if(sFTP.Count_Data_u32 <= sFTP.FirmWare_Length)
                {
                    aWrite_flash[Count] = sFTP.Data_buff_u8;
                    Count++;
                    if (Count >= 8) 
                    {
                        Count = 0;
                        
                        if (OnchipFlashWriteData(sFTP.Add_buff,aWrite_flash,8) != HAL_OK) {
                            sFTP.Update_success = FALSE;
                            sFTP.Receiver_u8 = SEND_UPDATE_FIRMWARE_FAIL_FLASH;
                            break;
                        }
                        sFTP.Add_buff+=8;
                    }else  //byte le
                    {
                        if((Count<8)&&(0<Count))
                        {
                            while(Count<8)
                            {
                                aWrite_flash[Count] = 0xFF;
                                Count++;
                                sFTP.FirmWare_Length++;
                                sFTP.Count_Data_u32++;
                            }
                            if (OnchipFlashWriteData(sFTP.Add_buff,aWrite_flash,8) != HAL_OK) {
                                sFTP.Update_success = FALSE;
                                sFTP.Receiver_u8 = SEND_UPDATE_FIRMWARE_FAIL_FLASH;
                                break;
                            }
                        }
                    }
                }
            }
        }
        
         //truong hop Update qua cong COM(sFTP.Count_Data_u32 >= sFTP.FirmWare_Length + 10)   //"\r\n+QFTPGET:30900"
         //truong hop Update qua RAM (sFTP.Count_Data_u32 >= sFTP.FirmWare_Length + 6)   //"\r\nOK\r\n
        
		if (sFTP.Count_Data_u32 >= sFTP.FirmWare_Length + 6)  
		{
            if(Check_Time_Out(sFTP.Systick_LandMarkFTP_new_u32,PERIOD_UART_SIM) == TRUE)
            {
                sFTP.Update_success = TRUE;
                sFTP.Receiver_u8 = SEND_UPDATE_FIRMWARE_OK;
                Erase_Firmware(ADDR_FLAG_HAVE_NEW_FW, 1);
                HAL_FLASH_Unlock();
                osDelay(10);
                HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,ADDR_FLAG_HAVE_NEW_FW,0xAA);
                HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,ADDR_FLAG_HAVE_NEW_FW+0x08,sFTP.Count_Data_u32);
                osDelay(10);
                HAL_FLASH_Lock();                
            }
		}
		if (Check_Time_Out(sFTP.Systick_LandMarkFTP_new_u32,TIME_DELAY_MQTT) == TRUE)
		{
			sFTP.Update_success = FALSE;
			sFTP.Receiver_u8 = SEND_UPDATE_FIRMWARE_FAIL_TIME;
            sTCP.aNeed_Send[UPDATE_FIRM_FAIL] = 1;
		}
		if (sFTP.Count_Data_u32 > sFTP.FirmWare_Length+6)// "\r\nOK\r\n"
		{
			sFTP.Update_success = FALSE;
			sFTP.Receiver_u8 = SEND_UPDATE_FIRMWARE_FAIL_LENGTH;
            sTCP.aNeed_Send[UPDATE_FIRM_FAIL] = 1;
		}
	}
}






