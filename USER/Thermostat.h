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
#define STATE_ACTIVE    0   // Active��B��LCM initial����ᣬ����90%
#define STATE_SLEEP     1   // Sleep��B��10��o���I������30%
#define STATE_SETTING   2   // Setting��B��Active�°��I�|�l�����{������
#define STATE_FUNC_SETTING 3 // Function Setting��B���tɫManu�°�Enter�M��
#define STATE_FUNC_SETTING_EDIT 4 // Function Setting�x헾�݋ģʽ��Edit�tɫ�r��Enter�M��
#define STATE_HEATING_SCHEDULE_MENU 5 // Heating Schedule Menu��棺Function Setting��Heating Schedule Enter�M��
#define STATE_HEATING_SCHEDULE_PROG_TYPE 6 // Heating Schedule Menu -> Program Type Enter : Program Type���
#define STATE_SCHEDULE_EDIT 7 // Schedule Edit��棺Heating Schedule Menu��Weekly Schedule Enter�M��
#define STATE_SCHEDULE_TIME_SETTING 8 // Schedule Time Setting��棺P1~P6 Enter�M��
#define STATE_CONTROL_ADJ_MENU 9     // Control Adj Menu Page��Function Setting��Control Adj Enter
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
#include "stm32f10x_usart.h"
#include "pwm.h"
#include "UI.h"
#include "lcd.h"
#include "GUI.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "delay.h"
#include "Peripheral.h"
#include "icons_32x32.h"
#include "icons_24x24.h"
#include "icons_16x16.h"
#include "icons_8x16.h"
#include "arial_digits_flash.h"
#include "rtc_driver.h"

#define			u8					uint8_t
#define			u16					uint16_t
#define			u32					uint32_t

#define			DEBUG_LOG_ON															(1)

#if DEBUG_LOG_ON

    #define LOGD(...) \
    { \
            printf("[Inf] "__VA_ARGS__); \
    }

    #define LOGE(...) \
    { \
            printf("[Err] "__VA_ARGS__); \
    }
#else
#define LOGD(...)
#define LOGE(...)
#endif

void my_RTC_Init(void);
void Key_Init(void);
void Backlight_Init(void);
void Backlight_SetDuty(uint8_t duty_percent);
uint8_t Key_Scan(void);
void Log_USART_Init(void);

extern float setting_number;
extern rtc_time_t rtc_time;
extern uint8_t key;

#endif

