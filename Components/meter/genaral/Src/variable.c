
#include "variable.h"
#include "t_mqtt.h"
#include "my_adc.h"
#include "myuart.h"
#include "rtc.h"

#include "myDefine.h"

uint8_t	        Device_Name_Buff[BUFF_LENGTH_SETUP] = "abc";
uint8_t	        Device_IP[BUFF_LENGTH_SETUP];
uint8_t	        Ping_IP[BUFF_LENGTH_SETUP];
uint8_t	        UART_Config[BUFF_LENGTH_SETUP];
uint8_t	        APN[BUFF_LENGTH_SETUP] = "V3G2057";   //"abcd";
uint8_t	        APN_Dial_String[BUFF_LENGTH_SETUP] = "12345";
uint8_t	        APN_Username[BUFF_LENGTH_SETUP] = "xyz";  //"xyz";
uint8_t	        APN_Password[BUFF_LENGTH_SETUP] = "1111";
uint8_t	        PasswordModerm[BUFF_LENGTH_SETUP] = "123456";
uint8_t         aATBuffSendData[BUFF_LENGTH_AT_SEND];
uint16_t        ATSendDataLength_u8 = 0;
uint8_t         aATBuffReceData[BUFF_LENGTH_AT_SEND];
uint16_t        ATReceDataLength_u8 = 0;
uint8_t         aBuffHansdshake[100] ={0};

uint8_t		                Buff_HTTP_URL[50]={"http://172.16.11.141:8909/FIRM_HCMCHES1_4.bin"};
uint8_t		                Buff_FTP_IP[LENGTH_FTP]={"124.158.5.154"};
uint8_t		                Buff_FTP_Port[LENGTH_FTP]={"8909"};
uint8_t		                aCrC_Firmware[MAX_LENGTH_PACKFIRM];
truct_String                sHandsake;	
truct_String                sFirmware_Version	= 	{(uint8_t *)"SV1_9_6", 7};
/*
 *				UART 1
 */
/*======================== SYSTEM ======================*/
uint32_t 	            RT_Count_Systick_u32 = 1;
ST_TIME_FORMAT          sRTC;
/*======================== Init Variables ======================*/
struct_Uart_Data 		Uart1_Control, UartSIM_Control; // Mang chua Data PC/SIM -> DCU
truct_SIM900_status 	sSim900_status;
struct_TCP   			sTCP;
struct_FTP				sFTP;
truct_String			sFTP_Path,sFTP_Name;
Status_Read_Meter_truct sStatus_Meter;
struct_Check_Factory 	sCheck_Factory;

//						Variable MQTT
Meter_Flash_Queue_Struct        *sFlash_Update;
DCU_struct 	                    sDCU;
Manage_Flash_Struct				Manage_Flash;
Manage_Flash_Struct				Manage_Flash_Lpf;
Manage_Flash_Struct				Manage_Flash_Bill;
Manage_Flash_Struct				Manage_Flash_log;

