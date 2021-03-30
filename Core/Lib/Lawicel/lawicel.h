/*
 * lawicel.h
 *
 *  Created on: Oct 8, 2020
 *      Author: sergi
 */

#ifndef INC_LAWICEL_H_
#define INC_LAWICEL_H_

#include "rbuffer.h"

/* UPLINK interface select */
#define LAWICEL_USB_AS_UPLINK
//#define LAWICEL_UART_AS_UPLINK

#if defined( LAWICEL_USB_AS_UPLINK )
#include "usbd_cdc_if.h"
#endif

#if defined( LAWICEL_UART_AS_UPLINK )
#include "usart.h"
#endif

#if !defined( LAWICEL_UART_AS_UPLINK ) && !defined( LAWICEL_USB_AS_UPLINK)
#error "LAWICEL: UART or USB uplink must be selected!"
#endif

/* version */

#define CONVERTER_VERSION_HW		"v002_EMU"
#define CONVERTER_VERSION_SW		"v007"

/* constants */

#define LAWICEL_RX_BUFFER_SIZE		128
#define LAWICEL_TX_BUFFER_SIZE		512
#define LAWICEL_CAN_TX_BUFFER_SIZE	512

#define LAWICEL_TIMER_MAX_MS		60000

/* LAWICEL protocol */

#define LAWICEL_RESPONCE_OK			0x0D
#define LAWICEL_RESPONCE_ERROR		0x07

#define LAWICEL_GET_VERSION			'v'
#define LAWICEL_GET_HARDWARE		'V'

#define LAWICEL_TRANSMIT_EXT		'T'
#define LAWICEL_TRANSMIT_STD		't'

#define LAWICEL_USE_TIMESTAMP		'Z'
#define LAWICEL_SET_BITRATE			'S'
#define LAWICEL_CLOSE_CAN			'C'
#define LAWICEL_OPEN_CAN			'O'

/* types */

typedef enum LAWICEL_TimestampState
{
	LAWICEL_TIMESTAMP_DISABLED = 0,
	LAWICEL_TIMESTAMP_ENABLED = 1,
} LAWICEL_TimestampState_t;

typedef struct LAWICEL_Instance
{
	UART_HandleTypeDef *huart;
	CAN_HandleTypeDef *hcan;

	rBuffer_t LawicelRx;
	rBuffer_t LawicelTx;
	rBuffer_t CanTx;

	LAWICEL_TimestampState_t TimestampState;
	uint32_t savedTicks;
	uint16_t timer;
} LAWICEL_Instance_t;

typedef struct CAN_Message
{
	struct
	{
		uint32_t ID;
		uint8_t IDE;
		uint8_t RTR;
		uint8_t DLC;
	} header;
	uint8_t data[8];
} CAN_Message_t;

/* exports */

extern LAWICEL_Instance_t *pLawicelInstance;

/* API functions */

ErrorStatus
LAWICEL_init(LAWICEL_Instance_t*, UART_HandleTypeDef *huart, CAN_HandleTypeDef *hcan);

ErrorStatus
LAWICEL_run(LAWICEL_Instance_t*);

ErrorStatus
LAWICEL_UPLINK_on_receive(LAWICEL_Instance_t*, uint8_t *pData, size_t size);

ErrorStatus
LAWICEL_UPLINK_transmit(LAWICEL_Instance_t*);

ErrorStatus
LAWICEL_CAN_on_receive(LAWICEL_Instance_t*, CAN_RxHeaderTypeDef *pHeader, uint8_t *pData);

ErrorStatus
LAWICEL_CAN_transmit(LAWICEL_Instance_t*);

/* callback funcrions */

void
LAWICEL_CAN_on_stop_callback(LAWICEL_Instance_t*);

void
LAWICEL_CAN_on_start_callback(LAWICEL_Instance_t*);

void
LAWICEL_CAN_on_transmit_callback(LAWICEL_Instance_t*, CAN_TxHeaderTypeDef *pHeader, uint8_t *pData);

void
LAWICEL_CAN_on_receive_callback(LAWICEL_Instance_t*, CAN_RxHeaderTypeDef *pHeader, uint8_t *pData);


//void
//lawicel_proceed(rBuffer_t *rx, rBuffer_t *tx);
//
//void
//lawicel_timer_tick();
//
//bool
//lawicel_can_set_bitrate(rBuffer_t *rx);
//
//bool
//lawicel_can_transmit(rBuffer_t *rx, CanTxMessage_t *msg);
//
//bool
//lawicel_can_receive(rBuffer_t *tx, CanRxMessage_t *msg);
#endif /* INC_LAWICEL_H_ */
