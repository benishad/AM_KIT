/*
 * esp32_at.h
 *
 *  Created on: Jun 18, 2025
 *      Author: DONGYOONLEE
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

#define AT_SNTP_UTC_OFFSET      AT_SNTP_UTC_OFFSET_KR

// 시간 관련 구조체
typedef struct tag_AT_UTC_Time
{
    int sYear;          // 연도 (예: 2025)
    int sMonth;         // 월 숫자 (1~12)
    int sDay;           // 일 (1~31)

    int sDayOfWeek;     // 요일 숫자 (0~6, 0=Sun, 1=Mon, ..., 6=Sat)

    int sHour;           // 시 (0~23)
    int sMinute;         // 분 (0~59)
    int sSecond;         // 초 (0~59)
} AT_UTC_Time, *PAT_UTC_Time;

PAT_UTC_Time AT_Get_UTC_Time(void);


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

// ==========================================================

enum ESP_AT_Status
{
    AT_OK = 0,                // AT 명령어 성공
    AT_ERROR,                 // AT 명령어 오류
};


// ==========================================================

// CCMRAM 초기화
void UTC_Time_Init(void);

// ===========================================================

void ESP_AT_Boot_6(void);
void ESP_AT_Boot_2(void);
void ESP_AT_Boot_3(void);
void ESP_AT_Boot_4(void);
void ESP_AT_Boot_5(void);
int ESP_AT_Boot(void);

int ESP_AT_Get_Firmware_Version(void);

int ESP_AT_Send_WiFi_Config(void);

void ESP_AT_Send_Command(const char* cmd);

void ESP_AT_Send_Command_Async(const char* cmd);
void ESP_AT_Send_Command_Sync(const char* cmd);
void ESP_AT_Send_Command_IT(const char* cmd);

int ESP_AT_Send_Command_Sync_Get_int(const char* cmd);

const char* ESP_AT_Send_Command_Sync_Get_Result(const char* cmd);

const char* ESP_AT_Get_Token(void);

const char* ESP_AT_Get_MAC_Address(void);

int ESP_AT_Set_SNTP_Time(int utcOffset);

void ESP_AT_Server_Init(void);
void ESP_AP_Server(void);

void Handle_IPD_and_Respond(void);      // 가장 동작 잘하는 HTML 함수
void Handle_IPD_and_Respond_1(void);
void Handle_IPD_and_Respond_2(void);    // HTML CSS 함수 - 작동 오류
void Handle_IPD_and_Respond_3(void);    // HTML에 CSS 스타일을 직접 기입함 - 작동 잘함
void Handle_IPD_and_Respond_4(void);

#endif /* INC_ESP32_AT_H_ */
