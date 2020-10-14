/*
 * uco_sdo.c
 *
 *  Created on: Oct 12, 2020
 *      Author: sergi
 *
 * -------------------------------------------------------------------------------------------------------------
 * @Ref: [CAN_and_CANOpen.pdf] 2.4 Accessing the CANopen Object Dictionary (OD)
 * 		 with Service Data Objects (SDO)
 *
 * @Ref: [CAN_and_CANOpen.pdf]
 *
 * NOTE:
 * 		Upload 		=> READ
 * 		Download 	=> WRITE
 */

#include "uco_sdo.h"

/* Segmented SDO variables */
static bool savedToggleBit;
static size_t savedDataSize;
static size_t savedDataOffset;
static uCO_OD_Item_t *savedItem;

/**
 *
 */
static uCO_ErrorStatus_t
SDO_abort(uCO_t *p, uint8_t *request, uint32_t reason)
{
	uCO_CanMessage_t reply = { 0 };

	reply.CobId = UCANOPEN_COB_ID_TSDO | p->NodeId;
	reply.length = UCANOPEN_SDO_LENGTH;

	reply.data[0] = UCANOPEN_SDO_ABORT_CS;

	/* index and sub */
	reply.data[1] = request[1];
	reply.data[2] = request[2];
	reply.data[3] = request[3];

	/* reason to little endian */
	reply.data[4] = (reason) & 0xFF;
	reply.data[5] = (reason >> 8) & 0xFF;
	reply.data[6] = (reason >> 16) & 0xFF;
	reply.data[7] = (reason >> 24) & 0xFF;

	return uco_send(p, &reply);
}

/**
 *
 */
static uCO_ErrorStatus_t
SDO_expedited_read_reply(uCO_t *p, uint8_t *request, uCO_OD_Item_t *item)
{
	uCO_CanMessage_t reply = { 0 };
	uint32_t value;

	reply.CobId = UCANOPEN_COB_ID_TSDO | p->NodeId;
	reply.length = UCANOPEN_SDO_LENGTH;

	/* index and sub */
	reply.data[1] = request[1];
	reply.data[2] = request[2];
	reply.data[3] = request[3];

	switch (item->Type)
	{
		case UNSIGNED8:
			reply.data[0] = UCANOPEN_SDO_EXPEDITED_READ_1BYTE_REPLY;

			value = *((uint8_t*) item->address);

			reply.data[4] = (value) & 0xFF;
			break;

		case UNSIGNED16:
			reply.data[0] = UCANOPEN_SDO_EXPEDITED_READ_2BYTES_REPLY;

			value = *((uint16_t*) item->address);

			reply.data[4] = (value) & 0xFF;
			reply.data[5] = (value >> 8) & 0xFF;
			break;

		case UNSIGNED32:
			reply.data[0] = UCANOPEN_SDO_EXPEDITED_READ_4BYTES_REPLY;

			value = *((uint32_t*) item->address);

			reply.data[4] = (value) & 0xFF;
			reply.data[5] = (value >> 8) & 0xFF;
			reply.data[6] = (value >> 16) & 0xFF;
			reply.data[7] = (value >> 24) & 0xFF;
			break;

		default:
			return UCANOPEN_ERROR;
	}
	return uco_send(p, &reply);
}

/**
 *
 */
static uCO_ErrorStatus_t
SDO_expedited_write_reply(uCO_t *p, uint8_t *request, uCO_OD_Item_t *item)
{
	uCO_CanMessage_t reply = { 0 };
	uint32_t value;

	reply.CobId = UCANOPEN_COB_ID_TSDO | p->NodeId;
	reply.length = UCANOPEN_SDO_LENGTH;

	/* index and sub */
	reply.data[1] = request[1];
	reply.data[2] = request[2];
	reply.data[3] = request[3];

	reply.data[0] = UCANOPEN_SDO_WRITE_REPLY;

	switch (request[0] /* CCS */)
	{
		case UCANOPEN_SDO_EXPEDITED_WRITE_1BYTE_REQUEST:
			value = request[4];
			*((uint8_t*) item->address) = value;
			break;

		case UCANOPEN_SDO_EXPEDITED_WRITE_2BYTES_REQUEST:
			value = request[4];
			value += request[5] << 8;
			*((uint16_t*) item->address) = value;
			break;

		case UCANOPEN_SDO_EXPEDITED_WRITE_4BYTES_REQUEST:
			value = request[4];
			value += request[5] << 8;
			value += request[6] << 16;
			value += request[7] << 24;
			*((uint32_t*) item->address) = value;
			break;

		default:
			return UCANOPEN_ERROR;
	}
	return uco_send(p, &reply);
}

/**
 *
 */
