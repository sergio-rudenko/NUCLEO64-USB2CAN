/*
 * device.c
 *
 *  Created on: Oct 19, 2020
 *      Author: sergi
 */

#include <string.h>
#include "device.h"

uint8_t deviceSchedules[DEVICE_SCHEDULES_NUMBER * sizeof(AccessSchedule_t)];
uint8_t deviceKeys[DEVICE_KEYS_NUMBER * sizeof(AccessKey_t)];
uint8_t deviceMasterKey[sizeof(AccessKey_t)];
uint8_t deviceKeyToAppend[sizeof(AccessKey_t)];
uint8_t deviceKeyToDelete[sizeof(AccessKey_t)];
uint16_t deviceKeysCount;

/* Received from scanner */
uint8_t receivedKey[sizeof(AccessKey_t)];

char deviceSoftwareVersion[32] = "v0.06_DEV";

bool deviceUplinkStatus;

void
uint32_to_array(uint8_t *data, const size_t size, const uint32_t source)
{
	if (size <= sizeof(uint32_t))
	{
		for (int i = 0; i < size; i++)
			data[i] = (source >> (8 * i)) & 0xFF;
	}
}

void
uint64_to_array(uint8_t *data, const size_t size, const uint64_t source)
{
	if (size <= sizeof(uint64_t))
	{
		for (int i = 0; i < size; i++)
			data[i] = (source >> (8 * i)) & 0xFF;
	}
}

/**
 *
 */
void
device_load_keys(uint8_t *ptr, uint16_t max)
{
	AccessKey_t *Key = (AccessKey_t*) ptr;
	deviceKeysCount = 5;

	Key[MASTER_KEY_INDEX].header.type = MASTER_KEY;
	Key[MASTER_KEY_INDEX].header.bits = 64;
	Key[MASTER_KEY_INDEX].header.schedule = SCHEDULE_ALWAYS_GRANT_ACCESS;
	uint64_to_array(Key[MASTER_KEY_INDEX].body, sizeof(uint64_t), 0x0123456789ABCDEFUL);

	for (int i = 1; i < deviceKeysCount; i++)
	{
		Key[i].header.type = ACCESS_KEY;
		Key[i].header.bits = 64;
		Key[i].header.schedule = SCHEDULE_RESTRICTED_ACCESS;
		uint64_to_array(Key[i].body, sizeof(uint64_t), 0x1111111111111111UL * i);
	}
}

/**
 *
 */
void
device_sort_keys(uint8_t *keys, uint16_t max)
{
	//TODO!
}

/**
 *
 */
uint16_t
device_find_key(uint8_t bits, uint8_t *body)
{
	uint16_t result = KEY_NOT_FOUND;

	if (body && bits > 0 && bits <= 64)
	{
		AccessKey_t *Key = (AccessKey_t*) &deviceKeys;

		for (int i = 0; i < deviceKeysCount; i++)
		{
			if (Key->header.bits == bits &&
				memcmp(Key[i].body, body, 8) == 0 /* FIXME! if sorted */)
			{
				return i;
			}
		}
	}
	return result;
}

/**
 *
 */
void
device_load_scedules(uint8_t *ptr, uint16_t max)
{
	AccessSchedule_t *Scedule = (AccessSchedule_t*) ptr;

	/* Scedule 0: No access restrictions */
	uint32_to_array(Scedule[0].mon.data, sizeof(uint32_t), 0x00FFFFFF);
	uint32_to_array(Scedule[0].tue.data, sizeof(uint32_t), 0x00FFFFFF);
	uint32_to_array(Scedule[0].wen.data, sizeof(uint32_t), 0x00FFFFFF);
	uint32_to_array(Scedule[0].thu.data, sizeof(uint32_t), 0x00FFFFFF);
	uint32_to_array(Scedule[0].fri.data, sizeof(uint32_t), 0x00FFFFFF);
	uint32_to_array(Scedule[0].sat.data, sizeof(uint32_t), 0x00FFFFFF);
	uint32_to_array(Scedule[0].sun.data, sizeof(uint32_t), 0x00FFFFFF);

	/* Scedule 0: Work days, 08:00 - 18:00 */
	uint32_to_array(Scedule[1].mon.data, sizeof(uint32_t), 0x0001FF00);
	uint32_to_array(Scedule[1].tue.data, sizeof(uint32_t), 0x0001FF00);
	uint32_to_array(Scedule[1].wen.data, sizeof(uint32_t), 0x0001FF00);
	uint32_to_array(Scedule[1].thu.data, sizeof(uint32_t), 0x0001FF00);
	uint32_to_array(Scedule[1].fri.data, sizeof(uint32_t), 0x0001FF00);
	uint32_to_array(Scedule[1].sat.data, sizeof(uint32_t), 0x00000000);
	uint32_to_array(Scedule[1].sun.data, sizeof(uint32_t), 0x00000000);
}

