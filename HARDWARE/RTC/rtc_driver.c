//---------------------------------------------------------------------------------------------------------
//
// Copyright(c) 2026 E-poly Technology Co., Ltd. All rights reserved.
//
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
// E-Poly North EU Thermostat Project
// Author: Benjamin Wang
// Date: 2026/04/20
// Email: Benjamin@epoly-tech.com
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
// File Function: RTC Driver Implementation
//      - RTC peripheral initialization
//      - Time read/write functions
//      - BKP register access for persistent storage
//---------------------------------------------------------------------------------------------------------

#include "rtc_driver.h"
#include "Peripheral.h"
#include <stddef.h>

// Private variables
// static uint32_t rtc_counter = 0;  // Reserved for future use

//---------------------------------------------------------------------------------------------------------
// Function: RTC_BCDToByte
// Description: Convert BCD format to byte
//---------------------------------------------------------------------------------------------------------
uint8_t RTC_BCDToByte(uint8_t bcd)
{
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

//---------------------------------------------------------------------------------------------------------
// Function: RTC_ByteToBCD
// Description: Convert byte to BCD format
//---------------------------------------------------------------------------------------------------------
uint8_t RTC_ByteToBCD(uint8_t byte)
{
    return ((byte / 10) << 4) | (byte % 10);
}

//---------------------------------------------------------------------------------------------------------
// Function: RTC_IsConfigured
// Description: Check if RTC has been configured before (using BKP_DR1 magic number)
// Return: 1 = configured, 0 = not configured
//---------------------------------------------------------------------------------------------------------
uint8_t RTC_IsConfigured(void)
{
    // Check if backup register has magic number indicating RTC was configured
    if (BKP_ReadBackupRegister(BKP_DR1) == 0xA5A5)
    {
        return 1;
    }
    return 0;
}

//---------------------------------------------------------------------------------------------------------
// Function: RTC_Init
// Description: Initialize RTC peripheral
//      - Enable PWR and BKP clocks
//      - Enable access to BKP registers
//      - Configure LSE as RTC clock source
//      - Set RTC prescaler for 1Hz tick
// Return: RTC_OK on success, RTC_ERROR on failure
//---------------------------------------------------------------------------------------------------------
uint8_t RTC_Init(void)
{
    uint16_t retry = 0;

    // Enable PWR and BKP clocks
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

    // Enable access to BKP registers
    PWR_BackupAccessCmd(ENABLE);

    // Check if RTC is already configured
    if (!RTC_IsConfigured())
    {
        // Reset Backup Domain
        BKP_DeInit();

        // Enable LSE (Low Speed External crystal)
        RCC_LSEConfig(RCC_LSE_ON);

        // Wait for LSE to stabilize
        retry = 0;
        while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
        {
            retry++;
            my_delay_ms(10);
            if (retry > 250) // Timeout after ~2.5 seconds
            {
                // LSE failed, try LSI as fallback
                RCC_LSEConfig(RCC_LSE_OFF);
                RCC_LSICmd(ENABLE);
                retry = 0;
                while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
                {
                    retry++;
                    my_delay_ms(10);
                    if (retry > 100)
                    {
                        return RTC_ERROR; // Both LSE and LSI failed
                    }
                }
                RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
            }
        }

        // Select LSE as RTC clock source
        if (RCC_GetFlagStatus(RCC_FLAG_LSERDY) != RESET)
        {
            RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
        }

        // Enable RTC clock
        RCC_RTCCLKCmd(ENABLE);

        // Wait for RTC registers synchronization
        RTC_WaitForSynchro();

        // Wait until last write operation on RTC registers has finished
        RTC_WaitForLastTask();

        // Set RTC prescaler: 32768Hz / 32767 + 1 = 1Hz
        // For LSE (32.768kHz)
        RTC_SetPrescaler(32767);

        // Wait until last write operation on RTC registers has finished
        RTC_WaitForLastTask();

        // NOTE: do NOT write the configured magic here.
        // 0xA5A5 is written by RTC_SetTime() only, so RTC_IsConfigured()
        // means "time has been explicitly set", not "RTC HW initialized".
    }
    else
    {
        // RTC already configured, just enable clock
        RCC_RTCCLKCmd(ENABLE);

        // Wait for RTC registers synchronization
        RTC_WaitForSynchro();

        // Wait until last write operation on RTC registers has finished
        RTC_WaitForLastTask();
    }

    // Enable RTC Second interrupt (optional - for timekeeping)
    RTC_ITConfig(RTC_IT_SEC, ENABLE);
    RTC_WaitForLastTask();

    return RTC_OK;
}

//---------------------------------------------------------------------------------------------------------
// Function: RTC_Start
// Description: Start RTC counting
//---------------------------------------------------------------------------------------------------------
void RTC_Start(void)
{
    // RTC is already running after Init, but ensure it's enabled
    // The counter starts automatically when RTC is enabled
}

//---------------------------------------------------------------------------------------------------------
// Function: RTC_GetDefaultTime
// Description: Get default time (2026/01/01 00:00:00)
//---------------------------------------------------------------------------------------------------------
void RTC_GetDefaultTime(rtc_time_t *time)
{
    if (time != NULL)
    {
        time->Year = 26;   // 2026
        time->Mon = 1;     // January
        time->Date = 1;    // 1st
        time->Hour = 0;    // 00:00:00
        time->Min = 0;
        time->Sec = 0;
    }
}

//---------------------------------------------------------------------------------------------------------
// Daylight Saving Time (DST) support - Norway / EU rule
//      - DST starts: last Sunday of March,   02:00 standard time -> 03:00
//      - DST ends:   last Sunday of October, 03:00 daylight time -> 02:00 standard time
//        (i.e. 02:00 standard time, since 03:00 CEST == 02:00 CET)
//      - Design: the RTC hardware counter ALWAYS stores STANDARD time (CET, UTC+1)
//        and is never adjusted by the DST switch itself - it just keeps counting.
//        RTC_ReadTime() / RTC_SetTime() are the only two places that convert between
//        the stored standard time and the "effective" wall-clock local time that the
//        user actually sees on screen / types in on the Set Time page.
//      - Known limitation: the ~1 hour ambiguous/non-existent wall-clock window on
//        the two switch days themselves is not perfectly resolvable (same limitation
//        every timezone-aware system has); in practice nobody sets the clock during
//        that specific hour, twice a year, so this is left unhandled by design.
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
// Function: DST_Last_Sunday_Of_Month
// Description: Return the day-of-month (1-31) of the last Sunday of the given month/year
// Input: year (offset from 2000, same convention as rtc_time_t.Year), month (1-12)
// Output: day-of-month of the last Sunday in that month
//---------------------------------------------------------------------------------------------------------
static uint8_t DST_Last_Sunday_Of_Month(uint16_t year, uint8_t month)
{
    const uint8_t days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    uint16_t full_year = 2000 + year;
    uint8_t dim = days_in_month[month - 1];
    uint8_t wd;        // 0=Monday ... 6=Sunday (see Peripheral.c getWeekday())
    uint8_t days_back; // days back from the last day of the month to the previous Sunday

    if (month == 2 && ((full_year % 4 == 0 && full_year % 100 != 0) || (full_year % 400 == 0)))
    {
        dim = 29;
    }

    wd = getWeekday(year, month, dim);
    days_back = (wd + 1) % 7;

    return dim - days_back;
}

//---------------------------------------------------------------------------------------------------------
// Function: DST_Is_Active
// Description: Decide whether DST (summer time) is active for a given STANDARD-time
//      reading. Called symmetrically from RTC_ReadTime() (on the stored standard time)
//      and RTC_SetTime() (on the user-entered wall-clock time, as an approximation -
//      see the ambiguous-hour note above).
// Input: year (offset from 2000), month (1-12), day (1-31), hour (0-23)
// Output: 1 = DST active (summer time), 0 = standard time (winter time)
//---------------------------------------------------------------------------------------------------------
static uint8_t DST_Is_Active(uint16_t year, uint8_t month, uint8_t day, uint8_t hour)
{
    uint8_t dst_start_day, dst_end_day;

    if (month < 3 || month > 10)
    {
        return 0;
    }
    if (month > 3 && month < 10)
    {
        return 1;
    }

    if (month == 3)
    {
        dst_start_day = DST_Last_Sunday_Of_Month(year, 3);
        if (day < dst_start_day) return 0;
        if (day > dst_start_day) return 1;
        return (hour >= 2) ? 1 : 0;   // switches at 02:00 standard time
    }

    // month == 10
    dst_end_day = DST_Last_Sunday_Of_Month(year, 10);
    if (day < dst_end_day) return 1;
    if (day > dst_end_day) return 0;
    return (hour < 2) ? 1 : 0;       // switches at 02:00 standard time (=03:00 daylight)
}

//---------------------------------------------------------------------------------------------------------
// Function: RTC_Counter_To_Calendar
// Description: Pure conversion of a raw counter value (seconds since 2000-01-01 00:00:00)
//      into calendar fields. No DST logic here - same math the old RTC_ReadTime() had.
//---------------------------------------------------------------------------------------------------------
static void RTC_Counter_To_Calendar(uint32_t counter, rtc_time_t *time)
{
    uint32_t days;
    uint32_t seconds;
    uint16_t year;
    uint8_t month;
    const uint8_t days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    // Convert to days and seconds
    days = counter / 86400;      // Days since reference
    seconds = counter % 86400;   // Remaining seconds in current day

    // Calculate time of day
    time->Hour = seconds / 3600;
    seconds %= 3600;
    time->Min = seconds / 60;
    time->Sec = seconds % 60;

    // Calculate date (starting from 2000-01-01)
    year = 2000;
    while (days >= 365)
    {
        // Check for leap year
        if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
        {
            if (days >= 366)
            {
                days -= 366;
                year++;
            }
            else
            {
                break;
            }
        }
        else
        {
            days -= 365;
            year++;
        }
    }

    time->Year = (uint8_t)(year - 2000); // Store as offset from 2000

    // Calculate month and day
    for (month = 0; month < 12; month++)
    {
        uint8_t dim = days_in_month[month];

        // February in leap year
        if (month == 1 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)))
        {
            dim = 29;
        }

        if (days < dim)
        {
            break;
        }
        days -= dim;
    }

    time->Mon = month + 1;  // 1-12
    time->Date = days + 1;  // 1-31
}

