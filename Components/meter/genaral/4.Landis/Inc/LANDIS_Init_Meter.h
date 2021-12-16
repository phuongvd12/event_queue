/*
 * Init_Meter.h
 *
 *  Created on: Dec 15, 2015
 *      Author: Administrator
 */



#ifndef _LANDIS_INIT_METER_
#define _LANDIS_INIT_METER_   _LANDIS_INIT_METER_H_

#include "variable.h"
#include "Init_All_Meter.h"

/*======================== Structs ======================*/
#define	LANDIS_Meter_Delay_ms	100
#define MAX_OBIS_INTAN          43
#define LANDIS_MAX_EVENT        5
 

extern Meter_Comm_Struct	    Get_Meter_ShortName;  // dinh nghia them
/*======================== Constant ======================*/
extern uint8_t 	                ID_Frame_ui8;
extern uint16_t                 OBIS_Order;

extern uint8_t 	                Meter_Type;
extern struct_Obis_Scale        Obis_Landis_Ins[MAX_OBIS_INTAN];
extern struct_Obis_event        Obis_landis_Event[LANDIS_MAX_EVENT];
//-------------------------Init function--------------------



#endif /* _START_INC_INIT_METER_H_ */





