/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32f2xx_hal.h"
#include "stdio.h"
#include "my_log.h"
#include "my_can.h"
#include "iwdg.h"
#include "mqtt.h"
#include "my_mqtt.h"
//#include "eeprom.h"
#include "httpd.h"
#include "my_httpd.h"

#include "ssd1306.h"
#include "netif.h"
#include "string.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
typedef StaticSemaphore_t osStaticSemaphoreDef_t;
/* USER CODE BEGIN PTD */

// ostatnia zajeta komorka FLASH w Buider Analizer to impure_data 0x0801ed98  96 bajtów (0x60)  = 1 EDF8

//VirtAddVarTab[NB_OF_VAR] = {0x5555, 0x6666, 0x7777};
//uint16_t resets = 0;


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
uint32_t defaultTaskBuffer[ 1024 ];
osStaticThreadDef_t defaultTaskControlBlock;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .cb_mem = &defaultTaskControlBlock,
  .cb_size = sizeof(defaultTaskControlBlock),
  .stack_mem = &defaultTaskBuffer[0],
  .stack_size = sizeof(defaultTaskBuffer),
  .priority = (osPriority_t) osPriorityNormal1,
};
/* Definitions for myTaskButton */
osThreadId_t myTaskButtonHandle;
uint32_t myTaskButtonBuffer[ 1024 ];
osStaticThreadDef_t myTaskButtonControlBlock;
const osThreadAttr_t myTaskButton_attributes = {
  .name = "myTaskButton",
  .cb_mem = &myTaskButtonControlBlock,
  .cb_size = sizeof(myTaskButtonControlBlock),
  .stack_mem = &myTaskButtonBuffer[0],
  .stack_size = sizeof(myTaskButtonBuffer),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for myTaskLCD */
osThreadId_t myTaskLCDHandle;
uint32_t myTaskLCDBuffer[ 1024 ];
osStaticThreadDef_t myTaskLCDControlBlock;
const osThreadAttr_t myTaskLCD_attributes = {
  .name = "myTaskLCD",
  .cb_mem = &myTaskLCDControlBlock,
  .cb_size = sizeof(myTaskLCDControlBlock),
  .stack_mem = &myTaskLCDBuffer[0],
  .stack_size = sizeof(myTaskLCDBuffer),
  .priority = (osPriority_t) osPriorityLow1,
};
/* Definitions for myTaskCan */
osThreadId_t myTaskCanHandle;
uint32_t myTaskCanBuffer[ 1024 ];
osStaticThreadDef_t myTaskCanControlBlock;
const osThreadAttr_t myTaskCan_attributes = {
  .name = "myTaskCan",
  .cb_mem = &myTaskCanControlBlock,
  .cb_size = sizeof(myTaskCanControlBlock),
  .stack_mem = &myTaskCanBuffer[0],
  .stack_size = sizeof(myTaskCanBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for QueueRxCan */
osMessageQueueId_t QueueRxCanHandle;
uint8_t QueueRxCanBuffer[ 32 * 32 ];
osStaticMessageQDef_t QueueRxCanControlBlock;
const osMessageQueueAttr_t QueueRxCan_attributes = {
  .name = "QueueRxCan",
  .cb_mem = &QueueRxCanControlBlock,
  .cb_size = sizeof(QueueRxCanControlBlock),
  .mq_mem = &QueueRxCanBuffer,
  .mq_size = sizeof(QueueRxCanBuffer)
};
/* Definitions for myBinarySemButton */
osSemaphoreId_t myBinarySemButtonHandle;
osStaticSemaphoreDef_t myBinarySemButtonControlBlock;
const osSemaphoreAttr_t myBinarySemButton_attributes = {
  .name = "myBinarySemButton",
  .cb_mem = &myBinarySemButtonControlBlock,
  .cb_size = sizeof(myBinarySemButtonControlBlock),
};
/* Definitions for myBinarySemLCD */
osSemaphoreId_t myBinarySemLCDHandle;
osStaticSemaphoreDef_t myBinarySemLCDControlBlock;
const osSemaphoreAttr_t myBinarySemLCD_attributes = {
  .name = "myBinarySemLCD",
  .cb_mem = &myBinarySemLCDControlBlock,
  .cb_size = sizeof(myBinarySemLCDControlBlock),
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == USER_Btn_Pin) {
		osSemaphoreRelease(myBinarySemButtonHandle);
	}
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
	HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &msg.RxHeader, &msg.RxData);
	osMessageQueuePut(QueueRxCanHandle, &msg, 0U, 0U);
}

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartTaskButton(void *argument);
void StartTaskLCD(void *argument);
void StartTaskCan(void *argument);

extern void MX_LWIP_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of myBinarySemButton */
  myBinarySemButtonHandle = osSemaphoreNew(1, 1, &myBinarySemButton_attributes);

  /* creation of myBinarySemLCD */
  myBinarySemLCDHandle = osSemaphoreNew(1, 1, &myBinarySemLCD_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of QueueRxCan */
  QueueRxCanHandle = osMessageQueueNew (32, 32, &QueueRxCan_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of myTaskButton */
  myTaskButtonHandle = osThreadNew(StartTaskButton, NULL, &myTaskButton_attributes);

  /* creation of myTaskLCD */
  myTaskLCDHandle = osThreadNew(StartTaskLCD, NULL, &myTaskLCD_attributes);

  /* creation of myTaskCan */
  myTaskCanHandle = osThreadNew(StartTaskCan, NULL, &myTaskCan_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* init code for LWIP */
  MX_LWIP_Init();
  /* USER CODE BEGIN StartDefaultTask */
	/* Infinite loop */

	//osDelay(2000);
	log_put("Init");

	extern struct netif gnetif;

	//u32_t  local_IP = netif_ip4_addr(&gnetif);

	//u32_t local_IP = lwIPLocalIPAddrGet(&gnetif);

	//u32_t local_IP = ip4_addr_get_u32(gnetif);
//	u32_t local_IP = netif_ip4_addr((&gnetif);

	u32_t local_IP = gnetif.ip_addr.addr;
	char s[50];
	sprintf(s, "IP %lu.%lu.%lu.%lu\n\r", (local_IP & 0xff), ((local_IP >> 8) & 0xff),
			((local_IP >> 16) & 0xff), (local_IP >> 24));
	log_put(s);

	// initializing the HTTPd [-HTTPd #2-]
	httpd_init();

	// initializing CGI  [= CGI #7 =]
	myCGIinit();

	// initializing SSI [* SSI #6 *]
	mySSIinit();

/*
  EE_Init();

  if(EE_ReadVariable(0, &resets) != HAL_OK) {
      Error_Handler();
  };

  resets++;

  if(EE_WriteVariable(0, &resets) != HAL_OK) {
        Error_Handler();
   };

*/

//  printf("Start");
	example_do_connect(&mqtt_client, "DataSoft/stm32");

	for (;;) {
		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
		set_stime();
		mqtt_my_publish(&mqtt_client, "licz", stime);

		log_put("");

		HAL_IWDG_Refresh(&hiwdg);
		osDelay(1000);
	}
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTaskButton */
/**
 * @brief Function implementing the myTaskButton thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTaskButton */
void StartTaskButton(void *argument)
{
  /* USER CODE BEGIN StartTaskButton */
	/* Infinite loop */
	for (;;) {
		osSemaphoreAcquire(myBinarySemButtonHandle, osWaitForever);
		LD1ON = !LD1ON;
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, LD1ON);
		char s[20];
		sprintf(s, "Led2 %d", LD1ON);
		log_put(s);

		osDelay(1);

		//mqtt_my_publish(&mqtt_client, "Led2", (char)LD1ON);

	}
  /* USER CODE END StartTaskButton */
}

/* USER CODE BEGIN Header_StartTaskLCD */
/**
 * @brief Function implementing the myTaskLCD thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTaskLCD */
void StartTaskLCD(void *argument)
{
  /* USER CODE BEGIN StartTaskLCD */
	/* Infinite loop */

//	ssd1306_Init();

	for (;;) {

		//osSemaphoreAcquire(myBinarySemLCDHandle, osWaitForever);

		for (uint8_t i = 0; i < LOG_MAX; i++) {
			ssd1306_SetCursor(0, 9 * i);
			char _s[LOG_LEN];
			strcpy(_s, log_items[ILinia(i)]);
			uint8_t l = strlen(_s);
			memset(&_s[l], ' ', LOG_LEN - l);
			_s[LOG_LEN] = '\0';
			ssd1306_WriteString(_s, Font_6x8, White);
		}
		ssd1306_UpdateScreen();

		osDelay(1000);
	}
  /* USER CODE END StartTaskLCD */
}

/* USER CODE BEGIN Header_StartTaskCan */
/**
 * @brief Function implementing the myTaskCan thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTaskCan */
void StartTaskCan(void *argument)
{
  /* USER CODE BEGIN StartTaskCan */
	/* Infinite loop */

//	CanConfig();

	osStatus_t status;

	for (;;) {
		//osSemaphoreWait(myBinarySemRXCanHandle, osWaitForever);
		//osSemaphoreAcquire(myBinarySemRXCanHandle, osWaitForever);

//		status = osMessageQueueGet(QueueRxCanHandle, &msg, NULL, 0U); // wait for message
//		if (status == osOK) {
//			Can_RX();
//		}
		osDelay(1);
	}
  /* USER CODE END StartTaskCan */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

