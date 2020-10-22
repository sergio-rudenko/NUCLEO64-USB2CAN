/*
 * uco_time.c
 *
 *  Created on: Oct 16, 2020
 *      Author: sergi
 */

#include "uco_time.h"
#include "unixtime.h"

uCO_ErrorStatus_t
uco_proceed_time_message(uCO_t *p, uint8_t *data)
{
	time_t ts;

	ts = data[0];
	ts += data[1] << 8;
	ts += data[2] << 16;
	ts += data[3] << 24;

	set_time(ts);

	return UCANOPEN_SUCCESS;
}
