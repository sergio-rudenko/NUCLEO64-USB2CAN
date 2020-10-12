/*
 * uco_EMCY.h
 *
 *  Created on: Oct 9, 2020
 *      Author: sergi
 */

#ifndef LIB_UCANOPEN_UCO_EMCY_H_
#define LIB_UCANOPEN_UCO_EMCY_H_

#include "uco_defs.h"

typedef struct uCO_EMCY
{

} uCO_EMCY_t;

/* prototypes */

uCO_ErrorStatus_t
uco_proceed_EMCY_message(uCO_NodeId_t NodeId, uint8_t *pData, uint32_t len);

#endif /* LIB_UCANOPEN_UCO_EMCY_H_ */
