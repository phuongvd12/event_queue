/*
 * Init_Meter.h
 *
 *  Created on: Dec 15, 2015
 *      Author: Administrator
 */

#include "variable.h"
#include "Init_All_Meter.h"

#ifndef _ELSTER_INIT_METER_H_
#define _ELSTER_INIT_METER_H_

/*======================== Structs ======================*/
#define	ELSTER_Meter_Delay_ms	300
#define MAX_OBIS_EVENT          20
#define MAX_RECORD_LPF          2000
#define MAX_RECORD_BILL         255

/*======================== Constant ======================*/
extern Meter_Comm_Struct				Meter_Handshake;
extern const	uint8_t	Decode_Password_Table[16];

extern uint8_t 	MeterRawData[64];
extern uint8_t	ELSTER_END[5];
extern uint8_t	ELSTER_Handshake1[5];
extern uint8_t	ELSTER_Handshake2[6];
extern uint8_t	ELSTER_Handshake3[24];
extern uint8_t	ELSTER_Hs_inbuff[16];
extern uint8_t	ELSTER_Hs_outbuff[16];

extern uint8_t ELSTER_Get_MID[16];

extern uint8_t  ELSTER_MInfo_Table[17][23];
extern uint8_t	ELSTER_MeterInfoMessIDTable[61];
extern uint8_t	ELSTER_MeterTuTiIDTable[3];

extern uint8_t  ELSTER_MBilling_Table[5];
extern uint8_t	ELSTER_MeterBillingMessIDTable[16];
extern uint8_t 	ELSTER_MeterBillingTemplate[16];

extern uint8_t  ELSTER_MEvent_Table[12][16];
extern uint8_t	ELSTER_MeterEventMessIDTable[13];
extern uint8_t	ME_ObisOderStart[5];
extern uint8_t	ME_ObisOderEnd[5];

extern uint8_t	MeterAlertMessIDTable[25];

extern uint8_t 	RMLP_First_Mess[18];
extern uint8_t 	RMLP_Second_Mess[16];
extern uint8_t	RMLP_GetData_Mess[16];

extern uint16_t	RMLP_NumDay;
extern uint16_t	Bill_NumDay;
extern uint16_t	ToTal_PackLpf;
extern uint8_t	MeterLProfMessIDTable[20];
extern uint8_t aLoadprofileDataPeriod[11];
extern uint8_t password[8];
extern uint8_t password_backup[8];

//-------------------------Init function--------------------
void Init_Meter_Handshake (void);


#endif /* _START_INC_INIT_METER_H_ */


