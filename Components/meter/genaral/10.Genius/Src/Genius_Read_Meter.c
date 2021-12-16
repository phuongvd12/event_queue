#include "variable.h"
#include "GENIUS_Init_Meter.h"
#include "GENIUS_Read_Meter.h"
#include "pushdata.h"

#include "command_line_protocol.h"
#include "crc_utility.h"
#include "ieee754_utility.h"

#include "string.h"
#include "math.h"

#include "myDefine.h"

Genius_Var_Struct           GeniusVar;
cmd_recordInfo_t 			recordInfo;
cmd_recordInfo_t 			recordBIll_Inf;

//khia b�o meter type
/*
    trong doc ToU se bo qua scale regis code: vi o day de mac dinh la primary:F7CX
    chi nhan chia voi scale code value:CFxx

    - Gia tri intan cua ToU chi can giai m� so float va double ra.
*/


cmd_channelInfo_t 	Lp1_Channel_Infor[20];
cmd_channelInfo_t 	Bill_Channel_Infor[20];

uint32_t 			lp1_All_ChannelInfoRegList[MAX_LENGHT_ALL_REG_INFOR];      //cu 1 channel se can  8 thanh ghi chua infor. nhan voi Max channel la 12.

uint8_t 			Buff_ReadFile_Check[4] = {0x03, 0x05, 0xF0, 0x08};          //0x0305F008;
uint8_t 			Buff_ReadFile_Check2[4] = {0x03, 0x25, 0xF0, 0x08};          //0x0325F008;
uint8_t 			Buff_MULREAD_ACK[] = {0x00, 0x00, 0xFF, 0xF0};   //ki tu neu lenh tra ve khong loi

uint32_t 			LP1_Reg_ReadFile = 0x0305F008;
uint32_t 			LP2_Reg_ReadFile = 0x0325F008; 

const uint32_t lp1RecordInfoRegList[] = 
{
	0x0305F011, //Mode load survey	1 byte
	0x0305F012, //No channel		1 byte
	0x0305F021, //Num of Entries	4 byte  //store entry
	0x0305F018, //record size		2 byte
    0x0305F014, //Interval
    0x0305F020, //First time record
	0
};

const  uint32_t lp1ChannelInfoRegList[] = 
{ 
	0x0305E000, 			//Channel x Reg    4byte
	0x0305E100, 			//Channel x Size   2byte
	0x0305E200, 			//Channel x type   1byte
	0x0305E300, 			//Channel x Unit   1byte
	0x0305E400, 			//Channel x Name   String. K�t thuc boi ki tu NULL 0x00
	0x0305E500, 			//Channel x Record offset 2 byte
	0x0305E600, 			//Channel x Scale  1 byte
	0x0305E800, 			//Channel x Scale factor  4 byte
	0
};

const  uint32_t lp2RecordInfoRegList[] = 
{
	0x0325F011, //Mode load survey	1 byte
	0x0325F012, //No channel		1 byte
	0x0325F021, //Num of Entries	4 byte  //store entry
	0x0325F018, //record size		2 byte
    0x0325F014, //Interval
    0x0325F020, //First time record
	0
};

const  uint32_t lp2ChannelInfoRegList[] = 
{ 
	0x0325E000, 			//Channel x Reg    4byte
	0x0325E100, 			//Channel x Size   2byte
	0x0325E200, 			//Channel x type   1byte
	0x0325E300, 			//Channel x Unit   1byte
	0x0325E400, 			//Channel x Name   String. K�t thuc boi ki tu NULL 0x00
	0x0325E500, 			//Channel x Record offset 2 byte
	0x0325E600, 			//Channel x Scale  1 byte
	0x0325E800, 			//Channel x Scale factor  4 byte
	0
};

const meterInterfaceInfo_t genius_loadProfileList[] =       
{		//Step_ID					//Cmd_Type						//List_Reg		   //Number_Reg			Callback					//Str_check_Recei					        //legnth data
	{ LP1_GET_RECORD_INFOR,		 MIF_COMMANDTYPE_MULTIPLE,  	&lp1RecordInfoRegList[0], 	6,	    	onSuccessGetRecordInfo, 		{(uint8_t*)&Buff_MULREAD_ACK[0], 4}, 		18 },	/*Get channel info*/
	{ LP1_GET_CHANNEL_INFOR,     MIF_COMMANDTYPE_MULTIPLE,  	&lp1ChannelInfoRegList[0],	8,			onSuccessGetChannelInfo, 		{(uint8_t*)&Buff_MULREAD_ACK[0], 4}, 		0xFFFF }, /*Get data*/
	{ LP1_READ_DATA,		     MIF_COMMANDTYPE_FILE_READ, 	&LP1_Reg_ReadFile, 			1, 		    onSuccessReadProfile, 			{(uint8_t*)&Buff_ReadFile_Check[0], 4}, 	0xFFFF },
    
	{ LP2_GET_RECORD_INFOR,		 MIF_COMMANDTYPE_MULTIPLE,  	&lp2RecordInfoRegList[0], 	6,	    	onSuccessGetRecordInfo, 		{(uint8_t*)&Buff_MULREAD_ACK[0], 4}, 		18 },	 /*Get channel info*/
	{ LP2_GET_CHANNEL_INFOR,     MIF_COMMANDTYPE_MULTIPLE,  	&lp2ChannelInfoRegList[0],	8,			onSuccessGetChannelInfo, 		{(uint8_t*)&Buff_MULREAD_ACK[0], 4}, 		0xFFFF }, /*Get data*/
	{ LP2_READ_DATA,		     MIF_COMMANDTYPE_FILE_READ, 	&LP2_Reg_ReadFile, 			1, 		    onSuccessReadProfile, 			{(uint8_t*)&Buff_ReadFile_Check2[0], 4},     0xFFFF },
	{ END_LP1, 					 MIF_COMMANDTYPE_LOGOUT,  		NULL, 						0,			NULL,				 			NULL, 					0 },
};


//load billing
uint32_t 			Bill_Reg_ReadFile = 0x0065F008; 
uint8_t 			Buff_ReadBill_Check[4] = {0x00, 0x65, 0xF0, 0x08};          //0x0305F008;
const uint32_t lbRecordInfoRegList[] = 
{
    0x0065F011, //Mode load survey
    0x0065F012, //No channel
    0x0065F018, //record size
    0x0065F021, //no stored record
    0 
};

const uint32_t lbChannelInfoRegList[] = { 0x0065E000, 0x0065E100, 0x0065E200, 0x0065E300, 0x0065E400, 0x0065E500, 0x0065E600, 0x0065E800, 0 }; 

const meterInterfaceInfo_t genius_BillingList[] =     
{		//Step_ID					//Cmd_Type						//List_Reg		   //Number_Reg			Callback					//Str_check_Recei					        //legnth data
	{ Bill_GET_RECORD_INFOR,		MIF_COMMANDTYPE_MULTIPLE,  	&lbRecordInfoRegList[0], 	4,	    	onSuccessGetRecordBill_Inf, 		{(uint8_t*)&Buff_MULREAD_ACK[0], 4}, 		8 },	/*Get channel info*/
	{ Bill_GET_CHANNEL_INFOR,       MIF_COMMANDTYPE_MULTIPLE,  	&lbChannelInfoRegList[0],	8,			onSuccessGetChannelBill_Inf, 		{(uint8_t*)&Buff_MULREAD_ACK[0], 4}, 		0xFFFF }, /*Get data*/
	{ Bill_READ_DATA,		        MIF_COMMANDTYPE_FILE_READ, 	&Bill_Reg_ReadFile, 		1, 		    onSuccessReadBill, 			        {(uint8_t*)&Buff_ReadBill_Check[0], 4}, 	0xFFFF },
};

const genius_regIDTagMapping_t regIDTagMappingList[] = 
{
    /*Load billing*/
    { .regID = 0x00010049, .tag = 0x27 },// import
    { .regID = 0x00010149, .tag = 0x28 },
    { .regID = 0x00010249, .tag = 0x29 },
    { .regID = 0x00010349, .tag = 0x2B },
    
    { .regID = 0x00010040, .tag = 0x1F }, //impert rate 1
    { .regID = 0x00010041, .tag = 0x20 },
    { .regID = 0x00010042, .tag = 0x21 },
    
    { .regID = 0x00010140, .tag = 0x23 }, //export rate 1
    { .regID = 0x00010141, .tag = 0x24 },
    { .regID = 0x00010142, .tag = 0x25 },
    
    { .regID = 0x00011040, .tag = 0x31 },  //maxdemmand import rate 1. tu co time di sau
    { .regID = 0x00011041, .tag = 0x35 },
    { .regID = 0x00011042, .tag = 0x39 },
   
    { .regID = 0x00011140, .tag = 0x33 },
    { .regID = 0x00011141, .tag = 0x37 },
    { .regID = 0x00011142, .tag = 0x3B },
   
//    { .regID = 0x00070000, .tag = 0xDA },
    { 0, 0 }
};


void GENIUS_Init_Function (uint8_t type)
{
    eMeter_20._f_Read_ID            = Genius_Get_Meter_ID;
    eMeter_20._f_Check_Reset_Meter  = Genius_CheckResetReadMeter;
  
    eMeter_20._f_Connect_Meter      = Genius_Connect_Meter_Handle;
    eMeter_20._f_Read_TSVH          = GENIUS_Read_TSVH;
    eMeter_20._f_Read_Bill          = GENIUS_Read_Bill;
    eMeter_20._f_Read_Event         = GENIUS_Read_Event;  
    eMeter_20._f_Read_Lpf           = GENIUS_Read_Lpf;
    eMeter_20._f_Read_InforMeter    = GENIUS_Read_Infor;
    eMeter_20._f_Get_UartData       = Genius_GetUART1Data;
    eMeter_20._f_Check_Meter        = Genius_Check_Meter;
    eMeter_20._f_Test1Cmd_Respond   = GENIUS_Send1Cmd_Test;
}

uint8_t GENIUS_Send1Cmd_Test (void)
{
    Init_UART2();
    cmd_generateCommand(MIF_COMMANDTYPE_CONNECT, 0, NULL, 0, 0, 0 ,0);        
    osSemaphoreWait(bsUART2PendingMessHandle,2000);
    
    if(UART1_Control.fRecei_Respond == 1) return 1;
    
    return 0;
}


uint8_t GENIUS_Read_TSVH (uint32_t Type)
{
    uint8_t         Temp_BBC = 0;
    truct_String    Str_Data_Write = {&Buff_Temp1[0], 0};
    uint8_t         Result = 0;
	uint8_t         Retry = 0;

    while ((Retry < 3) && (Result == 0))
    {
        //lay Tu Ti
        if(Genius_Read_TuTi() == 1) 
        {       
            Init_Meter_Info_Struct();
            if(Genius_ReadInfor_1 (&Genius_IDRegisIntan_Table[0], 37, &Get_Meter_Info) == 1)
            {
                if(Genius_ExtractMinfor_data() == 1)
                {
                    Init_Meter_Event_Struct();
                    Get_Meter_Event.Flag_Start_Pack = 1;  //k cho dong goi header
                    Genius_Read_event_TSVH();
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
                    Result = 1;
                }
            }else
                GeniusVar.Count_Error++;
        }
        Retry++;
    }
    return Result;
}


uint8_t GENIUS_Read_Bill (void)
{
    return Genius_Read_Billing();
}

uint8_t GENIUS_Read_Event (uint32_t TemValue)
{
    Init_Meter_Event_Struct();
    return Genius_Read_event();
}

