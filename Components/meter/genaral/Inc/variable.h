#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"

#include "stdint.h"
#include "stdbool.h"
#include "usart.h"


#include "My_LogDCU.h"
#include "t_mqtt.h"
#include "usart.h"



#ifndef __VARIABLE_H
#define __VARIABLE_H

/*======================== FreeRTOS ======================*/
#define PERIOD_UART_SIM	    2		// bAUD sim 19200 bps
#define UART_SERIAL         huart1
#define UART_METER          huart4
#define UART__METER         UART4
#define UART_SIM            huart3

#define SIZE_OF_FIRMWARE          14

//  				SYSTEM
extern uint32_t     RT_Count_Systick_u32;
#define BUFF_LENGTH_SETUP	    30
#define BUFF_LENGTH_AT_SEND	    256

extern uint8_t Device_Name_Buff[BUFF_LENGTH_SETUP];
extern uint8_t Device_IP[BUFF_LENGTH_SETUP];
extern uint8_t Ping_IP[BUFF_LENGTH_SETUP];
extern uint8_t UART_Config[BUFF_LENGTH_SETUP];
extern uint8_t APN[BUFF_LENGTH_SETUP];
extern uint8_t APN_Dial_String[BUFF_LENGTH_SETUP];
extern uint8_t APN_Username[BUFF_LENGTH_SETUP];
extern uint8_t APN_Password[BUFF_LENGTH_SETUP];
extern uint8_t PasswordModerm[BUFF_LENGTH_SETUP];
extern uint8_t aATBuffSendData[BUFF_LENGTH_AT_SEND];
extern uint8_t aATBuffReceData[BUFF_LENGTH_AT_SEND];
extern uint16_t ATSendDataLength_u8;
extern uint16_t ATReceDataLength_u8;
extern uint8_t aBuffHansdshake[100]; //  Chua data ban tin bat tay voi Server

//--------- Status UART --------------------------
#define 	MAX_LENGTH_UART1 		300
#define 	MAX_LENGTH_UARTSIM 		1500 	//use to Send and receiver MQTT Data
#define 	MAX_LENGTH_LOGDCU		256		// ghi log DCU
#define 	LENGTH_FTP				20
#define 	MAX_LENGTH_PACKFIRM		500

#define 	NONE			        0
#define 	TIME_FACTORY			15000		    // THOI GIAN CHO PHEP CAI DAT gtri mac dinh cho DCU
#define 	TIME_RESET_MCU 				5	        // TIME_RESET_MCU > TIME_RETRY_CHANCE_SERVER
#define 	TIME_RETRY_CHANCE_SERVER 	2
#define 	TIME_RETRY					2
#define 	TIME_DELAY_NORMAL		5000
#define 	TIME_DELAY_LONG			10000
#define 	TIME_DELAY_METER		30000
#define 	TIME_DELAY_FTP			180000
#define     TIME_DELAY_MQTT         60000     //20000
#define     TIME_DELAY_HTTP         60000   
#define		TOTAL_SLOT				25
#define		POS_MS_IMPORTANT		2
#define		POS_MS_DATA				12
#define		POS_MS_LOG				13
#define		POS_MS_PING				14
#define		MAX_RETRY_SEND_TCP	    0x02      //0xFF     //0x02
#define		TIME_DELAY_TCP  	    20000     //day sua lai sau
#define 	CALL_HANDLE				1         // 0: Ignore, 1: Reset
#define     MAX_TIME_TSVH_SENDTO_QUEUE   20000
#define		NOMAL_ACK				1
#define		NONE_ACK				0
#define		ONCE_ACK				2

// Harware Detect
#define METER_DETECT_PORT	        TP1_GPIO_Port
#define HARD_DETECT_PORT	        TP2_GPIO_Port

#define METER_DETECT_PIN	        TP1_Pin
#define HARD_DETECT_PIN	            TP2_Pin

#define METER_DETECT_VAL	        HAL_GPIO_ReadPin(METER_DETECT_PORT, METER_DETECT_PIN)
#define HARD_DETECT_VAL	            HAL_GPIO_ReadPin(HARD_DETECT_PORT, HARD_DETECT_PIN)

/* Configure GPIO                                                             */

// ********** ON - OFF POWER
#define SIM_PW_PORT1   ON_OFF_SIM_GPIO_Port
#define SIM_PW_PIN1    ON_OFF_SIM_Pin
#define SIM_PW_OFF1    HAL_GPIO_WritePin(SIM_PW_PORT1,SIM_PW_PIN1,GPIO_PIN_RESET)
#define SIM_PW_ON1     HAL_GPIO_WritePin(SIM_PW_PORT1,SIM_PW_PIN1,GPIO_PIN_SET)

