/*
 * main_proc.c
 *
 *  Created on: Jun 27, 2025
 *      Author: DONGYOONLEE
 */

#include "main_proc.h"
#include "main.h"




// 전역 변수 선언
uint16_t ms_tick_1       = 0;                // 1 ms 카운터
uint16_t g_nLed_tick_1       = 0;                // 1 ms 카운터
uint16_t g_nRX_Led_tick_1       = 0;                // 1 ms 카운터

// LED 상태 변수
uint8_t g_nLed_State        = 0;


// STM UART 변수 선언
uint16_t alive_counter   = 0;                // 10 s 생존 메시지 카운터

uint8_t txBuf[]                 = "Hello from STM32 IRQ!\r\n";
uint8_t rxByte;                                     // 1 바이트 UART RX 버퍼
uint8_t txAlive[]               = "ALIVE\n";        // 10 s 마다 보낼 메시지


uint8_t rxSIMByte;                                     // 1 바이트 UART SIM RX 버퍼

// ESP32 AT 명령어를 위한 변수 선언
uint8_t atCmd[]                 = "AT+GMR\r\n";
// uint8_t g_atRxByte;                                 // 1바이트씩 받기              / 외부 선언 해줬음
// static char  atLineBuf[AT_RX_BUF_SIZE];             // AT 명령어 수신 버퍼         / 외부 선언 해줬음
// static uint8_t atIdx            = 0;                // AT 명령어 수신 인덱스       / 외부 선언 해줬음


// 모드 변수
uint8_t g_nBoot_Status = BOOT_IN_PROGRESS;            // 부팅 상태 (0: 부팅 성공, 1: 부팅 중, 2: 부팅 실패 등)
uint8_t g_nMode = 0;                                  // 현재 모드 (0: 마스터, 1: 슬레이브, 2: AP 등)
uint8_t g_nWifi_Status = DEVICE_WIFI_DISCONNECTED;    // WiFi 연결 상태 (0: 연결 안됨, 1: 연결 됨)
uint8_t g_nTime_Status = DEVICE_TIME_NOT_SYNCED;      // 시간 동기화 상태 (0: 동기화 안됨, 1: 동기화 됨)
uint8_t g_nToken_Status = DEVICE_TOKEN_NOT_SET;       // 토큰 상태 (0: 토큰 없음, 1: 토큰 있음)

// ──────────────────────────────────────────────────────────────────────────────

// ──────────────────────────────────────────────────────────────────────────────

void SD_Card_Check(void)
{
    int isSd = SD_Card_Is_Exist(); // SD 카드 존재 여부 확인

    while (isSd == SD_ERROR)
    {
        HAL_GPIO_WritePin(RX_LED_GPIO_Port, RX_LED_Pin, GPIO_PIN_SET);  // LED ON
        HAL_Delay(200);  // 0.5초 대기
        HAL_GPIO_WritePin(RX_LED_GPIO_Port, RX_LED_Pin, GPIO_PIN_RESET); // LED OFF
        HAL_Delay(200);  // 0.5초 대기

        isSd = SD_Card_Is_Exist(); // SD 카드 존재 여부 확인
        if(isSd == SD_OK)
        {
            break; // SD 카드가 존재하면 루프 종료
        }
    }
}


// ──────────────────────────────────────────────────────────────────────────────

void Mode_Check(void)
{
    int mode = Device_Mode_Check(); // 현재 모드 확인

    // Device_Mode_Check 함수에서 모드가 변경되어 반영했으므로 아래는 따로 진행안해도 무관
    switch (mode)
    {
    case MODE_MASTER:
        break;
    case MODE_SLAVE:
        break;
    case MODE_AP:
        break;
    case MODE_DEBUG:
        break;
    case MODE_TEST:
        break;
    
    default:
        break;
    }

    g_nMode = mode; // 현재 모드
    
}
// ──────────────────────────────────────────────────────────────────────────────



// ──────────────────────────────────────────────────────────────────────────────

