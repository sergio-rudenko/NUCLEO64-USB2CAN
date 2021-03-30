/*
 * lawicel.c
 *
 *  Created on: Oct 8, 2020
 *      Author: sergi
 */

#include <stdio.h>
#include <string.h>

#include "lawicel.h"

/* const */
static const uint32_t bitrate_table[] = {
	10000UL,
	20000UL,
	50000UL,
	100000UL,
	125000UL,
	250000UL,
	500000UL,
	800000UL,
	1000000UL
};

/* buffers */
static uint8_t lawicel_rx[LAWICEL_RX_BUFFER_SIZE];
static uint8_t lawicel_tx[LAWICEL_TX_BUFFER_SIZE];
static uint8_t can_tx[LAWICEL_CAN_TX_BUFFER_SIZE];

/* instance */
static LAWICEL_Instance_t Instance;
LAWICEL_Instance_t *pLawicelInstance = &Instance;

/* CAN API */
#if __has_include( "CAN_api.h" )
#include "CAN_api.h"
#else

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

#endif /* CAN_api.h */

/**
 *
 */
#if defined( LAWICEL_UART_AS_UPLINK )

static ErrorStatus
uart_start_receive(LAWICEL_Instance_t* p)
{
if (ring_buffer_available_for_write(&p->LawicelRx))
{
	uint8_t ptr = ring_buffer_get_write_ptr(&p->LawicelRx);
	return (HAL_UART_Receive_IT(p->huart, ptr, 1) == HAL_OK) ? SUCCESS : ERROR;
}
return ERROR;
}

#endif /* LAWICEL_UART_AS_UPLINK */

static uint8_t
halfbyte_to_hexascii(uint8_t num)
{
	num &= 0x0F;

	if (num >= 10)
		return ('A' + num - 10);
	else
		return ('0' + num);
}

static uint8_t
hexascii_to_halfbyte(uint8_t chr)
{
	if ((chr >= '0') && (chr <= '9'))
		return (chr - '0');

	if ((chr >= 'a') && (chr <= 'f'))
		return (chr - 'a' + 10);

	if ((chr >= 'A') && (chr <= 'F'))
		return (chr - 'A' + 10);

	return (0xFF);
}

static ErrorStatus
lawicel_to_can_message(rBuffer_t *buf, CAN_Message_t *msg)
{
	switch (ring_buffer_read(buf))
	{
		case LAWICEL_TRANSMIT_STD:
			msg->header.IDE = CAN_ID_STD;
			msg->header.RTR = CAN_RTR_DATA;

			/* CAN ID */
			if ((hexascii_to_halfbyte(ring_buffer_at(buf, 0)) == 0xFF) ||
				(hexascii_to_halfbyte(ring_buffer_at(buf, 1)) == 0xFF) ||
				(hexascii_to_halfbyte(ring_buffer_at(buf, 2)) == 0xFF))
			{
				return ERROR;
			}
			msg->header.ID = hexascii_to_halfbyte(ring_buffer_read(buf)) << 8;
			msg->header.ID += hexascii_to_halfbyte(ring_buffer_read(buf)) << 4;
			msg->header.ID += hexascii_to_halfbyte(ring_buffer_read(buf));

			/* Data length */
			msg->header.DLC = hexascii_to_halfbyte(ring_buffer_read(buf));
			if (msg->header.DLC > 8)
			{
				return ERROR;
			}

			/* Data */
			for (int i = 0; i < msg->header.DLC; i++)
			{
				if ((hexascii_to_halfbyte(ring_buffer_at(buf, 0)) == 0xFF) ||
					(hexascii_to_halfbyte(ring_buffer_at(buf, 1)) == 0xFF))
				{
					return ERROR;
				}
				msg->data[i] = hexascii_to_halfbyte(ring_buffer_read(buf)) << 4;
				msg->data[i] += hexascii_to_halfbyte(ring_buffer_read(buf));
			}
			break;

		default:
			return ERROR;
	}

	/* Check command end */
	if (ring_buffer_peek(buf) == '\r')
	{
		return SUCCESS;
	}
	return ERROR;
}

/**
 * API: Initialization
 */
