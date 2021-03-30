/*
 * can_api.h
 *
 *  Created on: Oct 23, 2020
 *      Author: sergi
 */

#ifndef INC_CAN_API_H_
#define INC_CAN_API_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "can.h"

/* types */

typedef struct CanTxMessage
{
	CAN_TxHeaderTypeDef header;
	uint8_t data[8];
} CanTxMessage_t;

typedef struct CanRxMessage
{
	CAN_RxHeaderTypeDef header;
	uint8_t data[8];
} CanRxMessage_t;

typedef struct CAN_Instance
{
	CAN_HandleTypeDef *hcan;
	uint32_t bitrate;

	volatile uint32_t txMsgCounter;
	volatile uint32_t rxMsgCounter;
	volatile uint32_t errorCounter;
	volatile uint32_t restartCounter;
} CAN_Instance_t;

/* extern variables */

extern CAN_Instance_t *pCanInstance;

/* API functions */

ErrorStatus
CAN_stop(CAN_HandleTypeDef*);

ErrorStatus
CAN_start(CAN_HandleTypeDef*);

ErrorStatus
CAN_set_bitrate(CAN_HandleTypeDef*, uint32_t bitrate);

ErrorStatus
CAN_transmit(CAN_HandleTypeDef*, CAN_TxHeaderTypeDef *pHeader, uint8_t *pData);

ErrorStatus
CAN_on_receive(CAN_HandleTypeDef*, CAN_RxHeaderTypeDef *pHeader, uint8_t *pData);

#endif /* INC_CAN_API_H_ */
