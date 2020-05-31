/*
 * uartPC.c
 *
 *  Created on: May 31, 2020
 *      Author: raul
 */

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "uartPC.h"
#include <string.h>
#include "sapi.h"

#define UARTPC UART_USB
#define UARTPC_IRQn USART2_IRQn
#define UARTPC_BAUDRATE 115200
#define MAX_WORDS 4
#define MAX_CHAR 20
#define RX_BUFF_LENGTH (sizeof(RX_buffer) / sizeof(RX_buffer[0]))
#define SPC_TO_CONTENT 2
#define ID_STRING "id"
#define MSG_STRING "msg"
#define SEPARATOR ":"
#define MSG_SPC_TO_CONTENT 3

static uint8_t idToMessage[MAX_WORDS][MAX_CHAR] = {"Error", "Emergencia", "Normal", "Bateria baja"};
uint8_t RX_buffer[100];
uint8_t RXindex = 0;
SemaphoreHandle_t RX_Semaphore;

void RXCallback(void *pvParam);
static void uartPCTask(void *pvParameters);
static void uart_Parser(uint8_t *msg, event_t *pEvent);

void uartPC_Init(void)
{
    uartConfig(UARTPC, UARTPC_BAUDRATE);
    uartCallbackSet(UARTPC, UART_RECEIVE, RXCallback, NULL);
    NVIC_SetPriority(UARTPC_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY + 5);
    NVIC_EnableIRQ(UARTPC_IRQn);

    RX_Semaphore = xSemaphoreCreateBinary();

    xTaskCreate(
        uartPCTask,
        (const char *)"uartPCTask",
        (configMINIMAL_STACK_SIZE * 3),
        NULL,
        UARTPCtsk_PRIORITY,
        NULL);
}
void uartPC_SendEvent(event_t *pNewEvent)
{
    uint8_t messageToSend[100], msg[(pNewEvent->msgLength + 1)];
    memcpy(msg, pNewEvent->message, pNewEvent->msgLength);
    msg[pNewEvent->msgLength] = 0;
    if (pNewEvent->msgLength)
    {
        sprintf(messageToSend, "%s: %s\r\n", idToMessage[pNewEvent->msgId], msg);
    }
    else
    {
        sprintf(messageToSend, "%s\r\n", idToMessage[pNewEvent->msgId]);
    }
    uartWriteString(UARTPC, messageToSend);
}

void RXCallback(void *pvParam)
{
    BaseType_t xHigherPriorityTaskWoken;

    xHigherPriorityTaskWoken = pdFALSE;
    uint8_t c = uartRxRead(UARTPC);
    if (c == '{')
    {
        RXindex = 0;
        memset(RX_buffer, 0, RX_BUFF_LENGTH);
        RX_buffer[RXindex] = c;
    }
    else if (c == '\r')
    {
        xSemaphoreGiveFromISR(RX_Semaphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    else if ((c != '\n') && (c != '\r'))
    {
        RX_buffer[RXindex] = c;
    }
    RXindex++;
}

static void uartPCTask(void *pvParameters)
{
    event_t uartPC_newMsg;
    for (;;)
    {
        xSemaphoreTake(RX_Semaphore, portMAX_DELAY);
        uart_Parser(RX_buffer, &uartPC_newMsg);
        xQueueSend(FSMQueue, &uartPC_newMsg, pdMS_TO_TICKS(10));
    }
}

static void uart_Parser(uint8_t *msg, event_t *pEvent)
{
    pEvent->event = UARTPC_EVENT;
    uint8_t *buff = strstr(msg, ID_STRING);
    if (buff)
    {
        buff = strstr(buff, SEPARATOR);
        buff += SPC_TO_CONTENT;
        pEvent->msgId = atoi(buff);
        pEvent->msgLength = 0;
    }
    else
    {
        memset(pEvent->message, 0, sizeof(pEvent->message));
        buff = strstr(msg, MSG_STRING);
        if (buff != NULL)
        {
            buff = strstr(buff, SEPARATOR);
            buff += MSG_SPC_TO_CONTENT;
            pEvent->msgLength = getValueLength(buff);
            memcpy(pEvent->message, buff, pEvent->msgLength);
        }
    }
}