// AP 모드 프로세스
void AP_Mode_Proc(void)
{
    int step = 0;
    int result = 0;
    int bootLoop = 1;

    // ====================================================================

    // 부팅 동작
    while (bootLoop)
    {
        switch (step)
        {
        case 0:
            result = SD_Card_Boot(); // SD 카드 초기화 및 테스트 / 와이파이 파일 확인
            
            SD_Card_Log("SD Card Boot...\n");
            
            if (result == SD_OK)
            {
                SD_Card_Log("SD Card Booted Successfully!\n");
                step++; // 다음 단계로 이동
            }
            else
            {
                SD_Card_Log("SD Card Boot Failed!\n");
                SD_Card_Log("again...\n");
                // SD 카드 부팅 실패 시 에러 처리
                // Error_Handler();
                Error_Proc(1);
            }
            break;
            // ----------------------
        case 1:
            result = ESP_AT_Boot(); // ESP32 AT 테스트

            SD_Card_Log("ESP32 AT Boot...\n");

            if (result == AT_OK)
            {
                SD_Card_Log("ESP32 AT Booted Successfully!\n");
                step++; // 다음 단계로 이동
            }
            else
            {
                SD_Card_Log("ESP32 AT Boot Failed!\n");
                SD_Card_Log("again...\n");
                // ESP32 AT 부팅 실패 시 에러 처리
                //Error_Handler();
                Error_Proc(1);
            }
            break;
            // ----------------------
        case 2:
            bootLoop = 0; // 부팅 루프 종료
            break;
        
        default:
            break;
        }
    }
    


    // AP 모드일 때의 동작 처리
    // if (g_nMode == MODE_AP)
    // {
    //     ESP_AT_Server_Init(); // ESP32 AT 서버 초기화

    //     ESP_AP_Server(); // ESP32 AP 서버 시작
    // }

    // 와이파이 드라이버 초기화
    ESP_AT_Send_Command_Sync("AT+CWINIT=1\r\n");

    // softAP 모드로 설정
    ESP_AT_Send_Command_Sync("AT+CWMODE=3\r\n");
    
    // AP SSID와 비밀번호 설정
    // ESP_AT_Send_Command_Sync("AT+CWSAP=\"AMKIT_AP\",\"12345678\",5,3\r\n");
    ESP_AT_Send_Command_Sync("AT+CWSAP=\"AMKIT_AP\",\"\",5,0\r\n");

    // SoftAP DHCP 서버 활성화
    ESP_AT_Send_Command_Sync("AT+CWDHCP=1,2\r\n");

    // SoftAP IP 주소 설정
    ESP_AT_Send_Command_Sync("AT+CIPAP=\"192.168.4.1\",\"255.255.255.0\"\r\n");

    // 멀티플 커넥션 모드 설정
    ESP_AT_Send_Command_Sync("AT+CIPMUX=1\r\n");

    // HTTP 서버 시작
    ESP_AT_Send_Command_Sync("AT+CIPSERVER=1,80\r\n");

    Handle_IPD_and_Respond(); // 클라이언트 요청 처리

    while (1)
    {
        
    }
    
}






// ──────────────────────────────────────────────────────────────────────────────
// 파라미터 0 : Error_Handler 호출
// 파라미터 !0 : uart1 코맨트 전송
void Error_Proc(int errorCode)
{
    const char *comment = "Check Log.txt!\r\n";

    switch (errorCode)
    {
    case 0:
        Error_Handler();
        break;
    case 1:
        HAL_UART_Transmit(&huart1, (uint8_t*)comment, strlen(comment), HAL_MAX_DELAY);
        break;
    
    default:
        HAL_UART_Transmit(&huart1, (uint8_t*)comment, strlen(comment), HAL_MAX_DELAY);
        break;
    }
    
}

// ──────────────────────────────────────────────────────────────────────────────

void Master_Proc(void) 
{
    switch (g_nBoot_Step)
    {
    case 0:
            
      break;
    
    default:
      break;
    }

    SD_Card_Check(); // SD 카드 존재 여부 확인
}

// ─────────────────────────────────────────────────────────────────────────────

