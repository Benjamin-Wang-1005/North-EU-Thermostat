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
#include "delay.h"
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
            delay_ms(10);
            if (retry > 250) // Timeout after ~2.5 seconds
            {
                // LSE failed, try LSI as fallback
                RCC_LSEConfig(RCC_LSE_OFF);
                RCC_LSICmd(ENABLE);
                retry = 0;
                while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
                {
                    retry++;
                    delay_ms(10);
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
        
        // Mark RTC as configured
        BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
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
// Function: RTC_ReadTime
// Description: Read current time from RTC counter and convert to rtc_time_t structure
//      - Reads 32-bit RTC counter
//      - Converts to year/month/day/hour/min/sec
// Return: RTC_OK on success
//---------------------------------------------------------------------------------------------------------
uint8_t RTC_ReadTime(rtc_time_t *time)
{
    uint32_t counter;
    uint32_t days;
    uint32_t seconds;
    uint16_t year = 2000;
    uint8_t month;
    const uint8_t days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    if (time == NULL)
    {
        return RTC_ERROR;
    }
    
    // Read RTC counter (seconds since epoch or last set)
    counter = RTC_GetCounter();
    
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
    
    return RTC_OK;
}

//---------------------------------------------------------------------------------------------------------
// Function: RTC_SetTime
// Description: Set RTC time from rtc_time_t structure
//      - Converts rtc_time_t to 32-bit counter value
//      - Writes to RTC counter
// Return: RTC_OK on success
//---------------------------------------------------------------------------------------------------------
uint8_t RTC_SetTime(rtc_time_t *time)
{
    uint32_t counter = 0;
    uint16_t year;
    uint16_t y;
    uint8_t month;
    const uint8_t days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
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
    
    // Enable write access to RTC
    PWR_BackupAccessCmd(ENABLE);
    
    // Set RTC counter
    RTC_SetCounter(counter);
    RTC_WaitForLastTask();
    
    return RTC_OK;
}
