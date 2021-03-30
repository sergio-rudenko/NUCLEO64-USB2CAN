/*
 * SDO/sdo.c
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

#include "sdo.h"

/**
 *
 */
static void
sdo_reset_server_process(uCO_t *p)
{
	p->SDO.segmented.offset = 0;
	p->SDO.segmented.size = 0;

	p->SDO.Item = NULL;
	p->SDO.index = 0;
	p->SDO.sub = 0;

	p->SDO.Timeout = 0;
}

/**
 *
 */
static ErrorStatus
sdo_expedited_read_reply(uCO_t *p)
{
	uCO_CanMessage_t reply = { 0 };
	uint32_t value;

	/* data prepare */
	if (uco_sdo_prepare_data(p, p->SDO.index, p->SDO.sub) != SUCCESS)
	{
		uco_sdo_abort(p, UCANOPEN_SDO_ABORT_GENERAL_ERROR);
		return ERROR;
	}

	reply.CobId = UCANOPEN_COB_ID_TSDO | p->NodeId;
	reply.length = UCANOPEN_SDO_LENGTH;

	/* index and sub */
	reply.data[1] = (p->SDO.index) & 0xFF;
	reply.data[2] = (p->SDO.index >> 8) & 0xFF;
	reply.data[3] = (p->SDO.sub);

	switch (p->SDO.Item->Type)
	{
		case UNSIGNED8:
			reply.data[0] = UCANOPEN_SDO_EXPEDITED_READ_1BYTE_REPLY;

			value = *((uint8_t*) p->SDO.Item->address);

			reply.data[4] = (value) & 0xFF;
			break;

		case UNSIGNED16:
			reply.data[0] = UCANOPEN_SDO_EXPEDITED_READ_2BYTES_REPLY;

			value = *((uint16_t*) p->SDO.Item->address);

			reply.data[4] = (value) & 0xFF;
			reply.data[5] = (value >> 8) & 0xFF;
			break;

		case UNSIGNED32:
			reply.data[0] = UCANOPEN_SDO_EXPEDITED_READ_4BYTES_REPLY;

			value = *((uint32_t*) p->SDO.Item->address);

			reply.data[4] = (value) & 0xFF;
			reply.data[5] = (value >> 8) & 0xFF;
			reply.data[6] = (value >> 16) & 0xFF;
			reply.data[7] = (value >> 24) & 0xFF;
			break;

		default:
			return ERROR;
	}

	if (uco_send(p, &reply) == SUCCESS)
	{
		uco_sdo_on_read_success(p, p->SDO.index, p->SDO.sub);
		return SUCCESS;
	}
	return ERROR;
}

/**
 *
 */
static ErrorStatus
sdo_expedited_write_reply(uCO_t *p, uint8_t clientCommand, uint8_t *clientData)
{
	uCO_CanMessage_t reply = { 0 };
	uint32_t value;

	reply.CobId = UCANOPEN_COB_ID_TSDO | p->NodeId;
	reply.length = UCANOPEN_SDO_LENGTH;

	/* index and sub */
	reply.data[1] = (p->SDO.index) & 0xFF;
	reply.data[2] = (p->SDO.index >> 8) & 0xFF;
	reply.data[3] = (p->SDO.sub);

	reply.data[0] = UCANOPEN_SDO_WRITE_REPLY;

	switch (clientCommand)
	{
		case UCANOPEN_SDO_EXPEDITED_WRITE_1BYTE_REQUEST:
			if (p->SDO.Item->Type == UNSIGNED8)
			{
				value = clientData[0];
				*((uint8_t*) p->SDO.Item->address) = value;
			}
			else
			{
				uco_sdo_abort(p, UCANOPEN_SDO_ABORT_REASON_OUT_OF_MEMORY);
				return ERROR;
			}
			break;

		case UCANOPEN_SDO_EXPEDITED_WRITE_2BYTES_REQUEST:
			if (p->SDO.Item->Type == UNSIGNED16)
			{
				value = clientData[0];
				value += clientData[1] << 8;
				*((uint16_t*) p->SDO.Item->address) = value;
			}
			else
			{
				uco_sdo_abort(p, UCANOPEN_SDO_ABORT_REASON_OUT_OF_MEMORY);
				return ERROR;
			}
			break;

		case UCANOPEN_SDO_EXPEDITED_WRITE_4BYTES_REQUEST:
			if (p->SDO.Item->Type == UNSIGNED32)
			{
				value = clientData[0];
				value += clientData[1] << 8;
				value += clientData[2] << 16;
				value += clientData[3] << 24;
				*((uint32_t*) p->SDO.Item->address) = value;
			}
			else
			{
				uco_sdo_abort(p, UCANOPEN_SDO_ABORT_REASON_OUT_OF_MEMORY);
				return ERROR;
			}
			break;

		default:
			return ERROR;
	}

	/* validate check */
	if (uco_sdo_validate_data(p, p->SDO.index, p->SDO.sub) != SUCCESS)
	{
		uco_sdo_abort(p, UCANOPEN_SDO_ABORT_GENERAL_ERROR);
		return ERROR;
	}

	if (uco_send(p, &reply) == SUCCESS)
	{
		uco_sdo_on_write_success(p, p->SDO.index, p->SDO.sub);
		return SUCCESS;
	}
	return ERROR;
}

