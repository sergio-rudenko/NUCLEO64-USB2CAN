/*
 * PDO/pdo.h
 *
 *  Created on: Oct 9, 2020
 *      Author: sergi
 */

#ifndef LIB_UCANOPEN_UCO_PDO_H_
#define LIB_UCANOPEN_UCO_PDO_H_

#include <uCO_defs.h>

/* prototypes */

void
uco_pdo_on_tick(uCO_t *p);

ErrorStatus
uco_tpdo_transmit(uCO_t *p, int num);

/* callback`s */

ErrorStatus
uco_tpdo_prepare_data(uCO_t *p, int num);

void
uco_tpdo_on_send(uCO_t *p, int num);

#endif /* LIB_UCANOPEN_UCO_PDO_H_ */
