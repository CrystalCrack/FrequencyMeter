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
#include "measure.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
mode m = BLANK;     // 模式选择
uint8_t rx_get = 3; // 串口接收数据存储
uint8_t flag_wait = 0;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t method = MEASURE_FREQ_METHOD;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_TIM5_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  double freq;
  double gap;
  double high_lasting;
  double duty;
  double period;

  // printf("启动");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  HAL_UART_Receive_IT(&huart1, &rx_get, sizeof(rx_get));
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    switch (m)
    {
    case FREQUENCY:

      // printf("FRE\n");
      switch (method)
      {
      case MEASURE_FREQ_METHOD:
        printf("meth.txt=\"frequency method\"\xff\xff\xff");
        break;
      case MEASURE_PERIOD_HIGH_METHOD:
        printf("meth.txt=\"period method(100-10000Hz)\"\xff\xff\xff");
        break;
      case MEASURE_PERIOD_LOW_METHOD:
        printf("meth.txt=\"period method(1-100Hz)\"\xff\xff\xff");
        break;
      }
      freq = MeasureFreq_main();
      if (freq >= 1e6)
      {
        printf("fre.txt=\"%.6gMHz\"\xff\xff\xff", freq / 1e6);
        // printf("%.6gMHz\n", freq / 1e6);
      }
      else if (freq >= 1e3)
      {
        printf("fre.txt=\"%.6gKHz\"\xff\xff\xff", freq / 1e3);
        // printf("%.6gKHz\n", freq / 1e3);
      }
      else
      {
        printf("fre.txt=\"%.6gHz\"\xff\xff\xff", freq);
        // printf("%.6gHz\n", freq);
      }

      break;
    case GAP:
      if (flag_wait == 0)
      {
        flag_wait = 1;
        printf("itv.txt=\"waiting\"\xff\xff\xff");
      }
      m = FREQUENCY;
      freq = MeasureFreq_main();
      m = GAP;
      gap = MeasureGap();
      period = 1 / freq;
      if (gap > period)
      {
        gap -= (int)(gap / period) * period;
      }
      gap = (period - gap) > gap ? gap : (period - gap);
      // printf("signal interval:\n");
      if (gap <= 1e-6)
      {
        printf("itv.txt=\"%.6gns\"\xff\xff\xff", gap * 1e9);
      }
      else if (gap <= 1e-3)
      {
        printf("itv.txt=\"%.6gus\"\xff\xff\xff", gap * 1e6);
      }
      else if (gap < 1)
      {
        printf("itv.txt=\"%.6gms\"\xff\xff\xff", gap * 1e3);
      }
      else
      {
        printf("itv.txt=\"%.6gs\"\xff\xff\xff", gap);
      }
      HAL_Delay(200);
      break;
    case DUTY:
      m = FREQUENCY;
      freq = MeasureFreq_main();
      m = DUTY;
      high_lasting = MeasureHigh();
      period = 1 / freq;
      if (high_lasting > period)
        high_lasting -= (int)(high_lasting / period) * period;
      duty = (high_lasting / period) * 100;
      printf("%.4g%%", duty);
      break;
    case BLANK:
      // printf("NOT\n");
      break;
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

  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY))
  {
  }

  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY))
  {
  }

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
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
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
/*************串口中断************/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
    // printf("中断");
    flag_wait = 0;
    if (rx_get == 3)
      m = BLANK;
    else if (rx_get == 1)
      m = FREQUENCY;
    else if (rx_get == 2)
      m = GAP;
    else if (rx_get == 4)
      m = DUTY;
    HAL_UART_Receive_IT(&huart1, &rx_get, sizeof(rx_get));
  }
}
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
