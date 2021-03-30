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
//#define LAWICEL_USB_AS_UPLINK
#define LAWICEL_UART_AS_UPLINK

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

#define CONVERTER_VERSION_HW		"v001_EMU"
#define CONVERTER_VERSION_SW		"v008"

/* constants */

#define LAWICEL_RX_BUFFER_SIZE		128
#define LAWICEL_TX_BUFFER_SIZE		4096
#define LAWICEL_CAN_TX_BUFFER_SIZE	2048

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

typedef enum LAWICEL_CallbackType
{
	LAWICEL_ON_CAN_STOP = 0,
	LAWICEL_ON_CAN_START,
	LAWICEL_ON_CAN_RECEIVE,
	LAWICEL_ON_CAN_TRANSMIT,

	LAWICEL_CB_COUNT // Do not remove!
} LAWICEL_CallbackType_t;

typedef void
(*LAWICEL_Callback_t)(void*);

typedef struct LAWICEL_Instance
{
	UART_HandleTypeDef *huart;
	CAN_HandleTypeDef *hcan;

	rBuffer_t LawicelRx;
	rBuffer_t LawicelTx;
	rBuffer_t CanTx;

	LAWICEL_Callback_t Callback[LAWICEL_CB_COUNT];

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

ErrorStatus
LAWICEL_register_callback(LAWICEL_Instance_t*, LAWICEL_CallbackType_t Type, LAWICEL_Callback_t cb);

#endif /* INC_LAWICEL_H_ */
