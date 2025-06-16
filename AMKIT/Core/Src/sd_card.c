#include "sd_card.h"


void sd_card_boot_init(void)
{

    FATFS SDFatFS;  // SD 카드 FATFS
    FIL SDFile;     // SD 카드 파일 핸들
    FRESULT fres;   // FATFS 함수 결과
    UINT bw, br;    // 바이트 쓰기/읽기 변수

    /* 1) SD 카드 마운트 (SDPath는 fatfs.c 에 extern으로 선언됨) */
    fres = f_mount(&SDFatFS, SDPath, 1);    // SDPath는 "0:"으로 설정되어 있어야 함
    if (fres != FR_OK)
    {
        // 마운트 실패 처리
        Error_Handler();
    }

    /* 2) 새 파일 생성(덮어쓰기) */
    fres = f_open(&SDFile, "test.txt", FA_CREATE_ALWAYS | FA_WRITE);
    if (fres != FR_OK)
    {
        // 파일 열기 실패 처리
        Error_Handler();
    }
    /* 3) 파일에 데이터 쓰기 */
    const char *data = "Hello, STM32 SD Card! \n test";
    fres = f_write(&SDFile, data, strlen(data), &bw);
    if (fres != FR_OK || bw < strlen(data))
    {
        // 파일 쓰기 실패 처리
        f_close(&SDFile);
        Error_Handler();
    }
    /* 4) 파일 닫기 */
    fres = f_close(&SDFile);
    if (fres != FR_OK)
    {
        // 파일 닫기 실패 처리
        Error_Handler();
    }
    /* 5) 파일 읽기 */
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
    /* 6) SD카드 언마운트*/
    fres = f_mount(NULL, SDPath, 1); // SDPath는 "0:"으로 설정되어 있어야 함 마운트 0, 언마운트 1
    if (fres != FR_OK)
    {
        // 언마운트 실패 처리
        Error_Handler();
    }
}

