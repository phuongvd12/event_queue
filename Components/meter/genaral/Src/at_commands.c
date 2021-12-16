#include "at_commands.h"
#include "onchipflash.h"
#include "rtc.h"
#include "t_mqtt.h"
#include "variable.h"
#include "myuart.h"
#include "usart.h"
#include "sim900.h"
#include "pushdata.h"
#include "S25FL216K.h"



extern uint8_t              aServer_MQTT[DCU_ID_LENGTH];
uint8_t                     IEC620_56_Handshake[5]={0x2F,0x3F,0x21,0x0D,0x0A};
uint8_t                     DLMS_Handshake[2]={0x7E,0xA0};
uint8_t 		            PortConfig = 0;
extern xQueueHandle         qSIM_MeterHandle;	

uint8_t                     Buff_CmdAT[200];
truct_String                Str_Cmd_AT = {&Buff_CmdAT[0], 0};
uint8_t	                    tempBuff[128];

const struct_CheckList_GSM CheckList_AT[] = 
{		
//			id ENUM				kind process			str send		str Received
		{_ACCOUNT,		 		_fACCOUNT,				{NULL},			{(uint8_t*)"at+pppaccount=",14}}, // cac lenh check data va cau hinh  AT+PPPACCOUNT?
        {_QUERY_ACCOUNT,		_fQUERY_ACCOUNT,		{NULL},			{(uint8_t*)"at+pppaccount?",14}}, 
		{_PASSWORD, 			_fPASSWORD,				{NULL}, 		{(uint8_t*)"at+password=",12}},
        {_RESET_PASSWORD, 		_fRESET_PASSWORD,		{NULL}, 		{(uint8_t*)"at+resetpass",12}},
        {_QUERY_PASSWORD, 		_fQUERY_PASSWORD,		{NULL}, 		{(uint8_t*)"at+password?",12}},
		{_SET_DEV_SERIAL, 		_fSET_DEV_SERIAL,		{NULL},			{(uint8_t*)"at+seri=",8}},
		{_QUERY_DEV_SERIAL,		_fQUERY_DEV_SERIAL,		{NULL}, 		{(uint8_t*)"at+seri?",8}},
		{_SET_SERVER_INFO,	 	_fSET_SERVER_INFO,		{NULL}, 		{(uint8_t*)"at+tcp=",7}},
		{_QUERY_SERVER_INFO, 	_fQUERY_SERVER_INFO,	{NULL}, 		{(uint8_t*)"at+tcp?",7}},
		{_SET_CONNECT_MODE, 	_fSET_CONNECT_MODE,		{NULL}, 		{(uint8_t*)"at+modecon=",11}},//
		{_QUERY_CONNECT_MODE, 	_fQUERY_CONNECT_MODE,	{NULL}, 		{(uint8_t*)"at+modecon?",11}},
		{_SET_RTC_TIME, 		_fSET_RTC_TIME,			{NULL}, 		{(uint8_t*)"at+rtc_time=",12}},
		{_SET_RTC_DATE, 		_fSET_RTC_DATE,			{NULL}, 		{(uint8_t*)"at+rtc_date=",12}},
		{_QUERY_RTC, 			_fQUERY_RTC,			{NULL}, 		{(uint8_t*)"at+rtc?",7}},
		{_SET_CONNECT_TIME, 	_fSET_CONNECT_TIME,		{NULL}, 		{(uint8_t*)"at+tlogin=",10}},
		{_QUERY_CONNECT_TIME, 	_fQUERY_CONNECT_TIME,	{NULL}, 		{(uint8_t*)"at+tlogin?",10}},
		{_SET_CONNECT_PERIOD, 	_fSET_CONNECT_PERIOD,	{NULL}, 		{(uint8_t*)"at+dlogin=",10}},
		{_QUERY_CONNECT_PERIOD, _fQUERY_CONNECT_PERIOD,	{NULL}, 		{(uint8_t*)"at+dlogin?",10}}, // ko co
		{_SET_DISCONNECT_TIME, 	_fSET_DISCONNECT_TIME,	{NULL}, 		{(uint8_t*)"at+tlogout=",11}},
		{_QUERY_DISCONNECT_TIME,_fQUERY_DISCONNECT_TIME,{NULL}, 		{(uint8_t*)"at+tlogout?",11}},
		{_SET_DEV_IP, 			_fSET_DEV_IP,			{NULL},			{(uint8_t*)"at+ip=",6}},
		{_QUERY_DEV_IP,			_fQUERY_DEV_IP,			{NULL}, 		{(uint8_t*)"at+ip?",6}},
		{_SET_UART_PAR, 		_fSET_UART_PAR,			{NULL},			{(uint8_t*)"at+usmd=",8}},
		{_QUERY_UART_PAR,		_fQUERY_UART_PAR,		{NULL}, 		{(uint8_t*)"at+usmd?",8}},
		{_SEND_DATA_SERIAL,		_fSEND_DATA_SERIAL,		{NULL}, 		{(uint8_t*)"at+txmd",7}},  // cac lenh can thuc hien
		{_CONNECT_TCPIP,		_fCONNECT_TCPIP,		{NULL}, 		{(uint8_t*)"at+pppconn",10}},
		{_DISCONNECT_TCPIP,		_fDISCONNECT_TCPIP,		{NULL}, 		{(uint8_t*)"at+pppdisc",10}},
		{_PING_TCPIP,			_fPING_TCPIP,			{NULL}, 		{(uint8_t*)"at+ping",7}},
		{_CONNECT_SERVER,		_fCONNECT_SERVER,		{NULL}, 		{(uint8_t*)"at+tcpconn=",11}},
		{_SEND_DATA_SERVER,		_fSEND_DATA_SERVER,		{NULL}, 		{(uint8_t*)"at+tcpsend=",11}},
        {_CONFIG_RS485,	        _fCONFIG_RS485,	        {NULL}, 		{(uint8_t*)"at+rs485=",9}},
		{_DISCONNECT_SERVER,	_fDISCONNECT_SERVER,	{NULL}, 		{(uint8_t*)"at+tcpdisc=",11}},
        {_IEC620_56_METER,	    _fREQUEST_IEC62056,	    {NULL}, 		{&IEC620_56_Handshake[0],5}},
        {_DLMS_METER,	        _fREQUEST_DLMS,	        {NULL}, 		{&DLMS_Handshake[0],2}}, 
        //Sv
        {_ERASE_FLASH,          _fERASE_FLASH,	        {NULL}, 		{(uint8_t*)"at+eraseflash",13}}, 
        {_SET_FREQ_HEARTB,      _fSET_DUTY,	            {NULL}, 		{(uint8_t*)"at+duty=",8}},
        {_ON_DEBUG,             _fON_DEBUG,	            {NULL}, 		{(uint8_t*)"at+ondebug=",11}},
        {_SET_RADIO,            _fSET_RADIO,	        {NULL},         {(uint8_t*)"at+setradio=",12}},
        {_QUERY_RADIO,          _fQUERY_RADIO,	        {NULL},         {(uint8_t*)"at+radio?",9}},
        {_SET_DEV_ID, 		    _fSET_DEV_ID,		    {NULL},			{(uint8_t*)"at+iddev=",9}},
        {_QUERY_DEV_ID, 	    _fQUERY_DEV_ID,		    {NULL},			{(uint8_t*)"at+iddev?",9}},
        
        {_QUERY_BAND,	        _fQUERY_BAND,	        {NULL}, 		{(uint8_t*)"at+reband?",10}},  
        {_QUERY_CPSI,	        _fQUERY_CPSI,	        {NULL}, 		{(uint8_t*)"at+recpsi?",10}},  
        {_QUERY_CNMP,	        _fQUERY_CNMP,	        {NULL}, 		{(uint8_t*)"at+recnmp?",10}},
        //bo sung   
        {_SET_SEND_LPF_TIME, 	_fSET_SEND_LPF_TIME,    {NULL}, 		{(uint8_t*)"at+profiletime=",15}},
        {_QUERY_SEND_LPF_TIME , _fQUERY_SEND_LPF_TIME,	{NULL}, 		{(uint8_t*)"at+profiletime?",15}},   
        
        {_READ_LPF_FORTIME,	    _fGET_FORTIME_READ,	    {NULL}, 		{(uint8_t*)"at+readprofile=",15}},
        {_READ_INTAN_REGIS,	    _fGET_INTAN_REGIS,	    {NULL}, 		{(uint8_t*)"at+register?",12}},   
        {_READ_INFOR_METER,	    _fQUERY_INFOR_METER,	{NULL}, 		{(uint8_t*)"at+infometer?",13}}, 
        {_READ_BILL_FORTIME,	_fGET_FORTIME_BILL,	    {NULL}, 		{(uint8_t*)"at+billmonth=",13}},  
        {_QUERY_EVENT_METER,	_fQUERY_EVENT_METER,	{NULL}, 		{(uint8_t*)"at+event?",9}},     
          
        {_QUERY_VOLUME_MEM,     _fQUERY_VOLUME_MEM,	    {NULL}, 		{(uint8_t*)"at+volume?",10}}, 
        {_QUERY_NETIP,          _fQUERY_SIM_IP,	        {NULL}, 		{(uint8_t*)"at+netip?",9}},    
        {_RESET_DEVICE,         _fRESET_DEVICE,	        {NULL}, 		{(uint8_t*)"at+mreset?",10}}, 
        {_QUERY_EMEI_SIM,       _fQUERY_EMEI_SIM,	    {NULL}, 		{(uint8_t*)"at+imei?",8}},
        {_QUERY_FIRM_VER,       _fQUERY_FIRM_VER,	    {NULL}, 		{(uint8_t*)"at+fwversion?",13}},    
        {_QUERY_UP_FIRM_HTTP,   _fGET_URLHTTP_FIRM,	    {NULL}, 		{(uint8_t*)"at+userurl=",11}},      
        {_QUERY_UP_FIRM_FTP,    _fGET_URLFTP_FIRM,	    {NULL}, 		{(uint8_t*)"at+userftp=",11}},
        
        {_QUERY_INDEX_LOG,      _fQUERY_INDEX_LOG,	    {NULL}, 		{(uint8_t*)"at+logindex?",12}},
        {_TEST_LOG,             _fTEST_LOG,	            {NULL}, 		{(uint8_t*)"at+testlog?",11}},
        {_QUERY_READ_LOG_FOR,   _fGET_FOR_INDEX_LOG,    {NULL}, 		{(uint8_t*)"at+rdlogfor=",12}},     // at+rdlogfor=()()
        {_QUERY_READ_ALL_LOG,   _fQUERY_ALL_LOG,	    {NULL}, 		{(uint8_t*)"at+readlog?",11}},    
        {_QUERY_READ_INDEX_LOG, _fGET_INDEX_LOG,	    {NULL}, 		{(uint8_t*)"at+rdlogindex=",14}},     
};



void _fACCOUNT (truct_String *str_Receiv)
{
	uint16_t pointer=0,i=0;
    uint8_t check_u8 = 1;
    
	sInformation.APN.Length_u16 = 0;
	sInformation.APN_Dial_String.Length_u16 = 0;
	sInformation.APN_Username.Length_u16 = 0;
	sInformation.APN_Password.Length_u16 = 0;
	
    *(str_Receiv->Data_a8+200)  = ','; // find finish
	while(*(str_Receiv->Data_a8+pointer) != ',')
	{
		*(sInformation.APN.Data_a8+sInformation.APN.Length_u16) = *(str_Receiv->Data_a8+pointer);
		sInformation.APN.Length_u16++;
		pointer++;
        if(sInformation.APN.Length_u16 >= BUFF_LENGTH_SETUP)
            break;
	}
	pointer++; 
    if(pointer<200)
    {
        while(*(str_Receiv->Data_a8+pointer) != ',')
        {
            *(sInformation.APN_Dial_String.Data_a8+sInformation.APN_Dial_String.Length_u16) = *(str_Receiv->Data_a8+pointer);
            sInformation.APN_Dial_String.Length_u16++;
            pointer++;
            if(sInformation.APN_Dial_String.Length_u16 >= BUFF_LENGTH_SETUP)
                break;
        }
        pointer++;
        if(pointer < 200)
        {
            while(*(str_Receiv->Data_a8+pointer) != ',')
            {
                *(sInformation.APN_Username.Data_a8+sInformation.APN_Username.Length_u16) = *(str_Receiv->Data_a8+pointer);
                sInformation.APN_Username.Length_u16++;
                pointer++;
                if(sInformation.APN_Username.Length_u16 >= BUFF_LENGTH_SETUP)
                    break;
            }
            pointer++;
            if((pointer<200)&&(check_u8 == 1))
            {
                while((pointer < str_Receiv->Length_u16)&&(sInformation.APN_Password.Length_u16 < BUFF_LENGTH_SETUP))
                {
                    *(sInformation.APN_Password.Data_a8+sInformation.APN_Password.Length_u16) = *(str_Receiv->Data_a8+pointer);
                    sInformation.APN_Password.Length_u16++;
                    pointer++;
                    if(sInformation.APN_Password.Length_u16 >= BUFF_LENGTH_SETUP)
                        break;
                }
                if(check_u8 == 1) //Try to connect network use new settings	//if connect ok -> save new settings into onchipflash
                {
                    tempBuff[0] = (uint8_t)(str_Receiv->Length_u16);
                    for(i=0;i<str_Receiv->Length_u16;i++)
                        tempBuff[i+1] = *(str_Receiv->Data_a8+i);
                    tempBuff[119] = ','; // danh dau het data
                    OnchipFlashPageErase(ADDR_APN_INFO);
                    OnchipFlashWriteData(ADDR_APN_INFO,tempBuff,120);
                    DCU_Respond(PortConfig,(uint8_t *)"OK",2);
                }
            }
        }
	}
    Init_APN_Info();   //dat day de neu nhu config sai thi init lai lay gia tri ban dau
    if(sSim900_status.Step_Control_u8 < GSM_ATCOMMAND) sSim900_status.FlagSet_NewAPN = 1;
}

void _fQUERY_ACCOUNT (truct_String *str_Receiv)
{
    uint16_t tempLength = 0, i = 0;
	
	for(i=0;i<sInformation.APN.Length_u16;i++)
		tempBuff[tempLength++] = *(sInformation.APN.Data_a8+i);
	tempBuff[tempLength++] = ',';
    
    for(i=0;i<sInformation.APN_Dial_String.Length_u16;i++)
		tempBuff[tempLength++] = *(sInformation.APN_Dial_String.Data_a8+i);
	tempBuff[tempLength++] = ',';
    
    for(i=0;i<sInformation.APN_Username.Length_u16;i++)
		tempBuff[tempLength++] = *(sInformation.APN_Username.Data_a8+i);
	tempBuff[tempLength++] = ',';
    
	for(i=0;i<sInformation.APN_Password.Length_u16;i++)
		tempBuff[tempLength++] = *(sInformation.APN_Password.Data_a8+i);
	
	DCU_Respond(PortConfig, tempBuff,tempLength);
}


