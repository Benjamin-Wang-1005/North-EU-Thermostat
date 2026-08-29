#include "iap_flash.h"

void FLASH_If_Init(void)
{
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    FLASH_Lock();
}

uint32_t FLASH_If_Erase(uint32_t start)
{
    FLASH_Status status;
    uint32_t page_addr;
    uint32_t erase_end = FLASH_STORAGE_ADDRESS;

    if (start < FLASH_BASE || start >= erase_end) {
        return FLASHIF_ERASEKO;
    }

    FLASH_Unlock();

    for (page_addr = start; page_addr < erase_end; page_addr += FLASH_PAGE_SIZE) {
        status = FLASH_ErasePage(page_addr);
        if (status != FLASH_COMPLETE) {
            FLASH_Lock();
            return FLASHIF_ERASEKO;
        }
    }

    FLASH_Lock();
    return FLASHIF_OK;
}

uint32_t FLASH_If_Write(uint32_t destination, uint32_t *p_source, uint32_t length)
{
    FLASH_Status status;
    uint32_t i;

    if ((destination % 4) != 0) {
        return FLASHIF_WRITING_ERROR;
    }

    if (destination >= FLASH_STORAGE_ADDRESS) {
        return FLASHIF_WRITING_ERROR;
    }

    FLASH_Unlock();

    for (i = 0; i < length; i++) {
        status = FLASH_ProgramWord(destination + (i * 4), p_source[i]);
        if (status != FLASH_COMPLETE) {
            FLASH_Lock();
            return FLASHIF_WRITING_ERROR;
        }

    }

    FLASH_Lock();
    return FLASHIF_OK;
}
