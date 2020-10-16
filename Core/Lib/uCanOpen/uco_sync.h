/*
 * uco_sync.h
 *
 *  Created on: Oct 16, 2020
 *      Author: sergi
 */

#ifndef LIB_UCANOPEN_UCO_SYNC_H_
#define LIB_UCANOPEN_UCO_SYNC_H_

#include "uco_defs.h"

#define UCANOPEN_COB_ID_SYNC 				0x080
#define UCANOPEN_SYNC_MESSAGE_LENGTH		0

/* prototypes */

uCO_ErrorStatus_t
uco_proceed_sync_request(uCO_t *p);

#endif /* LIB_UCANOPEN_UCO_SYNC_H_ */
