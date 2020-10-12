/*
 * lawicel.c
 *
 *  Created on: Oct 8, 2020
 *      Author: sergi
 */

#include <stdio.h>
#include <string.h>

#include "lawicel.h"

#include "usart.h"
#include "can.h"

/* variables */
volatile uint16_t lawicelTimer;
static bool useTimeStamp;

inline void
lawicel_timer_tick()
{
	lawicelTimer = (lawicelTimer + 1) % LAWICEL_TIMER_MAX_MS;
}

uint8_t
halfbyte_to_hexascii(uint8_t num)
{
	num &= 0x0F;
	if (num >= 10)
		return ('A' + num - 10);
	else
		return ('0' + num);
}

uint8_t
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

bool
lawicel_can_set_bitrate(rBuffer_t *rx)
{
	uint32_t bitrate;

	if (ring_buffer_peek(rx) == LAWICEL_BITRATE)
	{
		ring_buffer_move_read_index(rx, 1); // skip
	}

	switch (hexascii_to_halfbyte(ring_buffer_read(rx)))
	{
		case 0: // S0 Setup 10Kbit
			bitrate = 10000UL;
			break;

		case 1: // S1 Setup 20Kbit
			bitrate = 20000UL;
			break;

		case 2: // S2 Setup 50Kbit
			bitrate = 50000UL;
			break;

		case 3: // S3 Setup 100Kbit
			bitrate = 100000UL;
			break;

		case 4: // S4 Setup 125Kbit
			bitrate = 125000UL;
			break;

		case 5: // S5 Setup 250Kbit
			bitrate = 250000UL;
			break;

		case 6: // S6 Setup 500Kbit
			bitrate = 500000UL;
			break;

		case 7: // S7 Setup 800Kbit
			bitrate = 800000UL;
			break;

		case 8: // S8 Setup 1Mbit
			bitrate = 1000000UL;
			break;

		default:
			return false;
	}

	return UART2CAN_CAN_SetBitRate(&hcan, bitrate);
}

bool
lawicel_can_receive(rBuffer_t *tx, CanRxMessage_t *msg)
{
	size_t num_bytes =
	1 + 3 + 1 + msg->header.DLC * 2 + (useTimeStamp) ? 4 : 0 + 1;

	if (ring_buffer_available_for_write(tx) < num_bytes)
	{
		return false;
	}

	ring_buffer_write(tx, 't');

	/* Standard Id */
	ring_buffer_write(tx, halfbyte_to_hexascii((msg->header.StdId) >> 8));
	ring_buffer_write(tx, halfbyte_to_hexascii((msg->header.StdId) >> 4));
	ring_buffer_write(tx, halfbyte_to_hexascii((msg->header.StdId)));

	/* Length of the frame */
	ring_buffer_write(tx, halfbyte_to_hexascii((msg->header.DLC)));

	/* Frame data */
	for (int i = 0; i < msg->header.DLC; i++)
	{
		ring_buffer_write(tx, halfbyte_to_hexascii((msg->data[i]) >> 4));
		ring_buffer_write(tx, halfbyte_to_hexascii((msg->data[i])));
	}

	/* TimeStamp */
	if (useTimeStamp)
	{
		ring_buffer_write(tx, halfbyte_to_hexascii((lawicelTimer) >> 12));
		ring_buffer_write(tx, halfbyte_to_hexascii((lawicelTimer) >> 8));
		ring_buffer_write(tx, halfbyte_to_hexascii((lawicelTimer) >> 4));
		ring_buffer_write(tx, halfbyte_to_hexascii((lawicelTimer)));
	}

	ring_buffer_write(tx, '\r');

	return true;
}

