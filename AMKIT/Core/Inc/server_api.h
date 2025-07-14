/*
 * server_api.h
 *
 *  Created on: Jun 23, 2025
 *      Author: DONGYOONLEE
 */

#ifndef INC_SERVER_API_H_
#define INC_SERVER_API_H_


// 서버 API 관련 변수 구조체
typedef struct tag_Server_API_Data
{
    char sToken[128];  // 토큰을 저장할 버퍼
    char sMac[18];     // MAC 주소 (xx:xx:xx:xx:xx:xx)? 
    char sUid[64];     // 사용자 ID
    char sPwd[64];     // 비밀번호
} Server_API_Data, *PServer_API_Data;

PServer_API_Data SERVER_API_Get_Data(void);

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// CCMRAM 초기화
void SERVER_API_Init(void);

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void SERVER_API_Set_Token(const char* token);
void SERVER_API_Set_MAC_Address(const char* mac);

int Server_Send_Boot(void);
int Test_Server_Send_Boot(void);

#endif /* INC_SERVER_API_H_ */
