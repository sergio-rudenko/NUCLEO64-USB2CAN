/*
 * ucanopen.c
 *
 *  Created on: Oct 9, 2020
 *      Author: sergi
 */

#include "ucanopen.h"

/* private variables */
static uint8_t rxData[UCANOPEN_RX_BUFFER_SIZE];
static uint8_t txData[UCANOPEN_TX_BUFFER_SIZE];

static rBuffer_t rxRingBuffer;
static rBuffer_t txRingBuffer;

/* instance */
uCO_t uCO;

/**
 *
 */
static ErrorStatus
pop_message(rBuffer_t *buf, uCO_CanMessage_t *msg)
{
	ErrorStatus result = ERROR;
	size_t numBytes = 2/* CobId */+ 1/* len */;

	if (ring_buffer_available(buf) >= numBytes)
	{
		/* COB ID */
		msg->CobId = ring_buffer_read(buf) << 8;
		msg->CobId += ring_buffer_read(buf);

		/* Data length */
		msg->length = ring_buffer_read(buf);

		/* Data */
		ring_buffer_read_bytes(buf, msg->data, msg->length);

		result = SUCCESS;
	}
	else
	{
		/* garbage in buffer */
		numBytes = ring_buffer_available(buf);
		for (int i = 0; i < numBytes; i++)
		{
			ring_buffer_read(buf);
		}
	}
	return result;
}

/**
 *
 */
static ErrorStatus
push_message(rBuffer_t *buf, uCO_CanMessage_t *msg)
{
	/* COB ID */
	ring_buffer_write(buf, (msg->CobId >> 8) & 0x7F);
	ring_buffer_write(buf, (msg->CobId) & 0xFF);

	/* Data length */
	ring_buffer_write(buf, msg->length);

	/* Data */
	ring_buffer_write_bytes(buf, msg->data, msg->length);

	return SUCCESS;
}

/**
 *
 */
static ErrorStatus
proceed_incoming(uCO_t *p, uCO_CanMessage_t *msg)
{
	ErrorStatus result = ERROR;
	uCO_NodeId_t NodeId;

	/* switch protocol */

	/**
	 * NMT Command
	 */
	if (__IS_UCANOPEN_COB_ID_NMT(msg->CobId) &&
		msg->length == UCANOPEN_NMT_MESSAGE_LENGTH)
	{
		NodeId = msg->data[1];

		if (NodeId == p->NodeId ||
			NodeId == UCANOPEN_NMT_BROADCAST_MESSAGE)
		{
			result = uco_proceed_nmt_command(p, msg->data[0], msg->data[1]);
		}
	}
	/**
	 * LSS [Master -> Slave] Messages
	 */
	else
	if (__IS_UCANOPEN_COB_ID_LSS_REQUEST(msg->CobId) &&
		msg->length == UCANOPEN_LSS_LENGTH /* Slave */)
	{
		result = uco_proceed_lss_request(p, msg->data);
	}
	/**
	 * LSS [Slave -> Master] Messages
	 */
	else
	if (__IS_UCANOPEN_COB_ID_LSS_RESPONCE(msg->CobId) &&
		msg->length == UCANOPEN_LSS_LENGTH /* Master */)
	{
		result = uco_proceed_lss_responce(p, msg->data);
	}
	/**
	 * SYNC Message
	 */
	else
	if (__IS_UCANOPEN_COB_ID_SYNC(msg->CobId) &&
		msg->length == UCANOPEN_SYNC_MESSAGE_LENGTH)
	{
		result = uco_proceed_sync_request(p);
	}
	/**
	 * EMCY Message
	 */
	else
	if (__IS_UCANOPEN_COB_ID_EMCY(msg->CobId) &&
		msg->length == UCANOPEN_EMCY_MESSAGE_LENGTH)
	{
		NodeId = __UCANOPEN_NODE_ID_FROM_COB_ID(msg->CobId);

		if (NodeId == 0 /* Broadcast EMCY */)
		{
			result = uco_proceed_emcy_message(p, msg->data);
		}
	}
	/**
	 * TIME Message
	 */
	else
	if (__IS_UCANOPEN_COB_ID_TIME(msg->CobId) &&
		msg->length == UCANOPEN_TIME_MESSAGE_LENGTH)
	{
		result = uco_proceed_time_message(p, msg->data);
	}
	/**
	 * SDO Server -> Client
	 */
	else
	if (__IS_UCANOPEN_COB_ID_TSDO(msg->CobId) &&
		msg->length == UCANOPEN_SDO_LENGTH)
	{
		/* Check address */
		if (__UCANOPEN_NODE_ID_FROM_COB_ID(msg->CobId) != p->NodeId)
			return ERROR;

		/* Check node status for SDO processing */
		if (p->NodeState == NODE_STATE_STOPPED ||
			p->NodeState == NODE_STATE_INITIALIZATION)
			return ERROR;

		result = uco_proceed_sdo_reply(p, msg->data);
	}
	/**
	 * SDO Client -> Server
	 */
	else
	if (__IS_UCANOPEN_COB_ID_RSDO(msg->CobId) &&
		msg->length == UCANOPEN_SDO_LENGTH)
	{
		/* Check address */
		if (__UCANOPEN_NODE_ID_FROM_COB_ID(msg->CobId) != p->NodeId)
			return ERROR;

		/* Check node status for SDO processing */
		if (p->NodeState == NODE_STATE_STOPPED ||
			p->NodeState == NODE_STATE_INITIALIZATION)
			return ERROR;

		result = uco_proceed_sdo_request(p, msg->data);
	}
	else
	if (__IS_UCANOPEN_COB_ID_TPDO(msg->CobId))
	{
		//TODO
	}
	else
	if (__IS_UCANOPEN_COB_ID_RPDO(msg->CobId))
	{
		//TODO
	}
	else
	{
		//TODO
	}
	return result;
}

