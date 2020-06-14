/*
 * uartPC.h
 *
 *  Created on: May 31, 2020
 *      Author: raul
 */

#ifndef UARTPC_H_
#define UARTPC_H_

#include "events.h"

#define UARTPC UART_USB
/**
 * Inicializa modulo UARTPC
 */
void uartPC_Init(void);
/**
 * Muestro en UARTPC eventos generados por bluetooth
 * @param pNewEvent puntero a evento procesado por deviceFSM
 */
void uartPC_SendEvent(event_t *pNewEvent);
/**
 * Funcion para mostrar los ultimos eventos guardados en SD
 * @param msg string con los ultimos eventos guardados en SD
 */
void uartPC_SendSyslog(uint8_t *msg);

#endif /* UARTPC_H_ */