void _fPASSWORD (truct_String *str_Receiv)
{
	uint8_t check_u8 = 1;
	uint16_t pointer=0;
    
    *(str_Receiv->Data_a8+63)  = ','; // find finish     
	while(*(str_Receiv->Data_a8+pointer) != ',')
	{
		if(*(sInformation.Password_Moderm.Data_a8+pointer) != *(str_Receiv->Data_a8+pointer))
        {
            check_u8 = 0;
            break;
        }
		pointer++;
	}
    if(check_u8 == 1)
    {
        pointer++;
        if(pointer < 63)
        {
            sInformation.Password_Moderm.Length_u16 = 0;
            while(pointer < str_Receiv->Length_u16)// lay mat khau moi
            {
                *(sInformation.Password_Moderm.Data_a8+sInformation.Password_Moderm.Length_u16) = *(str_Receiv->Data_a8+pointer);
                sInformation.Password_Moderm.Length_u16++;
                pointer++;
                if(sInformation.Password_Moderm.Length_u16 > BUFF_LENGTH_SETUP)
                {
                    sInformation.Password_Moderm.Length_u16 = *(__IO uint8_t*)(ADDR_PASS_MODERM+1);
                    OnchipFlashReadData(ADDR_PASS_MODERM+2,PasswordModerm,sInformation.Password_Moderm.Length_u16);
                    sInformation.Password_Moderm.Data_a8 = PasswordModerm;
                    DCU_Respond(PortConfig, (uint8_t *)"Incorrect password",18);   //bao loi
                    return;
                }
            }
            Save_PassModerm();
            DCU_Respond(PortConfig,(uint8_t *)"OK",2);
        }
    }
    else
        DCU_Respond(PortConfig, (uint8_t *)"Incorrect password",18);
}

void _fRESET_PASSWORD (truct_String *str_Receiv)
{
    sInformation.Password_Moderm.Length_u16 = 6;
    sInformation.Password_Moderm.Data_a8 = PasswordModerm;
    PasswordModerm[0] = '1'; PasswordModerm[1] = '2'; PasswordModerm[2] = '3';
    PasswordModerm[3] = '4'; PasswordModerm[4] = '5'; PasswordModerm[5] = '6';
    
    Save_PassModerm();
}


void _fQUERY_PASSWORD (truct_String *str_Receiv)
{
    DCU_Respond(PortConfig, sInformation.Password_Moderm.Data_a8,sInformation.Password_Moderm.Length_u16);
}


void _fSET_DEV_SERIAL (truct_String *str_Receiv)
{
	uint16_t i=0;
	uint16_t Pos = 0;
    
    for(Pos = 0; Pos < str_Receiv->Length_u16; Pos++)
      if(*(str_Receiv->Data_a8 + Pos) != ' ')   //khac khoang trang thi dung lai  
        break;

    if((Pos < str_Receiv->Length_u16) && ((str_Receiv->Length_u16 - Pos) < DCU_ID_LENGTH))   //neu qua length thi bao ERROR
    {
        sDCU.sDCU_Seri.Length_u16 = 0;
        for (i=0;i<(str_Receiv->Length_u16 - Pos);i++)
        {
            sDCU.sDCU_Seri.Data_a8[sDCU.sDCU_Seri.Length_u16++] = *(str_Receiv->Data_a8 + i + Pos);
            tempBuff[i+2] = sDCU.sDCU_Seri.Data_a8[i];
        }
        tempBuff[0] = 1;
        tempBuff[1] = sDCU.sDCU_Seri.Length_u16;
        
        OnchipFlashPageErase(ADDR_DEV_SERIAL);
        OnchipFlashWriteData(ADDR_DEV_SERIAL,tempBuff,40);
        DCU_Respond(PortConfig, (uint8_t *)"OK",2);
    }else DCU_Respond(PortConfig,(uint8_t *)"ERROR",5);
}


void _fSET_DEV_ID (truct_String *str_Receiv)
{
	uint16_t i=0;
	uint16_t Pos = 0;
    
    for(Pos = 0; Pos < str_Receiv->Length_u16; Pos++)
      if(*(str_Receiv->Data_a8 + Pos) != ' ')   //khac khoang trang thi dung lai
        break;
    
    if((Pos < str_Receiv->Length_u16) && ((str_Receiv->Length_u16 - Pos) < DCU_ID_LENGTH))
    {
        sDCU.sDCU_id.Length_u16 = 0;
        for (i=0;i<(str_Receiv->Length_u16 - Pos);i++)
        {
            sDCU.sDCU_id.Data_a8[sDCU.sDCU_id.Length_u16++] = *(str_Receiv->Data_a8 + i + Pos);
            tempBuff[i+2] = sDCU.sDCU_id.Data_a8[i];
        }
        tempBuff[0] = 1;
        tempBuff[1] = sDCU.sDCU_id.Length_u16;
        
        OnchipFlashPageErase(ADDR_DCUID);
        OnchipFlashWriteData(ADDR_DCUID,tempBuff,40);
        DCU_Respond(PortConfig, (uint8_t *)"OK",2);
    }else DCU_Respond(PortConfig,(uint8_t *)"ERROR",5);
}



void _fQUERY_DEV_SERIAL (truct_String *str_Receiv)
{
	DCU_Respond(PortConfig, sDCU.sDCU_Seri.Data_a8,sDCU.sDCU_Seri.Length_u16);
}

void _fQUERY_DEV_ID (truct_String *str_Receiv)
{
	DCU_Respond(PortConfig, sDCU.sDCU_id.Data_a8,sDCU.sDCU_id.Length_u16);
}

void _fQUERY_BAND (truct_String *str_Receiv)
{
    sSim900_status.fRequest_AT_cmd = 1;
    truct_String    StrSend  = {(uint8_t*) "AT+CNBP?\r", 9};
    truct_String    StrCheck = {(uint8_t*) "+CNBP:", 6};
    
    sSim900_status.StrCheck_AT_Req.Data_a8 = &BuffRecei[0];
    sSim900_status.StrCheck_AT_Req.Length_u16 = 0;
    Copy_String_2(&sSim900_status.StrCheck_AT_Req, &StrCheck);
    
    sSim900_status.sAT_CMD_Request.Data_a8 = &Buff_Cmd[0];
    sSim900_status.sAT_CMD_Request.Length_u16 = 0;
    
    Copy_String_2(&sSim900_status.sAT_CMD_Request, &StrSend);
}

void _fQUERY_CPSI (truct_String *str_Receiv)
{
    sSim900_status.fRequest_AT_cmd = 1;
    truct_String    StrSend  = {(uint8_t*) "AT+CPSI?\r", 9};
    truct_String    StrCheck_2 = {(uint8_t*) "+CPSI:", 6};  //+CPSI: 
    
    sSim900_status.StrCheck_AT_Req.Data_a8 = &BuffRecei[0];
    sSim900_status.StrCheck_AT_Req.Length_u16 = 0;
    Copy_String_2(&sSim900_status.StrCheck_AT_Req, &StrCheck_2);
    
    sSim900_status.sAT_CMD_Request.Data_a8 = &Buff_Cmd[0];
    sSim900_status.sAT_CMD_Request.Length_u16 = 0;
    
    Copy_String_2(&sSim900_status.sAT_CMD_Request, &StrSend);
}

void _fQUERY_CNMP (truct_String *str_Receiv)
{
    sSim900_status.fRequest_AT_cmd = 1;
    truct_String    StrSend  = {(uint8_t*) "AT+CNMP?\r", 9};
    truct_String    StrCheck_2 = {(uint8_t*) "+CNMP:", 6};
    
    sSim900_status.StrCheck_AT_Req.Data_a8 = &BuffRecei[0];
    sSim900_status.StrCheck_AT_Req.Length_u16 = 0;
    Copy_String_2(&sSim900_status.StrCheck_AT_Req, &StrCheck_2);
    
    sSim900_status.sAT_CMD_Request.Data_a8 = &Buff_Cmd[0];
    sSim900_status.sAT_CMD_Request.Length_u16 = 0;
    
    Copy_String_2(&sSim900_status.sAT_CMD_Request, &StrSend);
}

void _fSET_SERVER_INFO (truct_String *str_Receiv)
{
	uint16_t i=0,pointer=0;
	
    if(str_Receiv->Length_u16 > 27)     // 172.16.11.156:2000   bình thuong ca dau cach la 19 byte.
    {
        DCU_Respond(PortConfig,(uint8_t *)"ERROR",5);
        return;
    }
      
	sInformation.sServer_MQTT.Length_u16 = 0;
	sInformation.sPort_MQTT.Length_u16 = 0;
	
    for(pointer = 0; pointer < str_Receiv->Length_u16; pointer++)
      if(*(str_Receiv->Data_a8 + pointer) != ' ')   //khac khoang trang thi dung lai  
        break;
    
    *(str_Receiv->Data_a8+39)  = ':'; // find finish
	while(*(str_Receiv->Data_a8+pointer) != ':')
	{
		*(sInformation.sServer_MQTT.Data_a8+sInformation.sServer_MQTT.Length_u16) = *(str_Receiv->Data_a8+pointer);
		sInformation.sServer_MQTT.Length_u16++;
		pointer++;
        if(sInformation.sServer_MQTT.Length_u16 >= DCU_ID_LENGTH)
            break;
	}
	pointer++;
    if(pointer<39)
    {
        while((pointer < str_Receiv->Length_u16) && (sInformation.sPort_MQTT.Length_u16 < PASSWORD_LENGTH))    //Nho hon Max length Buff Port
        {
            *(sInformation.sPort_MQTT.Data_a8+sInformation.sPort_MQTT.Length_u16) = *(str_Receiv->Data_a8+pointer);
            sInformation.sPort_MQTT.Length_u16++;
            pointer++;
            if(sInformation.sPort_MQTT.Length_u16 >= PASSWORD_LENGTH)
                break;
        }
    }
	tempBuff[0] = 2;
    tempBuff[1] = sInformation.sServer_MQTT.Length_u16;
	for (i=0;i<sInformation.sServer_MQTT.Length_u16;i++)
		tempBuff[i+2] = *(sInformation.sServer_MQTT.Data_a8+i);
	tempBuff[sInformation.sServer_MQTT.Length_u16+2] = sInformation.sPort_MQTT.Length_u16;
	for (i=0;i<sInformation.sPort_MQTT.Length_u16;i++)
		tempBuff[sInformation.sServer_MQTT.Length_u16+i+3] = *(sInformation.sPort_MQTT.Data_a8+i);
    
    OnchipFlashPageErase(ADDR_MQTTSERVER);
	OnchipFlashWriteData(ADDR_MQTTSERVER,tempBuff,40);
	DCU_Respond(PortConfig,(uint8_t *)"OK",2);
}

void _fQUERY_SERVER_INFO (truct_String *str_Receiv)
{
	uint16_t tempLength = 0, i = 0;
	
	for(i=0;i<sInformation.sServer_MQTT.Length_u16;i++)
		tempBuff[tempLength++] = *(sInformation.sServer_MQTT.Data_a8+i);
	tempBuff[tempLength++] = ':';
	for(i=0;i<sInformation.sPort_MQTT.Length_u16;i++)
		tempBuff[tempLength++] = *(sInformation.sPort_MQTT.Data_a8+i);
	
	DCU_Respond(PortConfig,tempBuff,tempLength);
}

void _fSET_CONNECT_MODE (truct_String *str_Receiv)
{
	uint8_t	tempBuff[8];
	
	tempBuff[0] = *(str_Receiv->Data_a8);
	sDCU.Connect_Mode = tempBuff[0];
    OnchipFlashPageErase(ADDR_CONNECT_MODE);
	OnchipFlashWriteData(ADDR_CONNECT_MODE,tempBuff,8);
	DCU_Respond(PortConfig,(uint8_t *)"OK",2);
    if((tempBuff[0] == '1')&&(sSim900_status.Status_Connect_u8 == 0))
    {
        sSim900_status.CheckConnectATCommand = AT_CONNECT_SERVICE;
    }
}

void _fQUERY_CONNECT_MODE (truct_String *str_Receiv)
{
	DCU_Respond(PortConfig, &sDCU.Connect_Mode,1);
}


void _fSET_RTC_TIME (truct_String *str_Receiv)
{
    ST_TIME_FORMAT sRTC_temp = {0,0,0,0,0,0,0,0};
	
	if((*(str_Receiv->Data_a8+2)!= ':')||(*(str_Receiv->Data_a8+5)!= ':'))
	{
		//Send wrong format message
		DCU_Respond(PortConfig,(uint8_t *)"ERROR",5);
		return;
	}
	else
	{
		sRTC_temp.hour = ((*(str_Receiv->Data_a8))-0x30)*10 + (*(str_Receiv->Data_a8+1) - 0x30);
		sRTC_temp.min = ((*(str_Receiv->Data_a8+3))-0x30)*10 + (*(str_Receiv->Data_a8+4) - 0x30);
		sRTC_temp.sec = ((*(str_Receiv->Data_a8+6))-0x30)*10 + (*(str_Receiv->Data_a8+7) - 0x30);
		sRTC_temp.day = 0; sRTC_temp.date = 0; sRTC_temp.month = 0; sRTC_temp.year = 0;
		
		if(Check_update_Time(&sRTC_temp) != 1)
		{
			//Send wrong format message
			DCU_Respond(PortConfig,(uint8_t *)"ERROR",5);
			return;
		}
		else
		{
			sRTCTime.Seconds 	= sRTC_temp.sec;
			sRTCTime.Minutes 	= sRTC_temp.min;
			sRTCTime.Hours 		= sRTC_temp.hour;
			HAL_RTC_SetTime(&hrtc,&sRTCTime, RTC_FORMAT_BIN);
			HAL_RTC_SetDate(&hrtc,&sRTCDate, RTC_FORMAT_BIN);
			DCU_Respond(PortConfig,(uint8_t *)"OK",2);
		}
	}
}

void _fSET_RTC_DATE (truct_String *str_Receiv)
{
	ST_TIME_FORMAT sRTC_temp = {0,0,0,0,0,0,0,0};
	
	if((*(str_Receiv->Data_a8+1)!= '-')||(*(str_Receiv->Data_a8+4)!= '-')||(*(str_Receiv->Data_a8+7)!= '-'))
	{
		//Send wrong format message
		DCU_Respond(PortConfig,(uint8_t *)"ERROR",5);
		return;
	}
	else
	{
		sRTC_temp.day = (*(str_Receiv->Data_a8))-0x30 - 1;
		sRTC_temp.date = ((*(str_Receiv->Data_a8+2))-0x30)*10 + (*(str_Receiv->Data_a8+3) - 0x30);
		sRTC_temp.month = ((*(str_Receiv->Data_a8+5))-0x30)*10 + (*(str_Receiv->Data_a8+6) - 0x30);
		sRTC_temp.year = ((*(str_Receiv->Data_a8+8))-0x30)*10 + (*(str_Receiv->Data_a8+9) - 0x30);
		sRTC_temp.hour = 0; sRTC_temp.min = 0; sRTC_temp.sec = 0;
		
		if(Check_update_Time(&sRTC_temp) != 1)
		{
			//Send wrong format message
			DCU_Respond(PortConfig,(uint8_t *)"ERROR",5);
			return;
		}
		else
		{
			sRTCDate.WeekDay 	= sRTC_temp.day;
			sRTCDate.Date 		= sRTC_temp.date;
			sRTCDate.Month 		= sRTC_temp.month;
			sRTCDate.Year 		= sRTC_temp.year;
			HAL_RTC_SetTime(&hrtc,&sRTCTime,RTC_FORMAT_BIN);
			HAL_RTC_SetDate(&hrtc,&sRTCDate,RTC_FORMAT_BIN);
			DCU_Respond(PortConfig, (uint8_t *)"OK",2);
		}
	}
}

