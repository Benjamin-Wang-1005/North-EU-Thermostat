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

#include "flash_storage.h"
#include "Thermostat.h"
#include "Language.h"
#include <string.h>

// External global parameter structure
extern g_parameter_t g_parameter;

// Storage header structure (for validation)
typedef struct {
    uint32_t magic;         // Magic number for validation
    uint32_t version;       // Data structure version
    uint32_t size;          // Size of stored data
    uint32_t checksum;      // Simple checksum
} storage_header_t;

//---------------------------------------------------------------------------------------------------------
// Function: Calculate simple checksum
// Description: Calculate checksum of data buffer
//---------------------------------------------------------------------------------------------------------
static uint32_t Calculate_Checksum(uint8_t *data, uint32_t size)
{
    uint32_t checksum = 0;
		uint32_t i;
    for (i = 0; i < size; i++) {
        checksum += data[i];
    }
    return checksum;
}

//---------------------------------------------------------------------------------------------------------
// Function: Flash_Unlock
// Description: Unlock Flash for writing
//---------------------------------------------------------------------------------------------------------
static void Flash_Unlock(void)
{
    FLASH_Unlock();
}

//---------------------------------------------------------------------------------------------------------
// Function: Flash_Lock
// Description: Lock Flash after writing
//---------------------------------------------------------------------------------------------------------
static void Flash_Lock(void)
{
    FLASH_Lock();
}

//---------------------------------------------------------------------------------------------------------
// Function: Flash_Erase_Last_Page
// Description: Erase the last page of Flash (Page 127)
// Return: FLASH_OK on success, FLASH_ERROR_ERASE on failure
//---------------------------------------------------------------------------------------------------------
flash_status_t Flash_Erase_Last_Page(void)
{
    FLASH_Status status;
    
    // Erase the last page (Flash must be unlocked before calling this function)
    status = FLASH_ErasePage(FLASH_STORAGE_ADDRESS);
    
    if (status != FLASH_COMPLETE) {
        return FLASH_ERROR_ERASE;
    }
    
    return FLASH_OK;
}

//---------------------------------------------------------------------------------------------------------
// Function: Flash_Write_Word
// Description: Write a 32-bit word to Flash
// Parameters:
//      address - Flash address (must be aligned to 4 bytes)
//      data    - 32-bit data to write
// Return: FLASH_OK on success, FLASH_ERROR_WRITE on failure
//---------------------------------------------------------------------------------------------------------
static flash_status_t Flash_Write_Word(uint32_t address, uint32_t data)
{
    FLASH_Status status;
    
    status = FLASH_ProgramWord(address, data);
    
    if (status != FLASH_COMPLETE) {
        return FLASH_ERROR_WRITE;
    }
    
    // Verify written data
    if (*(volatile uint32_t *)address != data) {
        return FLASH_ERROR_VERIFY;
    }
    
    return FLASH_OK;
}

//---------------------------------------------------------------------------------------------------------
// Function: Flash_Save_Parameter
// Description: Save g_parameter structure to Flash
//      - Stores at the last page of Flash (0x0803F800)
//      - Includes header with magic number and checksum for validation
// Return: FLASH_OK on success, error code on failure
//---------------------------------------------------------------------------------------------------------
flash_status_t Flash_Save_Parameter(void)
{
    flash_status_t result;
    storage_header_t header;
    uint32_t write_address;
    uint32_t *data_ptr;
    uint32_t data_size;
    uint32_t word_count;
		uint32_t i;
    
    // Prepare header
    header.magic = FLASH_STORAGE_MAGIC;
    header.version = 2;
    header.size = sizeof(g_parameter_t);
    header.checksum = Calculate_Checksum((uint8_t *)&g_parameter, sizeof(g_parameter_t));
    
    // Unlock Flash
    Flash_Unlock();
    
    // Erase last page
    result = Flash_Erase_Last_Page();
    if (result != FLASH_OK) {
        Flash_Lock();
        return result;
    }
    
    // Write header
    write_address = FLASH_STORAGE_ADDRESS;
    data_ptr = (uint32_t *)&header;
    word_count = sizeof(storage_header_t) / 4;
    
    for (i = 0; i < word_count; i++) {
        result = Flash_Write_Word(write_address, *data_ptr);
        if (result != FLASH_OK) {
            Flash_Lock();
            return result;
        }
        write_address += 4;
        data_ptr++;
    }
    
    // Write g_parameter data
    data_ptr = (uint32_t *)&g_parameter;
    data_size = sizeof(g_parameter_t);
    word_count = (data_size + 3) / 4;  // Round up to nearest 4 bytes
    
    for (i = 0; i < word_count; i++) {
        result = Flash_Write_Word(write_address, *data_ptr);
        if (result != FLASH_OK) {
            Flash_Lock();
            return result;
        }
        write_address += 4;
        data_ptr++;
    }
    
    // Lock Flash
    Flash_Lock();
    
    return FLASH_OK;
}

