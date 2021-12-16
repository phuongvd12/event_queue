/*
 * Init_Meter.h
 *
 *  Created on: Dec 15, 2015
 *      Author: Administrator
 */

#include "variable.h"
#include "Init_All_Meter.h"

#ifndef _GELEX_INIT_METER_
#define _GELEX_INIT_METER_    _GELEX_INIT_METER_H_

extern uint8_t		Billing_Num_Record_ui8;

/*======================== Structs ======================*/
#define GELEX_MAX_OBIS_INTAN          47
#define GELEX_MAX_EVENT               29



extern  struct_Obis_Scale               Obis_Gelex_Ins[GELEX_MAX_OBIS_INTAN];
extern  struct_Obis_event               Obis_Gelex_Event[GELEX_MAX_EVENT];



#endif /* _START_INC_INIT_METER_H_ */
