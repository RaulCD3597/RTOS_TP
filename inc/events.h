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

extern QueueHandle_t FSMQueue;

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

uint8_t getValueLength(uint8_t *value);

#endif /* EVENTS_H_ */