const struct_CheckList_GSM CheckList_GSM[] = 
{		
//			id ENUM				kind process		str send								str Received
		{_RESET_SIM900, 		_fRESET_SIM900,		{(uint8_t*)"AT+CFUN=1,1\r",12},			{(uint8_t*)"NORMAL POWER DOWN",17}}, // OK
		{_RES_ALREADY_CONNECT, 	_fRES_ERROR,		{NULL}, 								{(uint8_t*)"ALREADY CONNECT",15}},   // 
		{_RES_SIM_LOST, 		_fRES_SIM_LOST,		{NULL},									{(uint8_t*)"SIM CRASH",9}},  
        {_RES_SIM_REMOVE, 		_fRES_SIM_LOST,		{NULL},									{(uint8_t*)"SIM REMOVED",11}},        // 
		{_RES_CLOSED,			_fRES_CLOSED,		{NULL}, 								{(uint8_t*)"+SERVER DISCONNECTED",20}}, //+SERVER DISCONNECTED:0
		{_RES_PDP_DEACT, 		_fRES_CLOSED,		{NULL}, 								{(uint8_t*)"+NETWORK DISCONNECTED",21}},   //+NETWORK DISCONNECTED:0
		{_RES_CALL_READY, 		_fRES_CALL_READY,	{NULL}, 								{(uint8_t*)"Call Ready",10}},
		{_RES_ERROR, 			_fRES_ERROR,		{NULL}, 								{(uint8_t*)"ERROR",5}},  
		{_FTP_GET_PENDING, 		NULL,				{NULL}, 								{(uint8_t*)"+FTPEXTGET:| 3|3",16}},
		{_FTP_GET_DONE, 		_fFTP_GET_DONE,		{NULL}, 								{(uint8_t*)"FTPETGET: 1,0",12}},
		{_AT_CHECK_AT, 			NULL,				{(uint8_t*)"AT\r",3}, 					{(uint8_t*)"OK",2}},
		{_AT_SIM_ID, 			_fAT_SIM_ID,		{(uint8_t*)"AT+CICCID\r",10}, 			{(uint8_t*)"+CICCID:",8}},
	// pre Setup
        {_AT_RESET_MODULE, 		NULL,				{(uint8_t*)"AT+CRESET\r",10},		    {(uint8_t*)"OK",2}},
		{_AT_BAUD_RATE, 		NULL,				{(uint8_t*)"AT+IPR=115200\r",14},		{(uint8_t*)"OK",2}},  
		{_AT_NDISPLAY_CMD, 		NULL,				{(uint8_t*)"ATE0\r",5},					{(uint8_t*)"OK",2}},
		{_AT_DISPLAY_CMD, 		NULL,				{(uint8_t*)"ATE1\r",5},					{(uint8_t*)"OK",2}},  
        {_AT_GET_IMEI, 		    _fGET_IMEI,			{(uint8_t*)"AT+CGSN\r",10},			    {(uint8_t*)"OK",2}},
		{_AT_SET_FUN, 	        NULL,				{(uint8_t*)"AT+CFUN=1,0\r",12},         {(uint8_t*)"OK",2}},
	// State Startup nontransfer  
        {_AT_TCP_COFI_CONTEXT_2,    _f_COFI_CONTEXT_2,	        {(uint8_t*)"AT+CGDCONT=1,\"IP\",\"",19}, 	    {NULL}},            
        {_AT_TCP_COFI_CONTEXT_3,    NULL,	                    {(uint8_t*)"\"\r",2}, 	                        {(uint8_t*)"OK",2}},

        {_AT_APN_AUTHEN_1,          _f_APN_AUTHEN_1,	        {(uint8_t*)"AT$QCPDPP=1,",12}, 	                {NULL}},
        {_AT_APN_AUTHEN_2,          NULL,           	        {(uint8_t*)"\r",1}, 	                        {(uint8_t*)"OK",2}},

        {_AT_TCP_NETOPEN,           NULL,                       {(uint8_t*)"AT+NETOPEN\r",11},                  {(uint8_t*)"+NETOPEN:SUCCESS",16}},
        {_AT_TCP_NETCLOSE,          NULL,                       {(uint8_t*)"AT+NETCLOSE\r",12},                 {(uint8_t*)"OK",2}},
        
		{_AT_TCP_OUT_DATAMODE, 	    _fNON_TRANPARENT_MODE,	    {(uint8_t*)"+++",3}, 			                {(uint8_t*)"OK",2}},       
        {_AT_TCP_TRANS, 		    _fTRANPARENT_MODE,			{(uint8_t*)"AT+CIPMODE=2\r",13}, 		        {(uint8_t*)"OK",2}}, 
        {_AT_TCP_TRANS_SETUP,       NULL,                       {(uint8_t*)"AT+MCIPCFGPL=0,1,0,0,0\r",23},      {(uint8_t*)"OK",2}}, 
		{_AT_TCP_IN_DATAMODE,       _fTRANPARENT_MODE,	        {(uint8_t*)"ATO\r",4}, 					        {(uint8_t*)"CONNECT",7}},      

        {_AT_SYN_TIME_ZONE, 	    NULL,	                    {(uint8_t*)"AT+MTZ=1\r",9},  			        {(uint8_t*)"OK",2}},     //timezone auto
        {_AT_GET_RTC, 			    _f_GET_CLOCK,		        {(uint8_t*)"AT+CCLK?\r",9},				        {(uint8_t*)"+CCLK:",6}},   //+CCLK: "80/01/06,00:00:31+00"
	// Check SIm
		{_AT_CHECK_SIM, 		    _fAT_CHECK_SIM, 	        {(uint8_t*)"AT+CPIN?\r",9}, 			        {(uint8_t*)"OK",2}},  //
		{_AT_CHECK_RSSI, 		    _fAT_CHECK_RSSI,	        {(uint8_t*)"AT+CSQ\r",7}, 				        {(uint8_t*)"CSQ:",4}},
		
	// Ring Pin
		{_AT_SAVE_CMD, 			    NULL,				        {(uint8_t*)"AT&W\r",5},					        {(uint8_t*)"OK",2}},
		{_AT_TEST_0, 			    NULL,				        {(uint8_t*)"AT+CGATT=1\r",11},		            {(uint8_t*)"OK",2}},
        {_AT_GET_IP_APN,    	    _fCHECK_APN_ACTIVE,         {(uint8_t*)"AT+CGPADDR=1\r",13},		        {(uint8_t*)"+CGPADDR: 1,",12}},
        {_AT_ACTIVE_APN, 			NULL,				        {(uint8_t*)"AT+CGACT=1,1\r",13},		        {(uint8_t*)"OK",2}},
        
    // 4G Module. Switch 4G 3G 2G
        {_AT_ACCESS_RA_AUTO,        NULL,                       {(uint8_t*)"AT+CNMP=2\r",10},                   {(uint8_t*)"OK",2}},
        {_AT_ACCESS_RA_2G,          NULL,                       {(uint8_t*)"AT+CNMP=13\r",11},                  {(uint8_t*)"OK",2}},
        {_AT_ACCESS_RA_3G,          NULL,                       {(uint8_t*)"AT+CNMP=14\r",11},                  {(uint8_t*)"OK",2}},
        {_AT_ACCESS_RA_4G,          NULL,                       {(uint8_t*)"AT+CNMP=38\r",11},                  {(uint8_t*)"OK",2}},
        
        {_AT_CHECK_ACCESS,          NULL,                       {(uint8_t*)"AT+CNMP?\r",9},                     {(uint8_t*)"+CNMP",5}},  //them callback luu respond vao uartport gia tri
        {_AT_CHECK_BAND,            NULL,                       {(uint8_t*)"AT+BAND?\r",9},                     {(uint8_t*)"+BAND",5}},
        {_AT_CHECK_ATTACH, 		    NULL,		                {(uint8_t*)"AT+CGATT?\r",10},			        {(uint8_t*)"+CGATT: 1",9}},
        
	// Setup GPRS
		{_AT_RECOUNT_FIRST,	        _fRESET_COUNT_FIRST,	    {NULL},									        {NULL}}, //124.158.5.155 
        
		{_AT_TCP_CONNECT, 		    _fTRANPARENT_MODE1,	        {(uint8_t*)"AT+CIPOPEN=0,\"TCP\",\"",20},       {NULL}}, //124.158.5.155
		{_AT_TCP_CONNECT1, 		    _fTRANPARENT_MODE2,	        {(uint8_t*)"\",",2}, 					        {NULL}},
		{_AT_TCP_CONNECT2, 		    _fOPEN_TCP,	                {(uint8_t*)",0\r",3},  					        {(uint8_t*)"CONNECT",7}},   

        {_AT_PING_TCP,          _fPING_TCP,	        {(uint8_t*)"AT+MPING=\"",10}, 	        {NULL}},  //chua sua
        {_AT_PING_TCP_2,        NULL,	            {(uint8_t*)"\",1,4\r",6},               {NULL}},   //
        {_AT_GET_IP_SOCKET,	    _fGet_IP_Module,	{(uint8_t*)"AT+IPADDR\r",10},			{(uint8_t*)"+IPADDR:",8}},   
        
        {_AT_PING_TEST,         _fPING_TEST,	    {(uint8_t*)"AT+MPING=\"",10}, 	        {NULL}},  //chua sua
        {_AT_PING_TEST_2,       NULL,	            {(uint8_t*)"\",1,4\r",6},               {(uint8_t*)"+MPING:3",8}},
        {_AT_MAN_LOG,           NULL,	            {(uint8_t*)"AT+MLOGK=5\r", 11}, 	    {(uint8_t*)"OK",2}}, 
	// Check IP
		{_AT_CHECK_CREG, 		_fAT_CHECK_CREG,	{(uint8_t*)"AT+CREG?\r",9}, 			{(uint8_t*)"CREG: 0,|1|5",12}}, // Gia tri sau 1,5 : ket noi OK
		{_AT_CHECK_TCP_STATUS, 	NULL,				{(uint8_t*)"AT+QISTATE\r",11}, 		    {(uint8_t*)"QISTATE:",8}},
        {_AT_TCP_SEND, 	        _fCHECK_ACK_MESS,	{(uint8_t*)"AT+QISTATE\r",11}, 		    {(uint8_t*)"ACK",3}},      //muon respond ACK l�nh nay
		{_AT_TCP_CLOSE,	 		NULL,       		{(uint8_t*)"AT+CIPCLOSE=0\r",14}, 		{(uint8_t*)"OK",2}}, 
    // File	
        {_AT_SYS_DEL_FILE_1,    _fSystem_Del_File,  {(uint8_t*)"AT+MFSDEL=\"",11},                          {NULL}},  
        {_AT_SYS_DEL_FILE_2,    NULL,               {(uint8_t*)"\"\r",2},                                   {(uint8_t*)"OK",2}},  
        {_AT_SYS_LIST_FILE,     NULL,               {(uint8_t*)"AT+MFSLS\r",9},                             {(uint8_t*)"OK",2}}, 
    // FTP		
		{_FTP_SERVER, 			_fFTP_SENDIP_2,		{(uint8_t*)"AT+CFTPSERV=\"",13}, 			            {NULL}},
        {_FTP_SERVER_2, 	    NULL,		        {(uint8_t*)"\"\r",2}, 			                        {(uint8_t*)"OK",2}},   //ip thoi
        {_FTP_PORT,             NULL,               {(uint8_t*)"AT+CFTPPORT=21\r",15},                      {(uint8_t*)"OK",2}},
        
		{_FTP_USERNAME, 		NULL,		        {(uint8_t*)"AT+CFTPUN=\"admin\"\r",18}, 				{(uint8_t*)"OK",2}},
		{_FTP_PASSWORD, 		NULL,		        {(uint8_t*)"AT+CFTPPW=\"admin\"\r",18}, 				{(uint8_t*)"OK",2}},
        {_FTP_TYPE, 		    NULL,		        {(uint8_t*)"AT+CFTPTYPE=\"I\"\r",16}, 				    {(uint8_t*)"OK",2}},
        
        {_FTP_LIST_FILE,        NULL,		        {(uint8_t*)"AT+CFTPLIST=\"/\"\r",16}, 				    {(uint8_t*)"+CFTPLIST:SUCCESS",17}}, 
        {_FTP_GET_FILE_1,       NULL,		        {(uint8_t*)"AT+CFTPGET=\"/Test/FIRM_HCMCHES2.bin\",10\r",40}, 	{(uint8_t*)"+CFTPGET:",9}}, 
        
        {_AT_FTP_GET_FILE1,     _fCFTP_GET_FILE1,       {(uint8_t*)"AT+CFTPGETFILE=\"/",17},                {NULL}},
        {_AT_FTP_GET_FILE2,     _fCFTP_GET_FILE2,       {(uint8_t*)"\",\"",3},                              {NULL}},
        {_AT_FTP_GET_FILE,      _fCFTP_GET_FILE,        {(uint8_t*)"\",0\r",4},                             {(uint8_t*)"+CFTPGETFILE:SUCCESS",20}},
        
        {_AT_FTP_READ_1,       _fFTP_READ_1,            {(uint8_t*)"AT+CFTPRDFILE=\"",15},                  {NULL}},
        {_AT_FTP_READ_2,       _fFTP_READ_2,            {(uint8_t*)"\r",1},                                 {(uint8_t*)"+CFTPRDFILE:SUCCESS",19}},  
        
    // HTTP	
        {_AT_HTTP_OPEN,         NULL,                   {(uint8_t*)"AT$HTTPOPEN\r",12},                     {(uint8_t*)"OK",2}},   
        {_AT_HTTP_CLOSE,        NULL,                   {(uint8_t*)"AT$HTTPCLOSE\r",13},                    {(uint8_t*)"OK",2}}, 
        {_AT_HTTP_SAVE_LOCAL,   NULL,                   {(uint8_t*)"AT$HTTPTYPE=1\r",14},                   {(uint8_t*)"OK",2}}, 
        
        {_AT_HTTP_SET_URL_1,    _fHTTP_SETURL_1,        {(uint8_t*)"AT$HTTPPARA=",12},                      {NULL}},
        {_AT_HTTP_SET_URL_2,    _fHTTP_SETURL_2,        {(uint8_t*)"\r",1},                                 {(uint8_t*)"OK",2}},
 
        {_AT_HTTP_REQUEST_GET,  NULL,                   {(uint8_t*)"AT$HTTPACTION=0,1\r",18},               {(uint8_t*)"OK",2}}, 
        {_AT_HTTP_LENGTH,       _fHTTP_FIRM_LENGTH,     {(uint8_t*)"AT$HTTPREAD=1\r",14},                   {(uint8_t*)"$HTTPREAD:1",11}}, 
        
        {_AT_HTTP_READ_1,       _fHTTP_READ_1,          {(uint8_t*)"AT$HTTPREAD=0,",14},                    {NULL}},
        {_AT_HTTP_READ_2,       _fHTTP_READ_2,          {(uint8_t*)"\r",1},                                 {(uint8_t*)"\r\nOK\r\n",6}},  
        
		// UPDATE SERVER TIME
		{_CORRECT_RESPONDING, 	NULL,			        {NULL}, 											{NULL}},
        {_INCORRECT_RESPONDING, NULL,		            {NULL}, 										    {NULL}},
        
        {_END_CHECK, 		    NULL,                   {(uint8_t*)"THE END\r",8},  						{(uint8_t*)"END CHECK\r\n",11}},
		{NULL, 					NULL,			        {NULL}, 											{NULL}}
};

