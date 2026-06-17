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
#include "rtc_driver.h"

#define		MAX_SET_TEMP														(45)
#define 	SIMULATION															(0)


typedef enum{
		Time_Clear = 0,
		Hour,
		Minume,
		Temperature
} set_time_t;

typedef struct{
		float temp_correct_internal;						//4 Bytes
		float temp_correct_external;						//4 Bytes
		float temp_swing;												
		int	temp_limit_max;											//4 Bytes
		int temp_limit_min;											//4 Bytes
		int 	temp_protect_max;									//4 Bytes
		int		temp_protect_min;									//4 Bytes
		int		setting_number;										//4 Bytes
		int		room_temp;												//4 Bytes
		unsigned char  temp_protect_max_switch;
		unsigned char  temp_protect_min_switch;
		unsigned char  power_on_state;
		unsigned char  window_fun;
		unsigned char  window_fun_temp;
		unsigned char  window_fun_time;
		unsigned char  sleep_backlight_duty;
		unsigned char  child_lock;
		unsigned char  pin_code[4];  // 4-digit PIN code for child lock
		unsigned char  current_prog_type;
		unsigned char  relay_staty;
		unsigned char  operation_mode;
		unsigned char  sensor_type;				// 0=Room, 1=Floor
		unsigned char  power_limit;
		unsigned char  power_limit_switch;
		unsigned char  temp_diff;
		unsigned char  comfort_mode;
		unsigned char  floor_material;
		unsigned char  rtc_set_flag;
		unsigned char  language;
		unsigned char  workday_schedule[6][4];
		unsigned char  holiday_schedule[6][4];
}g_parameter_t;

enum
{
    Display_Room_Temp = 0,
    Display_Setting_Temp
};

extern g_parameter_t g_parameter;

extern uint8_t current_prog_type;
extern uint8_t	Top_Bar_Active;
extern uint8_t schedule_edit_scroll;
extern uint8_t schedule_edit_source;
extern uint8_t item_selection;
//extern uint8_t schedule_settings[6][4];
//extern uint8_t schedule_settings_restday[6][4];
extern uint8_t schedule_time_edit_hour;        // (0-23)
extern uint8_t schedule_time_edit_min;        // (0-59)
extern uint8_t schedule_time_edit_temp;       // (0-45)
extern uint8_t schedule_time_on_off;           //  0=OFF, 1=ON
extern uint8_t control_adj_menu_scroll;				// 0=show row0-3, 1=show row1-4
extern uint8_t current_sensor_type;          // 0=Room, 1=Floor (default Room)
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
extern uint8_t operation_mode;
extern uint8_t window_fun;
extern uint8_t window_fun_temp;
extern uint8_t window_fun_time;
extern uint8_t window_fun_triggered;
extern uint8_t window_open_confirm_selection;
extern uint8_t window_open_flash_state;
extern float window_fun_temp_buffer[60];
extern uint8_t window_fun_buffer_count;
extern uint8_t window_fun_buffer_index;
extern volatile uint8_t window_fun_int_updated;
extern volatile uint8_t window_fun_ext_updated;
extern uint8_t sleep_backlight_duty;
extern uint8_t icon6_red_state;
extern uint8_t main_display_digi;
extern rtc_time_t temp_rtc;
//extern uint8_t old_relay_state;
extern uint8_t power_limit;
extern uint8_t power_limit_switch;
extern uint8_t comfort_mode;
extern uint8_t floor_material;
extern float temp_swing;
extern uint8_t child_lock_flag;

// Child Lock PIN code variables
extern uint8_t pin_code_input[4];      // Current PIN input buffer
extern uint8_t pin_code_confirm[4];    // PIN confirmation buffer
extern uint8_t pin_digit_index;        // Current digit index (0-3)
extern uint8_t pin_setup_stage;        // 0=enter PIN, 1=confirm PIN
extern uint8_t pin_digit_selected;     // Currently selected digit (0-9) for input
extern char* en_week_texts[7];
extern const char* lan_str;
extern uint8_t language_temp;

extern float	disp_fack_temp;

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
void window_function_reset(void);
void Draw_User_Setting_Window_Time_Content(uint8_t selection);
void Draw_User_Setting_SetTime_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_User_Setting_SetTime_Content(uint8_t selection);
void Draw_User_Setting_Setclk_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_User_Setting_Setclk_Content(uint8_t selection);
void Draw_User_Setting_Backlight_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_User_Setting_Backlight_Content(uint8_t selection);
void Draw_User_Setting_Reset_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_User_Setting_Reset_Contect(uint8_t selection);
void Clear_Number_Area(void);
void Draw_Window_Open_Confirm_Page(uint8_t selection);
void Display_Number(float number, uint16_t color, uint8_t show_decimal);
void Draw_Control_Adj_Power_Limit_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_Control_Adj_Power_Limit_Content(uint8_t selection);
void Draw_Control_Adj_Comfort_Mode_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_Control_Adj_Comfort_Mode_Content(uint8_t selection);
void Draw_Control_Adj_Comfort_Mode_Setting_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_Control_Adj_Comfort_Mode_Setting_Content(uint8_t selection);
void Draw_Control_Adj_Temp_Swing_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_Control_Adj_Temp_Swing_Content(uint8_t selection);
void Draw_User_Setting_Language_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_User_Setting_Language_Choice(uint8_t selection);

// Child Lock PIN functions
void Draw_Child_Lock_Pin_Page(uint8_t stage);
void Draw_Pin_Input_Box(uint8_t* pin_buffer, uint8_t digit_index);
void Draw_Pin_Digit_Selector(uint8_t selected_digit);
void Handle_Pin_Input(uint8_t key_val);
uint8_t Verify_Pin_Code(void);
void Clear_Pin_Input(void);

// Leave Schedule Mode Confirm dialog
void Draw_Leave_Schedule_Confirm_Page(uint8_t selection);

extern uint8_t number_area_dirty;

#endif




