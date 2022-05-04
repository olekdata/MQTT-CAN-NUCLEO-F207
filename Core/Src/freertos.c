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

#include "stdio.h"
#include "can.h"
#include "iwdg.h"

// including httpd.h [- HTTPd #1 -]
#include "lwip/apps/httpd.h"

// we include String.h for the strcmp() function [= CGI #1 =]
#include <string.h>
// we include this library to be able to use boolean variables for SSI
#include <stdbool.h>

#include "mqtt.h"


#include "ssd1306.h"
#include "netif.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */


bool LD1ON = false; // this variable will indicate if the LD3 LED on the board is ON or not
bool LD2ON = false; // this variable will indicate if our LD2 LED on the board is ON or not

#define LOG_MAX 6
#define LOG_LEN 40

char log_items[LOG_MAX][LOG_LEN];
int log_item = 0;

char stime[10];


mqtt_client_t mqtt_client;

void mqtt_my_publish(mqtt_client_t *client, const char *t, const char *m);


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// just declaring the function for the compiler [= CGI #2 =]
const char* LedCGIhandler(int iIndex, int iNumParams, char *pcParam[],
char *pcValue[]);

// in our SHTML file <form method="get" action="/leds.cgi"> [= CGI #3 =]
const tCGI LedCGI = { "/leds.cgi", LedCGIhandler };

// [= CGI #4 =]
tCGI theCGItable[1];

// just declaring SSI handler function [* SSI #1 *]
u16_t mySSIHandler(int iIndex, char *pcInsert, int iInsertLen);

// [* SSI #2 *]
#define numSSItags 9

// [* SSI #3 *]
char const *theSSItags[numSSItags] = { "tag1", "tag2", "w1", "w2","w3","w4","w5","w6","w7"};

// the actual function for handling CGI [= CGI #5 =]
const char* LedCGIhandler(int iIndex, int iNumParams, char *pcParam[],
char *pcValue[]) {
	uint32_t i = 0;
	if (iIndex == 0) {
		//turning the LED lights off
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
		// we put this variable to false to indicate that the LD2 LED on the board is not ON
		LD2ON = false;
		HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
		// we put this variable to false to indicate that the LD* LED on the board is not ON
		LD1ON = false;
		//mqtt_my_publish(&mqtt_client, "Led1", (char)LD1ON);
	}
	for (i = 0; i < iNumParams; i++) {
		if (strcmp(pcParam[i], "led") == 0) {
			if (strcmp(pcValue[i], "1") == 0) {
				HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
				// LD3 LED (red) on the board is ON!
				LD1ON = true;
			} else if (strcmp(pcValue[i], "2") == 0) {
				HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
				// LD2 LED (blue) on the board is ON!
				LD2ON = true;
			}
		}
	}
	// the extension .shtml for SSI to work
	return "/index.shtml";
} // END [= CGI #5 =]

// function to initialize CGI [= CGI #6 =]
void myCGIinit(void) {
	//add LED control CGI to the table
	theCGItable[0] = LedCGI;
	//give the table to the HTTP server
	http_set_cgi_handlers(theCGItable, 1);
} // END [= CGI #6 =]

// the actual function for SSI [* SSI #4 *]
u16_t mySSIHandler(int iIndex, char *pcInsert, int iInsertLen) {
	if (iIndex == 0) {
		if (LD1ON == false) {
			char myStr1[] = "<input value=\"1\" name=\"led\" type=\"checkbox\">";
			strcpy(pcInsert, myStr1);
			return strlen(myStr1);
		} else if (LD1ON == true) {
			// since the LD3 red LED on the board is ON we make its checkbox checked!
			char myStr1[] = "<input value=\"1\" name=\"led\" type=\"checkbox\" checked>";
			strcpy(pcInsert, myStr1);
			return strlen(myStr1);
		}
	} else if (iIndex == 1) {
		if (LD2ON == false) {
			char myStr2[] = "<input value=\"2\" name=\"led\" type=\"checkbox\">";
			strcpy(pcInsert, myStr2);
			return strlen(myStr2);
		} else if (LD2ON == true) {
			// since the LD2 blue LED on the board is ON we make its checkbox checked!
			char myStr2[] =  "<input value=\"2\" name=\"led\" type=\"checkbox\" checked>";
			strcpy(pcInsert, myStr2);
			return strlen(myStr2);
		}
	} else if (iIndex <10 ) {
		uint8_t i = ILinia(iIndex - 2);
		strcpy(pcInsert, log_items[i]);
		return strlen(log_items[i]);
	}
	return 0;
}

