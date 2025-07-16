/*
 * esp32_at.c
 *
 *  Created on: Jun 18, 2025
 *      Author: DONGYOONLEE
 */

// =======================================================================================================
// =======================================================================================================
//
//
// https://docs.espressif.com/projects/esp-at/en/latest/esp32c6/AT_Command_Set/Basic_AT_Commands.html
//
//
// =======================================================================================================
// =======================================================================================================

#include "esp32_at.h"
#include "main.h"
#include <string.h>



#define RESP_BUF_SIZE   1024    // 응답 버퍼 크기
#define BYTE_RX_TIMEOUT 300     // 한 바이트 최대 대기(ms)
#define OVERALL_TIMEOUT 20000   // 전체 응답 최대 대기(ms)
#define POST_OK_TIMEOUT 500     // OK 이후 유예 대기(ms)

// 페이지 연결 관련 매크로
#define IPD_HDR_MAX    64
#define PAYLOAD_MAX   512
#define RX_TIMEOUT    200   // ms
#define SEND_TIMEOUT 1000   // ms


// AT 응답의 종료 마커
static const char END_MARKER[] = "\r\nOK\r\n";
static const size_t END_MARKER_LEN = sizeof(END_MARKER) - 1;
// AT 명령어 오류 응답 마커
static const char ERR_MARKER[] = "\r\nERROR\r\n";
static const size_t ERR_MARKER_LEN = sizeof(ERR_MARKER) - 1;



uint8_t  g_atRxByte;
char     atLineBuf[AT_RX_BUF_SIZE];
uint16_t atIdx = 0;

char     lastCmdBuf[AT_RX_BUF_SIZE]; // 마지막 전송된 명령어 버퍼
int     skipEcho = 0; // 마지막 명령어가 전송되었는지 여부

// =========================================================

__CCMRAM__ AT_UTC_Time g_atUtcTime;

PAT_UTC_Time AT_Get_UTC_Time(void)
{
    // 현재 UTC 시간을 반환
    return &g_atUtcTime;
}

// CCMRAM 초기화
void UTC_Time_Init(void)
{
    memset(&g_atUtcTime, 0, sizeof(g_atUtcTime));
}

// =========================================================
#if 0
void ESP_AT_Boot_6(void)
{
    // 1) AT 명령 전송
    const uint8_t cmd[] = "AT+CWMODE?\r\n";
    if (HAL_UART_Transmit(&huart2, (uint8_t*)cmd, sizeof(cmd)-1, HAL_MAX_DELAY) != HAL_OK)
    {
        Error_Handler();
    }

    // 2) 응답 수신 버퍼
    char verBuf[80];
    uint8_t ch;
    int pos = 0;

    // '\n' 까지 읽거나, 버퍼 한계
    while (pos < (int)sizeof(verBuf)-1)
    {
        if (HAL_UART_Receive(&huart2, &ch, 1, 500) == HAL_OK)
        {
            if (ch == '\n')
            {
                verBuf[pos] = '\0';
                break;
            }
            else if (ch != '\r')
            {
                verBuf[pos++] = (char)ch;
            }
        }
        else
        {
            // 타임아웃 발생 시
            verBuf[pos] = '\0';
            break;
        }
    }

    // 3) PC(USART1)로 받은 문자열 출력
    if (pos > 0)
    {
        HAL_UART_Transmit(&huart1, (uint8_t*)verBuf, pos, HAL_MAX_DELAY);
        const char nl[] = "\r\n";
        HAL_UART_Transmit(&huart1, (uint8_t*)nl, sizeof(nl)-1, HAL_MAX_DELAY);
    }
}

void ESP_AT_Boot_2(void)
{
    // 1) AT+CWMODE? 전송
    const uint8_t cmd[] = "AT+CWMODE?\r\n";
    if (HAL_UART_Transmit(&huart2, (uint8_t*)cmd, sizeof(cmd)-1, HAL_MAX_DELAY) != HAL_OK)
    {
        Error_Handler();
    }

    // 2) 빈 줄 두 번 연속 나올 때까지 수신
    char  lineBuf[128]; // 한 줄 최대 127자 + NULL
    uint8_t ch;
    int   idx;
    int   emptyCnt = 0;

    while (emptyCnt < 2)
    {
        // 한 줄 읽기
        idx = 0;
        while (idx < 128 - 1)
        {
            if (HAL_UART_Receive(&huart2, &ch, 1, 200) == HAL_OK)
            {
                if (ch == '\r') 
                    continue;           // CR 무시
                if (ch == '\n') 
                    break;              // LF: 한 줄 끝
                lineBuf[idx++] = (char)ch;
            }
            else
            {
                // 바이트 타임아웃 → 줄 종료
                break;
            }
        }
        lineBuf[idx] = '\0';

        // 빈 줄이면 카운트, 아니면 PC로 전송
        if (idx == 0)
        {
            emptyCnt++;
        }
        else
        {
            emptyCnt = 0;
            // UART1(PC)로 출력
            HAL_UART_Transmit(&huart1, (uint8_t*)lineBuf, idx, HAL_MAX_DELAY);
            HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY);
        }
    }
}

void ESP_AT_Boot_3(void)
{
    uint32_t start = HAL_GetTick();
    char     respBuf[RESP_BUF_SIZE];
    uint16_t pos = 0;
    uint8_t  ch;

    // (1) 혹시 걸려 있던 UART2 IT 수신 취소
    HAL_UART_AbortReceive_IT(&huart2);

    // (2) AT 명령 전송
    const char cmd[] = "AT+GMR\r\n";
    if (HAL_UART_Transmit(&huart2, (uint8_t*)cmd, sizeof(cmd)-1, HAL_MAX_DELAY) != HAL_OK)
        Error_Handler();

    // (3) 바이트 스트림을 읽어들여 "\r\n\r\n" 이 나올 때까지 모은다
    while (pos < RESP_BUF_SIZE - 1)
    {
        // 바이트 단위로 최대 BYTE_RX_TIMEOUT 동안 대기
        if (HAL_UART_Receive(&huart2, &ch, 1, BYTE_RX_TIMEOUT) == HAL_OK)
        {
            respBuf[pos++] = (char)ch;

            // 마지막 네 글자가 "\r\n\r\n" 이면 종료
            if (pos >= 4
             && respBuf[pos-4] == '\r'
             && respBuf[pos-3] == '\n'
             && respBuf[pos-2] == '\r'
             && respBuf[pos-1] == '\n')
            {
                break;
            }

            // 바이트를 받을 때마다 전체 타이머도 리셋
            start = HAL_GetTick();
        }
        else
        {
            // 바이트 타임아웃: 더 들어올 데이터가 없으면 탈출
            if ((HAL_GetTick() - start) > OVERALL_TIMEOUT)
                break;
        }
    }
    respBuf[pos] = '\0';

    // (4) 수신한 전체 블록을 한 번에 PC로 전송
    if (pos > 0)
        HAL_UART_Transmit(&huart1, (uint8_t*)respBuf, pos, HAL_MAX_DELAY);

    // (5) 부트 끝나고 나서야 다시 인터럽트 기반 수신으로 전환
    if (HAL_UART_Receive_IT(&huart2, &g_atRxByte, 1) != HAL_OK)
        Error_Handler();
}

// 4번 부트 기능은 정상 동작함
void ESP_AT_Boot_4(void)
{
    // (1) AT+CWMODE? 전송
    const char cmd[] = "AT+GMR\r\n";
    if (HAL_UART_Transmit(&huart2, (uint8_t*)cmd,
                          sizeof(cmd)-1, HAL_MAX_DELAY) != HAL_OK)
        Error_Handler();

    // (2) 바이트 스트림을 읽어들여 END_MARKER가 나올 때까지 모은다
    char respBuf[RESP_BUF_SIZE];
    size_t pos = 0;
    uint8_t ch;
    uint32_t start = HAL_GetTick();

    while (pos < RESP_BUF_SIZE - 1)
    {
        // 1바이트씩 최대 BYTE_RX_TIMEOUT 동안 대기
        if (HAL_UART_Receive(&huart2, &ch, 1, BYTE_RX_TIMEOUT) == HAL_OK)
        {
            respBuf[pos++] = (char)ch;

            // 최근 들어온 END_MARKER와 일치하면 종료
            if (pos >= END_MARKER_LEN &&
                memcmp(&respBuf[pos - END_MARKER_LEN],
                       END_MARKER, END_MARKER_LEN) == 0)
            {
                break;
            }

            // 받은 순간 전체 타이머 리셋
            start = HAL_GetTick();
        }
        else
        {
            // 전체 OVERALL_TIMEOUT 경과 시 강제 종료
            if (HAL_GetTick() - start > OVERALL_TIMEOUT)
                break;
        }
    }
    respBuf[pos] = '\0';

    // (3) PC로 한 번에 출력
    if (pos > 0)
        HAL_UART_Transmit(&huart1, (uint8_t*)respBuf, pos, HAL_MAX_DELAY);

}