void DEBUG_Proc(void)
{
    // 디버그 모드인걸 uart로 알리기
    const char *debugMsg = "DEBUG MODE ACTIVE!\r\n";
    HAL_UART_Transmit(&huart1, (uint8_t*)debugMsg, strlen(debugMsg), HAL_MAX_DELAY);

    // SIM_Init(); // SIM 모듈 초기화, 앞에서 이미 했음

    // SIM 모듈 전원 끄기
    // HAL_TICK 사용
    uint32_t start = HAL_GetTick();
    uint8_t  ch;
    size_t   idx = 0;
#if 0
    // 펄스로 떨구기
    start = HAL_GetTick();
    while ((HAL_GetTick() - start) < 200)
    {   
        SIM_USIM_RESET_Set();
    }

    start = HAL_GetTick();
    while ((HAL_GetTick() - start) < 200)
    {   
        SIM_USIM_RESET_Clear();
    }
#endif
    // // SIM 모듈 상태 OFF 1초
    // HAL_UART_Transmit(&huart1, (uint8_t*)"SIM OFF!\r\n", 11, HAL_MAX_DELAY);
    // start = HAL_GetTick();
    // while ((HAL_GetTick() - start) < 100)
    // {   
    //     SIM_PWR_ON();  // OFF 해야 HIGH 신호 발생, 오실로스코프 찍어보셈
    // }

    start = HAL_GetTick();
    while ((HAL_GetTick() - start) < 200)
    {
        SIM_PWR_OFF();   // ON 해야 LOW 신호 발생
    }
    // SIM 모듈 상태 ON 1.1초
    HAL_UART_Transmit(&huart1, (uint8_t*)"SIM ON!\r\n", 10, HAL_MAX_DELAY);
    start = HAL_GetTick();
    while ((HAL_GetTick() - start) < 200)
    {
        SIM_PWR_ON();   // ON 해야 LOW 신호 발생
    }
    SIM_PWR_OFF();   // ON 해야 LOW 신호 발생
    

    // // SIM 모듈 다시 OFF
    // HAL_UART_Transmit(&huart1, (uint8_t*)"SIM OFF!\r\n", 11, HAL_MAX_DELAY);
    // start = HAL_GetTick();
    // while ((HAL_GetTick() - start) < 100)
    // {
    //     SIM_PWR_ON();  // OFF 해야 HIGH 신호 발생
    // }
#if 0
    HAL_UART_Transmit(&huart1, (uint8_t*)"NOW BUTTON!\r\n", 14, HAL_MAX_DELAY);
    start = HAL_GetTick();
    while ((HAL_GetTick() - start) < 3000)
    {
        
    }
    HAL_UART_Transmit(&huart1, (uint8_t*)"????\r\n", 7, HAL_MAX_DELAY);
#endif

#if 0
    if (SIM_AT_WaitReady(15000))
    {
        HAL_UART_Transmit(&huart1, (uint8_t*)"SIM READY!\r\n", 13, HAL_MAX_DELAY);
    }
    else
    {
        HAL_UART_Transmit(&huart1, (uint8_t*)"SIM NOT READY!\r\n", 17, HAL_MAX_DELAY);

        SIM_AT_Send_Command_Sync_Get_Result("AT\r\n");
    }
#endif
    // uart 3에서 들어오는 값 확인
    char resp[256];
    size_t n;
    n = SIM_UART_ReadData(resp, sizeof(resp), 10000, 1000); // 10초 전체 대기, 바이트당 1000ms 대기

    if (n > 0)
    {
        resp[n] = '\0'; // 문자열 종료
        HAL_UART_Transmit(&huart1, (uint8_t*)resp, n, HAL_MAX_DELAY); // uart1로 전송
    }
    else
    {
        HAL_UART_Transmit(&huart1, (uint8_t*)"No data received!\r\n", 20, HAL_MAX_DELAY);
    }

    
    while (1)
    {

    }
    

    while (1)
    {
        // SIM AT 명령 확인
        SIM_AT_Send_Command_Sync_Get_Result("AT\r\n");

        
        // 인터럽트 uart3 송신
        // HAL_UART_Transmit(&huart3, (uint8_t*)"AT\r", 3, HAL_MAX_DELAY);
       
        // HAL_UART_Transmit(&huart3, (uint8_t*)"AT\r\n", 11, HAL_MAX_DELAY);
        // // 어떤 명령을 보냈는지 PC로 전송
        // HAL_UART_Transmit(&huart1, (uint8_t*)"AT\r\n", 11, HAL_MAX_DELAY);

        // // SIM 모듈 PIN 상태 확인 CPIN 명령 전송 인터럽트 기반
        // HAL_UART_Transmit(&huart3, (uint8_t*)"AT+CPIN?\r\n", 11, HAL_MAX_DELAY);
        // // 어떤 명령을 보냈는지 PC로 전송
        // HAL_UART_Transmit(&huart1, (uint8_t*)"AT+CPIN?\r\n", 11, HAL_MAX_DELAY);


        // 루프 종료
        HAL_UART_Transmit(&huart1, (uint8_t*)"DEBUG MODE END!\r\n", 18, HAL_MAX_DELAY);

        break;
    }

    while (1)
    {
        // 루프 종료
        // HAL_UART_Transmit(&huart1, (uint8_t*)"DEBUG MODE!\r\n", 18, HAL_MAX_DELAY);

        
    }
    
    
}


