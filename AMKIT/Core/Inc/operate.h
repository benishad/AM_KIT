/*
 * operate.h
 *
 *  Created on: Jun 25, 2025
 *      Author: DONGYOONLEE
 */

#ifndef INC_OPERATE_H_
#define INC_OPERATE_H_

#include <stdint.h>


extern uint8_t g_nBoot_Status;                          // 부팅 상태 (0: 부팅 성공, 1: 부팅 중, 2: 부팅 실패 등)
extern int g_nBoot_Tick;                                 // 부팅 타이머 (ms 단위)
extern int g_nBoot_Step;                                  // 부팅 단계

extern uint8_t g_nMac_Status;                           // MAC 주소 상태 (0: MAC 주소 없음, 1: MAC 주소 있음)

void Oper_CCM_Init(void);

void Oper_Init(void);

  // 0. SD 카드 초기화
  // 1. AT 명령 테스트
  // 2. AT 펌웨어 조회
  // 3. wifi 연결
  // 4. UTC시간 서버 연결
  // 5. RTC 시간 동기화
  // 6. 토큰 요청 및 반환
  // 7. 기기 MAC 주소 조회 (기기 고유값)
void Oper_Boot(void);

// 부팅 상태
enum BootStatus 
{

    BOOT_SUCCESS = 0,              // 부팅 성공
    BOOT_IN_PROGRESS,         // 부팅 중
    BOOT_ERROR,               // 부팅 실패
};


#endif /* INC_OPERATE_H_ */