// 거의 최종 완성형 명령 전송 함수
void ESP_AT_Boot_5(void)
{
    char respBuf[RESP_BUF_SIZE];
    size_t pos = 0;
    uint8_t ch;
    uint32_t tick_0 = HAL_GetTick();

    // (1) AT 전송 , 응답 OK
    const char cmd[] = "AT\r\n";
    if (HAL_UART_Transmit(&huart2, (uint8_t*)cmd, sizeof(cmd)-1, HAL_MAX_DELAY) != HAL_OK)
    {
        Error_Handler();
    }

    // (2) END_MARKER 까지 수신
    while (pos < RESP_BUF_SIZE - 1)
    {
        // 1바이트씩 최대 BYTE_RX_TIMEOUT 동안 대기
        if (HAL_UART_Receive(&huart2, &ch, 1, BYTE_RX_TIMEOUT) == HAL_OK)
        {
            respBuf[pos++] = (char)ch;

            // 최근 들어온 END_MARKER와 일치하면 종료
            if (pos >= END_MARKER_LEN && memcmp(&respBuf[pos - END_MARKER_LEN], END_MARKER, END_MARKER_LEN) == 0)
            {
                break;
            }

            // 받은 순간 전체 타이머 리셋
            tick_0 = HAL_GetTick();
        }
        else
        {
            // 전체 OVERALL_TIMEOUT 경과 시 강제 종료
            if (HAL_GetTick() - tick_0 > OVERALL_TIMEOUT)
            {
                break;
            }
        }
    }

    // (3) OK 이후 추가 데이터 유무 확인 (POST_OK_TIMEOUT ms 동안)
    uint32_t tick_1 = HAL_GetTick();

    while (pos < RESP_BUF_SIZE-1 && HAL_GetTick() - tick_1 < POST_OK_TIMEOUT)
    {
        if (HAL_UART_Receive(&huart2, &ch, 1, BYTE_RX_TIMEOUT) == HAL_OK)
        {
            respBuf[pos++] = ch;
            tick_1 = HAL_GetTick();  // 새 데이터가 오면 다시 유예 시간만큼 더 기다림
        }
    }

    respBuf[pos] = '\0';

    // (3) PC로 한 번에 출력
    if (pos > 0)
    {
        HAL_UART_Transmit(&huart1, (uint8_t*)respBuf, pos, HAL_MAX_DELAY);
    }
}

#endif // 0

int ESP_AT_Boot(void)
{
    int result;
    // AT 커맨드로 AT 전송
    const char *cmd = "AT\r\n";
    const char *response = ESP_AT_Send_Command_Sync_Get_Result(cmd);
    const char *success = "AT command successful!\r\n";
    const char *fail = "AT command failed!\r\n";

    // 응답에서 OK 문자열 찾기
    if (response != NULL && strstr(response, "OK") != NULL)
    {
        // OK 응답이 있으면 성공
        HAL_UART_Transmit(&huart1, (uint8_t*)success, strlen(success), HAL_MAX_DELAY);

        result = AT_OK; // 성공 코드
    }
    else
    {
        // OK 응답이 없으면 실패
        HAL_UART_Transmit(&huart1, (uint8_t*)fail, strlen(fail), HAL_MAX_DELAY);

        result = AT_ERROR; // 실패 코드
    }

    // 결과 반환
    return result;
}




// SD카드에 WIFI 설정을 가져롸 AT 명령어로 ESP32에 전송하는 함수
int ESP_AT_Send_WiFi_Config(void)
{
    int result = 0; // 결과 변수 초기화
    // SD_Card_Get_WiFi_SSID(void) 함수로 SSID를 가져옴
    const char* ssid = SD_Card_Get_WiFi_SSID();
    const char* password = SD_Card_Get_WiFi_Password();
    
    const char * response = NULL;
    const char *success = "WiFi configuration sent successfully!\r\n";
    const char *fail = "Failed to send WiFi configuration!\r\n";

    char cmd[128]={0}; // AT 명령어를 저장할 버퍼

    int len = snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, password);

    if (len < 0 || len >= (int)sizeof(cmd))
    {
        // 버퍼 오버플로우 또는 snprintf 실패
        Error_Handler();
    }

    // ESP_AT_Send_Command_Sync(cmd);
    response = ESP_AT_Send_Command_Sync_Get_Result(cmd);
    
    if (response != NULL && strstr(response, "OK") != NULL)
    {
        // OK 응답이 있으면 성공
        HAL_UART_Transmit(&huart1, (uint8_t*)success, strlen(success), HAL_MAX_DELAY);
        result = AT_OK; // 성공 코드
    }
    else
    {
        // OK 응답이 없으면 실패
        HAL_UART_Transmit(&huart1, (uint8_t*)fail, strlen(fail), HAL_MAX_DELAY);
        result = AT_ERROR; // 실패 코드
    }
    
    // return DEVICE_WIFI_CONNECTED; // 성공
    return result;
}


// ESP32 AT 명령 전송 함수
void ESP_AT_Send_Command(const char* cmd)
{
    // AT 명령어 전송
    if (HAL_UART_Transmit(&huart2, (uint8_t*)cmd, strlen(cmd), HAL_MAX_DELAY) != HAL_OK)
    {
        // 전송 실패 처리
        Error_Handler();
    }

    // 어떤 명령을 보냈는지 PC(UART1)로 에코
    // (디버깅용)
    HAL_UART_Transmit(&huart1, (uint8_t*)cmd, strlen(cmd), HAL_MAX_DELAY);
}

// 비동기 방식 ESP32 AT 명령 전송 함수
// 이 함수는 명령어를 전송하고, 응답을 기다리지 않고 바로 리턴합니다.
void ESP_AT_Send_Command_Async(const char* cmd)
{
    // AT 명령어 전송
    if (HAL_UART_Transmit(&huart2, (uint8_t*)cmd, strlen(cmd), HAL_MAX_DELAY) != HAL_OK)
    {
        // 전송 실패 처리
        Error_Handler();
    }

    // 어떤 명령을 보냈는지 PC(UART1)로 에코
    // (디버깅용)
    HAL_UART_Transmit(&huart1, (uint8_t*)cmd, strlen(cmd), HAL_MAX_DELAY);
}

// 동기방식 ESP32 AT 명령 전송 함수
// 이 함수는 명령어를 전송하고, 응답을 기다려서 결과를 PC(UART1)로 전송합니다.

void ESP_AT_Send_Command_Sync(const char* cmd)
{
    char    respBuf[RESP_BUF_SIZE] = {0};       // 응답 버퍼
    size_t  pos   = 0;                          // 현재 응답 버퍼 위치
    uint8_t ch;                                 // 수신된 바이트
    uint32_t tick_0   = HAL_GetTick();          // 전체 타이머 시작

    // (1) AT 명령 전송
    size_t cmdLen = strlen(cmd);
    if (HAL_UART_Transmit(&huart2, (uint8_t*)cmd, cmdLen, HAL_MAX_DELAY) != HAL_OK)
    {
        Error_Handler();
    }

    // (2) END_MARKER ("\r\nOK\r\n") 까지 수신
    while (pos < RESP_BUF_SIZE - 1)
    {
        if (HAL_UART_Receive(&huart2, &ch, 1, BYTE_RX_TIMEOUT) == HAL_OK)
        {
            respBuf[pos++] = (char)ch;          // 수신된 바이트를 버퍼에 저장

            // 슬라이딩 윈도우로 END_MARKER 일치 검사
            // pos >= END_MARKER_LEN 조건은 END_MARKER 길이만큼의 데이터가 모였는지 확인
            if (pos >= END_MARKER_LEN && memcmp(&respBuf[pos - END_MARKER_LEN], END_MARKER, END_MARKER_LEN) == 0)
            {
                break;
            }
            // 에러마커 확인
            else if (pos >= ERR_MARKER_LEN && memcmp(&respBuf[pos - ERR_MARKER_LEN], ERR_MARKER, ERR_MARKER_LEN) == 0)
            {
                // 에러 발생 시 강제 종료
                // pos = 0;  // 버퍼 초기화
                break;
            }

            tick_0 = HAL_GetTick();  // 데이터 수신 시 전체 타이머 리셋
        }
        else if (HAL_GetTick() - tick_0 > OVERALL_TIMEOUT)
        {
            // 전체 대기 초과 시 강제 종료
            break;
        }
    }

    // (3) OK 이후 추가 URC 등 있을 수 있으니 잠깐 더 대기하며 수신
    uint32_t tick_1 = HAL_GetTick();
    while (pos < RESP_BUF_SIZE - 1 && HAL_GetTick() - tick_1 < POST_OK_TIMEOUT)
    {
        if (HAL_UART_Receive(&huart2, &ch, 1, BYTE_RX_TIMEOUT) == HAL_OK)
        {
            respBuf[pos++] = ch;
            tick_1 = HAL_GetTick();  // 추가 데이터 수신 시 유예 시간 리셋
        }
    }
    respBuf[pos] = '\0';

    // (4) 한 번에 PC(UART1)로 전송
    if (pos > 0)
    {
        HAL_UART_Transmit(&huart1, (uint8_t*)respBuf, pos, HAL_MAX_DELAY);
    }
}

