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
// File Function: RTC Driver Header
//      - RTC initialization and time management
//      - BKP register access for time storage
//---------------------------------------------------------------------------------------------------------

#ifndef _RTC_DRIVER_H
#define _RTC_DRIVER_H

#include "stm32f10x.h"
#include <stdint.h>

// RTC Time structure (defined here to avoid circular dependency)
// Note: STM32F103 RTC hardware is a 32-bit seconds counter only.
// Year range: 1970-2099 (full year value, not offset)
typedef struct
{
    uint16_t    Year;    // Full year: 1970-2099
    uint8_t     Mon;     // 1-12
    uint8_t     Date;    // 1-31
    uint8_t     Hour;    // 0-23
    uint8_t     Min;     // 0-59
    uint8_t     Sec;     // 0-59
} rtc_time_t;

// RTC Status definitions
#define RTC_OK      0
#define RTC_ERROR   1

// BKP Register definitions for RTC backup
#define BKP_DR1     ((uint16_t)0x0004)
#define BKP_DR2     ((uint16_t)0x0008)
#define BKP_DR3     ((uint16_t)0x000C)
#define BKP_DR4     ((uint16_t)0x0010)
#define BKP_DR5     ((uint16_t)0x0014)

// Function prototypes
uint8_t RTC_Init(void);
uint8_t RTC_ReadTime(rtc_time_t *time);
uint8_t RTC_SetTime(rtc_time_t *time);
void RTC_Start(void);
uint8_t RTC_IsConfigured(void);
void RTC_GetDefaultTime(rtc_time_t *time);

// Helper functions
uint8_t RTC_BCDToByte(uint8_t bcd);
uint8_t RTC_ByteToBCD(uint8_t byte);

#endif // _RTC_DRIVER_H
