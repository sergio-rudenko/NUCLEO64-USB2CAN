/*
 * LSS/defs.h
 *
 *  Created on: Oct 26, 2020
 *      Author: sergi
 */

#ifndef LIB_UCANOPEN_UCO_LSS_DEFS_H_
#define LIB_UCANOPEN_UCO_LSS_DEFS_H_

/* Enable Master functions */
#define UCANOPEN_LSS_MASTER_ENABLED
/* Enable Slave functions*/
#define UCANOPEN_LSS_SLAVE_ENABLED

/* LSS Message */
#define UCANOPEN_COB_ID_LSS_REQUEST						0x7E5
#define UCANOPEN_COB_ID_LSS_RESPONCE 					0x7E4
#define UCANOPEN_LSS_LENGTH								8

/* Timeout */
#define UCANOPEN_LSS_DEFAULT_TIMEOUT					20

/* LSS Mode switch */
#define UCANOPEN_LSS_CS_SWITCH_MODE_GLOBAL				0x04
#define UCANOPEN_LSS_CS_SWITCH_MODE_SELECTIVE_ADDR1		0x40
#define UCANOPEN_LSS_CS_SWITCH_MODE_SELECTIVE_ADDR2		0x41
#define UCANOPEN_LSS_CS_SWITCH_MODE_SELECTIVE_ADDR3		0x42
#define UCANOPEN_LSS_CS_SWITCH_MODE_SELECTIVE_ADDR4		0x43
#define UCANOPEN_LSS_CS_SWITCH_MODE_SELECTIVE_REPLY		0x44

/* LSS set NodeID */
#define UCANOPEN_LSS_CS_CONFIGURE_NODE_ID				0x11

/* LSS set BitTiming */
#define UCANOPEN_LSS_CS_CONFIGURE_BIT_TIMING			0x13
#define UCANOPEN_LSS_CS_ACTIVATE_BIT_TIMING				0x15

/* LSS store configuration */
#define UCANOPEN_LSS_CS_STORE_CONFIGURATION				0x17

/* LSS inquire slave */
#define UCANOPEN_LSS_CS_INQUIRE_IDENTITY_ADDR1			0x5A
#define UCANOPEN_LSS_CS_INQUIRE_IDENTITY_ADDR2			0x5B
#define UCANOPEN_LSS_CS_INQUIRE_IDENTITY_ADDR3			0x5C
#define UCANOPEN_LSS_CS_INQUIRE_IDENTITY_ADDR4			0x5D

/* LSS identify remote slave */
#define UCANOPEN_LSS_CS_IDENTIFY_REMOTE_SLAVE_ADDR1		0x46
#define UCANOPEN_LSS_CS_IDENTIFY_REMOTE_SLAVE_ADDR2		0x47
#define UCANOPEN_LSS_CS_IDENTIFY_REMOTE_SLAVE_ADDR3_LOW	0x48
#define UCANOPEN_LSS_CS_IDENTIFY_REMOTE_SLAVE_ADDR3_HI	0x49
#define UCANOPEN_LSS_CS_IDENTIFY_REMOTE_SLAVE_ADDR4_LOW	0x4A
#define UCANOPEN_LSS_CS_IDENTIFY_REMOTE_SLAVE_ADDR4_HI	0x4B
#define UCANOPEN_LSS_CS_IDENTIFY_SLAVE_REPLY			0x4F

/* LSS FastScan */
#define UCANOPEN_LSS_CS_FAST_SCAN_PROTOCOL				0x51
#define UCANOPEN_LSS_FAST_SCAN_NON_CONFIGURED_TIMEOUT	20 /* for 125K */
#define UCANOPEN_LSS_FAST_SCAN_REPLY_TIMEOUT			10 /* for 125K */

typedef enum uCO_LSS_SlaveMode
{
	LSS_SLAVE_MODE_WAITING = 0,
	LSS_SLAVE_MODE_CONFIGURATION = 1,
} uCO_LSS_SlaveMode_t;

typedef enum uCO_LSS_MasterState
{
	LSS_MASTER_STATE_IDLE = 0,
	LSS_MASTER_STATE_AWAITING_SWITCH_MODE_REPLY,
	LSS_MASTER_STATE_AWAITING_CONFIGURE_NODE_ID_REPLY,
	LSS_MASTER_STATE_AWAITING_CONFIGURE_BIT_TIMING_REPLY,
	LSS_MASTER_STATE_AWAITING_STORE_CONFIGURATION_REPLY,
	LSS_MASTER_STATE_AWAITING_INQUIRE_IDENTITY_REPLY,
	LSS_MASTER_STATE_AWAITING_IDENTIFY_SLAVE_REPLY,
	LSS_MASTER_STATE_AWAITING_FASTSCAN_CONFIRM,
	LSS_MASTER_STATE_AWAITING_FASTSCAN_REPLY,
} uCO_LSS_MasterState_t;

