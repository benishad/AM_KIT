/*
 * server_api.c
 *
 *  Created on: Jun 23, 2025
 *      Author: DONGYOONLEE
 */

#include "server_api.h"
#include "main.h"


__CCMRAM__ Server_API_Data g_serverApiData; // 서버 API 데이터 구조체 인스턴스

PServer_API_Data SERVER_API_Get_Data(void)
{
    // g_serverApiData의 주소를 반환
    return &g_serverApiData;
}

// CCMRAM 초기화
void SERVER_API_Init(void)
{
    // g_serverApiData 구조체를 0으로 초기화
    memset(&g_serverApiData, 0, sizeof(g_serverApiData));
}


// 토큰을 파라미터로 받아서 g_serverApiData에 저장하는 함수
void SERVER_API_Set_Token(const char* token)
{
    if (token != NULL && strlen(token) < sizeof(g_serverApiData.sToken))
    {
        strncpy(g_serverApiData.sToken, token, sizeof(g_serverApiData.sToken) - 1);
        g_serverApiData.sToken[sizeof(g_serverApiData.sToken) - 1] = '\0'; // 문자열 종료
    }
}

// MAC 주소를 파라미터로 받아서 g_serverApiData에 저장하는 함수
void SERVER_API_Set_MAC_Address(const char* mac)
{
    if (mac != NULL && strlen(mac) < sizeof(g_serverApiData.sMac))
    {
        strncpy(g_serverApiData.sMac, mac, sizeof(g_serverApiData.sMac) - 1);
        g_serverApiData.sMac[sizeof(g_serverApiData.sMac) - 1] = '\0'; // 문자열 종료
    }
}

// ──────────────────────────────────────────────────────────────────────────────
// ──────────────────────────────────────────────────────────────────────────────
// ──────────────────────────────────────────────────────────────────────────────
// =======================================================================================================
// =======================================================================================================


// AT 명령어로 Json 날리기 , 서버에 날리기, 함수는 성공 여부를 반환
int Server_Send_Boot(void)
{
    // 토큰을 가져올 구조체
    PServer_API_Data pServerApiData = SERVER_API_Get_Data();


    int result = 0; // 결과 변수 초기화
    const char *success = "Boot Data sent successfully!\r\n";
    const char *fail = "Failed to send Boot Data!\r\n";
    const char *tokenCheck = "Token is empty or NULL\r\n";
    const char *macPCheck = "MAC-P address is empty or NULL\r\n";
    const char *macCCheck = "MAC-C address is empty or NULL\r\n";

    const char* token = pServerApiData->sToken; // 서버 API 토큰 가져오기
    const char* macP = pServerApiData->sMac; // MAC 주소 가져오기
    const char* macC = pServerApiData->sMac; // MAC 주소 가져오기

    char jsonData[256]={0}; // AT 명령어를 저장할 버퍼 // 토큰이 128바이트 공간 차지하므로 커맨드 버퍼는 충분히 커야 함

    // 토큰 값 유효한지 확인
    if (token == NULL || strlen(token) == 0)
    {
        // 토큰이 비어있으면 에러 처리
        HAL_UART_Transmit(&huart1, (uint8_t*)tokenCheck, strlen(tokenCheck), HAL_MAX_DELAY);
        return AT_ERROR; // 실패 코드
    }
    // MAC 주소 값 유효한지 확인
    if (macP == NULL || strlen(macP) == 0)
    {
        // MAC 주소가 비어있으면 에러 처리
        HAL_UART_Transmit(&huart1, (uint8_t*)macPCheck, strlen(macPCheck), HAL_MAX_DELAY);
        return AT_ERROR; // 실패 코드
    }
    // MAC 주소 값 유효한지 확인
    if (macC == NULL || strlen(macC) == 0)
    {
        // MAC 주소가 비어있으면 에러 처리
        HAL_UART_Transmit(&huart1, (uint8_t*)macCCheck, strlen(macCCheck), HAL_MAX_DELAY);
        return AT_ERROR; // 실패 코드
    }
    
    // 1) 보낼 JSON 문자열 정의 (큰 따옴표는 백슬래시로 이스케이프)
    int jsonDataLen = snprintf(
        jsonData, 
        sizeof(jsonData), 
        "{\\\"token\\\":\\\"%s\\\"\\,\\\"articleid_p\\\":\\\"%s\\\"\\,\\\"articleid\\\":\\\"%s\\\"}",
        token, macP, macP
    );

    if (jsonDataLen < 0 || jsonDataLen >= (int)sizeof(jsonData)) 
    {
        // 버퍼 오버플로우 또는 snprintf 실패
        Error_Handler();
        result = AT_ERROR; // 실패 코드
    }
    
    char atCmd[512]; // 충분히 큰 버퍼

    int fullJson = snprintf(atCmd, sizeof(atCmd),
        "AT+HTTPCLIENT=3,1,\"https://dev-api.andamiro.net/test/boot\",\"dev-api.andamiro.net\",\"/test/boot\",2,\"%s\"\r\n",
        jsonData);

    if (fullJson < 0 || fullJson >= (int)sizeof(atCmd)) 
    {
        // snprintf 실패 또는 버퍼 부족
        Error_Handler();
        result = AT_ERROR; // 실패 코드
    }

    // AT 명령어 전송 및 응답 처리
    const char *response = ESP_AT_Send_Command_Sync_Get_Result(atCmd);
    
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

    return result; // 결과 반환
}