uint8_t 	Buff_recev_Uart1_u8[MAX_LENGTH_UART1]; // Mang chua bo dem nhan UART
uint8_t 	Buff_recev_UartSIM_u8[MAX_LENGTH_UARTSIM]; // Mang chua bo dem nhan UART
uint8_t		Buff_FTP_PATH[LENGTH_FTP] = {'T','e', 's', 't'};
uint8_t		Buff_FTP_NAME[LENGTH_FTP] = {'F', 'I', 'R', 'M', '_', 'H', 'C', 'M', 'C', 'H', 'E', 'S', '2', '.','b', 'i', 'n'};
uint8_t     firmFileName[]        = "/STM32Firm.bin";

uint8_t		sRec_TCP_DATA_Init[MAX_REC] 	= {_AT_TCP_OUT_DATAMODE, _AT_RECOUNT_FIRST, _AT_TCP_COFI_CONTEXT_2, _AT_TCP_COFI_CONTEXT_3,_AT_RECOUNT_FIRST, _AT_APN_AUTHEN_1, _AT_APN_AUTHEN_2,
                                               _AT_TCP_CLOSE, _AT_TCP_TRANS_SETUP, _AT_TCP_TRANS, _AT_TCP_NETOPEN, _AT_GET_IP_SOCKET, _AT_RECOUNT_FIRST,_AT_TCP_CONNECT,_AT_TCP_CONNECT1,_AT_TCP_CONNECT2,   0};  
                                                //
