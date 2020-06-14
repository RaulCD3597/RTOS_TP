/*
 * Buttons.c
 *
 *  Created on: May 30, 2020
 *      Author: raul
 */

#include "Buttons.h"
#include "chip.h"
#include "events.h"

#define UP 1
#define FALLING 2
#define DOWN 3
#define RISING 4

#define CANT_TECLAS 2
#define ANTIREBOTE_MS 20

enum Teclas_t
{
	Tecla1,
	Tecla2
}; //indices de teclas para el vector de estructuras

typedef struct
{ //estructura de control de datos capturados por la interrupciÃ³n
	uint8_t GPIO_event;
	uint8_t Flanco;
} Button_Control_t;

struct Buttons_SM_t
{ //estructura de control de la mÃ¡quina de estados de cada botÃ³n
	uint8_t Tecla;
	uint8_t Estado;
	xQueueHandle Cola;
};

QueueHandle_t FSMQueue;
//Definicion de vector de estructuras de control
struct Buttons_SM_t Buttons_SM[CANT_TECLAS];

static void ButtonTask(void *pvParameters);
static void configGPIO_Interrupts(void);

void Buttons_init(void)
{
	uint8_t Error_state = 0;

	configGPIO_Interrupts();
	// Creo colas para sincronizar eventos de interrupcion de teclas
	for (size_t i = CANT_TECLAS; i--; i >= 0)
	{
		Buttons_SM[i].Tecla = i;
		if (NULL == (Buttons_SM[i].Cola = xQueueCreate(2, sizeof(Button_Control_t))))
		{
			Error_state = 1;
		}
	}
	// Creo tareas para botones
	xTaskCreate(
		ButtonTask,
		(const char *)"Button1",
		configMINIMAL_STACK_SIZE,
		&Buttons_SM[0],
		Buttontsk_PRIORITY,
		NULL);

	xTaskCreate(
		ButtonTask,
		(const char *)"Button2",
		configMINIMAL_STACK_SIZE,
		&Buttons_SM[1],
		Buttontsk_PRIORITY,
		NULL);
}

static void ButtonTask(void *pvParameters)
{
	event_t buttonEvent;
	struct Buttons_SM_t *Config;
	Config = (struct Buttons_SM_t *)pvParameters;
	Config->Estado = UP;

	Button_Control_t Control;
	// Maquina de estados para debounce de teclas
	for (;;)
	{
		if (xQueueReceive(Config->Cola, &Control, portMAX_DELAY))
		{
			switch (Config->Estado)
			{
			case UP:
				if (Control.Flanco == FALLING)
				{
					if (pdFALSE == (xQueueReceive(Config->Cola, &Control, (ANTIREBOTE_MS / portTICK_RATE_MS))))
						Config->Estado = DOWN;
				}
				NVIC_EnableIRQ(PIN_INT0_IRQn);
				NVIC_EnableIRQ(PIN_INT2_IRQn);
				break;
			case DOWN:
				if (Control.Flanco == RISING)
				{
					if (pdFALSE == (xQueueReceive(Config->Cola, &Control, (ANTIREBOTE_MS / portTICK_RATE_MS))))
					{
						Config->Estado = UP;
						buttonEvent.event = Control.GPIO_event;
						// Si se confirma pulsacion de tecla notifico a maquina de estados deviceFSM
						send_Event(&buttonEvent);
					}
				}
				NVIC_EnableIRQ(PIN_INT1_IRQn);
				NVIC_EnableIRQ(PIN_INT3_IRQn);
				break;
			default:
				Config->Estado = UP;
				break;
			}
		}
	}
}

