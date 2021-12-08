

#ifndef T_MQTT_H_
#define T_MQTT_H_

#include "variable.h"
#include "sim900.h"



#define         PASSWORD_LENGTH			10 
#define         MAX_LENGTH_ALARM_POW    200	 
#define         MAX_LENGTH_DATA_TCP     1280    	 

typedef enum {
	DATA_temp,
    DATA_PRE_OPERA,
	DATA_OPERATION,                                     //1
    DATA_LOAD_PROFILE,		// Load Profile               2
    DATA_PRE_HISTORICAL,
	DATA_HISTORICAL,		// Bill                       3
    
	DATA_EVEN_METER,		// Even Meter                 5
    DATA_INTANTANIOUS,	
    
    DATA_METER_INFOR,
	DATA_LOG_DCU,
	DATA_RESPOND_AT,
    DATA_TUTI,
    
    NOTIF_CONNECT_METER,
    NOTIF_DISCONNECT_METER,

	ALARM_POWER_ON_MODERM,	         //       11
	ALARM_POWER_OFF_MODERM,			// Alarm Log
	ALARM_POWER_UP_METER,			// Alarm Log
	ALARM_POWER_DOWN_METER,
    UPDATE_FIRM_OK,             //16
    UPDATE_FIRM_FAIL,
} Type_Message_MQTT_Send;

typedef enum {
    // Don't need feedback
	SEND_UPDATE_FIRMWARE,
	SEND_UPDATE_FIRMWARE_PENDING,
	SEND_UPDATE_FIRMWARE_OK,
	SEND_UPDATE_FIRMWARE_FAIL_SETUP,
	SEND_UPDATE_FIRMWARE_FAIL_TIME,
	SEND_UPDATE_FIRMWARE_FAIL_FLASH,
	SEND_UPDATE_FIRMWARE_FAIL_LENGTH,
	SEND_ERASE_FLASH_SUCCESS,
	SEND_ERASE_FLASH_FAIL,
}Struct_Step_UPFirm;

typedef void (*task_callback_Send)(int Kind_Send);

typedef struct {
	int 				idMark;
	task_callback_Send 	CallBack;
	truct_String		sKind;					// Send -> Sim900
} struct_MARK_TYPE_Message_SEND;


extern uint8_t aServer_MQTT_RQ[DCU_ID_LENGTH];
extern uint8_t aPort_MQTT_RQ[PASSWORD_LENGTH];

extern uint8_t aServer_MQTT[DCU_ID_LENGTH];
extern uint8_t aPort_MQTT[PASSWORD_LENGTH];
	
//================================FUNCTION====================================
void        Init_DCU(void);
void        Init_Port_MQTT(void);
void        Fill_Table (void);
uint8_t     Check_Sector_Table_Empty(void);
void        Update_Table(Meter_Flash_Queue_Struct* SendUpdate);
uint8_t     Check_update_Time(ST_TIME_FORMAT *sRTC_temp);

//_f_callback TCP send
void            _mDATA_LOG_DCU(int Kind_Send);
void            _mDATA_NOTIF_DISCONNECT_MET(int Kind_Send);
void            _mDATA_NOTIF_CONNECT_MET(int Kind_Send);

void            _mDATA_ALARM_P_ON_MOD(int Kind_Send);
void            _mDATA_ALARM_P_OFF_MOD(int Kind_Send);
void            _mDATA_ALARM_P_UP_MET(int Kind_Send);
void            _mDATA_ALARM_P_DOW_MET(int Kind_Send);
//
void            _f_Send_Meter_Data (void);
void            _mUPDATE_FIRM_SUCCES(int Kind_Send);
void            _mUPDATE_FIRM_FAIL(int Kind_Send);
void            _mDATA_METER_INFOR(int Kind_Send);
void            _mDATA_AT_RESPOND(int Kind_Send) ;

void            _fSent_MetData_Delay (void);
void            _fSent_MetData_Important (void);
void            _fSent_LPF_DELAY(void);
void            _mDATA_GET_FROM_QUEUE(int Kind_Send) ;


#endif /* T_MQTT_H_ */