uint8_t		sRec_TCP_DATA_Close[MAX_REC] 	= {_AT_TCP_OUT_DATAMODE, _AT_TCP_CLOSE, _AT_TCP_NETCLOSE, 0}; 
uint8_t     sRec_FTP_Init[MAX_REC]          = { _AT_RECOUNT_FIRST, _AT_PING_TEST, _AT_PING_TEST_2, _FTP_PORT, _AT_RECOUNT_FIRST,_FTP_SERVER, _FTP_SERVER_2, _FTP_USERNAME,_FTP_PASSWORD,
                                                _FTP_TYPE, _AT_RECOUNT_FIRST, _AT_FTP_GET_FILE1, _AT_FTP_GET_FILE2, _AT_FTP_GET_FILE, _AT_SYS_LIST_FILE, 0}; 
uint8_t		sRec_TCP_Send_Ping[4]           = {_AT_RECOUNT_FIRST, _AT_PING_TCP, _AT_PING_TCP_2, 0};
uint8_t 	sRec_AT_CONNECT[MAX_REC] 		= {_AT_TCP_OUT_DATAMODE, _AT_RECOUNT_FIRST, _AT_TCP_COFI_CONTEXT_2, _AT_TCP_COFI_CONTEXT_3,_AT_RECOUNT_FIRST, _AT_APN_AUTHEN_1, _AT_APN_AUTHEN_2,
                                                _AT_TCP_TRANS_SETUP, _AT_TCP_TRANS, _AT_TCP_NETOPEN, _AT_GET_IP_SOCKET, _AT_RECOUNT_FIRST, _AT_TCP_CONNECT, _AT_TCP_CONNECT1,_AT_TCP_CONNECT2, 0};

