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
    header.version = 1;
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
    
    // Read header
    header = (storage_header_t *)FLASH_STORAGE_ADDRESS;
    
    // Validate magic number
    if (header->magic != FLASH_STORAGE_MAGIC) {
        return FLASH_ERROR_INVALID_DATA;
    }
    
    // Validate size
    if (header->size != sizeof(g_parameter_t)) {
        return FLASH_ERROR_INVALID_DATA;
    }
    
    // Read g_parameter data
    read_address = FLASH_STORAGE_ADDRESS + sizeof(storage_header_t);
    data_ptr = (uint32_t *)&g_parameter;
    data_size = sizeof(g_parameter_t);
    word_count = (data_size + 3) / 4;  // Round up to nearest 4 bytes
    
    for (i = 0; i < word_count; i++) {
        *data_ptr = *(volatile uint32_t *)read_address;
        data_ptr++;
        read_address += 4;
    }
    
    // Verify checksum
    checksum = Calculate_Checksum((uint8_t *)&g_parameter, sizeof(g_parameter_t));
    if (checksum != header->checksum) {
        return FLASH_ERROR_INVALID_DATA;
    }
    
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