// ─────────────────────────────────────────────────────────────────────────────

void Test_Proc(void)
{
    // 테스트 모드 프로세스
    // 마스터 시퀀스 테스트
    // 순서 ESP AP 모드열어서 휴대폰으로 WIFI 접속
    // 웹사이트 접근
    
    int step = 0;
    int result = 0;
    int bootLoop = 1;
 
    const char *testMsg = "TEST MODE ACTIVE!\r\n";
    HAL_UART_Transmit(&huart1, (uint8_t*)testMsg, strlen(testMsg), HAL_MAX_DELAY);

    // 테스트 모드 부팅 동작
    while (bootLoop)
    {
        switch (step)
        {
        case 0:
            result = SD_Card_Boot(); // SD 카드 초기화 및 테스트 / 와이파이 파일 확인
            
            SD_Card_Log("SD Card Boot...\n");
            
            if (result == SD_OK)
            {
                SD_Card_Log("SD Card Booted Successfully!\n");
                testMsg = "SD Card Booted Successfully!\n";
                HAL_UART_Transmit(&huart1, (uint8_t*)testMsg, strlen(testMsg), HAL_MAX_DELAY);
                step++; // 다음 단계로 이동
            }
            else
            {
                SD_Card_Log("SD Card Boot Failed!\n");
                SD_Card_Log("again...\n");
                // SD 카드 부팅 실패 시 에러 처리
                // Error_Handler();
                // Error_Proc(1);
            }
            break;
            // ----------------------
        case 1:
            result = ESP_AT_Boot(); // ESP32 AT 테스트

            SD_Card_Log("ESP32 AT Boot...\n");

            if (result == AT_OK)
            {
                SD_Card_Log("ESP32 AT Booted Successfully!\n");
                testMsg = "ESP32 AT Booted Successfully!\n";
                HAL_UART_Transmit(&huart1, (uint8_t*)testMsg, strlen(testMsg), HAL_MAX_DELAY);
                step++; // 다음 단계로 이동
            }
            else
            {
                SD_Card_Log("ESP32 AT Boot Failed!\n");
                SD_Card_Log("again...\n");
                // ESP32 AT 부팅 실패 시 에러 처리
                //Error_Handler();
                // Error_Proc(1);
            }
            break;
            // ----------------------
        case 2:
            // 와이파이 드라이버 초기화
            result = ESP_AT_Send_Command_Sync_Get_int("AT+CWINIT=1\r\n");

            SD_Card_Log("ESP32 WiFi Init...\n");

            if (result == AT_OK)
            {
                SD_Card_Log("ESP32 WiFi Init Success!\n");
                testMsg = "ESP32 WiFi Init Success!\n";
                HAL_UART_Transmit(&huart1, (uint8_t*)testMsg, strlen(testMsg), HAL_MAX_DELAY);
                step++; // 다음 단계로 이동
            }
            else
            {
                SD_Card_Log("ESP32 WiFi Init Failed!\n");
                SD_Card_Log("again...\n");
                // ESP32 WiFi 초기화 실패 시 에러 처리
                // Error_Proc(1);
            }
            break;
            // ----------------------
        case 3:
            // softAP 모드로 설정
            result = ESP_AT_Send_Command_Sync_Get_int("AT+CWMODE=2\r\n");
            SD_Card_Log("ESP32 Set SoftAP Mode...\n");
            if (result == AT_OK)
            {
                SD_Card_Log("ESP32 Set SoftAP Mode Success!\n");
                testMsg = "ESP32 Set SoftAP Mode Success!\n";
                HAL_UART_Transmit(&huart1, (uint8_t*)testMsg, strlen(testMsg), HAL_MAX_DELAY);
                step++; // 다음 단계로 이동
            }
            else
            {
                SD_Card_Log("ESP32 Set SoftAP Mode Failed!\n");
                SD_Card_Log("again...\n");
                // ESP32 SoftAP 모드 설정 실패 시 에러 처리
                // Error_Proc(1);
            }
            break;
            // ----------------------
        case 4:
            // AP SSID와 비밀번호 설정
            result = ESP_AT_Send_Command_Sync_Get_int("AT+CWSAP=\"AMKIT_AP\",\"\",5,0\r\n");
            SD_Card_Log("ESP32 Set SoftAP SSID and Password...\n");
            if (result == AT_OK)
            {
                SD_Card_Log("ESP32 Set SoftAP SSID and Password Success!\n");
                testMsg = "ESP32 Set SoftAP SSID and Password Success!\n";
                HAL_UART_Transmit(&huart1, (uint8_t*)testMsg, strlen(testMsg), HAL_MAX_DELAY);
                step++; // 다음 단계로 이동
            }
            else
            {
                SD_Card_Log("ESP32 Set SoftAP SSID and Password Failed!\n");
                SD_Card_Log("again...\n");
                // ESP32 SoftAP SSID와 비밀번호 설정 실패 시 에러 처리
                // Error_Proc(1);
            }
            break;
            // ----------------------
        case 5:
            // SoftAP DHCP 서버 활성화
            result = ESP_AT_Send_Command_Sync_Get_int("AT+CWDHCP=1,2\r\n");
            SD_Card_Log("ESP32 Enable SoftAP DHCP Server...\n");
            if (result == AT_OK)
            {
                SD_Card_Log("ESP32 Enable SoftAP DHCP Server Success!\n");
                testMsg = "ESP32 Enable SoftAP DHCP Server Success!\n";
                HAL_UART_Transmit(&huart1, (uint8_t*)testMsg, strlen(testMsg), HAL_MAX_DELAY);
                step++; // 다음 단계로 이동
            }
            else
            {
                SD_Card_Log("ESP32 Enable SoftAP DHCP Server Failed!\n");
                SD_Card_Log("again...\n");
                // ESP32 SoftAP DHCP 서버 활성화 실패 시 에러 처리
                // Error_Proc(1);
            }
            break;
            // ----------------------
        case 6:
        #if 0
            // SoftAP IP 주소 조회
            result = ESP_AT_Send_Command_Sync_Get_int("AT+CIPAP?\r\n");
            SD_Card_Log("ESP32 Get SoftAP IP Address...\n");
            if (result == AT_OK)
            {
                SD_Card_Log("ESP32 Get SoftAP IP Address Success!\n");
                testMsg = "ESP32 Get SoftAP IP Address Success!\n";
                HAL_UART_Transmit(&huart1, (uint8_t*)testMsg, strlen(testMsg), HAL_MAX_DELAY);
                step++; // 다음 단계로 이동
            }
            else
            {
                SD_Card_Log("ESP32 Get SoftAP IP Address Failed!\n");
                SD_Card_Log("again...\n");
                // ESP32 SoftAP IP 주소 조회 실패 시 에러 처리
                Error_Proc(1);
            }
        #else
            // SoftAP IP 주소 설정
            result = ESP_AT_Send_Command_Sync_Get_int("AT+CIPAP=\"192.168.4.1\",\"255.255.255.0\"\r\n");
            SD_Card_Log("ESP32 Set SoftAP IP Address...\n");
            if (result == AT_OK)
            {
                SD_Card_Log("ESP32 Set SoftAP IP Address Success!\n");
                testMsg = "ESP32 Set SoftAP IP Address Success!\n";
                HAL_UART_Transmit(&huart1, (uint8_t*)testMsg, strlen(testMsg), HAL_MAX_DELAY);
                step++; // 다음 단계로 이동
            }
            else
            {
                SD_Card_Log("ESP32 Set SoftAP IP Address Failed!\n");
                SD_Card_Log("again...\n");
                // ESP32 SoftAP IP 주소 설정 실패 시 에러 처리
                // Error_Proc(1);
            }
        #endif
            break;
            // ----------------------
        case 7:
            // 멀티플 커넥션 모드 설정
            result = ESP_AT_Send_Command_Sync_Get_int("AT+CIPMUX=1\r\n");
            SD_Card_Log("ESP32 Set Multiple Connection Mode...\n");
            if (result == AT_OK)
            {
                SD_Card_Log("ESP32 Set Multiple Connection Mode Success!\n");
                testMsg = "ESP32 Set Multiple Connection Mode Success!\n";
                HAL_UART_Transmit(&huart1, (uint8_t*)testMsg, strlen(testMsg), HAL_MAX_DELAY);
                step++; // 다음 단계로 이동
            }
            else
            {
                SD_Card_Log("ESP32 Set Multiple Connection Mode Failed!\n");
                SD_Card_Log("again...\n");
                // ESP32 멀티플 커넥션 모드 설정 실패 시 에러 처리
                // Error_Proc(1);
            }
            break;
            // ----------------------
        case 8:
            // HTTP 서버 시작
            result = ESP_AT_Send_Command_Sync_Get_int("AT+CIPSERVER=1,80\r\n");
            SD_Card_Log("ESP32 Start HTTP Server...\n");
            if (result == AT_OK)
            {
                SD_Card_Log("ESP32 Start HTTP Server Success!\n");
                testMsg = "ESP32 Start HTTP Server Success!\n";
                HAL_UART_Transmit(&huart1, (uint8_t*)testMsg, strlen(testMsg), HAL_MAX_DELAY);
                step++; // 다음 단계로 이동
            }
            else
            {
                SD_Card_Log("ESP32 Start HTTP Server Failed!\n");
                SD_Card_Log("again...\n");
                // ESP32 HTTP 서버 시작 실패 시 에러 처리
                // Error_Proc(1);
            }
            break;
            // ----------------------
        case 9:
            Handle_IPD_and_Respond_4(); // 클라이언트 요청 처리
            break;

        case 22:
            bootLoop = 0; // 부팅 루프 종료
            break;
        
        default:
            break;
        }
    }
    
}



