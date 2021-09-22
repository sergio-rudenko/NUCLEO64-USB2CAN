/*
 * LSS/master.c
 *
 *  Created on: Oct 26, 2020
 *      Author: sergi
 */

#include "lss.h"

#ifdef UCANOPEN_LSS_MASTER_ENABLED

static inline void
set_lss_timeout(uCO_t *p, uint16_t timeout)
{
  p->LSS.Timestamp = p->Timestamp;
  p->LSS.Timeout = timeout;
}

static inline void
reset_lss_timeout(uCO_t *p)
{
  p->LSS.Timestamp = p->Timestamp;
  p->LSS.Timeout = 0;
}

static ErrorStatus
send_factscan_request(uCO_t *p)
{
  /* Request message */
  uCO_CanMessage_t request =
    {
      .CobId = UCANOPEN_COB_ID_LSS_REQUEST,
      .length = UCANOPEN_LSS_LENGTH,
      .data =
        {
          /*0*/UCANOPEN_LSS_CS_FAST_SCAN_PROTOCOL,
          /*1*/(p->LSS.Master.FastScan.IDNumber) & 0xFF,
          /*2*/(p->LSS.Master.FastScan.IDNumber >> 8) & 0xFF,
          /*3*/(p->LSS.Master.FastScan.IDNumber >> 16) & 0xFF,
          /*4*/(p->LSS.Master.FastScan.IDNumber >> 24) & 0xFF,
          /*5*/ p->LSS.Master.FastScan.BitChecked,
          /*6*/ p->LSS.Master.FastScan.LSSSub,
          /*7*/ p->LSS.Master.FastScan.LSSNext,
        },
    };

  /* Set timeout */
  if (p->LSS.Master.FastScan.BitChecked == 0x80)
    set_lss_timeout(p, UCANOPEN_LSS_FAST_SCAN_NON_CONFIGURED_TIMEOUT);
  else
    set_lss_timeout(p, UCANOPEN_LSS_FAST_SCAN_REPLY_TIMEOUT);

  return uco_send(p, &request);
}

static ErrorStatus
on_fastscan_reply_timeout(uCO_t *p)
{
  ErrorStatus result = SUCCESS;

  if (p->LSS.Master.FastScan.BitChecked == 0x80)
  {
    /* NoError */
    uco_lss_master_on_fastscan_error(p, LSS_FASTSCAN_NO_ERROR);

    /* No more non-configured node(s) */
    p->LSS.Master.State = LSS_MASTER_STATE_IDLE;
    uco_lss_master_on_fastscan_end(p);
  }
  else
  {
    /* '1' in BitChecked position */
    p->LSS.Master.FastScan.IDNumber |=
      1 << p->LSS.Master.FastScan.BitChecked;

    /* BitChecked decrement */
    if (p->LSS.Master.FastScan.BitChecked >= 1)
      p->LSS.Master.FastScan.BitChecked -= 1;

    p->LSS.Master.State = LSS_MASTER_STATE_AWAITING_FASTSCAN_REPLY;
    send_factscan_request(p);
  }
  return result;
}

static ErrorStatus
on_fastscan_reply(uCO_t *p)
{
  ErrorStatus result = ERROR;

  if (p->LSS.Master.FastScan.BitChecked == 0x80)
  {
    /* Non-Configured Slave(s) present, proceed scan */
    p->LSS.Master.FastScan.BitChecked = 31;

    uco_lss_master_on_fastscan_addr(p, p->LSS.Master.FastScan.LSSSub);
    result = send_factscan_request(p);
  }
  else
    if (p->LSS.Master.FastScan.BitChecked >= 1 &&
        p->LSS.Master.FastScan.BitChecked <= 31)
    {
      /* '0' in BitChecked position confirmed */
      if (p->LSS.Master.FastScan.BitChecked >= 1)
        p->LSS.Master.FastScan.BitChecked -= 1;

      p->LSS.Master.State = LSS_MASTER_STATE_AWAITING_FASTSCAN_REPLY;
      result = send_factscan_request(p);
    }
    else
      if (p->LSS.Master.FastScan.BitChecked == 0)
      {
        /* Current LSS number determined, send complete number again */
        p->LSS.Master.State = LSS_MASTER_STATE_AWAITING_FASTSCAN_CONFIRM;

        /* Prepare Slave(s) to next scan */
        p->LSS.Master.FastScan.LSSNext += 1;
        p->LSS.Master.FastScan.LSSNext %= 4;

        result = send_factscan_request(p);
      }
      else
      {
        /* Unsupported BitChecked value... */
        uco_lss_master_on_fastscan_error(p, LSS_FASTSCAN_ERROR_BIT_CHECKED);
      }
  return result;
}

