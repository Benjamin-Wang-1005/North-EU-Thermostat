#include "iap_uart.h"
#include "iap_flash.h"
#include "menu.h"
#include "ymodem.h"

extern volatile uint32_t time_tick;

pFunction JumpToApplication;
uint32_t JumpAddress;
uint32_t FlashProtection = 0;
uint8_t aFileName[FILE_NAME_LENGTH];

static void SerialDownload(void)
{
    uint8_t number[11] = {0};
    uint32_t size = 0;
    COM_StatusTypeDef result;

    Serial_PutString((uint8_t *)"Waiting for the file to be sent ... (press 'a' to abort)\n\r");
    FLASH_If_Erase(APPLICATION_ADDRESS);
    result = Ymodem_Receive(&size);
    if (result == COM_OK)
    {
        Serial_PutString((uint8_t *)"\n\n\r Programming Completed Successfully!\n\r--------------------------------\r\n Name: ");
        Serial_PutString(aFileName);
        Int2Str(number, size);
        Serial_PutString((uint8_t *)"\n\r Size: ");
        Serial_PutString(number);
        Serial_PutString((uint8_t *)" Bytes\r\n");
        Serial_PutString((uint8_t *)"-------------------\n");
    }
    else if (result == COM_LIMIT)
    {
        Serial_PutString((uint8_t *)"\n\n\rThe image size is higher than the allowed space memory!\n\r");
    }
    else if (result == COM_DATA)
    {
        Serial_PutString((uint8_t *)"\n\n\rVerification failed!\n\r");
    }
    else if (result == COM_ABORT)
    {
        Serial_PutString((uint8_t *)"\r\n\nAborted by user.\n\r");
    }
    else
    {
        Serial_PutString((uint8_t *)"\n\rFailed to receive the file!\n\r");
    }
}
/*
static void SerialUpload(void)
{
    uint8_t status = 0;

    Serial_PutString((uint8_t *)"\n\n\rSelect Receive File\n\r");

    if (Serial_GetByte(&status, RX_TIMEOUT) == 0)
    {
        if (status == CRC16)
        {
            status = Ymodem_Transmit((uint8_t*)APPLICATION_ADDRESS, (const uint8_t*)"UploadedFlashImage.bin", USER_FLASH_SIZE);

            if (status != 0)
            {
                Serial_PutString((uint8_t *)"\n\rError Occurred while Transmitting File\n\r");
            }
            else
            {
                Serial_PutString((uint8_t *)"\n\rFile uploaded successfully \n\r");
            }
        }
    }
}*/

static void JumpToApp(void)
{
    if (((*(__IO uint32_t*)APPLICATION_ADDRESS) & 0x2FFE0000) == 0x20000000)
    {
        JumpAddress = *(__IO uint32_t*)(APPLICATION_ADDRESS + 4);
        JumpToApplication = (pFunction)JumpAddress;
        __disable_irq();
        SetMSP(*(__IO uint32_t*)APPLICATION_ADDRESS);
        JumpToApplication();
    }
}

void Main_Menu(void)
{
    uint8_t key = 0;
    uint32_t menu_tick = time_tick;

    Serial_PutString((uint8_t *)"\r\n======================================================================");
    Serial_PutString((uint8_t *)"\r\n=         (C) COPYRIGHT 2026 E-poly Technology Co., Ltd.             =");
    Serial_PutString((uint8_t *)"\r\n=                                                                    =");
    Serial_PutString((uint8_t *)"\r\n=  FT32F10X  In-Application Programming Application  (Version 1.0.0) =");
    Serial_PutString((uint8_t *)"\r\n=                                                                    =");
    Serial_PutString((uint8_t *)"\r\n=                                   By Benjamin Wang                 =");
    Serial_PutString((uint8_t *)"\r\n======================================================================");
    Serial_PutString((uint8_t *)"\r\n\r\n");

    Serial_PutString((uint8_t *)"\r\n=================== Main Menu ============================\r\n\n");
    Serial_PutString((uint8_t *)"  Download image to the internal Flash ----------------- 1\r\n\n");
    Serial_PutString((uint8_t *)"  Upload image from the internal Flash ----------------- 2\r\n\n");
    Serial_PutString((uint8_t *)"  Execute the loaded application ----------------------- 3\r\n\n");
    Serial_PutString((uint8_t *)"==========================================================\r\n\n");

    while (1)
    {
        Serial_Flush();

        if (Serial_GetByte(&key, 1000) == 0)
        {
            menu_tick = time_tick;

            switch (key)
            {
            case '1':
                SerialDownload();
                break;
            case '2':
                //SerialUpload();
                break;
            case '3':
                Serial_PutString((uint8_t *)"Start program execution......\r\n\n");
                JumpToApp();
                break;
            default:
                Serial_PutString((uint8_t *)"Invalid Number ! ==> The number should be either 1, 2 or 3\r");
                break;
            }
        }
        else
        {
            if (time_tick - menu_tick > 300000)
            {
                Serial_PutString((uint8_t *)"\r\nTimeout: Jump to application...\r\n");
                JumpToApp();
            }
        }
    }
}