// function to initialize SSI [* SSI #5 *]
void mySSIinit(void) {

	http_set_ssi_handler(mySSIHandler, (char const**) theSSItags,
	numSSItags);
}


// ramka CAN

//CAN_HandleTypeDef hcan;

typedef struct {
	uint8_t to;						// kierunek transiski ; do kogo
	uint8_t fun;					// funkcja
	uint8_t	val;					// wartosc (H)
	uint8_t	valL;					// wartosc (L)
} CanData_t;//CanTxData, CanRxData;
//CAN_TxHeaderTypeDef pCanTxHeader, pCanRxHeader;
uint32_t TxMailbox;
CAN_FilterTypeDef Can_FilterConfig;
uint8_t id_device = 1;								// identyfikator urządzenia 1 - fabryczny

typedef struct  {
	CAN_RxHeaderTypeDef RxHeader; // 7 x uint32_t = 28
	CanData_t RxData;							// 4 x uint8_t = 4
} MsgQRxCan_t;  														// size = 32 bajty

MsgQRxCan_t msg;


void CanConfig() {
    // Inicjacja magistrali CAN
      /*Na podstawie kodu z materiału https://www.youtube.com/watch?v=ymD3F0h-ilE&t=924s */



    //pCanTxHeader.DLC = 3; // długość danych TxData/RxData
    //pCanTxHeader.IDE = CAN_ID_STD;
    //pCanTxHeader.RTR = CAN_RTR_DATA;
    //pCanTxHeader.StdId = 0;

/*
      canfil.FilterBank = 0;
      canfil.FilterMode = CAN_FILTERMODE_IDMASK;
      canfil.FilterFIFOAssignment = CAN_RX_FIFO0;
      canfil.FilterIdHigh = 0;
      canfil.FilterIdLow = 0;
      canfil.FilterMaskIdHigh = 0;
      canfil.FilterMaskIdLow = 0;
      canfil.FilterScale = CAN_FILTERSCALE_32BIT;
      canfil.FilterActivation = ENABLE;
      canfil.SlaveStartFilterBank = 14
*/

    Can_FilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
    Can_FilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    Can_FilterConfig.FilterScale = CAN_FILTERSCALE_16BIT;
    Can_FilterConfig.FilterActivation=CAN_FILTER_ENABLE;
    Can_FilterConfig.FilterIdHigh = 0;
    Can_FilterConfig.FilterIdLow = 0;
    Can_FilterConfig.FilterMaskIdHigh = 0; 					//0xFFFF;
    Can_FilterConfig.FilterMaskIdLow = 0; 					//0xFFFF;
    Can_FilterConfig.SlaveStartFilterBank = 14;
    Can_FilterConfig.FilterBank = 0;

    HAL_CAN_ConfigFilter(&hcan1, &Can_FilterConfig);
    HAL_CAN_Start(&hcan1);
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
}