/**
 *
 */
static ErrorStatus
sdo_segmented_read_handshake(uCO_t *p)
{
	uCO_CanMessage_t reply = { 0 };

	/* data prepare */
	if (uco_sdo_prepare_data(p, p->SDO.index, p->SDO.sub) != SUCCESS)
	{
		uco_sdo_abort(p, UCANOPEN_SDO_ABORT_GENERAL_ERROR);
		return ERROR;
	}

	reply.CobId = UCANOPEN_COB_ID_TSDO | p->NodeId;
	reply.length = UCANOPEN_SDO_LENGTH;

	switch (p->SDO.Item->Type)
	{
		case UNSIGNED64:
			p->SDO.segmented.size = sizeof(uint64_t);
			break;

		case OCTET_STRING:
			p->SDO.segmented.size = uco_sdo_get_octet_string_size(p, p->SDO.index, p->SDO.sub);
			if (p->SDO.segmented.size == 0)
				p->SDO.segmented.size = p->SDO.Item->size;
			break;

		case VISIBLE_STRING:
			p->SDO.segmented.size = uco_sdo_get_visible_string_length(p, p->SDO.index, p->SDO.sub);
			if (p->SDO.segmented.size == 0)
				p->SDO.segmented.size = strlen((char*) p->SDO.Item->address);
			break;

		default:
			uco_sdo_abort(p, UCANOPEN_SDO_ABORT_REASON_UNSUPPORTED);
			return ERROR;
	}

	p->SDO.segmented.toggleBit = true;
	p->SDO.segmented.reading = true;
	p->SDO.segmented.offset = 0;

	reply.data[0] = UCANOPEN_SDO_SEGMENTED_READ_REPLY;

	/* index and sub */
	reply.data[1] = (p->SDO.index) & 0xFF;
	reply.data[2] = (p->SDO.index >> 8) & 0xFF;
	reply.data[3] = (p->SDO.sub);

	/* size of data */
	reply.data[4] = (p->SDO.segmented.size) & 0xFF;
	reply.data[5] = (p->SDO.segmented.size >> 8) & 0xFF;
	reply.data[6] = (p->SDO.segmented.size >> 16) & 0xFF;
	reply.data[7] = (p->SDO.segmented.size >> 24) & 0xFF;

	/* set timeout and update timestamp */
	p->SDO.Timeout = UCANOPEN_SDO_DEFAULT_TIMEOUT;
	p->SDO.Timestamp = p->Timestamp;

	return uco_send(p, &reply);
}

/**
 *
 */
