/*
 * Copyright (c) 2013,2014,2015 Smart Energy Division, Viettel ICT
 * All rights reserved.
 *
 * crc_utility.h
 *
 *  Created on: May 5, 2014
 *     Author: hoangnn4@viettel.com.vn
 */

#ifndef CRC_UTILITY_H_
#define CRC_UTILITY_H_

#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"

/**
 * @par Description: This function calculate CRC CCITT XModem for a HEX array
 * @param [in]  buff            The buff data need to calculate the CRC.
 * @param [in]  length          The length of buffer.
 * @retval      crc             crc
 */
 
uint16_t crc_calcCcittXmodem(uint8_t *buff, uint16_t length);
int16_t gencrc_16(int16_t i);
uint16_t CalculateCharacterCRC16( uint16_t crc, uint16_t c );

#endif /* CRC_UTILITY_H_ */
