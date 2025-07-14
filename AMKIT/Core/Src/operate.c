/*
 * operate.c
 *
 *  Created on: Jun 25, 2025
 *      Author: DONGYOONLEE
 */


#include "operate.h"
#include "main.h"



uint8_t g_nMac_Status = DEVICE_MAC_NOT_SET;           // MAC 주소 상태 (0: MAC 주소 없음, 1: MAC 주소 있음)

int g_nBoot_Tick = 0;
int g_nBoot_Step = 0;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//     _____ _____ __  __      _____ _   _ _____ _______ 
//    / ____/ ____|  \/  |    |_   _| \ | |_   _|__   __|
//   | |   | |    | \  / |______| | |  \| | | |    | |   
//   | |   | |    | |\/| |______| | | . ` | | |    | |   
//   | |___| |____| |  | |     _| |_| |\  |_| |_   | |   
//    \_____\_____|_|  |_|    |_____|_| \_|_____|  |_|   
//                                                       
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// CCM 영역 초기화 함수
void Oper_CCM_Init(void)
{
    UTC_Time_Init();      // UTC 시간 초기화
    SERVER_API_Init();    // 서버 API 초기화
    DEVICE_Init();        // 디바이스 초기화
}


// ──────────────────────────────────────────────────────────────────────────────
// ──────────────────────────────────────────────────────────────────────────────
// ──────────────────────────────────────────────────────────────────────────────



// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//    ____           _____ _____ _____    _____ _   _ _____ _______ 
//   |  _ \   /\    / ____|_   _/ ____|  |_   _| \ | |_   _|__   __|
//   | |_) | /  \  | (___   | || |   ______| | |  \| | | |    | |   
//   |  _ < / /\ \  \___ \  | || |  |______| | | . ` | | |    | |   
//   | |_) / ____ \ ____) |_| || |____    _| |_| |\  |_| |_   | |   
//   |____/_/    \_\_____/|_____\_____|  |_____|_| \_|_____|  |_|   
//                                                                                                                         
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// 기본 초기화
void Oper_Init(void)
{
    FRAM_Init(); // FRAM 초기화
    // 부팅 상태 초기화
    g_nBoot_Status = BOOT_IN_PROGRESS; // 부팅 상태를 부팅 중으로 설정

    LED_Init(); // LED 초기화
}


