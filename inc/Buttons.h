/*
 * Buttons.h
 *
 *  Created on: May 30, 2020
 *      Author: raul
 */

#ifndef PROGRAMS_RTOS_TP_INC_BUTTONS_H_
#define PROGRAMS_RTOS_TP_INC_BUTTONS_H_

#include "sapi.h"
#include <stdlib.h>
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

typedef struct{
	gpioMap_t button;
	bool_t ISRstate;
}debounce_t;



void Buttons_init(void);

#endif /* PROGRAMS_RTOS_TP_INC_BUTTONS_H_ */
