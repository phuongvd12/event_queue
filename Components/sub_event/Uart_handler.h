/*
 * myUart.h
 *
 *  Created on: Dec 7, 2021
 *      Author: lenovo
 */

#ifndef MYUART_H_
#define MYUART_H_

#define uart_debug	huart1
#define uart_sim	huart3

#include "stdint.h"
#include "myDefine.h"
#include "variable.h"

extern  uint8_t uartSimBuffReceive[100];
extern  uint8_t sim_rx_buff[1];
extern  uint8_t meter_rx_buff[1];
extern  uint8_t uartDebugBuff[100];
#endif /* MYUART_H_ */
