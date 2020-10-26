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
	static uint16_t keyIndex = 0; //FIXME!

	if (p->PrevState == ButtonState_Pressed)
	{
		signal_blink(pUserLed, 100, 2,
						signal_get_output(pUserLed) ?
							SIGNAL_INVERTED : SIGNAL_NORMAL);

		/* switch to next key */
		keyIndex = (keyIndex + 1 < deviceKeysCount) ?
			keyIndex + 1 : 1;

		memcpy(&receivedKey[0], &deviceKeys[sizeof(AccessKey_t) * keyIndex], sizeof(AccessKey_t));

		uco_tpdo_transmit(&uCO, 1);
		uco_tpdo_transmit(&uCO, 2);
	}
}

void
button_pressed_long_callback(Button_t *p)
{
	signal_blink(pUserLed, 150, 3,
					signal_get_output(pUserLed) ?
						SIGNAL_INVERTED : SIGNAL_NORMAL);

	memcpy(&receivedKey[0], &deviceKeys[sizeof(AccessKey_t) * MASTER_KEY_INDEX], sizeof(AccessKey_t));

	uco_tpdo_transmit(&uCO, 1);
	uco_tpdo_transmit(&uCO, 2);
}
