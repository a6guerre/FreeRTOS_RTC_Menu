/*
 * led_driver.c
 *
 *  Created on: Sep 10, 2022
 *      Author: a6gue
 */
#include "led_driver.h"

const char* const led_msg[5] = {"\n=========================\n",
                                 "|                     LED Effect                    |\n" ,
                                 "=========================\n"  ,
                                 "(none, e1, e2, e3, e4)\n"      ,
                                 "Enter your choice here: "};

void ledTask(void *argument)
{
  /* USER CODE BEGIN ledTask */
  /* Infinite loop */
  for(;;)
  {
    ulTaskNotifyTake( pdTRUE, portMAX_DELAY);
    for(int i = 0; i < 5; ++i)
    {
      xQueueSendToBack(printQueueHandle, &led_msg[i], portMAX_DELAY);
    }
    // recv from queue the command
  }
  /* USER CODE END ledTask */
}
void stopTimers(void)
{
  for(int i = 0; i < NUM_TIMERS; ++i)
  {
	  osTimerStop(ledTimerHandleArr[i]);
  }
}

/* ledTimerCb function */
void ledTimerCb1(void *argument)
{
  /* USER CODE BEGIN ledTimerCb */
  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7);
  HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);
  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
  HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_13);
  /* USER CODE END ledTimerCb */
}

void ledTimerCb2(void *argument)
{
  //TODO: Make this into for loop.
  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7);
  HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);
  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
  HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_13);
}

void ledTimerCb3(void *argument)
{
  static int ledState = 1;

  if(ledState == 1)
  {
    ledSetAllLow();
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7);
    ++ledState;
  }
  else if(ledState == 2)
  {
	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7);
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);
	++ledState;
  }
  else if(ledState == 3)
  {
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);
	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
	++ledState;
  }
  else if(ledState == 4)
  {
	HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_13);
	ledState = 1;
  }
}

void ledTimerCb4(void *argument)
{
	  static int ledState = 4;

	  if(ledState == 1)
	  {
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);
	    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_7);
	    ledState = 4;
	  }
	  else if(ledState == 2)
	  {
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_14);
		--ledState;
	  }
	  else if(ledState == 3)
	  {
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_13);
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
		--ledState;
	  }
	  else if(ledState == 4)
	  {
		ledSetAllLow();
		HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_13);
		--ledState;
	  }
}

void ledSetAllLow(void)
{
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5|GPIO_PIN_7, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);
}
