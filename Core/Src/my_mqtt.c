/*
 * my_mqtt.c
 *
 *  Created on: May 5, 2022
 *      Author: olek
 */

#include "my_mqtt.h"
#include "mqtt.h"
#include <string.h>
#include "cmsis_os2.h"
#include "my_log.h"


extern osMessageQueueId_t QueueTxMqttHandle;


mqtt_msg__t mqtt_msg;

mqtt_client_t mqtt_client;

static void mqtt_sub_request_cb(void *arg, err_t result)
{
  /* Just print the result code here for simplicity,
     normal behaviour would be to take some action if subscribe fails like
     notifying user, retry subscribe or disconnect from server */
  char s[LOG_LEN];
  sprintf(s, "Sub result: %d", result);
  log_put(s);
}

static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
//  char s[LOG_LEN];
//  sprintf(s, "in: %s %u", topic, (unsigned int)tot_len);
//  log_put(s);
}


static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{

//  char s[LOG_LEN];
//  strncpy(s, data, len);//  sprintf(s,"data: %s", (const char *)data);
//  s[len] = '\0';
//  sprintf(s2,"data: %s", s1);
//  log_put(s);

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
/*
    if(err != ERR_OK) {
    	char s[LOG_LEN];
	  	sprintf(s, "m_sub ret: %d", err);
      log_put(s);
    }
*/
    } else {
//    printf("mqtt_connection_cb: Disconnected, reason: %d\n", status);

    /* Its more nice to be connected, so try to reconnect */
      my_mqtt_do_connect(client);
    }
}



void my_mqtt_do_connect(mqtt_client_t *client)
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
  err = mqtt_client_connect(client, &mqttServerIP, MQTT_PORT, mqtt_connection_cb, NULL, &ci);

  /* For now just print the result code if something goes wrong */
  if(err != ERR_OK) {
	  char s[LOG_LEN];
      sprintf(s, "MQTT Con. error %d", err);
	  log_put(s);
  } else {
	  char s[LOG_LEN];
	  sprintf(s, "MQTT Connected");
	  log_put(s);
	}
}


/* Called when publish is complete either with sucess or failure */
static void mqtt_pub_request_cb(void *arg, err_t result)
{
  if(result != ERR_OK) {
	  char s[LOG_LEN];
	  sprintf(s, "Pub. result: %d\n", result);
	  log_put(s);
  }
}


//DataSoft/stm32/energia_odzyskana_total
//1234567890123456789012345678901234567890


void my_mqtt_publish(mqtt_client_t *client, const char *t, const char *m)
{
	if (! mqtt_client_is_connected(client)){
		printf("mqtt not connected\n");
		return;
	}
  err_t err;
  u8_t qos = 0; // 0 - bez potwierdzenia odbioru
  u8_t retain = 0; // No don't retain such crappy payload...
  char topic[50];
  sprintf(topic, "DataSoft/stm32/%s", t);

  err = mqtt_publish(client, topic, m, strlen(m), qos, retain, mqtt_pub_request_cb, 0);
  if(err != ERR_OK) {
	  char s[LOG_LEN];
    sprintf(s, "Pub. err: %d\n", err);
    log_put(s);
    osDelay(1000);
    HAL_NVIC_SystemReset();
    while (1) {}; // reset przez wotch dog
  }
}


void my_mqtt_to_Queue(const char *t, const char *m){

	osStatus_t status;

	strcpy(mqtt_msg.topic, t);
	strcpy(mqtt_msg.value, m);

	status = osMessageQueuePut(QueueTxMqttHandle, &mqtt_msg, 0U, 0U);
	if (status != osOK) {
		 char s[LOG_LEN];
		 sprintf(s, "QPut mqtt err %d\n", status);
		 log_put(s);
	}
}