// ********** PWKEY
#define SIM_PWKEY_PORT1  PWRKEY_GPIO_Port 
#define SIM_PWKEY_PIN1   PWRKEY_Pin
#define SIM_PWKEY_ON1    HAL_GPIO_WritePin(SIM_PWKEY_PORT1,SIM_PWKEY_PIN1,GPIO_PIN_SET)
#define SIM_PWKEY_OFF1   HAL_GPIO_WritePin(SIM_PWKEY_PORT1,SIM_PWKEY_PIN1,GPIO_PIN_RESET)

#define RS485_PORT 		NET485IO_GPIO_Port
#define RS485_PIN 		NET485IO_Pin
#define RS485_SEND		HAL_GPIO_WritePin(RS485_PORT,RS485_PIN,GPIO_PIN_SET)
#define RS485_RECIEVE	HAL_GPIO_WritePin(RS485_PORT,RS485_PIN,GPIO_PIN_RESET)

// ********** ON - OFF DTR  
#define SIM_DTR_OFF1    	HAL_GPIO_WritePin(SIM_DTR_GPIO_Port,SIM_DTR_Pin,GPIO_PIN_RESET)
#define SIM_DTR_ON1     	HAL_GPIO_WritePin(SIM_DTR_GPIO_Port,SIM_DTR_Pin,GPIO_PIN_SET)
/* */
//=============================LED===============================
#define LED_RED_port		PB15_GPIO_Port
#define LED_RED_pin			PB15_Pin //  13
#define LED_WHITE_port		PB13_GPIO_Port
#define LED_WHITE_pin		PB13_Pin // Sim
#define LED_BLUE_port		PB14_GPIO_Port
#define LED_BLUE_pin		PB14_Pin  // Server
#define LED_YELLOW_port		PB12_GPIO_Port
#define LED_YELLOW_pin		PB12_Pin  // Staus

#define	LED_ON				GPIO_PIN_RESET
#define	LED_OFF				GPIO_PIN_SET
/*------------------------------------------
						UART 1
------------------------------------------*/
#define TRUE 	1
#define FALSE 	0


#define _MUTEX_WAITED      1
#define _MUTEX_RELEASED    2

//Buff Readmeter
   
#define 					MInfoBuffLength				1280
#define 					MBillingBuffLength			2000
#define 					MBillingBuffFLashLength		1280	
#define 					MEventBuffLength			1280
#define 					MLProfBuffLength			1280
#define 					MAX_BUFF_EXFLASH			1280
#define 					MAX_LENGTH_BUFF_TEMP		6000

#define 					MAX_LOG_MESS            	256
#define 					METER_INFOR_LENGTH		    256
#define                     MAX_LENGTH_BUFF_NHAN		1200


#define                     MAX_BYTE_IN_PACKET  		1200

#define DEBUG_MODE
#if defined(DEBUG_MODE)
    #define _fPrint_Via_Debug(...)  HAL_UART_Transmit(__VA_ARGS__)
#else
    #define _fPrint_Via_Debug(...)
#endif

#define DEBUG_AT_SIM		1   //1: disable 0 la able


//----------------------------Struct, var------------------------------------
// RTC
typedef struct {
	uint8_t hour;  // read hour
	uint8_t min;   // read minute
	uint8_t sec;   // read second
    uint8_t day;   // read day
    uint8_t date;  // read date
    uint8_t month; // read month
    uint8_t year;  // read year
    
    uint32_t SubSeconds;
} ST_TIME_FORMAT;

extern ST_TIME_FORMAT   sRTC;

typedef struct {
	uint8_t			*Data_a8;					// string
	uint16_t		Length_u16;
} truct_String;



typedef struct Meter_Flash_Queue_Struct
{
 	uint8_t			Mess_Type_ui8;			// 0 - Operation; 1 - Alarm; 2 - Billing; 3 - Even
 	uint8_t			Mess_Direct_ui8;		// 0-Read	1-Write
 	truct_String 	str_Flash;
 	uint8_t			Mess_Status_ui8;		// 0 : don't RUN ; 1 : Success; 2 : Error
    uint8_t         WaitACK;                // 5/7/21
}Meter_Flash_Queue_Struct;
 
