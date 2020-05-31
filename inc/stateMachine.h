/*
 * stateMachine.h
 *
 *  Created on: May 12, 2020
 *      Author: raulcamacho
 */

#ifndef PROGRAMS_PDM_TPFINAL_INC_MYAPP_H_
#define PROGRAMS_PDM_TPFINAL_INC_MYAPP_H_
/**
 * @typedef deviceState_t
 * @brief tipo de dato que permite saber el estado en que se
 * encuentra la FSM de la aplicacion principal
 */
typedef enum{
	IDLE,
	EMERGENCY,
	NORMAL,
	LOW_BATT,
	EM_LOW_BATT,
	NOR_LOW_BATT
} deviceState_t;
/**
 * @brief Funcion que inicializa la FSM de la aplicacion principal
 */
void deviceSM_Init(void);
/**
 * @brief Funcion para el control de la FSM de la aplicacion principal
 * debe ser llamada continuamente dentro de la funcion main
 */
void deviceSM_Update(event_t newEvent);

#endif /* PROGRAMS_PDM_TPFINAL_INC_MYAPP_H_ */