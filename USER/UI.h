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
#define   NO_Power_Board													(0)
// External (floor) sensor hardware, 2026-11-11:
//   0 = old board: NTC on top, 470R series + 4.7K pull-down, 10K NTC only (External Sensor Type menu is ignored)
//   1 = new board: R13 33K pull-up to 3V3, NTC to GND, R14 1K + C15 100nF RC filter to PB1;
//       10K/12K/15K/50K/100K (all B3950) selected by g_parameter.ext_sensor_type
#define   MULTI_EXT_SENSOR												(1)

// Bottom-right buttons (Save / Next / Reset) are right-aligned: the text ends at
// BTN_RIGHT_X and the start x follows the string length of each language.
// 118 = end of English "Save" at the old fixed x=86, so English stays pixel-identical. (2026-11-06)
#define   BTN_RIGHT_X														(118)
#define   TOPBAR_EDIT_X														(107)	// Edit Icon x (16x16 at y=5)

// Language page layout (2026-11-06): LANG_MAX rows, item_selection 1..LANG_MAX = languages
#define   LANG_ROW_Y0														(28)
#define   LANG_ROW_H														(18)
#define   LANG_PAGE_SAVE_SEL												(LANG_MAX + 1)


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
		unsigned char  operation_mode;		// 0=Manual Mode, 1=Schedule Mode, 2=PWM Mode
		unsigned char  sensor_type;				// 0=Room, 1=Floor
		unsigned char  power_limit;
		unsigned char  power_limit_switch;
		//unsigned char  pwm_duty_cycle;
		//unsigned char  pwm_duration;
		unsigned char  temp_diff;
		unsigned char  comfort_mode;				// reserved: Comfort Mode removed (Flash layout kept)
		unsigned char  floor_material;				// reserved: Comfort Mode removed (Flash layout kept)
		unsigned char  rtc_set_flag;
		unsigned char  language;
		unsigned char  workday_schedule[6][4];
		unsigned char  holiday_schedule[6][4];
		rtc_time_t 		 backup_rtc;
		unsigned char  idle_comp_count;
		unsigned char  heatting_comp_count;
		unsigned char  font_size;								// 0=8x16, 1=12x24
		unsigned char  astro_day_hour;
		unsigned char  astro_day_min;
		unsigned char  astro_night_hour;
		unsigned char  astro_night_min;
		unsigned char  pwm_day_duty;
		unsigned char  pwm_night_duty;
		unsigned char  adaptive_start;
		float  r_mild;       // Adaptive Start rate-of-rise for mild zone (>= 16C)
		float  r_cold;       // Adaptive Start rate-of-rise for cold zone (< 16C)
		int    astro_tolerance_min;   // PWM Astro Time tolerance, -120..+120 minutes, default 0
		unsigned char  location;      // PWM Astro Time location: 0=Oslo, 1=Stockholm, 2=Helsinki, 3=Copenhagen, 4=Berlin (default 0)
		unsigned char  ext_sensor_type;	// External sensor resistance: EXT_SENSOR_10K..EXT_SENSOR_100K (default 10K). 2026-11-06: uses the struct tail padding after location, sizeof(g_parameter_t) unchanged

 }g_parameter_t;

// PWM Astro Time location choices (g_parameter.location)
// 2026-11-10: Copenhagen/Berlin APPENDED so values saved in Flash (0/1/2) keep their
// meaning. The Location page shows them in language order (NO/SV/DA/FI/DE capitals)
// through Astro_Location_To_Row()/Astro_Row_To_Location() in Control_UI.c.
enum
{
    ASTRO_LOCATION_OSLO = 0,
    ASTRO_LOCATION_STOCKHOLM,
    ASTRO_LOCATION_HELSINKI,
    ASTRO_LOCATION_COPENHAGEN,
    ASTRO_LOCATION_BERLIN,
    ASTRO_LOCATION_MAX
};