static ErrorStatus
sdo_segmented_read_process(uCO_t *p, uint8_t clientCommand)
{
	uCO_CanMessage_t reply = { 0 };

	bool toggleBit = (clientCommand >> 4) & 0x1;
	uint8_t *data = (uint8_t*) p->SDO.Item->address;

	bool lastSegment;
	size_t freeBytes;

	reply.CobId = UCANOPEN_COB_ID_TSDO | p->NodeId;
	reply.length = UCANOPEN_SDO_LENGTH;

	/* Check process flow */
	if (p->SDO.Item == NULL || p->SDO.segmented.reading == false)
	{
		uco_sdo_abort(p, UCANOPEN_SDO_ABORT_REASON_UNKNOWN_COMMAND);
		return ERROR;
	}

	/* Check Toggle Bit */
	if (toggleBit == p->SDO.segmented.toggleBit)
	{
		uco_sdo_abort(p, UCANOPEN_SDO_ABORT_TOGGLE_BIT_NOT_ALTERED);
		return ERROR;
	}
	else
	{
		p->SDO.segmented.toggleBit = toggleBit;
	}

	/* Check remaining data */
	if (p->SDO.segmented.offset >= p->SDO.segmented.size)
	{
		uco_sdo_abort(p, UCANOPEN_SDO_ABORT_REASON_OUT_OF_MEMORY);
		return ERROR;
	}

	if (p->SDO.segmented.size - p->SDO.segmented.offset <= 7)
	{
		freeBytes = (7 - (p->SDO.segmented.size - p->SDO.segmented.offset));
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
		reply.data[1 + i] = data[p->SDO.segmented.offset];
		p->SDO.segmented.offset++;
	}

	/* Server CS */
	reply.data[0] = (toggleBit << 4) | (freeBytes << 1) | lastSegment;

	if (uco_send(p, &reply) == SUCCESS)
	{
		if (lastSegment)
		{
			uco_sdo_on_read_success(p, p->SDO.index, p->SDO.sub);
			sdo_reset_server_process(p);
		}
		else
		{
			/* update timeout and timestamp */
			p->SDO.Timeout = UCANOPEN_SDO_DEFAULT_TIMEOUT;
			p->SDO.Timestamp = p->Timestamp;
		}
		return SUCCESS;
	}
	return ERROR;
}

/**
 *
 */
static ErrorStatus
sdo_segmented_write_handshake(uCO_t *p)
{
	uCO_CanMessage_t reply = { 0 };

	/* Check oversized write attempt */
	if ((p->SDO.Item->Type == UNSIGNED64) &&
		(p->SDO.segmented.size > sizeof(uint64_t)))
	{
		uco_sdo_abort(p, UCANOPEN_SDO_ABORT_REASON_OUT_OF_MEMORY);
		return ERROR;
	}
	if ((p->SDO.Item->Type == OCTET_STRING) &&
		(p->SDO.segmented.size > p->SDO.Item->size))
	{
		uco_sdo_abort(p, UCANOPEN_SDO_ABORT_REASON_OUT_OF_MEMORY);
		return ERROR;
	}
	if ((p->SDO.Item->Type == VISIBLE_STRING) &&
		(p->SDO.segmented.size > p->SDO.Item->size - 1))
	{
		uco_sdo_abort(p, UCANOPEN_SDO_ABORT_REASON_OUT_OF_MEMORY);
		return ERROR;
	}

	p->SDO.segmented.reading = false;
	p->SDO.segmented.toggleBit = true;
	p->SDO.segmented.offset = 0;

	/* Prepare reply */
	reply.CobId = UCANOPEN_COB_ID_TSDO | p->NodeId;
	reply.length = UCANOPEN_SDO_LENGTH;

	reply.data[0] = UCANOPEN_SDO_WRITE_REPLY;

	/* index and sub */
	reply.data[1] = (p->SDO.index) & 0xFF;
	reply.data[2] = (p->SDO.index >> 8) & 0xFF;
	reply.data[3] = p->SDO.sub;

	/* set timeout and update timestamp */
	p->SDO.Timeout = UCANOPEN_SDO_DEFAULT_TIMEOUT;
	p->SDO.Timestamp = p->Timestamp;

	return uco_send(p, &reply);
}

/**
 *
 */
