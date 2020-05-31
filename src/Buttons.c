/*
 * Buttons.c
 *
 *  Created on: May 30, 2020
 *      Author: raul
 */

#include "Buttons.h"
#include "chip.h"

// GPIO
#define GPIO1_GPIO_PORT  0
#define GPIO1_GPIO_PIN   8
#define GPIO0_GPIO_PORT  0
#define GPIO0_GPIO_PIN   4

// Interrupt
#define TEC2_INDEX			1
#define TEC2_ISR_HANDLER	GPIO1_IRQHandler
#define TEC2_INTERRUPT		PIN_INT1_IRQn
#define TEC1_INDEX          0
#define TEC1_ISR_HANDLER    GPIO0_IRQHandler
#define TEC1_INTERRUPT      PIN_INT0_IRQn

#define BQUEUE_LENGTH 1

static debounce_t ISRButton;
QueueHandle_t buttonsQueue;

static void ButtonTask(void * pvParameters);
static void configGPIO_Interrupts(void);

void Buttons_init(void) {

	buttonsQueue = xQueueCreate(BQUEUE_LENGTH, sizeof(debounce_t));
	if (buttonsQueue != NULL) {
		configGPIO_Interrupts();
	}

	xTaskCreate(
				ButtonTask,
				(const char *)"ButtonTask",
				configMINIMAL_STACK_SIZE,
				NULL,
				Buttontsk_PRIORITY,
				NULL
	);
}

static void ButtonTask(void * pvParameters) {
	debounce_t buttonEvent;
	for (;;) {
		xQueueReceive(buttonsQueue, &buttonEvent, portMAX_DELAY);

		if (buttonEvent.button == TEC1) {
			vTaskDelay(pdMS_TO_TICKS(40));
			if (!gpioRead(TEC1)) {
				gpioToggle(LEDB);
			}
			NVIC_EnableIRQ(TEC1_INTERRUPT);
		} else if (buttonEvent.button == TEC2) {
			vTaskDelay(pdMS_TO_TICKS(40));
			if (!gpioRead(TEC2)) {
				gpioToggle(LED1);
			}
			NVIC_EnableIRQ(TEC2_INTERRUPT);
		}
	}
}

static void configGPIO_Interrupts(void) {

	/* Configuracion de GPIO0 de la EDU-CIAA-NXP como entrada con pull-up */
	gpioConfig(GPIO0, GPIO_INPUT_PULLUP);

	// ---> Comienzo de funciones LPCOpen para configurar la interrupcion

	// Configure interrupt channel for the GPIO pin in SysCon block
	Chip_SCU_GPIOIntPinSel( TEC2_INDEX, GPIO1_GPIO_PORT, GPIO1_GPIO_PIN);
	Chip_SCU_GPIOIntPinSel( TEC1_INDEX, GPIO0_GPIO_PORT, GPIO0_GPIO_PIN);

	// Configure channel interrupt as edge sensitive and falling edge interrupt
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(TEC2_INDEX));
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH(TEC2_INDEX));
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH(TEC2_INDEX));

	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(TEC1_INDEX));
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH(TEC1_INDEX));
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH(TEC1_INDEX));

	// Enable interrupt in the NVIC
	NVIC_SetPriority(TEC2_INTERRUPT, configMAX_SYSCALL_INTERRUPT_PRIORITY+5);
	NVIC_SetPriority(TEC1_INTERRUPT, configMAX_SYSCALL_INTERRUPT_PRIORITY+5);
	NVIC_ClearPendingIRQ(TEC2_INTERRUPT);
	NVIC_EnableIRQ(TEC2_INTERRUPT);
	NVIC_ClearPendingIRQ(TEC1_INTERRUPT);
	NVIC_EnableIRQ(TEC1_INTERRUPT);
}

void TEC2_ISR_HANDLER(void) {
	NVIC_DisableIRQ(TEC2_INTERRUPT);
	BaseType_t xHigherPriorityTaskWoken;

	xHigherPriorityTaskWoken = pdFALSE;

	// Se da aviso que se trato la interrupcion
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(TEC2_INDEX));

	// Se realiza alguna accion.
//	ISRButton.ISRstate = gpioRead(TEC2);
	ISRButton.button = TEC2;
	xQueueSendFromISR(buttonsQueue, &ISRButton, &xHigherPriorityTaskWoken);

//	NVIC_EnableIRQ( TEC2_INTERRUPT);
}

void TEC1_ISR_HANDLER(void) {
	NVIC_DisableIRQ(TEC1_INTERRUPT);
	BaseType_t xHigherPriorityTaskWoken;

	xHigherPriorityTaskWoken = pdFALSE;

	// Se da aviso que se trato la interrupcion
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(TEC1_INDEX));

	// Se realiza alguna accion.
//	ISRButton.ISRstate = gpioRead(TEC1);
	ISRButton.button = TEC1;
	xQueueSendFromISR(buttonsQueue, &ISRButton, &xHigherPriorityTaskWoken);

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