// External sensor resistance choices (g_parameter.ext_sensor_type), 2026-11-06
// UI only for now - ADC.c still converts with the 10K table for every choice
enum
{
    EXT_SENSOR_10K = 0,
    EXT_SENSOR_12K,
    EXT_SENSOR_15K,
    EXT_SENSOR_50K,
    EXT_SENSOR_100K,
    EXT_SENSOR_TYPE_MAX
};

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
extern uint8_t set_time_from_boot;
extern uint8_t leave_icon_color;
extern uint8_t edit_icon_color;
//extern uint8_t schedule_settings[6][4];
//extern uint8_t schedule_settings_restday[6][4];
extern uint8_t schedule_time_edit_hour;        // (0-23)
extern uint8_t schedule_time_edit_min;        // (0-59)
extern uint8_t schedule_time_edit_temp;       // (0-45)
extern uint8_t schedule_time_on_off;           //  0=OFF, 1=ON
extern uint8_t control_adj_menu_scroll;				// 0=show row0-3, 1=show row1-4
extern uint8_t func_setting_scroll;					// Scroll offset for Function Setting page
extern uint8_t current_sensor_type;          // 0=Room, 1=Floor (default Room)
extern uint8_t current_ext_sensor_type;      // External sensor resistance being edited (EXT_SENSOR_xxx)
#if (MULTI_EXT_SENSOR)
extern volatile uint8_t  ext_sensor_updating;		// 1 = "Sensor Updating" shown, waiting for the first reading of the new type
extern volatile uint32_t ext_sensor_update_tick;	// time_tick when the update started (30 s safety timeout)
#define EXT_SENSOR_UPDATE_TIMEOUT	(30000)			// ms
void Ext_Sensor_Update_Finish(void);
#endif
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
extern float window_fun_buffer_a[16];
extern uint8_t window_fun_buffer_a_count;
extern uint8_t window_fun_b_divider;
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
extern uint8_t pwm_duty_cycle;
extern uint8_t pwm_duration;
extern uint8_t pwm_setting_selection;
extern uint8_t pwm_astro_time_selection;	// 0=Location, 1=Tolerance (STATE_CONTROL_ADJ_PWM_ASTRO_TIME option select)
extern uint8_t temp_location;						// Temp edit buffer for the Location page: DISPLAY ROW 0..ASTRO_LOCATION_MAX-1 (not the enum value)
extern int	 temp_astro_tolerance_min;		// Temp edit buffer for g_parameter.astro_tolerance_min
extern uint8_t temp_pwm_day_duty;
extern uint8_t temp_pwm_night_duty;
extern float temp_swing;
extern uint8_t child_lock_flag;

// Child Lock PIN code variables
extern uint8_t pin_code_input[4];      // Current PIN input buffer
extern uint8_t pin_code_confirm[4];    // PIN confirmation buffer
extern uint8_t pin_digit_index;        // Current digit index (0-3)
extern uint8_t pin_setup_stage;        // 0=enter PIN, 1=confirm PIN
extern uint8_t pin_digit_selected;     // Currently selected digit (0-9) for input
extern const char* lan_str;
extern uint8_t language_temp;
extern uint8_t adaptive_start;

extern float	disp_fack_temp;

void Draw_Active_Menu(void);
void Draw_Static_Icons(void);
void UI_Update(void);
void Draw_TopBar(uint8_t leave_col, uint8_t edit_col);
void Draw_TopBar_Leave(uint8_t leave_col);  // Intermediate menu pages: Leave Icon only
void Show_TopBar_Title(uint16_t x, char *str);	// page title at y=5; long title covers the Edit Icon (2026-11-06)
void Show_FuncSetting_Row_Text(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, char *str, uint8_t size);
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
void Draw_Control_Adj_Ext_Sensor_Type_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_Control_Adj_Ext_Sensor_Type_Choices(uint8_t selection);
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
void Draw_Control_Adj_PWM_Duty_Cycle_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_Control_Adj_PWM_Duty_Cycle_Content(uint8_t selection);
void Draw_Control_Adj_Power_Limit_Content(uint8_t selection);
void Draw_Control_Adj_Temp_Swing_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_Control_Adj_Temp_Swing_Content(uint8_t selection);
void Draw_User_Setting_Language_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_User_Setting_Language_Choice(uint8_t selection);
void Draw_User_Setting_Adaptive_Start_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_User_Setting_Adaptive_Start_Choice(uint8_t selection);

// PWM Setting page (Control Adj menu entry)
void Draw_Control_Adj_PWM_Setting_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_Control_Adj_PWM_Setting_Choices(uint8_t selection);
void Draw_Control_Adj_PWM_Astro_Time_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_Control_Adj_PWM_Astro_Time_Content(uint8_t selection);
void Draw_Control_Adj_PWM_Location_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_Control_Adj_PWM_Location_Choices(uint8_t selection);
uint8_t Astro_Location_To_Row(uint8_t location);
uint8_t Astro_Row_To_Location(uint8_t row);
void Draw_Control_Adj_PWM_Tolerance_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_Control_Adj_PWM_Tolerance_Content(uint8_t selection);

// PWM Operation Mode Setting
void Draw_Operation_Mode_PWM_Setting_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col);
void Draw_Operation_Mode_PWM_Setting_Content(uint8_t selection);

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