/**
 *
 */
void
uco_run(uCO_t *p)
{
	uCO_CanMessage_t msg;

	/* check timestamp and timeouts */
	if (p->ticks != HAL_GetTick())
	{
		uint16_t delta = HAL_GetTick() - p->ticks;
		p->Timestamp += delta;
		p->ticks += delta;

		uco_nmt_on_tick(p);
		uco_lss_on_tick(p);
		uco_pdo_on_tick(p);
		uco_sdo_on_tick(p);
	}

	/* check and proceed incoming messages */
	while (ring_buffer_available(p->rxBuf))
	{
		/* COB ID */
		msg.CobId = ring_buffer_read(p->rxBuf) << 8;
		msg.CobId += ring_buffer_read(p->rxBuf);

		/* Data length */
		msg.length = ring_buffer_read(p->rxBuf);

		/* Data */
		ring_buffer_read_bytes(p->rxBuf, msg.data, msg.length);

		/* proceeding */
		proceed_incoming(p, &msg);
	}

	/* transmit queued messages */
	while (uco_transmit_from_buffer(p) == SUCCESS);
}

/**
 *
 */
void
uco_init(uCO_t *p, const uOD_Item_t pOD[])
{
	/* Initialize ObjectDicrionary */
	p->OD = (uOD_Item_t*) pOD;

	/* Initialize RX buffer */
	p->rxBuf = &rxRingBuffer;
	ring_buffer_init(p->rxBuf, rxData, sizeof(rxData));

	/* Initialize TX buffer */
	p->txBuf = &txRingBuffer;
	ring_buffer_init(p->txBuf, txData, sizeof(txData));

	/* 128bit UID */
	p->ADDR[0] = HAL_GetUIDw0();
	p->ADDR[1] = HAL_GetUIDw1();
	p->ADDR[2] = HAL_GetUIDw2();
	p->ADDR[3] = 0x1UL; //serial

	p->NodeState = NODE_STATE_INITIALIZATION;
	p->NodeId = UCANOPEN_NODE_ID_UNCONFIGURED;

	/* Nodeguarding */
	p->NMT.NodeGuard.Timeout = 1500;
	p->NMT.NodeGuard.lifeTimeFactor = 2;

	/* Heartbeat */
	p->NMT.Heartbeat.sendOnSync = true;

	/* ticks sync */
	p->ticks = HAL_GetTick();
}

