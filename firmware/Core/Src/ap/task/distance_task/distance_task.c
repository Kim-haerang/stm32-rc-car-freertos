#include "distance_task.h"
#include "../../message.h"
#include "../../../driver/HCSR04/HCSR04.h"
#include "tim.h"

enum {
    SENSOR_LEFT,
    SENSOR_FRONT,
    SENSOR_RIGHT,
    SENSOR_COUNT
};

#define PING_INTERVAL_MS  60

typedef enum {
    START_MEASUREMENT,
    WAIT_RESULT,
    WAIT_BETWEEN_PINGS
} distance_phase_t;

static HCSR04_t sensors[SENSOR_COUNT];
static uint16_t pending_cm[SENSOR_COUNT];
static uint8_t pending_valid_mask;
static distance_phase_t phase = START_MEASUREMENT;
static uint8_t sensor = SENSOR_LEFT;
static uint32_t front_trigger_ms;
static uint32_t last_ping_ms;
static bool first_ping = true;

static void publish_invalid(uint32_t now)
{
    distance_data_t distance = {
        .measured_ms = now,
        .valid = false
    };
    xQueueOverwrite(disQueHandle, &distance);
}

static void finish_sensor(uint32_t now)
{
    distance_data_t distance = {
        .left_cm = pending_cm[SENSOR_LEFT],
        .front_cm = pending_cm[SENSOR_FRONT],
        .right_cm = pending_cm[SENSOR_RIGHT],
        .measured_ms = now,
        .front_trigger_ms = front_trigger_ms,
        .valid_mask = pending_valid_mask,
        .valid = pending_valid_mask == DISTANCE_ALL_VALID
    };
    xQueueOverwrite(disQueHandle, &distance);

    if (sensor == SENSOR_RIGHT) {
        sensor = SENSOR_LEFT;
    } else {
        sensor++;
    }
    phase = WAIT_BETWEEN_PINGS;
}

void distance_task_init(void)
{
    HCSR04_init(&sensors[SENSOR_LEFT],
                GPIOB, GPIO_PIN_13,
				GPIOB, GPIO_PIN_5, &htim2);
    HCSR04_init(&sensors[SENSOR_FRONT],
    		GPIOB, GPIO_PIN_15,
			GPIOB, GPIO_PIN_10, &htim2);
    HCSR04_init(&sensors[SENSOR_RIGHT],
    		GPIOB, GPIO_PIN_14,
			GPIOB, GPIO_PIN_4, &htim2);

    phase = START_MEASUREMENT;
    sensor = SENSOR_LEFT;
    first_ping = true;
    pending_valid_mask = 0;
    if (disQueHandle != NULL) {
        publish_invalid(HAL_GetTick());
    }
}

void distance_task(void)
{
    uint32_t now = HAL_GetTick();
    uint32_t echo_us;
    hcsr04_status_t result;

    if (disQueHandle == NULL) {
        return;
    }

    switch (phase) {
    case START_MEASUREMENT:
        if (!first_ping &&
            (uint32_t)(now - last_ping_ms) < PING_INTERVAL_MS) {
            return;
        }

        result = HCSR04_start(&sensors[sensor]);
        last_ping_ms = now;
        first_ping = false;

        if (result != HCSR04_IN_PROGRESS) {
            HCSR04_reset(&sensors[sensor]);
            pending_cm[sensor] = 0;
            pending_valid_mask &= (uint8_t)~(1U << sensor);
            finish_sensor(now);
            return;
        }

        if (sensor == SENSOR_FRONT) {
            front_trigger_ms = now;
        }
        phase = WAIT_RESULT;
        break;

    case WAIT_RESULT:
        result = HCSR04_getResult(&sensors[sensor], &echo_us);
        if (result == HCSR04_IN_PROGRESS) {
            return;
        }

        if (result != HCSR04_OK) {
            HCSR04_reset(&sensors[sensor]);
            pending_cm[sensor] = 0;
            pending_valid_mask &= (uint8_t)~(1U << sensor);
            finish_sensor(now);
            return;
        }

        pending_cm[sensor] = (uint16_t)(echo_us / 58U);
        pending_valid_mask |= (uint8_t)(1U << sensor);
        finish_sensor(now);
        break;

    case WAIT_BETWEEN_PINGS:
        if ((uint32_t)(now - last_ping_ms) >= PING_INTERVAL_MS) {
            phase = START_MEASUREMENT;
        }
        break;
    }
}