uint8_t 	sRec_INIT_APN[MAX_REC] 	        = {_AT_RECOUNT_FIRST, _AT_TCP_COFI_CONTEXT_2, _AT_TCP_COFI_CONTEXT_3,_AT_RECOUNT_FIRST, _AT_APN_AUTHEN_1, _AT_APN_AUTHEN_2, 
                                                _AT_TEST_0, _AT_GET_IP_APN };                                          
uint8_t 	sRec_HTTP_INIT[MAX_REC] 	    = { _AT_RECOUNT_FIRST, _AT_PING_TEST, _AT_PING_TEST_2,_AT_HTTP_CLOSE,  _AT_HTTP_OPEN, _AT_RECOUNT_FIRST, _AT_HTTP_SET_URL_1, _AT_HTTP_SET_URL_2,
                                               _AT_HTTP_SAVE_LOCAL, _AT_HTTP_REQUEST_GET, _AT_HTTP_LENGTH, 0}; // 
// _AT_TEST_0, _AT_ACTIVE_APN, 
uint8_t 	sRec_HTTP_ReadData[MAX_REC]     = {_AT_RECOUNT_FIRST, _AT_HTTP_READ_1, _AT_HTTP_READ_2, 0};
uint8_t 	sRec_FTP_ReadData_2[MAX_REC]    = {_AT_RECOUNT_FIRST, _AT_FTP_READ_1, _AT_FTP_READ_2, 0};

/*
 *			CAU HINH BAN TIN TRUYEN MQTT
 */

Information_Control_struct sInformation = {{NULL},{NULL},{NULL},{NULL},{NULL}};

//========================RCT&ADC===============================
RTC_TimeTypeDef 	sRTCTime;
RTC_DateTypeDef 	sRTCDate;
uint8_t 			Erase_Flash = 0;
uint8_t 			Min_Allow_Send_DATA_u8 = 0;
truct_String        Str_Infor_Meter;


//Read Meter

UART1_Control_Struct				UART1_Control;
uint8_t								UART1_Receive_Buff[MAX_LENGTH_BUFF_NHAN];
uint8_t						        aUART1_Receive_End[MAX_LENGTH_BUFF_NHAN];

//--------------------Khai bao them cai queue trong Readmeter
Meter_Flash_Queue_Struct		Queue_Meter_Info,*ptrQueue_Meter_Info;
Meter_Flash_Queue_Struct		Queue_Meter_Alert,*ptrQueue_Meter_Alert;
Meter_Flash_Queue_Struct		Queue_Meter_Billing,*ptrQueue_Meter_Billing;
Meter_Flash_Queue_Struct		Queue_Meter_Event,*ptrQueue_Meter_Event;
Meter_Flash_Queue_Struct		Queue_Meter_LProf,*ptrQueue_Meter_LProf;
Meter_Flash_Queue_Struct		Queue_Flash_MInfo,*ptrQueue_Flash_MInfo;
Meter_Flash_Queue_Struct		Queue_Flash_MInfo_2,*ptrQueue_Flash_MInfo_2;
Meter_Flash_Queue_Struct		Queue_Flash_MLpf,*ptrQueue_Flash_MLpf;
Meter_Flash_Queue_Struct		Queue_Flash_MBill,*ptrQueue_Flash_MBill;
Meter_Flash_Queue_Struct		Queue_Flash_MBill_2,*ptrQueue_Flash_MBill_2;

Meter_Flash_Queue_Struct		Queue_Meter_AddInfor,*ptrQueue_Met_Addinfor;
Meter_Flash_Queue_Struct		Queue_Flash_ReadLog,*ptrQueue_Flash_ReadLog;
Meter_Flash_Queue_Struct		Queue_Flash_DataLog,*ptrQueue_Flash_DataLog;


uint8_t		ReadLogBuff[MAX_LOG_MESS];
uint8_t		MeterInfoDataBuff[MBillingBuffLength];
uint8_t		MeterBillingDataBuff[MInfoBuffLength];
uint8_t		MeterBillingDataBuff_2[MBillingBuffLength];
uint8_t		MeterEventDataBuff[MEventBuffLength];
uint8_t		MeterEventDataBuff_2[MEventBuffLength];
uint8_t		MeterLProfDataBuff[MLProfBuffLength];
uint8_t		MeterTuTiDataBuff[50];


