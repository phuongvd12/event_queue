
#include "variable.h"
#include "t_mqtt.h"
#include "sim900.h"

#include "START_Init_Meter.h"
#include "START_Read_Meter.h"

#include "at_commands.h"
#include "pushdata.h"

#include "myDefine.h"

#define UNDER_COMP	0
#define OVER_COMP	1




void START_Init_Function (uint8_t type)
{
    eMeter_20._f_Read_ID            = START_Get_Meter_ID;
    eMeter_20._f_Check_Reset_Meter  = START_CheckResetReadMeter;
  
    eMeter_20._f_Connect_Meter      = START_Connect_Meter;
    eMeter_20._f_Read_TSVH          = START_Read_TSVH;
    eMeter_20._f_Read_Bill          = START_Read_Bill;
    eMeter_20._f_Read_Event         = START_Read_Event;  
    eMeter_20._f_Read_Lpf           = START_Read_Lpf;
    eMeter_20._f_Read_InforMeter    = START_Read_Infor;
    eMeter_20._f_Get_UartData       = START_GetUART2Data;
    eMeter_20._f_Check_Meter        = START_Check_Meter;
    eMeter_20._f_Test1Cmd_Respond   = START_Send1Cmd_Test;
}

uint8_t START_Send1Cmd_Test (void)
{
    HAL_UART_Transmit(&UART_METER,&Read_Meter_ID_Message[0],17,2000);
    
    osSemaphoreWait(bsUART2PendingMessHandle,5000);
    
    Init_UART2();
    if(UART1_Control.fRecei_Respond == 1) 
      return 1;
    
    return 0;
}

uint8_t START_Read_TSVH (uint32_t Type)
{
    uint8_t         Temp_BBC = 0;
    truct_String    Str_Data_Write = {&Buff_Temp1[0], 0};
    truct_String    StrTemp;
    
    Init_Meter_TuTi_Struct();
    if(Meter_Comm_Func(&Get_Meter_TuTi,DATA_TUTI,&START_MeterTiTuIDTable[0],&RMTuTi_Extract_Data,&RMI_Push_Mess_TuTi) != 1) 
    {
        Init_Meter_TuTi_Struct();
        if(Meter_Comm_Func(&Get_Meter_TuTi,DATA_TUTI,&START_MeterTiTuIDTable[0],&RMTuTi_Extract_Data,&RMI_Push_Mess_TuTi) != 1) 
        {
            Init_Meter_TuTi_Struct();
            if(Meter_Comm_Func(&Get_Meter_TuTi,DATA_TUTI,&START_MeterTiTuIDTable[0],&RMTuTi_Extract_Data,&RMI_Push_Mess_TuTi) != 1) 
              return 0;
        }
    }
    
    Init_Meter_Info_Struct();
    Init_MD_Bill();
    if(Meter_Comm_Func(&Get_Meter_Info,DATA_OPERATION,&START_MeterInfoMessIDTable[0],&RMI_Extract_Data,&RMI_Push_Mess) != 1)
    {
        Init_Meter_Info_Struct();
        if(Meter_Comm_Func(&Get_Meter_Info,DATA_OPERATION,&START_MeterInfoMessIDTable[0],&RMI_Extract_Data,&RMI_Push_Mess) != 1)
        {   
            Init_Meter_Info_Struct();
            if(Meter_Comm_Func(&Get_Meter_Info,DATA_OPERATION,&START_MeterInfoMessIDTable[0],&RMI_Extract_Data,&RMI_Push_Mess)!= 1) return 0;
        }
    }
        
    Pack_MD_Value_Time(DATA_OPERATION, &Get_Meter_Info, &MD_Plus_Bill, 0x2E, &Obis_Start_Ins[0], START_MAX_OBIS_INTAN);
    Pack_MD_Value_Time(DATA_OPERATION, &Get_Meter_Info, &MD_Sub_Bill, 0x30, &Obis_Start_Ins[0], START_MAX_OBIS_INTAN); 

    ReadEvent_TSVH();
    //
    StrTemp.Data_a8 = &MeterTuTiDataBuff[0];
    StrTemp.Length_u16 = Get_Meter_TuTi.Pos_Obis_Inbuff;
    Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Obis_Inbuff, &StrTemp);  //ghi obis vao
    Get_Meter_Info.Pos_Obis_Inbuff += StrTemp.Length_u16;
    Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
    
    StrTemp.Data_a8 = &MeterTuTiDataBuff[Get_Meter_TuTi.Pos_Obis_Inbuff];
    StrTemp.Length_u16 = Get_Meter_TuTi.Pos_Data_Inbuff - Get_Meter_TuTi.Pos_Obis_Inbuff;
    Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Data_Inbuff, &StrTemp);  //ghi data vao
    Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
    Get_Meter_Info.Numqty += Get_Meter_TuTi.Numqty;
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
                

void Cacul_ID_Table(uint8_t* buff, uint8_t LastBill)
{
    uint8_t i = 0;
    
    for(i = 0; i< 6; i++)
        *(buff + 2 + i) = MeterBillingMessIDTableFirst[2+i] + LastBill * 6;
}


uint8_t START_Read_Bill (void)
{
    truct_String    StrTemp;
    truct_String    Str_Data_Write={&Buff_Temp1[0], 0};
    uint8_t         Temp_BBC = 0;
    uint8_t         CountLastBill = 0;
    
    sDCU.FlagHave_BillMes = 0;
    Init_Meter_TuTi_Struct();
    if(Meter_Comm_Func(&Get_Meter_TuTi,DATA_TUTI,&START_MeterTiTuIDTable[0],&RMTuTi_Extract_Data,&RMI_Push_Mess_TuTi) != 1) 
      return 0;
    
    while ((CountLastBill < 12) &&(sInformation.Flag_Stop_ReadBill == 0))
    {
        Init_Meter_Billing_Struct();
        Init_MD_Bill();
        //
        Cacul_ID_Table(&MeterBillingMessIDTable[0], CountLastBill);
        //
        Meter_Comm_Func(&Get_Meter_Billing,DATA_HISTORICAL,&MeterBillingMessIDTable[0],&RMB_Extract_Data,&RMB_Push_Mess);
        
        CountLastBill++;
        if(sInformation.Flag_Request_Bill == 0) break;  //k request chi doc cai moi nhat
    }
    
    if(sDCU.FlagHave_BillMes == 0)
    {
        Reset_Buff(&Get_Meter_Billing.Str_Payload);
        Write_Header_His_Push103();
        
        StrTemp.Data_a8 = &MeterTuTiDataBuff[0];
        StrTemp.Length_u16 = Get_Meter_TuTi.Pos_Obis_Inbuff;
        Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Obis_Inbuff, &StrTemp);  //ghi obis vao
        Get_Meter_Billing.Pos_Obis_Inbuff += StrTemp.Length_u16;
        Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
        
        StrTemp.Data_a8 = &MeterTuTiDataBuff[Get_Meter_TuTi.Pos_Obis_Inbuff];
        StrTemp.Length_u16 = Get_Meter_TuTi.Pos_Data_Inbuff - Get_Meter_TuTi.Pos_Obis_Inbuff;
        Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Data_Inbuff, &StrTemp);  //ghi data vao
        Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
        Get_Meter_Billing.Numqty += Get_Meter_TuTi.Numqty;
        
        //lay het data moi ghep stime Bill vao
        Reset_Buff(&Str_Data_Write);
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
//        Copy_String_STime(&Str_Data_Write, sInformation.StartTime_GetBill);  //de trong
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')'; 
        
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
        Pack_HEXData_Frame_Uint64(&Str_Data_Write, 3, 0);
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')'; 
        
        Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.PosNumqty, &Str_Data_Write);
        //ETX
        *(Get_Meter_Billing.Str_Payload.Data_a8 + Get_Meter_Billing.Str_Payload.Length_u16++) = ETX; 
        //BBC
        Temp_BBC = BBC_Cacul(Get_Meter_Billing.Str_Payload.Data_a8 + 1,Get_Meter_Billing.Str_Payload.Length_u16 - 1);
        *(Get_Meter_Billing.Str_Payload.Data_a8 + Get_Meter_Billing.Str_Payload.Length_u16++) = Temp_BBC;
        
        Push_Bill_toQueue(DATA_HISTORICAL);
    }
    
    sInformation.Flag_Request_Bill = 0;
    return 1;
}

uint8_t START_Read_Event (uint32_t ForMin)
{
    uint8_t  i = 0;
    
    Init_Meter_TuTi_Struct();
    if(Meter_Comm_Func(&Get_Meter_TuTi,DATA_TUTI,&START_MeterTiTuIDTable[0],&RMTuTi_Extract_Data,&RMI_Push_Mess_TuTi) != 1) return 0;

    Init_Meter_Event_Struct();
    
    Queue_Meter_Event.Mess_Status_ui8 = 4;
    
    while (Get_Meter_Event.Num_Block_ui8 < 12)
    {
        //doc lan luot tung event
        MeterEventMessIDTable[0] = Get_Meter_Event.Num_Block_ui8*3;   //doc event Fail Vol Pha A
        for(i = 1; i < 11; i++)
        {
            MeterEventMessIDTable[i] = Get_Meter_Event.Num_Block_ui8*3 + 1;
            MeterEventMessIDTable[i + 10] = Get_Meter_Event.Num_Block_ui8*3 + 2;
        }
        MeterEventMessIDTable[21] = 0xFF;
        
        Get_Meter_Event.Flag_ui8 = 0;  //muon bien nay de set inde ban tin stime. cu 1 lan nhay len 1. tuong ung voi 1 thanh ghi thoi diem cua event. (1-10).
        //3 bien mess step va step da duoc reinit trong ham Meter_Comm_Func
        Meter_Comm_Func(&Get_Meter_Event,DATA_EVEN_METER,&MeterEventMessIDTable[0],&RME_Extract_Data,&RME_Push_Mess_2);  //
        
        Get_Meter_Event.Num_Block_ui8++;
    }
    
    //tiep tuc doc 
    for(i = 0; i < 10; i++)
    {
        MeterEventMessIDTable[i] = 36;
    }
    MeterEventMessIDTable[10] = 0xFF;
    Get_Meter_Event.Flag_ui8 = 0;  //muon bien nay de set inde ban tin stime. cu 1 lan nhay len 1. tuong ung voi 1 thanh ghi thoi diem cua event. (1-10).
        //3 bien mess step va step da duoc reinit trong ham Meter_Comm_Func
    Meter_Comm_Func(&Get_Meter_Event,DATA_EVEN_METER,&MeterEventMessIDTable[0],&RME_Extract_Data,&RME_Push_Mess_2);  //
    //
    for(i = 0; i < 10; i++)
    {
        MeterEventMessIDTable[i] = 37;
        MeterEventMessIDTable[i + 10] = 38;
    }
    MeterEventMessIDTable[20] = 0xFF;
    Get_Meter_Event.Flag_ui8 = 0;  //muon bien nay de set index ban tin stime. cu 1 lan nhay len 1. tuong ung voi 1 thanh ghi thoi diem cua event. (1-10).
    //3 bien mess step va step da duoc reinit trong ham Meter_Comm_Func
    Meter_Comm_Func(&Get_Meter_Event,DATA_EVEN_METER,&MeterEventMessIDTable[0],&RME_Extract_Data, &RME_Push_Mess); 
    
    return 1;
}

uint8_t START_Read_Lpf (void)
{
    truct_String    Str_Data_Write={&Buff_Temp1[0], 0};
    truct_String    StrTemp;
    
    sDCU.FlagHave_ProfMess = 0;
    Init_Meter_TuTi_Struct();
    if(Meter_Comm_Func(&Get_Meter_TuTi,DATA_TUTI,&START_MeterTiTuIDTable[0],&RMTuTi_Extract_Data,&RMI_Push_Mess_TuTi) != 1) return 0;
    
    RM_Load_Prof_Func();
    if(sDCU.FlagHave_ProfMess == 0)
    {
        //dong goi ban tin rong gui len
        Init_Meter_LProf_Struct(); 
        //wrire header TSVH vao
        Pack_Header_lpf_Pushdata103();
        
        StrTemp.Data_a8 = &MeterTuTiDataBuff[0];
        StrTemp.Length_u16 = Get_Meter_TuTi.Pos_Obis_Inbuff;
        Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Obis_Inbuff, &StrTemp);  //ghi obis vao
        Get_Meter_LProf.Pos_Obis_Inbuff += StrTemp.Length_u16;
        Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
        
        StrTemp.Data_a8 = &MeterTuTiDataBuff[Get_Meter_TuTi.Pos_Obis_Inbuff];
        StrTemp.Length_u16 = Get_Meter_TuTi.Pos_Data_Inbuff - Get_Meter_TuTi.Pos_Obis_Inbuff;
        Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Data_Inbuff, &StrTemp);  //ghi data vao
        Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
        
        Get_Meter_LProf.Numqty += Get_Meter_TuTi.Numqty;
        
        Reset_Buff(&Str_Data_Write);
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
//        Copy_String_STime(&Str_Data_Write, sInformation.EndTime_GetLpf);  //stime start   //de trong
        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')'; 
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
    }
    return 1;
}