/**
 *
 */
void
uco_receive_to_buffer(uCO_t *p, CAN_RxHeaderTypeDef *pHeader, uint8_t *pData)
{
	size_t numBytes;

	/* No RTR, No ExtId are allowed */
	if (pHeader->RTR || pHeader->IDE == CAN_ID_EXT) return;

	numBytes = 2/* CobId */+ 1/* len */+ pHeader->DLC;

	if (ring_buffer_available_for_write(p->rxBuf) >= numBytes)
	{
		/* COB ID */
		ring_buffer_write(p->rxBuf, (pHeader->StdId >> 8) & 0x7F);
		ring_buffer_write(p->rxBuf, (pHeader->StdId) & 0xFF);

		/* Data length */
		ring_buffer_write(p->rxBuf, pHeader->DLC);

		/* Data */
		ring_buffer_write_bytes(p->rxBuf, pData, pHeader->DLC);
	}
}

/**
 *
 */
ErrorStatus
uco_transmit_direct(uCO_t *p, uCO_CanMessage_t *umsg)
{
	ErrorStatus result = ERROR;
	CAN_TxHeaderTypeDef Header = { 0 };
	uint8_t data[8];
	uint32_t mbox;

	if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan))
	{
		Header.RTR = CAN_RTR_DATA;
		Header.IDE = CAN_ID_STD;

		/* COB ID */
		Header.StdId = umsg->CobId;

		/* Data length */
		Header.DLC = umsg->length;

		/* Data */
		memcpy(data, umsg->data, umsg->length);

		if (HAL_CAN_AddTxMessage(&hcan, &Header, data, &mbox) == HAL_OK)
		{
			result = SUCCESS;
		}
	}
	return result;
}

/**
 *
 */
ErrorStatus
uco_transmit_from_buffer(uCO_t *p)
{
	ErrorStatus result = ERROR;
	CAN_TxHeaderTypeDef Header = { 0 };
	uint8_t data[8];
	uint32_t mbox;

	if (ring_buffer_available(p->txBuf) &&
		HAL_CAN_GetTxMailboxesFreeLevel(&hcan))
	{
		Header.RTR = CAN_RTR_DATA;
		Header.IDE = CAN_ID_STD;

		/* COB ID */
		Header.StdId = ring_buffer_read(p->txBuf) << 8;
		Header.StdId += ring_buffer_read(p->txBuf);

		/* Set archived TPDO NodeId */
		if (__IS_UCANOPEN_COB_ID_TPDO(Header.StdId) &&
			(Header.StdId & UCANOPEN_NODE_ID_MASK) != p->NodeId)
		{
			Header.StdId &= ~(UCANOPEN_NODE_ID_MASK);
			Header.StdId |= p->NodeId;
		}

		/* Data length */
		Header.DLC = ring_buffer_read(p->txBuf);

		/* Data */
		ring_buffer_read_bytes(p->txBuf, data, Header.DLC);

		if (HAL_CAN_AddTxMessage(&hcan, &Header, data, &mbox) == HAL_OK)
		{
			result = SUCCESS;
		}
	}
	return result;
}

/**
 *
 */
ErrorStatus
uco_send(uCO_t *p, uCO_CanMessage_t *msg)
{
	size_t numBytes;
	uCO_CanMessage_t tmp;

	numBytes = 2/* CobId */+ 1/* len */+ msg->length;

	/* pop oldest message from buffer, if necessary */
	while (ring_buffer_available_for_write(p->txBuf) < numBytes)
	{
		pop_message(p->txBuf, &tmp);
	}

	return push_message(p->txBuf, msg);
}

