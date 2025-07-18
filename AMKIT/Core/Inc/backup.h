/*
 * backup.h
 *
 *  Created on: Jun 25, 2025
 *      Author: DONGYOONLEE
 */

#ifndef INC_BACKUP_H_
#define INC_BACKUP_H_

#include <stdint.h>
#include "stm32f4xx_hal.h"


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#define SPI_CS_LOW()   HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET)
#define SPI_CS_HIGH()  HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET)





// FM25V10 명령어
#define CMD_WREN            0x06    // Write Enable           // op-code 0000 0110
#define CMD_WRDI            0x04    // Write Disable          // op-code 0000 0100
#define CMD_RDSR            0x05    // Read Status Register   // op-code 0000 0101
#define CMD_WRSR            0x01    // Write Status Register  // op-code 0000 0001
#define CMD_READ            0x03    // Read Memory Data       // op-code 0000 0011
#define CMD_FSTRD           0x0B    // Fast Read Memory Data  // op-code 0000 1011
#define CMD_WRITE           0x02    // Write Memory Data      // op-code 0000 0010
#define CMD_SLEEP           0xB9    // Sleep Mode             // op-code 1011 1001
#define CMD_RDID            0x9F    // Read ID                // op-code 1001 1111
#define CMD_SNR             0xC3    // Read Serial Number     // op-code 1100 0011






// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//    _____            __  __            _____  _____  _____  
//   |  __ \     /\   |  \/  |     /\   |  __ \|  __ \|  __ \ .
//   | |__) |   /  \  | \  / |    /  \  | |  | | |  | | |__) |
//   |  _  /   / /\ \ | |\/| |   / /\ \ | |  | | |  | |  _  / 
//   | | \ \  / ____ \| |  | |  / ____ \| |__| | |__| | | \ \ .
//   |_|  \_\/_/    \_\_|  |_| /_/    \_\_____/|_____/|_|  \_\.
//                                                            
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// 주소는 임의 지정
#define TIME_FLAG_ADDR      0x0000  // FRAM 상의 저장 주소 // 시간 플래그 저장 위치
#define WIFI_FLAG_ADDR      0x0001  // FRAM 상의 저장 주소 // 시간 플래그 저장 위치

// 토큰이 있는지 확인하는 플래그
#define TOKEN_FLAG_ADDR     0x0010  // FRAM 상의 저장 주소 // 토큰 플래그 저장 위치
#define TOKEN_ADDR          0x0011  // FRAM 상의 저장 주소 // 토큰 저장 위치
// 토큰 최대 길이 128바이트 널 종료 포함
#define TOKEN_MAX_LEN       128

// MAC 주소가 있는지 확인하는 플래그
#define MAC_FLAG_ADDR       (TOKEN_ADDR + TOKEN_MAX_LEN)  // 0x0011 + 128 = 0x0091 // MAC 플래그 저장 위치
#define MAC_ADDR            (MAC_FLAG_ADDR + 1)  // 0x0092 // MAC 주소 저장 위치
// MAC 주소 최대 길이 6바이트
#define MAC_MAX_LEN         6

// WiFi SSID 가 있는지 확인하는 플래그
#define WIFI_SSID_FLAG_ADDR (MAC_ADDR + MAC_MAX_LEN)  // 0x0092 + 6 = 0x0098 // WiFi SSID 플래그 저장 위치
#define WIFI_SSID_ADDR      (WIFI_SSID_FLAG_ADDR + 1)  // 0x0099 // WiFi SSID 저장 위치
// WiFi SSID 최대 길이 64바이트 널 종료 포함

// WiFi 비밀번호가 있는지 확인하는 플래그
#define WIFI_PASSWORD_FLAG_ADDR (WIFI_SSID_ADDR + 64)  // 0x0099 + 64 = 0x00B9 // WiFi 비밀번호 플래그 저장 위치
#define WIFI_PASSWORD_ADDR  (WIFI_PASSWORD_FLAG_ADDR + 1)  // 0x00BA // WiFi 비밀번호 저장 위치
// WiFi 비밀번호 최대 길이 64바이트 널 종료 포함

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// 기본 조작 함수

HAL_StatusTypeDef FM25V10_Write(uint16_t addr, const uint8_t *pData, uint16_t len);
HAL_StatusTypeDef FM25V10_Read(uint16_t addr, uint8_t *pData, uint16_t len);

HAL_StatusTypeDef Memory_Write(uint16_t addr, const uint8_t *pData, uint16_t len);
HAL_StatusTypeDef Memory_Read(uint16_t addr, uint8_t *pData, uint16_t len);


void Memory_Write_Enable(void);
void Memory_Write_Disable(void);

HAL_StatusTypeDef Memory_WriteStatus(uint8_t value);
uint8_t Memory_ReadStatus(void);

void Memory_Sleep(void);
void Memory_WakeUp(void);

uint32_t Memory_ReadID(void);

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// 시간 상태 함수
HAL_StatusTypeDef Load_TimeStatus_FRAM(void);
HAL_StatusTypeDef Save_TimeStatus_FRAM(void);
HAL_StatusTypeDef Save_TimeStatus_FRAM_Dummy(void);

// 와이파이 상태 함수
HAL_StatusTypeDef Load_Wifi_Status_FRAM(void);
HAL_StatusTypeDef Save_Wifi_Status_FRAM(void);
HAL_StatusTypeDef Save_Wifi_Status_FRAM_Dummy(void);

// 와이파이 SSID 함수
HAL_StatusTypeDef Load_Wifi_SSID_Status_FRAM(void);
HAL_StatusTypeDef Save_Wifi_SSID_FRAM(const char *ssid);
HAL_StatusTypeDef Load_Wifi_SSID_FRAM(void);

// 와이파이 비밀번호 함수
HAL_StatusTypeDef Load_Wifi_Password_Status_FRAM(void);
HAL_StatusTypeDef Save_Wifi_Password_FRAM(const char *password);
HAL_StatusTypeDef Load_Wifi_Password_FRAM(void);

// 토큰 함수
HAL_StatusTypeDef Load_Token_Status_FRAM(void);
HAL_StatusTypeDef Save_Token_Status_FRAM(void);
HAL_StatusTypeDef Save_Token_FRAM(const char *token);
HAL_StatusTypeDef Load_Token_FRAM(void);

// MAC 주소 함수
HAL_StatusTypeDef Load_MAC_Status_FRAM(void);
HAL_StatusTypeDef Save_MAC_Status_FRAM(void);
HAL_StatusTypeDef Save_MAC_FRAM(const char *mac);
HAL_StatusTypeDef Load_MAC_FRAM(void);

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void FRAM_Init(void);

#endif /* INC_BACKUP_H_ */