typedef struct 
{	
    int				            aNeed_Send[TOTAL_SLOT];
    Meter_Flash_Queue_Struct    *paNeed_Send[TOTAL_SLOT];	// Mang chua dia chi Status cua cac ban tin Send trong mang aNeed_Send
    
    uint8_t			SendOk_u8;				        // TRUE : send Thanh cong
    uint8_t         KindFB;
    uint16_t        Count_Send_u8;
    uint32_t		Landmark_Send_Mess;	

    uint8_t 		Min_Allow_Send_DATA_u8;
    uint16_t		Compare_Allow_Send_DATA_u16;		// 3 so cuoi DCU_ID / 2 / 60
    uint32_t		Compare_Allow_Send_LPF_u16;
    uint32_t		Landmark_Allow_Send_DATA_u32;		// Moc thoi gian cho phep truyen Data Publish len server - 3 so cuoi DCU
    uint32_t	    Landmark_Allow_Send_LPF_u32;
    uint8_t         _fRequest_MetData_byAT;
    uint8_t         _fRequest_LPF_byAT;
    
    uint8_t         aCountRetryMess[TOTAL_SLOT];
} struct_TCP;

typedef struct {
	uint8_t 	Kind_Data_u8;	// 1 : SV command; 2 : AT command; 4 : MQTT DATA
	uint8_t 	Pending;			// 0 : can receiver DATA
	uint8_t		Success;			// 0 : none; 1 : Success; 2 : Error	; 4 : wait;
	uint8_t		OverStack;		// 0 : none; 1 : Overstack
	uint16_t	Time_Out_16u;	// ms - time wait process.
	uint32_t	Systick_Last_Recerver_u32;
	truct_String str_Receiv;
} struct_Uart_Data;

//-------------------------UART1--------------------------
typedef struct UART1_Control_Struct
{
	truct_String	UART1_Str_Recei;
	uint8_t			Mess_Pending_ui8;			//0x00-no mess			0x02-have mess cho nhan BBC
	uint8_t 		Flag_Have_0x02;
	uint8_t			Mode_ui8;					//0-Data	1-Handsheck
    uint32_t	    Systick_Last_Recerver_u32;
    
    uint16_t 	    Mess_Length_ui16;
	uint8_t		    Mess_Status_ui8;			//0x00-no mess			0x01-not check		0x02-checked-ok			0x04-check-fail
    uint8_t         fRecei_Respond;
    uint8_t         fETX_EOT;
}UART1_Control_Struct;

typedef struct {
	uint8_t		Check_Config;			// 0 : None; 1 : Config DCU; 8 : Read_Config; 9 : Read_Flash; 0xF0 : Config Done (ko chay Check Flash Full)
	uint32_t 	Landmark_Check_Setup; 	// Moc time -> check : Flash, config DCU
} struct_Check_Factory;


typedef enum
{
	_RESET_SIM900 = 0,		// PUSH COMMENT RESET
	_RES_ALREADY_CONNECT,
	_RES_SIM_LOST,
    _RES_SIM_REMOVE,
	_RES_CLOSED,		// responding from SIM900 : CLOSED connect
	_RES_PDP_DEACT,		// responding PDP deact : expired connect to server
	_RES_CALL_READY,
	_RES_ERROR,
	_FTP_GET_PENDING,
	_FTP_GET_DONE,
	_AT_CHECK_AT,
	_AT_SIM_ID, // 12
    //pre start
    _AT_RESET_MODULE,
	_AT_BAUD_RATE,
	_AT_NDISPLAY_CMD,
	_AT_DISPLAY_CMD, // 14
    _AT_GET_IMEI,
    _AT_SET_FUN,
    //start state
    _AT_TCP_COFI_CONTEXT_2,
    _AT_TCP_COFI_CONTEXT_3,
    
    _AT_APN_AUTHEN_1,
    _AT_APN_AUTHEN_2,
    
    _AT_TCP_NETOPEN,
    _AT_TCP_NETCLOSE,
    
	_AT_TCP_OUT_DATAMODE,
	_AT_TCP_TRANS,
    _AT_TCP_TRANS_SETUP,
	_AT_TCP_IN_DATAMODE,
    
    _AT_SYN_TIME_ZONE,
    _AT_GET_RTC,
    
	_AT_CHECK_SIM,		// CPIN?
	_AT_CHECK_RSSI,		// CSQ=?

	_AT_SAVE_CMD,
    _AT_TEST_0,
    _AT_GET_IP_APN,
    _AT_ACTIVE_APN,
    
    _AT_ACCESS_RA_AUTO,
    _AT_ACCESS_RA_2G,
    _AT_ACCESS_RA_3G,
    _AT_ACCESS_RA_4G,
      
    _AT_CHECK_ACCESS,
    _AT_CHECK_BAND,
    _AT_CHECK_ATTACH,
    
	_AT_RECOUNT_FIRST,
	_AT_TCP_CONNECT,
	_AT_TCP_CONNECT1,
	_AT_TCP_CONNECT2,  // 38
    
    _AT_PING_TCP,
    _AT_PING_TCP_2,
    _AT_GET_IP_SOCKET,
    
    _AT_PING_TEST,
    _AT_PING_TEST_2,
    _AT_MAN_LOG,
      
	_AT_CHECK_CREG,		// CREG?
	_AT_CHECK_TCP_STATUS,// CIPSTATUS  // 45
    _AT_TCP_SEND,
	_AT_TCP_CLOSE,

    _AT_SYS_DEL_FILE_1,
    _AT_SYS_DEL_FILE_2,
    _AT_SYS_LIST_FILE,
    
	_FTP_SERVER,
    _FTP_SERVER_2,
    _FTP_PORT,
    
	_FTP_USERNAME,
	_FTP_PASSWORD,
    _FTP_TYPE,
    _FTP_LIST_FILE,
    _FTP_GET_FILE_1,

    _AT_FTP_GET_FILE1,
    _AT_FTP_GET_FILE2,
    _AT_FTP_GET_FILE,

    _AT_FTP_READ_1,
    _AT_FTP_READ_2,
    
    _AT_HTTP_OPEN,
    _AT_HTTP_CLOSE,
    _AT_HTTP_SAVE_LOCAL,
    
    _AT_HTTP_SET_URL_1,
    _AT_HTTP_SET_URL_2,

    _AT_HTTP_REQUEST_GET,
    _AT_HTTP_LENGTH,

    _AT_HTTP_READ_1,
    _AT_HTTP_READ_2,
    
	_CORRECT_RESPONDING,
    _INCORRECT_RESPONDING,

    _ATSEND_END_DATA_OK,
    _END_CHECK,
}Type_Command_GSM;

