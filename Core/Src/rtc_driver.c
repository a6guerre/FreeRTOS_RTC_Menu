/*
 * rtc_driver.c
 *
 *  Created on: Sep 10, 2022
 *      Author: a6gue
 */
#include "rtc_driver.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef enum
{
  HOUR,
  MINUTE,
  SECOND,
  AM_PM
}RTCTimeState;

typedef enum
{
  DATE,
  MONTH,
  YEAR
}RTCDateState;

char rtc_format_str[40];
const char* const rtc_msg[8] = {"\n=========================\n",
        "|                     RTC                    |\n" ,
        "=========================\n \n"  ,
        "Current Time&Date:             ",
        "Configure Time:       ----> 0\n",
        "Configure Date:       ----> 1\n",
        "Enable Reporting:     ----> 2\n",
        "Exit                  ----> 3\n"};
static char *rtcFormatTimeAndDate(RTC_HandleTypeDef *hrtc);

static char *rtcFormatTimeAndDate(RTC_HandleTypeDef *hrtc)
{
  int idx;
  char buf[20];
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;

  HAL_RTC_GetTime(hrtc, &sTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(hrtc, &sDate, RTC_FORMAT_BIN);
  //TODO: Add AM/PM and Date
  //int ret = snprintf(rtc_format_str, 20, "%02d:%02d:%02d      %02d-%02d-20%02d\n",
	//	   sTime.Hours, sTime.Minutes ,sTime.Seconds, sDate.Date, sDate.Month, sDate.Year);

  if(sTime.Hours < 10)
  {
	buf[0] = '0';
    itoa(sTime.Hours, &buf[1], 10);
  }
  else
  {
    itoa(sTime.Hours, buf, 10);
  }
  strcpy(rtc_format_str, buf);
  idx = strlen(rtc_format_str);
  rtc_format_str[idx] = ':';
  ++idx;
  rtc_format_str[idx] = '\0';

  if(sTime.Minutes < 10)
  {
    buf[0] = '0';
	itoa(sTime.Minutes, &buf[1], 10);
  }
  else
  {
    itoa(sTime.Hours, buf, 10);
  }

  strcpy(&rtc_format_str[idx], buf);
  idx = strlen(rtc_format_str);
  rtc_format_str[idx] = ':';
  ++idx;
  rtc_format_str[idx] = '\0';

  if(sTime.Seconds < 10)
  {
    buf[0] = '0';
	itoa(sTime.Seconds, &buf[1], 10);
  }
  else
  {
    itoa(sTime.Seconds, buf, 10);
  }

  strcpy(&rtc_format_str[idx], buf);
  idx = strlen(rtc_format_str);

  if(sTime.TimeFormat)
  {
    strcat(rtc_format_str, " PM");
  }
  else
  {
    strcat(rtc_format_str, " AM");
  }
  idx = strlen(rtc_format_str);

  rtc_format_str[idx] = ' ';
  ++idx;
  rtc_format_str[idx] = ' ';
  ++idx;
  rtc_format_str[idx] = ' ';
  ++idx;

  if(sDate.Date < 10)
  {
    buf[0] = '0';
	itoa(sDate.Date, &buf[1], 10);
  }
  else
  {
    itoa(sDate.Date, buf, 10);
  }
  strcpy(&rtc_format_str[idx], buf);
  idx = strlen(rtc_format_str);
  rtc_format_str[idx] = '-';
  ++idx;
  rtc_format_str[idx] = '\0';

  if(sDate.Month < 10)
  {
    buf[0] = '0';
	itoa(sDate.Month, &buf[1], 10);
  }
  else
  {
    itoa(sDate.Month, buf, 10);
  }
  strcpy(&rtc_format_str[idx], buf);
  idx = strlen(rtc_format_str);
  rtc_format_str[idx] = '-';
  ++idx;
  rtc_format_str[idx] = '2';
  ++idx;
  rtc_format_str[idx] = '0';
  ++idx;

  if(sDate.Year < 10)
  {
    buf[0] = '0';
	itoa(sDate.Year, &buf[1], 10);
	buf[2] = '\n';
	buf[3] = '\0';
  }
  else
  {
    itoa(sDate.Year, buf, 10);
	buf[2] = '\n';
	buf[3] = '\0';
  }
  strcpy(&rtc_format_str[idx], buf);
  idx = strlen(rtc_format_str);

  /*if(sTime.TimeFormat)
  {
    strcat(rtc_format_str, "PM\n");
  }
  else
  {
    strcat(rtc_format_str, "AM\n");
  }
  idx = strlen(rtc_format_str);*/
  //rtc_format_str[idx] = '\0';

  return rtc_format_str;
}

void rtcPrintInitMessage(RTC_HandleTypeDef *hrtc)
{
  char *rtc_str;

  for(int i = 0; i < 8; ++i)
  {
    xQueueSendToBack(printQueueHandle, &rtc_msg[i], portMAX_DELAY);
    if(i == 3)
    {
      rtc_str = rtcFormatTimeAndDate(hrtc);
      xQueueSendToBack(printQueueHandle, &rtc_str, portMAX_DELAY);
    }
  }
}

void configureRTCTime(RTC_HandleTypeDef *hrtc)
{
  int recv_val, val, len;
  RTCTimeState state = HOUR;
  uint8_t recv_input = 1;
  char *rtc_buf = (char *)malloc(40);
  char *rtc_str = (char *)malloc(40);
  char *test_str = (char *)malloc(40);

  /*char rtc_buf[40];
  char rtc_str[40];
  char test_str[40];*/

  RTC_TimeTypeDef s_time, currentTime;
  char recv_buf[20];

  while(recv_input)
  {
    switch (state)
    {
      case HOUR:
        //TODO: Make into routine
        test_str = "Enter Hour (1-12): ";
        len = strlen(test_str);
        HAL_UART_Transmit(&huart2, test_str, len, 1000);

        recvMsgFromQueue(rtcQueueHandle, rtc_buf);
        recv_val = atoi(rtc_buf);

        if(recv_val >= 1 && recv_val <= 12)
        {

          //TODO: Add logic for AM/PM
          s_time.Hours = recv_val;
          HAL_RTC_SetTime(hrtc, &s_time, RTC_FORMAT_BIN);
          state = MINUTE;
          /*HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
          //TODO: Add AM/PM and Date
      	  snprintf(rtc_str, 20, "Current Time: %02d:%02d:%02d\n", sTime.Hours, sTime.Minutes ,sTime.Seconds);*/
      	  //rtc_str[19] = '\0';
      	  //xQueueSendToBack(printQueueHandle, &rtc_str, portMAX_DELAY);
		  //val = atoi(recv_buf);
        }
        else
        {
          test_str = "Invalid Option\n";
	      int len = strlen(test_str);
          HAL_UART_Transmit(&huart2, test_str, len, 1000);
          recv_input = 0;// returns
          setCommandTaskState(MAIN_MENU);
        }
        break;
      case MINUTE:
        test_str = "Enter minutes (0-59): ";
	    len = strlen(test_str);
        HAL_UART_Transmit(&huart2, test_str, len, 1000);

        //Will block until queue receives from command handling task.
        recvMsgFromQueue(rtcQueueHandle, rtc_buf);
        recv_val = atoi(rtc_buf);

        if(recv_val >= 0 && recv_val <= 59)
        {
          s_time.Minutes = recv_val;
          HAL_RTC_SetTime(hrtc, &s_time, RTC_FORMAT_BIN);
          //HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
          //TODO: Add AM/PM and Date
          //snprintf(rtc_str, 40, "Current Time: %02d:%02d:%02d\n", sTime.Hours, sTime.Minutes ,sTime.Seconds);
          //rtc_str[19] = '\0';
          //xQueueSendToBack(printQueueHandle, &rtc_str, portMAX_DELAY);
          //blocks until command queue pushes onto rtcQueueHandle
          state = SECOND;

          //recv_input = 0;
        }
        else
        {
          test_str = "Invalid Option\n";
  	      len = strlen(test_str);
          HAL_UART_Transmit(&huart2, test_str, len, 1000);
          recv_input = 0;// returns
          setCommandTaskState(MAIN_MENU);
        }
        break;
      case SECOND:
        test_str = "Enter Seconds (0-59): ";
  	    len = strlen(test_str);
  	    HAL_UART_Transmit(&huart2, test_str, len, 1000);

        recvMsgFromQueue(rtcQueueHandle, rtc_buf);
        recv_val = atoi(rtc_buf);

    	if(recv_val >= 0 && recv_val <= 59)
    	{
          s_time.Seconds = val;
          HAL_RTC_SetTime(hrtc, &s_time, RTC_FORMAT_BIN);
          state = AM_PM;
    	}
    	else
    	{
          test_str = "Invalid Option\n";
    	  len = strlen(test_str);
          HAL_UART_Transmit(&huart2, test_str, len, 1000);
          recv_input = 0;// returns
          setCommandTaskState(MAIN_MENU);
    	}
        break;
      case AM_PM:
        test_str = "Enter AM or PM\n";
    	len = strlen(test_str);
    	HAL_UART_Transmit(&huart2, test_str, len, 1000);

        recvMsgFromQueue(rtcQueueHandle, rtc_buf);

    	if((strcmp("AM", rtc_buf) == 0) || (strcmp("PM", rtc_buf) == 0))
    	{
    	  s_time.TimeFormat = (strcmp("AM", rtc_buf) == 0) ? 0 : 1;
    	  HAL_RTC_SetTime(hrtc, &s_time, RTC_FORMAT_BIN);
          HAL_RTC_GetTime(hrtc, &currentTime, RTC_FORMAT_BIN);
          rtc_str = rtcFormatTimeAndDate(hrtc);
    	  xQueueSendToBack(printQueueHandle, &rtc_str, portMAX_DELAY);

    	  recv_input = 0;// returns
    	  setCommandTaskState(MAIN_MENU);
    	}
    	else
    	{
          test_str = "Invalid Option\n";
      	  len = strlen(test_str);
          HAL_UART_Transmit(&huart2, test_str, len, 1000);
          recv_input = 0;// returns
          setCommandTaskState(MAIN_MENU);
    	}
        break;
    }
  }
}

void configureRTCDate(RTC_HandleTypeDef *hrtc)
{
  uint8_t recv_val, recv_input = 1;
  char rtc_recv_buf[40];
  RTC_DateTypeDef rtcSetDate, rtcCurrentDate;
  RTCDateState date_state = DATE;
  const char * const rtcDateMsg = "Enter Date (1-31): \n";
  const char * const rtcMonthMsg = "Enter Month (1-12): \n";
  const char * const rtcYearMsg = "Enter Year: \n";
  const char * const rtcInvalidMsg = "Invalid Option, Follow Directions You Bozo \n";
  const char * const rtcCompleteMsg = "Configuration Successful\n";

  char *date_str = malloc(40);
  char *date_time_str = malloc(40);

  while(recv_input)
  {
    switch(date_state)
    {
      case DATE:
        xQueueSendToBack(printQueueHandle, &rtcDateMsg, portMAX_DELAY);
        //Block this rtcTask until we have recv'd input from user.
        recvMsgFromQueue(rtcQueueHandle, rtc_recv_buf);
        recv_val = atoi(rtc_recv_buf);
        if (recv_val >= 1 && recv_val <= 31)
        {
         //store onto date struct.
          rtcSetDate.Date = recv_val;
          HAL_RTC_SetDate(hrtc, &rtcSetDate, RTC_FORMAT_BIN);
          date_state = MONTH;
        }
        else
        {
          xQueueSendToBack(printQueueHandle, &rtcInvalidMsg, portMAX_DELAY);
          // Stop recv input and return to MAIN_MENU state
          recv_input = 0;
          setCommandTaskState(MAIN_MENU);
        }
        break;
      case MONTH:
        xQueueSendToBack(printQueueHandle, &rtcMonthMsg, portMAX_DELAY);
        recvMsgFromQueue(rtcQueueHandle, rtc_recv_buf);
        recv_val = atoi(rtc_recv_buf);
        if (recv_val >= 1 && recv_val <= 12)
        {
         //store onto date struct.
          rtcSetDate.Month = recv_val;
          HAL_RTC_SetDate(hrtc, &rtcSetDate, RTC_FORMAT_BIN);
          date_state = YEAR;
        }
        else
        {
          xQueueSendToBack(printQueueHandle, &rtcInvalidMsg, portMAX_DELAY);
          // Stop recv input and return to MAIN_MENU state
          recv_input = 0;
          setCommandTaskState(MAIN_MENU);
        }
        break;
      case YEAR:
        xQueueSendToBack(printQueueHandle, &rtcYearMsg, portMAX_DELAY);
        recvMsgFromQueue(rtcQueueHandle, rtc_recv_buf);
        recv_val = atoi(rtc_recv_buf);
        if (recv_val >= 0 && recv_val <= 99)
        {
          //store onto date struct.
          rtcSetDate.Year = recv_val;
          HAL_RTC_SetDate(hrtc, &rtcSetDate, RTC_FORMAT_BIN);
          //print full date
          //HAL_RTC_GetDate(&hrtc, &rtcCurrentDate, RTC_FORMAT_BIN);
          xQueueSendToBack(printQueueHandle, &rtcCompleteMsg, portMAX_DELAY);
          date_time_str = rtcFormatTimeAndDate(hrtc);
          xQueueSendToBack(printQueueHandle, &date_time_str, portMAX_DELAY);
          recv_input = 0;
          setCommandTaskState(MAIN_MENU);
        }
        else
        {
          xQueueSendToBack(printQueueHandle, &rtcInvalidMsg, portMAX_DELAY);
          // Stop recv input and return to MAIN_MENU state
          recv_input = 0;
          setCommandTaskState(MAIN_MENU);
        }
        break;
        //error handling
    }
  }
}

void rtcReportingCb(void *args)
{
  RTC_HandleTypeDef *hrtc = args;
  char *rtc_str;
  rtc_str = rtcFormatTimeAndDate(hrtc);
  xQueueSendToBack(printQueueHandle, &rtc_str, portMAX_DELAY);
}