void _fQUERY_RTC (truct_String *str_Receiv)
{
	uint8_t length = 0;
	tempBuff[length++] = 't'; tempBuff[length++] = 'i'; tempBuff[length++] = 'm'; tempBuff[length++] = 'e'; 
//    tempBuff[length++] = ' '; 
    tempBuff[length++] = '='; 
//    tempBuff[length++] = ' ';
	tempBuff[length++] = sRTC.hour/10 + 0x30; tempBuff[length++] = sRTC.hour%10 + 0x30; tempBuff[length++] = ':';
	tempBuff[length++] = sRTC.min/10 + 0x30; tempBuff[length++] = sRTC.min%10 + 0x30; tempBuff[length++] = ':';
	tempBuff[length++] = sRTC.sec/10 + 0x30; tempBuff[length++] = sRTC.sec%10 + 0x30; tempBuff[length++] = ';';
    tempBuff[length++] = 0x0D; 
	tempBuff[length++] = 'd'; tempBuff[length++] = 'a'; tempBuff[length++] = 't'; tempBuff[length++] = 'e'; 
//    tempBuff[length++] = ' '; 
    tempBuff[length++] = '='; 
//    tempBuff[length++] = ' ';
	tempBuff[length++] = sRTC.day + 0x31; tempBuff[length++] = '-';
	tempBuff[length++] = sRTC.date/10 + 0x30; tempBuff[length++] = sRTC.date%10 + 0x30; tempBuff[length++] = '-';
	tempBuff[length++] = sRTC.month/10 + 0x30; tempBuff[length++] = sRTC.month%10 + 0x30; tempBuff[length++] = '-';
	tempBuff[length++] = sRTC.year/10 + 0x30; tempBuff[length++] = sRTC.year%10 + 0x30;
	
	DCU_Respond(PortConfig,tempBuff,length);
}

void _fSET_CONNECT_TIME (truct_String *str_Receiv)
{
	ST_TIME_FORMAT  sRTC_temp = {0,0,0,0,0,0,0,0};
	uint8_t	    tempBuff[16];
    uint16_t    i = 0;
	
	if((*(str_Receiv->Data_a8+2)!= ':')||(*(str_Receiv->Data_a8+5)!= ':'))
	{
		DCU_Respond(PortConfig,(uint8_t *)"ERROR",5);//Send wrong format message
	}
	else
	{
		sRTC_temp.hour = ((*(str_Receiv->Data_a8))-0x30)*10 + (*(str_Receiv->Data_a8+1) - 0x30);
		sRTC_temp.min = ((*(str_Receiv->Data_a8+3))-0x30)*10 + (*(str_Receiv->Data_a8+4) - 0x30);
		sRTC_temp.sec = ((*(str_Receiv->Data_a8+6))-0x30)*10 + (*(str_Receiv->Data_a8+7) - 0x30);
		
		if(Check_update_Time(&sRTC_temp) != 1)
		{
			DCU_Respond(PortConfig,(uint8_t *)"ERROR",5);//Send wrong format message 
		}
		else
		{
			sInformation.ConnectTime.Hours = sRTC_temp.hour;
			sInformation.ConnectTime.Minutes = sRTC_temp.min;
			sInformation.ConnectTime.Seconds = sRTC_temp.sec;
			tempBuff[1] = (uint8_t)(str_Receiv->Length_u16 >> 8);
			tempBuff[0] = (uint8_t)(str_Receiv->Length_u16);
			for(i=0;i<str_Receiv->Length_u16;i++)
				tempBuff[i+2] = *(str_Receiv->Data_a8+i);
            OnchipFlashPageErase(ADDR_CONNECT_TIME);
			OnchipFlashWriteData(ADDR_CONNECT_TIME,tempBuff,16);
			DCU_Respond(PortConfig, (uint8_t *)"OK",2);
		}
	}
}

void _fQUERY_CONNECT_TIME (truct_String *str_Receiv)
{
	uint8_t tempData[16];
	
	//tempData[0] = 't'; tempData[1] = 'i'; tempData[2] = 'm'; tempData[3] = 'e'; tempData[4] = '=';
	tempData[0] = sInformation.ConnectTime.Hours/10 + 0x30; tempData[1] = sInformation.ConnectTime.Hours%10 + 0x30; tempData[2] = ':';
	tempData[3] = sInformation.ConnectTime.Minutes/10 + 0x30; tempData[4] = sInformation.ConnectTime.Minutes%10 + 0x30; tempData[5] = ':';
	tempData[6] = sInformation.ConnectTime.Seconds/10 + 0x30; tempData[7] = sInformation.ConnectTime.Seconds%10 + 0x30;
	
	DCU_Respond(PortConfig,tempData,8);
}

void _fSET_CONNECT_PERIOD (truct_String *str_Receiv)
{
	ST_TIME_FORMAT sRTC_temp = {0,0,0,0,0,0,0,0};
	uint8_t	tempBuff[16], i = 0;
	
	if((*(str_Receiv->Data_a8+2)!= ':')||(*(str_Receiv->Data_a8+5)!= ':'))
	{
		//Send wrong format message
		DCU_Respond(PortConfig, (uint8_t *)"ERROR",5);
		return;
	}
	else
	{
		sRTC_temp.hour = ((*(str_Receiv->Data_a8))-0x30)*10 + (*(str_Receiv->Data_a8+1) - 0x30);
		sRTC_temp.min = ((*(str_Receiv->Data_a8+3))-0x30)*10 + (*(str_Receiv->Data_a8+4) - 0x30);
		sRTC_temp.sec = ((*(str_Receiv->Data_a8+6))-0x30)*10 + (*(str_Receiv->Data_a8+7) - 0x30);
		
		if(Check_update_Time(&sRTC_temp) != 1)
		{
			//Send wrong format message
			DCU_Respond(PortConfig,(uint8_t *)"ERROR",5);
			return;
		}
		else
		{
			sInformation.ConnectPeriod.Hours = sRTC_temp.hour;
			sInformation.ConnectPeriod.Minutes = sRTC_temp.min;
			sInformation.ConnectPeriod.Seconds = sRTC_temp.sec;
			
			//if connect ok -> save new settings into onchipflash
			tempBuff[1] = (uint8_t)(str_Receiv->Length_u16 >> 8);
			tempBuff[0] = (uint8_t)(str_Receiv->Length_u16);
			for(i=0;i<str_Receiv->Length_u16;i++)
				tempBuff[i+2] = *(str_Receiv->Data_a8+i);
            OnchipFlashPageErase(ADDR_CONNECT_PERIOD);
			OnchipFlashWriteData(ADDR_CONNECT_PERIOD,tempBuff,16);    //Test
			DCU_Respond(PortConfig,(uint8_t *)"OK",2);
		}
	}
}

void _fQUERY_CONNECT_PERIOD (truct_String *str_Receiv)
{
	uint8_t tempData[16];
	
	//tempData[0] = 't'; tempData[1] = 'i'; tempData[2] = 'm'; tempData[3] = 'e'; tempData[4] = '=';
	tempData[0] = sInformation.ConnectPeriod.Hours/10 + 0x30; tempData[1] = sInformation.ConnectPeriod.Hours%10 + 0x30; tempData[2] = ':';
	tempData[3] = sInformation.ConnectPeriod.Minutes/10 + 0x30; tempData[4] = sInformation.ConnectPeriod.Minutes%10 + 0x30; tempData[5] = ':';
	tempData[6] = sInformation.ConnectPeriod.Seconds/10 + 0x30; tempData[7] = sInformation.ConnectPeriod.Seconds%10 + 0x30;
	
	DCU_Respond(PortConfig,tempData,8);
}

void _fSET_DISCONNECT_TIME (truct_String *str_Receiv)
{
	ST_TIME_FORMAT sRTC_temp = {0,0,0,0,0,0,0,0};
	uint8_t	tempBuff[16], i = 0;
	
	if((*(str_Receiv->Data_a8+2)!= ':')||(*(str_Receiv->Data_a8+5)!= ':'))
	{
		//Send wrong format message
		DCU_Respond(PortConfig, (uint8_t *)"ERROR",5);
		return;
	}
	else
	{
		sRTC_temp.hour = ((*(str_Receiv->Data_a8))-0x30)*10 + (*(str_Receiv->Data_a8+1) - 0x30);
		sRTC_temp.min = ((*(str_Receiv->Data_a8+3))-0x30)*10 + (*(str_Receiv->Data_a8+4) - 0x30);
		sRTC_temp.sec = ((*(str_Receiv->Data_a8+6))-0x30)*10 + (*(str_Receiv->Data_a8+7) - 0x30);
		
		if(Check_update_Time(&sRTC_temp) != 1)
		{
			//Send wrong format message
			DCU_Respond(PortConfig, (uint8_t *)"ERROR",5);
			return;
		}
		else
		{
			sInformation.DisconnectTime.Hours = sRTC_temp.hour;
			sInformation.DisconnectTime.Minutes = sRTC_temp.min;
			sInformation.DisconnectTime.Seconds = sRTC_temp.sec;
			
			//if connect ok -> save new settings into onchipflash
			OnchipFlashPageErase(ADDR_DISCONNECT_TIME);
			tempBuff[1] = (uint8_t)(str_Receiv->Length_u16 >> 8);
			tempBuff[0] = (uint8_t)(str_Receiv->Length_u16);
			for(i=0;i<str_Receiv->Length_u16;i++)
				tempBuff[i+2] = *(str_Receiv->Data_a8+i);
			OnchipFlashWriteData(ADDR_DISCONNECT_TIME,tempBuff,16);
			DCU_Respond(PortConfig, (uint8_t *)"OK",2);
		}
	}
}

void _fQUERY_DISCONNECT_TIME (truct_String *str_Receiv)
{
	uint8_t tempData[16];
	
	//tempData[0] = 't'; tempData[1] = 'i'; tempData[2] = 'm'; tempData[3] = 'e'; tempData[4] = '=';
	tempData[0] = sInformation.DisconnectTime.Hours/10 + 0x30; tempData[1] = sInformation.DisconnectTime.Hours%10 + 0x30; tempData[2] = ':';
	tempData[3] = sInformation.DisconnectTime.Minutes/10 + 0x30; tempData[4] = sInformation.DisconnectTime.Minutes%10 + 0x30; tempData[5] = ':';
	tempData[6] = sInformation.DisconnectTime.Seconds/10 + 0x30; tempData[7] = sInformation.DisconnectTime.Seconds%10 + 0x30;
	
	DCU_Respond(PortConfig, tempData,8);
}

void _fSET_DEV_IP (truct_String *str_Receiv)   
{
	uint16_t i=0,pointer=0;
	
    if(str_Receiv->Length_u16 > (BUFF_LENGTH_SETUP - 5))
    {
        DCU_Respond(PortConfig, (uint8_t *)"ERROR",5);
		return;
    }
      
    *(str_Receiv->Data_a8+39) = ',';
	sInformation.Device_Name.Length_u16 = 0;
	sInformation.Device_IP.Length_u16 = 0;

	for(pointer = 0; pointer < str_Receiv->Length_u16; pointer++)
      if(*(str_Receiv->Data_a8 + pointer) != ' ')   //khac khoang trang thi dung lai
          break;
    
	while(*(str_Receiv->Data_a8+pointer) != ',')
	{
		*(sInformation.Device_Name.Data_a8+sInformation.Device_Name.Length_u16) = *(str_Receiv->Data_a8+pointer);
		sInformation.Device_Name.Length_u16++;
		pointer++;
	}
	pointer++; // ','
//    pointer++; // ' '
    if(pointer < 39)
    {
        while((pointer < str_Receiv->Length_u16) && (sInformation.Device_IP.Length_u16 < BUFF_LENGTH_SETUP))
        {
            *(sInformation.Device_IP.Data_a8+sInformation.Device_IP.Length_u16) = *(str_Receiv->Data_a8+pointer);
            sInformation.Device_IP.Length_u16++;
            pointer++;
        }
        // Chi luu device Name va ip
        tempBuff[0] = (uint8_t)str_Receiv->Length_u16;
        for(i=0;i<str_Receiv->Length_u16;i++)
            tempBuff[i+1] = *(str_Receiv->Data_a8+i);
        tempBuff[39] = ',';
        OnchipFlashPageErase(ADDR_DEVICE_IP);
        OnchipFlashWriteData(ADDR_DEVICE_IP,tempBuff,40);
        DCU_Respond(PortConfig,(uint8_t *)"OK",2);
    }
}

void _fQUERY_DEV_IP (truct_String *str_Receiv)
{
	uint16_t tempLength = 0, i = 0;
	
	for(i=0;i<sInformation.Device_Name.Length_u16;i++)
		tempBuff[tempLength++] = *(sInformation.Device_Name.Data_a8+i);
	tempBuff[tempLength++] = ',';
//    tempBuff[tempLength++] = ' ';
	for(i=0;i<sInformation.Device_IP.Length_u16;i++)
		tempBuff[tempLength++] = *(sInformation.Device_IP.Data_a8+i);
	
	DCU_Respond(PortConfig, tempBuff,tempLength);
}

void _fSET_UART_PAR (truct_String *str_Receiv)
{
	uint16_t tempData = 0, i = 0;
	
	if(str_Receiv->Length_u16 != 9)
	{
		//Send wrong format message
		DCU_Respond(PortConfig, (uint8_t *)"ERROR",5);
		return;
	}
	else
	{
		if((*(str_Receiv->Data_a8) != '(')||(*(str_Receiv->Data_a8+8) != ')')||(*(str_Receiv->Data_a8+2) != ',')||(*(str_Receiv->Data_a8+4) != ',')||(*(str_Receiv->Data_a8+6) != ','))
		{
			//Send wrong format message
			DCU_Respond(PortConfig, (uint8_t *)"ERROR",5);
			return;
		}
		else
		{
			tempData = *(str_Receiv->Data_a8+1);
			if(tempData>'6')
			{
				//Send wrong format message
				DCU_Respond(PortConfig, (uint8_t *)"ERROR",5);
				return;
			}
			tempData = *(str_Receiv->Data_a8+3);
			if((tempData!='N')&&(tempData!='E')&&(tempData!='O'))
			{
				//Send wrong format message
                DCU_Respond(PortConfig, (uint8_t *)"ERROR",5);
				return;
			}
			tempData = *(str_Receiv->Data_a8+5);
			if((tempData!='7')&&(tempData!='8'))
			{
				//Send wrong format message
				DCU_Respond(PortConfig, (uint8_t *)"ERROR",5);
				return;
			}
			tempData = *(str_Receiv->Data_a8+7);
			if((tempData!='1')&&(tempData!='2'))
			{
				//Send wrong format message
				DCU_Respond(PortConfig,(uint8_t *)"ERROR",5);
				return;
			}
			
			sInformation.UART_Config.Length_u16 = 9;
			for(i=0;i<str_Receiv->Length_u16;i++)
			{
				*(sInformation.UART_Config.Data_a8+i) = *(str_Receiv->Data_a8+i);
			}
            _fSave_Uart_Config();
			Init_UART_Config(); // Init uart
            DCU_Respond(PortConfig, (uint8_t *)"OK",2);
		}
	}
}

