/*
 * main_app.c
 *
 *  Created on: Nov 26, 2021
 *      Author: lenovo
 */


#include "main_app.h"
#include "stdint.h"
#include "sim_command.h"
#include "event_driven.h"
#include "elster.h"

static void AppInit(void);

void MainTask(void)
{
	AppInit();
	for (;;)
	{
		EventTask();
		SimTask();
		MeterTask();
	}
}


static void AppInit(void)
{
	Event_Init();
	SimInit();
	MeterInit();
}
