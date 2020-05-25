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
#include "chip.h"

// SCU
#define GPIO0_SCU_PORT   6
#define GPIO0_SCU_PIN    1
#define GPIO0_SCU_FUNC   SCU_MODE_FUNC0

// GPIO
#define GPIO1_GPIO_PORT  0
#define GPIO1_GPIO_PIN   8
#define GPIO0_GPIO_PORT  0
#define GPIO0_GPIO_PIN   4

// Interrupt
#define PININT1_INDEX         1
#define PININT1_IRQ_HANDLER   GPIO1_IRQHandler
#define PININT1_NVIC_NAME     PIN_INT1_IRQn
#define PININT_INDEX          0
#define PININT_IRQ_HANDLER    GPIO0_IRQHandler
#define PININT_NVIC_NAME      PIN_INT0_IRQn


void PININT1_IRQ_HANDLER(void) {
	// Se da aviso que se trato la interrupcion
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(PININT1_INDEX));

	// Se realiza alguna accion.
	gpioToggle(LEDB);
}

void PININT_IRQ_HANDLER(void) {
	// Se da aviso que se trato la interrupcion
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(PININT_INDEX));

	// Se realiza alguna accion.
	gpioToggle(LED1);
}

int main(void) {
	boardInit();

	/* Configuracion de GPIO0 de la EDU-CIAA-NXP como entrada con pull-up */
//	gpioConfig(GPIO1, GPIO_INPUT_PULLUP);
	gpioConfig(GPIO0, GPIO_INPUT_PULLUP);

	// ---> Comienzo de funciones LPCOpen para configurar la interrupcion

	// Configure interrupt channel for the GPIO pin in SysCon block
	Chip_SCU_GPIOIntPinSel( PININT1_INDEX, GPIO1_GPIO_PORT, GPIO1_GPIO_PIN);
	Chip_SCU_GPIOIntPinSel( PININT_INDEX, GPIO0_GPIO_PORT, GPIO0_GPIO_PIN);

	// Configure channel interrupt as edge sensitive and falling edge interrupt
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(PININT1_INDEX));
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH(PININT1_INDEX));
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH(PININT1_INDEX));
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(PININT_INDEX));
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH(PININT_INDEX));
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH(PININT_INDEX));

	// Enable interrupt in the NVIC
	NVIC_ClearPendingIRQ( PININT1_NVIC_NAME);
	NVIC_EnableIRQ( PININT1_NVIC_NAME);
	// Enable interrupt in the NVIC
	NVIC_ClearPendingIRQ( PININT_NVIC_NAME);
	NVIC_EnableIRQ( PININT_NVIC_NAME);

	while (true)
		;

	return 0;
}