void _fQUERY_UART_PAR (truct_String *str_Receiv)
{
	//send sInformation.UART_Config.Data_a8 & Length_u16
    GET_UART_Meter_Config();
	DCU_Respond(PortConfig, sInformation.UART_Config.Data_a8,sInformation.UART_Config.Length_u16);
}



void _fSEND_DATA_SERIAL (truct_String *str_Receiv)
{
    // Cat chuoi data nhan dc, -> Send toi Uart cua cong to theo cau hinh da config
    // Send data qua ca Rs232 va Rs485
    ATReceDataLength_u8 = 0;
	while(*(str_Receiv->Data_a8+ATReceDataLength_u8+1) != ')')
	{
		aATBuffReceData[ATReceDataLength_u8] = *(str_Receiv->Data_a8+ATReceDataLength_u8+1);
        ATReceDataLength_u8++;
        if((ATReceDataLength_u8 >= str_Receiv->Length_u16) || (ATReceDataLength_u8 >= BUFF_LENGTH_AT_SEND)) 
            break;
	}
    if(PortConfig != 1)
        sSim900_status.CheckConnectATCommand = AT_SEND_DATA_UART;
    else DCU_Respond(PortConfig,(uint8_t*)"ERROR",5);                  //phan hoi truong hop dung uart
}

void _fCONNECT_TCPIP (truct_String *str_Receiv)
{
    if(sSim900_status.Status_Connect_u8 == 1)
    {
        sSim900_status.CheckConnectATCommand = AT_CONNECT_SERVICE; 
        DCU_Respond(PortConfig,(uint8_t*)"Connected",9);
    }else DCU_Respond(PortConfig,(uint8_t*)"Fail",4);
}

void _fDISCONNECT_TCPIP (truct_String *str_Receiv)
{
	// Set step TCP -> Close trong 2 p -> khoi dong lai sim
	sSim900_status.CheckConnectATCommand = AT_DISCONNECT_SERVICE;
    DCU_Respond(PortConfig, (uint8_t*)"Disconnected",12);
//    sSim900_status.Step_Control_u8 = GSM_ATCOMMAND;
	// Check step
}

void _fPING_TCPIP (truct_String *str_Receiv)  //AT+PING 172.16.13.13
{
	// Send AT+CIPPING
	uint8_t pointer=0;
    
    if(str_Receiv->Length_u16 > 25)
    {
        DCU_Respond(PortConfig, (uint8_t *)"ERROR",5);
		  return;
    }
	sInformation.Ping_IP.Length_u16 = 0;
	pointer++; //dau khoang trang
	while(pointer < str_Receiv->Length_u16)
	{
		*(sInformation.Ping_IP.Data_a8+sInformation.Ping_IP.Length_u16) = *(str_Receiv->Data_a8+pointer);
		sInformation.Ping_IP.Length_u16++;
		pointer++;
	}
    sSim900_status.CheckConnectATCommand = AT_PING_TCP;
}

void _fCONNECT_SERVER (truct_String *str_Receiv)  //172.16.11.130:1330
{
	// Ket noi den dia chi Server nhan dc
	uint8_t pointer=0;
    
    if(str_Receiv->Length_u16 > (DCU_ID_LENGTH - 5))    //binh thuong max 18 ki tu
    {
        DCU_Respond(PortConfig, (uint8_t *)"ERROR",5);
	    return;
    }
    
	sInformation.sServer_MQTT_Request.Length_u16 = 0;
	sInformation.sPort_MQTT_Request.Length_u16 = 0;
    
	while(*(str_Receiv->Data_a8+pointer) != ':')
	{
        if(*(sInformation.sServer_MQTT_Request.Data_a8+sInformation.sServer_MQTT_Request.Length_u16) != *(str_Receiv->Data_a8+pointer))
          sSim900_status.ServerChange_u8 = 1;
        
		*(sInformation.sServer_MQTT_Request.Data_a8+sInformation.sServer_MQTT_Request.Length_u16) = *(str_Receiv->Data_a8+pointer);   
		sInformation.sServer_MQTT_Request.Length_u16++;
		pointer++;
	}
	pointer++;
	while((pointer < str_Receiv->Length_u16) && (sInformation.sPort_MQTT_Request.Length_u16 < PASSWORD_LENGTH))
	{
        if(*(sInformation.sPort_MQTT_Request.Data_a8+sInformation.sPort_MQTT_Request.Length_u16) != *(str_Receiv->Data_a8+pointer))
          sSim900_status.ServerChange_u8 = 1;
		*(sInformation.sPort_MQTT_Request.Data_a8+sInformation.sPort_MQTT_Request.Length_u16) = *(str_Receiv->Data_a8+pointer);
		sInformation.sPort_MQTT_Request.Length_u16++;
		pointer++;
	}
    if((sInformation.sPort_MQTT_Request.Length_u16 != 0) && (sInformation.sServer_MQTT_Request.Length_u16!= 0))
    {
        ResetCountGPRS();
//        sSim900_status.CheckConnectATCommand = AT_CONNECT_TCP;  //kiem tra gia tri config dung thoi
        DCU_Respond(PortConfig, (uint8_t*)"Connected!",10);  
    }else DCU_Respond(PortConfig, (uint8_t *)"Fail",4);
}

void _fSEND_DATA_SERVER (truct_String *str_Receiv) // Kiem tra IP, PORT neu dung gui du lieu den Server, sai -> ket noi voi Server
{
	uint8_t pointer=0;
	
    sSim900_status.ServerChange_u8 = 0;
	ATSendDataLength_u8 = 0;
	sInformation.sServer_MQTT_Request.Length_u16 = 0;
	sInformation.sPort_MQTT_Request.Length_u16 = 0;

	while(*(str_Receiv->Data_a8+pointer) != ':')
	{
        if(*(sInformation.sServer_MQTT_Request.Data_a8+sInformation.sServer_MQTT_Request.Length_u16) != *(str_Receiv->Data_a8+pointer))
          sSim900_status.ServerChange_u8 = 1;
		*(sInformation.sServer_MQTT_Request.Data_a8+sInformation.sServer_MQTT_Request.Length_u16) = *(str_Receiv->Data_a8+pointer);
		sInformation.sServer_MQTT_Request.Length_u16++;
		pointer++;
        if(sInformation.sServer_MQTT_Request.Length_u16 >= DCU_ID_LENGTH)
            break;
	}
	pointer++;
	while(*(str_Receiv->Data_a8+pointer) != ',')
	{
        if(*(sInformation.sPort_MQTT_Request.Data_a8+sInformation.sPort_MQTT_Request.Length_u16) != *(str_Receiv->Data_a8+pointer))
          sSim900_status.ServerChange_u8 = 1;
		*(sInformation.sPort_MQTT_Request.Data_a8+sInformation.sPort_MQTT_Request.Length_u16) = *(str_Receiv->Data_a8+pointer);
		sInformation.sPort_MQTT_Request.Length_u16++;
		pointer++;
        if(sInformation.sPort_MQTT_Request.Length_u16 >= PASSWORD_LENGTH)
            break;
	}
	pointer++;
	while(pointer < str_Receiv->Length_u16)
	{
		aATBuffSendData[ATSendDataLength_u8] = *(str_Receiv->Data_a8+pointer);
		pointer++;
		ATSendDataLength_u8++;
        if(ATSendDataLength_u8 >= BUFF_LENGTH_AT_SEND)
            break;
	}
    if((sInformation.sPort_MQTT_Request.Length_u16 != 0) && (sInformation.sServer_MQTT_Request.Length_u16!= 0))
    {
//        sInformation.Device_IP.Length_u16 = sInformation.sPort_MQTT_Request.Length_u16;
//        sSim900_status.CheckConnectATCommand = AT_SEND_DATA_TCP; //kiem tra gia tri config dung thoi
        DCU_Respond(PortConfig, (uint8_t*)"OK",2);
    }else DCU_Respond(PortConfig, (uint8_t *)"Fail",4);
}
void _fCONFIG_RS485 (truct_String *str_Receiv)
{
    uint8_t	tempBuff[4];
	
	tempBuff[0] = *(str_Receiv->Data_a8);
    if(tempBuff[0] == '1')
    {
       sSim900_status.Type_485 = 1;
       RS485_RECIEVE;
    }
    else
    {
       sSim900_status.Type_485 = 0; 
       RS485_SEND;
    }
	DCU_Respond(PortConfig, (uint8_t *)"OK",2);
}
void _fDISCONNECT_SERVER (truct_String *str_Receiv)
{
	// Lay Ip, port
	// Kiem tra IP va Port hien tai co dung voi data nhan ko
	// Dung -> ngat, sai ko lam gi
	uint16_t i = 0, check = 1;
	uint8_t pointer=0;
	uint8_t aNewServer[16];
	uint8_t aNewPort[16];

	truct_String sNewServer_MQTT;
	truct_String sNewPort_MQTT;
	
	sNewServer_MQTT.Data_a8 = &aNewServer[0];
	sNewServer_MQTT.Length_u16 = 0;
	sNewPort_MQTT.Data_a8 = &aNewPort[0];
	sNewPort_MQTT.Length_u16 = 0;

	while(*(str_Receiv->Data_a8+pointer) != ':')
	{
		*(sNewServer_MQTT.Data_a8+sNewServer_MQTT.Length_u16) = *(str_Receiv->Data_a8+pointer);
		sNewServer_MQTT.Length_u16++;
		pointer++;
        if(sNewServer_MQTT.Length_u16 >= sizeof(aNewServer))
            break;
	}
	pointer++;
	while((pointer < str_Receiv->Length_u16)&& (*(str_Receiv->Data_a8+pointer) != ','))
	{
		*(sNewPort_MQTT.Data_a8+sNewPort_MQTT.Length_u16) = *(str_Receiv->Data_a8+pointer);
		sNewPort_MQTT.Length_u16++;
		pointer++;
        if(sNewPort_MQTT.Length_u16 >= sizeof(aNewPort))
            break;
	}
	if(sNewServer_MQTT.Length_u16 == sInformation.sServer_MQTT.Length_u16)
	{
		if(sNewPort_MQTT.Length_u16 == sInformation.sPort_MQTT.Length_u16)
		{
			for(i=0;i<sInformation.sServer_MQTT.Length_u16;i++)
			{
				if(*(sNewServer_MQTT.Data_a8+i) != *(sInformation.sServer_MQTT.Data_a8+i))
				{
					check = 0;
					break;
				}
			}
			if(check == 1)
			{
				for(i=0;i<sInformation.sPort_MQTT.Length_u16;i++)	
				{
					if(*(sNewPort_MQTT.Data_a8+i) != *(sInformation.sPort_MQTT.Data_a8+i))
					{
						check = 0;
						break;
					}
				}
			}
		}
		else
			check = 0;
	}
	else
		check = 0;	
	//	
	if(check == 1)	
	{
		sSim900_status.CheckConnectATCommand = AT_DISCONNECT_TCP;   //kiem tra gia tri config dung thoi
        DCU_Respond(PortConfig,(uint8_t*)"Disconnected!",13);
		ResetCountGPRS();
	}else DCU_Respond(PortConfig, (uint8_t *)"Fail",4);
}

void _fREQUEST_IEC62056 (truct_String *str_Receiv)
{
    switch(UART_METER.Init.BaudRate)
    {
        case 300: 
           // Vinasino, DS26D la 485
           /*
            sSim900_status.Type_485 = 1;
            RS485_RECIEVE;
           */
            // cong to CPC - EMIC la 232.
            sSim900_status.Type_485 = 0;
            RS485_SEND;
            break;
        case 9600: // Elster
            sSim900_status.Type_485 = 0;
            RS485_SEND;
            break;
        default:
          break;    
    }
}
void _fREQUEST_DLMS (truct_String *str_Receiv)
{
    sSim900_status.Type_485 = 1;  
    RS485_RECIEVE;
}
void _fERASE_FLASH (truct_String *str_Receiv)
{
    DCU_Respond(PortConfig,(uint8_t *)"OK",2);
    Erase_Flash = 1;
}

void _fSET_DUTY (truct_String *str_Receiv)
{
     uint16_t   i =0;
     uint32_t   Temp = 0;
       
     //cai chu ki Ping heartbeat
     for(i = 0; i < str_Receiv->Length_u16; i++)
     {
        if((*(str_Receiv->Data_a8 + i) < 0x30) || (*(str_Receiv->Data_a8 + i) > 0x39)) 
        {
            break;
        }
        Temp = Temp *10 + (*(str_Receiv->Data_a8 + i) - 0x30);
     }
     if(Temp > 0)
     {
         sDCU.Freq_Send_Heartb_u32 = Temp * 1000;
         tempBuff[0] = (Temp >>24) & 0xFF;
         tempBuff[1] = (Temp >>16) & 0xFF;
         tempBuff[2] = (Temp >>8) & 0xFF;
         tempBuff[3] = Temp & 0xFF;
        
         Save_Array(ADDR_DUTY_HEARTB, &tempBuff[0], 4);
         DCU_Respond(PortConfig,(uint8_t *)"OK",2);
     }else DCU_Respond(PortConfig,(uint8_t *)"ERROR",5);
}

void _fON_DEBUG (truct_String *str_Receiv)
{
    uint8_t temp = 0;
    uint8_t i = 0;
    
    if(str_Receiv->Length_u16 >=2)
    {
        sCheck_Factory.Check_Config = 0;
        for(i = 0; i < 2; i++)
        {
            temp = *(str_Receiv->Data_a8 + i);

            if((*(str_Receiv->Data_a8 + i) >= 0x30) && ((*(str_Receiv->Data_a8 + i) <= 0x39)))
              temp =  temp - 0x30;
            else if (*(str_Receiv->Data_a8 + i) > 0x39) temp = temp - 0x37;
            else return;
            
            if (temp <= 0x0F)
            sCheck_Factory.Check_Config = sCheck_Factory.Check_Config <<8 + temp;
            DCU_Respond(PortConfig,(uint8_t *)"OK",2);
        }
    }else DCU_Respond(PortConfig,(uint8_t *)"ERROR",5);
}



void _fSET_RADIO (truct_String *str_Receiv)
{
    uint8_t temp = 0;
    
    temp = *(str_Receiv->Data_a8) - 0x30;
    
    if(temp <5)
    {
        sSim900_status.Radio_ConfigAT = temp;  
        if(sSim900_status.Status_Connect_u8 == 1) 
        {
            sSim900_status.FRequestAT_ByServer = 1;
        }

        sSim900_status.Falg_SetNewRadio = 1;  //quay den step set radio thuc hien va hard reset lai
    }else
        DCU_Respond(PortConfig,(uint8_t *)"ERROR",5);
}

void _fQUERY_RADIO (truct_String *str_Receiv)
{
    uint8_t length = 0;
    
    tempBuff[length++] = sSim900_status.Radio_4G_GSM + 0x30;
    tempBuff[length++] = ':';
    switch(sSim900_status.Radio_4G_GSM)
    {
        case 0:
            tempBuff[length++] = '2';
            tempBuff[length++] = 'G';
            break;
        case 1:
            tempBuff[length++] = '3';
            tempBuff[length++] = 'G';
            break;
        case 2:
            tempBuff[length++] = '4';
            tempBuff[length++] = 'G';
            break;
        case 3:
            tempBuff[length++] = 'A';
            tempBuff[length++] = 'U';
            tempBuff[length++] = 'T';
            tempBuff[length++] = 'O';
            break;
        default:
            break;
    }
    
    DCU_Respond(PortConfig, &tempBuff[0],length);
}