// ──────────────────────────────────────────────────────────────────────────────
// ──────────────────────────────────────────────────────────────────────────────
// ──────────────────────────────────────────────────────────────────────────────

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//    ____   ____   ____ _______ 
//   |  _ \ / __ \ / __ \__   __|
//   | |_) | |  | | |  | | | |   
//   |  _ <| |  | | |  | | | |   
//   | |_) | |__| | |__| | | |   
//   |____/ \____/ \____/  |_|   
//                               
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// 부팅 시퀀스
// 0. SD 카드 초기화
// 1. AT 명령 테스트
// 2. AT 펌웨어 조회
// 3. wifi 연결
// 4. UTC시간 서버 연결
// 5. RTC 시간 동기화
// 6. 토큰 요청 및 반환
// 7. 기기 MAC 주소 조회 (기기 고유값)
void Oper_Boot(void)
{
    int bootLoop = 1;
    int step = 0;
    int result = 0;
    const char *token;
    const char *macAddress;

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
            // ====================================================================
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

        case 2:
            result = ESP_AT_Get_Firmware_Version(); // ESP32 AT 명령어로 펌웨어 버전 조회
            
            SD_Card_Log("ESP32 AT Firmware Version Retrieval...\n");

            if (result == AT_OK)
            {
                SD_Card_Log("ESP32 AT Firmware Version Retrieved Successfully!\n");
                step++; // 다음 단계로 이동
            }
            else
            {
                SD_Card_Log("ESP32 AT Firmware Version Retrieval Failed!\n");
                SD_Card_Log("again...\n");
                // ESP32 AT 펌웨어 버전 조회 실패 시 에러 처리
                // Error_Handler();
                Error_Proc(1);
            }
            break;
            // ====================================================================
        case 3:
            // 현재 모드가 AP 모드인지 확인
            if (g_nMode == MODE_AP) // AP 모드인 경우
            {
                SD_Card_Log("AP Mode Detected. Skipping BOOT Configuration...\n");
                step = 0;
                bootLoop = 0; // 부팅 루프 종료
                break; // WiFi 설정 단계 건너뛰기
            }
            
            // Save_Wifi_Status_FRAM_Dummy(); // wifi 상태 초기화
            Load_Wifi_Status_FRAM(); // FRAM에서 WiFi 상태 로드

            g_nWifi_Status = DEVICE_WIFI_DISCONNECTED; // WiFi 연결 상태 초기화

            SD_Card_Log("WiFi Configuration...\n");

            if (g_nWifi_Status == DEVICE_WIFI_DISCONNECTED) // WiFi가 연결되지 않은 경우
            {
                // ESP32 AT 명령어를 통해 현재 WiFi 모드 조회
                ESP_AT_Send_Command_Sync("AT+CWMODE?\r\n");

                // ESP32 AT 명령어를 통해 WiFi 모드 설정 스테이션 모드 + AP모드
                ESP_AT_Send_Command_Sync("AT+CWMODE=3\r\n");

                // AP 모드 옵션 설정
                // <SSID>: 원하는 네트워크 이름 (최대 32자)
                // <PASSWORD>: 비밀번호 (8~64자), "" 로 두면 오픈(암호화 없음)
                // <CHANNEL>: 1~13 (국가 설정에 따라 제한) 거의 1,5,6,11 사용 12,13 미국 금지
                // <ENCRYPTION>:
                // 0 = OPEN
                // 1 = WEP
                // 2 = WPA_PSK
                // 3 = WPA2_PSK
                // 4 = WPA_WPA2_PSK
                ESP_AT_Send_Command_Sync("AT+CWSAP=\"AMKIT\",\"\",5,0\r\n");

                // 연결 가능한 WiFi AP 목록 조회
                ESP_AT_Send_Command_Sync("AT+CWLAP\r\n");

                result = ESP_AT_Send_WiFi_Config(); // WiFi 설정 전송

                if (result == AT_OK) // WiFi 설정 전송 성공
                {
                    g_nWifi_Status = DEVICE_WIFI_CONNECTED; // WiFi 연결 상태 업데이트
                }
                else
                {
                    g_nWifi_Status = DEVICE_WIFI_DISCONNECTED; // WiFi 연결 상태 업데이트 실패
                }
            
                Save_Wifi_Status_FRAM(); // FRAM에 WiFi 상태 저장

                result = AT_OK; // WiFi 설정 성공
            }
            if (g_nWifi_Status == DEVICE_WIFI_CONNECTED) // WiFi가 연결된 경우
            {
                // ESP32 AT 명령어를 통해 현재 WiFi IP 주소 조회
                ESP_AT_Send_Command_Sync("AT+CIFSR\r\n");
                result = AT_OK; // WiFi 연결 성공
            } 
            else // WiFi 연결 실패 시
            {
                result = AT_ERROR; // WiFi 연결 실패
            }

            if (result == AT_OK)
            {
                SD_Card_Log("WiFi Configuration Sent Successfully!\n");

                HAL_UART_Transmit(&huart1, (uint8_t*)"WiFi Configuration Sent Successfully!\n", 40, HAL_MAX_DELAY);
                
                step++; // 다음 단계로 이동
                // step++; // 다음 단계로 이동?
            }
            else
            {
                SD_Card_Log("WiFi Configuration Failed!\n");
                SD_Card_Log("again...\n");
                // WiFi 설정 실패 시 에러 처리
                // Error_Handler();
                Error_Proc(1);
            }
            break;
            // ====================================================================
        case 4:
            // Save_TimeStatus_FRAM_Dummy();  // 시간 상태 초기화
            // SNTP 서버 연결 및 시간 구조 저장
            Load_TimeStatus_FRAM(); // FRAM에서 시간 동기화 상태 로드

            g_nTime_Status = DEVICE_TIME_NOT_SYNCED; // 시간 동기화 상태 초기화

            SD_Card_Log("SNTP Server Connection...\n");

            if (g_nTime_Status == DEVICE_TIME_NOT_SYNCED) // 시간 동기화가 안 된 경우
            {
                // 서버 연결 후 시간 구조 저장 
                result = ESP_AT_Set_SNTP_Time(AT_SNTP_UTC_OFFSET);

                if (result == AT_OK) // SNTP 서버 연결 성공
                {
                    SD_Card_Log("SNTP Server Connected Successfully!\n");
                    step++; // 다음 단계로 이동
                }
                else // SNTP 서버 연결 실패 시
                {
                    SD_Card_Log("SNTP Server Connection Failed!\n");
                    SD_Card_Log("again...\n");
                    // SNTP 서버 연결 실패 시 에러 처리
                    // Error_Handler();
                    Error_Proc(1);
                }
            }
            else // 시간 동기화가 이미 된 경우
            {
                SD_Card_Log("Time Already Synced!\n");
                step++; // 다음 단계로 이동
            }
            break;
            // ====================================================================
        case 5:
            // 저장된 시간 구조 RTC 동기화
            if (g_nTime_Status == DEVICE_TIME_NOT_SYNCED) // 시간 동기화가 안 된 경우
            {
                result = RTC_Set_UTC(); // RTC 초기화 및 UTC 시간 설정
                
                g_nTime_Status = DEVICE_TIME_SYNCED; // 시간 동기화 상태 업데이트

                Save_TimeStatus_FRAM(); // FRAM에 시간 동기화 상태 저장
                
                if (result == RTC_OK) // RTC 설정 성공
                {
                    SD_Card_Log("RTC Set to UTC Successfully!\n");
                    step++; // 다음 단계로 이동
                }
                else // RTC 설정 실패 시
                {
                    SD_Card_Log("RTC Set to UTC Failed!\n");
                    SD_Card_Log("again...\n");
                    // RTC 설정 실패 시 에러 처리
                    // Error_Handler();
                    Error_Proc(1);
                }
            }
            else
            {
                SD_Card_Log("RTC Already Synced!\n");
                step++; // 다음 단계로 이동
            }
            break;

        case 6:
            // 토큰 상태 로드
            Load_Token_Status_FRAM(); // FRAM에서 토큰 상태 로드

            g_nToken_Status = DEVICE_TOKEN_NOT_SET;

            SD_Card_Log("Token Configuration...\n");

            if (g_nToken_Status == DEVICE_TOKEN_NOT_SET) // 토큰이 설정되지 않은 경우
            {
                token = ESP_AT_Get_Token(); // ESP32 AT 명령어를 통해 토큰 조회

                SERVER_API_Set_Token(token); // 서버 API 토큰 저장 함수 호출

                // 메모리에 토큰 저장
                Save_Token_FRAM(token); // FRAM에 토큰 저장

                g_nToken_Status = DEVICE_TOKEN_SET; // 토큰 상태 업데이트
                Save_Token_Status_FRAM(); // FRAM에 토큰 상태 저장

                SD_Card_Log("Token Set Successfully!\n");

                step++; // 다음 단계로 이동
            }
            else // 토큰이 이미 설정된 경우
            {
                // 토큰 불러오기
                Load_Token_FRAM(); // FRAM에서 토큰 로드

                SD_Card_Log("Token Already Set!\n");

                // 다음 스텝
                step++; // 다음 단계로 이동
            }
            break;
        case 7:
            // MAC 주소 상태 로드
            Load_MAC_Status_FRAM(); // FRAM에서 MAC 주소 상태 로드

            g_nMac_Status = DEVICE_MAC_NOT_SET;

            SD_Card_Log("MAC Address Configuration...\n");

            if (g_nMac_Status == DEVICE_MAC_NOT_SET) // MAC 주소가 설정되지 않은 경우
            {
                macAddress = ESP_AT_Get_MAC_Address(); // ESP32 AT 명령어를 통해 MAC 주소 조회

                // MAC 주소 저장
                SERVER_API_Set_MAC_Address(macAddress);  // 서버 API MAC 주소 저장 함수 호출

                // 메모리에 MAC 주소 저장
                Save_MAC_FRAM(macAddress); // FRAM에 MAC 주소 저장
                
                g_nMac_Status = DEVICE_MAC_SET; // MAC 주소 상태 업데이트
                Save_MAC_Status_FRAM(); // FRAM에 MAC 주소 상태 저장

                SD_Card_Log("MAC Address Set Successfully!\n");

                step++; // 다음 단계로 이동
            }
            else // MAC 주소가 이미 설정된 경우
            {
                // MAC 주소 불러오기
                Load_MAC_FRAM(); // FRAM에서 MAC 주소 로드

                SD_Card_Log("MAC Address Already Set!\n");

                step++; // 다음 단계로 이동
            }
            break;
        case 8:
            // 모든 초기화가 완료된 후 최종 상태 로그
            SD_Card_Log("Device Booted Successfully!\n");
            
            // 부팅 최종 시간 SD카드 기록
            SD_Card_Log("Current Boot Time: ");
            SD_Card_Log(RTC_Get_Synced_Year_String());
            SD_Card_Log("-");
            SD_Card_Log(RTC_Get_Synced_Month_String());
            SD_Card_Log("-");
            SD_Card_Log(RTC_Get_Synced_Date_String());
            SD_Card_Log(" ");
            SD_Card_Log(RTC_Get_Synced_Hour_String());
            SD_Card_Log(":");
            SD_Card_Log(RTC_Get_Synced_Minute_String());
            SD_Card_Log(":");
            SD_Card_Log(RTC_Get_Synced_Second_String());
            SD_Card_Log("\n");
            SD_Card_Log("==================<< DONE >>==================\n");

            bootLoop = 0; // 부팅 루프 종료
            break;
        
        default:
            break;
        }
        
        

    }
    // ──────────────────────────────────────────────────────────────────────────────


    
    // ──────────────────────────────────────────────────────────────────────────────
    // ──────────────────────────────────────────────────────────────────────────────
    // ──────────────────────────────────────────────────────────────────────────────

