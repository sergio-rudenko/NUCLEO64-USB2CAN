/*
 * uco_SDO.h
 *
 *  Created on: Oct 9, 2020
 *      Author: sergi
 */

#ifndef LIB_UCANOPEN_UCO_SDO_H_
#define LIB_UCANOPEN_UCO_SDO_H_

#include "uco_defs.h"

/* prototypes */

uCO_ErrorStatus_t
uco_proceed_SDO_request(uCO_NodeId_t NodeId, uint8_t *pData);

uCO_ErrorStatus_t
uco_proceed_SDO_reply(uCO_NodeId_t NodeId, uint8_t *pData);

#endif /* LIB_UCANOPEN_UCO_SDO_H_ */
