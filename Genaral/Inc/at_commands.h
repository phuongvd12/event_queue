
#ifndef at_commands
#define at_commands _at_commands_H

#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"

#include "stdint.h"
#include "stdbool.h"
#include "variable.h"




extern uint8_t 		                    PortConfig;
extern const struct_CheckList_GSM       CheckList_AT[];
extern truct_String                     Str_Cmd_AT;

typedef enum
{
    AT_CMD_WAIT,                    // 0 - khong dung
    AT_SEND_DATA_UART,              // 1 - gui du lieu den cong Uart doc cong to
    AT_CONNECT_SERVICE,             // 2 - ket noi mang - ket noi toi IP, PORT da cau hinh
    AT_CONNECT_SERVICE_SUCCESS,     // 3 - ket noi dich vu thanh cong
    AT_DISCONNECT_SERVICE,          // 4 - ngat ket noi mang - ngat ket noi voi IP, PORT da cau hinh
    AT_DISCONNECT_SERVICE_SUCCESS,  // 5 - ngat ket noi dich vu thanh cong
    AT_PING_TCP,                    // 6 - kiem tra ket noi cua IP, PORT nhan dc
    AT_CONNECT_TCP,                 // 7 - ket noi den Server - ket noi den IP, PORT nhan dc
    AT_CONNECT_TCP_SUCCESS,         // 8 - ket noi den Server thanh cong
    AT_SEND_DATA_TCP,               // 9 - Gui du lieu den Server - gui du lieu den IP, PORT va data nhan dc
    AT_DISCONNECT_TCP,              // 10- Ngat ket noi den Server - ngat ket noi den IP, PORT nhan dc
    AT_CHECK_CONNECT,               // Kiem tra trang thai ket noi cua thiet bi
}eAT_Command;



typedef enum
{
	_ACCOUNT,
    _QUERY_ACCOUNT,
	_PASSWORD,
    _RESET_PASSWORD,
    _QUERY_PASSWORD,
	_SET_DEV_SERIAL,
	_QUERY_DEV_SERIAL,
	_SET_SERVER_INFO,	
	_QUERY_SERVER_INFO,
	_SET_CONNECT_MODE,
	_QUERY_CONNECT_MODE,
	_SET_RTC_TIME,
	_SET_RTC_DATE, 
	_QUERY_RTC,
	_SET_CONNECT_TIME, 
	_QUERY_CONNECT_TIME,
	_SET_CONNECT_PERIOD,
	_QUERY_CONNECT_PERIOD,
	_SET_DISCONNECT_TIME,
	_QUERY_DISCONNECT_TIME,
	_SET_DEV_IP, 
	_QUERY_DEV_IP,	
	_SET_UART_PAR, 
	_QUERY_UART_PAR,
	_SEND_DATA_SERIAL,
	_CONNECT_TCPIP,
	_DISCONNECT_TCPIP,
	_PING_TCPIP,
	_CONNECT_SERVER,
	_SEND_DATA_SERVER,
    _CONFIG_RS485,
	_DISCONNECT_SERVER,
    //Sv
    _IEC620_56_METER,
    _DLMS_METER,
    _ERASE_FLASH,
    _SET_FREQ_HEARTB,
    _ON_DEBUG,
    _SET_RADIO,
    _QUERY_RADIO,
    _SET_DEV_ID,
    _QUERY_DEV_ID,
    _QUERY_BAND,
    _QUERY_CPSI,
    _QUERY_CNMP,
    //bo sung
    _SET_SEND_LPF_TIME,
    _QUERY_SEND_LPF_TIME,
    
    _READ_LPF_FORTIME,
    
    _READ_INTAN_REGIS,
    _READ_INFOR_METER,
    _READ_BILL_FORTIME,
    _QUERY_EVENT_METER,
    _QUERY_VOLUME_MEM,
    _QUERY_NETIP,
    _RESET_DEVICE,
    _QUERY_EMEI_SIM,
    _QUERY_FIRM_VER,
    _QUERY_UP_FIRM_HTTP,
    _QUERY_UP_FIRM_FTP,

    _QUERY_INDEX_LOG,
    _TEST_LOG, 
    _QUERY_READ_LOG_FOR,
    _QUERY_READ_ALL_LOG,
    _QUERY_READ_INDEX_LOG,

}Type_Command_AT;


