/*
 * NMT/nmt.c
 *
 *  Created on: Oct 12, 2020
 *      Author: sergi
 */

#include "nmt.h"

/**
 *
 */
void
uco_nmt_on_tick(uCO_t *p)
{
	uint16_t dt, uplinkLifeTimeout;

	/* Heartbeat message */
	if (p->NMT.Heartbeat.Timeout)
	{
		dt = p->Timestamp - p->NMT.Heartbeat.Timestamp;

		if (dt >= p->NMT.Heartbeat.Timeout)
		{
			uco_nmt_send_heartbeat_message(p);
			p->NMT.Heartbeat.Timestamp = p->Timestamp;
		}
	}
	else if (p->NMT.NodeGuard.Timeout)
	{
		dt = p->Timestamp - p->NMT.NodeGuard.Timestamp;
		uplinkLifeTimeout = p->NMT.NodeGuard.Timeout * p->NMT.NodeGuard.lifeTimeFactor;

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
ErrorStatus
uco_nmt_send_boot_message(uCO_t *p)
{
	uCO_CanMessage_t umsg = { 0 };

	umsg.CobId = UCANOPEN_COB_ID_HEARTBEAT | p->NodeId;
	umsg.length = UCANOPEN_HEARTBEAT_MESSAGE_LENGTH;
	umsg.data[0] = UCANOPEN_HEARTBEAT_BOOTUP_MESSAGE;

	return uco_transmit_direct(p, &umsg);
}

/**
 *
 */
ErrorStatus
uco_nmt_send_heartbeat_message(uCO_t *p)
{
	uCO_CanMessage_t umsg = { 0 };

	umsg.CobId = UCANOPEN_COB_ID_HEARTBEAT | p->NodeId;
	umsg.length = UCANOPEN_HEARTBEAT_MESSAGE_LENGTH;

	switch (p->NodeState)
	{
		case NODE_STATE_PREOPERATIONAL:
			umsg.data[0] = UCANOPEN_HEARTBEAT_STATE_PREOPERATIONAL;
			break;

		case NODE_STATE_OPERATIONAL:
			umsg.data[0] = UCANOPEN_HEARTBEAT_STATE_OPERATIONAL;
			break;

		case NODE_STATE_STOPPED:
			umsg.data[0] = UCANOPEN_HEARTBEAT_STATE_STOPPED;
			break;

		default:
			return ERROR;
	}
	return uco_transmit_direct(p, &umsg);
}

/**
 *
 */
ErrorStatus
uco_proceed_nmt_command(uCO_t *p, uint8_t cmd, uCO_NodeId_t addr)
{
	ErrorStatus result = SUCCESS;

	switch (cmd)
	{
		case UCANOPEN_NMT_CS_START_REMOTE_NODE:
			p->NodeState = NODE_STATE_OPERATIONAL;
			break;

		case UCANOPEN_NMT_CS_STOP_REMOTE_NODE:
			p->NodeState = NODE_STATE_STOPPED;
			break;

		case UCANOPEN_NMT_CS_ENTER_PRE_OPERATIONAL:
			p->NodeState = NODE_STATE_PREOPERATIONAL;
			break;

		case UCANOPEN_NMT_CS_RESET_NODE:
			NVIC_SystemReset();
			break;

		case UCANOPEN_NMT_CS_RESET_COMMUNICATION:
			p->NodeState = NODE_STATE_INITIALIZATION;
			p->NodeId = UCANOPEN_NODE_ID_UNCONFIGURED;
			HAL_CAN_Stop(&hcan);
			HAL_CAN_Start(&hcan);
			break;

		default:
			/* Can`t proceed command...*/
			result = ERROR;
			break;
	}
	return result;
}

/* Master API */

/**
 *
 */
ErrorStatus
uco_nmt_master_send_command(uCO_t *p, uint8_t Cmd, uCO_NodeId_t Addr)
{
	uCO_CanMessage_t umsg = { 0 };
	umsg.CobId = UCANOPEN_COB_ID_NMT;
	umsg.length = UCANOPEN_NMT_MESSAGE_LENGTH;

	umsg.data[0] = (uint8_t) Cmd;
	umsg.data[1] = (uint8_t) Addr;

	return uco_send(p, &umsg);
}

/* callback functions */

/**
 *
 */
__weak void
uco_nmt_on_uplink_status(uCO_t *p, bool alive)
{

}
