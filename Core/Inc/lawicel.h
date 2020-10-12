/*
 * lawicel.h
 *
 *  Created on: Oct 8, 2020
 *      Author: sergi
 */

#ifndef INC_LAWICEL_H_
#define INC_LAWICEL_H_

#include "usart.h"
#include "can.h"

#define UART2CAN_VERSION_HW		1
#define UART2CAN_VERSION_SW		3

#define LAWICEL_TIMER_MAX_MS	60000

#define LAWICEL_RESPONCE_ERROR	0x07
#define LAWICEL_RESPONCE_OK		0x0D

#define LAWICEL_GET_HARDWARE	'v'
#define LAWICEL_GET_VERSION		'V'
#define LAWICEL_TIMESTAMP		'Z'
#define LAWICEL_CLOSE_CAN		'C'
#define LAWICEL_OPEN_CAN		'O'
#define LAWICEL_TRANSMIT		't'
#define LAWICEL_BITRATE			'S'

/* Timestamp */
#define LAWICEL_TIMESTAMP_OFF	0x00
#define LAWICEL_TIMESTAMP_ON	0x01




/* exports */
extern volatile uint16_t lawicelTimer;

/* prototypes */

void lawicel_proceed(rBuffer_t *rx, rBuffer_t *tx);
void lawicel_timer_tick();

bool lawicel_can_set_bitrate(rBuffer_t *rx);
bool lawicel_can_transmit(rBuffer_t *rx, CanTxMessage_t *msg);
bool lawicel_can_receive(rBuffer_t *tx, CanRxMessage_t *msg);

#endif /* INC_LAWICEL_H_ */
