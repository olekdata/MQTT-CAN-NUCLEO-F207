/*
 * roleta.c
 *
 *  Created on: 17 maj 2022
 *      Author: olek
 */

#include "my_mcu.h"
#include "my_can.h"
#include "my_mqtt.h"
#include "stdio.h"
#include "string.h"


#define topic_root "DataSoft/"
#define topic_can  "DataSoft/can"
#define topic_roleta  "/roleta"
#define topic_switch1 "/switch1"
#define topic_switch2 "/switch2"
#define topic_roleta_pos "/roleta_pos"
#define topic_sensor1 "/sensor1"
#define topic_sensor2 "/sensor1"
#define topic_prad "/prad"
#define topic_temp "/temp"

#define TYP_WLACZNIK 1 				// zwykły puszkowy włącznik dwukanałowy
#define TYP_ROLETA_MAN 2			// roleta manualna - jak się trzyma to działą
#define TYP_ROLETA_AUT 3			// roleta automatyczna - raz nacisnąć do końca zamyka/otwiera
#define TYP_ROLETA_POZ 4			// roleta z pozycją zamknięciem i pomiarem prądu


// komunikacja CAN
#define CAN_KIER_PRZY 0
#define CAN_KIER_WYCH 1 // > 0 ; id urządzenie

#define CAN_FUN_ROLETA 1				// roleta dwu stanowa otwarta-zamknieta
#define CAN_FUN_SWITCH1 2
#define CAN_FUN_SWITCH2 3
#define CAN_FUN_SEN_BIN_1 4   			// sensor bin 1
#define CAN_FUN_SEN_BIN_2 5   			// sensor bin 2
#define CAN_FUN_ROLETA_POZ 6			// roleta proporcjonalna od 0(otwarta) - 100(zamklnieta)
#define CAN_FUN_SEN_PRAD 21   			// sensor prądu

#define CAN_FUN_KONF_ID 100   			// konfiguracja wyświetlenie, sprawdzenie id
#define CAN_FUN_KONF_TYP 101   			// konfiguracja wyświetlenie, sprawdzenie typ urządzenia
#define CAN_FUN_KONF_ZAP_CZAS 102 		// konfiguracja kalibracja czasu.
#define CAN_FUN_KONF_KALIBRACJA 103 	// konfiguracja kalibracja czasu.

#define DATA_VAL_OFF 0
#define DATA_VAL_ON 1
#define DATA_VAL_OPEN 0
#define DATA_VAL_CLOSE 1
#define DATA_VAL_STOP 255

//dla typ włącznik
#define I_WYL1 0
#define I_WYL2 1


// dla rolety
#define STAN_ROLETY_ZAMKNIETA 0
#define STAN_ROLETY_ZAMYKANA  1
#define STAN_ROLETY_OTWIERANA 2
#define STAN_ROLETY_OTWARTA 3
#define STAN_ROLETY_NIEUSTALONA 4
#define STAN_ROLETY_KALIBRACJA 6
#define STAN_ROLETY_DO_POZ 7

#define STAN_KALI_DOL 1
#define STAN_KALI_GORA 2


#define I_OTW 0
#define I_ZAM 1
#define I_3   2
#define I_4	  3


