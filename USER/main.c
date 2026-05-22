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


void Draw_Main_Page(void);



// Global variables
//float setting_number = 4.0f;     	// 設定數字，初始值 4.0
volatile uint8_t	Relay;								// Output power indicator
volatile uint8_t Schedule_Period;					// Schedule indicator
uint8_t (*sch_table)[4];
volatile uint8_t Update_Relay;


//RTC Time Default 2026/01/01 00:00:00
rtc_time_t rtc_time;

// LCD color definitions (if not already defined)
#ifndef BLUE
#define BLUE    0x001F
#endif
#ifndef RED
#define RED     0xF800
#endif
void test_system_tick(void)
{
		static uint32_t local_tick;
		if(0 == g_clock_time_exceed(local_tick, 500)){								
				return;
		}
		local_tick = time_tick;
		RELAY_PORT->ODR ^= RELAY_PIN;
		LOGD("Time Update\r\n");
	
}
int main(void)
{
//	uint8_t key_val;

	SystemInit();        // Initialize RCC, system clock to 64MHz (HSI/2 * 16 PLL)
	sys_tick_Init();	 // Initialize system tick clock
	Key_Init();          // Key initialization
	Backlight_Init();    // Backlight PWM initialization
	Backlight_SetDuty(0); 	//close LCM at initial
	Log_USART_Init();
	LCD_Init();	         // LCD initialization
	// Set rotation to 90 degrees
	LCD_direction(1);
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);			// Clear screen with white background
	LOGD("-----------------  Program Start --------------------------\n\r");
	
	my_RTC_Init();			 //Read RTC Time
	
	if (Flash_Load_Parameter() != FLASH_OK) {
        LOGD("Flash load failed, using default values\r\n");
        golbal_par_init();
        if(Flash_Save_Parameter() != FLASH_OK){
						LOGE("Flash write fail!\r\n");
				}
        LOGD("Default values saved to Flash\r\n");
   } else {
        LOGD("Parameters loaded from Flash\r\n");
   }
	 weekday = getWeekday(rtc_time.Year, rtc_time.Mon, rtc_time.Date);
	 UI_state = STATE_ACTIVE;
	 Backlight_SetDuty(BACKLIGHT_DUTY_ACTIVE);
	 if(register_alarm(Active_Alarm, ACTIVE_ALIVE_TIME) == eFALSE){
			 LOGE("Alarm Fail\r\n");
	 }
	 Draw_Active_Menu();
	 relay_init();
	 if(rtc_rest){
				LOGD("RTC Reset!\r\n");
	 }
		LOGD("%d/%d/%d  %d:%d  W:%s\r\n", rtc_time.Year, rtc_time.Mon, rtc_time.Date, rtc_time.Hour, rtc_time.Min, week_texts[weekday]);
		get_schedule_period();
		
	// Initialize ADC and measure VCC voltage
	if(register_alarm(Min_Update_Alarm, 60000) == 0){
			LOGE("Alarm Full!\r\n");
	}
#if(!SIMULATION)
	Thermostat_ADC_Init();
#endif
	
	// Main loop
	while(1)
	{
		// Scan keys
		Key_Scan();
		
		if(Factory_testing == 0){
				UI_Update();
		}

#if(!SIMULATION)		
		Thermostat_Update();
#endif
		
		g_relay_handler();
		
		g_cmd_handler();

		g_check_alarm();
		
	}

		
}













