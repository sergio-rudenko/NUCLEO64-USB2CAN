/*
 * uco_nmt_defs.h
 *
 *  Created on: Oct 27, 2020
 *      Author: sergi
 */

#ifndef LIB_UCANOPEN_UCO_NMT_DEFS_H_
#define LIB_UCANOPEN_UCO_NMT_DEFS_H_

#define UCANOPEN_COB_ID_NMT 							0x000
#define UCANOPEN_NMT_MESSAGE_LENGTH						2

#define UCANOPEN_COB_ID_HEARTBEAT						0x700
#define UCANOPEN_COB_ID_NODE_GUARDING					0x700
#define UCANOPEN_HEARTBEAT_MESSAGE_LENGTH				1

#define UCANOPEN_NMT_BROADCAST_MESSAGE					0x00

#define UCANOPEN_NMT_CS_START_REMOTE_NODE				0x01
#define UCANOPEN_NMT_CS_STOP_REMOTE_NODE				0x02
#define UCANOPEN_NMT_CS_ENTER_PRE_OPERATIONAL			0x80
#define UCANOPEN_NMT_CS_RESET_NODE						0x81
#define UCANOPEN_NMT_CS_RESET_COMMUNICATION				0x82

#define UCANOPEN_HEARTBEAT_BOOTUP_MESSAGE				0x00
#define UCANOPEN_HEARTBEAT_STATE_STOPPED				0x04
#define UCANOPEN_HEARTBEAT_STATE_OPERATIONAL			0x05
#define UCANOPEN_HEARTBEAT_STATE_PREOPERATIONAL 		0x7F

typedef struct uCO_NMT
{
	struct
	{
		bool sendOnSync;
		uint16_t Timestamp;
		uint16_t Timeout;
	} Heartbeat;

	struct
	{
		uint8_t lifeTimeFactor;
		uint16_t Timestamp;
		uint16_t Timeout;
	} NodeGuard;

	bool uplinkIsAlive;
} uCO_NMT_t;


#endif /* LIB_UCANOPEN_UCO_NMT_DEFS_H_ */
