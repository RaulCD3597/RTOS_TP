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

#include "sapi.h"
#include <string.h>
#include "bluetooth.h"
#include <stdint.h>
#include <stdbool.h>

#define ID_STRING "id"
#define MSG_STRING "msg"
#define SEPARATOR ":"
#define BLUETOOTH UART_232
#define BT_BAUDRATE 115200
#define SPC_TO_CONTENT 2
#define MSG_SPC_TO_CONTENT 3
#define MSG_SPC_TO_END 4
#define BT_MAX_WAIT_TIME 100
#define BLE_RX_BUFF_LENGTH (sizeof(BLE_RX_buffer)/sizeof(BLE_RX_buffer[0]))

uint8_t BLE_RX_buffer[100];
uint8_t BLE_RXindex = 0;

void BLE_ReceiveCallback(void *pvParam);
static void BLETask(void * pvParameters);

void bluetooth_Init(void){
	uartConfig(BLUETOOTH, BT_BAUDRATE);
	uartCallbackSet(BLUETOOTH, UART_RECEIVE, BLE_ReceiveCallback, NULL);
	uartInterrupt(BLUETOOTH, TRUE);

	xTaskCreate(
				BLETask,
				(const char *)"BLETask",
				(configMINIMAL_STACK_SIZE * 5),
				NULL,
				BLEtsk_PRIORITY,
				NULL
		);
}

void BLE_ReceiveCallback(void *pvParam) {
	uint8_t c = uartRxRead(BLUETOOTH);
	if (c == '{') {
		BLE_RXindex = 0;
		memset(BLE_RX_buffer, 0, BLE_RX_BUFF_LENGTH);
		BLE_RX_buffer[BLE_RXindex] = c;
	} else if (c == '\r') {
		// enviar por cola el string
	} else if ((c != '\n') && (c != '\r')) {
		BLE_RX_buffer[BLE_RXindex] = c;
	}
	BLE_RXindex++;
}

void BLETask(void * pvParameters) {
	for (;;) {
		/* Task code goes here. */
	}
}
