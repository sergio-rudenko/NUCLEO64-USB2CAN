/*
 * device_OD.c
 *
 *  Created on: Oct 15, 2020
 *      Author: sergi
 */

#include "ucanopen.h"
#include "device.h"

/**-------------------------------------------------
 * Manufacturer OD item (2000h): Keys Count & Array
 * -------------------------------------------------
 */
#define uCO_ODI_2000_01 deviceKeysCount
#define uCO_ODI_2000_02 deviceKeys

const uint8_t uCO_ODI_2000_00 = 2; /* Number of Entries */
const uOD_Item_t uCO_ODI_2000[] =
{
	/* sub | type | access | address */
	{ 0x00, UNSIGNED8, READ_ONLY, (void*) &uCO_ODI_2000_00 },
	{ 0x01, UNSIGNED16, READ_ONLY, (void*) &uCO_ODI_2000_01 },
	{ 0x02, OCTET_STRING, READ_WRITE, (void*) uCO_ODI_2000_02, sizeof(uCO_ODI_2000_02) },
};

/**-----------------------------------------
 * Manufacturer OD item (2001h): Master Key
 * -----------------------------------------
 */
#define uCO_ODI_2001_00 deviceMasterKey

/**-----------------------------------------
 * Manufacturer OD item (2002h): Append Key
 * -----------------------------------------
 */
#define uCO_ODI_2002_00 deviceKeyToAppend

/**-----------------------------------------
 * Manufacturer OD item (2003h): Delete Key
 * -----------------------------------------
 */
#define uCO_ODI_2003_00 deviceKeyToDelete

/**-------------------------------------------------
 * Manufacturer OD item (2000h): Keys Count & Array
 * -------------------------------------------------
 */
#define uCO_ODI_3000_01 (deviceSchedules[sizeof(AccessSchedule_t) * 1])
#define uCO_ODI_3000_02 (deviceSchedules[sizeof(AccessSchedule_t) * 2])
#define uCO_ODI_3000_03 (deviceSchedules[sizeof(AccessSchedule_t) * 3])
#define uCO_ODI_3000_04 (deviceSchedules[sizeof(AccessSchedule_t) * 4])
#define uCO_ODI_3000_05 (deviceSchedules[sizeof(AccessSchedule_t) * 5])
#define uCO_ODI_3000_06 (deviceSchedules[sizeof(AccessSchedule_t) * 6])
#define uCO_ODI_3000_07 (deviceSchedules[sizeof(AccessSchedule_t) * 7])

const uint8_t uCO_ODI_3000_00 = 7; /* Number of Entries */
const uOD_Item_t uCO_ODI_3000[] =
{
	/* sub | type | access | address */
	{ 0x00, UNSIGNED8, READ_ONLY, (void*) &uCO_ODI_3000_00 },
	{ 0x01, OCTET_STRING, READ_WRITE, (void*) &uCO_ODI_3000_01, sizeof(AccessSchedule_t) },
	{ 0x02, OCTET_STRING, READ_WRITE, (void*) &uCO_ODI_3000_02, sizeof(AccessSchedule_t) },
	{ 0x03, OCTET_STRING, READ_WRITE, (void*) &uCO_ODI_3000_03, sizeof(AccessSchedule_t) },
	{ 0x04, OCTET_STRING, READ_WRITE, (void*) &uCO_ODI_3000_04, sizeof(AccessSchedule_t) },
	{ 0x05, OCTET_STRING, READ_WRITE, (void*) &uCO_ODI_3000_05, sizeof(AccessSchedule_t) },
	{ 0x06, OCTET_STRING, READ_WRITE, (void*) &uCO_ODI_3000_06, sizeof(AccessSchedule_t) },
	{ 0x07, OCTET_STRING, READ_WRITE, (void*) &uCO_ODI_3000_07, sizeof(AccessSchedule_t) },
};

/**-----------------------------------------
 * Manufacturer OD item (2003h): Erase Keys
 * -----------------------------------------
 */
uint16_t uCO_ODI_2004_00;

/**
 * Manufacturer Object Dictionary
 */
const uOD_Item_t Manufacturer_OD[] =
{
	/* index | type | access | address */
	{ 0x2000, SUBARRAY, READ_ONLY, (void*) &uCO_ODI_2000 },
	{ 0x2001, OCTET_STRING, READ_WRITE, (void*) uCO_ODI_2001_00, sizeof(AccessKey_t) },
	{ 0x2002, OCTET_STRING, WRITE_ONLY, (void*) uCO_ODI_2002_00, sizeof(AccessKey_t) },
	{ 0x2003, OCTET_STRING, WRITE_ONLY, (void*) uCO_ODI_2003_00, sizeof(AccessKey_t) },
	{ 0x2004, UNSIGNED16, WRITE_ONLY, (void*) &uCO_ODI_2004_00 },
	{ 0x3000, SUBARRAY, READ_ONLY, (void*) &uCO_ODI_3000 },

	{ 0xFFFF, UNDEFINED, READ_ONLY, NULL } // End of dictionary, mandatory
};

uOD_Item_t*
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
				uOD_Item_t *item = Manufacturer_OD[i].address;
				uint8_t sub_count = *((uint8_t*) item[0].address);
				for (int j = 0; j < sub_count + 1; j++)
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
					&((uOD_Item_t*) Manufacturer_OD)[i] : NULL;
			}
		}
	}
	return NULL;
}

