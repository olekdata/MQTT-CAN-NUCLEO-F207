/*
 * my_mqtt.h
 *
 *  Created on: May 5, 2022
 *      Author: olek
 */

#ifndef INC_MY_MQTT_H_
#define INC_MY_MQTT_H_

#define MQTT_OUTPUT_RINGBUF_SIZE 512


#include "mqtt.h"

extern mqtt_client_t mqtt_client;

void example_do_connect(mqtt_client_t *client, const char *topic);
void mqtt_my_publish(mqtt_client_t *client, const char *t, const char *m);


#endif /* INC_MY_MQTT_H_ */
