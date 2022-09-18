/*
 * led_driver.h
 *
 *  Created on: Sep 10, 2022
 *      Author: a6gue
 */

#ifndef SRC_LED_DRIVER_H_
#define SRC_LED_DRIVER_H_
#include "main.h"
#include "cmsis_os.h"
#include "queue.h"
#include "timers.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h>

#define NUM_TIMERS 4

/* Definitions for ledTimer */
typedef struct timerConfig
{
  osTimerFunc_t timerCb;
  osTimerType_t  mode;
}timerConfig;

osTimerId_t ledTimerHandleArr[NUM_TIMERS];
osTimerId_t rtcTimerHandle;

void ledTask(void *argument);
void stopTimers(void);
void ledTimerCb1(void *argument);
void ledTimerCb2(void *argument);
void ledTimerCb3(void *argument);
void ledTimerCb4(void *argument);
void ledSetAllLow(void);

#endif /* SRC_LED_DRIVER_H_ */