void Can_RX(MsgQRxCan_t msg2){
//    HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &pCanRxHeader, &CanRxData);

//    if (pCanRxHeader.ExtId || (pCanRxHeader.StdId != 0x100 + id_device)) {
//    	return;
//    }{
    //sprintf(s, "can:%03x(%d) %d,%d,%d   ",pCanRxHeader.StdId, pCanRxHeader.DLC, CanRxData.to, CanRxData.fun, CanRxData.val);
    //ssd1306_SetCursor(0, 48);
    //ssd1306_WriteString(s,  Font_7x10, White);
    //log_put(s);

	uint16_t v1;
	char t[20];
	char s1[40];
	v1 = msg.RxData.to + 256*msg.RxData.fun;
 	sprintf(s1,"%d",v1);

 	int16_t v2;
	char s2[40];
 	v2 = msg.RxData.fun-msg.RxData.to;
 	sprintf(s2,"%d,%d",v2/10,v2%10);


	switch (msg.RxHeader.ExtId)
    {

     case 0x448041:
  	   	mqtt_my_publish(&mqtt_client, "wywiew_temp", s2);
  	   	break;

      case 0x488041:
        mqtt_my_publish(&mqtt_client, "wywiew_wilg", s2);
        break;

      case 0x44c041:
        mqtt_my_publish(&mqtt_client, "wyrzut_temp", s2);
        break;

      case 0x48C041:
        mqtt_my_publish(&mqtt_client, "wyrzut_wilg", s2);
        break;

      case 0x370041:
  	   	mqtt_my_publish(&mqtt_client, "czerpnia_temp", s2);
  	   	break;

      case 0x490041:
  	   	mqtt_my_publish(&mqtt_client, "czerpnia_wilg", s2);
  	   	break;

      case 0x458041:
  	   	mqtt_my_publish(&mqtt_client, "nawiew_temp", s2);
  	   	break;

      case 0x498041:
  	   	mqtt_my_publish(&mqtt_client, "nawiew_wilg", s2);
  	   	break;

      case 0x1DC041:
  	   	mqtt_my_publish(&mqtt_client, "wydajnosc_wyw", s1);
  	   	break;

      case 0x1E0041:
  	   	mqtt_my_publish(&mqtt_client, "wydajnosc_naw", s1);
  	   	break;

      case 0x1E4041:
  	   	mqtt_my_publish(&mqtt_client, "wywiew_wen", s1);
  	   	break;

      case 0x1E8041:
  	   	mqtt_my_publish(&mqtt_client, "nawiew_wen", s1);
  	   	break;

      case 0x200041:
  	   	mqtt_my_publish(&mqtt_client, "energia_curent", s1);
  	   	break;

      case 0x204041:
  	   	mqtt_my_publish(&mqtt_client, "energia_total_year", s1);
  	   	break;

      case 0x208041:
  	   	mqtt_my_publish(&mqtt_client, "energia_total", s1);
  	   	break;

      case 0x358041:
  	   	mqtt_my_publish(&mqtt_client, "energia_odzyskana_year", s1);
  	   	break;

      case 0x35C041:
  	   	mqtt_my_publish(&mqtt_client, "energia_odzyskana_total", s1);
  	   	break;

      default :
      	sprintf(t,"ExtID=%x", msg.RxHeader.ExtId);
      	sprintf(s1,"%d %d %d %d", msg.RxData.fun, msg.RxData.to, msg.RxData.val, msg.RxData.valL);
    	  mqtt_my_publish(&mqtt_client, t, s1);
    	  break;
    }
}


static void mqtt_sub_request_cb(void *arg, err_t result)
{
  /* Just print the result code here for simplicity,
     normal behaviour would be to take some action if subscribe fails like
     notifying user, retry subscribe or disconnect from server */
  char s[30];
  sprintf(s, "Sub result: %d", result);
  log_put(s);
}

static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
  char s[30];
//  sprintf(s, "in: %s %u", topic, (unsigned int)tot_len);
//  log_put(s);
}

static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{

  char s1[40];
  char s2[40];
  strncpy(s1, data, len);//  sprintf(s,"data: %s", (const char *)data);
  s1[len] = '\0';
//  sprintf(s2,"data: %s", s1);
//  log_put(s2);

  if(flags & MQTT_DATA_FLAG_LAST) {
    /* Last fragment of payload received (or whole part if payload fits receive buffer
       See MQTT_VAR_HEADER_BUFFER_LEN)  */

    /* Call function or do action depending on reference, in this case inpub_id */
  }
}



static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
  err_t err;
  if(status == MQTT_CONNECT_ACCEPTED) {
//    printf("mqtt_connection_cb: Successfully connected\n");

    /* Setup callback for incoming publish requests */
    mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, arg);

    /* Subscribe to a topic named "subtopic" with QoS level 1, call mqtt_sub_request_cb with result */
    err = mqtt_subscribe(client, "DataSoft/stm32/#", 1, mqtt_sub_request_cb, arg);

    if(err != ERR_OK) {
    	char s[30];
		sprintf(s, "m_sub ret: %d", err);
    	log_put(s);
    }
  } else {
//    printf("mqtt_connection_cb: Disconnected, reason: %d\n", status);

    /* Its more nice to be connected, so try to reconnect */
    example_do_connect(client, "DataSoft/stm32");
  }
}



