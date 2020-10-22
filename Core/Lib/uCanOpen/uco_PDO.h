/*
 * uco_PDO.h
 *
 *  Created on: Oct 9, 2020
 *      Author: sergi
 */

#ifndef LIB_UCANOPEN_UCO_PDO_H_
#define LIB_UCANOPEN_UCO_PDO_H_

#include "uco_defs.h"

#define UCANOPEN_COB_ID_TPDO_1 				0x180
#define UCANOPEN_COB_ID_TPDO_2 				0x280
#define UCANOPEN_COB_ID_TPDO_3 				0x380
#define UCANOPEN_COB_ID_TPDO_4 				0x480

#define UCANOPEN_COB_ID_RPDO_1 				0x200
#define UCANOPEN_COB_ID_RPDO_2 				0x300
#define UCANOPEN_COB_ID_RPDO_3 				0x400
#define UCANOPEN_COB_ID_RPDO_4 				0x500

/* prototypes */

void
uco_pdo_on_tick(uCO_t *p);

uCO_ErrorStatus_t
uco_tpdo_transmit(uCO_t *p, int num);

/* callback`s */

uCO_ErrorStatus_t
uco_tpdo_prepare_data(uCO_t *p, int num);

void
uco_tpdo_on_send(uCO_t *p, int num);


#endif /* LIB_UCANOPEN_UCO_PDO_H_ */
