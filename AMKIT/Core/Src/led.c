/*
 * led.c
 *
 *  Created on: Jun 30, 2025
 *      Author: DONGYOONLEE
 */

#include "led.h"
#include "main.h"


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
*
*/

// LED 초기화 함수
void LED_Init(void)
{
    // RCC AHB1 버스에서 GPIOE 클럭 enable (bit4)
    RCC->AHB1ENR |= (1 << 4);
    // 잠깐 대기하여 클럭이 안정화되도록
    volatile uint32_t tmp = RCC->AHB1ENR;

    // ==============================================================

    // PE8 모드를 ‘01’(General purpose output)로 설정
    // MODER 레지스터: 2bit/핀
    RX_LED_GPIO_Port->MODER &= ~(0x3U << (8 * 2));  // 먼저 클리어
    RX_LED_GPIO_Port->MODER |=  (0x1U << (8 * 2));  // 0b01: Output
    // 출력타입: Push-Pull (OTYPER bit=0)
    RX_LED_GPIO_Port->OTYPER &= ~(1U << 8);
    // RX_LED_GPIO_Port->OTYPER |=  (1U << 8); // open drain은 사용하지 않음
    // 출력속도: High speed (OSPEEDR 0b10 이상 권장)
    RX_LED_GPIO_Port->OSPEEDR &= ~(0x3U << (8 * 2));
    RX_LED_GPIO_Port->OSPEEDR |=  (0x2U << (8 * 2));
    // 풀업/풀다운 없음 (PUPDR = 0b00)
    RX_LED_GPIO_Port->PUPDR &= ~(0x3U << (8 * 2));

    // PE9 모드를 ‘01’(General purpose output)로 설정
    // MODER 레지스터: 2bit/핀
    TX_LED_GPIO_Port->MODER &= ~(0x3U << (9 * 2));  // 먼저 클리어
    TX_LED_GPIO_Port->MODER |=  (0x1U << (9 * 2));  // 0b01: Output
    // 출력타입: Push-Pull (OTYPER bit=0)
    TX_LED_GPIO_Port->OTYPER &= ~(1U << 9);
    // 출력속도: High speed (OSPEEDR 0b10 이상 권장)
    TX_LED_GPIO_Port->OSPEEDR &= ~(0x3U << (9 * 2));
    TX_LED_GPIO_Port->OSPEEDR |=  (0x2U << (9 * 2));
    // 풀업/풀다운 없음 (PUPDR = 0b00)
    TX_LED_GPIO_Port->PUPDR &= ~(0x3U << (9 * 2));


    // PE10 모드를 ‘01’(General purpose output)로 설정
    // MODER 레지스터: 2bit/핀
    STATUS_LED_GPIO_Port->MODER &= ~(0x3U << (10 * 2));  // 먼저 클리어
    STATUS_LED_GPIO_Port->MODER |=  (0x1U << (10 * 2));  // 0b01: Output
    // 출력타입: Push-Pull (OTYPER bit=0)
    STATUS_LED_GPIO_Port->OTYPER &= ~(1U << 10);
    // 출력속도: High speed (OSPEEDR 0b10 이상 권장)
    STATUS_LED_GPIO_Port->OSPEEDR &= ~(0x3U << (10 * 2));
    STATUS_LED_GPIO_Port->OSPEEDR |=  (0x2U << (10 * 2));
    // 풀업/풀다운 없음 (PUPDR = 0b00)
    STATUS_LED_GPIO_Port->PUPDR &= ~(0x3U << (10 * 2));
    
}


// LED On: PE8을 1로 설정 (BSRR 사용)
void RX_LED_On(void)
{
    RX_LED_GPIO_Port->BSRR = (1U << 8);       // set PE8
}

// LED Off: PE8을 0으로 설정 (BSRR 상위 16비트에 1을 쓰면 reset)
void RX_LED_Off(void)
{
    RX_LED_GPIO_Port->BSRR = (1U << (8 + 16)); // reset PE8
}

// RX LED Toggle: PE8을 토글
void RX_LED_Toggle(void)
{
    RX_LED_GPIO_Port->ODR ^= (1U << 8); // toggle PE8
}

// LED On: PE9을 1로 설정 (BSRR 사용)
void TX_LED_On(void)
{
    TX_LED_GPIO_Port->BSRR = (1U << 9);       // set PE9
}

// LED Off: PE9을 0으로 설정 (BSRR 상위 16비트에 1을 쓰면 reset)
void TX_LED_Off(void)
{
    TX_LED_GPIO_Port->BSRR = (1U << (9 + 16)); // reset PE9
}


// LED On: PE10을 1로 설정 (BSRR 사용)
void STATUS_LED_On(void)
{
    // BSRR 레지스터 하위 16비트에 1을 쓰면 set
    STATUS_LED_GPIO_Port->BSRR = (1U << 10);
}

// LED Off: PE10을 0으로 설정 (BSRR 상위 16비트에 1을 쓰면 reset)
void STATUS_LED_Off(void)
{
    // BSRR 레지스터 상위 16비트(bit16+핀번호)에 1을 쓰면 reset
    STATUS_LED_GPIO_Port->BSRR = (1U << (10 + 16));
}