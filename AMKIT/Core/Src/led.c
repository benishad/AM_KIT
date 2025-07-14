/*
 * led.c
 *
 *  Created on: Jun 30, 2025
 *      Author: DONGYOONLEE
 */

#include "led.h"
#include "main.h"


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