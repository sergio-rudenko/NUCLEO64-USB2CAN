/*
 * LSS/slave.c
 *
 *  Created on: Oct 26, 2020
 *      Author: sergi
 */

#include "lss.h"

#ifdef UCANOPEN_LSS_SLAVE_ENABLED

static ErrorStatus
on_switch_mode_global(uCO_t *p, uint8_t mode)
{
	p->LSS.Slave.Mode = (uCO_LSS_SlaveMode_t) mode;

	if (p->LSS.Slave.Mode == LSS_SLAVE_MODE_WAITING)
		uco_lss_slave_on_waiting_mode(p);

	if (p->LSS.Slave.Mode == LSS_SLAVE_MODE_CONFIGURATION)
		uco_lss_slave_on_configuration_mode(p);

	return SUCCESS;
}

static ErrorStatus
on_switch_mode_selective(uCO_t *p, uint8_t cs, uint32_t addr)
{
	static uint8_t addrIndex = 0;

	/* Prepare reply message */
	uCO_CanMessage_t reply = { 0 };
	reply.CobId = UCANOPEN_COB_ID_LSS_RESPONCE;
	reply.length = UCANOPEN_LSS_LENGTH;

	/* Switch Slave mode to default */
	if (p->LSS.Slave.Mode == LSS_SLAVE_MODE_CONFIGURATION)
	{
		p->LSS.Slave.Mode = LSS_SLAVE_MODE_WAITING;
		uco_lss_slave_on_waiting_mode(p);
	}

	/* Proceed address */
	switch (cs)
	{
		case UCANOPEN_LSS_CS_SWITCH_MODE_SELECTIVE_ADDR1:
			if (addr == p->ADDR[0])
			{
				addrIndex = 1;
			}
			else
			{
				addrIndex = 0xFF;
			}
			break;

		case UCANOPEN_LSS_CS_SWITCH_MODE_SELECTIVE_ADDR2:
			if (addrIndex != 0xFF && addr == p->ADDR[addrIndex])
			{
				addrIndex = 2;
			}
			else
			{
				addrIndex = 0xFF;
			}
			break;

		case UCANOPEN_LSS_CS_SWITCH_MODE_SELECTIVE_ADDR3:
			if (addrIndex != 0xFF && addr == p->ADDR[addrIndex])
			{
				addrIndex = 3;
			}
			else
			{
				addrIndex = 0xFF;
			}
			break;

		case UCANOPEN_LSS_CS_SWITCH_MODE_SELECTIVE_ADDR4:
			if (addrIndex != 0xFF && addr == p->ADDR[addrIndex])
			{
				/* Switch to Slave Configuration Mode */
				p->LSS.Slave.Mode = LSS_SLAVE_MODE_CONFIGURATION;
				uco_lss_slave_on_configuration_mode(p);

				reply.data[0] = UCANOPEN_LSS_CS_SWITCH_MODE_SELECTIVE_REPLY;
				reply.data[1] = p->LSS.Slave.Mode;

				/* Send reply */
				uco_send(p, &reply);
			}
			else
			{
				addrIndex = 0xFF;
			}
			break;
	}
	return SUCCESS;
}

static ErrorStatus
on_configure_node_id(uCO_t *p, uint8_t nodeId)
{
	ErrorStatus result = ERROR;

	/* Prepare reply message */
	uCO_CanMessage_t reply = { 0 };
	reply.CobId = UCANOPEN_COB_ID_LSS_RESPONCE;
	reply.length = UCANOPEN_LSS_LENGTH;

	if (nodeId > 0 && nodeId <= 127)
	{
		p->NodeId = nodeId;
		uco_lss_slave_on_configure_node_id(p);

		reply.data[1] = LSS_CONFIGURE_NODE_ID_NO_ERROR;
		result = SUCCESS;
	}
	else
	{
		reply.data[1] = LSS_CONFIGURE_NODE_ID_OUT_OF_RANGE;
	}

	reply.data[0] = UCANOPEN_LSS_CS_CONFIGURE_NODE_ID;
	uco_send(p, &reply);

	return result;
}

