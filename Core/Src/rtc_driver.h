/*
 * rtc_driver.h
 *
 *  Created on: Sep 10, 2022
 *      Author: a6gue
 */

#ifndef SRC_RTC_DRIVER_H_
#define SRC_RTC_DRIVER_H_
#include "main.h"
#include "cmsis_os.h"
#include "queue.h"
#include "timers.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h>

void rtcPrintInitMessage(RTC_HandleTypeDef *hrtc);
void configureRTCTime(RTC_HandleTypeDef *hrtc);
void configureRTCDate(RTC_HandleTypeDef *hrtc);
void rtcReportingCb(void *args);
#endif /* SRC_RTC_DRIVER_H_ */
