/*
 * esp32_at.c
 *
 *  Created on: Jun 18, 2025
 *      Author: PROGRAM
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

void ESP_AT_Boot(void)
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

    // (1) AT+CWMODE? 전송
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


// SD카드에 WIFI 설정을 가져롸 AT 명령어로 ESP32에 전송하는 함수
int ESP_AT_Send_WiFi_Config(void)
{
    // SD_Card_Get_WiFi_SSID(void) 함수로 SSID를 가져옴
    const char* ssid = SD_Card_Get_WiFi_SSID();
    const char* password = SD_Card_Get_WiFi_Password();
    char cmd[128]={0}; // AT 명령어를 저장할 버퍼

    int len = snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, password);

    if (len < 0 || len >= (int)sizeof(cmd))
    {
        // 버퍼 오버플로우 또는 snprintf 실패
        Error_Handler();
    }

    ESP_AT_Send_Command_Sync(cmd);

    // // AT 명령어 형식으로 SSID와 비밀번호 설정
    // snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, password);

    // // AT 명령어 전송
    // if (HAL_UART_Transmit(&huart2, (uint8_t*)cmd, strlen(cmd), HAL_MAX_DELAY) != HAL_OK)
    // {
    //     // 전송 실패 처리
    //     return -1; // 실패
    // }

    // // 응답을 uart1로 전송
    // HAL_UART_Transmit(&huart1, (uint8_t*)cmd, strlen(cmd), HAL_MAX_DELAY);

    // return 0; // 성공
    return 1; // 성공
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




// 와이파이 동작하도록 설정하는 함수
void ESP_AT_Setup_WiFi(void)
{
    // WiFi 모드 설정 (STA 모드)
    // AT+CWMODE=1 : STA 모드로 설정
    // AT+CWMODE=2 : SoftAP 모드로 설정
    // AT+CWMODE=3 : STA + SoftAP 모드로 설정
    // ESP_AT_Send_Command("AT+CWMODE=1\r\n");

    // // WiFi 연결
    // if (ESP_AT_Send_WiFi_Config() != 0)
    // {
    //     // WiFi 설정 전송 실패 처리
    //     Error_Handler();
    // }

    // // 연결 상태 확인
    // ESP_AT_Send_Command("AT+CWJAP?\r\n");

    // // IP 주소 확인
    // ESP_AT_Send_Command("AT+CIFSR\r\n");
}


void ESP_AT_Get_Token(void)
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
    SERVER_API_Set_Token(token); // 서버 API 토큰 저장 함수 호출
}

// ESP32 기기 고유값 반환
void ESP_AT_Get_MAC_Address(void)
{
    // AT+CIPSTAMAC? 명령어 전송
    const char *cmd = "AT+CIPSTAMAC?\r\n";
    const char *response = ESP_AT_Send_Command_Sync_Get_Result(cmd);

    // 응답에서 MAC 주소 추출
    const char *macStart = strstr(response, "CIPSTAMAC:\""); // CIPSTAMAC:" 문자열 찾기
    static char macAddress[18] = {0}; // MAC 주소를 저장할 버퍼 (17자 + NULL)

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

    // PC(UART1)로 MAC 주소 전송
    HAL_UART_Transmit(&huart1, (uint8_t*)macAddress, strlen(macAddress), HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart1, (uint8_t*)"\r\n", 2, HAL_MAX_DELAY); // 줄바꿈 추가

    // MAC 주소 저장
    SERVER_API_Set_MAC_Address(macAddress); // 서버 API MAC 주소 저장 함수 호출
    // SD카드에 MAC 주소를 로그에 저장
    SD_Card_Log("MAC Address: ");
    SD_Card_Log(macAddress); // MAC 주소 로그에 저장
    SD_Card_Log("\r\n"); // 줄바꿈 추가

}

// 파라미터로 UTC 매크로를 받아서 시간은 설정하는 함수
void ESP_AT_Set_SNTP_Time(int utcOffset)
{
    char cmd[128];
    
    // SNTP 서버 연결
    // 한국 NTP 서버 설정
    // ESP_AT_Send_Command_Sync("AT+CIPSNTPCFG=1,900,\"pool.ntp.org\",\"time.google.com\"\r\n"); // NTP 서버 설정
    int len = snprintf(cmd, sizeof(cmd), "AT+CIPSNTPCFG=1,%d,\"pool.ntp.org\",\"time.google.com\"\r\n", utcOffset);

    if (len < 0 || len >= (int)sizeof(cmd)) 
    {
        // 버퍼 오버플로우 또는 snprintf 실패
        Error_Handler();
    }

    ESP_AT_Send_Command_Sync(cmd);

    // 설정 확인
    ESP_AT_Send_Command_Sync("AT+CIPSNTPCFG?\r\n");

    // 시간 확인
    ESP_AT_Send_Command_Sync("AT+CIPSNTPTIME?\r\n");
}