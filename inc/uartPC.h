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

void uartPC_Init(void);
void uartPC_SendEvent(event_t *pNewEvent);
void uartPC_SendSyslog(uint8_t *msg);

#endif /* UARTPC_H_ */