uint8_t my_mcu_send_can(const MsgQRxCan_t *msg_can){ //oodbiór z mcu (can) i wysyłka MQTT

	char b[10];
	char t[50] = "can/";
	char s[50];

	 if (msg_can->RxHeader.ExtId)
	   return 0;

	//uint32_t id_device = msg_can->RxHeader.StdId - 0x100;
  if (msg_can->RxData.to == 0)
	   return 0;

	uint8_t id = msg_can->RxData.to;

	strcat(t, itoa(id, b, 10));
  if (msg_can->RxData.fun == CAN_FUN_ROLETA){
  	if (msg_can->RxData.val == DATA_VAL_STOP)
  		strcpy(s, "STOP");
  	else if (msg_can->RxData.val == 1)
  		strcpy(s, "ON");
  	else
  		strcpy(s, "OFF");
  	strcat(t, topic_roleta);
  	my_mqtt_to_Queue(t, s);
  	return 1;
  }

  if (msg_can->RxData.fun == CAN_FUN_ROLETA_POZ){
  	strcat(t, topic_roleta_pos);
		strcpy(s, itoa(msg_can->RxData.val, b, 10));
  	my_mqtt_to_Queue(t, s);
  	return 1;
  }

  if (msg_can->RxData.fun == CAN_FUN_SWITCH1){
  	if (msg_can->RxData.val == DATA_VAL_ON)
  		strcpy(s, "ON");
  	else
  		strcpy(s, "OFF");
  	strcat(t, topic_switch1);
  	my_mqtt_to_Queue(t, s);
  	return 1;
  }
  if (msg_can->RxData.fun == CAN_FUN_SWITCH2){
  	if (msg_can->RxData.val == DATA_VAL_ON)
  		strcpy(s, "ON");
  	else
  		strcpy(s, "OFF");
  	strcat(t, topic_switch2);
  	my_mqtt_to_Queue(t, s);
  	return 1;
  }
  if (msg_can->RxData.fun == CAN_FUN_SEN_BIN_1){
  	if (msg_can->RxData.val == DATA_VAL_ON)
  		strcpy(s, "ON");
  	else
  		strcpy(s, "OFF");
  	strcat(t, topic_sensor1);
  	my_mqtt_to_Queue(t, s);
  	return 1;
  }
  if (msg_can->RxData.fun == CAN_FUN_SEN_BIN_2){
  	if (msg_can->RxData.val == DATA_VAL_ON)
  		strcpy(s, "ON");
  	else
  		strcpy(s, "OFF");
  	strcat(t, topic_sensor2);
  	my_mqtt_to_Queue(t, s);
  	return 1;
  }
  if (msg_can->RxData.fun == CAN_FUN_SEN_PRAD){
		strcpy(s, itoa(256*msg_can->RxData.val + msg_can->RxData.valL, b, 10));
  	strcat(t, topic_prad);
  	my_mqtt_to_Queue(t, s);
  	return 1;
  }
  if (msg_can->RxData.fun == 0){
		strcpy(s, itoa(256*msg_can->RxData.val + msg_can->RxData.valL, b, 10));
  	strcat(t, topic_temp);
  	my_mqtt_to_Queue(t, s);
  	return 1;
  }

	sprintf(s,"to:%d fun:%d val:%d val2:%d", msg_can->RxData.to, msg_can->RxData.fun, msg_can->RxData.val, msg_can->RxData.valL);
	sprintf(t,"can_reszta/StdId=%d", msg_can->RxHeader.StdId);

	my_mqtt_to_Queue(t, s);
	return 1;


	return 0;
}

MsgQTxCan_t msg_TxCan;

uint8_t my_mcu_recive_mqtt_topic(const char *topic){ //oodbiór z mqtt i przesłanie do MCU (can)
	char *pset;
	char *pid;
	char *p;
	char sid[4];


	memset(&msg_TxCan, 0, sizeof(msg_TxCan));

	pset = strstr(topic, "/set");

	if (pset == NULL)
		return;

	pid = pset -1;

	while ((*pid >= '0' && *pid <= '9'))
		pid--;

	strncpy(sid, pid+1, pset-pid-1);

	msg_TxCan.TxHeader.StdId = 0x100 + atoi(sid);

	if (strstr(topic, topic_roleta_pos)) {
		msg_TxCan.TxData.fun = CAN_FUN_ROLETA_POZ;
		return;
	}
	if (strstr(topic, topic_roleta)) {
		msg_TxCan.TxData.fun = CAN_FUN_ROLETA;
		return;
	}
	if (strstr(topic, topic_switch1)) {
		msg_TxCan.TxData.fun = CAN_FUN_SWITCH1;
		return;
	}
	if (strstr(topic, topic_switch2)) {
		msg_TxCan.TxData.fun = CAN_FUN_SWITCH2;
		return;
	}
	if (strstr(topic, topic_sensor1)) {
		msg_TxCan.TxData.fun = CAN_FUN_SEN_BIN_1;
		return;
	}
	if (strstr(topic, topic_sensor2)) {
		msg_TxCan.TxData.fun = CAN_FUN_SEN_BIN_2;
		return;
	}

}


void my_mcu_recive_mqtt_data(const u8_t *data, u16_t len){ //oodbiór z mqtt i przesłanie do MCU (can)

	if (msg_TxCan.TxHeader.StdId == 0)
		return;



}
