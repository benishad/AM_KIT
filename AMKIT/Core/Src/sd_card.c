/*
 * sd_card.c
 *
 *  Created on: Jun 17, 2025
 *      Author: DONGYOONLEE
 */

#include "sd_card.h"
#include "fatfs.h"
#include <string.h>




// =========================================================
int SD_Card_Boot(void)
{
    FATFS SDFatFS;  // FATFS 구조체
    FIL SDFile;     // 파일 구조체
    FRESULT fres;   // FATFS 함수 결과
    UINT bw, br;    // 바이트 쓰기/읽기 변수

    int result = 0; // 결과 변수 초기화

    // --------------------------------------------------
#if 0
    // 1) SD 카드 마운트
    fres = f_mount(&SDFatFS, SDPath, 1); // SDPath는 "0:"으로 설정되어 있어야 함
    if (fres != FR_OK)
    {
        // 마운트 실패 처리
        Error_Handler();
    }

    // 2) 새 파일 생성(덮어쓰기)
    fres = f_open(&SDFile, "test.txt", FA_CREATE_ALWAYS | FA_WRITE);
    if (fres != FR_OK)
    {
        // 파일 열기 실패 처리
        Error_Handler();
    }

    // 3) 파일에 데이터 쓰기
    const char *data = "Hello, STM32 SD Card! \n test";
    fres = f_write(&SDFile, data, strlen(data), &bw);
    if (fres != FR_OK || bw < strlen(data))
    {
        // 파일 쓰기 실패 처리
        f_close(&SDFile);
        Error_Handler();
    }

    // 4) 파일 닫기
    fres = f_close(&SDFile);
    if (fres != FR_OK)
    {
        // 파일 닫기 실패 처리
        Error_Handler();
    }

    // 5) 파일 읽기
    fres = f_open(&SDFile, "test.txt", FA_READ);
    if (fres != FR_OK)
    {
        // 파일 열기 실패 처리
        Error_Handler();
    }

    char readBuffer[64];    // 읽기 버퍼
    fres = f_read(&SDFile, readBuffer, sizeof(readBuffer) - 1, &br);
    if (fres != FR_OK || br == 0)
    {
        // 파일 읽기 실패 처리
        f_close(&SDFile);
        Error_Handler();
    }
    readBuffer[br] = '\0'; // 문자열 종료

    // 6) 읽은 데이터 출력 (디버그용)
    // printf("Read from SD Card: %s\n", readBuffer); // 디버그 출력

    // 7) SD 카드 언마운트
    fres = f_mount(NULL, SDPath, 1); // SDPath는 "0:"으로 설정되어 있어야 함 마운트 0, 언마운트 1
    if (fres != FR_OK)
    {
        // 언마운트 실패 처리
        Error_Handler();
    }
#endif // 0
    // --------------------------------------------------

    // 1) SD 카드 마운트
    fres = f_mount(&SDFatFS, SDPath, 1); // SDPath는 "0:"으로 설정되어 있어야 함
    if (fres != FR_OK)
    {
        // 마운트 실패 처리
        Error_Handler();
    }

    // 2) 와이파이 파일 있는지 확인
    fres = f_open(&SDFile, "wifi.txt", FA_READ);
    if (fres == FR_OK)
    {
        // 파일이 존재하면 OK Sd카드 언마운트
        f_close(&SDFile);
        fres = f_mount(NULL, SDPath, 1); // SDPath는 "0:"으로 설정되어 있어야 함 마운트 0, 언마운트 1
        if (fres != FR_OK)
        {
            SD_Card_Log("SD Card Unmount Failed!\n");
            // 언마운트 실패 처리
            // Error_Handler();
            Error_Proc(1);

            result = SD_ERROR;
        }
        
        result = SD_OK; // 파일이 존재하면 OK
    }
    else
    {
        // 파일이 없으면 생성
        fres = f_open(&SDFile, "wifi.txt", FA_CREATE_ALWAYS | FA_WRITE);
        if (fres != FR_OK)
        {
            // 파일 열기 실패 처리
            // Error_Handler();
            Error_Proc(1);
            SD_Card_Log("SD Card File Open Failed!\n");
            result = SD_ERROR;
        }
        // 파일에 기본 Wi-Fi 설정 데이터 쓰기
        const char *wifiData = "SSID=YourSSID\nPassword=YourPassword\n";
        fres = f_write(&SDFile, wifiData, strlen(wifiData), &bw);
        if (fres != FR_OK || bw < strlen(wifiData))
        {
            // 파일 쓰기 실패 처리
            f_close(&SDFile);
            // Error_Handler();
            Error_Proc(1);
            SD_Card_Log("SD Card File Write Failed!\n");
            result = SD_ERROR;
        }
        // 파일 닫기
        fres = f_close(&SDFile);
        if (fres != FR_OK)
        {
            // 파일 닫기 실패 처리
            // Error_Handler();
            Error_Proc(1);
            SD_Card_Log("SD Card File Close Failed!\n");
            result = SD_ERROR;
        }

        result = SD_OK; // 파일 생성 성공
    }

    return result; // SD 카드 부팅 결과 반환
}


