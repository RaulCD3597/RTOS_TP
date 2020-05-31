/*
 *  bluetooth.h
 *
 *  Created on: May 30, 2020
 *      Author: raul
 */

#ifndef BLUETOOTH_H_
#define BLUETOOTH_H_

#include "events.h"

void bluetooth_Init(void);
void bluetooth_SendEvent(event_t *pNewEvent);

#endif /* BLUETOOTH_H_ */