uint8_t START_Read_Infor (void)
{
    Init_Meter_TuTi_Struct();
    if(Meter_Comm_Func(&Get_Meter_TuTi,DATA_OPERATION,&START_MeterTiTuIDTable[0],&RMTuTi_Extract_Data,&RMI_Push_Mess_TuTi) == 1)
    {
        Pack_PushData_103_Infor_Meter();
        return 1;
    }
    return 0;
}
 



uint8_t START_Connect_Meter(void)
{
	uint8_t ReadIDRetry = 2;

	while (ReadIDRetry>0)
	{
        osDelay(500);
		if (osMutexWait(mtFlashMeterHandle,1000) == osOK)
		{
			Read_Meter_ID_Success = START_Get_Meter_ID(0);
            
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


void ReadEvent_TSVH(void)
{
    uint8_t i = 0;
    
    Init_Meter_Event_Struct();
    Get_Meter_Event.Flag_Start_Pack = 1;  //khong dong goi header
    //doc VT fail va reverse
    while (Get_Meter_Event.Num_Block_ui8 < 3)
    {
        //doc lan luot tung event
        MeterEventMessIDTable[0] = Get_Meter_Event.Num_Block_ui8*3;   //doc event Fail Vol Pha A
        for(i = 1; i < 11; i++)
        {
            MeterEventMessIDTable[i] = Get_Meter_Event.Num_Block_ui8*3 + 1;
            MeterEventMessIDTable[i + 10] = Get_Meter_Event.Num_Block_ui8*3 + 2;
        }
        MeterEventMessIDTable[21] = 0xFF;
        
        Get_Meter_Event.Flag_ui8 = 0;  //muon bien nay de set inde ban tin stime. cu 1 lan nhay len 1. tuong ung voi 1 thanh ghi thoi diem cua event. (1-10).
        //3 bien mess step va step da duoc reinit trong ham Meter_Comm_Func
        Meter_Comm_Func(&Get_Meter_Event,DATA_EVEN_METER,&MeterEventMessIDTable[0],&RME_Extract_Data,&RME_Push_Mess_2);  //
        
        Get_Meter_Event.Num_Block_ui8++;
    }
    
    //doc Power off va power on
    for(i = 0; i < 10; i++)
    {
        MeterEventMessIDTable[i] = 37;
        MeterEventMessIDTable[i + 10] = 38;
    }
    MeterEventMessIDTable[20] = 0xFF;
    Get_Meter_Event.Flag_ui8 = 0;  //muon bien nay de set inde ban tin stime. cu 1 lan nhay len 1. tuong ung voi 1 thanh ghi thoi diem cua event. (1-10).
    //3 bien mess step va step da duoc reinit trong ham Meter_Comm_Func
    Meter_Comm_Func(&Get_Meter_Event,DATA_EVEN_METER,&MeterEventMessIDTable[0],&RME_Extract_Data, &RME_Push_Mess_TSVH); 
}


//gia tri deu phia tru di 0x33. sau do dao lai byte. duoc dang string dec. gi�ng elster
void Extract_One_Element_Meter_Info (uint8_t length, uint8_t buff_pos, truct_String *oBIS, truct_String *unit, uint8_t ScaleNum )   
{
	uint8_t         i=0,j=0;
    uint8_t         Buff_data[20];
    truct_String    Str_Data_Write = {&Buff_data[0], 0};
    uint8_t         BuffPayload[30];
    truct_String    StrAdd_To_payload = {&BuffPayload[0], 0};
    uint8_t         TempBuff[16];
    uint8_t         Flag_FirstZero = 0;
    
    if(Get_Meter_Info.Flag_Start_Pack == 0)
    {
        Reset_Buff(&Get_Meter_Info.Str_Payload);
        //wrire header TSVH vao
        Write_Header_TSVH_Push103();
        Get_Meter_Info.Flag_Start_Pack = 1;
    }  
     //ghi obis va data vao
    Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Obis_Inbuff, oBIS);
    Get_Meter_Info.Pos_Obis_Inbuff += oBIS->Length_u16;
    Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
    
    if(oBIS->Length_u16 != 0) Get_Meter_Info.Numqty ++;
    
    //
    j = buff_pos+length-1;
	for (i=0;i<length;i++)
        TempBuff[i] = UART1_Receive_Buff[j-i] - 0x33;

    //chuyen sang dang string dec
    for (i=0;i<length;i++)
    {
        if((TempBuff[i] != 0) || (Flag_FirstZero == 1))
        {
            if(Flag_FirstZero == 0)
            {
                if(((TempBuff[i]>>4) & 0x0F) != 0)
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) =  ((TempBuff[i]>>4) & 0x0F) + 0x30;
            }else *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) =  ((TempBuff[i]>>4) & 0x0F) + 0x30;
            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) =  (TempBuff[i] & 0x0F) + 0x30;
            Flag_FirstZero = 1;  //bao co data khac 0 r�i
        }
    }
    if(Flag_FirstZero == 0) *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '0';   //neu tat ca la 0 thi gh� so 0 vao
    
    //
    //them dau '.' vao vi tri tuong ung voi scale
    AddPoint_to_StringDec(&Str_Data_Write, ScaleNum);
    
    //copy ca chuoi du lieu bao gom ca  ().
    *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '(';
    for(i = 0; i < Str_Data_Write.Length_u16; i++)
      *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = *(Str_Data_Write.Data_a8 + i);
    if(unit->Length_u16 != 0)
        *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '*';
    Copy_String_2(&StrAdd_To_payload, unit);  //don vi
    *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = ')';
      
    Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.Pos_Data_Inbuff, &StrAdd_To_payload);
    Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
}

void Extract_TuTi_Meter_Info (Meter_Comm_Struct* GetMet, uint8_t type, uint8_t length, uint8_t buff_pos, truct_String *oBIS, truct_String *unit, uint8_t ScaleNum )   
{
    uint8_t         i=0,j=0;
    uint8_t         Buff_data[20];
    truct_String    Str_Data_Write = {&Buff_data[0], 0};
    uint8_t         BuffPayload[30];
    truct_String    StrAdd_To_payload = {&BuffPayload[0], 0};
    uint8_t         TempBuff[16];
    uint8_t         Flag_FirstZero = 0;

    if(type == 0)
    {
         //ghi obis va data vao
        Copy_String_toTaget(&GetMet->Str_Payload, GetMet->Pos_Obis_Inbuff, oBIS);
        GetMet->Pos_Obis_Inbuff += oBIS->Length_u16;
        GetMet->Pos_Data_Inbuff = GetMet->Str_Payload.Length_u16;
        
        if(oBIS->Length_u16 != 0) GetMet->Numqty ++;
        
        //
        j = buff_pos+length-1;
        for (i=0;i<length;i++)
            TempBuff[i] = UART1_Receive_Buff[j-i] - 0x33;

        //chuyen sang dang string dec
        for (i=0;i<length;i++)
        {
            if((TempBuff[i] != 0) || (Flag_FirstZero == 1))
            {
                if(Flag_FirstZero == 0)
                {
                    if(((TempBuff[i]>>4) & 0x0F) != 0)
                        *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) =  ((TempBuff[i]>>4) & 0x0F) + 0x30;
                }else *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) =  ((TempBuff[i]>>4) & 0x0F) + 0x30;
                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) =  (TempBuff[i] & 0x0F)+ 0x30;
                Flag_FirstZero = 1;  //bao co data khac 0 r�i
            }
        }
        if(Flag_FirstZero == 0) *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '0';   //neu tat ca la 0 thi gh� so 0 vao
        //
        //them dau '.' vao vi tri tuong ung voi scale
        AddPoint_to_StringDec(&Str_Data_Write, ScaleNum);
        
        //copy ca chuoi du lieu bao gom ca  ().
        *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '(';
        for(i = 0; i < Str_Data_Write.Length_u16; i++)
          *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = *(Str_Data_Write.Data_a8 + i);
        *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '/';
        *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '1';
        *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = ')';
          
        Copy_String_toTaget(&GetMet->Str_Payload, GetMet->Pos_Data_Inbuff, &StrAdd_To_payload);
        GetMet->Pos_Data_Inbuff = GetMet->Str_Payload.Length_u16;
    }else if(type == 1)
    {
        Copy_String_toTaget(&GetMet->Str_Payload, GetMet->Pos_Obis_Inbuff, &Str_Ob_Tu);  
        GetMet->Pos_Obis_Inbuff += Str_Ob_Tu.Length_u16;
        GetMet->Numqty++;
        Copy_String_toTaget(&GetMet->Str_Payload, GetMet->Pos_Obis_Inbuff, &Str_Ob_Ti);  
        GetMet->Pos_Obis_Inbuff += Str_Ob_Ti.Length_u16;
        GetMet->Numqty++;
        GetMet->Pos_Data_Inbuff = GetMet->Str_Payload.Length_u16;
        //copy ca chuoi du lieu bao gom ca  ().
        *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '(';
        *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '1';
        *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '/';
        *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '1';
        *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = ')';
        
        *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '(';
        *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '1';
        *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '/';
        *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '1';
        *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = ')';
          
        Copy_String_toTaget(&GetMet->Str_Payload, GetMet->Pos_Data_Inbuff, &StrAdd_To_payload);
        GetMet->Pos_Data_Inbuff = GetMet->Str_Payload.Length_u16;
    }else  //he so nhan
    {
        Copy_String_toTaget(&GetMet->Str_Payload, GetMet->Pos_Obis_Inbuff, &He_So_Nhan);  
        GetMet->Pos_Obis_Inbuff += He_So_Nhan.Length_u16;
        GetMet->Numqty++;
        GetMet->Pos_Data_Inbuff = GetMet->Str_Payload.Length_u16;
        
         //copy ca chuoi du lieu bao gom ca  ().
        *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '(';
        Pack_HEXData_Frame_Uint64(&StrAdd_To_payload, (uint64_t) sDCU.He_So_Nhan, START_SCALE_HE_SO_NHAN);
        *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = ')';
        
        Copy_String_toTaget(&GetMet->Str_Payload, GetMet->Pos_Data_Inbuff, &StrAdd_To_payload);
        GetMet->Pos_Data_Inbuff = GetMet->Str_Payload.Length_u16;
    }
}


void START_Extract_MAXDEMAND_Value (uint8_t length, uint8_t buff_pos, Struct_Maxdemand_Value *Struct_MaxD)
{
	uint8_t i=0,j=0;
    uint8_t         Buff_data[20];
    truct_String    Str_Data_Write = {&Buff_data[0], 0};

    uint8_t         TempBuff[16];
    uint8_t         Flag_FirstZero = 0;
    uint32_t        DataReal = 0;

    j = buff_pos+length-1;
	for (i=0;i<length;i++)
        TempBuff[i] = UART1_Receive_Buff[j-i] - 0x33;

    //chuyen sang dang string dec
    for (i=0;i<length;i++)
    {
        if((TempBuff[i] != 0) || (Flag_FirstZero == 1))
        {
            if(Flag_FirstZero == 0)
            {
                if(((TempBuff[i]>>4) & 0x0F) != 0)
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) =  ((TempBuff[i]>>4) & 0x0F) + 0x30;
            }else *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) =  ((TempBuff[i]>>4) & 0x0F) + 0x30;
            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) =  (TempBuff[i] & 0x0F)+ 0x30;
            Flag_FirstZero = 1;  //bao co data khac 0 r�i
        }
    }
    if(Flag_FirstZero == 0) *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '0';   //neu tat ca la 0 thi gh� so 0 vao
    //
    for(i = 0; i <Str_Data_Write.Length_u16; i++)
        DataReal = DataReal *10 + (*(Str_Data_Write.Data_a8 + i) - 0x30);
    Struct_MaxD->Value_u32[Struct_MaxD->NumValue_MD++] = DataReal;
}


