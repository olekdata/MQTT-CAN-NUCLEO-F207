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
typedef StaticSemaphore_t osStaticMutexDef_t;
typedef StaticSemaphore_t osStaticSemaphoreDef_t;
/* USER CODE BEGIN PTD */

// ostatnia zajeta komorka FLASH w Buider Analizer to impure_data 0x0801ed98  96 bajtów (0x60)  = 1 EDF8

//VirtAddVarTab[NB_OF_VAR] = {0x5555, 0x6666, 0x7777};
//uint16_t resets = 0;

uint8_t LWIP_Init_flag = 0;

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
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for TaskButton */
osThreadId_t TaskButtonHandle;
uint32_t TaskButtonBuffer[ 1024 ];
osStaticThreadDef_t TaskButtonControlBlock;
const osThreadAttr_t TaskButton_attributes = {
  .name = "TaskButton",
  .cb_mem = &TaskButtonControlBlock,
  .cb_size = sizeof(TaskButtonControlBlock),
  .stack_mem = &TaskButtonBuffer[0],
  .stack_size = sizeof(TaskButtonBuffer),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for TaskLCD */
osThreadId_t TaskLCDHandle;
uint32_t TaskLCDBuffer[ 1024 ];
osStaticThreadDef_t TaskLCDControlBlock;
const osThreadAttr_t TaskLCD_attributes = {
  .name = "TaskLCD",
  .cb_mem = &TaskLCDControlBlock,
  .cb_size = sizeof(TaskLCDControlBlock),
  .stack_mem = &TaskLCDBuffer[0],
  .stack_size = sizeof(TaskLCDBuffer),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for TaskCan */
osThreadId_t TaskCanHandle;
uint32_t TaskCanBuffer[ 1024 ];
osStaticThreadDef_t TaskCanControlBlock;
const osThreadAttr_t TaskCan_attributes = {
  .name = "TaskCan",
  .cb_mem = &TaskCanControlBlock,
  .cb_size = sizeof(TaskCanControlBlock),
  .stack_mem = &TaskCanBuffer[0],
  .stack_size = sizeof(TaskCanBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for TaskMQTT */
osThreadId_t TaskMQTTHandle;
uint32_t TaskMQTTBuffer[ 1024 ];
osStaticThreadDef_t TaskMQTTControlBlock;
const osThreadAttr_t TaskMQTT_attributes = {
  .name = "TaskMQTT",
  .cb_mem = &TaskMQTTControlBlock,
  .cb_size = sizeof(TaskMQTTControlBlock),
  .stack_mem = &TaskMQTTBuffer[0],
  .stack_size = sizeof(TaskMQTTBuffer),
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
/* Definitions for QueueTxMqtt */
osMessageQueueId_t QueueTxMqttHandle;
uint8_t QueueTxMqttBuffer[ 16 * 100 ];
osStaticMessageQDef_t QueueTxMqttControlBlock;
const osMessageQueueAttr_t QueueTxMqtt_attributes = {
  .name = "QueueTxMqtt",
  .cb_mem = &QueueTxMqttControlBlock,
  .cb_size = sizeof(QueueTxMqttControlBlock),
  .mq_mem = &QueueTxMqttBuffer,
  .mq_size = sizeof(QueueTxMqttBuffer)
};
/* Definitions for QueueTxCan */
osMessageQueueId_t QueueTxCanHandle;
uint8_t QueueTxCanBuffer[ 32 * 32 ];
osStaticMessageQDef_t QueueTxCanControlBlock;
const osMessageQueueAttr_t QueueTxCan_attributes = {
  .name = "QueueTxCan",
  .cb_mem = &QueueTxCanControlBlock,
  .cb_size = sizeof(QueueTxCanControlBlock),
  .mq_mem = &QueueTxCanBuffer,
  .mq_size = sizeof(QueueTxCanBuffer)
};
/* Definitions for logMutex */
osMutexId_t logMutexHandle;
osStaticMutexDef_t logMutexControlBlock;
const osMutexAttr_t logMutex_attributes = {
  .name = "logMutex",
  .cb_mem = &logMutexControlBlock,
  .cb_size = sizeof(logMutexControlBlock),
};
/* Definitions for BinarySemButton */
osSemaphoreId_t BinarySemButtonHandle;
osStaticSemaphoreDef_t BinarySemButtonControlBlock;
const osSemaphoreAttr_t BinarySemButton_attributes = {
  .name = "BinarySemButton",
  .cb_mem = &BinarySemButtonControlBlock,
  .cb_size = sizeof(BinarySemButtonControlBlock),
};
/* Definitions for BinarySemLCD */
osSemaphoreId_t BinarySemLCDHandle;
osStaticSemaphoreDef_t BinarySemLCDControlBlock;
const osSemaphoreAttr_t BinarySemLCD_attributes = {
  .name = "BinarySemLCD",
  .cb_mem = &BinarySemLCDControlBlock,
  .cb_size = sizeof(BinarySemLCDControlBlock),
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == USER_Btn_Pin) {
		osSemaphoreRelease(BinarySemButtonHandle);
	}
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
	MsgQRxCan_t msg_can;
	osStatus_t status;
	HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &msg_can.RxHeader, &msg_can.RxData);
	status = osMessageQueuePut(QueueRxCanHandle, &msg_can, 0U, 0U);
	if (status != osOK) {
		 char s[LOG_LEN];
		 sprintf(s, "QPut can err %d\n", status);
		 log_put(s);
	}
}

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartTaskButton(void *argument);
void StartTaskLCD(void *argument);
void StartTaskCan(void *argument);
void StartTaskMQTT(void *argument);

extern void MX_LWIP_Init(void);
extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void)
{

}

__weak unsigned long getRunTimeCounterValue(void)
{
return 0;
}
/* USER CODE END 1 */

/* USER CODE BEGIN 4 */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
}
/* USER CODE END 4 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of logMutex */
  logMutexHandle = osMutexNew(&logMutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of BinarySemButton */
  BinarySemButtonHandle = osSemaphoreNew(1, 1, &BinarySemButton_attributes);

  /* creation of BinarySemLCD */
  BinarySemLCDHandle = osSemaphoreNew(1, 1, &BinarySemLCD_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of QueueRxCan */
  QueueRxCanHandle = osMessageQueueNew (32, 32, &QueueRxCan_attributes);

  /* creation of QueueTxMqtt */
  QueueTxMqttHandle = osMessageQueueNew (16, 100, &QueueTxMqtt_attributes);

  /* creation of QueueTxCan */
  QueueTxCanHandle = osMessageQueueNew (32, 32, &QueueTxCan_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of TaskButton */
  TaskButtonHandle = osThreadNew(StartTaskButton, NULL, &TaskButton_attributes);

  /* creation of TaskLCD */
  TaskLCDHandle = osThreadNew(StartTaskLCD, NULL, &TaskLCD_attributes);

  /* creation of TaskCan */
  TaskCanHandle = osThreadNew(StartTaskCan, NULL, &TaskCan_attributes);

  /* creation of TaskMQTT */
  TaskMQTTHandle = osThreadNew(StartTaskMQTT, NULL, &TaskMQTT_attributes);

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

  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN StartDefaultTask */
	/* Infinite loop */
  LWIP_Init_flag = 1;
	//osDelay(2000);
	log_put("Init");


/*deb*/
	extern struct netif gnetif;
	u32_t local_IP = gnetif.ip_addr.addr;
	char s[LOG_LEN];
	sprintf(s, "IP %lu.%lu.%lu.%lu\n\r", (local_IP & 0xff), ((local_IP >> 8) & 0xff),
			((local_IP >> 16) & 0xff), (local_IP >> 24));
	log_put(s);
/**/

//deb

	httpd_init();
	myCGIinit();
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

//	stats_display();

	for (;;) {
		HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
		set_stime();
/*deb*/
   my_mqtt_to_Queue("stm32/licz", slicz);
   my_mqtt_to_Queue("stm32/czas", stime);
/**/
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
		osSemaphoreAcquire(BinarySemButtonHandle, osWaitForever);
		LD1ON = !LD1ON;
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, LD1ON);
		char s[LOG_LEN];
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

//deb
	ssd1306_Init();

	for (;;) {

//		osSemaphoreAcquire(myBinarySemLCDHandle, osWaitForever);

/*deb*/
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
/* */
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

	//deb

	while(1) {
		if (LWIP_Init_flag)
		  break;
		osDelay(10);
	}


	CanConfig();

	osStatus_t status;
	MsgQRxCan_t msg_RxCan;
	MsgQTxCan_t msg_TxCan;


	for (;;) {
		status = osMessageQueueGet(QueueRxCanHandle, &msg_RxCan, NULL, 0);
		if (status == osOK) {
			Can_RX(&msg_RxCan);
		}

		status = osMessageQueueGet(QueueTxCanHandle, &msg_TxCan, NULL, 0);
		if (status == osOK) {
			my_can_Tx(&msg_TxCan);
		}

		osDelay(1);
	}
  /* USER CODE END StartTaskCan */
}

/* USER CODE BEGIN Header_StartTaskMQTT */
/**
* @brief Function implementing the TaskMQTT thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTaskMQTT */
void StartTaskMQTT(void *argument)
{
  /* USER CODE BEGIN StartTaskMQTT */



//	extern struct netif gnetif;

	while(1) {
		if (LWIP_Init_flag)
		  break;
		osDelay(10);
	}

	my_mqtt_do_connect(&mqtt_client);


	osStatus_t status;
	mqtt_msg__t mqtt_msg;


  /* Infinite loop */
  for(;;)
  {
  	if (mqtt_client_is_connected(&mqtt_client)){

			status = osMessageQueueGet(QueueTxMqttHandle, &mqtt_msg, NULL, 0U); // wait for message
			if (status == osOK)
				my_mqtt_publish(&mqtt_client, mqtt_msg.topic, mqtt_msg.value);
		} else {
			log_put("mqtt not cone.");
 // 		my_mqtt_do_connect(&mqtt_client);
		}
    osDelay(1);
  }
  /* USER CODE END StartTaskMQTT */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

