/*
 * device.h
 *
 *  Created on: Jun 24, 2025
 *      Author: DONGYOONLEE
 */

#ifndef INC_DEVICE_H_
#define INC_DEVICE_H_


// 기기 조작 구조체
typedef struct tag_Device_Control
{
    int sWifiConnected;  // 와이파이 연결 여부 (0: 미연결, 1: 연결됨)

    int sEsp32TimeSynced; // ESP32 시간 동기화 여부 (0: 미동기화, 1: 동기화됨)

    int g_nMode;    // 현재 모드


} Device_Control, *PDevice_Control;

PDevice_Control DEVICE_Get_Control(void);




enum Device_Wifi_Status
{
    DEVICE_WIFI_DISCONNECTED = 0, // 와이파이 미연결
    DEVICE_WIFI_CONNECTED,         // 와이파이 연결됨
    DEVICE_WIFI_CONNECTING,        // 와이파이 연결 중
    DEVICE_WIFI_ERROR              // 와이파이 오류
};

enum Device_Time_Status
{
    DEVICE_TIME_NOT_SYNCED = 0, // 시간 미동기화
    DEVICE_TIME_SYNCED           // 시간 동기화됨
};

enum Device_Token_Status
{
    DEVICE_TOKEN_NOT_SET = 0, // 토큰 미설정
    DEVICE_TOKEN_SET          // 토큰 설정됨
};

enum Device_Mac_Status
{
    DEVICE_MAC_NOT_SET = 0, // MAC 주소 미설정
    DEVICE_MAC_SET          // MAC 주소 설정됨
};

// 기기 상태 모드
enum DeviceMode 
{
  MODE_MASTER = 0,          // 마스터 모드
  MODE_SLAVE,               // 슬레이브 모드
  MODE_AP,                  // AP 모드
  MODE_ERROR,               // 에러 모드
  MODE_MAINTENANCE,          // 유지보수 모드
  MODE_UNKNOWN,              // 알 수 없는 모드
  MODE_DEGUG
  
};

void DEVICE_Init(void);

int Device_Mode_Set(int mode);
int Device_Mode_Check(void);

void DEVICE_Set_WiFi_Status(int status);

#endif /* INC_DEVICE_H_ */
