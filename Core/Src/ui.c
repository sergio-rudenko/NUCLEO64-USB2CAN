/*
 * ui.c
 *
 *  Created on: Oct 23, 2020
 *      Author: sergi
 */

#include "ui.h"

#include "lawicel.h"
#include "ucanopen.h"

static Button_t UserButton;
Button_t *pUserButton = &UserButton;

static Signal_t UserLed;
Signal_t *pUserLed = &UserLed;

void
LAWICEL_CAN_on_stop_callback(LAWICEL_Instance_t *p)
{
	signal_level(pUserLed, RESET);
}

void
LAWICEL_CAN_on_start_callback(LAWICEL_Instance_t *p)
{
	signal_level(pUserLed, SET);
}

void
button_released_callback(Button_t *p)
{

	if (p->PrevState == ButtonState_Pressed)
	{
		signal_blink(pUserLed, 150, 1,
						signal_get_output(pUserLed) ?
							SIGNAL_INVERTED : SIGNAL_NORMAL);

		/* FIXME: Emulator */

#ifdef UCANOPEN_LSS_MASTER_ENABLED

		/* Switch all non-configured nodes to WAITING mode */
		uco_lss_master_switch_mode_global(&uCO, LSS_SLAVE_MODE_WAITING);

		/* Searching next non-configured node */
		uco_lss_master_start_fastscan(&uCO);

#endif /* UCANOPEN_LSS_MASTER_ENABLED */
	}
}

void
button_pressed_long_callback(Button_t *p)
{
	signal_blink(pUserLed, 150, 2,
					signal_get_output(pUserLed) ?
						SIGNAL_INVERTED : SIGNAL_NORMAL);

	/* FIXME: Emulator */

#ifdef UCANOPEN_LSS_MASTER_ENABLED

	/* Reset ALL Nodes */
	uco_nmt_master_send_command(&uCO, UCANOPEN_NMT_CS_RESET_COMMUNICATION, UCANOPEN_NMT_BROADCAST_MESSAGE);

#endif /* UCANOPEN_LSS_MASTER_ENABLED */
}