void _fGET_FORTIME_READ (truct_String *str_Receiv)
{
      uint16_t Pointer = 0;
      Flag_Request_Queue_struct qFlag_Request;
      
      if(sDCU.Flag_Get_sTime_OK == 1)
      {
          if(str_Receiv->Length_u16 > 1)
          {
              if(str_Receiv->Length_u16 == 24)  //(yymmddhhmm)(yymmddhhmm)
              {
                  while (Pointer < 20)
                  {
                      //lay start time 
                      while(*(str_Receiv->Data_a8 + Pointer) == '(')
                      {
                            //nam
                           Pointer++;
                           sInformation.StartTime_GetLpf.year = (*(str_Receiv->Data_a8 + Pointer) - 0x30) *10 + *(str_Receiv->Data_a8 + Pointer + 1) - 0x30;
                           Pointer += 2;
                           //thang
                           sInformation.StartTime_GetLpf.month = (*(str_Receiv->Data_a8 + Pointer) - 0x30) *10 + *(str_Receiv->Data_a8 + Pointer + 1) - 0x30;
                           Pointer += 2;
                           //ngay
                           sInformation.StartTime_GetLpf.date = (*(str_Receiv->Data_a8 + Pointer) - 0x30) *10 + *(str_Receiv->Data_a8 + Pointer + 1) - 0x30;
                           Pointer += 2;
                           //gio
                           sInformation.StartTime_GetLpf.hour = (*(str_Receiv->Data_a8 + Pointer) - 0x30) *10 + *(str_Receiv->Data_a8 + Pointer + 1) - 0x30;
                           Pointer += 2;
                           //phut
                           sInformation.StartTime_GetLpf.min = (*(str_Receiv->Data_a8 + Pointer) - 0x30) *10 + *(str_Receiv->Data_a8 + Pointer + 1) - 0x30;
                           Pointer += 2;
                           while(Pointer < 20)
                           {
                                while(*(str_Receiv->Data_a8 + Pointer) == '(')
                                { 
                                        //nam
                                       Pointer++;
                                       sInformation.EndTime_GetLpf.year = (*(str_Receiv->Data_a8 + Pointer) - 0x30) *10 + *(str_Receiv->Data_a8 + Pointer + 1) - 0x30;
                                       Pointer += 2;
                                       //thang
                                       sInformation.EndTime_GetLpf.month = (*(str_Receiv->Data_a8 + Pointer) - 0x30) *10 + *(str_Receiv->Data_a8 + Pointer + 1) - 0x30;
                                       Pointer += 2;
                                       //ngay
                                       sInformation.EndTime_GetLpf.date = (*(str_Receiv->Data_a8 + Pointer) - 0x30) *10 + *(str_Receiv->Data_a8 + Pointer + 1) - 0x30;
                                       Pointer += 2;
                                       //gio
                                       sInformation.EndTime_GetLpf.hour = (*(str_Receiv->Data_a8 + Pointer) - 0x30) *10 + *(str_Receiv->Data_a8 + Pointer + 1) - 0x30;
                                       Pointer += 2;
                                       //phut
                                       sInformation.EndTime_GetLpf.min = (*(str_Receiv->Data_a8 + Pointer) - 0x30) *10 + *(str_Receiv->Data_a8 + Pointer + 1) - 0x30;
                                       Pointer += 2;
                                       
                                       //tinh toan xem Index start va end tuong ung voi thoi gian la bao nhieu
                                       
                                       sInformation.IndexStartLpf = Find_Idex_By_sTime(sInformation.StartTime_GetLpf, sRTC);
                                       sInformation.IndexEndLpf = Find_Idex_By_sTime(sInformation.EndTime_GetLpf, sRTC);
                                       
                                         
                                       if((sInformation.IndexStartLpf != 0) && (sInformation.IndexEndLpf != 0))
                                       {
                                            if(sDCU.MeterType == METER_TYPE_UNKNOWN)
                                            {
                                                DCU_Respond(PortConfig, (uint8_t *)"ERROR",5);
                                                return;
                                            }else
                                                DCU_Respond(PortConfig, (uint8_t *)"OK",2);
                                           //Gui sang queue cho phep doc lpf tu cong to
                                            sStatus_Meter.Flag[DATA_LOAD_PROFILE] = 3;
                                            sStatus_Meter.Landmark_Flag[DATA_LOAD_PROFILE] = RT_Count_Systick_u32;
                                            qFlag_Request.Mess_Status_ui8 	= 0;
                                            qFlag_Request.Mess_add_u32 		= 0;
                                            qFlag_Request.Mess_Type_u8 		= DATA_LOAD_PROFILE;
                                            xQueueSend(qSIM_MeterHandle,&qFlag_Request, 1000);	
                                
                                            sInformation.Flag_Request_lpf = 1;
                                            sInformation.Flag_Stop_ReadLpf = 0;
                                            
                                            sTCP.Landmark_Allow_Send_LPF_u32 -= sTCP.Compare_Allow_Send_LPF_u16; //tru di de gui di luon
//                                            sSim900_status.Flag_WaitReadMet = 1;
//                                            sSim900_status.LandMarkWait_ReadMet = RT_Count_Systick_u32;
//                                            sSim900_status.TypeWait = DATA_LOAD_PROFILE;
                                       }else DCU_Respond(PortConfig, (uint8_t *)"ERROR",5);
                                       return;
                                }
                                Pointer++;
                           }
                      }
                      Pointer++;
                  }
              }else
              {
                    DCU_Respond(PortConfig,(uint8_t *)"ERROR",5);
                    return;
              }
          }else
          {
              if(*(str_Receiv->Data_a8) == '0')
              {
                  sInformation.Flag_Stop_ReadLpf = 1;
                  //truong hop ngung doc Lpf
//                  DCU_Respond(PortConfig,(uint8_t *)"OK",2);
                  DCU_Respond(PortConfig, (uint8_t *)"ERROR",5);
              }else DCU_Respond(PortConfig,(uint8_t *)"ERROR",5);
              
              return;
          }
      }//Neu chua lay dc thoi gian thuc ra thi ban len bao Doi lay gio sTime
      
      DCU_Respond(PortConfig,(uint8_t *)"FAIL",4);
}


void _fSET_SEND_LPF_TIME (truct_String *str_Receiv)
{
    ST_TIME_FORMAT sTimeStart_Temp;
    ST_TIME_FORMAT sTimeStop_Temp;
    uint8_t length = 0;

    if(str_Receiv->Length_u16 == 9)
    {
        sTimeStart_Temp.hour = (*(str_Receiv->Data_a8 + 0) - 0x30) *10 + *(str_Receiv->Data_a8 + 1) - 0x30;
        sTimeStart_Temp.min  = (*(str_Receiv->Data_a8 + 2) - 0x30) *10 + *(str_Receiv->Data_a8 + 3) - 0x30;
        //o giua co dau ':'
        sTimeStop_Temp.hour  = (*(str_Receiv->Data_a8 + 5) - 0x30) *10 + *(str_Receiv->Data_a8 + 6) - 0x30;
        sTimeStop_Temp.min   = (*(str_Receiv->Data_a8 + 7) - 0x30) *10 + *(str_Receiv->Data_a8 + 8) - 0x30;
        
        if((sTimeStart_Temp.hour <= 23) && (sTimeStart_Temp.min <= 60) && (sTimeStop_Temp.hour <= 23) && (sTimeStop_Temp.min <= 60))
        {
            sInformation.sTimeStart_Send_Lpf.hour = sTimeStart_Temp.hour;
            sInformation.sTimeStart_Send_Lpf.min = sTimeStart_Temp.min;
            sInformation.sTimeStop_Send_Lpf.hour = sTimeStop_Temp.hour;
            sInformation.sTimeStop_Send_Lpf.min = sTimeStop_Temp.min;
              
           
            tempBuff[length++] = 4;
            tempBuff[length++] = sInformation.sTimeStart_Send_Lpf.hour;
            tempBuff[length++] = sInformation.sTimeStart_Send_Lpf.min;
            tempBuff[length++] = sInformation.sTimeStop_Send_Lpf.hour;
            tempBuff[length++] = sInformation.sTimeStop_Send_Lpf.min;
            
            OnchipFlashPageErase(ADDR_TIMESEND_LPF);
			OnchipFlashWriteData(ADDR_TIMESEND_LPF,tempBuff,8);
            
            DCU_Respond(PortConfig,(uint8_t *)"OK",2);
        }
        else
            DCU_Respond(PortConfig,(uint8_t *)"ERROR",5);
    }else
        DCU_Respond(PortConfig,(uint8_t *)"ERROR",5); 
}

void _fQUERY_SEND_LPF_TIME (truct_String *str_Receiv)
{
    uint8_t tempData[9];
    
    tempData[0] = (sInformation.sTimeStart_Send_Lpf.hour / 10) + 0x30;
    tempData[1] = (sInformation.sTimeStart_Send_Lpf.hour % 10) + 0x30;
    tempData[2] = (sInformation.sTimeStart_Send_Lpf.min / 10) + 0x30;
    tempData[3] = (sInformation.sTimeStart_Send_Lpf.min % 10) + 0x30;
    tempData[4] = ':';
    tempData[5] = (sInformation.sTimeStop_Send_Lpf.hour / 10) + 0x30;
    tempData[6] = (sInformation.sTimeStop_Send_Lpf.hour % 10) + 0x30;
    tempData[7] = (sInformation.sTimeStop_Send_Lpf.min / 10) + 0x30;
    tempData[8] = (sInformation.sTimeStop_Send_Lpf.min % 10) + 0x30;
      
    DCU_Respond(PortConfig,tempData,9);
}

void _fGET_INTAN_REGIS (truct_String *str_Receiv)
{
    Flag_Request_Queue_struct qFlag_Request;  
    
    if(sDCU.MeterType == METER_TYPE_UNKNOWN)
    {
        DCU_Respond(PortConfig, (uint8_t *)"ERROR",5);
        return;
    }else
        DCU_Respond(PortConfig, (uint8_t *)"OK",2);
    sTCP.Landmark_Allow_Send_DATA_u32 -= sTCP.Compare_Allow_Send_DATA_u16;
    
    sStatus_Meter.Flag[DATA_INTANTANIOUS] = 3;
    sStatus_Meter.Landmark_Flag[DATA_INTANTANIOUS] = RT_Count_Systick_u32;
    qFlag_Request.Mess_Status_ui8 	= 0;
    qFlag_Request.Mess_add_u32 		= 0;
    qFlag_Request.Mess_Type_u8 		= DATA_INTANTANIOUS;
    xQueueSend(qSIM_MeterHandle,&qFlag_Request, 1000);
    osDelay(10);
    
//    sSim900_status.Flag_WaitReadMet = 1;
//    sSim900_status.LandMarkWait_ReadMet = RT_Count_Systick_u32;
//    sSim900_status.TypeWait = DATA_INTANTANIOUS;
}

void _fQUERY_INFOR_METER (truct_String *str_Receiv)
{    
    Flag_Request_Queue_struct qFlag_Request;
    
    if(sDCU.MeterType == METER_TYPE_UNKNOWN)
    {
        DCU_Respond(PortConfig, (uint8_t *)"ERROR",5);
        return;
    }else
        DCU_Respond(PortConfig, (uint8_t *)"OK",2);
    
    sStatus_Meter.Flag[DATA_METER_INFOR] = 3;
    sStatus_Meter.Landmark_Flag[DATA_METER_INFOR] = RT_Count_Systick_u32;
    qFlag_Request.Mess_Status_ui8 	= 0;
    qFlag_Request.Mess_add_u32 		= 0;
    qFlag_Request.Mess_Type_u8 		= DATA_METER_INFOR;
    xQueueSend(qSIM_MeterHandle,&qFlag_Request, 1000);
    osDelay(10);
    
//    sSim900_status.Flag_WaitReadMet = 1;
//    sSim900_status.LandMarkWait_ReadMet = RT_Count_Systick_u32;
//    sSim900_status.TypeWait = DATA_METER_INFOR;
}

