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
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>

#include "sd_card.h"
#include "esp32_at.h"
#include "server_api.h"

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;



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
#define DIP_3_Pin GPIO_PIN_2
#define DIP_3_GPIO_Port GPIOE
#define DIP_4_Pin GPIO_PIN_3
#define DIP_4_GPIO_Port GPIOE
#define ESP_TX_Pin GPIO_PIN_2
#define ESP_TX_GPIO_Port GPIOA
#define ESP_RX_Pin GPIO_PIN_3
#define ESP_RX_GPIO_Port GPIOA
#define RX_LED_Pin GPIO_PIN_8
#define RX_LED_GPIO_Port GPIOE
#define TX_LED_Pin GPIO_PIN_9
#define TX_LED_GPIO_Port GPIOE
#define STATUS_LED_Pin GPIO_PIN_10
#define STATUS_LED_GPIO_Port GPIOE
#define M_PWR_KEY_Pin GPIO_PIN_11
#define M_PWR_KEY_GPIO_Port GPIOE
#define ESP_EN_Pin GPIO_PIN_12
#define ESP_EN_GPIO_Port GPIOE
#define USIM_RESET_Pin GPIO_PIN_13
#define USIM_RESET_GPIO_Port GPIOE
#define SIM_TX_Pin GPIO_PIN_10
#define SIM_TX_GPIO_Port GPIOB
#define SIM_RX_Pin GPIO_PIN_11
#define SIM_RX_GPIO_Port GPIOB
#define UART_TX_Pin GPIO_PIN_9
#define UART_TX_GPIO_Port GPIOA
#define UART_RX_Pin GPIO_PIN_10
#define UART_RX_GPIO_Port GPIOA
#define DIP_1_Pin GPIO_PIN_0
#define DIP_1_GPIO_Port GPIOE
#define DIP_2_Pin GPIO_PIN_1
#define DIP_2_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */


#ifdef WIN32

	#define		__CCMRAM__

#else

	#ifdef STM32F407xx

		//	64k 사용 가능
		//	dma 를 거치지 않고 cpu 에서 제어 가능
		#define		__CCMRAM__								__attribute__( ( section( ".ccmram" ) ) )

	#else

		#define		__CCMRAM__

	#endif

#endif


// 기기 상태 모드
enum DeviceMode {
    MODE_MASTER = 0,          // 마스터 모드
    MODE_SLAVE,               // 슬레이브 모드
    MODE_AP,                  // AP 모드
    MODE_ERROR,               // 에러 모드
    MODE_MAINTENANCE,          // 유지보수 모드
    MODE_UNKNOWN              // 알 수 없는 모드
  
};



/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
