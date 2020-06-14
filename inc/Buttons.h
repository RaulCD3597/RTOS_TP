/*
 * Buttons.h
 *
 *  Created on: May 30, 2020
 *      Author: raul
 */

#ifndef BUTTONS_H_
#define BUTTONS_H_

#include "sapi.h"
#include <stdlib.h>
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
/**
 * Funcion que inicializa modulo de botones
 */
void Buttons_init(void);

#endif /* BUTTONS_H_ */
