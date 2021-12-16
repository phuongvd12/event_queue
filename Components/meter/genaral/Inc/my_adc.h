//*****************************************************************************

#ifndef my_adc
#define my_adc	_my_adc_H

#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"


//*****************************************************************************

uint16_t            HW_AdcReadChannel( uint32_t Channel, uint32_t Rank );
uint32_t            HW_GetBatteryLevel( void ) ;
uint32_t            Get_Value_ADC (uint32_t * Voltage_Result_mV, uint32_t Channel, uint32_t rank);
uint32_t            Funcion_Get_Vin (void);

#endif // 


