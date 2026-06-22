/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "../oled/driver_ssd1306_basic.h"
#include "../DS18b20/ds18b20.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define Key_None 0
#define Key_Add 1
#define Key_Cnt 2
#define Key_Temp 3
#define Key_Time 4
#define Key_Fan 5
#define Key_R 6
#define Key_G 7
#define Key_B 8
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
void HuoEr_state(void);
void key_state(void);
void beep_state(void);
void Power_led_state(void);
void fan_state(void);
void temp_state(void);
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
TIM_HandleTypeDef htim2;

char buf[80];
uint32_t led_cnt = 0;
uint32_t num = 0;
uint32_t key_num = 0;
uint32_t num_cnt = 0;
uint8_t beep_active = RESET;
uint8_t temp_tick = 0;
uint32_t target_temp = 0;
uint8_t Time_fen=0;
uint8_t Time_miao=0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void moto1_run()
{
  HAL_GPIO_WritePin(RGB_MOTO_AIN1_GPIO_Port, RGB_MOTO_AIN1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(RGB_MOTO_AIN2_GPIO_Port, RGB_MOTO_AIN2_Pin, GPIO_PIN_RESET);
}

void moto1_stop()
{
  HAL_GPIO_WritePin(RGB_MOTO_AIN1_GPIO_Port, RGB_MOTO_AIN1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(RGB_MOTO_AIN2_GPIO_Port, RGB_MOTO_AIN2_Pin, GPIO_PIN_SET);
}

void moto2_run()
{
  HAL_GPIO_WritePin(RGB_lvjing_BIN1_GPIO_Port, RGB_lvjing_BIN1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(RGB_lvjing_BIN2_GPIO_Port, RGB_lvjing_BIN2_Pin, GPIO_PIN_RESET);
}

void moto2_stop()
{
  HAL_GPIO_WritePin(RGB_lvjing_BIN1_GPIO_Port, RGB_lvjing_BIN1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(RGB_lvjing_BIN2_GPIO_Port, RGB_lvjing_BIN2_Pin, GPIO_PIN_SET);
}

void moto_state()
{
  if(num==1)
  {
    moto1_run();
  }
  else
  {
    moto1_stop();
  }

  if(num==2)
  {
    moto2_run();
  }
  else
  {
    moto2_stop();
  }
}
/**
 * P1  P2  PWM
 * 0  0    1    停止
 * 0  1    1    正转
 * 1  0    1    反转
 */
void PaErTie_Run()
{
  HAL_GPIO_WritePin(PaErTie_AIN1_GPIO_Port, PaErTie_PWM_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(PaErTie_AIN1_GPIO_Port, PaErTie_AIN1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(PaErTie_AIN2_GPIO_Port, PaErTie_AIN2_Pin, GPIO_PIN_RESET);
}

void PaErTie_Stop()
{
  HAL_GPIO_WritePin(PaErTie_AIN1_GPIO_Port, PaErTie_PWM_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(PaErTie_AIN1_GPIO_Port, PaErTie_AIN1_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(PaErTie_AIN2_GPIO_Port, PaErTie_AIN2_Pin, GPIO_PIN_RESET);
}


void HuoEr_state()
{
    if (HAL_GPIO_ReadPin(D0_LVJING_GPIO_Port, D0_LVJING_Pin) == GPIO_PIN_SET)
    {
      snprintf(buf, sizeof(buf), "HR1: %d", 1);
      ssd1306_basic_string_no_update(0, 12, buf, (uint16_t)strlen(buf), 0, SSD1306_FONT_12);
    }
    else
    {
      snprintf(buf, sizeof(buf), "HR1: %d", 0);
      ssd1306_basic_string_no_update(0, 12, buf, (uint16_t)strlen(buf), 0, SSD1306_FONT_12);
    }

    if (HAL_GPIO_ReadPin(D0_RGB_GPIO_Port, D0_RGB_Pin) == GPIO_PIN_SET)
    {
      snprintf(buf, sizeof(buf), "HR2: %d", 1);
      ssd1306_basic_string_no_update(0, 24, buf, (uint16_t)strlen(buf), 0, SSD1306_FONT_12);
    }
    else
    {
      snprintf(buf, sizeof(buf), "HR2: %d", 0);
      ssd1306_basic_string_no_update(0, 24, buf, (uint16_t)strlen(buf), 0, SSD1306_FONT_12);
    }
}

void key_state()
{
    if (HAL_GPIO_ReadPin(KEY_ADD_GPIO_Port, KEY_ADD_Pin) == GPIO_PIN_RESET)
    {
      num=Key_Add;
    }
    else if(HAL_GPIO_ReadPin(KEY_CNT_GPIO_Port, KEY_CNT_Pin) == GPIO_PIN_RESET)
    {
      num=Key_Cnt;
    }
    else if(HAL_GPIO_ReadPin(KEY_TEMP_GPIO_Port, KEY_TEMP_Pin) == GPIO_PIN_RESET)
    {
      num=Key_Temp;
    }
    else if(HAL_GPIO_ReadPin(KEY_TIME_GPIO_Port, KEY_TIME_Pin) == GPIO_PIN_RESET)
    {
      num=Key_Time;
    }
    else if(HAL_GPIO_ReadPin(KEY_FAN_GPIO_Port, KEY_FAN_Pin) == GPIO_PIN_RESET)
    {
      num=Key_Fan;
    }
    else if(HAL_GPIO_ReadPin(KEY_R_GPIO_Port, KEY_R_Pin) == GPIO_PIN_RESET)
    {
      num=Key_R;
    }
    else if(HAL_GPIO_ReadPin(KEY_G_GPIO_Port, KEY_G_Pin) == GPIO_PIN_RESET)
    {
      num=Key_G;
    }
    else if(HAL_GPIO_ReadPin(KEY_B_GPIO_Port, KEY_B_Pin) == GPIO_PIN_RESET)
    {
      num=Key_B;
      beep_active=SET;
    }
    else
    {
      num=Key_None;
    }

    switch(num)
    {
      case Key_Add:
      snprintf(buf, sizeof(buf), "key is %s", "ADD "); break;
      case Key_Cnt:
      snprintf(buf, sizeof(buf), "key is %s", "CNT "); break;
      case Key_Temp:
      snprintf(buf, sizeof(buf), "key is %s", "TEMP"); break;
      case Key_Time:
      snprintf(buf, sizeof(buf), "key is %s", "TIME"); break;
      case Key_Fan:
      snprintf(buf, sizeof(buf), "key is %s", "FAN "); break;
      case Key_R:
      snprintf(buf, sizeof(buf), "key is %s", "R   "); break;
      case Key_G:
      snprintf(buf, sizeof(buf), "key is %s", "G   "); break;
      case Key_B:
      snprintf(buf, sizeof(buf), "key is %s", "B   "); break;
      default:
      snprintf(buf, sizeof(buf), "key is %s", "NONE"); break;
    }
    ssd1306_basic_string_no_update(0, 36, buf, (uint16_t)strlen(buf), 0, SSD1306_FONT_12);
}

void beep_state()
{
  if(beep_active == SET)
  {
    HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_SET);
    HAL_Delay(300);
    HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_RESET);

    HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_SET);
    HAL_Delay(300);
    HAL_GPIO_WritePin(Beep_GPIO_Port, Beep_Pin, GPIO_PIN_RESET);
    beep_active = RESET;
  }
  
}

void Power_led_state()
{
  if(num==Key_R)
  {
    HAL_GPIO_WritePin(board_led_GPIO_Port, board_led_Pin, GPIO_PIN_SET);
  }
  else
  {
    HAL_GPIO_WritePin(board_led_GPIO_Port, board_led_Pin, GPIO_PIN_RESET);
  }

  if(num==Key_G)
  {
    HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_SET);
  }
  else
  {
    HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_RESET);
  }

  if(num==Key_B)
  {
    HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin, GPIO_PIN_SET);
  }
  else
  {
    HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin, GPIO_PIN_RESET);
  }

}

void fan_state()
{
  if(num==Key_Fan)
  {
    HAL_GPIO_WritePin(FAN_IO_GPIO_Port, FAN_IO_Pin, GPIO_PIN_SET);
  }
  else
  {
    HAL_GPIO_WritePin(FAN_IO_GPIO_Port, FAN_IO_Pin, GPIO_PIN_RESET);
  }
}

void temp_state()
{
  char tbuf[20];
  int16_t val = DS18B20_GetTempTenths();

  if (val == DS18B20_ERROR_VAL)
  {
    snprintf(tbuf, sizeof(tbuf), "TEMP: --.- C");
  }
  else
  {
    uint8_t neg = 0;
    if (val < 0) { neg = 1; val = (int16_t)(-val); }
    uint8_t int_part = (uint8_t)(val / 10);
    uint8_t dec_part = (uint8_t)(val % 10);
    snprintf(tbuf, sizeof(tbuf), "TEMP:%c%d.%d C",
             neg ? '-' : ' ', int_part, dec_part);
  }
  ssd1306_basic_string_no_update(0, 48, tbuf, (uint16_t)strlen(tbuf), 0, SSD1306_FONT_12);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM1)
    {
        led_cnt++;
        temp_tick = 1;                    /* 100 ms temperature update flag */
        if(led_cnt >= 10)
        {
            led_cnt = 0;
            num_cnt++;
        }
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_TIM1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
  HAL_TIM_Base_Start_IT(&htim1);
  ssd1306_basic_init(SSD1306_INTERFACE_IIC, SSD1306_ADDR_SA0_0);
  ssd1306_basic_display_on();
  DS18B20_Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    snprintf(buf, sizeof(buf), "num is %ld", num_cnt);
    ssd1306_basic_string_no_update(0, 0, buf, (uint16_t)strlen(buf), 0, SSD1306_FONT_12);

    HuoEr_state();
    key_state();
    beep_state();
    Power_led_state();
    fan_state();
    moto_state();

    /* ── 100 ms temperature update + display ── */
    if (temp_tick)
    {
        DS18B20_Process();
        temp_tick = 0;
    }
    temp_state();

    /* single GRAM flush per loop — all display writes above use _no_update */
    ssd1306_basic_gram_update();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
