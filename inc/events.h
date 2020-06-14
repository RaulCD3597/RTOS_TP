/*
 *  events.h
 *
 *  Created on: May 30, 2020
 *      Author: raul
 */

#ifndef EVENTS_H_
#define EVENTS_H_

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include <stdlib.h>
#include <stdint.h>

#define MAX_WORDS 4
#define MAX_CHAR 20
#define READ_SIZE 500

enum
{
    BLE_EVENT = 0,
    TEC1_EVENT,
    TEC2_EVENT,
    UARTPC_EVENT
};

enum
{
    NO_ID = 0,
    EMERGENCY_BT,
    NORMAL_BT,
    LOWBATT_BT
};

typedef struct
{
    uint8_t event;
    uint8_t msgId;
    uint8_t msgLength;
    uint8_t message[30];
} event_t;
/**
 * Funcion de utilidad que permite obtener la candidad 
 * de caracteres de un objeto JSON
 * @param value cadena que contiene el valor del JSON
 */
uint8_t getValueLength(uint8_t *value);
/**
 * Funcion para notificar eventos a deviceFSM
 * @param pEvent puntero al la estructura de  evento a notificar
 */
void send_Event(event_t *pEvent);

#endif /* EVENTS_H_ */