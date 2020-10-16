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
	uint16_t dt;

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
	//		if (p->NMT.HeartbeatTime &&
	//			p->NMT.HeartbeatTimestamp + p->NMT.HeartbeatTime <= p->Timestamp)
	//		{
	//			uco_send_HEARTBEAT_message(p);
	//			p->NMT.HeartbeatTimestamp = p->Timestamp;
	//		}
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

	switch (p->NodeState) {
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
uco_proceed_nmt_command(uCO_t *p, uint8_t *pData, uint32_t len)
{
	uCO_ErrorStatus_t result = UCANOPEN_ERROR;

	//TODO

	return result;
}

/**
 *
 */
uCO_ErrorStatus_t
uco_proceed_sync_request(uCO_t *p, uint8_t *pData, uint32_t len)
{
	uCO_ErrorStatus_t result = UCANOPEN_ERROR;

	//TODO

	return result;
}