void START_Extract_MAXDEMAND_Time (uint8_t length, uint8_t buff_pos, Struct_Maxdemand_Value *Struct_MaxD)
{
	uint8_t i=0,j=0;
    uint8_t         TempBuff[16];

    j = buff_pos+length-1;
	for (i=0;i<length;i++)
        TempBuff[i] = UART1_Receive_Buff[j-i] - 0x33;

    if(length >=5)
    {
        //chuyen sang dang string dec
        Struct_MaxD->sTime[Struct_MaxD->NumTariff].year     = ConvertHex_to_Dec(TempBuff[0]);
        Struct_MaxD->sTime[Struct_MaxD->NumTariff].month    = ConvertHex_to_Dec(TempBuff[1]);
        Struct_MaxD->sTime[Struct_MaxD->NumTariff].date     = ConvertHex_to_Dec(TempBuff[2]);
        Struct_MaxD->sTime[Struct_MaxD->NumTariff].hour     = ConvertHex_to_Dec(TempBuff[3]);
        Struct_MaxD->sTime[Struct_MaxD->NumTariff].min      = ConvertHex_to_Dec(TempBuff[4]);
        Struct_MaxD->sTime[Struct_MaxD->NumTariff].sec      = 0;
        Struct_MaxD->NumTariff++;
    }
}

uint8_t START_Extract_Bill_Time (uint8_t length, uint8_t buff_pos, Meter_Comm_Struct* GetMet)
{
    //ghi stime vao
    uint8_t j=0;
    ST_TIME_FORMAT  sTime_temp;
    uint8_t         BuffNum[30];
    truct_String    Str_Data_Write={&BuffNum[0], 0};
    uint32_t        DiffValue = 0;
      
    j = buff_pos+length-1;   
    
    sTime_temp.month = ConvertHex_to_Dec(UART1_Receive_Buff[j-1] - 0x33);
    sTime_temp.year  = ConvertHex_to_Dec(UART1_Receive_Buff[j-0] - 0x33);
    sTime_temp.date  = 1;
    sTime_temp.hour  = 0;
    sTime_temp.min   = 0; 
    sTime_temp.sec   = 0;    
    //check thoi gian voi thoi gian request
    if(sInformation.Flag_Request_Bill == 1)
    {
        if((sTime_temp.year != 0) || (sTime_temp.month != 0))
        {
            if(Check_DiffTime (sTime_temp, sInformation.EndTime_GetBill,&DiffValue) == 1)  //Neu endtime nho hon hien tai. thi k gui. van doc tiep
                Get_Meter_Billing.Collecting_Object_ui8 = 1;
            
            if(Check_DiffTime (sInformation.StartTime_GetBill, sTime_temp, &DiffValue) == 1)  //neu start lon hon stime hien tai thi k gui va dung lai
            {
                Get_Meter_Billing.Collecting_Object_ui8 = 1;
                sInformation.Flag_Stop_ReadBill = 1;
            }
        }else
        {
            sInformation.Flag_Stop_ReadBill = 1;
            Get_Meter_Billing.Collecting_Object_ui8 = 1;
        }
    }
    
    if(Get_Meter_Billing.Flag_Start_Pack == 0)
    {
        Reset_Buff(&Get_Meter_Billing.Str_Payload);
        //wrire header TSVH vao
        Write_Header_His_Push103();
        Get_Meter_Billing.Flag_Start_Pack = 1;
        Init_MD_Bill();
    }  
    
    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
    Copy_String_STime(&Str_Data_Write,sTime_temp);
    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
    //
    Copy_String_toTaget(&GetMet->Str_Payload, GetMet->PosNumqty, &Str_Data_Write);
    GetMet->PosNumqty += Str_Data_Write.Length_u16;
    GetMet->Pos_Obis_Inbuff += Str_Data_Write.Length_u16;
    GetMet->Pos_Data_Inbuff = GetMet->Str_Payload.Length_u16;
    
    return 1;
}



void Extract_One_Element_Meter_Billing (uint8_t length, uint8_t buff_pos, truct_String *oBIS, truct_String *unit, uint8_t ScaleNum ) 
{
	uint8_t i=0,j=0;
    uint8_t         Buff_data[20];
    truct_String    Str_Data_Write = {&Buff_data[0], 0};
    uint8_t         BuffPayload[30];
    truct_String    StrAdd_To_payload = {&BuffPayload[0], 0};
    uint8_t         TempBuff[16];
    uint8_t         Flag_FirstZero = 0;
    
    if(Get_Meter_Billing.Flag_Start_Pack == 0)
    {
        Reset_Buff(&Get_Meter_Billing.Str_Payload);
        //wrire header TSVH vao
        Write_Header_His_Push103();
        Get_Meter_Billing.Flag_Start_Pack = 1;
        Init_MD_Bill();
    }  
     //ghi obis va data vao
    Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Obis_Inbuff, oBIS);
    Get_Meter_Billing.Pos_Obis_Inbuff += oBIS->Length_u16;
    Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
    
    if(oBIS->Length_u16 != 0) Get_Meter_Billing.Numqty ++;
    
    //
    j = buff_pos+length-1;
	for (i=0;i<length;i++)
        TempBuff[i] = UART1_Receive_Buff[j-i] - 0x33;

    //chuyen sang dang string dec
    for (i=0;i<length;i++)
    {
        if((TempBuff[i] != 0) || (Flag_FirstZero == 1))
        {
            if(Flag_FirstZero == 0)
            {
                if(((TempBuff[i]>>4) & 0x0F) != 0)
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) =  ((TempBuff[i]>>4) & 0x0F) + 0x30;
            }else *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) =  ((TempBuff[i]>>4) & 0x0F) + 0x30;
            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) =  (TempBuff[i] & 0x0F) + 0x30;
            Flag_FirstZero = 1;  //bao co data khac 0 r�i
        }
    }
    if(Flag_FirstZero == 0) *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '0';   //neu tat ca la 0 thi gh� so 0 vao
    
    //
    //them dau '.' vao vi tri tuong ung voi scale
    AddPoint_to_StringDec(&Str_Data_Write, ScaleNum);
    
    //copy ca chuoi du lieu bao gom ca  ().
    *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '(';
    for(i = 0; i < Str_Data_Write.Length_u16; i++)
      *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = *(Str_Data_Write.Data_a8 + i);
    if(unit->Length_u16 != 0)
        *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '*';
    Copy_String_2(&StrAdd_To_payload, unit);  //don vi
    *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = ')';
      
    Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Data_Inbuff, &StrAdd_To_payload);
    Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
}

void UART2_Mess_Checksum(void)
{
	uint32_t	i=0;
	uint32_t	CheckSumByte=0;
	
	for (i=1;i<UART1_Control.Mess_Length_ui16-1;i++)
	{
		CheckSumByte += UART1_Receive_Buff[i];
	}
	
	CheckSumByte = CheckSumByte & 0x00FF;
	
	if (CheckSumByte == UART1_Receive_Buff[UART1_Control.Mess_Length_ui16-1])
	{
		UART1_Control.Mess_Status_ui8 = 2;
	}
	else
	{
		UART1_Control.Mess_Status_ui8 = 4;
	}
}

uint8_t START_GetUART2Data(void)
{
	uint8_t	temp_recieve = 0;
	
	if (UART1_Control.Mess_Pending_ui8 == 0)
	{
		temp_recieve = UART_METER.Instance->RDR & 0x00FF;
		UART1_Receive_Buff[UART1_Control.Mess_Length_ui16] = temp_recieve;
		
		if ((temp_recieve == 0x16)&&(UART1_Control.Mess_Length_ui16 == (UART1_Receive_Buff[10]+ 12)))
		{
			UART1_Control.Mess_Pending_ui8 = 1;
			UART1_Control.Mess_Status_ui8 = 1;
			osSemaphoreRelease(bsUART2PendingMessHandle);
		}
		else
		{
			UART1_Control.Mess_Length_ui16++;
			if (UART1_Control.Mess_Length_ui16 > 127)		
				UART1_Control.Mess_Length_ui16 = 0;
		}
	}
	else
		temp_recieve = UART_METER.Instance->RDR & 0x00FF;	
    UART1_Control.fRecei_Respond = 1;
    
    return 1;
}

void START_Fill_Mess (uint8_t message_code, uint8_t message_type)
{
	uint8_t temp = 0;
    uint8_t	TempChecksum=0;
    uint8_t i = 0;
      
	switch(message_type)
	{
		case DATA_OPERATION:
			temp = START_MeterInfoMessIDTable[message_code];
			Read_Meter_Info_Template[13] = Read_Meter_Info_Code[temp][0];
			Read_Meter_Info_Template[14] = Read_Meter_Info_Code[temp][1];
			Read_Meter_Info_Template[15] = Read_Meter_Info_Code[temp][2];
			break;
		case DATA_HISTORICAL:
			temp = MeterBillingMessIDTable[message_code];
			Read_Meter_Info_Template[13] = Read_Meter_Info_Code[temp][0];
			Read_Meter_Info_Template[14] = Read_Meter_Info_Code[temp][1];
			Read_Meter_Info_Template[15] = Read_Meter_Info_Code[temp][2];
			break;
		case DATA_EVEN_METER:
			temp = MeterEventMessIDTable[message_code];
			Read_Meter_Info_Template[13] =(uint8_t) (Read_Meter_Event_Code[temp][0] + Get_Meter_Event.Flag_ui8);
			Read_Meter_Info_Template[14] =(uint8_t)  Read_Meter_Event_Code[temp][1];
            //byte nay la check sum tu dau den byte 14. tinh lai
            TempChecksum = 0;
            for (i=3;i<15;i++)
                TempChecksum += Read_Meter_Info_Template[i];
            Read_Meter_Info_Template[15] = TempChecksum;
                
            if(Read_Meter_Event_Code[temp][4] == 0x01)   //doc xong start den stop. Khi ket thuc start dong goi data. se cho flag nay ve 0. de tiep tuc doc stop
               Get_Meter_Event.Flag_ui8 ++;
            
			break;
        case DATA_TUTI:
			temp = START_MeterTiTuIDTable[message_code];
			Read_Meter_Info_Template[13] = Read_Meter_Info_Code[temp][0];
			Read_Meter_Info_Template[14] = Read_Meter_Info_Code[temp][1];
			Read_Meter_Info_Template[15] = Read_Meter_Info_Code[temp][2];
			break;
		default:
			break;
	}
}

void RMTuTi_Extract_Data (void)
{
    switch (Get_Meter_TuTi.Mess_Step_ui8)
	{
		case 0:
            if (UART1_Receive_Buff[9]!= 0xC1)  // Fix 2 byte data
            {
                Extract_TuTi_Meter_Info(&Get_Meter_TuTi, 0, 2, 13, (truct_String*) &Str_Ob_Tu,  (truct_String*) NULL, START_SCALE_TU_TI);
                Extract_TuTi_Meter_Info(&Get_Meter_TuTi, 0, 2, 15, (truct_String*) &Str_Ob_Ti,  (truct_String*) NULL, START_SCALE_TU_TI); 
                Extract_TuTi_Meter_Info(&Get_Meter_TuTi, 2, 2, 15, (truct_String*) NULL,  (truct_String*) NULL, 0);  //he so nhan
                Type_Meter = 1;
            }
            else
            {
                Extract_TuTi_Meter_Info(&Get_Meter_TuTi, 1, 2, 13, (truct_String*) NULL,  (truct_String*) NULL, START_SCALE_TU_TI); 
                Extract_TuTi_Meter_Info(&Get_Meter_TuTi, 2, 2, 15, (truct_String*) NULL,  (truct_String*) NULL, 0);//he so nhan
                Type_Meter = 0;
            }
            break;
        default: 
          break;
    }
}

