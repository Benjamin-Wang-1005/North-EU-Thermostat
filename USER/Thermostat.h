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

// State definitions
#define STATE_ACTIVE    0   // Active狀態：LCM initial完成後，背光90%
#define STATE_SLEEP     1   // Sleep狀態：10秒無按鍵，背光30%
#define STATE_SETTING   2   // Setting狀態：Active下按鍵觸發，可調整數字
#define STATE_FUNC_SETTING 3 // Function Setting狀態：紅色Manu下按Enter進入
#define STATE_FUNC_SETTING_EDIT 4 // Function Setting選項編輯模式：Edit紅色時按Enter進入
#define STATE_HEATING_SCHEDULE_MENU 5 // Heating Schedule Menu頁面：Function Setting中Heating Schedule Enter進入
#define STATE_HEATING_SCHEDULE_PROG_TYPE 6 // Heating Schedule Menu -> Program Type Enter : Program Type頁面
#define STATE_SCHEDULE_EDIT 7 // Schedule Edit頁面：Heating Schedule Menu中Weekly Schedule Enter進入
#define STATE_SCHEDULE_TIME_SETTING 8 // Schedule Time Setting頁面：P1~P6 Enter進入
#define STATE_CONTROL_ADJ_MENU 9     // Control Adj Menu Page：Function Setting中Control Adj Enter
#define STATE_CONTROL_ADJ_SENSOR 10 // Control Adj Menu -> Sensor Enter : Sensor Setting Page
#define STATE_CONTROL_ADJ_TEMP_CORRECT 11 // Control Adj Menu -> Temp. Correct Enter : Temp. Correct Setting Page
#define STATE_CONTROL_ADJ_TEMP_LIMIT 12		// Control Adj Menu -> Temp. Limit Enter : Temp. Limit Setting Page
#define STATE_CONTROL_ADJ_TEMP_PROTECT 13 // Control Adj Menu -> Temp. Protect Enter : Temp Protect Setting Page
#define STATE_CONTROL_ADJ_TEMP_PROTECT_MAX 14
#define STATE_CONTROL_ADJ_TEMP_PROTECT_MIN 15
#define STATE_CONTROL_ADJ_POWER_ON_STATE 16
#define STATE_USER_SETTING_MENU 17
#define STATE_USER_SETTING_CHILD_LOCK 18
#define STATE_USER_SETTING_WINDOW_FUN 19
#define STATE_USER_SETTING_WINDOW_TIME 20
#define	STATE_USER_SETTING_SET_TIME 21
#define STATE_USER_SETTING_SET_CLK 22
#define STATE_USER_SETTING_BACKLIGHT 23
#define STATE_USER_SETTING_RESET 24

// Backlight PWM definitions
#define BACKLIGHT_PWM_FREQ    500   // 500Hz PWM frequency
#define BACKLIGHT_DUTY_ACTIVE 100   // 100% duty cycle for active state (full brightness)
#define BACKLIGHT_DUTY_SLEEP  20    // 20% duty cycle for sleep state


#include "stm32f10x_gpio.h"
#include "pwm.h"
#include "UI.h"
#include "lcd.h"
#include "GUI.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "delay.h"
#include "icons_32x32.h"
#include "icons_24x24.h"
#include "icons_16x16.h"
#include "icons_8x16.h"
#include "arial_digits_flash.h"

#define			u8					uint8_t
#define			u16					uint16_t
#define			u32					uint32_t

typedef struct
{
		uint8_t		Year;
		uint8_t		Mon;												
		uint8_t 	Date;
		uint8_t 	Hour;
		uint8_t		Min;
		uint8_t		Sec;
} rtc_time_t;

extern float setting_number;
extern rtc_time_t rtc_time;
extern uint8_t key;													

#endif

