/*
 * SD_Module.c
 *
 *  Created on: May 25, 2020
 *      Author: raul
 */

#include "SD_Module.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#include "sapi.h"

#include "ff.h"
#include "fssdc.h"

#define FILENAME "SDC:/syslog.txt"

static FATFS fs;
static FIL fp;

void FATTask(void *taskParmPtr);

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
