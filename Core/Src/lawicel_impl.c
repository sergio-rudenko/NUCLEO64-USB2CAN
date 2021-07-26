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
  num_bytes = 2/* 'FC' */ +  sizeof(uint32_t) * 4/* 128bit */ * 2/* chars */; // 34 bytes
  num_bytes += (p->TimestampState == LAWICEL_TIMESTAMP_ENABLED) ? 4 : 0;

  if (ring_buffer_available_for_write(&p->LawicelTx) >= num_bytes)
  {
    ring_buffer_write(&p->LawicelTx, 'F');
    ring_buffer_write(&p->LawicelTx, 'C');

    /* Slave Address (ID) */
    for (int i = 0; i < 16; i++)
    {
      ring_buffer_write(&p->LawicelTx, halfbyte_to_hexascii((data[i]) >> 4));
      ring_buffer_write(&p->LawicelTx, halfbyte_to_hexascii((data[i])));
    }

    /* TimeStamp */
    if (p->TimestampState == LAWICEL_TIMESTAMP_ENABLED)
    {
      ring_buffer_write(&p->LawicelTx, halfbyte_to_hexascii((p->timer) >> 12));
      ring_buffer_write(&p->LawicelTx, halfbyte_to_hexascii((p->timer) >> 8));
      ring_buffer_write(&p->LawicelTx, halfbyte_to_hexascii((p->timer) >> 4));
      ring_buffer_write(&p->LawicelTx, halfbyte_to_hexascii((p->timer)));
    }

    ring_buffer_write(&p->LawicelTx, '\r');
  }
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

  num_bytes = 2/* 'FE' */ +  sizeof(uint8_t); // 3 bytes
  num_bytes += (p->TimestampState == LAWICEL_TIMESTAMP_ENABLED) ? 4 : 0;

  if (ring_buffer_available_for_write(&p->LawicelTx) >= num_bytes)
  {
    ring_buffer_write(&p->LawicelTx, 'F');
    ring_buffer_write(&p->LawicelTx, 'E');
    ring_buffer_write(&p->LawicelTx, errorCode);

    /* TimeStamp */
    if (p->TimestampState == LAWICEL_TIMESTAMP_ENABLED)
    {
      ring_buffer_write(&p->LawicelTx, halfbyte_to_hexascii((p->timer) >> 12));
      ring_buffer_write(&p->LawicelTx, halfbyte_to_hexascii((p->timer) >> 8));
      ring_buffer_write(&p->LawicelTx, halfbyte_to_hexascii((p->timer) >> 4));
      ring_buffer_write(&p->LawicelTx, halfbyte_to_hexascii((p->timer)));
    }

    ring_buffer_write(&p->LawicelTx, '\r');
  }
}

void
uco_lss_master_on_fastscan_error(uCO_t *p, uCO_LSS_FastscanError_t ErrorCode)
{
  lawicel_on_lss_fastscan_error(pLawicelInstance, ErrorCode);
}


LAWICEL_Response_t
lawicel_start_lss_fastscan(LAWICEL_Instance_t* p)
{
  /* FIXME */ uco_lss_master_switch_mode_global(&uCO, LSS_SLAVE_MODE_WAITING);

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