// 명령 성공 실패를 AT_OK,AT_ERROR 으로 반환하는 함수
int ESP_AT_Send_Command_Sync_Get_int(const char* cmd)
{
    char    respBuf[RESP_BUF_SIZE] = {0};       // 응답 버퍼
    size_t  pos   = 0;                          // 현재 응답 버퍼 위치
    uint8_t ch;                                 // 수신된 바이트
    uint32_t tick_0   = HAL_GetTick();          // 전체 타이머 시작

    // (1) AT 명령 전송
    size_t cmdLen = strlen(cmd);
    if (HAL_UART_Transmit(&huart2, (uint8_t*)cmd, cmdLen, HAL_MAX_DELAY) != HAL_OK)
    {
        Error_Handler();
    }

    // (2) END_MARKER ("\r\nOK\r\n") 까지 수신
    while (pos < RESP_BUF_SIZE - 1)
    {
        if (HAL_UART_Receive(&huart2, &ch, 1, BYTE_RX_TIMEOUT) == HAL_OK)
        {
            respBuf[pos++] = (char)ch;          // 수신된 바이트를 버퍼에 저장

            // 슬라이딩 윈도우로 END_MARKER 일치 검사
            // pos >= END_MARKER_LEN 조건은 END_MARKER 길이만큼의 데이터가 모였는지 확인
            if (pos >= END_MARKER_LEN && memcmp(&respBuf[pos - END_MARKER_LEN], END_MARKER, END_MARKER_LEN) == 0)
            {
                break;
            }
            // 에러마커 확인
            else if (pos >= ERR_MARKER_LEN && memcmp(&respBuf[pos - ERR_MARKER_LEN], ERR_MARKER, ERR_MARKER_LEN) == 0)
            {
                // 에러 발생 시 강제 종료
                // pos = 0;  // 버퍼 초기화
                break;
            }

            tick_0 = HAL_GetTick();  // 데이터 수신 시 전체 타이머 리셋
        }
        else if (HAL_GetTick() - tick_0 > OVERALL_TIMEOUT)
        {
            // 전체 대기 초과 시 강제 종료
            break;
        }
    }

    // (3) OK 이후 추가 URC 등 있을 수 있으니 잠깐 더 대기하며 수신
    uint32_t tick_1 = HAL_GetTick();
    while (pos < RESP_BUF_SIZE - 1 && HAL_GetTick() - tick_1 < POST_OK_TIMEOUT)
    {
        if (HAL_UART_Receive(&huart2, &ch, 1, BYTE_RX_TIMEOUT) == HAL_OK)
        {
            respBuf[pos++] = ch;
            tick_1 = HAL_GetTick();  // 추가 데이터 수신 시 유예 시간 리셋
        }
    }
    respBuf[pos] = '\0';

    // (4) 한 번에 PC(UART1)로 전송
    if (pos > 0)
    {
        HAL_UART_Transmit(&huart1, (uint8_t*)respBuf, pos, HAL_MAX_DELAY);
    }

    // 응답 성공 반환
    if (strstr(respBuf, END_MARKER))
    {
        return AT_OK; // 성공
    }

    return AT_ERROR; // 실패
}

// 동기방식 ESP32 AT 명령 전송 함수
// 반환값으로 응답 문자열을 반환
const char* ESP_AT_Send_Command_Sync_Get_Result(const char* cmd)
{
    static char    respBuf[RESP_BUF_SIZE] = {0};       // 응답 버퍼
    size_t  pos   = 0;                          // 현재 응답 버퍼 위치
    uint8_t ch;                                 // 수신된 바이트
    uint32_t tick_0   = HAL_GetTick();          // 전체 타이머 시작

    // (1) AT 명령 전송
    size_t cmdLen = strlen(cmd);
    if (HAL_UART_Transmit(&huart2, (uint8_t*)cmd, cmdLen, HAL_MAX_DELAY) != HAL_OK)
    {
        Error_Handler();
    }

    // (2) END_MARKER ("\r\nOK\r\n") 까지 수신
    while (pos < RESP_BUF_SIZE - 1)
    {
        if (HAL_UART_Receive(&huart2, &ch, 1, BYTE_RX_TIMEOUT) == HAL_OK)
        {
            respBuf[pos++] = (char)ch;          // 수신된 바이트를 버퍼에 저장

            // 슬라이딩 윈도우로 END_MARKER 일치 검사
            // pos >= END_MARKER_LEN 조건은 END_MARKER 길이만큼의 데이터가 모였는지 확인
            if (pos >= END_MARKER_LEN && memcmp(&respBuf[pos - END_MARKER_LEN], END_MARKER, END_MARKER_LEN) == 0)
            {
                break;
            }
            // 에러마커 확인
            else if (pos >= ERR_MARKER_LEN && memcmp(&respBuf[pos - ERR_MARKER_LEN], ERR_MARKER, ERR_MARKER_LEN) == 0)
            {
                // 에러 발생 시 강제 종료
                // pos = 0;  // 버퍼 초기화
                break;
            }

            tick_0 = HAL_GetTick();  // 데이터 수신 시 전체 타이머 리셋
        }
        else if (HAL_GetTick() - tick_0 > OVERALL_TIMEOUT)
        {
            // 전체 대기 초과 시 강제 종료
            break;
        }
    }

    // (3) OK 이후 추가 URC 등 있을 수 있으니 잠깐 더 대기하며 수신
    uint32_t tick_1 = HAL_GetTick();
    while (pos < RESP_BUF_SIZE - 1 && HAL_GetTick() - tick_1 < POST_OK_TIMEOUT)
    {
        if (HAL_UART_Receive(&huart2, &ch, 1, BYTE_RX_TIMEOUT) == HAL_OK)
        {
            respBuf[pos++] = ch;
            tick_1 = HAL_GetTick();  // 추가 데이터 수신 시 유예 시간 리셋
        }
    }
    respBuf[pos] = '\0';

    // (4) 한 번에 PC(UART1)로 전송
    if (pos > 0)
    {
        HAL_UART_Transmit(&huart1, (uint8_t*)respBuf, pos, HAL_MAX_DELAY);
    }

    return respBuf; // 응답 버퍼를 반환
}



// 인터럽트 기반 ESP32 AT 명령 전송 함수
void ESP_AT_Send_Command_IT(const char* cmd)
{
     // 1) lastCmdBuf에 저장 (끝의 "\r\n" 제외)
    size_t len = strlen(cmd);

    while (len && (cmd[len-1]=='\r' || cmd[len-1]=='\n')) 
    {
        len--;
    }
    memcpy(lastCmdBuf, cmd, len);
    
    lastCmdBuf[len] = '\0';
    
    skipEcho = 1;                // 다음 라인은 에코이므로 건너뛰기

    // 2) UART2로 비동기 전송
    HAL_UART_Transmit_IT(&huart2, (uint8_t*)cmd, strlen(cmd));
}



// ESP32 AT 명령어로 펌웨어 조회
int ESP_AT_Get_Firmware_Version(void)
{
    int result = 0; // 결과 변수 초기화

    // AT 명령어 전송
    const char *cmd = "AT+GMR\r\n";
    const char *response = ESP_AT_Send_Command_Sync_Get_Result(cmd);
    const char *success = "AT firmware command successful!\r\n";
    const char *fail = "AT firmware command failed!\r\n";

    // 응답에서 OK 문자열 찾기
    if (response != NULL && strstr(response, "OK") != NULL)
    {
        // OK 응답이 있으면 성공
        HAL_UART_Transmit(&huart1, (uint8_t*)success, strlen(success), HAL_MAX_DELAY);
        result = AT_OK; // 성공 코드
    }
    else
    {
        // OK 응답이 없으면 실패
        HAL_UART_Transmit(&huart1, (uint8_t*)fail, strlen(fail), HAL_MAX_DELAY);
        result = AT_ERROR; // 실패 코드
    }

    // 결과 반환
    return result;
}






const char* ESP_AT_Get_Token(void)
{
    const char *response = NULL; // 응답 문자열을 저장할 변수

    // 1) 보낼 JSON 문자열 정의 (큰 따옴표는 백슬래시로 이스케이프)
    const char *jsonBody = "{\\\"uid\\\":\\\"kimss@andamiro.com\\\"\\,\\\"pwd\\\":\\\"temp1234!\\\"}";

    // 2) AT+HTTPCLIENT 명령어 문자열 생성
    char atCmd[256];
    int len = snprintf(atCmd, sizeof(atCmd),
        "AT+HTTPCLIENT=3,1,\"https://dev-api.andamiro.net/test/user\",\"dev-api.andamiro.net\",\"/test/user\",2,\"%s\"\r\n",
        jsonBody);
    
    if (len < 0 || len >= (int)sizeof(atCmd)) 
    {
        // snprintf 실패 또는 버퍼 부족
        Error_Handler();
    }

    // 3) 위 함수로 AT 명령 전송 및 응답 처리
    response = ESP_AT_Send_Command_Sync_Get_Result(atCmd);
    // response에는 
    //Ast/user","dev-api.andamiro.net","/test/user",2,"{\"uid\":\"kimss@andamiro.com\"\,\"pwd\":\"temp1234!\"}"+HTTPCLIENT:72,{"token":"eEx4NVhXZ2p5MlBBRjJ3eU9CbGJJeUtSbzlvUDU4TzJ0ZCtRSE9FdHZDTT0="}OK
    // 형태의 응답이 저장됨
    // token 값 추출
    const char *tokenStart = strstr(response, "\"token\":\"");

    static char token[128]; // 충분히 큰 버퍼

    // "token":" 문자열을 찾아서 토큰 시작 위치를 찾음
    if (tokenStart != NULL) 
    {
        tokenStart += strlen("\"token\":\""); // 토큰 시작 위치로 이동

        const char *tokenEnd = strchr(tokenStart, '\"'); // 다음 큰 따옴표 찾기
        
        if (tokenEnd != NULL) 
        {
            size_t tokenLength = tokenEnd - tokenStart; // 토큰 길이 계산

            // 순수 토큰만 추출
            if (tokenLength < sizeof(token)) 
            {
                //tokenLength -= 2; // 버퍼 크기 제한
                strncpy(token, tokenStart, tokenLength);
                token[tokenLength] = '\0'; // 문자열 종료
                // 이제 token 변수에 토큰 값이 저장됨
            }
        }
    }
    // 4) 추출된 토큰을 PC(UART1)로 전송
    if (token[0] != '\0') 
    {
        // tokjen을 PC로 전송
        HAL_UART_Transmit(&huart1, (uint8_t*)token, strlen(token), HAL_MAX_DELAY);
        HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY); // 줄바꿈 추가
    } 
    else 
    {
        // 토큰 추출 실패 시 에러 처리
        HAL_UART_Transmit(&huart1, (uint8_t*)"Token extraction failed\r\n", 25, HAL_MAX_DELAY);
    }

    // 토큰 저장
    // SERVER_API_Set_Token(token); // 서버 API 토큰 저장 함수 호출

    return token; // 토큰 반환
}





