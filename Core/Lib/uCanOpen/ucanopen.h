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
#include "uco_nmt.h"
#include "uco_sdo.h"

void
uco_init(uCO_t *p, const uCO_OD_Item_t *pOD);

uCO_ErrorStatus_t
uco_update();

uCO_ErrorStatus_t
uco_proceed(CanRxMessage_t *msg);

uCO_OD_Item_t*
uco_find_OD_item(uCO_t *p, uint16_t id, uint8_t sub);

uCO_OD_Item_t*
uco_find_RPDO_OD_item(uCO_t *p, uint16_t id, uint8_t sub);

uCO_OD_Item_t*
uco_find_TPDO_OD_item(uCO_t *p, uint16_t id, uint8_t sub);

uCO_OD_Item_t*
uco_find_Manufacturer_OD_item(uCO_t *p, uint16_t id, uint8_t sub);

#endif /* LIB_UCANOPEN_UCANOPEN_H_ */
