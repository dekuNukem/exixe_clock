/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
#include "stm32f0xx_hal.h"

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
#define GPS_RESET_Pin GPIO_PIN_13
#define GPS_RESET_GPIO_Port GPIOC
#define UP_BUTTON_Pin GPIO_PIN_0
#define UP_BUTTON_GPIO_Port GPIOF
#define DOWN_BUTTON_Pin GPIO_PIN_1
#define DOWN_BUTTON_GPIO_Port GPIOF
#define SD_CS_Pin GPIO_PIN_0
#define SD_CS_GPIO_Port GPIOA
#define OWIRE_DATA_Pin GPIO_PIN_6
#define OWIRE_DATA_GPIO_Port GPIOA
#define USER_LED_Pin GPIO_PIN_7
#define USER_LED_GPIO_Port GPIOA
#define EXIXE1_CS_Pin GPIO_PIN_0
#define EXIXE1_CS_GPIO_Port GPIOB
#define EXIXE2_CS_Pin GPIO_PIN_1
#define EXIXE2_CS_GPIO_Port GPIOB
#define EXIXE3_CS_Pin GPIO_PIN_2
#define EXIXE3_CS_GPIO_Port GPIOB
#define EXIXE4_CS_Pin GPIO_PIN_12
#define EXIXE4_CS_GPIO_Port GPIOB
#define EXIXE5_CS_Pin GPIO_PIN_13
#define EXIXE5_CS_GPIO_Port GPIOB
#define EXIXE6_CS_Pin GPIO_PIN_14
#define EXIXE6_CS_GPIO_Port GPIOB
#define GPS_TP_Pin GPIO_PIN_15
#define GPS_TP_GPIO_Port GPIOB
#define GPS_TP_EXTI_IRQn EXTI4_15_IRQn
#define EXIXE_RESET_Pin GPIO_PIN_15
#define EXIXE_RESET_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