void _fGET_FORTIME_BILL (truct_String *str_Receiv)
{
    uint16_t Pointer = 0;
    Flag_Request_Queue_struct qFlag_Request;
    uint16_t Month_Real = 0;
    uint16_t Month_Find = 0;
    
    if(sDCU.Flag_Get_sTime_OK == 1)
    {
        if(str_Receiv->Length_u16 >= 1)
        {
            if(str_Receiv->Length_u16 == 24)  //(yymmddhhmm)(yymmddhhmm)
            {
                while (Pointer < 20)
                {
                  //lay start time 
                  while(*(str_Receiv->Data_a8 + Pointer) == '(')
                  {
                        //nam
                       Pointer++;
                       sInformation.StartTime_GetBill.year = (*(str_Receiv->Data_a8 + Pointer) - 0x30) *10 + *(str_Receiv->Data_a8 + Pointer + 1) - 0x30;
                       Pointer += 2;
                       //thang
                       sInformation.StartTime_GetBill.month = (*(str_Receiv->Data_a8 + Pointer) - 0x30) *10 + *(str_Receiv->Data_a8 + Pointer + 1) - 0x30;
                       Pointer += 2;
                       //ngay
                       sInformation.StartTime_GetBill.date = (*(str_Receiv->Data_a8 + Pointer) - 0x30) *10 + *(str_Receiv->Data_a8 + Pointer + 1) - 0x30;
                       Pointer += 2;
                       //gio
                       sInformation.StartTime_GetBill.hour = (*(str_Receiv->Data_a8 + Pointer) - 0x30) *10 + *(str_Receiv->Data_a8 + Pointer + 1) - 0x30;
                       Pointer += 2;
                       //phut
                       sInformation.StartTime_GetBill.min = (*(str_Receiv->Data_a8 + Pointer) - 0x30) *10 + *(str_Receiv->Data_a8 + Pointer + 1) - 0x30;
                       Pointer += 2;
                       while(Pointer < 20)
                       {
                            while(*(str_Receiv->Data_a8 + Pointer) == '(')
                            { 
                                    //nam
                                   Pointer++;
                                   sInformation.EndTime_GetBill.year = (*(str_Receiv->Data_a8 + Pointer) - 0x30) *10 + *(str_Receiv->Data_a8 + Pointer + 1) - 0x30;
                                   Pointer += 2;
                                   //thang
                                   sInformation.EndTime_GetBill.month = (*(str_Receiv->Data_a8 + Pointer) - 0x30) *10 + *(str_Receiv->Data_a8 + Pointer + 1) - 0x30;
                                   Pointer += 2;
                                   //ngay
                                   sInformation.EndTime_GetBill.date = (*(str_Receiv->Data_a8 + Pointer) - 0x30) *10 + *(str_Receiv->Data_a8 + Pointer + 1) - 0x30;
                                   Pointer += 2;
                                   //gio
                                   sInformation.EndTime_GetBill.hour = (*(str_Receiv->Data_a8 + Pointer) - 0x30) *10 + *(str_Receiv->Data_a8 + Pointer + 1) - 0x30;
                                   Pointer += 2;
                                   //phut
                                   sInformation.EndTime_GetBill.min = (*(str_Receiv->Data_a8 + Pointer) - 0x30) *10 + *(str_Receiv->Data_a8 + Pointer + 1) - 0x30;
                                   Pointer += 2;
                                   
                                   //tinh toan xem Index start va end tuong ung voi thoi gian la bao nhieu
                                   Month_Real = sRTC.year *12 + sRTC.month;
                                   Month_Find = sInformation.StartTime_GetBill.year * 12 + sInformation.StartTime_GetBill.month;
                                   
                                   if(Month_Find > Month_Real)
                                   {
                                        sInformation.IndexStartBill = 0;
                                   }else  sInformation.IndexStartBill = Month_Real - Month_Find + 1;
                                   
                                   Month_Find = sInformation.EndTime_GetBill.year * 12 + sInformation.EndTime_GetBill.month;
                                   
                                   if(Month_Find > Month_Real)
                                   {
                                        sInformation.IndexEndBill = 0;
                                   }else  sInformation.IndexEndBill = Month_Real - Month_Find + 1;   //bill hien ta la thi index = 1.
                                   
                                   
                                   if((sInformation.IndexStartBill != 0) && (sInformation.IndexEndBill != 0))
                                   {
                                        if(sDCU.MeterType == METER_TYPE_UNKNOWN)
                                        {
                                            DCU_Respond(PortConfig, (uint8_t *)"ERROR",5);
                                            return;
                                        }else
                                            DCU_Respond(PortConfig, (uint8_t *)"OK",2);
                                       //Gui sang queue cho phep doc lpf tu cong to
                                        sStatus_Meter.Flag[DATA_HISTORICAL] = 3;
                                        sStatus_Meter.Landmark_Flag[DATA_HISTORICAL] = RT_Count_Systick_u32;
                                        qFlag_Request.Mess_Status_ui8 	= 0;
                                        qFlag_Request.Mess_add_u32 		= 0;
                                        qFlag_Request.Mess_Type_u8 		= DATA_HISTORICAL;
                                        xQueueSend(qSIM_MeterHandle,&qFlag_Request, 1000);	
                            
                                        sInformation.Flag_Request_Bill = 1;
                                        sInformation.Flag_Stop_ReadBill = 0;
                                        sTCP.Landmark_Allow_Send_DATA_u32 -= sTCP.Compare_Allow_Send_DATA_u16;    //tru di de gui di luon k bi delay
//                                        sSim900_status.Flag_WaitReadMet = 1;
//                                        sSim900_status.LandMarkWait_ReadMet = RT_Count_Systick_u32;
//                                        sSim900_status.TypeWait = DATA_HISTORICAL;
                                   }else 
                                        DCU_Respond(PortConfig, (uint8_t *)"ERROR",5);
                                   return;
                            }
                            Pointer++;
                       }
                  }
                  Pointer++;
                }
            }else 
            {
                DCU_Respond(PortConfig, (uint8_t *)"ERROR",5);
                return;
            }
        }else
        {
            if(*(str_Receiv->Data_a8) == '0')
            {
                sInformation.Flag_Stop_ReadBill = 1;
                //truong hop ngung doc Lpf
//                DCU_Respond(PortConfig, (uint8_t *)"OK",2);
                DCU_Respond(PortConfig, (uint8_t *)"ERROR",5);
            }else DCU_Respond(PortConfig, (uint8_t *)"ERROR",5);
            return;
        }
    }
    
    DCU_Respond(PortConfig,(uint8_t *)"FAIL",4);
}

void _fQUERY_EVENT_METER (truct_String *str_Receiv)
{
    Flag_Request_Queue_struct qFlag_Request;
    
    if(sDCU.MeterType == METER_TYPE_UNKNOWN)
    {
        DCU_Respond(PortConfig, (uint8_t *)"ERROR",5);
        return;
    }
    else
        DCU_Respond(PortConfig, (uint8_t *)"OK",2);
    
    sTCP.Landmark_Allow_Send_DATA_u32 -= sTCP.Compare_Allow_Send_DATA_u16;
   //Gui sang queue cho phep doc lpf tu cong to
    sStatus_Meter.Flag[DATA_EVEN_METER] = 3;
    sStatus_Meter.Landmark_Flag[DATA_EVEN_METER] = RT_Count_Systick_u32;
    qFlag_Request.Mess_Status_ui8 	= 0;
    qFlag_Request.Mess_add_u32 		= 0;
    qFlag_Request.Mess_Type_u8 		= DATA_EVEN_METER;
    xQueueSend(qSIM_MeterHandle,&qFlag_Request, 1000);
    
    osDelay(10);
//    sSim900_status.Flag_WaitReadMet = 1;
//    sSim900_status.LandMarkWait_ReadMet = RT_Count_Systick_u32;
//    sSim900_status.TypeWait = DATA_EVEN_METER;   
}

void _fQUERY_VOLUME_MEM (truct_String *str_Receiv)
{
    uint8_t Num = 0;
    uint8_t Count = 0;
    
    if(pos_MeterInfor_To_Send_u32 >= pos_MeterInfor_Sent_u32) 
      Num = 240 - (pos_MeterInfor_To_Send_u32 - pos_MeterInfor_Sent_u32) - 1;
    else Num = pos_MeterInfor_Sent_u32 - pos_MeterInfor_To_Send_u32 - 1;
    
    if(Num > 99)
    {
        tempBuff[Count++] = Num/100 + 0x30;
        tempBuff[Count++] = ((Num/10)%10) + 0x30;
        tempBuff[Count++] = (Num%10) + 0x30;
    }else if(Num > 9) 
    {
        tempBuff[Count++] = (Num/10) + 0x30;
        tempBuff[Count++] = (Num%10) + 0x30;
    }else
    {
        tempBuff[Count++] = Num + 0x30;
    }
              
              
    tempBuff[Count++] = ' ';   //dau khoang trang
    tempBuff[Count++] = 'R';   //Records 
    tempBuff[Count++] = 'e'; 
    tempBuff[Count++] = 'c'; 
    tempBuff[Count++] = 'o'; 
    tempBuff[Count++] = 'r'; 
    tempBuff[Count++] = 'd'; 
    tempBuff[Count++] = 's'; 
    
    DCU_Respond(PortConfig,&tempBuff[0], Count);
}

void _fQUERY_SIM_IP (truct_String *str_Receiv)
{
    uint16_t i = 0;
    uint8_t Count = 0;
    
    tempBuff[Count++] = 'D';
    tempBuff[Count++] = 'T';
    tempBuff[Count++] = 'U';
    tempBuff[Count++] = ',';
    
    for( i = 0; i < sDCU.Str_IP_Module.Length_u16; i++)
      tempBuff[Count++] = *(sDCU.Str_IP_Module.Data_a8 + i);
    
    DCU_Respond(PortConfig,&tempBuff[0], Count);
}

void _fRESET_DEVICE (truct_String *str_Receiv)
{
    //dung thuc thi các task. Reset thiet bi
    DCU_Respond(PortConfig, (uint8_t *)"OK",2);
    _fPackStringToLog ((uint8_t*) "Reset Chip\r\n", 12);
    osDelay(3000); // cho ban data OK. Neu bat buoc cho ACK thi check flag
	__disable_irq();
	NVIC_SystemReset(); 
}

void _fQUERY_EMEI_SIM (truct_String *str_Receiv)
{
    uint16_t i = 0;
    
    for( i = 0; i < sDCU.sDCU_id.Length_u16; i++)
      tempBuff[i] = *(sDCU.sDCU_id.Data_a8 + i);
    
    DCU_Respond(PortConfig,&tempBuff[0], sDCU.sDCU_id.Length_u16);
}
void _fQUERY_FIRM_VER (truct_String *str_Receiv)
{
    uint16_t i = 0;
    
    for(i=0;i<sFirmware_Version.Length_u16;i++) 
    {
        tempBuff[i] = *(sFirmware_Version.Data_a8+i);
    }
    
    DCU_Respond(PortConfig,&tempBuff[0], sFirmware_Version.Length_u16);
}

void _fGET_URLHTTP_FIRM (truct_String *str_Receiv)
{
    int Pos;
    uint8_t Flag_Get_name = 0;
    
    if((pos_MeterInfor_To_Send_u32 == 0)&&(pos_MeterInfor_Sent_u32 == 0)&&(pos_MeterLPF_Sent_u32 == 0) && (pos_MeterLPF_To_Send_u32 == 0) && 
          (pos_MeterBill_To_Sent_u32 == 0) && (pos_MeterBill_Sent_u32 == 0)&& (Manage_Flash.Error_ui8 == 0)) // wait Init Flash
	{
        DCU_Respond(PortConfig,(uint8_t *)"ERROR",5);
		return;
	}
       
    //lay URL firm
    sInformation.Str_URL_Firm_Add.Data_a8 = &Buff_FTP_IP[0];
    sInformation.Str_URL_Firm_Port.Data_a8 = &Buff_FTP_Port[0];
    sInformation.Str_URL_Firm_Path.Data_a8 = &Buff_FTP_PATH[0];
    sInformation.Str_URL_Firm_Name.Data_a8 = &Buff_FTP_NAME[0];
    sInformation.Str_URL_HTTP.Data_a8 = &Buff_HTTP_URL[0];
      
    Reset_Buff(&sInformation.Str_URL_Firm_Add);
    Reset_Buff(&sInformation.Str_URL_Firm_Port);
    Reset_Buff(&sInformation.Str_URL_Firm_Path); 
    Reset_Buff(&sInformation.Str_URL_Firm_Name);
    Reset_Buff(&sInformation.Str_URL_HTTP);
   
    truct_String Str1 = {(uint8_t*) "//", 2};
    
    Pos = Find_String_V2(&Str1, str_Receiv);
    if(Pos >=0)
    {
        Pos += Str1.Length_u16;
        while(*(str_Receiv->Data_a8 + Pos) != ':')
        {
            *(sInformation.Str_URL_Firm_Add.Data_a8 + sInformation.Str_URL_Firm_Add.Length_u16++) = *(str_Receiv->Data_a8 + Pos++);
            if((Pos >= str_Receiv->Length_u16) || (sInformation.Str_URL_Firm_Add.Length_u16 >= LENGTH_FTP))
            {
                DCU_Respond(PortConfig,(uint8_t *)"ERROR",5);
                return;
            }
        }
        Pos++;
        while(*(str_Receiv->Data_a8 + Pos) != '/')
        {
            *(sInformation.Str_URL_Firm_Port.Data_a8 + sInformation.Str_URL_Firm_Port.Length_u16++) = *(str_Receiv->Data_a8 + Pos++);
            if((Pos >= str_Receiv->Length_u16)|| (sInformation.Str_URL_Firm_Port.Length_u16 >= LENGTH_FTP))
            {
                DCU_Respond(PortConfig,(uint8_t *)"ERROR",5);
                return;
            }
        }
        Pos++;

        while(*(str_Receiv->Data_a8 + Pos) != '/')
        {
            *(sInformation.Str_URL_Firm_Path.Data_a8 + sInformation.Str_URL_Firm_Path.Length_u16++) = *(str_Receiv->Data_a8 + Pos++);
            if((Pos >= str_Receiv->Length_u16) || (sInformation.Str_URL_Firm_Path.Length_u16 >= LENGTH_FTP))
            {
                //neu het thi coi nhu day la name
                Copy_String_2(&sInformation.Str_URL_Firm_Name , &sInformation.Str_URL_Firm_Path);
                Reset_Buff(&sInformation.Str_URL_Firm_Path);
                Flag_Get_name = 1;
                break;
            }
        }
        
        while(Flag_Get_name == 0)
        {
            while(*(str_Receiv->Data_a8 + Pos) != '/')
            {
                *(sInformation.Str_URL_Firm_Name.Data_a8 + sInformation.Str_URL_Firm_Name.Length_u16++) = *(str_Receiv->Data_a8 + Pos++);
                if((Pos >= str_Receiv->Length_u16) || (sInformation.Str_URL_Firm_Name.Length_u16 >= LENGTH_FTP))
                {
                    Flag_Get_name = 1;
                    //neu het thi coi nhu day la name
                    break;
                }
            }
            //neu con xuong day thi tiep tuc copy vao path va tiep tuc lau name
            if(Flag_Get_name == 0)
            {
                Copy_String_2(&sInformation.Str_URL_Firm_Path , &sInformation.Str_URL_Firm_Name);
                Reset_Buff(&sInformation.Str_URL_Firm_Name);
                if((Pos >= str_Receiv->Length_u16) || (sInformation.Str_URL_Firm_Path.Length_u16 >= LENGTH_FTP))
                {
                    DCU_Respond(PortConfig,(uint8_t *)"ERROR",5);
                    return;
                }
            }
            Pos++;  
        }
        //check length
        Copy_String_2(&sInformation.Str_URL_HTTP , str_Receiv);
        DCU_Respond(PortConfig, (uint8_t *)"OK",2);
        //chuyen sang update firm
        sDCU.Mode_Connect_Future = MODE_CONNECT_HTTP;
		osThreadSuspend(ManageFlashHandle);
		osThreadSuspend(Task_Read_MeterHandle);
        sInformation.fUpdateFirmware = 1;
    }
}


