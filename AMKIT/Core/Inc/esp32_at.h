/*
 * esp32_at.h
 *
 *  Created on: Jun 18, 2025
 *      Author: PROGRAM
 */

#ifndef INC_ESP32_AT_H_
#define INC_ESP32_AT_H_

#include <stdint.h>



#define AT_RESP_BUF_SIZE 128            // AT 명령어 응답 버퍼 크기
#define AT_RX_BUF_SIZE  64              // AT 명령어 수신 버퍼 크기
#define AT_END_MARKER "OK\r\n"          // AT 명령어 응답 끝 마커


// UTC 오프셋 정의
// UTC 값은
// https://en.wikipedia.org/wiki/Time_zone#List_of_UTC_offsets
// 에서 확인
#define AT_SNTP_UTC_OFFSET_KR       900             // UTC+9 (South KOREA) 시간대 오프셋
#define AT_SNTP_UTC_OFFSET_US       -500            // UTC-5 (USA) 시간대 오프셋, -5 이외에도 많은 시간대 있음
#define AT_SNTP_UTC_OFFSET_CHN      800             // UTC+8 (CHINA) 시간대 오프셋
#define AT_SNTP_UTC_OFFSET_NZ       1200            // UTC+12 (NEW ZEALAND) 시간대 오프셋



// g_atRxByte extern
extern uint8_t g_atRxByte;

// atidx extern
extern uint16_t atIdx;

// skipEcho extern
extern int skipEcho;

// atLineBuf extern
extern char atLineBuf[AT_RX_BUF_SIZE];

// lastCmdBuf extern
extern char lastCmdBuf[AT_RX_BUF_SIZE];


void ESP_AT_Boot(void);
void ESP_AT_Boot_2(void);
void ESP_AT_Boot_3(void);
void ESP_AT_Boot_4(void);
void ESP_AT_Boot_5(void);

int ESP_AT_Send_WiFi_Config(void);

void ESP_AT_Send_Command(const char* cmd);
void ESP_AT_Setup_WiFi(void);

void ESP_AT_Send_Command_Async(const char* cmd);
void ESP_AT_Send_Command_Sync(const char* cmd);
void ESP_AT_Send_Command_IT(const char* cmd);

void ESP_AT_Get_Token(void);
void ESP_AT_Get_MAC_Address(void);

void ESP_AT_Set_SNTP_Time(int utcOffset);

#endif /* INC_ESP32_AT_H_ */
