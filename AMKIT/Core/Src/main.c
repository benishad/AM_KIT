/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */




/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define RTC_MAGIC_REG   RTC_BKP_DR1     // RTC 백업 레지스터 1번 사용
#define RTC_MAGIC_VALUE 0x32F2          // RTC 초기화 확인을 위한 매직 넘버



/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

SD_HandleTypeDef hsd;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim7;

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

#if 0
// // 전역 변수 선언
// static uint16_t ms_tick_1       = 0;                // 1 ms 카운터


// // STM UART 변수 선언
// static uint16_t alive_counter   = 0;                // 10 s 생존 메시지 카운터
// uint8_t txBuf[]                 = "Hello from STM32 IRQ!\r\n";
// uint8_t rxByte;                                     // 1 바이트 UART RX 버퍼
// uint8_t txAlive[]               = "ALIVE\n";        // 10 s 마다 보낼 메시지


// // ESP32 AT 명령어를 위한 변수 선언
// uint8_t atCmd[]                 = "AT+GMR\r\n";
// // uint8_t g_atRxByte;                                 // 1바이트씩 받기              / 외부 선언 해줬음
// // static char  atLineBuf[AT_RX_BUF_SIZE];             // AT 명령어 수신 버퍼         / 외부 선언 해줬음
// // static uint8_t atIdx            = 0;                // AT 명령어 수신 인덱스       / 외부 선언 해줬음

// // 모드 변수
// uint8_t g_nBoot_Status = BOOT_IN_PROGRESS;            // 부팅 상태 (0: 부팅 성공, 1: 부팅 중, 2: 부팅 실패 등)
// uint8_t g_nMode = 0;                                  // 현재 모드 (0: 마스터, 1: 슬레이브, 2: AP 등)
// uint8_t g_nWifi_Status = DEVICE_WIFI_DISCONNECTED;    // WiFi 연결 상태 (0: 연결 안됨, 1: 연결 됨)
// uint8_t g_nTime_Status = DEVICE_TIME_NOT_SYNCED;      // 시간 동기화 상태 (0: 동기화 안됨, 1: 동기화 됨)
// uint8_t g_nToken_Status = DEVICE_TOKEN_NOT_SET;       // 토큰 상태 (0: 토큰 없음, 1: 토큰 있음)

