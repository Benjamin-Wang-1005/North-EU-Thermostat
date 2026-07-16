//---------------------------------------------------------------------------------------------------------
//                                                                                                         
// Copyright(c) 2026 E-poly Technology Co., Ltd. All rights reserved.                                           
//                                                                                                         
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
// E-Poly North EU Thermostat Project 
// Author: Benjamin Wang
// Date: 2026/04/06
// Email: Benjamin@epoly-tech.com
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
//File Function: Thermostat UI Flow
//		- 128 * 160 RGB(8 bit) LCM
//		- 
//---------------------------------------------------------------------------------------------------------
#include "Thermostat.h"



// Color definitions
#define EDIT_COLOR      BLUE    // Temperature at edit state

// Timeout definitions (in 10ms units)
//#define SLEEP_TIMEOUT   1000  // 10 seconds = 1000 * 10ms
//#define EDIT_TIMEOUT    500   // 5 seconds for setting mode

// ------------------------------------  Global variables  ------------------------------------------------

g_parameter_t g_parameter;
uint8_t UI_state = STATE_ACTIVE;  // Initial UI state is Active
uint8_t icon6_red_state = 0;          // Icon6 red state flag (0=Black, 1=Red)

uint8_t leave_icon_color;							
uint8_t edit_icon_color;
uint8_t item_selection;
uint8_t	Top_Bar_Active = 0;						// Top_Bar_Active cursor stay in Leave/Edit
float	temp_int;									// Temporary variable for setting number
#if(SIMULATION)
float		disp_fack_temp = 17.5f;
#endif

const char* lan_str;

uint8_t schedule_edit_scroll = 0;     // Scroll offset: 0=show P1-P4, 1=show P2-P5, 2=show P3-P6
uint8_t control_adj_menu_scroll = 0;    // Scroll offset: 0=show row0-3, 1=show row1-4
uint8_t func_setting_scroll = 0;        // Scroll offset for Function Setting page
uint8_t user_setting_menu_scroll = 0;
uint8_t number_area_dirty = 1;          // 1=number area was cleared, need full redraw

// ------------------------------------  Global variables  ------------------------------------------------

float temp_correct_internal = -1.0f;     // Internal sensor correction value
float temp_correct_external = -0.5f;     // External sensor correction value
int	temp_limit_max = 45;						 // Setup temp limit max value
int temp_limit_min = 0;						 		// Setup temp limit min value 
int 	temp_protect_max = 60;
int		temp_protect_min = 4;
uint8_t temp_protect_max_switch = 0;
uint8_t temp_protect_min_switch = 1;
uint8_t operation_mode;
uint8_t current_sensor_type = 0;          // Sensor type: 0=Room, 1=Floor (default Room)
uint8_t power_on_state = 1;
uint8_t window_fun = 1;
uint8_t window_fun_temp = 10;
uint8_t window_fun_time = 30;
uint8_t window_fun_triggered = 0;
float window_fun_temp_buffer[60];
uint8_t window_fun_buffer_count = 0;
uint8_t window_fun_buffer_index = 0;
volatile uint8_t window_fun_int_updated = 0;
volatile uint8_t window_fun_ext_updated = 0;
uint8_t sleep_backlight_duty = 20;
uint8_t child_lock = 0;
uint8_t current_prog_type = 0;        // 0="5+2", 1="6+1", 2="7"
rtc_time_t temp_rtc;
uint8_t main_display_digi = Display_Room_Temp;
//uint8_t old_relay_state;
uint8_t power_limit;
uint8_t power_limit_switch;
uint8_t comfort_mode;
uint8_t pwm_duty_cycle = 50;  // PWM duty cycle percentage (10-90)
uint8_t pwm_duration = 1;  // PWM duration in hours (1-24)
uint8_t pwm_setting_selection = 0;  // 0=Astro Time, 1=PWM Duty

// Temp variables for editing astro time
uint8_t temp_astro_day_hour;

// Temp variables for editing PWM duty
uint8_t temp_pwm_day_duty;
uint8_t temp_pwm_night_duty;
uint8_t temp_astro_day_min;
uint8_t temp_astro_night_hour;
uint8_t temp_astro_night_min;
uint8_t floor_material;					// '0'=Wood/Laminate, '1'=Tile/Concrete, '2'=Fast Response
float temp_swing;
uint8_t child_lock_flag;
uint8_t language_temp;
uint8_t adaptive_start;

char* en_week_texts[7] = {"MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"}; 
char* no_week_texts[7] = {"MAN", "TIR", "ONS", "TOR", "FRE", "L\xD8R", "S\xD8N"};

// Child Lock PIN code variables
uint8_t pin_code_input[4] = {0, 0, 0, 0};      // Current PIN input buffer
uint8_t pin_code_confirm[4] = {0, 0, 0, 0};    // PIN confirmation buffer
uint8_t pin_digit_index = 0;                   // Current digit index (0-3)
uint8_t pin_setup_stage = 0;                   // 0=enter PIN, 1=confirm PIN
uint8_t pin_digit_selected = 0;                // Currently selected digit (0-9) for input

// ------------------------------------  Global variables  ------------------------------------------------
uint8_t schedule_time_setting_period = 1;  // Current schedule period (1-6)
// ------------------------------------  Global variables  ------------------------------------------------
// ------------------------------------  Global variables  ------------------------------------------------
uint8_t schedule_time_on_off = 1;
uint8_t leave_schedule_confirm_selection = 0;  // 0=Cancel, 1=Yes           // ON/OFF state: 0=OFF, 1=ON
uint8_t window_open_confirm_selection = 0;     // 0=Cancel Detect, 1=Reset Detect
uint8_t window_open_flash_state = 0;           // Flash color toggle for window open detected

// ------------------------------------  Global variables  ------------------------------------------------
uint8_t schedule_time_edit_hour;        // Hour (0-23)
uint8_t schedule_time_edit_min;        // Minute (0-59)
uint8_t schedule_time_edit_temp;       // Temperature (0-45)



// ------------------------------------  Global variables  ------------------------------------------------
uint8_t schedule_edit_source = 0;
	
// Menu item definitions for Control Adj Menu
const MenuItem_t Control_Adj_Menu_Items[] = {
	{STR_Sensor, STATE_CONTROL_ADJ_SENSOR, Draw_Control_Adj_Sensor_Page, NULL},
	{STR_Temperature_Swing, STATE_CONTROL_TEMP_SWING, Draw_Control_Adj_Temp_Swing_Page, NULL},
	{STR_PWM_Setting, STATE_CONTROL_ADJ_PWM_SETTING, Draw_Control_Adj_PWM_Setting_Page, NULL},
	{STR_Power_Limit, STATE_CONTROL_POWER_LIMIT, Draw_Control_Adj_Power_Limit_Page, NULL},
	{STR_Sensor_Calibrate, STATE_CONTROL_ADJ_TEMP_CORRECT, Draw_Control_Adj_TempCorrect_Page, NULL},
	{STR_Input_Temperature_Limit, STATE_CONTROL_ADJ_TEMP_LIMIT, Draw_Control_Adj_TempLimit_Page, NULL},
	{STR_Protect_Temperature, STATE_CONTROL_ADJ_TEMP_PROTECT, Draw_Control_Adj_TempProtect_Page, NULL},
	{STR_Power_On_State, STATE_CONTROL_ADJ_POWER_ON_STATE, Draw_Control_Adj_Power_On_State_Page, NULL},
};
const uint8_t Control_Adj_Menu_Item_Count = sizeof(Control_Adj_Menu_Items) / sizeof(Control_Adj_Menu_Items[0]);

const MenuItem_t Control_Adj_Menu_Items_12x24[] ={
	{STR_Sensor, STATE_CONTROL_ADJ_SENSOR, Draw_Control_Adj_Sensor_Page, NULL},
	{STR_hysteresis, STATE_CONTROL_TEMP_SWING, Draw_Control_Adj_Temp_Swing_Page, NULL},
	{STR_PWM_Setting, STATE_CONTROL_ADJ_PWM_SETTING, Draw_Control_Adj_PWM_Setting_Page, NULL},
	{STR_Power_Limit, STATE_CONTROL_POWER_LIMIT, Draw_Control_Adj_Power_Limit_Page, NULL},
	{STR_Calibrate, STATE_CONTROL_ADJ_TEMP_CORRECT, Draw_Control_Adj_TempCorrect_Page, NULL},
	{STR_Input_Limit, STATE_CONTROL_ADJ_TEMP_LIMIT, Draw_Control_Adj_TempLimit_Page, NULL},
	{STR_Protect_Temperature, STATE_CONTROL_ADJ_TEMP_PROTECT, Draw_Control_Adj_TempProtect_Page, NULL},
	{STR_Power_On_State, STATE_CONTROL_ADJ_POWER_ON_STATE, Draw_Control_Adj_Power_On_State_Page, NULL},
};

// Menu item definitions for User Setting Menu
const MenuItem_t User_Setting_Menu_Items[] = {
	{STR_Child_lock, STATE_USER_SETTING_CHILD_LOCK, Draw_User_Setting_Child_Lock_Page, NULL},
	{STR_Window_Function, STATE_USER_SETTING_WINDOW_FUN, Draw_User_Setting_Window_Fun_Page, NULL},
	{STR_Set_Time, STATE_USER_SETTING_SET_TIME, Draw_User_Setting_SetTime_Page, NULL},
	{STR_Language, STATE_USER_SETTING_LANGUAGE, Draw_User_Setting_Language_Page, NULL},
	{STR_Adaptive_Start, STATE_USER_SETTING_ADAPTIVE_START, Draw_User_Setting_Adaptive_Start_Page, NULL},
	{STR_Set_Backlight, STATE_USER_SETTING_BACKLIGHT, Draw_User_Setting_Backlight_Page, NULL},
	{STR_Factory_Reset, STATE_USER_SETTING_RESET, Draw_User_Setting_Reset_Page, NULL},
};
const uint8_t User_Setting_Menu_Item_Count = sizeof(User_Setting_Menu_Items) / sizeof(User_Setting_Menu_Items[0]);

// Find 1-based menu index by next_state
uint8_t Find_Control_Adj_Menu_Index(uint8_t state)
{
	uint8_t i;
	for(i = 0; i < Control_Adj_Menu_Item_Count; i++) {
		if(Control_Adj_Menu_Items[i].next_state == state) {
			return i + 1;
		}
	}
	return 1;
}

uint8_t Find_User_Setting_Menu_Index(uint8_t state)
{
	uint8_t i;
	lan_str = LanguageTable[STR_Sensor][g_parameter.language];
	for(i = 0; i < User_Setting_Menu_Item_Count; i++) {
		if(User_Setting_Menu_Items[i].next_state == state) {
			return i + 1;
		}
	}
	return 1;
}

// ------------------------------------  Global variables  ------------------------------------------------


void Display_Number(float number, uint16_t color, uint8_t show_decimal);
void Draw_Static_Icons(void);

//---------------------------------------------------------------------------------------------------------
//  Draw Initial Page content include
//	- RTC Time
//  - State Icon
//  - Tempreature
//  - Program/Manual Mode
//	- Setting Icon
//---------------------------------------------------------------------------------------------------------

void Draw_Active_Menu(void)
{
		float display_temp;
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;

		// Force number redraw because this function paints the full Active screen
		// (other code may have cleared the LCD with LCD_Fill without telling Display_Number)
		number_area_dirty = 1;
		
		//rtc_time.Hour = 10;				//For test
		//rtc_time.Min = 32;				//For test
		//rtc_time.Year = 26;
		//rtc_time.Mon = 01;
		//rtc_time.Date = 01;
		
		GUI_DrawBigDigit(5, 5, BLACK, WHITE, (rtc_time.Hour/10)+0x30 , 1);
		GUI_DrawBigDigit(21, 5, BLACK, WHITE, (rtc_time.Hour%10)+0x30, 1);
		// colon
		LCD_Fill(40, 15, 43, 18, BLACK);
		LCD_Fill(40, 25, 43, 28, BLACK);
		GUI_DrawBigDigit(47, 5, BLACK, WHITE, (rtc_time.Min/10)+0x30, 1);
		GUI_DrawBigDigit(63, 5, BLACK, WHITE, (rtc_time.Min%10)+0x30, 1);
		Draw_Static_Icons();
		
		// Initial display: 4.0 in BLACK (no blinking)
		if(main_display_digi == Display_Room_Temp){
				
				if(g_parameter.sensor_type == 0) {  // Room (Internal)
						display_temp = Average_INT_Temp;
						display_temp += g_parameter.temp_correct_internal;
				} else {  // Floor (External)
						display_temp = Average_EXT_Temp;
						display_temp += g_parameter.temp_correct_external;
				}
#if(SIMULATION)
				display_temp = disp_fack_temp + g_parameter.temp_correct_internal;
				if(Relay == RELAY_OFF){
						Display_Number(display_temp, BLACK, 1);  // show_decimal = 1
				}else{
						Display_Number(display_temp, RED, 1);
				}
#else
				if(display_temp > -99){
						if(Relay == RELAY_OFF){
								Display_Number(display_temp, BLACK, 1);  // show_decimal = 1
						}else{
								Display_Number(display_temp, RED, 1);
						}
				}else{
						//LCD_Fill(24, 74, 51, 78, BLACK);
						LCD_Fill(23, 87, 53, 91, BLACK);
						//LCD_Fill(60, 74, 91, 78, BLACK);
						LCD_Fill(57, 87, 87, 91, BLACK);
				}
#endif
				// Celsius Icon at (100, 45)
				//GUI_DrawMonoIcon24x24(100, 45, BLACK, WHITE, Celsius_Icon_24x24);
		}else if(main_display_digi == Display_Setting_Temp){
				Display_Number(g_parameter.setting_number, BLUE, 0);
				// Celsius Icon at (100, 45)
				//GUI_DrawMonoIcon24x24(100, 45, BLACK, WHITE, Celsius_Icon_24x24);
		}
		
		
	
}



/*****************************************************************************
 * @name       :void Draw_Static_Icons(void)
 * @date       :2025-04-01 (Updated for 24x24 monochrome icons)
 * @function   :Draw static icons on the screen using 24x24 monochrome bitmaps
 * @parameters :None
 * @retvalue   :None
 * @note       :Icon mapping:
 *              - Icon1 (24x24) at (120, 5)
 *              - Icon2 (24x24) at (120, 5) - same position, different icon
 *              - Icon3 (24x24) at (144, 5) - rightmost
 *              - Icon8 (24x24) at (144, 45)
 *              - Icon6 (24x24) at (144, 85)
 *              Using 24x24 instead of 32x32 to avoid overlapping with time display
******************************************************************************/

void Draw_Static_Icons(void)
{
	//uint8_t display_num = 0;
	//uint8_t	x_pos;
	const uint8_t *icons[6] = {P1_Icon_24x24, P2_Icon_24x24, P3_Icon_24x24, P4_Icon_24x24, P5_Icon_24x24, P6_Icon_24x24};
	// Icons on the RIGHT side - now using 24x24 monochrome icons
	// Smaller size (24x24) to avoid overlapping with time display at left
	// Screen width is 160px, icons positioned to stay within bounds

	// Clear entire top icon bar to avoid ghost icons when Relay state or display_num changes
	//LCD_Fill(84, 5, 160, 29, WHITE);
	LCD_Fill(80, 5, 128, 29, WHITE);

	// Icon3 at rightmost (136, 5) - rightmost, RED foreground (136+24=160)
	
	// Icon2 at (112, 5) - to the left of Icon3
	
	
	// Icon1 at (88, 5) - to the left of Icon2
	
	
	// Celsius Icon at (100, 45)
	GUI_DrawMonoIcon24x24(93, 53, BLACK, WHITE, Celsius_Icon_24x24);
	
	// Clear entire top icon bar to avoid ghost icons when Relay state or display_num changes
	LCD_Fill(4, 130, 128, 160, WHITE);
	// Icon8 at (136, 45) - below Icon3
	if(g_parameter.operation_mode == 0){
			GUI_DrawMonoIcon24x24(28, 131, BLACK, WHITE, M_Icon_24x24);
	}else if(g_parameter.operation_mode == 1){
			GUI_DrawMonoIcon24x24(28, 131, BLACK, WHITE, icons[Schedule_Period]);	
	}else if(g_parameter.operation_mode == 2){
			GUI_DrawMonoIcon24x24(28, 131, BLACK, WHITE, PWM_Icon_24x24);
	}
	
	// Icon6 at (136, 85) - below Icon8
	
	if(icon6_red_state){
			GUI_DrawMonoIcon24x24(4, 131, RED, WHITE, Menu_Icon_24x24);
	} else {
			GUI_DrawMonoIcon24x24(4, 131, BLACK, WHITE, Menu_Icon_24x24);
	}
	
	if(Relay == RELAY_ON){
			GUI_DrawMonoIcon24x24(52, 131, RED, WHITE, Heating_Icon_24x24);
			//display_num++;
	}
	
	if(g_parameter.child_lock == 1){
			//x_pos = 99 - (display_num * 24);
			GUI_DrawMonoIcon24x24(76, 131, BLACK, WHITE, Lock_Icon_24x24);
			//display_num++;
	}
	
	if(g_parameter.window_fun == 1){
			//x_pos = 99 - (display_num *24);
			GUI_DrawMonoIcon24x24(100, 131, BLACK, WHITE, Window_Icon_24x24);
	}
	
	//Draw weekday
	//LCD_Fill(136, 106, 160, 120, WHITE);
	if ((g_parameter.current_prog_type == 0 && (weekday == 5 || weekday == 6)) || (g_parameter.current_prog_type == 1 && weekday == 6)) {
			if(g_parameter.language == LANG_ENGLISH){
					Show_Str(84, 5, RED, WHITE, en_week_texts[weekday], 32, 1);
			}else if(g_parameter.language == LANG_Norwegian){
					Show_Str(84, 5, RED, WHITE, no_week_texts[weekday], 32, 1);
			}
	}else{
			if(g_parameter.language == LANG_ENGLISH){
					Show_Str(84, 5, BLUE, WHITE, en_week_texts[weekday], 32, 1);
			}else if(g_parameter.language == LANG_Norwegian){
					Show_Str(84, 5, BLUE, WHITE, no_week_texts[weekday], 32, 1);
			}
	}
	
}

// Display number on LCD
// number: value to display
// color: text color
// show_decimal: 1 = show decimal point and fraction, 0 = show integer only
void Display_Number(float number, uint16_t color, uint8_t show_decimal)
{
	//static float prev_number = -999.0f;
	static uint16_t prev_color = 0xFFFF;
	static uint8_t prev_show_decimal = 0xFF;
	static int prev_integer_part = -999;
	static int prev_decimal_part = -999;
	static uint8_t prev_has_tens = 0xFF;
	static uint8_t prev_is_negative = 0xFF;

	int integer_part;
	int decimal_part;
	uint8_t digit1, digit2;
	uint8_t has_tens = 0;
	uint8_t is_negative = 0;
	uint8_t force_redraw = 0;
	uint8_t integer_changed = 0;
	uint8_t decimal_changed = 0;
	uint8_t color_changed = 0;
	uint8_t tens_changed = 0;
	uint8_t sign_changed = 0;

	// Check if negative
	if(number < 0)
	{
		is_negative = 1;
		integer_part = (int)(-number);
	}
	else
	{
		integer_part = (int)number;
	}

	// Calculate decimal part from absolute value
	decimal_part = (int)((number >= 0 ? number : -number) * 10) % 10;

	// Ensure decimal part is 0-9
	if(decimal_part < 0) decimal_part = 0;
	if(decimal_part > 9) decimal_part = 9;

	// Calculate digits
	if(integer_part >= 10)
	{
		digit1 = integer_part / 10;  // Tens
		digit2 = integer_part % 10;  // Ones
		has_tens = 1;
	}
	else
	{
		digit1 = 0;
		digit2 = integer_part;       // Ones only
		has_tens = 0;
	}

	// Check what has changed
	if(number_area_dirty)
	{
		force_redraw = 1;
		number_area_dirty = 0;
	}
	if(color != prev_color) color_changed = 1;
	if(show_decimal != prev_show_decimal) force_redraw = 1;
	if(integer_part != prev_integer_part) integer_changed = 1;
	if(decimal_part != prev_decimal_part) decimal_changed = 1;
	if(has_tens != prev_has_tens) tens_changed = 1;
	if(is_negative != prev_is_negative) sign_changed = 1;

	// If nothing changed at all, return
	if(!force_redraw && !color_changed && !integer_changed && !decimal_changed && !tens_changed && !sign_changed)
	{
		return;
	}

	// Update integer part if needed
	if(force_redraw || color_changed || integer_changed || tens_changed || sign_changed)
	{
		// Handle sign
		if(is_negative)
		{
			if(has_tens)
				LCD_Fill(7, 83, 23, 87, color);
			else
				LCD_Fill(39, 83, 55, 87, color);
		}
		else
		{
			// Clear minus sign area
			if(prev_is_negative)
				LCD_Fill(39, 83, 55, 87, WHITE);
		}

		// Draw digits with per-digit clear-draw
		if(has_tens)
		{
			// Two digits: clear tens, draw tens, clear ones, draw ones
			LCD_Fill(23, 53, 55, 117, WHITE);   // Clear tens digit area
			GUI_DrawBigDigit(23, 53, color, WHITE, '0' + digit1, 0);  // Draw tens

			LCD_Fill(55, 53, 87, 117, WHITE);   // Clear ones digit area
			GUI_DrawBigDigit(55, 53, color, WHITE, '0' + digit2, 0);  // Draw ones
		}
		else
		{
			// Single digit - need to clear tens area if previously had tens
			if(prev_has_tens)
			{
				LCD_Fill(23, 53, 55, 117, WHITE);   // Clear tens digit area
			}
			GUI_DrawBigDigit(23, 53, color, WHITE, '0' + digit2, 0);  // Draw ones
		}
	}

	// Update decimal part if needed
	if(show_decimal && (force_redraw || color_changed || decimal_changed))
	{
		// Clear decimal digit area
		LCD_Fill(87, 85, 128, 117, WHITE);

		// Draw decimal point (circular dot) at fixed position
		//LCD_Fill(96, 94, 100, 98, color);
		LCD_Fill(89, 111, 93, 115, color);
		//LCD_Fill(95, 95, 101, 97, color);
		LCD_Fill(88, 112, 94, 114, color);
		//LCD_Fill(96, 99, 100, 99, color);
		LCD_Fill(89, 116, 93, 116, color);
		//LCD_Fill(96, 93, 100, 93, color);
		LCD_Fill(89, 110, 93, 110, color);

		// Draw decimal digit using smaller font (16x32)
		GUI_DrawBigDigit(99, 85, color, WHITE, '0' + decimal_part, 1);
	}

	// Save state
	//prev_number = number;
	prev_color = color;
	prev_show_decimal = show_decimal;
	prev_integer_part = integer_part;
	prev_decimal_part = decimal_part;
	prev_has_tens = has_tens;
	prev_is_negative = is_negative;
}


