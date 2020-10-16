/*
 * uco_time.h
 *
 *  Created on: Oct 16, 2020
 *      Author: sergi
 */

#ifndef LIB_UCANOPEN_UCO_TIME_H_
#define LIB_UCANOPEN_UCO_TIME_H_

#include "uco_defs.h"

#define UCANOPEN_COB_ID_TIME 				0x100
#define UCANOPEN_TIME_MESSAGE_LENGTH		6

/* prototypes */

uCO_ErrorStatus_t
uco_proceed_time_message(uCO_t *p, uint8_t *data);

#endif /* LIB_UCANOPEN_UCO_TIME_H_ */