#endif
// RTC 변수 선언
RTC_TimeTypeDef g_Time;                             // RTC 시간 구조체
RTC_DateTypeDef g_Date;                             // RTC 날짜 구조체


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_RTC_Init(void);
static void MX_SDIO_SD_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM7_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_UART4_Init(void);
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

	// =======================================================================================================
	//    __  __            _____  _   _
	//   |  \/  |    /\    |_   _|| \ | |
	//   | \  / |   /  \     | |  |  \| |
	//   | |\/| |  / /\ \    | |  | . ` |
	//   | |  | | / ____ \  _| |_ | |\  |
	//   |_|  |_|/_/    \_\|_____||_| \_|
	//
	// =======================================================================================================

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
  MX_RTC_Init();
  MX_SDIO_SD_Init();
  MX_SPI1_Init();
  MX_TIM7_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_FATFS_Init();
  MX_UART4_Init();
  /* USER CODE BEGIN 2 */

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // CCM 영역 초기화

  Oper_CCM_Init();
  
  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

  // 타이머 인터럽트 모드로 시작 (TIM7은 이미 PSC/ARR로 1 kHz, 1 ms 인터럽트로 설정되어 있다고 가정)
  if (HAL_TIM_Base_Start_IT(&htim7) != HAL_OK)
  {
    Error_Handler();
  }

  // 1) 콜백 기반 수신을 시작 (1바이트)
  if (HAL_UART_Receive_IT(&huart1, &rxByte, 1) != HAL_OK)
  {
    // 수신 시작 실패 처리
    Error_Handler();
  }

  // SIM uart 콜백 기반 수신 시작
  if (HAL_UART_Receive_IT(&huart3, &rxSIMByte, 1) != HAL_OK)
  {
    // SIM 수신 시작 실패 처리
    Error_Handler();
  }

  // 2) 와이파이 설정 이후에 UART2 인터럽트 활성화 (1바이트씩)
  // if (HAL_UART_Receive_IT(&huart2, &g_atRxByte, 1) != HAL_OK)
  // {
  //   // 수신 시작 실패 처리
  //   Error_Handler();
  // }

  // ──────────────────────────────────────────────────────────────────────────────

  // CCM은 아니지만 기본 초기화
  Oper_Init();

  // ──────────────────────────────────────────────────────────────────────────────
  
  // 딥스위치 체크
  Mode_Check(); // AP 모드 체크 및 설정
  
  // ──────────────────────────────────────────────────────────────────────────────

  Main_System();

  // ──────────────────────────────────────────────────────────────────────────────

  // ──────────────────────────────────────────────────────────────────────────────

  // ──────────────────────────────────────────────────────────────────────────────

  Oper_Boot();

  // AP 모드 인지 확인
  if (g_nMode != MODE_AP)
  {
    // 토큰 값, 모기체 고유 값, 자기체 고유 값 서버로 전송
    Server_Send_Boot();
    // Test_Server_Send_Boot(); // 박과장님 서버로 날림
  }


  g_nBoot_Status = BOOT_SUCCESS; // 부팅 성공 상태로 설정


  // ESP_AT_Send_Command_Sync("AT+CIPMUX=1\r\n");  // 멀티 커넥션 모드 활성화
  // ESP_AT_Send_Command_Sync("AT+CIPSERVER=1,80\r\n"); // TCP 서버 시작 (포트 80)
  
  
  

  

  // ──────────────────────────────────────────────────────────────────────────────



  while (1)
  {
    Master_Proc(); // 메인 프로세스 실행
    

  }
  
  

  // ──────────────────────────────────────────────────────────────────────────────


  // ──────────────────────────────────────────────────────────────────────────────


  // ──────────────────────────────────────────────────────────────────────────────
  

  // ──────────────────────────────────────────────────────────────────────────────




  // dip 스위치 1번 상태 OFF(default) 상태일 때
  if (HAL_GPIO_ReadPin(DIP_1_GPIO_Port, DIP_1_Pin) == GPIO_PIN_RESET)
  {
    g_nMode = MODE_MASTER; // 마스터 모드로 설정
  }
  else
  {
    g_nMode = MODE_SLAVE; // 슬레이브 모드로 설정
  }
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {






#if 0
    switch (g_nMode)
    {
      case MODE_MASTER:
        // 마스터 모드 동작
        // STATUS LED 0.5초 ON, 0.5초 OFF
        HAL_GPIO_WritePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin, GPIO_PIN_SET);  // LED ON
        HAL_Delay(500);  // 0.5초 대기
        HAL_GPIO_WritePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin, GPIO_PIN_RESET); // LED OFF
        HAL_Delay(500);  // 0.5초 대기
        break;
      
      case MODE_SLAVE:
        break; // 슬레이브 모드 동작

      case MODE_AP:
        break;

      case MODE_UNKNOWN:
        break;

      default:
        break;
    }
#endif
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */
  // =======================================================================================================
  //    __  __ __   __           _____  _______  _____            _____         _  _
  //   |  \/  |\ \ / /          |  __ \|__   __|/ ____|          |_   _|       (_)| |
  //   | \  / | \ V /   ______  | |__) |  | |  | |       ______    | |   _ __   _ | |_
  //   | |\/| |  > <   |______| |  _  /   | |  | |      |______|   | |  | '_ \ | || __|
  //   | |  | | / . \           | | \ \   | |  | |____            _| |_ | | | || || |_
  //   |_|  |_|/_/ \_\          |_|  \_\  |_|   \_____|          |_____||_| |_||_| \__|
  //
  //
  // =======================================================================================================

  /* USER CODE END RTC_Init 0 */

  // RTC_TimeTypeDef sTime = {0};
  // RTC_DateTypeDef sDate = {0};
  // RTC_AlarmTypeDef sAlarm = {0};

  /* USER CODE BEGIN RTC_Init 1 */
  // 위 3개 구조체 변수는 사용안함, 주석처리
  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  // RTC 백업 레지스터를 읽어 초기화 여부 확인
  if (HAL_RTCEx_BKUPRead(&hrtc, RTC_MAGIC_REG) != RTC_MAGIC_VALUE)
  {
    // RTC 백업 레지스터가 초기화되지 않았거나 값이 일치하지 않으면 RTC를 초기화
    // RTC 백업 레지스터에 초기화 마크를 기록
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_MAGIC_REG, RTC_MAGIC_VALUE);

    // RTC 시간과 날짜를 초기화
    g_Time.Hours = 0x0;  // 00:00:00
    g_Time.Minutes = 0x0;
    g_Time.Seconds = 0x0;
    g_Time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    g_Time.StoreOperation = RTC_STOREOPERATION_RESET;

    if (HAL_RTC_SetTime(&hrtc, &g_Time, RTC_FORMAT_BCD) != HAL_OK)
    {
      // 초기화 실패 처리
      Error_Handler();
    }
    g_Date.WeekDay = RTC_WEEKDAY_MONDAY;    // 월요일
    g_Date.Month = RTC_MONTH_JANUARY;       // 1월
    g_Date.Date = 0x1;                      // 1일
    g_Date.Year = 0x25;                     // 2025년 (BCD 형식으로 25)

    if (HAL_RTC_SetDate(&hrtc, &g_Date, RTC_FORMAT_BCD) != HAL_OK)
    {
      // 초기화 실패 처리
      Error_Handler();
    }

  }
  else
  {
    // RTC 백업 레지스터가 초기화되어 있으면 현재 시간과 날짜를 읽어옴
    HAL_RTC_GetTime(&hrtc, &g_Time, RTC_FORMAT_BCD);
    HAL_RTC_GetDate(&hrtc, &g_Date, RTC_FORMAT_BCD);
  }