ErrorStatus
uco_tpdo_prepare_data(uCO_t *p, int num)
{
	ErrorStatus result = ERROR;

	static uint8_t tpdoData1[8];
	static uint8_t tpdoData2[8];
	static uint8_t tpdoData3[4];

	uint32_t unixtime = now();

	if (num == 1)
	{
		tpdoData1[0] = (unixtime) & 0xFF;
		tpdoData1[1] = (unixtime >> 8) & 0xFF;
		tpdoData1[2] = (unixtime >> 16) & 0xFF;
		tpdoData1[3] = (unixtime >> 24) & 0xFF;

		memcpy(&tpdoData1[4], &receivedKey[0], 4);

		p->TPDO[0].data.address = tpdoData1;
		p->TPDO[0].data.size = sizeof(tpdoData1);

		result = SUCCESS;
	}

	if (num == 2)
	{
		memcpy(tpdoData2, &receivedKey[4], 8);

		p->TPDO[1].data.address = tpdoData2;
		p->TPDO[1].data.size = sizeof(tpdoData2);

		result = SUCCESS;
	}

	if (num == 3)
	{
		tpdoData3[0] = (unixtime) & 0xFF;
		tpdoData3[1] = (unixtime >> 8) & 0xFF;
		tpdoData3[2] = (unixtime >> 16) & 0xFF;
		tpdoData3[3] = (unixtime >> 24) & 0xFF;

		p->TPDO[2].data.address = tpdoData3;
		p->TPDO[2].data.size = sizeof(tpdoData3);

		result = SUCCESS;
	}

	return result;
}

/**
 *
 */
void
uco_nmt_on_uplink_status(uCO_t *p, bool alive)
{
	deviceUplinkStatus = alive;
}

/**
 *
 */
size_t
uco_sdo_get_octet_string_size(uCO_t *p, uint16_t index, uint8_t sub)
{
	size_t size = 0;
	uOD_Item_t *item = uco_find_od_item(p, index, sub);

	if (item && item->address == deviceKeys)
	{
		size = sizeof(AccessKey_t) * deviceKeysCount;
	}
	return size;
}

/**
 *
 */
ErrorStatus
uco_sdo_prepare_data(uCO_t *p, uint16_t index, uint8_t sub)
{
	/* Master Key */
	if (index == 0x2001 && sub == 00)
	{
		memcpy(deviceMasterKey, deviceKeys, sizeof(AccessKey_t));
	}

	return SUCCESS;
}

ErrorStatus
uco_sdo_validate_data(uCO_t *p, uint16_t index, uint8_t sub)
{
	/* Master Key */
	if (index == 0x2001 && sub == 00)
	{
		AccessKey_t *NewMasterKey = (AccessKey_t*) &deviceMasterKey;

		if (NewMasterKey->header.type != MASTER_KEY ||
			NewMasterKey->header.schedule != SCHEDULE_ALWAYS_GRANT_ACCESS)
		{
			return ERROR;
		}
		else
		{
			/* copy Master Key to keys */
			memcpy(deviceKeys, deviceMasterKey, sizeof(AccessKey_t));
		}
	}

	/* Append Key */
	if (index == 0x2002 && sub == 00)
	{
		AccessKey_t *KeyToAppend = (AccessKey_t*) &deviceKeyToAppend;

		if (deviceKeysCount >= 1000 ||
			KeyToAppend->header.bits == 0 ||
			KeyToAppend->header.bits > 64 ||
			KeyToAppend->header.type == MASTER_KEY)
		{
			return ERROR;
		}
		else
		{
			/* copy Appending Key to keys */
			uint16_t offset = sizeof(AccessKey_t) * deviceKeysCount;
			memcpy(&deviceKeys[offset], deviceKeyToAppend, sizeof(AccessKey_t));

			deviceKeysCount += 1;
			device_sort_keys(deviceKeys, deviceKeysCount);
		}
	}

	/* Delete Key */
	if (index == 0x2003 && sub == 00)
	{
		AccessKey_t *KeyToDelete = (AccessKey_t*) &deviceKeyToDelete;

		if (deviceKeysCount <= 1 ||
			KeyToDelete->header.bits == 0 ||
			KeyToDelete->header.bits > 64 ||
			KeyToDelete->header.type == MASTER_KEY)
		{
			return ERROR;
		}
		else
		{
			/* remove Deleting KeyToDelete from keys */
			uint16_t index = device_find_key(KeyToDelete->header.bits, KeyToDelete->body);

			if (index != MASTER_KEY_INDEX && index != KEY_NOT_FOUND)
			{
				for (int i = index; i < deviceKeysCount; i++)
				{
					uint16_t offset_dst = sizeof(AccessKey_t) * i;
					uint16_t offset_src = sizeof(AccessKey_t) * (i + 1);

					memcpy(&deviceKeys[offset_dst], &deviceKeys[offset_src], sizeof(AccessKey_t));
				}
				deviceKeysCount -= 1;
				memset(&deviceKeys[sizeof(AccessKey_t) * deviceKeysCount], 0x00, sizeof(AccessKey_t));
			}
		}
	}
	return SUCCESS;
}
