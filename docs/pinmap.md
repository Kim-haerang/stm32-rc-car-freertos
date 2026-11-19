# STM32F411RE RC카 핀맵

- **MCU:** STM32F411RE
- **보드 제품명:** 발표자료는 STM32F411RE 기반 보드로 표기하며 `.ioc`는 `board=custom`입니다. 제품명 확인 후 추가합니다.

아래는 MCU GPIO 이름입니다. 보드 커넥터의 물리적 번호나 Arduino D/A 번호와 구분합니다.

| 구성 | 핀 | 용도 |
| --- | --- | --- |
| 좌측 HC-SR04 | PB13 / PB5 | TRIG / ECHO |
| 전방 HC-SR04 | PB15 / PB10 | TRIG / ECHO |
| 우측 HC-SR04 | PB14 / PB4 | TRIG / ECHO |
| RC / AUTO / SEMI_AUTO 버튼 | PC0 / PC1 / PC2 | 모드 선택 |
| 좌측 모터 | PC6, PB6 / PA7 | TIM3 CH1 PWM, 방향 제어 |
| 우측 모터 | PC8, PA5 / PA6 | TIM3 CH3 PWM, 방향 제어 |
| 좌우 LED | PA0 / PA1 | 주행 상태 표시 |
| 부저 | PB7 | TIM4 CH2 PWM |
| LCD | PB8 / PB9 | I2C1 SCL / SDA |
| HC-05 | PA11 / PA12 | USART6 TX / RX |
| USART2 | PA2 / PA3 | 115200bps 설정 |

TIM2는 Echo 상승·하강 시각을 읽는 자유 실행 카운터입니다. TIM10은 HAL 시간 기준이며 RTOS 태스크용 틱과 구분합니다.