// WIFI SSID를 반환하는 함수
const char* SD_Card_Get_WiFi_SSID(void)
{
    static char ssid[32]; // SSID를 저장할 버퍼
    FATFS SDFatFS;        // FATFS 구조체
    FIL SDFile;           // 파일 구조체
    FRESULT fres;         // FATFS 함수 결과
    UINT br;              // 바이트 읽기 변수

    // --------------------------------------------------

    // 1) SD 카드 마운트
    fres = f_mount(&SDFatFS, SDPath, 1); // SDPath는 "0:"으로 설정되어 있어야 함
    if (fres != FR_OK)
    {
        // 마운트 실패 처리
        Error_Handler();
    }

    // 2) Wi-Fi 설정 파일 열기
    fres = f_open(&SDFile, "wifi.txt", FA_READ);
    if (fres != FR_OK)
    {
        // 파일 열기 실패 처리
        Error_Handler();
    }

    // 3) 파일에서 SSID 읽기
    char buffer[64]; // 임시 버퍼
    fres = f_read(&SDFile, buffer, sizeof(buffer) - 1, &br);
    if (fres != FR_OK || br == 0)
    {
        // 파일 읽기 실패 처리
        f_close(&SDFile);
        Error_Handler();
    }
    
    buffer[br] = '\0'; // 문자열 종료

    // 4) SSID (최초 부트에서 "SSID=YourSSID" 형식으로 저장)
    sscanf(buffer, "SSID=%31s", ssid);

    // 5) 파일 닫기
    fres = f_close(&SDFile);
    if (fres != FR_OK)
    {
        // 파일 닫기 실패 처리
        Error_Handler();
    }

    // 6) SD 카드 언마운트
    fres = f_mount(NULL, SDPath, 1); // SDPath는 "0:"으로 설정되어 있어야 함 마운트 0, 언마운트 1
    if (fres != FR_OK)
    {
        // 언마운트 실패 처리
        Error_Handler();
    }

    return ssid; // SSID 반환
}

