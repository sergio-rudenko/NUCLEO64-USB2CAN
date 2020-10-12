/*
 * ucanopen.h
 *
 *  Created on: Oct 9, 2020
 *      Author: sergi
 */

#ifndef LIB_UCANOPEN_UCANOPEN_H_
#define LIB_UCANOPEN_UCANOPEN_H_

#include "uco_defs.h"

#if defined( UCANOPEN_EMCY_ENABLED )
#include "uco_EMCY.h"
#endif /* UCANOPEN_EMCY_ENABLED */

typedef struct uCO {
	uCO_NodeId_t NodeId;

#if defined( UCANOPEN_EMCY_ENABLED )
	uCO_EMCY_t EMCY;
#endif /* UCANOPEN_EMCY_ENABLED */

} uCO_t;

#endif /* LIB_UCANOPEN_UCANOPEN_H_ */