static uCO_ErrorStatus_t
SDO_segmented_read_handshake(uCO_t *p, uint8_t *request, uCO_OD_Item_t *item)
{
	uCO_CanMessage_t reply = { 0 };

	reply.CobId = UCANOPEN_COB_ID_TSDO | p->NodeId;
	reply.length = UCANOPEN_SDO_LENGTH;

	switch (item->Type)
	{
		case UNSIGNED64:
			savedDataSize = sizeof(uint64_t);
			break;

		case OCTET_STRING:
			savedDataSize = item->size;
			break;

		case VISIBLE_STRING:
			savedDataSize = strlen((char*) item->address);
			break;

		default:
			SDO_abort(p, request, UCANOPEN_SDO_ABORT_REASON_UNSUPPORTED);
			return UCANOPEN_ERROR;
	}

	savedToggleBit = true;
	savedDataOffset = 0;
	savedItem = item;

	reply.data[0] = UCANOPEN_SDO_SEGMENTED_READ_REPLY;

	/* index and sub */
	reply.data[1] = request[1];
	reply.data[2] = request[2];
	reply.data[3] = request[3];

	/* size of data */
	reply.data[4] = (savedDataSize) & 0xFF;
	reply.data[5] = (savedDataSize >> 8) & 0xFF;
	reply.data[6] = (savedDataSize >> 16) & 0xFF;
	reply.data[7] = (savedDataSize >> 24) & 0xFF;

	return uco_send(p, &reply);
}

/**
 *
 */
static uCO_ErrorStatus_t
SDO_segmented_read_process(uCO_t *p, uint8_t *request)
{
	bool lastSegment;
	size_t freeBytes;

	bool toggleBit = (request[0] >> 4) & 0x1;
	uint8_t *data = (uint8_t*) savedItem->address;

	uCO_CanMessage_t reply = { 0 };

	reply.CobId = UCANOPEN_COB_ID_TSDO | p->NodeId;
	reply.length = UCANOPEN_SDO_LENGTH;

	/* Check Toggle Bit */
	if (toggleBit == savedToggleBit)
	{
		SDO_abort(p, request, UCANOPEN_SDO_ABORT_TOGGLE_BIT_NOT_ALTERED);
		return UCANOPEN_ERROR;
	}
	else
	{
		savedToggleBit = toggleBit;
	}

	/* Check remaining data */
	if (savedDataSize - savedDataOffset <= 7)
	{
		freeBytes = (7 - (savedDataSize - savedDataOffset));
		lastSegment = true;
	}
	else
	{
		freeBytes = 0;
		lastSegment = false;
	}

	/* Data */
	for (int i = 0; i < (7 - freeBytes); i++)
	{
		reply.data[1 + i] = data[savedDataOffset];
		savedDataOffset++;
	}

	/* Server CS */
	reply.data[0] = (toggleBit << 4) | (freeBytes << 1) | lastSegment;

	return uco_send(p, &reply);
}

/**
 *
 */
static uCO_ErrorStatus_t
SDO_segmented_write_handshake(uCO_t *p, uint8_t *request, uCO_OD_Item_t *item)
{
	uCO_CanMessage_t reply = { 0 };

	savedDataSize = request[4];
	savedDataSize += request[5] << 8;
	savedDataSize += request[6] << 16;
	savedDataSize += request[7] << 24;

	/* Check oversized write attempt */
	if ((item->Type == UNSIGNED64) && (savedDataSize > sizeof(uint64_t)))
	{
		SDO_abort(p, request, UCANOPEN_SDO_ABORT_REASON_OUT_OF_MEMORY);
		return UCANOPEN_ERROR;
	}
	if ((item->Type == OCTET_STRING) && (savedDataSize > item->size))
	{
		SDO_abort(p, request, UCANOPEN_SDO_ABORT_REASON_OUT_OF_MEMORY);
		return UCANOPEN_ERROR;
	}
	if ((item->Type == VISIBLE_STRING) && (savedDataSize > item->size - 1))
	{
		SDO_abort(p, request, UCANOPEN_SDO_ABORT_REASON_OUT_OF_MEMORY);
		return UCANOPEN_ERROR;
	}

	savedToggleBit = true;
	savedDataOffset = 0;
	savedItem = item;

	/* Prepare reply */
	reply.CobId = UCANOPEN_COB_ID_TSDO | p->NodeId;
	reply.length = UCANOPEN_SDO_LENGTH;

	/* index and sub */
	reply.data[1] = request[1];
	reply.data[2] = request[2];
	reply.data[3] = request[3];

	reply.data[0] = UCANOPEN_SDO_WRITE_REPLY;

	return uco_send(p, &reply);
}

/**
 *
 */
