/*
 * lawicel_impl.c
 *
 *  Created on: 26 июл. 2021 г.
 *      Author: sa100 (sergio.rudenko@gmail.com)
 */

#include "lawicel.h"
#include "ucanopen.h"




#ifdef UCANOPEN_LSS_MASTER_ENABLED

void
lawicel_on_lss_fastscan_complete(LAWICEL_Instance_t *p, const uint32_t slaveADDR[])
{
  uint8_t* data;
  size_t num_bytes;

  data = (uint8_t*) slaveADDR;
  num_bytes = 2/* 'Lc' */ +  sizeof(uint32_t) * 4/* 128bit */ * 2/* chars */; // 34 bytes

  if (ring_buffer_available_for_write(&p->LawicelTx) >= num_bytes)
  {
    ring_buffer_write(&p->LawicelTx, LAWICEL_CANOPEN_LSS);
    ring_buffer_write(&p->LawicelTx, LAWICEL_FASTSCAN_COMPLETE);

    /* Slave Address (ID) */
    for (int i = 0; i < 16; i++)
    {
      ring_buffer_write(&p->LawicelTx, halfbyte_to_hexascii((data[i]) >> 4));
      ring_buffer_write(&p->LawicelTx, halfbyte_to_hexascii((data[i])));
    }
    ring_buffer_write(&p->LawicelTx, '\r');
  }
  p->Flag.lssFastScan = false;
}

void
uco_lss_master_on_fastscan_success(uCO_t *p)
{
  lawicel_on_lss_fastscan_complete(pLawicelInstance, p->LSS.Master.SlaveADDR);
}


void
lawicel_on_lss_fastscan_error(LAWICEL_Instance_t *p, uint8_t errorCode)
{
  size_t num_bytes;

  num_bytes = 2/* 'Le' */ +  sizeof(uint8_t) * 2; // 4 bytes

  if (ring_buffer_available_for_write(&p->LawicelTx) >= num_bytes)
  {
    ring_buffer_write(&p->LawicelTx, LAWICEL_CANOPEN_LSS);
    ring_buffer_write(&p->LawicelTx, LAWICEL_FASTSCAN_ERROR);
    ring_buffer_write(&p->LawicelTx, halfbyte_to_hexascii(errorCode >> 4));
    ring_buffer_write(&p->LawicelTx, halfbyte_to_hexascii(errorCode));
    ring_buffer_write(&p->LawicelTx, '\r');
  }
  p->Flag.lssFastScan = false;
}

void
uco_lss_master_on_fastscan_error(uCO_t *p, uCO_LSS_FastscanError_t ErrorCode)
{
  lawicel_on_lss_fastscan_error(pLawicelInstance, ErrorCode);
}


LAWICEL_Response_t
lawicel_start_lss_fastscan(LAWICEL_Instance_t* p)
{
  p->Flag.lssFastScan = true;

  /* Prepare non-configured nodes to fastscan process */
  uco_lss_master_switch_mode_global(&uCO, LSS_SLAVE_MODE_WAITING);

  /* Searching next non-configured node */
  return (uco_lss_master_start_fastscan(&uCO) == SUCCESS) ?
         LAWICEL_RESPONSE_OK : LAWICEL_RESPONSE_ERROR;
}

///* Switch all non-configured nodes to WAITING mode */
//uco_lss_master_switch_mode_global(&uCO, LSS_SLAVE_MODE_WAITING);

///* Reset ALL Nodes */
//uco_nmt_master_send_command(&uCO, UCANOPEN_NMT_CS_RESET_COMMUNICATION, UCANOPEN_NMT_BROADCAST_MESSAGE);
//slaveNodeId = 100;

#endif /* UCANOPEN_LSS_MASTER_ENABLED */
