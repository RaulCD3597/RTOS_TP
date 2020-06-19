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
#include "Buttons.h"
#include "bluetooth.h"
#include "stateMachine.h"
#include "uartPC.h"

#define FILENAME "SDC:/syslog.txt"

static FATFS fs;
static FIL fp;
static rtc_t rtc = {
	2020, // Anho
	6,	  // Mes
	16,	  // Dia ( 0 - 31 )
	2,	  // Dia de la semana ( 1 - 7 )
	17,	  // Hora
	48,	  // Minutos
	0};	  // Segundos
static uint8_t idToMessage[MAX_WORDS][MAX_CHAR] = {"Error", "Emergencia", "Normal", "Bateria baja"};

static void FATTask(void *taskParmPtr);
static void SD_BLEEvent(event_t *newEvent);
static void SD_TEC1Event(event_t *newEvent);
static void SD_UARTEvent(event_t *newEvent);

void SD_Init(void)
{

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
		uartWriteString(UARTPC, "fail mounting SD");
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
	/**
	 * Note: Aqui inicializo todos los modulos restantes
	 * para poder usar el vTaskDelay en la libreria de RTC
	 * que fue modificada para que funcione con freeRTOS.
	 * Los delays que tiene RTC en sapi son necesario para
	 * correcto funcionamiento
	 */
	rtcInit();		// Inicializar RTC
	rtcWrite(&rtc); // Establecer fecha y hora
	vTaskDelay(pdMS_TO_TICKS(1000));
	bluetooth_Init();
	uartPC_Init();
	Buttons_init();
	deviceSM_Init();
	uartWriteString(UARTPC, "Sistema inicializado!\r\n");

	portTickType xPeriodicity = 10 / portTICK_RATE_MS;
	portTickType xLastWakeTime = xTaskGetTickCount();

	for (;;)
	{
		/**
		 * disk_timerproc necesita ejecutarse cada 10 ms para correcto
		 * funcionamiento de libreria FAT usada para guardar en SD
		 */
		disk_timerproc();
		vTaskDelayUntil(&xLastWakeTime, xPeriodicity);
	}
}

void SD_WriteSyslog(event_t *newEvent)
{
	/**
	 * 1. Verifico tipo de evento que se quiere escribir en SD
	 * 2. Escribo el evento guardando el tiempo de recepcion del evento
	 *    con formato segun el evento
	 * Note: El formato de cada evento esta en cada funcion que se llama
	 * 	  en el switch que sigue. Las funciones hacen practicamente el mismo
	 * 	  proceso.
	 */
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
	/**
	 * 1. Intento abrir el archivo donde se tienen guardados lo eventos
	 * 2. Si el documento existe y logro abrirlo, me voy hasta el final del
	 * archivo y retrocedo READ_SIZE posiciones para poder leer esta cantidad
	 * de caracteres.
	 * 3. Una vez posicionado READ_SIZE caracteres antes del final, voy leyendo
	 * y guardando en buff
	 * 4. Escribo en la UART conectada a la PC lo que tengo en buff
	 * 5. Cierro el archivo.
	 */
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
