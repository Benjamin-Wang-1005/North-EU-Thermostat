//---------------------------------------------------------------------------------------------------------
//
// Copyright(c) 2026 E-Poly Technology Co., Ltd. All rights reserved.
//
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
// E-poly North EU Thermostat Project
// Author: Benjamin Wang
// Date: 2026/04/06
// Email: Benjamin@epoly-tech.com
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
// File Function: Thermostat project base header file
//---------------------------------------------------------------------------------------------------------

#ifndef _THERMOSTAT_H
#define _THERMOSTAT_H

// Key definitions
#define UP_KEY_PIN    GPIO_Pin_0   // PA0
#define UP_KEY_PORT   GPIOA
#define DOWN_KEY_PIN  GPIO_Pin_13  // PC13
#define DOWN_KEY_PORT GPIOC
#define ENTER_KEY_PIN    GPIO_Pin_4   // PA4
#define ENTER_KEY_PORT   GPIOA
#define RELAY_PIN		 	GPIO_Pin_8		//PB8
#define RELAY_PORT	 	GPIOB

enum{
		STATE_ACTIVE = 0,
		STATE_SLEEP,
		STATE_SETTING,
		STATE_FUNC_SETTING,
		STATE_FUNC_SETTING_EDIT,
		STATE_OPERATION_MODE_MENU,
	
		STATE_HEATING_SCHEDULE_MENU,
		STATE_HEATING_SCHEDULE_PROG_TYPE,
		STATE_SCHEDULE_EDIT,
		STATE_SCHEDULE_TIME_SETTING,
		STATE_LEAVE_SCHEDULE_CONFIRM,
		STATE_WINDOW_OPEN_DETECTED,
		STATE_WINDOW_OPEN_CONFIRM,
		
		STATE_CONTROL_ADJ_MENU,
		STATE_CONTROL_ADJ_SENSOR,
		STATE_CONTROL_POWER_LIMIT,
		//STATE_CONTROL_COMFORT_MODE,
		//STATE_CONTROL_COMFORT_MODE_SETTING,
		STATE_CONTROL_TEMP_SWING,
		STATE_CONTROL_ADJ_TEMP_CORRECT,
		STATE_CONTROL_ADJ_TEMP_LIMIT,
		STATE_CONTROL_ADJ_TEMP_PROTECT,
		STATE_CONTROL_ADJ_TEMP_PROTECT_MAX,
		STATE_CONTROL_ADJ_TEMP_PROTECT_MIN,
		STATE_CONTROL_ADJ_POWER_ON_STATE,
		STATE_CONTROL_ADJ_PWM_SETTING,
		STATE_CONTROL_ADJ_PWM_DUTY,
		STATE_CONTROL_ADJ_PWM_ASTRO_TIME,
	
		STATE_USER_SETTING_MENU,
		STATE_USER_SETTING_CHILD_LOCK,
		STATE_CHILD_LOCK_PIN_SETUP,
		STATE_CHILD_LOCK_PIN_VERIFY,
		STATE_USER_SETTING_WINDOW_FUN,
		STATE_USER_SETTING_WINDOW_TIME,
		STATE_USER_SETTING_SET_TIME,
		STATE_USER_SETTING_SET_CLK,
		//STATE_USER_SETTING_COMFORT_MODE,
		STATE_USER_SETTING_LANGUAGE,
		STATE_USER_SETTING_ADAPTIVE_START,
		//STATE_USER_SETTING_COMFORT_MODE_SETTING,
		STATE_USER_SETTING_BACKLIGHT,
		STATE_USER_SETTING_RESET,
		
		STATE_OPERATION_MODE_PWM_SETTING,

		STATE_FACTORY_TEST,

};

#include <stdint.h>

// Menu definition structures
#define MENU_VISIBLE_ROWS 4

typedef struct {
	uint8_t text_id;
	uint8_t next_state;
	void (*draw_page)(uint8_t, uint8_t, uint8_t);
	void (*init_func)(void);
} MenuItem_t;

extern const MenuItem_t Control_Adj_Menu_Items[];
extern const uint8_t Control_Adj_Menu_Item_Count;

extern const MenuItem_t User_Setting_Menu_Items[];
extern const uint8_t User_Setting_Menu_Item_Count;


