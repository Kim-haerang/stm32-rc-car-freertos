# STM32 · FreeRTOS RC카

STM32F411RE 기반 RC카에서 블루투스 수동 조작, 초음파 장애물 회피, 수동 개입이 가능한 반자동 주행을 구현한 팀 프로젝트입니다.

**본인 담당: AUTO·SEMI_AUTO 제어**

**C · STM32 HAL · FreeRTOS · Queue · Mutex · UART · Timer/PWM**

## 시연

[AUTO 장애물 회피](https://drive.google.com/file/d/1t_CEiHCWLjFkZ1zzyVovJlfsNegVc83t/view) · [SEMI_AUTO](https://drive.google.com/file/d/1KM_rYslX-1d9qEa9f7ZQAqc1H_H4JfmB/view) · [블루투스 수동 조작](https://drive.google.com/file/d/1Gq8mVAsGUAhyRUPcj9X02W0VArgi7ETX/view)

## 담당 역할과 모드

본인은 AUTO 장애물 회피와 SEMI_AUTO 수동 개입 제어를 담당했습니다. 블루투스 입력·센서·표시·사운드 연동은 팀 전체 시스템 구성으로 설명합니다.

| 모드 | 동작 |
| --- | --- |
| RC | 블루투스 명령으로 전진·후진·좌우 회전·정지 |
| AUTO | 거리 조건으로 직진·후진·회전·복구·정지 판단 |
| SEMI_AUTO | 자동주행 중 방향 입력으로 수동 개입, 해제 시 자동 제어 복귀 |

## 본인 구현

- **장애물 회피 FSM:** 좌·전방·우측 거리를 이용해 회피 방향을 결정하고, 회전 실패 시 후진·방향 재선택을 수행합니다.
- **직진 복귀 조건:** 최소 회전 시간 이후 서로 다른 전방 측정에서 개방 조건을 2회 연속 확인합니다.
- **거리 유효성 처리:** 측정 실패 또는 오래된 데이터로 주행하지 않도록 정지하고, AUTO는 새 전방 측정 후 판단을 재개합니다.
- **반자동 제어:** 방향 입력·해제·STOP·RUN을 처리하고, 수동 개입에도 전방·측면 거리 제한을 적용합니다. 후방 센서는 없습니다.

## 핵심 코드 바로가기

| 파일 | 확인할 내용 |
| --- | --- |
| [motor_control_task.c](firmware/Core/Src/ap/task/motor_control_task/motor_control_task.c) | 본인 담당 `motor_AUTO_control_task()`, `motor_SEMI_control_task()` |
| [distance_task.c](firmware/Core/Src/ap/task/distance_task/distance_task.c) | 센서 순차 측정, 유효성 정보, 최신 거리 큐 갱신 |
| [freertos.c](firmware/Core/Src/freertos.c) | 태스크·큐·뮤텍스 구성 |
| [message.h](firmware/Core/Src/ap/message.h) | 거리 구조체와 명령 정의 |

거리값은 깊이 1 큐에 덮어쓰고 모터 태스크가 최신 값을 읽습니다. 명령 큐와 별도로 LCD 접근에는 뮤텍스를 사용합니다. [아키텍처·상태 머신](docs/architecture.md) · [RTOS·통신 상세](docs/rtos.md)

## 트러블슈팅

- 구석에서 회전이 반복되는 상황에 후진 복구와 방향 재선택, 복구 횟수 제한을 적용했습니다.
- 한 번 열린 전방값으로 조기 직진하지 않도록 회전 시간과 연속 측정 조건을 사용했습니다.
- 정상 주행 중 거리값이 간헐적으로 사라지는 현상에서는 연장 점퍼선 접촉 문제를 의심했습니다. 원인은 확정하지 못했으며, 측정 실패 시 주행을 제한하는 처리는 별도로 구현되어 있습니다.

[설계 판단과 문제 분석 자세히 보기](docs/design-notes.md)

## 프로젝트 구성과 실행

- `firmware/`: CubeIDE 프로젝트와 HAL·FreeRTOS 포함
- `docs/`: [핀맵](docs/pinmap.md), 아키텍처, RTOS·통신, 설계 기록

CubeIDE에서 `firmware/`의 `20261116_RC_AUTO`를 가져옵니다. 모드 버튼은 PC0/PC1/PC2이며 HC-05는 USART6 9600bps를 사용합니다. 블루투스 명령은 `F*`, `L*`, `STOP*`, `RUN*`처럼 `*`로 끝납니다.
