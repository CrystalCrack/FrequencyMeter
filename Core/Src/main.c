/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
int capture_index = 0;
int capture_buf[3];
char msg[100];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  switch (capture_index)
  {
  case 1:
    capture_buf[0] = HAL_TIM_ReadCapturedValue(&htim4, TIM_CHANNEL_1);
    __HAL_TIM_SET_CAPTUREPOLARITY(&htim4, TIM_CHANNEL_1, TIM_ICPOLARITY_FALLING);
    capture_index++;
    break;
  case 2:
    capture_buf[1] = HAL_TIM_ReadCapturedValue(&htim4, TIM_CHANNEL_1);
    __HAL_TIM_SET_CAPTUREPOLARITY(&htim4, TIM_CHANNEL_1, TIM_ICPOLARITY_RISING);
    capture_index++;
    break;
  case 3:
    capture_buf[2] = HAL_TIM_ReadCapturedValue(&htim4, TIM_CHANNEL_1);
    capture_index++;
    HAL_TIM_IC_Stop_IT(&htim4, TIM_CHANNEL_1);
    break;
  }
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
  unsigned int freq_int = LittleEndianHextoDec(msg, msg + 4);
  unsigned int duty_cycle_rcv = LittleEndianHextoDec(msg + 4, msg + 8);
  float freq = freq_int;
  setPWM(freq, duty_cycle_rcv);
  HAL_UARTEx_ReceiveToIdle_IT(&huart6, msg, 100);
}

void setPWM(float freq, int duty_cycle)
{
  float TimeClockFrequency = (float)HAL_RCC_GetPCLK1Freq();
  TimeClockFrequency *= 2;
  uint16_t prescaler = floor(sqrt(TimeClockFrequency/freq))-1;
  uint16_t ARR_Value = ceil(sqrt(TimeClockFrequency/freq))-1;
  uint16_t CRR_Value = (int)round((float)(ARR_Value+1) * duty_cycle / 100);

  __HAL_TIM_SET_PRESCALER(&htim3, prescaler);
  __HAL_TIM_SET_AUTORELOAD(&htim3, ARR_Value);
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, CRR_Value);
  HAL_TIM_GenerateEvent(&htim3, TIM_EVENTSOURCE_UPDATE);
}

unsigned int LittleEndianHextoDec(uint8_t *start_byte, uint8_t *end_byte)
{
  if (end_byte - start_byte <= 0)
    return 0;
  unsigned int result = 0;
  for (uint8_t *cur = start_byte; cur < end_byte; cur++)
  {
    result |= (unsigned int)(*cur) << 8 * (cur - start_byte);
  }
  return result;
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
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_UARTEx_ReceiveToIdle_IT(&huart6, msg, 100);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, 1200);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    if (capture_index == 4)
    {
      int period = capture_buf[2] - capture_buf[0];
      int high = capture_buf[1] - capture_buf[0];
      float freq = 1e6 / period;
      int duty_cycle = (int)round((double)high * 100 / period);
      char freqmsg[100];
      char dutymsg[100];
      sprintf(freqmsg, "t2.txt=\"Measured Frequency:%.2fHz\"\xFF\xFF\xFF", freq);
      sprintf(dutymsg, "t3.txt=\"Measured Duty Cycle:%d%%\"\xFF\xFF\xFF", duty_cycle);
      HAL_UART_Transmit(&huart6, freqmsg, strlen(freqmsg), 100);
      HAL_UART_Transmit(&huart6, dutymsg, strlen(dutymsg), 100);
      capture_index = 0;
    }
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

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
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

#ifdef  USE_FULL_ASSERT
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