//---------------------------------------------------------------------------------------------------------
// Function: Clamp_Parameter_Ranges
// Description: Sanity-clamp g_parameter fields loaded from Flash.
//      Flash data may pass checksum yet contain out-of-range values
//      (old firmware variants, partial writes). Index-type fields can
//      otherwise cause HardFaults (e.g. LanguageTable out-of-bounds).
//      Fallback values match golbal_par_init() defaults in Board_init.c.
//---------------------------------------------------------------------------------------------------------
static void Clamp_Parameter_Ranges(void)
{
    uint8_t i, j;
    // Default schedule entries - keep in sync with Board_init.c
    static const uint8_t default_workday[6][4] = {
        {5, 30, 20, 1}, {8, 30, 20, 1}, {10, 30, 20, 1},
        {12, 30, 20, 1}, {18, 30, 20, 1}, {21, 30, 20, 1}
    };
    static const uint8_t default_holiday[6][4] = {
        {7, 0, 20, 1}, {9, 0, 20, 1}, {12, 0, 20, 1},
        {14, 0, 20, 1}, {18, 0, 20, 1}, {22, 0, 20, 1}
    };

    //--- Group 1: index/enum fields (out-of-range can crash or draw garbage) ---
    if (g_parameter.language >= LANG_MAX)        g_parameter.language = 0;
    if (g_parameter.current_prog_type > 2)       g_parameter.current_prog_type = 0;
    if (g_parameter.operation_mode > 2)          g_parameter.operation_mode = 0;
    if (g_parameter.sensor_type > 1)             g_parameter.sensor_type = 0;
    if (g_parameter.power_on_state < 1 ||
        g_parameter.power_on_state > 3)          g_parameter.power_on_state = 2;
    if (g_parameter.floor_material > 2)          g_parameter.floor_material = 1;
    if (g_parameter.comfort_mode > 1)            g_parameter.comfort_mode = 0;
    if (g_parameter.font_size > 1)               g_parameter.font_size = 0;
    if (g_parameter.location >= ASTRO_LOCATION_MAX) g_parameter.location = ASTRO_LOCATION_OSLO;

    //--- Group 2: boolean switches (invalid -> safe state) ---
    if (g_parameter.child_lock > 1)              g_parameter.child_lock = 0;
    if (g_parameter.window_fun > 1)              g_parameter.window_fun = 0;
    if (g_parameter.adaptive_start > 1)          g_parameter.adaptive_start = 0;
    if (g_parameter.power_limit_switch > 1)      g_parameter.power_limit_switch = 0;
    if (g_parameter.temp_protect_max_switch > 1) g_parameter.temp_protect_max_switch = 0;
    if (g_parameter.temp_protect_min_switch > 1) g_parameter.temp_protect_min_switch = 1; // frost protection default ON

    //--- Group 3: ranged numeric fields (aligned with UI limits) ---
    if (g_parameter.window_fun_time < 5 || g_parameter.window_fun_time > 60)
        g_parameter.window_fun_time = 30;
    if (g_parameter.window_fun_temp < 3 || g_parameter.window_fun_temp > 20)
        g_parameter.window_fun_temp = 5;
    if (g_parameter.sleep_backlight_duty > 70)
        g_parameter.sleep_backlight_duty = 20;
    if (g_parameter.power_limit < 1 || g_parameter.power_limit > 30)
        g_parameter.power_limit = 6;
    if (g_parameter.pwm_day_duty < 10 || g_parameter.pwm_day_duty > 90)
        g_parameter.pwm_day_duty = 50;
    if (g_parameter.pwm_night_duty < 10 || g_parameter.pwm_night_duty > 90)
        g_parameter.pwm_night_duty = 50;
    if (g_parameter.temp_swing < 0.5f || g_parameter.temp_swing > 3.0f)
        g_parameter.temp_swing = 1.0f;
    if (g_parameter.temp_limit_max < 20 || g_parameter.temp_limit_max > DEVICE_OPERATION_TEMPERATURE_MAX)
        g_parameter.temp_limit_max = INPUT_TEMPERATURE_MAX_DEFAULT;
    if (g_parameter.temp_limit_min < (DEVICE_OPERATION_TEMPERATURE_MIN + 5) || g_parameter.temp_limit_min > 10)
        g_parameter.temp_limit_min = INPUT_TEMPERATURE_MIN_DEFAULT;
    if (g_parameter.temp_protect_max < MIN_TEMP_HIGH_TEMP_PROTECT || g_parameter.temp_protect_max > DEVICE_OPERATION_TEMPERATURE_MAX)
        g_parameter.temp_protect_max = DEVICE_PROTECT_TEMP_MAX_DEFAULT;
    if (g_parameter.temp_protect_min < DEVICE_OPERATION_TEMPERATURE_MIN || g_parameter.temp_protect_min > MAX_TEMP_LOW_TEMP_PROTECT)
        g_parameter.temp_protect_min = DEVICE_PROTECT_TEMP_MIN_DEFAULT;
    if (g_parameter.astro_day_hour > 23)   g_parameter.astro_day_hour = 5;
    if (g_parameter.astro_day_min > 59)    g_parameter.astro_day_min = 30;
    if (g_parameter.astro_night_hour > 23) g_parameter.astro_night_hour = 18;
    if (g_parameter.astro_night_min > 59)  g_parameter.astro_night_min = 0;
    if (g_parameter.astro_tolerance_min < -120 || g_parameter.astro_tolerance_min > 120)
        g_parameter.astro_tolerance_min = 0;
    if (g_parameter.r_mild <= 0.001f || g_parameter.r_mild > 1.0f)
        g_parameter.r_mild = AS_DEFAULT_R_MILD;
    if (g_parameter.r_cold <= 0.001f || g_parameter.r_cold > 1.0f)
        g_parameter.r_cold = AS_DEFAULT_R_COLD;

    //--- Group 4: schedule entries (restore whole entry if any field invalid) ---
    for (i = 0; i < 6; i++) {
        if (g_parameter.workday_schedule[i][0] > 23 || g_parameter.workday_schedule[i][1] > 59 ||
            g_parameter.workday_schedule[i][2] > 70 || g_parameter.workday_schedule[i][3] > 1) {
            for (j = 0; j < 4; j++) g_parameter.workday_schedule[i][j] = default_workday[i][j];
        }
        if (g_parameter.holiday_schedule[i][0] > 23 || g_parameter.holiday_schedule[i][1] > 59 ||
            g_parameter.holiday_schedule[i][2] > 70 || g_parameter.holiday_schedule[i][3] > 1) {
            for (j = 0; j < 4; j++) g_parameter.holiday_schedule[i][j] = default_holiday[i][j];
        }
    }

    //--- Group 5: dependent clamp (must run after temp limits are fixed) ---
    if (g_parameter.setting_number < g_parameter.temp_limit_min)
        g_parameter.setting_number = g_parameter.temp_limit_min;
    if (g_parameter.setting_number > g_parameter.temp_limit_max)
        g_parameter.setting_number = g_parameter.temp_limit_max;
}

