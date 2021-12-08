
#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"

#include "variable.h"
#include "My_LogDCU.h"
#include "S25FL216K.h"
#include "myuart.h"
#include "pushdata.h"

/*-----------------------------Define---------------*/


/*-----------------------------var struct---------------*/
uint8_t             aLogDCU_Total[MAX_LENGTH_BUFF_LOG_TOTAL];
uint8_t             aCutLogToSave[200];
uint8_t             aStringHexParam[200];
struct_Log_Manage   sLogDCU;

/*-----------------------------Function---------------*/
void InitLogDCU_Var (void)
{
    sLogDCU.LogUart.Data_a8 = &aUART1_Receive_End[0];
    sLogDCU.LogUart.Length_u16 = 0;
    sLogDCU.strLogTotal.Data_a8 = &aLogDCU_Total[0];
    sLogDCU.strLogTotal.Length_u16 = 0;
    sLogDCU.strCutLog.Data_a8 = &aCutLogToSave[0];
    sLogDCU.strCutLog.Length_u16 = 0;
}

void _fLogDCU_ParaImportant (void)
{
    uint8_t         count = 0, leng = 0, i = 0;
    uint8_t         aLogVar[100];
    
    //Ghi data
    for(i = 0; i < 4; i++)
      aLogVar[count++] = (uint8_t) ((pos_MeterInfor_To_Send_u32 >> (8*(3- i))) & 0xFF);
    aLogVar[count++] = 0xFF; 
    //Pos Minfo sent
    for(i = 0; i < 4; i++)
      aLogVar[count++] = (uint8_t) ((pos_MeterInfor_Sent_u32 >> (8*(3- i))) & 0xFF);
    aLogVar[count++] = 0xFF;
    //Pos MLpf to sent
    for(i = 0; i < 4; i++)
      aLogVar[count++] = (uint8_t) ((pos_MeterLPF_To_Send_u32 >> (8*(3- i))) & 0xFF);
    aLogVar[count++] = 0xFF;
    //Pos Mlpf sent
    for(i = 0; i < 4; i++)
      aLogVar[count++] = (uint8_t) ((pos_MeterLPF_Sent_u32 >> (8*(3- i))) & 0xFF);
    aLogVar[count++] = 0xFF;
    //Pos MBill to sent
    for(i = 0; i < 4; i++)
      aLogVar[count++] = (uint8_t) ((pos_MeterBill_To_Sent_u32 >> (8*(3- i))) & 0xFF);
    aLogVar[count++] = 0xFF;
    //Pos MBill sent
    for(i = 0; i < 4; i++)
      aLogVar[count++] = (uint8_t) ((pos_MeterBill_Sent_u32 >> (8*(3- i))) & 0xFF);
    aLogVar[count++] = 0xFF;
    for(i = 0; i < 4; i++)
      aLogVar[count++] = (uint8_t) ((RT_Count_Systick_u32 >> (8*(3- i))) & 0xFF);
    aLogVar[count++] = 0xFF;
    //c?c vari fix thuoc control sim
    aLogVar[count++] = sSim900_status.Step_Control_u8;
    aLogVar[count++] = 0xFF;
    aLogVar[count++] = sSim900_status.Radio_4G_GSM;
    aLogVar[count++] = 0xFF;
    aLogVar[count++] = sSim900_status.CheckConnectATCommand;
    aLogVar[count++] = 0xFF;
    
    leng = ConvertHex_2StringHEX(&aLogVar[0], count, &aStringHexParam[0], 0);
    
    _fPackStringToLog(&aStringHexParam[0], leng);
}

void _fPackStringToLog (uint8_t* Buff, uint8_t length)
{
    uint16_t i = 0;
    
    if (sLogDCU.strLogTotal.Length_u16 + length < MAX_LENGTH_BUFF_LOG_TOTAL)   //Con cho de luu log vao thi moi cho ghi vao
        for (i = 0; i < length; i++)
            *(sLogDCU.strLogTotal.Data_a8 + sLogDCU.strLogTotal.Length_u16++) = *(Buff + i);
    else 
        _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nWarning: Full Buff Log Total", 30, 1000);
}


