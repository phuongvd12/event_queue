#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"

#include "my_adc.h"
#include "adc.h"
#include "stm32l4xx_hal_adc_ex.h"

uint8_t                     AdcInitialized = 0;

/*!
 * \brief ADC Vbat measurement constants
 */

 /* Internal voltage reference, parameter VREFINT_CAL*/
#define VREFINT_CAL                 ((uint16_t*) ((uint32_t) 0x1FFFF7BA)) 
#define LORAWAN_MAX_BAT             100

#define VDDA_VREFINT_CAL            ((uint32_t) 3300)        

#define VDD_BAT                     4000     
#define VDD_MIN                     1800





/*------------------------Funcion------------------------*/

uint16_t HW_AdcReadChannel( uint32_t Channel, uint32_t Rank )
{
    ADC_ChannelConfTypeDef sConfig;

    uint16_t adcData = 0;

    MX_ADC1_Init();
    
    if( AdcInitialized == 1 )
    { 
        __HAL_RCC_ADC_CLK_ENABLE();
        
        /*calibrate ADC if any calibraiton hardware*/
        HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED ); 
             
        /** Configure Regular Channel 
        */
        sConfig.Channel = Channel;
        sConfig.Rank = Rank;
        sConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;
        sConfig.SingleDiff = ADC_SINGLE_ENDED;
        sConfig.OffsetNumber = ADC_OFFSET_NONE;
        sConfig.Offset = 0;
        if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
        {
            Error_Handler();
        }

        /* Start the conversion process */
        HAL_ADC_Start( &hadc1);
          
         /* Wait for the end of conversion */
        HAL_ADC_PollForConversion( &hadc1, HAL_MAX_DELAY ); 

        /* Get the converted value of regular channel */
        adcData = HAL_ADC_GetValue ( &hadc1);

        HAL_ADC_Stop (&hadc1);

        __HAL_RCC_ADC_CLK_DISABLE();
    }
    return adcData;
}



/**
  * @brief This function return the battery level
  * @param none
  * @retval the battery level  1 (very low) to 254 (fully charged)
  */



uint32_t HW_GetBatteryLevel( void ) 
{
  uint16_t measuredLevel = 0;
  uint32_t batteryLevelmV = 0;
  
	
  measuredLevel = HW_AdcReadChannel( ADC_CHANNEL_VREFINT, ADC_REGULAR_RANK_2 );
  
  if (measuredLevel == 0)
  {
    batteryLevelmV = 0;
  }
  else
  {
    batteryLevelmV= (( (uint32_t) VDDA_VREFINT_CAL * (*VREFINT_CAL ) )/ measuredLevel);
  }

  return batteryLevelmV;
}




uint32_t Get_Value_ADC (uint32_t * Voltage_Result_mV, uint32_t Channel, uint32_t rank)
{
//	uint32_t 	Bat_Voltage_u32 = 0;
	uint32_t 	Value_ADC_u32 = 0;
    uint32_t    Temp_mV = 0;
    uint16_t    batteryLevel = 0;
	
	Value_ADC_u32 = HW_AdcReadChannel( Channel, rank ); 
	//lay dien ap pin ve day la dien ap tham chieu
//	Bat_Voltage_u32 = HW_GetBatteryLevel();
	//quy doi sang mV cho gia tri ADC doc ve
//	Temp_mV = (Bat_Voltage_u32 * Value_ADC_u32) / 4095; 
    
    Temp_mV = (3300 * Value_ADC_u32 * 2) / 4095; 
    
    (* Voltage_Result_mV)  = Temp_mV; 
    
    if (Temp_mV > VDD_BAT)
    {
        batteryLevel = LORAWAN_MAX_BAT;
    }
    else if (Temp_mV < VDD_MIN)
    {
        batteryLevel = 0;
    }
    else
    {
        batteryLevel = (( (uint32_t) (Temp_mV - VDD_MIN)*LORAWAN_MAX_BAT) /(VDD_BAT-VDD_MIN) ); 
    }
    return batteryLevel;
}

uint32_t Funcion_Get_Vin (void)
{
    uint32_t Result = 0;
    Get_Value_ADC (&Result, ADC_CHANNEL_1, ADC_REGULAR_RANK_1);
    
    return Result;
}














