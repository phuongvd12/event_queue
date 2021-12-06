/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define PB12_Pin GPIO_PIN_1
#define PB12_GPIO_Port GPIOC
#define PB13_Pin GPIO_PIN_3
#define PB13_GPIO_Port GPIOC
#define HOLD_RESET_FLASH_Pin GPIO_PIN_1
#define HOLD_RESET_FLASH_GPIO_Port GPIOA
#define PB14_Pin GPIO_PIN_2
#define PB14_GPIO_Port GPIOA
#define PB15_Pin GPIO_PIN_3
#define PB15_GPIO_Port GPIOA
#define SPI_NSS_Pin GPIO_PIN_4
#define SPI_NSS_GPIO_Port GPIOC
#define RESET_UC15_Pin GPIO_PIN_5
#define RESET_UC15_GPIO_Port GPIOC
#define PWRKEY_Pin GPIO_PIN_0
#define PWRKEY_GPIO_Port GPIOB
#define SIM_PRE_Pin GPIO_PIN_1
#define SIM_PRE_GPIO_Port GPIOB
#define SIM_TX_Pin GPIO_PIN_10
#define SIM_TX_GPIO_Port GPIOB
#define SIM_RX_Pin GPIO_PIN_11
#define SIM_RX_GPIO_Port GPIOB
#define SIM_DTR_Pin GPIO_PIN_13
#define SIM_DTR_GPIO_Port GPIOB
#define SIM_RTS_Pin GPIO_PIN_14
#define SIM_RTS_GPIO_Port GPIOB
#define TP1_Pin GPIO_PIN_7
#define TP1_GPIO_Port GPIOC
#define TP2_Pin GPIO_PIN_8
#define TP2_GPIO_Port GPIOC
#define NET485IO_Pin GPIO_PIN_8
#define NET485IO_GPIO_Port GPIOA
#define UART_TXD_Pin GPIO_PIN_9
#define UART_TXD_GPIO_Port GPIOA
#define UART_RXD_Pin GPIO_PIN_10
#define UART_RXD_GPIO_Port GPIOA
#define MCU_232_CTS_Pin GPIO_PIN_11
#define MCU_232_CTS_GPIO_Port GPIOA
#define MCU_232_RTS_Pin GPIO_PIN_12
#define MCU_232_RTS_GPIO_Port GPIOA
#define TX_DEBUG_Pin GPIO_PIN_10
#define TX_DEBUG_GPIO_Port GPIOC
#define RX_DEBUG_Pin GPIO_PIN_11
#define RX_DEBUG_GPIO_Port GPIOC
#define ON_OFF_SIM_Pin GPIO_PIN_3
#define ON_OFF_SIM_GPIO_Port GPIOB
#define CE_BQ_Pin GPIO_PIN_4
#define CE_BQ_GPIO_Port GPIOB
#define CHG_Pin GPIO_PIN_5
#define CHG_GPIO_Port GPIOB
#define PGOOD_Pin GPIO_PIN_8
#define PGOOD_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
