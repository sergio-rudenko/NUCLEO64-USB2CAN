/*
 * uco_pdo.c
 *
 *  Created on: Oct 15, 2020
 *      Author: sergi
 */

#include "uco_pdo.h"

/**
 *
 */
void
uco_pdo_on_tick(uCO_t *p)
{
	uint16_t dt;

	/* TPDO periodic transmit */
	for (int i = 0; i < sizeof(p->TPDO) / sizeof(p->TPDO[0]); i++)
	{
		if (p->TPDO[i].InhibitTime)
		{
			continue;
		}

		if (p->TPDO[i].EventTime)
		{
			dt = p->Timestamp - p->TPDO[i].EventTimestamp;

			if (dt >= p->TPDO[i].EventTime)
			{
				uco_tpdo_transmit(p, i);
				p->TPDO[i].EventTimestamp = p->Timestamp;
			}
		}
	}
}

uCO_ErrorStatus_t
uco_tpdo_init(uCO_TPDO_t *Tpdo, void *address, size_t size)
{
	uCO_ErrorStatus_t result = UCANOPEN_ERROR;

	if (size <= 8)
	{
		Tpdo->EventTime = 0;
		Tpdo->InhibitTime = 0;

		Tpdo->data.address = address;
		Tpdo->data.size = size;

		result = UCANOPEN_SUCCESS;
	}
	return result;
}

/**
 *
 */
uCO_ErrorStatus_t
uco_tpdo_transmit(uCO_t *p, int num)
{
	uCO_ErrorStatus_t result = UCANOPEN_ERROR;
	uCO_CanMessage_t message = { 0 };
	uCO_TPDO_t *tpdo;

	if (p->NodeState != NODE_STATE_OPERATIONAL)
	{
		return UCANOPEN_ERROR;
	}
	if (num < 0 || num > 4)
	{
		return UCANOPEN_ERROR;
	}

	if (uco_tpdo_prepare_data(p, num) == UCANOPEN_SUCCESS)
	{
		switch (num)
		{
			case 1:
				message.CobId = UCANOPEN_COB_ID_TPDO_1;
				tpdo = &p->TPDO[0];
				break;
			case 2:
				message.CobId = UCANOPEN_COB_ID_TPDO_2;
				tpdo = &p->TPDO[1];
				break;
			case 3:
				message.CobId = UCANOPEN_COB_ID_TPDO_3;
				tpdo = &p->TPDO[2];
				break;
			case 4:
				message.CobId = UCANOPEN_COB_ID_TPDO_4;
				tpdo = &p->TPDO[3];
				break;
		}

		if (tpdo->data.address && tpdo->data.size <= 8)
		{
			message.CobId |= p->NodeId;
			message.length = tpdo->data.size;
			memcpy(message.data, tpdo->data.address, tpdo->data.size);

			result = uco_send(p, &message);
		}
	}
	return result;
}

__weak uCO_ErrorStatus_t
uco_tpdo_prepare_data(uCO_t *p, int num)
{
	uCO_ErrorStatus_t result = UCANOPEN_ERROR;
	return result;
}

