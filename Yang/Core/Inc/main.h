/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
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
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#define FFT_LENGTH 1
#include <stdio.h>
#include "math.h"
#include "stdlib.h"
#include <stdbool.h>
#include "local_it.h"

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
  extern uint8_t mode;   // 1为捕获模式�?��?�测周法�???2为测频法�???3为外部时钟法
  extern uint32_t PAUSE; // 调整高电平持续时�???
  extern uint32_t PERIOD;
  extern uint32_t capture_state;  // 设置捕获模式的状�???
  extern uint8_t mes_uart[20];    // 接收串口数据
  extern uint32_t capture_buf[4]; // 接受捕获到的时间�???
  extern float high_time;
  extern float low_time;
  extern float period_time;
  extern float high_time_old;
  extern float low_time_old;
  extern float period_time_old; // 都是记录测量时间的变�???
  extern uint32_t flag_cnt;     // 测量十次取平均的标志�???
  extern uint32_t new_flag;
  extern uint32_t pwm_val;
  extern uint32_t pwm_flag;
  extern uint16_t adc_value[FFT_LENGTH];
	void adc_start();
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

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
