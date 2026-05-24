#ifndef __MENU_H
#define __MENU_H

#include "iap_flash.h"
#include "ymodem.h"

typedef void (*pFunction)(void);

extern uint8_t aFileName[FILE_NAME_LENGTH];

void Main_Menu(void);

void SetMSP(uint32_t addr);

#endif

