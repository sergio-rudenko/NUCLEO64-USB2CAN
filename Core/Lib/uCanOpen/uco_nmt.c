/*
 * uco_nmt.c
 *
 *  Created on: Oct 12, 2020
 *      Author: sergi
 */

#include "uco_nmt.h"

uCO_ErrorStatus_t
uco_send_HEARTBEAT_message(uCO_t *p)
{
	uCO_CanMessage_t message = { 0 };

	message.CobId = UCANOPEN_COB_ID_HEARTBEAT | p->NodeId;
	message.length = UCANOPEN_HEARTBEAT_MESSAGE_LENGTH;

	switch (p->NodeState) {
		case NODE_STATE_PREOPERATIONAL:
			message.data[0] = UCANOPEN_HEARTBEAT_NODE_STATE_PREOPERATIONAL;
			break;

		case NODE_STATE_OPERATIONAL:
			message.data[0] = UCANOPEN_HEARTBEAT_NODE_STATE_OPERATIONAL;
			break;

		case NODE_STATE_STOPPED:
			message.data[0] = UCANOPEN_HEARTBEAT_NODE_STATE_STOPPED;
			break;

		default:
			return UCANOPEN_ERROR;
	}
	return uco_send(p, &message);
}


uCO_ErrorStatus_t
uco_proceed_NMT_command(uCO_t *p, uint8_t *pData, uint32_t len)
{
	uCO_ErrorStatus_t result = UCANOPEN_ERROR;

	//TODO

	return result;
}

uCO_ErrorStatus_t
uco_proceed_SYNC_request(uCO_t *p, uint8_t *pData, uint32_t len)
{
	uCO_ErrorStatus_t result = UCANOPEN_ERROR;

	//TODO

	return result;
}
