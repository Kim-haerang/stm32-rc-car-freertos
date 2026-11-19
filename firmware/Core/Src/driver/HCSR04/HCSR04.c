#include "HCSR04.h"
#include "../../common/delay/delay.h"

#define ECHO_TIMEOUT_MS 50U
#define MAX_ECHO_US 30000U

/* DistanceTask starts only one sensor at a time. */
static HCSR04_t *volatile active_sensor;
static TIM_HandleTypeDef *started_timer;

void HCSR04_init(HCSR04_t *sensor,
                 GPIO_TypeDef *trig_port, uint16_t trig_pin,
                 GPIO_TypeDef *echo_port, uint16_t echo_pin,
                 TIM_HandleTypeDef *timer)
{
    sensor->trig_port = trig_port;
    sensor->trig_pin = trig_pin;
    sensor->echo_port = echo_port;
    sensor->echo_pin = echo_pin;
    sensor->timer = timer;
    sensor->state = STATE_IDLE;
    sensor->rise_count = 0;
    sensor->fall_count = 0;
    sensor->started_ms = 0;

    HAL_GPIO_WritePin(trig_port, trig_pin, GPIO_PIN_RESET);
    if (started_timer == NULL) {
        delay_init();
        HAL_TIM_Base_Start(timer);
        started_timer = timer;
    }
}

void HCSR04_reset(HCSR04_t *sensor)
{
    if (active_sensor == sensor) {
        active_sensor = NULL;
    }
    sensor->state = STATE_IDLE;
}

hcsr04_status_t HCSR04_start(HCSR04_t *sensor)
{
    if (active_sensor != NULL || sensor->state != STATE_IDLE) {
        return HCSR04_ERR_BUSY;
    }
    if (HAL_GPIO_ReadPin(sensor->echo_port, sensor->echo_pin) == GPIO_PIN_SET) {
        return HCSR04_ERR_BUSY;
    }

    sensor->rise_count = 0;
    sensor->fall_count = 0;
    sensor->started_ms = HAL_GetTick();
    sensor->state = STATE_WAIT_RISE;
    active_sensor = sensor;

    HAL_GPIO_WritePin(sensor->trig_port, sensor->trig_pin, GPIO_PIN_SET);
    delay_us(10);
    HAL_GPIO_WritePin(sensor->trig_port, sensor->trig_pin, GPIO_PIN_RESET);
    return HCSR04_IN_PROGRESS;
}

/* Echo pins must be configured for both rising and falling EXTI edges. */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    HCSR04_t *sensor = active_sensor;
    if (sensor == NULL || GPIO_Pin != sensor->echo_pin) {
        return;
    }

    GPIO_PinState level = HAL_GPIO_ReadPin(sensor->echo_port, sensor->echo_pin);
    uint32_t count = __HAL_TIM_GET_COUNTER(sensor->timer);

    if (sensor->state == STATE_WAIT_RISE && level == GPIO_PIN_SET) {
        sensor->rise_count = count;
        sensor->state = STATE_WAIT_FALL;
    } else if (sensor->state == STATE_WAIT_FALL && level == GPIO_PIN_RESET) {
        sensor->fall_count = count;
        sensor->state = STATE_DONE;
    }
}

hcsr04_status_t HCSR04_getResult(HCSR04_t *sensor, uint32_t *echo_us)
{
    if (echo_us == NULL) {
        return HCSR04_ERR_NOT_STARTED;
    }
    if (sensor->state == STATE_DONE) {
        uint32_t duration = sensor->fall_count - sensor->rise_count;
        HCSR04_reset(sensor);
        if (duration > MAX_ECHO_US) {
            return HCSR04_ERR_OUT_OF_RANGE;
        }
        *echo_us = duration;
        return HCSR04_OK;
    }
    if (sensor->state == STATE_IDLE) {
        return HCSR04_ERR_NOT_STARTED;
    }
    if ((uint32_t)(HAL_GetTick() - sensor->started_ms) >= ECHO_TIMEOUT_MS) {
        HCSR04_reset(sensor);
        return HCSR04_ERR_NO_ECHO;
    }
    return HCSR04_IN_PROGRESS;
}
