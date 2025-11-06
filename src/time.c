#include "../include/time.h"
#include "../include/string.h"
#include "../include/io.h"
#include "stdint.h"

#define RTCaddress 0x70
#define RTCdata 0x71

// CMOS Registers
#define CMOS_RTC_SECONDS 0x00
#define CMOS_RTC_MINUTES 0x02
#define CMOS_RTC_HOURS   0x04
#define CMOS_RTC_DAY_MONTH 0x07
#define CMOS_RTC_MONTH 0x08
#define CMOS_RTC_YEAR 0x09
#define CMOS_STATUS_A 0x0A
#define CMOS_STATUS_B 0x0B

// TIMEZONE OFFSET (Pakistan = UTC + 5)
#define TIMEZONE_OFFSET 5

uint8_t get_update_in_progress_flag()
{
    output_bytes(RTCaddress, CMOS_STATUS_A);
    return (input_bytes(RTCdata) & 0x80);
}

uint32_t get_RTC_register(uint8_t reg)
{
    output_bytes(RTCaddress, reg);
    return input_bytes(RTCdata);
}

static inline void convert_bcd(uint32_t *value)
{
    *value = (*value & 0x0F) + ((*value / 16) * 10);
}

void datetime()
{
    uint32_t seconds, minutes, hours, day, month, year, regB;
    char buffer[22];

    while (get_update_in_progress_flag()) {}

    seconds = get_RTC_register(CMOS_RTC_SECONDS);
    minutes = get_RTC_register(CMOS_RTC_MINUTES);
    hours   = get_RTC_register(CMOS_RTC_HOURS);
    day     = get_RTC_register(CMOS_RTC_DAY_MONTH);
    month   = get_RTC_register(CMOS_RTC_MONTH);
    year    = get_RTC_register(CMOS_RTC_YEAR);
    regB    = get_RTC_register(CMOS_STATUS_B);

    // Convert BCD if needed
    if (!(regB & 0x04))
    {
        convert_bcd(&seconds);
        convert_bcd(&minutes);
        convert_bcd(&hours);
        convert_bcd(&day);
        convert_bcd(&month);
        convert_bcd(&year);
    }

    // Convert 12-hour -> 24-hour mode
    if (!(regB & 0x02) && (hours & 0x80))
        hours = ((hours & 0x7F) + 12) % 24;

    // Apply Pakistan timezone offset
    hours = (hours + TIMEZONE_OFFSET) % 24;

    // Convert 2-digit year to full year
    year += 2000;

    sprintf(buffer, "Current datetime: %02u:%02u:%02u - %02u/%02u/%u", hours, minutes, seconds, day, month, year);
    printk("%s\n", buffer);
}

void date()
{
    uint32_t day, month, year, regB;
    char buffer[12];

    while (get_update_in_progress_flag()) {}

    day = get_RTC_register(CMOS_RTC_DAY_MONTH);
    month = get_RTC_register(CMOS_RTC_MONTH);
    year = get_RTC_register(CMOS_RTC_YEAR);
    regB = get_RTC_register(CMOS_STATUS_B);

    if (!(regB & 0x04))
    {
        convert_bcd(&day);
        convert_bcd(&month);
        convert_bcd(&year);
    }

    year += 2000;

    sprintf(buffer, "Current date: %02u/%02u/%u", day, month, year);
    printk("%s\n", buffer);
}

void clock()
{
    uint32_t seconds, minutes, hours, regB;
    char buffer[10];

    while (get_update_in_progress_flag()) {}

    seconds = get_RTC_register(CMOS_RTC_SECONDS);
    minutes = get_RTC_register(CMOS_RTC_MINUTES);
    hours   = get_RTC_register(CMOS_RTC_HOURS);
    regB    = get_RTC_register(CMOS_STATUS_B);

    if (!(regB & 0x04))
    {
        convert_bcd(&seconds);
        convert_bcd(&minutes);
        convert_bcd(&hours);
    }

    if (!(regB & 0x02) && (hours & 0x80))
        hours = ((hours & 0x7F) + 12) % 24;

    hours = (hours + TIMEZONE_OFFSET) % 24;

    sprintf(buffer, "Current clock: %02u:%02u:%02u", hours, minutes, seconds);
    printk("%s\n", buffer);
}
