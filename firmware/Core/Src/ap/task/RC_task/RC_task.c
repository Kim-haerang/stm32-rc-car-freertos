/*
 * RC_task.c
 *
 *  Created on: 2026. 11. 12.
 *      Author: kccistc
 */

#include "RC_task.h"
#include "../../message.h"
#include "usart.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static RC_data_t RC_data;

static uint8_t rx_data = 0;
static uint8_t rx_index = 0;
static uint8_t rx_cplt_flag = 0;
static char rx_buff[32];

void RC_task_init(void)
{
	HAL_UART_Receive_IT(&huart6, &rx_data, 1);
}

void RC_task(void)
{
	if(rx_cplt_flag) {
		rx_cplt_flag = 0;
		if(strncmp(rx_buff, "F", 1) == 0) {
			RC_data.move = RC_FORWARD;
			osMessagePut(RCQueHandle, (uint32_t)&RC_data, 10);
		}
		else if (strncmp(rx_buff, "L", 1) == 0){
			RC_data.move = RC_LEFT;
			osMessagePut(RCQueHandle, (uint32_t)&RC_data, 10);
		}
		else if (strncmp(rx_buff, "RUN", 3) == 0){
			RC_data.move = RC_RUN;
			osMessagePut(RCQueHandle, (uint32_t)&RC_data, 10);
		}
		else if (strncmp(rx_buff, "R", 1) == 0){
			RC_data.move = RC_RIGHT;
			osMessagePut(RCQueHandle, (uint32_t)&RC_data, 10);
		}
		else if (strncmp(rx_buff, "B", 1) == 0){
			RC_data.move = RC_BACKWARD;
			osMessagePut(RCQueHandle, (uint32_t)&RC_data, 10);
		}
		else if (strncmp(rx_buff, "STOP", 3) == 0){
			RC_data.move = RC_STOP;
			osMessagePut(RCQueHandle, (uint32_t)&RC_data, 10);
		}
		else if (strncmp(rx_buff, "f", 1) == 0){
			RC_data.move = RC_RELEASE;
			osMessagePut(RCQueHandle, (uint32_t)&RC_data, 10);
		}
		else if (strncmp(rx_buff, "b", 1) == 0){
			RC_data.move = RC_RELEASE;
			osMessagePut(RCQueHandle, (uint32_t)&RC_data, 10);
		}
		else if (strncmp(rx_buff, "l", 1) == 0){
			RC_data.move = RC_RELEASE;
			osMessagePut(RCQueHandle, (uint32_t)&RC_data, 10);
		}
		else if (strncmp(rx_buff, "r", 1) == 0){
			RC_data.move = RC_RELEASE;
			osMessagePut(RCQueHandle, (uint32_t)&RC_data, 10);
		}
		else if (strncmp(rx_buff, "SPD", 3) == 0){
			RC_data.move = RC_SPEED;
			RC_data.speed = atoi(&rx_buff[3]);
			osMessagePut(RCQueHandle, (uint32_t)&RC_data, 10);
		}
		else {
			osMessagePut(RCQueHandle, RC_NO, 10);
		}

		rx_index = 0;

	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART6) {
		if(rx_data == '*'){
			rx_buff[rx_index++] = '\0'; // '\0' null 문자
			//		  rx_buff[rx_index++] = 0; // 0 null
			rx_cplt_flag = 1;
		}
		else {
			rx_buff[rx_index++] = rx_data;

		}

		HAL_UART_Receive_IT(&huart6, &rx_data, 1);
	}
}