bool
lawicel_can_transmit(rBuffer_t *rx, CanTxMessage_t *msg)
{
	if (ring_buffer_peek(rx) == LAWICEL_TRANSMIT)
	{
		ring_buffer_move_read_index(rx, 1); // skip
	}

	/* Standard Id */
	if ((hexascii_to_halfbyte(ring_buffer_at(rx, 0)) == 0xFF) ||
		(hexascii_to_halfbyte(ring_buffer_at(rx, 1)) == 0xFF) ||
		(hexascii_to_halfbyte(ring_buffer_at(rx, 2)) == 0xFF))
	{
		return false;
	}
	msg->header.StdId = hexascii_to_halfbyte(ring_buffer_read(rx)) << 8;
	msg->header.StdId += hexascii_to_halfbyte(ring_buffer_read(rx)) << 4;
	msg->header.StdId += hexascii_to_halfbyte(ring_buffer_read(rx));

	/* Length of the frame */
	msg->header.DLC = hexascii_to_halfbyte(ring_buffer_read(rx));
	if (msg->header.DLC > 8)
	{
		return false;
	}

	/* Frame data */
	for (int i = 0; i < msg->header.DLC; i++)
	{
		if ((hexascii_to_halfbyte(ring_buffer_at(rx, 0)) == 0xFF) ||
			(hexascii_to_halfbyte(ring_buffer_at(rx, 1)) == 0xFF))
		{
			return false;
		}
		msg->data[i] = hexascii_to_halfbyte(ring_buffer_read(rx)) << 4;
		msg->data[i] += hexascii_to_halfbyte(ring_buffer_read(rx));
	}

	/* Check command end */
	if (ring_buffer_peek(rx) != '\r')
	{
		return false;
	}

	/* Sending message */
	msg->header.RTR = CAN_RTR_DATA;
	msg->header.IDE = CAN_ID_STD;
	msg->header.ExtId = 0x0;

	return UART2CAN_CAN_Transmit(&hcan, msg);
}

/**
 *
 */
void
lawicel_proceed(rBuffer_t *rx, rBuffer_t *tx)
{
	uint8_t reply_data[32];
	size_t reply_size = 0;

	while (ring_buffer_available(rx))
	{
		if (ring_buffer_find(rx, '\r'))
		{
			switch (ring_buffer_peek(rx))
			{
				case LAWICEL_GET_HARDWARE:
					reply_size = sprintf((char*) reply_data,
											"vSTM32\r");
					break;

				case LAWICEL_GET_VERSION:
					reply_size = sprintf((char*) reply_data,
											"V%02d%02d\r",
											UART2CAN_VERSION_HW,
											UART2CAN_VERSION_SW);
					break;

				case LAWICEL_CLOSE_CAN:
					if (UART2CAN_CAN_Stop(&hcan))
					{
						HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
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
					if (UART2CAN_CAN_Start(&hcan))
					{
						HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
						reply_data[0] = LAWICEL_RESPONCE_OK;
						reply_size = 1;
					}
					else
					{
						reply_data[0] = LAWICEL_RESPONCE_ERROR;
						reply_size = 1;
					}
					break;

				case LAWICEL_TRANSMIT:
					//				//FIXME! echo
					//				do {
					//					reply_data[reply_size++] = ring_buffer_read(rx);
					//				} while (reply_data[reply_size - 1] != '\r');

					if (lawicel_can_transmit(rx, &canTxMsg))
					{
						reply_data[0] = LAWICEL_RESPONCE_OK;
						reply_size = 1;
					}
					else
					{
						reply_data[0] = LAWICEL_RESPONCE_ERROR;
						reply_size = 1;
					}
					break;

				case LAWICEL_TIMESTAMP:
					switch (hexascii_to_halfbyte(ring_buffer_at(rx, 1)))
					{
						case LAWICEL_TIMESTAMP_OFF:
							useTimeStamp = false;
							reply_data[0] = LAWICEL_RESPONCE_OK;
							reply_size = 1;
							break;

						case LAWICEL_TIMESTAMP_ON:
							useTimeStamp = true;
							reply_data[0] = LAWICEL_RESPONCE_OK;
							reply_size = 1;
							break;

						default:
							reply_data[0] = LAWICEL_RESPONCE_ERROR;
							reply_size = 1;
					}
					break;

				case LAWICEL_BITRATE:
					if (lawicel_can_set_bitrate(rx))
					{
						reply_data[0] = LAWICEL_RESPONCE_OK;
						reply_size = 1;
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
				ring_buffer_write_bytes(tx, reply_data, reply_size);
				reply_size = 0;

				/* Flush remaining data from RX buffer */
				ring_buffer_seek(rx, '\r');
				ring_buffer_move_read_index(rx, 1);
			}
			else
			{
				/* Skip one byte */
				ring_buffer_move_read_index(rx, 1);
			}
		}
	}

	/* Start UART Transmit if necessary */
	if (ring_buffer_available(tx) && !UART2CAN_UART_Is_Tx_Busy(&huart2))
	{
		UART2CAN_UART_Transmit(&huart2, tx);
	}
}

