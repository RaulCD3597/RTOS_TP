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
#include "SD_Module.h"

int main(void)
{
	boardInit();
	/**
	 * Solo inicializo el modulo de SD para poder configurar RTC
	 * despues de iniciar RTC inicializo el resto de modulos en la
	 * tarea de SD.
	 */
	SD_Init();

	vTaskStartScheduler();
	// Nunca debe llegar al return.
	return 0;
}