ST_TIME_FORMAT  sTimeSTART;
void RMI_Extract_Data (void)
{
	uint8_t	        j=0,k=0,temp_leng=0;
    int8_t          RowObis103 = 0;
    uint8_t         ObisHex = 0;
    uint8_t         BuffNum[30];
    truct_String    Str_Data_Write={&BuffNum[0], 0};
    
	switch (Get_Meter_Info.Mess_Step_ui8)
	{
		case 0:
            if((UART1_Control.Mess_Length_ui16-1) >= 17)
            {
                //ghi stime vao buff
                sTimeSTART.day   = ConvertHex_to_Dec(UART1_Receive_Buff[UART1_Control.Mess_Length_ui16- 5] - 0x33); 
                sTimeSTART.date  = ConvertHex_to_Dec(UART1_Receive_Buff[UART1_Control.Mess_Length_ui16 - 4] - 0x33); 
                sTimeSTART.month = ConvertHex_to_Dec(UART1_Receive_Buff[UART1_Control.Mess_Length_ui16 - 3] - 0x33); 
                sTimeSTART.year  = ConvertHex_to_Dec(UART1_Receive_Buff[UART1_Control.Mess_Length_ui16 - 2] - 0x33);  
            }
			break;
		case 1:
            if((UART1_Control.Mess_Length_ui16-1) >= 16)
            {
                sTimeSTART.sec   = ConvertHex_to_Dec(UART1_Receive_Buff[UART1_Control.Mess_Length_ui16 - 4] - 0x33);
                sTimeSTART.min   = ConvertHex_to_Dec(UART1_Receive_Buff[UART1_Control.Mess_Length_ui16 - 3] - 0x33);
                sTimeSTART.hour  = ConvertHex_to_Dec(UART1_Receive_Buff[UART1_Control.Mess_Length_ui16 - 2] - 0x33);
            }
            if(Get_Meter_Info.Flag_Start_Pack == 0)
            {
                Reset_Buff(&Get_Meter_Info.Str_Payload);
                //wrire header TSVH vao
                Write_Header_TSVH_Push103();
                Get_Meter_Info.Flag_Start_Pack = 1;
            }  
            //ghi vao buff
            Reset_Buff(&Str_Data_Write);
            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
            Copy_String_STime(&Str_Data_Write,sTimeSTART);
            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
            //
            Copy_String_toTaget(&Get_Meter_Info.Str_Payload, Get_Meter_Info.PosNumqty, &Str_Data_Write);
            Get_Meter_Info.PosNumqty += Str_Data_Write.Length_u16;
            Get_Meter_Info.Pos_Obis_Inbuff += Str_Data_Write.Length_u16;
            Get_Meter_Info.Pos_Data_Inbuff = Get_Meter_Info.Str_Payload.Length_u16;
			break;
		case 14:
			temp_leng = (UART1_Receive_Buff[10]-2)/18;
			START_Extract_MAXDEMAND_Time(temp_leng,13, &MD_Plus_Bill);                       //max dem plus time, 1, 2, 3, 4  Struct_Maxdemand_Value      MD_Plus_Bill;      MD_Sub_Bill;
			START_Extract_MAXDEMAND_Time(temp_leng,13+1*temp_leng, &MD_Plus_Bill);
			START_Extract_MAXDEMAND_Time(temp_leng,13+2*temp_leng, &MD_Plus_Bill);
			START_Extract_MAXDEMAND_Time(temp_leng,13+3*temp_leng, &MD_Plus_Bill);
			START_Extract_MAXDEMAND_Time(temp_leng,13+4*temp_leng, &MD_Plus_Bill);
            
			START_Extract_MAXDEMAND_Time(temp_leng,13+5*temp_leng, &MD_Sub_Bill);           //max dem Sub time, 1, 2, 3, 4
			START_Extract_MAXDEMAND_Time(temp_leng,13+6*temp_leng, &MD_Sub_Bill);
			START_Extract_MAXDEMAND_Time(temp_leng,13+7*temp_leng, &MD_Sub_Bill);
			START_Extract_MAXDEMAND_Time(temp_leng,13+8*temp_leng, &MD_Sub_Bill);
			START_Extract_MAXDEMAND_Time(temp_leng,13+9*temp_leng, &MD_Sub_Bill);
			break;
		case 15:
			temp_leng = (UART1_Receive_Buff[10]-2)/18;
			START_Extract_MAXDEMAND_Value(temp_leng,13, &MD_Plus_Bill);                      //max dem plus Value, 1, 2, 3, 4                
			START_Extract_MAXDEMAND_Value(temp_leng,13+1*temp_leng, &MD_Plus_Bill);
			START_Extract_MAXDEMAND_Value(temp_leng,13+2*temp_leng, &MD_Plus_Bill);
			START_Extract_MAXDEMAND_Value(temp_leng,13+3*temp_leng, &MD_Plus_Bill);
			START_Extract_MAXDEMAND_Value(temp_leng,13+4*temp_leng, &MD_Plus_Bill);
            
			START_Extract_MAXDEMAND_Value(temp_leng,13+5*temp_leng, &MD_Sub_Bill);          ////max dem sub Value, 1, 2, 3, 4 
			START_Extract_MAXDEMAND_Value(temp_leng,13+6*temp_leng, &MD_Sub_Bill);
			START_Extract_MAXDEMAND_Value(temp_leng,13+7*temp_leng, &MD_Sub_Bill);
			START_Extract_MAXDEMAND_Value(temp_leng,13+8*temp_leng, &MD_Sub_Bill);
			START_Extract_MAXDEMAND_Value(temp_leng,13+9*temp_leng, &MD_Sub_Bill);
			break;
		case 16:
			if(Type_Meter == 1)
            {
                temp_leng = (UART1_Receive_Buff[10]-2)/4;	
                //PowerFactor
                Extract_One_Element_Meter_Info(temp_leng,13, (truct_String*) &Str_Ob_PoFacA,  (truct_String*) &StrNull, START_SCALE_POW_FACTOR);   //total
                //Phase A
                Extract_One_Element_Meter_Info(temp_leng,13+temp_leng,(truct_String*) &Str_Ob_PoFacB, (truct_String*) &StrNull, START_SCALE_POW_FACTOR);
                //Phase B
                Extract_One_Element_Meter_Info(temp_leng,13+temp_leng+temp_leng, (truct_String*) &Str_Ob_PoFacC, (truct_String*) &StrNull, START_SCALE_POW_FACTOR);
                //Phase C
                Extract_One_Element_Meter_Info(temp_leng,13+temp_leng+temp_leng+temp_leng, (truct_String*) &Str_Ob_PoFac, (truct_String*) &StrNull, START_SCALE_POW_FACTOR);			
			}
			else {
                temp_leng = (UART1_Receive_Buff[10]-2)/4;	
                //Phase C
                Extract_One_Element_Meter_Info(temp_leng,13,(truct_String*) &Str_Ob_PoFac, (truct_String*) &StrNull, START_SCALE_POW_FACTOR);	
                //Phase B
                Extract_One_Element_Meter_Info(temp_leng,13+temp_leng,(truct_String*) &Str_Ob_PoFacC, (truct_String*) &StrNull, START_SCALE_POW_FACTOR);
                //Phase A
                Extract_One_Element_Meter_Info(temp_leng,13+temp_leng+temp_leng, (truct_String*) &Str_Ob_PoFacB, (truct_String*) &StrNull, START_SCALE_POW_FACTOR);
                //PowerFactor
                Extract_One_Element_Meter_Info(temp_leng,13+temp_leng+temp_leng+temp_leng, (truct_String*) &Str_Ob_PoFacA,  (truct_String*) &StrNull, START_SCALE_POW_FACTOR);
			}
			break;
		case 17:
			temp_leng = (UART1_Receive_Buff[10]-2)/4;
			//ActivePowerPhaseTotal
			Extract_One_Element_Meter_Info(temp_leng,13, (truct_String*) &Str_Ob_AcPowTo, (truct_String*) &Unit_Active_Intan_Kw, START_SCALE_ACTIVE_POW);	
			//Phase A
			Extract_One_Element_Meter_Info(temp_leng,13+temp_leng, (truct_String*) &Str_Ob_AcPowA, (truct_String*) &Unit_Active_Intan_Kw, START_SCALE_ACTIVE_POW);	
			//Phase B
			Extract_One_Element_Meter_Info(temp_leng,13+temp_leng+temp_leng, (truct_String*) &Str_Ob_AcPowB, (truct_String*) &Unit_Active_Intan_Kw, START_SCALE_ACTIVE_POW);
			//Phase C
			Extract_One_Element_Meter_Info(temp_leng,13+temp_leng+temp_leng+temp_leng, (truct_String*) &Str_Ob_AcPowC,(truct_String*) &Unit_Active_Intan_Kw, START_SCALE_ACTIVE_POW);
			break;
		case 18:
			temp_leng = (UART1_Receive_Buff[10]-2)/4;
			//ReActivePowerPhaseTotal
			Extract_One_Element_Meter_Info(temp_leng,13, (truct_String*) &Str_Ob_RePowTo, (truct_String*) &Unit_Reactive_Intan_Kvar, START_SCALE_REACTIVE_POW);
			//Phase A
			Extract_One_Element_Meter_Info(temp_leng,13+temp_leng, (truct_String*) &Str_Ob_RePowA, (truct_String*) &Unit_Reactive_Intan_Kvar, START_SCALE_REACTIVE_POW);  
			//Phase B
			Extract_One_Element_Meter_Info(temp_leng,13+temp_leng+temp_leng, (truct_String*) &Str_Ob_RePowB, (truct_String*) &Unit_Reactive_Intan_Kvar, START_SCALE_REACTIVE_POW);
			//Phase C
			Extract_One_Element_Meter_Info(temp_leng,13+temp_leng+temp_leng+temp_leng, (truct_String*) &Str_Ob_RePowC, (truct_String*) &Unit_Reactive_Intan_Kvar, START_SCALE_REACTIVE_POW);		
			break;
		case 34:
//			if (UART1_Receive_Buff[9]!= 0xC1)  // Fix 2 byte data
//			{
//                Extract_TuTi_Meter_Info(&Get_Meter_Info, 0, 2, 15, (truct_String*) &Str_Ob_Ti,  (truct_String*) NULL, START_SCALE_TU_TI); 
//                Extract_TuTi_Meter_Info(&Get_Meter_Info, 0, 2, 13, (truct_String*) &Str_Ob_Tu,  (truct_String*) NULL, START_SCALE_TU_TI);
//                Extract_TuTi_Meter_Info(&Get_Meter_Info, 2, 2, 15, (truct_String*) NULL,  (truct_String*) NULL, 0);  //he so nhan
//				Type_Meter = 1;
//			}
//			else
//			{
//                Extract_TuTi_Meter_Info(&Get_Meter_Info, 1, 2, 13, (truct_String*) NULL,  (truct_String*) NULL, START_SCALE_TU_TI); 
//                Extract_TuTi_Meter_Info(&Get_Meter_Info, 2, 2, 15, (truct_String*) NULL,  (truct_String*) NULL, 0);
//				Type_Meter = 0;
//			}
			break;
		default:
			//MessOBIS
			j = START_MeterInfoMessIDTable[Get_Meter_Info.Mess_Step_ui8];
			ObisHex = Read_Meter_Info_Code[j][3];
            RowObis103 = Check_Row_Obis103(&Obis_Start_Ins[0], ObisHex, START_MAX_OBIS_INTAN);
            if(RowObis103 >=0)
            {
                //Fill data into buffer
                j = UART1_Control.Mess_Length_ui16-2;
                k = UART1_Control.Mess_Length_ui16-14;
                if(Obis_Start_Ins[RowObis103].Str_Obis != NULL)
                {
                    Extract_One_Element_Meter_Info(k, j+1-k, (truct_String*) Obis_Start_Ins[RowObis103].Str_Obis, (truct_String*) Obis_Start_Ins[RowObis103].Str_Unit, Obis_Start_Ins[RowObis103].scale);  ////j  = buff_pos+length-1; o phia trong
                }
            }
			break;
	}
}


void RMI_Push_Mess (void)
{
	
}

void RMI_Push_Mess_Instant (void)
{
	
}

void RMI_Push_Mess_TuTi (void)
{
	
}

