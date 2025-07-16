/*
 * sim.c
 *
 *  Created on: Jul 14, 2025
 *      Author: DONGYOONLEE
 */


#include "sim.h"
#include "main.h"


#define RSIM_BUF_SIZE   1024    // 응답 버퍼 크기
#define BYTE_RX_TIMEOUT 300     // 한 바이트 최대 대기(ms)
#define OVERALL_TIMEOUT 1000    //20000   // 전체 응답 최대 대기(ms)
#define POST_OK_TIMEOUT 500     // OK 이후 유예 대기(ms)

// RDY 응답 대기
#define RDY_MARKER      "RDY"
#define RDY_MARKER_LEN  (sizeof(RDY_MARKER) - 1)

/*
* 레지스터 설정 방법
* 1. GPIO* 클럭 활성화
*    (RCC->AHB1ENR |= (1 << GPIOE_PIN_NUMBER);)
*    (volatile uint32_t tmp = RCC->AHB1ENR;) // 클럭 안정화 대기
*    (volatile 변수 사용으로 컴파일러 최적화 방지)
*
* 2. GPIO* 핀 모드 설정 (출력)
*    (GPIO*->MODER &= ~(0x3U << (PIN_NUMBER * 2));) // 먼저 클리어
*    (GPIO*->MODER |=  (0x1U << (PIN_NUMBER * 2));) // 0b01: Output)
*
* 3. GPIO* 핀 출력 타입 설정 (푸시풀/오픈드레인)
*    (GPIO*->OTYPER &= ~(1U << PIN_NUMBER);) // Push-Pull)
*    (GPIO*->OTYPER |=  (1U << PIN_NUMBER);) // Open Drain)
*    (푸시풀은 일반적으로 사용, 오픈드레인은 필요 시 사용)
*
* 4. GPIO* 핀 출력 속도 설정 (Low/Medium/High/Very High)
*    (GPIO*->OSPEEDR &= ~(0x3U << (PIN_NUMBER * 2));) // 먼저 클리어
*    (GPIO*->OSPEEDR |=  (0x0U << (PIN_NUMBER * 2));) // Low speed)
*    (GPIO*->OSPEEDR |=  (0x1U << (PIN_NUMBER * 2));) // Medium speed)
*    (GPIO*->OSPEEDR |=  (0x2U << (PIN_NUMBER * 2));) // High speed)
*    (GPIO*->OSPEEDR |=  (0x3U << (PIN_NUMBER * 2));) // Very high speed)
*    (High speed 이상 권장, Low speed는 일반적으로 사용하지 않음)
*    (속도는 사용 환경에 따라 조정)
*
* 5. GPIO* 핀 풀업/풀다운 설정 (없음/풀업/풀다운)
*    (GPIO*->PUPDR &= ~(0x3U << (PIN_NUMBER * 2));) // 먼저 클리어
*    (GPIO*->PUPDR |=  (0x0U << (PIN_NUMBER * 2));) // No pull-up/pull-down)
*    (GPIO*->PUPDR |=  (0x1U << (PIN_NUMBER * 2));) // Pull-up)
*    (GPIO*->PUPDR |=  (0x2U << (PIN_NUMBER * 2));) // Pull-down)
*    (풀업/풀다운은 필요에 따라 설정, 일반적으로 No pull-up/pull-down 사용)
*
* 6. GPIO* 핀 출력 (BSRR 레지스터 사용)
*    (GPIO*->BSRR = (1U << PIN_NUMBER);) // set)
*    (GPIO*->BSRR = (1U << (PIN_NUMBER + 16));) // reset)
*/



// AT 응답의 종료 마커
// static const char END_MARKER[] = "\r\nOK\r\n";
// static const size_t END_MARKER_LEN = sizeof(END_MARKER) - 1;
static const uint8_t END_MARKER[] = { 0x90, 0x00 };
static const size_t END_MARKER_LEN = sizeof(END_MARKER);
// AT 명령어 오류 응답 마커
static const char ERR_MARKER[] = "\r\nERROR\r\n";
static const size_t ERR_MARKER_LEN = sizeof(ERR_MARKER) - 1;


// ──────────────────────────────────────────────────────────────────────────────
// ──────────────────────────────────────────────────────────────────────────────

