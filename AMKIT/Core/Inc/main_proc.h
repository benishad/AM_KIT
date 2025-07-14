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




void Mode_Check(void);
void AP_Mode_Proc(void);

void Master_Proc(void);

void DEBUG_Proc(void);

// 파라미터 0 : Error_Handler 호출
// 파라미터 !0 : uart1 코맨트 전송
void Error_Proc(int errorCode);

void Timer_Interrupt_Proc(void);

void Main_System(void);

#endif /* INC_MAIN_PROC_H_ */