uint8_t Check_New_Log_Modem (void)
{
    uint16_t  i = 0;
    
    if (sLogDCU.strLogTotal.Length_u16 != 0)
    {
        Reset_Buff(&sLogDCU.strCutLog);
        //Cat ra 190 (MAX_LENGTH_SAVE_LOG) byte dau tien trong total de gui sang queue log
        if (sLogDCU.strLogTotal.Length_u16 <= MAX_LENGTH_SAVE_LOG)
        {
            for (i = 0; i < sLogDCU.strLogTotal.Length_u16; i++)
                *(sLogDCU.strCutLog.Data_a8 + sLogDCU.strCutLog.Length_u16++) = *(sLogDCU.strLogTotal.Data_a8 + i);
            //Xoa buff total
            Reset_Buff(&sLogDCU.strLogTotal);
        } else
        {
            //Copy 190 byte dau tien. va day cac byte sau ve truoc
            for (i = 0; i < MAX_LENGTH_SAVE_LOG; i++)
                *(sLogDCU.strCutLog.Data_a8 + sLogDCU.strCutLog.Length_u16 ++) = *(sLogDCU.strLogTotal.Data_a8 + i);
            
            //Dich chuyen cac byte phia sau ve truoc 190 byte
            for (i = MAX_LENGTH_SAVE_LOG; i < sLogDCU.strLogTotal.Length_u16; i++)
                *(sLogDCU.strLogTotal.Data_a8 + i - MAX_LENGTH_SAVE_LOG) = *(sLogDCU.strLogTotal.Data_a8 + i);
            sLogDCU.strLogTotal.Length_u16 -= MAX_LENGTH_SAVE_LOG;
        }
        
        Packet_Data_Log_DCU_To_Form_Hes(&sLogDCU.strCutLog);
            
        return 1;
    }
    
    return 0;
}