uint8_t GENIUS_Read_Lpf (void)
{
    uint8_t Result = 0;
    
    sDCU.FlagHave_ProfMess = 0;
    Init_Meter_LProf_Struct();
    if(Genius_Read_Loadprofile() == 1) Result = 1;
    
    if(sDCU.FlagHave_ProfMess == 0)
    {
        _fSend_Empty_Lpf();
    }
    
    sInformation.Flag_Stop_ReadLpf = 0;
    return Result;
}

uint8_t GENIUS_Read_Infor (void)
{
    if(Genius_Read_TuTi() == 0) return 0;
    Pack_PushData_103_Infor_Meter();
        
    return 1;
}

 
uint8_t Genius_Connect_Meter_Handle(void)
{
	uint8_t ReadIDRetry = 2;
    
	while (ReadIDRetry>0)
	{
        osDelay(500);
        if (osMutexWait(mtFlashMeterHandle,1000) == osOK)
		{
            Read_Meter_ID_Success = Genius_Get_Meter_ID(0);
            
            if (osMutexRelease(mtFlashMeterHandle) != osOK)
                osMutexRelease(mtFlashMeterHandle);
            
            if (Read_Meter_ID_Success == 1)
                break;
            ReadIDRetry--;
        } 
	}
    if(Read_Meter_ID_Success == 1) 
        return 1;
    else return 0;
}

//Meter ID
uint8_t Genius_Get_Meter_ID (uint32_t TempValue)
{
	uint8_t GetMeterIDRetry = 2;
	uint8_t BuffIDtemp[DCU_ID_LENGTH];
    uint8_t Buffcheck[2] = {0xF0,0x02};
    truct_String    String = {&Buffcheck[0], 2};
    int             Pos_Find = 0;
    uint8_t         ID_Length = 0;
    uint16_t        i = 0;
    
	while (GetMeterIDRetry--)
	{
        if (Genius_Handshake_Handle() == 1)
        {
            Init_UART2();
            cmd_generateCommand(MIF_COMMANDTYPE_READ, (uint32_t) REG_ID_METER , NULL, 0, 0, 0, 0);
            if (osSemaphoreWait(bsUART2PendingMessHandle,5000) == osOK) 
            {
                //Convert lai chuoi nhan duoc neu co cac ki tu dac biet
                Convert_Genius_Respond (&UART1_Control.UART1_Str_Recei);
                //Check sum
                if(Genius_Check_Sum(&UART1_Control.UART1_Str_Recei) == 1)
                {
                    Pos_Find = Find_String_V2((truct_String*) &String, &UART1_Control.UART1_Str_Recei);
                    //cat data instant
                    if(Pos_Find >= 0)
                    {
                        Pos_Find += String.Length_u16;
                        for(i = Pos_Find; i < UART1_Control.UART1_Str_Recei.Length_u16 -3 - 1; i++)  //byte 0x00 cach
                        {
                            BuffIDtemp[ID_Length++] = *(UART1_Control.UART1_Str_Recei.Data_a8 + i);
                            if(ID_Length >= DCU_ID_LENGTH)
                              break;
                        }
                        
                        //lay ID o day
                        if (sDCU.sMeter_id_now.Length_u16 != ID_Length)
                            Read_Meter_ID_Change = 1;
                        for (i=0;i<ID_Length;i++)
                        {
                            if ((*(sDCU.sMeter_id_now.Data_a8+i)) != BuffIDtemp[i])
                            {
                                Read_Meter_ID_Change = 1;
                                break;   //break vong for. nen chua can release 
                            } 
                        }
                        if (Read_Meter_ID_Change == 1)
                        {
                            sDCU.sMeter_id_now.Length_u16 = ID_Length;
                            for (i=0;i<ID_Length;i++)
                                aMeter_ID[i] =  BuffIDtemp[i];
//                                Save_Meter_ID();
                        } 
                        return 1;
                    }   
                }else
                {
                    //logount
                    Init_UART2();
                    cmd_generateCommand(MIF_COMMANDTYPE_LOGOUT, 0, NULL, 0, 0, 0, 0);
                    osDelay(1000);
                }
            }else
            {
                //bao loi va logount
                Init_UART2();
                cmd_generateCommand(MIF_COMMANDTYPE_LOGOUT, 0, NULL, 0, 0, 0, 0);
                osDelay(1000);
            }
        } else
            osDelay(4000);
	}
	
	return 0;
}


uint8_t Genius_Handshake_Handle (void)
{
    uint8_t          Step = GE_STEP_FIRST;
    uint8_t          Buff_ACK[5] = {0x02, 0x06, 0x06, 0xA4, 0x03}; ;
	truct_String     StringCheck = {&Buff_ACK[0], 5};  
    uint8_t 		 Result = 0;
	uint8_t			 Pending = 0;
    
	while (Pending == 0)  
	{
		switch (Step)
		{
			case GE_STEP_FIRST:	//Send first Char
				Step++;
                Init_UART2();
                cmd_generateCommand(MIF_COMMANDTYPE_LOGOUT, 0, NULL, 0, 0, 0, 0);
                osSemaphoreWait(bsUART2PendingMessHandle,5000);
                Init_UART2();
				cmd_generateCommand(MIF_COMMANDTYPE_CONNECT, 0, NULL, 0, 0, 0 ,0);
				break;
			case GE_CHECK_ACK_1:
                if (osSemaphoreWait(bsUART2PendingMessHandle,5000) == osOK) 
                {
                    Step++;
					if(UART1_Control.UART1_Str_Recei.Length_u16 < 5) 
                        Step = GE_STEP_END;
					if(Find_String_V2((truct_String*) &StringCheck, &UART1_Control.UART1_Str_Recei) < 0)
                        Step = GE_STEP_END;
				}else 
                    Step = GE_STEP_END;				
				break;
			case GE_SEND_LOGIN:	//Send Handshake 2
                Init_UART2();
				Step++;
				cmd_generateCommand(MIF_COMMANDTYPE_LOGON, 0, NULL, 0, 0, 0 ,0);
				break;
			case GE_CHECK_ACK_2:
                if (osSemaphoreWait(bsUART2PendingMessHandle,5000) == osOK) 
                {
                    Step++;
					if(UART1_Control.UART1_Str_Recei.Length_u16 < 5)
                        Step = GE_STEP_END;
                    
					if(Find_String_V2((truct_String*) &StringCheck, &UART1_Control.UART1_Str_Recei) >= 0)
					{
						Result = 1;
					}else   //sai Pass user
                    {
                        Step = GE_STEP_END;
                        if(StrUser.Length_u16 == 4)
                        {
                            StrUser.Data_a8 = &cmd_username[0];
                            StrUser.Length_u16 = 6;
                            StrPass.Data_a8 = &cmd_password[0];
                            StrPass.Length_u16 = 6;
                        }else
                        {
                            StrUser.Data_a8 = &cmd_username2[0];
                            StrUser.Length_u16 = 4;
                            StrPass.Data_a8 = &cmd_password2[0];
                            StrPass.Length_u16 = 8;
                        }
                    }
				}
				else 
                    Step = GE_STEP_END;
            case GE_STEP_END:
				Pending = 1;
				Init_UART2();
                if (Result == 0)
				{
					cmd_generateCommand(MIF_COMMANDTYPE_LOGOUT, 0, NULL, 0, 0, 0 ,0);
				}
                break;
			default:
				break;		
		}
	}
	return Result;
}



uint8_t Genius_GetUART1Data(void)
{
	uint8_t	temp_recieve = 0;

	if (UART1_Control.Mess_Pending_ui8 == 0)
	{
		temp_recieve = (uint8_t)(UART_METER.Instance->RDR&0xFF);
		
		if((temp_recieve == 0x02) || (UART1_Control.Flag_Have_0x02 != 0))    //bat dau ghi vai buff tu 0x02
		{
			UART1_Control.Flag_Have_0x02 = 1;  //DA NHAN DUOC KI TU BAT DAU LENH
			
			*(UART1_Control.UART1_Str_Recei.Data_a8 + UART1_Control.UART1_Str_Recei.Length_u16) = temp_recieve;
			UART1_Control.UART1_Str_Recei.Length_u16++;
            UART1_Control.Mess_Length_ui16++;
			
			if (temp_recieve == 0x03) // end message
			{
				UART1_Control.Mess_Pending_ui8 = 1;
                osSemaphoreRelease(bsUART2PendingMessHandle);
			}
			else
			{
				if (UART1_Control.UART1_Str_Recei.Length_u16 > MAX_LENGTH_BUFF_NHAN)	
					UART1_Control.UART1_Str_Recei.Length_u16 = 0;
			}
		}
	}else
		temp_recieve = (uint8_t)(UART_METER.Instance->RDR&0xFF);
    
    UART1_Control.fRecei_Respond = 1;
    return 1;
}


void Convert_Genius_Respond (truct_String* Str)
{	
	uint16_t 	i = 0;
	uint16_t 	j = 0;
	
	for(i = 0; i < Str->Length_u16; i++)
	{
		if(*(Str->Data_a8 + i) == 0x10)
		{
			*(Str->Data_a8 + i) = *(Str->Data_a8 + i + 1) - 0x40;
			//
			for(j = i + 1; j<Str->Length_u16; j++)
			{
				*(Str->Data_a8 + j) = *(Str->Data_a8 + j + 1);
			}
			//giam length di 1
			Str->Length_u16 --;
		}	
	}
}

uint8_t Genius_Check_Sum (truct_String* Str)
{
	uint16_t i =0;
	uint16_t crc;
	
	
	crc = CalculateCharacterCRC16(0,CMD_STX);
	
	//tinh crc
	for (i =1; i<(Str->Length_u16 - 3); i++) 
	{
		crc = CalculateCharacterCRC16(crc,*(Str->Data_a8 + i));
	}
	
	if(((*(Str->Data_a8 + Str->Length_u16 - 3)) == ((uint8_t) (crc >> 8))) && ((*(Str->Data_a8 + Str->Length_u16 - 2)) == ((uint8_t) crc)))
		 return 1;
	else return 0;
			
}


uint8_t	Genius_ReadInfor_1 (const uint32_t* Reg, uint8_t length, Meter_Comm_Struct* Get_Meter)
{
	int         Pos_Find = 0;
    uint8_t 	Buff_MULTI_READ_RES[4]  = {0x00, 0x00, 0xFF, 0xF0};    //1 loai loi se bao loi ra
    truct_String      Str_Multi_Read_Check = {&Buff_MULTI_READ_RES[0], 4};
	uint16_t    i = 0;
    
    
	//Reset lai uart nhan
	Init_UART2();
	if(Genius_Handshake_Handle() == 1)
    {
        cmd_generateCommand(MIF_COMMANDTYPE_MULTIPLE, 0, Reg, length, 0, 0, 0);
        if (osSemaphoreWait(bsUART2PendingMessHandle,5000) == osOK) 
        {
            //Convert lai chuoi nhan duoc neu co cac ki tu dac biet
            Convert_Genius_Respond (&UART1_Control.UART1_Str_Recei);
            //Check Crc
            if(Genius_Check_Sum(&UART1_Control.UART1_Str_Recei) == 0)
                return 0;
            
            Pos_Find = Find_String_V2((truct_String*) &Str_Multi_Read_Check, &UART1_Control.UART1_Str_Recei);
            //cat data instant
            if(Pos_Find >= 0)
            {
                Pos_Find += Str_Multi_Read_Check.Length_u16;
                //Cat data theo length da fix cua tung gia tri
                for(i = Pos_Find; i < (UART1_Control.UART1_Str_Recei.Length_u16 - 3); i++)
                    Meter_TempBuff[Get_Meter->Data_Buff_Pointer_ui16++] = *(UART1_Control.UART1_Str_Recei.Data_a8 + i);
                return 1;
            }

        }
    }
	
	return 0;				
}