// ESP32 기기 고유값 반환
const char* ESP_AT_Get_MAC_Address(void)
{
    // AT+CIPSTAMAC? 명령어 전송
    const char *cmd = "AT+CIPSTAMAC?\r\n";
    const char *response = ESP_AT_Send_Command_Sync_Get_Result(cmd);

    // 응답에서 MAC 주소 추출
    // "40:4c:ca:50:2c:04" 형식에서 :을 제외한 영문대문자숫자 12개로 구성된 MAC 주소를 추출
    // 예: +CIPSTAMAC:"40:4c:ca:50:2c:04" -> 404CCA502C04
    const char *macStart = strstr(response, "CIPSTAMAC:\""); // CIPSTAMAC:" 문자열 찾기
    static char macAddress[18] = {0}; // MAC 주소를 저장할 버퍼 (17자 + NULL)

    // 응답에서 MAC 주소 추출, 콜론을 제외한 문자만 추출
    if (macStart != NULL) 
    {
        macStart += strlen("CIPSTAMAC:\""); // 처음 따옴표 다음 위치로 이동
        const char *macEnd = strchr(macStart, '\"'); // 두번째 따옴표 찾기
        if (macEnd != NULL) 
        {
            size_t macLength = macEnd - macStart; // MAC 주소 길이 계산
            
            if (macLength < 18) // MAC 주소는 17자 + NULL, 콜른을 제외한 길이 12자 + NULL
            {
                // 콜론을 제외한 문자만 추출
                size_t j = 0;
                for (size_t i = 0; i < macLength; i++) 
                {
                    if (macStart[i] != ':') 
                    {
                        macAddress[j++] = toupper(macStart[i]); // 대문자로 변환하여 저장
                    }
                }
                macAddress[j] = '\0'; // 문자열 종료
            }
        }
    }


#if 0
    if (macStart != NULL) 
    {
        macStart += strlen("CIPSTAMAC:\""); // 처음 따옴표 다음 위치로 이동
        const char *macEnd = strchr(macStart, '\"'); // 두번째 따옴표 찾기
        if (macEnd != NULL) 
        {
            size_t macLength = macEnd - macStart; // MAC 주소 길이 계산
            
            if (macLength < 18) // MAC 주소는 17자 + NULL
            {
                strncpy(macAddress, macStart, macLength);
                macAddress[macLength] = '\0'; // 문자열 종료
            }
        }
    }
#endif
    // PC(UART1)로 MAC 주소 전송
    HAL_UART_Transmit(&huart1, (uint8_t*)macAddress, strlen(macAddress), HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY); // 줄바꿈 추가

    // 반환할 주소가 없다면
    if (macAddress[0] == '\0') 
    {
        // MAC 주소 추출 실패 시 에러 처리
        HAL_UART_Transmit(&huart1, (uint8_t*)"MAC address extraction failed\r\n", 31, HAL_MAX_DELAY);
        return NULL; // 실패 시 NULL 반환
    }

    return macAddress; // MAC 주소 반환
}


// 파라미터로 UTC 매크로를 받아서 시간은 설정하는 함수
int ESP_AT_Set_SNTP_Time(int utcOffset)
{
    int result = 0; // 결과 변수 초기화

    PAT_UTC_Time pAtUtcTime = AT_Get_UTC_Time(); // UTC 시간 구조체 포인터
    // ──────────────────────────────────────────────────────────────────────────────
    char cmd[128];
    
    // SNTP 서버 연결
    // 한국 NTP 서버 설정
    // ESP_AT_Send_Command_Sync("AT+CIPSNTPCFG=1,900,\"pool.ntp.org\",\"time.google.com\"\r\n"); // NTP 서버 설정
    int len = snprintf(cmd, sizeof(cmd), "AT+CIPSNTPCFG=1,%d,\"pool.ntp.org\",\"time.google.com\"\r\n", utcOffset);

    if (len < 0 || len >= (int)sizeof(cmd)) 
    {
        // 버퍼 오버플로우 또는 snprintf 실패
        Error_Handler();
        result = AT_ERROR; // 실패 코드
    }

    ESP_AT_Send_Command_Sync(cmd);

    // 설정 확인
    ESP_AT_Send_Command_Sync("AT+CIPSNTPCFG?\r\n");

    // 시간 확인
    const char *response = ESP_AT_Send_Command_Sync_Get_Result("AT+CIPSNTPTIME?\r\n");

    // 예: +CIPSNTPTIME:Tue Jun 24 14:59:31 2025 형식으로 응답

    // ──────────────────────────────────────────────────────────────────────────────

    pAtUtcTime = AT_Get_UTC_Time();

    // ──────────────────────────────────────────────────────────────────────────────
    
    // 응답에서 요일 추출
    const char *timeStart = strstr(response, "CIPSNTPTIME:"); // CIPSTAMAC:" 문자열 찾기
    
    static char dayOfWeek[4] = {0}; // 요일을 저장할 버퍼 (3자 + NULL)

    // 응답에서 요일 추출
    if (timeStart != NULL) 
    {
        timeStart += strlen("CIPSNTPTIME:");

        strncpy(dayOfWeek, timeStart, 3);
        dayOfWeek[3] = '\0'; // 문자열 종료
    }

    // 요일 RTC숫자로 변환하여 구조체에 저장
    pAtUtcTime->sDayOfWeek = Month_String_To_Number(dayOfWeek); // 구조체에 요일 RTC숫자 저장

    HAL_UART_Transmit(&huart1, (uint8_t*)dayOfWeek, strlen(dayOfWeek), HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY); // 줄바꿈 추가

    // ──────────────────────────────────────────────────────────────────────────────

    static char month[4] = {0}; // 월을 저장할 버퍼 (3자 + NULL)

    // 응답에서 월 추출
    if (timeStart != NULL) 
    {
        timeStart += 4; // 요일 다음 공백 문자로 이동

        // 월 이름을 3글자 추출
        strncpy(month, timeStart, 3);
        month[3] = '\0'; // 문자열 종료
    }

    // 월 이름을 RTC숫자로 변환하여 구조체에 저장
    pAtUtcTime->sMonth = Month_String_To_Number(month); // 구조체에 월 RTC숫자 저장
    
    HAL_UART_Transmit(&huart1, (uint8_t*)month, strlen(month), HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY); // 줄바꿈 추가

    // ──────────────────────────────────────────────────────────────────────────────
    
    static char day[3] = {0}; // 일을 저장할 버퍼 (2자 + NULL)

    // 응답에서 일 추출
    if (timeStart != NULL) 
    {
        timeStart += 4; // 월 다음 공백 문자로 이동

        // 일 숫자를 2글자 추출
        strncpy(day, timeStart, 2);
        day[2] = '\0'; // 문자열 종료
    }
    
    pAtUtcTime->sDay = atoi(day); // 문자열을 정수로 변환하여 구조체에 저장

    HAL_UART_Transmit(&huart1, (uint8_t*)day, strlen(day), HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY); // 줄바꿈 추가

    // ──────────────────────────────────────────────────────────────────────────────

    static char time[3] = {0}; // 시를 저장할 버퍼 (2자 + NULL)

    // 응답에서 시 추출
    if (timeStart != NULL) 
    {
        timeStart += 3; // 일 다음 콜론 문자로 이동

        // 시 숫자를 2글자 추출
        strncpy(time, timeStart, 2);
        time[2] = '\0'; // 문자열 종료
    }

    pAtUtcTime->sHour = atoi(time); // 문자열을 정수로 변환하여 구조체에 저장

    HAL_UART_Transmit(&huart1, (uint8_t*)time, strlen(time), HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY); // 줄바꿈 추가

    // ──────────────────────────────────────────────────────────────────────────────

    static char minute[3] = {0}; // 분을 저장할 버퍼 (2자 + NULL)

    // 응답에서 분 추출
    if (timeStart != NULL) 
    {
        timeStart += 3; // 시 다음 콜론 문자로 이동

        // 분 숫자를 2글자 추출
        strncpy(minute, timeStart, 2);
        minute[2] = '\0'; // 문자열 종료
    }

    pAtUtcTime->sMinute = atoi(minute); // 문자열을 정수로 변환하여 구조체에 저장

    HAL_UART_Transmit(&huart1, (uint8_t*)minute, strlen(minute), HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY); // 줄바꿈 추가

    // ──────────────────────────────────────────────────────────────────────────────

    static char second[3] = {0}; // 초를 저장할 버퍼 (2자 + NULL)

    // 응답에서 초 추출
    if (timeStart != NULL) 
    {
        timeStart += 3; // 분 다음 콜론 문자로 이동

        // 초 숫자를 2글자 추출
        strncpy(second, timeStart, 2);
        second[2] = '\0'; // 문자열 종료
    }

    pAtUtcTime->sSecond = atoi(second); // 문자열을 정수로 변환하여 구조체에 저장

    HAL_UART_Transmit(&huart1, (uint8_t*)second, strlen(second), HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY); // 줄바꿈 추가

    // ──────────────────────────────────────────────────────────────────────────────

    static char year[5] = {0}; // 년도를 저장할 버퍼 (4자 + NULL)

    // 응답에서 년도 추출
    if (timeStart != NULL) 
    {
        timeStart += 3; // 초 다음 공백 문자로 이동

        // 년도 숫자를 4글자 추출
        strncpy(year, timeStart, 4);
        year[4] = '\0'; // 문자열 종료
    }

    pAtUtcTime->sYear = atoi(year); // 문자열을 정수로 변환하여 구조체에 저장

    HAL_UART_Transmit(&huart1, (uint8_t*)year, strlen(year), HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY); // 줄바꿈 추가

    // ──────────────────────────────────────────────────────────────────────────────

    if (result == AT_ERROR) // 에러 발생 시
    {
        return AT_ERROR; // 실패 코드 반환
    }
    
    result = AT_OK; // 성공 코드

    return result; // 결과 반환
}

// ──────────────────────────────────────────────────────────────────────────────
// ──────────────────────────────────────────────────────────────────────────────
// ──────────────────────────────────────────────────────────────────────────────