void RMB_Extract_Data (void)
{
	switch (Get_Meter_Billing.Mess_Step_ui8)
	{
		case 0://bo qua stime 
			break;
		case 1:					
			break;
		case 2: //ActivePower
            // Ghi gia tri thoi gian vao   
			START_Extract_Bill_Time(0x02,13, &Get_Meter_Billing);
            //
			Extract_One_Element_Meter_Billing(0x05,15, (truct_String*) &Str_Ob_AcImTotal_Chot,  (truct_String*) &Unit_Active_EnTotal, START_SCALE_TOTAL_ENERGY);  //Import wh total
			Extract_One_Element_Meter_Billing(0x05,20, (truct_String*) &Str_Ob_AcPlus_Rate1_chot, (truct_String*) &Unit_Active_EnTotal, START_SCALE_TARRIFF); 
			Extract_One_Element_Meter_Billing(0x05,25, (truct_String*) &Str_Ob_AcPlus_Rate2_chot, (truct_String*) &Unit_Active_EnTotal, START_SCALE_TARRIFF); 
			Extract_One_Element_Meter_Billing(0x05,30, (truct_String*) &Str_Ob_AcPlus_Rate3_chot, (truct_String*) &Unit_Active_EnTotal, START_SCALE_TARRIFF); 
			Extract_One_Element_Meter_Billing(0x05,35, (truct_String*) &Str_Ob_AcPlus_Rate4_chot, (truct_String*) &Unit_Active_EnTotal, START_SCALE_TARRIFF);      //bieu 4
            //
			Extract_One_Element_Meter_Billing(0x05,60, (truct_String*) &Str_Ob_AcExTotal_Chot, (truct_String*) &Unit_Active_EnTotal, START_SCALE_TOTAL_ENERGY);    //export total
			Extract_One_Element_Meter_Billing(0x05,65, (truct_String*) &Str_Ob_AcSub_Rate1_chot, (truct_String*) &Unit_Active_EnTotal, START_SCALE_TARRIFF);
			Extract_One_Element_Meter_Billing(0x05,70, (truct_String*) &Str_Ob_AcSub_Rate2_chot, (truct_String*) &Unit_Active_EnTotal, START_SCALE_TARRIFF);
			Extract_One_Element_Meter_Billing(0x05,75, (truct_String*) &Str_Ob_AcSub_Rate3_chot, (truct_String*) &Unit_Active_EnTotal, START_SCALE_TARRIFF);
			Extract_One_Element_Meter_Billing(0x05,80, (truct_String*) &Str_Ob_AcSub_Rate4_chot, (truct_String*) &Unit_Active_EnTotal, START_SCALE_TARRIFF);    //bieu 4
			break;
		case 3: //Max Demand Active  Time

			START_Extract_MAXDEMAND_Time(0x05,15, &MD_Plus_Bill);            //Maxdemand plus time 0.
			START_Extract_MAXDEMAND_Time(0x05,20, &MD_Plus_Bill);
			START_Extract_MAXDEMAND_Time(0x05,25, &MD_Plus_Bill);
			START_Extract_MAXDEMAND_Time(0x05,30, &MD_Plus_Bill);
			START_Extract_MAXDEMAND_Time(0x05,35, &MD_Plus_Bill);             //md plus time 4. (40, 45, 50, 55,    - 60: sub rate 0
            
            START_Extract_MAXDEMAND_Time(0x05,60, &MD_Sub_Bill);              //md sub r0
			START_Extract_MAXDEMAND_Time(0x05,65, &MD_Sub_Bill);               //md sub r1
			START_Extract_MAXDEMAND_Time(0x05,70, &MD_Sub_Bill);
			START_Extract_MAXDEMAND_Time(0x05,75, &MD_Sub_Bill);
			START_Extract_MAXDEMAND_Time(0x05,80, &MD_Sub_Bill);               //md sub r4
			break;
		case 4: //Max Demand Active Power
			START_Extract_MAXDEMAND_Value(0x03,15, &MD_Plus_Bill);
			START_Extract_MAXDEMAND_Value(0x03,18, &MD_Plus_Bill);
			START_Extract_MAXDEMAND_Value(0x03,21, &MD_Plus_Bill);
			START_Extract_MAXDEMAND_Value(0x03,24, &MD_Plus_Bill);
			START_Extract_MAXDEMAND_Value(0x03,27, &MD_Plus_Bill);   //(30 33, 36, 39===== 5-8
            
            START_Extract_MAXDEMAND_Value(0x03,42, &MD_Sub_Bill); 
			START_Extract_MAXDEMAND_Value(0x03,45, &MD_Sub_Bill); 
			START_Extract_MAXDEMAND_Value(0x03,48, &MD_Sub_Bill); 
			START_Extract_MAXDEMAND_Value(0x03,51, &MD_Sub_Bill); 
			START_Extract_MAXDEMAND_Value(0x03,54, &MD_Sub_Bill); 
			break;
		case 5: //ReactivePower
			Extract_One_Element_Meter_Billing(0x05,15, (truct_String*) &Str_Ob_ReImTotal_Chot, (truct_String*) &Unit_Reactive_EnTotal, START_SCALE_TOTAL_ENERGY); 
//			Extract_One_Element_Meter_Billing(0x49,0x05,20);
//			Extract_One_Element_Meter_Billing(0x4A,0x05,25);
//			Extract_One_Element_Meter_Billing(0x4B,0x05,30);
//			Extract_One_Element_Meter_Billing(0x4C,0x05,35);
			Extract_One_Element_Meter_Billing(0x05,60, (truct_String*) &Str_Ob_ReExTotal_Chot, (truct_String*) &Unit_Reactive_EnTotal, START_SCALE_TOTAL_ENERGY);
//			Extract_One_Element_Meter_Billing(0x4D,0x05,65);
//			Extract_One_Element_Meter_Billing(0x4E,0x05,70);
//			Extract_One_Element_Meter_Billing(0x4F,0x05,75);
//			Extract_One_Element_Meter_Billing(0x50,0x05,80);
			break;
		case 6: //Max Demand reactive  Time	
//			Extract_One_Element_Meter_Billing(0x5A,0x05,15);
			break;
		case 7: //Max Demand reactive Power
//			Extract_One_Element_Meter_Billing(0x59,0x03,15);
			break;
		default:				
			break;		
	}
}


void RMB_Push_Mess (void)
{
    uint8_t         Temp_BBC = 0;
    uint8_t         BuffNum[30];
    truct_String    Str_Data_Write={&BuffNum[0], 0};
    truct_String    StrTemp;
    
    if(Get_Meter_Billing.Collecting_Object_ui8 == 0)
    {
        Pack_MD_Value_Time(DATA_HISTORICAL, &Get_Meter_Billing, &MD_Plus_Bill, 0x2E, &Obis_Start_Ins[0], START_MAX_OBIS_INTAN);
        Pack_MD_Value_Time(DATA_HISTORICAL, &Get_Meter_Billing, &MD_Sub_Bill, 0x30, &Obis_Start_Ins[0], START_MAX_OBIS_INTAN); 
        
    //    Add_TuTI_toPayload(&Get_Meter_Billing);
        StrTemp.Data_a8 = &MeterTuTiDataBuff[0];
        StrTemp.Length_u16 = Get_Meter_TuTi.Pos_Obis_Inbuff;
        Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Obis_Inbuff, &StrTemp);  //ghi obis vao
        Get_Meter_Billing.Pos_Obis_Inbuff += StrTemp.Length_u16;
        Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
        
        StrTemp.Data_a8 = &MeterTuTiDataBuff[Get_Meter_TuTi.Pos_Obis_Inbuff];
        StrTemp.Length_u16 = Get_Meter_TuTi.Pos_Data_Inbuff - Get_Meter_TuTi.Pos_Obis_Inbuff;
        Copy_String_toTaget(&Get_Meter_Billing.Str_Payload, Get_Meter_Billing.Pos_Data_Inbuff, &StrTemp);  //ghi data vao
        Get_Meter_Billing.Pos_Data_Inbuff = Get_Meter_Billing.Str_Payload.Length_u16;
        
        Get_Meter_Billing.Numqty += Get_Meter_TuTi.Numqty;
        
        //Ghu numqty. stime da ghi vao roi 
        Reset_Buff(&Str_Data_Write);
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
}

ST_TIME_FORMAT  sTimeEvent={0,0,0, 0,0,0, 0, 0};

void RME_Extract_Data (void)
{
	uint8_t	        i=0,j=0,k=0, m = 0, PosData = 0;
    uint8_t         ObisHex = 0;
    int8_t          RowObis103 = 0;
    ST_TIME_FORMAT  sTimeTemp;
    uint32_t        DiffsTime = 0;
    uint8_t         Buff_data[15];
    truct_String    Str_Data_Write = {&Buff_data[0], 0};
    uint8_t         TempBuff[8];
    uint8_t         BuffPayload[20];
    truct_String    StrAdd_To_payload = {&BuffPayload[0], 0};
    uint8_t         Flag_FirstZero = 0;
      
    if(Get_Meter_Event.Flag_Start_Pack == 0)
    {
        Reset_Buff(&Get_Meter_Event.Str_Payload);
        Header_event_103(&Get_Meter_Event.Str_Payload, 1);
        Get_Meter_Event.Flag_Start_Pack = 1;
    }
    
	j = MeterEventMessIDTable[Get_Meter_Event.Mess_Step_ui8];
    m = MeterEventMessIDTable[Get_Meter_Event.Mess_Step_ui8 + 1];
	k = UART1_Receive_Buff[10] - 2; // Length Data
    
    if(Read_Meter_Event_Code[j][4] == 0)  //type value
    {
        m = MeterEventMessIDTable[Get_Meter_Event.Mess_Step_ui8];
        ObisHex = Read_Meter_Event_Code[m][3];
        RowObis103 = Check_Obis_event(&Obis_Start_Event[0], ObisHex, START_MAX_EVENT);   
        
        if(RowObis103 >= 0)
        {
             //ghi obis va data vao
            Copy_String_toTaget(&Get_Meter_Event.Str_Payload, Get_Meter_Event.Pos_Obis_Inbuff, Obis_Start_Event[RowObis103].Str_Obis);
            Get_Meter_Event.Pos_Obis_Inbuff += Obis_Start_Event[RowObis103].Str_Obis->Length_u16;
            Get_Meter_Event.Pos_Data_Inbuff = Get_Meter_Event.Str_Payload.Length_u16;
            Get_Meter_Event.Numqty ++;
            //
            PosData = UART1_Control.Mess_Length_ui16-2;  // length Message
            for(i = 0; i<k; i++)
                TempBuff[i] = UART1_Receive_Buff[PosData-i] - 0x33;;
            
            for (i=0;i<k;i++)
            {
                if((TempBuff[i] != 0) || (Flag_FirstZero == 1))
                {
                    if(Flag_FirstZero == 0)
                    {
                        if(((TempBuff[i]>>4) & 0x0F) != 0)
                            *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) =  ((TempBuff[i]>>4) & 0x0F) + 0x30;
                    }else *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) =  ((TempBuff[i]>>4) & 0x0F) + 0x30;
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) =  (TempBuff[i] & 0x0F)+ 0x30;
                    Flag_FirstZero = 1;  //bao co data khac 0 r�i
                }
            }
            if(Flag_FirstZero == 0) *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '0';   //neu tat ca la 0 thi gh� so 0 vao
            
            *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '(';
            for(i = 0; i < Str_Data_Write.Length_u16; i++)
              *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = *(Str_Data_Write.Data_a8 + i);
            *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = ')';
              
            Copy_String_toTaget(&Get_Meter_Event.Str_Payload, Get_Meter_Event.Pos_Data_Inbuff, &StrAdd_To_payload);
            Get_Meter_Event.Pos_Data_Inbuff = Get_Meter_Event.Str_Payload.Length_u16;
        }
        Get_Meter_Event.Flag_ui8 = 0; 
    }else if(Read_Meter_Event_Code[j][4] == 1)
    {
        PosData = UART1_Control.Mess_Length_ui16-2;  // length Message
        // lay data cho vao 1 buff. lay thoi gia ra. roi so sanh voi thoi gian truoc. Neu lon hon thi ghi de len. de chi lay cai moi nhat
        if(k>= 5)
        {
            sTimeTemp.year  = ConvertHex_to_Dec(UART1_Receive_Buff[PosData-0] - 0x33);
            sTimeTemp.month = ConvertHex_to_Dec(UART1_Receive_Buff[PosData-1] - 0x33);
            sTimeTemp.date  = ConvertHex_to_Dec(UART1_Receive_Buff[PosData-2] - 0x33);
            sTimeTemp.hour  = ConvertHex_to_Dec(UART1_Receive_Buff[PosData-3] - 0x33);
            sTimeTemp.min   = ConvertHex_to_Dec(UART1_Receive_Buff[PosData-4] - 0x33);
            sTimeTemp.sec  = 0;
        }
        if((sTimeTemp.year != 0) || (sTimeTemp.month != 0) || (sTimeTemp.date != 0) || (sTimeTemp.hour != 0) || (sTimeTemp.min != 0) || (sTimeTemp.sec != 0))
          if((sTimeEvent.year != 0) || (sTimeEvent.month != 0) || (sTimeEvent.date != 0) || (sTimeEvent.hour != 0) || (sTimeEvent.min != 0) || (sTimeEvent.sec != 0))
          {
            if(Check_DiffTime(sTimeTemp, sTimeEvent, &DiffsTime) == 1)
                Copy_STime_fromsTime(&sTimeEvent, sTimeTemp.year, sTimeTemp.month, sTimeTemp.date, sTimeTemp.hour, sTimeTemp.min, sTimeTemp.sec);
          }else  
            Copy_STime_fromsTime(&sTimeEvent, sTimeTemp.year, sTimeTemp.month, sTimeTemp.date, sTimeTemp.hour, sTimeTemp.min, sTimeTemp.sec);
        // neu ket thuc cac thoi diem cung obis thi dong goi gia tri lon nhat
        if(Read_Meter_Event_Code[j][3] != Read_Meter_Event_Code[m][3]) // cung obis - chua lay obis cuoi cung
        {
            m = MeterEventMessIDTable[Get_Meter_Event.Mess_Step_ui8];
            ObisHex = Read_Meter_Event_Code[m][3];
            RowObis103 = Check_Obis_event(&Obis_Start_Event[0], ObisHex, START_MAX_EVENT);
            
            if(RowObis103 >= 0)
            {
                //dong goi sTime moi nhat vao //ghi obis va data vao
                Copy_String_toTaget(&Get_Meter_Event.Str_Payload, Get_Meter_Event.Pos_Obis_Inbuff, Obis_Start_Event[RowObis103].Str_Obis);
                Get_Meter_Event.Pos_Obis_Inbuff += Obis_Start_Event[RowObis103].Str_Obis->Length_u16;
                Get_Meter_Event.Pos_Data_Inbuff = Get_Meter_Event.Str_Payload.Length_u16;
                Get_Meter_Event.Numqty ++;
                
                *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = '(';
                Copy_String_STime(&StrAdd_To_payload, sTimeEvent);
                *(StrAdd_To_payload.Data_a8 + StrAdd_To_payload.Length_u16++) = ')';
                
                Copy_String_toTaget(&Get_Meter_Event.Str_Payload, Get_Meter_Event.Pos_Data_Inbuff, &StrAdd_To_payload);
                Get_Meter_Event.Pos_Data_Inbuff = Get_Meter_Event.Str_Payload.Length_u16;
             }
            Copy_STime_fromsTime(&sTimeEvent, 0, 0, 0, 0, 0, 0); //reset lai stime de so sanh
            Get_Meter_Event.Flag_ui8 = 0; //neu doc xong start cho bien nay ve 0. de doc tiep stop.
        }
    }
}

