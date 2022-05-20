/*
 * my_can.h
 *
 *  Created on: May 5, 2022
 *      Author: olek
 */

#ifndef INC_MY_CAN_H_
#define INC_MY_CAN_H_

#include "can.h"

typedef struct {
	uint8_t to;						// kierunek transiski ; do kogo
	uint8_t fun;					// funkcja
	uint8_t	val;					// wartosc (H)
	uint8_t	valL;					// wartosc (L)
	uint8_t xxx[4];				// rezerwa
} CanData_t;

extern uint32_t TxMailbox;
extern CAN_FilterTypeDef Can_FilterConfig;


//uint8_t id_device = 1;								// identyfikator urządzenia 1 - fabryczny

typedef struct  {
	CAN_RxHeaderTypeDef RxHeader; // 7 x uint32_t = 28
	CanData_t RxData;							// 4 x uint8_t = 4
} MsgQRxCan_t;  														// size = 32 bajty

typedef struct  {
	CAN_TxHeaderTypeDef TxHeader; // 5 x uint32_t + 1 = 21
	CanData_t TxData;							// 4 x uint8_t = 4
} MsgQTxCan_t;

void CanConfig(void);
void Can_RX(const MsgQRxCan_t *msg_can);


#endif /* INC_MY_CAN_H_ */
