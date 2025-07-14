/*
 * device.c
 *
 *  Created on: Jun 24, 2025
 *      Author: DONGYOONLEE
 */

#include "device.h"
#include "main.h"


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// CCMRAM 초기화
__CCMRAM__ Device_Control g_deviceControl; // 기기 조작 구조체 인스턴스
PDevice_Control DEVICE_Get_Control(void)
{
    // g_deviceControl의 주소를 반환
    return &g_deviceControl;
}

void DEVICE_Init(void)
{
    // g_deviceControl 구조체를 0으로 초기화
    memset(&g_deviceControl, 0, sizeof(g_deviceControl));

}
// ──────────────────────────────────────────────────────────────────────────────

int Device_Mode_Set(int mode)
{
    PDevice_Control pDeviceControl = DEVICE_Get_Control();

    // 모드 설정
    switch (mode)
    {
    case MODE_MASTER:
        pDeviceControl->g_nMode = MODE_MASTER; // 마스터 모드로 설정
        break;
    case MODE_SLAVE:
        pDeviceControl->g_nMode = MODE_SLAVE; // 슬레이브 모드로 설정
        break;
    case MODE_AP:
        pDeviceControl->g_nMode = MODE_AP; // AP 모드로 설정
        break;
    case MODE_DEGUG:
        pDeviceControl->g_nMode = MODE_DEGUG; // 디버그 모드로 설정
        break;
    default:
        return -1; // 잘못된 모드 값 처리
    }

    return pDeviceControl->g_nMode; // 현재 모드 반환
}



int Device_Mode_Check(void)
{
    PDevice_Control pDeviceControl = DEVICE_Get_Control();

    int mode = pDeviceControl->g_nMode; // 현재 모드 가져오기

    if ((DIP_1_GPIO_Port->IDR & DIP_1_Pin) == DIP_1_Pin && 
        (DIP_2_GPIO_Port->IDR & DIP_2_Pin) == DIP_2_Pin && 
        (DIP_3_GPIO_Port->IDR & DIP_3_Pin) == DIP_3_Pin &&
        (DIP_4_GPIO_Port->IDR & DIP_4_Pin) == DIP_4_Pin)
    {
        mode = Device_Mode_Set(MODE_MASTER); // 마스터 모드로 설정 및 업데이트

        return mode;
    }

    // 딥스위치 상태 확인 esp32를 AP모드 사용할지 결정
    // 3,4번 DIP 스위치가 모두 HIGH 상태일 때 AP 모드로 설정
    if ((DIP_1_GPIO_Port->IDR & DIP_1_Pin) == DIP_1_Pin && 
        (DIP_2_GPIO_Port->IDR & DIP_2_Pin) == DIP_2_Pin && 
        (DIP_3_GPIO_Port->IDR & DIP_3_Pin) != DIP_3_Pin && 
        (DIP_4_GPIO_Port->IDR & DIP_4_Pin) != DIP_4_Pin)
    {
        mode = Device_Mode_Set(MODE_AP); // AP 모드로 설정 및 업데이트

        // pDeviceControl->g_nMode = MODE_AP; // AP 모드로 설정
        // mode = pDeviceControl->g_nMode; // 현재 모드 업데이트

        // g_nMode = MODE_AP; // AP 모드로 설정

        return mode;
    }
    else
    {
        g_nMode = MODE_MASTER; // 기본적으로 마스터 모드로 설정
    }

    // 딥스위치 1,3,4 번 HIGH 상태일때 디버그 모드
    if ((DIP_1_GPIO_Port->IDR & DIP_1_Pin) != DIP_1_Pin && 
        (DIP_2_GPIO_Port->IDR & DIP_2_Pin) == DIP_2_Pin && 
        (DIP_3_GPIO_Port->IDR & DIP_3_Pin) != DIP_3_Pin &&
        (DIP_4_GPIO_Port->IDR & DIP_4_Pin) != DIP_4_Pin)
    {
        mode = Device_Mode_Set(MODE_DEGUG); // 디버그 모드로 설정 및 업데이트

        // pDeviceControl->g_nMode = MODE_DEGUG; // 디버그 모드로 설정
        // mode = pDeviceControl->g_nMode; // 현재 모드 업데이트
        
        // g_nMode = MODE_DEGUG; // 디버그 모드로 설정

        return mode;
    }

    return mode;
}

// ──────────────────────────────────────────────────────────────────────────────


// 와이파이 연결 상태를 설정하는 함수
void DEVICE_Set_WiFi_Status(int status)
{
    PDevice_Control pDeviceControl = DEVICE_Get_Control();

    switch (status)
    {
    case DEVICE_WIFI_DISCONNECTED:
        pDeviceControl->sWifiConnected = DEVICE_WIFI_DISCONNECTED; // 와이파이 미연결 상태로 설정
        break;
    case DEVICE_WIFI_CONNECTED:
        pDeviceControl->sWifiConnected = DEVICE_WIFI_CONNECTED; // 와이파이 연결 상태로 설정
        break;
    case DEVICE_WIFI_CONNECTING:
        pDeviceControl->sWifiConnected = DEVICE_WIFI_CONNECTING; // 와이파이 연결 중 상태로 설정
        break;
    case DEVICE_WIFI_ERROR:
        pDeviceControl->sWifiConnected = DEVICE_WIFI_ERROR; // 와이파이 오류 상태로 설정
        break;
    default:
        // 잘못된 상태 값 처리 (예: 오류 로그 출력)
        Error_Handler();
        break;
    }

}