// AT 명령어로 Json 날리기 , 서버에 날리기, 함수는 성공 여부를 반환
int Test_Server_Send_Boot(void)
{
    // 토큰을 가져올 구조체
    PServer_API_Data pServerApiData = SERVER_API_Get_Data();


    int result = 0; // 결과 변수 초기화
    const char *success = "Boot Data sent successfully!\r\n";
    const char *fail = "Failed to send Boot Data!\r\n";
    const char *tokenCheck = "Token is empty or NULL\r\n";
    const char *macPCheck = "MAC-P address is empty or NULL\r\n";
    const char *macCCheck = "MAC-C address is empty or NULL\r\n";

    const char* token = pServerApiData->sToken; // 서버 API 토큰 가져오기
    const char* macP = pServerApiData->sMac; // MAC 주소 가져오기
    const char* macC = pServerApiData->sMac; // MAC 주소 가져오기

    char jsonData[256]={0}; // AT 명령어를 저장할 버퍼 // 토큰이 128바이트 공간 차지하므로 커맨드 버퍼는 충분히 커야 함

    // 토큰 값 유효한지 확인
    if (token == NULL || strlen(token) == 0)
    {
        // 토큰이 비어있으면 에러 처리
        HAL_UART_Transmit(&huart1, (uint8_t*)tokenCheck, strlen(tokenCheck), HAL_MAX_DELAY);
        return AT_ERROR; // 실패 코드
    }
    // MAC 주소 값 유효한지 확인
    if (macP == NULL || strlen(macP) == 0)
    {
        // MAC 주소가 비어있으면 에러 처리
        HAL_UART_Transmit(&huart1, (uint8_t*)macPCheck, strlen(macPCheck), HAL_MAX_DELAY);
        return AT_ERROR; // 실패 코드
    }
    // MAC 주소 값 유효한지 확인
    if (macC == NULL || strlen(macC) == 0)
    {
        // MAC 주소가 비어있으면 에러 처리
        HAL_UART_Transmit(&huart1, (uint8_t*)macCCheck, strlen(macCCheck), HAL_MAX_DELAY);
        return AT_ERROR; // 실패 코드
    }
    
    // 1) 보낼 JSON 문자열 정의 (큰 따옴표는 백슬래시로 이스케이프)
    int jsonDataLen = snprintf(
        jsonData, 
        sizeof(jsonData), 
        "{\\\"token\\\":\\\"%s\\\"\\,\\\"articleid_p\\\":\\\"%s\\\"\\,\\\"articleid\\\":\\\"%s\\\"}",
        token, macP, macP
    );

    if (jsonDataLen < 0 || jsonDataLen >= (int)sizeof(jsonData)) 
    {
        // 버퍼 오버플로우 또는 snprintf 실패
        Error_Handler();
        result = AT_ERROR; // 실패 코드
    }
    
    char atCmd[512]; // 충분히 큰 버퍼

    int fullJson = snprintf(atCmd, sizeof(atCmd),
        "AT+HTTPCLIENT=3,1,\"https://esp32-log-server.onrender.com/data\",\"esp32-log-server.onrender.com\",\"/data\",2,\"%s\"\r\n",
        jsonData);

    if (fullJson < 0 || fullJson >= (int)sizeof(atCmd)) 
    {
        // snprintf 실패 또는 버퍼 부족
        Error_Handler();
        result = AT_ERROR; // 실패 코드
    }

    // AT 명령어 전송 및 응답 처리
    const char *response = ESP_AT_Send_Command_Sync_Get_Result(atCmd);
    
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

    return result; // 결과 반환
}