void ESP_AT_Server_Init(void)
{
    int bootLoop = 1;
    int step = 0;
    int result = 0;

    while (bootLoop)
    {
        switch (step)
        {
        case 0:
            result = ESP_AT_Boot(); // ESP32 AT 테스트

            if (result == AT_OK)
            {
                // SD_Card_Log("ESP32 AT Booted Successfully!\n");
                step++; // 다음 단계로 이동
            }
            else
            {
                // SD_Card_Log("ESP32 AT Boot Failed!\n");
                // SD_Card_Log("again...\n");
                // ESP32 AT 부팅 실패 시 에러 처리
                //Error_Handler();
                Error_Proc(1);
            }
            break;
        case 1:
            result = ESP_AT_Get_Firmware_Version(); // ESP32 AT 명령어로 펌웨어 버전 조회
            
            // SD_Card_Log("ESP32 AT Firmware Version Retrieval...\n");

            if (result == AT_OK)
            {
                // SD_Card_Log("ESP32 AT Firmware Version Retrieved Successfully!\n");
                step++; // 다음 단계로 이동
            }
            else
            {
                // SD_Card_Log("ESP32 AT Firmware Version Retrieval Failed!\n");
                // SD_Card_Log("again...\n");
                // ESP32 AT 펌웨어 버전 조회 실패 시 에러 처리
                // Error_Handler();
                Error_Proc(1);
            }
            break;
        case 2:
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

            // OK수신까지 1~2초 대기

            // 펌웨어 커스텀으로 추가함
            // ESP_AT_Send_Command_Sync("AT+CAPTIVE\r\n");

            // 멀티 커넥션 모드 활성화
            ESP_AT_Send_Command_Sync("AT+CIPMUX=1\r\n");

            // TCP 서버 시작
            ESP_AT_Send_Command_Sync("AT+CIPSERVER=1,80\r\n");

            step++; // 다음 단계로 이동

            break;
        case 3:
            bootLoop = 0; // 모든 단계 완료 후 루프 종료
            break;

        default:
            break;
        }
    }
}


void ESP_AP_Server(void)
{
    int MAX_RX = 256;
    char rxBuf[MAX_RX];
    uint8_t ch;
    int  pos = 0;
    memset(rxBuf, 0, sizeof(rxBuf)); // 수신 버퍼 초기화

    
    // IPD URC 수신 대기 루프
    while (1)
    {
        // UART2로부터 1바이트씩 수신
        if (HAL_UART_Receive(&huart2, &ch, 1, HAL_MAX_DELAY) != HAL_OK)
        {
            // 수신 실패 시 에러 처리
            Error_Handler();
        }
        // 수신된 바이트를 버퍼에 저장
        if (ch == '\n' || ch == '\r') // 줄바꿈 문자
        {
            if (pos > 0) // 버퍼에 데이터가 있다면
            {
                rxBuf[pos] = '\0'; // 문자열 종료
                pos = 0; // 버퍼 초기화
                // IPD URC 처리
                if (strstr(rxBuf, "+IPD") != NULL)
                {
                    // 클라이언트 요청 처리
                    // 예: 요청 내용 파싱, 응답 생성 등
                    // 여기서는 단순히 수신된 데이터를 그대로 에코하는 예시
                    HAL_UART_Transmit(&huart1, (uint8_t*)rxBuf, strlen(rxBuf), HAL_MAX_DELAY);

                    int linkID = 0, dataLen = 0;

                    if (sscanf(rxBuf, "+IPD,%d,%d:", &linkID, &dataLen) == 2)
                    {
                        // 보낼 HTML 응답 생성
                        // const char htmlResponse[] = 
                        //     "HTTP/1.1 200 OK\r\n"
                        //     "Content-Type: text/html\r\n"
                        //     "Connection: close\r\n"
                        //     "\r\n"
                        //     "<!DOCTYPE html><html><body>"
                        //     "<h1>ESP32 Web Page</h1>"
                        //     "<p>Hello from STM32!</p>"
                        //     "</body></html>";
                        const char htmlResponse[] = 
                            "HTTP/1.1 200 OK\r\n"
                            "Content-Type: text/html\r\n"
                            "Connection: keep-alive\r\n"
                            "\r\n"
                            "<!DOCTYPE html><html><body>"
                            "<h1>ESP32 Web Page</h1>"
                            "<p>Hello from STM32!</p>"
                            "</body></html>";
                        int htmlLen = sizeof(htmlResponse) - 1; // 문자열 길이

                        // CIPSEND 명령어로 응답 전송
                        char sendCmd[50];
                        int cmdLen = snprintf(sendCmd, sizeof(sendCmd), "AT+CIPSEND=%d,%d\r\n", linkID, htmlLen);
                        if (cmdLen < 0 || cmdLen >= sizeof(sendCmd))
                        {
                            // 명령어 생성 실패 처리
                            Error_Proc(1);
                        }
                        // CIPSEND 명령어 전송
                        ESP_AT_Send_Command_Sync(sendCmd);

                        // HTML 응답 전송
                        if (HAL_UART_Transmit(&huart2, (uint8_t*)htmlResponse, htmlLen, HAL_MAX_DELAY) != HAL_OK)
                        {
                            // 응답 전송 실패 처리
                            Error_Proc(1);
                        }

                        // 클라이언트 연결 종료
                        snprintf(sendCmd, sizeof(sendCmd), "AT+CIPCLOSE=%d\r\n", linkID);
                        ESP_AT_Send_Command_Sync(sendCmd);
                        // ESP_AT_Send_Command_Sync("AT+CIPCLOSE\r\n");

                        // 버퍼 초기화
                        pos = 0;
                        rxBuf[0] = '\0'; // 문자열 종료

                    }
                }
            }
        }
        else if (pos < MAX_RX - 1) // 버퍼가 가득 차지 않았다면
        {
            rxBuf[pos++] = ch; // 수신된 바이트 저장
        }
        else
        {
        // 버퍼가 가득 찼을 때는 초기화
            pos = 0; // 버퍼 초기화
            rxBuf[pos] = '\0'; // 문자열 종료
        }
    }
}


// ──────────────────────────────────────────────────────────────────────────────
// ──────────────────────────────────────────────────────────────────────────────

void Handle_IPD_and_Respond(void)
{
    uint8_t  ch;
    char     ipdHdr[IPD_HDR_MAX];
    char     payload[PAYLOAD_MAX];
    uint32_t start;
    uint16_t linkID, dataLen;

    while (1)
    {
        int  hdrPos = 0, payPos = 0;

        // — 1) '+IPD' 헤더 수집 (‘:’ 포함)
        do {
            if (HAL_UART_Receive(&huart2, &ch, 1, RX_TIMEOUT) != HAL_OK)
            {
                continue;
            }
        } while (ch != '+');
        ipdHdr[hdrPos++] = '+';

        while (hdrPos < IPD_HDR_MAX-1)
        {
            if (HAL_UART_Receive(&huart2, &ch, 1, RX_TIMEOUT) != HAL_OK)
            {
                Error_Handler();
            }
            ipdHdr[hdrPos++] = ch;
            if (ch == ':')
            {
                break;
            }
        }
        ipdHdr[hdrPos] = '\0';

        // — 2) linkID, dataLen 파싱
        if (sscanf(ipdHdr, "+IPD,%hu,%hu:", &linkID, &dataLen) != 2)
        {
            continue;
        }

        // — 3) dataLen 바이트만큼 payload 수집 (CR/LF 포함)
        for (uint16_t i = 0; i < dataLen && payPos < PAYLOAD_MAX-1; ++i)
        {
            if (HAL_UART_Receive(&huart2, &ch, 1, RX_TIMEOUT) != HAL_OK)
            {
                Error_Handler();
            }
            payload[payPos++] = ch;
        }
        payload[payPos] = '\0';

        // — 4) GET 라인 파싱
        //    예: "GET /path HTTP/1.1"
        char method[8], url[128];
        int isIcon = 0;
        if (sscanf(payload, "%7s %127s", method, url) == 2)
        {
            if (strcmp(url, "/apple-touch-icon-precomposed.png") == 0)
            {
                isIcon = 1;
            }
        }

        // — 5) 응답 헤더/바디 준비
        char  respHdr[128];
        int   hdrLen, bodyLen;
        if (isIcon)
        {
            // 204 No Content
            const char *hdr204 =
              "HTTP/1.1 204 No Content\r\n"
              "Connection: close\r\n"
              "\r\n";
            hdrLen  = strlen(hdr204);
            strncpy(respHdr, hdr204, hdrLen);
            bodyLen = 0;
        }
        else
        {
            // 200 OK + HTML
            // bodyLen = sizeof(htmlBody) - 1;
            bodyLen = htmlBodyLen;
            hdrLen  = snprintf(respHdr, sizeof(respHdr),
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/html\r\n"
              "Content-Length: %d\r\n"
              "Connection: close\r\n"
              "\r\n",
              bodyLen);
            if (hdrLen < 0 || hdrLen >= (int)sizeof(respHdr))
            {
                Error_Handler();
            }
        }

        // — 6) AT+CIPSEND=<linkID>,<hdrLen+bodyLen>
        int totalLen = hdrLen + bodyLen;
        char cmd[64];
        int  cmdLen = snprintf(cmd, sizeof(cmd),
                        "AT+CIPSEND=%d,%d\r\n",
                        linkID, totalLen);
        if (cmdLen < 0 || cmdLen >= (int)sizeof(cmd))
        {
            Error_Handler();
        }
        HAL_UART_Transmit(&huart2, (uint8_t*)cmd, cmdLen, HAL_MAX_DELAY);

        // — 7) '>' 프롬프트 대기
        start = HAL_GetTick();
        do
        {
            if (HAL_UART_Receive(&huart2, &ch, 1, RX_TIMEOUT)== HAL_OK
                && ch == '>')
            {
                break;
            }
        } while (HAL_GetTick() - start < RX_TIMEOUT);

        // — 8) 헤더 + (icon이면 바디 없음 / 아니면 htmlBody) 전송
        HAL_UART_Transmit(&huart2, (uint8_t*)respHdr, hdrLen, HAL_MAX_DELAY);
        if (!isIcon)
        {
            HAL_UART_Transmit(&huart2, (uint8_t*)htmlBody, bodyLen, HAL_MAX_DELAY);
        }

        // — 9) “SEND OK” URC 대기 (최대 SEND_TIMEOUT)
        start = HAL_GetTick();
        const char sendOk[] = "SEND OK";
        int  match = 0;
        while (HAL_GetTick() - start < SEND_TIMEOUT)
        {
            if (HAL_UART_Receive(&huart2, &ch, 1, RX_TIMEOUT)== HAL_OK)
            {
                if (ch == sendOk[match])
                {
                    if (++match == (int)strlen(sendOk))
                    {
                        break;
                    }
                }
                else
                {
                    match = 0;
                }
            }
        }

        // — 10) 안전 딜레이 후 연결 종료
        HAL_Delay(50);
        cmdLen = snprintf(cmd, sizeof(cmd),
                         "AT+CIPCLOSE=%d\r\n",
                         linkID);
        ESP_AT_Send_Command_Sync_Get_Result(cmd);

        // 다음 요청 대기
    }
}


