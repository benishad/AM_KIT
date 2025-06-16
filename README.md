# AM_KIT

**프로젝트 CUBEIDE .ioc 가이드**

이 문서는 STM32CubeIDE를 사용하여 STM32F407VGT 마이크로컨트롤러 기반 프로젝트를 설정하기 위한 가이드입니다.

---

## 1. 개발 환경

* **IDE**: STM32CubeIDE
* **MCU**: STM32F407VGT

## 2. 프로젝트 생성

1. STM32CubeIDE를 실행합니다.
2. `File` → `New` → `STM32 Project`를 선택합니다.
3. `Board Selector` 탭에서 `STM32F407VGT`를 검색하고 선택한 후 `Next`를 클릭합니다.
4. 프로젝트 이름 및 위치를 지정하고 `Finish`를 클릭합니다.

## 3. Pinout & Configuration 설정

1. 상단 탭에서 `Pinout & Configuration`을 선택합니다.
2. 왼쪽 메뉴에서 **SystemCore** → **SYS** 항목을 클릭합니다.

   * **Mode debug**: `Trace Asynchronous Sw` 선택
   * **Timebase Source**: `SysTick` 선택

## 4. 클록(RCC) 설정

1. 왼쪽 메뉴에서 **RCC**를 선택합니다.
2. **Mode HighSpeedClock** 옵션에서 `Crystal / Ceramic Resonator` 선택

## 5. 타이머 설정 (TIM7)

1. 왼쪽 메뉴에서 **Timers** → **TIM7**를 선택합니다.
2. **Mode**: `Activate` 체크
3. **Configuration** 탭으로 이동하여 다음 파라미터를 설정합니다:

   * **Counter Setting**

     * Prescaler (PSC): `100-1`
     * Counter Period: `840-1`
4. **NVIC Settings** 탭에서 `TIM7 global interrupt`를 `Enabled` 체크

## 6. RTC 설정

1. 왼쪽 메뉴에서 **RTC**를 선택합니다.
2. **Mode** 탭에서 다음을 체크합니다:

   * `Activate Clock Source`
   * `Activate Calendar`
3. **Alarm A/B / Wakeup**에서 필요에 맞는 모드를 선택합니다.

   * (본 프로젝트: Alarm A → `Internal Alarm` 선택)
4. **NVIC Settings** 탭에서 RTC 관련 인터럽트를 `Enabled` 체크

## 7. 저속 클록 설정 (RCC)

1. 왼쪽 메뉴에서 **RCC** → **Low Speed Clock**을 선택합니다.
2. `Crystal / Ceramic Resonator` 선택

## 8. Connectivity 설정 (SDIO)

1. 왼쪽 메뉴에서 **Connectivity** → **SDIO**를 선택합니다.
2. **Mode**: `SD 1-bit` 선택

   * 주의: `SD 4-bit` 선택 시 오류 핸들러 진입 가능, 오류 해결시 SD 4-bit 선택

## 9. Middleware 및 Software Packs 설정

1. 왼쪽 메뉴에서 **Middleware** → **FATFS**를 선택합니다.
2. **Mode**: `SDCARD` 선택

## 10. USART 설정

1. 왼쪽 메뉴에서 **Connectivity** → **USART1**를 선택합니다.

   * **Mode**: `Asynchronous` 선택
   * **NVIC Settings**: `USART1 global interrupt`를 `Enabled` 체크
2. **USART2** 및 **USART3**에 대해 동일하게 설정합니다.

   * **Mode**: `Asynchronous`
   * **NVIC Settings**: 각각 `USART2 global interrupt`, `USART3 global interrupt`를 `Enabled` 체크

3. 각 설명

    * **USART1** : 디버깅 확인 or 통신
    * **USART2** : ESP32 통신용
    * **USART3** : USIM LTE 통신용

## 11. SPI 설정 (SPI1)

1. 왼쪽 메뉴에서 **Connectivity** → **SPI1**를 선택합니다.
2. **Mode**: `Full-Duplex Master` 선택
3. 설명

    * FM25V10 칩과 연결되어 있어 메모리에 읽고 쓰기가 가능 합니다.

## 12. Clock Configuration

1. 상단 탭에서 **Clock Configuration**을 선택합니다.
2. **HSE**에 외부 크리스탈(8 MHz) 사용을 위해 `8` 입력
3. **PLL Source Mux**: `HSE` 선택
4. PLL 매개변수:

   * PLLM: `4`
   * PLLN: `168`
   * PLLP: `2`
   * PLLQ: `7`
5. **PLLCLK**를 활성화
6. **APB1 Prescaler**: `4`
7. **APB2 Prescaler**: `2`
8. **LSE** 활성화:

   * 외부 32.768 kHz 크리스탈이 부착되어 있으므로 `LSE` 선택
   * 오류 발생 시 `Resolve Issue` 클릭하여 자동으로 32.768 kHz 선택

---

## 13. Input Output 설정

1. Input:
    
    * INPUT1: PE0 - DIP1
    * INPUT2: PE1 - DIP2
    * INPUT3: PE2 - DIP3
    * INPUT4: PE3 - DIP4
    * INPUT1: PD8
    * INPUT2: PD9
    * INPUT3: PD10
    * INPUT4: PD11
    * INPUT5: PD12
    * INPUT6: PD13
    * INPUT7: PD14
    * INPUT8: PD15

2. Output:

    * OUTPUT1: PE8 - RX_LED
    * OUTPUT2: PE9 - TX_LED
    * OUTPUT3: PE10 - STATUS_LED
    * OUTPUT4: PE11 - M_PWR_KEY
    * OUTPUT5: PE12 - ESP_EN
    * OUTPUT6: PE13 - USIM_RESRT
    * OUTPUT7: PE14
    * OUTPUT8: PE15

## 14. 코드 생성 및 빌드

1. Ctrl+S 혹은 상단의 `Project` 메뉴에서 `Generate Code`를 클릭합니다.
2. 프로젝트를 빌드하려면 `Project` → `Build Project` 또는 툴바의 해머 아이콘을 클릭합니다.

## 15. 디버깅

1. 툴바의 벌레(bug) 아이콘을 클릭하여 디버그 모드를 시작합니다.
2. 디버거가 연결되면 설정한 Trace, SysTick 기반 타이머, RTC 인터럽트, SDIO, USART, SPI 동작 등을 확인합니다.

---

