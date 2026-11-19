# 태스크·큐·뮤텍스와 통신

## 태스크 구성

FreeRTOS 설정은 선점형 스케줄링, 1000Hz 틱입니다. 7개 태스크는 모두 Normal 우선순위입니다.

| 태스크 | 역할 | 루프 끝 대기 |
| --- | --- | --- |
| MotorTask | 모드 선택과 주행 제어 | osDelay(10) |
| ButtonTask | 모드 버튼과 LCD 메시지 | osDelay(10) |
| RCTask | 수신 완료 문자열 해석 | osDelay(1) |
| DistanceTask | 초음파 순차 측정과 최신 거리 발행 | osDelay(10) |
| SoundTask | 사운드 명령 소비와 음표 갱신 | osDelay(10) |
| defaultTask | LCD 초기화 후 대기 | osDelay(1) |
| CommTask | 현재 시작 함수는 대기 루프 | osDelay(1) |

`osDelay(10)`은 작업 이후 대기이므로 작업 시간까지 포함한 정확한 10ms 실행 주기를 보장하지 않습니다.

## 데이터 전달

| 객체 | 용량 | 전달 방식 |
| --- | --- | --- |
| btnQue | 16 | RC / AUTO / SEMI_AUTO 모드 값 |
| RCQue | 16 | 정적 `RC_data` 구조체의 주소 |
| disQue | 1 | `distance_data_t` 구조체 값 복사·덮어쓰기 |
| soundQue | 16 | 사운드 명령 값 |
| commQueue | 16 | 소비 코드는 있으나 CommTask가 생산 함수를 호출하지 않음 |

거리 구조체에는 좌·전방·우측 거리, 유효성 비트, 전체 유효성, 구조체 갱신 시각, 전방 트리거 시각이 포함됩니다. 600ms 검사는 구조체 갱신 시각 기준이며 센서별 독립 타임스탬프 검사는 아닙니다.

`lcdMutex`는 ButtonTask와 MotorTask 등의 LCD 접근 구간을 보호합니다. `dispMutex`·`ledMutex`도 생성되지만, 제공된 활성 코드에서 이 뮤텍스들을 사용하는 처리 구간은 확인되지 않습니다. 큐는 데이터 전달, 뮤텍스는 공유 장치 접근 조정이라는 역할 차이가 있습니다.

## Bluetooth 명령

HC-05 연결 USART6는 9600bps, 8N1입니다. 문자열 끝에 `*`를 붙입니다.

| 문자열 | 해석 |
| --- | --- |
| F / B / L / R | 전진 / 후진 / 좌회전 / 우회전 |
| f / b / l / r | 방향 버튼 해제 |
| STOP | 정지 요청; SEMI_AUTO에서는 일시정지 유지 |
| RUN | SEMI_AUTO의 일시정지 해제·자동 제어 재개 |
| SPD숫자 | 속도 요청 파싱; 실제 적용은 모드와 상태에 따라 다름 |

RC 모드의 속도 요청 처리는 후속 상태 코드에서 덮어써질 수 있으므로 지속적인 속도 유지 성과로 표현하지 않습니다. AUTO는 속도 설정값 400을 사용하며 SEMI_AUTO 수동 진입도 400을 설정합니다. 이 값은 실제 속도(cm/s) 측정값이 아닙니다.

소스: [freertos.c](../firmware/Core/Src/freertos.c), [RC_task.c](../firmware/Core/Src/ap/task/RC_task/RC_task.c), [message.h](../firmware/Core/Src/ap/message.h).
