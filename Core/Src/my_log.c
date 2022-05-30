/*
 * my_log.c
 *
 *  Created on: 8 maj 2022
 *      Author: olek
 */

#include "my_log.h"
#include "cmsis_os2.h"
#include "string.h"
#include "stats.h"


char log_items[LOG_MAX][LOG_LEN];
uint8_t log_item = 0;


uint32_t itime = 0;
char stime[12];


extern osSemaphoreId_t BinarySemLCDHandle;

int ILinia(uint8_t lp) {
	return (log_item + lp) % LOG_MAX;
}

void log_put(const char *s) {
	char sl[LOG_LEN];
	if (s[0] == 0) {
		sprintf(sl, "+%s\n", stime);
		strcpy(log_items[log_item], sl);
		if (itime % 60 == 0){
			printf(sl);
			stats_display();
			//MEM_STATS_DISPLAY();
		}

	} else {
		sprintf(sl, "%s-%s\n", stime, s);
		strcpy(log_items[log_item], sl);
		log_item = (log_item + 1) % LOG_MAX;
  	printf(sl);
	}

//	osSemaphoreRelease(myBinarySemLCDHandle);
//  	printf(sl);
//	uint8_t len = strlen(sl);
//  CDC_Transmit_FS(sl,  len);
//  CDC_Transmit_FS("\n",  1);
}

void set_stime(){

	itime = osKernelGetTickCount() / osKernelGetTickFreq();
	uint8_t h = itime / (60 * 60);
	uint8_t m = (itime / 60) % 60;
	uint8_t s = itime % 60;
//		sprintf(stime, "%d/%d:%02d:%02d", resets, h, m, s);
	sprintf(stime, "%d:%02d:%02d", h, m, s);
//	sprintf(stime, "%lu", itime);

}


