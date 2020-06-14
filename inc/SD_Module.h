/*
 * SD_Module.h
 *
 *  Created on: May 25, 2020
 *      Author: raul
 */

#ifndef SD_MODULE_H_
#define SD_MODULE_H_

#include "events.h"
/**
 * Funcion para inicilizar modulo de memoria SD
 */
void SD_Init(void);
/**
 * Funcion que permite guardar en SD un evento procesado por deviceFSM
 * @param newEvent evento a guardar en memoria SD
 */
void SD_WriteSyslog(event_t *newEvent);
/**
 * Funcion que permite mostrar syslog guardado en SD a traves de UART_USB
 */
void SD_ShowSyslog(void);

#endif /* SD_MODULE_H_ */
