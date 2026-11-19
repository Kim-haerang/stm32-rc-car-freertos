/*
 * button_task.c
 *
 *  Created on: 2026. 11. 12.
 *      Author: kccistc
 */

#include "button_task.h"
#include "../../../driver/button/button.h"
#include "../../../driver/lcd/lcd.h"

#include "../../message.h"

button_t btn_rc, btn_auto, btn_semiAuto;

void button_task_init(void)
{
	button_init(&btn_rc, GPIOC, GPIO_PIN_0);
	button_init(&btn_auto, GPIOC, GPIO_PIN_1);
	button_init(&btn_semiAuto, GPIOC, GPIO_PIN_2);
}

void button_task(void)
{
	if(button_getState(&btn_rc) == ACT_RELEASE){
		osMessagePut(btnQueHandle, RC, 10);
		if(osMutexWait(lcdMutexHandle, osWaitForever) == osOK){
			Lcd_writeStringXY(1, 0, "BTN_RUN_RC");
			osMutexRelease(lcdMutexHandle);
		}
	}
	else if(button_getState(&btn_auto) == ACT_RELEASE){
		osMessagePut(btnQueHandle, AUTO, 10);
		if(osMutexWait(lcdMutexHandle, osWaitForever) == osOK){
			Lcd_writeStringXY(1, 0, "BTN_AUTO");
			osMutexRelease(lcdMutexHandle);
		}
	}
	else if(button_getState(&btn_semiAuto) == ACT_RELEASE){
		osMessagePut(btnQueHandle, SEMI_AUTO, 10);
		if(osMutexWait(lcdMutexHandle, osWaitForever) == osOK){
			Lcd_writeStringXY(1, 0, "BTN_SEMI_AUTO");
			osMutexRelease(lcdMutexHandle);
		}
	}
}
