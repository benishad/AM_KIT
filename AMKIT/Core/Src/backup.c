/*
 * backup.c
 *
 *  Created on: Jun 25, 2025
 *      Author: DONGYOONLEE
 */

// FM25V10 메모리 백업 관련 함수들
// PA4 = SPI CS
// PA5 = SPI1 SCK
// PA6 = SPI1 MISO
// PA7 = SPI1 MOSI

#include "backup.h"
#include "main.h"








// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// 파라미터로 위 매크로 명령어 받아서 전달하는 함수
void FM25V10_Select_Command(uint8_t command)
{
    // SPI CS 핀을 LOW로 설정하여
    HAL_SPI_Transmit(&hspi1, &command, 1, HAL_MAX_DELAY); // 명령어 전송
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// 내부 헬퍼: 쓰기 가능 상태 설정
static void FM25V10_Set_Write_Enable(void)
{
    uint8_t CMD = CMD_WREN; // WREN 명령어 정의

    SPI_CS_LOW(); // CS 핀을 LOW로 설정하여 SPI 통신 시작
    FM25V10_Select_Command(CMD); // WREN 명령 전송
    SPI_CS_HIGH(); // CS 핀을 HIGH로 설정하여 SPI 통신 종료
}

static void FM25V10_Set_Write_Disable(void)
{
    uint8_t CMD = CMD_WRDI; // WRDI 명령어 정의

    SPI_CS_LOW(); // CS 핀을 LOW로 설정하여 SPI 통신 시작
    FM25V10_Select_Command(CMD); // WRDI 명령 전송
    SPI_CS_HIGH(); // CS 핀을 HIGH로 설정하여 SPI 통신 종료
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void Memory_Write_Enable(void)
{
    // FM25V10 메모리 쓰기 가능 상태로 설정
    FM25V10_Set_Write_Enable();
}


void Memory_Write_Disable(void)
{
    // FM25V10 메모리 쓰기 불가능 상태로 설정
    FM25V10_Set_Write_Disable();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// 메모리에 쓰기
// addr : 메모리 주소 (0x0000 ~ 0x1FFF)
// pData: 쓸 데이터 버퍼  
// len  : 바이트 수  
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
HAL_StatusTypeDef FM25V10_Write(uint16_t addr, const uint8_t *pData, uint16_t len)
{
    // 길이가 0이면 예외
    if (len == 0 || pData == NULL)
    {
        return HAL_ERROR; // 잘못된 파라미터
    }

    // 1) 쓰기 허가
    Memory_Write_Enable();

    // 2) CS LOW , FRAM 활성화
    SPI_CS_LOW();

    // 3) WRITE 명령 + 주소
#if 0
    uint8_t header[3] = {
        CMD_WRITE,
        (addr >> 8) & 0xFF,
        (addr      ) & 0xFF
    };
    HAL_StatusTypeDef st = HAL_SPI_Transmit(&hspi1, header, 3, HAL_MAX_DELAY);
#endif
    uint8_t header[4] = {
        CMD_WRITE,
        (uint8_t)(addr >> 16),   // 상위 주소 바이트 (addr이 16비트라면 항상 0)
        (uint8_t)(addr >> 8),    // 중간 주소 바이트
        (uint8_t)(addr)          // 하위 주소 바이트
    };
    HAL_StatusTypeDef st = HAL_SPI_Transmit(&hspi1, header, sizeof(header), HAL_MAX_DELAY);
    if (st != HAL_OK)
    {
        SPI_CS_HIGH();
        return st;
    }

    // 4) 데이터 전송
    st = HAL_SPI_Transmit(&hspi1, (uint8_t*)pData, len, HAL_MAX_DELAY);

    // 5) CS HIGH
    SPI_CS_HIGH();

    // 6) 쓰기 종료
    Memory_Write_Disable();

    return st;
}

HAL_StatusTypeDef Memory_Write(uint16_t addr, const uint8_t *pData, uint16_t len)
{
    // 내부 드라이버 함수 호출
    return FM25V10_Write(addr, pData, len);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// 메모리에서 읽기
// addr : 메모리 주소 (0x0000 ~ 0x1FFF)
// pData: 읽은 데이터 버퍼
// len  : 바이트 수
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
HAL_StatusTypeDef FM25V10_Read(uint16_t addr, uint8_t *pData, uint16_t len)
{
    // 길이가 0이면 예외
    if (len == 0 || pData == NULL)
    {
        return HAL_ERROR; // 잘못된 파라미터
    }

    // 1) CS LOW , FRAM 활성화
    SPI_CS_LOW();

    // 2) READ 명령 + 주소
#if 0
    uint8_t header[3] = {
        CMD_READ,
        (addr >> 8) & 0xFF,
        (addr     ) & 0xFF
    };
    HAL_StatusTypeDef st = HAL_SPI_Transmit(&hspi1, header, 3, HAL_MAX_DELAY);
#endif
    uint8_t header[4] = {
        CMD_READ,
        (uint8_t)(addr >> 16),
        (uint8_t)(addr >> 8),
        (uint8_t)(addr)
    };
    HAL_StatusTypeDef st = HAL_SPI_Transmit(&hspi1, header, sizeof(header), HAL_MAX_DELAY);
    if (st != HAL_OK)
    {
        SPI_CS_HIGH();
        return st;
    }

    // 데이터 수신
    st = HAL_SPI_Receive(&hspi1, pData, len, HAL_MAX_DELAY);
    SPI_CS_HIGH();

    return st;
}

HAL_StatusTypeDef Memory_Read(uint16_t addr, uint8_t *pData, uint16_t len)
{
    // 내부 드라이버 함수 호출
    return FM25V10_Read(addr, pData, len);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// 상태 레지스터 쓰기
HAL_StatusTypeDef FM25V10_WriteStatus(uint8_t value)
{
    uint8_t tx[2] = { CMD_WRSR, value };
    Memory_Write_Enable();
    SPI_CS_LOW();
    HAL_SPI_Transmit(&hspi1, tx, 2, HAL_MAX_DELAY);
    SPI_CS_HIGH();
    Memory_Write_Disable();
    return HAL_OK;
}

HAL_StatusTypeDef Memory_WriteStatus(uint8_t value)
{
    // 내부 드라이버 함수 호출
    return FM25V10_WriteStatus(value);
}

// 상태 레지스터 읽기
uint8_t FM25V10_ReadStatus(void)
{
    uint8_t cmd = CMD_RDSR, status = 0;
    SPI_CS_LOW();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive (&hspi1, &status, 1, HAL_MAX_DELAY);
    SPI_CS_HIGH();
    return status;
}

// 상태 레지스터 읽기
uint8_t Memory_ReadStatus(void)
{
    // 내부 드라이버 함수 호출
    return FM25V10_ReadStatus();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// 전원 절약이 필요시

// 슬립 진입
void FM25V10_Sleep(void)
{
    uint8_t cmd = CMD_SLEEP;
    SPI_CS_LOW();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    SPI_CS_HIGH();
}

void Memory_Sleep(void)
{
    // 내부 드라이버 함수 호출
    FM25V10_Sleep();
}

// 웨이크업: 아무 명령 전송으로 풀림
void FM25V10_Wakeup(void)
{
    uint8_t dummy = 0xFF;
    SPI_CS_LOW();
    HAL_SPI_Transmit(&hspi1, &dummy, 1, HAL_MAX_DELAY);
    SPI_CS_HIGH();
}

void Memory_Wakeup(void)
{
    // 내부 드라이버 함수 호출
    FM25V10_Wakeup();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// ID 읽기
// FM25V10의 ID는 3바이트로 구성되어 있음
uint32_t FM25V10_ReadID(void)
{
    uint8_t cmd = CMD_RDID, idb[3];
    SPI_CS_LOW();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive (&hspi1, idb,  3, HAL_MAX_DELAY);
    SPI_CS_HIGH();
    return (idb[0]<<16)|(idb[1]<<8)|idb[2];
}

uint32_t Memory_ReadID(void)
{
    // 내부 드라이버 함수 호출
    return FM25V10_ReadID();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//    _______ _____ __  __ ______             _____ _______    _______ _    _  _____ 
//   |__   __|_   _|  \/  |  ____|           / ____|__   __|/\|__   __| |  | |/ ____|
//      | |    | | | \  / | |__     ______  | (___    | |  /  \  | |  | |  | | (___  
//      | |    | | | |\/| |  __|   |______|  \___ \   | | / /\ \ | |  | |  | |\___ \ .
//      | |   _| |_| |  | | |____            ____) |  | |/ ____ \| |  | |__| |____) |
//      |_|  |_____|_|  |_|______|          |_____/   |_/_/    \_\_|   \____/|_____/ 
//                                                                                   
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// 부팅 시 호출: 플래그 복원
HAL_StatusTypeDef Load_TimeStatus_FRAM(void)
{
    uint8_t flag;

    HAL_StatusTypeDef st = Memory_Read(TIME_FLAG_ADDR, &flag, 1);

    g_nTime_Status = (flag==DEVICE_TIME_SYNCED)
                    ? DEVICE_TIME_SYNCED
                    : DEVICE_TIME_NOT_SYNCED;
#if DEBUG_MODE
    // 저장된 시간 동기화 상태 g_nTime_Status을 uart1로 출력
    char timeStatusMsg[50];
    snprintf(timeStatusMsg, sizeof(timeStatusMsg), "LOAD Time Status: %d\n", flag);
    HAL_UART_Transmit(&huart1, (uint8_t*)timeStatusMsg, strlen(timeStatusMsg), HAL_MAX_DELAY);
#endif
                    
    return st;
}

// 동기화 완료 시 호출: 플래그 저장
HAL_StatusTypeDef Save_TimeStatus_FRAM(void)
{
    uint8_t flag = (uint8_t)g_nTime_Status;

#if DEBUG_MODE
    // 저장된 시간 동기화 상태 g_nTime_Status을 uart1로 출력
    char timeStatusMsg[50];
    snprintf(timeStatusMsg, sizeof(timeStatusMsg), "SAVE Time Status: %d\n", flag);
    HAL_UART_Transmit(&huart1, (uint8_t*)timeStatusMsg, strlen(timeStatusMsg), HAL_MAX_DELAY);
#endif

    return Memory_Write(TIME_FLAG_ADDR, &flag, 1);
}

// 시간 상태를 저장하는 더미 함수
HAL_StatusTypeDef Save_TimeStatus_FRAM_Dummy(void)
{
    uint8_t flag = (uint8_t)g_nTime_Status;

    // g_nTime_Status 값을 동기화 안 됨으로 설정
    g_nTime_Status = DEVICE_TIME_NOT_SYNCED;
    flag = (uint8_t)g_nTime_Status; // 다시 플래그로 저장

#if DEBUG_MODE
    // 저장된 시간 동기화 상태 g_nTime_Status을 uart1로 출력
    char timeStatusMsg[50];
    snprintf(timeStatusMsg, sizeof(timeStatusMsg), "SAVE Time Status: %d\n", flag);
    HAL_UART_Transmit(&huart1, (uint8_t*)timeStatusMsg, strlen(timeStatusMsg), HAL_MAX_DELAY);
#endif

    return Memory_Write(TIME_FLAG_ADDR, &flag, 1);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//   __          _______ ______ _____             _____ _______    _______ _    _  _____ 
//   \ \        / /_   _|  ____|_   _|           / ____|__   __|/\|__   __| |  | |/ ____|
//    \ \  /\  / /  | | | |__    | |    ______  | (___    | |  /  \  | |  | |  | | (___  
//     \ \/  \/ /   | | |  __|   | |   |______|  \___ \   | | / /\ \ | |  | |  | |\___ \ .
//      \  /\  /   _| |_| |     _| |_            ____) |  | |/ ____ \| |  | |__| |____) |
//       \/  \/   |_____|_|    |_____|          |_____/   |_/_/    \_\_|   \____/|_____/ 
//                                                                                       
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
HAL_StatusTypeDef Load_Wifi_Status_FRAM(void)
{
    uint8_t flag;

    HAL_StatusTypeDef st = Memory_Read(WIFI_FLAG_ADDR, &flag, 1);

    g_nWifi_Status = (flag==DEVICE_WIFI_CONNECTED)
                    ? DEVICE_WIFI_CONNECTED
                    : DEVICE_WIFI_DISCONNECTED;

#if DEBUG_MODE
    char timeStatusMsg[50];
    snprintf(timeStatusMsg, sizeof(timeStatusMsg), "LOAD Wifi Status: %d\n", flag);
    HAL_UART_Transmit(&huart1, (uint8_t*)timeStatusMsg, strlen(timeStatusMsg), HAL_MAX_DELAY);
#endif
    return st;
}

// 동기화 완료 시 호출: 플래그 저장
HAL_StatusTypeDef Save_Wifi_Status_FRAM(void)
{
    uint8_t flag = (uint8_t)g_nWifi_Status;

#if DEBUG_MODE
    char timeStatusMsg[50];
    snprintf(timeStatusMsg, sizeof(timeStatusMsg), "SAVE Wifi Status: %d\n", flag);
    HAL_UART_Transmit(&huart1, (uint8_t*)timeStatusMsg, strlen(timeStatusMsg), HAL_MAX_DELAY);
#endif

    return Memory_Write(WIFI_FLAG_ADDR, &flag, 1);
}

// 와이파이 상태를 저장하는 더미 함수
HAL_StatusTypeDef Save_Wifi_Status_FRAM_Dummy(void)
{
    uint8_t flag = (uint8_t)g_nWifi_Status;

    // g_nWifi_Status 값을 연결 끊기로 설정
    g_nWifi_Status = DEVICE_WIFI_DISCONNECTED;

    flag = (uint8_t)g_nWifi_Status; // 다시 플래그로 저장
    
#if DEBUG_MODE
    char timeStatusMsg[50];
    snprintf(timeStatusMsg, sizeof(timeStatusMsg), "SAVE Wifi Status: %d\n", flag);
    HAL_UART_Transmit(&huart1, (uint8_t*)timeStatusMsg, strlen(timeStatusMsg), HAL_MAX_DELAY);
#endif

    return Memory_Write(WIFI_FLAG_ADDR, &flag, 1);
}



// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//     _____ _____ _____ _____              _____ ____  _   _ ______ _____ _____ 
//    / ____/ ____|_   _|  __ \            / ____/ __ \| \ | |  ____|_   _/ ____|
//   | (___| (___   | | | |  | |  ______  | |   | |  | |  \| | |__    | || |  __ 
//    \___ \\___ \  | | | |  | | |______| | |   | |  | | . ` |  __|   | || | |_ |
//    ____) |___) |_| |_| |__| |          | |___| |__| | |\  | |     _| || |__| |
//   |_____/_____/|_____|_____/            \_____\____/|_| \_|_|    |_____\_____|
//                                                                               
// ==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// 와이파이 SSID 상태를 불러오는 함수
// 이 함수는 WiFi SSID 플래그를 읽고, g_nWifi_SSID_Status를 업데이트합니다.
HAL_StatusTypeDef Load_Wifi_SSID_Status_FRAM(void)
{
    uint8_t flag;

    HAL_StatusTypeDef st = Memory_Read(WIFI_SSID_FLAG_ADDR, &flag, 1);

    g_nWifi_SSID_Status = (flag==DEVICE_WIFI_SSID_SET)
                    ? DEVICE_WIFI_SSID_SET
                    : DEVICE_WIFI_SSID_NOT_SET;

#if DEBUG_MODE
    char timeStatusMsg[50];
    snprintf(timeStatusMsg, sizeof(timeStatusMsg), "LOAD Wifi SSID Status: %d\n", flag);
    HAL_UART_Transmit(&huart1, (uint8_t*)timeStatusMsg, strlen(timeStatusMsg), HAL_MAX_DELAY);
#endif
    return st;
}

// 와이파이 SSID 저장 함수
HAL_StatusTypeDef Save_Wifi_SSID_FRAM(const char *ssid)
{
    PAT_WiFi_Info pWiFiInfo = AT_Get_WiFi_Info();

    if (pWiFiInfo == NULL)
    {
        return HAL_ERROR; // WiFi 정보 구조체가 NULL인 경우
    }

    // ssid가 NULL이거나 길이가 64바이트를 초과하거나 빈 문자열인 경우
    if (ssid == NULL || strlen(ssid) + 1 > sizeof(pWiFiInfo->ssid) || *ssid == '\0')
    {
        return HAL_ERROR; // 잘못된 파라미터
    }

    uint8_t flag = DEVICE_WIFI_SSID_SET; // SSID가 설정됨을 나타내는 플래그

    // WiFi SSID 플래그 저장
    HAL_StatusTypeDef st = Memory_Write(WIFI_SSID_FLAG_ADDR, &flag, 1);
    if (st != HAL_OK)
    {
        return st; // 플래그 저장 실패
    }

    uint16_t len = (uint16_t)(strlen(ssid) + 1); // 문자열 길이 + NULL 문자

    // WiFi SSID 데이터 저장
    st = Memory_Write(WIFI_SSID_ADDR, (const uint8_t*)ssid, len);

    return st;
}

// 와이파이 SSID 읽기 함수
HAL_StatusTypeDef Load_Wifi_SSID_FRAM(void)
{
    // WiFi 정보를 저장하는 구조체를 가져옴
    PAT_WiFi_Info pWiFiInfo = AT_Get_WiFi_Info();
    if (pWiFiInfo == NULL)
    {
        return HAL_ERROR; // WiFi 정보 구조체가 NULL인 경우
    }

    uint8_t flag;
    HAL_StatusTypeDef st = Memory_Read(WIFI_SSID_FLAG_ADDR, &flag, 1);
    if (st != HAL_OK)
    {
        return st; // 플래그 읽기 실패
    }

    if (flag == DEVICE_WIFI_SSID_SET)
    {
        // SSID가 설정되어 있으면 SSID 데이터를 읽음
        st = Memory_Read(WIFI_SSID_ADDR, (uint8_t*)pWiFiInfo->ssid, sizeof(pWiFiInfo->ssid) - 1);
        if (st == HAL_OK)
        {
            pWiFiInfo->ssid[sizeof(pWiFiInfo->ssid) - 1] = '\0'; // 문자열 종료
        }
    }
    else
    {
        // SSID가 설정되어 있지 않으면 빈 문자열로 초기화
        pWiFiInfo->ssid[0] = '\0';
    }

    // WiFi SSID 상태를 업데이트
    g_nWifi_SSID_Status = (flag == DEVICE_WIFI_SSID_SET) ? DEVICE_WIFI_SSID_SET : DEVICE_WIFI_SSID_NOT_SET;

#if DEBUG_MODE
    // 로드된 WiFi SSID 상태를 uart1로 출력
    char ssidStatusMsg[50];
    snprintf(ssidStatusMsg, sizeof(ssidStatusMsg), "LOAD Wifi SSID Status: %d\n", g_nWifi_SSID_Status);
    HAL_UART_Transmit(&huart1, (uint8_t*)ssidStatusMsg, strlen(ssidStatusMsg), HAL_MAX_DELAY);
#endif
    return st;
}


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//    _______          __        _____ ____  _   _ ______ _____ _____ 
//   |  __ \ \        / /       / ____/ __ \| \ | |  ____|_   _/ ____|
//   | |__) \ \  /\  / /_____  | |   | |  | |  \| | |__    | || |  __ 
//   |  ___/ \ \/  \/ /______| | |   | |  | | . ` |  __|   | || | |_ |
//   | |      \  /\  /         | |___| |__| | |\  | |     _| || |__| |
//   |_|       \/  \/           \_____\____/|_| \_|_|    |_____\_____|
//                                                                    
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// 와이파이 PW 상태를 불러오는 함수
// 이 함수는 WiFi 비밀번호 플래그를 읽고, g_nWifi_Password_Status를 업데이트합니다.
HAL_StatusTypeDef Load_Wifi_Password_Status_FRAM(void)
{
    uint8_t flag;

    HAL_StatusTypeDef st = Memory_Read(WIFI_PASSWORD_FLAG_ADDR, &flag, 1);

    g_nWifi_Password_Status = (flag==DEVICE_WIFI_PASSWORD_SET)
                    ? DEVICE_WIFI_PASSWORD_SET
                    : DEVICE_WIFI_PASSWORD_NOT_SET;
#if DEBUG_MODE
    char timeStatusMsg[50];
    snprintf(timeStatusMsg, sizeof(timeStatusMsg), "LOAD Wifi Password Status: %d\n", g_nWifi_Password_Status);
    HAL_UART_Transmit(&huart1, (uint8_t*)timeStatusMsg, strlen(timeStatusMsg), HAL_MAX_DELAY);
#endif
    return st;
}

// 와이파이 PW 저장 함수
HAL_StatusTypeDef Save_Wifi_Password_FRAM(const char *password)
{
    PAT_WiFi_Info pWiFiInfo = AT_Get_WiFi_Info();

    if (pWiFiInfo == NULL)
    {
        return HAL_ERROR; // WiFi 정보 구조체가 NULL인 경우
    }

    // ssid가 NULL이거나 길이가 64바이트를 초과하거나 빈 문자열인 경우
    if (password == NULL || strlen(password) + 1 > sizeof(pWiFiInfo->password) || *password == '\0')
    {
        return HAL_ERROR; // 잘못된 파라미터
    }

    uint8_t flag = DEVICE_WIFI_PASSWORD_SET; // 비밀번호가 설정됨을 나타내는 플래그

    // WiFi 비밀번호 플래그 저장
    HAL_StatusTypeDef st = Memory_Write(WIFI_PASSWORD_FLAG_ADDR, &flag, 1);
    if (st != HAL_OK)
    {
        return st; // 플래그 저장 실패
    }

    uint16_t len = (uint16_t)(strlen(password) + 1); // 문자열 길이 + NULL 문자

    // WiFi 비밀번호 데이터 저장
    st = Memory_Write(WIFI_PASSWORD_ADDR, (const uint8_t*)password, len);

    return st;
}

// 와이파이 PW 읽기 함수
HAL_StatusTypeDef Load_Wifi_Password_FRAM(void)
{
    // WiFi 정보를 저장하는 구조체를 가져옴
    PAT_WiFi_Info pWiFiInfo = AT_Get_WiFi_Info();
    if (pWiFiInfo == NULL)
    {
        return HAL_ERROR; // WiFi 정보 구조체가 NULL인 경우
    }

    uint8_t flag;
    HAL_StatusTypeDef st = Memory_Read(WIFI_PASSWORD_FLAG_ADDR, &flag, 1);
    if (st != HAL_OK)
    {
        return st; // 플래그 읽기 실패
    }

    if (flag == DEVICE_WIFI_PASSWORD_SET)
    {
        // 비밀번호가 설정되어 있으면 비밀번호 데이터를 읽음
        st = Memory_Read(WIFI_PASSWORD_ADDR, (uint8_t*)pWiFiInfo->password, sizeof(pWiFiInfo->password) - 1);
        if (st == HAL_OK)
        {
            pWiFiInfo->password[sizeof(pWiFiInfo->password) - 1] = '\0'; // 문자열 종료
        }
    }
    else
    {
        // 비밀번호가 설정되어 있지 않으면 빈 문자열로 초기화
        pWiFiInfo->password[0] = '\0';
    }

    // WiFi 비밀번호 상태를 업데이트
    g_nWifi_Password_Status = (flag == DEVICE_WIFI_PASSWORD_SET) ? DEVICE_WIFI_PASSWORD_SET : DEVICE_WIFI_PASSWORD_NOT_SET;

#if DEBUG_MODE
    // 로드된 WiFi 비밀번호 상태를 uart1로 출력
    char passwordStatusMsg[50];
    snprintf(passwordStatusMsg, sizeof(passwordStatusMsg), "LOAD Wifi Password Status: %d\n", g_nWifi_Password_Status);
    HAL_UART_Transmit(&huart1, (uint8_t*)passwordStatusMsg, strlen(passwordStatusMsg), HAL_MAX_DELAY);
#endif
    return st;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//    _______ ____  _  ________ _   _        _____ _______      _    _  _____ 
//   |__   __/ __ \| |/ /  ____| \ | |      / ____|__   __|/\  | |  | |/ ____|
//      | | | |  | | ' /| |__  |  \| |_____| (___    | |  /  \ | |  | | (___  
//      | | | |  | |  < |  __| | . ` |______\___ \   | | / /\ \| |  | |\___ \ .
//      | | | |__| | . \| |____| |\  |      ____) |  | |/ ____ \ |__| |____) |
//      |_|  \____/|_|\_\______|_| \_|     |_____/   |_/_/    \_\____/|_____/ 
//                                                                            
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// 토큰 상태를 로드하는 함수
HAL_StatusTypeDef Load_Token_Status_FRAM(void)
{
    uint8_t flag;

    HAL_StatusTypeDef st = Memory_Read(TOKEN_FLAG_ADDR, &flag, 1);

    g_nToken_Status = (flag==DEVICE_TOKEN_SET)
                    ? DEVICE_TOKEN_SET
                    : DEVICE_TOKEN_NOT_SET;

    return st;
}

// 토큰 상태를 저장하는 함수
HAL_StatusTypeDef Save_Token_Status_FRAM(void)
{
    uint8_t flag = (uint8_t)g_nToken_Status;

    return Memory_Write(TOKEN_FLAG_ADDR, &flag, 1);
}

// 토큰을 저장하는 함수
HAL_StatusTypeDef Save_Token_FRAM(const char *token)
{
    PServer_API_Data pApiData = SERVER_API_Get_Data();
    if (pApiData == NULL)
    {
        return HAL_ERROR; // API 데이터 구조체가 NULL인 경우
    }

    if (token == NULL || strlen(token)+1 > sizeof(pApiData->sToken) || *token == '\0')
    {
        return HAL_ERROR; // 잘못된 파라미터
    }

    uint8_t flag = DEVICE_TOKEN_SET; // 토큰이 설정됨을 나타내는 플래그

    // 토큰 플래그 저장
    HAL_StatusTypeDef st = Memory_Write(TOKEN_FLAG_ADDR, &flag, 1);
    if (st != HAL_OK)
    {
        return st; // 플래그 저장 실패
    }

    uint16_t len = (uint16_t)(strlen(token)+1); // 문자열 길이 + NULL 문자

    // 토큰 데이터 저장
    st = Memory_Write(TOKEN_ADDR, (const uint8_t*)token, len);
    
    return st;
}

// 토큰을 읽는 함수
HAL_StatusTypeDef Load_Token_FRAM(void)
{
    // 토큰을 저장하는 구조체를 가져옴
    PServer_API_Data pApiData = SERVER_API_Get_Data();
    if (pApiData == NULL)
    {
        return HAL_ERROR; // API 데이터 구조체가 NULL인 경우
    }
    uint8_t flag;
    HAL_StatusTypeDef st = Memory_Read(TOKEN_FLAG_ADDR, &flag, 1);
    if (st != HAL_OK)
    {
        return st; // 플래그 읽기 실패
    }
    if (flag == DEVICE_TOKEN_SET)
    {
        // 토큰이 설정되어 있으면 토큰 데이터를 읽음
        st = Memory_Read(TOKEN_ADDR, (uint8_t*)pApiData->sToken, sizeof(pApiData->sToken) - 1);
        if (st == HAL_OK)
        {
            pApiData->sToken[sizeof(pApiData->sToken) - 1] = '\0'; // 문자열 종료
        }
    }
    else
    {
        // 토큰이 설정되어 있지 않으면 빈 문자열로 초기화
        pApiData->sToken[0] = '\0';
    }
    // 토큰 상태를 업데이트
    g_nToken_Status = (flag == DEVICE_TOKEN_SET) ? DEVICE_TOKEN_SET : DEVICE_TOKEN_NOT_SET;
    
#if DEBUG_MODE
    // 로드된 토큰 상태를 uart1로 출력
    char tokenStatusMsg[50];
    snprintf(tokenStatusMsg, sizeof(tokenStatusMsg), "LOAD Token Status: %d\n", flag);
    HAL_UART_Transmit(&huart1, (uint8_t*)tokenStatusMsg, strlen(tokenStatusMsg), HAL_MAX_DELAY);
    if (flag == DEVICE_TOKEN_SET)
    {
        // 토큰이 설정되어 있으면 토큰 값도 출력
        HAL_UART_Transmit(&huart1, (uint8_t*)pApiData->sToken, strlen(pApiData->sToken), HAL_MAX_DELAY);
        HAL_UART_Transmit(&huart1, (uint8_t*)"\n", 1, HAL_MAX_DELAY); // 줄바꿈 추가
    }
#endif
    return st;

}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//    __  __          _____      _____ _______      _    _  _____ 
//   |  \/  |   /\   / ____|    / ____|__   __|/\  | |  | |/ ____|
//   | \  / |  /  \ | |   _____| (___    | |  /  \ | |  | | (___  
//   | |\/| | / /\ \| |  |______\___ \   | | / /\ \| |  | |\___ \ .
//   | |  | |/ ____ \ |____     ____) |  | |/ ____ \ |__| |____) |
//   |_|  |_/_/    \_\_____|   |_____/   |_/_/    \_\____/|_____/ 
//                                                                
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// MAC 주소가 설정되어 있는지 확인하는 함수
HAL_StatusTypeDef Load_MAC_Status_FRAM(void)
{
    uint8_t flag;

    HAL_StatusTypeDef st = Memory_Read(MAC_FLAG_ADDR, &flag, 1);

    g_nMac_Status = (flag==DEVICE_MAC_SET)
                    ? DEVICE_MAC_SET
                    : DEVICE_MAC_NOT_SET;

#if DEBUG_MODE
    // 저장된 MAC 주소 상태 g_nMac_Status을 uart1로 출력
    char macStatusMsg[50];
    snprintf(macStatusMsg, sizeof(macStatusMsg), "LOAD MAC Status: %d\n", flag);
    HAL_UART_Transmit(&huart1, (uint8_t*)macStatusMsg, strlen(macStatusMsg), HAL_MAX_DELAY);
#endif

    return st;
}

// Mac 주소 상태를 저장하는 함수
HAL_StatusTypeDef Save_MAC_Status_FRAM(void)
{
    uint8_t flag = (uint8_t)g_nMac_Status;

#if DEBUG_MODE
    // 저장된 MAC 주소 상태 g_nMac_Status을 uart1로 출력
    char macStatusMsg[50];
    snprintf(macStatusMsg, sizeof(macStatusMsg), "SAVE MAC Status: %d\n", flag);
    HAL_UART_Transmit(&huart1, (uint8_t*)macStatusMsg, strlen(macStatusMsg), HAL_MAX_DELAY);
#endif

    return Memory_Write(MAC_FLAG_ADDR, &flag, 1);
}

// MAC 주소를 저장하는 함수
HAL_StatusTypeDef Save_MAC_FRAM(const char *mac)
{
    if (mac == NULL)
    {
        return HAL_ERROR; // 잘못된 파라미터
    }

    uint8_t flag = DEVICE_MAC_SET; // MAC 주소가 설정됨을 나타내는 플래그

    // MAC 주소 플래그 저장
    HAL_StatusTypeDef st = Memory_Write(MAC_FLAG_ADDR, &flag, 1);
    if (st != HAL_OK)
    {
        return st; // 플래그 저장 실패
    }

    // MAC 주소 데이터 저장
    st = Memory_Write(MAC_ADDR, (const uint8_t*)mac, strlen(mac)+1); // MAC 주소는 6바이트

    return st;
}

// MAC 주소를 읽는 함수
HAL_StatusTypeDef Load_MAC_FRAM(void)
{
    // MAC 주소를 저장하는 구조체를 가져옴
    PServer_API_Data pApiData = SERVER_API_Get_Data();
    if (pApiData == NULL)
    {
        return HAL_ERROR; // API 데이터 구조체가 NULL인 경우
    }
    
    uint8_t flag;
    HAL_StatusTypeDef st = Memory_Read(MAC_FLAG_ADDR, &flag, 1);
    if (st != HAL_OK)
    {
        return st; // 플래그 읽기 실패
    }
    
    if (flag == DEVICE_MAC_SET)
    {
        // MAC 주소가 설정되어 있으면 MAC 주소 데이터를 읽음
        st = Memory_Read(MAC_ADDR, (uint8_t*)pApiData->sMac, sizeof(pApiData->sMac) - 1);
        if (st == HAL_OK)
        {
            // MAC 주소는 6바이트이므로 문자열 종료 필요 없음
            pApiData->sMac[sizeof(pApiData->sMac)-1] = '\0'; // 문자열 종료
        }
    }
    else
    {
        // MAC 주소가 설정되어 있지 않으면 빈 문자열로 초기화
        memset(pApiData->sMac, 0, sizeof(pApiData->sMac));
    }
    
    // MAC 주소 상태를 업데이트
    g_nMac_Status = (flag == DEVICE_MAC_SET) ? DEVICE_MAC_SET : DEVICE_MAC_NOT_SET;

#if DEBUG_MODE
    // 로드된 MAC 주소 상태를 uart1로 출력
    char macStatusMsg[50];
    snprintf(macStatusMsg, sizeof(macStatusMsg), "LOAD MAC Status: %d\n", flag);
    HAL_UART_Transmit(&huart1, (uint8_t*)macStatusMsg, strlen(macStatusMsg), HAL_MAX_DELAY);
    if (flag == DEVICE_MAC_SET)
    {
        // MAC 주소가 설정되어 있으면 MAC 주소 값도 출력
        HAL_UART_Transmit(&huart1, (uint8_t*)pApiData->sMac, strlen(pApiData->sMac), HAL_MAX_DELAY);
        HAL_UART_Transmit(&huart1, (uint8_t*)"\n", 1, HAL_MAX_DELAY); // 줄바꿈 추가
    }
#endif

    return st;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// 초기화 함수
void FRAM_Init(void)
{
    SPI_CS_HIGH();
}