ErrorStatus
LAWICEL_init(LAWICEL_Instance_t *p, UART_HandleTypeDef *huart, CAN_HandleTypeDef *hcan)
{
	p->huart = huart;
	p->hcan = hcan;

	ring_buffer_init(&p->LawicelRx, lawicel_rx, sizeof(lawicel_rx));
	ring_buffer_init(&p->LawicelTx, lawicel_tx, sizeof(lawicel_tx));
	ring_buffer_init(&p->CanTx, can_tx, sizeof(can_tx));

#if defined( LAWICEL_UART_AS_UPLINK )

	uart_start_receive(p);

#endif /* LAWICEL_UART_AS_UPLINK */

	return SUCCESS;
}

/**
 * API: Run
 */
ErrorStatus
LAWICEL_run(LAWICEL_Instance_t *p)
{
	uint8_t reply_data[32];
	size_t reply_size = 0;

	/* Ticks and Timer */
	if (p->savedTicks != HAL_GetTick())
	{
		uint16_t dt = HAL_GetTick() - p->savedTicks;
		p->savedTicks = HAL_GetTick();

		if (p->TimestampState == LAWICEL_TIMESTAMP_ENABLED)
			p->timer = (p->timer + dt) % LAWICEL_TIMER_MAX_MS;
		else
			p->timer = 0;
	}

	/* proceed RX */
	while (ring_buffer_available(&p->LawicelRx))
	{
		if (ring_buffer_find(&p->LawicelRx, '\r'))
		{
			switch (ring_buffer_peek(&p->LawicelRx))
			{
				case LAWICEL_GET_HARDWARE:
					reply_size = sprintf((char*) reply_data, "%s\r", CONVERTER_VERSION_HW);
					break;

				case LAWICEL_GET_VERSION:
					reply_size = sprintf((char*) reply_data, "%s\r", CONVERTER_VERSION_SW);
					break;

				case LAWICEL_SET_BITRATE:
					ring_buffer_move_read_index(&p->LawicelRx, 1); // skip byte
					{
						uint8_t bIndex = hexascii_to_halfbyte(ring_buffer_read(&p->LawicelRx));

						if (bIndex <= 8 &&
							CAN_set_bitrate(p->hcan, bitrate_table[bIndex]) == SUCCESS)
						{
							reply_data[0] = LAWICEL_RESPONCE_OK;
							reply_size = 1;
						}
						else
						{
							reply_data[0] = LAWICEL_RESPONCE_ERROR;
							reply_size = 1;
						}
					}
					break;

				case LAWICEL_CLOSE_CAN:
					if (CAN_stop(p->hcan) == SUCCESS)
					{
						LAWICEL_CAN_on_stop_callback(p);
						reply_data[0] = LAWICEL_RESPONCE_OK;
						reply_size = 1;
					}
					else
					{
						reply_data[0] = LAWICEL_RESPONCE_ERROR;
						reply_size = 1;
					}
					break;

				case LAWICEL_OPEN_CAN:
					if (CAN_start(p->hcan) == SUCCESS)
					{
						LAWICEL_CAN_on_start_callback(p);
						reply_data[0] = LAWICEL_RESPONCE_OK;
						reply_size = 1;
					}
					else
					{
						reply_data[0] = LAWICEL_RESPONCE_ERROR;
						reply_size = 1;
					}
					break;

				case LAWICEL_USE_TIMESTAMP:
					ring_buffer_move_read_index(&p->LawicelRx, 1); // skip byte
					switch (hexascii_to_halfbyte(ring_buffer_read(&p->LawicelRx)))
					{
						case LAWICEL_TIMESTAMP_DISABLED:
							p->TimestampState = LAWICEL_TIMESTAMP_DISABLED;
							reply_data[0] = LAWICEL_RESPONCE_OK;
							reply_size = 1;
							break;

						case LAWICEL_TIMESTAMP_ENABLED:
							p->TimestampState = LAWICEL_TIMESTAMP_ENABLED;
							reply_data[0] = LAWICEL_RESPONCE_OK;
							reply_size = 1;
							break;

						default:
							reply_data[0] = LAWICEL_RESPONCE_ERROR;
							reply_size = 1;
					}
					break;

				case LAWICEL_TRANSMIT_STD:
					if (p->hcan->State == HAL_CAN_STATE_LISTENING)
					{
						CAN_Message_t msg;

						if (lawicel_to_can_message(&p->LawicelRx, &msg) == SUCCESS)
						{
							/* CAN ID Type */
							ring_buffer_write(&p->CanTx, msg.header.IDE);

							/* CAN ID */
							ring_buffer_write(&p->CanTx, (msg.header.ID) & 0xFF);
							ring_buffer_write(&p->CanTx, (msg.header.ID >> 8) & 0xFF);
							ring_buffer_write(&p->CanTx, (msg.header.ID >> 16) & 0xFF);
							ring_buffer_write(&p->CanTx, (msg.header.ID >> 24) & 0xFF);

							/* RTR, Data length */
							ring_buffer_write(&p->CanTx, msg.header.RTR);
							ring_buffer_write(&p->CanTx, msg.header.DLC);

							/* Data */
							for (int i = 0; i < msg.header.DLC; i++)
								ring_buffer_write(&p->CanTx, msg.data[i]);

							LAWICEL_CAN_transmit(p);
							reply_data[0] = LAWICEL_RESPONCE_OK;
							reply_size = 1;
						}
						else
						{
							reply_data[0] = LAWICEL_RESPONCE_ERROR;
							reply_size = 1;
						}
					}
					else
					{
						reply_data[0] = LAWICEL_RESPONCE_ERROR;
						reply_size = 1;
					}
					break;

				default:
					reply_data[0] = LAWICEL_RESPONCE_OK;
					reply_size = 1;
					break;
			}

			if (reply_size)
			{
				/* Move reply to TX buffer */
				ring_buffer_write_bytes(&p->LawicelTx, reply_data, reply_size);
				reply_size = 0;

				/* Flush remaining data from RX buffer */
				ring_buffer_seek(&p->LawicelRx, '\r');
				ring_buffer_move_read_index(&p->LawicelRx, 1);
			}
			else
			{
				/* Skip one byte */
				ring_buffer_move_read_index(&p->LawicelRx, 1);
			}
		}
	}

	/* Start Transmit if necessary */
	LAWICEL_UPLINK_transmit(p);

	return SUCCESS;
}