static uCO_ErrorStatus_t
SDO_segmented_write_process(uCO_t *p, uint8_t *request)
{
	bool toggleBit = (request[0] >> 4) & 0x1;
//	bool lastSegment = (request[0]) & 0x1;
	size_t freeBytes = (request[0] >> 1) & 0x7;
	uint8_t *data = (uint8_t*) savedItem->address;

	/* Check Toggle Bit */
	if (toggleBit == savedToggleBit)
	{
		SDO_abort(p, request, UCANOPEN_SDO_ABORT_TOGGLE_BIT_NOT_ALTERED);
		return UCANOPEN_ERROR;
	}
	else
	{
		savedToggleBit = toggleBit;
	}

	/* Wipe old data at start */
	if (savedDataOffset == 0)
	{
		memset(data, 0x00, savedDataSize);
	}

	/* Write data */
	for (int i = 0; i < (7 - freeBytes); i++)
	{
		data[savedDataOffset] = request[1 + i];
		savedDataOffset++;
	}

	uCO_CanMessage_t reply = { 0 };

	reply.CobId = UCANOPEN_COB_ID_TSDO | p->NodeId;
	reply.length = UCANOPEN_SDO_LENGTH;

	reply.data[0] = UCANOPEN_SDO_SEGMENTED_WRITE_SEGMENT_REPLY;
	reply.data[0] |= toggleBit << 4;

	return uco_send(p, &reply);
}

/**
 * SDO Server
 */
uCO_ErrorStatus_t
uco_proceed_SDO_request(uCO_t *p, uint8_t *request)
{
	uCO_ErrorStatus_t result = UCANOPEN_ERROR;

	uCO_OD_Item_t *item;
	uint16_t index;
	uint8_t sub;

	/* (Segmented) READ next segment request */
	if (request[0] == UCANOPEN_SDO_SEGMENTED_READ_SEGMENT_T0_REQUEST ||
		request[0] == UCANOPEN_SDO_SEGMENTED_READ_SEGMENT_T1_REQUEST)
	{
		return SDO_segmented_read_process(p, request);
	}

	/* (Segmented) WRITE next segment request */
	if ((request[0] & 0xF0) == UCANOPEN_SDO_SEGMENTED_WRITE_SEGMENT_T0_REQUEST ||
		(request[0] & 0xF0) == UCANOPEN_SDO_SEGMENTED_WRITE_SEGMENT_T1_REQUEST)
	{
		return SDO_segmented_write_process(p, request);
	}

	/* Try to find Object Dictionary item by index */
	index = __UCANOPEN_16BIT_FROM_LE(request[1], request[2]);
	item = uco_find_OD_item(p, index, 0x00/* sub */);

	if (!item)
	{
		SDO_abort(p, request, UCANOPEN_SDO_ABORT_REASON_NOT_EXISTS);
		return UCANOPEN_ERROR;
	}

	/* Try to find Object Dictionary item by index and sub */
	sub = request[3];
	item = uco_find_OD_item(p, index, sub);

	if (!item)
	{
		SDO_abort(p, request, UCANOPEN_SDO_ABORT_REASON_UNKNOWN_SUB);
		return UCANOPEN_ERROR;
	}

	/* Switch CCS */

	/* (Expedited|Segmented) READ OD item request */
	if (request[0] == UCANOPEN_SDO_READ_REQUEST)
	{
		if (item->Type == UNSIGNED64 ||
			item->Type == OCTET_STRING ||
			item->Type == VISIBLE_STRING)
		{
			result = SDO_segmented_read_handshake(p, request, item);
		}
		else
		{
			result = SDO_expedited_read_reply(p, request, item);
		}
	}
	/* (Expedited) WRITE OD item request */
	else
	if (request[0] == UCANOPEN_SDO_EXPEDITED_WRITE_1BYTE_REQUEST ||
		request[0] == UCANOPEN_SDO_EXPEDITED_WRITE_2BYTES_REQUEST ||
		request[0] == UCANOPEN_SDO_EXPEDITED_WRITE_4BYTES_REQUEST)
	{
		if (item->Access == READ_WRITE)
		{
			result = SDO_expedited_write_reply(p, request, item);
		}
		else
		{
			SDO_abort(p, request, UCANOPEN_SDO_ABORT_REASON_READONLY);
			return UCANOPEN_ERROR;
		}
	}
	/* (Segmented) WRITE OD item request */
	else
	if (request[0] == UCANOPEN_SDO_SEGMENTED_WRITE_REQUEST)
	{
		if (item->Access == READ_WRITE)
		{
			result = SDO_segmented_write_handshake(p, request, item);
		}
		else
		{
			SDO_abort(p, request, UCANOPEN_SDO_ABORT_REASON_READONLY);
			return UCANOPEN_ERROR;
		}
	}
	else
	{
		return UCANOPEN_ERROR;
	}
	return result;
}

/**
 * SDO Client
 */
uCO_ErrorStatus_t
uco_proceed_SDO_reply(uCO_t *p, uint8_t *data)
{
	uCO_ErrorStatus_t result = UCANOPEN_ERROR;

//TODO

	return result;
}
