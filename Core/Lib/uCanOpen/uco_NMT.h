/*
 * uco_NMT.h
 *
 *  Created on: Oct 9, 2020
 *      Author: sergi
 */

#ifndef LIB_UCANOPEN_UCO_NMT_H_
#define LIB_UCANOPEN_UCO_NMT_H_

#include "uco_defs.h"

#define UCANOPEN_COB_ID_HEARTBEAT						0x700
#define UCANOPEN_HEARTBEAT_MESSAGE_LENGTH				0x01
#define UCANOPEN_HEARTBEAT_BOOTUP_MESSAGE				0x00
#define UCANOPEN_HEARTBEAT_NODE_STATE_STOPPED			0x04
#define UCANOPEN_HEARTBEAT_NODE_STATE_OPERATIONAL		0x05
#define UCANOPEN_HEARTBEAT_NODE_STATE_PREOPERATIONAL 	0x7F

/* prototypes */

uCO_ErrorStatus_t
uco_proceed_NMT_command(uCO_t *p, uint8_t *pData, uint32_t len);

uCO_ErrorStatus_t
uco_proceed_SYNC_request(uCO_t *p, uint8_t *pData, uint32_t len);

uCO_ErrorStatus_t
uco_send_BOOT_message(uCO_t *p);

uCO_ErrorStatus_t
uco_send_HEARTBEAT_message(uCO_t *p);

#endif /* LIB_UCANOPEN_UCO_NMT_H_ */