uint8_t Genius_Get_Meter_Time (uint32_t RegID, ST_TIME_FORMAT* sTime)
{
	int Pos_Find;
	uint8_t 			Buff_Reg_ID_Time[4];
    truct_String 		Str_Reg_ID_Time = {&Buff_Reg_ID_Time[0], 0};
    
    Add_Reg_2Buff(&Str_Reg_ID_Time,RegID);
	Init_UART2();
    if(Genius_Handshake_Handle() == 1)
    {
        cmd_generateCommand(MIF_COMMANDTYPE_READ, RegID , NULL, 0, 0, 0, 0);
        if (osSemaphoreWait(bsUART2PendingMessHandle,5000) == osOK) 
        {
            //Convert lai chuoi nhan duoc neu co cac ki tu dac biet
            Convert_Genius_Respond (&UART1_Control.UART1_Str_Recei);
            //Check sum
            if(Genius_Check_Sum(&UART1_Control.UART1_Str_Recei) == 1)
            {
                Pos_Find = Find_String_V2((truct_String*) &Str_Reg_ID_Time, &UART1_Control.UART1_Str_Recei);
                //cat data instant
                if(Pos_Find >= 0)
                {
                    Pos_Find += Str_Reg_ID_Time.Length_u16;
                    //check length
                    if((UART1_Control.UART1_Str_Recei.Length_u16 - Pos_Find) == (6 + 3))   //6 byte ngay/thang/nam gio/phut/giay
                    {
                        sTime->date     = *(UART1_Control.UART1_Str_Recei.Data_a8 + Pos_Find++);
                        sTime->month    = *(UART1_Control.UART1_Str_Recei.Data_a8 + Pos_Find++);
                        sTime->year     = *(UART1_Control.UART1_Str_Recei.Data_a8 + Pos_Find++);
                        sTime->hour     = *(UART1_Control.UART1_Str_Recei.Data_a8 + Pos_Find++);
                        sTime->min      = *(UART1_Control.UART1_Str_Recei.Data_a8 + Pos_Find++);
                        sTime->sec      = *(UART1_Control.UART1_Str_Recei.Data_a8 + Pos_Find++);
                        return 1;
                    }
                }
			}
		}
	}
	return 0;
 }

uint8_t Genius_ExtractMinfor_data (void)
{
    Genius_Minfor_value(10, 4, 0, Str_Ob_VolA, Unit_Voltage, GENIUS_SCALE_VOLTAGE, 1);
    Genius_Minfor_value(14, 4, 0, Str_Ob_VolB, Unit_Voltage, GENIUS_SCALE_VOLTAGE, 1);
    Genius_Minfor_value(18, 4, 0, Str_Ob_VolC, Unit_Voltage, GENIUS_SCALE_VOLTAGE, 1);
    
    Genius_Minfor_value(22, 4, 0, Str_Ob_CurA, Unit_Current, GENIUS_SCALE_CURRENT, 1);
    Genius_Minfor_value(26, 4, 0, Str_Ob_CurB, Unit_Current, GENIUS_SCALE_CURRENT, 1);
    Genius_Minfor_value(30, 4, 0, Str_Ob_CurC, Unit_Current, GENIUS_SCALE_CURRENT, 1);
    
    Genius_Minfor_value(34, 4, 0, Str_Ob_AcPowA, Unit_Active_Intan_Kw, GENIUS_SCALE_ACTIVE_POW, 0);
    Genius_Minfor_value(38, 4, 0, Str_Ob_AcPowB, Unit_Active_Intan_Kw, GENIUS_SCALE_ACTIVE_POW, 0);
    Genius_Minfor_value(42, 4, 0, Str_Ob_AcPowC, Unit_Active_Intan_Kw, GENIUS_SCALE_ACTIVE_POW, 0);
    Genius_Minfor_value(46, 4, 0, Str_Ob_AcPowTo, Unit_Active_Intan_Kw, GENIUS_SCALE_ACTIVE_POW, 0);
    
    Genius_Minfor_value(50, 4, 0, Str_Ob_RePowA, Unit_Reactive_Intan_Kvar, GENIUS_SCALE_REACTIVE_POW, 0);    
    Genius_Minfor_value(54, 4, 0, Str_Ob_RePowB, Unit_Reactive_Intan_Kvar, GENIUS_SCALE_REACTIVE_POW, 0);
    Genius_Minfor_value(58, 4, 0, Str_Ob_RePowC, Unit_Reactive_Intan_Kvar, GENIUS_SCALE_REACTIVE_POW, 0);
    Genius_Minfor_value(62, 4, 0, Str_Ob_RePowTo, Unit_Reactive_Intan_Kvar, GENIUS_SCALE_REACTIVE_POW, 0);
    
    Genius_Minfor_value(66, 4, 0, Str_Ob_Freq, Unit_Freq, GENIUS_SCALE_FREQ, 1);   
    Genius_Minfor_value(70, 4, 0, Str_Ob_PoFac, StrNull, GENIUS_SCALE_POW_FACTOR, 1);  
    //tariff
    Genius_Minfor_value(74, 8, 0, Str_Ob_En_ImportWh, Unit_Active_EnTotal, GENIUS_SCALE_TOTAL_ENERGY, 0);       
    Genius_Minfor_value(82, 8, 0, Str_Ob_AcPlus_Rate1, Unit_Active_EnTotal, GENIUS_SCALE_TARRIFF, 0);
    Genius_Minfor_value(90, 8, 0, Str_Ob_AcPlus_Rate2, Unit_Active_EnTotal, GENIUS_SCALE_TARRIFF, 0);
    Genius_Minfor_value(98, 8, 0, Str_Ob_AcPlus_Rate3, Unit_Active_EnTotal, GENIUS_SCALE_TARRIFF, 0);
    
    Genius_Minfor_value(106, 8, 0, Str_Ob_En_ExportWh, Unit_Active_EnTotal, GENIUS_SCALE_TOTAL_ENERGY, 0 );           
    Genius_Minfor_value(114, 8, 0, Str_Ob_AcSub_Rate1, Unit_Active_EnTotal, GENIUS_SCALE_TARRIFF, 0);
    Genius_Minfor_value(122, 8, 0, Str_Ob_AcSub_Rate2, Unit_Active_EnTotal, GENIUS_SCALE_TARRIFF, 0);
    Genius_Minfor_value(130, 8, 0, Str_Ob_AcSub_Rate3, Unit_Active_EnTotal, GENIUS_SCALE_TARRIFF, 0);
    
    Genius_Minfor_value(138, 8, 0, Str_Ob_En_ImportVar, Unit_Reactive_EnTotal, GENIUS_SCALE_TOTAL_ENERGY, 0 );     
    Genius_Minfor_value(146, 8, 0, Str_Ob_En_ExportVar, Unit_Reactive_EnTotal, GENIUS_SCALE_TOTAL_ENERGY, 0 ); 
    //maxdemand
    Genius_Minfor_value(154, 4, 0, Str_Ob_MaxDeRate1, Unit_Active_Intan_Kw, GENIUS_SCALE_MAX_DEMAND, 0 );   
    Genius_Minfor_value(158, 6, 1, StrNull, StrNull, 0, 0 ); 
    
    Genius_Minfor_value(164, 4, 0, Str_Ob_MaxDeRate2, Unit_Active_Intan_Kw, GENIUS_SCALE_MAX_DEMAND, 0 );   
    Genius_Minfor_value(168, 6, 1, StrNull, StrNull, 0, 0 ); 
    
    Genius_Minfor_value(174, 4, 0, Str_Ob_MaxDeRate3, Unit_Active_Intan_Kw, GENIUS_SCALE_MAX_DEMAND, 0 );   
    Genius_Minfor_value(178, 6, 1, StrNull, StrNull, 0, 0 ); 
    //Tu Ti
//    Genius_Minfor_value(184, 4, 0, Str_Ob_Ti, StrNull, GENIUS_SCALE_TU_TI );       
//    Genius_Minfor_value(188, 4, 2, Str_Ob_Ti, StrNull, GENIUS_SCALE_TU_TI);
//    Genius_Minfor_value(192, 4, 0, Str_Ob_Tu, StrNull, GENIUS_SCALE_TU_TI);
//    Genius_Minfor_value(196, 4, 2, Str_Ob_Tu, StrNull, GENIUS_SCALE_TU_TI);
    return Genius_Minfor_value(200, 6, 3, StrNull, StrNull, 0,  0);
}

uint8_t Genius_Minfor_value(uint16_t PosStart, uint8_t length, uint8_t type, truct_String oBIS, truct_String unit, uint8_t ScaleNum, uint8_t fEnablescale)
{
    uint32_t TempU32 = 0;
    uint64_t TempU64 = 0;
    uint8_t  i = 0;
    uint16_t Pos = PosStart;  
    int 	 scaleVal = 1;
    uint8_t  BuffNum[30];
    truct_String Str_Data_Write={&BuffNum[0], 0};
//    uint8_t     Temp_BBC = 0;
    ST_TIME_FORMAT  sTimetemp;
    
    //Get scale value
    if(fEnablescale == 1)  //chi co nhung gia tri khong can fix scale (de don vi nho nhat) thi moi can nhan them de b� vao
        for (i = 0; i < ScaleNum; i++) 
            scaleVal *= 10;
    
    if(Get_Meter_Info.Flag_Start_Pack == 0)
    {
        Reset_Buff(&Get_Meter_Info.Str_Payload);
        //wrire header TSVH vao
        Write_Header_TSVH_Push103();
        Get_Meter_Info.Flag_Start_Pack = 1;
    } 
    
    switch(type)
    {
        case 0:
            if(length == 4)
            {
                for(i = 0; i < length; i++)
                    TempU32 = (TempU32 <<8) | Meter_TempBuff[Pos++];
                
                //ghi obis va data vao
                Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Obis_Inbuff, &oBIS);
                Get_Meter_Info.Pos_Obis_Inbuff += oBIS.Length_u16;
                Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
                if(oBIS.Length_u16 != 0) Get_Meter_Info.Numqty ++;
                
                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                Pack_HEXData_Frame(&Str_Data_Write, (int64_t) (Convert_FloatPoint_2Float(TempU32) * scaleVal), ScaleNum);
                if(unit.Length_u16 != 0)
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                Copy_String_2(&Str_Data_Write, &unit);  //don vi
                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
            
                Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Data_Inbuff, &Str_Data_Write);
                Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
            }else if (length == 8)
            {
                for(i = 0; i < length; i++)
                    TempU64 = (TempU64 << 8) | Meter_TempBuff[Pos++];
                
                //ghi obis va data vao
                Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Obis_Inbuff, &oBIS);
                Get_Meter_Info.Pos_Obis_Inbuff += oBIS.Length_u16;
                Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
                if(oBIS.Length_u16 != 0) Get_Meter_Info.Numqty ++;
                
                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                Pack_HEXData_Frame(&Str_Data_Write, (int64_t) (Convert_DoublePoint_2Double(TempU64) * scaleVal), ScaleNum);
                if(unit.Length_u16 != 0)
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                Copy_String_2(&Str_Data_Write, &unit);  //don vi
                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
            
                Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Data_Inbuff, &Str_Data_Write);
                Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
            }
            break;
        case 1:   //stime maxdemand
            sTimetemp.date  = Meter_TempBuff[Pos+0];
            sTimetemp.month = Meter_TempBuff[Pos+1];
            sTimetemp.year  = Meter_TempBuff[Pos+2];
            sTimetemp.hour  = Meter_TempBuff[Pos+3];
            sTimetemp.min   = Meter_TempBuff[Pos+4];
            sTimetemp.sec   = Meter_TempBuff[Pos+5];
            
            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
            Copy_String_STime(&Str_Data_Write, sTimetemp);
            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
            
            Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Data_Inbuff, &Str_Data_Write);
            Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
            break;
        case 2:  //tum tim
            Get_Meter_Info.Str_Payload.Length_u16--;
            Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
            
            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '/';
            for(i = 0; i<length; i++)
                    TempU32 = (TempU32 <<8) | Meter_TempBuff[Pos++];
            Pack_HEXData_Frame(&Str_Data_Write, (int64_t) (Convert_FloatPoint_2Float(TempU32) * scaleVal), ScaleNum);
            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
            
            Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Data_Inbuff, &Str_Data_Write);
            Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
            break;
        case 3:   //stime intan
            //Ghi them data he so nhan vao