// Clear number display area
// Only clear the digit area, NOT the icons below
void Clear_Number_Area(void)
{
	// Clear entire number display area including:
	// - minus sign at x=5
	// - both digits at x=22 and x=59
	// - decimal point at x=96
	// - decimal digit at x=109
	// x: 5 to 130 (cover all number elements)
	// y: 40 to 108 (cover full digit height, avoid clearing Icon6 and Icon8 at the bottom)
	LCD_Fill(5, 38, 128, 129, WHITE);
	number_area_dirty = 1;
}


//--------------------------------------------------------------------------------------------
//																		Add by Benjamin
//--------------------------------------------------------------------------------------------



void Draw_TopBar(uint8_t leave_col, uint8_t edit_col)
{
			LCD_Fill(0, 0, 21, 21, WHITE);		//Clear Leave Icon
			LCD_Fill(107, 0, 128, 21, WHITE);	//Clear Edit Icon
			
			if(leave_col){
					GUI_DrawMonoIcon16x16(5, 5, RED, WHITE, Icon16x16_Leave);   // Leave red
			}else{
					GUI_DrawMonoIcon16x16(5, 5, BLACK, WHITE, Icon16x16_Leave);  // Leave black
			}
			
			if(edit_col){
					GUI_DrawMonoIcon16x16(107, 5, RED, WHITE, Icon16x16_Edit);  // Edit red
			}else{
					GUI_DrawMonoIcon16x16(107, 5, BLACK, WHITE, Icon16x16_Edit);  // Edit black
			}
	
}

void Update_TopBar(void)
{
		// Toggle between Edit red and Leave red
		if(edit_icon_color){
				edit_icon_color = 0;
				leave_icon_color = 1;
		}else{
				edit_icon_color = 1;
				leave_icon_color = 0;
		}
							
		// Only redraw Top Bar (much faster than full page redraw)
		Draw_TopBar(leave_icon_color, edit_icon_color);
		//delay_ms(100);  // Debounce
	
}

// Helper: display Function Setting row text in portrait mode (128px wide)
// Long strings like "Operation Mode" are split into two lines automatically
void Show_FuncSetting_Row_Text(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, char *str, uint8_t size)
{
	uint8_t len = 0;
	uint8_t i;
	uint8_t split_pos = 11;
	char buf1[20], buf2[20];

	// Calculate string length
	while(str[len] != 0) len++;

	// font_size=0: 8px per char, available = 128-30 = 98px, max 12 chars
	if(len <= 10) {
		// Short enough for one line - center vertically in 2-line space
		Show_Str(x, y + size / 2, fc, bc, str, size, 0);
		return;
	}

	// Long string: find space to split (search backward from position 11)
	for(i = 11; i > 0; i--) {
		if(str[i] == ' ') {
			split_pos = i;
			break;
		}
	}

	// No space found and string fits in one line → single line centered
	if(split_pos == 11 && len <= 12) {
		Show_Str(x, y + size / 2, fc, bc, str, size, 0);
		return;
	}

	// Copy first line (split_pos chars)
	for(i = 0; i < split_pos; i++) buf1[i] = str[i];
	buf1[split_pos] = 0;

	// Skip spaces before second line
	i = split_pos;
	while(str[i] == ' ') i++;

	// Copy second line
	{
		uint8_t j = 0;
		while(str[i] != 0) buf2[j++] = str[i++];
		buf2[j] = 0;
	}

	// Display two lines (16px font each, 32px row height)
	Show_Str(x, y, fc, bc, buf1, size, 0);
	Show_Str(x, y + size, fc, bc, buf2, size, 0);
}

// Draw a single row in Function Setting Edit mode (for optimized update)
// row: 0=Heating Schedule, 1=Control Adj, 2=User Settings
// selected: 1=selected (red bg), 0=not selected
void Draw_Function_Setting_Edit_Row(uint8_t row, uint8_t selected)
{
	uint16_t row_y[4];  // Y positions for display rows (max 4)
	uint16_t text_color, bg_color;
	uint8_t row_height;
	uint8_t visible_rows;
	uint8_t text_idx = func_setting_scroll + row;

	if(g_parameter.font_size == 0){
		row_y[0] = 27;
		row_y[1] = 59;
		row_y[2] = 91;
		row_y[3] = 123;
		row_height = 32;
		visible_rows = 4;
	}else if(g_parameter.font_size == 1){
		row_y[0] = 28;
		row_y[1] = 58;
		row_y[2] = 88;
		row_height = 30;
		visible_rows = 3;
	}

	// Bounds check
	if(text_idx >= 4) return;
	if(row >= visible_rows) return;

	if(selected) {
		text_color = WHITE;
		bg_color = RED;
		// Draw red background
		LCD_Fill(0, row_y[row], lcddev.width, row_y[row] + row_height, RED);
	} else {
		text_color = BLACK;
		bg_color = WHITE;
		// Clear background to white
		LCD_Fill(0, row_y[row], lcddev.width, row_y[row] + row_height, WHITE);
	}

	// Draw the row content based on item index
	if(text_idx == 0) {
		GUI_DrawMonoIcon16x16(6, row_y[row] + (row_height - 16) / 2, text_color, bg_color, OP_Icon_16x16);
		POINT_COLOR = text_color;
		BACK_COLOR = bg_color;
		if(g_parameter.font_size == 0){
			lan_str = LanguageTable[STR_Operation_Mode][g_parameter.language];
			Show_FuncSetting_Row_Text(30, row_y[row], text_color, bg_color, (char*)lan_str, 16);
		}else if(g_parameter.font_size == 1){
			lan_str = LanguageTable[STR_OP_Mode][g_parameter.language];
			Show_Str(30, row_y[row] + (row_height - 24) / 2, text_color, bg_color, (char*)lan_str, 24, 1);
		}
	} else if(text_idx == 1) {
		GUI_DrawMonoIcon16x16(6, row_y[row] + (row_height - 16) / 2, text_color, bg_color, Schedule_Icon_16x16);
		POINT_COLOR = text_color;
		BACK_COLOR = bg_color;
		if(g_parameter.font_size == 0){
			lan_str = LanguageTable[STR_Heating_Schedule][g_parameter.language];
			Show_FuncSetting_Row_Text(30, row_y[row], text_color, bg_color, (char*)lan_str, 16);
		}else if(g_parameter.font_size == 1){
			lan_str = LanguageTable[STR_Schedule][g_parameter.language];
			Show_Str(30, row_y[row] + (row_height - 24) / 2, text_color, bg_color, (char*)lan_str, 24, 1);
		}
	} else if(text_idx == 2) {
		GUI_DrawMonoIcon16x16(6, row_y[row] + (row_height - 16) / 2, text_color, bg_color, Control_Icon_16x16);
		POINT_COLOR = text_color;
		BACK_COLOR = bg_color;
		if(g_parameter.font_size == 0){
			lan_str = LanguageTable[STR_Control_Adjustment][g_parameter.language];
			Show_FuncSetting_Row_Text(30, row_y[row], text_color, bg_color, (char*)lan_str, 16);
		}else if(g_parameter.font_size == 1){
			lan_str = LanguageTable[STR_Adjust][g_parameter.language];
			Show_Str(30, row_y[row] + (row_height - 24) / 2, text_color, bg_color, (char*)lan_str, 24, 1);
		}
	} else if(text_idx == 3) {
		GUI_DrawMonoIcon16x16(6, row_y[row] + (row_height - 16) / 2, text_color, bg_color, User_Icon_16x16);
		POINT_COLOR = text_color;
		BACK_COLOR = bg_color;
		if(g_parameter.font_size == 0){
			lan_str = LanguageTable[STR_User_Settings][g_parameter.language];
			Show_FuncSetting_Row_Text(30, row_y[row], text_color, bg_color, (char*)lan_str, 16);
		}else if(g_parameter.font_size == 1){
			lan_str = LanguageTable[STR_Setting][g_parameter.language];
			Show_Str(30, row_y[row] + (row_height - 24) / 2, text_color, bg_color, (char*)lan_str, 24, 1);
		}
	}
}

// Draw Function Setting Edit mode page
// This page shows:
// - Top Bar
// - Row 1: Icon16 + "Heating Schedule" (selected=white on red, normal=black)
// - Row 2: Icon17 + "Control Adj." (selected=white on red, normal=black)
// - Row 3: Icon18 + "User Settings" (selected=white on red, normal=black)
// selection: 0=Heating Schedule, 1=Control Adj, 2=User Settings
void Draw_Function_Setting_Page(uint8_t selection, uint8_t Leave_col, uint8_t Edit_col)
{
	uint8_t i;
	uint8_t num_visible_rows;
	uint8_t visible_rows;
	uint8_t max_scroll;

	if(g_parameter.font_size == 0){
		visible_rows = 4;
	}else if(g_parameter.font_size == 1){
		visible_rows = 3;
	}
	max_scroll = (4 > visible_rows) ? (4 - visible_rows) : 0;

	// Auto-adjust scroll so that selected item is visible
	if(!Top_Bar_Active && selection > 0) {
		if(selection > func_setting_scroll + visible_rows) {
			func_setting_scroll = selection - visible_rows;
		}
		if(selection - 1 < func_setting_scroll) {
			func_setting_scroll = selection - 1;
		}
	}

	// Ensure scroll is valid
	if(func_setting_scroll > max_scroll) func_setting_scroll = max_scroll;

	// Clear screen
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);

	// Draw Top Bar
	Draw_TopBar(Leave_col, Edit_col);

	// Draw horizontal separator line below TopBar
	LCD_Fill(4, 24, lcddev.width - 4, 25, BLACK);

	// Calculate how many rows to draw (visible_rows or less if near end)
	num_visible_rows = 4 - func_setting_scroll;
	if(num_visible_rows > visible_rows) num_visible_rows = visible_rows;

	// Draw rows based on scroll position
	for(i = 0; i < num_visible_rows; i++) {
		uint8_t option_idx = func_setting_scroll + i;
		uint8_t is_selected = 0;

		if(!Top_Bar_Active && (option_idx + 1 == selection)) {
			is_selected = 1;
		}
		Draw_Function_Setting_Edit_Row(i, is_selected);
	}
}

void Draw_Opeaation_Mode_Choices(uint8_t selection)
{
		uint16_t row_y[] = {48, 78, 108};  // Y for Manual, Schedule, PWM
	uint8_t row_height = 26;
	uint8_t i;
	
	for(i = 0; i < 3; i++) {
		LCD_Fill(0, row_y[i], lcddev.width, row_y[i] + row_height, WHITE);
		
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		if(i == 0) {
			Show_FuncSetting_Row_Text(34, row_y[i], BLACK, WHITE, (char*)LanguageTable[STR_Manual_Mode][g_parameter.language], 16);
		} else if(i == 1) {
			Show_FuncSetting_Row_Text(34, row_y[i], BLACK, WHITE, (char*)LanguageTable[STR_Schedule_Mode][g_parameter.language], 16);
		} else {
			Show_FuncSetting_Row_Text(34, row_y[i], BLACK, WHITE, (char*)LanguageTable[STR_PWM_Mode][g_parameter.language], 16);
		}
		
		// Arrow icon: centered for multi-line, aligned to top for single-line
		uint8_t arrow_y = (i == 2) ? row_y[i] : row_y[i] + (row_height - 16) / 2;
		if((selection == 0) || (selection == 4)){
				if(operation_mode == i){
						GUI_DrawMonoIcon16x16(12, arrow_y, WHITE, BLACK, Icon16x16_Arrow);
				}else{
						LCD_Fill(12, arrow_y, 28, arrow_y + 16, WHITE);
				}
		}else{
				if((i+1) == selection) {
					GUI_DrawMonoIcon16x16(12, arrow_y, WHITE, RED, Icon16x16_Arrow);
				}else{
						LCD_Fill(12, arrow_y, 28, arrow_y + 16, WHITE);
				}
		}
		
	}
}

void Draw_Operation_Mode_Menu_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
		Draw_TopBar(leave_col, edit_col);
		
	
		// Draw "Sensor" title closer to Top Bar (left aligned, y=24 - moved up)
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		//lan_str = LanguageTable[STR_Operation_Mode][g_parameter.language];
		//Show_Str(10, 24, BLACK, WHITE, "Operation Mode", 16, 0);
		Show_Str(10, 24, BLACK, WHITE, (char*)LanguageTable[STR_OP_Mode][g_parameter.language], 16, 0);
	
		// Draw horizontal line below "Sensor" title (y=42 to y=43, leave 4px margin on both sides)
		LCD_Fill(4, 42, lcddev.width - 4, 43, BLACK);
	
		Draw_Opeaation_Mode_Choices(selection);
		//lan_str = LanguageTable[STR_Save][g_parameter.language];
		if(selection == 4){
				//Show_Str(127, 108, RED, WHITE, "Save", 16, 0);
				Show_Str(86, 142, RED, WHITE, (char*)LanguageTable[STR_Save][g_parameter.language], 16, 0);
		}else{
				//Show_Str(127, 108, BLACK, WHITE, "Save", 16, 0);
				Show_Str(86, 142, BLACK, WHITE, (char*)LanguageTable[STR_Save][g_parameter.language], 16, 0);
		}
}

void Process_Relay_Update(void)
{
		if(adc_ctrl.int_sensor_error || adc_ctrl.ext_sensor_error){
			
		}else{
				if(UI_state == STATE_ACTIVE){
						update_alarm(Active_Alarm);
						Draw_Active_Menu();
				}else if(UI_state == STATE_SLEEP){
						Draw_Active_Menu();
				}
		}
		Update_Relay = eFALSE;
		if(Relay == RELAY_ON){
				LOGD("Relay ON\r\n");
		}else{
				LOGD("Relay OFF\r\n");
		}
}

//---------------------------------------------------------------------------------------------------------
// Funcation: UI_Update
// Description: LCM UI main flow
// Input: 
// Output:
// Date: 2026/04/06
// Update:
//---------------------------------------------------------------------------------------------------------


