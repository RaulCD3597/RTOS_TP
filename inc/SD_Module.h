/*
 * SD_Module.h
 *
 *  Created on: May 25, 2020
 *      Author: raul
 */

#ifndef SD_MODULE_H_
#define SD_MODULE_H_

#include "events.h"

void SD_Init(void);
void SD_WriteSyslog(event_t *newEvent);

#endif /* SD_MODULE_H_ */
