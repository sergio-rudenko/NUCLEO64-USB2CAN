/*
 * ucanopen.h
 *
 *  Created on: Oct 9, 2020
 *      Author: sergi
 */

#ifndef LIB_UCANOPEN_UCANOPEN_H_
#define LIB_UCANOPEN_UCANOPEN_H_

#include <uCO_defs.h>
#include "EMCY/emcy.h"
#include "SYNC/sync.h"
#include "TIME/time.h"
#include "LSS/lss.h"
#include "NMT/nmt.h"
#include "PDO/pdo.h"
#include "SDO/sdo.h"

#define UCANOPEN_RX_BUFFER_SIZE	64
#define UCANOPEN_TX_BUFFER_SIZE 256


void
uco_run(uCO_t *p);

void
uco_init(uCO_t *p, const uOD_Item_t *pOD);

void
uco_receive_to_buffer(uCO_t *p, CAN_RxHeaderTypeDef *pHeader, uint8_t *pData);

ErrorStatus
uco_transmit_from_buffer(uCO_t *p);

#endif /* LIB_UCANOPEN_UCANOPEN_H_ */