void UI_Update(void)
{

	if(Update_Relay == eTRUE) Process_Relay_Update();
	

	if(key.key_active == 0){
			return;
	}
	
	if(adc_ctrl.int_sensor_error || adc_ctrl.ext_sensor_error){
			if((UI_state == STATE_ACTIVE) || (UI_state == STATE_SLEEP)){
					key.key_val = NONKEY;
					key.key_active = 0;
					if(key.key_press != DOUBLEKEY){
							key.key_press = 0;
					}
					return;
			}
	}

	if(UI_state == STATE_ACTIVE)
	{
			// Active state: handle key presses
			if(key.key_val == UPKEY || key.key_val == DOWNKEY)  // Up or Down key pressed
			{
				// Do not allow set-temperature mode until the selected sensor has a valid reading.
				// Before then the Active page displays "--" and Average_*_Temp remains below -99.
				if(((g_parameter.sensor_type == 0) && (Average_INT_Temp <= -99.0f)) ||
					 ((g_parameter.sensor_type != 0) && (Average_EXT_Temp <= -99.0f)))
				{
					// Ignore Up/Down while the temperature is not ready.
				}
				else
				{
				// If Manu Icon is red, change it back to black first
				if(icon6_red_state)
				{
					icon6_red_state = 0;
					GUI_DrawMonoIcon24x24(4, 131, BLACK, WHITE, Menu_Icon_24x24);				// Draw black Manu Icon6 
				}
				
				// Check if in Schedule Mode - if so, show confirmation dialog
				if(g_parameter.operation_mode == 1){
					// Schedule Mode - show leave confirmation dialog
					UI_state = STATE_LEAVE_SCHEDULE_CONFIRM;
					leave_schedule_confirm_selection = 0;  // Default to Cancel
					delete_alarm(Active_Alarm);
					if(register_alarm(Setting_Menu_Alarm, ACTIVE_ALIVE_TIME) == eFALSE){
							LOGE("Alarm Full!\r\n");
					}
					Draw_Leave_Schedule_Confirm_Page(leave_schedule_confirm_selection);
				}else{
					// Manual Mode - directly enter Setting state
					UI_state = STATE_SETTING;
					temp_int = g_parameter.setting_number;  // Initialize temp_int when entering setting mode
					delete_alarm(Active_Alarm);
					if(register_alarm(Setting_Digi_Alarm, DIGI_UPDATE_TIME) == eFALSE){
							LOGE("Alarm Full!\r\n");
					}
					main_display_digi = Display_Setting_Temp;
					// Clear and display in BLUE without decimal
					Clear_Number_Area();
					Display_Number(g_parameter.setting_number, EDIT_COLOR, 0);  // show_decimal = 0, BLUE color
					// Celsius Icon at (100, 45)
					GUI_DrawMonoIcon24x24(93, 53, BLACK, WHITE, Celsius_Icon_24x24);
				}
				
				//delay_ms(100);  // Debounce
				}
			}
			else if(key.key_val == ENTERKEY)  // Enter key pressed
			{
				if(icon6_red_state)
				{
					// Manu Icon is red -> enter Function Setting state
					UI_state = STATE_FUNC_SETTING;
					delete_alarm(Active_Alarm);
					if(register_alarm(Setting_Menu_Alarm, SETTING_UPDATE_TIME) == eFALSE){
							LOGE("Alarm Full!\r\n");
					}
					
					// Draw Function Setting page
					leave_icon_color = 0;
					edit_icon_color = 1;
					item_selection = 0;
					func_setting_scroll = 0;  // Reset scroll
					Draw_Function_Setting_Page(item_selection, leave_icon_color, edit_icon_color);		
					Top_Bar_Active = 1;
					
				}
				else
				{
					// Manu Icon is black -> toggle to red
					icon6_red_state = 1;							//	red state
					GUI_DrawMonoIcon24x24(4, 131, RED, WHITE, Menu_Icon_24x24);;  // Draw red Manu Icon
					//update_alarm(Active_Alarm);
				}

			}
			
	}
	else if(UI_state == STATE_SLEEP)
	{
			// Sleep state: wait for any key press to wake up
			if((key.key_val != NONKEY) && (key.key_val != SYS_RESET))  // Any key pressed
			{
				if(g_parameter.child_lock == 1){
						child_lock_flag = 1;
						// Child lock enabled - enter PIN verification
						UI_state = STATE_CHILD_LOCK_PIN_VERIFY;
						pin_digit_index = 0;
						pin_digit_selected = 0;
						Clear_Pin_Input();
						Backlight_SetDuty(BACKLIGHT_DUTY_ACTIVE);
						if(register_alarm(Setting_Menu_Alarm, ACTIVE_ALIVE_TIME) == eFALSE){
								LOGE("Alarm Full!\r\n");
						}
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						POINT_COLOR = BLACK;
						BACK_COLOR = WHITE;
						//lan_str = LanguageTable[STR_Enter_Pin_Code][g_parameter.language];
						if(g_parameter.language == LANG_ENGLISH){
								Show_Str(10, 10, BLACK, WHITE, "Enter Pin Code", 16, 0);
						}else if(g_parameter.language == LANG_Norwegian){
								Show_Str(10, 10, BLACK, WHITE, "Tast inn", 16, 0);
								Show_Str(10, 26, BLACK, WHITE, "PIN-kode", 16, 0);
						}
						Draw_Pin_Input_Box(pin_code_input, pin_digit_index);
						Draw_Pin_Digit_Selector(pin_digit_selected);
				}else{
						// Child lock disabled - wake up normally
						UI_state = STATE_ACTIVE;
						if(register_alarm(Active_Alarm, ACTIVE_ALIVE_TIME) == eFALSE){
								LOGE("Alarm Full!\r\n");
						}
						// Set backlight back to 90%
						Backlight_SetDuty(BACKLIGHT_DUTY_ACTIVE);
				}

			}else if(key.key_val == SYS_RESET){
						UI_state = STATE_USER_SETTING_RESET;
						Top_Bar_Active = 1;
						leave_icon_color = 0;
						edit_icon_color = 1;
						item_selection = 0;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_User_Setting_Reset_Page(item_selection, leave_icon_color, edit_icon_color);
						Backlight_SetDuty(BACKLIGHT_DUTY_ACTIVE);
						if(register_alarm(Setting_Menu_Alarm, ACTIVE_ALIVE_TIME) == eFALSE){
							LOGE("Alarm Full!\r\n");
					}
				
			}
	}
	else if(UI_state == STATE_SETTING)			//Setting Thermostat active Temperature
	{
			// Handle key input (temp_int is initialized when entering this state)
			if(key.key_val == UPKEY)  // Up key pressed
			{
				if(temp_int < g_parameter.temp_limit_max){			//Max input value is temp_linit_max
						temp_int += 1;
				}
				// Redisplay in BLUE
				LOGD("Disp:%.1f\r\n", temp_int);
				Clear_Number_Area();
				Display_Number(temp_int, EDIT_COLOR, 0);
				// Celsius Icon at (100, 45)
				GUI_DrawMonoIcon24x24(93, 53, BLACK, WHITE, Celsius_Icon_24x24);

				//update_alarm(Setting_Digi_Alarm);
			}
			else if(key.key_val == DOWNKEY)  // Down key pressed
			{
				if(temp_int > g_parameter.temp_limit_min){
						temp_int -= 1;
				}
				// Redisplay in BLUE
				LOGD("Disp:%.1f\r\n", temp_int);
				Clear_Number_Area();
				Display_Number(temp_int, EDIT_COLOR, 0);
				// Celsius Icon at (100, 45)
				GUI_DrawMonoIcon24x24(93, 53, BLACK, WHITE, Celsius_Icon_24x24);

				//update_alarm(Setting_Digi_Alarm);
			}
			else if(key.key_val == ENTERKEY)  // Enter key pressed -> exit Setting immediately
			{
				// Exit Setting state immediately, return to Active
				UI_state = STATE_ACTIVE;
				g_parameter.setting_number = temp_int;
				delete_alarm(Setting_Digi_Alarm);
				// Display final number with decimal in BLACK
				main_display_digi = Display_Room_Temp;
				Backlight_SetDuty(BACKLIGHT_DUTY_ACTIVE);
				if(register_alarm(Active_Alarm, ACTIVE_ALIVE_TIME) == eFALSE){
						LOGE("Alarm Fail\r\n");
				}
				Draw_Active_Menu();
				//Clear_Number_Area();
				//Display_Number(g_parameter.setting_number, BLACK, 1);  // show_decimal = 1
				// Celsius Icon at (100, 45)
				//GUI_DrawMonoIcon24x24(100, 45, BLACK, WHITE, Celsius_Icon_24x24);
				if(Flash_Save_Parameter() != FLASH_OK){
						LOGE("Flash write fail!\r\n");
				}
				
				//if(register_alarm(Active_Alarm, ACTIVE_ALIVE_TIME) == eFALSE){
				//		LOGE("Alarm Full!\r\n");
				//}
			}

	}
	else if(UI_state == STATE_FUNC_SETTING)
	{
		
			if(Top_Bar_Active){
					if(key.key_val == UPKEY || key.key_val == DOWNKEY)  // Up or Down key pressed -> toggle Icon colors
					{
							Update_TopBar();
					}
					else if(key.key_val == ENTERKEY)  // Enter key pressed
					{
							if(edit_icon_color)
							{
									// Edit Icon is red -> enter Function Setting 
									item_selection = 1;  // Start with item selected showing
									func_setting_scroll = 0;  // Reset scroll
									Top_Bar_Active = 0;
									leave_icon_color = 0;
									edit_icon_color = 0;
									Draw_Function_Setting_Page(item_selection, leave_icon_color, edit_icon_color);
							}
							else
							{
									// Leave Icon is red -> return to Active state
									UI_state = STATE_ACTIVE;
									Top_Bar_Active = 0;
									icon6_red_state = 0;
									main_display_digi = Display_Room_Temp;
									LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
									delete_alarm(Setting_Menu_Alarm);
									Backlight_SetDuty(BACKLIGHT_DUTY_ACTIVE);
									if(register_alarm(Active_Alarm, ACTIVE_ALIVE_TIME) == eFALSE){
											LOGE("Alarm Fail\r\n");
									}
									Draw_Active_Menu();
									//if(register_alarm(Active_Alarm, ACTIVE_ALIVE_TIME) == eFALSE){
									//		LOGE("Alarm Full!\r\n");
									//}
							}
							//delay_ms(100);  // Debounce
					}
			}else{				
					// Function Setting Edit mode navigation
					if(key.key_val == DOWNKEY)  // Down key pressed
					{
							if(item_selection < 4)
							{
									uint8_t old_sel = item_selection;
									uint8_t old_scroll = func_setting_scroll;
									item_selection++;
									
									if(g_parameter.font_size == 0){
										if(item_selection > func_setting_scroll + 4) {
											func_setting_scroll = item_selection - 4;
										}
										if(func_setting_scroll > 0) func_setting_scroll = 0;
									}else if(g_parameter.font_size == 1){
										if(item_selection > func_setting_scroll + 3) {
											func_setting_scroll = item_selection - 3;
										}
										if(func_setting_scroll > 1) func_setting_scroll = 1;
									}
									
									// If scroll changed, redraw entire page
									if(old_scroll != func_setting_scroll) {
										Draw_Function_Setting_Page(item_selection, leave_icon_color, edit_icon_color);
									} else {
										Draw_Function_Setting_Edit_Row((old_sel - 1) - func_setting_scroll, 0);
										Draw_Function_Setting_Edit_Row((item_selection - 1) - func_setting_scroll, 1);
									}
							}
					}
					else if(key.key_val == UPKEY)  // Up key pressed
					{
							if(item_selection > 1)
							{	
									uint8_t old_sel = item_selection;
									uint8_t old_scroll = func_setting_scroll;
									item_selection--;
									
									// Check if scroll position needs to change
									if(item_selection - 1 < func_setting_scroll) {
										func_setting_scroll = item_selection - 1;
									}
									if(g_parameter.font_size == 0){
										if(func_setting_scroll > 0) func_setting_scroll = 0;
									}else if(g_parameter.font_size == 1){
										if(func_setting_scroll > 1) func_setting_scroll = 1;
									}
									
									// If scroll changed, redraw entire page
									if(old_scroll != func_setting_scroll) {
										Draw_Function_Setting_Page(item_selection, leave_icon_color, edit_icon_color);
									} else {
										Draw_Function_Setting_Edit_Row((old_sel - 1) - func_setting_scroll, 0);
										Draw_Function_Setting_Edit_Row((item_selection - 1) - func_setting_scroll, 1);
									}
							}
							else
							{
									// At top, go back to Top Bar Active
									Top_Bar_Active = 1;
									leave_icon_color = 1;
									edit_icon_color = 0;
									Draw_TopBar(leave_icon_color, edit_icon_color); 		 // ReDraw Top Bar
									item_selection = 0;  
									func_setting_scroll = 0;  // Reset scroll when returning to TopBar
									Draw_Function_Setting_Edit_Row(item_selection, 0);		// Clear item 1 backfround
							}
							
					}
					else if(key.key_val == ENTERKEY)  // Enter key pressed
					{
							if(item_selection == 1)			// "Operation Mode" selected
								{
										//Enter Operation Mode
										UI_state = STATE_OPERATION_MODE_MENU;
										Top_Bar_Active = 1;
										item_selection = 0;
										leave_icon_color = 0;
										edit_icon_color = 1;
										operation_mode = g_parameter.operation_mode;
										LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
										Draw_Operation_Mode_Menu_Page(item_selection,leave_icon_color, edit_icon_color);
								}
								else if(item_selection == 2) // "Heating Schedule" selected
								{
										// Enter Heating Schedule Menu page
										UI_state = STATE_HEATING_SCHEDULE_MENU;
										Top_Bar_Active = 1;
										item_selection = 0;
										leave_icon_color = 0;
										edit_icon_color = 1;
										LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
										Draw_Heating_Schedule_Menu_Page(item_selection, leave_icon_color, edit_icon_color);  
								}
								else if(item_selection == 3) // "Control Adj." selected
								{
										// Enter Control Adj Menu page
										UI_state = STATE_CONTROL_ADJ_MENU;
										Top_Bar_Active =1;
										item_selection = 0;
										leave_icon_color = 0;
										edit_icon_color = 1;
										control_adj_menu_scroll = 0;     // Reset scroll
										LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
										Draw_Control_Adj_Menu_Page(item_selection, leave_icon_color, edit_icon_color);  
								}
								else if(item_selection == 4)		// "User Setting" selected
								{
										//Enter User Setting 
										UI_state = STATE_USER_SETTING_MENU;
										Top_Bar_Active = 1;
										item_selection = 0;
										leave_icon_color = 0;
										edit_icon_color = 1;
										LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
										Draw_User_Setting_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
									
								}
						}
	}
	}
	else if(UI_state == STATE_OPERATION_MODE_MENU)
	{
			if(Top_Bar_Active)  // TopBar mode: Edit/Leave
			{
				if(key.key_val == UPKEY || key.key_val == DOWNKEY)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key.key_val == ENTERKEY)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter menu scroll, Edit/Leave both black, highlight current mode
						Top_Bar_Active = 0;
						item_selection = operation_mode + 1;
						leave_icon_color = 0;
						edit_icon_color = 0;
						Draw_Operation_Mode_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					else  // Leave Icon is red
					{
						// Go back to Function Setting Edit page
						UI_state = STATE_FUNC_SETTING;
						func_setting_scroll = 0;  // Reset scroll
						Top_Bar_Active = 0;
						item_selection = 1;
						leave_icon_color = 0;
						edit_icon_color = 0;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Function_Setting_Page(item_selection, leave_icon_color, edit_icon_color);
					}
				}
			}
			else if((item_selection >= 1) && (item_selection <= 3))
				{
						if(key.key_val == DOWNKEY){		//Down key
								if(item_selection < 3){
										item_selection++;
										Draw_Opeaation_Mode_Choices(item_selection);
								}
						}else if(key.key_val == UPKEY){		//Up key
								if(item_selection > 1){
										item_selection--;
										Draw_Opeaation_Mode_Choices(item_selection);
								}else if(item_selection == 1){	//go back Top_Bar
										Top_Bar_Active = 1;
										item_selection = 0;
										leave_icon_color = 1;
										edit_icon_color = 0;
										Draw_Operation_Mode_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
								}
								
						}else if(key.key_val == ENTERKEY){		//Enter key
								if(item_selection == 1){
										operation_mode = 0;
										item_selection = 4;
										Draw_Operation_Mode_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
								}else if(item_selection == 2){
										if(RTC_IsConfigured()){
												operation_mode = 1;
												item_selection = 4;
												Draw_Operation_Mode_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
										}else{
												// RTC not configured - redirect to Set Time page
												UI_state = STATE_USER_SETTING_SET_TIME;
												Top_Bar_Active = 1;
												item_selection = 0;
												leave_icon_color = 0;
												edit_icon_color = 1;
												temp_rtc.Year = rtc_time.Year;
												temp_rtc.Mon = rtc_time.Mon;
												temp_rtc.Date = rtc_time.Date;
												temp_rtc.Hour = rtc_time.Hour;
												temp_rtc.Min = rtc_time.Min;
												LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
												Draw_User_Setting_SetTime_Page(item_selection, leave_icon_color, edit_icon_color);
										}
								}else if(item_selection == 3){
										operation_mode = 2;
										item_selection = 4;
										Draw_Operation_Mode_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
								}
								//delay_ms(100);
						}
				}
				else if(item_selection == 4){
					if(key.key_val == ENTERKEY){
							g_parameter.operation_mode = operation_mode;
							if(Flash_Save_Parameter() != FLASH_OK){
									LOGE("Flash write fail!\r\n");
							}
							UI_state = STATE_FUNC_SETTING;
							func_setting_scroll = 0;  // Reset scroll
							Top_Bar_Active = 0;
							item_selection = 1;
							leave_icon_color = 0;
							edit_icon_color = 0;
							LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
							Draw_Function_Setting_Page(item_selection, leave_icon_color, edit_icon_color);
					}else if((key.key_val == UPKEY) || (key.key_val == DOWNKEY)){
							item_selection = 1;
							leave_icon_color = 0;
							edit_icon_color = 0;
							Draw_Operation_Mode_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}						
				}
	}

	else if(UI_state == STATE_HEATING_SCHEDULE_MENU)
	{
			if(Top_Bar_Active)  // TopBar mode: Edit/Leave
			{
				if(key.key_val == UPKEY || key.key_val == DOWNKEY)  // Up or Down key pressed
				{
						Update_TopBar();
				}
				else if(key.key_val == ENTERKEY)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						Top_Bar_Active = 0;
						item_selection = 1;
						leave_icon_color = 0;
						edit_icon_color = 0;
						Draw_Heating_Schedule_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					else  // Leave Icon is red
					{
						// Go back to Function Setting in Heating Schedule item
						UI_state = STATE_FUNC_SETTING;
						func_setting_scroll = 0;  // Reset scroll
						Top_Bar_Active = 0;
						item_selection = 2;
						leave_icon_color = 0;
						edit_icon_color = 0;
						Draw_Function_Setting_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					
				}
			}
			else   // Program Type / Workday Setting / Restday Setting mode
			{
				if(key.key_val == DOWNKEY)  // Down key pressed
				{
					if(item_selection < 3)  // Can go to Restday Setting (2)
					{
						// Move selection down - only update changed rows
						Draw_Heating_Schedule_Menu_Row(item_selection, 0);  // Old row unselected
						item_selection++;
						Draw_Heating_Schedule_Menu_Row(item_selection, 1);  // New row selected
					}
					
				}
				else if(key.key_val == UPKEY)  // Up key pressed
				{
					if(item_selection > 1)
					{
						// Move selection up - only update changed rows
						Draw_Heating_Schedule_Menu_Row(item_selection, 0);  // Old row unselected
						item_selection--;
						Draw_Heating_Schedule_Menu_Row(item_selection, 1);  // New row selected
					}
					else  // At top menu, go back to TopBar mode
					{
						Top_Bar_Active = 1;
						item_selection = 0;
						leave_icon_color = 1;
						edit_icon_color = 0;
						Draw_TopBar(leave_icon_color, edit_icon_color);
						Draw_Heating_Schedule_Menu_Row(1, item_selection); 	//Redraw RAW 1
					}
					
				}
				else if(key.key_val == ENTERKEY)  // Enter key pressed
				{
					if(item_selection == 1)  // Program Type selected
					{
						// Enter Program Type page
						UI_state = STATE_HEATING_SCHEDULE_PROG_TYPE;
						current_prog_type = g_parameter.current_prog_type;
						Top_Bar_Active = 1;
						leave_icon_color = 0;
						edit_icon_color = 1;
						item_selection = 0;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Heating_Schedule_Prog_Type_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					else if(item_selection == 2 || item_selection == 3)  // Workday or Restday Setting selected
					{
						// Set schedule edit source: 0=Workday, 1=Restday
						schedule_edit_source = (item_selection == 2) ? 0 : 1;
						UI_state = STATE_SCHEDULE_EDIT;
						Top_Bar_Active = 1;
						leave_icon_color = 0;
						edit_icon_color = 1;
						item_selection = 0;
						schedule_edit_scroll = 0;     // Reset scroll bar 
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Schedule_Edit_Page(item_selection, leave_icon_color, edit_icon_color);  
					}
					
				}
			}
	}
	else if(UI_state == STATE_HEATING_SCHEDULE_PROG_TYPE)
	{
			// Program Type page navigation
			if(Top_Bar_Active == 1) // Top Bar (Leave / Edit)
			{
				if(key.key_val == UPKEY || key.key_val == DOWNKEY)  // Up or Down
				{
					Update_TopBar();
				}
				else if(key.key_val == ENTERKEY)  // Enter key
				{
					if(leave_icon_color) {
						//go back to Heating schedul menu page and stay in program type item
						UI_state = STATE_HEATING_SCHEDULE_MENU;
						item_selection = 1;
						leave_icon_color = 0;
						edit_icon_color = 0;
						Top_Bar_Active = 0;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Heating_Schedule_Menu_Page(item_selection, leave_icon_color, edit_icon_color);  
					} else { // Edit selected
						item_selection = 1;
						leave_icon_color = 0;
						edit_icon_color = 0;
						Top_Bar_Active = 0;
						Draw_Heating_Schedule_Prog_Type_Page(item_selection, leave_icon_color, edit_icon_color);
					}

				}
			}
			else if(item_selection == 1)
			{
				if(key.key_val == DOWNKEY) // Down
				{
					if(current_prog_type < 2) {
						current_prog_type++;
						Draw_Heating_Schedule_Prog_Type_Content(current_prog_type);
					}
					
				}
				else if(key.key_val == UPKEY) // Up
				{
					if(current_prog_type > 0) {
						current_prog_type--;
						Draw_Heating_Schedule_Prog_Type_Content(current_prog_type);
					} else {
						// At first item, go back to TopBar with Leave red
						Top_Bar_Active = 1;
						item_selection = 0;
						leave_icon_color = 1;
						edit_icon_color = 0;
						Draw_TopBar(leave_icon_color, edit_icon_color);
						Draw_Heating_Schedule_Prog_Type_Content(current_prog_type);
					}
					
				}
				else if(key.key_val == ENTERKEY) // Enter -> Move to Save button
				{
					//Top_Bar_Active = 2;
					item_selection = 2; // Save Prompt
					Draw_Heating_Schedule_Prog_Type_Page(item_selection, leave_icon_color, edit_icon_color);
					
				}
			}
			else if(item_selection == 2) // Save prompt
			{
				if(key.key_val == UPKEY || key.key_val == DOWNKEY) // Up or Down -> Cancel save, go back to Choosing
				{
					Top_Bar_Active = 0;
					item_selection = 1;
					Draw_Heating_Schedule_Prog_Type_Page(item_selection, leave_icon_color, edit_icon_color);
					
				}
				else if(key.key_val == ENTERKEY) // Enter -> Save and go back to Heating Schedule Menu
				{
					// Go back to Heating Schedule Menu with Leave red
					UI_state = STATE_HEATING_SCHEDULE_MENU;
					Top_Bar_Active = 0;   // Exit TopBar mode, enter edit mode
					leave_icon_color = 0;
					edit_icon_color = 0;
					item_selection = 1;
					g_parameter.current_prog_type = current_prog_type; // Save selection
					if(Flash_Save_Parameter() != FLASH_OK){
							LOGE("Flash write fail!\r\n");
					}
					LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
					Draw_Heating_Schedule_Menu_Page(item_selection, leave_icon_color, edit_icon_color);  
					//delay_ms(100);
				}
			}
	}
	else if(UI_state == STATE_SCHEDULE_EDIT)
	{
			// Schedule Edit page navigation (P1-P6)
			if(Top_Bar_Active)  // TopBar mode: Edit/Leave
			{
				if(key.key_val == UPKEY || key.key_val == DOWNKEY)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key.key_val == ENTERKEY)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter P1-P6/Finish mode, Edit/Leave both black, P1 Arrow red
						Top_Bar_Active = 0;
						leave_icon_color = 0;
						edit_icon_color = 0;
						item_selection = 1;
						Draw_Schedule_Edit_Page(item_selection, leave_icon_color, edit_icon_color); 
					}
					else  // Leave Icon is red
					{
						// Go back to Heating Schedule Menu 
						UI_state = STATE_HEATING_SCHEDULE_MENU;
						Top_Bar_Active = 0;
						leave_icon_color = 0;
						edit_icon_color = 0;
						item_selection = (schedule_edit_source == 0) ? 2 : 3;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Heating_Schedule_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					//delay_ms(100);
				}
			}
			else   // P1-P6/Finish mode
			{
				if(key.key_val == DOWNKEY)  // Down key pressed
				{
					if(item_selection < 7)  // Can go to Finish (7)
					{
						uint8_t old_sel = item_selection;
						uint8_t old_scroll;
						item_selection++;
						
						// Check if scroll position needs to change
						// scroll=0: P1-P4, scroll=1: P2-P5, scroll=2: P3-P6, scroll=3: P4-P6+Finish
						// Down scroll logic: scroll when new selection goes below visible area
						old_scroll = schedule_edit_scroll;
						if(item_selection <= 6) {
							// Regular period (P1-P6): scroll if below visible bottom
							if(item_selection > schedule_edit_scroll + 4) {
								schedule_edit_scroll = item_selection - 4;
							}
						} else {
							// Finish (sel=6): need scroll=3 to show Finish at row 3
							if(schedule_edit_scroll < 3) {
								schedule_edit_scroll = 3;
							}
						}
						
						// If scroll changed, redraw entire page
						if(old_scroll != schedule_edit_scroll) {
							Draw_Schedule_Edit_Page(item_selection, 0, 0);
						} else {
							// Only update changed rows within the 4 visible rows
							uint8_t old_row = old_sel - schedule_edit_scroll;
							uint8_t new_row = item_selection - schedule_edit_scroll;
							
							// Handle transition to/from Finish
							if(old_sel == 7) {
								// From Finish to P6
								Draw_Schedule_Edit_Finish(0);
								Draw_Schedule_Edit_Row(new_row-1, item_selection - 1, 1);
							} else if(item_selection == 7) {
								// From P6 to Finish
								Draw_Schedule_Edit_Row(old_row-1, old_sel, 0);
								Draw_Schedule_Edit_Finish(1);
							} else {
								// Normal row transition
								Draw_Schedule_Edit_Row(old_row-1, old_sel - 1, 0);
								Draw_Schedule_Edit_Row(new_row-1, item_selection - 1, 1);
							}
						}
					}
					
				}
				else if(key.key_val == UPKEY)  // Up key pressed
				{
					if(item_selection > 1)
					{
						uint8_t old_sel = item_selection;
						uint8_t old_scroll;
						item_selection--;
						
						// Check if scroll position needs to change
						// scroll=0: P1-P4, scroll=1: P2-P5, scroll=2: P3-P6, scroll=3: P4-P6+Finish
						// Up scroll logic: scroll when new selection goes above visible area
						old_scroll = schedule_edit_scroll;
						if((item_selection - 1) < schedule_edit_scroll) {
							schedule_edit_scroll = item_selection - 1;
						}
						
						// If scroll changed, redraw entire page
						if((old_scroll != schedule_edit_scroll) && (item_selection != 6)) {
							Draw_Schedule_Edit_Page(item_selection, 0, 0);
						} else {
							// Only update changed rows within the 4 visible rows
							uint8_t old_row = old_sel - schedule_edit_scroll;
							uint8_t new_row = item_selection - schedule_edit_scroll;
							
							// Handle transition to/from Finish
							if(old_sel == 7) {
								// From Finish to P6
								Draw_Schedule_Edit_Row(new_row-1, item_selection - 1, 1);
								Draw_Schedule_Edit_Finish(0);
							} else if(item_selection == 7) {
								// From P6 to Finish
								Draw_Schedule_Edit_Row(old_row, old_sel, 0);
								Draw_Schedule_Edit_Finish(1);
							} else {
								// Normal row transition
								Draw_Schedule_Edit_Row(old_row-1, old_sel-1, 0);
								Draw_Schedule_Edit_Row(new_row-1, item_selection - 1, 1);
							}
						}
					}
					else  // At P1, go back to TopBar mode
					{
						//schedule_edit_substate = 0;
						//schedule_edit_top_sel = 1;  // Edit red
						// Redraw entire page to show TopBar with Edit red and clear P1 selection
						Top_Bar_Active = 1;
						leave_icon_color = 1;
						edit_icon_color = 0;
						item_selection = 0;
						Draw_Schedule_Edit_Page(item_selection, leave_icon_color, edit_icon_color);  
					}
					
				}
				else if(key.key_val == ENTERKEY)  // Enter key pressed
				{
					if(item_selection == 7)  // Finish selected
					{
						// Go back to Heating Schedule Menu 
						UI_state = STATE_HEATING_SCHEDULE_MENU;
						Top_Bar_Active = 0;
						leave_icon_color = 0;
						edit_icon_color = 0;
						item_selection = (schedule_edit_source == 0) ? 2 : 3;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Heating_Schedule_Menu_Page(item_selection, leave_icon_color, edit_icon_color);  
					}
					else if(item_selection <= 6)  // P1-P6 selected
					{
						// Enter Schedule Time Setting page
						UI_state = STATE_SCHEDULE_TIME_SETTING;
						schedule_time_setting_period = item_selection;  // P1=1, P2=2, ..., P6=6
						Top_Bar_Active = 1;
						leave_icon_color = 0;
						edit_icon_color = 1;
						item_selection = 0;
						// Initialize edit values from schedule_settings array (Workday or Restday)
						{
							uint8_t (*sched_ptr)[4] = schedule_edit_source ? g_parameter.holiday_schedule : g_parameter.workday_schedule;
							schedule_time_edit_hour = sched_ptr[schedule_time_setting_period - 1][0];
							schedule_time_edit_min = sched_ptr[schedule_time_setting_period - 1][1];
							schedule_time_edit_temp = sched_ptr[schedule_time_setting_period - 1][2];
							schedule_time_on_off = sched_ptr[schedule_time_setting_period - 1][3];
						}
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Schedule_Time_Setting_Page(schedule_time_setting_period, leave_icon_color, edit_icon_color);
					}
					//delay_ms(100);
				}
			}
	}
	else if(UI_state == STATE_SCHEDULE_TIME_SETTING)
	{
			// Schedule Time Setting page navigation
			if(Top_Bar_Active)  // TopBar mode: Edit/Leave
			{
				if(key.key_val == UPKEY || key.key_val == DOWNKEY)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key.key_val == ENTERKEY)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter Hour edit mode
						item_selection = 1;
						leave_icon_color = 0;
						edit_icon_color = 0;
						Top_Bar_Active = 0;
						Draw_TopBar(leave_icon_color, edit_icon_color);
						Draw_Schedule_Time_Setting_TimeDigits();
					}
					else  // Leave Icon is red
					{
						// Go back to Schedule Edit
						UI_state = STATE_SCHEDULE_EDIT;
						Top_Bar_Active = 0;
						leave_icon_color = 0;
						edit_icon_color = 0;
						item_selection = schedule_time_setting_period;
						// Set scroll based on selection to ensure correct rows are displayed
						if(item_selection <= 4) {
							schedule_edit_scroll = 0;  // P1-P4 visible
						} else {
							schedule_edit_scroll = item_selection - 4;  // Ensure selected item is at bottom row
						}
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Schedule_Edit_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					
				}
			}
			else if(item_selection == 1)  // Edit Hour
			{
				if(key.key_val == UPKEY)  // Up key - increase hour or go to TopBar
				{
					if(schedule_time_edit_hour < 23) {
						schedule_time_edit_hour++;
						Draw_Schedule_Time_Setting_TimeDigits();
					} else {
						Top_Bar_Active = 1;
						item_selection = 0;
						leave_icon_color = 1;
						edit_icon_color = 0;
						Draw_Schedule_Time_Setting_Page(schedule_time_setting_period, 1, 0);
					}
					
				}
				else if(key.key_val == DOWNKEY)  // Down key - decrease hour
				{
					if(schedule_time_edit_hour > 0) schedule_time_edit_hour--;
					else schedule_time_edit_hour = 23;
					// Only update time digits
					Draw_Schedule_Time_Setting_TimeDigits();
					
				}
				else if(key.key_val == ENTERKEY)  // Enter key - move to minute edit
				{
					item_selection = 2;  // edit min
					Draw_Schedule_Time_Setting_TimeDigits();
					
				}
			}
			else if(item_selection == 2)  // Edit Minume
			{
				if(key.key_val == UPKEY)  // Up key - increase minute
				{
					if(schedule_time_edit_min < 59) schedule_time_edit_min++;
					else schedule_time_edit_min = 0;
					// Only update time digits
					Draw_Schedule_Time_Setting_TimeDigits();
					
				}
				else if(key.key_val == DOWNKEY)  // Down key - decrease minute
				{
					if(schedule_time_edit_min > 0) schedule_time_edit_min--;
					else schedule_time_edit_min = 59;
					// Only update time digits
					Draw_Schedule_Time_Setting_TimeDigits();
					
				}
				else if(key.key_val == ENTERKEY)  // Enter key - move to temperature edit
				{
					item_selection = 3;  // Edit Temperature
					Draw_Schedule_Time_Setting_TimeDigits();
					Draw_Schedule_Time_Setting_TempDigits();
					
				}
			}
			else if(item_selection == 3)  // Edit Temperature
			{
				if(key.key_val == UPKEY)  // Up key - increase temperature
				{
					if(schedule_time_edit_temp < g_parameter.temp_limit_max) schedule_time_edit_temp++;
					// Only update temperature digits
					Draw_Schedule_Time_Setting_TempDigits();
					
				}
				else if(key.key_val == DOWNKEY)  // Down key - decrease temperature
				{
					if(schedule_time_edit_temp > g_parameter.temp_limit_min) schedule_time_edit_temp--;
					// Only update temperature digits
					Draw_Schedule_Time_Setting_TempDigits();
					
				}
				else if(key.key_val == ENTERKEY)  // Enter key - move to ON/OFF
				{
					item_selection = 4;  // Edit ON/OFF
					Draw_Schedule_Time_Setting_TempDigits();
					Draw_Schedule_Time_Setting_OnOff();
					
				}
			}
			else if(item_selection == 4)  // Edit ON/OFF
			{
				if(key.key_val == UPKEY || key.key_val == DOWNKEY)  // Up/Down - toggle ON/OFF
				{
					schedule_time_on_off = (schedule_time_on_off == 1) ? 0 : 1;
					Draw_Schedule_Time_Setting_OnOff();
					
				}
				else if(key.key_val == ENTERKEY)  // Enter - move to Save
				{
					item_selection = 5;  // Save mode
					Draw_Schedule_Time_Setting_OnOff();
					Draw_Schedule_Time_Setting_Save();
					
				}
			}
			else if(item_selection == 5)  // Save mode
			{
				if(key.key_val == ENTERKEY)  // Enter key pressed
				{
					// Save settings to array (P1~P6 index 0~5, Workday or Restday)
					{
						uint8_t idx = schedule_time_setting_period - 1;
						uint8_t (*sched_ptr)[4] = schedule_edit_source ? g_parameter.holiday_schedule : g_parameter.workday_schedule;
						sched_ptr[idx][0] = schedule_time_edit_hour;
						sched_ptr[idx][1] = schedule_time_edit_min;
						sched_ptr[idx][2] = schedule_time_edit_temp;
						sched_ptr[idx][3] = schedule_time_on_off;
					}
					if(Flash_Save_Parameter() != FLASH_OK){
							LOGE("Flash write fail!\r\n");
					}
					// Save and go back to Schedule Edit
					UI_state = STATE_SCHEDULE_EDIT;
					//schedule_edit_substate = 1;
					item_selection = schedule_time_setting_period;  // P1-P6 -> 1-6
					// Set scroll based on selection to ensure correct rows are displayed
					if(item_selection <= 4) {
						schedule_edit_scroll = 0;  // P1-P4 visible
					} else {
						schedule_edit_scroll = item_selection - 4;  // Ensure selected item is at bottom row
					}
					LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
					Draw_Schedule_Edit_Page(item_selection, leave_icon_color, edit_icon_color);
					
				}
			}
	}
	else if(UI_state == STATE_CONTROL_ADJ_MENU)
	{
			
			if(Top_Bar_Active)  // TopBar mode: Edit/Leave
			{
				if(key.key_val == UPKEY || key.key_val == DOWNKEY)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key.key_val == ENTERKEY)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Edit/Leave both black, Sensor selected
						Top_Bar_Active = 0;
						item_selection = 1;
						leave_icon_color = 0;
						edit_icon_color = 0;
						control_adj_menu_scroll = 0;  // Reset scroll
						Draw_Control_Adj_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					else  // Leave Icon is red
					{
						// Go back to Function Setting Edit mode with cursor on Control Adj
						UI_state = STATE_FUNC_SETTING;
						func_setting_scroll = 0;  // Reset scroll
						Top_Bar_Active = 0;
						item_selection = 3;
						leave_icon_color = 0;
						edit_icon_color = 0;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Function_Setting_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					
				}
			}
			else // Menu Page: Sensor/Power Limit/PWM/Temp.Correct/Temp.Limit/Temp.Protect/Power On State
			{
				if(key.key_val == DOWNKEY)  // Down key pressed
				{
					if(item_selection < Control_Adj_Menu_Item_Count)  // Can go down
					{
						uint8_t old_sel = item_selection;
						uint8_t old_scroll = control_adj_menu_scroll;
						item_selection++;
						
						// Check if scroll position needs to change
						// Adjust visible rows based on font size
						if(g_parameter.font_size == 0){
							if(item_selection > control_adj_menu_scroll + 4) {
								control_adj_menu_scroll = item_selection - 4;
							}
						}else if(g_parameter.font_size == 1){
							if(item_selection > control_adj_menu_scroll + 3) {
								control_adj_menu_scroll = item_selection - 3;
							}
							if(control_adj_menu_scroll > 4) control_adj_menu_scroll = 4;
						}
						// If scroll changed, redraw entire page
						if(old_scroll != control_adj_menu_scroll) {
							Draw_Control_Adj_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
						} else {
							// Only update changed rows
							Draw_Control_Adj_Menu_Row((old_sel - 1) - control_adj_menu_scroll, 0);  // Old row unselected
							Draw_Control_Adj_Menu_Row((item_selection - 1) - control_adj_menu_scroll, 1);  // New row selected
						}
					}
					
				}
				else if(key.key_val == UPKEY)  // Up key pressed
				{
					if(item_selection > 1)
					{
						uint8_t old_sel = item_selection;
						uint8_t old_scroll = control_adj_menu_scroll;
						item_selection--;
						
						// Check if scroll position needs to change
						if(item_selection - 1 < control_adj_menu_scroll) {
							control_adj_menu_scroll = item_selection - 1;
						}
						if(g_parameter.font_size == 0){
							if(control_adj_menu_scroll > 3) control_adj_menu_scroll = 3;
						}else if(g_parameter.font_size == 1){
							if(control_adj_menu_scroll > 4) control_adj_menu_scroll = 4;
						}
						// If scroll changed, redraw entire page
						if(old_scroll != control_adj_menu_scroll) {
							Draw_Control_Adj_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
						} else {
							// Only update changed rows
							Draw_Control_Adj_Menu_Row((old_sel -1) - control_adj_menu_scroll, 0);  // Old row unselected
							Draw_Control_Adj_Menu_Row((item_selection -1) - control_adj_menu_scroll, 1);  // New row selected
						}
					}
					else  // At Sensor (sel=0), go back to TopBar mode
					{
						Top_Bar_Active = 1;
						leave_icon_color = 1;
						edit_icon_color = 0;
						item_selection = 0;
						// Redraw entire page to show TopBar with Edit red
						Draw_Control_Adj_Menu_Page(item_selection, leave_icon_color, edit_icon_color);  // 0xFF = TopBar mode, Edit red
					}
					
				}
//"Sensor", "Comfort Mode", "Power Limit", "Sensor Calibrate", "Input Temp.Limit", "Protect Temp.", "Power On State"
				else if(key.key_val == ENTERKEY)  // Enter key pressed
				{
					uint8_t menu_idx = item_selection - 1;
					if(menu_idx < Control_Adj_Menu_Item_Count)
					{
						UI_state = Control_Adj_Menu_Items[menu_idx].next_state;
						Top_Bar_Active = 1;
						item_selection = 0;
						leave_icon_color = 0;
						edit_icon_color = 1;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);

						if(UI_state == STATE_CONTROL_ADJ_SENSOR)
						{
							current_sensor_type = g_parameter.sensor_type;
							Draw_Control_Adj_Sensor_Page(item_selection, leave_icon_color, edit_icon_color);
						}
						else if(UI_state == STATE_CONTROL_POWER_LIMIT)
						{
							power_limit = g_parameter.power_limit;
							power_limit_switch = g_parameter.power_limit_switch;
							Draw_Control_Adj_Power_Limit_Page(item_selection, leave_icon_color, edit_icon_color);
						}
						else if(UI_state == STATE_CONTROL_ADJ_TEMP_CORRECT)
						{
							Draw_Control_Adj_TempCorrect_Page(item_selection, leave_icon_color, edit_icon_color);
						}
						else if(UI_state == STATE_CONTROL_ADJ_TEMP_LIMIT)
						{
							Draw_Control_Adj_TempLimit_Page(item_selection, leave_icon_color, edit_icon_color);
						}
						else if(UI_state == STATE_CONTROL_ADJ_TEMP_PROTECT)
						{
							Draw_Control_Adj_TempProtect_Page(item_selection, leave_icon_color, edit_icon_color);
						}
						else if(UI_state == STATE_CONTROL_ADJ_POWER_ON_STATE)
						{
							power_on_state = g_parameter.power_on_state;
							Draw_Control_Adj_Power_On_State_Page(item_selection, leave_icon_color, edit_icon_color);
						}
						else if(UI_state == STATE_CONTROL_TEMP_SWING){
								temp_swing = g_parameter.temp_swing;
								Draw_Control_Adj_Temp_Swing_Page(item_selection, leave_icon_color, edit_icon_color);
						}
						else if(UI_state == STATE_CONTROL_ADJ_PWM_SETTING){
								pwm_setting_selection = 0;
								Draw_Control_Adj_PWM_Setting_Page(item_selection, leave_icon_color, edit_icon_color);
						}
					}
				}
			}
	}

		else if(UI_state == STATE_CONTROL_ADJ_PWM_SETTING)
	{
			if(Top_Bar_Active == 1)  // TopBar mode: Edit/Leave
			{
				if(key.key_val == UPKEY || key.key_val == DOWNKEY)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key.key_val == ENTERKEY)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter option selection mode
						Top_Bar_Active = 0;
						edit_icon_color = 0;
						leave_icon_color = 0;
						item_selection = 1;
						pwm_setting_selection = 0;  // Default to Astro Time
						Draw_Control_Adj_PWM_Setting_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					else  // Leave Icon is red
					{
						UI_state = STATE_CONTROL_ADJ_MENU;
						Top_Bar_Active = 0;
						leave_icon_color = 0;
						edit_icon_color = 0;
						item_selection = Find_Control_Adj_Menu_Index(STATE_CONTROL_ADJ_PWM_SETTING);
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Control_Adj_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}
				}
			}
			else if(item_selection == 1)  // Option selection mode
			{
				if(key.key_val == DOWNKEY){  // Down key - next option
					if(pwm_setting_selection < 1){
						pwm_setting_selection++;
						Draw_Control_Adj_PWM_Setting_Choices(item_selection);
					}
				}else if(key.key_val == UPKEY){  // Up key - prev option or TopBar
					if(pwm_setting_selection > 0){
						pwm_setting_selection--;
						Draw_Control_Adj_PWM_Setting_Choices(item_selection);
					}else{  // At first option, go back to TopBar
						Top_Bar_Active = 1;
						edit_icon_color = 0;
						leave_icon_color = 1;
						item_selection = 0;
						Draw_Control_Adj_PWM_Setting_Page(item_selection, leave_icon_color, edit_icon_color);
					}
				}else if(key.key_val == ENTERKEY){  // Enter - go directly to selected page
					if(pwm_setting_selection == 0){
							UI_state = STATE_CONTROL_ADJ_PWM_ASTRO_TIME;
							Top_Bar_Active = 1;
							item_selection = 0;
							leave_icon_color = 0;
							edit_icon_color = 1;
							LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
							Draw_Control_Adj_PWM_Astro_Time_Page(item_selection, leave_icon_color, edit_icon_color);
					}else{
							UI_state = STATE_CONTROL_ADJ_PWM_DUTY;
							Top_Bar_Active = 1;
							item_selection = 0;
							leave_icon_color = 0;
							edit_icon_color = 1;
							LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
							Draw_Control_Adj_PWM_Duty_Cycle_Page(item_selection, leave_icon_color, edit_icon_color);
					}
				}
			}	
	}

		else if(UI_state == STATE_CONTROL_ADJ_PWM_ASTRO_TIME)
	{
			if(Top_Bar_Active == 1)  // TopBar mode: Edit/Leave
			{
				if(key.key_val == UPKEY || key.key_val == DOWNKEY)
				{
					Update_TopBar();
				}
				else if(key.key_val == ENTERKEY)
				{
					if(edit_icon_color)  // Edit Icon is red → enter edit mode
					{
						temp_astro_day_hour = g_parameter.astro_day_hour;
						temp_astro_day_min = g_parameter.astro_day_min;
						temp_astro_night_hour = g_parameter.astro_night_hour;
						temp_astro_night_min = g_parameter.astro_night_min;

						Top_Bar_Active = 0;
						item_selection = 1;
						leave_icon_color = 0;
						edit_icon_color = 0;
						Draw_Control_Adj_PWM_Astro_Time_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					else  // Leave Icon is red → go back (no changes)
					{
						UI_state = STATE_CONTROL_ADJ_PWM_SETTING;
						Top_Bar_Active = 0;
						item_selection = 1;
						leave_icon_color = 0;
						edit_icon_color = 0;
						pwm_setting_selection = 0;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Control_Adj_PWM_Setting_Page(item_selection, leave_icon_color, edit_icon_color);
					}
				}
			}
			else if(item_selection == 1){  // Day Hour
					if(key.key_val == DOWNKEY){
							if(temp_astro_day_hour > 0){
									temp_astro_day_hour--;
									Draw_Control_Adj_PWM_Astro_Time_Content(item_selection);
							}
					}else if(key.key_val == UPKEY){
							if(temp_astro_day_hour < 23){
									temp_astro_day_hour++;
									Draw_Control_Adj_PWM_Astro_Time_Content(item_selection);
							}
					}else if(key.key_val == ENTERKEY){
							item_selection = 2;
							Draw_Control_Adj_PWM_Astro_Time_Content(item_selection);
					}
			}else if(item_selection == 2){  // Day Minute
					if(key.key_val == DOWNKEY){
							if(temp_astro_day_min > 0){
									temp_astro_day_min--;
									Draw_Control_Adj_PWM_Astro_Time_Content(item_selection);
							}
					}else if(key.key_val == UPKEY){
							if(temp_astro_day_min < 59){
									temp_astro_day_min++;
									Draw_Control_Adj_PWM_Astro_Time_Content(item_selection);
							}
					}else if(key.key_val == ENTERKEY){
							item_selection = 3;
							Draw_Control_Adj_PWM_Astro_Time_Content(item_selection);
					}
			}else if(item_selection == 3){  // Night Hour
					if(key.key_val == DOWNKEY){
							if(temp_astro_night_hour > 0){
									temp_astro_night_hour--;
									Draw_Control_Adj_PWM_Astro_Time_Content(item_selection);
							}
					}else if(key.key_val == UPKEY){
							if(temp_astro_night_hour < 23){
									temp_astro_night_hour++;
									Draw_Control_Adj_PWM_Astro_Time_Content(item_selection);
							}
					}else if(key.key_val == ENTERKEY){
							item_selection = 4;
							Draw_Control_Adj_PWM_Astro_Time_Content(item_selection);
					}
			}else if(item_selection == 4){  // Night Minute
					if(key.key_val == DOWNKEY){
							if(temp_astro_night_min > 0){
									temp_astro_night_min--;
									Draw_Control_Adj_PWM_Astro_Time_Content(item_selection);
							}
					}else if(key.key_val == UPKEY){
							if(temp_astro_night_min < 59){
									temp_astro_night_min++;
									Draw_Control_Adj_PWM_Astro_Time_Content(item_selection);
							}
					}else if(key.key_val == ENTERKEY){
							item_selection = 5;
							Draw_Control_Adj_PWM_Astro_Time_Content(item_selection);
					}
			}else if(item_selection == 5){  // Save button
					if((key.key_val == UPKEY) || (key.key_val == DOWNKEY)){
							item_selection = 4;
							Draw_Control_Adj_PWM_Astro_Time_Content(item_selection);
					}else if(key.key_val == ENTERKEY){
							g_parameter.astro_day_hour = temp_astro_day_hour;
							g_parameter.astro_day_min = temp_astro_day_min;
							g_parameter.astro_night_hour = temp_astro_night_hour;
							g_parameter.astro_night_min = temp_astro_night_min;
							if(Flash_Save_Parameter() != FLASH_OK){
									LOGE("Flash write fail!\r\n");
							}
							UI_state = STATE_CONTROL_ADJ_PWM_SETTING;
							Top_Bar_Active = 0;
							item_selection = 1;
							leave_icon_color = 0;
							edit_icon_color = 0;
							pwm_setting_selection = 0;
							LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
							Draw_Control_Adj_PWM_Setting_Page(item_selection, leave_icon_color, edit_icon_color);
					}
			}
	}

		else if(UI_state == STATE_CONTROL_ADJ_PWM_DUTY)
	{
			if(Top_Bar_Active == 1)  // TopBar mode: Edit/Leave
			{
				if(key.key_val == UPKEY || key.key_val == DOWNKEY)
				{
					Update_TopBar();
				}
				else if(key.key_val == ENTERKEY)
				{
					if(edit_icon_color)  // Edit Icon is red → enter edit mode
					{
						temp_pwm_day_duty = g_parameter.pwm_day_duty;
						temp_pwm_night_duty = g_parameter.pwm_night_duty;

						Top_Bar_Active = 0;
						item_selection = 1;
						leave_icon_color = 0;
						edit_icon_color = 0;
						Draw_Control_Adj_PWM_Duty_Cycle_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					else  // Leave Icon is red → go back (no changes)
					{
						UI_state = STATE_CONTROL_ADJ_PWM_SETTING;
						Top_Bar_Active = 0;
						item_selection = 1;
						leave_icon_color = 0;
						edit_icon_color = 0;
						pwm_setting_selection = 1;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Control_Adj_PWM_Setting_Page(item_selection, leave_icon_color, edit_icon_color);
					}
				}
			}
			else if(item_selection == 1){  // Day duty
					if(key.key_val == DOWNKEY){
							if(temp_pwm_day_duty > 10){
									temp_pwm_day_duty -= 10;
									Draw_Control_Adj_PWM_Duty_Cycle_Content(item_selection);
							}
					}else if(key.key_val == UPKEY){
							if(temp_pwm_day_duty < 90){
									temp_pwm_day_duty += 10;
									Draw_Control_Adj_PWM_Duty_Cycle_Content(item_selection);
							}
					}else if(key.key_val == ENTERKEY){
							item_selection = 2;
							Draw_Control_Adj_PWM_Duty_Cycle_Content(item_selection);
					}
			}else if(item_selection == 2){  // Night duty
					if(key.key_val == DOWNKEY){
							if(temp_pwm_night_duty > 10){
									temp_pwm_night_duty -= 10;
									Draw_Control_Adj_PWM_Duty_Cycle_Content(item_selection);
							}
					}else if(key.key_val == UPKEY){
							if(temp_pwm_night_duty < 90){
									temp_pwm_night_duty += 10;
									Draw_Control_Adj_PWM_Duty_Cycle_Content(item_selection);
							}
					}else if(key.key_val == ENTERKEY){
							item_selection = 3;
							Draw_Control_Adj_PWM_Duty_Cycle_Content(item_selection);
					}
			}else if(item_selection == 3){  // Save button
					if((key.key_val == UPKEY) || (key.key_val == DOWNKEY)){
							item_selection = 2;
							Draw_Control_Adj_PWM_Duty_Cycle_Content(item_selection);
					}else if(key.key_val == ENTERKEY){
							g_parameter.pwm_day_duty = temp_pwm_day_duty;
							g_parameter.pwm_night_duty = temp_pwm_night_duty;
							if(Flash_Save_Parameter() != FLASH_OK){
									LOGE("Flash write fail!\r\n");
							}
							UI_state = STATE_CONTROL_ADJ_PWM_SETTING;
							Top_Bar_Active = 0;
							item_selection = 1;
							leave_icon_color = 0;
							edit_icon_color = 0;
							pwm_setting_selection = 1;
							LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
							Draw_Control_Adj_PWM_Setting_Page(item_selection, leave_icon_color, edit_icon_color);
					}
			}
	}

		else if(UI_state == STATE_CONTROL_ADJ_SENSOR)
	{
			
			if(Top_Bar_Active == 1)  // TopBar mode: Edit/Leave
			{
				if(key.key_val == UPKEY || key.key_val == DOWNKEY)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key.key_val == ENTERKEY)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter Room/Floor menu
						Top_Bar_Active = 0;
						edit_icon_color = 0;
						leave_icon_color = 0;
						item_selection = 1;
						Draw_Control_Adj_Sensor_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					else  // Leave Icon is red
					{
						// Go back to Control_Adj_menu page
						UI_state = STATE_CONTROL_ADJ_MENU;
						Top_Bar_Active = 0;
						leave_icon_color = 0;
						edit_icon_color = 0;
						item_selection = Find_Control_Adj_Menu_Index(STATE_CONTROL_ADJ_SENSOR);
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Control_Adj_Menu_Page(item_selection, leave_icon_color, edit_icon_color);  
					}
					
				}
			}
			else if(item_selection == 1)  // Room/Floor selection (swapped order)
			{
				if(key.key_val == DOWNKEY)  // Down key pressed
				{
					if(current_sensor_type < 1)  // Can go to Floor (1)
					{
						// Move selection down (Room -> Floor)
						current_sensor_type++;
						// Update choices display
						Draw_Control_Adj_Sensor_Choices(item_selection);
					}
					//delay_ms(100);
				}
				else if(key.key_val == UPKEY)  // Up key pressed
				{
					if(current_sensor_type > 0)  // Can go to Room (0)
					{
						// Move selection up (Floor -> Room)
						current_sensor_type--;
						// Update choices display
						Draw_Control_Adj_Sensor_Choices(item_selection);
					}
					else  // At Room, go back to TopBar mode
					{
						Top_Bar_Active = 1;
						edit_icon_color = 1;
						leave_icon_color = 0;
						item_selection = 0;
						Draw_Control_Adj_Sensor_Page(item_selection, leave_icon_color, edit_icon_color);  
					}
					
				}
				else if(key.key_val == ENTERKEY)  // Enter key pressed
				{
					// Enter Save mode
					item_selection = 2;
					Draw_Control_Adj_Sensor_Choices(item_selection);
					
				}
			}
			else if(item_selection == 2)  // Save mode
			{
				if(key.key_val == ENTERKEY)  // Enter key pressed
				{
					// Save setting and return to Control Adj Menu
					UI_state = STATE_CONTROL_ADJ_MENU;
					Top_Bar_Active = 0;
					leave_icon_color = 0;
					edit_icon_color = 0;
					item_selection = Find_Control_Adj_Menu_Index(STATE_CONTROL_ADJ_SENSOR);
					g_parameter.sensor_type = current_sensor_type;  // 0=Room, 1=Floor
					if(Flash_Save_Parameter() != FLASH_OK){
							LOGE("Flash write fail!\r\n");
					}
					LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
					Draw_Control_Adj_Menu_Page(item_selection, leave_icon_color, edit_icon_color);  
					
				}
				else if(key.key_val == UPKEY || key.key_val == DOWNKEY)  // Up or Down key in Save mode - cancel save
				{
					// Cancel Save mode, back to choices mode
					item_selection = 1;
					Draw_Control_Adj_Sensor_Choices(item_selection);
					
				}
			}
	}
	else if(UI_state == STATE_CONTROL_TEMP_SWING)
	{
		if(Top_Bar_Active == 1)
		{
			if(key.key_val == UPKEY || key.key_val == DOWNKEY)
			{
				Update_TopBar();
			}
			else if(key.key_val == ENTERKEY)
			{
				if(edit_icon_color)
				{
					Top_Bar_Active = 0;
					item_selection = 1;
					leave_icon_color = 0;
					edit_icon_color = 0;
					temp_swing = g_parameter.temp_swing;
					Draw_TopBar(leave_icon_color, edit_icon_color);
					Draw_Control_Adj_Temp_Swing_Content(item_selection);
				}
				else
				{
					UI_state = STATE_CONTROL_ADJ_MENU;
					Top_Bar_Active = 0;
					leave_icon_color = 0;
					edit_icon_color = 0;
					item_selection = Find_Control_Adj_Menu_Index(STATE_CONTROL_TEMP_SWING);
					LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
					Draw_Control_Adj_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
				}
			}
		}
		else if(item_selection == 1)
		{
			if(key.key_val == DOWNKEY)
			{
				if(temp_swing > 0.5)
				{
					temp_swing -= 0.5;
					Draw_Control_Adj_Temp_Swing_Content(item_selection);
				}
			}
			else if(key.key_val == UPKEY)
			{
				if(temp_swing < 3)
				{
					temp_swing += 0.5;
					Draw_Control_Adj_Temp_Swing_Content(item_selection);
				}
			}
			else if(key.key_val == ENTERKEY)
			{
				item_selection = 2;
				Draw_Control_Adj_Temp_Swing_Content(item_selection);
			}
		}
		else if(item_selection == 2)
		{
			if((key.key_val == UPKEY) || (key.key_val == DOWNKEY))
			{
				item_selection = 1;
				Draw_Control_Adj_Temp_Swing_Content(item_selection);
			}
			else if(key.key_val == ENTERKEY)
			{
				UI_state = STATE_CONTROL_ADJ_MENU;
				item_selection = Find_Control_Adj_Menu_Index(STATE_CONTROL_TEMP_SWING);
				Top_Bar_Active = 0;
				leave_icon_color = 0;
				edit_icon_color = 0;
				g_parameter.temp_swing = temp_swing;
				if(Flash_Save_Parameter() != FLASH_OK)
				{
					LOGE("Flash write fail!\r\n");
				}
				LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
				Draw_Control_Adj_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
			}
		}
	}
	else if(UI_state == STATE_CONTROL_ADJ_TEMP_CORRECT)
	{
		if(Top_Bar_Active == 1)  // TopBar mode: Edit/Leave
			{
				if(key.key_val == UPKEY || key.key_val == DOWNKEY)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key.key_val == ENTERKEY)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter Sensor Cal. edit mode
						Top_Bar_Active = 0;
						edit_icon_color = 0;
						leave_icon_color = 0;
						item_selection = 1;
						temp_correct_internal = g_parameter.temp_correct_internal;
						temp_correct_external = g_parameter.temp_correct_external;
						Draw_Control_Adj_TempCorrect_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					else  // Leave Icon is red
					{
						// Go back to Control Adj Menu with Leave red
						UI_state = STATE_CONTROL_ADJ_MENU;
						Top_Bar_Active = 0;
						leave_icon_color = 0;
						edit_icon_color = 0;
						item_selection = Find_Control_Adj_Menu_Index(STATE_CONTROL_ADJ_TEMP_CORRECT);
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Control_Adj_Menu_Page(item_selection, leave_icon_color, edit_icon_color);  
					}
					
				}
			}
			else if(item_selection == 1)		//internal sensor setting
			{
					if(key.key_val == UPKEY)  // Up key pressed
					{
							if(temp_correct_internal < 5.0){
									temp_correct_internal += 0.5;
									Draw_Control_Adj_TempCorrect_Contect(item_selection);
							}
					}
					else if(key.key_val == DOWNKEY) 	// Down key pressed
					{
							if(temp_correct_internal > -5.0){
									temp_correct_internal -= 0.5;
									Draw_Control_Adj_TempCorrect_Contect(item_selection);
							}
					}
					else if(key.key_val ==ENTERKEY)		// Enter key pressed
					{
							item_selection = 2;
							Draw_Control_Adj_TempCorrect_Contect(item_selection);
					}
					
			}
			else if(item_selection == 2)  //external sensor setting
			{
					if(key.key_val == UPKEY)  // Up key pressed
					{
							if(temp_correct_external < 5.0){
									temp_correct_external += 0.5;
									Draw_Control_Adj_TempCorrect_Contect(item_selection);
							}
					}
					else if(key.key_val == DOWNKEY) 	// Down key pressed
					{
							if(temp_correct_external > -5.0){
									temp_correct_external -= 0.5;
									Draw_Control_Adj_TempCorrect_Contect(item_selection);
							}
					}
					else if(key.key_val ==ENTERKEY)		// Enter key pressed
					{
							item_selection = 3;
							Draw_Control_Adj_TempCorrect_Contect(item_selection);
					}
					////delay_ms(100);
			}
			else if(item_selection == 3)	//enter save state
			{
					if(key.key_val == ENTERKEY)		//skip Up/Down key
					{
							UI_state = STATE_CONTROL_ADJ_MENU;
							Top_Bar_Active = 0;
							leave_icon_color = 0;
							edit_icon_color = 0;
							item_selection = Find_Control_Adj_Menu_Index(STATE_CONTROL_ADJ_TEMP_CORRECT);
							g_parameter.temp_correct_internal = temp_correct_internal;
							g_parameter.temp_correct_external = temp_correct_external;
							if(Flash_Save_Parameter() != FLASH_OK){
									LOGE("Flash write fail!\r\n");
							}
							LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
							Draw_Control_Adj_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}else if((key.key_val == UPKEY) || (key.key_val == DOWNKEY)){
							item_selection = 1;
							Draw_Control_Adj_TempCorrect_Contect(item_selection);
					}
			}
	}
	else if(UI_state == STATE_CONTROL_ADJ_TEMP_LIMIT)
	{
		if(Top_Bar_Active == 1)  // TopBar mode: Edit/Leave
		{
				if(key.key_val == UPKEY || key.key_val == DOWNKEY)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key.key_val == ENTERKEY)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						
						Top_Bar_Active = 0;
						edit_icon_color = 0;
						leave_icon_color = 0;
						item_selection = 1;
						temp_limit_max = g_parameter.temp_limit_max;
						temp_limit_min = g_parameter.temp_limit_min;
						Draw_TopBar(leave_icon_color, edit_icon_color);
						Draw_Control_Adj_TempLimit_Content(item_selection);
					}
					else  // Leave Icon is red
					{
						// Go back to Control Adj Menu with Leave red
						UI_state = STATE_CONTROL_ADJ_MENU;
						Top_Bar_Active = 0;
						leave_icon_color = 0;
						edit_icon_color = 0;
						item_selection = Find_Control_Adj_Menu_Index(STATE_CONTROL_ADJ_TEMP_LIMIT);
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Control_Adj_Menu_Page(item_selection, leave_icon_color, edit_icon_color);  
					}
					
				}
		}
		else if(item_selection == 1)	//Setup temp max limit
		{
					if(key.key_val == UPKEY)  // Up key pressed
					{
							if(temp_limit_max < DEVICE_OPERATION_TEMPERATURE_MAX){
									temp_limit_max += 1;
									Draw_Control_Adj_TempLimit_Content(item_selection);
							}
					}
					else if(key.key_val == DOWNKEY) 	// Down key pressed
					{
							if(temp_limit_max > 20){			//Max limit can't under 20
									temp_limit_max -= 1;
									Draw_Control_Adj_TempLimit_Content(item_selection);
							}
					}
					else if(key.key_val ==ENTERKEY)		// Enter key pressed
					{
							item_selection = 2;
							Draw_Control_Adj_TempLimit_Content(item_selection);
					}
		}
		else if(item_selection == 2)		//setup temp min limit
		{
					if(key.key_val == UPKEY)  // Up key pressed
					{
							if(temp_limit_min < 10){		//Lower limit can not over 10
									temp_limit_min += 1;
									Draw_Control_Adj_TempLimit_Content(item_selection);
							}
					}
					else if(key.key_val == DOWNKEY) 	// Down key pressed
					{
							if(temp_limit_min > DEVICE_OPERATION_TEMPERATURE_MIN +5){
									temp_limit_min -= 1;
									Draw_Control_Adj_TempLimit_Content(item_selection);
							}
					}
					else if(key.key_val ==ENTERKEY)		// Enter key pressed
					{
							item_selection = 3;
							Draw_Control_Adj_TempLimit_Content(item_selection);
					}
		}
		else if(item_selection == 3)
		{
					if(key.key_val == ENTERKEY)		//skip Up/Down key
					{
							UI_state = STATE_CONTROL_ADJ_MENU;
							Top_Bar_Active = 0;
							leave_icon_color = 0;
							edit_icon_color = 0;
							item_selection = Find_Control_Adj_Menu_Index(STATE_CONTROL_ADJ_TEMP_LIMIT);
							g_parameter.temp_limit_max = temp_limit_max;
							g_parameter.temp_limit_min = temp_limit_min;
							if(Flash_Save_Parameter() != FLASH_OK){
									LOGE("Flash write fail!\r\n");
							}
							LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
							Draw_Control_Adj_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}else if((key.key_val == UPKEY) || (key.key_val  == DOWNKEY)){
							item_selection = 1;
							Draw_Control_Adj_TempLimit_Content(item_selection);
					}
		}
		
	}
	else if(UI_state == STATE_CONTROL_ADJ_TEMP_PROTECT)
	{
		if(Top_Bar_Active == 1)  // TopBar mode: Edit/Leave
		{
				if(key.key_val == UPKEY || key.key_val == DOWNKEY)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key.key_val == ENTERKEY)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter Correct num setting
						Top_Bar_Active = 0;
						edit_icon_color = 0;
						leave_icon_color = 0;
						item_selection = 1;
						Draw_Control_Adj_TempProtect_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					else  // Leave Icon is red
					{
						// Go back to Control Adj Menu with Leave red
						UI_state = STATE_CONTROL_ADJ_MENU;
						Top_Bar_Active = 0;
						leave_icon_color = 0;
						edit_icon_color = 0;
						item_selection = Find_Control_Adj_Menu_Index(STATE_CONTROL_ADJ_TEMP_PROTECT);
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Control_Adj_Menu_Page(item_selection, leave_icon_color, edit_icon_color);  
					}
					
				}
		}
		else 
		{
				if(key.key_val == DOWNKEY)	//Down Key
				{
						if(item_selection < 2){
								item_selection++;
								Draw_Control_Adj_TempProtect_Page(item_selection, leave_icon_color, edit_icon_color);
						}
				}else if(key.key_val == UPKEY){		//Up Key
						if(item_selection == 2){
								item_selection--;
								Draw_Control_Adj_TempProtect_Page(item_selection, leave_icon_color, edit_icon_color);
						}else if(item_selection == 1){		//go back Top_Bar Active
								item_selection = 0;
								leave_icon_color = 1;
								edit_icon_color = 0;
								Top_Bar_Active = 1;
								Draw_Control_Adj_TempProtect_Page(item_selection, leave_icon_color, edit_icon_color);
						}
				}else if(key.key_val == ENTERKEY){		//Enter Key
						if(item_selection == 1){		//Max Temp Protect Setting
								UI_state = STATE_CONTROL_ADJ_TEMP_PROTECT_MAX;
								temp_protect_max = g_parameter.temp_protect_max;
								temp_protect_max_switch = g_parameter.temp_protect_max_switch;
								Top_Bar_Active = 1;
								leave_icon_color = 0;
								edit_icon_color = 1;
								item_selection = 0;
								LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
								Draw_Control_Adj_TempProtect_Max_Page(item_selection, leave_icon_color, edit_icon_color);
						}else if(item_selection == 2){		//Min Temp Protect Setting
								UI_state = STATE_CONTROL_ADJ_TEMP_PROTECT_MIN;
								temp_protect_min = g_parameter.temp_protect_min;
								temp_protect_min_switch = g_parameter.temp_protect_min_switch;
								Top_Bar_Active = 1;
								leave_icon_color = 0;
								edit_icon_color = 1;
								item_selection = 0;
								LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
								Draw_Control_Adj_TempProtect_Min_Page(item_selection, leave_icon_color, edit_icon_color);
						}
				}
		}
	}
	else if(UI_state == STATE_CONTROL_ADJ_TEMP_PROTECT_MAX)
	{
		if(Top_Bar_Active == 1)  // TopBar mode: Edit/Leave
		{
				if(key.key_val == UPKEY || key.key_val == DOWNKEY)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key.key_val == ENTERKEY)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter Correct num setting
						temp_protect_max = g_parameter.temp_protect_max;
						temp_protect_max_switch = g_parameter.temp_protect_max_switch;
						Top_Bar_Active = 0;
						edit_icon_color = 0;
						leave_icon_color = 0;
						item_selection = 1;  // Enter ON/OFF mode first
						Draw_Control_Adj_TempProtect_Max_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					else  // Leave Icon is red
					{
						// Go back to Control Adj Menu with Leave red
						UI_state = STATE_CONTROL_ADJ_TEMP_PROTECT;
						Top_Bar_Active = 0;
						leave_icon_color = 0;
						edit_icon_color = 0;
						item_selection = 1;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Control_Adj_TempProtect_Page(item_selection, leave_icon_color, edit_icon_color);  
					}
					//delay_ms(100);
				}
		}
		else if(item_selection == 1)  // ON/OFF toggle
			{
					if((key.key_val == UPKEY) || (key.key_val == DOWNKEY)){
							if(temp_protect_max_switch){
									temp_protect_max_switch = 0;
							}else{
									temp_protect_max_switch = 1;
							}
							Draw_Control_Adj_TempProtect_Max_Content(item_selection);
					}
					else if(key.key_val == ENTERKEY){
							if(temp_protect_max_switch) {
								item_selection = 2;
							} else {
								item_selection = 3;
							}
							Draw_Control_Adj_TempProtect_Max_Content(item_selection);
					}
			}
			else if(item_selection == 2)  // Number edit
			{
					if(key.key_val == UPKEY){
							if((temp_protect_max < DEVICE_OPERATION_TEMPERATURE_MAX) && (temp_protect_max < g_parameter.temp_limit_max)){
									temp_protect_max++;
									 Draw_Control_Adj_TempProtect_Max_Content(item_selection);
							}
					}else if(key.key_val == DOWNKEY){
							if(temp_protect_max > MIN_TEMP_HIGH_TEMP_PROTECT){
									temp_protect_max--;
									Draw_Control_Adj_TempProtect_Max_Content(item_selection);
							}
					}else if(key.key_val == ENTERKEY){
							item_selection = 3;
							Draw_Control_Adj_TempProtect_Max_Content(item_selection);
					}
			}