#if 0
    SD_Card_Boot(); // SD 카드 초기화 및 테스트 / 와이파이 파일 확인

    // SD카드 로그 기록
    SD_Card_Log("STM32 Booted Successfully!\n");

    // ──────────────────────────────────────────────────────────────────────────────

    // ESP_AT_Boot(); // ESP32 AT 명령어 초기화 및 버전 조회
    ESP_AT_Boot_5();

    // ESP32 AT 명령어를 통해 펌웨어 버전 조회
    ESP_AT_Send_Command_Sync("AT+GMR\r\n");

    // Save_Wifi_Status_FRAM_Dummy(); // wifi 상태 초기화
    Load_Wifi_Status_FRAM(); // FRAM에서 WiFi 상태 로드
    if (g_nWifi_Status == DEVICE_WIFI_DISCONNECTED) // WiFi가 연결되지 않은 경우
    {
        // ESP32 AT 명령어를 통해 현재 WiFi 모드 조회
        ESP_AT_Send_Command_Sync("AT+CWMODE?\r\n");

        // ESP32 AT 명령어를 통해 WiFi 모드 설정
        ESP_AT_Send_Command_Sync("AT+CWMODE=1\r\n");

        // 연결 가능한 WiFi AP 목록 조회
        ESP_AT_Send_Command_Sync("AT+CWLAP\r\n");
  
        // ──────────────────────────────────────────────────────────────────────────────
        // 와이파이는 2.4GHz 대역만 지원하므로, 5GHz AP는 연결하지 않음, 아니 안테나 없어서 지금까지 에러남 안테나 달아야함
        // SD카드에서 WiFi SSID와 비밀번호를 읽어와서 연결
        // ESP_AT_Send_WiFi_Config();

        // WiFi 연결 상태 업데이트
        // g_nWifi_Status = DEVICE_WIFI_CONNECTED; // WiFi 연결 상태 업데이트
        g_nWifi_Status = ESP_AT_Send_WiFi_Config();
    
        Save_Wifi_Status_FRAM(); // FRAM에 WiFi 상태 저장
    }
    if (g_nWifi_Status == DEVICE_WIFI_CONNECTED) // WiFi가 연결된 경우
    {
        // ESP32 AT 명령어를 통해 현재 WiFi IP 주소 조회
        ESP_AT_Send_Command_Sync("AT+CIFSR\r\n");
    } 
    else // WiFi 연결 실패 시
    {
        // SD 카드에 로그 기록
        SD_Card_Log("WiFi Connection Failed!\n");
    }

    // Save_TimeStatus_FRAM_Dummy();  // 시간 상태 초기화
    // SNTP 서버 연결 및 시간 동기화
    Load_TimeStatus_FRAM(); // FRAM에서 시간 동기화 상태 로드
    if (g_nTime_Status == DEVICE_TIME_NOT_SYNCED) // 시간 동기화가 안 된 경우
    {
        ESP_AT_Set_SNTP_Time(AT_SNTP_UTC_OFFSET_KR);

        // RTC 초기화 및 UTC 시간 설정
        RTC_Set_UTC();

        g_nTime_Status = DEVICE_TIME_SYNCED; // 시간 동기화 상태 업데이트

        Save_TimeStatus_FRAM(); // FRAM에 시간 동기화 상태 저장
    }
#endif
}