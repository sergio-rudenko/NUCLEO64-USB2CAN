/*
 * PDO/pdo.c
 *
 *  Created on: Oct 15, 2020
 *      Author: sergi
 */

#include "pdo.h"

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
		if (p->TPDO[i].Timeout)
		{
			dt = p->Timestamp - p->TPDO[i].Timestamp;

			if (dt >= p->TPDO[i].Timeout)
			{
				uco_tpdo_transmit(p, i + 1);
				p->TPDO[i].Timestamp = p->Timestamp;
			}
		}
	}
}

/**
 *
 */
ErrorStatus
uco_tpdo_transmit(uCO_t *p, int num)
{
	ErrorStatus result = ERROR;
	uCO_CanMessage_t message = { 0 };
	uCO_TPDO_t *tpdo;

	if (p->NodeState != NODE_STATE_OPERATIONAL)
	{
		return ERROR;
	}

	if (uco_tpdo_prepare_data(p, num) == SUCCESS)
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

			default:
				return ERROR;
		}

		if (tpdo->data.address && tpdo->data.size <= 8)
		{
			message.CobId |= p->NodeId;
			message.length = tpdo->data.size;

			memcpy(message.data,
					tpdo->data.address,
					tpdo->data.size);

			result = uco_send(p, &message);
		}
	}
	return result;
}

__weak ErrorStatus
uco_tpdo_prepare_data(uCO_t *p, int num)
{
	ErrorStatus result = ERROR;

	if (p->TPDO[num].data.address &&
		p->TPDO[num].data.size <= 8)
	{
		result = SUCCESS;
	}
	return result;
}