// 아래 코드 사용안함===============================================================================
  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  // sTime.Hours = 0x0;
  // sTime.Minutes = 0x0;
  // sTime.Seconds = 0x0;
  // sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  // sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  // if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  // {
  //   Error_Handler();
  // }
  // sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  // sDate.Month = RTC_MONTH_JANUARY;
  // sDate.Date = 0x1;
  // sDate.Year = 0x0;

  // if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  // {
  //   Error_Handler();
  // }

  // /** Enable the Alarm A
  // */
  // sAlarm.AlarmTime.Hours = 0x0;
  // sAlarm.AlarmTime.Minutes = 0x0;
  // sAlarm.AlarmTime.Seconds = 0x0;
  // sAlarm.AlarmTime.SubSeconds = 0x0;
  // sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  // sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  // sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
  // sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  // sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  // sAlarm.AlarmDateWeekDay = 0x1;
  // sAlarm.Alarm = RTC_ALARM_A;
  // if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
  // {
  //   Error_Handler();
  // }
  /* USER CODE BEGIN RTC_Init 2 */
// 위 코드 사용안함===============================================================================
  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SDIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_SDIO_SD_Init(void)
{

  /* USER CODE BEGIN SDIO_Init 0 */

  /* USER CODE END SDIO_Init 0 */

  /* USER CODE BEGIN SDIO_Init 1 */

  /* USER CODE END SDIO_Init 1 */
  hsd.Instance = SDIO;
  hsd.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
  hsd.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
  hsd.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
  hsd.Init.BusWide = SDIO_BUS_WIDE_1B;
  hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd.Init.ClockDiv = 0;
  /* USER CODE BEGIN SDIO_Init 2 */

  /* USER CODE END SDIO_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 100-1;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 840-1;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, RX_LED_Pin|TX_LED_Pin|STATUS_LED_Pin|M_PWR_KEY_Pin
                          |ESP_EN_Pin|USIM_RESET_Pin|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pins : DIP_3_Pin DIP_4_Pin DIP_1_Pin DIP_2_Pin */
  GPIO_InitStruct.Pin = DIP_3_Pin|DIP_4_Pin|DIP_1_Pin|DIP_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI_CS_Pin */
  GPIO_InitStruct.Pin = SPI_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SPI_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : RX_LED_Pin TX_LED_Pin STATUS_LED_Pin M_PWR_KEY_Pin
                           ESP_EN_Pin USIM_RESET_Pin PE14 PE15 */
  GPIO_InitStruct.Pin = RX_LED_Pin|TX_LED_Pin|STATUS_LED_Pin|M_PWR_KEY_Pin
                          |ESP_EN_Pin|USIM_RESET_Pin|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PD8 PD9 PD10 PD11
                           PD12 PD13 PD14 PD15 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */


// =======================================================================================================
//    _______  _____  __  __       ______
//   |__   __||_   _||  \/  |     |____  |
//      | |     | |  | \  / | ______  / /
//      | |     | |  | |\/| ||______|/ /
//      | |    _| |_ | |  | |       / /
//      |_|   |_____||_|  |_|      /_/
//
//
// =======================================================================================================
/* tim 7 인터럽트 처리부 */
/* TIM7 업데이트 인터럽트 콜백 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM7)
  {
    Timer_Interrupt_Proc();
#if 0
    ms_tick_1++;
    alive_counter++;
    g_nBoot_Tick++; // 부팅 타이머 증가

    if (ms_tick_1 >= 200)         // 200 ms 경과 체크
    {
      ms_tick_1 = 0;
      // HAL_GPIO_TogglePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin);
    }

    if (g_nBoot_Status == BOOT_SUCCESS && alive_counter >= 5000)    // 10 s 경과 체크
    {
      alive_counter = 0;

      /* 1) RTC에서 현재 시간 읽기 */
      // RTC_TimeTypeDef sTime;
      // RTC_DateTypeDef sDate;
      HAL_RTC_GetTime(&hrtc, &g_Time, RTC_FORMAT_BIN);
      HAL_RTC_GetDate(&hrtc, &g_Date, RTC_FORMAT_BIN);

      /* 2) 문자열로 포맷 */
      char buf[64];

      int len = snprintf(buf, sizeof(buf), "ALIVE: %02d.%02d / %02d:%02d:%02d\n",
                        g_Date.Month,g_Date.Date, g_Time.Hours, g_Time.Minutes, g_Time.Seconds);

      // 생존 메시지 전송
      //HAL_UART_Transmit_IT(&huart1, txAlive, sizeof(txAlive) - 1);

      /* 3) UART로 생존 및 시간 전송 */
      // UART1로 현재 시간 전송
      HAL_UART_Transmit_IT(&huart1, (uint8_t *)buf, len);

      // if (HAL_UART_Transmit_IT(&huart2, atCmd, sizeof(atCmd) - 1) != HAL_OK)
      // {
      //   // AT 명령 전송 실패 처리
      //   Error_Handler();
      // }      
    }
