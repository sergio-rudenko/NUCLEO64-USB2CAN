/*
 * device.h
 *
 *  Created on: Oct 19, 2020
 *      Author: sergi
 */

#ifndef INC_DEVICE_H_
#define INC_DEVICE_H_

#include "button.h"
#include "signal.h"

#include "unixtime.h"

#define DEVICE_KEYS_NUMBER		1000
#define DEVICE_SCHEDULES_NUMBER	8

typedef enum AccessKeyType
{
	MASTER_KEY = 0,
	ACCESS_KEY = 0x01,
	SECURITY_KEY = 0x02,
	SECURITY_ACCESS_KEY = 0x03,
} AccessKeyType_t;

#pragma pack(push,1)
typedef union AccessKey
{
	struct
	{
		struct
		{
			uint8_t type;
			uint8_t bits;
			uint8_t schedule;
			uint8_t reserved;
		} header;
		uint8_t body[8];
	};
	uint8_t data[12];
} AccessKey_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef union AccessScheduleDay
{
	struct
	{
		uint8_t hours_00_07;
		uint8_t hours_08_15;
		uint8_t hours_16_23;
		uint8_t reserved;
	};
	uint8_t data[4];
} AccessScheduleDay_t;

typedef struct AccessSchedule
{
	AccessScheduleDay_t mon;
	AccessScheduleDay_t tue;
	AccessScheduleDay_t wen;
	AccessScheduleDay_t thu;
	AccessScheduleDay_t fri;
	AccessScheduleDay_t sat;
	AccessScheduleDay_t sun;
} AccessSchedule_t;
#pragma pack(pop)

extern char deviceSoftwareVersion[32];

extern bool deviceUplinkStatus;

#define MASTER_KEY_INDEX				0
#define KEY_NOT_FOUND					0xFFFE
extern uint16_t deviceKeysCount;
extern uint8_t deviceKeys[DEVICE_KEYS_NUMBER * sizeof(AccessKey_t)];
extern uint8_t deviceMasterKey[sizeof(AccessKey_t)];
extern uint8_t deviceKeyToAppend[sizeof(AccessKey_t)];
extern uint8_t deviceKeyToDelete[sizeof(AccessKey_t)];

extern uint8_t receivedKey[sizeof(AccessKey_t)];

#define SCHEDULE_ALWAYS_GRANT_ACCESS 	0
#define SCHEDULE_RESTRICTED_ACCESS 		1
extern uint8_t deviceSchedules[DEVICE_SCHEDULES_NUMBER * sizeof(AccessSchedule_t)];

extern Signal_t *pUserLed;
extern Button_t *pUserButton;

/* prototypes */

void
device_load_scedules(uint8_t *scedules, uint16_t max);

void
device_load_keys(uint8_t *keys, uint16_t max);

void
device_sort_keys(uint8_t *keys, uint16_t max);

uint16_t
device_find_key(uint8_t bits, uint8_t *body);

#endif /* INC_DEVICE_H_ */
