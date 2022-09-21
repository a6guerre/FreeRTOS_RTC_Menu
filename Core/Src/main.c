/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "queue.h"
#include "timers.h"
#include "led_driver.h"
#include "rtc_driver.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum
{
  INIT,
  TIME,
  DATE_CONFIG
}RTCMetaState;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

const osTimerAttr_t ledTimer_attributes[NUM_TIMERS] =
{
  {.name = "ledTimer1"},{.name = "ledTimer2"},{.name = "ledTimer3"}, {.name = "ledTimer4"}
};

const osTimerAttr_t rtc_timer_attr = {.name = "rtcReportingTimer"};

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;
state current_state = MAIN_MENU;
timerConfig timerConfigArr[NUM_TIMERS];

/* Definitions for printTask */
osThreadId_t printTaskHandle;
const osThreadAttr_t printTask_attributes = {
  .name = "printTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for menu_task */
osThreadId_t menu_taskHandle;
const osThreadAttr_t menu_task_attributes = {
  .name = "menu_task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for led_task */
osThreadId_t led_taskHandle;
const osThreadAttr_t led_task_attributes = {
  .name = "led_task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for commandHandling */
osThreadId_t commandHandlingHandle;
const osThreadAttr_t commandHandling_attributes = {
  .name = "commandHandling",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for rtc_task */
osThreadId_t rtc_taskHandle;
const osThreadAttr_t rtc_task_attributes = {
  .name = "rtc_task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for printQueue */
const osMessageQueueAttr_t printQueue_attributes = {
  .name = "printQueue"
};
/* Definitions for commandQueue */
osMessageQueueId_t commandQueueHandle;
const osMessageQueueAttr_t commandQueue_attributes = {
  .name = "commandQueue"
};
/* Definitions for myQueue03 */
const osMessageQueueAttr_t myQueue03_attributes = {
  .name = "rtcQueue"
};

const osTimerAttr_t ledTimer1_attributes = {
  .name = "ledTimer1"
};

osTimerId_t ledTimerHandle2;
const osTimerAttr_t ledTime2_attributes = {
  .name = "ledTimer2"
};

osTimerId_t ledTimerHandle3;
const osTimerAttr_t ledTime3_attributes = {
  .name = "ledTimer3"
};

osTimerId_t ledTimerHandle4;
const osTimerAttr_t ledTime4_attributes = {
  .name = "ledTimer4"
};

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_RTC_Init(void);

int setCommandTaskState(state newState);
void print_task(void *argument);
void menuTask(void *argument);
void ledTask(void *argument);
void commandTask(void *argument);
void rtcTask(void *argument);
void ledTimerCb1(void *argument);
void ledTimerCb2(void *argument);
void ledTimerCb3(void *argument);
void ledTimerCb4(void *argument);

/* USER CODE BEGIN PFP */
uint8_t data;
const char* const menu_msg[7] = {"\n=========================\n",
		             "|                       Menu                      |\n" ,
                     "=========================\n"  ,
                     "LED effect               ----> 0\n"      ,
                     "Date and time        ----> 1\n"      ,
                     "Exit                            ----> 2\n"      ,
                     "Enter your choice here: "     };

const char* const invalid_opt_str = "Invalid Option";
//make const chars
// Maybe make it array of char * and have task iterate the length of array until
// complete msg is sent

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    BaseType_t xHigherPriorityTaskWoken;

    if(uxQueueMessagesWaitingFromISR(commandQueueHandle) < 32)
    {
    	xQueueSendToBackFromISR(commandQueueHandle, &data, &xHigherPriorityTaskWoken);
    	if(data == '\n')
    	{
    		vTaskNotifyGiveFromISR(commandHandlingHandle, &xHigherPriorityTaskWoken);
    	}
    }
    else
    {
       vTaskNotifyGiveFromISR(commandHandlingHandle, &xHigherPriorityTaskWoken);
    }
    HAL_UART_Receive_IT(&huart2, &data, 1);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart2, &data, 1);
  //TODO: Init timer config strucutres
  osTimerFunc_t timerCbArr[NUM_TIMERS] = {ledTimerCb1, ledTimerCb2, ledTimerCb3, ledTimerCb4};
  for (int i = 0; i < NUM_TIMERS; ++i)
  {
	 timerConfigArr[i].timerCb =  timerCbArr[i];
	 timerConfigArr[i].mode = osTimerPeriodic;
  }
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* creation of ledTimer */
  //TODO: Make function out of this.
  for( int i = 0; i < sizeof(timerConfigArr)/sizeof(timerConfigArr[0]); ++i)
  {
    //ledTimerHandle = osTimerNew(ledTimerCb, osTimerPeriodic, NULL, &ledTimer_attributes);
	ledTimerHandleArr[i] = osTimerNew(timerConfigArr[i].timerCb, timerConfigArr[i].mode , NULL, &ledTimer_attributes[i]);
  }
  rtcTimerHandle = osTimerNew(rtcReportingCb, osTimerPeriodic, (void *)&hrtc, &rtc_timer_attr);
  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of printQueue */
  printQueueHandle = osMessageQueueNew (200, sizeof(char *), &printQueue_attributes);

  /* creation of commandQueue */
  commandQueueHandle = osMessageQueueNew (32, sizeof(uint8_t), &commandQueue_attributes);

  /* creation of myQueue03 */
  rtcQueueHandle = osMessageQueueNew (16, sizeof(uint16_t), &myQueue03_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of printTask */
  printTaskHandle = osThreadNew(print_task, NULL, &printTask_attributes);

  /* creation of menu_task */
  menu_taskHandle = osThreadNew(menuTask, NULL, &menu_task_attributes);

  /* creation of led_task */
  led_taskHandle = osThreadNew(ledTask, NULL, &led_task_attributes);

  /* creation of commandHandling */
  commandHandlingHandle = osThreadNew(commandTask, NULL, &commandHandling_attributes);

  /* creation of rtc_task */
  rtc_taskHandle = osThreadNew(rtcTask, NULL, &rtc_task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  RTC_TimeTypeDef s_time;
  s_time.Seconds = 0;
  s_time.Minutes = 0;
  s_time.Hours = 4;
  s_time.TimeFormat = 0;
  HAL_RTC_SetTime(&hrtc, &s_time, RTC_FORMAT_BIN);
// HAL_RTC_GetTime(&hrtc, &s_time, RTC_FORMAT_BIN);
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();
  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_OFF;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_12;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB12 PB13 PB14 PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

void startAllTimers(void)
{
  for(int i = 0; i < NUM_TIMERS; ++i)
  {
    osTimerStart(ledTimerHandleArr[i], pdMS_TO_TICKS(500));
  }
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_print_task */
/**
  * @brief  Function implementing the printTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_print_task */
void print_task(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  char *msg = (char *)malloc(200);
  for(;;)
  {
	xQueueReceive(printQueueHandle, &msg, portMAX_DELAY);
	int len = strlen(msg);
    HAL_UART_Transmit(&huart2, msg, len, 1000);
    osDelay(1);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_menuTask */
/**
* @brief Function implementing the menu_task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_menuTask */
void menuTask(void *argument)
{
  /* USER CODE BEGIN menuTask */
  /* Infinite loop */

  for(;;)
  {
    osDelay(1000);
    int i;
    for (i = 0; i < 7; ++i)
    {
      xQueueSendToBack(printQueueHandle, &menu_msg[i], portMAX_DELAY);
    }
    ulTaskNotifyTake( pdTRUE, portMAX_DELAY);
  }
  /* USER CODE END menuTask */
}

/* USER CODE BEGIN Header_ledTask */
/**
* @brief Function implementing the led_task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_ledTask */

void commandTaskRecv(char *recv_buffer)
{
	int i = 0;
	//TODO: (URGENT) Convert this to one function
	do
    {
	   xQueueReceive(commandQueueHandle, &recv_buffer[i], portMAX_DELAY);
	   ++i;
    }while(uxQueueMessagesWaiting(commandQueueHandle) > 0);

    recv_buffer[i - 1] = '\0';
}

int setCommandTaskState(state newState)
{
  int err;
  err = (newState >= MAIN_MENU && newState <= EXIT) ? 0 : -1;
  if (!err)
    current_state = newState;

  if(current_state == MAIN_MENU)
  {
    xTaskNotifyGive(menu_taskHandle);
  }

  return err;
}

/* USER CODE BEGIN Header_commandTask */
/**
* @brief Function implementing the commandHandling thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_commandTask */
void commandTask(void *argument)
{
  /* USER CODE BEGIN commandTask */
  /* Infinite loop */
  int str_len, idx;
  char recv_cmd[100];

  for(;;)
  {
    commandTaskRecv(recv_cmd);
	switch(current_state) {
	   case MAIN_MENU :
		   if(strcmp(recv_cmd, "0") == 0)
		   {
	         current_state = LED_MENU;
	         xTaskNotifyGive(led_taskHandle);
		   }
		   else if(strcmp(recv_cmd, "1") == 0)
		   {
             current_state = RTC_MENU;
             xTaskNotifyGive(rtc_taskHandle);
		   }
		   else if(strcmp(recv_cmd, "2") == 0)
		   {
             current_state = EXIT;
		   }
		   else
		   {
             xQueueSendToBack(printQueueHandle, &invalid_opt_str, portMAX_DELAY);
             xTaskNotifyGive(menu_taskHandle);
		   }
		   break;
	   case LED_MENU:
		   if(strcmp(recv_cmd, "none") == 0)
		   {
			     stopTimers();
			     ledSetAllLow();
		   }
		   else if(strcmp(recv_cmd, "e1") == 0)
		   {
			     stopTimers();
			     ledSetAllLow();
			     osTimerStart(ledTimerHandleArr[0], pdMS_TO_TICKS(500));
			     //TODO: COnsider moving this logic to LED Task
		   }
		   else if(strcmp(recv_cmd, "e2") == 0)
		   {
			     stopTimers();
			     ledSetAllLow();
			     HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14|GPIO_PIN_13, GPIO_PIN_SET);
			     osTimerStart(ledTimerHandleArr[1], pdMS_TO_TICKS(500));
		   }
		   else if(strcmp(recv_cmd, "e3") == 0)
		   {
			     stopTimers();
			     osTimerStart(ledTimerHandleArr[2], pdMS_TO_TICKS(500));
		   }
		   else if(strcmp(recv_cmd, "e4") == 0)
		   {
			     stopTimers();
			     osTimerStart(ledTimerHandleArr[3], pdMS_TO_TICKS(500));
		   }
		   else
		   {
		     xQueueSendToBack(printQueueHandle, &invalid_opt_str, portMAX_DELAY);
		   }
		   xTaskNotifyGive(menu_taskHandle);
		   current_state = MAIN_MENU;
		   break;
	   case RTC_MENU:
		   //TODO: Make this a function.
		   str_len = (int)strlen(recv_cmd);
		   for (idx = 0; idx < str_len; ++idx)
		   {
		     xQueueSendToBack(rtcQueueHandle, &recv_cmd[idx], portMAX_DELAY);
		   }
		   break;
	}
  }
  /* USER CODE END commandTask */
}

void recvMsgFromQueue(QueueHandle_t queueHandle, char *buf)
{
  int count = 0;
  do
  {
	//TODO: Error handling of return value
    xQueueReceive(queueHandle, &buf[count], portMAX_DELAY);
	++count;
  }while(uxQueueMessagesWaiting(queueHandle) > 0);
  buf[count] = '\0';
}
/* USER CODE BEGIN Header_rtcTask */
/**
* @brief Function implementing the rtc_task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_rtcTask */
void rtcTask(void *argument)
{
  /* USER CODE BEGIN rtcTask */
  /* Infinite loop */
  char *recv_cmd = malloc(30);
  for(;;)
  {
	  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
	  rtcPrintInitMessage(&hrtc);
      recvMsgFromQueue(rtcQueueHandle, recv_cmd);

	   if (strcmp(recv_cmd, "0") == 0)
	   {
         configureRTCTime(&hrtc);
	   }
	   else if (strcmp(recv_cmd, "1") == 0)
	   {
         configureRTCDate(&hrtc);
		 //setCommandTaskState(MAIN_MENU);
	   }
	   else if (strcmp(recv_cmd, "2") == 0)
	   {
         // Enable reporting
		 setCommandTaskState(MAIN_MENU);
		 osTimerStart(rtcTimerHandle, pdMS_TO_TICKS(4000));
	   }
	   else if (strcmp(recv_cmd, "3") == 0)
	   {
	     setCommandTaskState(MAIN_MENU);
	   }
	   else
	   {
		 xQueueSendToBack(printQueueHandle, &invalid_opt_str, portMAX_DELAY);
		 setCommandTaskState(MAIN_MENU);
	   }
  }
  /* USER CODE END rtcTask */
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
