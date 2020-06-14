/*
 * SD_Module.c
 *
 *  Created on: May 25, 2020
 *      Author: raul
 */

#include "events.h"
#include "SD_Module.h"
#include "sapi.h"
#include "ff.h"
#include "fssdc.h"
#include <string.h>
#include "uartPC.h"

#define FILENAME "SDC:/syslog.txt"

static FATFS fs;
static FIL fp;
static rtc_t rtc = {
	2020,
	6,
	6,
	6,
	9,
	0,
	0};
static uint8_t idToMessage[MAX_WORDS][MAX_CHAR] = {"Error", "Emergencia", "Normal", "Bateria baja"};

static void FATTask(void *taskParmPtr);
static void SD_BLEEvent(event_t *newEvent);
static void SD_TEC1Event(event_t *newEvent);
static void SD_UARTEvent(event_t *newEvent);

void SD_Init(void)
{
	rtcInit();		// Inicializar RTC
	rtcWrite(&rtc); // Establecer fecha y hora

	// SPI configuration
	spiConfig(SPI0);

	// Initialize SD card driver
	FSSDC_InitSPI();
	// Give a work area to the default drive
	if (f_mount(&fs, "SDC:", 0) != FR_OK)
	{
		// If this fails, it means that the function could
		// not register a file system object.
		// Check whether the SD card is correctly connected
	}

	xTaskCreate(
		FATTask,
		(const char *)"FATTask",
		configMINIMAL_STACK_SIZE,
		NULL,
		FATtsk_PRIORITY,
		NULL);
}

void FATTask(void *taskParmPtr)
{
	portTickType xPeriodicity = 10 / portTICK_RATE_MS;
	portTickType xLastWakeTime = xTaskGetTickCount();

	for (;;)
	{
		disk_timerproc();
		vTaskDelayUntil(&xLastWakeTime, xPeriodicity);
	}
}

void SD_WriteSyslog(event_t *newEvent)
{
	switch (newEvent->event)
	{
	case BLE_EVENT:
		SD_BLEEvent(newEvent);
		break;
	case TEC1_EVENT:
		SD_TEC1Event(newEvent);
		break;
	case UARTPC_EVENT:
		SD_UARTEvent(newEvent);
		break;
	default:
		break;
	}
}

static void SD_BLEEvent(event_t *pNewEvent)
{
	rtcRead(&rtc);

	if (f_open(&fp, FILENAME, FA_WRITE | FA_OPEN_APPEND) == FR_OK)
	{
		int nbytes, length;
		uint8_t buff[200];
		uint8_t msg[(pNewEvent->msgLength + 1)];
		memcpy(msg, pNewEvent->message, pNewEvent->msgLength);

		if (pNewEvent->msgLength > 0)
		{
			length = sprintf(buff, "%04d-%02d-%02d %02d:%02d:%02d Paciente envia notificacion %s: %s\r\n",
							 rtc.year,
							 rtc.month,
							 rtc.mday,
							 rtc.hour,
							 rtc.min,
							 rtc.sec,
							 idToMessage[pNewEvent->msgId],
							 msg);
		}
		else
		{
			length = sprintf(buff, "%04d-%02d-%02d %02d:%02d:%02d Paciente envia notificacion %s\r\n",
							 rtc.year,
							 rtc.month,
							 rtc.mday,
							 rtc.hour,
							 rtc.min,
							 rtc.sec,
							 idToMessage[pNewEvent->msgId]);
		}

		f_write(&fp, buff, length, &nbytes);

		f_close(&fp);
	}
}

static void SD_TEC1Event(event_t *pNewEvent)
{
	rtcRead(&rtc);

	if (f_open(&fp, FILENAME, FA_WRITE | FA_OPEN_APPEND) == FR_OK)
	{
		int nbytes, length;
		uint8_t buff[100];

		length = sprintf(buff, "%04d-%02d-%02d %02d:%02d:%02d Paciente atendido\r\n",
						 rtc.year,
						 rtc.month,
						 rtc.mday,
						 rtc.hour,
						 rtc.min,
						 rtc.sec);

		f_write(&fp, buff, length, &nbytes);

		f_close(&fp);
	}
}

static void SD_UARTEvent(event_t *pNewEvent)
{
	rtcRead(&rtc);
	uint8_t msg[(pNewEvent->msgLength + 1)];
	memcpy(msg, pNewEvent->message, pNewEvent->msgLength);

	if (f_open(&fp, FILENAME, FA_WRITE | FA_OPEN_APPEND) == FR_OK)
	{
		int nbytes, length;
		uint8_t buff[200] = "";
		uint8_t msg[(pNewEvent->msgLength + 1)];
		memcpy(msg, pNewEvent->message, pNewEvent->msgLength);
		msg[pNewEvent->msgLength] = 0;

		if (pNewEvent->msgLength > 0)
		{
			length = sprintf(buff, "%04d-%02d-%02d %02d:%02d:%02d Mensaje central enfermera: %s\r\n",
							 rtc.year,
							 rtc.month,
							 rtc.mday,
							 rtc.hour,
							 rtc.min,
							 rtc.sec,
							 msg);
		}
		else
		{
			length = sprintf(buff, "%04d-%02d-%02d %02d:%02d:%02d Central de Enfermera pide %s microfono\r\n",
							 rtc.year,
							 rtc.month,
							 rtc.mday,
							 rtc.hour,
							 rtc.min,
							 rtc.sec,
							 pNewEvent->msgId ? "cerrar" : "abrir");
		}

		f_write(&fp, buff, length, &nbytes);

		f_close(&fp);
	}
}

void SD_ShowSyslog(void)
{
	uint8_t buff[READ_SIZE];
	int nbytes;
	if (f_open(&fp, FILENAME, FA_READ) == FR_OK)
	{
		if (f_lseek(&fp, f_size(&fp) - READ_SIZE) == FR_OK)
		{
			f_read(&fp, buff, READ_SIZE, &nbytes);
			uartPC_SendSyslog(buff);
		}
		f_close(&fp);
	}
}
