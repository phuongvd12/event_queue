#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"
#include "variable.h"

#ifndef __SIM900_H
#define __SIM900_H


#define FUNC_RETRY function_retry()
#define FUNC_WHILE
#define CHECK_STEP_MEAS(no_Meas, Meas) 	no_Meas
#define FUNC_SEND_DATA_FROM_UART function_Send_Data_From_Uart()
#define FUNC_CHECK_DATA_FROM_UART function_Check_Data_With_MQTT()
#define RESEND_UART_DATA	1

void    function_retry(void);
void    function_Send_Data_From_Uart(void);
void    function_Check_Data_With_MQTT(void);
/*
 * define : CHECK FTP SERVER
 */
//#define FTP_CONNECT_IMMEDIATELY
#ifdef FTP_CONNECT_IMMEDIATELY
#define FTP_MODE_IMMEDIATELY 	2

#else
#define FTP_MODE_IMMEDIATELY 	0
#endif

#define CHECK_FTP_TCP(A,B,C)   ((FTP_MODE_IMMEDIATELY > 1) ? (A = B) : (A = C))

/*----------------------------------------------------------------------------*/

#define SIM900_MAXLENGTH_LOG				512

#define DCU_ID_LENGTH			30
#define SIM_ID_LENGTH			21
#define METER_LENGTH			15
#define DUTY_CHECK_FLASH    	5000
#define MAX_RETRY_SEND_1MESS    3    
/*----------------------------------------------------------------------------*/
/*======================== variable ======================*/
extern uint8_t             aDCU_ID[DCU_ID_LENGTH];
extern uint8_t             aDCU_SERI[DCU_ID_LENGTH];
extern uint8_t             aMeter_ID[METER_LENGTH];
extern uint8_t             Buff_Recei_Ping[500];
extern uint8_t		       Read_Meter_ID_Success;
extern uint8_t		       Read_Meter_ID_Change;

extern uint8_t Buff_Cmd[50];
extern uint8_t BuffRecei[20];





/*======================== FUNCTION ======================*/
void        Check_Reset_MCU(uint32_t TimeOut);
void        init_Sim900(void);
void        Init_DCU_METER_ID(void);
void        Reset_GPIO(void);
void        Set_default_Sim900(void);
void        Fun_SimControl (void);

void        Check_mode(uint8_t *Step_Control,uint8_t Step_HTTP, uint8_t Step_FTP, uint8_t Step_TCP);
uint8_t     Hard_Reset(void);
uint8_t     Soft_Reset(void);
uint8_t     Check_Tranparent_Mode(uint8_t GSM_Step);
void        Load_Receipt(truct_String *sReceipt,uint8_t *Array_Receipt);
uint8_t	    Do_Receipt(truct_String *sReceipt, int *No_Process, uint8_t *Flag_Success, uint8_t Flag_Next, uint8_t Flag_Return);
uint8_t     State_Init_GPRS(void);
int8_t      Step_Config(int *No_Process, int Process_Begin, int Process_End);

int         Classify_Response(truct_String *Str_Final, int Process_Begin, int Process_End);
uint16_t    Check_Responding(uint16_t No_Responding_wait);

void        _fRESET_SIM900(truct_String *str_Receiv);
void        _fRES_SIM_LOST(truct_String *str_Receiv);
void        _fRES_CLOSED(truct_String *str_Receiv);
void        _fRES_CALL_READY(truct_String *str_Receiv);
void        _fRES_ALREADY_CONNECT(truct_String *str_Receiv);
void        _fAT_CHECK_SIM(truct_String *str_Receiv);
void        _fAT_CHECK_CREG(truct_String *str_Receiv);
void        _fAT_DELAY(truct_String *str_Receiv);
void        _fAT_CHECK_IP(truct_String *str_Receiv);
void        _fAT_CHECK_RSSI(truct_String *str_Receiv);

void        _fFTP_GET_DONE(truct_String *str_Receiv);
void        _fRES_ERROR(truct_String *str_Receiv);
void        _fAT_SIM_ID(truct_String *str_Receiv);

void        _fRESET_COUNT_FIRST(truct_String *str_Receiv);

void        _fTRANPARENT_MODE1(truct_String *str_Receiv);
void        _fTRANPARENT_MODE2(truct_String *str_Receiv);
void        _fOPEN_TCP(truct_String *str_Receiv);
void        _fTRANPARENT_MODE(truct_String *str_Receiv);
void        _fNON_TRANPARENT_MODE(truct_String *str_Receiv);

void        _f_GET_CLOCK(truct_String *str_Receiv);
void        _f_COFI_CONTEXT(truct_String *str_Receiv);
void        _f_COFI_CONTEXT_2(truct_String *str_Receiv);

void        _fSystem_Del_File(truct_String *str_Receiv);
void        _fCFTP_GET_FILE1(truct_String *str_Receiv);
void        _fCFTP_GET_FILE2(truct_String *str_Receiv);
void        _fCFTP_GET_FILE(truct_String *str_Receiv);

//
void        Togle_LED(void);
void        _fControlLed(void);
uint8_t     SendDataTCP_IP(uint8_t* data, uint16_t length, uint8_t Max_Resend, uint32_t Max_Time_Delay);
void        GetHandshakeData(void);
void        SendDatatoSerial(uint8_t *data, uint8_t length);
uint8_t     CheckTimeAutoConnect(void);
void        InitATCommandBuffData(void);

void        _fGet_IP_Module(truct_String *str_Receiv);
void        _fGET_IMEI(truct_String *str_Receiv);

void        Request_Meter(void);
void        _fCHECK_ACK_MESS(truct_String *str_Receiv);
void        _f_TCP_SEND_SERVER (uint8_t* data, uint16_t length, uint8_t Max_Resend, uint32_t Max_Time_Delay);
void        Convert_Time_GMT (ST_TIME_FORMAT* sRTC_Check, uint8_t GMT);

uint8_t     _f_Check_NewMessInfor_Inflash (void);
void        _f_Check_NewMessLpf_Inflash (void);
void        _f_Check_NewMessBill_Inflash (void);
uint8_t     _f_Send_Alarm_To_Queue (uint8_t Type);
void        _fPING_TCP(truct_String *str_Receiv);
uint8_t     Check_Finish_Ping (truct_String* StrBuff);

void        _fFTP_SENDIP_2(truct_String *str_Receiv);
uint8_t     Request_AT_Command (truct_String StrSend, truct_String Str_Recei);

void        _fHTTP_SETURL_1(truct_String *str_Receiv);
void        _fHTTP_SETURL_2(truct_String *str_Receiv);

void        _f_APN_AUTHEN_1(truct_String *str_Receiv);

void        _fHTTP_READ_1(truct_String *str_Receiv);
void        _fHTTP_READ_2(truct_String *str_Receiv);
void        _fHTTP_FIRM_LENGTH(truct_String *str_Receiv);

void        _fFTP_READ_1(truct_String *str_Receiv);
void        _fFTP_READ_2(truct_String *str_Receiv);

void        _f_TCP_SEND_SERVER_DIRECT (uint8_t* data, uint16_t length);
void        _fCHECK_APN_ACTIVE(truct_String *str_Receiv);
void        _fPING_TEST(truct_String *str_Receiv);

void        _fLog_PacketTSVH_ToSectorB (uint8_t Type);
uint8_t     _fCheckCrcPacket (uint8_t *Buff,  uint16_t Length);
void        _fLog_PacketBill_ToSectorB (uint8_t Type);

#endif /* __SIM900_H */