/**
 * API: On received data from uplink
 */
ErrorStatus
LAWICEL_UPLINK_on_receive(LAWICEL_Instance_t *p, uint8_t *pData, size_t size)
{
	if (!p || !p->LawicelRx.data)
	{
		return ERROR;
	}

#if defined( LAWICEL_UART_AS_UPLINK )

	if (*ring_buffer_get_write_ptr(&p->LawicelRx) != '\n')
	{
		ring_buffer_move_write_index(&p->LawicelRx, 1);
	}
	return uart_start_receive(p);

#endif /* LAWICEL_UART_AS_UPLINK */

#if defined( LAWICEL_USB_AS_UPLINK )

	if (ring_buffer_available_for_write(&p->LawicelRx) >= size)
	{
		ring_buffer_write_bytes(&p->LawicelRx, pData, size);
		return SUCCESS;
	}

#endif /* LAWICEL_USB_AS_UPLINK */

	return ERROR;
}

/**
 * API: Transmit data to uplink
 */
ErrorStatus
LAWICEL_UPLINK_transmit(LAWICEL_Instance_t *p)
{
	uint8_t *pData;
	size_t size;

	if (!p || !p->LawicelTx.data)
	{
		return ERROR;
	}

	if (!ring_buffer_available(&p->LawicelTx))
	{
		return SUCCESS;
	}
	else
	{
		pData = ring_buffer_get_read_ptr(&p->LawicelTx);
		size = ring_buffer_get_read_linear_size(&p->LawicelTx);
	}

#if defined( LAWICEL_UART_AS_UPLINK )

	if (p->huart->gState != HAL_UART_STATE_BUSY_TX &&
		p->huart->gState != HAL_UART_STATE_BUSY_TX_RX)
	{
		if (HAL_UART_Transmit_IT(p->huart, pData, size) == HAL_OK)
		{
			ring_buffer_move_read_index(&p->LawicelTx, size);
			return SUCCESS;
		}
	}

#endif /* LAWICEL_UART_AS_UPLINK */

#if defined( LAWICEL_USB_AS_UPLINK )

	if (CDC_Transmit_FS(pData, size) != USBD_BUSY)
	{
		ring_buffer_move_read_index(&p->LawicelTx, size);
		return SUCCESS;
	}

#endif /* LAWICEL_USB_AS_UPLINK */

	return ERROR;
}

/**
 * API: On received data from CAN
 */