else if(item_selection == 3)
		{
				if(key.key_val == ENTERKEY){
						UI_state = STATE_CONTROL_ADJ_TEMP_PROTECT;
						Top_Bar_Active = 0;
						leave_icon_color = 0;
						edit_icon_color = 0;
						item_selection = 1;
						g_parameter.temp_protect_max = temp_protect_max;
						g_parameter.temp_protect_max_switch = temp_protect_max_switch;
						if(Flash_Save_Parameter() != FLASH_OK){
								LOGE("Flash write fail!\r\n");
						}
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Control_Adj_TempProtect_Page(item_selection, leave_icon_color, edit_icon_color);
				}else if((key.key_val == UPKEY) || (key.key_val == DOWNKEY)){
						item_selection = 1;
						Draw_Control_Adj_TempProtect_Max_Content(item_selection);
				}
		}
	}
	else if(UI_state == STATE_CONTROL_ADJ_TEMP_PROTECT_MIN)
	{
		if(Top_Bar_Active == 1)  // TopBar mode: Edit/Leave
		{
				if(key.key_val == UPKEY || key.key_val == DOWNKEY)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key.key_val == ENTERKEY)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter Correct num setting
						temp_protect_min = g_parameter.temp_protect_min;
						temp_protect_min_switch = g_parameter.temp_protect_min_switch;
						Top_Bar_Active = 0;
						edit_icon_color = 0;
						leave_icon_color = 0;
						item_selection = 1;  // Enter ON/OFF mode first
						Draw_Control_Adj_TempProtect_Min_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					else  // Leave Icon is red
					{
						// Go back to Control Adj Menu with Leave red
						UI_state = STATE_CONTROL_ADJ_TEMP_PROTECT;
						Top_Bar_Active = 0;
						leave_icon_color = 0;
						edit_icon_color = 0;
						item_selection = 2;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Control_Adj_TempProtect_Page(item_selection, leave_icon_color, edit_icon_color);  
					}
					//delay_ms(100);
				}
		}
		else if(item_selection == 1)  // ON/OFF toggle
			{
					if((key.key_val == UPKEY) || (key.key_val == DOWNKEY)){
							temp_protect_min_switch = (temp_protect_min_switch == 1) ? 0 : 1;
							Draw_Control_Adj_TempProtect_Min_Content(item_selection);
					}
					else if(key.key_val == ENTERKEY){
							if(temp_protect_min_switch) {
								item_selection = 2;
							} else {
								item_selection = 3;
							}
							Draw_Control_Adj_TempProtect_Min_Content(item_selection);
					}
			}
			else if(item_selection == 2)  // Number edit
			{
					if(key.key_val == UPKEY){
							if(temp_protect_min < MAX_TEMP_LOW_TEMP_PROTECT){
									temp_protect_min++;
									 Draw_Control_Adj_TempProtect_Min_Content(item_selection);
							}
					}else if(key.key_val == DOWNKEY){
							if(temp_protect_min > (DEVICE_OPERATION_TEMPERATURE_MIN + 5) && (temp_protect_min > g_parameter.temp_limit_min)){
									temp_protect_min--;
									Draw_Control_Adj_TempProtect_Min_Content(item_selection);
							}
					}else if(key.key_val == ENTERKEY){
							item_selection = 3;
							Draw_Control_Adj_TempProtect_Min_Content(item_selection);
					}
			}