//---------------------------------------------------------------------------------------------------------
// Function: RTC_ReadTime
// Description: Read current time from RTC counter and convert to rtc_time_t structure
//      - Reads 32-bit RTC counter (always stored as STANDARD time, see DST notes above)
//      - Converts to year/month/day/hour/min/sec
//      - Applies the DST offset (if currently in the summer-time period) so the value
//        returned is the effective wall-clock local time
// Return: RTC_OK on success
//---------------------------------------------------------------------------------------------------------
uint8_t RTC_ReadTime(rtc_time_t *time)
{
    uint32_t counter;
    rtc_time_t std_time;

    if (time == NULL)
    {
        return RTC_ERROR;
    }

    // Read RTC counter (seconds since epoch or last set) - standard time, never adjusted
    counter = RTC_GetCounter();
    RTC_Counter_To_Calendar(counter, &std_time);

    // Apply DST offset (if any) to get the effective time to display/use
    if (DST_Is_Active(std_time.Year, std_time.Mon, std_time.Date, std_time.Hour))
    {
        RTC_Counter_To_Calendar(counter + 3600, time);
    }
    else
    {
        *time = std_time;
    }

    return RTC_OK;
}

//---------------------------------------------------------------------------------------------------------
// Function: RTC_Is_DST_Active_Today
// Description: Public wrapper exposing the DST decision for "right now", so other
//      features (e.g. the PWM Astro Time sunrise/sunset calculation in Peripheral.c,
//      which needs to know each selectable city's CURRENT UTC offset) can reuse the
//      same EU DST rule without duplicating the last-Sunday-of-month arithmetic.
// Return: 1 = DST (summer time) active today, 0 = standard time
//---------------------------------------------------------------------------------------------------------
uint8_t RTC_Is_DST_Active_Today(void)
{
    uint32_t counter = RTC_GetCounter();
    rtc_time_t std_time;

    RTC_Counter_To_Calendar(counter, &std_time);
    return DST_Is_Active(std_time.Year, std_time.Mon, std_time.Date, std_time.Hour);
}