/*-------------------Function-------------------------*/
void        _fACCOUNT (truct_String *str_Receiv);
void        _fQUERY_ACCOUNT (truct_String *str_Receiv);
void        _fPASSWORD (truct_String *str_Receiv);
void        _fSET_DEV_SERIAL (truct_String *str_Receiv);
void        _fQUERY_DEV_SERIAL (truct_String *str_Receiv);
void        _fSET_SERVER_INFO (truct_String *str_Receiv);
void        _fQUERY_SERVER_INFO (truct_String *str_Receiv);
void        _fSET_CONNECT_MODE (truct_String *str_Receiv);
void        _fQUERY_CONNECT_MODE (truct_String *str_Receiv);
void        _fSET_RTC_TIME (truct_String *str_Receiv);
void        _fSET_RTC_DATE (truct_String *str_Receiv);
void        _fQUERY_RTC (truct_String *str_Receiv);
void        _fSET_CONNECT_TIME (truct_String *str_Receiv);
void        _fQUERY_CONNECT_TIME (truct_String *str_Receiv);
void        _fSET_CONNECT_PERIOD (truct_String *str_Receiv);
void        _fQUERY_CONNECT_PERIOD (truct_String *str_Receiv);
void        _fSET_DISCONNECT_TIME (truct_String *str_Receiv);
void        _fQUERY_DISCONNECT_TIME (truct_String *str_Receiv);
void        _fSET_DEV_IP (truct_String *str_Receiv);
void        _fQUERY_DEV_IP (truct_String *str_Receiv);
void        _fSET_UART_PAR (truct_String *str_Receiv);
void        _fQUERY_UART_PAR (truct_String *str_Receiv);
void        _fSEND_DATA_SERIAL (truct_String *str_Receiv);
void        _fCONNECT_TCPIP (truct_String *str_Receiv);
void        _fDISCONNECT_TCPIP (truct_String *str_Receiv);
void        _fPING_TCPIP (truct_String *str_Receiv);
void        _fCONNECT_SERVER (truct_String *str_Receiv);
void        _fSEND_DATA_SERVER (truct_String *str_Receiv);
void        _fCONFIG_RS485 (truct_String *str_Receiv);
void        _fDISCONNECT_SERVER (truct_String *str_Receiv);
void        _fREQUEST_IEC62056 (truct_String *str_Receiv);
void        _fREQUEST_DLMS (truct_String *str_Receiv);
void        _fGET_FORTIME_READ (truct_String *str_Receiv);

void        _fSET_SEND_LPF_TIME (truct_String *str_Receiv);
void        _fQUERY_SEND_LPF_TIME (truct_String *str_Receiv);
void        _fGET_INTAN_REGIS (truct_String *str_Receiv);
void        _fQUERY_INFOR_METER (truct_String *str_Receiv);
void        _fGET_FORTIME_BILL (truct_String *str_Receiv);
void        _fQUERY_EVENT_METER (truct_String *str_Receiv);
void        _fQUERY_VOLUME_MEM (truct_String *str_Receiv);
void        _fQUERY_SIM_IP (truct_String *str_Receiv);
void        _fRESET_DEVICE (truct_String *str_Receiv);
void        _fQUERY_EMEI_SIM (truct_String *str_Receiv);
void        _fQUERY_FIRM_VER (truct_String *str_Receiv);
void        _fGET_URLHTTP_FIRM (truct_String *str_Receiv);
void        _fGET_URLFTP_FIRM (truct_String *str_Receiv);

void        _fERASE_FLASH (truct_String *str_Receiv);
void        _fSET_DUTY (truct_String *str_Receiv);
void        _fON_DEBUG (truct_String *str_Receiv);
void        _fSET_RADIO (truct_String *str_Receiv);
void        _fQUERY_RADIO (truct_String *str_Receiv);
void        _fSET_DEV_ID (truct_String *str_Receiv);
void        _fQUERY_DEV_ID (truct_String *str_Receiv);
void        _fQUERY_PASSWORD (truct_String *str_Receiv);
void        _fRESET_PASSWORD (truct_String *str_Receiv);

void        _fQUERY_BAND (truct_String *str_Receiv);
void        _fQUERY_CPSI (truct_String *str_Receiv);
void        _fQUERY_CNMP (truct_String *str_Receiv);
void        _fQUERY_INDEX_LOG (truct_String *str_Receiv);
void        _fTEST_LOG (truct_String *str_Receiv);

void        _fGET_INDEX_LOG (truct_String *str_Receiv);
void        _fQUERY_ALL_LOG (truct_String *str_Receiv);
void        _fGET_FOR_INDEX_LOG (truct_String *str_Receiv);

//
void        Init_APN_Info (void);
void        Init_Auto_Connect_Time (void);
void        Init_Device_IP (void);
	
uint8_t     CheckATCommandPC(void);
uint8_t     CheckATCommandServer(void);
uint8_t     CheckATCommandSMS(void);
void        Init_UART_Config (void);
void        GET_UART_Meter_Config (void);
void        Set_UartMeterConfig (void);

void        DCU_Respond(uint8_t portNo, uint8_t *data, uint8_t length);
uint16_t    _fPacket_RespondAT (uint8_t* Buff, uint8_t* Cmd, uint16_t lenCmd, uint8_t* aData, uint16_t lendata);
void        ResetCountGPRS(void);
void        DCU_Respond_Direct (uint8_t portNo, uint8_t *data, uint8_t length);


#endif



