/*
 * uco_sync.c
 *
 *  Created on: Oct 16, 2020
 *      Author: sergi
 */

#include "uco_sync.h"
#include "uco_nmt.h"
#include "uco_pdo.h"

/**
 *
 */
uCO_ErrorStatus_t
uco_proceed_sync_request(uCO_t *p)
{
	/* Synchronized Heartbeat */
	if (p->NMT.heartbeatOnSync && p->NMT.HeartbeatTime == 0)
	{
		/* Update Nodeguard */
		p->NMT.GuardTimestamp = p->Timestamp;

		uco_send_heartbeat_message(p);
	}

	/* TPDO Synchronized transmit */
	for (int i = 0; i < sizeof(p->TPDO) / sizeof(p->TPDO[0]); i++)
	{
		if (p->TPDO[i].sendOnSync)
		{
			uco_tpdo_transmit(p, i + 1);
		}
	}
	return UCANOPEN_SUCCESS;
}
