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

#define MAX_LINIA 7
#define LEN_LINIA 40


char linia[MAX_LINIA][LEN_LINIA];
int top_linia = 0;


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
		strcpy(pcInsert, linia[i]);
		return strlen(linia[i]);
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

struct CanData_t {
	uint8_t to;						// kierunek transiski ; do kogo
	uint8_t fun;					// funkcja
	uint8_t	val;					// wartosc (H)
	uint8_t	valL;					// wartosc (L)
} CanTxData, CanRxData;
CAN_TxHeaderTypeDef pCanTxHeader, pCanRxHeader;
uint32_t TxMailbox;
CAN_FilterTypeDef Can_FilterConfig;
uint8_t id_device = 1;								// identyfikator urządzenia 1 - fabryczny


mqtt_client_t mqtt_client;


void CanConfig() {
    // Inicjacja magistrali CAN
      /*Na podstawie kodu z materiału https://www.youtube.com/watch?v=ymD3F0h-ilE&t=924s */
    pCanTxHeader.DLC = 3; // długość danych TxData/RxData
    pCanTxHeader.IDE = CAN_ID_STD;
    pCanTxHeader.RTR = CAN_RTR_DATA;
    pCanTxHeader.StdId = 0;

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

void Can_RX(){
    char s[80];

//    HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &pCanRxHeader, &CanRxData);

//    if (pCanRxHeader.ExtId || (pCanRxHeader.StdId != 0x100 + id_device)) {
//    	return;
//    }{
    sprintf(s, "can:%03x(%d) %d,%d,%d   ",pCanRxHeader.StdId, pCanRxHeader.DLC, CanRxData.to, CanRxData.fun, CanRxData.val);
    //ssd1306_SetCursor(0, 48);
    //ssd1306_WriteString(s,  Font_7x10, White);
    printLCD(s);

    if (pCanRxHeader.ExtId == 0x001E8041) {
    	uint16_t v = CanRxData.to + 256*CanRxData.fun;
    	char sv[10];
    	sprintf(sv,"%d",v);
  	   	mqtt_my_publish(&mqtt_client, "RekWenNaw", sv);
    }
}


static void mqtt_sub_request_cb(void *arg, err_t result)
{
  /* Just print the result code here for simplicity,
     normal behaviour would be to take some action if subscribe fails like
     notifying user, retry subscribe or disconnect from server */
  char s[30];
  sprintf(s, "Sub result: %d", result);
  printLCD(s);
}

static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
  char s[30];
  sprintf(s, "in: %s %u", topic, (unsigned int)tot_len);
  printLCD(s);
}

static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{

  char s1[40];
  char s2[40];
  strncpy(s1, data, len);//  sprintf(s,"data: %s", (const char *)data);
  s1[len] = '\0';
  sprintf(s2,"data: %s", s1);
  printLCD(s2);

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
    	printLCD(s);
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
	  printLCD(s);
  } else {
	  char s[30];
	  sprintf(s, "MQTT Connected");
	  printLCD(s);
	}
}


/* Called when publish is complete either with sucess or failure */
static void mqtt_pub_request_cb(void *arg, err_t result)
{
  if(result != ERR_OK) {
	  char s[30];
	  sprintf(s, "Publish result: %d\n", result);
	  printLCD(s);
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
    printLCD(s);
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
    printLCD(s);
  }
}



/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId myTaskButtonHandle;
osThreadId myTaskLCDHandle;
osThreadId myTaskCanHandle;
osMessageQId myQueue01Handle;
osSemaphoreId myBinarySemButtonHandle;
osSemaphoreId myBinarySemLCDHandle;
osSemaphoreId myBinarySemRXCanHandle;

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
    HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &pCanRxHeader, &CanRxData);
	osSemaphoreRelease(myBinarySemRXCanHandle);
}


/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartTaskButton(void const * argument);
void StartTaskLCD(void const * argument);
void StartTaskCan(void const * argument);