#endif
  }
}

// =======================================================================================================
//    _    _           _____  _______  
//   | |  | |   /\    |  __ \|__   __|  
//   | |  | |  /  \   | |__) |  | | 
//   | |  | | / /\ \  |  _  /   | |
//   | |__| |/ ____ \ | | \ \   | | 
//    \____//_/    \_\|_|  \_\  |_| 
//
//
// =======================================================================================================
/* uart 1 처리부 */
/* UART Rx Complete 콜백 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  // uart1에서 수신된 바이트를 처리
  if (huart->Instance == USART1)
  {
    // 1) 에코: 받은 바이트를 바로 송신
    HAL_UART_Transmit_IT(&huart1, &rxByte, 1);

    // 2) 다시 수신 대기
    HAL_UART_Receive_IT(&huart1, &rxByte, 1);
  }

  // uart2에서 수신된 바이트를 처리

  if (huart->Instance == USART2)
  {
    char c = g_atRxByte;

    // 1) 수신 바이트를 라인 버퍼에 저장
    if (atIdx < AT_RX_BUF_SIZE-1)
    {
      atLineBuf[atIdx++] = c;
    }

    // 2) '\n' 이 들어오면 한 줄 완성
    if (c == '\n' || c == '\r')
    {
      atLineBuf[atIdx] = '\0';  // 문자열 종료

      // AT 명령어 처리 로직 (예: AT 명령어 파싱 및 응답)
      // 여기서 atLineBuf를 사용하여 AT 명령어를 처리
      // 받은 명령을 다시 STM uart 송신
      HAL_UART_Transmit_IT(&huart1, (uint8_t *)atLineBuf, atIdx);

      atIdx = 0;  // 인덱스 초기화
    }

    // 2) 다시 수신 대기
    HAL_UART_Receive_IT(&huart2, &g_atRxByte, 1);
  }

  // uart3에서 수신된 바이트를 처리
  if (huart->Instance == USART3)
  {
    // SIM 카드에서 수신된 바이트를 처리
    // 예: SIM 카드 응답을 UART1로 전달
    HAL_UART_Transmit_IT(&huart1, &rxSIMByte, 1);

    // 다시 수신 대기
    HAL_UART_Receive_IT(&huart3, &rxSIMByte, 1);
  }

#if 0
  if (huart->Instance == USART2)
    {
        char c = g_atRxByte;

        // CR(\r)은 무시, LF(\n)으로만 라인 완성 체크
        if (c != '\r')
        {
            if (c == '\n')
            {
                atLineBuf[atIdx] = '\0';  // 문자열 종료

                // 1) 에코(AT… 명령어)인지 체크
                if (skipEcho && strcmp(atLineBuf, lastCmdBuf) == 0)
                {
                    // 에코이므로 무시
                    skipEcho = 0;
                }
                else
                {
                    // 실제 응답 또는 URC → PC로 전송
                    HAL_UART_Transmit_IT(&huart1,
                                         (uint8_t *)atLineBuf,
                                         strlen(atLineBuf));
                    // 필요하면 줄바꿈도 함께
                    HAL_UART_Transmit_IT(&huart1,
                                         (uint8_t *)"\r\n", 2);
                }

                atIdx = 0;  // 다음 라인용 버퍼 인덱스 초기화
            }
            else
            {
                // 일반 문자면 버퍼에 저장
                if (atIdx < AT_RX_BUF_SIZE - 1)
                    atLineBuf[atIdx++] = c;
            }
        }

        // 2) 다시 UART2 수신 대기
        HAL_UART_Receive_IT(&huart2, &g_atRxByte, 1);
    }
#endif

}

/* UART 전송 완료 콜백 (필요 시..) */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  // 전송 완료 후 다른 처리가 필요하면 여기에…
}




// ================================================================================
// ================================================================================

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
