/*
 * unixtime.h
 *
 *  Created on: Aug 7, 2020
 *      Author: sa100
 */

#ifndef LIB_BASTION_UNIXTIME_UNIXTIME_H_
#define LIB_BASTION_UNIXTIME_UNIXTIME_H_

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#include "rtc.h"

#define SECONDS_PER_MINUTE   60UL
#define SECONDS_PER_HOUR     3600UL
#define SECONDS_PER_DAY      86400UL

#define DEVICE_LIFE_START_TS 1596289020UL

#define bcd2bin(v)  __LL_RTC_CONVERT_BCD2BIN(v)
#define bin2bcd(v)  __LL_RTC_CONVERT_BIN2BCD(v)

bool
is_leap_year(const uint32_t year);

uint8_t
get_week_day(const time_t ts);

uint32_t
days_in_month(const uint32_t month, const uint32_t year);

uint32_t
days_to_month(const uint32_t month, const uint32_t year);

uint32_t
days_to_year(const uint32_t year);

time_t
now(void);

time_t
get_date(void);

time_t
get_time_of_day(void);

void
set_time(const time_t ts);

#endif /* LIB_BASTION_UNIXTIME_UNIXTIME_H_ */