void _fGET_URLFTP_FIRM (truct_String *str_Receiv)
{
//    int Pos;
//    uint8_t Flag_Get_name = 0;
//    
//    if((pos_MeterInfor_To_Send_u32 == 0)&&(pos_MeterInfor_Sent_u32 == 0)&&(pos_MeterLPF_Sent_u32 == 0) && (pos_MeterLPF_To_Send_u32 == 0) && 
//          (pos_MeterBill_To_Sent_u32 == 0) && (pos_MeterBill_Sent_u32 == 0)&& (Manage_Flash.Error_ui8 == 0)) // wait Init Flash
//	{
//        DCU_Respond(PortConfig,(uint8_t *)"ERROR",5);
//		return;
//	}
//       
//    //lay URL firm
//    sInformation.Str_URL_Firm_Add.Data_a8 = &Buff_FTP_IP[0];
//    sInformation.Str_URL_Firm_Port.Data_a8 = &Buff_FTP_Port[0];
//    sInformation.Str_URL_Firm_Path.Data_a8 = &Buff_FTP_PATH[0];
//    sInformation.Str_URL_Firm_Name.Data_a8 = &Buff_FTP_NAME[0];
//    sInformation.Str_URL_HTTP.Data_a8 = &Buff_HTTP_URL[0];
//      
//    Reset_Buff(&sInformation.Str_URL_Firm_Add);
//    Reset_Buff(&sInformation.Str_URL_Firm_Port);
//    Reset_Buff(&sInformation.Str_URL_Firm_Path); 
//    Reset_Buff(&sInformation.Str_URL_Firm_Name);
//    Reset_Buff(&sInformation.Str_URL_HTTP);
//   
//    Copy_String_2(&sInformation.Str_URL_HTTP , str_Receiv);
//    
//    truct_String Str1 = {(uint8_t*) "//", 2};
//    
//    Pos = Find_String_V2(&Str1, str_Receiv);
//    if(Pos >=0)
//    {
//        Pos += Str1.Length_u16;
//        while(*(str_Receiv->Data_a8 + Pos) != ':')
//        {
//            *(sInformation.Str_URL_Firm_Add.Data_a8 + sInformation.Str_URL_Firm_Add.Length_u16++) = *(str_Receiv->Data_a8 + Pos++);
//            if(Pos >= str_Receiv->Length_u16)
//            {
//                DCU_Respond(PortConfig,(uint8_t *)"ERROR",5);
//                return;
//            }
//        }
//        Pos++;
//        while(*(str_Receiv->Data_a8 + Pos) != '/')
//        {
//            *(sInformation.Str_URL_Firm_Port.Data_a8 + sInformation.Str_URL_Firm_Port.Length_u16++) = *(str_Receiv->Data_a8 + Pos++);
//            if(Pos >= str_Receiv->Length_u16)
//            {
//                DCU_Respond(PortConfig,(uint8_t *)"ERROR",5);
//                return;
//            }
//        }
//        Pos++;
//
//        while(*(str_Receiv->Data_a8 + Pos) != '/')
//        {
//            *(sInformation.Str_URL_Firm_Path.Data_a8 + sInformation.Str_URL_Firm_Path.Length_u16++) = *(str_Receiv->Data_a8 + Pos++);
//            if(Pos >= str_Receiv->Length_u16)
//            {
//                //neu het thi coi nhu day la name
//                Copy_String_2(&sInformation.Str_URL_Firm_Name , &sInformation.Str_URL_Firm_Path);
//                Reset_Buff(&sInformation.Str_URL_Firm_Path);
//                Flag_Get_name = 1;
//                break;
//            }
//        }
//        
//        while(Flag_Get_name == 0)
//        {
//            while(*(str_Receiv->Data_a8 + Pos) != '/')
//            {
//                *(sInformation.Str_URL_Firm_Name.Data_a8 + sInformation.Str_URL_Firm_Name.Length_u16++) = *(str_Receiv->Data_a8 + Pos++);
//                if(Pos >= str_Receiv->Length_u16)
//                {
//                    Flag_Get_name = 1;
//                    //neu het thi coi nhu day la name
//                    break;
//                }
//            }
//            //neu con xuong day thi tiep tuc copy vao path va tiep tuc lau name
//            if(Flag_Get_name == 0)
//            {
//                Copy_String_2(&sInformation.Str_URL_Firm_Path , &sInformation.Str_URL_Firm_Name);
//                Reset_Buff(&sInformation.Str_URL_Firm_Name);
//                if(Pos >= str_Receiv->Length_u16)
//                {
//                    DCU_Respond(PortConfig,(uint8_t *)"ERROR",5);
//                    return;
//                }
//            }
//            Pos++;  
//        }
//        DCU_Respond(PortConfig, (uint8_t *)"OK",2);
//        //chuyen sang update firm
//        sDCU.Mode_Connect_Future = MODE_CONNECT_FTP;
//		osThreadSuspend(ManageFlashHandle);
//		osThreadSuspend(Task_Read_MeterHandle);
//        sInformation.fUpdateFirmware = 1;
//    }
}

void _fQUERY_INDEX_LOG (truct_String *str_Receiv)
{
    uint16_t        IndexLog = 0;
    uint8_t         aNum[10];
    truct_String    StrDec = {&aNum[0], 0};
    
    IndexLog = (pos_DCU_u32 - ADDR_BASE_LOG_DCU) / S25FL_PAGE_SIZE;
    
    if(IndexLog != 0)
    {
        ConvertHexDatatoStringDec(IndexLog, &StrDec);
        DCU_Respond(PortConfig, StrDec.Data_a8, StrDec.Length_u16);
    }else DCU_Respond(PortConfig, (uint8_t*) "0", 1);
}

void _fTEST_LOG (truct_String *str_Receiv)
{
    sDCU.fLog_DCU_Intan = 1;
    DCU_Respond(PortConfig, (uint8_t *)"OK",2);
}



void _fQUERY_ALL_LOG (truct_String *str_Receiv)
{
    DCU_Respond(PortConfig, (uint8_t *)"OK",2);
//    xQueueSend(qSIM_FlashHandle, &Add_Read_Operation, 1000);
}

void _fGET_INDEX_LOG (truct_String *str_Receiv)
{
    uint8_t     temp = 0, pos = 0, count = 0;
    uint16_t    Result = 0;
    
    temp = *(str_Receiv->Data_a8 + pos++);
    while ((temp >= 0x30) && (temp <= 0x39))
    {
        Result = Result * 10 + (temp - 0x30);
        count++;
        temp = *(str_Receiv->Data_a8 + pos++);
        if((count >=8) || (pos > str_Receiv->Length_u16))
          break;
    }
    if((count > 0) && (Result > 0))
    {
        sLogDCU.IndexStart = Result;
        sLogDCU.IndexStop  = Result + 1;
        sLogDCU.fReq_SendLog = 1;
        sLogDCU.CountRead = 0;
    }else 
    {
        DCU_Respond(PortConfig,(uint8_t *)"ERROR",5);
        return;
    }
    
    sInformation.fReq_RLogByServer = PortConfig;       
    
    DCU_Respond(PortConfig, (uint8_t *)"OK",2);
}

void _fGET_FOR_INDEX_LOG (truct_String *str_Receiv)
{
    uint8_t     temp = 0, pos = 0, count = 0;
    uint8_t     Mark = 0;
    uint16_t    Value = 0;
    uint16_t    Start = 0, Stop = 0;
    
    while (pos < str_Receiv->Length_u16)
    {
        if(*(str_Receiv->Data_a8 + pos++) == '(')
        {
            Value = 0;
            count = 0;
            temp = *(str_Receiv->Data_a8 + pos++);
            while ((temp >= 0x30) && (temp <= 0x39))
            {
                Value = Value * 10 + (temp - 0x30);
                count++;
                temp = *(str_Receiv->Data_a8 + pos++);
                if((count >=8) || (pos > str_Receiv->Length_u16))
                  break;
            }
            Mark++;
        }
        
        if(Mark == 1)
            Start = Value;
        else if(Mark == 2)
        {
            Stop = Value;
            break;
        }
    }
    //
    if(Start == 0)
    {
        sLogDCU.fReq_SendLog = 0;   //dung doc log
    }else
    {
        //Check 2 so lay ra duoc
        if((count > 0) && (Start > 0) && (Stop > 0))
        {
            sLogDCU.IndexStart = Start;
            sLogDCU.IndexStop  = Stop + 1;
            sLogDCU.fReq_SendLog = 1;
            sLogDCU.CountRead = 0;
            
        }else 
        {
            DCU_Respond(PortConfig,(uint8_t *)"ERROR",5);
            return;
        }
    }
    
    sInformation.fReq_RLogByServer = PortConfig;       
    
    DCU_Respond(PortConfig, (uint8_t *)"OK",2);
}




//
void Init_APN_Info (void)
{
	uint8_t	 pointer = 0;
	uint16_t tempLength = 0;
	sInformation.APN.Length_u16 = 0;
	sInformation.APN_Dial_String.Length_u16 = 0;
	sInformation.APN_Username.Length_u16 = 0;
	sInformation.APN_Password.Length_u16 = 0;
    
	tempLength = *(__IO uint8_t*)ADDR_APN_INFO; 
    pointer++;
	if((tempLength != 0xFF)&&(*(__IO uint8_t*)(ADDR_APN_INFO+119) == ','))
	{
        while(*(__IO uint8_t*)(ADDR_APN_INFO+pointer) != ',')
        {
            *(sInformation.APN.Data_a8+sInformation.APN.Length_u16) = *(__IO uint8_t*)(ADDR_APN_INFO+pointer);
            sInformation.APN.Length_u16++;
            pointer++;
            if(sInformation.APN.Length_u16 >= BUFF_LENGTH_SETUP)
                break;
        }
        pointer++;
        if(pointer < BUFF_LENGTH_SETUP)
        {
            while(*(__IO uint8_t*)(ADDR_APN_INFO+pointer) != ',')
            {
                *(sInformation.APN_Dial_String.Data_a8+sInformation.APN_Dial_String.Length_u16) = *(__IO uint8_t*)(ADDR_APN_INFO+pointer);
                sInformation.APN_Dial_String.Length_u16++;
                pointer++;
                if(sInformation.APN_Dial_String.Length_u16 >= BUFF_LENGTH_SETUP)
                    break;
            }
            pointer++;
            while(*(__IO uint8_t*)(ADDR_APN_INFO+pointer) != ',')
            {
                *(sInformation.APN_Username.Data_a8+sInformation.APN_Username.Length_u16) = *(__IO uint8_t*)(ADDR_APN_INFO+pointer);
                sInformation.APN_Username.Length_u16++;
                pointer++;
                if(sInformation.APN_Username.Length_u16 >= BUFF_LENGTH_SETUP)
                    break;
            }
            pointer++;
            while((pointer < (tempLength+1)) && (sInformation.APN_Password.Length_u16 < BUFF_LENGTH_SETUP))
            {
                *(sInformation.APN_Password.Data_a8+sInformation.APN_Password.Length_u16) = *(__IO uint8_t*)(ADDR_APN_INFO+pointer);
                sInformation.APN_Password.Length_u16++;
                pointer++;
                if(sInformation.APN_Password.Length_u16 >= BUFF_LENGTH_SETUP)
                    break;
            }
        }
	}
    else
    {
        sInformation.APN.Length_u16 = 7;
        sInformation.APN_Dial_String.Length_u16 = 5;
        sInformation.APN_Username.Length_u16 = 0;
        sInformation.APN_Password.Length_u16 = 0;
    }
}



void Init_Auto_Connect_Time (void)
{
	uint16_t tempLength = 0;
	
	tempLength = *(__IO uint16_t*)ADDR_CONNECT_TIME;
	
	if(tempLength != 8)
	{
		sInformation.ConnectTime.Hours = 0x08;
		sInformation.ConnectTime.Minutes = 0x00;
		sInformation.ConnectTime.Seconds = 0x00;
	}
	else
	{
		sInformation.ConnectTime.Hours = ((*(__IO uint8_t*)(ADDR_CONNECT_TIME+2))-0x30)*10 + (*(__IO uint8_t*)(ADDR_CONNECT_TIME+3) - 0x30);
		sInformation.ConnectTime.Minutes = ((*(__IO uint8_t*)(ADDR_CONNECT_TIME+5))-0x30)*10 + (*(__IO uint8_t*)(ADDR_CONNECT_TIME+6) - 0x30);
		sInformation.ConnectTime.Seconds = ((*(__IO uint8_t*)(ADDR_CONNECT_TIME+8))-0x30)*10 + (*(__IO uint8_t*)(ADDR_CONNECT_TIME+9) - 0x30);
	}
	
	tempLength = *(__IO uint16_t*)ADDR_CONNECT_PERIOD;
	
	if(tempLength != 8)
	{
		sInformation.ConnectPeriod.Hours = 0x08;
		sInformation.ConnectPeriod.Minutes = 0x00;
		sInformation.ConnectPeriod.Seconds = 0x00;
	}
	else
	{
		sInformation.ConnectPeriod.Hours = ((*(__IO uint8_t*)(ADDR_CONNECT_PERIOD+2))-0x30)*10 + (*(__IO uint8_t*)(ADDR_CONNECT_PERIOD+3) - 0x30);
		sInformation.ConnectPeriod.Minutes = ((*(__IO uint8_t*)(ADDR_CONNECT_PERIOD+5))-0x30)*10 + (*(__IO uint8_t*)(ADDR_CONNECT_PERIOD+6) - 0x30);
		sInformation.ConnectPeriod.Seconds = ((*(__IO uint8_t*)(ADDR_CONNECT_PERIOD+8))-0x30)*10 + (*(__IO uint8_t*)(ADDR_CONNECT_PERIOD+9) - 0x30);
	}
	
	tempLength = *(__IO uint16_t*)ADDR_DISCONNECT_TIME;
	
	if(tempLength != 8)
	{
		sInformation.DisconnectTime.Hours = 0x08;
		sInformation.DisconnectTime.Minutes = 0x00;
		sInformation.DisconnectTime.Seconds = 0x00;
	}
	else
	{
		sInformation.DisconnectTime.Hours = ((*(__IO uint8_t*)(ADDR_DISCONNECT_TIME+2))-0x30)*10 + (*(__IO uint8_t*)(ADDR_DISCONNECT_TIME+3) - 0x30);
		sInformation.DisconnectTime.Minutes = ((*(__IO uint8_t*)(ADDR_DISCONNECT_TIME+5))-0x30)*10 + (*(__IO uint8_t*)(ADDR_DISCONNECT_TIME+6) - 0x30);
		sInformation.DisconnectTime.Seconds = ((*(__IO uint8_t*)(ADDR_DISCONNECT_TIME+8))-0x30)*10 + (*(__IO uint8_t*)(ADDR_DISCONNECT_TIME+9) - 0x30);
	}
    
    //Init Time send Profile
    tempLength = *(__IO uint8_t*)ADDR_TIMESEND_LPF;
	if(tempLength != 4)
	{
		sInformation.sTimeStart_Send_Lpf.hour = 0;
        sInformation.sTimeStart_Send_Lpf.min = 0;
        sInformation.sTimeStop_Send_Lpf.hour = 0;
        sInformation.sTimeStop_Send_Lpf.min = 1;
	}
	else
	{
		sInformation.sTimeStart_Send_Lpf.hour   = *(__IO uint8_t*)(ADDR_TIMESEND_LPF+1);
        sInformation.sTimeStart_Send_Lpf.min    = *(__IO uint8_t*)(ADDR_TIMESEND_LPF+2);
        sInformation.sTimeStop_Send_Lpf.hour    = *(__IO uint8_t*)(ADDR_TIMESEND_LPF+3);
        sInformation.sTimeStop_Send_Lpf.min     = *(__IO uint8_t*)(ADDR_TIMESEND_LPF+4);
	}
}

void Init_Device_IP (void)
{
	uint8_t	 pointer = 0;
	uint16_t tempLength = 0;
	uint8_t  i=0;
    
	tempLength = *(__IO uint8_t*)ADDR_DEVICE_IP;
    pointer++;
	if((tempLength != 0xFF)&&(*(__IO uint8_t*)(ADDR_DEVICE_IP+39) == ','))
	{
		while(*(__IO uint8_t*)(ADDR_DEVICE_IP+pointer) != ',')
		{
			*(sInformation.Device_Name.Data_a8+sInformation.Device_Name.Length_u16) = *(__IO uint8_t*)(ADDR_DEVICE_IP+pointer);
			sInformation.Device_Name.Length_u16++;
			pointer++;
		}
//        pointer++; // ' '
        pointer++;
        while(pointer < (tempLength+1))
        {
            *(sInformation.Device_IP.Data_a8+sInformation.Device_IP.Length_u16) = *(__IO uint8_t*)(ADDR_DEVICE_IP+pointer);
            sInformation.Device_IP.Length_u16++;
            pointer++;
        }
    }
    else
    {
        //Device name
        sInformation.Device_Name.Length_u16 = 3;
    
        for(i=0; i< sInformation.sServer_MQTT.Length_u16; i++)
            *(sInformation.Device_IP.Data_a8+i) = aServer_MQTT[i];
        sInformation.Device_IP.Length_u16 = sInformation.sServer_MQTT.Length_u16;    
    }
}

