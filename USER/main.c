//////////////////////////////////////////////////////////////////////////////////	 
// STM32 LCD Controller with Key Input
// Display: Setting number with Up/Down key control
// PA0  = Up Key (High Active)
// PC13 = Down Key (High Active)
//////////////////////////////////////////////////////////////////////////////////	 	
#include "Thermostat.h"
#include "sys.h"
#include "lcd.h"
#include "touch.h"
#include "gui.h"
#include "test.h"
#include "arial_digits_flash.h"
#include "icons_32x32.h"  // New 32x32 monochrome icons
#include "icons_8x16.h"   // New 8x16 monochrome icons for Function Setting
#include "icons_16x16.h"  // New 16x16 monochrome icons for Schedule Edit
#include "icons_24x24.h"  // New 24x24 monochrome icons for Active page
#include "stm32f10x.h"
#include "stm32f10x_tim.h"




// Function prototypes
void Key_Init(void);
void Backlight_Init(void);
void Backlight_SetDuty(uint8_t duty_percent);
uint8_t Key_Scan(void);
void Display_Number(float number, uint16_t color, uint8_t show_decimal);
void Clear_Number_Area(void);

void Draw_Main_Page(void);



// Global variables
float setting_number = 4.0f;     // 設定數字，初始值 4.0
uint8_t key;													// key status


//RTC Time Default 2026/01/01 00:00:00
rtc_time_t rtc_time;

// LCD color definitions (if not already defined)
#ifndef BLUE
#define BLUE    0x001F
#endif
#ifndef RED
#define RED     0xF800
#endif

int main(void)
{

	SystemInit();        // Initialize RCC, system clock to 72MHZ
	delay_init(72);	     // Delay initialization
	LCD_Init();	         // LCD initialization
	Key_Init();          // Key initialization
	Backlight_Init();    // Backlight PWM initialization
	Backlight_SetDuty(0); 	//close LCM at initial

	// Set rotation to 90 degrees
	LCD_direction(1);

	// Clear screen with white background
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);

	// Display static elements (time, icons)
	Draw_Active_Menu();
	

	// Main loop
	while(1)
	{
		// Scan keys
		key = Key_Scan();
		
		UI_Update();
		
	}

		
}

// Initialize GPIO for keys
void Key_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	// Enable clocks
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);

	// PA0 - Up Key (Input with pull-down)
	GPIO_InitStructure.GPIO_Pin = UP_KEY_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;  // Input pull-down
	GPIO_Init(UP_KEY_PORT, &GPIO_InitStructure);

	// PC13 - Down Key (Input with pull-down)
	GPIO_InitStructure.GPIO_Pin = DOWN_KEY_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;  // Input pull-down
	GPIO_Init(DOWN_KEY_PORT, &GPIO_InitStructure);

	// PA4 - Enter Key (Input with pull-down)
	GPIO_InitStructure.GPIO_Pin = ENTER_KEY_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;  // Input pull-down
	GPIO_Init(ENTER_KEY_PORT, &GPIO_InitStructure);
}



// Scan keys
// Return: 0 = no key, 1 = Up key, 2 = Down key, 3 = Enter key
uint8_t Key_Scan(void)
{
	static uint8_t key_up = 1;  // Key release flag
	static uint8_t key_release_cnt = 0;  // Key release debounce counter

	if(key_up)
	{
		if(GPIO_ReadInputDataBit(UP_KEY_PORT, UP_KEY_PIN) == 1)
		{
			delay_ms(5);  // Debounce (reduced from 10ms)
			if(GPIO_ReadInputDataBit(UP_KEY_PORT, UP_KEY_PIN) == 1)
			{
				key_up = 0;
				key_release_cnt = 0;
				return 1;  // Up key pressed
			}
		}
		else if(GPIO_ReadInputDataBit(DOWN_KEY_PORT, DOWN_KEY_PIN) == 1)
		{
			delay_ms(5);  // Debounce (reduced from 10ms)
			if(GPIO_ReadInputDataBit(DOWN_KEY_PORT, DOWN_KEY_PIN) == 1)
			{
				key_up = 0;
				key_release_cnt = 0;
				return 2;  // Down key pressed
			}
		}
		else if(GPIO_ReadInputDataBit(ENTER_KEY_PORT, ENTER_KEY_PIN) == 1)
		{
			delay_ms(5);  // Debounce (reduced from 10ms)
			if(GPIO_ReadInputDataBit(ENTER_KEY_PORT, ENTER_KEY_PIN) == 1)
			{
				key_up = 0;
				key_release_cnt = 0;
				return 3;  // Enter key pressed
			}
		}
	}
	else
	{
		// Wait for key release with debounce
		if(GPIO_ReadInputDataBit(UP_KEY_PORT, UP_KEY_PIN) == 0 &&
		   GPIO_ReadInputDataBit(DOWN_KEY_PORT, DOWN_KEY_PIN) == 0 &&
		   GPIO_ReadInputDataBit(ENTER_KEY_PORT, ENTER_KEY_PIN) == 0)
		{
			key_release_cnt++;
			if(key_release_cnt >= 3)  // Require 3 consecutive scans to confirm release
			{
				key_up = 1;
				key_release_cnt = 0;
			}
		}
		else
		{
			key_release_cnt = 0;
		}
	}

	return 0;  // No key
}