typedef enum {
	GSM_SHUT_DOWN,
	GSM_HARD_RESET,
	GSM_PRE_SETUP,
	GSM_SOFT_RESET,
	GSM_STATE_STARTUP,
	GSM_STATE_SMS,
	GSM_STATE_SMS_DONE,
	GSM_STATE_INIT_GPRS,
	GSM_STATE_TCP_SEND_CONNECT,
	GSM_STATE_INIT_MQTT,
	GSM_STATE_MQTT_CONNECTED,
    GSM_CONFIG_RADIO,
	GSM_FTP_TRANSFER,
	GSM_FTP_TRANSFER_DOWNLOAD,
	GSM_FTP_TRANSFER_DONE,
    GSM_HTTP_TRANSFER,
    GSM_HTTP_TRANSFER_DOWNLOAD,
	GSM_CHECK_ENVIROMENT,
	GSM_CHECK_CELLID,
	GSM_CLOSING,
	GSM_MEAS_RSSI,
	GSM_CALL_HANDLE,
	GSM_ATCOMMAND,
} Type_GSM_STATE;


typedef enum {
    CONNECT_METER,
    DISCONECT_METER,
	POW_ON_MODERM,
	POW_OFF_MODERM,
	POW_UP_METER,
    POW_DOWN_METER,
}Type_Alarm;    
/*
 *			CAU HINH BAN TIN TRUYEN MQTT
 */

#define 	INIT_SETUP 		0x01
#define 	INIT_CONNECT 	0x02
#define 	MQTT_CONNECT 	0x04
#define 	TIME_CONNECT 	0x10
#define 	FTP_CONNECT 	0x80


