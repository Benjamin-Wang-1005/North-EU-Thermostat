#ifndef __IAP_FLASH_H
#define __IAP_FLASH_H

#include "stm32f10x.h"
#include "flash_storage.h"

#define APPLICATION_ADDRESS     0x08004000
#define IAP_FLASH_SIZE          (256 * 1024)
#define USER_FLASH_SIZE         0x0003B800
#define USER_FLASH_END_ADDRESS  0x08040000

#define FLASHIF_OK                  0
#define FLASHIF_ERASEKO             1
#define FLASHIF_WRITING_ERROR       2
#define FLASHIF_WRITINGCTRL_ERROR   3

void FLASH_If_Init(void);
uint32_t FLASH_If_Erase(uint32_t start);
uint32_t FLASH_If_Write(uint32_t destination, uint32_t *p_source, uint32_t length);

#endif

