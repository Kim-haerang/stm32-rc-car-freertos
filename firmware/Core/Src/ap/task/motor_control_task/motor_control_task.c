/*
 * motor_control.c
 *
 *  Created on: 2026. 11. 12.
 *      Author: kccistc
 */

#include "motor_control_task.h"

#include "../../../driver/dc_motor/dc_motor.h"
#include "../../../driver/lcd/lcd.h"
#include "../../../driver/led/led.h"
#include "../../../driver/button/button.h"
#include "../sound_task/sound_task.h"
#include "../../message.h"


extern TIM_HandleTypeDef htim3;

typedef enum{STOP, RUN, LEFT, RIGHT, BACK, DIAGLEFT, DIAGRIGHT} motor_state_t;
static motor_t motor_l, motor_r;
static motor_state_t motor_state = STOP;

static comm_data_t *pcomm_data;
static RC_data_t *prc_data;
static comm_data_t comm_data_dummy = {0};
static RC_data_t rc_data_dummy = {0};

static btn_status_t btn_status = RC;
static led_t led_l, led_r;
static int16_t c_speed = 0, n_speed = 0;

void motor_control_task_init(void)
{
	c_speed = 900;
	n_speed = c_speed;
	motor_init(&motor_r, &htim3, TIM_CHANNEL_3, GPIOA, GPIO_PIN_5, GPIOA, GPIO_PIN_6);
	motor_init(&motor_l, &htim3, TIM_CHANNEL_1, GPIOB, GPIO_PIN_6, GPIOA, GPIO_PIN_7);
	motor_set_speed(c_speed);
	led_init(&led_l, GPIOA, GPIO_PIN_0);
	led_init(&led_r, GPIOA, GPIO_PIN_1);
}