uint8_t		ReadMeterInfoFromA[MInfoBuffLength];
uint8_t		ReadMeterLpfFromA[MInfoBuffLength];
uint8_t		ReadMeterBillFromA[MInfoBuffLength];

uint8_t		InfoMeterBuff[METER_INFOR_LENGTH];
uint8_t		ReadBackBuff[MAX_BUFF_EXFLASH];

uint8_t	    Read_Meter_LProf_Day[8] = {0};
uint8_t	    Meter_TempBuff[MAX_LENGTH_BUFF_TEMP];          //Buffer nay dang 6000 byte
uint8_t     BuffRecord[1000];    //bufff tam luu data roi moi dong goi
uint8_t     Buff_Temp1[50];     //Buff de sap xep data trong ham doc TSVH, Bill, LPF to


Struct_Maxdemand_Value      MD_Plus_Bill;
Struct_Maxdemand_Value      MD_Sub_Bill;
//start

uint8_t                     Str_Stime_bill[20];
truct_String                StrSTime_Bill = {&Str_Stime_bill[0], 0};

uint32_t                    PeriodLpf_Min = 0;
truct_String                Str_event_Temp = {(uint8_t*) "(0000)", 6};
//Struct_Manage_Trace_Freertos    sTraceRTOS;

//--------------------Function-------------------------------
void Init_Queue_Struct (void)
{
    _fInit_Queue(&Queue_Flash_ReadLog);
	ptrQueue_Flash_ReadLog = &Queue_Flash_ReadLog;	

    _fInit_Queue(&Queue_Flash_DataLog);
	ptrQueue_Flash_DataLog = &Queue_Flash_DataLog;	
    	
	_fInit_Queue(&Queue_Meter_Info);
	ptrQueue_Meter_Info = &Queue_Meter_Info;

    _fInit_Queue(&Queue_Meter_Alert);
	ptrQueue_Meter_Alert = &Queue_Meter_Alert;

	_fInit_Queue(&Queue_Meter_Billing);
	ptrQueue_Meter_Billing = &Queue_Meter_Billing;

	_fInit_Queue(&Queue_Meter_Event);
	ptrQueue_Meter_Event = &Queue_Meter_Event;

	_fInit_Queue(&Queue_Meter_LProf);
	ptrQueue_Meter_LProf = &Queue_Meter_LProf;

	_fInit_Queue(&Queue_Flash_MInfo);
	ptrQueue_Flash_MInfo = &Queue_Flash_MInfo;
    
    _fInit_Queue(&Queue_Flash_MInfo_2);
	ptrQueue_Flash_MInfo_2 = &Queue_Flash_MInfo_2;
    
   _fInit_Queue(&Queue_Flash_MLpf);
	ptrQueue_Flash_MLpf = &Queue_Flash_MLpf;
    
    _fInit_Queue(&Queue_Flash_MBill);
    ptrQueue_Flash_MBill = &Queue_Flash_MBill;
    
    _fInit_Queue(&Queue_Flash_MBill_2);
    ptrQueue_Flash_MBill_2 = &Queue_Flash_MBill_2;
    
    _fInit_Queue(&Queue_Meter_AddInfor);
    ptrQueue_Met_Addinfor = &Queue_Meter_AddInfor;
}

void _fInit_Queue (Meter_Flash_Queue_Struct *qTaget)
{
    qTaget->Mess_Direct_ui8 = 0;
	qTaget->str_Flash.Length_u16 = 0;
	qTaget->Mess_Status_ui8 = 0;
	qTaget->Mess_Type_ui8 = 0;
	qTaget->str_Flash.Data_a8 = 0;
}


void Init_Manage_Flash_Struct (void)
{
    _fInit_Flash_Struct(&Manage_Flash);
	_fInit_Flash_Struct(&Manage_Flash_Lpf);
    _fInit_Flash_Struct(&Manage_Flash_Bill);
}

void _fInit_Flash_Struct (Manage_Flash_Struct *strManage)
{
    strManage->Step_ui8 = 0;
	strManage->Error_ui8 = 0;
	strManage->BuffA_Writting_S_ui16 = 0;
	strManage->BuffB_Writting_S_ui16 = 0;
	strManage->BuffA_Change_Sector_ui8 = 0;
}


void Reset_Chip (void)
{           
    __disable_irq();
    NVIC_SystemReset(); // Reset MCU
}

void f_Get_batery_Level (void)
{
    uint32_t Bat_mV = 0;
    
    sDCU.BatLevel_Per = Get_Value_ADC (&Bat_mV, ADC_CHANNEL_1, ADC_REGULAR_RANK_1);
}