static ErrorStatus
on_fastscan_confirm(uCO_t *p)
{
  ErrorStatus result = ERROR;

  /* Save Slave address */
  p->LSS.Master.SlaveADDR[p->LSS.Master.FastScan.LSSSub] =
    p->LSS.Master.FastScan.IDNumber;

  if (p->LSS.Master.FastScan.LSSNext == 0)
  {
    p->LSS.Master.State = LSS_MASTER_STATE_IDLE;
    uco_lss_master_on_fastscan_success(p);

    /* Slave determined and ready to Configuring */
    uco_lss_master_on_slave_in_config_mode(p, p->LSS.Master.SlaveADDR);
  }
  else
  {
    /* Start next part scan */
    p->LSS.Master.FastScan.LSSSub = p->LSS.Master.FastScan.LSSNext;
    p->LSS.Master.FastScan.BitChecked = 31;
    p->LSS.Master.FastScan.IDNumber = 0UL;

    uco_lss_master_on_fastscan_addr(p, p->LSS.Master.FastScan.LSSSub);

    p->LSS.Master.State = LSS_MASTER_STATE_AWAITING_FASTSCAN_REPLY;
    result = send_factscan_request(p);
  }
  return result;
}

static ErrorStatus
on_identify_slave(uCO_t *p)
{
  //TODO
  return ERROR;
}

/**
 *
 */
void
uco_lss_on_tick(uCO_t *p)
{
  uint16_t dt;

  /* LSS process timeout */
  if (p->LSS.Timeout)
  {
    dt = p->Timestamp - p->LSS.Timestamp;

    if (dt >= p->LSS.Timeout)
    {
      reset_lss_timeout(p);

      /* Request timeout */
      switch (p->LSS.Master.State)
      {
        case LSS_MASTER_STATE_AWAITING_FASTSCAN_REPLY:
          on_fastscan_reply_timeout(p);
          break;

        case LSS_MASTER_STATE_AWAITING_FASTSCAN_REPLY_PAUSE:
          on_fastscan_reply(p);
          break;

        case LSS_MASTER_STATE_AWAITING_FASTSCAN_CONFIRM:
          /* Address value not confirmed... */
          uco_lss_master_on_fastscan_error(p, LSS_FASTSCAN_ERROR_ADDR_CONFIRM);
          p->LSS.Timeout = 0;
          break;

        case LSS_MASTER_STATE_AWAITING_CONFIGURE_NODE_ID_REPLY:
          p->LSS.Master.State = LSS_MASTER_STATE_IDLE;
          uco_lss_master_on_slave_configure_node_id(p, LSS_CONFIGURE_NODE_ID_TIMEOUT_ERROR);
          break;

        case LSS_MASTER_STATE_AWAITING_CONFIGURE_BIT_TIMING_REPLY:
          p->LSS.Master.State = LSS_MASTER_STATE_IDLE;
          uco_lss_master_on_slave_configure_bit_timing(p, LSS_CONFIGURE_BIT_TIMING_TIMEOUT_ERROR);
          break;

        case LSS_MASTER_STATE_AWAITING_STORE_CONFIGURATION_REPLY:
          p->LSS.Master.State = LSS_MASTER_STATE_IDLE;
          uco_lss_master_on_slave_store_configuration(p, LSS_STORE_CONFIGURATION_TIMEOUT_ERROR);
          break;

        case LSS_MASTER_STATE_AWAITING_SWITCH_MODE_REPLY:
          p->LSS.Master.State = LSS_MASTER_STATE_IDLE;
          //TODO: ???
          break;

        default:
          p->LSS.Master.State = LSS_MASTER_STATE_IDLE;
          break;
      }
    }
  }
}

/**
 *
 */
