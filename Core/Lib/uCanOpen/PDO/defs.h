/*
 * PDO/defs.h
 *
 *  Created on: Oct 27, 2020
 *      Author: sergi
 */

#ifndef LIB_UCANOPEN_UCO_PDO_DEFS_H_
#define LIB_UCANOPEN_UCO_PDO_DEFS_H_

#define UCANOPEN_COB_ID_TPDO_1 				0x180
#define UCANOPEN_COB_ID_TPDO_2 				0x280
#define UCANOPEN_COB_ID_TPDO_3 				0x380
#define UCANOPEN_COB_ID_TPDO_4 				0x480

#define UCANOPEN_COB_ID_RPDO_1 				0x200
#define UCANOPEN_COB_ID_RPDO_2 				0x300
#define UCANOPEN_COB_ID_RPDO_3 				0x400
#define UCANOPEN_COB_ID_RPDO_4 				0x500

/* TPDO */
typedef struct uCO_TPDO
{
	bool sendOnSync;

	struct
	{
		void *address;
		size_t size;
	} data;

	uint16_t Timestamp;
	uint16_t Timeout;
} uCO_TPDO_t;

/* RPDO */
typedef struct uCO_RPDO
{
	bool enabled;
	struct
	{
		void *address;
		size_t size;
	} data;
} uCO_RPDO_t;

#endif /* LIB_UCANOPEN_UCO_PDO_DEFS_H_ */
