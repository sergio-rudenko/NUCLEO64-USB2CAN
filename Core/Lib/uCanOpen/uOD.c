/*
 * uOD.c
 *
 *  Created on: Oct 12, 2020
 *      Author: sergi
 *
 * ---------------------------------------------------
 *  [CAN_and_CANOpen.pdf] 2.2.4 Communication Entries
 * ---------------------------------------------------
 * The communication entries in the Object Dictionary describe most of the aspects
 * of the CANopen communications used by the node. Many of the entries are or can
 * be made writeable, allowing configuration of a node by other nodes on the network.
 * The entries occupy the Index range 1000h – 1FFFh in the Object Dictionary.
 *
 * Table 2.7 gives an overview of all the communication entries. Following the
 * table, the mandatory entries are described to give some examples for available
 * entries. Mandatory entries are those that must be implemented in a node
 * in order to be CANopen compliant. An additional listing can be found in the
 * reference section and in the CANopen standard [CiADS301].
 *
 * ==================================================
 *  Index			| Name
 * ==================================================
 *  1000h			| Device Type
 *  1001h			| Error Register
 *  1002h			| Manufacturer Status Register
 *  1003h			| Pre-defined Error Field
 *  1005h			| COB ID SYNC
 *  1006h			| Communication Cycle Period
 *  1007h			| Synchronous Window Length
 *	1008h			| Manufacturer Device Name
 *	1009h			| Manufacturer Hardware Version
 *	100Ah			| Manufacturer Software Version
 *	100Ch			| Guard Time
 *	100Dh			| Life Time Factor
 *	1010h			| Store Parameters
 *	1011h			| Restore Default Parameters
 *	1012h			| COB ID Time
 *	1013h			| High Resolution Time Stamp
 *	1014h			| COB ID EMCY
 *	1015h			| Inhibit Time EMCY
 *	1016h			| Consumer Heartbeat Time
 *	1017h			| Producer Heartbeat Time
 *	1018h			| Identity Object
 *	1200h – 127Fh	| Server SDO Parameters
 *	1280h – 12FFh	| Client SDO Parameters
 *	1400h – 15FFh	| RxPDO Communication Parameters
 *	1600h – 17FFh	| RxPDO Mapping Parameters
 *	1800h – 19FFh	| TxPDO Communication Parameters
 *	1A00h – 1BFFh	| TxPDO Mapping Parameters
 *	2000h - 5FFFh	| Manufacturer Specific Registers
 *	------------------------------------------------------
 *	@Ref: [CAN_and_CANOpen.pdf] D.3 Communication Profile
 */

#include <uCO_defs.h>

/** -------------------------------------------------
 * [CAN_and_CANOpen.pdf] 2.2.5.1 Device Type (1000h)
 * --------------------------------------------------
 * The Device Type is a 32-bit value that describes in a limited way some of the
 * capabilities of the node. For example, it can describe if the node is a digital
 * input/output module, and if so, whether inputs and/or outputs are implemented.
 * 3.4.2.1	Index [1000h]: Device Type
 * The device profile for generic I/O specifies that the low word of the 32-bit
 * device type field contains 0191h (= 401d, the device profile number).
 * Bits 16 through 19 provide information about the type of I/O provided. There is
 * one bit each that can be set to signal the support of a specific I/O type
 * as listed in Table 3.32.
 *
 * Bit	| Description
 * ---------------------
 * 16 	| Digital Input
 * 17 	| Digital Output
 * 18 	| Analog Input
 * 19 	| Analog Output
 *
 * Example: 00030191h
 *          Digital input/output module 191h = 401, which is the number of the
 *          digital input/output device profile. 0003h = the module implements
 *          both digital inputs and outputs.
 */
const uint32_t uCO_ODI_1000_00 = 0x0UL;

