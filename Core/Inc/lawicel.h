/*
 * lawicel.h
 *
 *  Created on: Oct 8, 2020
 *      Author: sergi
 */

#ifndef INC_LAWICEL_H_
#define INC_LAWICEL_H_

#include "rbuffer.h"
#include "usart.h"
#include "can.h"

#define UART2CAN_VERSION_HW		1
#define UART2CAN_VERSION_SW		2

#define LAWICEL_TIMER_MAX_MS	60000

#define LAWICEL_RESPONCE_ERROR	0x07
#define LAWICEL_RESPONCE_OK		0x0D

#define LAWICEL_GET_HARDWARE	'v'
#define LAWICEL_GET_VERSION		'V'
#define LAWICEL_CLOSE_CAN		'C'
#define LAWICEL_OPEN_CAN		'O'
#define LAWICEL_TRANSMIT		't'

/* exports */
extern volatile uint16_t lawicelTimer;

/* prototypes */
void lawicel_proceed(rBuffer_t *rx, rBuffer_t *tx);
void lawicel_timer_tick();

bool lawicel_can_transmit(rBuffer_t *rx, CanTxMessage_t *msg);
bool lawicel_can_receive(rBuffer_t *tx, CanRxMessage_t *msg);

#endif /* INC_LAWICEL_H_ */