//            Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Obis_Inbuff, &He_So_Nhan);
//            Get_Meter_Info.Pos_Obis_Inbuff += He_So_Nhan.Length_u16;
//            Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
//            Get_Meter_Info.Numqty++;
//            //he so nhan	
//            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
//            Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) sDCU.He_So_Nhan, GENIUS_SCALE_HE_SO_NHAN); 
//            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
//            
//            Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Data_Inbuff, &Str_Data_Write);
            
            Str_Data_Write.Length_u16 = 0;  //reset
            sTimetemp.date  = Meter_TempBuff[Pos+0];
            sTimetemp.month = Meter_TempBuff[Pos+1];
            sTimetemp.year  = Meter_TempBuff[Pos+2];
            sTimetemp.hour  = Meter_TempBuff[Pos+3];
            sTimetemp.min   = Meter_TempBuff[Pos+4];
            sTimetemp.sec   = Meter_TempBuff[Pos+5];
            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
            Copy_String_STime(&Str_Data_Write, sTimetemp); 
            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
            
//            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
//            Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) Get_Meter_Info.Numqty, 0);
//            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')'; 
            
            Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.PosNumqty, &Str_Data_Write);
            Get_Meter_Info.PosNumqty += Str_Data_Write.Length_u16;
            Get_Meter_Info.Pos_Obis_Inbuff += Str_Data_Write.Length_u16;
            Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
//            //ETX
//            *(Get_Meter_Info.Str_Payload.Data_a8 + Get_Meter_Info.Str_Payload.Length_u16++) = ETX; 
//            //BBC
//            Temp_BBC = BBC_Cacul(Get_Meter_Info.Str_Payload.Data_a8 + 1,Get_Meter_Info.Str_Payload.Length_u16 - 1);
//            *(Get_Meter_Info.Str_Payload.Data_a8 + Get_Meter_Info.Str_Payload.Length_u16++) = Temp_BBC;
            return 1;
            
        default: 
            break;
    }
    
    return 0;
}




uint8_t Genius_Check_Meter(void)
{
    // Check Error of Meter ID
	if (Read_Meter_ID_Success == 1) 
	{ 
        if(GeniusVar.Count_Error >=2)
        {
            Read_Meter_ID_Success = Genius_Get_Meter_ID(0);
        }
	} 
    return 1;
} 

