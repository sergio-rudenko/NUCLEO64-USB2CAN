/*
 * SDO/sdo.h
 *
 *  Created on: Oct 9, 2020
 *      Author: sergi
 *
 *  @ref
 */

#ifndef LIB_UCANOPEN_UCO_SDO_H_
#define LIB_UCANOPEN_UCO_SDO_H_

#include <uCO_defs.h>

/* prototypes */

void
uco_sdo_on_tick(uCO_t *p);

ErrorStatus
uco_sdo_abort(uCO_t *p, uint32_t reason);

ErrorStatus
uco_proceed_sdo_request(uCO_t *p, uint8_t *data);

ErrorStatus
uco_proceed_sdo_reply(uCO_t *p, uint8_t *data);

/* callback`s */

size_t
uco_sdo_get_octet_string_size(uCO_t *p, uint16_t index, uint8_t sub);

size_t
uco_sdo_get_visible_string_length(uCO_t *p, uint16_t index, uint8_t sub);

ErrorStatus
uco_sdo_prepare_data(uCO_t *p, uint16_t index, uint8_t sub);

ErrorStatus
uco_sdo_validate_data(uCO_t *p, uint16_t index, uint8_t sub);

void
uco_sdo_on_write_success(uCO_t *p, uint16_t index, uint8_t sub);

void
uco_sdo_on_read_success(uCO_t *p, uint16_t index, uint8_t sub);

void
uco_sdo_on_abort(uCO_t *p, uint16_t index, uint8_t sub, uint32_t reason);

#endif /* LIB_UCANOPEN_UCO_SDO_H_ */
