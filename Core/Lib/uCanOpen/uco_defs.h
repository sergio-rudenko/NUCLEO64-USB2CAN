/*
 * uco_defs.h
 *
 *  Created on: Oct 9, 2020
 *      Author: sergi
 */

#ifndef LIB_UCANOPEN_UCO_DEFS_H_
#define LIB_UCANOPEN_UCO_DEFS_H_

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#define UCANOPEN_EMCY_ENABLED
#define UCANOPEN_NMT_ENABLED
#define UCANOPEN_LSS_ENABLED
#define UCANOPEN_SDO_ENABLED
#define UCANOPEN_PDO_ENABLED

typedef int8_t uCO_NodeId_t;
typedef uint16_t uCO_CobId_t;

typedef struct uCO_CanMessage {
	uCO_CobId_t COB_ID;
	uint8_t len;
	uint8_t buf[8];
} uCO_CanMessage_t;

#endif /* LIB_UCANOPEN_UCO_DEFS_H_ */
