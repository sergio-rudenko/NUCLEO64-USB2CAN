/*
 * uco_NMT.h
 *
 *  Created on: Oct 9, 2020
 *      Author: sergi
 */

#ifndef LIB_UCANOPEN_UCO_NMT_H_
#define LIB_UCANOPEN_UCO_NMT_H_

#include <uCO_defs.h>

/* prototypes */

void
uco_nmt_on_tick(uCO_t *p);

ErrorStatus
uco_nmt_send_boot_message(uCO_t *p);

ErrorStatus
uco_nmt_send_heartbeat_message(uCO_t *p);

ErrorStatus
uco_proceed_nmt_command(uCO_t *p, uint8_t cmd, uCO_NodeId_t addr);

/* Master API */

ErrorStatus
uco_nmt_master_send_command(uCO_t *p, uint8_t cmd, uCO_NodeId_t addr);

/* callback`s */

void
uco_nmt_on_uplink_status(uCO_t *p, bool alive);

#endif /* LIB_UCANOPEN_UCO_NMT_H_ */