uint8_t Genius_CheckResetReadMeter(uint32_t Timeout)
{
	// Reset neu doc sai cong to
    if ((sDCU.Status_Meter_u8 == 0) && (Check_Time_Out(sDCU.LandMark_Count_Reset_Find_Meter,600000) == TRUE)) // 10p
    {
        osDelay(Timeout); // 5p
        Read_Meter_ID_Success = Genius_Get_Meter_ID(0);
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

    
    
    
uint8_t Test_Read_1reg (uint32_t RegID, uint8_t* Buff)
{
	int Pos_Find;
	uint8_t 			Buff_Reg_ID_Time[4];
    truct_String 		Str_Reg_ID_Time = {&Buff_Reg_ID_Time[0], 0};
    uint8_t             Length = 0;
    uint16_t  i = 0;
      
    Add_Reg_2Buff(&Str_Reg_ID_Time,RegID);
	Init_UART2();
    if(Genius_Handshake_Handle() == 1)
    {
        cmd_generateCommand(MIF_COMMANDTYPE_READ, RegID , NULL, 0, 0, 0, 0);
        if (osSemaphoreWait(bsUART2PendingMessHandle,5000) == osOK) 
        {
            //Convert lai chuoi nhan duoc neu co cac ki tu dac biet
            Convert_Genius_Respond (&UART1_Control.UART1_Str_Recei);
            //Check sum
            if(Genius_Check_Sum(&UART1_Control.UART1_Str_Recei) == 1)
            {
                Pos_Find = Find_String_V2((truct_String*) &Str_Reg_ID_Time, &UART1_Control.UART1_Str_Recei);
                //cat data instant
                if(Pos_Find >= 0)
                {
                    Pos_Find += Str_Reg_ID_Time.Length_u16;
                    //check length
                    
                    for(i = Pos_Find; i < UART1_Control.UART1_Str_Recei.Length_u16 - 3; i++)
                       *(Buff+ Length++) = *(UART1_Control.UART1_Str_Recei.Data_a8 + i);
                    return 1;
                }
			}
		}
	}
	return 0;
 }



//Read lpf

uint8_t Read_Step_Lp (uint8_t Step)
{
	int 		Pos_Find = 0;
	uint8_t 	length = 0;
	uint8_t 	i = 0;
	uint8_t 	Count = 0;
	uint8_t 	j = 0;
	
	//Reset lai uart nhan
	Init_UART2();

    if(Genius_Handshake_Handle() == 1)
    {
        //truyen lenh doc cac thanh ghi Infor Record
        if((Step == LP1_GET_CHANNEL_INFOR) || Step == LP2_GET_CHANNEL_INFOR)
        {
            for(i = 0 ; i < MAX_LENGHT_ALL_REG_INFOR; i++)
                lp1_All_ChannelInfoRegList[i] = 0;
            //
            for (Count = 0; Count < (recordInfo.noChannel + 1); Count++)  //co 1 channel status nuass
                for (j = 0; j < genius_loadProfileList[Step].Number_Reg; j++)
                    lp1_All_ChannelInfoRegList[length++] = *(genius_loadProfileList[Step].Buff_List_Reg + j) + Count;

            
            cmd_generateCommand(genius_loadProfileList[Step].cmdType, 0, &lp1_All_ChannelInfoRegList[0], length, 0, 0, 0);
        }else if((Step == LP1_READ_DATA) || (Step == LP2_READ_DATA))
        {
            cmd_generateCommand(genius_loadProfileList[Step].cmdType, (uint32_t) *genius_loadProfileList[Step].Buff_List_Reg ,NULL, 0, 
                                recordInfo.startRecordIndex + recordInfo.currentInd , recordInfo.NumReadone, recordInfo.recordSize);  
        }else
            cmd_generateCommand(genius_loadProfileList[Step].cmdType, 0,genius_loadProfileList[Step].Buff_List_Reg, genius_loadProfileList[Step].Number_Reg, 0, 0, 0);        
        
        if (osSemaphoreWait(bsUART2PendingMessHandle,5000) == osOK) 
        {
            //Convert lai chuoi nhan duoc neu co cac ki tu dac biet
            Convert_Genius_Respond (&UART1_Control.UART1_Str_Recei);
            //Check Crc
            if(Genius_Check_Sum(&UART1_Control.UART1_Str_Recei) == 0)
                return 0;
            
            Pos_Find = Find_String_V2((truct_String*) &genius_loadProfileList[Step].Str_Recei, &UART1_Control.UART1_Str_Recei);
            //cat data instant
            if(Pos_Find >= 0)
            {
                Pos_Find += genius_loadProfileList[Step].Str_Recei.Length_u16;
                //check length
                if(genius_loadProfileList[Step].Length_Data == 0xFFFF)   //kh�ng c�n check length data nh�n ve
                {
                    if(genius_loadProfileList[Step].CallBack(&UART1_Control.UART1_Str_Recei, Pos_Find) == 1)
                        return 1;
                }else
                {
                    if((UART1_Control.UART1_Str_Recei.Length_u16 - Pos_Find) == (genius_loadProfileList[Step].Length_Data + 3))
                    {
                        if(genius_loadProfileList[Step].CallBack(&UART1_Control.UART1_Str_Recei, Pos_Find) == 1)
                            return 1;
                    }
                }
            }
        }
    }

	return 0;
}


uint8_t onSuccessGetRecordInfo (truct_String* Str, uint16_t Pos)
{
	//Get load survey mode
	recordInfo.modeLoadSurvey = *(Str->Data_a8 + Pos );
	Pos++;
	//Get no channel
	recordInfo.noChannel = *(Str->Data_a8 + Pos );
	Pos++;
	//Get Num Entries
	recordInfo.Number_Entries = (*(Str->Data_a8 + Pos ) << 24) + (*(Str->Data_a8 + Pos + 1 ) << 16) + (*(Str->Data_a8 + Pos + 2 ) << 8) + *(Str->Data_a8 + Pos + 3) ;
	Pos += 4;
	//Get record size
	recordInfo.recordSize = (*(Str->Data_a8 + Pos ) << 8) + *(Str->Data_a8 + Pos + 1);
	Pos += 2;
    //Get interval
    recordInfo.Interval = (*(Str->Data_a8 + Pos ) << 24) + (*(Str->Data_a8 + Pos + 1 ) << 16) + (*(Str->Data_a8 + Pos + 2 ) << 8) + *(Str->Data_a8 + Pos + 3) ;
    Pos += 4;
    
    recordInfo.sTimeStart.date  = *(Str->Data_a8 + Pos++ );
    recordInfo.sTimeStart.month = *(Str->Data_a8 + Pos++ );
    recordInfo.sTimeStart.year  = *(Str->Data_a8 + Pos++ );
    recordInfo.sTimeStart.hour  = *(Str->Data_a8 + Pos++ );
    recordInfo.sTimeStart.min   = *(Str->Data_a8 + Pos++ );
    recordInfo.sTimeStart.sec   = *(Str->Data_a8 + Pos++ );
    
	return 1;
}

uint8_t onSuccessGetChannelInfo (truct_String* Str, uint16_t Pos)
{
	//them 1 channel status
	int loopTmp = 0;
	uint32_t Temp_Scale = 0;
	
	for (loopTmp = 0; (loopTmp < recordInfo.noChannel + 1) && ((Pos + 4) < Str->Length_u16); loopTmp++) 
	{
		//Get channel register ID
		Lp1_Channel_Infor[loopTmp].regID = (*(Str->Data_a8 + Pos ) << 24) | (*(Str->Data_a8 + Pos + 1 ) << 16) | (*(Str->Data_a8 + Pos + 2 ) << 8) | *(Str->Data_a8 + Pos + 3 );
		Pos += 4;
		//Get channel size
		Lp1_Channel_Infor[loopTmp].size = (*(Str->Data_a8 + Pos ) << 8) + *(Str->Data_a8 + Pos + 1);
		Pos += 2;
		//Skip type and unit
		Pos += 2;

		//Skip channel name
		char* tmpChar = (char*) (Str->Data_a8 + Pos);
		Lp1_Channel_Infor[loopTmp].tag = getTagByChannelName(tmpChar);
		int dataTmp = strlen((char*) (Str->Data_a8 + Pos));
		Pos += (dataTmp + 1);

		//Get channel offset
		Lp1_Channel_Infor[loopTmp].offset = (*(Str->Data_a8 + Pos ) << 8) + *(Str->Data_a8 + Pos + 1);
		Pos += 2;

		//Skip scale
		Pos++;
		Temp_Scale = (*(Str->Data_a8 + Pos ) << 24) | (*(Str->Data_a8 + Pos + 1 ) << 16) | (*(Str->Data_a8 + Pos + 2 ) << 8) | *(Str->Data_a8 + Pos + 3 );
		//Get Scale factor
		Lp1_Channel_Infor[loopTmp].scale = Convert_FloatPoint_2Float (Temp_Scale);
		Pos += 4;
	}
	return 1;
}


uint8_t onSuccessReadProfile (truct_String* Str, uint16_t Pos)
{
	uint8_t         i = 0;
	uint8_t         j = 0;
	uint64_t        Temp_Value = 0;
//	uint32_t 	    Actual_Record_Read = 0;
	uint8_t         Count = 0;
    uint8_t         BuffNum[30];
	truct_String    Str_Data_Write={&BuffNum[0], 0};
    ST_TIME_FORMAT		sTimeTemp;
    uint32_t        TimeStampStart = 0;
    uint8_t         FlagAlowPackObis = 0;
    
	if((Str->Length_u16 - Pos) == (recordInfo.recordSize* recordInfo.NumReadone + 10 + 3))    //4byte Actual Start Re + 2 byte no Re + 2 byte offset + 2 byte Size
	{
//		//check xem tra ve Actual Start Re co dung khong
//		Actual_Record_Read = (*(Str->Data_a8 + Pos ) << 24) | (*(Str->Data_a8 + Pos + 1 ) << 16) | (*(Str->Data_a8 + Pos + 2 ) << 8) | *(Str->Data_a8 + Pos + 3 );
//		if(Actual_Record_Read != (recordInfo.Number_Entries - 1)) return 0;
		//Test lai dk nay xem
		Pos += 10;
		//cat data theo vi tri offset va size
        while (Count < recordInfo.NumReadone)  //xem lai numrecord tinh toan cho chuan
        {          
            if(Get_Meter_LProf.Flag_Start_Pack == 0)
            {
                FlagAlowPackObis = 0;
                Reset_Buff(&Get_Meter_LProf.Str_Payload);
                Pack_Header_lpf_Pushdata103();
                Get_Meter_LProf.Flag_Start_Pack = 1;
                TimeStampStart = HW_RTC_GetCalendarValue_Second(recordInfo.sTimeStart, 3);
                Epoch_to_date_time(&sTimeTemp, (TimeStampStart + (recordInfo.startRecordIndex + recordInfo.currentInd + Count) * recordInfo.Interval), 3);
            }

            for(i = 0; i < recordInfo.noChannel + 1; i++)
            {
                Temp_Value = 0;
                Reset_Buff(&Str_Data_Write);
                
                for(j = 0; j < Lp1_Channel_Infor[i].size; j++)
                    Temp_Value = (Temp_Value << 8) + UART1_Receive_Buff[Pos + Lp1_Channel_Infor[i].offset + j]; 
 
                switch(Lp1_Channel_Infor[i].tag)
                {
                    
                    case 0x27:    
                        //ghi obis vao
                        if(FlagAlowPackObis == 0)  //chi ghi obis 1 lan. cac lan sau ghi data
                        {
                            Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Obis_Inbuff, &Str_Ob_En_ImportWh);        
                            Get_Meter_LProf.Pos_Obis_Inbuff += Str_Ob_En_ImportWh.Length_u16;
                            Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
                            Get_Meter_LProf.Numqty++;
                        }
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                        Pack_HEXData_Frame(&Str_Data_Write, (int64_t) (Temp_Value* Lp1_Channel_Infor[i].scale), 3);   //fix scale 3 de thanh kw
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                        Copy_String_2(&Str_Data_Write, &Unit_Active_EnTotal);   
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                        Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Data_Inbuff, &Str_Data_Write);
                        Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
                        break;
                    case 0x28:
                        //ghi obis vao
                        if(FlagAlowPackObis == 0)  //chi ghi obis 1 lan. cac lan sau ghi data
                        {
                            Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Obis_Inbuff, &Str_Ob_En_ExportWh);
                            Get_Meter_LProf.Pos_Obis_Inbuff += Str_Ob_En_ExportWh.Length_u16;
                            Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
                            Get_Meter_LProf.Numqty++;
                        }
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                        Pack_HEXData_Frame(&Str_Data_Write, (int64_t) (Temp_Value* Lp1_Channel_Infor[i].scale), 3);
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                        Copy_String_2(&Str_Data_Write, &Unit_Active_EnTotal);  
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                        Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Data_Inbuff, &Str_Data_Write);
                        Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
                        break;
                    case 0x29:
                        //ghi obis vao
                        if(FlagAlowPackObis == 0)  //chi ghi obis 1 lan. cac lan sau ghi data
                        {
                            Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Obis_Inbuff, &Str_Ob_En_ImportVar);
                            Get_Meter_LProf.Pos_Obis_Inbuff += Str_Ob_En_ImportVar.Length_u16;
                            Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
                            Get_Meter_LProf.Numqty++;
                        }
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                        Pack_HEXData_Frame(&Str_Data_Write, (int64_t) (Convert_DoublePoint_2Double(Temp_Value)), 3);
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                        Copy_String_2(&Str_Data_Write, &Unit_Reactive_EnTotal); 
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                        Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Data_Inbuff, &Str_Data_Write);
                        Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
                        break;  
                    case 0x2B:      
                        //ghi obis vao
                        if(FlagAlowPackObis == 0)  //chi ghi obis 1 lan. cac lan sau ghi data
                        {
                            Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Obis_Inbuff, &Str_Ob_En_ExportVar);
                            Get_Meter_LProf.Pos_Obis_Inbuff += Str_Ob_En_ExportVar.Length_u16;
                            Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
                            Get_Meter_LProf.Numqty++;
                        }
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                        Pack_HEXData_Frame(&Str_Data_Write, (int64_t) (Temp_Value* Lp1_Channel_Infor[i].scale), 3);
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                        Copy_String_2(&Str_Data_Write, &Unit_Reactive_EnTotal);  
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                        Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Data_Inbuff, &Str_Data_Write);
                        Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
                        break;
                    case 0x02:    
                        //ghi obis vao
                        if(FlagAlowPackObis == 0)  //chi ghi obis 1 lan. cac lan sau ghi data
                        {
                            Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Obis_Inbuff, &Str_Ob_lpf[8]);
                            Get_Meter_LProf.Pos_Obis_Inbuff += Str_Ob_lpf[8].Length_u16;
                            Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
                            Get_Meter_LProf.Numqty++;
                        }
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                        Pack_HEXData_Frame(&Str_Data_Write, (int64_t) (Temp_Value* Lp1_Channel_Infor[i].scale  * 100), 2);
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                        Copy_String_2(&Str_Data_Write, &Unit_Lpf[8]);  //don vi  
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                        Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Data_Inbuff, &Str_Data_Write);
                        Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
                        break;
                    case 0x03:
                        //ghi obis vao
                        if(FlagAlowPackObis == 0)  //chi ghi obis 1 lan. cac lan sau ghi data
                        {
                            Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Obis_Inbuff, &Str_Ob_lpf[9]);
                            Get_Meter_LProf.Pos_Obis_Inbuff += Str_Ob_lpf[9].Length_u16;
                            Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
                            Get_Meter_LProf.Numqty++;
                        }
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                        Pack_HEXData_Frame(&Str_Data_Write, (int64_t) (Temp_Value* Lp1_Channel_Infor[i].scale  * 100), 2);
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                        Copy_String_2(&Str_Data_Write, &Unit_Lpf[9]);  //don vi
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                        Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Data_Inbuff, &Str_Data_Write);
                        Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16; 
                        break;
                    case 0x04:
                        if(FlagAlowPackObis == 0)  //chi ghi obis 1 lan. cac lan sau ghi data
                        {
                            Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Obis_Inbuff, &Str_Ob_lpf[10]);
                            Get_Meter_LProf.Pos_Obis_Inbuff += Str_Ob_lpf[10].Length_u16;
                            Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
                            Get_Meter_LProf.Numqty++;
                        }
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                        Pack_HEXData_Frame(&Str_Data_Write, (int64_t) (Temp_Value* Lp1_Channel_Infor[i].scale  * 100), 2);
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                        Copy_String_2(&Str_Data_Write, &Unit_Lpf[10]);  //don vi
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                        Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Data_Inbuff, &Str_Data_Write);
                        Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16; 
                        break;  
                    case 0x05:
                        if(FlagAlowPackObis == 0)  //chi ghi obis 1 lan. cac lan sau ghi data
                        {
                            Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Obis_Inbuff, &Str_Ob_lpf[11]);
                            Get_Meter_LProf.Pos_Obis_Inbuff += Str_Ob_lpf[11].Length_u16;
                            Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
                            Get_Meter_LProf.Numqty++;
                        }
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                        Pack_HEXData_Frame(&Str_Data_Write, (int64_t) (Temp_Value* Lp1_Channel_Infor[i].scale * 100), 2);
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                        Copy_String_2(&Str_Data_Write, &Unit_Lpf[11]);  //don vi
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                        Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Data_Inbuff, &Str_Data_Write);
                        Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16; 
                        break;
                    case 0x06:    
                        if(FlagAlowPackObis == 0)  //chi ghi obis 1 lan. cac lan sau ghi data
                        {
                            Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Obis_Inbuff, &Str_Ob_lpf[12]);
                            Get_Meter_LProf.Pos_Obis_Inbuff += Str_Ob_lpf[12].Length_u16;
                            Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
                            Get_Meter_LProf.Numqty++;
                        }
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                        Pack_HEXData_Frame(&Str_Data_Write, (int64_t) ( Temp_Value* Lp1_Channel_Infor[i].scale * 100), 2);   //Convert_FloatPoint_2Float((uint32_t) Temp_Value) 
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                        Copy_String_2(&Str_Data_Write, &Unit_Lpf[12]);  //don vi
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                        Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Data_Inbuff, &Str_Data_Write);
                        Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16; 
                        break;
                    case 0x07:
                        if(FlagAlowPackObis == 0)  //chi ghi obis 1 lan. cac lan sau ghi data
                        {
                            Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Obis_Inbuff, &Str_Ob_lpf[13]);
                            Get_Meter_LProf.Pos_Obis_Inbuff += Str_Ob_lpf[13].Length_u16;
                            Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
                            Get_Meter_LProf.Numqty++;
                        }
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                        Pack_HEXData_Frame(&Str_Data_Write, (int64_t) (Temp_Value* Lp1_Channel_Infor[i].scale * 100), 2);
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                        Copy_String_2(&Str_Data_Write, &Unit_Lpf[13]);  //don vi
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                        Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Data_Inbuff, &Str_Data_Write);
                        Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16; 
                        break;;
                    default:
                        break; 
                }
                
            }
            if(FlagAlowPackObis == 0)  //chi ghi obis 1 lan. cac lan sau ghi data
            {
                Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Obis_Inbuff, &Str_Ob_Tu);  
                Get_Meter_LProf.Pos_Obis_Inbuff += Str_Ob_Tu.Length_u16;
                Get_Meter_LProf.Numqty++;
                Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Obis_Inbuff, &Str_Ob_Ti);  
                Get_Meter_LProf.Pos_Obis_Inbuff += Str_Ob_Ti.Length_u16;
                Get_Meter_LProf.Numqty++;
                Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Obis_Inbuff, &He_So_Nhan);  
                Get_Meter_LProf.Pos_Obis_Inbuff += He_So_Nhan.Length_u16;
                Get_Meter_LProf.Numqty++;
                Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
            }
            Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Data_Inbuff, &Get_Meter_TuTi.Str_Payload);
            Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
            
            FlagAlowPackObis = 1;  //khong cho ghi obis vao nua
            Count++;
