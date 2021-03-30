/*
 * uco_defs.h
 *
 *  Created on: Oct 9, 2020
 *      Author: sergi
 */

#ifndef LIB_UCANOPEN_UCO_DEFS_H_
#define LIB_UCANOPEN_UCO_DEFS_H_

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#include <rbuffer.h>

#include <uOD_defs.h>
#include "EMCY/defs.h"
#include "SYNC/defs.h"
#include "TIME/defs.h"
#include "LSS/defs.h"
#include "NMT/defs.h"
#include "PDO/defs.h"
#include "SDO/defs.h"

#include "can.h"

/** ----------------------------------------------------
 * [CAN_and_CANOpen.pdf] F.1 Pre-defined Connection Set
 * -----------------------------------------------------
 * The following table lists the default COB IDs used for the various CANopen
 * communication objects. This collection of defaults is referred to as the Pre-
 * defined Connection Set.
 *
 * --------[Table F.1 Pre-defined Connection Set]-------------
 * COB ID 		Used For 				 	Constructed Using
 * -----------------------------------------------------------
 * 000h 		NMT (Network Management) 	-
 * 001h 		Global Failsafe Command 	-
 * 071h – 076h 	Flying Master Protocol 		-
 * 080h 		SYNC 						-
 * 081h – 0FFh 	Emergency 					80h + Node ID
 * 100h 		Time Stamp 					-
 * 101h – 180h 	Safety Relevant
 * 				Data Objects				100h + Node ID
 * 181h – 1FFh 	Transmit PDO 1 				180h + Node ID
 * 201h – 27Fh 	Receive PDO 1 				200h + Node ID
 * 281h – 2FFh 	Transmit PDO 2 				280h + Node ID
 * 301h – 37Fh 	Receive PDO 2 				300h + Node ID
 * 381h – 3FFh 	Transmit PDO 3 				380h + Node ID
 * 401h – 47Fh 	Receive PDO 3 				400h + Node ID
 * 481h – 4FFh 	Transmit PDO 4 				480h + Node ID
 * 501h – 57Fh 	Receive PDO 4 				500h + Node ID
 * 581h – 5FFh 	Transmit SDO 				580h + Node ID
 * 601h – 67Fh 	Receive SDO 				600h + Node ID
 * 6E0h 		Dynamic SDO Request			-
 * 701h – 77Fh 	NMT Error Control
 * 				(Heartbeat and Node
 * 				Guarding) 					700h + Node ID
 */
#define UCANOPEN_COB_ID_MASK 				0x780
#define UCANOPEN_NODE_ID_MASK 				0x07F

#define __IS_UCANOPEN_COB_ID_NMT(COB_ID)			(COB_ID == UCANOPEN_COB_ID_NMT)

#define __IS_UCANOPEN_COB_ID_LSS_REQUEST(COB_ID)	(COB_ID == UCANOPEN_COB_ID_LSS_REQUEST)
#define __IS_UCANOPEN_COB_ID_LSS_RESPONCE(COB_ID)	(COB_ID == UCANOPEN_COB_ID_LSS_RESPONCE)

#define __IS_UCANOPEN_COB_ID_SYNC(COB_ID)			(COB_ID == UCANOPEN_COB_ID_SYNC)

#define __IS_UCANOPEN_COB_ID_EMCY(COB_ID)			((COB_ID & UCANOPEN_COB_ID_MASK) == UCANOPEN_COB_ID_EMCY)

#define __IS_UCANOPEN_COB_ID_TIME(COB_ID)			((COB_ID & UCANOPEN_COB_ID_MASK) == UCANOPEN_COB_ID_TIME)

#define __IS_UCANOPEN_COB_ID_TPDO(COB_ID)			(((COB_ID & UCANOPEN_COB_ID_MASK) == UCANOPEN_COB_ID_TPDO_1) || \
											 	 	 ((COB_ID & UCANOPEN_COB_ID_MASK) == UCANOPEN_COB_ID_TPDO_2) || \
													 ((COB_ID & UCANOPEN_COB_ID_MASK) == UCANOPEN_COB_ID_TPDO_3) || \
													 ((COB_ID & UCANOPEN_COB_ID_MASK) == UCANOPEN_COB_ID_TPDO_4))

#define __IS_UCANOPEN_COB_ID_RPDO(COB_ID)			(((COB_ID & UCANOPEN_COB_ID_MASK) == UCANOPEN_COB_ID_RPDO_1) || \
											 	 	 ((COB_ID & UCANOPEN_COB_ID_MASK) == UCANOPEN_COB_ID_RPDO_2) || \
													 ((COB_ID & UCANOPEN_COB_ID_MASK) == UCANOPEN_COB_ID_RPDO_3) || \
													 ((COB_ID & UCANOPEN_COB_ID_MASK) == UCANOPEN_COB_ID_RPDO_4))

