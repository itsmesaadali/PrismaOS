#include "../include/time.h"
#include "../include/string.h"
#include "../include/io.h"
#include "stdint.h"

#define RTCaddress 0x70
#define RTCdata 0x71

// CMOS Register identifiers
#define CMOS_RTC_SECONDS      0x00
#define CMOS_RTC_MINUTES      0x02
#define CMOS_RTC_HOURS        0x04
#define CMOS_RTC_DAY_MONTH    0x07
#define CMOS_RTC_MONTH        0x08
#define CMOS_RTC_YEAR         0x09
#define CMOS_STATUS_B         0x0B

// Timezone offset in hours (Pakistan: UTC+5)
#define TIMEZONE_OFFSET_HOURS 5

uint8_t get_update_in_progress_flag()
{
    output_bytes(RTCaddress, CMOS_STATUS_B);
    return (input_bytes(RTCdata) & 0x80);
}

uint32_t get_RTC_register(uint8_t reg)
{
    output_bytes(RTCaddress, reg);
    return input_bytes(RTCdata);
}

static inline void convert_bcd_values(uint32_t *seconds, uint32_t *minutes, uint32_t *hours,
                                      uint32_t *day, uint32_t *month, uint32_t *year, uint32_t registerB)
{
    if (!(registerB & 0x04))  // Check if values are in BCD mode
    {
        if (seconds) *seconds = (*seconds & 0x0F) + ((*seconds / 16) * 10);
        if (minutes) *minutes = (*minutes & 0x0F) + ((*minutes / 16) * 10);
        if (hours)   *hours   = ((*hours & 0x0F) + (((*hours & 0x70) / 16) * 10)) | (*hours & 0x80);
        if (day)     *day     = (*day & 0x0F) + ((*day / 16) * 10);
        if (month)   *month   = (*month & 0x0F) + ((*month / 16) * 10);
        if (year)    *year    = (*year & 0x0F) + ((*year / 16) * 10);
    }
}

static inline uint16_t convert_to_full_year(uint32_t year)
{
    return (uint16_t)(year + 2000); // Assumes years 2000–2099
}

static inline int is_leap_year(uint16_t year)
{
    return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
}

static inline uint8_t days_in_month(uint32_t month, uint16_t year)
{
    static const uint8_t mdays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (month == 2) return mdays[1] + (is_leap_year(year) ? 1 : 0);
    if (month >= 1 && month <= 12) return mdays[month - 1];
    return 31;
}

// Apply an hour offset to a datetime, adjusting day/month/year on overflow/underflow
static void apply_timezone_offset(uint32_t *hours, uint32_t *day, uint32_t *month, uint32_t *year, int offset_hours)
{
    if (!hours) return;

    int h = (int)(*hours);
    int d = day ? (int)(*day) : 0;
    int m = month ? (int)(*month) : 1;
    int y = year ? (int)(*year) : 1970;

    h += offset_hours;

    // adjust days when hours overflow/underflow
    while (h < 0) { h += 24; d -= 1; }
    while (h >= 24) { h -= 24; d += 1; }

    // propagate day changes into month/year
    if (day && month && year)
    {
        while (d < 1)
        {
            m -= 1;
            if (m < 1) { m = 12; y -= 1; }
            d += days_in_month(m, (uint16_t)y);
        }
        while (d > days_in_month(m, (uint16_t)y))
        {
            d -= days_in_month(m, (uint16_t)y);
            m += 1;
            if (m > 12) { m = 1; y += 1; }
        }

        *day = (uint32_t)d;
        *month = (uint32_t)m;
        *year = (uint32_t)y;
    }

    *hours = (uint32_t)h;
}

void datetime()
{
    uint32_t seconds, minutes, hours, day, month, year, registerB;
    char current_datetime[22];

    while (get_update_in_progress_flag());

    seconds = get_RTC_register(CMOS_RTC_SECONDS);
    minutes = get_RTC_register(CMOS_RTC_MINUTES);
    hours   = get_RTC_register(CMOS_RTC_HOURS);
    day     = get_RTC_register(CMOS_RTC_DAY_MONTH);
    month   = get_RTC_register(CMOS_RTC_MONTH);
    year    = get_RTC_register(CMOS_RTC_YEAR);
    registerB = get_RTC_register(CMOS_STATUS_B);

    convert_bcd_values(&seconds, &minutes, &hours, &day, &month, &year, registerB);
    year = convert_to_full_year(year);
    // convert RTC (assumed UTC) to Pakistan time (UTC+5)
    apply_timezone_offset(&hours, &day, &month, &year, TIMEZONE_OFFSET_HOURS);

    sprintf(current_datetime, "%02u:%02u:%02u - %02u/%02u/%u", hours, minutes, seconds, day, month, year);
    printk("%s\n", current_datetime);
}

