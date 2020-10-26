/*
 * ucanopen.h
 *
 *  Created on: Oct 9, 2020
 *      Author: sergi
 */

#ifndef LIB_UCANOPEN_UCANOPEN_H_
#define LIB_UCANOPEN_UCANOPEN_H_

#include "uco_defs.h"
#include "uco_emcy.h"
#include "uco_sync.h"
#include "uco_time.h"
#include "uco_nmt.h"
#include "uco_pdo.h"
#include "uco_sdo.h"

#define UCANOPEN_RX_BUFFER_SIZE	64
#define UCANOPEN_TX_BUFFER_SIZE 256

void
uco_run(uCO_t *p);

void
uco_init(uCO_t *p, const uCO_OD_Item_t *pOD);

void
uco_receive_to_buffer(uCO_t *p, CAN_RxHeaderTypeDef *pHeader, uint8_t *pData);

uCO_ErrorStatus_t
uco_transmit_from_buffer(uCO_t *p);

#endif /* LIB_UCANOPEN_UCANOPEN_H_ */