// ─────────────────────────────────────────────────────────────────────────────


void Timer_Interrupt_Proc(void)
{
    int rxTimeVal_master    = 100; // RX LED 깜빡임 시간 (ms 단위)
    int rxTimeVal_slave     = 120; // RX LED 깜빡임 시간 (ms 단위)
    ms_tick_1++;

    g_nLed_tick_1++; // LED 관련 1 ms 카운터
    
    g_nRX_Led_tick_1++; // RX LED 관련 1 ms 카운터

    alive_counter++;
    g_nBoot_Tick++; // 부팅 타이머 증가

    if (ms_tick_1 >= 200)         // 200 ms 경과 체크
    {
      ms_tick_1 = 0;
      // HAL_GPIO_TogglePin(STATUS_LED_GPIO_Port, STATUS_LED_Pin);
    }

    // ====================================================================
    // 10초마다 생존 메시지 전송
    if (g_nBoot_Status == BOOT_SUCCESS && alive_counter >= 15000)    // 10 s 경과 체크
    {
      alive_counter = 0;

      /* 1) RTC에서 현재 시간 읽기 */
      HAL_RTC_GetTime(&hrtc, &g_Time, RTC_FORMAT_BIN);
      HAL_RTC_GetDate(&hrtc, &g_Date, RTC_FORMAT_BIN);

      /* 2) 문자열로 포맷 */
      char buf[64];

      int len = snprintf(buf, sizeof(buf), "ALIVE: %02d.%02d / %02d:%02d:%02d\n",
                        g_Date.Month,g_Date.Date, g_Time.Hours, g_Time.Minutes, g_Time.Seconds);

      /* 3) UART로 생존 및 시간 전송 */
      // UART1로 현재 시간 전송
      HAL_UART_Transmit_IT(&huart1, (uint8_t *)buf, len);
    }

    // ====================================================================
    // 100 ms마다 Status LED 상태 변경
    // 비트연산할 변수하나 만들고 NOT 연산으로 LED 상태 변경
    // 예시: g_nLed_tick_1 = ~g_nLed_tick_1
    // 100 ms마다 LED 상태 변경
    if (g_nMode != MODE_AP) // AP 모드가 아닐 때만 LED 상태 변경
    {
        if (g_nLed_tick_1 >= 100) // 100 ms 경과
        {
            g_nLed_tick_1 = 0; // 카운터 초기화
            g_nLed_State = !g_nLed_State; // LED 상태 토글

            if (g_nLed_State)
            {
                STATUS_LED_On();  // LED ON
            }
            else
            {
                STATUS_LED_Off(); // LED OFF
            }
        }
    

        // ====================================================================
        if (g_nBoot_Status == BOOT_SUCCESS)
        {
            // 마스터 모드, 모기체
            if ((DIP_1_GPIO_Port->IDR & DIP_1_Pin) == 0)
            {
                // IDR 레지스터의 해당 핀이 0이면 (접지되어 있으면) MODE_MASTER
                g_nMode = MODE_MASTER;
                // RX_LED_On();  // LED ON
                // TX_LED_Off();  // LED OFF
            }
            else    // 슬레이브 모드, 자기체
            {
                // 핀이 1이면 (풀업되어 있으면) 다른 모드로…
                g_nMode = MODE_SLAVE;  // 예시
                // TX_LED_On();  // LED ON
                // RX_LED_Off();  // LED OFF
            }
        }


        // 부팅 성공 이후부터 상태 LED표시 
        // 마스터 두번씩 깜빡임
        // 슬레이브 세번씩 깜빡임
        if (g_nBoot_Status == BOOT_SUCCESS)
        {
            if (g_nMode == MODE_MASTER)
            {
                // 마스터 모드일 때 LED 2번 깜빡임
                if (g_nRX_Led_tick_1 == rxTimeVal_master)
                {
                    RX_LED_On();
                }
                else if (g_nRX_Led_tick_1 == rxTimeVal_master*2)
                {
                    RX_LED_Off();
                }
                else if (g_nRX_Led_tick_1 == rxTimeVal_master*3)
                {
                    RX_LED_On();
                }
                else if (g_nRX_Led_tick_1 == rxTimeVal_master*4)
                {
                    RX_LED_Off();
                }
                else if (g_nRX_Led_tick_1 >= 1000)
                {
                    RX_LED_Off();
                    g_nRX_Led_tick_1 = 0; // 카운터 초기화
                }
            }
            if (g_nMode == MODE_SLAVE)
            {
                // 슬레이브 모드일 때 LED 3번 깜빡임
                if (g_nRX_Led_tick_1 == rxTimeVal_slave)
                {
                    RX_LED_On();
                }
                else if (g_nRX_Led_tick_1 == rxTimeVal_slave*2)
                {
                    RX_LED_Off();
                }
                else if (g_nRX_Led_tick_1 == rxTimeVal_slave*3)
                {
                    RX_LED_On();
                }
                else if (g_nRX_Led_tick_1 == rxTimeVal_slave*4)
                {
                    RX_LED_Off();
                }
                else if (g_nRX_Led_tick_1 == rxTimeVal_slave*5)
                {
                    RX_LED_On();
                }
                else if (g_nRX_Led_tick_1 == rxTimeVal_slave*6)
                {
                    RX_LED_Off();
                }
                else if (g_nRX_Led_tick_1 >= 1500)
                {
                    RX_LED_Off();
                    g_nRX_Led_tick_1 = 0; // 카운터 초기화
                }
            }
        }
    
    }
    else
    {
        RX_LED_On();
        TX_LED_On();
        STATUS_LED_On();
    }


    
}


// ─────────────────────────────────────────────────────────────────────────────
// ─────────────────────────────────────────────────────────────────────────────
// ─────────────────────────────────────────────────────────────────────────────

void Main_System(void)
{
    switch (g_nMode)
    {
    case MODE_MASTER:
        Master_Proc();
        break;
    case MODE_SLAVE:
        break;
    case MODE_AP:
        AP_Mode_Proc();
        break;
    case MODE_DEBUG:
        DEBUG_Proc();
        break;
    case MODE_TEST:
        // 테스트 모드 동작
        Test_Proc();
        break;
    
    default:
        break;
    }
}