// ──────────────────────────────────────────────────────────────────────────────
// ESP32 AT 서버 함수 및 HTML 바디


// HTML 바디만 정의 테스트
// IOS safari엔진은 HTTP/1.1에서 Content-Length 헤더나 
// Transfer-Encoding: chunked 없이 응답 바디의 끝을 정확히 알 수 없으면, 
// 바디가 아직 더 남아 있다고 판단해서 렌더링을 보류한다고 함.
// 그래서 호환성을 위해 Content-Length 헤더를 반드시 포함해야 함.
// 또한, HTML 바디는 UTF-8로 인코딩되어야 함.
// 아래는 Content-Length 헤더로 정확한 바디 길이를 명시했음.
// static const char htmlBody[] =
//     "<!DOCTYPE html><html><body>"
//     "<h1>ESP32 Web Page</h1>"
//     "<p>Hello from STM32!</p>"
//     "</body></html>";

// IPD URC를 처리하고 응답을 생성 테스트
void Handle_IPD_and_Respond_1(void)
{
    uint8_t  ch;
    char     ipdHdr[IPD_HDR_MAX];
    char     payload[PAYLOAD_MAX];
    uint32_t start;
    uint16_t linkID, dataLen;

    while (1)
    {
        int  hdrPos = 0, payPos = 0;

        // — 1) '+IPD' 헤더 수집 (‘:’ 포함)
        do
        {
            if (HAL_UART_Receive(&huart2, &ch, 1, RX_TIMEOUT) != HAL_OK)
            {
                continue;
            }
        } while (ch != '+');
        ipdHdr[hdrPos++] = '+';

        while (hdrPos < IPD_HDR_MAX-1)
        {
            if (HAL_UART_Receive(&huart2, &ch, 1, RX_TIMEOUT) != HAL_OK)
            {
                Error_Handler();
            }
            ipdHdr[hdrPos++] = ch;
            if (ch == ':')
            {
                break;
            }
        }
        ipdHdr[hdrPos] = '\0';

        // — 2) linkID, dataLen 파싱
        if (sscanf(ipdHdr, "+IPD,%hu,%hu:", &linkID, &dataLen) != 2)
        {
            continue;
        }

        // — 3) dataLen 바이트만큼 payload 수집 (CR/LF 포함)
        // for (uint16_t i = 0; i < dataLen && payPos < PAYLOAD_MAX-1; ++i)
        // {
        //     if (HAL_UART_Receive(&huart2, &ch, 1, RX_TIMEOUT) != HAL_OK)
        //     {
        //         Error_Handler();
        //     }
        //     payload[payPos++] = ch;
        // }
        // payload[payPos] = '\0';

        for (uint16_t i = 0; i < dataLen; ++i)
        {
            if (HAL_UART_Receive(&huart2, &ch, 1, RX_TIMEOUT) != HAL_OK)
            {
                Error_Handler();
            }
            // 가득 차지 않았다면 버퍼에 저장
            if (payPos < PAYLOAD_MAX-1)
            {
                payload[payPos++] = ch;
            }
        }
        // payload는 CR/LF 포함
        payload[payPos] = '\0';

        // — 4) GET 라인 파싱
        //    예: "GET /path HTTP/1.1"
        char method[8], url[128];
        int isIcon = 0;
        int isCss = 0;
        if (sscanf(payload, "%7s %127s", method, url) == 2)
        {
            if (strcmp(url, "/style.css") == 0)
            {
                isCss = 1;
            }
            else if (strcmp(url, "/favicon.ico")==0)
            {
                isIcon = 1;
            }
            else if (strcmp(url, "/apple-touch-icon-precomposed.png") == 0)
            {
                isIcon = 1;
            }
        }

        // — 5) 응답 헤더/바디 준비
        char  respHdr[128];
        int   hdrLen, bodyLen;
        if (isCss)
        {
            // 200 OK + CSS
            bodyLen = cssStyleLen; // cssBodyLen은 미리 정의된 CSS 바디 길이
            // hdrLen  = snprintf(respHdr, sizeof(respHdr),
            //   "HTTP/1.1 200 OK\r\n"
            //   "Content-Type: text/css\r\n"
            //   "Content-Length: %d\r\n"
            //   "Connection: close\r\n"
            //   "\r\n",
            //   bodyLen);
            hdrLen  = snprintf(respHdr, sizeof(respHdr),
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/css\r\n"
              "Content-Length: %d\r\n"
              "Connection: keep-alive\r\n"
              "\r\n",
              bodyLen); 
            if (hdrLen < 0 || hdrLen >= (int)sizeof(respHdr))
            {
                Error_Handler();
            }
        }
        else if (isIcon)
        {
            // 204 No Content
            // const char *hdr204 =
            //   "HTTP/1.1 204 No Content\r\n"
            //   "Connection: close\r\n"
            //   "\r\n";
            const char *hdr204 =
              "HTTP/1.1 204 No Content\r\n"
              "Connection: keep-alive\r\n"
              "\r\n";
            hdrLen  = strlen(hdr204);
            strncpy(respHdr, hdr204, hdrLen);
            bodyLen = 0;
        }
        else 
        {
            // 200 OK + HTML
            // bodyLen = sizeof(htmlBody) - 1;
            bodyLen = htmlBody_2Len;
            // hdrLen  = snprintf(respHdr, sizeof(respHdr),
            //   "HTTP/1.1 200 OK\r\n"
            //   "Content-Type: text/html\r\n"
            //   "Content-Length: %d\r\n"
            //   "Connection: close\r\n"
            //   "\r\n",
            //   bodyLen);
            hdrLen  = snprintf(respHdr, sizeof(respHdr),
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/html\r\n"
              "Content-Length: %d\r\n"
              "Connection: keep-alive\r\n"
              "\r\n",
              bodyLen);
            if (hdrLen < 0 || hdrLen >= (int)sizeof(respHdr))
            {
                Error_Handler();
            }
        }

        // — 6) AT+CIPSEND=<linkID>,<hdrLen+bodyLen>
        int totalLen = hdrLen + bodyLen;
        char cmd[64];
        int  cmdLen = snprintf(cmd, sizeof(cmd),
                        "AT+CIPSEND=%hu,%d\r\n",
                        linkID, totalLen);
        if (cmdLen < 0 || cmdLen >= (int)sizeof(cmd))
        {
            Error_Handler();
        }
        HAL_UART_Transmit(&huart2, (uint8_t*)cmd, cmdLen, HAL_MAX_DELAY);

        // — 7) '>' 프롬프트 대기
        start = HAL_GetTick();
        do
        {
            if (HAL_UART_Receive(&huart2, &ch, 1, RX_TIMEOUT)== HAL_OK
                && ch == '>')
            {
                break;
            }
        } while (HAL_GetTick() - start < RX_TIMEOUT);

        // — 8) 헤더 + (icon이면 바디 없음 / 아니면 htmlBody) 전송
        HAL_UART_Transmit(&huart2, (uint8_t*)respHdr, hdrLen, HAL_MAX_DELAY);
        if (isCss)
        {
            HAL_UART_Transmit(&huart2, (uint8_t*)cssStyle, cssStyleLen, HAL_MAX_DELAY);
        }
        else if (!isIcon)
        {
            HAL_UART_Transmit(&huart2, (uint8_t*)htmlBody_2, htmlBody_2Len, HAL_MAX_DELAY);
        }

        // — 9) “SEND OK” URC 대기 (최대 SEND_TIMEOUT)
        start = HAL_GetTick();
        const char sendOk[] = "SEND OK";
        int  match = 0;
        while (HAL_GetTick() - start < SEND_TIMEOUT)
        {
            if (HAL_UART_Receive(&huart2, &ch, 1, RX_TIMEOUT)== HAL_OK)
            {
                if (ch == sendOk[match])
                {
                    if (++match == (int)strlen(sendOk))
                    {
                        break;
                    }
                }
                else
                {
                    match = 0;
                }
            }
        }

        // — 10) 안전 딜레이 후 연결 종료
        HAL_Delay(50);
        // cmdLen = snprintf(cmd, sizeof(cmd),
        //                  "AT+CIPCLOSE=%d\r\n",
        //                  linkID);

        int closeLen = snprintf(cmd, sizeof(cmd),
                       "AT+CIPCLOSE=%hu\r\n",
                       linkID);
        if (closeLen < 0 || closeLen >= (int)sizeof(cmd))
        {
            Error_Handler();
        }

        ESP_AT_Send_Command_Sync_Get_Result(cmd);

        // 다음 요청 대기
    }
}