static void motor_RC_control_task(void)
{
	osEvent comm_evt;
	comm_evt = osMessageGet(commQueueHandle, 0);
	if(comm_evt.status == osEventMessage){
		pcomm_data = comm_evt.value.p;
	}
	else {
		pcomm_data = &comm_data_dummy;
	}
	osEvent RC_evt;
	RC_evt = osMessageGet(RCQueHandle, 0);
	if(RC_evt.status == osEventMessage){
		prc_data = RC_evt.value.p;
	}
	else {
		prc_data = &rc_data_dummy;
	}

	if(pcomm_data->status == COMM_SPEED) {
		motor_set_speed(pcomm_data->speed);
	}
	if(prc_data->move == RC_SPEED) {
		motor_set_speed(prc_data->speed);
	}

	switch(motor_state)
	{
	case STOP:
		if((pcomm_data->status == COMM_RUN)||(prc_data->move == RC_FORWARD)) {
			motor_state = RUN;
			sound_beepbeep();
		}
		else if(prc_data->move == RC_RIGHT) {
			motor_state = RIGHT;
			sound_beepbeep();
			n_speed = 700;
		}
		//		else if(btn_status == BTN_LEFT) {
		//			motor_state = LEFT;
		//		}
		else if(prc_data->move == RC_LEFT) {
			motor_state = LEFT;
			sound_beepbeep();
			n_speed = 700;
		}
		else if((prc_data->move == RC_LEFT)&&(prc_data->move == RC_FORWARD)) {
			motor_state = DIAGLEFT;
			sound_beepbeep();
		}
		else if((prc_data->move == RC_RIGHT)&&(prc_data->move == RC_FORWARD)) {
			motor_state = DIAGRIGHT;
			sound_beepbeep();
		}
		else if(prc_data->move == RC_BACKWARD) {
			motor_state = BACK;
			sound_beepbeep();
		}
		motor_stop(&motor_l, &motor_r);

		//				if(osMutexWait(ledMutexHandle, osWaitForever) == osOK){
		//					led_off(&led_l);
		//					led_off(&led_r);
		//					osMutexRelease(ledMutexHandle);
		//				}
		//				if(osMutexWait(lcdMutexHandle, osWaitForever) == osOK){
		//					Lcd_writeStringXY(0, 0, "Motorstate stop ");
		//					osMutexRelease(lcdMutexHandle);
		//				}
		if(osMutexWait(lcdMutexHandle, osWaitForever) == osOK){
			led_off(&led_l);
			led_off(&led_r);
			Lcd_writeStringXY(0, 0, "Motorstate stop ");
			osMutexRelease(lcdMutexHandle);
		}
		break;
	case RUN:
		if((pcomm_data->status  == COMM_STOP)||(prc_data->move == RC_STOP)||(prc_data->move == RC_RELEASE)) {
			motor_state = STOP;
			sound_beepbeep();
		}
		else if(prc_data->move == RC_RIGHT) {
			motor_state = RIGHT;
			sound_beepbeep();
			n_speed = 700;
		}
		else if(prc_data->move == RC_LEFT) {
			motor_state = LEFT;
			sound_beepbeep();
			n_speed = 700;
		}
		else if((prc_data->move == RC_LEFT)&&(prc_data->move == RC_FORWARD)) {
			motor_state = DIAGLEFT;
			sound_beepbeep();
		}
		else if((prc_data->move == RC_RIGHT)&&(prc_data->move == RC_FORWARD)) {
			motor_state = DIAGRIGHT;
			sound_beepbeep();
		}
		else if(prc_data->move == RC_BACKWARD) {
			motor_state = BACK;
			sound_beepbeep();
		}
		motor_forward(&motor_l, &motor_r);
		c_speed = n_speed;
		motor_set_speed(c_speed);
		//				if(osMutexWait(ledMutexHandle, osWaitForever) == osOK){
		//					led_off(&led_l);
		//					led_off(&led_r);
		//					osMutexRelease(ledMutexHandle);
		//				}
		//				if(osMutexWait(lcdMutexHandle, osWaitForever) == osOK){
		//					Lcd_writeStringXY(0, 0, "Motorstate run  ");
		//					osMutexRelease(lcdMutexHandle);
		//				}
		if(osMutexWait(lcdMutexHandle, osWaitForever) == osOK){

			Lcd_writeStringXY(0, 0, "Motorstate run ");
			osMutexRelease(lcdMutexHandle);
		}
		break;
	case LEFT:
		if((prc_data->move == RC_STOP)||(prc_data->move == RC_RELEASE)) {
			motor_state = STOP;
			sound_beepbeep();
		}
		else if(prc_data->move == RC_FORWARD) {
			motor_state = RUN;
			sound_beepbeep();
		}
		else if(prc_data->move == RC_RIGHT) {
			motor_state = RIGHT;
			sound_beepbeep();
			n_speed = 700;
		}
		else if(prc_data->move == RC_LEFT) {
			motor_state = LEFT;
			sound_beepbeep();
			n_speed = 700;
		}
		else if((prc_data->move == RC_LEFT)&&(prc_data->move == RC_FORWARD)) {
			motor_state = DIAGLEFT;
			sound_beepbeep();
		}
		else if((prc_data->move == RC_RIGHT)&&(prc_data->move == RC_FORWARD)) {
			motor_state = DIAGRIGHT;
			sound_beepbeep();
		}
		else if(prc_data->move == RC_BACKWARD) {
			motor_state = BACK;
			sound_beepbeep();
		}
		motor_left(&motor_l, &motor_r);
		c_speed = n_speed;
		motor_set_speed(c_speed);

		//				if(osMutexWait(ledMutexHandle, osWaitForever) == osOK){
		//					led_on(&led_l);
		//					led_off(&led_r);
		//					osMutexRelease(ledMutexHandle);
		//				}
		//				if(osMutexWait(lcdMutexHandle, osWaitForever) == osOK){
		//					Lcd_writeStringXY(0, 0, "Motorstate left ");
		//					osMutexRelease(lcdMutexHandle);
		//				}
		if(osMutexWait(lcdMutexHandle, osWaitForever) == osOK){
			led_on(&led_l);
			Lcd_writeStringXY(0, 0, "Motorstate left  ");
			osMutexRelease(lcdMutexHandle);
		}
		break;
	case DIAGLEFT:
		if((prc_data->move == RC_STOP)||(prc_data->move == RC_RELEASE)) {
			motor_state = STOP;
			sound_beepbeep();
		}
		else if(prc_data->move == RC_FORWARD) {
			motor_state = RUN;
			sound_beepbeep();
		}
		else if(prc_data->move == RC_RIGHT) {
			motor_state = RIGHT;
			sound_beepbeep();
		}
		else if(prc_data->move == RC_LEFT) {
			motor_state = LEFT;
			sound_beepbeep();
		}
		else if((prc_data->move == RC_LEFT)&&(prc_data->move == RC_FORWARD)) {
			motor_state = DIAGLEFT;
			sound_beepbeep();
		}
		else if((prc_data->move == RC_RIGHT)&&(prc_data->move == RC_FORWARD)) {
			motor_state = DIAGRIGHT;
			sound_beepbeep();
		}
		else if(prc_data->move == RC_BACKWARD) {
			motor_state = BACK;
			sound_beepbeep();
		}
		motor_diag_left(&motor_l, &motor_r);
		motor_set_speed(c_speed);

		//				if(osMutexWait(ledMutexHandle, osWaitForever) == osOK){
		//					led_on(&led_l);
		//					led_off(&led_r);
		//					osMutexRelease(ledMutexHandle);
		//				}
		//				if(osMutexWait(lcdMutexHandle, osWaitForever) == osOK){
		//					Lcd_writeStringXY(0, 0, "Motorstate left ");
		//					osMutexRelease(lcdMutexHandle);
		//				}
		if(osMutexWait(lcdMutexHandle, osWaitForever) == osOK){
			led_on(&led_l);
			Lcd_writeStringXY(0, 0, "Motorstate drift ");
			osMutexRelease(lcdMutexHandle);
		}
		break;
	case RIGHT:
		if((prc_data->move == RC_STOP)||(prc_data->move == RC_RELEASE)) {
			motor_state = STOP;
			sound_beepbeep();
		}
		else if(prc_data->move == RC_FORWARD) {
			motor_state = RUN;
			sound_beepbeep();
		}
		else if(prc_data->move == RC_RIGHT) {
			motor_state = RIGHT;
			sound_beepbeep();
			n_speed = 700;
		}
		else if(prc_data->move == RC_LEFT) {
			motor_state = LEFT;
			sound_beepbeep();
			n_speed = 700;
		}
		else if((prc_data->move == RC_LEFT)&&(prc_data->move == RC_FORWARD)) {
			motor_state = DIAGLEFT;
			sound_beepbeep();
		}
		else if((prc_data->move == RC_RIGHT)&&(prc_data->move == RC_FORWARD)) {
			motor_state = DIAGRIGHT;
			sound_beepbeep();
		}
		else if(prc_data->move == RC_BACKWARD) {
			motor_state = BACK;
			sound_beepbeep();
		}
		motor_right(&motor_l, &motor_r);
		c_speed = n_speed;
		motor_set_speed(c_speed);

		if(osMutexWait(lcdMutexHandle, osWaitForever) == osOK){

			led_on(&led_r);
			Lcd_writeStringXY(0, 0, "Motorstate right ");
			osMutexRelease(lcdMutexHandle);
		}
		break;
	case DIAGRIGHT:
		if((prc_data->move == RC_STOP)||(prc_data->move == RC_RELEASE)) {
			motor_state = STOP;
			sound_beepbeep();
		}
		else if(prc_data->move == RC_FORWARD) {
			motor_state = RUN;
			sound_beepbeep();
		}
		else if(prc_data->move == RC_RIGHT) {
			motor_state = RIGHT;
			sound_beepbeep();
		}
		else if(prc_data->move == RC_LEFT) {
			motor_state = LEFT;
			sound_beepbeep();
		}
		else if((prc_data->move == RC_LEFT)&&(prc_data->move == RC_FORWARD)) {
			motor_state = DIAGLEFT;
			sound_beepbeep();
		}
		else if((prc_data->move == RC_RIGHT)&&(prc_data->move == RC_FORWARD)) {
			motor_state = DIAGRIGHT;
			sound_beepbeep();
		}
		else if(prc_data->move == RC_BACKWARD) {
			motor_state = BACK;
			sound_beepbeep();
		}
		motor_diag_right(&motor_l, &motor_r);

		if(osMutexWait(lcdMutexHandle, osWaitForever) == osOK){

			led_on(&led_r);
			Lcd_writeStringXY(0, 0, "Motorstate drift ");
			osMutexRelease(lcdMutexHandle);
		}
		break;
	case BACK:
		if((pcomm_data->status  == COMM_STOP)||(prc_data->move == RC_STOP)||(prc_data->move == RC_RELEASE)) {
			motor_state = STOP;
			sound_beepbeep();
		}
		else if(prc_data->move == RC_FORWARD) {
			motor_state = RUN;
			sound_beepbeep();
		}
		else if(prc_data->move == RC_RIGHT) {
			motor_state = RIGHT;
			sound_beepbeep();
		}
		else if(prc_data->move == RC_LEFT) {
			motor_state = LEFT;
			sound_beepbeep();
		}
		else if((prc_data->move == RC_LEFT)&&(prc_data->move == RC_FORWARD)) {
			motor_state = DIAGLEFT;
			sound_beepbeep();
		}
		else if((prc_data->move == RC_RIGHT)&&(prc_data->move == RC_FORWARD)) {
			motor_state = DIAGRIGHT;
			sound_beepbeep();
		}
		else if(prc_data->move == RC_BACKWARD) {
			motor_state = BACK;
			sound_beepbeep();
		}
		motor_backward(&motor_l, &motor_r);
		motor_set_speed(c_speed);

		if(osMutexWait(lcdMutexHandle, osWaitForever) == osOK){
			led_on(&led_l);
			led_on(&led_r);
			Lcd_writeStringXY(0, 0, "Motorstate back ");
			osMutexRelease(lcdMutexHandle);
		}
		break;
	default:
		break;
	}
}

