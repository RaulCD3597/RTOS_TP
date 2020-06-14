/*
 * stateMachine.h
 *
 *  Created on: May 12, 2020
 *      Author: raulcamacho
 */

#ifndef STATE_MACHINE_H_
#define STATE_MACHINE_H_

typedef enum
{
	IDLE,
	EMERGENCY,
	NORMAL,
	LOW_BATT,
	EM_LOW_BATT,
	NOR_LOW_BATT
} deviceState_t;
/**
 * Inicializa maquina de estados principal deviceFSM
 */
void deviceSM_Init(void);

#endif /* STATE_MACHINE_H_ */