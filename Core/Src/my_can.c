/*
 * my_can.c
 *
 *  Created on: May 5, 2022
 *      Author: olek
// */

#include "my_mcu.h"
#include "my_can.h"
#include "my_mqtt.h"
#include "my_log.h"
#include "cmsis_os2.h"

uint32_t TxMailbox;
CAN_FilterTypeDef Can_FilterConfig;

extern osMessageQueueId_t QueueTxCanHandle;


//MsgQRxCan_t msg_can;


void CanConfig(void) {
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

struct sofar_t {
	uint32_t ExtId;
	char topic[30];
	uint8_t val_typ;
} sofar[] = {
{ 0x448041, "wywiew_temp", 0}, 		//0
{ 0x488041, "wywiew_wilg", 1}, 		//1
{ 0x44c041, "wyrzut_temp", 0},
{ 0x48C041, "wyrzut_wilg", 1},
{ 0x370041, "czerpnia_temp", 0},  //4
{ 0x490041, "czerpnia_wilg", 1},
{ 0x458041, "nawiew_temp", 0},
{ 0x498041, "nawiew_wilg", 1},
{ 0x1DC041, "wydajnosc_wyw", 2},
{ 0x1E0041, "wydajnosc_naw", 2},
{ 0x1E4041, "wywiew_wen", 2},
{ 0x1E8041, "nawiew_wen", 2},
{ 0x200041, "energia_curent", 2},
{ 0x204041, "energia_total_year", 2},
{ 0x208041, "energia_total", 2},
{ 0x358041, "energia_odzyskana_year", 2},
{ 0x35C041, "energia_odzyskana_total", 2}
//           12345678901234567890123
};


uint8_t sofar_len = sizeof(sofar) / sizeof(sofar[0]);

uint8_t Sofar_RX(const MsgQRxCan_t *msg_can) {
	for (uint8_t i = 0; i < sofar_len; i++)
		if (sofar[i].ExtId == msg_can->RxHeader.ExtId){
			int16_t v;
  		char s[LOG_LEN];
  		char t[50];
			sprintf(s,"??");

			switch (sofar[i].val_typ){
			  case 0:
				   v = msg_can->RxData.to-msg_can->RxData.fun;
				   sprintf(s,"%d,%d",v/10,v%10);
				   break;
			  case 1:
				   v = msg_can->RxData.to;
				   sprintf(s,"%d",v);
				   break;
			  case 2:
			  	 v = msg_can->RxData.to + 256*msg_can->RxData.fun;
			 		 sprintf(s,"%d",v);
			 		 break;
			};

			sprintf(t,"rekuperator/%s",sofar[i].topic);
			my_mqtt_to_Queue(t, s);

			return 1;
	  };
	return 0;
}


void Can_RX(const MsgQRxCan_t *msg_can){

	if (Sofar_RX(msg_can))
		return;

	if (my_mcu_send_can(msg_can))
		return;

	char t[50];
	char s[LOG_LEN];

	sprintf(t,"reszta/ExtID=%lX", msg_can->RxHeader.ExtId);
	sprintf(s,"%d %d %d %d %d", msg_can->RxHeader.DLC,  msg_can->RxData.to, msg_can->RxData.fun, msg_can->RxData.val, msg_can->RxData.valL);
	// deb
	my_mqtt_to_Queue(t, s);
}


void my_can_Tx_Queue(const MsgQTxCan_t *msg){

	osStatus_t status;

  status = osMessageQueuePut(QueueTxCanHandle, msg, 0U, 0U);
	if (status != osOK) {
		 char s[LOG_LEN];
		 sprintf(s, "QPut can er %d\n", status);
		 log_put(s);
	}
}

void my_can_Tx(const MsgQTxCan_t *msg){
	HAL_CAN_AddTxMessage(&hcan1, &msg->TxHeader, &msg->TxData, &TxMailbox);
}


