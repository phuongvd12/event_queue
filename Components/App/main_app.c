/*
 * main_app.c
 *
 *  Created on: Nov 26, 2021
 *      Author: lenovo
 */


#include "main_app.h"
#include "sim_command.h"
#include "meter_test.h"
#include "flash_ext.h"
#include "event_driven.h"

static void AppInit(void);

void MainTask(void)
{
	AppInit();

	for (;;)
	{
		EventTask();
		SimTask();
		MeterTask();
		FlashTask();
	}
}


static void AppInit(void)
{
	Event_Init();
	SimInit();
	MeterInit();
	FlashInit();
}