typedef struct 
{
	uint16_t			Count_Hard_Reset;			// Dem so Lan softReset lai;
	uint16_t			Count_Soft_Reset;			// Dem so Lan softReset lai;
	uint32_t 			Timeout_SIMReset;
	uint16_t			Count_Init_GPRS;
	uint16_t			Count_Startup_GPRS;

	uint8_t 			Call_Ready_u8;				// bao co song
	uint8_t 			Sim_inserted_u8;			// Bao co sim
	uint8_t				SMS_index;

	uint8_t				GPRS_u8;					// 0 : Ko co GPRS; 1 : TCP; 2 : FTP
	int8_t				RSSI_c8;
	int8_t				Ber_c8;
	int8_t				temp_i8;					// Nhiet do
	
	uint8_t				Step_Control_u8;			// count step to control
	uint8_t				Mode_Tranfer_u8;			// = 	1 - tranparent Mode
													// 		0 - non tranparent
	uint32_t			Systick_LandMarkReset_u32; 	// Moc thoi gian Landmark reset

	int 				No_Process_i16;				// Dem so buoc khoi tao.
	int 				No_ResProcess_i16;			// So Buoc feedback lai

	truct_String 		sCell_ID;
	uint8_t				CheckConnectATCommand;
 
    uint8_t			    ServerChange_u8;
    uint8_t			    Status_Connect_u8;
    uint32_t			systick_CheckConnectATcommand;
    uint8_t			    Cmd_Uart_u8;
    uint32_t			TimeoutCancelCmdUart_u32;
    uint8_t			    Request_Meter;
    uint8_t             Type_485;
    uint32_t			TimeoutSendHeartBeat_u32;
    uint8_t             Init_Uart_u8;
    uint8_t             Baudrate_Uart_u8;
    uint8_t             Flag_Get_sTime;
    uint8_t             Radio_ConfigAT;
    uint8_t             Radio_4G_GSM;
    uint8_t             FRequestAT_ByServer;
    uint8_t             Falg_SetNewRadio;
    
    uint8_t             Flag_WaitReadMet;
    uint32_t            LandMarkWait_ReadMet;
    uint8_t             TypeWait;
    uint8_t             FlagSet_NewAPN;
    truct_String        sAT_CMD_Request;
    truct_String        StrCheck_AT_Req;
    uint8_t             fRequest_AT_cmd;
    uint8_t             fERROR_AT_Cmd;
    uint8_t             factiveAPN;
} truct_SIM900_status;

typedef struct {
	uint32_t			FirmWare_Length;
	truct_String		strFirmWare_Length;			// string Length receive from SIM module
	uint8_t				Receiver_u8;				// = 1 : active Mode FTP
	uint8_t				Update_success;				// Bo dem Uart FTP
	uint8_t				Data_buff_u8;				// Bo dem Uart FTP
	uint32_t			Add_buff;
	uint8_t				Pending_u8;				// Doi tinh toan uart FTP
	uint32_t			Count_Data_u32;
	uint32_t			Systick_LandMarkFTP_u32; 		// Moc thoi gian Landmark reset
	uint32_t			Systick_LandMarkFTP_new_u32;	// Moc nhan dc UART DATA - check out FTP mode
	uint32_t			Min_Period_Recei_u32;
	uint32_t			Max_Period_Recei_u32;
    //
    uint32_t            Offset;
    uint8_t             IndexTemp;
    uint16_t            Count_PacketFirm_ExFlash;
    uint8_t             LastCrcFile;
    uint8_t             LasCrcCal;
    uint8_t             HeaderSV_OK;
}struct_FTP;



typedef void (*task_callback_f)(truct_String *str_Receiv);
typedef struct {
	int 			idStep;
	task_callback_f	CallBack;
	truct_String	sSender;					// Send -> Sim900
	truct_String	sTempReceiver;				// temp Receive <- Sim900
} struct_CheckList_GSM;

extern const struct_CheckList_GSM CheckList_GSM[];

#define 	MODE_CONNECT_DATA		2
#define 	MODE_CONNECT_FTP		3
#define     MODE_CONNECT_HTTP       4

#define 	UART_WAIT_LONG				250
#define 	UART_WAIT_NORMAL			20