// M_PWR_KEY 초기화
void SIM_M_PWR_KEY_Init(void)
{
#if 0
    // RCC AHB1 버스에서 GPIOE 클럭 enable (bit4)
    RCC->AHB1ENR |= (1 << 4);
    // 잠깐 대기하여 클럭이 안정화되도록
    volatile uint32_t tmp = RCC->AHB1ENR;

    // ==============================================================

    // M_PWR_KEY_Pin GPIOE 핀을 출력으로 설정
    // GPIOE->MODER 레지스터의 해당 핀을 01로 설정
    M_PWR_KEY_GPIO_Port->MODER &= ~(0x3U << (11 * 2));  // 먼저 클리어
    M_PWR_KEY_GPIO_Port->MODER |=  (0x1U << (11 * 2));  // 출력 모드 설정

    // 출력 타입을 푸시풀로 설정
    M_PWR_KEY_GPIO_Port->OTYPER &= ~(1U << 11); // Push‑Pull
    // M_PWR_KEY_GPIO_Port->OTYPER |=  (1U << 11); // open drain은 사용하지 않음

    // 출력 속도 설정
    // 00: Low speed, 01: Medium speed, 10: High speed, 11: Very high speed
    M_PWR_KEY_GPIO_Port->OSPEEDR &= ~(0x3U << (11 * 2));  // 먼저 클리어
    // M_PWR_KEY_GPIO_Port->OSPEEDR |=  (0x0U << (11 * 2));  // Low speed
    // M_PWR_KEY_GPIO_Port->OSPEEDR |=  (0x1U << (11 * 2));  // Medium speed
    M_PWR_KEY_GPIO_Port->OSPEEDR |=  (0x2U << (11 * 2));  // High speed
    // M_PWR_KEY_GPIO_Port->OSPEEDR |=  (0x3U << (11 * 2));  // Very high speed

    // 출력 풀업/풀다운 설정
    // 00: No pull-up, pull-down, 01: Pull-up,
    M_PWR_KEY_GPIO_Port->PUPDR &= ~(0x3U << (11 * 2));  // 먼저 클리어
    // 설정 없으면 풀업/풀다운 없음

    // M_PWR_KEY_GPIO_Port->BSRR = (1U << (11 + 16));  // ODR[11] ← 0
    M_PWR_KEY_GPIO_Port->BSRR = (1U << 11); // set PE11
#else
    // HAL라이브러리 사용해서 GPIO 초기화
    HAL_GPIO_WritePin(M_PWR_KEY_GPIO_Port, M_PWR_KEY_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(M_PWR_KEY_GPIO_Port, M_PWR_KEY_Pin, GPIO_PIN_RESET);
    HAL_Delay(1100); // 1.1초 대기
    HAL_GPIO_WritePin(M_PWR_KEY_GPIO_Port, M_PWR_KEY_Pin, GPIO_PIN_SET);


#endif
}

void SIM_PWR_ON(void)
{
    // M_PWR_KEY 핀을 HIGH로 설정하여 SIM 모듈 전원 ON
    M_PWR_KEY_GPIO_Port->BSRR = (1U << 11); // set PE11
}

void SIM_PWR_OFF(void)
{
    // M_PWR_KEY 핀을 LOW로 설정하여 SIM 모듈 전원 OFF
    M_PWR_KEY_GPIO_Port->BSRR = (1U << (11 + 16)); // reset PE11
}

void SIM_Toggle_PWR(void)
{
    // M_PWR_KEY 핀을 토글하여 SIM 모듈 전원 ON/OFF
    M_PWR_KEY_GPIO_Port->ODR ^= (1U << 11); // toggle PE11
}


// ──────────────────────────────────────────────────────────────────────────────

// USIM_RESET 초기화
void SIM_USIM_RESET_Init(void)
{
    // RCC AHB1 버스에서 GPIOE 클럭 enable (bit4)
    RCC->AHB1ENR |= (1 << 4);
    // 잠깐 대기하여 클럭이 안정화되도록
    volatile uint32_t tmp = RCC->AHB1ENR;

    // ==============================================================

    // 사용할 포트와 핀은 아래와 같음
    // #define USIM_RESET_Pin GPIO_PIN_13
    // #define USIM_RESET_GPIO_Port GPIOE

    // USIM_RESET_Pin GPIOE 핀을 출력으로 설정
    // GPIOE->MODER 레지스터의 해당 핀을 01로 설정
    USIM_RESET_GPIO_Port->MODER &= ~(0x3U << (13 * 2));  // 먼저 클리어
    USIM_RESET_GPIO_Port->MODER |=  (0x1U << (13 * 2));  // 출력 모드 설정

    // 출력 타입을 푸시풀로 설정
    USIM_RESET_GPIO_Port->OTYPER &= ~(1U << 13); // Push‑Pull
    // USIM_RESET_GPIO_Port->OTYPER |=  (1U << 13); // open drain은 사용하지 않음

    // 출력 속도 설정
    // 00: Low speed, 01: Medium speed, 10: High speed, 11: Very high speed
    USIM_RESET_GPIO_Port->OSPEEDR &= ~(0x3U << (13 * 2));  // 먼저 클리어
    // USIM_RESET_GPIO_Port->OSPEEDR |=  (0x0U << (13 * 2));  // Low speed
    // USIM_RESET_GPIO_Port->OSPEEDR |=  (0x1U << (13 * 2));  // Medium speed
    USIM_RESET_GPIO_Port->OSPEEDR |=  (0x2U << (13 * 2));  // High speed
    // USIM_RESET_GPIO_Port->OSPEEDR |=  (0x3U << (13 * 2));  // Very high speed

    // 출력 풀업/풀다운 설정
    // 00: No pull-up, pull-down, 01: Pull-up,
    USIM_RESET_GPIO_Port->PUPDR &= ~(0x3U << (13 * 2));  // 먼저 클리어
    // 설정 없으면 풀업/풀다운 없음 , OUTPUT이기 때문에 풀업/풀다운은 필요 없음
    // 풀업 설정
    // USIM_RESET_GPIO_Port->PUPDR |=  (0x1U << (13 * 2));  // Pull-up
    // 풀다운 설정
    // USIM_RESET_GPIO_Port->PUPDR |=  (0x2U << (13 * 2));  // Pull-down

    // USIM_RESET 핀 초기 상태 설정
    // USIM_RESET_GPIO_Port->BSRR = (1U << 13);
    USIM_RESET_GPIO_Port->BSRR = (1U << (13 + 16)); // 리셋 PE13 → MCU ODR=0
}

void SIM_USIM_RESET_Set(void)
{
    // USIM_RESET 핀을 HIGH로 설정하여 USIM 리셋
    USIM_RESET_GPIO_Port->BSRR = (1U << 13); // set PE13
}

void SIM_USIM_RESET_Clear(void)
{
    // USIM_RESET 핀을 LOW로 설정하여 USIM 리셋 해제
    USIM_RESET_GPIO_Port->BSRR = (1U << (13 + 16)); // reset PE13
}


// ──────────────────────────────────────────────────────────────────────────────


/**
 * @brief  USART3 로 들어오는 데이터를 읽어 옵니다.
 * @param  buf             읽어온 데이터를 저장할 버퍼
 * @param  buf_len         buf 의 전체 크기 (널종료 공간 포함)
 * @param  overall_timeout_ms  전체 읽기 최대 대기 시간 (ms)
 * @param  byte_timeout_ms     바이트당 최대 대기 시간 (ms)
 * @retval 실제 읽어온 바이트 수 (널종료 문자는 제외)
 */
size_t SIM_UART_ReadData(char *buf, size_t buf_len, uint32_t overall_timeout_ms, uint32_t byte_timeout_ms)
{
    uint32_t start = HAL_GetTick();
    size_t   pos   = 0;
    uint8_t  ch;

    // 읽기 루프
    while ((HAL_GetTick() - start) < overall_timeout_ms && pos < buf_len - 1)
    {
        // 1바이트 받기 (타임아웃 byte_timeout_ms)
        if (HAL_UART_Receive(&huart3, &ch, 1, byte_timeout_ms) == HAL_OK)
        {
            buf[pos++] = (char)ch;
            start = HAL_GetTick(); // 새 데이터 수신 시 전체 타임아웃 리셋
        }
        // else: byte_timeout_ms 경과 시 재시도, overall_timeout_ms 만료 전까지
    }

    buf[pos] = '\0'; // 널종료
    return pos;
}

// ──────────────────────────────────────────────────────────────────────────────

// 동기방식 SIM AT 명령 전송 함수
// 반환값으로 응답 문자열을 반환
const char* SIM_AT_Send_Command_Sync_Get_Result(const char* cmd)
{
    static char    respBuf[RSIM_BUF_SIZE] = {0};       // 응답 버퍼
    size_t  pos   = 0;                          // 현재 응답 버퍼 위치
    uint8_t ch;                                 // 수신된 바이트
    uint32_t tick_0   = HAL_GetTick();          // 전체 타이머 시작

    // (1) AT 명령 전송
    size_t cmdLen = strlen(cmd);
    if (HAL_UART_Transmit(&huart3, (uint8_t*)cmd, cmdLen, HAL_MAX_DELAY) != HAL_OK)
    {
        Error_Handler();
    }

    // (2) END_MARKER ("\r\nOK\r\n") 까지 수신
    while (pos < RSIM_BUF_SIZE - 1)
    {
        if (HAL_UART_Receive(&huart3, &ch, 1, BYTE_RX_TIMEOUT) == HAL_OK)
        {
            respBuf[pos++] = (char)ch;          // 수신된 바이트를 버퍼에 저장

            // 슬라이딩 윈도우로 END_MARKER 일치 검사
            // pos >= END_MARKER_LEN 조건은 END_MARKER 길이만큼의 데이터가 모였는지 확인
            if (pos >= END_MARKER_LEN && memcmp(&respBuf[pos - END_MARKER_LEN], END_MARKER, END_MARKER_LEN) == 0)
            {
                break;
            }
            // 에러마커 확인
            else if (pos >= ERR_MARKER_LEN && memcmp(&respBuf[pos - ERR_MARKER_LEN], ERR_MARKER, ERR_MARKER_LEN) == 0)
            {
                // 에러 발생 시 강제 종료
                // pos = 0;  // 버퍼 초기화
                break;
            }

            tick_0 = HAL_GetTick();  // 데이터 수신 시 전체 타이머 리셋
        }
        else if (HAL_GetTick() - tick_0 > OVERALL_TIMEOUT)
        {
            // 전체 대기 초과 시 강제 종료
            break;
        }
    }

    // (3) OK 이후 추가 URC 등 있을 수 있으니 잠깐 더 대기하며 수신
    uint32_t tick_1 = HAL_GetTick();
    while (pos < RSIM_BUF_SIZE - 1 && HAL_GetTick() - tick_1 < POST_OK_TIMEOUT)
    {
        if (HAL_UART_Receive(&huart3, &ch, 1, BYTE_RX_TIMEOUT) == HAL_OK)
        {
            respBuf[pos++] = ch;
            tick_1 = HAL_GetTick();  // 추가 데이터 수신 시 유예 시간 리셋
        }
    }
    respBuf[pos] = '\0';

    // (4) 한 번에 PC(UART1)로 전송
    if (pos > 0)
    {
        HAL_UART_Transmit(&huart1, (uint8_t*)respBuf, pos, HAL_MAX_DELAY);
    }

    return respBuf; // 응답 버퍼를 반환
}

// ──────────────────────────────────────────────────────────────────────────────

// RDY 응답 대기
// 전원 ON 직후 RDY URC 대기 함수
// timeout_ms 내에 "RDY"가 수신되면 true 반환, 아니면 false
int SIM_AT_WaitReady(uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    uint8_t  ch;
    char     win[RDY_MARKER_LEN] = {0};
    size_t   idx = 0;

    while ((HAL_GetTick() - start) < timeout_ms)
    {
        // 한 바이트씩 읽되, 최대 BYTE_RX_TIMEOUT 대기
        if (HAL_UART_Receive(&huart3, &ch, 1, BYTE_RX_TIMEOUT) == HAL_OK)
        {
            // 슬라이딩 윈도우 갱신
            win[idx % RDY_MARKER_LEN] = (char)ch;
            idx++;

            // 버퍼가 쌓였으면 매번 비교
            if (idx >= RDY_MARKER_LEN)
            {
                // 윈도우를 순환하여 "RDY" 매칭 검사
                int match = 1;
                for (size_t i = 0; i < RDY_MARKER_LEN; i++)
                {
                    if (win[(idx + i) % RDY_MARKER_LEN] != RDY_MARKER[i])
                    {
                        match = 0;
                        break;
                    }
                }
                if (match)
                {
                    return 1;  // RDY 발견
                }
            }
        }
        // else 타임아웃이더라도 전체 타임아웃까지 루프를 계속 돈다
    }
    return 0;  // 타임아웃
}