void Manage_PIN_Charge (void)
{
    if(Check_Time_Out(sDCU.LandMark_Check_Bat, 5000) == 1)
    {
        f_Get_batery_Level();
//      if(HAL_GPIO_ReadPin (SIM_PRE_GPIO_Port, SIM_PRE_Pin) == 0)  sDCU.ReadPin_DetectCardSim = 1;
        sDCU.LandMark_Check_Bat = RT_Count_Systick_u32;
    }
    
    if(sDCU.BatLevel_Per < 97)
    {   
        HAL_GPIO_WritePin (CE_BQ_GPIO_Port, CE_BQ_Pin, GPIO_PIN_RESET);
        if(HAL_GPIO_ReadPin(CHG_GPIO_Port, CHG_Pin) == GPIO_PIN_RESET) 
        {
            HAL_GPIO_WritePin (CE_BQ_GPIO_Port, CE_BQ_Pin, GPIO_PIN_SET);
        }
        sDCU.LandMark_CheckCHG = RT_Count_Systick_u32;
    }else
    {
        if(HAL_GPIO_ReadPin(CHG_GPIO_Port, CHG_Pin) == GPIO_PIN_RESET)   //neu bao day PIN roi
        {
            HAL_GPIO_WritePin (CE_BQ_GPIO_Port, CE_BQ_Pin, GPIO_PIN_SET);   //Ngat sac
        }else
        {
            if(Check_Time_Out(sDCU.LandMark_CheckCHG, 18000000) == 1)  //5h          //neu chua day PIN ma sau 5h chua day> ngat sac va cho sac lai
            {   
                HAL_GPIO_WritePin (CE_BQ_GPIO_Port, CE_BQ_Pin, GPIO_PIN_SET);       //ngat sac PIn
                sDCU.LandMark_CheckDelay = RT_Count_Systick_u32;                    //1s sau moi enable chan CE lai
                sDCU.LandMark_CheckCHG += 16200000;                                  //De cho no sac khoang 30 roi check lai tiep
            }
            if(Check_Time_Out(sDCU.LandMark_CheckDelay, 1000) == 1)                 //Check enable sac lai
            {
                HAL_GPIO_WritePin (CE_BQ_GPIO_Port, CE_BQ_Pin, GPIO_PIN_RESET); 
            }
        }
    }
}


uint32_t converStringToDec(uint8_t *pData, uint8_t lenData)
{
    uint8_t index = 0;
    uint8_t tempData = 0;
    uint32_t reVal = 0;

    for (index = 0; index < lenData; index++)
    {
        if (('0' <= *(pData + index)) && (*(pData + index) <= '9'))
        {
            tempData = (*(pData + index) - 0x30);
        }
        else
        {
            tempData = 0;
        }
        if (index == 0)
            reVal = tempData;
        else
            reVal = (reVal * 10) + tempData;
    }

    return reVal;
}



void CheckStatusMeter(void)
{
	if (Read_Meter_ID_Success == 1) 
	{ 
        sDCU.Status_Meter_u8 = 2; 
		
		if (Read_Meter_ID_Change == 1) 
        {
			sDCU.Status_Meter_u8 = 1;
			Read_Meter_ID_Change = 0;
		}
	} else {
		sDCU.Status_Meter_u8 = 0;	// khong co Dong ho
	}
} 

/*
    - Muc 1 la co cable ket noi vao mach
    - Muc 0 la khong co cable ket noi
*/

uint8_t Check_HardPin_DectectMeter (void)
{
    return METER_DETECT_VAL; 
//    return 0;
}

/*
    //Them 2 byte lenth va 1 byte crc vao 1buff 
*/
uint16_t RePacket_Message (uint8_t* Buff, uint16_t length)
{
    uint16_t    i = 0;
    uint32_t	temp=0;
     
    for(i = 0; i < length; i++)
        *(Buff + length + 1 - i) = *(Buff + length - i - 1);

    length += 2;
    *(Buff)     = (uint8_t) ((length + 1) >> 8) ;
    *(Buff + 1) = (uint8_t) (length + 1) ;

    //Generate checksum byte
    for (i=0;i<length;i++)
        temp += *(Buff + i);
    
    temp = temp & 0x000000FF;

    *(Buff + length++) = (uint8_t)temp; 
    
    return length;
}



uint16_t ConvertHex_2StringHEX (uint8_t* Buff, uint16_t Length, uint8_t* aTaget, uint16_t PosTaget)
{
    uint16_t i = 0;
    uint8_t Temp = 0;
    uint16_t Pos = PosTaget;
    
    for( i = 0; i < Length; i++)
    {
        //4 bit dau
        Temp = (*(Buff + i) >> 4) & 0x0F;
        if(Temp <= 9)
          Temp += 0x30;
        else Temp += 0x37;
        *(aTaget + Pos) = Temp;
        Pos++;
        //4 bit sau
        Temp = *(Buff + i) & 0x0F;
        if(Temp <= 9)
          Temp += 0x30;
        else Temp += 0x37;
        *(aTaget + Pos) = Temp; 
        Pos++;
    }
    return Pos;
}


void ConvertHexDatatoStringDec (int64_t Data, truct_String* StrDec)
{
    uint8_t  PDu = 0;
    uint16_t i = 0;
    uint8_t Temp = 0;
    uint8_t Sign = 0;    //1 la so am. 0 la so duong
    
    Reset_Buff(StrDec);
    //neu la so am. Se danh dau '-' truoc va sau do l� giai tri duong.
    if(Data < 0)
    {
        Sign = 1;
        Data = 0-Data;
    }else Sign = 0;
    
    while (Data != 0)
    {
        PDu = (uint8_t) (Data%10);
        Data = (Data/10);
        *(StrDec->Data_a8 + StrDec->Length_u16++) = PDu + 0x30;
    }
    //them dau '- ' o cuoi
    if(Sign == 1)
        *(StrDec->Data_a8 + StrDec->Length_u16++) = '-';
    
    //�ao nguoc lai
    for(i = 0; i <(StrDec->Length_u16/2); i++)
    {
        Temp = *(StrDec->Data_a8 + StrDec->Length_u16 - i - 1);
        *(StrDec->Data_a8 + StrDec->Length_u16 - i - 1) = *(StrDec->Data_a8 + i);
        *(StrDec->Data_a8 + i) = Temp;
    }
}



