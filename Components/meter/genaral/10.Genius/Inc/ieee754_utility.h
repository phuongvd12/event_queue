
#ifndef IEEE754_UTILITY_H_
#define IEEE754_UTILITY_H_

#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"


#include "crc_utility.h"
#include "command_line_protocol.h"

///////
float 				ieee754_convertToFloat(uint32_t data);  
double 				ieee754_convertTodouble(uint64_t data);  
//convert theo con tro
double 				Convert_DoublePoint_2Double (uint64_t Double_Point);
	

#endif /* IEEE754_UTILITY_H_ */
