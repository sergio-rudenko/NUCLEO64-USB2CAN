/*
 * unixtime.c
 *
 *  Created on: Aug 7, 2020
 *      Author: sa100
 *
 *  Based on: http://prolog-plc.ru/docs/TimeExperience.pdf
 */

#include "unixtime.h"

/**
 * @brief  Функция вычисляющая, является ли год (в формате BDC) високосным
 * @param  year   : BDC, 0x00 -> 1970 год
 * @return : true - високосный год, false - нет
 */
bool is_leap_year(const uint32_t year)
{
    uint16_t bin_year = bcd2bin(year) + 1970;
    return ((bin_year % 400 == 0) ||
            ((bin_year % 4 == 0) && (bin_year % 100 != 0)));
}

/**
 * @brief  Функция, рассчитывающая количество дней в месяце
 * @param  Month   : [LL_RTC_MONTH_JANUARY .. LL_RTC_MONTH_DECEMBER]
 * @param  year : BDC, 0x00 -> 1970 год
 * @return : количество дней в месяце
 */
uint32_t days_in_month(const uint32_t month, const uint32_t year)
{
    const uint8_t dim[12] = {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    uint32_t m, result = 0;

    m = bcd2bin(month);

    if (m >= LL_RTC_MONTH_JANUARY && m <= LL_RTC_MONTH_DECEMBER) {
        result += dim[m - 1];
        result += (m == LL_RTC_MONTH_FEBRUARY && is_leap_year(year));
    }
    return result;
}

/**
 * @brief  Функция, рассчитывающая количество дней с Нового Года до начала месяца
 * @param  Month   : [LL_RTC_MONTH_JANUARY .. LL_RTC_MONTH_DECEMBER]
 * @param  year : BDC, 0x00 -> 1970 год
 * @return : количество дней с Нового Года до начала месяца
 */
uint32_t days_to_month(const uint32_t month, const uint32_t year)
{
    const uint16_t dtm[12] = {
        0, 31, 59, 90, 120, 151,
        181, 212, 243, 273, 304, 334
    };
    
    uint32_t m, result = 0;

    m = bcd2bin(month);
    
    if (m >= LL_RTC_MONTH_JANUARY && m <= LL_RTC_MONTH_DECEMBER) {
        result += dtm[m - 1];
        result += (m > LL_RTC_MONTH_FEBRUARY && is_leap_year(year));
    }
    return result;
}

/**
 * @brief  Функция, рассчитывающая количество дней до начала года
 * @param  year : BDC (0x00 -> 1970 год)
 * @return : количество дней до начала года
 */
uint32_t days_to_year(const uint32_t year)
{
    uint32_t y, result = 0;

    y = bcd2bin(year);

    if (year < 0x99) {
        result = y * 365 + (y + 1) / 4 + (y / 69) / 100 + (y / 369) / 400;
    }
    return result;
}

/**
 * @brief  Функция вычисления дня недели из time_t
 * @param  t : количество секунд с 1970-01-01 (LL_RTC_WEEKDAY_THURSDAY)
 * @return : [LL_RTC_WEEKDAY_MONDAY .. LL_RTC_WEEKDAY_SUNDAY]
 */
uint8_t get_week_day(const time_t ts)
{
    uint32_t n, result = 0;

    n = ts / SECONDS_PER_DAY;

    result = ((n + LL_RTC_WEEKDAY_THURSDAY) % 7);
    return (result > 0) ? result : LL_RTC_WEEKDAY_SUNDAY;
}

/**
 * @brief  Функция возвращает количество секунд, прошедших с 00:00:00 текущей даты RTC
 * @return :  количество секунд, прошедших с полуночи
 */
time_t get_time_of_day(void) {
    time_t tod = 0UL;

    tod += bcd2bin(LL_RTC_TIME_GetHour(RTC)) * SECONDS_PER_HOUR;
    tod += bcd2bin(LL_RTC_TIME_GetMinute(RTC)) * SECONDS_PER_MINUTE;
    tod += bcd2bin(LL_RTC_TIME_GetSecond(RTC));

    return tod;
}

/**
 @brief  Функция возвращает количество секунд до текущей даты RTC
 @return : количество секунд с 1970-01-01 00:00:00 до 00:00:00 даты RTC
 */
time_t get_date(void)
{
    time_t ts = 0UL;

    ts += days_to_year(LL_RTC_DATE_GetYear(RTC));
    ts += days_to_month(LL_RTC_DATE_GetMonth(RTC), LL_RTC_DATE_GetYear(RTC));
    ts += bcd2bin(LL_RTC_DATE_GetDay(RTC)) - 1;
    ts *= SECONDS_PER_DAY;

    return ts;
}

/**
 @brief  Функция возвращает количество секунд на основании RTC
 @return : количество секунд с 1970-01-01 00:00:00 (time_t)
 */
time_t now(void)
{
    time_t ts = 0UL;

    ts += get_date();
    ts += get_time_of_day();

    return ts;
}

/**
 * @brief  Функция, устанавливающая время на основании time_t
 * @param  ts : количество секунд с 1970-01-01 00:00:00
 * @return none
 */
void set_time(const time_t ts)
{
    uint32_t dty, dtm, days, value;

    LL_RTC_DateTypeDef RTC_DateStruct;
    LL_RTC_TimeTypeDef RTC_TimeStruct;
    
    /* --------- Вычисление даты ---------------------------------------- */
    days = ts / SECONDS_PER_DAY;

    /* грубо определяем год */
    value = days / 365;
    dty = days_to_year(bin2bcd(value));

    /* уточняем год */
    if (days < dty) {
        value -= 1;
        dty = days_to_year(bin2bcd(value));
    }
    RTC_DateStruct.Year = value;

    /* определяем день года */
    days = days - dty + 1;

    /* грубо определяем месяц */
    value = days / 29 + 1;
    value = (value < 12) ? value : 12;
    dtm = days_to_month(bin2bcd(value), bin2bcd(RTC_DateStruct.Year));

    /* уточняем месяц */
    if (days <= dtm) {
        value -= 1;
        dtm = days_to_month(bin2bcd(value), bin2bcd(RTC_DateStruct.Year));
    }
    RTC_DateStruct.Month = value;

    /* вычисляем день */
    value = days - dtm;
    RTC_DateStruct.Day = value;

    RTC_DateStruct.WeekDay = get_week_day(ts);
    LL_RTC_DATE_Init(RTC, LL_RTC_FORMAT_BIN, &RTC_DateStruct);

    /* --------- Вычисление времени ------------------------------------- */
    value = ts - (dty + dtm + value - 1) * SECONDS_PER_DAY;
    RTC_TimeStruct.Hours = value / SECONDS_PER_HOUR;

    value %= SECONDS_PER_HOUR;
    RTC_TimeStruct.Minutes = value / SECONDS_PER_MINUTE;

    value %= SECONDS_PER_MINUTE;
    RTC_TimeStruct.Seconds = value;

    RTC_TimeStruct.TimeFormat = LL_RTC_TIME_FORMAT_AM_OR_24;
    LL_RTC_TIME_Init(RTC, LL_RTC_FORMAT_BIN, &RTC_TimeStruct);
}

