#ifndef SRC_DRIVER_HCSR04_HCSR04_H_
#define SRC_DRIVER_HCSR04_HCSR04_H_

#include "main.h"

typedef enum {
    STATE_IDLE,
    STATE_WAIT_RISE,
    STATE_WAIT_FALL,
    STATE_DONE
} sr04_state_t;

typedef enum {
    HCSR04_OK = 0,
    HCSR04_IN_PROGRESS,
    HCSR04_ERR_NOT_STARTED,
    HCSR04_ERR_BUSY,
    HCSR04_ERR_NO_ECHO,
    HCSR04_ERR_OUT_OF_RANGE
} hcsr04_status_t;

typedef struct {
    GPIO_TypeDef *trig_port;
    uint16_t trig_pin;
    GPIO_TypeDef *echo_port;
    uint16_t echo_pin;
    TIM_HandleTypeDef *timer;
    volatile sr04_state_t state;
    volatile uint32_t rise_count;
    volatile uint32_t fall_count;
    uint32_t started_ms;
} HCSR04_t;

void HCSR04_init(HCSR04_t *sensor,
                 GPIO_TypeDef *trig_port, uint16_t trig_pin,
                 GPIO_TypeDef *echo_port, uint16_t echo_pin,
                 TIM_HandleTypeDef *timer);
hcsr04_status_t HCSR04_start(HCSR04_t *sensor);
hcsr04_status_t HCSR04_getResult(HCSR04_t *sensor, uint32_t *echo_us);
void HCSR04_reset(HCSR04_t *sensor);

#endif /* SRC_DRIVER_HCSR04_HCSR04_H_ */