// ──────────────────────────────────────────────────────────────────────────────

void Handle_IPD_and_Respond_2(void)
{
    uint8_t  ch;
    char     ipdHdr[IPD_HDR_MAX];
    char     payload[PAYLOAD_MAX];
    uint16_t linkID, dataLen;
    int      hdrPos, payPos;
    uint32_t start;
    char     respHdr[128];
    int      hdrLen, bodyLen;
    char     cmd[64];
    int      cmdLen, match;

    while (1)
    {
        IPD_START:
        // — 1) "+IPD" 헤더 수집 ("+IPD,<linkID>,<len>:" 형태)
        hdrPos = 0;
        // '+' 문자 대기
        do {
            if (HAL_UART_Receive(&huart2, &ch, 1, RX_TIMEOUT) != HAL_OK)
            {
                // return;
                goto IPD_START; // 재시도
            }
        } while (ch != '+');
        ipdHdr[hdrPos++] = '+';

        // ':'까지 읽기
        while (hdrPos < IPD_HDR_MAX - 1)
        {
            if (HAL_UART_Receive(&huart2, &ch, 1, RX_TIMEOUT) != HAL_OK)
            {
                // return;
                goto IPD_START; // 재시도
            }
            ipdHdr[hdrPos++] = ch;
            if (ch == ':')
            {
                break;
            }
        }
        ipdHdr[hdrPos] = '\0';

        // -------------------------------------

        // — 2) linkID, dataLen 파싱
        if (sscanf(ipdHdr, "+IPD,%hu,%hu:", &linkID, &dataLen) != 2)
        {
            // return;
            goto IPD_START; // 재시도
        }

        // -------------------------------------

        // — 3) payload 읽기 (최대 PAYLOAD_MAX-1 바이트)
        payPos = 0;
        for (uint16_t i = 0; i < dataLen; ++i)
        {
            if (HAL_UART_Receive(&huart2, &ch, 1, RX_TIMEOUT) != HAL_OK)
            {
                // return;
                goto IPD_START; // 재시도
            }
            if (payPos < PAYLOAD_MAX - 1)
            {
                payload[payPos++] = ch;
            }
        }
        payload[payPos] = '\0';

        // -------------------------------------

        // — 4) GET 라인에서 URL 추출
        //    예: "GET /style.css HTTP/1.1"
        char method[8] = {0}, url[128] = {0};
        int  isCss = 0, isIcon = 0;
        if (sscanf(payload, "%7s %127s", method, url) == 2)
        {
            if (strcmp(url, "/style.css") == 0)
            {
                isCss = 1;
            }
            else if (strcmp(url, "/favicon.ico") == 0 ||
                    strcmp(url, "/apple-touch-icon-precomposed.png") == 0)
            {
                isIcon = 1;
            }
        }

        // -------------------------------------

        // — 5) 응답 헤더 + 바디 길이 결정
        #if 1
        if (isCss)
        {
            bodyLen = cssStyleLen;
            hdrLen  = snprintf(respHdr, sizeof(respHdr),
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/css\r\n"
                    "Content-Length: %d\r\n"
                    "Connection: close\r\n"
                    "\r\n",
                    bodyLen);
        }
        else if (isIcon)
        {
            bodyLen = 0;
            hdrLen  = snprintf(respHdr, sizeof(respHdr),
                    "HTTP/1.1 204 No Content\r\n"
                    "Connection: close\r\n"
                    "\r\n");
        }
        else
        {
            bodyLen = htmlBody_2Len;
            hdrLen  = snprintf(respHdr, sizeof(respHdr),
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/html; charset=utf-8\r\n"
                    "Content-Length: %d\r\n"
                    "Connection: close\r\n"
                    "\r\n",
                    bodyLen);
        }
        if (hdrLen < 0 || hdrLen >= (int)sizeof(respHdr))
        {
            // return;
            goto IPD_START; // 재시도
        }
        #else
        if (isCss)
        {
            bodyLen = cssStyleLen;
            hdrLen  = snprintf(respHdr, sizeof(respHdr),
                    "HTTP/1.0 200 OK\r\n"
                    "Content-Type: text/css\r\n"
                    "Content-Length: %d\r\n"
                    "Connection: close\r\n"
                    "\r\n",
                    bodyLen);
        }
        else if (isIcon)
        {
            bodyLen = 0;
            hdrLen  = snprintf(respHdr, sizeof(respHdr),
                    "HTTP/1.0 204 No Content\r\n"
                    "Connection: close\r\n"
                    "\r\n");
        }
        else
        {
            bodyLen = htmlBody_2Len;
            hdrLen  = snprintf(respHdr, sizeof(respHdr),
                    "HTTP/1.0 200 OK\r\n"
                    "Content-Type: text/html; charset=utf-8\r\n"
                    "Content-Length: %d\r\n"
                    "Connection: close\r\n"
                    "\r\n",
                    bodyLen);
        }
        if (hdrLen < 0 || hdrLen >= (int)sizeof(respHdr))
        {
            // header 생성 실패 처리
            return;
        }
        #endif

        // -------------------------------------

        // — 6) AT+CIPSEND=<linkID>,<hdrLen+bodyLen>
        cmdLen = snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%hu,%d\r\n",
                        linkID, hdrLen + bodyLen);
        if (cmdLen < 0 || cmdLen >= (int)sizeof(cmd))
        {
            return;
        }

        HAL_UART_Transmit(&huart2, (uint8_t*)cmd, cmdLen, HAL_MAX_DELAY);

        // -------------------------------------

        // — 7) '>' 프롬프트 대기
        start = HAL_GetTick();
        do {
            if (HAL_UART_Receive(&huart2, &ch, 1, RX_TIMEOUT) == HAL_OK &&
                ch == '>')
            {
                break;
            }
        } while (HAL_GetTick() - start < RX_TIMEOUT);

        // -------------------------------------

        // — 8) 헤더/바디 전송
        HAL_UART_Transmit(&huart2, (uint8_t*)respHdr, hdrLen, HAL_MAX_DELAY);
        HAL_UART_Transmit(&huart1, (uint8_t*)respHdr, hdrLen, HAL_MAX_DELAY);
        if (isCss)
        {
            HAL_UART_Transmit(&huart2, (uint8_t*)cssStyle, cssStyleLen, HAL_MAX_DELAY);
        }
        else if (!isIcon)
        {
            HAL_UART_Transmit(&huart2, (uint8_t*)htmlBody_2, htmlBody_2Len, HAL_MAX_DELAY);
        }

        // -------------------------------------

        // — 9) "SEND OK" URC 대기
        const char sendOk[] = "SEND OK";
        match = 0;
        start = HAL_GetTick();
        while (HAL_GetTick() - start < SEND_TIMEOUT)
        {
            if (HAL_UART_Receive(&huart2, &ch, 1, RX_TIMEOUT) == HAL_OK)
            {
                if (ch == sendOk[match])
                {
                    if (++match == (int)strlen(sendOk))
                    {
                        break;
                    }
                }
                else
                {
                    match = 0;
                }
            }
        }

        // -------------------------------------

        // — 10) 연결 종료
        // HAL_Delay(50);
        cmdLen = snprintf(cmd, sizeof(cmd), "AT+CIPCLOSE=%hu\r\n", linkID);
        ESP_AT_Send_Command_Sync_Get_Result(cmd);
        break; // HTML 응답 후 루프 종료

        // if (!isCss && !isIcon)
        // {
        //     ESP_AT_Send_Command_Sync_Get_Result(cmd);

        //     break; // HTML 응답 후 루프 종료
        // }
    }
}