else if(item_selection == 3)
		{
				if(key.key_val == ENTERKEY){
						UI_state = STATE_CONTROL_ADJ_TEMP_PROTECT;
						Top_Bar_Active = 0;
						leave_icon_color = 0;
						edit_icon_color = 0;
						item_selection = 2;
						g_parameter.temp_protect_min = temp_protect_min;
						g_parameter.temp_protect_min_switch = temp_protect_min_switch;
						if(Flash_Save_Parameter() != FLASH_OK){
								LOGE("Flash write fail!\r\n");
						}
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Control_Adj_TempProtect_Page(item_selection, leave_icon_color, edit_icon_color);
				}else if((key.key_val == UPKEY) || (key.key_val == DOWNKEY)){
						item_selection = 1;
						Draw_Control_Adj_TempProtect_Min_Content(item_selection);
				}
		}
	}
	else if(UI_state == STATE_CONTROL_ADJ_POWER_ON_STATE)
	{
		if(Top_Bar_Active == 1)  // TopBar mode: Edit/Leave
		{
				if(key.key_val == UPKEY || key.key_val == DOWNKEY)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key.key_val == ENTERKEY)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter Correct num setting
						Top_Bar_Active = 0;
						edit_icon_color = 0;
						leave_icon_color = 0;
						item_selection = 1;
						Draw_Control_Adj_Power_On_State_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					else  // Leave Icon is red
					{
						
						UI_state = STATE_CONTROL_ADJ_MENU;
						Top_Bar_Active = 0;
						leave_icon_color = 0;
						edit_icon_color = 0;
						item_selection = Find_Control_Adj_Menu_Index(STATE_CONTROL_ADJ_POWER_ON_STATE);
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Control_Adj_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					
				}
		}
		else if(item_selection == 1)
		{
				if(key.key_val == UPKEY){		//Up key
						if(power_on_state == 1){	//go back to Top Bar
								Top_Bar_Active = 1;
								leave_icon_color = 1;
								edit_icon_color = 0;
								item_selection = 0;
								Draw_Control_Adj_Power_On_State_Page(item_selection, leave_icon_color, edit_icon_color);
						}else{
								power_on_state--;
								Draw_Control_Adj_Power_On_State_Arrow(item_selection);
						}
				}else if(key.key_val == DOWNKEY){
						if(power_on_state < 3){
								power_on_state++;
								Draw_Control_Adj_Power_On_State_Arrow(item_selection);
						}
				}else if(key.key_val == ENTERKEY){
						//power_on_state = item_selection;
						item_selection = 2;
						Draw_Control_Adj_Power_On_State_Arrow(item_selection);
				}
		}else if(item_selection == 2){
				if(key.key_val == ENTERKEY){
						UI_state = STATE_CONTROL_ADJ_MENU;
						Top_Bar_Active = 0;
						leave_icon_color = 0;
						edit_icon_color = 0;
						item_selection = Find_Control_Adj_Menu_Index(STATE_CONTROL_ADJ_POWER_ON_STATE);
						g_parameter.power_on_state = power_on_state;
						if(Flash_Save_Parameter() != FLASH_OK){
								LOGE("Flash write fail!\r\n");
						}
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Control_Adj_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
				}else if((key.key_val == UPKEY) || (key.key_val == DOWNKEY)){
						item_selection = 1;
						Draw_Control_Adj_Power_On_State_Page(item_selection, leave_icon_color, edit_icon_color);
				}
		}
	}
	else if(UI_state == STATE_USER_SETTING_MENU)
	{
			// Control Adj Menu page navigation
			if(Top_Bar_Active)  // TopBar mode: Edit/Leave
			{
				if(key.key_val == UPKEY || key.key_val == DOWNKEY)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key.key_val == ENTERKEY)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter menu selection, Edit/Leave both black, Sensor selected
						Top_Bar_Active = 0;
						item_selection = 1;
						leave_icon_color = 0;
						edit_icon_color = 0;
						control_adj_menu_scroll = 0;  // Reset scroll
						Draw_User_Setting_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					else  // Leave Icon is red
					{
						// Go back to Function Setting Edit mode with cursor on Control Adj
						UI_state = STATE_FUNC_SETTING;
						func_setting_scroll = 0;  // Reset scroll
						Top_Bar_Active = 0;
						item_selection = 4;
						leave_icon_color = 0;
						edit_icon_color = 0;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Function_Setting_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					
				}
			}
			else //if(control_adj_menu_substate == 1)  // Child lock / Window function / Set time / Backlight / Reset
			{
				if(key.key_val == DOWNKEY)  // Down key pressed
				{
					if(item_selection < User_Setting_Menu_Item_Count)  // Can go down
					{
						uint8_t old_sel = item_selection;
						uint8_t old_scroll = user_setting_menu_scroll;
						item_selection++;
						
						// Check if scroll position needs to change
						// 4 visible rows at a time, scroll when selection goes below visible area
						if(item_selection > user_setting_menu_scroll + 4) {
							user_setting_menu_scroll = item_selection - 4;
						}
						
						// If scroll changed, redraw entire page
						if(old_scroll != user_setting_menu_scroll) {
							Draw_User_Setting_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
						} else {
							// Only update changed rows
							Draw_User_Setting_Menu_Row((old_sel - 1) - user_setting_menu_scroll, 0);  // Old row unselected
							Draw_User_Setting_Menu_Row((item_selection - 1) - user_setting_menu_scroll, 1);  // New row selected
						}
					}
					
				}
				else if(key.key_val == UPKEY)  // Up key pressed
				{
					if(item_selection > 1)
					{
						uint8_t old_sel = item_selection;
						uint8_t old_scroll = user_setting_menu_scroll;
						item_selection--;
						
						// Check if scroll position needs to change
						if(item_selection - 1 < user_setting_menu_scroll + 1) {
							user_setting_menu_scroll = 0;
						}
						
						// If scroll changed, redraw entire page
						if(old_scroll != user_setting_menu_scroll) {
							Draw_User_Setting_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
						} else {
							// Only update changed rows
							Draw_User_Setting_Menu_Row((old_sel -1) - user_setting_menu_scroll, 0);  // Old row unselected
							Draw_User_Setting_Menu_Row((item_selection -1) - user_setting_menu_scroll, 1);  // New row selected
						}
					}
					else  // At Sensor (sel=0), go back to TopBar mode
					{
						Top_Bar_Active = 1;
						leave_icon_color = 1;
						edit_icon_color = 0;
						item_selection = 0;
						// Redraw entire page to show TopBar with Edit red
						Draw_User_Setting_Menu_Page(item_selection, leave_icon_color, edit_icon_color);  // 0xFF = TopBar mode, Edit red
					}
					
				}
				else if(key.key_val == ENTERKEY)  // Enter key pressed
				{
					uint8_t user_menu_idx = item_selection - 1;
					if(user_menu_idx < User_Setting_Menu_Item_Count)
					{
						UI_state = User_Setting_Menu_Items[user_menu_idx].next_state;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);

						if(UI_state == STATE_USER_SETTING_CHILD_LOCK)
						{
							Top_Bar_Active = 1;
							item_selection = 0;
							leave_icon_color = 0;
							edit_icon_color = 1;
							child_lock = g_parameter.child_lock;
							Draw_User_Setting_Child_Lock_Page(item_selection, leave_icon_color, edit_icon_color);
						}
						else if(UI_state == STATE_USER_SETTING_WINDOW_FUN)
						{
							Top_Bar_Active = 1;
							item_selection = 0;
							leave_icon_color = 0;
							edit_icon_color = 1;
							window_fun = g_parameter.window_fun;
							window_fun_temp = g_parameter.window_fun_temp;
							window_fun_time = g_parameter.window_fun_time;
							Draw_User_Setting_Window_Fun_Page(item_selection, leave_icon_color, edit_icon_color);
						}
						else if(UI_state == STATE_USER_SETTING_SET_TIME)
						{
							Top_Bar_Active = 1;
							item_selection = 0;
							leave_icon_color = 0;
							edit_icon_color = 1;
							temp_rtc.Year = rtc_time.Year;
							temp_rtc.Mon = rtc_time.Mon;
							temp_rtc.Date = rtc_time.Date;
							temp_rtc.Hour = rtc_time.Hour;
							temp_rtc.Min = rtc_time.Min;
							Draw_User_Setting_SetTime_Page(item_selection, leave_icon_color, edit_icon_color);
						}
						else if(UI_state == STATE_USER_SETTING_BACKLIGHT)
						{
							Top_Bar_Active = 1;
							item_selection = 0;
							leave_icon_color = 0;
							edit_icon_color = 1;
							sleep_backlight_duty = g_parameter.sleep_backlight_duty;
							Draw_User_Setting_Backlight_Page(item_selection, leave_icon_color, edit_icon_color);
						}
						else if(UI_state == STATE_USER_SETTING_RESET)
						{
							Top_Bar_Active = 1;
							item_selection = 0;
							leave_icon_color = 0;
							edit_icon_color = 1;
							Draw_User_Setting_Reset_Page(item_selection, leave_icon_color, edit_icon_color);
						}
						else if(UI_state == STATE_USER_SETTING_COMFORT_MODE)
						{
							Top_Bar_Active = 1;
							item_selection = 0;
							leave_icon_color = 0;
							edit_icon_color = 1;
							comfort_mode = g_parameter.comfort_mode;
							floor_material = g_parameter.floor_material;
							Draw_Control_Adj_Comfort_Mode_Page(item_selection, leave_icon_color, edit_icon_color);
						}
						else if(UI_state == STATE_USER_SETTING_LANGUAGE)
						{
							Top_Bar_Active = 1;
							item_selection = 0;
							leave_icon_color = 0;
							edit_icon_color = 1;
							language_temp = g_parameter.language;
							Draw_User_Setting_Language_Page(item_selection, leave_icon_color, edit_icon_color);
						}
						else if(UI_state == STATE_USER_SETTING_ADAPTIVE_START)
						{
							Top_Bar_Active = 1;
							item_selection = 0;
							leave_icon_color = 0;
							edit_icon_color = 1;
							adaptive_start = g_parameter.adaptive_start;
							Draw_User_Setting_Adaptive_Start_Page(item_selection, leave_icon_color, edit_icon_color);
						}
					}
				}
			}
	}
	else if(UI_state == STATE_USER_SETTING_CHILD_LOCK)
	{
		// User setting child lock page navigation
			if(Top_Bar_Active)  // TopBar mode: Edit/Leave
			{
				if(key.key_val == UPKEY || key.key_val == DOWNKEY)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key.key_val == ENTERKEY)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter Child Lock edit mode
						Top_Bar_Active = 0;
						item_selection = 1;
						leave_icon_color = 0;
						edit_icon_color = 0;
						//child_lock = g_parameter.child_lock;
						Draw_User_Setting_Child_Lock_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					else  // Leave Icon is red
					{
						// Go back to Function Setting Edit mode with cursor on Control Adj
						UI_state = STATE_USER_SETTING_MENU;
						Top_Bar_Active = 0;
						item_selection = Find_User_Setting_Menu_Index(STATE_USER_SETTING_CHILD_LOCK);
						leave_icon_color = 0;
						edit_icon_color = 0;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_User_Setting_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					
				}
			}
			else if((item_selection == 1) || (item_selection == 2))
			{
					if(key.key_val == DOWNKEY){		//Down key
							if(item_selection == 1){
									item_selection = 2;
									Draw_User_Setting_Child_Lock_Choice(item_selection);
							}
							
					}else if(key.key_val == UPKEY){		//Up key
							if(item_selection == 2){
									item_selection = 1;
									Draw_User_Setting_Child_Lock_Choice(item_selection);
							}else if(item_selection == 1){	//go back Top_Bar
									Top_Bar_Active = 1;
									item_selection = 0;
									leave_icon_color = 0;
									edit_icon_color = 1;
									Draw_User_Setting_Child_Lock_Page(item_selection, leave_icon_color, edit_icon_color);
							}
							
					}else if(key.key_val == ENTERKEY){		//Enter key
							child_lock = (item_selection == 1) ? 0: 1;
							item_selection = 3;
							Draw_User_Setting_Child_Lock_Page(item_selection, leave_icon_color, edit_icon_color);
							
					}
			}else if(item_selection == 3){
					if(key.key_val == ENTERKEY){
							g_parameter.child_lock = child_lock;
							if(child_lock == 1){
									// Child lock ON - enter PIN setup
									UI_state = STATE_CHILD_LOCK_PIN_SETUP;
									pin_setup_stage = 0;  // First stage: enter PIN
									pin_digit_index = 0;
									pin_digit_selected = 0;
									Clear_Pin_Input();
									LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
									Draw_Child_Lock_Pin_Page(pin_setup_stage);
							}else{
									// Child lock OFF - save and return
									child_lock_flag = 0;				//Release child lock flag
									UI_state = STATE_USER_SETTING_MENU;
									Top_Bar_Active = 0;
									item_selection = Find_User_Setting_Menu_Index(STATE_USER_SETTING_CHILD_LOCK);
									leave_icon_color = 0;
									edit_icon_color = 0;
									if(Flash_Save_Parameter() != FLASH_OK){
											LOGE("Flash write fail!\r\n");
									}
									LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
									Draw_User_Setting_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
							}
					}else if((key.key_val == UPKEY) || (key.key_val == DOWNKEY)){
							item_selection = 1;
							Draw_User_Setting_Child_Lock_Page(item_selection, leave_icon_color, edit_icon_color);
					}
			}
	}
	else if(UI_state == STATE_USER_SETTING_WINDOW_FUN)
	{
			if(Top_Bar_Active)  // TopBar mode: Edit/Leave
			{
				if(key.key_val == UPKEY || key.key_val == DOWNKEY)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key.key_val == ENTERKEY)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter Window Function edit mode
						Top_Bar_Active = 0;
						item_selection = 1;
						leave_icon_color = 0;
						edit_icon_color = 0;
						Draw_User_Setting_Window_Fun_Content(item_selection, leave_icon_color, edit_icon_color);
					}
					else  // Leave Icon is red
					{
						// Go back to Function Setting Edit mode with cursor on Control Adj
						UI_state = STATE_USER_SETTING_MENU;
						Top_Bar_Active = 0;
						item_selection = Find_User_Setting_Menu_Index(STATE_USER_SETTING_WINDOW_FUN);
						leave_icon_color = 0;
						edit_icon_color = 0;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_User_Setting_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					//delay_ms(100);
				}
			}else if(item_selection == 1){
					if((key.key_val == DOWNKEY) || (key.key_val == UPKEY)){		
							if(window_fun){
									window_fun = 0;
							}else{
									window_fun = 1;
							}
							Draw_User_Setting_Window_Fun_Content(item_selection, leave_icon_color, edit_icon_color);
					}
					if(key.key_val == ENTERKEY){
							if(window_fun){
									item_selection = 2;
									Draw_User_Setting_Window_Fun_Content(item_selection, leave_icon_color, edit_icon_color);
							}else{
									UI_state = STATE_USER_SETTING_MENU;
									Top_Bar_Active = 0;
									item_selection = Find_User_Setting_Menu_Index(STATE_USER_SETTING_WINDOW_FUN);
									leave_icon_color = 0;
									edit_icon_color = 0;
									g_parameter.window_fun = window_fun;
									if(Flash_Save_Parameter() != FLASH_OK){
											LOGE("Flash write fail!\r\n");
									}
									LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
									Draw_User_Setting_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
							}
					}
			}else if(item_selection == 2){
					if(key.key_val == ENTERKEY){
							UI_state = STATE_USER_SETTING_WINDOW_TIME;
							Top_Bar_Active = 1;
							item_selection = 0;
							leave_icon_color = 0;
							edit_icon_color = 1;
							//window_fun = g_parameter.window_fun;
							//window_fun_temp = g_parameter.window_fun_temp;
							//window_fun_time = g_parameter.window_fun_time;
							LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
							Draw_User_Setting_Window_Time_Page(item_selection, leave_icon_color, edit_icon_color);
					}
			}
	}
	else if(UI_state == STATE_USER_SETTING_WINDOW_TIME){
			if(Top_Bar_Active)  // TopBar mode: Edit/Leave
			{
				if(key.key_val == UPKEY || key.key_val == DOWNKEY)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key.key_val == ENTERKEY)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						
						Top_Bar_Active = 0;
						item_selection = 1;
						leave_icon_color = 0;
						edit_icon_color = 0;
						window_fun_temp = g_parameter.window_fun_temp;
						window_fun_time = g_parameter.window_fun_time;
						Draw_User_Setting_Window_Time_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					else  // Leave Icon is red
					{
						
						UI_state = STATE_USER_SETTING_WINDOW_FUN;
						Top_Bar_Active = 0;
						item_selection = 1;			//go back to ON/OFF selection
						leave_icon_color = 0;
						edit_icon_color = 0;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_User_Setting_Window_Fun_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					
				}
			}else if(item_selection == 1){
					if(key.key_val == DOWNKEY){		//Down key
							if(window_fun_temp > 3){
									window_fun_temp--;
									Draw_User_Setting_Window_Time_Content(item_selection);
							}
					}else if(key.key_val == UPKEY){		//Up key
							if(window_fun_temp < 20){
									window_fun_temp++;
									Draw_User_Setting_Window_Time_Content(item_selection);
							}
					}else if(key.key_val == ENTERKEY){		//Enter key
							item_selection = 2;
							Draw_User_Setting_Window_Time_Content(item_selection);
					}
			}else if(item_selection == 2){
					if(key.key_val == DOWNKEY){		//Down key
							if(window_fun_time > 5){
									window_fun_time--;
									Draw_User_Setting_Window_Time_Content(item_selection);
							}
					}else if(key.key_val == UPKEY){		//Up key
							if(window_fun_time < 60){
									window_fun_time++;
									Draw_User_Setting_Window_Time_Content(item_selection);
							}
					}else if(key.key_val == ENTERKEY){		//Enter key
							item_selection = 3;
							Draw_User_Setting_Window_Time_Content(item_selection);
					}
			}else if(item_selection == 3){
					if(key.key_val == ENTERKEY){
							UI_state = STATE_USER_SETTING_MENU;
							Top_Bar_Active = 0;
							item_selection = Find_User_Setting_Menu_Index(STATE_USER_SETTING_WINDOW_FUN);
							leave_icon_color = 0;
							edit_icon_color = 0;
							g_parameter.window_fun = window_fun;
							g_parameter.window_fun_temp = window_fun_temp;
							g_parameter.window_fun_time = window_fun_time;
							if(Flash_Save_Parameter() != FLASH_OK){
									LOGE("Flash write fail!\r\n");
							}
							LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
							Draw_User_Setting_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}else if((key.key_val == UPKEY) || (key.key_val == DOWNKEY)){
							item_selection = 1;
							Draw_User_Setting_Window_Time_Content(item_selection);
					}
			}
	}
	else if(UI_state == STATE_USER_SETTING_SET_TIME)
	{
			if(Top_Bar_Active)  // TopBar mode: Edit/Leave
			{
				if(key.key_val == UPKEY || key.key_val == DOWNKEY)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key.key_val == ENTERKEY)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						Top_Bar_Active = 0;
						item_selection = 1;
						leave_icon_color = 0;
						edit_icon_color = 0;
						Draw_User_Setting_SetTime_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					else  // Leave Icon is red
					{
						// Go back to Function Setting Edit mode with cursor on Control Adj
						UI_state = STATE_USER_SETTING_MENU;
						Top_Bar_Active = 0;
						item_selection = Find_User_Setting_Menu_Index(STATE_USER_SETTING_SET_TIME);
						leave_icon_color = 0;
						edit_icon_color = 0;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_User_Setting_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					
				}
			}else if(item_selection == 1){
					if(key.key_val == DOWNKEY){		//Down key
							if(temp_rtc.Year > 26){
									temp_rtc.Year--;
									Draw_User_Setting_SetTime_Content(item_selection);
							}
					}else if(key.key_val == UPKEY){		//Up key
							if(temp_rtc.Year < 99){
									temp_rtc.Year++;
									Draw_User_Setting_SetTime_Content(item_selection);
							}
					}else if(key.key_val == ENTERKEY){		//Enter key
							item_selection = 2;
							Draw_User_Setting_SetTime_Content(item_selection);
					}
			}else if(item_selection == 2){
					if(key.key_val == DOWNKEY){		//Down key
							if(temp_rtc.Mon > 1){
									temp_rtc.Mon--;
									Draw_User_Setting_SetTime_Content(item_selection);
							}
					}else if(key.key_val == UPKEY){		//Up key
							if(temp_rtc.Mon < 12){
									temp_rtc.Mon++;
									Draw_User_Setting_SetTime_Content(item_selection);
							}
					}else if(key.key_val == ENTERKEY){		//Enter key
							item_selection = 3;
							Draw_User_Setting_SetTime_Content(item_selection);
					}
			}else if(item_selection == 3){
					if(key.key_val == DOWNKEY){		//Down key
							if(temp_rtc.Date > 1){
									temp_rtc.Date--;
									Draw_User_Setting_SetTime_Content(item_selection);
							}
					}else if(key.key_val == UPKEY){		//Up key
							if((temp_rtc.Mon == 1) || (temp_rtc.Mon == 3) || (temp_rtc.Mon == 5) || (temp_rtc.Mon == 7) || (temp_rtc.Mon == 8) || (temp_rtc.Mon == 10) || (temp_rtc.Mon == 12)){
									if(temp_rtc.Date < 31){
											temp_rtc.Date++;
											Draw_User_Setting_SetTime_Content(item_selection);
									}
							}else if((temp_rtc.Mon == 4) || (temp_rtc.Mon == 6) || (temp_rtc.Mon == 9) || (temp_rtc.Mon == 11)){
									if(temp_rtc.Date < 30){
											temp_rtc.Date++;
											Draw_User_Setting_SetTime_Content(item_selection);
									}
							}else if(temp_rtc.Mon == 2){
									if((temp_rtc.Year%4) == 0){
											if(temp_rtc.Date < 29){
													temp_rtc.Date++;
											}
									}else{
											if(temp_rtc.Date < 28){
													temp_rtc.Date++;
											}
									}
									Draw_User_Setting_SetTime_Content(item_selection);
							}
					}else if(key.key_val == ENTERKEY){		//Enter key
							item_selection = 4;
							Draw_User_Setting_SetTime_Content(item_selection);
					}
			}else if(item_selection == 4){
					if((key.key_val == UPKEY) || (key.key_val == DOWNKEY)){
							item_selection = 1;
							Draw_User_Setting_SetTime_Content(item_selection);
							//lan_str = LanguageTable[STR_Set_Clock][g_parameter.language];
							//Show_Str(87, 108, BLACK, WHITE, "Set Clock", 16, 0);
							Show_Str(87, 108, BLACK, WHITE, (char*)LanguageTable[STR_Set_Clock][g_parameter.language], 16, 0);
					}else if(key.key_val == ENTERKEY){
							UI_state = STATE_USER_SETTING_SET_CLK;
							Top_Bar_Active = 1;
							item_selection = 0;
							leave_icon_color = 0;
							edit_icon_color = 1;
							LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
							Draw_User_Setting_Setclk_Page(item_selection, leave_icon_color, edit_icon_color);
					}
			}
	}
	else if(UI_state == STATE_USER_SETTING_SET_CLK)
	{
		if(Top_Bar_Active)  // TopBar mode: Edit/Leave
			{
				if(key.key_val == UPKEY || key.key_val == DOWNKEY)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key.key_val == ENTERKEY)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						Top_Bar_Active = 0;
						item_selection = 1;
						leave_icon_color = 0;
						edit_icon_color = 0;
						Draw_User_Setting_Setclk_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					else  // Leave Icon is red
					{
						
						UI_state = STATE_USER_SETTING_SET_TIME;
						Top_Bar_Active = 1;
						item_selection = 0;
						leave_icon_color = 1;
						edit_icon_color = 0;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_User_Setting_SetTime_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					
				}
			}else if(item_selection == 1){
					if(key.key_val == DOWNKEY){		//Down key
							if(temp_rtc.Hour > 0){
									temp_rtc.Hour--;
									Draw_User_Setting_Setclk_Content(item_selection);
							}
					}else if(key.key_val == UPKEY){		//Up key
							if(temp_rtc.Hour < 23){
									temp_rtc.Hour++;
									Draw_User_Setting_Setclk_Content(item_selection);
							}
					}else if(key.key_val == ENTERKEY){		//Enter key
							item_selection = 2;
							Draw_User_Setting_Setclk_Content(item_selection);
					}
			}else if(item_selection == 2){
					if(key.key_val == DOWNKEY){		//Down key
							if(temp_rtc.Min > 0){
									temp_rtc.Min--;
									Draw_User_Setting_Setclk_Content(item_selection);
							}
					}else if(key.key_val == UPKEY){		//Up key
							if(temp_rtc.Min < 60){
									temp_rtc.Min++;
									Draw_User_Setting_Setclk_Content(item_selection);
							}
					}else if(key.key_val == ENTERKEY){		//Enter key
							item_selection = 3;
							Draw_User_Setting_Setclk_Content(item_selection);
					}
			}else if(item_selection == 3){
					if((key.key_val == UPKEY) || (key.key_val == DOWNKEY)){
							item_selection = 1;
							Draw_User_Setting_Setclk_Content(item_selection);
					}else if(key.key_val == ENTERKEY){
							UI_state = STATE_USER_SETTING_MENU;
							Top_Bar_Active = 0;
							item_selection = Find_User_Setting_Menu_Index(STATE_USER_SETTING_SET_TIME);
							leave_icon_color = 0;
							edit_icon_color = 0;
							LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
							Draw_User_Setting_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
							rtc_time.Year = temp_rtc.Year;
							rtc_time.Mon = temp_rtc.Mon;
							rtc_time.Date = temp_rtc.Date;
							rtc_time.Hour = temp_rtc.Hour;
							rtc_time.Min = temp_rtc.Min;
							rtc_time.Sec = 0;
							RTC_SetTime(&rtc_time);
							weekday = getWeekday(rtc_time.Year, rtc_time.Mon, rtc_time.Date);
					}
			}
	}
	else if(UI_state == STATE_USER_SETTING_BACKLIGHT){
			if(Top_Bar_Active)  // TopBar mode: Edit/Leave
			{
				if(key.key_val == UPKEY || key.key_val == DOWNKEY)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key.key_val == ENTERKEY)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter Backlight edit mode
						Top_Bar_Active = 0;
						item_selection = 1;
						leave_icon_color = 0;
						edit_icon_color = 0;
						sleep_backlight_duty = g_parameter.sleep_backlight_duty;
						Draw_User_Setting_Backlight_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					else  // Leave Icon is red
					{
						// Go back to Function Setting Edit mode with cursor on Control Adj
						UI_state = STATE_USER_SETTING_MENU;
						Top_Bar_Active = 0;
						item_selection = Find_User_Setting_Menu_Index(STATE_USER_SETTING_BACKLIGHT);
						leave_icon_color = 0;
						edit_icon_color = 0;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_User_Setting_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					//delay_ms(100);
				}
			}else if(item_selection == 1){
					if(key.key_val == DOWNKEY){		//Down key
							if(sleep_backlight_duty > 0){
									sleep_backlight_duty -= 10;
									Draw_User_Setting_Backlight_Content(item_selection);
							}
					}else if(key.key_val == UPKEY){		//Up key
							if(sleep_backlight_duty < 50){
									sleep_backlight_duty += 10;
									Draw_User_Setting_Backlight_Content(item_selection);
							}
					}else if(key.key_val == ENTERKEY){		//Enter key
							item_selection = 2;
							Draw_User_Setting_Backlight_Content(item_selection);
					}
					
			}else if(item_selection == 2){
					if((key.key_val == UPKEY) || (key.key_val == DOWNKEY)){
							item_selection = 1;
							Draw_User_Setting_Backlight_Content(item_selection);
					}else if(key.key_val == ENTERKEY){
							UI_state = STATE_USER_SETTING_MENU;
							item_selection = Find_User_Setting_Menu_Index(STATE_USER_SETTING_BACKLIGHT);
							Top_Bar_Active = 0;
							leave_icon_color = 0;
							edit_icon_color = 0;
							g_parameter.sleep_backlight_duty = sleep_backlight_duty;
							if(Flash_Save_Parameter() != FLASH_OK){
									LOGE("Flash write fail!\r\n");
							}
							LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
							Draw_User_Setting_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}
			}
	}
	else if(UI_state == STATE_USER_SETTING_RESET)
	{
			if(Top_Bar_Active)  // TopBar mode: Edit/Leave
			{
				if(key.key_val == UPKEY || key.key_val == DOWNKEY)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key.key_val == ENTERKEY)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						
						Top_Bar_Active = 0;
						item_selection = 1;
						leave_icon_color = 0;
						edit_icon_color = 0;
						Draw_User_Setting_Reset_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					else  // Leave Icon is red
					{
						// Go back to Function Setting Edit mode with cursor on Control Adj
						UI_state = STATE_USER_SETTING_MENU;
						Top_Bar_Active = 0;
						item_selection = Find_User_Setting_Menu_Index(STATE_USER_SETTING_RESET);
						leave_icon_color = 0;
						edit_icon_color = 0;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_User_Setting_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					
				}
			}else if(item_selection == 1){
					if((key.key_val == UPKEY) || (key.key_val == DOWNKEY)){
							Top_Bar_Active = 1;
							item_selection = 0;
							leave_icon_color = 1;
							edit_icon_color = 0;
							Draw_User_Setting_Reset_Page(item_selection, leave_icon_color, edit_icon_color);
					}else if(key.key_val == ENTERKEY){
							UI_state = STATE_ACTIVE;
							Top_Bar_Active = 0;
							item_selection = 0;
							leave_icon_color = 0;
							edit_icon_color = 0;
							golbal_par_init();			//Set golbal parameter to defaul value
							if(Flash_Save_Parameter() != FLASH_OK){
									LOGE("Flash write fail!\r\n");
							}
							main_display_digi = Display_Room_Temp;
							LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
							Backlight_SetDuty(BACKLIGHT_DUTY_ACTIVE);
							delete_alarm(Setting_Menu_Alarm);
							if(register_alarm(Active_Alarm, ACTIVE_ALIVE_TIME) == eFALSE){
									LOGE("Alarm Fail\r\n");
							}
							Draw_Active_Menu();
					}
			}
	}
	else if(UI_state == STATE_CHILD_LOCK_PIN_SETUP)
	{
		// PIN setup state - handle Up/Down/Enter for digit selection
		Handle_Pin_Input(key.key_val);
	}
	else if(UI_state == STATE_CHILD_LOCK_PIN_VERIFY)
	{
		// PIN verification state (when waking from sleep with child lock enabled)
		if(key.key_val == UPKEY){
			if(pin_digit_selected < 9){
				pin_digit_selected++;
				Draw_Pin_Digit_Selector(pin_digit_selected);
			}
			//update_alarm(Setting_Menu_Alarm);
		}else if(key.key_val == DOWNKEY){
			if(pin_digit_selected > 0){
				pin_digit_selected--;
				Draw_Pin_Digit_Selector(pin_digit_selected);
			}
			//update_alarm(Setting_Menu_Alarm);
		}else if(key.key_val == ENTERKEY){
			//update_alarm(Setting_Menu_Alarm);
			// Enter the selected digit
			pin_code_input[pin_digit_index] = pin_digit_selected;
			pin_digit_index++;
			
			if(pin_digit_index >= 4){
				// All 4 digits entered - verify
				if(Verify_Pin_Code()){
					// PIN correct - return to Active state
					UI_state = STATE_ACTIVE;
					pin_digit_index = 0;
					pin_digit_selected = 0;
					Clear_Pin_Input();
					delete_alarm(Setting_Menu_Alarm);
					child_lock_flag = 0;
					//if(register_alarm(Active_Alarm, ACTIVE_ALIVE_TIME) == eFALSE){
					//	LOGE("Alarm Full!\r\n");
					//}
					Backlight_SetDuty(BACKLIGHT_DUTY_ACTIVE);
					main_display_digi = Display_Room_Temp;
					LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
					if(register_alarm(Active_Alarm, ACTIVE_ALIVE_TIME) == eFALSE){
							LOGE("Alarm Fail\r\n");
					}
					Draw_Active_Menu();
				}else{
					// PIN incorrect - clear and stay in verify state
					pin_digit_index = 0;
					pin_digit_selected = 0;
					Clear_Pin_Input();
					// Redraw to show cleared input
					LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
					POINT_COLOR = BLACK;
					BACK_COLOR = WHITE;
					//lan_str = LanguageTable[STR_Enter_Pin_Code][g_parameter.language];
					//Show_Str(10, 10, BLACK, WHITE, "Enter Pin Code", 16, 0);
					Show_Str(10, 10, BLACK, WHITE, (char*)LanguageTable[STR_Enter_Pin_Code][g_parameter.language], 16, 0);
					Draw_Pin_Input_Box(pin_code_input, pin_digit_index);
					Draw_Pin_Digit_Selector(pin_digit_selected);
				}
			}else{
				// Update display with new digit
				Draw_Pin_Input_Box(pin_code_input, pin_digit_index);
			}
		}
	}
	else if(UI_state == STATE_LEAVE_SCHEDULE_CONFIRM)
	{
		// Leave Schedule Mode confirmation dialog
		if(key.key_val == UPKEY || key.key_val == DOWNKEY){
			// Toggle between Cancel (0) and Yes (1)
			leave_schedule_confirm_selection = (leave_schedule_confirm_selection == 0) ? 1 : 0;
			//update_alarm(Setting_Menu_Alarm);
			Draw_Leave_Schedule_Confirm_Page(leave_schedule_confirm_selection);
		}else if(key.key_val == ENTERKEY){
			//update_alarm(Setting_Menu_Alarm);
			if(leave_schedule_confirm_selection == 1){
				// Yes selected - cancel this schedule section till next schedule section
				//g_parameter.operation_mode = 0;  // Switch to Manual mode
				//if(Flash_Save_Parameter() != FLASH_OK){
				//	LOGE("Flash write fail!\r\n");
				//}
				
				// Transition to Setting state
				delete_alarm(Setting_Menu_Alarm);
				UI_state = STATE_SETTING;
				//LOGD("UI: %d\r\n", UI_state);
				temp_int = g_parameter.setting_number;
				
				
				// Clear and redraw full Active menu, then highlight setting number
				LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
				main_display_digi = Display_Setting_Temp;
				g_parameter.operation_mode = 0;				//Change to manual mode
				Backlight_SetDuty(BACKLIGHT_DUTY_ACTIVE);
				if(register_alarm(Setting_Digi_Alarm, DIGI_UPDATE_TIME) == eFALSE){
						LOGE("Alarm Full!\r\n");
				}
				Draw_Active_Menu();
				//LOGD("Disp:%.1f\r\n", temp_int);
				//LOGD("UI: %d\r\n", UI_state);
				//Clear_Number_Area();
				//Display_Number(g_parameter.setting_number, EDIT_COLOR, 0);
			}else{
				// Cancel selected - return to Active state
				delete_alarm(Setting_Menu_Alarm);
				UI_state = STATE_ACTIVE;
				//if(register_alarm(Active_Alarm, ACTIVE_ALIVE_TIME) == eFALSE){
				//	LOGE("Alarm Full!\r\n");
				//}
				// Redraw Active page
				LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
				main_display_digi = Display_Room_Temp;
				Backlight_SetDuty(BACKLIGHT_DUTY_ACTIVE);
				if(register_alarm(Active_Alarm, ACTIVE_ALIVE_TIME) == eFALSE){
						LOGE("Alarm Fail\r\n");
				}
				Draw_Active_Menu();
			}
		}
	}
	else if(UI_state == STATE_WINDOW_OPEN_DETECTED)
	{
		// Any key press -> show confirmation dialog
		if(key.key_val != NONKEY && key.key_val != SYS_RESET)
		{
			UI_state = STATE_WINDOW_OPEN_CONFIRM;
			window_open_confirm_selection = 0; // Default to Cancel
			Backlight_SetDuty(BACKLIGHT_DUTY_ACTIVE);
			// Delete all alarms except Min_Update_Alarm
			delete_alarm(Active_Alarm);
			delete_alarm(Setting_Digi_Alarm);
			delete_alarm(Setting_Menu_Alarm);
			// Register alarm for confirmation dialog timeout
			if(register_alarm(Setting_Menu_Alarm, ACTIVE_ALIVE_TIME) == eFALSE){
				LOGE("Alarm Full!\r\n");
			}
			Draw_Window_Open_Confirm_Page(window_open_confirm_selection);
		}
	}
	else if(UI_state == STATE_WINDOW_OPEN_CONFIRM)
	{
		if(key.key_val == UPKEY || key.key_val == DOWNKEY){
			// Toggle between Cancel (0) and Reset (1)
			window_open_confirm_selection = (window_open_confirm_selection == 0) ? 1 : 0;
			Draw_Window_Open_Confirm_Page(window_open_confirm_selection);
		}else if(key.key_val == ENTERKEY){
			if(window_open_confirm_selection == 1){
				// Reset Window Open Detect
				delete_alarm(Setting_Menu_Alarm);
				window_function_reset();
				UI_state = STATE_ACTIVE;
				LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
				main_display_digi = Display_Room_Temp;
				Backlight_SetDuty(BACKLIGHT_DUTY_ACTIVE);
				if(register_alarm(Active_Alarm, ACTIVE_ALIVE_TIME) == eFALSE){
					LOGE("Alarm Fail\r\n");
				}
				Draw_Active_Menu();
			}else{
				// Cancel Window Open Detect
				delete_alarm(Setting_Menu_Alarm);
				g_parameter.window_fun = 0;
				if(Flash_Save_Parameter() != FLASH_OK){
					LOGE("Flash write fail!\r\n");
				}
				UI_state = STATE_ACTIVE;
				LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
				main_display_digi = Display_Room_Temp;
				Backlight_SetDuty(BACKLIGHT_DUTY_ACTIVE);
				if(register_alarm(Active_Alarm, ACTIVE_ALIVE_TIME) == eFALSE){
					LOGE("Alarm Fail\r\n");
				}
				Draw_Active_Menu();
			}
		}
	}
	else if(UI_state == STATE_CONTROL_POWER_LIMIT)
	{
		if(Top_Bar_Active)  // TopBar mode: Edit/Leave
			{
				if(key.key_val == UPKEY || key.key_val == DOWNKEY)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key.key_val == ENTERKEY)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter Power Limit edit mode
						Top_Bar_Active = 0;
						item_selection = 1;
						leave_icon_color = 0;
						edit_icon_color = 0;
						Draw_Control_Adj_Power_Limit_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					else  // Leave Icon is red
					{
						// Go back to Function Setting Edit mode with cursor on Control Adj
						UI_state = STATE_CONTROL_ADJ_MENU;
						Top_Bar_Active = 0;
						item_selection = Find_Control_Adj_Menu_Index(STATE_CONTROL_POWER_LIMIT);
						leave_icon_color = 0;
						edit_icon_color = 0;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Control_Adj_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					//delay_ms(100);
				}
			}else if(item_selection == 1){
					if((key.key_val == DOWNKEY) || (key.key_val == UPKEY)){		//Down key
						power_limit_switch = (power_limit_switch == 0) ? 1 : 0;
					}else if(key.key_val == ENTERKEY){		//Enter key
							if(power_limit_switch == 0){
									item_selection = 3;
							}else{
									item_selection = 2;
							}
					}
					Draw_Control_Adj_Power_Limit_Content(item_selection);
			}else if(item_selection == 2){
					if(key.key_val == UPKEY){
							if(power_limit < 30){
									power_limit++;
							}
					}else if(key.key_val == DOWNKEY){
							if(power_limit > 1){
									power_limit--;
							}
					}else if(key.key_val == ENTERKEY){
							item_selection = 3;
					}
					Draw_Control_Adj_Power_Limit_Content(item_selection);
					
			}else if(item_selection == 3){
					if((key.key_val == UPKEY) || (key.key_val == DOWNKEY)){
							item_selection = 1;
							Draw_Control_Adj_Power_Limit_Content(item_selection);
					}else if(key.key_val == ENTERKEY){
							UI_state = STATE_CONTROL_ADJ_MENU;
							item_selection = Find_Control_Adj_Menu_Index(STATE_CONTROL_POWER_LIMIT);
							Top_Bar_Active = 0;
							leave_icon_color = 0;
							edit_icon_color = 0;
							g_parameter.power_limit = power_limit;
							g_parameter.power_limit_switch = power_limit_switch;
							if(Flash_Save_Parameter() != FLASH_OK){
									LOGE("Flash write fail!\r\n");
							}
							LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
							Draw_Control_Adj_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}
			}
		
	}
	else if(UI_state == STATE_USER_SETTING_COMFORT_MODE)
	{
			if(Top_Bar_Active)  // TopBar mode: Edit/Leave
			{
				if(key.key_val == UPKEY || key.key_val == DOWNKEY)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key.key_val == ENTERKEY)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter Backlight edit mode
						Top_Bar_Active = 0;
						item_selection = 1;
						leave_icon_color = 0;
						edit_icon_color = 0;
						Draw_TopBar(leave_icon_color, edit_icon_color);
						Draw_Control_Adj_Comfort_Mode_Content(item_selection);
					}
					else  // Leave Icon is red
					{
						// Go back to Function Setting Edit mode with cursor on Control Adj
						UI_state = STATE_USER_SETTING_MENU;
						Top_Bar_Active = 0;
						item_selection = Find_User_Setting_Menu_Index(STATE_USER_SETTING_COMFORT_MODE);
						leave_icon_color = 0;
						edit_icon_color = 0;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_User_Setting_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					//delay_ms(100);
				}
		}else if(item_selection ==1){
				if((key.key_val == UPKEY) || (key.key_val == DOWNKEY)){
					comfort_mode = (comfort_mode == 1) ? 0 : 1;
				}else if(key.key_val == ENTERKEY){
						item_selection = 2;
				}
				Draw_Control_Adj_Comfort_Mode_Content(item_selection);
		}else if(item_selection == 2){
				if(comfort_mode == 0){
						if((key.key_val == UPKEY) || (key.key_val == DOWNKEY)){
								item_selection = 1;
								Draw_Control_Adj_Comfort_Mode_Content(item_selection);
						}else if(key.key_val == ENTERKEY){
								UI_state = STATE_USER_SETTING_MENU;
								item_selection = Find_User_Setting_Menu_Index(STATE_USER_SETTING_COMFORT_MODE);
								Top_Bar_Active = 0;
								leave_icon_color = 0;
								edit_icon_color = 0;
								g_parameter.comfort_mode = comfort_mode;
								if(Flash_Save_Parameter() != FLASH_OK){
										LOGE("Flash write fail!\r\n");
								}
								LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
								Draw_User_Setting_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
						}
				}else if(comfort_mode == 1){
						UI_state = STATE_USER_SETTING_COMFORT_MODE_SETTING;
						item_selection = 0;
						Top_Bar_Active = 1;
						leave_icon_color = 0;
						edit_icon_color = 1;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Control_Adj_Comfort_Mode_Setting_Page(item_selection, leave_icon_color, edit_icon_color);
				}
		}
	}
	else if(UI_state == STATE_USER_SETTING_COMFORT_MODE_SETTING)
	{
			if(Top_Bar_Active)  // TopBar mode: Edit/Leave
			{
				if(key.key_val == UPKEY || key.key_val == DOWNKEY)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key.key_val == ENTERKEY)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter Backlight edit mode
						Top_Bar_Active = 0;
						item_selection = 1;
						leave_icon_color = 0;
						edit_icon_color = 0;
						Draw_TopBar(leave_icon_color, edit_icon_color);
						Draw_Control_Adj_Comfort_Mode_Setting_Content(item_selection);
					}
					else  // Leave Icon is red
					{
						// Go back to Function Setting Edit mode with cursor on Control Adj
						UI_state = STATE_USER_SETTING_COMFORT_MODE;
						Top_Bar_Active = 0;
						item_selection = 1;
						leave_icon_color = 0;
						edit_icon_color = 0;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Control_Adj_Comfort_Mode_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					//delay_ms(100);
				}
			}else if(item_selection == 1){
					if(key.key_val == DOWNKEY){
							if(floor_material < 2){
									floor_material++;
							}
					}else if(key.key_val == UPKEY){
							if(floor_material > 0){
									floor_material--;
							}else{
									Top_Bar_Active = 1;
									leave_icon_color = 1;
									edit_icon_color = 0;
									item_selection = 0;
									Draw_Control_Adj_Comfort_Mode_Setting_Page(item_selection, leave_icon_color, edit_icon_color);
							}
					}else if(key.key_val == ENTERKEY){
							item_selection = 2;
					}
					Draw_Control_Adj_Comfort_Mode_Setting_Content(item_selection);
			}else if(item_selection == 2){
					if((key.key_val == UPKEY) || (key.key_val == DOWNKEY)){
							item_selection = 1;
							Draw_Control_Adj_Comfort_Mode_Setting_Content(item_selection);
					}else if(key.key_val == ENTERKEY){
							UI_state = STATE_USER_SETTING_MENU;
							Top_Bar_Active = 0;
							item_selection = Find_User_Setting_Menu_Index(STATE_USER_SETTING_COMFORT_MODE);
							leave_icon_color = 0;
							edit_icon_color = 0;
							g_parameter.comfort_mode = comfort_mode;
							g_parameter.floor_material = floor_material;
							if(Flash_Save_Parameter() != FLASH_OK){
									LOGE("Flash write fail!\r\n");
							}
							LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
							Draw_User_Setting_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}
			}
	}
	else if(UI_state == STATE_USER_SETTING_LANGUAGE)
	{
		if(Top_Bar_Active)  // TopBar mode: Edit/Leave
			{
				if(key.key_val == UPKEY || key.key_val == DOWNKEY)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key.key_val == ENTERKEY)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter Backlight edit mode
						Top_Bar_Active = 0;
						item_selection = 1;
						leave_icon_color = 0;
						edit_icon_color = 0;
						Draw_TopBar(leave_icon_color, edit_icon_color);
						Draw_User_Setting_Language_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					else  // Leave Icon is red
					{
						// Go back to Function Setting Edit mode with cursor on Control Adj
							UI_state = STATE_USER_SETTING_MENU;
						Top_Bar_Active = 0;
							item_selection = Find_User_Setting_Menu_Index(STATE_USER_SETTING_LANGUAGE);
						leave_icon_color = 0;
						edit_icon_color = 0;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_User_Setting_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					//delay_ms(100);
				}
			}
			else if((item_selection == 1) || (item_selection == 2))
			{
					if(key.key_val == DOWNKEY){		//Down key
							if(item_selection == 1){
									item_selection = 2;
									Draw_User_Setting_Language_Choice(item_selection);
							}
							
					}else if(key.key_val == UPKEY){		//Up key
							if(item_selection == 2){
									item_selection = 1;
									Draw_User_Setting_Language_Choice(item_selection);
							}else if(item_selection == 1){	//go back Top_Bar
									Top_Bar_Active = 1;
									item_selection = 0;
									leave_icon_color = 1;
									edit_icon_color = 0;
									Draw_User_Setting_Language_Page(item_selection, leave_icon_color, edit_icon_color);
							}
							
					}else if(key.key_val == ENTERKEY){		//Enter key
							language_temp = (item_selection == 1) ? 0: 1;
							item_selection = 3;
							Draw_User_Setting_Language_Page(item_selection, leave_icon_color, edit_icon_color);
							
					}
			}else if(item_selection == 3){
					if(key.key_val == ENTERKEY){
							g_parameter.language = language_temp;
							UI_state = STATE_USER_SETTING_MENU;
							Top_Bar_Active = 0;
							item_selection = Find_User_Setting_Menu_Index(STATE_USER_SETTING_LANGUAGE);
							leave_icon_color = 0;
							edit_icon_color = 0;
							if(Flash_Save_Parameter() != FLASH_OK){
									LOGE("Flash write fail!\r\n");
							}
							LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
							Draw_User_Setting_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
							
					}else if((key.key_val == UPKEY) || (key.key_val == DOWNKEY)){
							item_selection = language_temp + 1;
							Draw_User_Setting_Language_Page(item_selection, leave_icon_color, edit_icon_color);
					}
			}	
			
	}
	else if(UI_state == STATE_USER_SETTING_ADAPTIVE_START)
	{
		if(Top_Bar_Active)
			{
				if(key.key_val == UPKEY || key.key_val == DOWNKEY)
				{
					Update_TopBar();
				}
				else if(key.key_val == ENTERKEY)
				{
					if(edit_icon_color)
					{
						Top_Bar_Active = 0;
						item_selection = 1;
						leave_icon_color = 0;
						edit_icon_color = 0;
						Draw_TopBar(leave_icon_color, edit_icon_color);
						Draw_User_Setting_Adaptive_Start_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					else
					{
						UI_state = STATE_USER_SETTING_MENU;
						Top_Bar_Active = 0;
						item_selection = Find_User_Setting_Menu_Index(STATE_USER_SETTING_ADAPTIVE_START);
						leave_icon_color = 0;
						edit_icon_color = 0;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_User_Setting_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}
				}
			}
			else if((item_selection == 1) || (item_selection == 2))
			{
				if(key.key_val == DOWNKEY){
					if(item_selection == 1){
						item_selection = 2;
						Draw_User_Setting_Adaptive_Start_Choice(item_selection);
					}
				}else if(key.key_val == UPKEY){
					if(item_selection == 2){
						item_selection = 1;
						Draw_User_Setting_Adaptive_Start_Choice(item_selection);
					}else if(item_selection == 1){
						Top_Bar_Active = 1;
						item_selection = 0;
						leave_icon_color = 1;
						edit_icon_color = 0;
						Draw_User_Setting_Adaptive_Start_Page(item_selection, leave_icon_color, edit_icon_color);
					}
				}else if(key.key_val == ENTERKEY){
					adaptive_start = (item_selection == 1) ? 0 : 1;
					item_selection = 3;
					Draw_User_Setting_Adaptive_Start_Page(item_selection, leave_icon_color, edit_icon_color);
				}
			}else if(item_selection == 3){
				if(key.key_val == ENTERKEY){
					g_parameter.adaptive_start = adaptive_start;
					UI_state = STATE_USER_SETTING_MENU;
					Top_Bar_Active = 0;
					item_selection = Find_User_Setting_Menu_Index(STATE_USER_SETTING_ADAPTIVE_START);
					leave_icon_color = 0;
					edit_icon_color = 0;
					if(Flash_Save_Parameter() != FLASH_OK){
						LOGE("Flash write fail!\r\n");
					}
					LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
					Draw_User_Setting_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
				}else if((key.key_val == UPKEY) || (key.key_val == DOWNKEY)){
					item_selection = adaptive_start + 1;
					Draw_User_Setting_Adaptive_Start_Page(item_selection, leave_icon_color, edit_icon_color);
				}
			}
	}

	key.key_val = NONKEY;
	key.key_active = 0;
	if(key.key_press != DOUBLEKEY){
			key.key_press = 0;
	}

}