static ErrorStatus
sdo_segmented_write_process(uCO_t *p, uint8_t clientCommand, uint8_t *clientData)
{
	bool toggleBit = (clientCommand >> 4) & 0x1;
	bool lastSegment = (clientCommand) & 0x1;
	size_t freeBytes = (clientCommand >> 1) & 0x7;

	uint8_t *data = (uint8_t*) p->SDO.Item->address;

	/* Check process flow */
	if (p->SDO.Item == NULL || p->SDO.segmented.reading == true)
	{
		uco_sdo_abort(p, UCANOPEN_SDO_ABORT_REASON_UNKNOWN_COMMAND);
		return ERROR;
	}

	/* Check Toggle Bit */
	if (toggleBit == p->SDO.segmented.toggleBit)
	{
		uco_sdo_abort(p, UCANOPEN_SDO_ABORT_TOGGLE_BIT_NOT_ALTERED);
		return ERROR;
	}
	else
	{
		p->SDO.segmented.toggleBit = toggleBit;
	}

	/* Wipe old data at start */
	if (p->SDO.segmented.offset == 0)
	{
		memset(data, 0x00, p->SDO.segmented.size);
	}

	/* Check remaining data */
	if (p->SDO.segmented.offset + (7 - freeBytes) > p->SDO.segmented.size)
	{
		uco_sdo_abort(p, UCANOPEN_SDO_ABORT_REASON_OUT_OF_MEMORY);
		return ERROR;
	}

	/* Write data */
	for (int i = 0; i < (7 - freeBytes); i++)
	{
		data[p->SDO.segmented.offset] = clientData[i];
		p->SDO.segmented.offset++;
	}

	uCO_CanMessage_t reply = { 0 };

	reply.CobId = UCANOPEN_COB_ID_TSDO | p->NodeId;
	reply.length = UCANOPEN_SDO_LENGTH;

	reply.data[0] = UCANOPEN_SDO_SEGMENTED_WRITE_SEGMENT_REPLY;
	reply.data[0] |= toggleBit << 4;

	if (lastSegment)
	{
		/* validate check */
		if (uco_sdo_validate_data(p, p->SDO.index, p->SDO.sub) != SUCCESS)
		{
			uco_sdo_abort(p, UCANOPEN_SDO_ABORT_GENERAL_ERROR);
			return ERROR;
		}
		else
		{
			uco_sdo_on_write_success(p, p->SDO.index, p->SDO.sub);
			sdo_reset_server_process(p);
		}
	}
	else
	{
		/* update timeout and timestamp */
		p->SDO.Timeout = UCANOPEN_SDO_DEFAULT_TIMEOUT;
		p->SDO.Timestamp = p->Timestamp;
	}
	return uco_send(p, &reply);
}

/**
 *
 */
void
uco_sdo_on_tick(uCO_t *p)
{
	uint16_t dt;

	/* SDO process timeout */
	if (p->SDO.Timeout)
	{
		dt = p->Timestamp - p->SDO.Timestamp;

		if (dt >= p->SDO.Timeout)
		{
//FIXME!	uco_SDO_abort(p, NULL, UCANOPEN_SDO_ABORT_REASON_TIMED_OUT);
			p->SDO.Timestamp = p->Timestamp;
			p->SDO.Timeout = 0;
		}
	}
}

/**
 *
 */
ErrorStatus
uco_sdo_abort(uCO_t *p, uint32_t reason)
{
	uCO_CanMessage_t reply = { 0 };

	reply.CobId = UCANOPEN_COB_ID_TSDO | p->NodeId;
	reply.length = UCANOPEN_SDO_LENGTH;

	reply.data[0] = UCANOPEN_SDO_ABORT_CS;

	/* index and sub */
	reply.data[1] = (p->SDO.index) & 0xFF;
	reply.data[2] = (p->SDO.index >> 8) & 0xFF;
	reply.data[3] = p->SDO.sub;

	/* reason */
	reply.data[4] = (reason) & 0xFF;
	reply.data[5] = (reason >> 8) & 0xFF;
	reply.data[6] = (reason >> 16) & 0xFF;
	reply.data[7] = (reason >> 24) & 0xFF;

	/* abort callback */
	uco_sdo_on_abort(p, p->SDO.index, p->SDO.sub, reason);

	sdo_reset_server_process(p);
	return uco_send(p, &reply);
}

/**
 * SDO Server
 */