void RME_Push_Mess_2 (void)
{
    
}
void RME_Push_Mess (void)
{
    uint8_t         BuffNum[15];
	truct_String    Str_Data_Write={&BuffNum[0], 0};
    uint8_t         Temp_BBC = 0;
    truct_String    StrTemp;
    
	//Day du lieu len Hes
    StrTemp.Data_a8 = &MeterTuTiDataBuff[0];
    StrTemp.Length_u16 = Get_Meter_TuTi.Pos_Obis_Inbuff;
    Copy_String_toTaget(&Get_Meter_Event.Str_Payload, Get_Meter_Event.Pos_Obis_Inbuff, &StrTemp);  //ghi obis vao
    Get_Meter_Event.Pos_Obis_Inbuff += StrTemp.Length_u16;
    Get_Meter_Event.Pos_Data_Inbuff = Get_Meter_Event.Str_Payload.Length_u16;
    
    StrTemp.Data_a8 = &MeterTuTiDataBuff[Get_Meter_TuTi.Pos_Obis_Inbuff];
    StrTemp.Length_u16 = Get_Meter_TuTi.Pos_Data_Inbuff - Get_Meter_TuTi.Pos_Obis_Inbuff;
    Copy_String_toTaget(&Get_Meter_Event.Str_Payload, Get_Meter_Event.Pos_Data_Inbuff, &StrTemp);  //ghi data vao
    Get_Meter_Event.Pos_Data_Inbuff = Get_Meter_Event.Str_Payload.Length_u16;
    
    Get_Meter_Event.Numqty += Get_Meter_TuTi.Numqty;
    
    Reset_Buff(&Str_Data_Write);   
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
    
    Push_Even_toQueue(0);
}


void RME_Push_Mess_TSVH (void)
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


/*
  * @brief Read infomation from Meter
  * @param *Meter_Comm: Meter_Comm_Struct type
  * @param mess_type:
  *        This parameter can be one of the following values:
	*          @arg 0:MeterInfo
	*          @arg 1:MeterAlert
	*          @arg 2:MeterBilling
	*          @arg 3:MeterEvent
	*          @arg 4:MeterProfile
	* @param *ptr_code_table: pointer to code table
  *        This parameter can be one of the following values:
	*          @arg START_MeterInfoMessIDTable[0]
	*          @arg START_MeterAlertMessIDTable[0]
	*          @arg MeterBillingMessIDTable[0]
	*          @arg 
	*          @arg 
	* @param *ExtractDataFunc: pointer to extract data function
	* @param *DataHandle: pointer to push data function
	* @retval Finish status: 0-unfinished 1-finished
 */
uint8_t Meter_Comm_Func (Meter_Comm_Struct *Meter_Comm,uint8_t mess_type, uint8_t *ptr_code_table, void (*ExtractDataFunc)(void), void (*DataHandle)(void))
{
	uint8_t mReVal = 0;
	
	//Start read
	Meter_Comm->Reading_ui8 = 1;
    Meter_Comm->Step_ui8 = 1;
    Meter_Comm->Mess_Step_ui8 = 0;
    
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
					UART2_Mess_Checksum();
					
					if (UART1_Control.Mess_Status_ui8 == 2)
					{
						Meter_Comm->Success_Read_Mess_ui32++;
						//Extract raw data
						ExtractDataFunc();
						//Reset UART2 status
                        Init_UART2();
//						UART1_Control.Mess_Pending_ui8 = 0;
//						UART1_Control.Mess_Length_ui16 = 0;
//						UART1_Control.Mess_Status_ui8 = 0;
						//Goto next message
						Meter_Comm->Mess_Step_ui8++;
						ptr_code_table++;
						Meter_Comm->Flag_ui32 = 0;
						Meter_Comm->Error_ui8 = 0x00;
						Meter_Comm->Step_ui8 = 1;
						//If all mess sent -> finish
						if (*ptr_code_table == 0xFF)
						{
							//Push mess or check alarm
							DataHandle();
							mReVal = 1;
							
							Meter_Comm->Step_ui8 = 2;
						}
					}
					else
					{
						Meter_Comm->Error_Wrong_Mess_Format_ui32++;
						osDelay(2000);
						//Try to resend 3 times, if Meter norespond, jump out of reading cycle
						Meter_Comm->Step_ui8 = 1;
						Meter_Comm->Flag_ui32++;
						//Reset UART
                        Init_UART2();			
						if (Meter_Comm->Flag_ui32 > 2)
						{
							Meter_Comm->Flag_ui32 = 0;
							Meter_Comm->Step_ui8 = 2;
							Meter_Comm->Error_ui8 = 0x02;
						}
					}
				}
				else	//If Meter no respond after 2000ms: wait 2000ms - resend the message (3 times)
				{
					Meter_Comm->Error_Meter_Norespond_ui32++;
					Meter_Comm->Step_ui8 = 1;
					Meter_Comm->Flag_ui32++;
					//Reset UART
                    Init_UART2();
					if (Meter_Comm->Flag_ui32 > 2)
					{
						Meter_Comm->Flag_ui32 = 0;
						Meter_Comm->Step_ui8 = 2;
						Meter_Comm->Error_ui8 = 0x01;
					}
				}
				break;
			case 1:
				osDelay(10); // Delay for send message
				START_Fill_Mess(Meter_Comm->Mess_Step_ui8,mess_type);
				HAL_UART_Transmit(&UART_METER,&Read_Meter_Info_Template[0],17,1000);
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
				Meter_Comm->Data_Buff_Pointer_ui16 = 10;
				break;					
			default:
				break;				
		}
	}
	return mReVal;
}

ST_TIME_FORMAT  	sTimeStartlpf;
ST_TIME_FORMAT  	sTimeStoplpf;
void RM_Load_Prof_Func (void) //o cho ham doc lpf
{
	uint16_t 	temp=0;
	uint8_t 	ChecksumByte=0;
	uint8_t 	i=0;
	uint8_t     FlagFirst = 0;
    uint32_t            DiffValue = 0;
	uint32_t            SecondStart = 0;
    
    //xu ly ngay doc de cho doc tung ngay 1
    if(sInformation.Flag_Request_lpf != 1)
    {
        sTimeStartlpf.sec   = 0; 
        sTimeStartlpf.min   = 0;
        sTimeStartlpf.hour  = 0;
        sTimeStartlpf.date  = Read_Meter_LProf_Day[0];
        sTimeStartlpf.month = Read_Meter_LProf_Day[1];
        sTimeStartlpf.year  = Read_Meter_LProf_Day[2];
        
        sTimeStoplpf.sec   = 59; 
        sTimeStoplpf.min   = 59;
        sTimeStoplpf.hour  = 23;
        sTimeStoplpf.date  = Read_Meter_LProf_Day[0];
        sTimeStoplpf.month = Read_Meter_LProf_Day[1];
        sTimeStoplpf.year  = Read_Meter_LProf_Day[2];
    }else
    {
        Copy_STime_fromsTime(&sTimeStartlpf, sInformation.StartTime_GetLpf.year, sInformation.StartTime_GetLpf.month, sInformation.StartTime_GetLpf.date, 
                             sInformation.StartTime_GetLpf.hour, sInformation.StartTime_GetLpf.min,sInformation.StartTime_GetLpf.sec );
        Copy_STime_fromsTime(&sTimeStoplpf, sInformation.EndTime_GetLpf.year, sInformation.EndTime_GetLpf.month, sInformation.EndTime_GetLpf.date, 
                             sInformation.EndTime_GetLpf.hour, sInformation.EndTime_GetLpf.min,sInformation.EndTime_GetLpf.sec );
    }
    
    while (sInformation.Flag_Stop_ReadLpf == 0)
    { 
        Init_Meter_LProf_Struct();
        FlagFirst = 0;
        Get_Meter_LProf.Reading_ui8 = 1;
        while((Get_Meter_LProf.Reading_ui8 == 1) && (sInformation.Flag_Stop_ReadLpf == 0))
        {
            switch(Get_Meter_LProf.Step_ui8)
            {
                case 0: //step nay de lay start record va num record read
                    Read_Meter_LProf_Template1[15] = UInt8_To_Hex(sTimeStartlpf.date) + 0x33;   //gui lenh ngay doc 
                    Read_Meter_LProf_Template1[16] = UInt8_To_Hex(sTimeStartlpf.month) + 0x33;
                    Read_Meter_LProf_Template1[17] = UInt8_To_Hex(sTimeStartlpf.year) + 0x33;
                
                    for (i=3;i<18;i++)
                        temp += Read_Meter_LProf_Template1[i];
                    
                    ChecksumByte = (uint8_t)(temp&0x00FF);
                    Read_Meter_LProf_Template1[18] = ChecksumByte;
                    HAL_UART_Transmit(&UART_METER,&Read_Meter_LProf_Template1[0],20,1000);
                    Get_Meter_LProf.Step_ui8 = 1;
                    break;
                case 1:
                    Get_Meter_LProf.Total_Mess_Sent_ui32++;
                    //Wait for respond message from Meter in 2000ms: checksum - extract raw data - save to buffer - goto next message
                    if (osSemaphoreWait(bsUART2PendingMessHandle,5000) == osOK)
                    {
                        //Checksum
                        UART2_Mess_Checksum();
                        
                        if (UART1_Control.Mess_Status_ui8 == 2)
                        {
                            Get_Meter_LProf.Success_Read_Mess_ui32++;
                            //Extract raw data
                            if (Get_Meter_LProf.First_Mess_Ok_ui8 == 0)
                            {
                                RMLP_Extract_First_Adress();
                                if ((Get_Meter_LProf.Mess_Step_ui8%8)>0)
                                    Get_Meter_LProf.Mess_To_Send_ui8 = Get_Meter_LProf.Mess_Step_ui8/8 + 1; 
                                else
                                    Get_Meter_LProf.Mess_To_Send_ui8 = Get_Meter_LProf.Mess_Step_ui8/8;
                                Get_Meter_LProf.First_Mess_Ok_ui8 = 1;
                            }
                            else
                            {
                                START_RMLP_Extract_Data();
                                Get_Meter_LProf.Mess_Count_ui8++;
    //							if ((Get_Meter_LProf.Mess_Count_ui8%6)==0)   //6 record thi dong goi gui 1 lan. de k bi tra buff data. c� the tang giam gia tri nay
    //							{
    //								if (Get_Meter_LProf.Mess_Count_ui8 < Get_Meter_LProf.Mess_To_Send_ui8)
    //								{
    //									START_RMLP_Push_Mess();
    //								}
    //							}
                            }
                            //Reset UART2 status
                            Init_UART2();
                            //Goto next message
                            Get_Meter_LProf.Flag_ui32 = 0;
                            Get_Meter_LProf.Error_ui8 = 0x00;
                            Get_Meter_LProf.Step_ui8 = 2;
                            if (Get_Meter_LProf.Mess_Count_ui8 >= Get_Meter_LProf.Mess_To_Send_ui8)
                            {
    //							START_RMLP_Push_Mess();
                                Get_Meter_LProf.Step_ui8 = 3;
                            }
                        }
                        else
                        {
                            Get_Meter_LProf.Error_Wrong_Mess_Format_ui32++;
                            osDelay(2000);
                            //Try to resend 3 times, if Meter norespond, jump out of reading cycle
                            if (Get_Meter_LProf.First_Mess_Ok_ui8 == 0)
                                Get_Meter_LProf.Step_ui8 = 0;
                            else
                                Get_Meter_LProf.Step_ui8 = 1;
                            Get_Meter_LProf.Flag_ui32++;
                            //Reset UART
                            Init_UART2();                            
                            if (Get_Meter_LProf.Flag_ui32 > 2)
                            {
                                Get_Meter_LProf.Flag_ui32 = 0;
                                Get_Meter_LProf.Step_ui8 = 3;
                                Get_Meter_LProf.Error_ui8 = 0x02;
                            }
                        }
                    }
                    else	//If Meter no respond after 2000ms: wait 2000ms - resend the message (3 times)
                    {
                        Get_Meter_LProf.Error_Meter_Norespond_ui32++;
                        osDelay(2000);
                        if (Get_Meter_LProf.First_Mess_Ok_ui8 == 0)
                            Get_Meter_LProf.Step_ui8 = 0;
                        else
                            Get_Meter_LProf.Step_ui8 = 1;
                        Get_Meter_LProf.Flag_ui32++;
                        //Reset UART
                        Init_UART2();
                        if (Get_Meter_LProf.Flag_ui32 > 2)
                        {
                            Get_Meter_LProf.Flag_ui32 = 0;
                            Get_Meter_LProf.Step_ui8 = 3;
                            Get_Meter_LProf.Error_ui8 = 0x01;
                        }
                    }				
                    break;
                case 2:  // start code = 01 00 thi sao. sau khi cong nhu nay se thanh 0200.  //hay la k co khi nao start record ==00
                    Read_Meter_LProf_Template2[15] = Read_Meter_LProf_StartCode[1]+Get_Meter_LProf.Mess_Count_ui8+0x33;   
                    if (Read_Meter_LProf_Template2[15]==0x33)
                        if(FlagFirst != 0)
                            Read_Meter_LProf_StartCode[0]++;   //
                    Read_Meter_LProf_Template2[16] = Read_Meter_LProf_StartCode[0]+0x33;
                    temp = 0;
                    for (i=3;i<17;i++)
                        temp += Read_Meter_LProf_Template2[i];
                    
                    ChecksumByte = (uint8_t)(temp&0x00FF);
                    Read_Meter_LProf_Template2[17] = ChecksumByte;
                    HAL_UART_Transmit(&UART_METER,&Read_Meter_LProf_Template2[0],19,2000);
                    Get_Meter_LProf.Step_ui8 = 1;
                    FlagFirst = 1;    //xac dinh lan dau tien vao step 2
                    break;
                case 3:
                    Init_UART2();
                    //Jump out of reading cycle, wait for next cycle (after 30mins)
                    Get_Meter_LProf.Reading_ui8 = 0;
                    Get_Meter_LProf.Step_ui8 = 0;
                    Get_Meter_LProf.Mess_Step_ui8 = 0;	
                    Get_Meter_LProf.Mess_Count_ui8 = 0;
                    Get_Meter_LProf.Mess_To_Send_ui8 = 0;
                    Get_Meter_LProf.First_Mess_Ok_ui8 = 0;
                    //Reset pointer
                    Get_Meter_LProf.Data_Buff_Pointer_ui16 = 10;				
                    break;
                default:
                    break;
            }
        }
        //tang start time l�n 1 ngay
        SecondStart = HW_RTC_GetCalendarValue_Second(sTimeStartlpf, 0);
        SecondStart = SecondStart - SecondStart%86400;
        Epoch_to_date_time(&sTimeStartlpf, SecondStart + 86400, 0); //cong them 1 ngay
                
        if(Check_DiffTime (sTimeStartlpf, sTimeStoplpf, &DiffValue) == 1)  //neu start lon hon stop time thi dung lai
            sInformation.Flag_Stop_ReadLpf = 1;
    }
    sInformation.Flag_Request_lpf = 0;
    sInformation.Flag_Stop_ReadLpf = 0;
}