void Handle_IPD_and_Respond_3(void)
{
    uint8_t  ch;
    char     ipdHdr[IPD_HDR_MAX];
    char     payload[PAYLOAD_MAX];
    uint16_t linkID, dataLen;
    int      hdrPos, payPos;
    uint32_t start;
    char     respHdr[128];
    int      hdrLen, bodyLen;
    char     cmd[64];
    int      cmdLen, match;

    while (1)
    {
        IPD_START:
        // — 1) "+IPD" 헤더 수집 ("+IPD,<linkID>,<len>:" 형태)
        hdrPos = 0;
        // '+' 문자 대기
        do {
            if (HAL_UART_Receive(&huart2, &ch, 1, RX_TIMEOUT) != HAL_OK)
            {
                // return;
                goto IPD_START; // 재시도
            }
        } while (ch != '+');
        ipdHdr[hdrPos++] = '+';

        // ':'까지 읽기
        while (hdrPos < IPD_HDR_MAX - 1)
        {
            if (HAL_UART_Receive(&huart2, &ch, 1, RX_TIMEOUT) != HAL_OK)
            {
                // return;
                goto IPD_START; // 재시도
            }
            ipdHdr[hdrPos++] = ch;
            if (ch == ':')
            {
                break;
            }
        }
        ipdHdr[hdrPos] = '\0';

        // -------------------------------------

        // — 2) linkID, dataLen 파싱
        if (sscanf(ipdHdr, "+IPD,%hu,%hu:", &linkID, &dataLen) != 2)
        {
            // return;
            goto IPD_START; // 재시도
        }

        // -------------------------------------

        // — 3) payload 읽기 (최대 PAYLOAD_MAX-1 바이트)
        payPos = 0;
        for (uint16_t i = 0; i < dataLen; ++i)
        {
            if (HAL_UART_Receive(&huart2, &ch, 1, RX_TIMEOUT) != HAL_OK)
            {
                // return;
                goto IPD_START; // 재시도
            }
            if (payPos < PAYLOAD_MAX - 1)
            {
                payload[payPos++] = ch;
            }
        }
        payload[payPos] = '\0';

        // -------------------------------------

        // — 4) GET 라인에서 URL 추출
        //    예: "GET /style.css HTTP/1.1"
        char method[8] = {0}, url[128] = {0};
        int  isCss = 0, isIcon = 0;
        if (sscanf(payload, "%7s %127s", method, url) == 2)
        {
            if (strcmp(url, "/favicon.ico") == 0 || strstr(url, "apple-touch-icon") != NULL){
                isIcon = 1;
            }
            
        }

        // -------------------------------------

        // — 5) 응답 헤더 + 바디 길이 결정
        if (isIcon)
        {
            bodyLen = 0;
            hdrLen = snprintf(respHdr, sizeof(respHdr),
                "HTTP/1.1 204 No Content\r\n"
                "Connection: close\r\n"
                "\r\n");
        }
        else
        {
            bodyLen = htmlBody_inlineLen;
            hdrLen = snprintf(respHdr, sizeof(respHdr),
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html; charset=utf-8\r\n"
                "Content-Length: %d\r\n"
                "Connection: close\r\n"
                "\r\n",
                bodyLen);
        }
        if (hdrLen < 0 || hdrLen >= (int)sizeof(respHdr))
        {
            // return;
            goto IPD_START; // 재시도
        }

        // -------------------------------------

        // — 6) AT+CIPSEND=<linkID>,<hdrLen+bodyLen>
        cmdLen = snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%hu,%d\r\n",
                        linkID, hdrLen + bodyLen);
        if (cmdLen < 0 || cmdLen >= (int)sizeof(cmd))
        {
            return;
        }

        HAL_UART_Transmit(&huart2, (uint8_t*)cmd, cmdLen, HAL_MAX_DELAY);

        // -------------------------------------

        // — 7) '>' 프롬프트 대기
        start = HAL_GetTick();
        do {
            if (HAL_UART_Receive(&huart2, &ch, 1, RX_TIMEOUT) == HAL_OK &&
                ch == '>')
            {
                break;
            }
        } while (HAL_GetTick() - start < RX_TIMEOUT);

        // -------------------------------------

        // — 8) 헤더/바디 전송
        HAL_UART_Transmit(&huart2, (uint8_t*)respHdr, hdrLen, HAL_MAX_DELAY);
        HAL_UART_Transmit(&huart1, (uint8_t*)respHdr, hdrLen, HAL_MAX_DELAY);
        if (!isIcon)
        {
            HAL_UART_Transmit(&huart2, (uint8_t*)htmlBody_inline, htmlBody_inlineLen, HAL_MAX_DELAY);
        }

        // -------------------------------------

        // — 9) "SEND OK" URC 대기
        const char sendOk[] = "SEND OK";
        match = 0;
        start = HAL_GetTick();
        while (HAL_GetTick() - start < SEND_TIMEOUT)
        {
            if (HAL_UART_Receive(&huart2, &ch, 1, RX_TIMEOUT) == HAL_OK)
            {
                if (ch == sendOk[match])
                {
                    if (++match == (int)strlen(sendOk))
                    {
                        break;
                    }
                }
                else
                {
                    match = 0;
                }
            }
        }

        // -------------------------------------

        // — 10) 연결 종료
        // HAL_Delay(50);
        cmdLen = snprintf(cmd, sizeof(cmd), "AT+CIPCLOSE=%hu\r\n", linkID);
        ESP_AT_Send_Command_Sync_Get_Result(cmd);
        break; // HTML 응답 후 루프 종료

        // if (!isCss && !isIcon)
        // {
        //     ESP_AT_Send_Command_Sync_Get_Result(cmd);

        //     break; // HTML 응답 후 루프 종료
        // }
    }
}



void Handle_IPD_and_Respond_4(void)
{
    uint8_t  ch;
    char     ipdHdr[IPD_HDR_MAX];
    char     payload[PAYLOAD_MAX];
    uint16_t linkID, dataLen;
    int      hdrPos, payPos;
    uint32_t start;
    char     respHdr[128];
    int      hdrLen, bodyLen;
    char     cmd[64];
    int      cmdLen, match;
    int      isLed=0, isIcon=0;

    while (1)
    {
        IPD_START:
        // — 1) "+IPD" 헤더 수집 ("+IPD,<linkID>,<len>:" 형태)
        hdrPos = 0;
        // '+' 문자 대기
        do {
            if (HAL_UART_Receive(&huart2, &ch, 1, RX_TIMEOUT) != HAL_OK)
            {
                // return;
                goto IPD_START; // 재시도
            }
        } while (ch != '+');
        ipdHdr[hdrPos++] = '+';

        // ':'까지 읽기
        while (hdrPos < IPD_HDR_MAX - 1)
        {
            if (HAL_UART_Receive(&huart2, &ch, 1, RX_TIMEOUT) != HAL_OK)
            {
                // return;
                goto IPD_START; // 재시도
            }
            ipdHdr[hdrPos++] = ch;
            if (ch == ':')
            {
                break;
            }
        }
        ipdHdr[hdrPos] = '\0';

        // -------------------------------------

        // — 2) linkID, dataLen 파싱
        if (sscanf(ipdHdr, "+IPD,%hu,%hu:", &linkID, &dataLen) != 2)
        {
            // return;
            goto IPD_START; // 재시도
        }

        // -------------------------------------

        // — 3) payload 읽기 (최대 PAYLOAD_MAX-1 바이트)
        payPos = 0;
        for (uint16_t i = 0; i < dataLen; ++i)
        {
            if (HAL_UART_Receive(&huart2, &ch, 1, RX_TIMEOUT) != HAL_OK)
            {
                // return;
                goto IPD_START; // 재시도
            }
            if (payPos < PAYLOAD_MAX - 1)
            {
                payload[payPos++] = ch;
            }
        }
        payload[payPos] = '\0';

        // -------------------------------------

        // — 4) GET 라인에서 URL 추출
        //    예: "GET /style.css HTTP/1.1"
        char method[8] = {0}, url[128] = {0};
        if (sscanf(payload, "%7s %127s", method, url) == 2)
        {
            if (strcmp(url, "/led") == 0)
            {
                isLed = 1;
            }
            else if (strcmp(url, "/favicon.ico") == 0 || strstr(url, "apple-touch-icon") != NULL)
            {
                isIcon = 1;
            }
            
        }

        // -------------------------------------

        // — 5) 응답 헤더 + 바디 길이 결정
        if (isIcon)
        {
            bodyLen = 0;
            hdrLen = snprintf(respHdr, sizeof(respHdr),
                "HTTP/1.1 204 No Content\r\n"
                "Connection: close\r\n"
                "\r\n");
        }
        else
        {
            bodyLen = htmlBody_inline_2Len;
            hdrLen = snprintf(respHdr, sizeof(respHdr),
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html; charset=utf-8\r\n"
                "Content-Length: %d\r\n"
                "Connection: close\r\n"
                "\r\n",
                bodyLen);
        }
        if (hdrLen < 0 || hdrLen >= (int)sizeof(respHdr))
        {
            // return;
            goto IPD_START; // 재시도
        }

        // 만약 /quit 요청이면 STM32 쪽으로 UART1 신호 전송
        if (isLed)
        {
            const char stmMsg[] = "JS+LED\r\n";
            // HAL_UART_Transmit(&huart1, (uint8_t*)stmMsg, strlen(stmMsg), HAL_MAX_DELAY);
            // 인터럽트로 전송
            HAL_UART_Transmit_IT(&huart1, (uint8_t*)stmMsg, strlen(stmMsg));
            RX_LED_Toggle();
        }

        // -------------------------------------

        // — 6) AT+CIPSEND=<linkID>,<hdrLen+bodyLen>
        cmdLen = snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%hu,%d\r\n",
                        linkID, hdrLen + bodyLen);
        if (cmdLen < 0 || cmdLen >= (int)sizeof(cmd))
        {
            return;
        }

        HAL_UART_Transmit(&huart2, (uint8_t*)cmd, cmdLen, HAL_MAX_DELAY);

        // -------------------------------------

        // — 7) '>' 프롬프트 대기
        start = HAL_GetTick();
        do {
            if (HAL_UART_Receive(&huart2, &ch, 1, RX_TIMEOUT) == HAL_OK &&
                ch == '>')
            {
                break;
            }
        } while (HAL_GetTick() - start < RX_TIMEOUT);

        // -------------------------------------

        // — 8) 헤더/바디 전송
        HAL_UART_Transmit(&huart2, (uint8_t*)respHdr, hdrLen, HAL_MAX_DELAY);
        // HAL_UART_Transmit(&huart1, (uint8_t*)respHdr, hdrLen, HAL_MAX_DELAY);
        if (!isIcon)
        {
            HAL_UART_Transmit(&huart2, (uint8_t*)htmlBody_inline_2, htmlBody_inline_2Len, HAL_MAX_DELAY);
        }

        // -------------------------------------

        // — 9) "SEND OK" URC 대기
        const char sendOk[] = "SEND OK";
        match = 0;
        start = HAL_GetTick();
        while (HAL_GetTick() - start < SEND_TIMEOUT)
        {
            if (HAL_UART_Receive(&huart2, &ch, 1, RX_TIMEOUT) == HAL_OK)
            {
                if (ch == sendOk[match])
                {
                    if (++match == (int)strlen(sendOk))
                    {
                        break;
                    }
                }
                else
                {
                    match = 0;
                }
            }
        }

        // -------------------------------------

        // — 10) 연결 종료
        // HAL_Delay(50);
        cmdLen = snprintf(cmd, sizeof(cmd), "AT+CIPCLOSE=%hu\r\n", linkID);
        ESP_AT_Send_Command_Sync_Get_Result(cmd);
        break; // HTML 응답 후 루프 종료
    }
}