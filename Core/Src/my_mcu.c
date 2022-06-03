/*
 * roleta.c
 *
 *  Created on: 17 maj 2022
 *      Author: olek
 */

#include "my_mcu.h"
#include "my_can.h"
#include "my_mqtt.h"
#include "my_log.h"
#include "stdio.h"
#include "string.h"


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
	char t[TOPIC_LEN] = "can/";
	char s[LOG_LEN];

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

void  my_mcu_recive_mqtt_topic(const char *topic){ //oodbiór z mqtt i przesłanie do MCU (can)
	char *pid;
	char sid[10];


	memset(&msg_TxCan, 0, sizeof(msg_TxCan));

	if (strstr(topic, "/set") == NULL)
		return;

	pid = topic + strlen(topic_can)+1;

	u8_t l = 0;

	while ((*(pid+l) >= '0' && *(pid+l) <= '9'))
		l++;

	strncpy(sid, pid, l);

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

	char d[] = {0,0,0,0,0,0,0,0,0,0};
	strncpy(d, data, len);

	if (strcmp(d,"ON") == 0) {
		msg_TxCan.TxData.val = 1;
		goto next;
	}
	if (strcmp(d,"OFF") == 0) {
		msg_TxCan.TxData.val = 0;
		goto next;
	}
	if (strcmp(d,"STOP") == 0) {
		msg_TxCan.TxData.val = 255;
		goto next;
	}

	msg_TxCan.TxData.val = atoi(d);

	next:

	msg_TxCan.TxHeader.DLC = 3; // długość danych TxData/RxData
	msg_TxCan.TxHeader.IDE = CAN_ID_STD;
	msg_TxCan.TxHeader.RTR = CAN_RTR_DATA;
	my_can_Tx_Queue(&msg_TxCan);

	return;
}