//---------------------------------------------------------------------------------------------------------
// Function: RTC_Calendar_To_Counter
// Description: Pure conversion of calendar fields into a raw counter value (seconds
//      since 2000-01-01 00:00:00). No DST logic here - same math the old RTC_SetTime() had.
//---------------------------------------------------------------------------------------------------------
static uint32_t RTC_Calendar_To_Counter(const rtc_time_t *time)
{
    uint32_t counter = 0;
    uint16_t year;
    uint16_t y;
    uint8_t month;
    const uint8_t days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    // Calculate days from 2000 to target year
    year = 2000 + time->Year;
    for (y = 2000; y < year; y++)
    {
        if ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0))
        {
            counter += 366;
        }
        else
        {
            counter += 365;
        }
    }

    // Add days for months passed in current year
    for (month = 0; month < (time->Mon - 1); month++)
    {
        uint8_t dim = days_in_month[month];
        if (month == 1 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)))
        {
            dim = 29;
        }
        counter += dim;
    }

    // Add days in current month
    counter += (time->Date - 1);

    // Convert days to seconds and add time of day
    counter = counter * 86400;
    counter += time->Hour * 3600;
    counter += time->Min * 60;
    counter += time->Sec;

    return counter;
}

//---------------------------------------------------------------------------------------------------------
// Function: RTC_SetTime
// Description: Set RTC time from rtc_time_t structure
//      - 'time' is the effective wall-clock value the user just entered (already
//        includes DST if the current date/time falls in the summer-time period)
//      - Converts it to the STANDARD-time counter value that the hardware always stores
//      - Writes to RTC counter
// Return: RTC_OK on success
//---------------------------------------------------------------------------------------------------------
uint8_t RTC_SetTime(rtc_time_t *time)
{
    uint32_t counter;

    if (time == NULL)
    {
        return RTC_ERROR;
    }

    // Validate input
    if (time->Year > 99 || time->Mon < 1 || time->Mon > 12 ||
        time->Date < 1 || time->Date > 31 ||
        time->Hour > 23 || time->Min > 59 || time->Sec > 59)
    {
        return RTC_ERROR;
    }

    counter = RTC_Calendar_To_Counter(time);

    // 'time' is the wall-clock value the user sees (may already be DST-shifted);
    // convert it back to the standard-time value the counter stores.
    if (DST_Is_Active(time->Year, time->Mon, time->Date, time->Hour))
    {
        counter -= 3600;
    }

    // Enable write access to RTC
    PWR_BackupAccessCmd(ENABLE);

    // Set RTC counter
    RTC_SetCounter(counter);
    RTC_WaitForLastTask();

    // Time explicitly set: mark RTC as configured
    BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);

    return RTC_OK;
}
