/*
 * uco_LSS.h
 *
 *  Created on: Oct 9, 2020
 *      Author: sergi
 */

#ifndef LIB_UCANOPEN_UCO_LSS_H_
#define LIB_UCANOPEN_UCO_LSS_H_

#include <uCO_defs.h>

/* prototypes */

void
uco_lss_on_tick(uCO_t *p);

ErrorStatus
uco_proceed_lss_request(uCO_t *p, uint8_t *data);

ErrorStatus
uco_proceed_lss_responce(uCO_t *p, uint8_t *data);

/* API prototypes */

ErrorStatus
uco_lss_master_switch_mode_global(uCO_t *p, uCO_LSS_SlaveMode_t Mode);

ErrorStatus
uco_lss_master_start_fastscan(uCO_t *p);

ErrorStatus
uco_lss_master_set_node_id(uCO_t *p, uCO_NodeId_t NodeId);

/* Slave callback functions */

void
uco_lss_slave_on_waiting_mode(uCO_t *p);

void
uco_lss_slave_on_configuration_mode(uCO_t *p);

void
uco_lss_slave_on_configure_node_id(uCO_t *p);

void
uco_lss_slave_on_configure_bit_timing(uCO_t *p);

void
uco_lss_slave_on_activate_bit_timing(uCO_t *p);

void
uco_lss_slave_on_store_configuration(uCO_t *p);

/* Master callback functions */

void
uco_lss_master_on_fastscan_error(uCO_t *p, uCO_LSS_FastscanError_t);

void
uco_lss_master_on_fastscan_success(uCO_t *p);

void
uco_lss_master_on_fastscan_addr(uCO_t *p, uint8_t);

void
uco_lss_master_on_fastscan_end(uCO_t *p);

void
uco_lss_master_on_slave_in_config_mode(uCO_t *p, uint32_t *ADDR);

void
uco_lss_master_on_slave_configure_node_id(uCO_t *p, uCO_LSS_ConfigureNodeIdError_t err);

void
uco_lss_master_on_slave_configure_bit_timing(uCO_t *p, uCO_LSS_ConfigureBitTimingError_t err);

void
uco_lss_master_on_slave_store_configuration(uCO_t *p, uCO_LSS_StoreConfigurationError_t err);

void
uco_lss_master_on_slave_inquire_identity(uCO_t *p, uint32_t *ADDR, uint8_t index);

#endif /* LIB_UCANOPEN_UCO_LSS_H_ */
