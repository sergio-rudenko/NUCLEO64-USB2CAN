/*
 * uco_SDO.h
 *
 *  Created on: Oct 9, 2020
 *      Author: sergi
 *
 *  @ref
 */

#ifndef LIB_UCANOPEN_UCO_SDO_H_
#define LIB_UCANOPEN_UCO_SDO_H_

#include "uco_defs.h"

#define UCANOPEN_COB_ID_TSDO 							0x580
#define UCANOPEN_COB_ID_RSDO 							0x600
#define UCANOPEN_SDO_LENGTH								8

/* SDO Read */
#define UCANOPEN_SDO_READ_REQUEST						0x40

/* SDO Write */
#define UCANOPEN_SDO_WRITE_REPLY						0x60

/* SDO Expedited Read +*/
#define UCANOPEN_SDO_EXPEDITED_READ_4BYTES_REPLY		0x43
#define UCANOPEN_SDO_EXPEDITED_READ_2BYTES_REPLY		0x4B
#define UCANOPEN_SDO_EXPEDITED_READ_1BYTE_REPLY			0x4F

/* SDO Segmented Read */
#define UCANOPEN_SDO_SEGMENTED_READ_REPLY 				0x41
#define UCANOPEN_SDO_SEGMENTED_READ_SEGMENT_T0_REQUEST 	0x60
#define UCANOPEN_SDO_SEGMENTED_READ_SEGMENT_T1_REQUEST 	0x70

/* SDO Expedited Write +*/
#define UCANOPEN_SDO_EXPEDITED_WRITE_4BYTES_REQUEST		0x23
#define UCANOPEN_SDO_EXPEDITED_WRITE_2BYTES_REQUEST		0x2B
#define UCANOPEN_SDO_EXPEDITED_WRITE_1BYTE_REQUEST		0x2F

/* SDO Segmented Write */
#define UCANOPEN_SDO_SEGMENTED_WRITE_REQUEST			0x21
#define UCANOPEN_SDO_SEGMENTED_WRITE_SEGMENT_T0_REQUEST 0x00
#define UCANOPEN_SDO_SEGMENTED_WRITE_SEGMENT_T1_REQUEST 0x10
#define UCANOPEN_SDO_SEGMENTED_WRITE_SEGMENT_REPLY		0x20

/* Abort|ERROR */
#define UCANOPEN_SDO_ABORT_CS 							0x80
#define UCANOPEN_SDO_ABORT_TOGGLE_BIT_NOT_ALTERED		0x05030000UL
#define UCANOPEN_SDO_ABORT_REASON_TIMED_OUT				0x05040000UL
#define UCANOPEN_SDO_ABORT_REASON_OUT_OF_MEMORY 		0x05040005UL
#define UCANOPEN_SDO_ABORT_REASON_UNKNOWN_COMMAND 		0x05040001UL
#define UCANOPEN_SDO_ABORT_REASON_UNSUPPORTED     		0x06010000UL
#define UCANOPEN_SDO_ABORT_REASON_NOT_EXISTS      		0x06020000UL
#define UCANOPEN_SDO_ABORT_REASON_WRITEONLY        		0x06010001UL
#define UCANOPEN_SDO_ABORT_REASON_READONLY        		0x06010002UL
#define UCANOPEN_SDO_ABORT_REASON_TYPEMISMATCH    		0x06070010UL
#define UCANOPEN_SDO_ABORT_REASON_UNKNOWN_SUB      		0x06090011UL
#define UCANOPEN_SDO_ABORT_GENERAL_ERROR				0x08000000UL

/**/
#define UCANOPEN_SDO_TIMEOUT 							1000

/* prototypes */

void
uco_sdo_on_tick(uCO_t *p);

uCO_ErrorStatus_t
uco_sdo_abort(uCO_t *p, uint32_t reason);

uCO_ErrorStatus_t
uco_proceed_sdo_request(uCO_t *p, uint8_t *data);

uCO_ErrorStatus_t
uco_proceed_sdo_reply(uCO_t *p, uint8_t *data);

/* callback`s */

size_t
uco_sdo_get_octet_string_size(uCO_t *p, uint16_t index, uint8_t sub);

size_t
uco_sdo_get_visible_string_length(uCO_t *p, uint16_t index, uint8_t sub);

uCO_ErrorStatus_t
uco_sdo_prepare_data(uCO_t *p, uint16_t index, uint8_t sub);

uCO_ErrorStatus_t
uco_sdo_validate_data(uCO_t *p, uint16_t index, uint8_t sub);

void
uco_sdo_on_write_success(uCO_t *p, uint16_t index, uint8_t sub);

void
uco_sdo_on_read_success(uCO_t *p, uint16_t index, uint8_t sub);

void
uco_sdo_on_abort(uCO_t *p, uint16_t index, uint8_t sub, uint32_t reason);

#endif /* LIB_UCANOPEN_UCO_SDO_H_ */