ErrorStatus
uco_proceed_sdo_request(uCO_t *p, uint8_t *request)
{
	ErrorStatus result = ERROR;

	uint8_t clientCommand = request[0];
	uint8_t ccs = clientCommand & 0xF0;

	/* Abort command from client */
	if (ccs == UCANOPEN_SDO_ABORT_CS)
	{
		sdo_reset_server_process(p);
		return SUCCESS;
	}

	/* (Segmented) READ next segment request */
	if (ccs == UCANOPEN_SDO_SEGMENTED_READ_SEGMENT_T0_REQUEST ||
		ccs == UCANOPEN_SDO_SEGMENTED_READ_SEGMENT_T1_REQUEST)
	{
		return sdo_segmented_read_process(p, clientCommand);
	}

	/* (Segmented) WRITE next segment request */
	if (ccs == UCANOPEN_SDO_SEGMENTED_WRITE_SEGMENT_T0_REQUEST ||
		ccs == UCANOPEN_SDO_SEGMENTED_WRITE_SEGMENT_T1_REQUEST)
	{
		return sdo_segmented_write_process(p, clientCommand, &request[1]);
	}

	/* Get requested index and sub */
	p->SDO.index = request[1] | (request[2] << 8);
	p->SDO.sub = request[3];

	/* Try to find Object Dictionary item by index */
	p->SDO.Item = uco_find_od_item(p, p->SDO.index, 0x00/* sub */);

	if (!p->SDO.Item)
	{
		uco_sdo_abort(p, UCANOPEN_SDO_ABORT_REASON_NOT_EXISTS);
		return ERROR;
	}

	/* Try to find Object Dictionary item by index and sub */
	p->SDO.Item = uco_find_od_item(p, p->SDO.index, p->SDO.sub);

	if (!p->SDO.Item)
	{
		uco_sdo_abort(p, UCANOPEN_SDO_ABORT_REASON_UNKNOWN_SUB);
		return ERROR;
	}

	/* Switch CCS */

	/* (Expedited|Segmented) READ OD item request */
	if (request[0] == UCANOPEN_SDO_READ_REQUEST)
	{
		if (p->SDO.Item->Access == WRITE_ONLY)
		{
			uco_sdo_abort(p, UCANOPEN_SDO_ABORT_REASON_WRITEONLY);
			result = ERROR;
		}

		if (p->SDO.Item->Type == UNSIGNED64 ||
			p->SDO.Item->Type == OCTET_STRING ||
			p->SDO.Item->Type == VISIBLE_STRING)
		{
			result = sdo_segmented_read_handshake(p);
		}
		else
		{
			result = sdo_expedited_read_reply(p);
		}
	}
	/* (Expedited) WRITE OD item request */
	else
	if (request[0] == UCANOPEN_SDO_EXPEDITED_WRITE_1BYTE_REQUEST ||
		request[0] == UCANOPEN_SDO_EXPEDITED_WRITE_2BYTES_REQUEST ||
		request[0] == UCANOPEN_SDO_EXPEDITED_WRITE_4BYTES_REQUEST)
	{
		if (p->SDO.Item->Access != READ_ONLY)
		{
			result = sdo_expedited_write_reply(p, clientCommand, &request[4]);
		}
		else
		{
			uco_sdo_abort(p, UCANOPEN_SDO_ABORT_REASON_READONLY);
			result = ERROR;
		}
	}
	/* (Segmented) WRITE OD item request */
	else
	if (request[0] == UCANOPEN_SDO_SEGMENTED_WRITE_REQUEST)
	{
		if (p->SDO.Item->Access != READ_ONLY)
		{
			/* requested size */
			p->SDO.segmented.size = request[4];
			p->SDO.segmented.size += request[5] << 8;
			p->SDO.segmented.size += request[6] << 16;
			p->SDO.segmented.size += request[7] << 24;

			result = sdo_segmented_write_handshake(p);
		}
		else
		{
			uco_sdo_abort(p, UCANOPEN_SDO_ABORT_REASON_READONLY);
			result = ERROR;
		}
	}
	else
	{
		result = ERROR;
	}
	return result;
}

/**
 * SDO Client
 */
ErrorStatus
uco_proceed_sdo_reply(uCO_t *p, uint8_t *data)
{
	ErrorStatus result = ERROR;

//TODO

	return result;
}

/* callback`s */

__weak size_t
uco_sdo_get_octet_string_size(uCO_t *p, uint16_t index, uint8_t sub)
{
	size_t size = 0;
	uOD_Item_t *item = uco_find_od_item(p, index, sub);

	if (item && item->Type == OCTET_STRING)
	{
		size = item->size;
	}
	return size;
}

__weak size_t
uco_sdo_get_visible_string_length(uCO_t *p, uint16_t index, uint8_t sub)
{
	size_t length = 0;
	uOD_Item_t *item = uco_find_od_item(p, index, sub);

	if (item && item->Type == VISIBLE_STRING)
	{
		length = strlen((char*) item->address);
	}
	return length;
}

/**
 *
 */
__weak ErrorStatus
uco_sdo_prepare_data(uCO_t *p, uint16_t index, uint8_t sub)
{
	return SUCCESS;
}

/**
 *
 */
__weak ErrorStatus
uco_sdo_validate_data(uCO_t *p, uint16_t index, uint8_t sub)
{
	return SUCCESS;
}

/**
 *
 */
__weak void
uco_sdo_on_write_success(uCO_t *p, uint16_t index, uint8_t sub)
{
}

/**
 *
 */
__weak void
uco_sdo_on_read_success(uCO_t *p, uint16_t index, uint8_t sub)
{
}

/**
 *
 */
__weak void
uco_sdo_on_abort(uCO_t *p, uint16_t index, uint8_t sub, uint32_t reason)
{
}