//---------------------------------------------------------------------------------------------------------
// Function: Flash_Load_Parameter
// Description: Load g_parameter structure from Flash
//      - Reads from the last page of Flash (0x0803F800)
//      - Validates magic number and checksum before loading
// Return: FLASH_OK on success, error code on failure
// Note: If no valid data found, g_parameter remains unchanged
//---------------------------------------------------------------------------------------------------------
flash_status_t Flash_Load_Parameter(void)
{
    storage_header_t *header;
    uint32_t read_address;
    uint32_t *data_ptr;
    uint32_t data_size;
    uint32_t word_count;
    uint32_t checksum;
		uint32_t i;
		uint8_t is_old_version = 0;
    
    // Read header
    header = (storage_header_t *)FLASH_STORAGE_ADDRESS;
    
    // Validate magic number
    if (header->magic != FLASH_STORAGE_MAGIC) {
        return FLASH_ERROR_INVALID_DATA;
    }
    
    // Validate size - accept current or legacy version 1 size
    if (header->size == sizeof(g_parameter_t)) {
        is_old_version = 0;
    } else if (header->size == FLASH_PARAM_VERSION_1_SIZE) {
        is_old_version = 1;
    } else {
        return FLASH_ERROR_INVALID_DATA;
    }
    
    // Read g_parameter data
    read_address = FLASH_STORAGE_ADDRESS + sizeof(storage_header_t);
    data_ptr = (uint32_t *)&g_parameter;
    data_size = is_old_version ? FLASH_PARAM_VERSION_1_SIZE : sizeof(g_parameter_t);
    word_count = (data_size + 3) / 4;
    
    for (i = 0; i < word_count; i++) {
        *data_ptr = *(volatile uint32_t *)read_address;
        data_ptr++;
        read_address += 4;
    }
    
    // Verify checksum (using the stored data size)
    checksum = Calculate_Checksum((uint8_t *)&g_parameter, data_size);
    if (checksum != header->checksum) {
        return FLASH_ERROR_INVALID_DATA;
    }
    
    // If loading legacy data, initialize new fields to defaults
    if (is_old_version) {
        g_parameter.r_mild = AS_DEFAULT_R_MILD;
        g_parameter.r_cold = AS_DEFAULT_R_COLD;
    }

    // Clamp field ranges (defense against out-of-range Flash data)
    Clamp_Parameter_Ranges();
    
    return FLASH_OK;
}

//---------------------------------------------------------------------------------------------------------
// Function: Flash_Get_Parameter_Size
// Description: Get the size of g_parameter_t structure
// Return: Size in bytes
//---------------------------------------------------------------------------------------------------------
uint32_t Flash_Get_Parameter_Size(void)
{
    return sizeof(g_parameter_t);
}
