/*
 * my_httpd.h
 *
 *  Created on: 8 maj 2022
 *      Author: olek
 */

#ifndef INC_MY_HTTPD_H_
#define INC_MY_HTTPD_H_

#include "stdbool.h"

extern bool LD1ON;
extern bool LD2ON;

void myCGIinit(void);
void mySSIinit(void);



#endif /* INC_MY_HTTPD_H_ */
