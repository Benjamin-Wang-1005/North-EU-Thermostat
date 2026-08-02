//---------------------------------------------------------------------------------------------------------
//
// Copyright(c) 2026 E-Poly Technology Co., Ltd. All rights reserved.
//
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
// E-Poly North EU Thermostat Project
// Author: Benjamin Wang
// Date: 2026/05/13
// Email: Benjamin@epoly-tech.com
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
// File Function: Language header file
//      Define multi-language string use for LCD display
//---------------------------------------------------------------------------------------------------------

#ifndef _LANGUAGE_H_
#define _LANGUAGE_H_

#include <stdint.h>

typedef enum {
 LANG_ENGLISH = 0,
 LANG_Norwegian, // Norway Language
 LANG_MAX
} Language_t;

typedef enum {
 STR_Save = 0,
 STR_Temperature_Read_From,
 STR_Room,
 STR_Floor,
 STR_Sensor_Calibrate,
 STR_Temperature_Limit,
 STR_Max,
 STR_Min,
 STR_Max_Temperature_Protect,
 STR_Min_Temperature_Protect,
 STR_Power_On_State,
 STR_Keep_State,
 STR_Device_Close,
 STR_Device_Open,
 STR_Power_Limit,
 STR_OFF,
 STR_ON,
 STR_Language,
 STR_Menu_Language,
 STR_English,
 STR_Norwegian,
 STR_Temperature_Swing,
 STR_Temp_Swing,
 STR_Program_Type,
 STR_Type,
 STR_Workday_Setting,
 STR_Workday,
 STR_Restday_Setting,
 STR_Restday,
 STR_Current_Type,
 STR_Finish,
 STR_Period_1_On,
 STR_Period_1_Off,
 STR_Leave_Schedule,
 STR_Mode,
 STR_Cancel,
 STR_Yes,
 STR_Window_Open,
 STR_Detected,
 STR_Cancel_Detect,
 STR_Reset_Detect,
 //STR_FRI,
 //STR_MON,
 //STR_SAT,
 //STR_SUN,
 //STR_THU,
 //STR_TUE,
 //STR_WED,
 STR_Sensor,
 STR_Input_Temperature_Limit,
 STR_Protect_Temperature,
 STR_Protect_Temperature_short,
 STR_Child_lock,
 STR_Window_Function,
 STR_Set_Time,
 STR_Set_Backlight,
 STR_Factory_Reset,
 STR_Operation_Mode,
 STR_Heating_Schedule,
 STR_Control_Adjustment,
 STR_User_Settings,
 STR_Manual_Mode,
 STR_Manual,
 STR_Schedule_Mode,
 STR_Schedule,
 STR_Enter_Pin_Code,
 STR_User_Setting,
 STR_Set_Clock,
 STR_Trigger_Temp,
 STR_And_Time,
 STR_Temperature,
 STR_Time,
 STR_Year_Month_Date,
 STR_Hour_Minute,
 STR_Reset,
 STR_Reset_to,
 STR_Default,
 STR_This_will,
 STR_erase_all,
 STR_saved_settings,
 STR_Setup_Pin,
	STR_Confirm_Pin, 
	STR_Set_Date,
	STR_OP_Mode,
 //STR_Schedule,
 STR_Adjust,
 STR_Setting,
 STR_hysteresis,
 STR_Calibrate,
 STR_Input_Limit,
 STR_Sensor_Cal_Short,
 STR_Keep,
 STR_State,
 STR_PWM_Mode,
 STR_PWM_Duty_Cycle,
 STR_Next,
 STR_PWM_Setting,
 STR_Duty,
 STR_Duration,
 STR_Astro_Time,
 STR_PWM_Duty,
 STR_Internal,
 STR_External,
	STR_Sensor_Error,
	STR_Astro_Setting,
	STR_Day,
	STR_Night,
	STR_Adaptive_Start,
	STR_Adaptive,
	STR_Hour,
	STR_MAX,
 
} StringID_t;


extern const char* const LanguageTable[STR_MAX][LANG_MAX];

#endif
