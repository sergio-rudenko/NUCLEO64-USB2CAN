/*
 * uco_EMCY.h
 *
 *  Created on: Oct 9, 2020
 *      Author: sergi
 */

#ifndef LIB_UCANOPEN_UCO_EMCY_H_
#define LIB_UCANOPEN_UCO_EMCY_H_

#include "uco_defs.h"

#define UCANOPEN_COB_ID_EMCY 				0x080
#define UCANOPEN_EMCY_MESSAGE_LENGTH		8

typedef struct uCO_EMCY
{

} uCO_EMCY_t;

/* prototypes */

uCO_ErrorStatus_t
uco_proceed_emcy_message(uCO_t *p, uint8_t *data);

#endif /* LIB_UCANOPEN_UCO_EMCY_H_ */
