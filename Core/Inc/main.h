/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

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
#define board_led_Pin GPIO_PIN_13
#define board_led_GPIO_Port GPIOC
#define KEY_R_Pin GPIO_PIN_0
#define KEY_R_GPIO_Port GPIOA
#define KEY_ADD_Pin GPIO_PIN_1
#define KEY_ADD_GPIO_Port GPIOA
#define KEY_TIME_Pin GPIO_PIN_4
#define KEY_TIME_GPIO_Port GPIOA
#define KEY_G_Pin GPIO_PIN_5
#define KEY_G_GPIO_Port GPIOA
#define OLED_SDA_Pin GPIO_PIN_6
#define OLED_SDA_GPIO_Port GPIOA
#define OLED_SCL_Pin GPIO_PIN_7
#define OLED_SCL_GPIO_Port GPIOA
#define KEY_B_Pin GPIO_PIN_0
#define KEY_B_GPIO_Port GPIOB
#define D0_LVJING_Pin GPIO_PIN_1
#define D0_LVJING_GPIO_Port GPIOB
#define D0_RGB_Pin GPIO_PIN_11
#define D0_RGB_GPIO_Port GPIOB
#define Beep_Pin GPIO_PIN_12
#define Beep_GPIO_Port GPIOA
#define KEY_FAN_Pin GPIO_PIN_4
#define KEY_FAN_GPIO_Port GPIOB
#define KEY_CNT_Pin GPIO_PIN_5
#define KEY_CNT_GPIO_Port GPIOB
#define KEY_TEMP_Pin GPIO_PIN_6
#define KEY_TEMP_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
