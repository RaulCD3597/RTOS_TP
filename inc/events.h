/*
 *  events.h
 *
 *  Created on: May 30, 2020
 *      Author: raul
 */

#ifndef EVENTS_H_
#define EVENTS_H_

#include <stdlib.h>
#include <stdint.h>

enum{
    BLE_EVENT = 0,
    TEC1_EVENT,
    TEC2_EVENT
};

enum{
	NO_ID = 0,
	EMERGENCY_BT,
	NORMAL_BT,
	LOWBATT_BT
};

typedef struct{
    uint8_t     event;
    uint8_t     msgId;
    uint8_t     message[30];
}event_t;

#endif /* EVENTS_H_ */