// State definitions
/*
#define STATE_ACTIVE    0   // 
#define STATE_SLEEP     1   // 
#define STATE_SETTING   2   // 
#define STATE_FUNC_SETTING 3 // 
#define STATE_FUNC_SETTING_EDIT 4 //
#define STATE_OPERATION_MODE_MENU 5

#define STATE_HEATING_SCHEDULE_MENU 6 // 
#define STATE_HEATING_SCHEDULE_PROG_TYPE 7 // 
#define STATE_SCHEDULE_EDIT 8 // 
#define STATE_SCHEDULE_TIME_SETTING 9 // 

#define STATE_CONTROL_ADJ_MENU 10     // 
#define STATE_CONTROL_ADJ_SENSOR 11 // Control Adj Menu -> Sensor Enter : Sensor Setting Page
#define STATE_CONTROL_ADJ_TEMP_CORRECT 12 // Control Adj Menu -> Temp. Correct Enter : Temp. Correct Setting Page
#define STATE_CONTROL_ADJ_TEMP_LIMIT 13		// Control Adj Menu -> Temp. Limit Enter : Temp. Limit Setting Page
#define STATE_CONTROL_ADJ_TEMP_PROTECT 14 // Control Adj Menu -> Temp. Protect Enter : Temp Protect Setting Page
#define STATE_CONTROL_ADJ_TEMP_PROTECT_MAX 15
#define STATE_CONTROL_ADJ_TEMP_PROTECT_MIN 16
#define STATE_CONTROL_ADJ_POWER_ON_STATE 17
#define STATE_CONTROL_PWM_DUTY_CYCLE 18

#define STATE_USER_SETTING_MENU 19
#define STATE_USER_SETTING_CHILD_LOCK 20
#define STATE_USER_SETTING_WINDOW_FUN 21
#define STATE_USER_SETTING_WINDOW_TIME 22
#define	STATE_USER_SETTING_SET_TIME 23
#define STATE_USER_SETTING_SET_CLK 24
#define STATE_USER_SETTING_BACKLIGHT 25
#define STATE_USER_SETTING_RESET 26
#define STATE_CHILD_LOCK_PIN_SETUP 27
#define STATE_CHILD_LOCK_PIN_VERIFY 28
#define STATE_LEAVE_SCHEDULE_CONFIRM 29 */

// Thermostat Temperature Limit Define
#define INPUT_TEMPERATURE_MAX_DEFAULT 55
#define INPUT_TEMPERATURE_MIN_DEFAULT 0
#define DEVICE_OPERATION_TEMPERATURE_MAX 70
#define DEVICE_OPERATION_TEMPERATURE_MIN -20
#define DEVICE_PROTECT_TEMP_MAX_DEFAULT 55
#define DEVICE_PROTECT_TEMP_MIN_DEFAULT 4
#define MIN_TEMP_HIGH_TEMP_PROTECT 30
#define MAX_TEMP_LOW_TEMP_PROTECT 10

// Backlight PWM definitions
#define BACKLIGHT_PWM_FREQ    500   // 500Hz PWM frequency
#define BACKLIGHT_DUTY_ACTIVE 100   // 100% duty cycle for active state (full brightness)
#define BACKLIGHT_DUTY_SLEEP  20    // 20% duty cycle for sleep state
typedef enum
{
    eFALSE = 0,
    eTRUE
} bool_t;

#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "pwm.h"
#include "UI.h"
#include "lcd.h"
#include "GUI.h"
#include <stdio.h>
#include <string.h>
#include "Peripheral.h"
#include "ADC.h"
#include "icons_32x32.h"
#include "icons_24x24.h"
#include "icons_16x16.h"
#include "icons_8x16.h"
#include "arial_digits_flash.h"
#include "rtc_driver.h"
#include "flash_storage.h"
#include "Language.h"
#include "Adaptive_Start.h"

#define			u8					uint8_t
#define			u16					uint16_t
#define			u32					uint32_t

#define			DEBUG_LOG_ON															(1)



#if DEBUG_LOG_ON

    #define LOGD(...) \
    { \
            printf("[Inf]"__VA_ARGS__); \
    }

    #define LOGE(...) \
    { \
            printf("[Err]"__VA_ARGS__); \
    }
		#define TCMD(...) \
		{ \
						printf("[CMD]"__VA_ARGS__);\
		}
#else
#define LOGD(...)
#define LOGE(...)
#endif

enum
{
		RELAY_OFF = 0,
		RELAY_ON
};		
		
		
void my_RTC_Init(void);
void Key_Init(void);
void Backlight_Init(void);
void Backlight_SetDuty(uint8_t duty_percent);
bool_t register_alarm(alarm_source_t source, uint32_t dur);
void Log_USART_Init(void);
void sys_tick_Init(void);
void golbal_par_init(void);
void relay_init(void);
void Thermostat_Update(void);
void g_check_rtc(void);

extern float setting_number;
extern rtc_time_t rtc_time;
extern volatile uint8_t	Relay;
extern volatile uint8_t Schedule_Period;
extern uint8_t (*sch_table)[4];
extern volatile uint8_t		Factory_testing;
extern volatile uint8_t Update_Relay;


#endif