ErrorStatus
uco_proceed_lss_responce(uCO_t *p, uint8_t *data)
{
  ErrorStatus result = ERROR;

  if (data[0] == UCANOPEN_LSS_CS_IDENTIFY_SLAVE_REPLY)
  {
    switch (p->LSS.Master.State)
    {
      case LSS_MASTER_STATE_AWAITING_FASTSCAN_REPLY:
      case LSS_MASTER_STATE_AWAITING_FASTSCAN_REPLY_PAUSE:
        p->LSS.Master.State = LSS_MASTER_STATE_AWAITING_FASTSCAN_REPLY_PAUSE;
        set_lss_timeout(p, UCANOPEN_LSS_FAST_SCAN_REPLY_PAUSE);
        break;

      case LSS_MASTER_STATE_AWAITING_IDENTIFY_SLAVE_REPLY:
        p->LSS.Master.State = LSS_MASTER_STATE_IDLE;
        reset_lss_timeout(p);

        result = on_identify_slave(p);
        break;

      case LSS_MASTER_STATE_AWAITING_FASTSCAN_CONFIRM:
        result = on_fastscan_confirm(p);
        break;

      default:
        break;
    }
  }
  else
    if (data[0] == UCANOPEN_LSS_CS_SWITCH_MODE_SELECTIVE_REPLY)
    {
      p->LSS.Master.State = LSS_MASTER_STATE_IDLE;
      reset_lss_timeout(p);

      /* Slave determined and ready to Configuring */
      uco_lss_master_on_slave_in_config_mode(p, p->LSS.Master.SlaveADDR);
      result = SUCCESS;
    }
    else
      if (data[0] == UCANOPEN_LSS_CS_CONFIGURE_NODE_ID &&
          p->LSS.Master.State == LSS_MASTER_STATE_AWAITING_CONFIGURE_NODE_ID_REPLY)
      {
        p->LSS.Master.State = LSS_MASTER_STATE_IDLE;
        reset_lss_timeout(p);

        /* Slave return result */
        uco_lss_master_on_slave_configure_node_id(p, data[1]);
        result = SUCCESS;
      }
      else
        if (data[0] == UCANOPEN_LSS_CS_CONFIGURE_BIT_TIMING &&
            p->LSS.Master.State == LSS_MASTER_STATE_AWAITING_CONFIGURE_BIT_TIMING_REPLY)
        {
          p->LSS.Master.State = LSS_MASTER_STATE_IDLE;
          reset_lss_timeout(p);

          /* Slave return result */
          uco_lss_master_on_slave_configure_bit_timing(p, data[1]);
          result = SUCCESS;
        }
        else
          if (data[0] == UCANOPEN_LSS_CS_STORE_CONFIGURATION &&
              p->LSS.Master.State == LSS_MASTER_STATE_AWAITING_STORE_CONFIGURATION_REPLY)
          {
            p->LSS.Master.State = LSS_MASTER_STATE_IDLE;
            reset_lss_timeout(p);

            /* Slave return result */
            uco_lss_master_on_slave_store_configuration(p, data[1]);
            result = SUCCESS;
          }
          else
            if (data[0] == UCANOPEN_LSS_CS_INQUIRE_IDENTITY_ADDR1 ||
                data[0] == UCANOPEN_LSS_CS_INQUIRE_IDENTITY_ADDR2 ||
                data[0] == UCANOPEN_LSS_CS_INQUIRE_IDENTITY_ADDR3 ||
                data[0] == UCANOPEN_LSS_CS_INQUIRE_IDENTITY_ADDR4)
            {
              if (p->LSS.Master.State == LSS_MASTER_STATE_AWAITING_INQUIRE_IDENTITY_REPLY)
              {
                uint8_t index;

                switch (data[0])
                {
                  case UCANOPEN_LSS_CS_INQUIRE_IDENTITY_ADDR1:
                    index = 0;
                    break;
                  case UCANOPEN_LSS_CS_INQUIRE_IDENTITY_ADDR2:
                    index = 1;
                    break;
                  case UCANOPEN_LSS_CS_INQUIRE_IDENTITY_ADDR3:
                    index = 2;
                    break;
                  case UCANOPEN_LSS_CS_INQUIRE_IDENTITY_ADDR4:
                    index = 3;
                    break;
                }

                p->LSS.Master.State = LSS_MASTER_STATE_IDLE;
                reset_lss_timeout(p);

                /* Slave return result */
                p->LSS.Master.SlaveADDR[index] = data[1] + (data[2] << 8) + (data[3] << 16) + (data[4] << 24);
                uco_lss_master_on_slave_inquire_identity(p, p->LSS.Master.SlaveADDR, index);
                result = SUCCESS;
              }
            }
            else
            {
              /* Can`t proceed Slave reply... */
            }
  return result;
}

