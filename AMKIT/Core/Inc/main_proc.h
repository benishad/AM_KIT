/*
 * main_proc.h
 *
 *  Created on: Jun 27, 2025
 *      Author: DONGYOONLEE
 */

#ifndef INC_MAIN_PROC_H_
#define INC_MAIN_PROC_H_

#include <stdint.h>


extern uint16_t ms_tick_1;                            // 1 ms 카운터
extern uint16_t g_nLed_tick_1;                        // LED 관련 1 ms 카운터
extern uint16_t alive_counter;                        // 10초 생존 메시지 카운터

extern uint8_t g_nLed_State;                          // LED 상태 변수 (0: OFF, 1: ON 등)

extern uint8_t txBuf[];                                 // STM32에서 전송할 메시지 버퍼
extern uint8_t rxByte;                                  // 1 바이트 UART RX 버퍼
extern uint8_t txAlive[];                               // 10초마다 보낼 생존 메시지 버퍼

extern uint8_t rxSIMByte;                               // 1 바이트 UART SIM RX 버퍼

extern uint8_t atCmd[];                                 // AT 명령어 전송 버퍼

extern uint8_t g_nBoot_Status;                          // 부팅 단계 (0: 초기화, 1: RTC 설정, 2: WiFi 설정 등)
extern uint8_t g_nMode;                                 // 현재 모드 (0: 마스터, 1: 슬레이브, 2: AP 등)
extern uint8_t g_nTime_Status;                          // 시간 동기화 상태 (0: 동
extern uint8_t g_nWifi_Status;                          // WiFi 연결 상태 (0: 연결 안됨, 1: 연결 됨)
extern uint8_t g_nToken_Status;                         // 토큰 상태 (0: 토큰 없음, 1: 토큰 있음)
extern uint8_t g_nWifi_SSID_Status;                    // WiFi SSID 상태 (0: SSID 없음, 1: SSID 있음)
extern uint8_t g_nWifi_Password_Status;                 // WiFi 비밀번호 상태 (0: 비밀번호 없음, 1: 비밀번호 있음)

enum Test_Step
{
    TEST_STEP_SD_BOOT = 0,          // SD 카드 부팅 단계
    TEST_STEP_ESP_BOOT,              // ESP32 AT 부팅 단계
    TEST_STEP_MAC_CONFIG,            // MAC 주소 설정 단계
    TEST_STEP_WIFI_CONFIG,           // WiFi 연결 단계
    TEST_STEP_WIFI_MODE_SET,        // WiFi 모드 설정 단계
    TEST_STEP_WIFI_SCAN,             // WiFi 스캔 단계
    TEST_STEP_SOFTAP_SET,          // SoftAP 설정 단계
    TEST_STEP_SOFTAP_DHCP,          // SoftAP DHCP 서버 활성화 단계
    TEST_STEP_SOFTAP_IP_SET,        // SoftAP IP 주소 설정 단계
    TEST_STEP_MULTIPLE_CONNECTION,   // 멀티플 커넥션 모드 설정 단계
    TEST_STEP_HTTP_SERVER_START,     // HTTP 서버 시작 단계
    TEST_STEP_CLIENT_REQUEST,         // 클라이언트 요청 처리 단계
};

void Mode_Check(void);
void AP_Mode_Proc(void);

void Master_Proc(void);

void DEBUG_Proc(void);

void Test_Proc(void);

// 파라미터 0 : Error_Handler 호출
// 파라미터 !0 : uart1 코맨트 전송
void Error_Proc(int errorCode);

void Timer_Interrupt_Proc(void);

void Main_System(void);

#endif /* INC_MAIN_PROC_H_ */