//            if((Get_Meter_LProf.Str_Payload.Length_u16 >= 900) || (Count >= recordInfo.NumReadone))
//            {
                 //lay het data moi ghep stime lpf vao
                Reset_Buff(&Str_Data_Write);
                 *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
                Copy_String_STime(&Str_Data_Write, sTimeTemp); 
                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')'; 
                
                //event
                Copy_String_2(&Str_Data_Write, &Str_event_Temp);
                //period
                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
                Pack_HEXData_Frame_Uint64(&Str_Data_Write, recordInfo.Interval/60 ,0);
                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')'; 
                //num chanel
                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
                Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) Get_Meter_LProf.Numqty, 0);  
                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')'; 
                
                Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.PosNumqty, &Str_Data_Write);
        
                Send_MessLpf_toQueue();
//                if(Send_MessLpf_toQueue() == 0) return 0;
                Get_Meter_LProf.Flag_Start_Pack = 0;
//            }else
//            {
//                *(Get_Meter_LProf.Str_Payload.Data_a8 + Get_Meter_LProf.Str_Payload.Length_u16++) = 0x0D;
//                *(Get_Meter_LProf.Str_Payload.Data_a8 + Get_Meter_LProf.Str_Payload.Length_u16++) = 0x0A;
//                Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
//            }  
        }
		return 1;
	}
	return 0;
}


uint8_t getTagByChannelName(char* channelName) 
{
	uint8_t tag = 0xFF;
	
	if (memcmp((uint8_t*) channelName, (uint8_t*) "Avg Watts Total", strlen(channelName)) == 0) 
	{
		tag = 0xFF;
	} else if (memcmp((uint8_t*) channelName, (uint8_t*) "Avg var Total", strlen(channelName)) == 0) 
	{
		tag = 0xFF;
	} else if (memcmp((uint8_t*) channelName, (uint8_t*) "Avg VA Total", strlen(channelName)) == 0) 
	{
		tag = 0xFF;   
	} else if (memcmp((uint8_t*) channelName, (uint8_t*) "Record Status", strlen(channelName)) == 0) 
	{
		tag = 0xFF;
	} else if (memcmp((uint8_t*) channelName, (uint8_t*) "Export Wh Total", strlen(channelName)) == 0) 
	{
		tag = 0x27;   //tam doi. k bie ansi hay IEC
	} else if (memcmp((uint8_t*) channelName, (uint8_t*) "Import Wh Total", strlen(channelName)) == 0) 
	{
		tag = 0x28;
	} else if (memcmp((uint8_t*) channelName, (uint8_t*) "Export varh Total", strlen(channelName)) == 0) 
	{
		tag = 0x29;
	} else if (memcmp((uint8_t*) channelName, (uint8_t*) "Import varh Total", strlen(channelName)) == 0) 
	{
		tag = 0x2B;
	} else if (memcmp((uint8_t*) channelName, (uint8_t*) "Avg Voltage Ph-A", strlen(channelName)) == 0) 
	{
		tag = 0x02;
	} else if (memcmp((uint8_t*) channelName, (uint8_t*) "Avg Voltage Ph-B", strlen(channelName)) == 0) 
	{
		tag = 0x03;
	} else if (memcmp((uint8_t*) channelName, (uint8_t*) "Avg Voltage Ph-C", strlen(channelName)) == 0) 
	{
		tag = 0x04;
	} else if (memcmp((uint8_t*) channelName, (uint8_t*) "Avg Current Ph-A", strlen(channelName)) == 0) 
	{
		tag = 0x05;
	} else if (memcmp((uint8_t*) channelName, (uint8_t*) "Avg Current Ph-B", strlen(channelName)) == 0) 
	{
		tag = 0x06;
	} else if (memcmp((uint8_t*) channelName, (uint8_t*) "Avg Current Ph-C", strlen(channelName)) == 0) 
	{
		tag = 0x07;
	} else if (memcmp((uint8_t*) channelName, (uint8_t*) "Avg Power Factor Total", strlen(channelName)) == 0) 
	{
		tag = 0xFF;
	} else if (memcmp((uint8_t*) channelName, (uint8_t*) "Avg Exp PF Total", strlen(channelName)) == 0) 
	{
		tag = 0xFF;
	}

	return tag;
}


uint8_t Genius_Read_Loadprofile (void)   
{
    uint32_t StartTimeRecord = 0;
    uint32_t TimeStart = 0;
    uint32_t TimeStop = 0;
    uint32_t LastTime = 0;
    uint8_t result = 0;
    uint8_t     FlagOverTime = 0;
    
    //lay Tu Ti
    if(Genius_Read_TuTi() == 0) return 0;
    //
	if(Read_Step_Lp(LP1_GET_RECORD_INFOR) == 1)
	{
        recordInfo.NumReadone = 1;
        recordInfo.currentInd = 0;
        
        if(sInformation.Flag_Request_lpf == 1)
        {
            //chuyen khoang thoi gian kia thanh num record doc  
            StartTimeRecord = HW_RTC_GetCalendarValue_Second(recordInfo.sTimeStart, 3);
            TimeStart = HW_RTC_GetCalendarValue_Second(sInformation.StartTime_GetLpf, 3)/recordInfo.Interval*recordInfo.Interval + recordInfo.Interval;
            TimeStop =  HW_RTC_GetCalendarValue_Second(sInformation.EndTime_GetLpf, 3);
            LastTime =  StartTimeRecord + (recordInfo.Number_Entries - 1) *recordInfo.Interval;   //tesst lai tinh time. 
            
            if(TimeStart < StartTimeRecord) TimeStart = StartTimeRecord;
            if((TimeStart > LastTime) || (TimeStart > TimeStop)) return 0;
            if(TimeStop > LastTime) 
            {
                TimeStop = LastTime;
                FlagOverTime = 1;
            }
            
            recordInfo.startRecordIndex = (recordInfo.Number_Entries - 1) - (LastTime - TimeStart)/recordInfo.Interval;
            recordInfo.NumRecordRead = (TimeStop - TimeStart + FlagOverTime)/recordInfo.Interval;
        }else 
        {
            recordInfo.NumRecordRead = 86400/recordInfo.Interval;
            recordInfo.startRecordIndex = recordInfo.Number_Entries - recordInfo.NumRecordRead;
        }
    
		if(Read_Step_Lp(LP1_GET_CHANNEL_INFOR) == 1)
        {
            while(recordInfo.currentInd < recordInfo.NumRecordRead)
            {
                if(recordInfo.currentInd < (recordInfo.NumRecordRead - 6)) 
                  recordInfo.NumReadone = 6;
                else recordInfo.NumReadone = recordInfo.NumRecordRead - recordInfo.currentInd;         //gioi han doc cu 1 lan toi da 5 record. buf uart 800 byte. size record 18 byte
                  
                Read_Step_Lp(LP1_READ_DATA); 
                recordInfo.currentInd += recordInfo.NumReadone;
            }
        }
        result = 1;   //neu dung ca lp2 thi comment day
	}
    
    //
//    if(Read_Step_Lp(LP2_GET_RECORD_INFOR) == 1)
//	{
//            recordInfo.NumReadone = 1;
//        recordInfo.currentInd = 0;
//        if(sInformation.Flag_Request_lpf == 1)
//        {
//            //chuyen khoang thoi gian kia thanh num record doc 
//            StartTimeRecord = HW_RTC_GetCalendarValue_Second(recordInfo.sTimeStart, 3);
//            TimeStart = HW_RTC_GetCalendarValue_Second(sInformation.StartTime_GetLpf, 3);
//            TimeStop =  HW_RTC_GetCalendarValue_Second(sInformation.EndTime_GetLpf, 3);
//            LastTime =  StartTimeRecord + (recordInfo.Number_Entries - 1) *recordInfo.Interval;   //interval tinh theo s
//            
//            if(TimeStart < StartTimeRecord) TimeStart = StartTimeRecord;
//            if((TimeStart > LastTime) || (TimeStart > TimeStop)) return 0;
//            if(TimeStop > LastTime) TimeStop = LastTime;
//            
//            recordInfo.startRecordIndex = recordInfo.Number_Entries - (LastTime - TimeStart)/recordInfo.Interval;
//            recordInfo.NumRecordRead = (TimeStop - TimeStart)/recordInfo.Interval;
//            if(recordInfo.NumRecordRead == 0)
//            {
//                //dong goi tin rong gui len
//              
//            }
//        }else recordInfo.NumRecordRead = 86400/recordInfo.Interval;;
//        
//		if(Read_Step_Lp(LP2_GET_CHANNEL_INFOR) == 1)
//		{
//            sInformation.Flag_Request_lpf = 0;
//			if(Read_Step_Lp(LP2_READ_DATA) == 1) return 1;
//		}
//	}
    
    sInformation.Flag_Request_lpf = 0;
	return result;
}

int64_t packLoadProfileImportExportData(uint8_t* data, float scaleFactor, uint8_t Scale) 
{
	uint32_t     dataValue = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
	int64_t dataFinal = 0;

	if (dataValue != 0) 
	{
		uint32_t scaleValue = 1;
		for (uint32_t i = 0; i < Scale; i++) 
		{
			scaleValue *= 10;
		}

		double dTmp = (double) dataValue * scaleFactor * scaleValue;
		dataFinal = (int64_t) dTmp;
        return dataFinal;
	}
    
    return dataValue;
}




int32_t packLoadProfileVoltageCurrentData(uint8_t* data, float scaleFactor, uint8_t Scale) 
{
	int32_t dataValue = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];

	if (dataValue != 0) 
    {
		uint32_t scaleValue = 1;
		for (uint32_t i = 0; i < Scale; i++) 
        {
			scaleValue *= 10;
		}

		float dTmp = (float) dataValue * scaleFactor * scaleValue;
		dataValue = (int32_t) dTmp;
        return dataValue;
	}

	return 0;
}

//
void Genius_SetTime_GetBill (void)
{
    if(sInformation.Flag_Request_Bill != 1)
    {
        sInformation.EndTime_GetBill.year     = sRTC.year;  //lay start time truoc 1 thang. con stop time la thoi diem hien tai
        sInformation.EndTime_GetBill.month    = sRTC.month;
        sInformation.EndTime_GetBill.date     = sRTC.date;
        sInformation.EndTime_GetBill.hour     = sRTC.hour;
        sInformation.EndTime_GetBill.min      = sRTC.min;
        sInformation.EndTime_GetBill.sec      = sRTC.sec; 
        
        if (sRTC.month == 1)
        {
            sInformation.StartTime_GetBill.year     = (sRTC.year + 99)%100;  //lay start time truoc 1 thang. 
            sInformation.StartTime_GetBill.month    = 12;
            sInformation.StartTime_GetBill.date     = sRTC.date;
            sInformation.StartTime_GetBill.hour     = sRTC.hour;
            sInformation.StartTime_GetBill.min      = sRTC.min;
            sInformation.StartTime_GetBill.sec      = sRTC.sec; 
        }
        else
        {
            sInformation.StartTime_GetBill.year     = sRTC.year;  //lay start time truoc 1 thang. con stop time la thoi diem hien tai
            sInformation.StartTime_GetBill.month    = sRTC.month - 1;
            sInformation.StartTime_GetBill.date     = sRTC.date;
            sInformation.StartTime_GetBill.hour     = sRTC.hour;
            sInformation.StartTime_GetBill.min      = sRTC.min;
            sInformation.StartTime_GetBill.sec      = sRTC.sec;                 
        }
    }
    sInformation.Flag_Request_Bill = 0;
}