/** ----------------------------------------------------
 * [CAN_and_CANOpen.pdf] 2.2.5.2 Error Register (1001h)
 * -----------------------------------------------------
 * The Error Register is an 8-bit value that can indicate if various generic errors have
 * occurred in the node, for example, current error, temperature error, communication
 * error, etc. The only bit that must be implemented is the generic error bit. There is a
 * manufacturer specific bit available to indicate an application specific error. This byte
 * is also transmitted in Emergency Objects.
 *
 * Description:		The error register value indicates if various types of errors have
 * 					occurred. The following table indicates the bits used. Bit zero must be
 * 					implemented. All other bits are optional.
 *
 * Bit	|	Description				| Mandatory
 * ------------------------------------------
 * 0 	|	Generic Error 			| Yes
 * 1 	|	Current 				| No
 * 2 	|	Voltage 				| No
 * 3 	|	Temperature 			| No
 * 4 	|	Communication Error 	| No
 * 5 	|	Device Profile			|
 * 		|	defined Error 			| No
 * 6 	|	Reserved (always zero) 	| No
 * 7 	|	Manufacturer Specific	|
 * 		|	error 					| No
 */
#define uCO_ODI_1001_00	uCO.ErrorRegister.value

/** ----------------------------------------------------------------
 * [CAN_and_CANOpen.pdf] D.3.4 Manufacturer Status Register (1002h)
 * -----------------------------------------------------------------
 * 	Index			1002h
 * 	Name			Manufacturer Status Register
 * 	Mandatory		No
 *	Subindex		00h
 * 	Name			Manufacturer Status Register
 * 	Type			UNSIGNED32
 * 	Default Value	Not defined
 * 	Access			Read Only
 * 	Mandatory		No
 * 	Map to PDO		Yes
 *
 * Description:		The Manufacturer Status register contents are undefined in the CAN-
 * 					open specification. Manufacturers may use this entry for any purpose
 * 					desired.
 */
uint32_t uCO_ODI_1002_00 = 0xDEADBEEF;

/**-------------------------------------------------------------
 * [CAN_and_CANOpen.pdf] D.3.9 Manufacturer Device Name (1008h)
 * -------------------------------------------------------------
 */
const char uCO_ODI_1008_00[] = "BASTION";

/**-------------------------------------------------------------------
 * [CAN_and_CANOpen.pdf] D.3.10 Manufacturer Hardware Version (1009h)
 * -------------------------------------------------------------------
 */
const char uCO_ODI_1009_00[] = "NUCLEO F072 Prototype 1.1";

/**-------------------------------------------------------------------
 * [CAN_and_CANOpen.pdf] D.3.11 Manufacturer Software Version (100Ah)
 * -------------------------------------------------------------------
 */
const char uCO_ODI_100A_00[] = "v007_DEV";

/** -------------------------------------------------
 * [CAN_and_CANOpen.pdf] 2.2.5.3 Guard Time (100Ch)
 * --------------------------------------------------
 * The Guard Time is a 16-bit value that specifies how frequently the node guarding request
 * is transmitted by the master or must be received by the node. This entry must be imple-
 * mented if heartbeats are not used.
 */
#define uCO_ODI_100C_00	uCO.NMT.NodeGuard.Timeout


/** ------------------------------------------------------
 * [CAN_and_CANOpen.pdf] 2.2.5.4 Life Time Factor (100Dh)
 * -------------------------------------------------------
 * The Life Time Factor is an 8-bit value that works with the Guard Time. It specifies
 * how many multiples of the Guard Time must pass without transmission from the
 * master or reception of a response from a slave before an error condition is generated.
 * This entry must be implemented if heartbeats are not used.
 */
#define uCO_ODI_100D_00	uCO.NMT.NodeGuard.lifeTimeFactor

/** -------------------------------------------------------------
 * [CAN_and_CANOpen.pdf] 2.2.5.5 Producer Heartbeat Time (1017h)
 * --------------------------------------------------------------
 * If the node is not using node guarding then it must implement heartbeats. This entry
 * specifies how often the node should transmit heartbeat messages. It can be set to zero,
 * however, to disable heartbeat transmission. This entry must be implemented if node
 * guarding is not used.
 *
 * Description:		A node must support either node guarding or heartbeat generation. If
 * 					the node generates heartbeats then this entry must be implemented.
 * 					The value of the entry specifies in milliseconds the time between
 * 					transmission of heartbeat messages. A value of zero disables trans-
 * 					mission of heartbeat messages by the node.
 * 					Because the entry is writeable, the value of the entry may change at
 * 					any time.
 * 					Note that a node must implement either heartbeats, node guarding or
 * 					both heartbeats and node guarding.
 *
 * Example:			4455h = 17493
 * 					The node will transmit a heartbeat message every 17.493 seconds
 */