void example_do_connect(mqtt_client_t *client, const char *topic)
{
  struct mqtt_connect_client_info_t ci;
  err_t err;

  /* Setup an empty client info structure */
  memset(&ci, 0, sizeof(ci));

  /* Minimal amount of information required is client identifier, so set it here */
  ci.client_id = "stm32";
  //ci.client_user = "mosquitto";
  //ci.client_pass = "chupasangre"; /* Tiempo en mi caso */


  /* Initiate client and connect to server, if this fails immediately an error code is returned
     otherwise mqtt_connection_cb will be called with connection result after attempting
     to establish a connection with the server.
     For now MQTT version 3.1.1 is always used */
  ip_addr_t mqttServerIP;
  IP4_ADDR(&mqttServerIP, 192, 168, 1, 2);
//  err = mqtt_client_connect(client, &mqttServerIP, MQTT_PORT, mqtt_connection_cb, 0, &ci);
  err = mqtt_client_connect(client, &mqttServerIP, MQTT_PORT, mqtt_connection_cb, &topic, &ci);

  /* For now just print the result code if something goes wrong */
  if(err != ERR_OK) {
	  char s[30];
      sprintf(s, "MQTT Con. error %d", err);
	  log_put(s);
  } else {
	  char s[30];
	  sprintf(s, "MQTT Connected");
	  log_put(s);
	}
}


/* Called when publish is complete either with sucess or failure */
static void mqtt_pub_request_cb(void *arg, err_t result)
{
  if(result != ERR_OK) {
	  char s[30];
	  sprintf(s, "Publish result: %d\n", result);
	  log_put(s);
  }
}


/*
void example_publish(mqtt_client_t *client, void *arg)
{
  const char *pub_payload= "Test z NUCLEO-F207";
  err_t err;
  u8_t qos = 0; //* 0 1 or 2, see MQTT specification
  u8_t retain = 0; // No don't retain such crappy payload...
  err = mqtt_publish(client, "DataSoft/stm32", pub_payload, strlen(pub_payload), qos, retain, mqtt_pub_request_cb, arg);
  if(err != ERR_OK) {
	char s[20];
    sprintf(s, "Publish err: %d\n", err);
    log_put(s);
  }
}
*/

void mqtt_my_publish(mqtt_client_t *client, const char *t, const char *m)
{
  err_t err;
  u8_t qos = 2; // 0 1 or 2, see MQTT specification
  u8_t retain = 0; // No don't retain such crappy payload...
  char topic[30];
  sprintf(topic, "DataSoft/stm32/%s", t);

  err = mqtt_publish(client, topic, m, strlen(m), qos, retain, mqtt_pub_request_cb, 0);
  if(err != ERR_OK) {
	char s[20];
    sprintf(s, "Publish err: %d\n", err);
    log_put(s);
  }
}