static void format_distance_line(char line[17], const distance_data_t *distance)
{
    char left[4] = "---";
    char front[4] = "---";
    char right[4] = "---";

    if (distance->valid_mask & DISTANCE_LEFT_VALID) {
        snprintf(left, sizeof(left), "%03u",
                 distance->left_cm > 999U ? 999U : distance->left_cm);
    }
    if (distance->valid_mask & DISTANCE_FRONT_VALID) {
        snprintf(front, sizeof(front), "%03u",
                 distance->front_cm > 999U ? 999U : distance->front_cm);
    }
    if (distance->valid_mask & DISTANCE_RIGHT_VALID) {
        snprintf(right, sizeof(right), "%03u",
                 distance->right_cm > 999U ? 999U : distance->right_cm);
    }
    snprintf(line, 17, "L%s F%s R%s  ", left, front, right);
}

static void motor_AUTO_control_task(bool reset)
{
    /* Initial tuning values: adjust for actual track width and turning speed. */
    enum {
        FRONT_STOP_CM = 20,
        FRONT_RESUME_CM = 30,

        SIDE_MIN_CM = 15,
        SIDE_CRITICAL_CM = 5,
        BACK_TIME_MS = 250,
        RECOVERY_BACK_MS = 250,
        TURN_MIN_MS = 350,
        TURN_MAX_MS = 1300,
        TURN_SIDE_GRACE_MS = 500,
        TURN_SIDE_FAIL_CM = 8,
        DIRECTION_MARGIN_CM = 10,
        MAX_RECOVERIES = 5,
        DATA_MAX_AGE_MS = 600,
        LCD_TIME_MS = 200,
        AUTO_SPEED = 400
    };

    typedef enum {
        AUTO_FORWARD,
        AUTO_BACK,
        AUTO_TURN_LEFT,
        AUTO_TURN_RIGHT,
        AUTO_RECOVERY_BACK,
        AUTO_SAFE_STOP
    } auto_state_t;

    static auto_state_t state = AUTO_FORWARD;
    static uint32_t state_started_ms;
    static uint32_t lcd_updated_ms;
    static uint32_t invalid_at_ms;
    static uint32_t last_front_sample_ms;
    static uint8_t clear_front_samples;
    static uint8_t recoveries;
    static bool need_new_front = true;
    static bool last_turn_left = false;
    static bool have_last_turn = false;
    static int16_t applied_speed = -1;

    distance_data_t distance = {0};
    uint32_t now = HAL_GetTick();
    bool have_distance = disQueHandle != NULL &&
                         xQueuePeek(disQueHandle, &distance, 0) == pdPASS;

    if (reset) {
        state = AUTO_FORWARD;
        state_started_ms = now;
        lcd_updated_ms = now - LCD_TIME_MS;
        invalid_at_ms = now;
        last_front_sample_ms = 0;
        clear_front_samples = 0;
        recoveries = 0;
        need_new_front = true;
        have_last_turn = false;
        applied_speed = -1;
        motor_stop(&motor_l, &motor_r);
        motor_state = STOP;
    }

    /* Never drive with invalid or stale sensor data. */
    if (!have_distance || !distance.valid ||
        (uint32_t)(now - distance.measured_ms) > DATA_MAX_AGE_MS) {
        invalid_at_ms = now;
        need_new_front = true;
        motor_stop(&motor_l, &motor_r);
        motor_state = STOP;
        if ((uint32_t)(now - lcd_updated_ms) >= LCD_TIME_MS &&
            osMutexWait(lcdMutexHandle, 0) == osOK) {
            char line[17] = "L--- F--- R---  ";
            if (have_distance &&
                (uint32_t)(now - distance.measured_ms) <= DATA_MAX_AGE_MS) {
                format_distance_line(line, &distance);
            }
            Lcd_writeStringXY(0, 0, "AUTO SENSOR WAIT");
            Lcd_writeStringXY(1, 0, line);
            osMutexRelease(lcdMutexHandle);
            lcd_updated_ms = now;
        }
        return;
    }

    /* On entering AUTO or after a sensor fault, require a new front ping. */
    if (need_new_front) {
        if ((int32_t)(distance.front_trigger_ms - invalid_at_ms) <= 0) {
            motor_stop(&motor_l, &motor_r);
            motor_state = STOP;
            if ((uint32_t)(now - lcd_updated_ms) >= LCD_TIME_MS &&
                osMutexWait(lcdMutexHandle, 0) == osOK) {
                char line[17];
                format_distance_line(line, &distance);
                Lcd_writeStringXY(0, 0, "AUTO FRONT WAIT ");
                Lcd_writeStringXY(1, 0, line);
                osMutexRelease(lcdMutexHandle);
                lcd_updated_ms = now;
            }
            return;
        }
        need_new_front = false;
        /* Restart the maneuver after an interruption, not mid-turn. */
        if (!reset) {
            state = AUTO_FORWARD;
            state_started_ms = now;
            clear_front_samples = 0;
        }
    }

    if (applied_speed != AUTO_SPEED) {
        motor_set_speed(AUTO_SPEED);
        applied_speed = AUTO_SPEED;
    }

    if (state == AUTO_FORWARD) {
        if (distance.front_cm <= FRONT_STOP_CM ||
            (distance.left_cm < SIDE_MIN_CM &&
             distance.right_cm < SIDE_MIN_CM)) {
            state = AUTO_BACK;
            state_started_ms = now;
            clear_front_samples = 0;
        } else if (distance.left_cm < SIDE_MIN_CM) {
            state = AUTO_TURN_RIGHT;
            state_started_ms = now;
            last_turn_left = false;
            have_last_turn = true;
            clear_front_samples = 0;
        } else if (distance.right_cm < SIDE_MIN_CM) {
            state = AUTO_TURN_LEFT;
            state_started_ms = now;
            last_turn_left = true;
            have_last_turn = true;
            clear_front_samples = 0;
        } else {
            recoveries = 0;
        }
    }

    if ((state == AUTO_BACK &&
         (uint32_t)(now - state_started_ms) >= BACK_TIME_MS) ||
        (state == AUTO_RECOVERY_BACK &&
         (uint32_t)(now - state_started_ms) >= RECOVERY_BACK_MS)) {
        bool recovery = (state == AUTO_RECOVERY_BACK);
        bool turn_left;

        if (recovery && have_last_turn &&
            ((distance.left_cm > distance.right_cm
                  ? distance.left_cm - distance.right_cm
                  : distance.right_cm - distance.left_cm) < DIRECTION_MARGIN_CM)) {
            /* Ambiguous reading: avoid retrying the failed direction. */
            turn_left = !last_turn_left;
        } else if (distance.left_cm > distance.right_cm + DIRECTION_MARGIN_CM) {
            turn_left = true;
        } else if (distance.right_cm > distance.left_cm + DIRECTION_MARGIN_CM) {
            turn_left = false;
        } else if (have_last_turn) {
            turn_left = recovery ? !last_turn_left : last_turn_left;
        } else {
            turn_left = (distance.left_cm >= distance.right_cm);
        }

        state = turn_left ? AUTO_TURN_LEFT : AUTO_TURN_RIGHT;
        last_turn_left = turn_left;
        have_last_turn = true;
        state_started_ms = now;
        clear_front_samples = 0;
        last_front_sample_ms = distance.front_trigger_ms;
    }

    if (state == AUTO_TURN_LEFT || state == AUTO_TURN_RIGHT) {
        uint32_t elapsed = (uint32_t)(now - state_started_ms);
        uint16_t turn_side_cm = state == AUTO_TURN_LEFT
                              ? distance.left_cm : distance.right_cm;

        /* Stop turning only after minimum rotation and two distinct
           front measurements showing an open corridor. */
        if (elapsed >= TURN_MIN_MS &&
            (int32_t)(distance.front_trigger_ms - last_front_sample_ms) > 0) {
            last_front_sample_ms = distance.front_trigger_ms;
            if (distance.front_cm >= FRONT_RESUME_CM &&
                distance.left_cm >= SIDE_MIN_CM &&
                distance.right_cm >= SIDE_MIN_CM) {
                if (clear_front_samples < 2U) {
                    clear_front_samples++;
                }
            } else {
                clear_front_samples = 0;
            }
        }

        if (clear_front_samples >= 2U) {
            state = AUTO_FORWARD;
            state_started_ms = now;
            recoveries = 0;
        } else if (elapsed >= TURN_MAX_MS ||
                   turn_side_cm <= SIDE_CRITICAL_CM ||
                   (elapsed >= TURN_SIDE_GRACE_MS &&
                    turn_side_cm < TURN_SIDE_FAIL_CM)) {
            if (recoveries >= MAX_RECOVERIES) {
                state = AUTO_SAFE_STOP;
            } else {
                recoveries++;
                state = AUTO_RECOVERY_BACK;
                state_started_ms = now;
            }
            clear_front_samples = 0;
        }
    }

    switch (state) {
    case AUTO_FORWARD:
        motor_forward(&motor_l, &motor_r);
        motor_state = RUN;
        break;
    case AUTO_BACK:
    case AUTO_RECOVERY_BACK:
        motor_backward(&motor_l, &motor_r);
        motor_state = BACK;
        break;
    case AUTO_TURN_LEFT:
        motor_left(&motor_l, &motor_r);
        motor_state = LEFT;
        break;
    case AUTO_TURN_RIGHT:
        motor_right(&motor_l, &motor_r);
        motor_state = RIGHT;
        break;
    case AUTO_SAFE_STOP:
    default:
        motor_stop(&motor_l, &motor_r);
        motor_state = STOP;
        break;
    }

    if ((uint32_t)(now - lcd_updated_ms) >= LCD_TIME_MS &&
        osMutexWait(lcdMutexHandle, 0) == osOK) {
        char line[17];
        format_distance_line(line, &distance);
        if (state == AUTO_SAFE_STOP) {
            Lcd_writeStringXY(0, 0, "AUTO RECOV LIMIT");
        } else if (state == AUTO_RECOVERY_BACK) {
            Lcd_writeStringXY(0, 0, "AUTO RECOVERY   ");
        } else {
            Lcd_writeStringXY(0, 0, "AUTO            ");
        }
        Lcd_writeStringXY(1, 0, line);
        osMutexRelease(lcdMutexHandle);
        lcd_updated_ms = now;
    }
}

