/*
 * ui.c
 *
 *  Created on: Oct 26, 2020
 *      Author: sergi
 */

#include "ucanopen.h"
#include "lawicel.h"
#include "ui.h"

static Button_t UserButton;
Button_t *pUserButton = &UserButton;

static Signal_t UserLed;
Signal_t *pUserLed = &UserLed;

void
button_released_callback(Button_t *p)
{

	if (p->PrevState == ButtonState_Pressed)
	{
		/* FIXME: Emulator */

#ifdef UCANOPEN_LSS_MASTER_ENABLED

		signal_blink(pUserLed, 150, 1,
						signal_get_output(pUserLed) ?
							SIGNAL_INVERTED : SIGNAL_NORMAL);

		/* Switch all non-configured nodes to WAITING mode */
		uco_lss_master_switch_mode_global(&uCO, LSS_SLAVE_MODE_WAITING);

		/* Searching next non-configured node */
		uco_lss_master_start_fastscan(&uCO);

#else

		static uint16_t keyIndex = 0;

		signal_blink(pUserLed, 150, 2,
						signal_get_output(pUserLed) ?
							SIGNAL_INVERTED : SIGNAL_NORMAL);

		/* switch to next key */
		keyIndex = (keyIndex + 1 < deviceKeysCount) ?
			keyIndex + 1 : 1;

		memcpy(&receivedKey[0], &deviceKeys[sizeof(AccessKey_t) * keyIndex], sizeof(AccessKey_t));

		uco_tpdo_transmit(&uCO, 1);
		uco_tpdo_transmit(&uCO, 2);

#endif /* UCANOPEN_LSS_MASTER_ENABLED */
	}
}

void
button_pressed_long_callback(Button_t *p)
{
	/* FIXME: Emulator */

#ifdef UCANOPEN_LSS_MASTER_ENABLED

	signal_blink(pUserLed, 150, 2,
					signal_get_output(pUserLed) ?
						SIGNAL_INVERTED : SIGNAL_NORMAL);

	/* Reset ALL Nodes */
	uco_nmt_master_send_command(&uCO, UCANOPEN_NMT_CS_RESET_COMMUNICATION, UCANOPEN_NMT_BROADCAST_MESSAGE);

#else

	signal_blink(pUserLed, 150, 3,
					signal_get_output(pUserLed) ?
						SIGNAL_INVERTED : SIGNAL_NORMAL);

	memcpy(&receivedKey[0], &deviceKeys[sizeof(AccessKey_t) * MASTER_KEY_INDEX], sizeof(AccessKey_t));

	uco_tpdo_transmit(&uCO, 1);
	uco_tpdo_transmit(&uCO, 2);

#endif /* UCANOPEN_LSS_MASTER_ENABLED */
}