typedef struct {
	uint8_t			Connect_Mode;           // 1 - luon ket noi den dich vu, 2 - ket noi va ngat ket noi tu dong
	int 			Kind_DCU_i32;			// Loai DCU -> Header MQTT
	uint8_t			Mode_Connect_Now;		// Che do ket noi cua SIM900 : DATA, DATA2, FTP
	uint8_t			Mode_Connect_Future;
	truct_String  	sReceipt;				// Step to control follow sSim900_status.Step_Control_u8	
	uint16_t		Keep_Check_UART_i16;	// Wait check port UART

	truct_String  	sDCU_id;
    truct_String  	sDCU_Seri;
	truct_String  	sSIM_id;
	truct_String  	sMeter_id_now;			// Meter ID hien tai do co the co nhieu meter.
    truct_String  	sMeter_id_Read;
    uint8_t         MeterType;
    uint8_t         LastMeterType;
	truct_String 	sPayload;

//  Subcribe
	uint8_t			Status_Meter_u8;		// 1 :  thay doi Meter; 0 : not found Meter; 2 : detected Meter; 3 : Mat ket noi Meter; 4 : Read meter error; 8 : Checking
	uint8_t			Pos_Error_Meter_u8;		// Vi tri ban tin doc loi Meter;
	uint32_t 		LandMark_Count_Wait_Find_Meter;
	uint32_t 		LandMark_Count_Reset_Find_Meter;
    
    uint8_t         Flag_AlowSendTCP;
    truct_String  	Str_IP_Module;
    uint8_t         Init_ID_Flash;
    uint8_t         Power_Status_u8;
    uint8_t         Power_Event_u8;       //0 kh�ng c� su kien  ; 1: su kien mat dien;  2: Su kien c� dien tro lai
    uint8_t         Flag_Get_sTime_OK;

    uint16_t        He_So_Nhan;
    uint8_t         Flag_ConnectNewServer;
    
    truct_String    StrPing_Recei;
    uint8_t         Fl_Finish_Ping;
    uint32_t        Freq_Send_Heartb_u32;
    uint16_t        BatLevel_Per;
    uint32_t        LandMark_CheckCHG;
    uint32_t        LandMark_Check_Bat; 
    uint32_t        LandMark_CheckDelay;
      
    uint8_t         Flag_ReadEventSVH;
    
    uint8_t         FlagHave_BillMes;
    uint8_t         FlagHave_EventMess;
    uint8_t         FlagHave_ProfMess;
    uint8_t         ReadPin_DetectCardSim;
    uint8_t         Fl_Scan_Meter;
    
    uint8_t         fRecei_StrAT_OK;
    uint32_t        LandMark_Check_PowMeter;
    uint8_t         fPowOn_Meter;
    uint32_t        LandMark_ControlLed;
    uint32_t        LandMark_ControlLed_2;
    
    uint32_t        LandMark_ControlLed_Status;
    uint8_t         fLog_DCU_Intan;
    uint32_t        LandMark_ChecksRTC;
    uint8_t         sRTC_Sec_Old;
    uint8_t         TypeMessTSVH;           //0 la ban tin khong cat ra. 1 la ban tin goi 1 khi cat;  2 la ban tin ket thuc cut
    uint8_t         Mutex_ReadMeter_Status_u8;   //0: Dang wait  ; 1: Da release
} DCU_struct;


typedef struct {
	truct_String  	Meter_ID[32];
	truct_String  	sServer_MQTT;
	truct_String  	sPort_MQTT;

    truct_String  	sServer_MQTT_Request;
	truct_String  	sPort_MQTT_Request;
	
	truct_String	Device_Name;
	truct_String	Device_IP;
	truct_String	Ping_IP;
    
	truct_String	UART_Config;
	
	truct_String	APN;
	truct_String	APN_Dial_String;
	truct_String	APN_Username;
	truct_String	APN_Password;
    
    truct_String	Password_Moderm;
	
	RTC_TimeTypeDef 	ConnectTime;
	RTC_TimeTypeDef 	ConnectPeriod;
	RTC_TimeTypeDef 	DisconnectTime;
    
    ST_TIME_FORMAT  	StartTime_GetLpf;
    ST_TIME_FORMAT  	EndTime_GetLpf;
    
    uint16_t            IndexStartLpf;
    uint16_t            IndexEndLpf;
    
    uint8_t             Flag_Stop_ReadLpf;
    
    ST_TIME_FORMAT  	StartTime_GetBill;
    ST_TIME_FORMAT  	EndTime_GetBill;
    
    uint16_t            IndexStartBill;
    uint16_t            IndexEndBill;
    
    uint8_t             Flag_Stop_ReadBill;
    
    ST_TIME_FORMAT  	sTimeStart_Send_Lpf;
    ST_TIME_FORMAT  	sTimeStop_Send_Lpf;
    
    truct_String	    Str_URL_Firm_Add;
    truct_String	    Str_URL_Firm_Port;
    truct_String	    Str_URL_Firm_Path;
    truct_String	    Str_URL_Firm_Name;
    truct_String	    Str_URL_HTTP;
    
    uint8_t             Flag_Request_lpf;
    uint8_t             Flag_Request_Bill;   
    uint8_t             fUpdateFirmware; 
    
    uint8_t             fReq_RLogByServer;    //0: serial, 1: server
    uint16_t            PosReadLogDCU;
} Information_Control_struct;

extern Information_Control_struct sInformation;


typedef struct {
	uint8_t		Mess_Type_u8;	// Yeu cau doc ban tin : intantanious, Operation, Historical
	uint8_t		Mess_add_u32;	// 0 : ban tin tuc thoi; "Number" : vi tri ban tin
	uint8_t		Mess_Status_ui8; // 0 : don't RUN ; 1 : Success; 2 : Error; 3 : pending
}	Flag_Request_Queue_struct;

typedef struct {
	uint8_t 	Flag[10];	// pos 0 :Operation; 1 : Historical; 2 : Even DCU; 3 : Even Meter
							// value = 1 : Read success Message; 2 : error; 3 : pending
	uint32_t 	Landmark_sendq_Even_DCU;
	uint32_t 	Landmark_Flag[10];
} Status_Read_Meter_truct;


