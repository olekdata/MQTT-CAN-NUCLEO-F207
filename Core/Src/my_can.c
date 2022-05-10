/*
 * my_can.c
 *
 *  Created on: May 5, 2022
 *      Author: olek
// */

#include "my_can.h"
#include "my_mqtt.h"

uint32_t TxMailbox;
CAN_FilterTypeDef Can_FilterConfig;


MsgQRxCan_t msg;


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
{ 0x448041, "wywiew_temp", 0},
{ 0x488041, "wywiew_wilg", 1},
{ 0x44c041, "wyrzut_temp", 0},
{ 0x48C041, "wyrzut_wilg", 1},
{ 0x370041, "czerpnia_temp", 0},
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
};


uint8_t sofar_len = sizeof(sofar) / sizeof(sofar[0]);

uint8_t Sofar_RX(void) {
	for (uint8_t i = 0; i < sofar_len; i++)
		if (sofar[i].ExtId == msg.RxHeader.ExtId){
			int16_t v;
  		char s[40];
			sprintf(s,"??");

			switch (sofar[i].val_typ){
			  case 0:
				   v = msg.RxData.to-msg.RxData.fun;
				   sprintf(s,"%d,%d",v/10,v%10);
				   break;
			  case 1:
				   v = msg.RxData.to;
				   sprintf(s,"%d",v);
				   break;
			  case 2:
			  	 v = msg.RxData.to + 256*msg.RxData.fun;
			 		 sprintf(s,"%d",v);
			 		 break;
			};

   	  mqtt_my_publish(&mqtt_client, sofar[i].topic, s);
			return 1;
	  };
	return 0;
}


void Can_RX(void){

	if (Sofar_RX())
		return;

	char t[20];
	char s[40];

	sprintf(t,"ExtID=%lX", msg.RxHeader.ExtId);
	sprintf(s,"%d %d %d %d %d", msg.RxHeader.DLC,  msg.RxData.to, msg.RxData.fun, msg.RxData.val, msg.RxData.valL);
	mqtt_my_publish(&mqtt_client, t, s);
}


