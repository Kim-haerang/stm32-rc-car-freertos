/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ap/task/button_task/button_task.h"
#include "ap/task/motor_control_task/motor_control_task.h"
#include "ap/task/RC_task/RC_task.h"
#include "ap/task/distance_task/distance_task.h"
#include "ap/task/sound_task/sound_task.h"
#include "ap/message.h"
#include "i2c.h"
#include "driver/lcd/lcd.h"
#include "driver/led/led.h"
#include <stdio.h>
#include "usart.h"
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
QueueHandle_t disQueHandle;

/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId MotorTaskHandle;
osThreadId ButtonTaskHandle;
osThreadId CommTaskHandle;
osThreadId RCTaskHandle;
osThreadId DistanceTaskHandle;
osThreadId SoundTaskHandle;
osMessageQId btnQueHandle;
osMessageQId commQueueHandle;
osMessageQId RCQueHandle;
osMessageQId soundQueHandle;
osMutexId lcdMutexHandle;
osMutexId dispMutexHandle;
osMutexId ledMutexHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartMotorTask(void const * argument);
void StartButtonTask(void const * argument);
void StartCommTask(void const * argument);
void StartRCTask(void const * argument);
void StartTask06(void const * argument);
void StartTask07(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
	*ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
	*ppxIdleTaskStackBuffer = &xIdleStack[0];
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
	/* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* definition and creation of lcdMutex */
  osMutexDef(lcdMutex);
  lcdMutexHandle = osMutexCreate(osMutex(lcdMutex));

  /* definition and creation of dispMutex */
  osMutexDef(dispMutex);
  dispMutexHandle = osMutexCreate(osMutex(dispMutex));

  /* definition and creation of ledMutex */
  osMutexDef(ledMutex);
  ledMutexHandle = osMutexCreate(osMutex(ledMutex));

  /* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of btnQue */
  osMessageQDef(btnQue, 16, uint32_t);
  btnQueHandle = osMessageCreate(osMessageQ(btnQue), NULL);

  /* definition and creation of commQueue */
  osMessageQDef(commQueue, 16, uint32_t);
  commQueueHandle = osMessageCreate(osMessageQ(commQueue), NULL);

  /* definition and creation of RCQue */
  osMessageQDef(RCQue, 16, uint32_t);
  RCQueHandle = osMessageCreate(osMessageQ(RCQue), NULL);

  /* definition and creation of soundQue */
  osMessageQDef(soundQue, 16, uint32_t);
  soundQueHandle = osMessageCreate(osMessageQ(soundQue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  disQueHandle = xQueueCreate(1, sizeof(distance_data_t));
	/* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of MotorTask */
  osThreadDef(MotorTask, StartMotorTask, osPriorityNormal, 0, 256);
  MotorTaskHandle = osThreadCreate(osThread(MotorTask), NULL);

  /* definition and creation of ButtonTask */
  osThreadDef(ButtonTask, StartButtonTask, osPriorityNormal, 0, 128);
  ButtonTaskHandle = osThreadCreate(osThread(ButtonTask), NULL);

  /* definition and creation of CommTask */
  osThreadDef(CommTask, StartCommTask, osPriorityNormal, 0, 128);
  CommTaskHandle = osThreadCreate(osThread(CommTask), NULL);

  /* definition and creation of RCTask */
  osThreadDef(RCTask, StartRCTask, osPriorityNormal, 0, 128);
  RCTaskHandle = osThreadCreate(osThread(RCTask), NULL);

  /* definition and creation of DistanceTask */
  osThreadDef(DistanceTask, StartTask06, osPriorityNormal, 0, 128);
  DistanceTaskHandle = osThreadCreate(osThread(DistanceTask), NULL);

  /* definition and creation of SoundTask */
  osThreadDef(SoundTask, StartTask07, osPriorityNormal, 0, 128);
  SoundTaskHandle = osThreadCreate(osThread(SoundTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
	if(osMutexWait(lcdMutexHandle, osWaitForever) == osOK){
		Lcd_init(&hi2c1);
		osMutexRelease(lcdMutexHandle);
	}
	/* Infinite loop */
	for(;;)
	{
		osDelay(1);
	}
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartMotorTask */
/**
 * @brief Function implementing the MotorTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartMotorTask */
void StartMotorTask(void const * argument)
{
  /* USER CODE BEGIN StartMotorTask */

	motor_control_task_init();

	/* Infinite loop */
	for(;;)
	{
		motor_control_task();

		osDelay(10);
	}
  /* USER CODE END StartMotorTask */
}

/* USER CODE BEGIN Header_StartButtonTask */
/**
 * @brief Function implementing the ButtonTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartButtonTask */
void StartButtonTask(void const * argument)
{
  /* USER CODE BEGIN StartButtonTask */

	button_task_init();
	/* Infinite loop */
	for(;;)
	{
		button_task();
		osDelay(10);
	}
  /* USER CODE END StartButtonTask */
}

/* USER CODE BEGIN Header_StartCommTask */
/**
 * @brief Function implementing the CommTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartCommTask */
void StartCommTask(void const * argument)
{
  /* USER CODE BEGIN StartCommTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartCommTask */
}

/* USER CODE BEGIN Header_StartRCTask */
/**
* @brief Function implementing the RCTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartRCTask */
void StartRCTask(void const * argument)
{
  /* USER CODE BEGIN StartRCTask */
	RC_task_init();
  /* Infinite loop */
  for(;;)
  {
	  RC_task();

    osDelay(1);
  }
  /* USER CODE END StartRCTask */
}

/* USER CODE BEGIN Header_StartTask06 */
/**
* @brief Function implementing the disTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask06 */
void StartTask06(void const * argument)
{
  /* USER CODE BEGIN StartTask06 */
  distance_task_init();
  for(;;)
  {
    distance_task();
    osDelay(10);
  }
  /* USER CODE END StartTask06 */
}

/* USER CODE BEGIN Header_StartTask07 */
/**
* @brief Function implementing the SoundTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask07 */
void StartTask07(void const * argument)
{
  /* USER CODE BEGIN StartTask07 */
  sound_task_init();
  for(;;)
  {
    sound_task();
    osDelay(10);
  }
  /* USER CODE END StartTask07 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