uint8_t Packet_Data_Log_DCU_To_Form_Hes (truct_String *str)
{
    uint16_t        i = 0;
    uint8_t         Count = 0;
    uint32_t	    temp=0;
    
    uint8_t         aStrDec[15];
    truct_String    StrDec = {&aStrDec[0], 0};
    uint16_t        Numlog = 0;
    uint8_t         aTEMP_LOG[50]; 
    
    if (str->Length_u16 >= 195) 
        str->Length_u16 = 195;      //gioi han cho du Page 256b byte
    
    //Pos Minfo to sent
    aTEMP_LOG[Count++] = 'L';
    aTEMP_LOG[Count++] = 'o';
    aTEMP_LOG[Count++] = 'g';
    //Ghi numlog
    Numlog = (pos_DCU_u32 - ADDR_BASE_LOG_DCU)/ S25FL_PAGE_SIZE + 1; 
    StrDec.Length_u16 = 0;
    ConvertHexDatatoStringDec(Numlog, &StrDec);
    for (i = 0; i < StrDec.Length_u16; i++)
        aTEMP_LOG[Count++] = *(StrDec.Data_a8 + i);     //khoang 5 byte
    
    aTEMP_LOG[Count++] = '.';
    //thoi gian
    aTEMP_LOG[Count++] = (uint8_t) ((sRTC.year/10)%10 + 0x30);
    aTEMP_LOG[Count++] = (uint8_t) (sRTC.year%10 + 0x30);
    aTEMP_LOG[Count++] = '/';
    aTEMP_LOG[Count++] = (uint8_t) ((sRTC.month/10)%10 + 0x30);
    aTEMP_LOG[Count++] = (uint8_t) (sRTC.month%10 + 0x30);
    aTEMP_LOG[Count++] = '/';
    aTEMP_LOG[Count++] = (uint8_t) ((sRTC.date/10)%10 + 0x30);
    aTEMP_LOG[Count++] = (uint8_t) (sRTC.date%10 + 0x30);
    aTEMP_LOG[Count++] = ' ';
    
    aTEMP_LOG[Count++] = (uint8_t) ((sRTC.hour/10)%10 + 0x30);
    aTEMP_LOG[Count++] = (uint8_t) (sRTC.hour%10 + 0x30);
    aTEMP_LOG[Count++] = ':';
    aTEMP_LOG[Count++] = (uint8_t) ((sRTC.min/10)%10 + 0x30);
    aTEMP_LOG[Count++] = (uint8_t) (sRTC.min%10 + 0x30);
    aTEMP_LOG[Count++] = ':';
    aTEMP_LOG[Count++] = (uint8_t) ((sRTC.sec/10)%10 + 0x30);
    aTEMP_LOG[Count++] = (uint8_t) (sRTC.sec%10 + 0x30);
    aTEMP_LOG[Count++] = '.';                                        //doan stime 18 byte
    //Emei DCU
    for (i = 0; i < sDCU.sDCU_id.Length_u16; i++)
        aTEMP_LOG[Count++] = *(sDCU.sDCU_id.Data_a8 + i);
    aTEMP_LOG[Count++] = '.';                                        //ID dang de 12 byte + 1
    //status: meter type
    StrDec.Length_u16 = 0;
    Copy_String_2(&StrDec, &Str_MeterType_u8[sDCU.MeterType]);          //Meter type 8 byte
    for (i = 0; i < StrDec.Length_u16; i++)
        aTEMP_LOG[Count++] = *(StrDec.Data_a8 + i);
    aTEMP_LOG[Count++] = '.';
    
    //Chen header vao truoc content. Dich tât cac cac byte sang phia sau count vi tri
    for (i = 0; i < str->Length_u16; i++)
        *(str->Data_a8 + str->Length_u16 + Count - i - 1) = *(str->Data_a8 + str->Length_u16 - i - 1);
    
    for (i = 0; i < Count; i++)
        *(str->Data_a8 + i) = aTEMP_LOG[i];
    
    str->Length_u16 += Count;
    //tiep tuc ghi them cac du lieu khac
    *(str->Data_a8 + str->Length_u16++) = '.';                                        //noi dung = 256 -  header - tuti = 256 - 57 = 199 byte nua.
    //Tuti
    *(str->Data_a8 + str->Length_u16++) = '(';                                        //Fix tuti = 10 byte
    *(str->Data_a8 + str->Length_u16++) = '1';
    *(str->Data_a8 + str->Length_u16++) = '/';
    *(str->Data_a8 + str->Length_u16++) = '1';
    *(str->Data_a8 + str->Length_u16++) = ')';
    *(str->Data_a8 + str->Length_u16++) = '(';
    *(str->Data_a8 + str->Length_u16++) = '1';
    *(str->Data_a8 + str->Length_u16++) = '/';
    *(str->Data_a8 + str->Length_u16++) = '1';
    *(str->Data_a8 + str->Length_u16++) = ')';
    
    //dich ra sau 1 byte cho length
    for (i = 0; i < str->Length_u16; i++)
        *(str->Data_a8 + str->Length_u16 - i) = *(str->Data_a8 + str->Length_u16 - i - 1);
    
    str->Length_u16 += 1;
    *(str->Data_a8)  = str->Length_u16 + 1;
       
    //Generate checksum byte
    for (i = 0; i < str->Length_u16; i++)
        temp += *(str->Data_a8 + i);
    
    temp = temp & 0x000000FF;
    
    *(str->Data_a8 + str->Length_u16++) = (uint8_t) temp;  
      
    _fPrint_Via_Debug(&UART_SERIAL, str->Data_a8, str->Length_u16, 1000);
    
    return 1;
}