static ErrorStatus
on_configure_bit_timing(uCO_t *p, uint8_t tableSelector, uint8_t tableIndex)
{
	ErrorStatus result = ERROR;

	/* Prepare reply message */
	uCO_CanMessage_t reply = { 0 };
	reply.CobId = UCANOPEN_COB_ID_LSS_RESPONCE;
	reply.length = UCANOPEN_LSS_LENGTH;

	//TODO
	UNUSED(tableSelector);
	UNUSED(tableIndex);

	uco_lss_slave_on_configure_bit_timing(p);

	reply.data[1] = LSS_CONFIGURE_BIT_TIMING_NOT_SUPPORTED;

	reply.data[0] = UCANOPEN_LSS_CS_CONFIGURE_BIT_TIMING;
	uco_send(p, &reply);

	return result;
}

static ErrorStatus
on_activate_bit_timing(uCO_t *p, uint16_t switchDelay)
{
	ErrorStatus result = ERROR;

	//TODO
	UNUSED(switchDelay);

	uco_lss_slave_on_activate_bit_timing(p);

	return result;
}

static ErrorStatus
on_store_configuration(uCO_t *p)
{
	ErrorStatus result = ERROR;

	/* Prepare reply message */
	uCO_CanMessage_t reply = { 0 };
	reply.CobId = UCANOPEN_COB_ID_LSS_RESPONCE;
	reply.length = UCANOPEN_LSS_LENGTH;

	//TODO
	uco_lss_slave_on_store_configuration(p);

	reply.data[1] = LSS_STORE_CONFIGURATION_NOT_SUPPORTED;

	reply.data[0] = UCANOPEN_LSS_CS_STORE_CONFIGURATION;
	uco_send(p, &reply);

	return result;
}

static ErrorStatus
on_inquire_identity(uCO_t *p, uint8_t cs)
{
	uint32_t addr;

	/* Prepare reply message */
	uCO_CanMessage_t reply = { 0 };
	reply.CobId = UCANOPEN_COB_ID_LSS_RESPONCE;
	reply.length = UCANOPEN_LSS_LENGTH;

	switch (cs)
	{
		case UCANOPEN_LSS_CS_INQUIRE_IDENTITY_ADDR1:
			addr = p->ADDR[0];
			break;

		case UCANOPEN_LSS_CS_INQUIRE_IDENTITY_ADDR2:
			addr = p->ADDR[1];
			break;

		case UCANOPEN_LSS_CS_INQUIRE_IDENTITY_ADDR3:
			addr = p->ADDR[2];
			break;

		case UCANOPEN_LSS_CS_INQUIRE_IDENTITY_ADDR4:
			addr = p->ADDR[3];
			break;
	}

	reply.data[1] = (addr) & 0xFF;
	reply.data[2] = (addr >> 8) & 0xFF;
	reply.data[3] = (addr >> 16) & 0xFF;
	reply.data[4] = (addr >> 24) & 0xFF;

	reply.data[0] = cs;
	uco_send(p, &reply);

	return SUCCESS;
}

