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

// including httpd.h [- HTTPd #1 -]
#include "lwip/apps/httpd.h"

// we include String.h for the strcmp() function [= CGI #1 =]
#include <String.h>
// we include this library to be able to use boolean variables for SSI
#include <Stdbool.h>

bool LD1ON = false; // this variable will indicate if the LD3 LED on the board is ON or not
bool LD2ON = false; // this variable will indicate if our LD2 LED on the board is ON or not

#include "mqtt.h"


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

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
#define numSSItags 2

// [* SSI #3 *]
char const *theSSItags[numSSItags] = { "tag1", "tag2" };

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

		if (strcmp(pcParam[i], "led") == 0)

		{

			if (strcmp(pcValue[i], "1") == 0) {

				HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);

				// LD3 LED (red) on the board is ON!
				LD1ON = true;

			}

			else if (strcmp(pcValue[i], "2") == 0) {

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
		}

		else if (LD1ON == true) {

			// since the LD3 red LED on the board is ON we make its checkbox checked!
			char myStr1[] =
			"<input value=\"1\" name=\"led\" type=\"checkbox\" checked>";
			strcpy(pcInsert, myStr1);

			return strlen(myStr1);

		}

	}

	else if (iIndex == 1)

	{
		if (LD2ON == false) {
			char myStr2[] = "<input value=\"2\" name=\"led\" type=\"checkbox\">";
			strcpy(pcInsert, myStr2);

			return strlen(myStr2);
		}

		else if (LD2ON == true) {

			// since the LD2 blue LED on the board is ON we make its checkbox checked!
			char myStr2[] =
			"<input value=\"2\" name=\"led\" type=\"checkbox\" checked>";
			strcpy(pcInsert, myStr2);

			return strlen(myStr2);

		}

	}

	return 0;

}

// function to initialize SSI [* SSI #5 *]
void mySSIinit(void) {

	http_set_ssi_handler(mySSIHandler, (char const**) theSSItags,
	numSSItags);
}

mqtt_client_t mqtt_client;

static void mqtt_sub_request_cb(void *arg, err_t result)
{
  /* Just print the result code here for simplicity,
     normal behaviour would be to take some action if subscribe fails like
     notifying user, retry subscribe or disconnect from server */
//  printf("Subscribe result: %d\n", result);
}

static int inpub_id;
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
//  printf("Incoming publish at topic %s with total length %u\n", topic, (unsigned int)tot_len);

  /* Decode topic string into a user defined reference */
  if(strcmp(topic, "print_payload") == 0) {
    inpub_id = 0;
  } else if(topic[0] == 'A') {
    /* All topics starting with 'A' might be handled at the same way */
    inpub_id = 1;
  } else {
    /* For all other topics */
    inpub_id = 2;
  }
}

static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
//  printf("Incoming publish payload with length %d, flags %u\n", len, (unsigned int)flags);

  if(flags & MQTT_DATA_FLAG_LAST) {
    /* Last fragment of payload received (or whole part if payload fits receive buffer
       See MQTT_VAR_HEADER_BUFFER_LEN)  */

    /* Call function or do action depending on reference, in this case inpub_id */
    if(inpub_id == 0) {
      /* Don't trust the publisher, check zero termination */
      if(data[len-1] == 0) {
//        printf("mqtt_incoming_data_cb: %s\n", (const char *)data);
      }
    } else if(inpub_id == 1) {
      /* Call an 'A' function... */
    } else {
//      printf("mqtt_incoming_data_cb: Ignoring payload...\n");
    }
  } else {
    /* Handle fragmented payload, store in buffer, write to file or whatever */
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
    err = mqtt_subscribe(client, "subtopic", 1, mqtt_sub_request_cb, arg);

    if(err != ERR_OK) {
//      printf("mqtt_subscribe return: %d\n", err);
    }
  } else {
//    printf("mqtt_connection_cb: Disconnected, reason: %d\n", status);

    /* Its more nice to be connected, so try to reconnect */
    example_do_connect(client);
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
  IP4_ADDR(&mqttServerIP, 192, 168, 1, 200);
//  err = mqtt_client_connect(client, &mqttServerIP, MQTT_PORT, mqtt_connection_cb, 0, &ci);
  err = mqtt_client_connect(client, &mqttServerIP, MQTT_PORT, mqtt_connection_cb, &topic, &ci);

  /* For now just print the result code if something goes wrong */
  if(err != ERR_OK) {
//    LCD_UsrLog("mqtt_connect return %d\n", err);
  }
	else
	{
//		LCD_UsrLog("Connected");
	}
}


/* Called when publish is complete either with sucess or failure */
static void mqtt_pub_request_cb(void *arg, err_t result)
{
  if(result != ERR_OK) {
//    printf("Publish result: %d\n", result);
  }
}


void example_publish(mqtt_client_t *client, void *arg)
{
  const char *pub_payload= "Test z NUCLEO-F207";
  err_t err;
  u8_t qos = 2; /* 0 1 or 2, see MQTT specification */
  u8_t retain = 0; /* No don't retain such crappy payload... */
  err = mqtt_publish(client, "DataSoft/stm32", pub_payload, strlen(pub_payload), qos, retain, mqtt_pub_request_cb, arg);
  if(err != ERR_OK) {
//    printf("Publish err: %d\n", err);
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

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);

extern void MX_LWIP_Init(void);
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

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 512);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

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
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */

  // initializing the HTTPd [-HTTPd #2-]
  httpd_init();

  // initializing CGI  [= CGI #7 =]
  myCGIinit();

  // initializing SSI [* SSI #6 *]
  mySSIinit();
  example_do_connect(&mqtt_client, 'DataSoft/stm32');

  for(;;)
  {
	  HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
	  example_publish(&mqtt_client, 0);
	  osDelay(1000);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
