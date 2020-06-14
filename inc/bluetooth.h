/*
 *  bluetooth.h
 *
 *  Created on: May 30, 2020
 *      Author: raul
 */

#ifndef BLUETOOTH_H_
#define BLUETOOTH_H_

#include "events.h"
/**
 * Funcion para inicializar modulo bluetooth
 */
void bluetooth_Init(void);
/**
 * Funcion para notificar evento a llamador
 * @param pNewEvent envento a notificar al llamador
 */
void bluetooth_SendEvent(event_t *pNewEvent);

#endif /* BLUETOOTH_H_ */
