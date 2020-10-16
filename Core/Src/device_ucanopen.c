/*
 * device_OD.c
 *
 *  Created on: Oct 15, 2020
 *      Author: sergi
 */

#include "ucanopen.h"

/**-------------------------------------------------------------
 * Manufacturer OD item (2000h): Number of Keys
 * -------------------------------------------------------------
 */
extern uint16_t deviceTest16bitRO;

/**-------------------------------------------------------------
 * Manufacturer OD item (2001h): Number of Keys
 * -------------------------------------------------------------
 */
extern uint8_t deviceTestArray12bytes[12];

/**-------------------------------------------------------------
 * Manufacturer OD item (2002h): Number of Keys
 * -------------------------------------------------------------
 */
extern uint8_t deviceTestArray256bytes[256];

/**
 * Manufacturer Object Dictionary
 */
const uCO_OD_Item_t Manufacturer_OD[] =
{
	/* index | type | access | address */
	{ 0x2000, UNSIGNED16, READ_ONLY, (void*) &deviceTest16bitRO },
	{ 0x2001, OCTET_STRING, READ_WRITE, (void*) &deviceTestArray12bytes, sizeof(deviceTestArray12bytes) },
	{ 0x2002, OCTET_STRING, READ_WRITE, (void*) &deviceTestArray256bytes, sizeof(deviceTestArray256bytes) },

	{ 0xFFFF, UNDEFINED, READ_ONLY, NULL } // End of dictionary, mandatory
};

uCO_OD_Item_t*
uco_find_od_manufacturer_item(uCO_t *p, uint16_t id, uint8_t sub)
{
	/** --------------
	 *  Search in OD
	 *  --------------
	 */
	for (int i = 0; Manufacturer_OD[i].index != 0xFFFF; i++)
	{
		if (Manufacturer_OD[i].index == id)
		{
			if (Manufacturer_OD[i].Type == SUBARRAY)
			{
				uCO_OD_Item_t *item = Manufacturer_OD[i].address;
				uint8_t sub_count = *((uint8_t*) item[0].address);
				for (int j = 1; j < sub_count; j++)
				{
					if (item[j].index == sub)
					{
						return &item[j];
					}
				}
			}
			else
			{
				return (sub == 0) ?
					&((uCO_OD_Item_t*) Manufacturer_OD)[i] : NULL;
			}
		}
	}
	return NULL;
}

