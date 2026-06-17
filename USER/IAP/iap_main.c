#include "stm32f10x.h"
#include "iap_flash.h"
#include "iap_uart.h"
#include "menu.h"

volatile uint32_t time_tick = 0;

int main(void)
{
    SystemInit();
    SysTick_Config(SystemCoreClock / 1000);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);

    if (BKP_ReadBackupRegister(BKP_DR2) == 0x5AA5)
    {
        BKP_WriteBackupRegister(BKP_DR2, 0x0000);

        FLASH_If_Init();
        IAP_UART_Init();
        Main_Menu();
    }
    else
    {
        if (((*(__IO uint32_t*)APPLICATION_ADDRESS) & 0x2FFE0000) == 0x20000000)
        {
            uint32_t JumpAddress = *(__IO uint32_t*)(APPLICATION_ADDRESS + 4);
            void (*JumpToApplication)(void) = (void (*)(void))JumpAddress;
            SetMSP(*(__IO uint32_t*)APPLICATION_ADDRESS);
            JumpToApplication();
        }
    }

    while (1)
    {
    }
}

void SysTick_Handler(void)
{
    time_tick++;
}
