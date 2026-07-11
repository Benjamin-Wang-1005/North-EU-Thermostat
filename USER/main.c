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
//#include "Language.h"




// Function prototypes


void Draw_Main_Page(void);



// Global variables

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
	uint32_t temp_time;
	SystemInit();        // Initialize RCC, system clock to 64MHz (HSI/2 * 16 PLL)
	//SCB->VTOR = 0x08004000;  // Vector table relocated for IAP application area
	sys_tick_Init();	 // Initialize system tick clock
	Key_Init();          // Key initialization
	Backlight_Init();    // Backlight PWM initialization
	Backlight_SetDuty(0); 	//close LCM at initial
	Log_USART_Init();
	LCD_Init();	         // LCD initialization
	// Set rotation to 0 degrees
	LCD_direction(2);
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);			// Clear screen with white background
	LOGD("-----------------  Program Start --------------------------\n\r");
	
	my_RTC_Init();			 //Read RTC Time

#if(!SIMULATION)
	Thermostat_ADC_Init();
	LOGD("Comp_step:%.2f\r\n", adc_ctrl.idle_comp_step_val);
#else
	adc_ctrl.int_ntc_valid = 1;
	adc_ctrl.ext_ntc_valid = 1;
#endif
	
	if (Flash_Load_Parameter() != FLASH_OK) {
        LOGD("Flash load failed, using default values\r\n");
        golbal_par_init();
				adc_ctrl.idle_comp_count = 0;
				adc_ctrl.heating_comp_count = 0;
        if(Flash_Save_Parameter() != FLASH_OK){
						LOGE("Flash write fail!\r\n");
				}
        LOGD("Default values saved to Flash\r\n");
   } else {
        LOGD("Parameters loaded from Flash\r\n");
				
				if(rtc_rest){
						adc_ctrl.idle_comp_count = 0;
						adc_ctrl.heating_comp_count = 0;
				}else{
						temp_time = g_check_power_off_time();
						if(temp_time >= 60){
								adc_ctrl.idle_comp_count = 0;
								adc_ctrl.heating_comp_count = 0;
						}else{
								if((g_parameter.idle_comp_count <= 60) && (g_parameter.idle_comp_count >= (temp_time))){
										adc_ctrl.idle_comp_count = g_parameter.idle_comp_count - (temp_time);
										adc_ctrl.idle_comp_val = adc_ctrl.idle_comp_step_val * adc_ctrl.idle_comp_count;
								}else{
										adc_ctrl.idle_comp_count = 0;
										adc_ctrl.idle_comp_val = 0;
								}
								if((g_parameter.heatting_comp_count <= 60) && (g_parameter.heatting_comp_count >= temp_time)){
										adc_ctrl.heating_comp_count = g_parameter.heatting_comp_count - temp_time;
										adc_ctrl.heating_comp_val = adc_ctrl.heatting_comp_step_val * adc_ctrl.heating_comp_count;
								}else{
										adc_ctrl.heating_comp_count = 0;
										adc_ctrl.heating_comp_val = 0;
								}
						}
						LOGD("idle_count : %d\r\n",adc_ctrl.idle_comp_count);
						LOGD("heatting_count : %d\r\n", adc_ctrl.heating_comp_count);
				}
   }
	 relay_init();

	 // Initialize PWM day/night state from current RTC time
	 {
			 uint16_t now_min = rtc_time.Hour * 60 + rtc_time.Min;
			 uint16_t day_start = g_parameter.astro_day_hour * 60 + g_parameter.astro_day_min;
			 uint16_t night_start = g_parameter.astro_night_hour * 60 + g_parameter.astro_night_min;
			 pwm_is_daytime = (now_min >= day_start && now_min < night_start) ? 1 : 0;
	 }

	 weekday = getWeekday(rtc_time.Year, rtc_time.Mon, rtc_time.Date);
	 g_parameter.font_size = 0;			//for test
	 UI_state = STATE_ACTIVE;
	 Backlight_SetDuty(BACKLIGHT_DUTY_ACTIVE);
	 if(register_alarm(Active_Alarm, ACTIVE_ALIVE_TIME) == eFALSE){
			 LOGE("Alarm Fail\r\n");
	 }
	 Draw_Active_Menu();
	 
	 if(rtc_rest){
				LOGD("RTC Reset!\r\n");
	 }else{
			LOGD("%d/%d/%d  %d:%d  W:%s\r\n", rtc_time.Year, rtc_time.Mon, rtc_time.Date, rtc_time.Hour, rtc_time.Min, en_week_texts[weekday]);	
	 }
	 get_schedule_period();
	  
		
	// Initialize ADC and measure VCC voltage
	if(register_alarm(Min_Update_Alarm, 60000) == 0){
			LOGE("Alarm Full!\r\n");
	}

	
	// Main loop
	while(1)
	{
		// Scan keys
		Key_Scan();
		
		if(Factory_testing == 0){
				UI_Update();
		}

#if(!SIMULATION)		
		Thermostat_Update();					//ADC Update
#endif
		
		g_relay_handler();
		
		g_cmd_handler();

		g_check_alarm();
		
	}

		
}













