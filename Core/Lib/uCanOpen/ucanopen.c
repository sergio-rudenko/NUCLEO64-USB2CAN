/*
 * ucanopen.c
 *
 *  Created on: Oct 9, 2020
 *      Author: sergi
 */

#include "ucanopen.h"

/* instance */
uCO_t uCO;

/**
 *
 */
void
uco_init(uCO_t *pCO, const uCO_OD_Item_t pOD[])
{
	pCO->OD = (uCO_OD_Item_t*) pOD;

	/* 128bit UID */
	pCO->UID[0] = HAL_GetUIDw0();
	pCO->UID[1] = HAL_GetUIDw1();
	pCO->UID[2] = HAL_GetUIDw2();
	pCO->UID[3] = 0x0UL; //serial

	pCO->NodeState = INITIALIZATION;
	pCO->NodeId = UCANOPEN_NODE_ID_UNDEFINED;
}


/**
 *
 */
uCO_ErrorStatus_t
uco_proceed(CanRxMessage_t *msg)
{
	uCO_ErrorStatus_t result = UCANOPEN_ERROR;

	/* No RTR, No ExtId are allowed */
	if (msg->header.RTR || msg->header.ExtId) return result;

	/* switch protocol */
	if (__IS_UCANOPEN_COB_ID_NMT(msg->header.StdId))
	{
		result = uco_proceed_NMT_command(msg->data, msg->header.DLC);
	}
	else if (__IS_UCANOPEN_COB_ID_SYNC(msg->header.StdId))
	{
		result = uco_proceed_SYNC_request(msg->data, msg->header.DLC);
	}
	else if (__IS_UCANOPEN_COB_ID_EMCY(msg->header.StdId))
	{
		uCO_NodeId_t NodeId = __UCANOPEN_NODE_ID_FROM_COB_ID(msg->header.StdId);
		result = uco_proceed_EMCY_message(NodeId, msg->data, msg->header.DLC);
	}
	else if (__IS_UCANOPEN_COB_ID_TSDO(msg->header.StdId))
	{
		uCO_NodeId_t NodeId = __UCANOPEN_NODE_ID_FROM_COB_ID(msg->header.StdId);
		result = uco_proceed_SDO_reply(NodeId, msg->data);
	}
	else if (__IS_UCANOPEN_COB_ID_RSDO(msg->header.StdId))
	{
		uCO_NodeId_t NodeId = __UCANOPEN_NODE_ID_FROM_COB_ID(msg->header.StdId);
		result = uco_proceed_SDO_request(NodeId, msg->data);
	}
	else if (__IS_UCANOPEN_COB_ID_TPDO(msg->header.StdId))
	{
		//TODO
	}
	else if (__IS_UCANOPEN_COB_ID_RPDO(msg->header.StdId))
	{
		//TODO
	}
	else
	{
		//TODO
	}
	return result;
}

uCO_OD_Item_t*
uco_find_OD_item(uCO_t *p, uint16_t id, uint8_t sub)
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
		return uco_find_Manufacturer_OD_item(p, id, sub);
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
		return uco_find_RPDO_OD_item(p, id, sub);
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
		return uco_find_TPDO_OD_item(p, id, sub);
	}

	/* Search in OD */
	for (int i = 0; p->OD[i].index != 0xFFFF; i++)
	{
		if (p->OD[i].index == id)
		{
			if (p->OD[i].Type == SUBARRAY)
			{
				uCO_OD_Item_t *item = p->OD[i].address;
				uint8_t sub_count = *((uint8_t*) item[0].address);
				for (int j = 1; j < sub_count; j++)
				{
					if (item[j].index == sub)
					{
						return &item[j];
					}
				}
			}
			else
			{
				return (sub == 0) ? &p->OD[i] : NULL;
			}
		}
	}
	return NULL;
}

__weak uCO_OD_Item_t*
uco_find_RPDO_OD_item(uCO_t *p, uint16_t id, uint8_t sub)
{
	return NULL;
}

__weak uCO_OD_Item_t*
uco_find_TPDO_OD_item(uCO_t *p, uint16_t id, uint8_t sub)
{
	return NULL;
}

__weak uCO_OD_Item_t*
uco_find_Manufacturer_OD_item(uCO_t *p, uint16_t id, uint8_t sub)
{
	return NULL;
}
