/*
 * queue_p.c
 *
 *  Created on: Nov 30, 2021
 *      Author: lenovo
 */

#define QUEUE_SIZE	30

#include "queue_p.h"


static uint8_t aQueueSimStep[QUEUE_SIZE];
static uint8_t aQueueMeterCmd[QUEUE_SIZE];
static uint8_t aQueueMeterMessage[QUEUE_SIZE];

sQueue_Struct_TypeDef QueueSimStep =
{
		.number = 0,
		.head = 0,
		.tail = 0,
		.status = 0,
		.address = aQueueSimStep,
};

sQueue_Struct_TypeDef QueueMeterCmd =
{
		.number = 0,
		.head = 0,
		.tail = 0,
		.status = 0,
		.address = aQueueMeterCmd,
};

sQueue_Struct_TypeDef QueueMeterMessage =
{
		.number = 0,
		.head = 0,
		.tail = 0,
		.status = 0,
		.address = aQueueMeterMessage,
};



uint8_t Q_push_data_to_queue(sQueue_Struct_TypeDef *sQueue, uint8_t value)
{
	if (sQueue->number >= QUEUE_SIZE)
		return 0;
	*(sQueue->address + sQueue->head) = value;

	sQueue->number += 1;
	sQueue->head += 1;
	if (sQueue->head >= QUEUE_SIZE)
		sQueue->head = 0;

	return 1;
}

uint8_t Q_get_data_from_queue(sQueue_Struct_TypeDef *sQueue, uint8_t Type)
{
	uint8_t value = 0;

	if (sQueue->number == 0)
			return 0;
	value = *(sQueue->address + sQueue->tail);
	if (Type > 0) // clear
	{
		sQueue->number -= 1;
		sQueue->tail += 1;
		if (sQueue->tail >= QUEUE_SIZE)
			sQueue->tail = 0;
	}
	return value;
}

uint8_t Q_clear_queue(sQueue_Struct_TypeDef *sQueue)
{
	sQueue->number = 0;
	sQueue->tail = sQueue->head;
//	sQueue->status = 0;

	return 1;
}

uint8_t Q_get_number_items(sQueue_Struct_TypeDef *sQueue)
{
	return sQueue->number;
}