// WIFI 비밀번호를 반환하는 함수
const char* SD_Card_Get_WiFi_Password(void)
{
    static char password[32]; // 비밀번호를 저장할 버퍼
    FATFS SDFatFS;            // FATFS 구조체
    FIL SDFile;               // 파일 구조체
    FRESULT fres;             // FATFS 함수 결과
    UINT br;                  // 바이트 읽기 변수

    // --------------------------------------------------

    // 1) SD 카드 마운트
    fres = f_mount(&SDFatFS, SDPath, 1); // SDPath는 "0:"으로 설정되어 있어야 함
    if (fres != FR_OK)
    {
        // 마운트 실패 처리
        Error_Handler();
    }

    // 2) Wi-Fi 설정 파일 열기
    fres = f_open(&SDFile, "wifi.txt", FA_READ);
    if (fres != FR_OK)
    {
        // 파일 열기 실패 처리
        Error_Handler();
    }

    // 3) 파일에서 비밀번호 읽기
    char buffer[64]; // 임시 버퍼
    fres = f_read(&SDFile, buffer, sizeof(buffer) - 1, &br);
    if (fres != FR_OK || br == 0)
    {
        // 파일 읽기 실패 처리
        f_close(&SDFile);
        Error_Handler();
    }
    
    buffer[br] = '\0'; // 문자열 종료

    // 4) 비밀번호 (최초 부트에서 "Password=YourPassword" 형식으로 저장)
    const char *p = strstr(buffer, "Password=");
    if(p)
    {
        sscanf(p, "Password=%31s", password);
    }
    // sscanf(buffer, "Password=%31s", password);

    // 5) 파일 닫기
    fres = f_close(&SDFile);
    if (fres != FR_OK)
    {
        // 파일 닫기 실패 처리
        Error_Handler();
    }

    // 6) SD 카드 언마운트
    fres = f_mount(NULL, SDPath, 1); // SDPath는 "0:"으로 설정되어 있어야 함 마운트 0, 언마운트 1
    if (fres != FR_OK)
    {
        // 언마운트 실패 처리
        Error_Handler();
    }

    return password; // 비밀번호 반환
}



// SD카드에 로그를 남기는 함수
void SD_Card_Log(const char *logMessage)
{
    FATFS SDFatFS;  // FATFS 구조체
    FIL SDFile;     // 파일 구조체
    FRESULT fres;   // FATFS 함수 결과
    UINT bw;        // 바이트 쓰기 변수

    // --------------------------------------------------

    // 1) SD 카드 마운트
    fres = f_mount(&SDFatFS, SDPath, 1); // SDPath는 "0:"으로 설정되어 있어야 함
    if (fres != FR_OK)
    {
        // 마운트 실패 처리
        Error_Handler();
    }

    // 2) 로그 파일 열기(없으면 생성)
    fres = f_open(&SDFile, "log.txt", FA_OPEN_APPEND | FA_WRITE);
    if (fres != FR_OK)
    {
        // 파일 열기 실패 처리
        Error_Handler();
    }

    // 3) 로그 메시지 쓰기
    fres = f_write(&SDFile, logMessage, strlen(logMessage), &bw);
    if (fres != FR_OK || bw < strlen(logMessage))
    {
        // 파일 쓰기 실패 처리
        f_close(&SDFile);
        Error_Handler();
    }

    // 4) 파일 닫기
    fres = f_close(&SDFile);
    if (fres != FR_OK)
    {
        // 파일 닫기 실패 처리
        Error_Handler();
    }

    // 5) SD 카드 언마운트
    fres = f_mount(NULL, SDPath, 1); // SDPath는 "0:"으로 설정되어 있어야 함 마운트 0, 언마운트 1
    if (fres != FR_OK)
    {
        // 언마운트 실패 처리
        Error_Handler();
    }
}

// SD카드가 있는지 확인하여 반환하는 함수
int SD_Card_Is_Exist(void)
{
    FATFS SDFatFS;  // FATFS 구조체
    FRESULT fres;   // FATFS 함수 결과

    // --------------------------------------------------

    // 1) SD 카드 마운트 시도
    fres = f_mount(&SDFatFS, SDPath, 1); // SDPath는 "0:"으로 설정되어 있어야 함
    if (fres == FR_OK)
    {
        // 마운트 성공 시 언마운트 후 OK 반환
        f_mount(NULL, SDPath, 1); // SDPath는 "0:"으로 설정되어 있어야 함 마운트 0, 언마운트 1
        return SD_OK; // SD 카드 존재
    }
    
    return SD_ERROR; // SD 카드 없음
}