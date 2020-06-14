/*
 *  stateMachine.c
 *
 *  Created on: May 30, 2020
 *      Author: raulcamacho
 */
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "sapi.h"
#include "events.h"
#include "stateMachine.h"
#include "uartPC.h"
#include "bluetooth.h"
#include "SD_Module.h"

static deviceState_t mainState;
static QueueHandle_t FSMQueue;

static void FSMTask(void *pvParameters);
static void deviceSM_Update(event_t newEvent);
static void led_Update();

void deviceSM_Init(void)
{
    mainState = IDLE;
    FSMQueue = xQueueCreate(2, sizeof(event_t));
    if (FSMQueue != NULL)
    {
        xTaskCreate(
            FSMTask,
            (const char *)"FSMTask",
            (configMINIMAL_STACK_SIZE * 5),
            NULL,
            FSMtsk_PRIORITY,
            NULL);
    }
}

void FSMTask(void *pvParameters)
{
    event_t newEvent;
    for (;;)
    {
        xQueueReceive(FSMQueue, &newEvent, portMAX_DELAY);
        deviceSM_Update(newEvent);
    }
}

static void deviceSM_Update(event_t newEvent)
{
    uint8_t msg[100] = "";
    uint8_t receiveBuffer[50] = "";
    uint8_t id = 0;
    switch (mainState)
    {
    case IDLE:
        switch (newEvent.event)
        {
        case BLE_EVENT:
            if (newEvent.msgId != NO_ID)
            {
                uartPC_SendEvent(&newEvent);
                SD_WriteSyslog(&newEvent);
                mainState += newEvent.msgId;
            }
            break;
        case UARTPC_EVENT:
            bluetooth_SendEvent(&newEvent);
            SD_WriteSyslog(&newEvent);
            break;
        case TEC2_EVENT:
            SD_ShowSyslog();
            break;
        default:
            break;
        }
        led_Update();
        break;
    case EMERGENCY:
        switch (newEvent.event)
        {
        case BLE_EVENT:
            if (newEvent.msgId == LOWBATT_BT)
            {
                uartPC_SendEvent(&newEvent);
                SD_WriteSyslog(&newEvent);
                mainState = EM_LOW_BATT;
            }
            break;
        case TEC1_EVENT:
            mainState = IDLE;
            SD_WriteSyslog(&newEvent);
            break;
        case UARTPC_EVENT:
            bluetooth_SendEvent(&newEvent);
            SD_WriteSyslog(&newEvent);
            break;
        case TEC2_EVENT:
            SD_ShowSyslog();
            break;
        default:
            break;
        }
        led_Update();
        break;
    case NORMAL:
        switch (newEvent.event)
        {
        case BLE_EVENT:
            if (newEvent.msgId != NORMAL_BT && newEvent.msgId != NO_ID)
            {
                uartPC_SendEvent(&newEvent);
                SD_WriteSyslog(&newEvent);
                mainState = (newEvent.msgId == EMERGENCY_BT) ? EMERGENCY : NOR_LOW_BATT;
            }
            break;
        case TEC1_EVENT:
            mainState = IDLE;
            SD_WriteSyslog(&newEvent);
            break;
        case UARTPC_EVENT:
            bluetooth_SendEvent(&newEvent);
            SD_WriteSyslog(&newEvent);
            break;
        case TEC2_EVENT:
            SD_ShowSyslog();
            break;
        default:
            break;
        }
        led_Update();
        break;
    case LOW_BATT:
        switch (newEvent.event)
        {
        case BLE_EVENT:
            if (newEvent.msgId != LOWBATT_BT && newEvent.msgId != NO_ID)
            {
                uartPC_SendEvent(&newEvent);
                SD_WriteSyslog(&newEvent);
                mainState += newEvent.msgId;
            }
            break;
        case TEC1_EVENT:
            mainState = IDLE;
            SD_WriteSyslog(&newEvent);
            break;
        case UARTPC_EVENT:
            bluetooth_SendEvent(&newEvent);
            SD_WriteSyslog(&newEvent);
            break;
        case TEC2_EVENT:
            SD_ShowSyslog();
            break;
        default:
            break;
        }
        led_Update();
        break;
    case EM_LOW_BATT:
        switch (newEvent.event)
        {
        case TEC1_EVENT:
            mainState = IDLE;
            SD_WriteSyslog(&newEvent);
            break;
        case UARTPC_EVENT:
            bluetooth_SendEvent(&newEvent);
            SD_WriteSyslog(&newEvent);
            break;
        case TEC2_EVENT:
            SD_ShowSyslog();
            break;
        default:
            break;
        }
        led_Update();
        break;
    case NOR_LOW_BATT:
        switch (newEvent.event)
        {
        case BLE_EVENT:
            if (newEvent.msgId == EMERGENCY_BT)
            {
                uartPC_SendEvent(&newEvent);
                SD_WriteSyslog(&newEvent);
                mainState = EM_LOW_BATT;
            }
            break;
        case TEC1_EVENT:
            mainState = IDLE;
            SD_WriteSyslog(&newEvent);
            break;
        case UARTPC_EVENT:
            bluetooth_SendEvent(&newEvent);
            SD_WriteSyslog(&newEvent);
            break;
        case TEC2_EVENT:
            SD_ShowSyslog();
            break;
        default:
            break;
        }
        led_Update();
        break;
    default:
        deviceSM_Init();
        break;
    }
}

static void led_Update()
{
    switch (mainState)
    {
    case IDLE:
        gpioWrite(LED1, OFF);
        gpioWrite(LED2, OFF);
        gpioWrite(LEDB, OFF);
        break;
    case EMERGENCY:
        gpioWrite(LED1, OFF);
        gpioWrite(LED2, ON);
        gpioWrite(LEDB, OFF);
        break;
    case NORMAL:
        gpioWrite(LED1, ON);
        gpioWrite(LED2, OFF);
        gpioWrite(LEDB, OFF);
        break;
    case LOW_BATT:
        gpioWrite(LED1, OFF);
        gpioWrite(LED2, OFF);
        gpioWrite(LEDB, ON);
        break;
    case EM_LOW_BATT:
        gpioWrite(LED1, OFF);
        gpioWrite(LED2, ON);
        gpioWrite(LEDB, ON);
        break;
    case NOR_LOW_BATT:
        gpioWrite(LED1, ON);
        gpioWrite(LED2, OFF);
        gpioWrite(LEDB, ON);
        break;
    default:
        break;
    }
}

void send_Event(event_t *pEvent)
{
    xQueueSend(FSMQueue, pEvent, portMAX_DELAY);
}