uint8_t UInt8_To_Hex (uint8_t mNum)
{
	uint8_t temp1=0;
	uint8_t temp2=0;
	uint8_t mReVal=0;
	
	temp1 = mNum/10;
	temp2 = mNum%10;
	
	mReVal = ((temp1<<4)&0xF0) + temp2;
	
	return mReVal;
}

void RMLP_Extract_First_Adress(void)
{
	uint8_t 	temp1=0;
	uint8_t 	temp2=0;
	uint16_t	temp3=0;
	
	temp1 = UART1_Receive_Buff[16]-0x33;
	temp2 = UART1_Receive_Buff[17]-0x33;
	temp3 = temp2;
	temp3 = (temp3<<8)+temp1;
	temp3 = temp3/8;
	Read_Meter_LProf_StartCode[0] = (uint8_t)((temp3>>8)&0x00FF);   //tinh dc start record
	Read_Meter_LProf_StartCode[1] = (uint8_t)(temp3&0x00FF);
	
	temp1 = UART1_Receive_Buff[18]-0x33;
	temp2 = UART1_Receive_Buff[19]-0x33;
	temp3 = temp2;
	temp3 = (temp3<<8)+temp1;
	Get_Meter_LProf.Mess_Step_ui8 = temp3;    //Mess step: tong so record ghi trong ngay
}


void START_RMLP_Extract_Data (void)
{
	uint8_t	i=0,j=0,k=0;
    ST_TIME_FORMAT  sTime_temp;
    uint8_t         BuffNum[30];
    truct_String    Str_Data_Write={&BuffNum[0], 0};
    uint64_t        TempDataU64 = 0;
    truct_String    StrTemp;
    uint32_t        diffTimeStampStart = 0;
    uint32_t LasTimeRecord = 0;
    
    
    PeriodLpf_Min = 30;
	//Fill data into buffer
	k = UART1_Control.Mess_Length_ui16-16;
	while (i<k)
	{
		j = i%13;
		switch (j)
		{
			case 0:
                //
                sTime_temp.year     = ConvertHex_to_Dec(UART1_Receive_Buff[6+i+13] - 0x33);  // 8B 56 5B 3C 48 - (33) = 58 23 28 09 15, gi� tr? luu t? 23 gi? 58 ph�t ng�y 28/09/2015
                sTime_temp.month    = ConvertHex_to_Dec(UART1_Receive_Buff[5+i+13] - 0x33);
                sTime_temp.date     = ConvertHex_to_Dec(UART1_Receive_Buff[4+i+13] - 0x33);
                sTime_temp.hour     = ConvertHex_to_Dec(UART1_Receive_Buff[3+i+13] - 0x33);
                sTime_temp.min      = ConvertHex_to_Dec(UART1_Receive_Buff[2+i+13] - 0x33);
                
                //neu nho hon start time thi bo qua va tiep tuc dong goi
                if((Check_DiffTime(sTime_temp, sTimeStartlpf, &diffTimeStampStart) == 0) || ((sTime_temp.year == 0) && (sTime_temp.month == 0) && (sTime_temp.date == 0) 
                                                                                             && (sTime_temp.hour == 0) && (sTime_temp.min == 0)))
                {
                    i += 13;
                    Get_Meter_LProf.Flag_Start_Pack = 0;  
                    break;
                }
                //neu lon hon end time thi cho Stop luon vi�c doc. doc xong
                if(Check_DiffTime(sTimeStoplpf, sTime_temp, &diffTimeStampStart) == 0)
                {
                    sInformation.Flag_Stop_ReadBill = 1;  //dung flag nay de dung vi�c doc. vi qua thoi gian stop time.
                    return;
                }
                
                if(Get_Meter_LProf.Flag_Start_Pack == 0)
                {
                    Get_Meter_LProf.Flag_Start_Pack = 1;
                    Reset_Buff(&Get_Meter_LProf.Str_Payload);
                    //wrire header TSVH vao
                    Pack_Header_lpf_Pushdata103();

                    Reset_Buff(&Str_Data_Write);
                    //
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
                    Copy_String_STime(&Str_Data_Write, sTime_temp);
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                    //
                    Copy_String_2(&Str_Data_Write, &Str_event_Temp);
//                    //period
//                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
//                    Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t)(PeriodLpf_Min) ,0);
//                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')'; 
//                    //num chanel
//                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
//                    Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) Get_Meter_LProf.Numqty, 0);
//                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')'; 
                    
                    Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.PosNumqty, &Str_Data_Write);//ghi stime vao
                    Get_Meter_LProf.PosNumqty += Str_Data_Write.Length_u16;
                    Get_Meter_LProf.Pos_Obis_Inbuff += Str_Data_Write.Length_u16;
                    Get_Meter_LProf.Pos_Data_Inbuff  = Get_Meter_LProf.Str_Payload.Length_u16;
                    
                    Get_Meter_LProf.Numqty = 7;
                    
                    Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Obis_Inbuff, &Str_Ob_lpf_type2[0]);//ghi dai luong 1
                    Get_Meter_LProf.Pos_Obis_Inbuff += Str_Ob_lpf_type2[0].Length_u16; 
                    Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Obis_Inbuff, &Str_Ob_lpf_type2[1]);//ghi dai luong 2
                    Get_Meter_LProf.Pos_Obis_Inbuff += Str_Ob_lpf_type2[1].Length_u16; 
                    Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Obis_Inbuff, &Str_Ob_lpf_type2[2]);//ghi dai luong 3
                    Get_Meter_LProf.Pos_Obis_Inbuff += Str_Ob_lpf_type2[2].Length_u16; 
                    Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Obis_Inbuff, &Str_Ob_lpf_type2[3]);//ghi dai luong 4
                    Get_Meter_LProf.Pos_Obis_Inbuff += Str_Ob_lpf_type2[3].Length_u16; 
                    Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Obis_Inbuff, &Str_Ob_Tu);//ghi tu
                    Get_Meter_LProf.Pos_Obis_Inbuff += Str_Ob_Tu.Length_u16; 
                    Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Obis_Inbuff, &Str_Ob_Ti);//ghi ti
                    Get_Meter_LProf.Pos_Obis_Inbuff += Str_Ob_Ti.Length_u16; 
                    Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Obis_Inbuff, &He_So_Nhan);//ghi hsn
                    Get_Meter_LProf.Pos_Obis_Inbuff += He_So_Nhan.Length_u16; 
                    
                    Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;  
                      
                    if(LasTimeRecord == 0)
                    {
                        PeriodLpf_Min = 30;
                        LasTimeRecord = HW_RTC_GetCalendarValue_Second(sTime_temp, 0);
                    }else
                    {
                        PeriodLpf_Min = (HW_RTC_GetCalendarValue_Second(sTime_temp, 0) - LasTimeRecord)/60;  //tinh ra min
                        LasTimeRecord = HW_RTC_GetCalendarValue_Second(sTime_temp, 0);
                    }                
                }
//                else 
//                {
//                    *(Get_Meter_LProf.Str_Payload.Data_a8 + Get_Meter_LProf.Str_Payload.Length_u16++) = 0x0D;
//                    *(Get_Meter_LProf.Str_Payload.Data_a8 + Get_Meter_LProf.Str_Payload.Length_u16++) = 0x0A;
//                    Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
                    