typedef enum uCO_LSS_FastscanError
{
	LSS_FASTSCAN_NO_ERROR = 0,
	LSS_FASTSCAN_ERROR_BIT_CHECKED,
	LSS_FASTSCAN_ERROR_ADDR_CONFIRM,
} uCO_LSS_FastscanError_t;

typedef enum uCO_LSS_ConfigureNodeIdError
{
	LSS_CONFIGURE_NODE_ID_NO_ERROR = 0,
	LSS_CONFIGURE_NODE_ID_OUT_OF_RANGE = 1,
	LSS_CONFIGURE_NODE_ID_TIMEOUT_ERROR = 0xFE,
	LSS_CONFIGURE_NODE_ID_VENDOR_ERROR = 0xFF,
} uCO_LSS_ConfigureNodeIdError_t;

typedef enum uCO_LSS_ConfigureBitTimingError
{
	LSS_CONFIGURE_BIT_TIMING_NO_ERROR = 0,
	LSS_CONFIGURE_BIT_TIMING_NOT_SUPPORTED = 1,
	LSS_CONFIGURE_BIT_TIMING_TIMEOUT_ERROR = 0xFE,
	LSS_CONFIGURE_BIT_TIMING_VENDOR_ERROR = 0xFF,
} uCO_LSS_ConfigureBitTimingError_t;

typedef enum uCO_LSS_StoreConfigurationError
{
	LSS_STORE_CONFIGURATION_NO_ERROR = 0,
	LSS_STORE_CONFIGURATION_NOT_SUPPORTED = 1,
	LSS_STORE_CONFIGURATION_TIMEOUT_ERROR = 0xFE,
	LSS_STORE_CONFIGURATION_VENDOR_ERROR = 0xFF,
} uCO_LSS_StoreConfigurationError_t;

#ifdef UCANOPEN_LSS_SLAVE_ENABLED
/** --------------------------------------------------------------------------
 *  LSS Slave
 * -------------------------------------------------------------------------*/
typedef enum uCO_LSS_SlaveCallbackType
{
	LSS_SLAVE_ON_SWITCH_TO_WAITING_MODE = 0,
	LSS_SLAVE_ON_SWITCH_TO_CONFIGURATION_MODE,
	LSS_SLAVE_ON_CONFIGURE_NODE_ID,
	LSS_SLAVE_ON_CONFIGURE_BIT_TIMING,
	LSS_SLAVE_ON_ACTIVATE_BIT_TIMING,
	LSS_SLAVE_ON_STORE_CONFIGURATION,

	LSS_SLAVE_CB_COUNT, // Do not remove!
} uCO_LSS_SlaveCallbackType_t;

/* callback prototype */
typedef void
(*uCO_LSS_SlaveCallback_t)(void*);

typedef struct uCO_LSS_Slave
{
	uCO_LSS_SlaveMode_t Mode;
	uCO_LSS_SlaveCallback_t Callback[LSS_SLAVE_CB_COUNT];
	struct
	{
		uint8_t LSSPos;
	} FastScan;
} uCO_LSS_Slave_t;

#endif /* UCANOPEN_LSS_SLAVE_ENABLED */

#ifdef UCANOPEN_LSS_MASTER_ENABLED
/** --------------------------------------------------------------------------
 *  LSS Master
 * -------------------------------------------------------------------------*/
typedef enum uCO_LSS_MasterCallbackType
{
	LSS_MASTER_ON_SET_SLAVE_MODE = 0,
	LSS_MASTER_ON_SET_SLAVE_NODE_ID,
	LSS_MASTER_ON_SET_SLAVE_BIT_TIMING,
	LSS_MASTER_ON_STORE_SLAVE_CONFIGURATION,
	LSS_MASTER_ON_INQUIRE_SLAVE_IDENTITY,

	LSS_MASTER_CB_COUNT, // Do not remove!
} uCO_LSS_MasterCallbackType_t;

/* callback prototype */
typedef void
(*uCO_LSS_MasterCallback_t)(void*, uint32_t);

typedef struct uCO_LSS_Master
{
	uCO_LSS_MasterState_t State;
	uCO_LSS_MasterCallbackType_t Callback[LSS_MASTER_CB_COUNT];

	uint32_t SlaveADDR[4];

	struct
	{
		uint32_t IDNumber;
		uint8_t BitChecked;
		uint8_t LSSSub;
		uint8_t LSSNext;
	} FastScan;
} uCO_LSS_Master_t;

#endif /* UCANOPEN_LSS_MASTER_ENABLED */

typedef struct uCO_LSS
{
#ifdef UCANOPEN_LSS_MASTER_ENABLED

	uCO_LSS_Master_t Master;

#endif /* UCANOPEN_LSS_MASTER_ENABLED */

#ifdef UCANOPEN_LSS_SLAVE_ENABLED

	uCO_LSS_Slave_t Slave;

#endif /* UCANOPEN_LSS_SLAVE_ENABLED */

	uint16_t Timestamp;
	uint16_t Timeout;
} uCO_LSS_t;

#endif /* LIB_UCANOPEN_UCO_LSS_DEFS_H_ */
