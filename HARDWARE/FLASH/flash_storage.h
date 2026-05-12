//---------------------------------------------------------------------------------------------------------
//                                                                                                         
// Copyright(c) 2026 E-poly Technology Co., Ltd. All rights reserved.                                            
//                                                                                                         
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
// E-Poly North EU Thermostat Project 
// Author: Benjamin Wang
// Date: 2026/04/23
// Email: Benjamin@epoly-tech.com
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
// File Function: Flash storage for g_parameter structure
//      - Store g_parameter_t at the last page of 256KB Flash
//      - STM32F103RCT6: 256KB Flash = 128 pages (2KB per page)
//      - Last page address: 0x0803F800 (Page 127)
//---------------------------------------------------------------------------------------------------------

#ifndef __FLASH_STORAGE_H
#define __FLASH_STORAGE_H

#include <stdint.h>
#include "stm32f10x.h"

// Flash configuration for STM32F103RCT6 (256KB)
#define FLASH_SIZE_KB           256
#define FLASH_PAGE_SIZE         2048    // 2KB per page
#define FLASH_PAGE_COUNT        128     // 256KB / 2KB = 128 pages
#define FLASH_LAST_PAGE_NUM     127     // Last page number (0-based)
#define FLASH_STORAGE_ADDRESS   0x0803F800  // Base address of last page

// Magic number for data validation
#define FLASH_STORAGE_MAGIC     0xA5A5A5A5

// Error codes
typedef enum {
    FLASH_OK = 0,
    FLASH_ERROR_WRITE,
    FLASH_ERROR_ERASE,
    FLASH_ERROR_VERIFY,
    FLASH_ERROR_INVALID_DATA
} flash_status_t;

// Function prototypes
flash_status_t Flash_Load_Parameter(void);
flash_status_t Flash_Save_Parameter(void);
flash_status_t Flash_Erase_Last_Page(void);
uint32_t Flash_Get_Parameter_Size(void);

#endif /* __FLASH_STORAGE_H */
