/*
 * main_app.h
 *
 *  Created on: Nov 26, 2021
 *      Author: lenovo
 */

#ifndef MAIN_APP_H_
#define MAIN_APP_H_
#include "stdint.h"

typedef enum
{
	APP_INT = 0,
	APP_SETTING,
	APP_GET_DATA,
}app_step_TypeDef;

void MainTask(void);
#endif /* MAIN_APP_H_ */
