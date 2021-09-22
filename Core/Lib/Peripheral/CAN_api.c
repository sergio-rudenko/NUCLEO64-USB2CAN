/*
 * can_api.c
 *
 *  Created on: Oct 23, 2020
 *      Author: sergi
 */

#include "CAN_api.h"

/* instance */
static CAN_Instance_t CanInstance;
CAN_Instance_t *pCanInstance = &CanInstance;

ErrorStatus
CAN_stop(CAN_HandleTypeDef *hcan)
{
	if (hcan && hcan->State != HAL_CAN_STATE_READY)
	{
		if (HAL_CAN_Stop(hcan) == HAL_OK)
		{
			return SUCCESS;
		}
	}
	else
	{
		return SUCCESS;
	}
	return ERROR;
}

ErrorStatus
CAN_start(CAN_HandleTypeDef *hcan)
{
	if (hcan && hcan->State != HAL_CAN_STATE_LISTENING)
	{
		/* Configure the CAN Filter */
		CAN_FilterTypeDef CanFilterConfig;
		uint32_t interrupts =
		CAN_IT_ERROR |
		CAN_IT_RX_FIFO0_MSG_PENDING |
		CAN_IT_RX_FIFO1_MSG_PENDING |
		CAN_IT_TX_MAILBOX_EMPTY;

		CanFilterConfig.FilterBank = 0;
		CanFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
		CanFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
		CanFilterConfig.FilterIdHigh = 0x0000;
		CanFilterConfig.FilterIdLow = 0x0000;
		CanFilterConfig.FilterMaskIdHigh = 0x0000;
		CanFilterConfig.FilterMaskIdLow = 0x0000;
		CanFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
		CanFilterConfig.FilterActivation = ENABLE;
		CanFilterConfig.SlaveStartFilterBank = 14;

		if (HAL_CAN_ConfigFilter(hcan, &CanFilterConfig) == HAL_OK &&
			HAL_CAN_ActivateNotification(hcan, interrupts) == HAL_OK &&
			HAL_CAN_Start(hcan) == HAL_OK)
		{
			return SUCCESS;
		}
	}
	else
	{
		return SUCCESS;
	}
	return ERROR;
}

ErrorStatus
CAN_set_bitrate(CAN_HandleTypeDef *hcan, uint32_t bitrate)
{
	if (hcan && hcan->State == HAL_CAN_STATE_READY)
	{
		hcan->Init.Prescaler = SystemCoreClock
			/ (bitrate * 12 /* 1TQ + 5TQ + 6TQ*/);

		if (HAL_CAN_Init(hcan) == HAL_OK)
		{
			return SUCCESS;
		}
	}
	return ERROR;
}

/**
 *
 */
//ErrorStatus
//CAN_transmit(CAN_Instance_t *p, CanTxMessage_t *m)
//{
//	static uint32_t mailbox;
//
//	if (HAL_CAN_GetTxMailboxesFreeLevel(p->hcan))
//	{
//		if (HAL_CAN_AddTxMessage(p->hcan, &m->header, &m->data[0], &mailbox) != HAL_OK)
//		{
//			CAN_stop(p);
//			CAN_start(p);
//			p->restartCounter++;
//		}
//		else
//		{
//			return SUCCESS;
//		}
//	}
//	return ERROR;
//}