void date()
{
    uint32_t day, month, year, hours, registerB;
    char current_date[12];

    while (get_update_in_progress_flag());

    day   = get_RTC_register(CMOS_RTC_DAY_MONTH);
    month = get_RTC_register(CMOS_RTC_MONTH);
    year  = get_RTC_register(CMOS_RTC_YEAR);
    hours = get_RTC_register(CMOS_RTC_HOURS);
    registerB = get_RTC_register(CMOS_STATUS_B);
    convert_bcd_values(NULL, NULL, &hours, &day, &month, &year, registerB);
    year = convert_to_full_year(year);

    // adjust date for timezone
    apply_timezone_offset(&hours, &day, &month, &year, TIMEZONE_OFFSET_HOURS);

    sprintf(current_date, "%02u/%02u/%u", day, month, year);
    printk("%s\n", current_date);
}

void clock()
{
    uint32_t seconds, minutes, hours, day, month, year, registerB;
    char current_time[10];

    while (get_update_in_progress_flag());

    seconds = get_RTC_register(CMOS_RTC_SECONDS);
    minutes = get_RTC_register(CMOS_RTC_MINUTES);
    hours   = get_RTC_register(CMOS_RTC_HOURS);
    day     = get_RTC_register(CMOS_RTC_DAY_MONTH);
    month   = get_RTC_register(CMOS_RTC_MONTH);
    year    = get_RTC_register(CMOS_RTC_YEAR);
    registerB = get_RTC_register(CMOS_STATUS_B);
    convert_bcd_values(&seconds, &minutes, &hours, &day, &month, &year, registerB);
    year = convert_to_full_year(year);

    apply_timezone_offset(&hours, &day, &month, &year, TIMEZONE_OFFSET_HOURS);

    sprintf(current_time, "%02u:%02u:%02u", hours, minutes, seconds);
    printk("%s\n", current_time);
}

uint32_t current_seconds()
{
    while (get_update_in_progress_flag());
    uint32_t seconds = get_RTC_register(CMOS_RTC_SECONDS);
    uint32_t registerB = get_RTC_register(CMOS_STATUS_B);
    convert_bcd_values(&seconds, NULL, NULL, NULL, NULL, NULL, registerB);
    return seconds;
}

uint32_t current_minutes()
{
    while (get_update_in_progress_flag());
    uint32_t minutes = get_RTC_register(CMOS_RTC_MINUTES);
    uint32_t registerB = get_RTC_register(CMOS_STATUS_B);
    convert_bcd_values(NULL, &minutes, NULL, NULL, NULL, NULL, registerB);
    return minutes;
}

uint32_t current_hour()
{
    while (get_update_in_progress_flag());
    uint32_t hours = get_RTC_register(CMOS_RTC_HOURS);
    uint32_t day = get_RTC_register(CMOS_RTC_DAY_MONTH);
    uint32_t month = get_RTC_register(CMOS_RTC_MONTH);
    uint32_t year = get_RTC_register(CMOS_RTC_YEAR);
    uint32_t registerB = get_RTC_register(CMOS_STATUS_B);
    convert_bcd_values(NULL, NULL, &hours, &day, &month, &year, registerB);
    year = convert_to_full_year(year);
    apply_timezone_offset(&hours, &day, &month, &year, TIMEZONE_OFFSET_HOURS);
    return hours;
}

uint32_t current_day()
{
    while (get_update_in_progress_flag());
    uint32_t day = get_RTC_register(CMOS_RTC_DAY_MONTH);
    uint32_t hours = get_RTC_register(CMOS_RTC_HOURS);
    uint32_t month = get_RTC_register(CMOS_RTC_MONTH);
    uint32_t year = get_RTC_register(CMOS_RTC_YEAR);
    uint32_t registerB = get_RTC_register(CMOS_STATUS_B);
    convert_bcd_values(NULL, NULL, &hours, &day, &month, &year, registerB);
    year = convert_to_full_year(year);
    apply_timezone_offset(&hours, &day, &month, &year, TIMEZONE_OFFSET_HOURS);
    return day;
}

uint32_t current_month()
{
    while (get_update_in_progress_flag());
    uint32_t month = get_RTC_register(CMOS_RTC_MONTH);
    uint32_t hours = get_RTC_register(CMOS_RTC_HOURS);
    uint32_t day = get_RTC_register(CMOS_RTC_DAY_MONTH);
    uint32_t year = get_RTC_register(CMOS_RTC_YEAR);
    uint32_t registerB = get_RTC_register(CMOS_STATUS_B);
    convert_bcd_values(NULL, NULL, &hours, &day, &month, &year, registerB);
    year = convert_to_full_year(year);
    apply_timezone_offset(&hours, &day, &month, &year, TIMEZONE_OFFSET_HOURS);
    return month;
}

uint32_t current_year()
{
    while (get_update_in_progress_flag());
    uint32_t year = get_RTC_register(CMOS_RTC_YEAR);
    uint32_t hours = get_RTC_register(CMOS_RTC_HOURS);
    uint32_t day = get_RTC_register(CMOS_RTC_DAY_MONTH);
    uint32_t month = get_RTC_register(CMOS_RTC_MONTH);
    uint32_t registerB = get_RTC_register(CMOS_STATUS_B);
    convert_bcd_values(NULL, NULL, &hours, &day, &month, &year, registerB);
    year = convert_to_full_year(year);
    apply_timezone_offset(&hours, &day, &month, &year, TIMEZONE_OFFSET_HOURS);
    return year;
}