uint8_t Genius_Read_Billing (void)   
{
    uint8_t Result = 0;
    
    //lay Tu Ti
    if(Genius_Read_TuTi() == 0) 
        return 0;
    //
	if(Read_Step_Billing(Bill_GET_RECORD_INFOR) == 1)
	{
   		if(Read_Step_Billing(Bill_GET_CHANNEL_INFOR) == 1)
        {
            //Tinh lai Stime start va sTime Stop ke ca 2 truong hop co request va k request
            Genius_SetTime_GetBill();
            //
            recordBIll_Inf.startRecordIndex = 0;
            recordBIll_Inf.currentInd = 0;
            while(recordBIll_Inf.currentInd < recordBIll_Inf.Number_Entries)
            {
                Init_Meter_Billing_Struct();
                Read_Step_Billing(Bill_READ_DATA);
                recordBIll_Inf.currentInd++;
            }
            //truong hop k doc duoc ban ghi nao. can gui len goi tin rong
            if(recordBIll_Inf.ToTalRead == 0)   //Flag nay da reset khi doc infor record roi
            {
                if(Get_Meter_Billing.Flag_Start_Pack == 0)
                {
                    Packet_Empty_MessHistorical();
                }
            }
            Result =  1;
        }
	}
   
	return Result;
}

uint8_t Read_Step_Billing (uint8_t Step)
{
	int 		Pos_Find = 0;
	uint8_t 	length = 0;
	uint8_t 	i = 0;
	uint8_t 	Count = 0;
	uint8_t 	j = 0, result = 0;
	
	//Reset lai uart nhan
	Init_UART2();

    if(Genius_Handshake_Handle() == 1)
    {
        //truyen lenh doc cac thanh ghi Infor Record
        if(Step == Bill_GET_CHANNEL_INFOR)
        {
            for(i = 0 ; i < MAX_LENGHT_ALL_REG_INFOR; i++)
                lp1_All_ChannelInfoRegList[i] = 0;
            //
            for (Count = 0; Count < (recordBIll_Inf.noChannel + 1); Count++)    //1channel status nua
                for (j = 0; j < genius_BillingList[Step].Number_Reg; j++)
                    lp1_All_ChannelInfoRegList[length++] = *(genius_BillingList[Step].Buff_List_Reg + j) + Count;

            cmd_generateCommand(genius_BillingList[Step].cmdType, 0, &lp1_All_ChannelInfoRegList[0], length, 0, 0, 0);
        }else if(Step == Bill_READ_DATA)
        {
            cmd_generateCommand(genius_BillingList[Step].cmdType, (uint32_t) *(genius_BillingList[Step].Buff_List_Reg) ,NULL, 0, 
                                recordBIll_Inf.startRecordIndex + recordBIll_Inf.currentInd, 1, recordBIll_Inf.recordSize);  
        }else
            cmd_generateCommand(genius_BillingList[Step].cmdType, 0,genius_BillingList[Step].Buff_List_Reg, genius_BillingList[Step].Number_Reg, 0, 0, 0);        
        
        if (osSemaphoreWait(bsUART2PendingMessHandle,5000) == osOK) 
        {
            //Convert lai chuoi nhan duoc neu co cac ki tu dac biet
            Convert_Genius_Respond (&UART1_Control.UART1_Str_Recei);
            //Check Crc
            if(Genius_Check_Sum(&UART1_Control.UART1_Str_Recei) == 0)
                return 0;
            
            Pos_Find = Find_String_V2((truct_String*) &genius_BillingList[Step].Str_Recei, &UART1_Control.UART1_Str_Recei);
            //cat data instant
            if(Pos_Find >= 0)
            {
                Pos_Find += genius_BillingList[Step].Str_Recei.Length_u16;
                //check length
                if(genius_BillingList[Step].Length_Data == 0xFFFF)   //kh�ng c�n check length data nh�n ve
                    result = genius_BillingList[Step].CallBack(&UART1_Control.UART1_Str_Recei, Pos_Find);
                else
                    if((UART1_Control.UART1_Str_Recei.Length_u16 - Pos_Find) == (genius_BillingList[Step].Length_Data + 3))
                       result = genius_BillingList[Step].CallBack(&UART1_Control.UART1_Str_Recei, Pos_Find);
            }
        }
    }
	return result;
}

uint8_t onSuccessGetRecordBill_Inf (truct_String* Str, uint16_t Pos)
{
	//Get load survey mode
	recordBIll_Inf.modeLoadSurvey  = *(Str->Data_a8 + Pos );
	Pos++;
	//Get no channel
	recordBIll_Inf.noChannel = *(Str->Data_a8 + Pos );
	Pos++;
	//Get record size
	recordBIll_Inf.recordSize = (*(Str->Data_a8 + Pos ) << 8) + *(Str->Data_a8 + Pos + 1);
	Pos += 2;
	//Get no stored record
	recordBIll_Inf.Number_Entries = (*(Str->Data_a8 + Pos ) << 24) + (*(Str->Data_a8 + Pos + 1 ) << 16) + (*(Str->Data_a8 + Pos + 2 ) << 8) + *(Str->Data_a8 + Pos + 3) ;
    //reset var
    recordBIll_Inf.startRecordIndex = 0;
    recordBIll_Inf.currentInd = 0;
    recordBIll_Inf.ToTalRead = 0;
    return 1;
}

uint8_t onSuccessGetChannelBill_Inf (truct_String* Str, uint16_t Pos)
{
    //them 1 channel status
	int loopTmp = 0;
	uint32_t Temp_Scale = 0;
	
	for (loopTmp = 0; (loopTmp < recordBIll_Inf.noChannel) && ((Pos + 4) < (Str->Length_u16 - 3)); loopTmp++)   //kiem tra lai nochannel
	{
		//Get channel register ID
		Bill_Channel_Infor[loopTmp].regID = (*(Str->Data_a8 + Pos ) << 24) | (*(Str->Data_a8 + Pos + 1 ) << 16) | (*(Str->Data_a8 + Pos + 2 ) << 8) | *(Str->Data_a8 + Pos + 3 );
		Pos += 4;
		//Get channel size
		Bill_Channel_Infor[loopTmp].size = (*(Str->Data_a8 + Pos ) << 8) + *(Str->Data_a8 + Pos + 1);
		Pos += 2;
		//Skip type and unit
		Pos += 2;

		//Skip channel name
		int dataTmp = strlen((char*) (Str->Data_a8 + Pos));
		Pos += (dataTmp + 1);

		//Get channel offset
		Bill_Channel_Infor[loopTmp].offset = (*(Str->Data_a8 + Pos ) << 8) + *(Str->Data_a8 + Pos + 1);
		Pos += 2;

		//Skip scale
		Pos++;
		Temp_Scale = (*(Str->Data_a8 + Pos ) << 24) | (*(Str->Data_a8 + Pos + 1 ) << 16) | (*(Str->Data_a8 + Pos + 2 ) << 8) | *(Str->Data_a8 + Pos + 3 );
		//Get Scale factor
		Bill_Channel_Infor[loopTmp].scale = Convert_FloatPoint_2Float (Temp_Scale);
		Pos += 4;
	}
	return 1;
}

uint8_t onSuccessReadBill (truct_String* Str, uint16_t Pos)  //thu doc tung record 1 thoi
{
    uint8_t         i = 0, Result = 0, j = 0;
    uint64_t        dataValue = 0;  //int64_t
	uint32_t 	    Actual_Record_Read = 0;
	
    uint8_t         BuffNum[30];
	truct_String    Str_Data_Write={&BuffNum[0], 0};
    
    ST_TIME_FORMAT		sTimeTemp;
    uint32_t            resetBillingTimeStamp = 0;
    uint8_t             OffsetTemp = 0;
    uint32_t            TagTemp = 0;
    int8_t              RowObis = 0;
    ST_TIME_FORMAT      sTime_MaxD;
    uint8_t             Temp_BBC = 0,  Temp = 0; 
    double dTmp = 0;
      
	if((Str->Length_u16 - Pos) == (recordBIll_Inf.recordSize + 10 + 3))    //4byte Actual Start Re + 2 byte no Re + 2 byte offset + 2 byte Size
	{
		//check xem tra ve Actual Start Re co dung khong
		Actual_Record_Read = (*(Str->Data_a8 + Pos ) << 24) | (*(Str->Data_a8 + Pos + 1 ) << 16) | (*(Str->Data_a8 + Pos + 2 ) << 8) | *(Str->Data_a8 + Pos + 3 );
		if(Actual_Record_Read != (recordBIll_Inf.startRecordIndex + recordBIll_Inf.currentInd)) return 0;
		//Test lai dk nay xem
		Pos += 10;
		//cat data theo vi tri offset va size
		resetBillingTimeStamp = (*(Str->Data_a8 + Pos + 2) << 24) | (*(Str->Data_a8 + Pos + 3 ) << 16) | (*(Str->Data_a8 + Pos + 4 ) << 8) | *(Str->Data_a8 + Pos + 5 );
        
        if(genius_isResetBillingTimeInRange(resetBillingTimeStamp, &sTimeTemp) == 1)
        {  
            recordBIll_Inf.ToTalRead++;
            //stime ti nua se dong goi
            if(Get_Meter_Billing.Flag_Start_Pack == 0)
            {
                Reset_Buff(&Get_Meter_Billing.Str_Payload);
                //wrire header TSVH vao
                Write_Header_His_Push103();
                Get_Meter_Billing.Flag_Start_Pack = 1;
            }
            //Bo qua channel status
            for(i = 2; i < recordBIll_Inf.noChannel - 1; i++)   //trong soure code l� -1
            {
                OffsetTemp = Bill_Channel_Infor[i].offset;
                TagTemp = genius_getTag(Bill_Channel_Infor[i].regID);
                
                dataValue = 0;
                if(TagTemp != 0)
                {
                    if(TagTemp <= 0x31)
                    {
                        if (Bill_Channel_Infor[i].size == 8) 
						{  
                            for (j = 0; j < 8; j++)
                            {
                                Temp = UART1_Receive_Buff[Pos + OffsetTemp + j];
                                dataValue |= ((uint64_t) Temp) << (8*(7-j));                   //moi sua 28_10_21
                            }
						}else if (Bill_Channel_Infor[i].size == 4)
						{ 
							for(j = 0; j < 4; j++)
                                dataValue = (dataValue << 8) + UART1_Receive_Buff[Pos + OffsetTemp + j];
						}
                        
                        if (dataValue != 0) 
						{ 
							uint32_t scaleValue = 1;
//							for (uint32_t count = 0; count < GENIUS_SCALE_BILL; count++)    //comment day de. fix lai unit la kwh va kw.
//								scaleValue *= 10; // nhan 1000

							dTmp = dataValue * Bill_Channel_Infor[i].scale * scaleValue;   //kiem tra xem co phai convert tu float point sang float k
                            dataValue = (int64_t) dTmp; 
                        }
                        RowObis = Genius_Check_Row_Obis103(&Obis_Genius[0], TagTemp);
                        {
                            if(RowObis >= 0)
                            {
                                Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Obis_Inbuff, Obis_Genius[RowObis].StrObis_Bill);
                                Get_Meter_Billing.Pos_Obis_Inbuff += Obis_Genius[RowObis].StrObis_Bill->Length_u16;
                                Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
                                Get_Meter_Billing.Numqty++;
                                Reset_Buff(&Str_Data_Write);
                                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                                Pack_HEXData_Frame(&Str_Data_Write, (int64_t) dataValue, GENIUS_SCALE_BILL);
                                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                                Copy_String_2(&Str_Data_Write, Obis_Genius[RowObis].Str_Unit);  //don vi
                                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                                Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Data_Inbuff, &Str_Data_Write);
                                Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16; 
                            }
                        }
                    }else  //maxdemand plus
                    {
                        for(j = 0; j < 4; j++)
                            dataValue = dataValue << 8 | UART1_Receive_Buff[Pos + OffsetTemp + j];

                        if (dataValue != 0) 
						{ 
							uint32_t scaleValue = 1;
//							for (uint32_t count = 0; count < GENIUS_SCALE_BILL; count++)  //fix lai unit dai luong maxdemand la kw.
//								scaleValue *= 10; // nhan 1000

							dTmp = dataValue * Bill_Channel_Infor[i].scale * scaleValue;   //kiem tra xem co phai convert tu float point sang float k
                            dataValue = (int64_t) dTmp; 
                        }
                        RowObis = Genius_Check_Row_Obis103(&Obis_Genius[0], TagTemp);
                        {
                            if(RowObis >= 0)
                            {
                                Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Obis_Inbuff, Obis_Genius[RowObis].StrObis_Bill);
                                Get_Meter_Billing.Pos_Obis_Inbuff += Obis_Genius[RowObis].StrObis_Bill->Length_u16;
                                Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
                                Get_Meter_Billing.Numqty++;
                                Reset_Buff(&Str_Data_Write);
                                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                                Pack_HEXData_Frame(&Str_Data_Write, (int64_t) dataValue, GENIUS_SCALE_BILL);
                                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                                Copy_String_2(&Str_Data_Write, Obis_Genius[RowObis].Str_Unit);  //don vi
                                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';                                
                                //time
                                dataValue = 0;
                                for(j = 4; j < 8; j++)
                                    dataValue = dataValue << 8 | UART1_Receive_Buff[Pos + OffsetTemp + j]; 
                                Epoch_to_date_time(&sTime_MaxD, dataValue, 3);

                                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                                Copy_String_STime(&Str_Data_Write,sTime_MaxD);
                                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                                Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Data_Inbuff, &Str_Data_Write);
                                Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
                            }
                        }
                    }                 
                }
            }
            //dong goi gui len Hes
            Add_TuTI_toPayload(&Get_Meter_Billing);
            //lay het data moi ghep stime Bill vao
            Reset_Buff(&Str_Data_Write);
            
            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
            Copy_String_STime(&Str_Data_Write, sTimeTemp);  //thoi gian
            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')'; 
        
            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
            Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) Get_Meter_Billing.Numqty, 0);
            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')'; 
            
            Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.PosNumqty, &Str_Data_Write);
            //ETX
            *(Get_Meter_Billing.Str_Payload.Data_a8 + Get_Meter_Billing.Str_Payload.Length_u16++) = ETX; 
            //BBC
            Temp_BBC = BBC_Cacul(Get_Meter_Billing.Str_Payload.Data_a8 + 1,Get_Meter_Billing.Str_Payload.Length_u16 - 1);
            *(Get_Meter_Billing.Str_Payload.Data_a8 + Get_Meter_Billing.Str_Payload.Length_u16++) = Temp_BBC;
            
            Push_Bill_toQueue(DATA_HISTORICAL);
            Result = 1;
        }
	}
	return Result;
}