static ErrorStatus
on_fastscan_request(uCO_t *p, uint8_t *pData)
{
	uint8_t LSSSub = pData[6];
	uint8_t LSSNext = pData[7];
	uint8_t BitChecked = pData[5];

	uint32_t IDNumber, ADDRMasked, mask;

	/* Prepare reply message */
	uCO_CanMessage_t
	reply =
	{
		.CobId = UCANOPEN_COB_ID_LSS_RESPONCE,
		.length = UCANOPEN_LSS_LENGTH,
		.data = { UCANOPEN_LSS_CS_IDENTIFY_SLAVE_REPLY,
			0, 0, 0, 0, 0, 0, 0,
		},
	};

	/* Switch Slave mode to default */
	if (p->LSS.Slave.Mode == LSS_SLAVE_MODE_CONFIGURATION)
	{
		p->LSS.Slave.Mode = LSS_SLAVE_MODE_WAITING;
		uco_lss_slave_on_waiting_mode(p);
	}

	if (BitChecked == 0x80 /* Initiate FastScan */)
	{
		p->LSS.Slave.FastScan.LSSPos = 0;
		uco_send(p, &reply);

		return SUCCESS;
	}
	else
	{
		if (p->LSS.Slave.FastScan.LSSPos != LSSSub)
		{
			/* Not participate */
			return ERROR;
		}

		mask = (uint32_t) 0xFFFFFFFF << BitChecked;

		/* Prepare IDNumber */
		IDNumber = pData[1]
			+ (pData[2] << 8)
			+ (pData[3] << 16)
			+ (pData[4] << 24);
		IDNumber &= mask;

		/* Prepare masked ADDR number */
		ADDRMasked = p->ADDR[p->LSS.Slave.FastScan.LSSPos];
		ADDRMasked &= mask;

		if (ADDRMasked != IDNumber)
		{
			if (LSSSub != LSSNext)
			{
				/* Address is NOT confirmed,
				   stop participating in this scan */
				p->LSS.Slave.FastScan.LSSPos = 0xFF;
			}
			/* Not participate */
			return ERROR;
		}

		if (BitChecked == 0 && (LSSNext < LSSSub))
		{
			/* Switch to Slave Configuration Mode */
			p->LSS.Slave.Mode = LSS_SLAVE_MODE_CONFIGURATION;
			uco_lss_slave_on_configuration_mode(p);
		}

		p->LSS.Slave.FastScan.LSSPos = LSSNext;
		uco_send(p, &reply);
	}
	return SUCCESS;
}

/** --------------------------------------------------------------------------------------------------
 * (DSP305 v2.2) 4.5.2 LSS finite state automaton
 * The LSS FSA is defined in Figure 2 and it shall provide the following states:
 * [0] Initial: Pseudo state, indicating the activation of the FSA.
 * [1] LSS waiting: In this state, only the services as defined in Table 1 shall be supported.
 * [2] LSS configuration: In this state, only the services as defined in Table 1 shall be supported.
 * [3] Final: Pseudo state, indicating the deactivation of the FSA.
 * ----------------------------------------------------------------------------[TABLE 1]--------------
 * Services 							LSS waiting 	LSS configuration
 * Switch state global 					yes 			yes
 * Switch state selective 				yes 			no
 * Activate bit timing parameters 		no 				yes
 * Configure bit timing parameters 		no 				yes
 * Configure node-ID 					no 				yes
 * Store configuration 					no				yes
 * Inquire LSS address 					no 				yes
 * Inquire node-ID 						no 				yes
 * LSS identify remote slave			yes             yes
 * LSS identify slave					yes				yes
 * LSS identify non-configured
 * remote slave 						yes 			yes
 * LSS identify non-configured slave	yes 			yes
 * LSS Fastscan 						yes 			no
 *
 * The LSS FSA shall support the following transitions as defined in Table 2.
 * ----------------------------------------------------------------------------[TABLE 2]
 * Transition 	Event(s) 											Actions(s)
 * [0]->[1]		Automatic transition after initial entry into
 * 				either NMT pre-operational state, or NMT stopped
 * 				state, or NMT reset communication sub-state with
 * 				node-ID equals 0xFF									none
 * [1]->[2]		LSS switch state global command with parameter
 * 				'configuration_switch' or switch state selective
 * 				command 											none
 * [2]->[1]		LSS switch state global command with parameter
 * 				'waiting_switch' 									none
 * [1]->[3]		Automatic transition if invalid node-ID has
 * 				been changed and the new node-ID has been
 * 				successfully stored in non-volatile memory
 * 				AND state switch to LSS waiting was commanded		none
 */
