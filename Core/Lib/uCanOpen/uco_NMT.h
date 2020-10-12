/*
 * uco_NMT.h
 *
 *  Created on: Oct 9, 2020
 *      Author: sergi
 */

#ifndef LIB_UCANOPEN_UCO_NMT_H_
#define LIB_UCANOPEN_UCO_NMT_H_

#include "uco_defs.h"

/* prototypes */

uCO_ErrorStatus_t
uco_proceed_NMT_command(uint8_t *pData, uint32_t len);

uCO_ErrorStatus_t
uco_proceed_SYNC_request(uint8_t *pData, uint32_t len);

#endif /* LIB_UCANOPEN_UCO_NMT_H_ */