uint8_t genius_isResetBillingTimeInRange(uint32_t timeStamp, ST_TIME_FORMAT *p_timeMeter)
{
	uint8_t         Result = 0;
	uint32_t        diffTimeStampStart = 0;
    uint32_t        diffTimeStampEnd = 0;;
	uint32_t        status1, status2;
//    uint8_t         diffmonth = 0;

	Epoch_to_date_time(p_timeMeter, timeStamp, 3); // Convert tu timestamp to sTime
	// So sanh 2 khoang thoi gian

    status1 = Check_DiffTime(*p_timeMeter, sInformation.StartTime_GetBill, &diffTimeStampStart);  
    status2 = Check_DiffTime(sInformation.EndTime_GetBill, *p_timeMeter, &diffTimeStampEnd);
    
    if ((status1 > 0) && (status2 > 0)) 
    {
        Result = 1;
    }else
    {
//        //neu nho hon start thi doc tiep. lon hon stop thi dung doc
//        diffmonth = (diffTimeStampStart/86400)/31;   //cu lay thang lon nhat 31 ngay. Neu no cach xa hon n thang thi tang index len (n-1). de ti thoat ra ngoai tang 1 nua
//        if(diffmonth >= 2)
//          recordBIll_Inf.currentInd += diffmonth -1;
        if(diffTimeStampEnd > 0) 
          recordBIll_Inf.currentInd = recordBIll_Inf.Number_Entries;
    }
      
	return Result;
}



uint8_t genius_getTag(uint32_t regIDFind)
{
	genius_regIDTagMapping_t* p_regIDMapping = (genius_regIDTagMapping_t*) regIDTagMappingList;

	while ((p_regIDMapping->regID != 0) && (p_regIDMapping->regID != regIDFind)) 
    {
		p_regIDMapping++;
	}

	return p_regIDMapping->tag;
}


int8_t Genius_Check_Row_Obis103 (struct_Obis_Genius* structObis, uint8_t ObisHex)
{
    uint8_t  i = 0;
    
    for(i = 0; i < MAX_OBIS_GENIUS; i++)
      if(structObis[i].ObisHex == ObisHex) return i; 
    
    return -1;
}


//Event

uint8_t Genius_Read_event (void)
{
    uint8_t         BuffNum[15];
	truct_String    Str_Data_Write={&BuffNum[0], 0};
    uint8_t         Temp_BBC = 0;
    //lay Tu Ti
    if(Genius_Read_TuTi() == 0) 
        return 0;
    //
    if(Genius_ReadInfor_1(&Genius_IDRegisEvent[0], 3, &Get_Meter_Event) == 1)
    {
        if(Get_Meter_Event.Flag_Start_Pack == 0)
        {
            Reset_Buff(&Get_Meter_Event.Str_Payload);
            Header_event_103(&Get_Meter_Event.Str_Payload, 1);
            Get_Meter_Event.Flag_Start_Pack = 1;
        }

        Genius_Extract_Data_Event(&Get_Meter_Event);
        //Day du lieu len Hes
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
    
    return 1;
}
 

uint8_t Genius_Read_event_TSVH (void)
{
	truct_String    StrTemp;

    if(Genius_ReadInfor_1(&Genius_IDRegisEvent[0], 3, &Get_Meter_Event) == 0) return 0;
        Genius_Extract_Data_Event(&Get_Meter_Info);

    //copy sang buff TSVH
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
    return 0;
}
       
uint8_t Genius_Extract_Data_Event (Meter_Comm_Struct* Get_Met)
{
    Genius_GetData_Event(Get_Met, 10, 4, 0, StrobisEvent[0]);
    Genius_GetData_Event(Get_Met, 14, 6, 1, StrobisEvent[1]);
    Genius_GetData_Event(Get_Met, 20, 6, 1, StrobisEvent[2]);
    return 1;
}
       
void Genius_GetData_Event (Meter_Comm_Struct* Get_Met, uint16_t Pos, uint8_t length, uint8_t type, truct_String oBIS)
{
    uint16_t i = 0;
    uint32_t TempU32 = 0;
    ST_TIME_FORMAT  sTimetemp;
    uint8_t         BuffNum[15];
	truct_String    Str_Data_Write={&BuffNum[0], 0};
    
    if(type == 0)
    {
        for(i = 0; i<length; i++)
            TempU32 = (TempU32 <<8) | Meter_TempBuff[Pos+i];
        
        //ghi obis va data vao
        Copy_String_toTaget(&Get_Met->Str_Payload, Get_Met->Pos_Obis_Inbuff, &oBIS);
        Get_Met->Pos_Obis_Inbuff += oBIS.Length_u16;
        Get_Met->Pos_Data_Inbuff = Get_Met->Str_Payload.Length_u16;
        if(oBIS.Length_u16 != 0) Get_Met->Numqty ++;
        
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
        Pack_HEXData_Frame_Uint64(&Str_Data_Write, TempU32, 0);
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';

        Copy_String_toTaget(&Get_Met->Str_Payload, Get_Met->Pos_Data_Inbuff, &Str_Data_Write);
        Get_Met->Pos_Data_Inbuff = Get_Met->Str_Payload.Length_u16;
    }else if (type == 1)
    {   
        //ghi obis va data vao
        Copy_String_toTaget(&Get_Met->Str_Payload, Get_Met->Pos_Obis_Inbuff, &oBIS);
        Get_Met->Pos_Obis_Inbuff += oBIS.Length_u16;
        Get_Met->Pos_Data_Inbuff = Get_Met->Str_Payload.Length_u16;
        if(oBIS.Length_u16 != 0) Get_Met->Numqty ++;
        
        sTimetemp.date  = Meter_TempBuff[Pos+0];
        sTimetemp.month = Meter_TempBuff[Pos+1];
        sTimetemp.year  = Meter_TempBuff[Pos+2];
        sTimetemp.hour  = Meter_TempBuff[Pos+3];
        sTimetemp.min   = Meter_TempBuff[Pos+4];
        sTimetemp.sec   = Meter_TempBuff[Pos+5];
        
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
        Copy_String_STime(&Str_Data_Write, sTimetemp);
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
        
        Copy_String_toTaget(&Get_Met->Str_Payload, Get_Met->Pos_Data_Inbuff, &Str_Data_Write);
        Get_Met->Pos_Data_Inbuff = Get_Met->Str_Payload.Length_u16;
    }
}
       
 //read Tu TI Tum Tim
uint8_t Genius_Read_TuTi (void)
{
    Init_Meter_TuTi_Struct();
    if(Genius_ReadInfor_1(&Genius_IDRegisIntan_Table[32], 4, &Get_Meter_TuTi) == 1)
    {
        Genius_ExtractMTuTi_data();
        *(Get_Meter_TuTi.Str_Payload.Data_a8 + Get_Meter_TuTi.Str_Payload.Length_u16++) = '(';
        Pack_HEXData_Frame_Uint64(&Get_Meter_TuTi.Str_Payload, sDCU.He_So_Nhan, 0);
        *(Get_Meter_TuTi.Str_Payload.Data_a8 + Get_Meter_TuTi.Str_Payload.Length_u16++) = ')';
        return 1;
    }
    return 0;
}
       
void Genius_ExtractMTuTi_data (void)
{
    //Tu Ti
    Genius_MTuTi_value(18, 4, 0, Str_Ob_Tu);
    Genius_MTuTi_value(22, 4, 1, Str_Ob_Tu);
    Genius_MTuTi_value(10, 4, 0, Str_Ob_Ti);       
    Genius_MTuTi_value(14, 4, 1, Str_Ob_Ti); 
}

void Genius_MTuTi_value(uint16_t Pos, uint8_t length, uint8_t type, truct_String oBIS)
{
    uint16_t i = 0;
    uint32_t TempU32 = 0;
    
    for(i = 0; i<length; i++)
        TempU32 = (TempU32 <<8) | Meter_TempBuff[Pos+i];
    
    if(type == 0)
    {
        *(Get_Meter_TuTi.Str_Payload.Data_a8 + Get_Meter_TuTi.Str_Payload.Length_u16++) = '(';
        Pack_HEXData_Frame_Uint64(&Get_Meter_TuTi.Str_Payload, (uint64_t)Convert_FloatPoint_2Float(TempU32), 0);
        *(Get_Meter_TuTi.Str_Payload.Data_a8 + Get_Meter_TuTi.Str_Payload.Length_u16++) = ')';
    }else
    {
        Get_Meter_TuTi.Str_Payload.Length_u16--;
        *(Get_Meter_TuTi.Str_Payload.Data_a8 + Get_Meter_TuTi.Str_Payload.Length_u16++) = '/';
        Pack_HEXData_Frame_Uint64(&Get_Meter_TuTi.Str_Payload, (uint64_t)Convert_FloatPoint_2Float(TempU32), 0);
        *(Get_Meter_TuTi.Str_Payload.Data_a8 + Get_Meter_TuTi.Str_Payload.Length_u16++) = ')';
    }
}
       