/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for myTaskButton */
osThreadId_t myTaskButtonHandle;
const osThreadAttr_t myTaskButton_attributes = {
  .name = "myTaskButton",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for myTaskLCD */
osThreadId_t myTaskLCDHandle;
const osThreadAttr_t myTaskLCD_attributes = {
  .name = "myTaskLCD",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for myTaskCan */
osThreadId_t myTaskCanHandle;
const osThreadAttr_t myTaskCan_attributes = {
  .name = "myTaskCan",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for QueueRxCan */
osMessageQueueId_t QueueRxCanHandle;
const osMessageQueueAttr_t QueueRxCan_attributes = {
  .name = "QueueRxCan"
};
/* Definitions for myBinarySemButton */
osSemaphoreId_t myBinarySemButtonHandle;
const osSemaphoreAttr_t myBinarySemButton_attributes = {
  .name = "myBinarySemButton"
};
/* Definitions for myBinarySemLCD */
osSemaphoreId_t myBinarySemLCDHandle;
const osSemaphoreAttr_t myBinarySemLCD_attributes = {
  .name = "myBinarySemLCD"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == USER_Btn_Pin) {
		osSemaphoreRelease(myBinarySemButtonHandle);
	}
}


void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{

  HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &msg.RxHeader, &msg.RxData);
  osMessageQueuePut(QueueRxCanHandle, &msg, 0U, 0U);

  //osSemaphoreRelease(myBinarySemRXCanHandle);
}


/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartTaskButton(void *argument);
void StartTaskLCD(void *argument);
void StartTaskCan(void *argument);

extern void MX_LWIP_Init(void);
extern void MX_USB_DEVICE_Init(void);
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

  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */


  //osDelay(2000);

  log_put("Init");

  extern struct netif gnetif;

   //u32_t  local_IP = netif_ip4_addr(&gnetif);


   //u32_t local_IP = lwIPLocalIPAddrGet(&gnetif);

   //u32_t local_IP = ip4_addr_get_u32(gnetif);
    //u32_t local_IP = netif_ip4_addr((&gnetif);


   u32_t local_IP = gnetif.ip_addr.addr;

  char s[50];

  sprintf(s, "IP %d.%d.%d.%d\n\r",(local_IP & 0xff), ((local_IP >> 8) & 0xff), ((local_IP >> 16) & 0xff), (local_IP >> 24));

  log_put(s);



  // initializing the HTTPd [-HTTPd #2-]
  httpd_init();

  // initializing CGI  [= CGI #7 =]
  myCGIinit();

  // initializing SSI [* SSI #6 *]
  mySSIinit();

  printf("Start");
  example_do_connect(&mqtt_client, "DataSoft/stm32");


  for(;;)
  {
	  HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
  	uint32_t t = osKernelGetTickCount() /  osKernelGetTickFreq();
  	uint8_t h = t /( 60 * 60 );
  	uint8_t m = (t / 60) % 60;
  	uint8_t s = t % 60;
  	sprintf(stime, "%d:%02d:%02d", h,m,s);

  	if (mqtt_client.conn_state == MQTT_CONNECT_REFUSED_SERVER) {
  	  //example_do_connect(&mqtt_client, "DataSoft/stm32");
  	}

  	HAL_IWDG_Refresh(&hiwdg);
	  osDelay(500);
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
  for(;;)
  {
  	osSemaphoreAcquire(myBinarySemButtonHandle, osWaitForever);
  	LD1ON = !LD1ON;
  	HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, LD1ON);
  	char s[20];
  	int  i = sprintf(s, "Led2 %d", LD1ON);
  	log_put(s);
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

  ssd1306_Init();

  for(;;) {
	  //osSemaphoreWait(myBinarySemLCDHandle, osWaitForever);
	  osSemaphoreAcquire(myBinarySemLCDHandle, osWaitForever);

	  for (uint8_t i=0; i<LOG_MAX ; i++){
		  ssd1306_SetCursor(0, 9*i);
		  char _s[LOG_LEN];
		  strcpy(_s, log_items[ILinia(i)]);
		  uint8_t l= strlen(_s);
		  memset(&_s[l], ' ', LOG_LEN-l);
		  _s[20] = '\0';
		  ssd1306_WriteString(_s, Font_6x8, White);
	  }
	  ssd1306_UpdateScreen();

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

  CanConfig();

  osStatus_t status;

  for(;;)
  {
	//osSemaphoreWait(myBinarySemRXCanHandle, osWaitForever);
	//osSemaphoreAcquire(myBinarySemRXCanHandle, osWaitForever);

  	status = osMessageQueueGet(QueueRxCanHandle, &msg, NULL, 0U);   // wait for message
  	if (status == osOK) {
  		Can_RX(msg);
    }
    osDelay(1);
  }
  /* USER CODE END StartTaskCan */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

int ILinia(int lp)
{
	return (log_item + lp) % LOG_MAX;
}

void log_put(const char *s)
{
	char sl[LOG_LEN];
	sprintf(sl, "%s-%s", stime, s);
	strcpy(log_items[log_item], sl);
	log_item = (log_item + 1) % LOG_MAX;
	osSemaphoreRelease( myBinarySemLCDHandle);
	printf(sl);
}



/* USER CODE END Application */

