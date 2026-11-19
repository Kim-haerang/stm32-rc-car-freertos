/*
 * message.h
 *
 *  Created on: 2026. 11. 12.
 *      Author: kccistc
 */

#ifndef SRC_AP_MESSAGE_H_
#define SRC_AP_MESSAGE_H_

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "cmsis_os.h"
#include "queue.h"
#include "../driver/led/led.h"

typedef enum{RC, AUTO, SEMI_AUTO} btn_status_t;
extern osMessageQId btnQueHandle;

#define DISTANCE_LEFT_VALID  (1U << 0)
#define DISTANCE_FRONT_VALID (1U << 1)
#define DISTANCE_RIGHT_VALID (1U << 2)
#define DISTANCE_ALL_VALID   (DISTANCE_LEFT_VALID | DISTANCE_FRONT_VALID | DISTANCE_RIGHT_VALID)

typedef struct {
    uint16_t left_cm;
    uint16_t front_cm;
    uint16_t right_cm;
    uint32_t measured_ms;
    uint32_t front_trigger_ms;
    uint8_t valid_mask;
    bool valid;
} distance_data_t;
extern QueueHandle_t disQueHandle;
extern osMessageQId soundQueHandle;

typedef enum {
	COMM_NO,
	COMM_STOP,
	COMM_RUN,
	COMM_SPEED
}comm_status_t;

typedef struct {
	comm_status_t status;
	int speed;
} comm_data_t;

typedef enum {
	RC_NO,
	RC_FORWARD,
	RC_BACKWARD,
	RC_RIGHT,
	RC_LEFT,
	RC_SPEED,
	RC_RUN,
	RC_STOP,
	RC_RELEASE,
	RC_DIAGRIGHT,
	RC_DIAGLEFT
}RC_move_t;

typedef struct {
	RC_move_t move;
	int speed;
} RC_data_t;

extern osMessageQId commQueueHandle;

extern osMutexId lcdMutexHandle;
extern osMutexId ledMutexHandle;
extern osMutexId dispMutexHandle;

extern osMessageQId RCQueHandle;

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;

#endif /* SRC_AP_MESSAGE_H_ */