//                    PeriodLpf_Min = (HW_RTC_GetCalendarValue_Second(sTime_temp, 0) - LasTimeRecord)/60;  //tinh ra min
//                    LasTimeRecord = HW_RTC_GetCalendarValue_Second(sTime_temp, 0);
//                }
                
				i += 5;
				break;
			case 5:
                TempDataU64 = ConvertHex_to_Dec(UART1_Receive_Buff[i+16] - 0x33)*100 + ConvertHex_to_Dec(UART1_Receive_Buff[i+15] - 0x33);
                Reset_Buff(&Str_Data_Write);
                //copy ca chuoi du lieu bao gom ca  ().
                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) TempDataU64, START_SCALE_LPF);
                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                Copy_String_2(&Str_Data_Write, &Unit_Lpf_type2[0]);  //don vi
                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                  
                Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Data_Inbuff, &Str_Data_Write);
                Get_Meter_LProf.Pos_Obis_Inbuff += Str_Data_Write.Length_u16;
                Get_Meter_LProf.Pos_Data_Inbuff  = Get_Meter_LProf.Str_Payload.Length_u16;
                  
				i += 2;
				break;
			case 7:
                TempDataU64 = ConvertHex_to_Dec(UART1_Receive_Buff[i+16] - 0x33)*100 + ConvertHex_to_Dec(UART1_Receive_Buff[i+15] - 0x33);
                Reset_Buff(&Str_Data_Write);
                //copy ca chuoi du lieu bao gom ca  ().
                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) TempDataU64, START_SCALE_LPF);
                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                Copy_String_2(&Str_Data_Write, &Unit_Lpf_type2[1]);  //don vi
                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                  
                Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Data_Inbuff, &Str_Data_Write);
                Get_Meter_LProf.Pos_Obis_Inbuff += Str_Data_Write.Length_u16;
                Get_Meter_LProf.Pos_Data_Inbuff  = Get_Meter_LProf.Str_Payload.Length_u16;
				i += 2;			
				break;
			case 9:
                TempDataU64 = ConvertHex_to_Dec(UART1_Receive_Buff[i+16] - 0x33)*100 + ConvertHex_to_Dec(UART1_Receive_Buff[i+15] - 0x33);
                Reset_Buff(&Str_Data_Write);
                //copy ca chuoi du lieu bao gom ca  ().
                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) TempDataU64, START_SCALE_LPF);
                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                Copy_String_2(&Str_Data_Write, &Unit_Lpf_type2[2]);  //don vi
                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                  
                Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Data_Inbuff, &Str_Data_Write);
                Get_Meter_LProf.Pos_Obis_Inbuff += Str_Data_Write.Length_u16;
                Get_Meter_LProf.Pos_Data_Inbuff  = Get_Meter_LProf.Str_Payload.Length_u16;
				i += 2;			
				break;
			case 11:
                TempDataU64 = ConvertHex_to_Dec(UART1_Receive_Buff[i+16] - 0x33)*100 + ConvertHex_to_Dec(UART1_Receive_Buff[i+15] - 0x33);
                Reset_Buff(&Str_Data_Write);
                //copy ca chuoi du lieu bao gom ca  ().
                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) TempDataU64, START_SCALE_LPF);
                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                Copy_String_2(&Str_Data_Write, &Unit_Lpf_type2[3]);  //don vi
                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';
                  
                Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Data_Inbuff, &Str_Data_Write);
                Get_Meter_LProf.Pos_Obis_Inbuff += Str_Data_Write.Length_u16;
                Get_Meter_LProf.Pos_Data_Inbuff  = Get_Meter_LProf.Str_Payload.Length_u16;
                
                //them ti tu data vao

                StrTemp.Data_a8 = &MeterTuTiDataBuff[Get_Meter_TuTi.Pos_Obis_Inbuff];
                StrTemp.Length_u16 = Get_Meter_TuTi.Pos_Data_Inbuff - Get_Meter_TuTi.Pos_Obis_Inbuff;
                Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.Pos_Data_Inbuff, &StrTemp);  //ghi data vao
                Get_Meter_LProf.Pos_Data_Inbuff = Get_Meter_LProf.Str_Payload.Length_u16;
                
                START_RMLP_Push_Mess();
				i += 2;	
				break;
		}
	}
}

void START_RMLP_Push_Mess (void)
{
    uint8_t         BuffNum[30];
    truct_String    Str_Data_Write={&BuffNum[0], 0};
    
    //ghi Period ,numqty
    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
    Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t)(PeriodLpf_Min) ,0);
    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')'; 
    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '('; 
    Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) Get_Meter_LProf.Numqty, 0);
    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')'; 
    
    Copy_String_toTaget(&Get_Meter_LProf.Str_Payload, Get_Meter_LProf.PosNumqty, &Str_Data_Write);
    Get_Meter_LProf.PosNumqty += Str_Data_Write.Length_u16;
    Get_Meter_LProf.Pos_Obis_Inbuff += Str_Data_Write.Length_u16;
    Get_Meter_LProf.Pos_Data_Inbuff  = Get_Meter_LProf.Str_Payload.Length_u16;
    //dong goi
    Send_MessLpf_toQueue();
    Get_Meter_LProf.Flag_Start_Pack = 0;
}


uint8_t START_Get_Meter_ID (uint32_t Tempvalue)
{
	uint8_t     GetMeterIDSuccess = 0;
	uint8_t     GetMeterIDRetry = 1;
	uint8_t	    i=0,j=0;
	uint8_t	    TempChecksum=0;
	uint8_t     Temp_MeterID[12];
    uint8_t     fMarkFirstZero = 0;
	
	while (GetMeterIDRetry--)
	{
		HAL_UART_Transmit(&UART_METER, &Read_Meter_ID_Message[0], 17, 2000);
        
		if (osSemaphoreWait(bsUART2PendingMessHandle,5000) == osOK)   
		{ 
			//Check sum
			UART2_Mess_Checksum();
			
			if (UART1_Control.Mess_Status_ui8 == 2)
			{
				//Check Meter ID. 28/7/21: Sua Code cu 12 byte. Bay gio check thay 00 o truoc thi se bo di
				for (i=0;i<6;i++)
				{
                    if((UART1_Receive_Buff[7-i] == 0) && (fMarkFirstZero == 0))  //bo qua 2 chu so 0 trong byte dau tien
                        continue;
                    
                    fMarkFirstZero = 1;
					Temp_MeterID[j] = (UART1_Receive_Buff[7-i]>>4) + 0x30;
					j++;
					Temp_MeterID[j] = (UART1_Receive_Buff[7-i]&0x0F) + 0x30;
					j++;
				}
                //
				for (i=0;i<j;i++)
				{
					if ((*(sDCU.sMeter_id_now.Data_a8 + i)) != Temp_MeterID[i])
					{
						Read_Meter_ID_Change = 1;
						break;
					}
				}

				if (Read_Meter_ID_Change == 1)
				{
					for (i = 0; i < METER_LENGTH; i++)
						aMeter_ID[i] =  0;
					for (i=0;i<j;i++)
						aMeter_ID[i] =  Temp_MeterID[i];
                    sDCU.sMeter_id_now.Length_u16 = j;
				}

				//Put meter id into message template. O day c� 2-7
				for (i=2;i<8;i++)
				{
					Read_Meter_Info_Template[i+2] = UART1_Receive_Buff[i];
					Read_Meter_LProf_Template1[i+2] = UART1_Receive_Buff[i];
					Read_Meter_LProf_Template2[i+2] = UART1_Receive_Buff[i];
				}
				
				//Calculate checksum byte meter info. T�nh byte check sum v? tri thu 3 trong mang cac thanh ghi Infor va event
				TempChecksum = 0;
				for (i=3;i<13;i++)
					TempChecksum += Read_Meter_Info_Template[i];
				for (i=0;i<START_MAX_REGIS_INFOR;i++)
					Read_Meter_Info_Code[i][2] = TempChecksum + Read_Meter_Info_Code[i][0] + Read_Meter_Info_Code[i][1];   //tinh lai cho nay. vi moi them cac thanh ghi moi
				
				//Calculate checksum byte meter event
				for (i=0;i<START_MAX_REGIS_EVENT;i++)
					Read_Meter_Event_Code[i][2] = TempChecksum + Read_Meter_Event_Code[i][0] + Read_Meter_Event_Code[i][1];
				
                Init_UART2();			
				GetMeterIDSuccess = 1;
				break;
			} else
                Init_UART2();
		} else
            Init_UART2();
	}
	
	return GetMeterIDSuccess;
}

//
uint8_t START_CheckResetReadMeter(uint32_t Timeout)
{
	// Reset neu doc sai cong to
		if ((sDCU.Status_Meter_u8 == 0) &&
				(Check_Time_Out(sDCU.LandMark_Count_Reset_Find_Meter,600000) == TRUE)) // 10p
		{
			osDelay(Timeout); // 5p
			Read_Meter_ID_Success = START_Get_Meter_ID(0);
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


uint8_t START_Check_Meter(void)
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
			Read_Meter_ID_Success = START_Get_Meter_ID(0);
        }
	} 
    return 1;
}  




void Pack_MD_Value_Time (uint8_t Type, Meter_Comm_Struct* Get_Meter, Struct_Maxdemand_Value* Struct_MD, uint8_t FirstobisHex, struct_Obis_START* StructObis, uint8_t Max_Obis)  //chuyen sang hex
{
    uint16_t        i = 0;
    uint8_t         Num_Bieu = 0;
    uint32_t        Temp_data_32 = 0;
    uint8_t         TempObis = 0;
    uint8_t         BuffNum[30];
    truct_String    Str_Data_Write={&BuffNum[0], 0};
    int8_t          Row = 0;

    
    if(Struct_MD->NumTariff > Struct_MD->NumValue_MD) 
        Num_Bieu = Struct_MD->NumValue_MD;
    else Num_Bieu = Struct_MD->NumTariff;
    
    if(Num_Bieu > 5) Num_Bieu = 5;    //gioi han 5 goi max demand bao gom ca tong
    Row = Check_Row_Obis103(&StructObis[0],FirstobisHex, Max_Obis);
    
    for(i = 0; i < Num_Bieu; i++)
    {
        Reset_Buff(&Str_Data_Write);
        TempObis = Row + i*4;
        if(TempObis >= Max_Obis) break;   //neu qua so obis trong mang thi dung lai
        Temp_data_32 = Struct_MD->Value_u32[i];
        
        if(Type == DATA_OPERATION)
        {
            if(StructObis[TempObis].Str_Obis != NULL)
            {
                Copy_String_toTaget(&Get_Meter->Str_Payload, Get_Meter->Pos_Obis_Inbuff, StructObis[TempObis].Str_Obis);  
                Get_Meter->Pos_Obis_Inbuff += StructObis[TempObis].Str_Obis->Length_u16;
                Get_Meter->Numqty++;
                Get_Meter->Pos_Data_Inbuff = Get_Meter->Str_Payload.Length_u16;

                //o d�y l� so khong dau
                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) (Temp_data_32), StructObis[TempObis].scale);
                if(StructObis[TempObis].Str_Unit != NULL)
                {
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                    Copy_String_2(&Str_Data_Write, StructObis[TempObis].Str_Unit);  //don vi bieu
                }

                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';

                Copy_String_toTaget(&Get_Meter->Str_Payload, Get_Meter->Pos_Data_Inbuff, &Str_Data_Write);
                Get_Meter->Pos_Data_Inbuff = Get_Meter->Str_Payload.Length_u16;
                 //ghi stime vao
                *(Get_Meter->Str_Payload.Data_a8 + Get_Meter->Str_Payload.Length_u16++) = '(';
                Copy_String_STime(&Get_Meter->Str_Payload, Struct_MD->sTime[i]);
                *(Get_Meter->Str_Payload.Data_a8 + Get_Meter->Str_Payload.Length_u16++) = ')';
                Get_Meter->Pos_Data_Inbuff = Get_Meter->Str_Payload.Length_u16;
            }
        }else if(Type == DATA_HISTORICAL)
        {
            if(StructObis[TempObis].StrObis_Bill != NULL)
            {
                Copy_String_toTaget(&Get_Meter->Str_Payload, Get_Meter->Pos_Obis_Inbuff, StructObis[TempObis].StrObis_Bill);  
                Get_Meter->Pos_Obis_Inbuff += StructObis[TempObis].StrObis_Bill->Length_u16;
                Get_Meter->Numqty++;
                Get_Meter->Pos_Data_Inbuff = Get_Meter->Str_Payload.Length_u16;
                //o d�y l� so khong dau
                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '(';
                Pack_HEXData_Frame_Uint64(&Str_Data_Write, (uint64_t) (Temp_data_32), StructObis[TempObis].scale);
                if(StructObis[TempObis].Str_Unit != NULL)
                {
                    *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = '*';
                    Copy_String_2(&Str_Data_Write, StructObis[TempObis].Str_Unit);  //don vi bieu
                }

                *(Str_Data_Write.Data_a8 + Str_Data_Write.Length_u16++) = ')';

                Copy_String_toTaget(&Get_Meter->Str_Payload, Get_Meter->Pos_Data_Inbuff, &Str_Data_Write);
                Get_Meter->Pos_Data_Inbuff = Get_Meter->Str_Payload.Length_u16;
                 //ghi stime vao
                *(Get_Meter->Str_Payload.Data_a8 + Get_Meter->Str_Payload.Length_u16++) = '(';
                Copy_String_STime(&Get_Meter->Str_Payload, Struct_MD->sTime[i]);
                *(Get_Meter->Str_Payload.Data_a8 + Get_Meter->Str_Payload.Length_u16++) = ')';
                Get_Meter->Pos_Data_Inbuff = Get_Meter->Str_Payload.Length_u16;
            }
        }
    }
}


int8_t Check_Row_Obis103 (struct_Obis_START* structObis, uint8_t ObisHex, uint8_t MaxObis)
{
    uint8_t  i = 0;
    
    for(i = 0; i < MaxObis; i++)
      if(structObis[i].ObisHex == ObisHex) return i; 
    
    return -1;
}





