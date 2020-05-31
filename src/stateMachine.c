/*
 *  stateMachine.c
 *
 *  Created on: May 30, 2020
 *      Author: raulcamacho
 */

#include "sapi.h"
#include "events.h"
#include "stateMachine.h"

static deviceState_t mainState;

void deviceSM_Init(void)
{
    mainState = IDLE;
}

void deviceSM_Update(event_t newEvent)
{
    uint8_t msg[100] = "";
    uint8_t receiveBuffer[50] = "";
    uint8_t id = 0;
    switch (mainState)
    {
    case IDLE:
        gpioWrite(LED1, OFF);
        gpioWrite(LED2, OFF);
        gpioWrite(LEDB, OFF);
        switch (newEvent.event)
        {
        case BLE_EVENT:
            // miApp_UART_Send(newEvent.msgId, newEvent.message);
            // mainState += newEvent.msgId;
            break;
        default:
            break;
        }
        break;
    case EMERGENCY:
        gpioWrite(LED1, OFF);
        gpioWrite(LED2, ON);
        gpioWrite(LEDB, OFF);
        switch (newEvent.event)
        {
        case BLE_EVENT:
            if (newEvent.msgId == LOWBATT_BT)
            {
                // miApp_UART_Send(newEvent.msgId, newEvent.message);
                // mainState += newEvent.msgId;
            }
            break;
        case TEC1_EVENT:
            mainState = IDLE;
            break;
        default:
            break;
        }
        break;
    case NORMAL:
        gpioWrite(LED1, ON);
        gpioWrite(LED2, OFF);
        gpioWrite(LEDB, OFF);
        switch (newEvent.event)
        {
        case BLE_EVENT:
            if (newEvent.msgId != NORMAL_BT)
            {
                // miApp_UART_Send(newEvent.msgId, newEvent.message);
                // mainState += newEvent.msgId;
            }
            break;
        case  TEC1_EVENT:
            mainState = IDLE;
            break;
        default:
            break;
        }
        break;
    case LOW_BATT:
        gpioWrite(LED1, OFF);
        gpioWrite(LED2, OFF);
        gpioWrite(LEDB, ON);
        switch (newEvent.event)
        {
        case BLE_EVENT:
            if (newEvent.msgId != LOWBATT_BT)
            {
                // miApp_UART_Send(newEvent.msgId, newEvent.message);
                // mainState += newEvent.msgId;
            }
            break;
        case  TEC1_EVENT:
            mainState = IDLE;
            break;
        default:
            break;
        }
        break;
    case EM_LOW_BATT:
        gpioWrite(LED1, OFF);
        gpioWrite(LED2, ON);
        gpioWrite(LEDB, ON);
        switch (newEvent.event)
        {
        case  TEC1_EVENT:
            mainState = IDLE;
            break;
        default:
            break;
        }
        break;
    case NOR_LOW_BATT:
        gpioWrite(LED1, ON);
        gpioWrite(LED2, OFF);
        gpioWrite(LEDB, ON);
        switch (newEvent.event)
        {
        case BLE_EVENT:
            if (newEvent.msgId != EMERGENCY_BT)
            {
                // miApp_UART_Send(newEvent.msgId, newEvent.message);
                // mainState += newEvent.msgId;
            }
            break;
        case  TEC1_EVENT:
            mainState = IDLE;
            break;
        default:
            break;
        }
        break;
    default:
        deviceSM_Init();
        break;
    }
}
