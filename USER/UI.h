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
// File Function: UI header file
//---------------------------------------------------------------------------------------------------------

#ifndef __UI_H 
#define __UI_H 
#include <stdint.h>

#define		MAX_SET_TEMP														(45)


typedef enum{
		Time_Clear = 0,
		Hour,
		Minume,
		Temperature
} set_time_t;


extern uint8_t current_prog_type;
extern uint8_t	Top_Bar_Active;
extern uint8_t schedule_edit_scroll;
extern uint8_t schedule_edit_source;
extern uint8_t item_selection;
extern uint8_t schedule_settings[6][4];
extern uint8_t schedule_settings_restday[6][4];
extern uint8_t schedule_time_edit_hour;        // 小時 (0-23)
extern uint8_t schedule_time_edit_min;        // 分 (0-59)
extern uint8_t schedule_time_edit_temp;       // 溫度 (0-45)
extern uint8_t schedule_time_on_off;           // ON/OFF狀態: 0=OFF, 1=ON
extern uint8_t control_adj_menu_scroll;				// 捲動偏移: 0=show row0-3, 1=show row1-4
extern uint8_t current_sensor_type;          // 儲存設定: 0=Room, 1=Floor (default Room)
extern float temp_correct_internal;    			 // Internal sensor correct num
extern float temp_correct_external;					 // External sensor correct num
extern int	 temp_limit_max;
extern int	 temp_limit_min;
extern int 	 temp_protect_max;
extern int	 temp_protect_min;
extern uint8_t temp_protect_max_switch;
extern uint8_t temp_protect_min_switch;
extern uint8_t UI_state;
extern uint8_t power_on_state;
extern uint8_t user_setting_menu_scroll;
extern uint8_t child_lock;
extern uint8_t window_fun;
extern uint8_t window_fun_temp;
extern uint8_t window_fun_time;
extern uint8_t sleep_backlight_duty;


void Draw_Active_Menu(void);
void Draw_Static_Icons(void);
void UI_Update(void);
void Draw_TopBar(uint8_t leave_col, uint8_t edit_col);
void Draw_Heating_Schedule_Menu_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_Heating_Schedule_Menu_Row(uint8_t row, uint8_t selected);
void Draw_Heating_Schedule_Prog_Type_Page(uint8_t selected, uint8_t leave_col, uint8_t edit_col);
void Draw_Schedule_Edit_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_Heating_Schedule_Prog_Type_Content(uint8_t selected);
void Draw_Schedule_Edit_Finish(uint8_t selected);
void Draw_Schedule_Edit_Row(uint8_t row, uint8_t period_idx, uint8_t selected);
void Draw_Schedule_Time_Setting_Page(uint8_t period_num, uint8_t leave_col, uint8_t edit_col);
void Draw_Schedule_Time_Setting_Arrows(uint8_t show);
void Clear_Schedule_Time_Setting_Arror(uint8_t clr);
void Draw_Schedule_Time_Setting_TimeDigits(void);
void Draw_Schedule_Time_Setting_TempDigits(void);
void Draw_Schedule_Time_Setting_OnOff(void);
void Draw_Schedule_Time_Setting_Save(void);
void Draw_Control_Adj_Menu_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_Control_Adj_Menu_Row(uint8_t row, uint8_t selected);
void Draw_Control_Adj_Sensor_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_Control_Adj_TempCorrect_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_Control_Adj_Sensor_Choices(uint8_t selection);
void Draw_Control_Adj_Sensor_Save(uint8_t selected);
void Draw_Control_Adj_TempCorrect_Contect(uint8_t selection);
void Display_Adj_Number(float number, uint16_t color, uint8_t show_decimal);
void Draw_Control_Adj_TempLimit_Page(uint8_t item_selection, uint8_t leave_col, uint8_t edit_col);
void Draw_Control_Adj_TempLimit_Content(uint8_t selection);
void Draw_Control_Adj_TempProtect_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_Control_Adj_TempProtect_Max_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_Control_Adj_TempProtect_Max_Content(uint8_t selection);
void Draw_Control_Adj_TempProtect_Min_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_Control_Adj_TempProtect_Min_Content(uint8_t selection);
void Draw_Control_Adj_Power_On_State_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_Control_Adj_Power_On_State_Arrow(uint8_t selection);
void Draw_User_Setting_Menu_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_User_Setting_Menu_Row(uint8_t row, uint8_t selected);
void Draw_User_Setting_Child_Lock_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_User_Setting_Child_Lock_Choice(uint8_t selection);
void Draw_User_Setting_Window_Fun_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_User_Setting_Window_Fun_Content(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_User_Setting_Window_Time_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_User_Setting_Window_Time_Content(uint8_t selection);
void Draw_User_Setting_SetTime_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_User_Setting_SetTime_Content(uint8_t selection);
void Draw_User_Setting_Setclk_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_User_Setting_Setclk_Content(uint8_t selection);
void Draw_User_Setting_Backlight_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_User_Setting_Backlight_Content(uint8_t selection);
void Draw_User_Setting_Reset_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_User_Setting_Reset_Contect(uint8_t selection);


#endif




