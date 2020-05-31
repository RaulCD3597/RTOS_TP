/*=============================================================================
 * Copyright (c) 2020, Raul Camacho <raulc3597@gmail.com>
 * All rights reserved.
 * License: bsd-3-clause (see LICENSE.txt)
 * Date: 2020/05/25
 * Version: 1.0
 *===========================================================================*/

/*=====[Inclusions of function dependencies]=================================*/

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#include "sapi.h"
#include "Buttons.h"
#include "SD_Module.h"
#include "bluetooth.h"


int main(void) {
	boardInit();

	Buttons_init();
	SD_Init();
	bluetooth_Init();

	vTaskStartScheduler();

	return 0;
}