/**
 *
 */
ErrorStatus
uco_lss_master_start_fastscan(uCO_t *p)
{
  /* Initiate variables */
  p->LSS.Master.FastScan.BitChecked = 0x80;
  p->LSS.Master.FastScan.IDNumber = 0UL;
  p->LSS.Master.FastScan.LSSNext = 0;
  p->LSS.Master.FastScan.LSSSub = 0;

  p->LSS.Master.State = LSS_MASTER_STATE_AWAITING_FASTSCAN_REPLY;
  return send_factscan_request(p);
}

/**
 *
 */
ErrorStatus
uco_lss_master_set_node_id(uCO_t *p, uCO_NodeId_t NodeId)
{
  uCO_CanMessage_t request = { 0 };
  request.CobId = UCANOPEN_COB_ID_LSS_REQUEST;
  request.length = UCANOPEN_LSS_LENGTH;

  request.data[0] = UCANOPEN_LSS_CS_CONFIGURE_NODE_ID;
  request.data[1] = (uint8_t) NodeId;

  p->LSS.Master.State = LSS_MASTER_STATE_AWAITING_CONFIGURE_NODE_ID_REPLY;
  set_lss_timeout(p, UCANOPEN_LSS_DEFAULT_TIMEOUT);

  return uco_send(p, &request);
}

/**
 *
 */
ErrorStatus
uco_lss_master_switch_mode_global(uCO_t *p, uCO_LSS_SlaveMode_t Mode)
{
  uCO_CanMessage_t request = { 0 };
  request.CobId = UCANOPEN_COB_ID_LSS_REQUEST;
  request.length = UCANOPEN_LSS_LENGTH;

  request.data[0] = UCANOPEN_LSS_CS_SWITCH_MODE_GLOBAL;
  request.data[1] = (uint8_t) Mode;

  p->LSS.Master.State = LSS_MASTER_STATE_IDLE;
  return uco_send(p, &request);
}

/* callback functions */

/**
 *
 */
__weak void
uco_lss_master_on_fastscan_error(uCO_t *p, uCO_LSS_FastscanError_t err)
{
  UNUSED(err);
}

/**
 *
 */
__weak void
uco_lss_master_on_fastscan_success(uCO_t *p)
{
  UNUSED(p);
}

/**
 *
 */
__weak void
uco_lss_master_on_fastscan_addr(uCO_t *p, uint8_t index)
{
  UNUSED(index);
}

/**
 *
 */
__weak void
uco_lss_master_on_fastscan_end(uCO_t *p)
{
}

/**
 *
 */
__weak void
uco_lss_master_on_slave_in_config_mode(uCO_t *p, uint32_t *ADDR)
{
  UNUSED(ADDR);
}

/**
 *
 */
__weak void
uco_lss_master_on_slave_configure_node_id(uCO_t *p, uCO_LSS_ConfigureNodeIdError_t err)
{
  /* FIXME: Emulator */ //UNUSED(err);
  if (err == LSS_CONFIGURE_NODE_ID_NO_ERROR)
  {
    /* Switch all non-configured nodes to WAITING mode */
    uco_lss_master_switch_mode_global(p, LSS_SLAVE_MODE_WAITING);

    /* Searching next non-configured node */
    uco_lss_master_start_fastscan(p);
  }
}

/**
 *
 */
__weak void
uco_lss_master_on_slave_configure_bit_timing(uCO_t *p, uCO_LSS_ConfigureBitTimingError_t err)
{
  UNUSED(err);
}

/**
 *
 */
__weak void
uco_lss_master_on_slave_store_configuration(uCO_t *p, uCO_LSS_StoreConfigurationError_t err)
{
  UNUSED(err);
}

/**
 *
 */
__weak void
uco_lss_master_on_slave_inquire_identity(uCO_t *p, uint32_t *ADDR, uint8_t index)
{
  UNUSED(ADDR);
  UNUSED(index);
}

#endif /* UCANOPEN_LSS_MASTER_ENABLED */