#define uCO_ODI_1017_00 uCO.NMT.Heartbeat.Timeout

/** -----------------------------------------------------
 * [CAN_and_CANOpen.pdf] 2.2.5.6 Identity Object (1018h)
 * ------------------------------------------------------
 * The Identity Object provides identifying information about the node. It must contain
 * at a minimum the CAN In Automation assigned Vendor ID, which is unique to a par-
 * ticular vendor. It may also contain a product code to identify the product the node is
 * in, a revision number and a serial number.
 *
 * Description:		The Identity entry provides some basic information about the node in
 * 					order to provide a standard way of differentiating between different
 * 					versions of a node.
 * 					All nodes must implement Subindexes 00h and 01h. The remaining
 * 					Subindexes are optional.
 * 					The Vendor ID is a unique ID assigned to each CANopen vendor by
 * 					CAN in Automation. This allows the source of the node to be identi-
 * 					fied.
 *
 * The product code and serial number formats are manufacturer specific, however the
 * revision number has the following format:
 * Bits				Description
 * 0 – 15			Minor Revision Number
 * 					Identifies different versions of the node where the CANopen behav-
 * 					ior has not changed.
 * 16 – 31			Major Revision Number
 * 					Identifies different versions of the node where the CANopen behav-
 * 					ior has changed.
 *
 * For example, if a new version of the node is produced with any difference in the
 * CANopen messages, transmission types, Object Dictionary entries, etc., then the
 * major revision number must be increased, otherwise the minor revision number must
 * be increased.
 *
 * Example:			00050001h
 * 					Revision number 5.1
 *
 * NOTE: uCanOpen realization on stm32 use MCU ID as 01..03 sub`s
 */
const uint8_t uCO_ODI_1018_00 = 4; /* Number of Entries */
const uOD_Item_t uCO_ODI_1018[] =
{
	/* sub | type | access | address */
	{ 0x00, UNSIGNED8, READ_ONLY, (void*) &uCO_ODI_1018_00, sizeof(uCO_ODI_1018_00) },
	{ 0x01, UNSIGNED32, READ_ONLY, (void*) &uCO.ADDR[0], sizeof(uCO.ADDR[0]) },
	{ 0x02, UNSIGNED32, READ_ONLY, (void*) &uCO.ADDR[1], sizeof(uCO.ADDR[1]) },
	{ 0x03, UNSIGNED32, READ_ONLY, (void*) &uCO.ADDR[2], sizeof(uCO.ADDR[2]) },
	{ 0x04, UNSIGNED32, READ_ONLY, (void*) &uCO.ADDR[3], sizeof(uCO.ADDR[3]) },
};

/**
 * uCacOpen Object Dictionary
 */
const uOD_Item_t uCO_OD[] =
{
	/* index | type | access | address */
	{ 0x1000, UNSIGNED32, READ_ONLY, (void*) &uCO_ODI_1000_00 }, 	// mandatory
	{ 0x1001, UNSIGNED8, READ_ONLY, (void*) &uCO_ODI_1001_00 },		// mandatory
	{ 0x1002, UNSIGNED32, READ_ONLY, (void*) &uCO_ODI_1002_00 },	// optional
	{ 0x1008, VISIBLE_STRING, READ_ONLY, (void*) &uCO_ODI_1008_00, sizeof(uCO_ODI_1008_00) },	// optional
	{ 0x1009, VISIBLE_STRING, READ_ONLY, (void*) &uCO_ODI_1009_00, sizeof(uCO_ODI_1009_00) },	// optional
	{ 0x100A, VISIBLE_STRING, READ_ONLY, (void*) &uCO_ODI_100A_00, sizeof(uCO_ODI_100A_00) },	// optional
	{ 0x100C, UNSIGNED16, READ_WRITE, (void*) &uCO_ODI_100C_00 },	// mandatory
	{ 0x100D, UNSIGNED8, READ_WRITE, (void*) &uCO_ODI_100D_00 },	// mandatory
	{ 0x1017, UNSIGNED16, READ_WRITE, (void*) &uCO_ODI_1017_00 },	// mandatory
	{ 0x1018, SUBARRAY, READ_ONLY, (void*) &uCO_ODI_1018 },			// mandatory

	{ 0xFFFF, UNDEFINED, READ_ONLY, NULL } // End of dictionary, mandatory
};

//
