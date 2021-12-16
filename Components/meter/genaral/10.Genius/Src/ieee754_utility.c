
#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"

#include "ieee754_utility.h"
#include "string.h"
#include "math.h"


float ieee754_convertToFloat(uint32_t data) 
{
	float Temp_Data = 0;
	uint8_t  So_Mu = 0;
	uint8_t  Sign = 0;
	
	Sign = (uint8_t) (data >> 31);
	
	So_Mu = (uint8_t)(data >> 23) - 127;
	
	Temp_Data = 1 + ((data & 0x007FFFFF) /(pow (2, 23)));
	
	Temp_Data = Temp_Data * (pow (2,So_Mu));
	
	if(Sign == 1) Temp_Data = -Temp_Data;
	return Temp_Data;
}

//Convert chuan ieee754 64 bit


double ieee754_convertTodouble(uint64_t data) 
{
	double Temp_Data = 0;
	
	uint16_t  So_Mu = 0;
	uint8_t   Sign = 0;
	
	Sign = (uint8_t) (data >> 63);
	
	So_Mu = ((uint16_t)(data >> 52)) & 0x07FF - 1023;
	
	Temp_Data = 1 + ((data & 0x007FFFFF) /(pow (2, 52)));
	
	Temp_Data = Temp_Data * (pow (2,So_Mu));
	
	if(Sign == 1) Temp_Data = - Temp_Data;
	
	return Temp_Data;
}
//convert theo con tro


double Convert_DoublePoint_2Double (uint64_t Double_Point)
{
	return *((double*)(&Double_Point));
}