uint8_t _fReadLogDCU (uint16_t Index, uint8_t ReqBy)
{
    uint32_t        AddIndexLog = 0;
    uint8_t         Length = 0, i = 0, result = 0;
    uint8_t	        ChecksumByte=0;
    
    if(Index == 0)
       return 0;
    AddIndexLog = ADDR_BASE_LOG_DCU + (Index - 1) * MAX_LOG_MESS;
    
    if(AddIndexLog > ADDR_TOP_LOG_DCU)
      AddIndexLog = (ADDR_TOP_LOG_DCU/MAX_LOG_MESS) * MAX_LOG_MESS;
    
    Flash_S25FL_BufferRead(&ReadBackBuff[0], AddIndexLog, MAX_LOG_MESS);

    //If have data to send
    if (ReadBackBuff[0] != 0xFF)
    {
        Length = (uint16_t)ReadBackBuff[0];
        
        ChecksumByte = 0;
        for (i=0; i < Length-1; i++)
            ChecksumByte += ReadBackBuff[i];

        if (ChecksumByte == ReadBackBuff[Length-1])
        {
            switch(ReqBy)
            {
                case 1:  //gui qua terminal luon
                    HAL_UART_Transmit(&UART_SERIAL, &ReadBackBuff[1], Length - 2,1000);
                    result = 1;
                    break;
                case 3:  //gui sang queue de gui len server
                    Queue_Flash_ReadLog.Mess_Direct_ui8 = 1;
                    Queue_Flash_ReadLog.str_Flash.Length_u16 = Length - 2;
                    Queue_Flash_ReadLog.Mess_Status_ui8 = 0;
                    Queue_Flash_ReadLog.Mess_Type_ui8 = DATA_LOG_DCU;
                    Queue_Flash_ReadLog.str_Flash.Data_a8 = &ReadBackBuff[1];
                    
                    xQueueSend(qFlash_SIM900Handle,(void *)&ptrQueue_Flash_ReadLog,0);
                    result = 1;
                    break;
                default:
                    break;
            }
        }
    }else 
        result = 0;
    
    return result;
}



/*
    - khi cho Flag Req_ = 1 
    - Cung voi do phai set duoc: Index Start va Index Stop
    - Reset ben CountRead = 0
*/

void Check_Request_Read_Log (void)
{
    if(Manage_Flash.Error_ui8 == 0)
    {
        if(sLogDCU.fReq_SendLog == 1)        //Falg nay reset ve 0 se dung viec check doc log. 
        {
            if(sInformation.fReq_RLogByServer == 1)
            {
                if(Check_Time_Out(sLogDCU.Landmark_SendLog, 2000) == 1) //gui cach nhau 2s
                {
                    if(fReadIndexLog_to_Serial(sLogDCU.IndexStart, sLogDCU.IndexStop) == 0)         //gui log qua uart
                      sLogDCU.fReq_SendLog = 0;
                    sLogDCU.Landmark_SendLog = RT_Count_Systick_u32;
                }
            }
            else
            {
                if(Check_Time_Out(sLogDCU.Landmark_SendLog, 2000) == 1) //gui cach nhau 2s
                {
                    if(fReadIndexLog_to_Server(sLogDCU.IndexStart, sLogDCU.IndexStop) == 0)         //Gui log qua server
                        sLogDCU.fReq_SendLog = 0;
                    sLogDCU.Landmark_SendLog = RT_Count_Systick_u32;
                }
            }
        }
    }
}

/*
    - Request doc tu Index a - Indexb
    - Hai bien nay luu trong struc LogVar
        + truong hop a va b nam tren va duoi cua Pos hien tai
    - Doc ra truyen ra Uart thi de
    - Doc ra gui len server
        + Doc ra tai vi tri Index. Gui sang queue Flash meter.
        + Check Sent OK thi moi cho doc vi tri tiep theo
        + Timoue 20s. Neu khong gui thanh cong. return Fail
*/

