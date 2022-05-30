/*
 * my_mqtt.h
 *
 *  Created on: May 5, 2022
 *      Author: olek
 */

#ifndef INC_MY_MQTT_H_
#define INC_MY_MQTT_H_

#define MQTT_OUTPUT_RINGBUF_SIZE 1024 // systemowa LWIP

#include "mqtt.h"


#define MQTT_MSG_TOPIC_LEN  50
#define MQTT_MSG_VALUE_LEN  50

typedef struct {
	char topic[MQTT_MSG_TOPIC_LEN];
	char value[MQTT_MSG_VALUE_LEN];
} mqtt_msg__t;

//extern mqtt_msg__t mqtt_msg;


extern mqtt_client_t mqtt_client;

void my_mqtt_do_connect(mqtt_client_t *client);
void my_mqtt_publish(mqtt_client_t *client, const char *t, const char *m);
void my_mqtt_to_Queue(const char *t, const char *m);


#endif /* INC_MY_MQTT_H_ */