void _Off_fprint_Debug (UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{

}


void SystemClock_Config_HSI_LSI(void)
{
      RCC_OscInitTypeDef RCC_OscInitStruct = {0};
      RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
      RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

      /** Initializes the RCC Oscillators according to the specified parameters
      * in the RCC_OscInitTypeDef structure.
      */
      RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
      RCC_OscInitStruct.HSIState = RCC_HSI_ON;
      RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
      RCC_OscInitStruct.LSIState = RCC_LSI_ON;
      RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
      RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
      RCC_OscInitStruct.PLL.PLLM = 2;
      RCC_OscInitStruct.PLL.PLLN = 12;
      RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
      RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
      RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
      if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
      {
        Error_Handler();
      }
      /** Initializes the CPU, AHB and APB buses clocks
      */
      RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                  |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
      RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
      RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
      RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
      RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

      if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
      {
        Error_Handler();
      }
      PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USART1
                                  |RCC_PERIPHCLK_USART3|RCC_PERIPHCLK_UART4
                                  |RCC_PERIPHCLK_ADC;
      PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
      PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
      PeriphClkInit.Uart4ClockSelection = RCC_UART4CLKSOURCE_PCLK1;
      PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_PLLSAI1;
      PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
      PeriphClkInit.PLLSAI1.PLLSAI1Source = RCC_PLLSOURCE_HSI;
      PeriphClkInit.PLLSAI1.PLLSAI1M = 2;
      PeriphClkInit.PLLSAI1.PLLSAI1N = 16;
      PeriphClkInit.PLLSAI1.PLLSAI1P = RCC_PLLP_DIV7;
      PeriphClkInit.PLLSAI1.PLLSAI1Q = RCC_PLLQ_DIV2;
      PeriphClkInit.PLLSAI1.PLLSAI1R = RCC_PLLR_DIV2;
      PeriphClkInit.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_ADC1CLK;
      if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
      {
        Error_Handler();
      }
      /** Configure the main internal regulator output voltage
      */
      if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
      {
        Error_Handler();
      }
}



void SystemClock_Config_OFF_LSE(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI
                              |RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.LSEState = RCC_LSE_OFF;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 2;
  RCC_OscInitStruct.PLL.PLLN = 12;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}


extern void SystemClock_Config(void);

void Check_sRTC_Active (void)
{
    if(Check_Time_Out (sDCU.LandMark_ChecksRTC, 5000) == 1)
    {
        if(sDCU.sRTC_Sec_Old == sRTC.sec)
        {
            //OFF LSE khoi tao LSI
            SystemClock_Config_OFF_LSE();
            _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nLoi LSE\r\n", 11, 1000);
            osDelay(2000);
            SystemClock_Config_HSI_LSI();
            ReInit_RTC_LSI();
            // OFF_LSE -> initRTC dung LSE
//            SystemClock_Config_OFF_LSE();
//            _fPrint_Via_Debug(&UART_SERIAL, (uint8_t*) "\r\nLoi LSE\r\n", 11, 1000);
//            osDelay(2000);
//            SystemClock_Config();
//            ReInit_RTC_LSI();
        }
        sDCU.sRTC_Sec_Old = sRTC.sec;
        sDCU.LandMark_ChecksRTC = RT_Count_Systick_u32;
    }
}


void ReInit_RTC_LSI (void)
{
    hrtc.Instance = RTC;

    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv = 127;               //32K = 250 * 128
    hrtc.Init.SynchPrediv = 249;
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
    hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
    if (HAL_RTC_Init(&hrtc) != HAL_OK)
    {
        Error_Handler();
    }
}


Meter_Flash_Queue_Struct        sQueueMessErrorFlash;
Meter_Flash_Queue_Struct        *ptrQueueMessErrorFlash;
//void Forward_Queue_To_Queue (osMessageQId qTaget, Meter_Flash_Queue_Struct *ptrsQSource)
//{
//    ptrQueueMessErrorFlash = &sQueueMessErrorFlash;
//
//    sQueueMessErrorFlash.Mess_Type_ui8      = ptrsQSource->Mess_Type_ui8;
//    sQueueMessErrorFlash.Mess_Status_ui8    = ptrsQSource->Mess_Status_ui8;
//    sQueueMessErrorFlash.Mess_Direct_ui8    = ptrsQSource->Mess_Direct_ui8;
//    sQueueMessErrorFlash.str_Flash.Data_a8  = ptrsQSource->str_Flash.Data_a8 + 2;
//    sQueueMessErrorFlash.str_Flash.Length_u16 = ptrsQSource->str_Flash.Length_u16 - 3;
//    sQueueMessErrorFlash.WaitACK              = NONE_ACK;
//
//    xQueueSend(qTaget, (void *)&ptrQueueMessErrorFlash, 1000);
//}
