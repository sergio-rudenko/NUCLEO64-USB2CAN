/*
 * TIME/time.h
 *
 *  Created on: Oct 16, 2020
 *      Author: sergi
 */

#ifndef LIB_UCANOPEN_UCO_TIME_H_
#define LIB_UCANOPEN_UCO_TIME_H_

#include <uCO_defs.h>

/* prototypes */

uCO_ErrorStatus_t
uco_proceed_time_message(uCO_t *p, uint8_t *data);

#endif /* LIB_UCANOPEN_UCO_TIME_H_ */