void Handle_Pin_Input(uint8_t key_val)
{
	if(key_val == UPKEY){
		// Move digit selector up
		if(pin_digit_selected < 9){
			pin_digit_selected++;
			Draw_Pin_Digit_Selector(pin_digit_selected);
		}
	}else if(key_val == DOWNKEY){
		// Move digit selector down
		if(pin_digit_selected > 0){
			pin_digit_selected--;
			Draw_Pin_Digit_Selector(pin_digit_selected);
		}
	}else if(key_val == ENTERKEY){
		// Enter the selected digit
		if(pin_setup_stage == 0){
			pin_code_input[pin_digit_index] = pin_digit_selected;
		}else{
			pin_code_confirm[pin_digit_index] = pin_digit_selected;
		}
		
		// Move to next digit
		pin_digit_index++;
		
		if(pin_digit_index >= 4){
			// All 4 digits entered
			if(pin_setup_stage == 0){
				// First stage complete - move to confirmation
				pin_setup_stage = 1;
				pin_digit_index = 0;
				pin_digit_selected = 0;
				LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
				Draw_Child_Lock_Pin_Page(pin_setup_stage);
			}else{
				// Confirmation stage complete - verify
				uint8_t i;
				uint8_t match = 1;
				for(i = 0; i < 4; i++){
					if(pin_code_input[i] != pin_code_confirm[i]){
						match = 0;
						break;
					}
				}
				
				if(match){
					// PINs match - save and return
					for(i = 0; i < 4; i++){
						g_parameter.pin_code[i] = pin_code_input[i];
					}
					if(Flash_Save_Parameter() != FLASH_OK){
						LOGE("Flash write fail!\r\n");
					}
					
					// Return to User Setting Menu
					UI_state = STATE_USER_SETTING_MENU;
					Top_Bar_Active = 0;
					item_selection = Find_User_Setting_Menu_Index(STATE_USER_SETTING_CHILD_LOCK);
					leave_icon_color = 0;
					edit_icon_color = 0;
					LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
					Draw_User_Setting_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
				}else{
					// PINs don't match - restart setup
					pin_setup_stage = 0;
					pin_digit_index = 0;
					pin_digit_selected = 0;
					Clear_Pin_Input();
					LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
					Draw_Child_Lock_Pin_Page(pin_setup_stage);
				}
			}
		}else{
			// Update display with new digit
			if(pin_setup_stage == 0){
				Draw_Pin_Input_Box(pin_code_input, pin_digit_index);
			}else{
				Draw_Pin_Input_Box(pin_code_confirm, pin_digit_index);
			}
		}
	}
}

