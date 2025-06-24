/*
 * server_api.c
 *
 *  Created on: Jun 23, 2025
 *      Author: PROGRAM
 */

#include "server_api.h"
#include "main.h"


__CCMRAM__ Server_API_Data g_serverApiData; // 서버 API 데이터 구조체 인스턴스

PServer_API_Data SERVER_API_Get_Data(void)
{
    // g_serverApiData의 주소를 반환
    return &g_serverApiData;
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