#define __IS_UCANOPEN_COB_ID_TSDO(COB_ID)			((COB_ID & UCANOPEN_COB_ID_MASK) == UCANOPEN_COB_ID_TSDO)
#define __IS_UCANOPEN_COB_ID_RSDO(COB_ID)			((COB_ID & UCANOPEN_COB_ID_MASK) == UCANOPEN_COB_ID_RSDO)

#define __UCANOPEN_NODE_ID_FROM_COB_ID(COB_ID)		(COB_ID & ~(UCANOPEN_COB_ID_MASK))

/* Endianess */
#define __UCANOPEN_16BIT_FROM_LE(x0, x1)			(x0 + (x1 << 8))
#define __UCANOPEN_32BIT_FROM_LE(x0, x1, x2, x3)	(x0 + (x1 << 8) + (x2 << 16) + (x3 << 24))

/* Object Dictionary Defines */
#define UCANOPEN_OD_RPDO_REGISTER_FIRST				0x1400
#define UCANOPEN_OD_RPDO_REGISTER_LAST				0x15FF
#define UCANOPEN_OD_TPDO_REGISTER_FIRST				0x1800
#define UCANOPEN_OD_TPDO_REGISTER_LAST				0x19FF
#define UCANOPEN_OD_MANUFACTURER_REGISTER_FIRST		0x2000
#define UCANOPEN_OD_MANUFACTURER_REGISTER_LAST		0x5FFF

/* types */
typedef int8_t uCO_NodeId_t;
#define UCANOPEN_NODE_ID_UNCONFIGURED	((uCO_NodeId_t) 0xFF)

typedef uint16_t uCO_CobId_t;
#define UCANOPEN_COB_ID_UNDEFINED		((uCO_CobId_t) 0xFFF)

/* bitTiming */
typedef enum uCO_BitTiming
{
	CAN_BAUDRATE_1000K = 0x00,
	CAN_BAUDRATE_800K = 0x01,
	CAN_BAUDRATE_500K = 0x02,
	CAN_BAUDRATE_250K = 0x03,
	CAN_BAUDRATE_125K = 0x04,
	CAN_BAUDRATE_100K = 0x05,
	CAN_BAUDRATE_50K = 0x06,
	CAN_BAUDRATE_20K = 0x07,
	CAN_BAUDRATE_10K = 0x08,
} uCO_BitTiming_t;

typedef enum uCO_NodeState
{
	NODE_STATE_INITIALIZATION = 0,
	NODE_STATE_PREOPERATIONAL,
	NODE_STATE_OPERATIONAL,
	NODE_STATE_STOPPED,
} uCO_NodeState_t;

typedef struct uCO_CanMessage
{
	uCO_CobId_t CobId;
	uint8_t length;
	uint8_t data[8];
} uCO_CanMessage_t;

typedef union uCO_ErrorRegister
{
	struct
	{
		bool GenericError :1;
		bool Current :1;
		bool Voltage :1;
		bool Temperatuwre :1;
		bool CommunicationError :1;
		bool DeviceProfileDefinedError :1;
		bool Reserved :1;
		bool ManufacturerSpecificError :1;
	};
	uint8_t value;
} uCO_ErrorRegister_t;

/* Instance */
typedef struct uCO
{
	uint32_t ADDR[4]; // 128bit

	uCO_NodeId_t NodeId;
	uCO_NodeState_t NodeState;
	uCO_BitTiming_t BitTiming;
	uCO_ErrorRegister_t ErrorRegister;

	uint16_t Timestamp;
	uint32_t ticks;

	/* protocols */
	uCO_NMT_t NMT;
	uCO_LSS_t LSS;
	uCO_SDO_t SDO;

	/* PDO */
	uCO_TPDO_t TPDO[4];
	uCO_RPDO_t RPDO[4];

	/* Object Dictionary */
	uOD_Item_t *OD;

	/* buffers */
	rBuffer_t *rxBuf;
	rBuffer_t *txBuf;
} uCO_t;

/* externs */
extern uCO_t uCO;
extern const uOD_Item_t uCO_OD[];

/* uCO API prototypes */

ErrorStatus
uco_send(uCO_t *p, uCO_CanMessage_t *m);

ErrorStatus
uco_transmit_direct(uCO_t *p, uCO_CanMessage_t *umsg);

/* Object Dictionary prototypes */

uOD_Item_t*
uco_find_od_item(uCO_t *p, uint16_t id, uint8_t sub);

uOD_Item_t*
uco_find_od_rpdo_item(uCO_t *p, uint16_t id, uint8_t sub);

uOD_Item_t*
uco_find_od_tpdo_item(uCO_t *p, uint16_t id, uint8_t sub);

uOD_Item_t*
uco_find_od_manufacturer_item(uCO_t *p, uint16_t id, uint8_t sub);

#endif /* LIB_UCANOPEN_UCO_DEFS_H_ */
