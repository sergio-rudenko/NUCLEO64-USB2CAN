/*
 * uco_PDO.h
 *
 *  Created on: Oct 9, 2020
 *      Author: sergi
 */

#ifndef LIB_UCANOPEN_UCO_PDO_H_
#define LIB_UCANOPEN_UCO_PDO_H_

/** ---------------------------------------------------------
 * [CAN_and_CANOpen.pdf] 2.5.5 RPDO Communication Parameters
 * [CAN_and_CANOpen.pdf] 2.5.6 TPDO Communication Parameters
 * ----------------------------------------------------------
 *	Sub	| Name	 				| Data type
 *	------------------------------------------
 *	 0	| Number of entries		| UNSIGNED8
 *	 1	| COB ID				| UNSIGNED32
 *	 2	| Transmission type		| UNSIGNED8
 *	 3	| Inhibit Time			| UNSIGNED16
 *	 4	| Reserved				| UNSIGNED8
 *	 5	| Event Timer			| UNSIGNED16
 */
typedef enum uCO_PDO_TransmissionType
{
	IMMEDIATELY = 0,
	ON_SYNC
} uCO_PDO_TransmissionType_t;

typedef struct uCO_PDO
{
	uCO_CobId_t CobID;
	uCO_PDO_TransmissionType_t TransmissionType;
	uCO_Time_t InhibitTime;
	uint8_t reserved;
	uCO_Time_t EventTimer;
} uCO_PDO_t;

#endif /* LIB_UCANOPEN_UCO_PDO_H_ */
