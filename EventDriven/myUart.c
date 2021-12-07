/*
 * myUart.c
 *
 *  Created on: Dec 7, 2021
 *      Author: lenovo
 */

#include "myDefine.h"

/* callback uart */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == uart_sim.Instance)
	{
		*(sUartSim.data+sUartSim.length) = sim_rx_buff[0];
		sUartSim.length++;
		if (sUartSim.length >= sizeof(uartSimBuffReceive))
		{
			sUartSim.length = 0;
		}
		HAL_UART_Receive_IT(&uart_sim, sim_rx_buff, 1);
		fevent_enable(s_event_sim_handler, EVENT_SIM_UART_RECEIVE);
	}

	if (huart->Instance == uart_meter.Instance)
	{
		*(sUartMeter.data+sUartMeter.length) = meter_rx_buff[0];
		sUartMeter.length++;
		if (sUartMeter.length >= sizeof(uartMeterBuffReceive))
		{
			sUartMeter.length = 0;
		}
		HAL_UART_Receive_IT(&uart_meter, meter_rx_buff, 1);
		fevent_enable(s_event_meter_handler, EVENT_METER_UART_RECEIVE);
	}
}


