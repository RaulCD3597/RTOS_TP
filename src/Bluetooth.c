/*
 * Bluetooth.c
 *
 *  Created on: May 30, 2020
 *      Author: raul
 */

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "sapi.h"
#include <string.h>
#include "bluetooth.h"
#include <stdint.h>
#include <stdbool.h>
#include "events.h"
#include "stateMachine.h"

#define ID_STRING "id"
#define MSG_STRING "msg"
#define SEPARATOR ":"
#define BLUETOOTH UART_232
#define BLUETOOTH_IRQn USART3_IRQn
#define BT_BAUDRATE 115200
#define SPC_TO_CONTENT 2
#define MSG_SPC_TO_CONTENT 3
#define BT_MAX_WAIT_TIME 100
#define BLE_RX_BUFF_LENGTH (sizeof(BLE_RX_buffer) / sizeof(BLE_RX_buffer[0]))
#define BLE_EMERGENCY_ID "1"
#define BLE_NORMAL_ID "2"
#define BLE_LOWBATT_ID "3"

uint8_t BLE_RX_buffer[100];
uint8_t BLE_RXindex = 0;
SemaphoreHandle_t BLE_Semaphore;
QueueHandle_t FSMQueue;

void BLE_ReceiveCallback(void *pvParam);
static void BLETask(void *pvParameters);
static void bluetooth_Parser(uint8_t *msg, event_t *pBLEevent);
static void get_ID(uint8_t *msg, uint8_t *pId);
static void get_Msg(uint8_t *msg, uint8_t *receiveBuffer, uint8_t *length);

void bluetooth_Init(void)
{
	uartConfig(BLUETOOTH, BT_BAUDRATE);
	uartCallbackSet(BLUETOOTH, UART_RECEIVE, BLE_ReceiveCallback, NULL);
	NVIC_SetPriority(BLUETOOTH_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY + 5);
	NVIC_EnableIRQ(BLUETOOTH_IRQn);

	BLE_Semaphore = xSemaphoreCreateBinary();

	xTaskCreate(
		BLETask,
		(const char *)"BLETask",
		(configMINIMAL_STACK_SIZE * 2),
		NULL,
		BLEtsk_PRIORITY,
		NULL);
}

void BLE_ReceiveCallback(void *pvParam)
{
	BaseType_t xHigherPriorityTaskWoken;

	xHigherPriorityTaskWoken = pdFALSE;
	uint8_t c = uartRxRead(BLUETOOTH);
	if (c == '{')
	{
		BLE_RXindex = 0;
		memset(BLE_RX_buffer, 0, BLE_RX_BUFF_LENGTH);
		BLE_RX_buffer[BLE_RXindex] = c;
	}
	else if (c == '\r')
	{
		xSemaphoreGiveFromISR(BLE_Semaphore, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
	else if ((c != '\n') && (c != '\r'))
	{
		BLE_RX_buffer[BLE_RXindex] = c;
	}
	BLE_RXindex++;
}

void BLETask(void *pvParameters)
{
	event_t BLE_newMsg;
	for (;;)
	{
		xSemaphoreTake(BLE_Semaphore, portMAX_DELAY);
		bluetooth_Parser(BLE_RX_buffer, &BLE_newMsg);
		xQueueSend(FSMQueue, &BLE_newMsg, pdMS_TO_TICKS(10));
	}
}

static void bluetooth_Parser(uint8_t *msg, event_t *pBLEevent)
{
	pBLEevent->event = BLE_EVENT;
	get_ID(msg, &pBLEevent->msgId);
	get_Msg(msg, pBLEevent->message, &pBLEevent->msgLength);
}

static void get_ID(uint8_t *msg, uint8_t *pId)
{
	uint8_t *search = strstr(msg, ID_STRING);
	if (search != NULL)
	{
		search = strstr(search, SEPARATOR);
		search += SPC_TO_CONTENT;
		if (!strncmp(search, BLE_EMERGENCY_ID, 1))
		{
			*pId = EMERGENCY_BT;
		}
		else if ((!strncmp(search, BLE_NORMAL_ID, 1)))
		{
			*pId = NORMAL_BT;
		}
		else if ((!strncmp(search, BLE_LOWBATT_ID, 1)))
		{
			*pId = LOWBATT_BT;
		}
		else
		{
			*pId = NO_ID;
		}
	}
}

static void get_Msg(uint8_t *msg, uint8_t *receiveBuffer, uint8_t *length)
{
	memset(receiveBuffer, 0, sizeof(receiveBuffer));
	uint8_t *search = strstr(msg, MSG_STRING);
	if (search != NULL)
	{
		search = strstr(search, SEPARATOR);
		search += MSG_SPC_TO_CONTENT;
		*length = getValueLength(search);
		memcpy(receiveBuffer, search, *length);
	}
	else
	{
		*length = 0;
	}
}

void bluetooth_SendEvent(event_t *pNewEvent)
{
	uint8_t messageToSend[100], msg[(pNewEvent->msgLength + 1)];
	memcpy(msg, pNewEvent->message, pNewEvent->msgLength);
	msg[pNewEvent->msgLength] = 0;
	if (pNewEvent->msgLength)
	{
		sprintf(messageToSend, "{\"msg\": \"%s\"}\r\n", msg);
	}
	else
	{
		sprintf(messageToSend, "{\"openMic\": %s}\r\n", pNewEvent->msgId ? "true" : "false");
	}
	uartWriteString(BLUETOOTH, messageToSend);
}