//-------------------------FLASH--------------------------
typedef struct Manage_Flash_Struct
{
	uint8_t			Step_ui8;
	uint8_t			Error_ui8;							//0-no error	1-write error		2-two logs equal
	uint16_t		BuffA_Writting_S_ui16;				// Meter -> Flash  : Doc meter thanh cong
	uint16_t		BuffB_Writting_S_ui16;				// Sim900 -> Flash : da day len Server thanh cong
	uint8_t			BuffA_Change_Sector_ui8;
	uint32_t 		Time_outFB_MQTT;
	uint32_t 		TimeOut_Check_Flash;
}Manage_Flash_Struct;


extern struct 		Manage_Flash_Struct				Manage_Flash;
extern struct 		Manage_Flash_Struct				Manage_Flash_Lpf;
extern struct 		Manage_Flash_Struct				Manage_Flash_Bill;
extern struct 		Manage_Flash_Struct				Manage_Flash_log;

   
typedef struct
{
    uint8_t             NumTariff;
    ST_TIME_FORMAT      sTime[9];
    uint32_t            Value_u32[9];
    uint8_t             NumValue_MD;
}Struct_Maxdemand_Value;

//typedef struct
//{  
//    uint32_t        HighSpaceStackUartTask_u32;
//    uint32_t        HighSpaceStackSimTask_u32;
//    uint32_t        HighSpaceStackFlashTask_u32;
//    uint32_t        HighSpaceStackMeterTask_u32;
//}Struct_Manage_Trace_Freertos;
//
//extern Struct_Manage_Trace_Freertos    sTraceRTOS;
extern struct_Check_Factory         sCheck_Factory;
extern truct_String                 sHandsake;
extern Struct_Maxdemand_Value       MD_Plus_Bill;
extern Struct_Maxdemand_Value       MD_Sub_Bill;
extern truct_String                 StrSTime_Bill;
extern uint32_t                     PeriodLpf_Min;
extern truct_String                 Str_event_Temp; 

/*======================== Init Variables ======================*/
extern DCU_struct 	                sDCU;
extern struct_Uart_Data	 		    Uart1_Control, UartSIM_Control; // Mang chua Data PC/SIM -> DCU
extern struct_TCP				    sTCP;
extern struct_FTP				    sFTP;
extern truct_SIM900_status 		    sSim900_status;
extern Status_Read_Meter_truct      sStatus_Meter;
extern Meter_Flash_Queue_Struct     *sFlash_Update;

extern uint8_t 	    Buff_recev_Uart1_u8[MAX_LENGTH_UART1]; // Mang chua bo dem nhan UART
extern uint8_t 	    Buff_recev_UartSIM_u8[MAX_LENGTH_UARTSIM]; // Mang chua bo dem nhan UART
extern uint8_t		Buff_FTP_PATH[LENGTH_FTP] ;
extern uint8_t		Buff_FTP_NAME[LENGTH_FTP] ;
extern uint8_t		Buff_FTP_IP[LENGTH_FTP];
extern uint8_t		Buff_FTP_Port[LENGTH_FTP];
extern uint8_t		Buff_HTTP_URL[50];
extern uint8_t		aCrC_Firmware[MAX_LENGTH_PACKFIRM];

// Record receipt to control MQTT, FTP
#define 	        MAX_REC		25
extern uint8_t		sRec_TCP_DATA_Init[MAX_REC];
extern uint8_t		sRec_TCP_DATA_Close[MAX_REC];
extern uint8_t		sRec_FTP_Init[MAX_REC];
extern uint8_t 		sRec_AT_CONNECT[MAX_REC] ;
extern uint8_t		sRec_TCP_Send_Ping[4];
extern uint8_t 	    sRec_HTTP_INIT[MAX_REC]; 
extern uint8_t 	    sRec_HTTP_ReadData[MAX_REC];
extern uint8_t 	    sRec_FTP_ReadData_2[MAX_REC]; 
extern uint8_t 	    sRec_INIT_APN[MAX_REC];

extern UART1_Control_Struct			UART1_Control;
extern uint8_t						UART1_Receive_Buff[MAX_LENGTH_BUFF_NHAN];
extern uint8_t						aUART1_Receive_End[MAX_LENGTH_BUFF_NHAN];
extern truct_String                 sFirmware_Version;
/*======================== Variables ======================*/
extern uint32_t         RT_Count_Systick_u32;
extern uint8_t 			Erase_Flash;
extern truct_String     Str_Infor_Meter;
/*======================== Functions ======================*/