void Init_UART_Config (void)
{
	uint16_t i=0;
	
    if(*(__IO uint8_t*)(ADDR_UART_CONFIG != 0xFF))
    {
        for(i=0;i<9;i++)
            *(sInformation.UART_Config.Data_a8+i) = *(__IO uint8_t*)(ADDR_UART_CONFIG+i+2);
        
        sInformation.UART_Config.Length_u16 = 9;
    }
    else
    {
        *(sInformation.UART_Config.Data_a8) = '(';
        *(sInformation.UART_Config.Data_a8+1) = '5';
        *(sInformation.UART_Config.Data_a8+2) = ',';
        *(sInformation.UART_Config.Data_a8+3) = 'N';
        *(sInformation.UART_Config.Data_a8+4) = ',';
        *(sInformation.UART_Config.Data_a8+5) = '8';
        *(sInformation.UART_Config.Data_a8+6) = ',';
        *(sInformation.UART_Config.Data_a8+7) = '1';
        *(sInformation.UART_Config.Data_a8+8) = ')';
        //
        sInformation.UART_Config.Length_u16 = 9;
        //luu lai
        _fSave_Uart_Config();
    }
}

void Set_UartMeterConfig (void)
{
    // Init uartcofig
    UART_METER.Instance = UART__METER;
    switch(*(sInformation.UART_Config.Data_a8+1))
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
    switch(*(sInformation.UART_Config.Data_a8+3))
    {
        case 'N':
            UART_METER.Init.Parity = UART_PARITY_NONE;
            break;
        case 'E':
            UART_METER.Init.Parity = UART_PARITY_EVEN;
            break;
        case 'O':
            UART_METER.Init.Parity = UART_PARITY_ODD;
            break;
        default:
            break;
    }
    switch(*(sInformation.UART_Config.Data_a8+5))
    {
        case '7':
            if(UART_METER.Init.Parity == UART_PARITY_NONE)
                UART_METER.Init.WordLength = UART_WORDLENGTH_7B;
            else
                UART_METER.Init.WordLength = UART_WORDLENGTH_8B;
            break;
        case '8':
            if(UART_METER.Init.Parity == UART_PARITY_NONE)
                UART_METER.Init.WordLength = UART_WORDLENGTH_8B;
            else
                UART_METER.Init.WordLength = UART_WORDLENGTH_9B;
            break;
        default:
            break;
    }
    switch(*(sInformation.UART_Config.Data_a8+7))
    {
        case '1':
            UART_METER.Init.StopBits = UART_STOPBITS_1;
            break;
        case '2':
            UART_METER.Init.StopBits = UART_STOPBITS_2;
            break;
        default:
            break;
    }		
    UART_METER.Init.Mode = UART_MODE_TX_RX;
    UART_METER.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    UART_METER.Init.OverSampling = UART_OVERSAMPLING_16;
    UART_METER.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE ;
    UART_METER.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    HAL_UART_Init(&UART_METER);
}

void GET_UART_Meter_Config (void)
{
	uint8_t     Baud = 0, Parity = 0, Databit = 0, stopbit = 0;
    
    switch(UART_METER.Init.BaudRate)
    {
        case 300:  Baud = '0';  break;
        case 600:  Baud = '1';  break;
        case 1200: Baud = '2';  break;
        case 2400: Baud = '3';  break;
        case 4800: Baud = '4';  break;
        case 9600: Baud = '5';  break;
        case 19200: Baud = '6'; break;	
        default:  break;       
    }
    //
    switch(UART_METER.Init.Parity)
    {
        case UART_PARITY_NONE: Parity = 'N';  break;
        case UART_PARITY_EVEN: Parity = 'E';  break;
        case UART_PARITY_ODD:  Parity = 'O';  break;
        default:  break;
            
    }
    //
    switch(UART_METER.Init.WordLength)
    {
        case UART_WORDLENGTH_7B:
            Databit = '7';
            break;
        case UART_WORDLENGTH_8B:
            if(UART_METER.Init.Parity == UART_PARITY_NONE)
                Databit = '8'; 
            else
                Databit = '7';
            break;
        case UART_WORDLENGTH_9B:
            if(UART_METER.Init.Parity == UART_PARITY_NONE)
                Databit = '9'; 
            else
                Databit = '8';
            break;
        default:
            break;
    }
    //
    switch(UART_METER.Init.StopBits)
    {
        case UART_STOPBITS_1:
            stopbit = '1';
            break;
        case UART_STOPBITS_2:
            stopbit = '2';
            break;
        default:
            break;
    }		
    //
    *(sInformation.UART_Config.Data_a8) = '(';    
    *(sInformation.UART_Config.Data_a8+1) = Baud;
    *(sInformation.UART_Config.Data_a8+2) = ',';
    *(sInformation.UART_Config.Data_a8+3) = Parity;
    *(sInformation.UART_Config.Data_a8+4) = ',';
    *(sInformation.UART_Config.Data_a8+5) = Databit;
    *(sInformation.UART_Config.Data_a8+6) = ',';
    *(sInformation.UART_Config.Data_a8+7) = stopbit;
    *(sInformation.UART_Config.Data_a8+8) = ')';
    
    sInformation.UART_Config.Length_u16 = 9;
}



uint8_t aDataTemp[64] = {0};
uint8_t CheckATCommandPC(void)
{
	uint8_t var;
	int Pos_Str = -1;
	uint16_t i = 0;
    truct_String Tempstring;

	Tempstring.Data_a8 = &aDataTemp[0];
    uint16_t j = 0;
    
    //convert lai chu in hoa thanh chu thuong
    for(j = 0; j < Uart1_Control.str_Receiv.Length_u16; j++)
    {
      if((*(Uart1_Control.str_Receiv.Data_a8 + j) == '=') || (*(Uart1_Control.str_Receiv.Data_a8 + j) == '?'))
        break;
      if((*(Uart1_Control.str_Receiv.Data_a8 + j) >= 0x41) && (*(Uart1_Control.str_Receiv.Data_a8 + j) <= 0x5A))
         *(Uart1_Control.str_Receiv.Data_a8 + j) = *(Uart1_Control.str_Receiv.Data_a8 + j) + 0x20;
    }
    
	for (var = _ACCOUNT; var <= _QUERY_READ_INDEX_LOG; var++) 
    {
		Pos_Str = Find_String_V2((truct_String*) &CheckList_AT[var].sTempReceiver,&Uart1_Control.str_Receiv);
		if ((Pos_Str >= 0) && (CheckList_AT[var].CallBack != NULL))
		{
            //kiem tra xem có 2 kí tu CR va LF cuoi cung khong thi xoa di
            if((*(Uart1_Control.str_Receiv.Data_a8 + Uart1_Control.str_Receiv.Length_u16 - 2) == 0x0D) &&
               (*(Uart1_Control.str_Receiv.Data_a8 + Uart1_Control.str_Receiv.Length_u16 - 1) == 0x0A))
             Uart1_Control.str_Receiv.Length_u16 -= 2;
            
			Pos_Str += CheckList_AT[var].sTempReceiver.Length_u16;
			//Copy_String_section(&Tempstring,&Uart1_Control.str_Receiv,0,Pos_Str,Uart1_Control.str_Receiv.Length_u16 - CheckList_AT[var].sTempReceiver.Length_u16,0xFF);
			for(i = 0; i<Uart1_Control.str_Receiv.Length_u16 - Pos_Str;i++)
			{
				aDataTemp[i] = *(Uart1_Control.str_Receiv.Data_a8+Pos_Str+i);
			}
			//Tempstring.Length_u16 = Uart1_Control.str_Receiv.Length_u16 - Pos_Str;
            Tempstring.Length_u16 = Uart1_Control.str_Receiv.Length_u16 - Pos_Str; // CR|LF // CR|LF. khi test k có hai ki tu nay
			PortConfig = 1;
			CheckList_AT[var].CallBack(&Tempstring);
			return 1;
		}
	}
	return 0;
}


// Server
uint8_t CheckATCommandServer(void)
{
//	uint8_t aDataTemp[64] = {0};
	truct_String Tempstring;
	uint8_t var;
	int Pos_Str = -1;
	uint16_t i = 0;

	Tempstring.Data_a8 = &aDataTemp[0];
    uint16_t j = 0;
    
    //convert lai chu in hoa thanh chu thuong
    for(j = 0; j < UartSIM_Control.str_Receiv.Length_u16; j++)
    {
        if((*(UartSIM_Control.str_Receiv.Data_a8 + j) == '=') || (*(UartSIM_Control.str_Receiv.Data_a8 + j) == '?'))
            break;
        
        if((*(UartSIM_Control.str_Receiv.Data_a8 + j) >= 0x41) && (*(UartSIM_Control.str_Receiv.Data_a8 + j) <= 0x5A))
            *(UartSIM_Control.str_Receiv.Data_a8 + j) = *(UartSIM_Control.str_Receiv.Data_a8 + j) + 0x20;
    }
         
	for (var = _ACCOUNT; var <= _QUERY_READ_INDEX_LOG; var++) {
		Pos_Str = Find_String_V2((truct_String*) &CheckList_AT[var].sTempReceiver,&UartSIM_Control.str_Receiv);
		if ((Pos_Str >= 0) && (CheckList_AT[var].CallBack != NULL))
		{
            //kiem tra xem có 2 kí tu CR va LF cuoi cung khong thi xoa di
            if((*(UartSIM_Control.str_Receiv.Data_a8 + UartSIM_Control.str_Receiv.Length_u16 - 2) == 0x0D) &&
               (*(UartSIM_Control.str_Receiv.Data_a8 + UartSIM_Control.str_Receiv.Length_u16 - 1) == 0x0A))
             UartSIM_Control.str_Receiv.Length_u16 -= 2;
              
            Reset_Buff(&Str_Cmd_AT);
            //Copy lenh vao buff. de repond kem theo lenh
            for(i = 0; i <(UartSIM_Control.str_Receiv.Length_u16 - Pos_Str); i++)
            {
                if(((*(UartSIM_Control.str_Receiv.Data_a8+Pos_Str+i) == '?') || (*(UartSIM_Control.str_Receiv.Data_a8+Pos_Str+i) == '=')) && (var != _SEND_DATA_SERIAL))
                    break;
                else 
                    *(Str_Cmd_AT.Data_a8 + Str_Cmd_AT.Length_u16++) = *(UartSIM_Control.str_Receiv.Data_a8+Pos_Str+i); 
            }
    
			Pos_Str += CheckList_AT[var].sTempReceiver.Length_u16;
			
			for(i = 0; i<UartSIM_Control.str_Receiv.Length_u16 - Pos_Str;i++)
			{
				aDataTemp[i] = *(UartSIM_Control.str_Receiv.Data_a8+Pos_Str+i);
			}
			//Tempstring.Length_u16 = UartSIM_Control.str_Receiv.Length_u16 - CheckList_AT[var].sTempReceiver.Length_u16;
            Tempstring.Length_u16 = UartSIM_Control.str_Receiv.Length_u16 - Pos_Str; 
            
			PortConfig = 3;
			CheckList_AT[var].CallBack(&Tempstring);
			return 1;
		}
	}
	return 0;
}
// SMS
uint8_t CheckATCommandSMS(void)
{
    return 1;
}
//
void DCU_Respond(uint8_t portNo, uint8_t *data, uint8_t length)
{    
	switch(portNo)
	{
	case 1://response via serial port
		HAL_UART_Transmit(&UART_SERIAL,data,length,1000);
		break;
	case 2://response via SIM - SMS
		break;
	case 3: //response Server
        //Dong goi theo mau: ban tin push phan hoi Lenh AT: <STX>RESPONSE(IMEI)(meterID)(meterType)(command)(response)<ETX><BCC>
        ATSendDataLength_u8 = _fPacket_RespondAT(&aATBuffSendData[0], Str_Cmd_AT.Data_a8, Str_Cmd_AT.Length_u16, data, length);     
        sTCP.aNeed_Send[DATA_RESPOND_AT] = 1;
        osDelay(100);
		break;
	default:
		break;
	}
}

uint16_t _fPacket_RespondAT (uint8_t* Buff, uint8_t* Cmd, uint16_t lenCmd, uint8_t* aData, uint16_t lendata)
{
    uint16_t    i = 0;
    uint8_t     Temp_BBC = 0;
    uint8_t     Length = 0;
    
    *(Buff + Length++) = 0x02;
    //Obis Response
    for(i = 0; i < Str_OB_RESPONSE_AT.Length_u16; i++)
        *(Buff + Length++) = *(Str_OB_RESPONSE_AT.Data_a8 + i);
    // IMEI Moderm
    *(Buff + Length++) = '(';
    for(i=0;i<sDCU.sDCU_id.Length_u16;i++) 
        *(Buff + Length++) = *(sDCU.sDCU_id.Data_a8+i);
    *(Buff + Length++) = ')';
    //Meter ID
    *(Buff + Length++) = '(';
    for(i=0;i<sDCU.sMeter_id_now.Length_u16;i++) 
        *(Buff + Length++) = *(sDCU.sMeter_id_now.Data_a8+i);
    *(Buff + Length++) = ')';
    //Meter type
    *(Buff + Length++) = '(';
    for(i=0;i<Str_MeterType_u8[sDCU.MeterType].Length_u16;i++) 
        *(Buff + Length++) = *(Str_MeterType_u8[sDCU.MeterType].Data_a8+i);
    *(Buff + Length++) = ')';
    //cmd
    *(Buff + Length++) = '(';
    for(i=0;i< lenCmd;i++) 
        *(Buff + Length++) = *(Cmd+i);
    *(Buff + Length++) = ')';
    //Response
    *(Buff + Length++) = '(';
    for(i=0; i< lendata; i++)
        *(Buff + Length++) = *(aData+i);
    *(Buff + Length++) = ')';
    //ETX và BBC
    *(Buff + Length++) = 0x03;
    
    Temp_BBC = BBC_Cacul(&Buff[1],Length - 1);
    *(Buff + Length++) = Temp_BBC;
    
    return Length;
}

void DCU_Respond_Direct (uint8_t portNo, uint8_t *data, uint8_t length)
{   
	switch(portNo)
	{
	case 1://response via serial port
		HAL_UART_Transmit(&UART_SERIAL,data,length,1000);
		break;
	case 2://response via SIM - SMS
		break;
	case 3: //response Server
        //Dong goi theo mau: ban tin push phan hoi Lenh AT: <STX>RESPONSE(IMEI)(meterID)(meterType)(command)(response)<ETX><BCC>
        ATSendDataLength_u8 = _fPacket_RespondAT(&aATBuffSendData[0], Str_Cmd_AT.Data_a8, Str_Cmd_AT.Length_u16, data, length);  
        _f_TCP_SEND_SERVER_DIRECT(&aATBuffSendData[0], ATSendDataLength_u8);
		break;
	default:
		break;
	}
}



//
void ResetCountGPRS(void)
{
	sSim900_status.Count_Startup_GPRS = 0;
	sSim900_status.Count_Init_GPRS = 0;
	sSim900_status.Count_Soft_Reset = 0;
}

