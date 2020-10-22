/*
 * uco_NMT.h
 *
 *  Created on: Oct 9, 2020
 *      Author: sergi
 */

#ifndef LIB_UCANOPEN_UCO_NMT_H_
#define LIB_UCANOPEN_UCO_NMT_H_

#include "uco_defs.h"

#define UCANOPEN_COB_ID_NMT 							0x000
#define UCANOPEN_COB_ID_HEARTBEAT						0x700
#define UCANOPEN_COB_ID_NODE_GUARDING					0x700
#define UCANOPEN_NMT_MESSAGE_LENGTH						1

#define UCANOPEN_HEARTBEAT_MESSAGE_LENGTH				0x01
#define UCANOPEN_HEARTBEAT_BOOTUP_MESSAGE				0x00
#define UCANOPEN_HEARTBEAT_NODE_STATE_STOPPED			0x04
#define UCANOPEN_HEARTBEAT_NODE_STATE_OPERATIONAL		0x05
#define UCANOPEN_HEARTBEAT_NODE_STATE_PREOPERATIONAL 	0x7F

/* prototypes */

void
uco_nmt_on_tick(uCO_t *p);

uCO_ErrorStatus_t
uco_send_boot_message(uCO_t *p);

uCO_ErrorStatus_t
uco_send_heartbeat_message(uCO_t *p);

uCO_ErrorStatus_t
uco_proceed_nmt_command(uCO_t *p, uint8_t *data);

/* callback`s */

void
uco_nmt_on_uplink_status(uCO_t *p, bool alive);

#endif /* LIB_UCANOPEN_UCO_NMT_H_ */