uint8_t fReadIndexLog_to_Serial (uint16_t IndexStart, uint16_t IndexStop)
{
    uint16_t    Pos = 0;
    
    if(IndexStop >= MAX_INDEX_LOG)
        IndexStop = MAX_INDEX_LOG;
    //Tim vi tri doc ban tin ra
    Pos = (IndexStart + sLogDCU.CountRead)% (MAX_INDEX_LOG + 1);
    
    if(Pos == 0) 
        Pos = 1;
    
    if(Pos == IndexStop)            //Vi tri cuoi cung thi dung lai
        return 0;
    _fReadLogDCU(Pos, 1);
    
    sLogDCU.CountRead++;         //Tang vi tri doc len 1
    
    return 1;
}


uint8_t fReadIndexLog_to_Server (uint16_t IndexStart, uint16_t IndexStop)
{
    uint8_t     Result = 1;
    uint16_t    Pos = 0;
    
    if(IndexStop >= MAX_INDEX_LOG)
        IndexStop = MAX_INDEX_LOG;
    //Tim vi tri doc ban tin ra
    Pos = (IndexStart + sLogDCU.CountRead)% (MAX_INDEX_LOG + 1);
    
    if(Pos == 0)
        Pos = 1;
    
    if(Manage_Flash.Error_ui8 == 0)
    {
        switch (Manage_Flash_log.Step_ui8)
        {
            case 0:
                if((sSim900_status.Step_Control_u8 == GSM_ATCOMMAND) && (sSim900_status.CheckConnectATCommand == AT_CMD_WAIT))
                {
                    if(Pos == IndexStop)   //Vi tri cuoi cung thi dung lai
                        return 0;
                    if(_fReadLogDCU(Pos, 3) == 1)       //Doc log tai vi tri Pos  
                    {
                        Manage_Flash_log.Step_ui8 = 1;     //Doc thanh con gui qua queue, se sang step 1 de cho gui thanh cong
                        Manage_Flash_log.Time_outFB_MQTT = RT_Count_Systick_u32;
                    }
                    sLogDCU.CountRead++;  //tang vi tri len 1
                }
                    
                break;
            case 1:
                if (Queue_Flash_ReadLog.Mess_Status_ui8 == 1)   
                {
                    Manage_Flash_log.Step_ui8 = 0;    //Neu gui thanh cong thi se tro len tren de doc vi tri tiep theo
                }
                //10s mà ban tin van chua dc gui di thi se dung lai
                if (Check_Time_Out(Manage_Flash_log.Time_outFB_MQTT,20000) == TRUE )
                {
                    Manage_Flash_log.Step_ui8 = 0; 
                    Result = 0;         //bao loi va dung lai
                }
                break;
            default:
                Manage_Flash_log.Step_ui8 = 0; 
                break;
        }
    }else 
        Result = 0;
    
    return Result;
}


void Log_Last_100Byte_UartSim (truct_String strUart)
{
    uint8_t     Temp = 0;
    uint16_t    PosLogUartSim = 0;
    uint16_t    LengthCopy = 0;
    //Copy buff uart nhan duoc sang buff temp. ghi lai 100 byte cuoi cùng
    
    if (strUart.Length_u16 >= 100)
    {
        PosLogUartSim = strUart.Length_u16 - 100;
        LengthCopy    = 100;
    } else
    {
        PosLogUartSim = 0;
        LengthCopy    = strUart.Length_u16;
    }
    
    if (sLogDCU.LogUart.Length_u16 >= 100)
        Reset_Buff(&sLogDCU.LogUart);
    
    if ((sLogDCU.LogUart.Length_u16 + LengthCopy) >= 100)
    {
        Temp = 100 - LengthCopy;                //lây môt so byte sau cung truoc
        for (uint16_t i = 0; i < Temp; i++)
            *(sLogDCU.LogUart.Data_a8 + i) = *(sLogDCU.LogUart.Data_a8 + sLogDCU.LogUart.Length_u16 - Temp + i);
        
        sLogDCU.LogUart.Length_u16 = Temp;
    } 

    Copy_String_section(&sLogDCU.LogUart, &strUart, sLogDCU.LogUart.Length_u16, PosLogUartSim, LengthCopy, 255);
}





