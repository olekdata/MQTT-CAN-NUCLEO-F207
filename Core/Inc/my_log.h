/*
 * my_log.h
 *
 *  Created on: 8 maj 2022
 *      Author: olek
 */

#ifndef INC_MY_LOG_H_
#define INC_MY_LOG_H_

#define LOG_MAX 7
#define LOG_LEN 70

#include "stdio.h"


extern char log_items[LOG_MAX][LOG_LEN];
extern uint8_t log_item;

extern uint32_t itime;
extern char stime[20];
extern char slicz[20];

int ILinia(uint8_t lp);
void log_put(const char *s);
void set_stime();

#endif /* INC_MY_LOG_H_ */
