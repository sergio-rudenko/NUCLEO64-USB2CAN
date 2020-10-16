/*
 * uco_nmt.c
 *
 *  Created on: Oct 12, 2020
 *      Author: sergi
 */

#include "uco_nmt.h"

/**
 *
 */
void
uco_nmt_on_tick(uCO_t *p)
{
	uint16_t dt, uplinkLifeTimeout;

	/* Heartbeat message */
	if (p->NMT.HeartbeatTime)
	{
		dt = p->Timestamp - p->NMT.HeartbeatTimestamp;

		if (dt >= p->NMT.HeartbeatTime)
		{
			uco_send_heartbeat_message(p);
			p->NMT.HeartbeatTimestamp = p->Timestamp;
		}
	}
	else if (p->NMT.GuardTime)
	{
		dt = p->Timestamp - p->NMT.GuardTimestamp;
		uplinkLifeTimeout = p->NMT.GuardTime * p->NMT.lifeTimeFactor;

		if (p->NMT.uplinkIsAlive != (dt < uplinkLifeTimeout))
		{
			p->NMT.uplinkIsAlive = (dt < uplinkLifeTimeout);
			uco_nmt_on_uplink_status(p, p->NMT.uplinkIsAlive);
		}
	}
}

/**
 *
 */
uCO_ErrorStatus_t
uco_send_heartbeat_message(uCO_t *p)
{
	uCO_CanMessage_t msg = { 0 };

	msg.CobId = UCANOPEN_COB_ID_HEARTBEAT | p->NodeId;
	msg.length = UCANOPEN_HEARTBEAT_MESSAGE_LENGTH;

	switch (p->NodeState)
	{
		case NODE_STATE_PREOPERATIONAL:
			msg.data[0] = UCANOPEN_HEARTBEAT_NODE_STATE_PREOPERATIONAL;
			break;

		case NODE_STATE_OPERATIONAL:
			msg.data[0] = UCANOPEN_HEARTBEAT_NODE_STATE_OPERATIONAL;
			break;

		case NODE_STATE_STOPPED:
			msg.data[0] = UCANOPEN_HEARTBEAT_NODE_STATE_STOPPED;
			break;

		default:
			return UCANOPEN_ERROR;
	}
	return uco_transmit_direct(p, &msg);
}

/**
 *
 */
uCO_ErrorStatus_t
uco_proceed_nmt_command(uCO_t *p, uint8_t *pData)
{
	uCO_ErrorStatus_t result = UCANOPEN_ERROR;

//TODO

	return result;
}

/**
 *
 */
__weak void
uco_nmt_on_uplink_status(uCO_t *p, bool alive)
{

}