uOD_Item_t*
uco_find_od_item(uCO_t *p, uint16_t id, uint8_t sub)
{
	/** ---------------------------------------------------------
	 * [CAN_and_CANOpen.pdf] 2.2.6 Manufacturer Specific Entries
	 * ----------------------------------------------------------
	 * This section of the Object Dictionary, using Indexes from 2000h to 5FFFh is left completely open
	 * by the CANopen specification for application specific use. Whenever the application requires
	 * storage of data or configuration of operations that are outside of any CANopen standard
	 * (including frameworks, device profiles and other standards), they are located in this
	 * section of the Object Dictionary.
	 */
	if (id >= UCANOPEN_OD_MANUFACTURER_REGISTER_FIRST &&
		id <= UCANOPEN_OD_MANUFACTURER_REGISTER_LAST)
	{
		return uco_find_od_manufacturer_item(p, id, sub);
	}

	/** ---------------------------------------------------------
	 * [CAN_and_CANOpen.pdf] 2.5.5 RPDO Communication Parameters
	 * ----------------------------------------------------------
	 * In the Object Dictionary the Index area from 1400h to 15FFh is reserved for the RPDO communication
	 * parameters. The Index range of 512 (200h) ensures that a maximum of 512 RPDOs can be configured in
	 * the Object Dictionary of a single CANopen node. The parameters for the first RPDO (RPDO1)
	 * are located at Index 1400h, the parameters for the second at 1401h (RPDO2),
	 * for the third at 1402h (RPDO3) and so on.
	 */
	if (id >= UCANOPEN_OD_RPDO_REGISTER_FIRST &&
		id <= UCANOPEN_OD_RPDO_REGISTER_LAST)
	{
		return uco_find_od_rpdo_item(p, id, sub);
	}

	/** ---------------------------------------------------------
	 * [CAN_and_CANOpen.pdf] 2.5.6 TPDO Communication Parameters
	 * ----------------------------------------------------------
	 * In the Object Dictionary the Index area from 1800h to 19FFh is reserved for the TPDO communication
	 * parameters. As with the RPDOs, the Index range ensures that a maximum of 512 TPDOs can be configured
	 * for a single CANopen node. The parameters for the first TPDO (TPDO1) are located at Index 1800h,
	 * the parameters for the second at 1801h (TPDO2), for the third at 1802h (TPDO3) and so on.
	 */
	if (id >= UCANOPEN_OD_TPDO_REGISTER_FIRST &&
		id <= UCANOPEN_OD_TPDO_REGISTER_LAST)
	{
		return uco_find_od_tpdo_item(p, id, sub);
	}

	/** --------------
	 *  Search in OD
	 *  --------------
	 */
	for (int i = 0; p->OD[i].index != 0xFFFF; i++)
	{
		if (p->OD[i].index == id)
		{
			if (p->OD[i].Type == SUBARRAY)
			{
				uOD_Item_t *item = p->OD[i].address;
				uint8_t sub_count = *((uint8_t*) item[0].address);
				for (int j = 0; j < sub_count + 1; j++)
				{
					if (item[j].index == sub)
					{
						return &item[j];
					}
				}
			}
			else
			{
				return (sub == 0) ?
					&p->OD[i] : NULL;
			}
		}
	}
	return NULL;
}

__weak uOD_Item_t*
uco_find_od_rpdo_item(uCO_t *p, uint16_t id, uint8_t sub)
{
	return NULL;
}

__weak uOD_Item_t*
uco_find_od_tpdo_item(uCO_t *p, uint16_t id, uint8_t sub)
{
	return NULL;
}

__weak uOD_Item_t*
uco_find_od_manufacturer_item(uCO_t *p, uint16_t id, uint8_t sub)
{
	return NULL;
}