//=============================RTC===============================
extern RTC_TimeTypeDef 		sRTCTime;
extern RTC_DateTypeDef 		sRTCDate;
extern uint8_t              Min_Allow_Send_DATA_u8;
/*======================== Queue Readmeter ======================*/

extern struct 		Meter_Flash_Queue_Struct		Queue_Meter_Info,*ptrQueue_Meter_Info;
extern struct 		Meter_Flash_Queue_Struct		Queue_Meter_Alert,*ptrQueue_Meter_Alert;
extern struct 		Meter_Flash_Queue_Struct		Queue_Meter_Billing,*ptrQueue_Meter_Billing;
extern struct 		Meter_Flash_Queue_Struct		Queue_Meter_Event,*ptrQueue_Meter_Event;
extern struct 		Meter_Flash_Queue_Struct		Queue_Meter_LProf,*ptrQueue_Meter_LProf;
extern struct       Meter_Flash_Queue_Struct		Queue_Meter_AddInfor,*ptrQueue_Met_Addinfor;

extern struct 		Meter_Flash_Queue_Struct		Queue_Flash_MInfo_2,*ptrQueue_Flash_MInfo_2;
extern struct 		Meter_Flash_Queue_Struct		Queue_Flash_MInfo,*ptrQueue_Flash_MInfo;
extern struct 		Meter_Flash_Queue_Struct		Queue_Flash_MLpf,*ptrQueue_Flash_MLpf;
extern struct       Meter_Flash_Queue_Struct		Queue_Flash_MBill,*ptrQueue_Flash_MBill;
extern struct       Meter_Flash_Queue_Struct		Queue_Flash_MBill_2,*ptrQueue_Flash_MBill_2;

extern struct 		Meter_Flash_Queue_Struct		Queue_Flash_ReadLog,*ptrQueue_Flash_ReadLog;
extern struct 		Meter_Flash_Queue_Struct		Queue_Flash_DataLog,*ptrQueue_Flash_DataLog;

extern uint8_t		ReadLogBuff[MAX_LOG_MESS];
extern uint8_t		MeterInfoDataBuff[MBillingBuffLength];
extern uint8_t		MeterBillingDataBuff[MInfoBuffLength]; 
extern uint8_t		MeterBillingDataBuff_2[MBillingBuffLength]; 
extern uint8_t		MeterEventDataBuff[MEventBuffLength];
extern uint8_t		MeterEventDataBuff_2[MEventBuffLength];

extern uint8_t		MeterLProfDataBuff[MLProfBuffLength];
extern uint8_t		MeterTuTiDataBuff[50];
  
extern uint8_t		InfoMeterBuff[METER_INFOR_LENGTH];

extern uint8_t		ReadMeterInfoFromA[MInfoBuffLength];
extern uint8_t		ReadMeterLpfFromA[MLProfBuffLength];
extern uint8_t		ReadMeterBillFromA[MInfoBuffLength];

extern uint8_t		ReadBackBuff[MAX_BUFF_EXFLASH];
extern uint8_t	    Read_Meter_LProf_Day[8];
extern uint8_t	    Meter_TempBuff[MAX_LENGTH_BUFF_TEMP];   //do lpf cua Landis nhieu thong so se mat nhieu byte
extern uint8_t      BuffRecord[1000];  //500
extern uint8_t      Buff_Temp1[50];
extern uint8_t      firmFileName[];



/*======================== Function ======================*/
void            Init_Queue_Struct (void);
void            _fInit_Queue (Meter_Flash_Queue_Struct *qTaget);
void            Init_Manage_Flash_Struct (void);
void            _fInit_Flash_Struct (Manage_Flash_Struct *strManage);
void            Reset_Chip (void);
void            f_Get_batery_Level (void);
uint32_t        converStringToDec(uint8_t *pData, uint8_t lenData);
void            CheckStatusMeter(void);
void            Manage_PIN_Charge (void);
uint8_t         Check_HardPin_DectectMeter (void);
uint16_t        RePacket_Message (uint8_t* Buff, uint16_t length);
uint16_t        ConvertHex_2StringHEX (uint8_t* Buff, uint16_t Length, uint8_t* aTaget, uint16_t PosTaget);
void            _Off_fprint_Debug (UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout);
void            ConvertHexDatatoStringDec (int64_t Data, truct_String* StrDec);
void            SystemClock_Config_HSI_LSI(void);
void            Check_sRTC_Active (void);
void            ReInit_RTC_LSI (void);
void            SystemClock_Config_OFF_LSE(void);
//void            Forward_Queue_To_Queue (osMessageQId qTaget, Meter_Flash_Queue_Struct *ptrsQSource);

#endif /* __VARIABLE_H */