static void configGPIO_Interrupts(void)
{
	//Inicializamos las interrupciones (LPCopen)
	Chip_PININT_Init(LPC_GPIO_PIN_INT);

	// TEC1 FALL
	Chip_SCU_GPIOIntPinSel(0, 0, 4);
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH0); //Se configura el canal para que se active por flanco
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH0);	 //Se configura para que el flanco sea el de bajada

	// TEC1 RISE
	Chip_SCU_GPIOIntPinSel(1, 0, 4);
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH1); //Se configura el canal para que se active por flanco
	Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT, PININTCH1);	 //En este caso el flanco es de subida

	// TEC2 FALL
	Chip_SCU_GPIOIntPinSel(2, 0, 8);
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH2);
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH2);

	// TEC1 RISE
	Chip_SCU_GPIOIntPinSel(3, 0, 8);
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH3);
	Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT, PININTCH3);

	//Una vez que se han configurado los eventos para cada canal de interrupcion
	//Se activan las interrupciones para que comiencen a llamar al handler
	NVIC_SetPriority(PIN_INT0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
	NVIC_EnableIRQ(PIN_INT0_IRQn);
	NVIC_SetPriority(PIN_INT1_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
	NVIC_EnableIRQ(PIN_INT1_IRQn);
	NVIC_SetPriority(PIN_INT2_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
	NVIC_EnableIRQ(PIN_INT2_IRQn);
	NVIC_SetPriority(PIN_INT3_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
	NVIC_EnableIRQ(PIN_INT3_IRQn);
}

void GPIO0_IRQHandler(void)
{
	NVIC_DisableIRQ(PIN_INT0_IRQn);
	BaseType_t xHigherPriorityTaskWoken = pdFALSE; //Comenzamos definiendo la variable

	if (Chip_PININT_GetFallStates(LPC_GPIO_PIN_INT) & PININTCH0)
	{															 //Verificamos que la interrupcion es la esperada
		Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH0); //Borramos el flag de interrupcion
		//codigo a ejecutar si ocurrio la interrupcion

		Button_Control_t Snapshot;
		Snapshot.Flanco = FALLING;
		Snapshot.GPIO_event = TEC1_EVENT;

		xQueueSendFromISR(Buttons_SM[Tecla1].Cola, &Snapshot, &xHigherPriorityTaskWoken);
	}

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void GPIO1_IRQHandler(void)
{
	NVIC_DisableIRQ(PIN_INT1_IRQn);
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if (Chip_PININT_GetRiseStates(LPC_GPIO_PIN_INT) & PININTCH1)
	{
		Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH1);
		//codigo a ejecutar si ocurrio la interrupcion
		Button_Control_t Snapshot;
		Snapshot.Flanco = RISING;
		Snapshot.GPIO_event = TEC1_EVENT;
		xQueueSendFromISR(Buttons_SM[Tecla1].Cola, &Snapshot, &xHigherPriorityTaskWoken);
	}
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void GPIO2_IRQHandler(void)
{
	NVIC_DisableIRQ(PIN_INT2_IRQn);
	BaseType_t xHigherPriorityTaskWoken = pdFALSE; //Comenzamos definiendo la variable

	if (Chip_PININT_GetFallStates(LPC_GPIO_PIN_INT) & PININTCH2)
	{															 //Verificamos que la interrupcion es la esperada
		Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH2); //Borramos el flag de interrupcion
		//codigo a ejecutar si ocurrio la interrupcion
		Button_Control_t Snapshot;
		Snapshot.Flanco = FALLING;
		Snapshot.GPIO_event = TEC2_EVENT;
		xQueueSendFromISR(Buttons_SM[Tecla2].Cola, &Snapshot, &xHigherPriorityTaskWoken);
	}

	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void GPIO3_IRQHandler(void)
{
	NVIC_DisableIRQ(PIN_INT3_IRQn);
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if (Chip_PININT_GetRiseStates(LPC_GPIO_PIN_INT) & PININTCH3)
	{
		Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH3);
		//codigo a ejecutar si ocurrio la interrupcion
		Button_Control_t Snapshot;
		Snapshot.Flanco = RISING;
		Snapshot.GPIO_event = TEC2_EVENT;
		xQueueSendFromISR(Buttons_SM[Tecla2].Cola, &Snapshot, &xHigherPriorityTaskWoken);
	}
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