extern void MX_LWIP_Init(void);
extern void MX_USB_DEVICE_Init(void);
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

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of myBinarySemButton */
  osSemaphoreDef(myBinarySemButton);
  myBinarySemButtonHandle = osSemaphoreCreate(osSemaphore(myBinarySemButton), 1);

  /* definition and creation of myBinarySemLCD */
  osSemaphoreDef(myBinarySemLCD);
  myBinarySemLCDHandle = osSemaphoreCreate(osSemaphore(myBinarySemLCD), 1);

  /* definition and creation of myBinarySemRXCan */
  osSemaphoreDef(myBinarySemRXCan);
  myBinarySemRXCanHandle = osSemaphoreCreate(osSemaphore(myBinarySemRXCan), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of myQueue01 */
  osMessageQDef(myQueue01, 16, 32);
  myQueue01Handle = osMessageCreate(osMessageQ(myQueue01), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 512);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of myTaskButton */
  osThreadDef(myTaskButton, StartTaskButton, osPriorityNormal, 0, 512);
  myTaskButtonHandle = osThreadCreate(osThread(myTaskButton), NULL);

  /* definition and creation of myTaskLCD */
  osThreadDef(myTaskLCD, StartTaskLCD, osPriorityNormal, 0, 512);
  myTaskLCDHandle = osThreadCreate(osThread(myTaskLCD), NULL);

  /* definition and creation of myTaskCan */
  osThreadDef(myTaskCan, StartTaskCan, osPriorityHigh, 0, 512);
  myTaskCanHandle = osThreadCreate(osThread(myTaskCan), NULL);

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
  /* init code for LWIP */
  MX_LWIP_Init();

  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */


  //osDelay(2000);

  ssd1306_Init();
  printLCD("Init");

  extern struct netif gnetif;

   //u32_t  local_IP = netif_ip4_addr(&gnetif);


   //u32_t local_IP = lwIPLocalIPAddrGet(&gnetif);

   //u32_t local_IP = ip4_addr_get_u32(gnetif);
    //u32_t local_IP = netif_ip4_addr((&gnetif);


   u32_t local_IP = gnetif.ip_addr.addr;

  char s[50];

  sprintf(s, "IP %d.%d.%d.%d\n\r",(local_IP & 0xff), ((local_IP >> 8) & 0xff), ((local_IP >> 16) & 0xff), (local_IP >> 24));

  printLCD(s);



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
//	  example_publish(&mqtt_client, 0);
	  printf("Idle");
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
void StartTaskButton(void const * argument)
{
  /* USER CODE BEGIN StartTaskButton */
  /* Infinite loop */
  for(;;)
  {
	osSemaphoreWait(myBinarySemButtonHandle, osWaitForever);

	LD1ON = !LD1ON;


	HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, LD1ON);
	char s[20];
	int  i = sprintf(s, "Led2 %d", LD1ON);
	printLCD(s);
	mqtt_my_publish(&mqtt_client, "Led2", (char)LD1ON);

	//CDC_Transmit_FS(s,  i);

	//printf("");
	osDelay(1);

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
void StartTaskLCD(void const * argument)
{
  /* USER CODE BEGIN StartTaskLCD */
  /* Infinite loop */
  for(;;) {
	  osSemaphoreWait(myBinarySemLCDHandle, osWaitForever);

	  for (uint8_t i=0; i<MAX_LINIA ; i++){
		  ssd1306_SetCursor(0, 9*i);
		  char _s[LEN_LINIA];
		  strcpy(_s, linia[ILinia(i)]);
		  uint8_t l= strlen(_s);
		  memset(&_s[l], ' ', LEN_LINIA-l);
		  _s[20] = '\0';
		  ssd1306_WriteString(_s, Font_6x8, White);
	  }
	  ssd1306_UpdateScreen();
      osDelay(1);

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
void StartTaskCan(void const * argument)
{
  /* USER CODE BEGIN StartTaskCan */
  /* Infinite loop */

  CanConfig();
  for(;;)
  {
	osSemaphoreWait(myBinarySemRXCanHandle, osWaitForever);
   Can_RX();
   int i = 0;
   i++;

  }
  /* USER CODE END StartTaskCan */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

int ILinia(int lp)
{
	return (top_linia + lp) % MAX_LINIA;
}

void printLCD(const char *s)
{
	printf(s);
	strcpy(linia[top_linia], s);
	top_linia = (top_linia + 1) % MAX_LINIA;
	osSemaphoreRelease(myBinarySemLCDHandle);
}


/* USER CODE END Application */

