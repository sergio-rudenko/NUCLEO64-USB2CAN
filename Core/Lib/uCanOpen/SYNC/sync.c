/*
 * SYNC/sync.c
 *
 *  Created on: Oct 16, 2020
 *      Author: sergi
 */

#include "sync.h"
#include "NMT/nmt.h"
#include "PDO/pdo.h"

/**
 *
 */
ErrorStatus
uco_proceed_sync_request(uCO_t *p)
{
	/* Synchronized Heartbeat */
	if (p->NMT.Heartbeat.sendOnSync &&
		p->NMT.Heartbeat.Timeout == 0)
	{
		/* Update Nodeguard */
		p->NMT.NodeGuard.Timestamp = p->Timestamp;

		uco_nmt_send_heartbeat_message(p);
	}

	/* TPDO Synchronized transmit */
	for (int i = 0; i < sizeof(p->TPDO) / sizeof(p->TPDO[0]); i++)
	{
		if (p->TPDO[i].sendOnSync)
		{
			uco_tpdo_transmit(p, i + 1);
		}
	}
	return SUCCESS;
}