ErrorStatus
uco_proceed_lss_request(uCO_t *p, uint8_t *pData)
{
	ErrorStatus result = ERROR;

	/* FastScan Protocol */
	if (pData[0] == UCANOPEN_LSS_CS_FAST_SCAN_PROTOCOL &&
		p->NodeId == UCANOPEN_NODE_ID_UNCONFIGURED)
	{
		result = on_fastscan_request(p, pData);
	}
	else
	/**
	 * Switch mode global
	 */
	if (pData[0] == UCANOPEN_LSS_CS_SWITCH_MODE_GLOBAL &&
		pData[1] <= LSS_SLAVE_MODE_CONFIGURATION)
	{
		result = on_switch_mode_global(p, pData[1]);
	}
	else
	/**
	 * Switch mode selective
	 */
	if (pData[0] >= UCANOPEN_LSS_CS_SWITCH_MODE_SELECTIVE_ADDR1 &&
		pData[0] <= UCANOPEN_LSS_CS_SWITCH_MODE_SELECTIVE_ADDR4)
	{
		uint32_t
		addr = pData[1]
			+ (pData[2] << 8)
			+ (pData[3] << 16)
			+ (pData[4] << 24);

		result = on_switch_mode_selective(p, pData[0], addr);
	}
	else
	/**
	 * Set NodeId
	 */
	if (pData[0] == UCANOPEN_LSS_CS_CONFIGURE_NODE_ID &&
		p->LSS.Slave.Mode == LSS_SLAVE_MODE_CONFIGURATION)
	{
		result = on_configure_node_id(p, pData[1]);
	}
	else
	/**
	 * Set BitTiming
	 */
	if (pData[0] == UCANOPEN_LSS_CS_CONFIGURE_BIT_TIMING &&
		p->LSS.Slave.Mode == LSS_SLAVE_MODE_CONFIGURATION)
	{
		result = on_configure_bit_timing(p, pData[1], pData[2]);
	}
	else
	/**
	 * Activate BitTiming
	 */
	if (pData[0] == UCANOPEN_LSS_CS_ACTIVATE_BIT_TIMING &&
		p->LSS.Slave.Mode == LSS_SLAVE_MODE_CONFIGURATION)
	{
		uint16_t switchDelay = pData[1] + (pData[2] << 8);
		result = on_activate_bit_timing(p, switchDelay);
	}
	else
	/**
	 * Store Configuration
	 */
	if (pData[0] == UCANOPEN_LSS_CS_STORE_CONFIGURATION &&
		p->LSS.Slave.Mode == LSS_SLAVE_MODE_CONFIGURATION)
	{
		result = on_store_configuration(p);
	}
	else
	/**
	 * Inquire Identity
	 */
	if (pData[0] >= UCANOPEN_LSS_CS_INQUIRE_IDENTITY_ADDR1 &&
		pData[0] <= UCANOPEN_LSS_CS_INQUIRE_IDENTITY_ADDR4 &&
		p->LSS.Slave.Mode == LSS_SLAVE_MODE_CONFIGURATION)
	{
		result = on_inquire_identity(p, pData[0]);
	}
	else
	/**
	 * Identify remote slave
	 */
	if (pData[0] >= UCANOPEN_LSS_CS_IDENTIFY_REMOTE_SLAVE_ADDR1 &&
		pData[0] <= UCANOPEN_LSS_CS_IDENTIFY_REMOTE_SLAVE_ADDR4_HI)
	{
		//TODO
		result = SUCCESS;
	}

	return result;
}

/* callback functions */

/**
 *
 */
__weak void
uco_lss_slave_on_waiting_mode(uCO_t *p)
{
}

/**
 *
 */
__weak void
uco_lss_slave_on_configuration_mode(uCO_t *p)
{
}

/**
 *
 */
__weak void
uco_lss_slave_on_configure_node_id(uCO_t *p)
{
}

/**
 *
 */
__weak void
uco_lss_slave_on_configure_bit_timing(uCO_t *p)
{
}

/**
 *
 */
__weak void
uco_lss_slave_on_activate_bit_timing(uCO_t *p)
{
}

/**
 *
 */
__weak void
uco_lss_slave_on_store_configuration(uCO_t *p)
{
}

#endif /* UCANOPEN_LSS_SLAVE_ENABLED */
