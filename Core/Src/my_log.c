/*
 * my_log.c
 *
 *  Created on: 8 maj 2022
 *      Author: olek
 */

#include "my_log.h"
#include "cmsis_os2.h"
#include "string.h"

#define LOG_MAX 7
#define LOG_LEN 40

char log_items[LOG_MAX][LOG_LEN];
uint8_t log_item = 0;

uint32_t itime = 0;
char stime[10];


int ILinia(uint8_t lp) {
	return (log_item + lp) % LOG_MAX;
}

void log_put(const char *s) {
	char sl[LOG_LEN];
	if (s[0] == 0) {
		sprintf(sl, "+%s", stime);
		strcpy(log_items[log_item], sl);
	} else {
		sprintf(sl, "%s-%s", stime, s);
		strcpy(log_items[log_item], sl);
		log_item = (log_item + 1) % LOG_MAX;
	}

	//osSemaphoreRelease(myBinarySemLCDHandle);
//	printf(sl);
}

void set_stime(){

	itime = osKernelGetTickCount() / osKernelGetTickFreq();
	//uint8_t h = t / (60 * 60);
	//uint8_t m = (t / 60) % 60;
	//uint8_t s = t % 60;
	//		sprintf(stime, "%d/%d:%02d:%02d", resets, h, m, s);
	//		sprintf(stime, "%d:%02d:%02d", h, m, s);
	sprintf(stime, "%lu", itime);

}