ErrorStatus
LAWICEL_CAN_on_receive(LAWICEL_Instance_t *p, CAN_RxHeaderTypeDef *pHeader, uint8_t *pData)
{
//TODO! 29bit messages

	size_t num_bytes = 1 /* 't' */+ 3 /* ID */+ 1 /* DLC */;
	num_bytes += (pHeader->DLC * 2) /* Data */+ 1 /* '\r' */;
	num_bytes += (p->TimestampState == LAWICEL_TIMESTAMP_ENABLED) ? 4 : 0;

	if (ring_buffer_available_for_write(&p->LawicelTx) >= num_bytes)
	{
		ring_buffer_write(&p->LawicelTx, 't');

		/* Standard Id */
		ring_buffer_write(&p->LawicelTx, halfbyte_to_hexascii((pHeader->StdId) >> 8));
		ring_buffer_write(&p->LawicelTx, halfbyte_to_hexascii((pHeader->StdId) >> 4));
		ring_buffer_write(&p->LawicelTx, halfbyte_to_hexascii((pHeader->StdId)));

		/* Length of the frame */
		ring_buffer_write(&p->LawicelTx, halfbyte_to_hexascii((pHeader->DLC)));

		/* Frame data */
		for (int i = 0; i < pHeader->DLC; i++)
		{
			ring_buffer_write(&p->LawicelTx, halfbyte_to_hexascii((pData[i]) >> 4));
			ring_buffer_write(&p->LawicelTx, halfbyte_to_hexascii((pData[i])));
		}

		/* TimeStamp */
		if (p->TimestampState == LAWICEL_TIMESTAMP_ENABLED)
		{
			ring_buffer_write(&p->LawicelTx, halfbyte_to_hexascii((p->timer) >> 12));
			ring_buffer_write(&p->LawicelTx, halfbyte_to_hexascii((p->timer) >> 8));
			ring_buffer_write(&p->LawicelTx, halfbyte_to_hexascii((p->timer) >> 4));
			ring_buffer_write(&p->LawicelTx, halfbyte_to_hexascii((p->timer)));
		}

		ring_buffer_write(&p->LawicelTx, '\r');

		LAWICEL_CAN_on_receive_callback(p, pHeader, pData);
		return SUCCESS;
	}
	return ERROR;
}

/**
 * API: Transmit message to CAN
 */
ErrorStatus
LAWICEL_CAN_transmit(LAWICEL_Instance_t *p)
{
	static uint32_t mailbox;

	CAN_TxHeaderTypeDef Header;
	uint8_t data[8];

	if (ring_buffer_available(&p->CanTx) &&
		HAL_CAN_GetTxMailboxesFreeLevel(p->hcan))
	{
		/* CAN ID Type */
		Header.IDE = ring_buffer_read(&p->CanTx);

		/* CAN ID */
		Header.StdId = ring_buffer_read(&p->CanTx);
		Header.StdId += ring_buffer_read(&p->CanTx) << 8;
		Header.StdId += ring_buffer_read(&p->CanTx) << 16;
		Header.StdId += ring_buffer_read(&p->CanTx) << 24;
		Header.ExtId = Header.StdId;

		/* RTR, Data length */
		Header.RTR = ring_buffer_read(&p->CanTx);
		Header.DLC = ring_buffer_read(&p->CanTx);

		/* Data */
		for (int i = 0; i < Header.DLC; i++)
			data[i] = ring_buffer_read(&p->CanTx);

		if (HAL_CAN_AddTxMessage(p->hcan, &Header, &data[0], &mailbox) == HAL_OK)
		{
			LAWICEL_CAN_on_transmit_callback(p, &Header, &data[0]);
			return SUCCESS;
		}
	}
	return ERROR;
}

/* callback funcrions */

__weak void
LAWICEL_CAN_on_stop_callback(LAWICEL_Instance_t *p)
{
	UNUSED(p);
}

__weak void
LAWICEL_CAN_on_start_callback(LAWICEL_Instance_t *p)
{
	UNUSED(p);
}

void
LAWICEL_CAN_on_transmit_callback(LAWICEL_Instance_t *p, CAN_TxHeaderTypeDef *pHeader, uint8_t *pData)
{
	UNUSED(p);
	UNUSED(pHeader);
	UNUSED(pData);
}

void
LAWICEL_CAN_on_receive_callback(LAWICEL_Instance_t *p, CAN_RxHeaderTypeDef *pHeader, uint8_t *pData)
{
	UNUSED(p);
	UNUSED(pHeader);
	UNUSED(pData);
}
