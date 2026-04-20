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
	Backlight_Init();    // Backlight PWM initialization
	Backlight_SetDuty(0); 	//close LCM at initial
	LCD_Init();	         // LCD initialization
	// Set rotation to 90 degrees
	LCD_direction(1);

	// Clear screen with white background
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);

	// Display static elements (time, icons)
	Draw_Active_Menu();
	Log_USART_Init();
	//my_RTC_Init();			 //Read RTC Time
	Key_Init();          // Key initialization
	
	
	
	LOGD("-----------------  Program Start --------------------------\n\r");
	// Main loop
	while(1)
	{
		// Scan keys
		key = Key_Scan();
		
		UI_Update();
		
	}

		
}













