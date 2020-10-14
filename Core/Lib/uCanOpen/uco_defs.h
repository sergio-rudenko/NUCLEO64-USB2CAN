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
#define UCANOPEN_COB_ID_MASK 	0x780

#define UCANOPEN_COB_ID_NMT 	0x000
#define UCANOPEN_COB_ID_SYNC 	0x080
#define UCANOPEN_COB_ID_EMCY 	0x080
#define UCANOPEN_COB_ID_TIME 	0x100

#define UCANOPEN_COB_ID_TPDO_1 	0x180
#define UCANOPEN_COB_ID_RPDO_1 	0x200
#define UCANOPEN_COB_ID_TPDO_2 	0x280
#define UCANOPEN_COB_ID_RPDO_2 	0x300
#define UCANOPEN_COB_ID_TPDO_3 	0x380
#define UCANOPEN_COB_ID_RPDO_3 	0x400
#define UCANOPEN_COB_ID_TPDO_4 	0x480
#define UCANOPEN_COB_ID_RPDO_4 	0x500

#define __IS_UCANOPEN_COB_ID_NMT(COB_ID)	(COB_ID == UCANOPEN_COB_ID_NMT)

#define __IS_UCANOPEN_COB_ID_SYNC(COB_ID)	(COB_ID == UCANOPEN_COB_ID_SYNC)

#define __IS_UCANOPEN_COB_ID_EMCY(COB_ID)	((COB_ID & UCANOPEN_COB_ID_MASK) == UCANOPEN_COB_ID_EMCY)

#define __IS_UCANOPEN_COB_ID_TPDO(COB_ID)	(((COB_ID & UCANOPEN_COB_ID_MASK) == UCANOPEN_COB_ID_TPDO_1) || \
											 ((COB_ID & UCANOPEN_COB_ID_MASK) == UCANOPEN_COB_ID_TPDO_2) || \
											 ((COB_ID & UCANOPEN_COB_ID_MASK) == UCANOPEN_COB_ID_TPDO_3) || \
											 ((COB_ID & UCANOPEN_COB_ID_MASK) == UCANOPEN_COB_ID_TPDO_4))

#define __IS_UCANOPEN_COB_ID_RPDO(COB_ID)	(((COB_ID & UCANOPEN_COB_ID_MASK) == UCANOPEN_COB_ID_RPDO_1) || \
											 ((COB_ID & UCANOPEN_COB_ID_MASK) == UCANOPEN_COB_ID_RPDO_2) || \
											 ((COB_ID & UCANOPEN_COB_ID_MASK) == UCANOPEN_COB_ID_RPDO_3) || \
											 ((COB_ID & UCANOPEN_COB_ID_MASK) == UCANOPEN_COB_ID_RPDO_4))

#define __IS_UCANOPEN_COB_ID_TSDO(COB_ID)	((COB_ID & UCANOPEN_COB_ID_MASK) == UCANOPEN_COB_ID_TSDO)
#define __IS_UCANOPEN_COB_ID_RSDO(COB_ID)	((COB_ID & UCANOPEN_COB_ID_MASK) == UCANOPEN_COB_ID_RSDO)

#define __UCANOPEN_NODE_ID_FROM_COB_ID(COB_ID)	(~(UCANOPEN_COB_ID_MASK) & COB_ID)

/* Endianess */
#define __UCANOPEN_16BIT_FROM_LE(x0, x1)			(x0 + (x1 << 8))
#define __UCANOPEN_32BIT_FROM_LE(x0, x1, x2, x3)	(x0 + (x1 << 8) + (x2 << 16) + (x3 << 24))

/* Object Dictionary Defines */
#define UCANOPEN_OD_RPDO_REGISTER_FIRST			0x1400
#define UCANOPEN_OD_RPDO_REGISTER_LAST			0x15FF
#define UCANOPEN_OD_TPDO_REGISTER_FIRST			0x1800
#define UCANOPEN_OD_TPDO_REGISTER_LAST			0x19FF
#define UCANOPEN_OD_MANUFACTURER_REGISTER_FIRST	0x2000
#define UCANOPEN_OD_MANUFACTURER_REGISTER_LAST	0x5FFF

/* types */
typedef int8_t uCO_NodeId_t;
#define UCANOPEN_NODE_ID_UNDEFINED	((uCO_NodeId_t) 0xFF)

typedef uint16_t uCO_CobId_t;
#define UCANOPEN_COB_ID_UNDEFINED	((uCO_CobId_t) 0xFFF)

typedef uint16_t uCO_Time_t;

typedef enum uCO_ErrorStatus
{
	UCANOPEN_SUCCESS = 0U,
	UCANOPEN_ERROR,
} uCO_ErrorStatus_t;

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

typedef enum uCO_OD_ItemType
{
	UNDEFINED = 0,
	UNSIGNED8 ,
	UNSIGNED16 ,
	UNSIGNED32 ,
	UNSIGNED64,
	VISIBLE_STRING,
	OCTET_STRING,
	SUBARRAY,
} uCO_OD_ItemType_t;

typedef enum uCO_OD_ItemAccess
{
	READ_ONLY = 0,
	READ_WRITE,
} uCO_OD_ItemAccess_t;

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

typedef struct uCO_OD_Item
{
	uint16_t index;
	uCO_OD_ItemType_t Type;
	uCO_OD_ItemAccess_t Access;
	void *address;
	size_t size;
} uCO_OD_Item_t;

typedef struct uCO
{
	uCO_NodeId_t NodeId;
	uCO_NodeState_t NodeState;
	uCO_BitTiming_t BitTiming;
	uCO_ErrorRegister_t ErrorRegister;

	uCO_Time_t HeartbeatTime;
	uCO_Time_t HeartbeatTimestamp;

	uint32_t UID[4]; // 128bit

	uCO_OD_Item_t *OD;

	rBuffer_t *rxBuf;
	rBuffer_t *txBuf;
} uCO_t;

/* externs */
extern uCO_t uCO;
extern const uCO_OD_Item_t uCO_OD[];

/* uCO API prototypes */

uCO_ErrorStatus_t
uco_send(uCO_t *p, uCO_CanMessage_t *m);

/* Object Dictionary prototypes */

uCO_OD_Item_t*
uco_find_OD_item(uCO_t *p, uint16_t id, uint8_t sub);

uCO_OD_Item_t*
uco_find_OD_RPDO_item(uCO_t *p, uint16_t id, uint8_t sub);

uCO_OD_Item_t*
uco_find_OD_TPDO_item(uCO_t *p, uint16_t id, uint8_t sub);

uCO_OD_Item_t*
uco_find_OD_Manufacturer_item(uCO_t *p, uint16_t id, uint8_t sub);

#endif /* LIB_UCANOPEN_UCO_DEFS_H_ */
