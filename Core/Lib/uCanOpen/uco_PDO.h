/*
 * uco_PDO.h
 *
 *  Created on: Oct 9, 2020
 *      Author: sergi
 */

#ifndef LIB_UCANOPEN_UCO_PDO_H_
#define LIB_UCANOPEN_UCO_PDO_H_

#include "uco_defs.h"

/** ---------------------------------------------------------
 * [CAN_and_CANOpen.pdf] 2.5.5 RPDO Communication Parameters
 * [CAN_and_CANOpen.pdf] 2.5.6 TPDO Communication Parameters
 * ----------------------------------------------------------
 *	Sub	| Name	 				| Data type
 *	------------------------------------------
 *	 0	| Number of entries		| UNSIGNED8
 *	 1	| COB ID				| UNSIGNED32
 *	 2	| Transmission type		| UNSIGNED8
 *	 3	| Inhibit Time			| UNSIGNED16
 *	 4	| Reserved				| UNSIGNED8
 *	 5	| Event Timer			| UNSIGNED16
 */

/* prototypes */

void
uco_pdo_on_tick(uCO_t *p);

uCO_ErrorStatus_t
uco_tpdo_init(uCO_TPDO_t *Tpdo, void *address, size_t size);

uCO_ErrorStatus_t
uco_tpdo_transmit(uCO_t *p, int num);

uCO_ErrorStatus_t
uco_tpdo_prepare_data(uCO_t *p, int num);

#endif /* LIB_UCANOPEN_UCO_PDO_H_ */
