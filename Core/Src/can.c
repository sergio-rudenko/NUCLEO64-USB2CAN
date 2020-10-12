/**
 ******************************************************************************
 * File Name          : CAN.c
 * Description        : This file provides code for the configuration
 *                      of the CAN instances.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "can.h"

/* USER CODE BEGIN 0 */
#include "lawicel.h"
#include "ucanopen.h"

CanTxMessage_t canTxMsg;
CanRxMessage_t canRxMsg;

volatile uint32_t canTxCounter;
volatile uint32_t canRxCounter;
volatile uint32_t canErrorsCounter;
volatile uint32_t canRestartCounter;

/* USER CODE END 0 */

CAN_HandleTypeDef hcan;

/* CAN init function */
void
MX_CAN_Init(void)
{

	hcan.Instance = CAN;
	hcan.Init.Prescaler = 32;
	hcan.Init.Mode = CAN_MODE_NORMAL;
	hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
	hcan.Init.TimeSeg1 = CAN_BS1_6TQ;
	hcan.Init.TimeSeg2 = CAN_BS2_5TQ;
	hcan.Init.TimeTriggeredMode = DISABLE;
	hcan.Init.AutoBusOff = DISABLE;
	hcan.Init.AutoWakeUp = DISABLE;
	hcan.Init.AutoRetransmission = DISABLE;
	hcan.Init.ReceiveFifoLocked = DISABLE;
	hcan.Init.TransmitFifoPriority = DISABLE;
	if (HAL_CAN_Init(&hcan) != HAL_OK)
	{
		Error_Handler();
	}

}

void
HAL_CAN_MspInit(CAN_HandleTypeDef *canHandle)
{

	GPIO_InitTypeDef GPIO_InitStruct =
	{ 0 };
	if (canHandle->Instance == CAN)
	{
		/* USER CODE BEGIN CAN_MspInit 0 */

		/* USER CODE END CAN_MspInit 0 */
		/* CAN clock enable */
		__HAL_RCC_CAN1_CLK_ENABLE();

		__HAL_RCC_GPIOB_CLK_ENABLE();
		/**CAN GPIO Configuration
		 PB8     ------> CAN_RX
		 PB9     ------> CAN_TX
		 */
		GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF4_CAN;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

		/* CAN interrupt Init */
		HAL_NVIC_SetPriority(CEC_CAN_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(CEC_CAN_IRQn);
		/* USER CODE BEGIN CAN_MspInit 1 */

		/* USER CODE END CAN_MspInit 1 */
	}
}

void
HAL_CAN_MspDeInit(CAN_HandleTypeDef *canHandle)
{

	if (canHandle->Instance == CAN)
	{
		/* USER CODE BEGIN CAN_MspDeInit 0 */

		/* USER CODE END CAN_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_CAN1_CLK_DISABLE();

		/**CAN GPIO Configuration
		 PB8     ------> CAN_RX
		 PB9     ------> CAN_TX
		 */
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8 | GPIO_PIN_9);

		/* CAN interrupt Deinit */
		HAL_NVIC_DisableIRQ(CEC_CAN_IRQn);
		/* USER CODE BEGIN CAN_MspDeInit 1 */

		/* USER CODE END CAN_MspDeInit 1 */
	}
}

/* USER CODE BEGIN 1 */

bool
UART2CAN_CAN_SetBitRate(CAN_HandleTypeDef *handle, uint32_t bitrate)
{
	if (handle->State == HAL_CAN_STATE_LISTENING)
	{
		return false; // ERROR: CAN Started
	}

	/* calculate prescaler for bitrate */
	handle->Init.Prescaler = SystemCoreClock
		/ (bitrate * 12 /* 1TQ + 5TQ + 6TQ*/);

	return (HAL_CAN_Init(handle) == HAL_OK);
}

bool
UART2CAN_CAN_Stop(CAN_HandleTypeDef *handle)
{
	if (handle->State == HAL_CAN_STATE_LISTENING)
	{
		return (HAL_CAN_Stop(handle) == HAL_OK);
	}
	else
	{
		return true;
	}
}

bool
UART2CAN_CAN_Start(CAN_HandleTypeDef *handle)
{
	bool result;

	/* Configure the CAN Filter */
	CAN_FilterTypeDef CanFilterConfig;

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
	result = (HAL_CAN_ConfigFilter(handle, &CanFilterConfig) == HAL_OK);

	if (result)
	{
		/* Enable interrupts */
		result = (HAL_CAN_ActivateNotification(handle,
		CAN_IT_ERROR |
		CAN_IT_TX_MAILBOX_EMPTY |
		CAN_IT_RX_FIFO0_MSG_PENDING |
		CAN_IT_RX_FIFO1_MSG_PENDING) == HAL_OK);
	}

	if (result)
	{
		/* Start */
		result = (HAL_CAN_Start(handle) == HAL_OK);
	}

	return result;
}

bool
UART2CAN_CAN_Transmit(CAN_HandleTypeDef *handle, CanTxMessage_t *m)
{
	static uint32_t mailbox;

	if (HAL_CAN_AddTxMessage(handle, &m->header, m->data, &mailbox) != HAL_OK)
	{
		UART2CAN_CAN_Stop(handle);
		UART2CAN_CAN_Start(handle);
		canRestartCounter++;
		return false;
	}
	return true;
}

void
HAL_CAN_ErrorCallback(CAN_HandleTypeDef *handle)
{
	canErrorsCounter++;

	if (handle->ErrorCode & HAL_CAN_ERROR_TX_TERR0)
	{
		HAL_CAN_AbortTxRequest(handle, CAN_TX_MAILBOX0);
	}
	if (handle->ErrorCode & HAL_CAN_ERROR_TX_TERR1)
	{
		HAL_CAN_AbortTxRequest(handle, CAN_TX_MAILBOX1);
	}
	if (handle->ErrorCode & HAL_CAN_ERROR_TX_TERR2)
	{
		HAL_CAN_AbortTxRequest(handle, CAN_TX_MAILBOX2);
	}

	if (handle->ErrorCode & (
	HAL_CAN_ERROR_EPV |
	HAL_CAN_ERROR_BOF |
	HAL_CAN_ERROR_STF |
	HAL_CAN_ERROR_FOR |
	HAL_CAN_ERROR_ACK |
	HAL_CAN_ERROR_CRC |
	HAL_CAN_ERROR_BR |
	HAL_CAN_ERROR_BD))
	{

		UART2CAN_CAN_Stop(handle);
		UART2CAN_CAN_Start(handle);
		canRestartCounter++;
	}

	HAL_CAN_ResetError(handle);
}

void
HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *handle)
{
	canTxCounter++;
}

void
HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *handle)
{
	canTxCounter++;
}

void
HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *handle)
{
	canTxCounter++;
}

void
HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *handle)
{
	/* Get RX message */
	if (HAL_CAN_GetRxMessage(handle, CAN_RX_FIFO0, &canRxMsg.header,
								canRxMsg.data) == HAL_OK)
	{

		lawicel_can_receive(pUartTxBuf, &canRxMsg);
		uco_proceed(&canRxMsg);
		canRxCounter++;
	}
}

void
HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *handle)
{
	/* Get RX message */
	if (HAL_CAN_GetRxMessage(handle, CAN_RX_FIFO1, &canRxMsg.header,
								canRxMsg.data) == HAL_OK)
	{

		lawicel_can_receive(pUartTxBuf, &canRxMsg);
		uco_proceed(&canRxMsg);
		canRxCounter++;
	}
}

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