static void motor_SEMI_display_task(bool reset)
{
    static uint32_t lcd_updated_ms;
    uint32_t now = HAL_GetTick();
    distance_data_t distance;

    if (reset) {
        lcd_updated_ms = now - 200U;
    }
    if ((uint32_t)(now - lcd_updated_ms) < 200U ||
        osMutexWait(lcdMutexHandle, 0) != osOK) {
        return;
    }

    if (disQueHandle != NULL &&
        xQueuePeek(disQueHandle, &distance, 0) == pdPASS &&
        distance.valid_mask != 0U &&
        (uint32_t)(now - distance.measured_ms) <= 600U) {
        char line[17];
        format_distance_line(line, &distance);
        Lcd_writeStringXY(0, 0, "SEMI            ");
        Lcd_writeStringXY(1, 0, line);
    } else {
        Lcd_writeStringXY(0, 0, "SEMI SENSOR WAIT");
        Lcd_writeStringXY(1, 0, "L--- F--- R---  ");
    }

    osMutexRelease(lcdMutexHandle);
    lcd_updated_ms = now;
}

static void motor_SEMI_control_task(bool reset)
{
    static RC_move_t manual_move = RC_NO;
    static bool manual_active;
    static bool paused;
    static bool auto_needs_reset = true;
    osEvent rc_evt;

    if (reset) {
        manual_move = RC_NO;
        manual_active = false;
        paused = false;
        auto_needs_reset = true;
        if (RCQueHandle != NULL) {
            xQueueReset(RCQueHandle);
        }
    }

    while (RCQueHandle != NULL &&
           (rc_evt = osMessageGet(RCQueHandle, 0)).status == osEventMessage) {
        RC_data_t *received = rc_evt.value.p;
        RC_move_t move = received != NULL ? received->move : RC_NO;

        if (move == RC_STOP) {
            paused = true;
            manual_active = false;
            manual_move = RC_NO;
            auto_needs_reset = true;
        } else if (move == RC_RELEASE) {
            manual_active = false;
            manual_move = RC_NO;
            auto_needs_reset = true;
        } else if (move == RC_RUN) {
            paused = false;
            manual_active = false;
            manual_move = RC_NO;
            auto_needs_reset = true;
        } else if (move == RC_FORWARD || move == RC_BACKWARD ||
                   move == RC_LEFT || move == RC_RIGHT) {
            if (!paused) {
                if (!manual_active) {
                    motor_set_speed(400);
                }
                manual_active = true;
                manual_move = move;
                auto_needs_reset = true;
            }
        }
    }

    if (paused) {
        motor_stop(&motor_l, &motor_r);
        motor_state = STOP;
        return;
    }

    if (!manual_active) {
        motor_AUTO_control_task(auto_needs_reset);
        auto_needs_reset = false;
        return;
    }

    distance_data_t distance;
    uint32_t now = HAL_GetTick();
    if (disQueHandle == NULL ||
        xQueuePeek(disQueHandle, &distance, 0) != pdPASS ||
        !distance.valid ||
        (uint32_t)(now - distance.measured_ms) > 600U) {
        motor_stop(&motor_l, &motor_r);
        motor_state = STOP;
        return;
    }

    switch (manual_move) {
    case RC_FORWARD:
        if (distance.front_cm <= 20U) {
            motor_stop(&motor_l, &motor_r);
            motor_state = STOP;
        } else {
            motor_forward(&motor_l, &motor_r);
            motor_state = RUN;
        }
        break;
    case RC_BACKWARD:
        motor_backward(&motor_l, &motor_r);
        motor_state = BACK;
        break;
    case RC_LEFT:
        if (distance.left_cm <= 5U) {
            motor_stop(&motor_l, &motor_r);
            motor_state = STOP;
        } else {
            motor_left(&motor_l, &motor_r);
            motor_state = LEFT;
        }
        break;
    case RC_RIGHT:
        if (distance.right_cm <= 5U) {
            motor_stop(&motor_l, &motor_r);
            motor_state = STOP;
        } else {
            motor_right(&motor_l, &motor_r);
            motor_state = RIGHT;
        }
        break;
    default:
        motor_stop(&motor_l, &motor_r);
        motor_state = STOP;
        break;
    }
}

void motor_control_task(void)
{
	static bool auto_was_running = false;
	static bool semi_was_running = false;
	osEvent btn_evt = osMessageGet(btnQueHandle, 0);
	if (btn_evt.status == osEventMessage) {
		btn_status_t requested = (btn_status_t)btn_evt.value.v;
		if (requested == RC || requested == AUTO || requested == SEMI_AUTO) {
			if (requested != btn_status) {
				motor_stop(&motor_l, &motor_r);
				motor_state = STOP;
				if (requested == RC) {
					sound_rc();
				} else if (requested == AUTO) {
					sound_auto();
				} else {
					sound_semi();
				}
			}
			btn_status = requested;
		}
	}

	if (btn_status == RC) {
		auto_was_running = false;
		semi_was_running = false;
		motor_RC_control_task();
	} else if (btn_status == AUTO) {
		motor_AUTO_control_task(!auto_was_running);
		auto_was_running = true;
		semi_was_running = false;
	} else {
		auto_was_running = false;
		motor_SEMI_control_task(!semi_was_running);
		motor_SEMI_display_task(!semi_was_running);
		semi_was_running = true;
	}
}
