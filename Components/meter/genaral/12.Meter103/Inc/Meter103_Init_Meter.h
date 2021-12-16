/*
 * Init_Meter.h
 *
 *  Created on: Dec 15, 2015
 *      Author: Administrator
 */

#include "variable.h"
#include "Init_All_Meter.h"


#ifndef _METER_103_INIT_METER_H_
#define _METER_103_INIT_METER_H_



#define     METER103_MAX_OBIS_TSVH           55
#define     MAX_OBIS_TUTI           2
#define     MAX_OBIS_EVEN           50
#define     MAX_OBIS_BILL           50
#define     MAX_OBIS_LPF            2

/*======================== struct and var ======================*/
extern  uint8_t	        MET103_GELEX_MANUFAC[3];
extern  uint8_t	        MET103_HH_MANUFAC[3];
extern  uint8_t	        MET103_VSE_MANUFAC[3];

extern  uint8_t	        MET103_REQUEST_BAUD[6];
extern  uint8_t	        MET103_Handshake[5];
extern  uint8_t	        MET103_SEND_PASS[16];
extern  uint8_t         METER103_PASS_GEL[8];
extern  uint8_t         METER103_PASS_HHM[8];
extern  uint8_t         METER103_PASS_VSE[8];
extern  uint8_t         METER103_PASS_WRITE_HH[8];

extern truct_String    METER103_TSVH_ObisCode[METER103_MAX_OBIS_TSVH];
extern truct_String    METER103_TUTI_ObisCode[MAX_OBIS_TUTI];
extern truct_String    METER103_BILL_ObisCode[MAX_OBIS_BILL];
extern truct_String    METER103_EVENT_ObisCode[MAX_OBIS_EVEN];
extern truct_String    METER103_LPF_ObisCode[MAX_OBIS_LPF];
//
extern uint8_t         METER103_MeterInfoMessIDTable[60];
extern uint8_t         METER103_MeterTuTi_IDTable[3];
extern uint8_t         METER103_MeterBillMessIDTable[30];
extern uint8_t         METER103_MeterEvenMessIDTable[30];
extern uint8_t         METER103_MeterLPFMessIDTable[3];

extern uint8_t         METER103_LPF_StartTime[5];
extern uint8_t         METER103_LPF_StopTime[5];
/*======================== Function ======================*/



#endif /* _START_INC_INIT_METER_H_ */



