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
#define SLEEP_TIMEOUT   1000  // 10 seconds = 1000 * 10ms
#define EDIT_TIMEOUT    500   // 5 seconds for setting mode

// ------------------------------------  Global variables  ------------------------------------------------
uint8_t UI_state = STATE_ACTIVE;  // 開機後進入Active狀態
uint32_t sleep_timeout_counter = 0;   // Sleep狀態計時器
uint32_t edit_timeout_counter = 0;    // Setting狀態計時器
uint8_t icon6_red_state = 0;          // Icon6紅色狀態標記 (0=Black, 1=Red)

uint8_t leave_icon_color;							
uint8_t edit_icon_color;
uint8_t item_selection;
uint8_t	Top_Bar_Active = 0;						// Top_Bar_Active cursor stay in Leave/Edit
uint8_t current_prog_type = 0;        // 0="5+2", 1="6+1", 2="7"
//uint8_t prog_type_substate = 0;       // 0=TopBar, 1=Choosing, 2=Save prompt
uint8_t prog_type_top_sel = 1;        // 0=Leave, 1=Edit
//uint8_t prog_type_edit_sel = 0;       // Temporary selection for program type

//FUNC_SETTING Page val
//uint8_t func_setting_item = 0;				//FUNC_SETTING Page curror position

// Schedule Edit頁面變量 (合併P1-P6)
uint8_t schedule_edit_substate = 0;   // 0=TopBar(Edit/Leave), 1=P1-P6/Finish選項模式
uint8_t schedule_edit_top_sel = 1;    // TopBar狀態: 0=Leave紅色, 1=Edit紅色
//uint8_t schedule_edit_sel = 0;        // P1-P6/Finish選擇: 0=P1, 1=P2, 2=P3, 3=P4, 4=P5, 5=P6, 6=Finish
uint8_t schedule_edit_scroll = 0;     // 捲動偏移: 0=顯示P1-P4, 1=顯示P2-P5, 2=顯示P3-P6

//HEATING_SCHEDULE_MENU page val
//uint8_t heating_schedule_menu_item = 0;		//HEATING_SCHEDULE_MENU page curror position

// Control Adj Menu頁面變量
//uint8_t control_adj_menu_substate = 0;  // 0=TopBar, 1=選項模式
//uint8_t control_adj_menu_top_sel = 1;   // TopBar: 0=Leave紅, 1=Edit紅
//uint8_t control_adj_menu_sel = 0;       // 選項: 0=Sensor, 1=Temp.Correct, 2=Temp.Limit, 3=Temp.Protect, 4=Power On State
uint8_t control_adj_menu_scroll = 0;    // 捲動偏移: 0=顯示row0-3, 1=顯示row1-4
uint8_t user_setting_menu_scroll = 0;

// Control Adj Sensor Setting頁面變量
//uint8_t control_adj_sensor_substate = 0;  // 0=TopBar, 1=Floor/Room選項模式, 2=Save模式
//uint8_t control_adj_sensor_top_sel = 1;   // TopBar: 0=Leave紅, 1=Edit紅
uint8_t control_adj_sensor_sel = 0;       // 選項: 0=Room, 1=Floor (swapped order)
uint8_t current_sensor_type = 0;          // 儲存設定: 0=Room, 1=Floor (default Room)

// Control Adj Temp. Correct Setting頁面變量
uint8_t control_adj_temp_correct_substate = 0;  // 0=TopBar, 1=Internal/External選項模式, 2=Internal編輯, 3=External編輯, 4=Save模式
float temp_correct_internal = -1.0f;     // 內部溫度校正值
float temp_correct_external = -0.5f;     // 外部溫度校正值
int	temp_limit_max = 45;						 // Setup temp limit max value
int temp_limit_min = 0;						 		// Setup temp limit min value 
int 	temp_protect_max = 60;
int		temp_protect_min = 4;
uint8_t temp_protect_max_switch = 0;
uint8_t temp_protect_min_switch = 1;
uint8_t power_on_state = 1;
uint8_t window_fun = 1;
uint8_t window_fun_temp = 10;
uint8_t window_fun_time = 30;
uint8_t sleep_backlight_duty = 20;

uint8_t child_lock = 0;

// Schedule Time Setting頁面變量
uint8_t schedule_time_setting_period = 1;  // 當前設定的Period編號 (1~6)
//uint8_t schedule_time_setting_substate = 0; // 0=TopBar, 1=小時編輯, 2=分編輯, 3=溫度編輯, 4=ON/OFF編輯, 5=Save
//uint8_t schedule_time_setting_top_sel = 1;  // TopBar: 0=Leave紅, 1=Edit紅
uint8_t schedule_time_on_off = 1;           // ON/OFF狀態: 0=OFF, 1=ON

// Schedule Time Setting 編輯變量
uint8_t schedule_time_edit_hour = 5;        // 小時 (0-23)
uint8_t schedule_time_edit_min = 30;        // 分 (0-59)
uint8_t schedule_time_edit_temp = 30;       // 溫度 (0-45)

// Schedule 設定儲存陣列 - Workday (P1~P6)
// 每個時段: hour, min, temp, on_off (1=ON, 0=OFF)
uint8_t schedule_settings[6][4] = {
	{5, 30, 30, 1},   // P1: 05:30, 30°C, ON
	{8, 30, 30, 1},   // P2: 08:30, 30°C, ON
	{10, 30, 30, 1},  // P3: 10:30, 30°C, ON
	{12, 30, 30, 1},  // P4: 12:30, 30°C, ON
	{18, 30, 30, 1},  // P5: 18:30, 30°C, ON
	{21, 30, 30, 1}   // P6: 21:30, 30°C, ON
};

// Schedule 設定儲存陣列 - Restday (P1~P6)
uint8_t schedule_settings_restday[6][4] = {
	{7, 0, 30, 1},    // P1: 07:00, 30°C, ON
	{9, 0, 30, 1},    // P2: 09:00, 30°C, ON
	{12, 0, 30, 1},   // P3: 12:00, 30°C, ON
	{14, 0, 30, 1},   // P4: 14:00, 30°C, ON
	{18, 0, 30, 1},   // P5: 18:00, 30°C, ON
	{22, 0, 30, 1}    // P6: 22:00, 30°C, ON
};

// Schedule Edit來源: 0=Workday, 1=Restday (用以區分讀寫哪份schedule_settings)
uint8_t schedule_edit_source = 0;

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
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		
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
		Display_Number(setting_number, BLACK, 1);  // show_decimal = 1

		// Set initial state: Active with 90% backlight
		UI_state = STATE_ACTIVE;
		Backlight_SetDuty(BACKLIGHT_DUTY_ACTIVE);
		sleep_timeout_counter = 0;
	
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
	// Icons on the RIGHT side - now using 24x24 monochrome icons
	// Smaller size (24x24) to avoid overlapping with time display at left
	// Screen width is 160px, icons positioned to stay within bounds
	
	// Icon3 at rightmost (136, 5) - rightmost, RED foreground (136+24=160)
	GUI_DrawMonoIcon24x24(136, 5, RED, WHITE, Icon03_24x24);
	
	// Icon2 at (112, 5) - to the left of Icon3
	GUI_DrawMonoIcon24x24(112, 5, BLACK, WHITE, Icon02_24x24);
	
	// Icon1 at (88, 5) - to the left of Icon2
	GUI_DrawMonoIcon24x24(88, 5, BLACK, WHITE, Icon01_24x24);
	
	// Celsius Icon at (100, 45)
	GUI_DrawMonoIcon24x24(100, 45, BLACK, WHITE, Celsius_Icon_24x24);
	
	// Icon8 at (136, 45) - below Icon3
	GUI_DrawMonoIcon24x24(136, 45, BLACK, WHITE, Icon08_24x24);
	
	// Icon6 at (136, 85) - below Icon8
	if(icon6_red_state){
			GUI_DrawMonoIcon24x24(136, 85, RED, WHITE, Icon06_24x24);
	} else {
			GUI_DrawMonoIcon24x24(136, 85, BLACK, WHITE, Icon06_24x24);
	}
}

// Display number on LCD
// number: value to display
// color: text color
// show_decimal: 1 = show decimal point and fraction, 0 = show integer only
void Display_Number(float number, uint16_t color, uint8_t show_decimal)
{
	int integer_part;
	int decimal_part;
	uint8_t digit1, digit2;
	uint8_t has_tens = 0;
	uint8_t is_negative = 0;
	
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
	
	// Draw minus sign if negative
	if(is_negative)
	{
		// Draw minus sign - position depends on number of digits
		if(has_tens)
		{
			// Two digits: minus sign at far left (x=5)
			LCD_Fill(5, 80, 21, 84, color);
		}
		else
		{
			// Single digit: minus sign closer to the digit (x=35)
			LCD_Fill(35, 80, 51, 84, color);
		}
	}
	else
	{
		// Clear minus sign area (for when going from negative to positive)
		LCD_Fill(5, 80, 51, 84, WHITE);
	}
	
	// Draw digits
	// Position matches original -14.5 display layout
	// "4" is at x=59 (same as original "4" in -14.5)
	if(has_tens)
	{
		// Two digits - clear both digit areas first, then draw
		// This ensures no residual pixels from previous display
		LCD_Fill(20, 42, 55, 106, WHITE);   // Clear tens digit area
		LCD_Fill(56, 42, 95, 106, WHITE);   // Clear ones digit area
		// "1" at x=22, "4" at x=59
		GUI_DrawBigDigit(22, 42, color, WHITE, '0' + digit1, 0);  // 32x64 font - tens
		GUI_DrawBigDigit(59, 42, color, WHITE, '0' + digit2, 0);  // 32x64 font - ones
	}
	else
	{
		// Single digit - draw at x=59
		// Note: Clear_Number_Area() already cleared everything before this function is called
		GUI_DrawBigDigit(59, 42, color, WHITE, '0' + digit2, 0);  // 32x64 font (with bg)
	}
	
		// Draw decimal point and fraction if needed
	if(show_decimal)
	{
		// Draw decimal point (circular dot) at fixed position
		LCD_Fill(96, 94, 100, 98, color);
		LCD_Fill(95, 95, 101, 97, color);
		LCD_Fill(96, 99, 100, 99, color);
		LCD_Fill(96, 93, 100, 93, color);
		
		// Draw decimal digit using smaller font (16x32)
		GUI_DrawBigDigit(109, 74, color, WHITE, '0' + decimal_part, 1);
	}
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
	LCD_Fill(5, 40, 130, 108, WHITE);
}


//--------------------------------------------------------------------------------------------
//																		Add by Benjamin
//--------------------------------------------------------------------------------------------



void Draw_TopBar(uint8_t leave_col, uint8_t edit_col)
{
			LCD_Fill(0, 0, 21, 21, WHITE);		//Clear Leave Icon
			LCD_Fill(144, 0, 160, 21, WHITE);	//Clear Edit Icon
			
			if(leave_col){
					GUI_DrawMonoIcon16x16(5, 5, RED, WHITE, Icon16x16_Leave);   // Leave red
			}else{
					GUI_DrawMonoIcon16x16(5, 5, BLACK, WHITE, Icon16x16_Leave);  // Leave black
			}
			
			if(edit_col){
					GUI_DrawMonoIcon16x16(140, 5, RED, WHITE, Icon16x16_Edit);  // Edit red
			}else{
					GUI_DrawMonoIcon16x16(140, 5, BLACK, WHITE, Icon16x16_Edit);  // Edit black
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
		delay_ms(100);  // Debounce
	
}

// Draw a single row in Function Setting Edit mode (for optimized update)
// row: 0=Heating Schedule, 1=Control Adj, 2=User Settings
// selected: 1=selected (red bg), 0=not selected
void Draw_Function_Setting_Edit_Row(uint8_t row, uint8_t selected)
{
	uint16_t row_y[] = {28, 52, 76};  // Y positions for each row (same as Draw_Function_Setting_Page)
	uint16_t text_color, bg_color;
	
	if(selected) {
		text_color = WHITE;
		bg_color = RED;
		// Draw red background (row height ~24 pixels)
		LCD_Fill(0, row_y[row], lcddev.width, row_y[row] + 24, RED);
	} else {
		text_color = BLACK;
		bg_color = WHITE;
		// Clear background to white
		LCD_Fill(0, row_y[row], lcddev.width, row_y[row] + 24, WHITE);
	}
	
	// Draw the row content based on row number
	// Icon and text moved down by 2 pixels to center in the background row
	if(row == 0) {
		GUI_DrawMonoIcon8x16(10, row_y[row] + 4, text_color, bg_color, Icon16_8x16);
		POINT_COLOR = text_color;
		BACK_COLOR = bg_color;
		Show_Str(30, row_y[row] + 2, text_color, bg_color, "Heating Schedule", 16, 0);
	} else if(row == 1) {
		GUI_DrawMonoIcon8x16(10, row_y[row] + 4, text_color, bg_color, Icon17_8x16);
		POINT_COLOR = text_color;
		BACK_COLOR = bg_color;
		Show_Str(30, row_y[row] + 2, text_color, bg_color, "Control Adj.", 16, 0);
	} else {
		GUI_DrawMonoIcon8x16(10, row_y[row] + 4, text_color, bg_color, Icon18_8x16);
		POINT_COLOR = text_color;
		BACK_COLOR = bg_color;
		Show_Str(30, row_y[row] + 2, text_color, bg_color, "User Settings", 16, 0);
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
	// Clear screen
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);

	// Draw Top Bar (both icons black in edit mode)
	Draw_TopBar(Leave_col, Edit_col);

	// Draw all three rows
	Draw_Function_Setting_Edit_Row(0, (selection == 1) ? 1 : 0);
	Draw_Function_Setting_Edit_Row(1, (selection == 2) ? 1 : 0);
	Draw_Function_Setting_Edit_Row(2, (selection == 3) ? 1 : 0);
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
	if(UI_state == STATE_ACTIVE)
	{
			// Active state: handle key presses
			if(key == 1 || key == 2)  // Up or Down key pressed -> enter Setting mode
			{
				// If Manu Icon is red, change it back to black first
				if(icon6_red_state)
				{
					icon6_red_state = 0;
					GUI_DrawMonoIcon24x24(136, 85, BLACK, WHITE, Icon06_24x24);				// Draw black Manu Icon6 
				}

				// Transition to Setting state
				UI_state = STATE_SETTING;
				sleep_timeout_counter = 0;
				edit_timeout_counter = 0;

				// Clear and display in BLUE without decimal
				Clear_Number_Area();
				Display_Number(setting_number, EDIT_COLOR, 0);  // show_decimal = 0, BLUE color

				delay_ms(100);  // Debounce
			}
			else if(key == 3)  // Enter key pressed
			{
				if(icon6_red_state)
				{
					// Manu Icon is red -> enter Function Setting state
					UI_state = STATE_FUNC_SETTING;
					sleep_timeout_counter = 0;
					// Draw Function Setting page
					leave_icon_color = 0;
					edit_icon_color = 1;
					item_selection = 0;
					Draw_Function_Setting_Page(item_selection, leave_icon_color, edit_icon_color);		//Edit Icon red
					//Draw_TopBar(0, 1);					//Edit icon red
					Top_Bar_Active = 1;
					
				}
				else
				{
					// Manu Icon is black -> toggle to red
					icon6_red_state = 1;							//	red state
					GUI_DrawMonoIcon24x24(136, 85, RED, WHITE, Icon06_24x24);;  // Draw red Manu Icon
				}
				sleep_timeout_counter = 0;  // Reset sleep timeout

				delay_ms(100);  // Debounce
			}
			else
			{
				// No key pressed, increment sleep timeout counter
				if(++sleep_timeout_counter >= SLEEP_TIMEOUT)  // 1000 * 10ms = 10 seconds
				{
					// Timeout - enter Sleep state
					// If Manu Icon is red, change it back to black first
					if(icon6_red_state)
					{
						icon6_red_state = 0;
						GUI_DrawMonoIcon24x24(136, 85, BLACK, WHITE, Icon06_24x24);  // Draw black Manu Icon
					}

					UI_state = STATE_SLEEP;
					sleep_timeout_counter = 0;

					// Set backlight to 20%
					Backlight_SetDuty(sleep_backlight_duty);
					LOGD("SET Backlight\n\r");
				}
			}
	}
	else if(UI_state == STATE_SLEEP)
	{
			// Sleep state: wait for any key press to wake up
			if(key != 0)  // Up or Down key pressed
			{
				// Wake up - return to Active state
				UI_state = STATE_ACTIVE;
				sleep_timeout_counter = 0;

				// Set backlight back to 90%
				Backlight_SetDuty(BACKLIGHT_DUTY_ACTIVE);

				delay_ms(100);  // Debounce
			}
	}
	else if(UI_state == STATE_SETTING)			//Setting Thermostat active Temperature
	{
			// Handle key input
			if(key == 1)  // Up key pressed
			{
				setting_number += 1.0f;
				if(setting_number > 99.0f) setting_number = 99.0f;  // Max limit
				sleep_timeout_counter = 0;   // Reset sleep timeout
				edit_timeout_counter = 0;    // Reset edit timeout

				// Redisplay in BLUE
				Clear_Number_Area();
				Display_Number(setting_number, EDIT_COLOR, 0);

				delay_ms(100);  // Debounce
			}
			else if(key == 2)  // Down key pressed
			{
				setting_number -= 1.0f;
				// Allow negative numbers, limit to -99
				if(setting_number < -99.0f) setting_number = -99.0f;
				sleep_timeout_counter = 0;   // Reset sleep timeout
				edit_timeout_counter = 0;    // Reset edit timeout

				// Redisplay in BLUE
				Clear_Number_Area();
				Display_Number(setting_number, EDIT_COLOR, 0);

				delay_ms(100);  // Debounce
			}
			else if(key == 3)  // Enter key pressed -> exit Setting immediately
			{
				// Exit Setting state immediately, return to Active
				UI_state = STATE_ACTIVE;
				edit_timeout_counter = 0;
				sleep_timeout_counter = 0;

				// Display final number with decimal in BLACK
				Clear_Number_Area();
				Display_Number(setting_number, BLACK, 1);  // show_decimal = 1

				delay_ms(100);  // Debounce
			}

			// Check for timeout (5 seconds) - return to Active state
			if(++edit_timeout_counter >= EDIT_TIMEOUT)  // 500 * 10ms = 5 seconds
			{
				// Timeout - return to Active state
				UI_state = STATE_ACTIVE;
				edit_timeout_counter = 0;
				sleep_timeout_counter = 0;

				// Display final number with decimal in BLACK
				Clear_Number_Area();
				Display_Number(setting_number, BLACK, 1);  // show_decimal = 1
			}
	}
	else if(UI_state == STATE_FUNC_SETTING)
	{
		
			if(Top_Bar_Active){
					// Function Setting state
					if(key == 1 || key == 2)  // Up or Down key pressed -> toggle Icon colors
					{
							Update_TopBar();
					}
					else if(key == 3)  // Enter key pressed
					{
							if(edit_icon_color)
							{
									// Edit Icon is red -> enter Function Setting 選項編輯模式
									item_selection = 1;  // Start with Heating Schedule selected
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
									sleep_timeout_counter = 0;
									icon6_red_state = 0;  // Reset Manu Icon to black
									// Clear screen
									LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
									// Redraw main page
									Draw_Active_Menu();
							}
							delay_ms(100);  // Debounce
					}
			}else{				//Leave Top Bar Area
				
					// Function Setting Edit mode navigation
					if(key == 2)  // Down key pressed
					{
							if(item_selection < 3)
							{
									// Move selection down - only update the changed rows
									Draw_Function_Setting_Edit_Row(item_selection - 1, 0);
									item_selection++;
									Draw_Function_Setting_Edit_Row(item_selection - 1, 1);
							}
							delay_ms(100);
					}
					else if(key == 1)  // Up key pressed
					{
							if(item_selection > 1)
							{	
									// Move selection up - only update the changed rows
									Draw_Function_Setting_Edit_Row(item_selection - 1, 0);
									item_selection--;
									Draw_Function_Setting_Edit_Row(item_selection - 1, 1);
							}
							else
							{
									// At top, go back to Function Setting with Edit red
									Top_Bar_Active = 1;
									leave_icon_color = 0;
									edit_icon_color = 1;
									Draw_TopBar(leave_icon_color, edit_icon_color); 		 // Leave black, Edit red
									item_selection = 0;  
									Draw_Function_Setting_Edit_Row(0, 0);
							}
							delay_ms(100);
					}
					else if(key == 3)  // Enter key pressed
					{
							if(item_selection == 1) // "Heating Schedule" selected
							{
									// Enter Heating Schedule Menu page
									UI_state = STATE_HEATING_SCHEDULE_MENU;
									Top_Bar_Active = 1;
									item_selection = 0;
									leave_icon_color = 0;
									edit_icon_color = 1;
									LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
									Draw_Heating_Schedule_Menu_Page(item_selection, leave_icon_color, edit_icon_color);  // 0xFF = TopBar mode with Edit red
							}
							else if(item_selection == 2) // "Control Adj." selected
							{
									// Enter Control Adj Menu頁面
									UI_state = STATE_CONTROL_ADJ_MENU;
									Top_Bar_Active =1;
									item_selection = 0;
									leave_icon_color = 0;
									edit_icon_color = 1;
									control_adj_menu_scroll = 0;     // Reset scroll
									LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
									Draw_Control_Adj_Menu_Page(item_selection, leave_icon_color, edit_icon_color);  // 0xFF = TopBar mode with Edit red
							}
							else if(item_selection == 3)
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
							
							delay_ms(100);
					}
				
				
			}
			
	}
//	else if(UI_state == STATE_FUNC_SETTING_EDIT)
//	{
			
//	}
	else if(UI_state == STATE_HEATING_SCHEDULE_MENU)
	{
			// Heating Schedule Menu page navigation
			if(Top_Bar_Active)  // TopBar mode: Edit/Leave
			{
				if(key == 1 || key == 2)  // Up or Down key pressed
				{
						Update_TopBar();
				}
				else if(key == 3)  // Enter key pressed
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
						// Go back to Function Setting with Leave red
						UI_state = STATE_FUNC_SETTING;
						Top_Bar_Active = 0;
						item_selection = 1;
						leave_icon_color = 0;
						edit_icon_color = 0;
						Draw_Function_Setting_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					delay_ms(100);
				}
			}
			else   // Program Type / Workday Setting / Restday Setting mode
			{
				if(key == 2)  // Down key pressed
				{
					if(item_selection < 3)  // Can go to Restday Setting (2)
					{
						// Move selection down - only update changed rows
						Draw_Heating_Schedule_Menu_Row(item_selection, 0);  // Old row unselected
						item_selection++;
						Draw_Heating_Schedule_Menu_Row(item_selection, 1);  // New row selected
					}
					delay_ms(100);
				}
				else if(key == 1)  // Up key pressed
				{
					if(item_selection > 1)
					{
						// Move selection up - only update changed rows
						Draw_Heating_Schedule_Menu_Row(item_selection, 0);  // Old row unselected
						item_selection--;
						Draw_Heating_Schedule_Menu_Row(item_selection, 1);  // New row selected
					}
					else  // At Program Type, go back to TopBar mode
					{
						Top_Bar_Active = 1;
						item_selection = 0;
						leave_icon_color = 0;
						edit_icon_color = 1;
						Draw_TopBar(leave_icon_color, edit_icon_color);
						Draw_Heating_Schedule_Menu_Row(1, item_selection); 	//Redraw RAW 1
					}
					delay_ms(100);
				}
				else if(key == 3)  // Enter key pressed
				{
					if(item_selection == 1)  // Program Type selected
					{
						// Enter Program Type page
						UI_state = STATE_HEATING_SCHEDULE_PROG_TYPE;
						Top_Bar_Active = 1;
						leave_icon_color = 0;
						edit_icon_color = 1;
						item_selection = 0;
						// Clear screen the go to schedule prog type
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Heating_Schedule_Prog_Type_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					else if(item_selection == 2 || item_selection == 3)  // Workday or Restday Setting selected
					{
						// Set schedule edit source: 0=Workday, 1=Restday
						schedule_edit_source = (item_selection == 2) ? 0 : 1;
						// Enter Schedule Edit page 
						UI_state = STATE_SCHEDULE_EDIT;
						Top_Bar_Active = 1;
						leave_icon_color = 0;
						edit_icon_color = 1;
						item_selection = 0;
						schedule_edit_scroll = 0;     // Reset scroll bar 
						// Clear screen the go to schedule prog type
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Schedule_Edit_Page(item_selection, leave_icon_color, edit_icon_color);  
					}
					delay_ms(100);
				}
			}
	}
	else if(UI_state == STATE_HEATING_SCHEDULE_PROG_TYPE)
	{
			// Program Type page navigation
			if(Top_Bar_Active == 1) // Top Bar (Leave / Edit)
			{
				if(key == 1 || key == 2)  // Up or Down
				{
					Update_TopBar();
				}
				else if(key == 3)  // Enter key
				{
					if(leave_icon_color) { // Leave selected
						// Go back to Heating Schedule Menu with Leave red
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
						//Draw_Heating_Schedule_Prog_Type_Page(item_selection, leave_icon_color, edit_icon_color);
						Draw_TopBar(leave_icon_color, edit_icon_color);
						Draw_Heating_Schedule_Prog_Type_Content(item_selection -1);
					}
					delay_ms(100);
				}
			}
			else if(Top_Bar_Active == 0)
			{
				if(key == 2) // Down (Left)
				{
					if(item_selection > 1) {
						// Move selection left - only update changed choices
						item_selection--;
						Draw_Heating_Schedule_Prog_Type_Content(item_selection - 1);
					}
					delay_ms(100);
				}
				else if(key == 1) // Up (Right)
				{
					if(item_selection < 3) {
						// Move selection right - only update changed choices
						item_selection++;
						Draw_Heating_Schedule_Prog_Type_Content(item_selection - 1);
					}
					delay_ms(100);
				}
				else if(key == 3) // Enter -> Move to Save button
				{
					Top_Bar_Active = 2;
					//item_selection = 4; // Save Prompt
					Draw_Heating_Schedule_Prog_Type_Page(item_selection, leave_icon_color, edit_icon_color);
					delay_ms(100);
				}
			}
			else if(Top_Bar_Active == 2) // Save prompt
			{
				if(key == 1 || key == 2) // Up or Down -> Cancel save, go back to Choosing
				{
					Top_Bar_Active = 0;
					item_selection = 3;
					Draw_Heating_Schedule_Prog_Type_Page(item_selection, leave_icon_color, edit_icon_color);
					delay_ms(100);
				}
				else if(key == 3) // Enter -> Save and go back to Heating Schedule Menu
				{
					current_prog_type = item_selection - 1; // Save selection
					// Go back to Heating Schedule Menu with Leave red
					UI_state = STATE_HEATING_SCHEDULE_MENU;
					Top_Bar_Active = 0;   // TopBar模式
					leave_icon_color = 0;
					edit_icon_color = 0;
					item_selection = 1;
					// Clear screen
					LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
					Draw_Heating_Schedule_Menu_Page(item_selection, leave_icon_color, edit_icon_color);  
					delay_ms(100);
				}
			}
	}
	else if(UI_state == STATE_SCHEDULE_EDIT)
	{
			// Schedule Edit page navigation (合併P1-P6)
			if(Top_Bar_Active)  // TopBar mode: Edit/Leave
			{
				if(key == 1 || key == 2)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key == 3)  // Enter key pressed
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
						// Go back to Heating Schedule Menu with Leave red
						UI_state = STATE_HEATING_SCHEDULE_MENU;
						Top_Bar_Active = 1;
						leave_icon_color = 1;
						edit_icon_color = 0;
						item_selection = 0;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Heating_Schedule_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					delay_ms(100);
				}
			}
			else   // P1-P6/Finish mode
			{
				if(key == 2)  // Down key pressed
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
					delay_ms(100);
				}
				else if(key == 1)  // Up key pressed
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
						schedule_edit_substate = 0;
						schedule_edit_top_sel = 1;  // Edit red
						// Redraw entire page to show TopBar with Edit red and clear P1 selection
						Top_Bar_Active = 1;
						leave_icon_color = 1;
						edit_icon_color = 0;
						item_selection = 0;
						Draw_Schedule_Edit_Page(item_selection, leave_icon_color, edit_icon_color);  // 0xFF = TopBar mode, Edit red
					}
					delay_ms(100);
				}
				else if(key == 3)  // Enter key pressed
				{
					if(item_selection == 7)  // Finish selected
					{
						// Go back to Heating Schedule Menu with Leave red
						UI_state = STATE_HEATING_SCHEDULE_MENU;
						Top_Bar_Active = 0;
						leave_icon_color = 0;
						edit_icon_color = 0;
						item_selection = (schedule_edit_source == 0) ? 2 : 3;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Heating_Schedule_Menu_Page(item_selection, leave_icon_color, edit_icon_color);  // 0xFE = TopBar mode with Leave red
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
							uint8_t (*sched_ptr)[4] = schedule_edit_source ? schedule_settings_restday : schedule_settings;
							schedule_time_edit_hour = sched_ptr[schedule_time_setting_period - 1][0];
							schedule_time_edit_min = sched_ptr[schedule_time_setting_period - 1][1];
							schedule_time_edit_temp = sched_ptr[schedule_time_setting_period - 1][2];
							schedule_time_on_off = sched_ptr[schedule_time_setting_period - 1][3];
						}
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Schedule_Time_Setting_Page(schedule_time_setting_period, leave_icon_color, edit_icon_color);
					}
					delay_ms(100);
				}
			}
	}
	else if(UI_state == STATE_SCHEDULE_TIME_SETTING)
	{
			// Schedule Time Setting page navigation
			if(Top_Bar_Active)  // TopBar mode: Edit/Leave
			{
				if(key == 1 || key == 2)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key == 3)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter Hour
						item_selection = 1;
						// Initialize edit values from schedule_settings array (Workday or Restday)
						//{
						//	uint8_t (*sched_ptr)[4] = schedule_edit_source ? schedule_settings_restday : schedule_settings;
						//	schedule_time_edit_hour = sched_ptr[schedule_time_setting_period - 1][0];
						//	schedule_time_edit_min = sched_ptr[schedule_time_setting_period - 1][1];
						//	schedule_time_edit_temp = sched_ptr[schedule_time_setting_period - 1][2];
						//	schedule_time_on_off = sched_ptr[schedule_time_setting_period - 1][3];
						//}
						// Update Top Bar (both icons black in edit mode) and draw hour arrows
						leave_icon_color = 0;
						edit_icon_color = 0;
						Top_Bar_Active = 0;
						Draw_TopBar(leave_icon_color, edit_icon_color);
						Draw_Schedule_Time_Setting_Arrows(Hour);
					}
					else  // Leave Icon is red
					{
						// Go back to Schedule Edit
						UI_state = STATE_SCHEDULE_EDIT;
						Top_Bar_Active = 1;
						leave_icon_color = 1;
						edit_icon_color = 0;
						item_selection = 0;
						//schedule_edit_sel = schedule_time_setting_period - 1;  // P1-P6 -> 0-5
						// Set scroll based on selection to ensure correct rows are displayed
						if(item_selection <= 4) {
							schedule_edit_scroll = 0;  // P1-P4 visible
						} else {
							schedule_edit_scroll = item_selection - 4;  // Ensure selected item is at bottom row
						}
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Schedule_Edit_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					delay_ms(100);
				}
			}
			else if(item_selection == 1)  // Edit Hour
			{
				if(key == 1)  // Up key - increase hour
				{
					if(schedule_time_edit_hour < 23) schedule_time_edit_hour++;
					else schedule_time_edit_hour = 0;
					// Only update time digits
					Draw_Schedule_Time_Setting_TimeDigits();
					delay_ms(100);
				}
				else if(key == 2)  // Down key - decrease hour
				{
					if(schedule_time_edit_hour > 0) schedule_time_edit_hour--;
					else schedule_time_edit_hour = 23;
					// Only update time digits
					Draw_Schedule_Time_Setting_TimeDigits();
					delay_ms(100);
				}
				else if(key == 3)  // Enter key - move to minute edit
				{
					item_selection = 2;  // edit min
					Clear_Schedule_Time_Setting_Arror(Hour);
					Draw_Schedule_Time_Setting_Arrows(Minume);
					delay_ms(100);
				}
			}
			else if(item_selection == 2)  // Edit Minume
			{
				if(key == 1)  // Up key - increase minute
				{
					if(schedule_time_edit_min < 59) schedule_time_edit_min++;
					else schedule_time_edit_min = 0;
					// Only update time digits
					Draw_Schedule_Time_Setting_TimeDigits();
					delay_ms(100);
				}
				else if(key == 2)  // Down key - decrease minute
				{
					if(schedule_time_edit_min > 0) schedule_time_edit_min--;
					else schedule_time_edit_min = 59;
					// Only update time digits
					Draw_Schedule_Time_Setting_TimeDigits();
					delay_ms(100);
				}
				else if(key == 3)  // Enter key - move to temperature edit
				{
					item_selection = 3;  // Edit Temperature
					// Clear all arrows and draw temperature arrows only
					Clear_Schedule_Time_Setting_Arror(Minume);
					Draw_Schedule_Time_Setting_Arrows(Temperature);
					delay_ms(100);
				}
			}
			else if(item_selection == 3)  // Edit Temperature
			{
				if(key == 1)  // Up key - increase temperature
				{
					if(schedule_time_edit_temp < MAX_SET_TEMP) schedule_time_edit_temp++;
					// Only update temperature digits
					Draw_Schedule_Time_Setting_TempDigits();
					delay_ms(100);
				}
				else if(key == 2)  // Down key - decrease temperature
				{
					if(schedule_time_edit_temp > 0) schedule_time_edit_temp--;
					// Only update temperature digits
					Draw_Schedule_Time_Setting_TempDigits();
					delay_ms(100);
				}
				else if(key == 3)  // Enter key - move to ON/OFF edit
				{
					item_selection = 4;  // Edit ON/OFF
					// Clear all arrows and update ON/OFF section
					//Draw_Schedule_Time_Setting_Arrows(Time_Clear);
					Clear_Schedule_Time_Setting_Arror(Temperature);
					Draw_Schedule_Time_Setting_OnOff();
					delay_ms(100);
				}
			}
			else if(item_selection == 4)  // Edit ON/OFF
			{
				if(key == 1 || key == 2)  // Up or Down key - toggle ON/OFF
				{
					schedule_time_on_off = !schedule_time_on_off;
					// Only update ON/OFF section
					Draw_Schedule_Time_Setting_OnOff();
					delay_ms(100);
				}
				else if(key == 3)  // Enter key - move to Save
				{
					item_selection = 5;  // Save mode
					// Update ON/OFF section (exit edit mode) and Save button
					Draw_Schedule_Time_Setting_OnOff();
					Draw_Schedule_Time_Setting_Save();
					delay_ms(100);
				}
			}
			else if(item_selection == 5)  // Save mode
			{
				if(key == 3)  // Enter key pressed
				{
					// Save settings to array (P1~P6 index 0~5, Workday or Restday)
					{
						uint8_t idx = schedule_time_setting_period - 1;
						uint8_t (*sched_ptr)[4] = schedule_edit_source ? schedule_settings_restday : schedule_settings;
						sched_ptr[idx][0] = schedule_time_edit_hour;
						sched_ptr[idx][1] = schedule_time_edit_min;
						sched_ptr[idx][2] = schedule_time_edit_temp;
						sched_ptr[idx][3] = schedule_time_on_off;
					}
					
					// Save and go back to Schedule Edit
					UI_state = STATE_SCHEDULE_EDIT;
					schedule_edit_substate = 1;
					item_selection = schedule_time_setting_period;  // P1-P6 -> 1-6
					// Set scroll based on selection to ensure correct rows are displayed
					if(item_selection <= 4) {
						schedule_edit_scroll = 0;  // P1-P4 visible
					} else {
						schedule_edit_scroll = item_selection - 4;  // Ensure selected item is at bottom row
					}
					Draw_Schedule_Edit_Page(item_selection, leave_icon_color, edit_icon_color);
					delay_ms(100);
				}
			}
	}
	else if(UI_state == STATE_CONTROL_ADJ_MENU)
	{
			// Control Adj Menu page navigation
			if(Top_Bar_Active)  // TopBar mode: Edit/Leave
			{
				if(key == 1 || key == 2)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key == 3)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter選項模式, Edit/Leave both black, Sensor selected
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
						Top_Bar_Active = 0;
						item_selection = 2;
						leave_icon_color = 0;
						edit_icon_color = 0;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Function_Setting_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					delay_ms(100);
				}
			}
			else //if(control_adj_menu_substate == 1)  // 選項模式: Sensor/Temp.Correct/Temp.Limit/Temp.Protect/Power On State
			{
				if(key == 2)  // Down key pressed
				{
					if(item_selection < 5)  // Can go down (max=4, Power On State)
					{
						uint8_t old_sel = item_selection;
						uint8_t old_scroll = control_adj_menu_scroll;
						item_selection++;
						
						// Check if scroll position needs to change
						// 4 visible rows at a time, scroll when selection goes below visible area
						if(item_selection > control_adj_menu_scroll + 4) {
							control_adj_menu_scroll = item_selection - 4;
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
					delay_ms(100);
				}
				else if(key == 1)  // Up key pressed
				{
					if(item_selection > 1)
					{
						uint8_t old_sel = item_selection;
						uint8_t old_scroll = control_adj_menu_scroll;
						item_selection--;
						
						// Check if scroll position needs to change
						if(item_selection - 1 < control_adj_menu_scroll + 1) {
							control_adj_menu_scroll = 0;
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
					delay_ms(100);
				}
				else if(key == 3)  // Enter key pressed
				{
					if(item_selection == 1)  // Sensor selected
					{
						// Enter Sensor Setting頁面
						UI_state = STATE_CONTROL_ADJ_SENSOR;
						Top_Bar_Active = 1;
						item_selection = 0;
						leave_icon_color = 0;
						edit_icon_color = 1;
						control_adj_sensor_sel = current_sensor_type;  
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Control_Adj_Sensor_Page(item_selection, leave_icon_color, edit_icon_color);  // 0xFF = TopBar mode, Edit red
					}
					else if(item_selection == 2)  // Temp. Correct selected
					{
						// Enter Temp. Correct Setting頁面
						UI_state = STATE_CONTROL_ADJ_TEMP_CORRECT;
						Top_Bar_Active = 1;
						leave_icon_color = 0;
						edit_icon_color = 1;
						item_selection = 0;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Control_Adj_TempCorrect_Page(item_selection, leave_icon_color, edit_icon_color); // 0xFF = TopBar mode, Edit red
					}
					else if(item_selection == 3)		//Temp. Limit selected
					{
							UI_state = STATE_CONTROL_ADJ_TEMP_LIMIT;
							Top_Bar_Active = 1;
							leave_icon_color = 0;
							edit_icon_color = 1;
							item_selection = 0;
							LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
							Draw_Control_Adj_TempLimit_Page(item_selection, leave_icon_color, edit_icon_color);
							
					}
					else if(item_selection == 4)
					{
							UI_state = STATE_CONTROL_ADJ_TEMP_PROTECT;
							Top_Bar_Active = 1;
							leave_icon_color = 0;
							edit_icon_color = 1;
							item_selection = 0;
							LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
							Draw_Control_Adj_TempProtect_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					else if(item_selection == 5)
					{
							UI_state = STATE_CONTROL_ADJ_POWER_ON_STATE;
							Top_Bar_Active = 1;
							leave_icon_color = 0;
							edit_icon_color = 1;
							item_selection = 0;
							LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
							Draw_Control_Adj_Power_On_State_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					// TODO: 其他選項的子頁面處理 (Temp.Limit, Temp.Protect, Power On State)
					delay_ms(100);
				}
			}
	}
	else if(UI_state == STATE_CONTROL_ADJ_SENSOR)
	{
			// Control Adj Sensor Setting page navigation
			if(Top_Bar_Active == 1)  // TopBar mode: Edit/Leave
			{
				if(key == 1 || key == 2)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key == 3)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter Floor/Room選項模式
						Top_Bar_Active = 0;
						edit_icon_color = 0;
						leave_icon_color = 0;
						item_selection = 1;
						Draw_Control_Adj_Sensor_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					else  // Leave Icon is red
					{
						// Go back to Control Adj Menu with Leave red
						UI_state = STATE_CONTROL_ADJ_MENU;
						Top_Bar_Active = 0;
						leave_icon_color = 0;
						edit_icon_color = 0;
						item_selection = 1;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Control_Adj_Menu_Page(item_selection, leave_icon_color, edit_icon_color);  // 0xFE = TopBar mode, Leave red
					}
					delay_ms(100);
				}
			}
			else if(item_selection == 1)  // Room/Floor選項模式 (swapped order)
			{
				if(key == 2)  // Down key pressed
				{
					if(control_adj_sensor_sel < 1)  // Can go to Floor (1)
					{
						// Move selection down (Room -> Floor)
						control_adj_sensor_sel++;
						// Update choices display
						Draw_Control_Adj_Sensor_Choices(control_adj_sensor_sel + 1);
					}
					delay_ms(100);
				}
				else if(key == 1)  // Up key pressed
				{
					if(control_adj_sensor_sel > 0)  // Can go to Room (0)
					{
						// Move selection up (Floor -> Room)
						control_adj_sensor_sel--;
						// Update choices display
						Draw_Control_Adj_Sensor_Choices(control_adj_sensor_sel + 1);
					}
					else  // At Room, go back to TopBar mode
					{
						Top_Bar_Active = 1;
						edit_icon_color = 1;
						leave_icon_color = 0;
						item_selection = 0;
						Draw_Control_Adj_Sensor_Page(item_selection, leave_icon_color, edit_icon_color);  
					}
					delay_ms(100);
				}
				else if(key == 3)  // Enter key pressed
				{
					// Enter Save mode
					item_selection = 2;
					Draw_Control_Adj_Sensor_Save(1);  // Save red
					delay_ms(100);
				}
			}
			else if(item_selection == 2)  // Save模式
			{
				if(key == 3)  // Enter key pressed
				{
					// Save setting and return to Control Adj Menu
					current_sensor_type = control_adj_sensor_sel;  // 0=Room, 1=Floor (swapped order)
					UI_state = STATE_CONTROL_ADJ_MENU;
					Top_Bar_Active = 0;
					leave_icon_color = 0;
					edit_icon_color = 0;
					item_selection = 1;
					LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
					Draw_Control_Adj_Menu_Page(item_selection, leave_icon_color, edit_icon_color);  
					delay_ms(100);
				}
				else if(key == 1 || key == 2)  // Up or Down key in Save mode - cancel save
				{
					// Cancel Save mode, back to choices mode
					//item_selection = 1;
					//Draw_Control_Adj_Sensor_Choices(control_adj_sensor_sel + 1);
					//delay_ms(100);
				}
			}
	}
	else if(UI_state == STATE_CONTROL_ADJ_TEMP_CORRECT)
	{
		if(Top_Bar_Active == 1)  // TopBar mode: Edit/Leave
			{
				if(key == 1 || key == 2)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key == 3)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter Correct num setting
						Top_Bar_Active = 0;
						edit_icon_color = 0;
						leave_icon_color = 0;
						item_selection = 1;
						Draw_TopBar(leave_icon_color, edit_icon_color);
						Draw_Control_Adj_TempCorrect_Contect(item_selection);
					}
					else  // Leave Icon is red
					{
						// Go back to Control Adj Menu with Leave red
						UI_state = STATE_CONTROL_ADJ_MENU;
						Top_Bar_Active = 0;
						leave_icon_color = 0;
						edit_icon_color = 0;
						item_selection = 2;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Control_Adj_Menu_Page(item_selection, leave_icon_color, edit_icon_color);  // 0xFE = TopBar mode, Leave red
					}
					delay_ms(100);
				}
			}
			else if(item_selection == 1)		//internal sensor setting
			{
					if(key == 1)  // Up key pressed
					{
							if(temp_correct_internal < 5.0){
									temp_correct_internal += 0.5;
									Draw_Control_Adj_TempCorrect_Contect(item_selection);
							}
					}
					else if(key == 2) 	// Down key pressed
					{
							if(temp_correct_internal > -5.0){
									temp_correct_internal -= 0.5;
									Draw_Control_Adj_TempCorrect_Contect(item_selection);
							}
					}
					else if(key ==3)		// Enter key pressed
					{
							item_selection = 2;
							Draw_Control_Adj_TempCorrect_Contect(item_selection);
					}
					//delay_ms(100);
			}
			else if(item_selection == 2)  //external sensor setting
			{
					if(key == 1)  // Up key pressed
					{
							if(temp_correct_external < 5.0){
									temp_correct_external += 0.5;
									Draw_Control_Adj_TempCorrect_Contect(item_selection);
							}
					}
					else if(key == 2) 	// Down key pressed
					{
							if(temp_correct_external > -5.0){
									temp_correct_external -= 0.5;
									Draw_Control_Adj_TempCorrect_Contect(item_selection);
							}
					}
					else if(key ==3)		// Enter key pressed
					{
							item_selection = 3;
							Draw_Control_Adj_TempCorrect_Contect(item_selection);
					}
					//delay_ms(100);
			}
			else if(item_selection == 3)	//enter save state
			{
					if(key == 3)		//skip Up/Down key
					{
							UI_state = STATE_CONTROL_ADJ_MENU;
							Top_Bar_Active = 0;
							leave_icon_color = 0;
							edit_icon_color = 0;
							item_selection = 2;
							LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
							Draw_Control_Adj_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}
			}
	}
	else if(UI_state == STATE_CONTROL_ADJ_TEMP_LIMIT)
	{
		if(Top_Bar_Active == 1)  // TopBar mode: Edit/Leave
		{
				if(key == 1 || key == 2)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key == 3)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter Correct num setting
						Top_Bar_Active = 0;
						edit_icon_color = 0;
						leave_icon_color = 0;
						item_selection = 1;
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
						item_selection = 3;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Control_Adj_Menu_Page(item_selection, leave_icon_color, edit_icon_color);  // 0xFE = TopBar mode, Leave red
					}
					delay_ms(100);
				}
		}
		else if(item_selection == 1)	//Setup temp max limit
		{
					if(key == 1)  // Up key pressed
					{
							if(temp_limit_max < 60){
									temp_limit_max += 1;
									Draw_Control_Adj_TempLimit_Content(item_selection);
							}
					}
					else if(key == 2) 	// Down key pressed
					{
							if(temp_limit_max > 20){
									temp_limit_max -= 1;
									Draw_Control_Adj_TempLimit_Content(item_selection);
							}
					}
					else if(key ==3)		// Enter key pressed
					{
							item_selection = 2;
							Draw_Control_Adj_TempLimit_Content(item_selection);
					}
		}
		else if(item_selection == 2)		//setup temp min limit
		{
					if(key == 1)  // Up key pressed
					{
							if(temp_limit_min < 10){
									temp_limit_min += 1;
									Draw_Control_Adj_TempLimit_Content(item_selection);
							}
					}
					else if(key == 2) 	// Down key pressed
					{
							if(temp_limit_min > -10){
									temp_limit_min -= 1;
									Draw_Control_Adj_TempLimit_Content(item_selection);
							}
					}
					else if(key ==3)		// Enter key pressed
					{
							item_selection = 3;
							Draw_Control_Adj_TempLimit_Content(item_selection);
					}
		}
		else if(item_selection == 3)
		{
					if(key == 3)		//skip Up/Down key
					{
							UI_state = STATE_CONTROL_ADJ_MENU;
							Top_Bar_Active = 0;
							leave_icon_color = 0;
							edit_icon_color = 0;
							item_selection = 3;
							LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
							Draw_Control_Adj_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}
		}
		
	}
	else if(UI_state == STATE_CONTROL_ADJ_TEMP_PROTECT)
	{
		if(Top_Bar_Active == 1)  // TopBar mode: Edit/Leave
		{
				if(key == 1 || key == 2)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key == 3)  // Enter key pressed
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
						item_selection = 4;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Control_Adj_Menu_Page(item_selection, leave_icon_color, edit_icon_color);  
					}
					delay_ms(100);
				}
		}
		else 
		{
				if(key == 2)	//Down Key
				{
						if(item_selection < 2){
								item_selection++;
								Draw_Control_Adj_TempProtect_Page(item_selection, leave_icon_color, edit_icon_color);
						}
				}else if(key == 1){		//Up Key
						if(item_selection == 2){
								item_selection--;
								Draw_Control_Adj_TempProtect_Page(item_selection, leave_icon_color, edit_icon_color);
						}else if(item_selection == 1){		//go back Top_Bar Active
								item_selection = 0;
								leave_icon_color = 0;
								edit_icon_color = 1;
								Top_Bar_Active = 1;
								Draw_Control_Adj_TempProtect_Page(item_selection, leave_icon_color, edit_icon_color);
						}
				}else if(key == 3){		//Enter Key
						if(item_selection == 1){		//Max Temp Protect Setting
								UI_state = STATE_CONTROL_ADJ_TEMP_PROTECT_MAX;
								Top_Bar_Active = 1;
								leave_icon_color = 0;
								edit_icon_color = 1;
								item_selection = 0;
								LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
								Draw_Control_Adj_TempProtect_Max_Page(item_selection, leave_icon_color, edit_icon_color);
						}else if(item_selection == 2){		//Min Temp Protect Setting
								UI_state = STATE_CONTROL_ADJ_TEMP_PROTECT_MIN;
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
				if(key == 1 || key == 2)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key == 3)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter Correct num setting
						Top_Bar_Active = 0;
						edit_icon_color = 0;
						leave_icon_color = 0;
						item_selection = 1;
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
					delay_ms(100);
				}
		}
		else if(item_selection == 1)
		{
				if(key == 1){		//Up key
						if(temp_protect_max < 70){
								temp_protect_max++;
								 Draw_Control_Adj_TempProtect_Max_Content(item_selection);
						}
				}else if(key == 2){		//Down Key
						if(temp_protect_max > 45){
								temp_protect_max--;
								Draw_Control_Adj_TempProtect_Max_Content(item_selection);
						}
				}else if(key == 3){		//ENTER Key
						item_selection = 2;
						Draw_Control_Adj_TempProtect_Max_Content(item_selection);
				}
		}
		else if(item_selection == 2)
		{
				if((key == 1) || (key == 2)){		//Up Key or Down key
						if(temp_protect_max_switch){
								temp_protect_max_switch = 0;
						}else{
								temp_protect_max_switch = 1;
						}
						Draw_Control_Adj_TempProtect_Max_Content(item_selection);
				}
				else if(key == 3){
						item_selection = 3;
						Draw_Control_Adj_TempProtect_Max_Content(item_selection);
				}
		}
		else if(item_selection == 3)
		{
				if(key == 3){
						UI_state = STATE_CONTROL_ADJ_TEMP_PROTECT;
						Top_Bar_Active = 0;
						leave_icon_color = 0;
						edit_icon_color = 0;
						item_selection = 1;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Control_Adj_TempProtect_Page(item_selection, leave_icon_color, edit_icon_color);
				}
		}
	}
	else if(UI_state == STATE_CONTROL_ADJ_TEMP_PROTECT_MIN)
	{
		if(Top_Bar_Active == 1)  // TopBar mode: Edit/Leave
		{
				if(key == 1 || key == 2)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key == 3)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter Correct num setting
						Top_Bar_Active = 0;
						edit_icon_color = 0;
						leave_icon_color = 0;
						item_selection = 1;
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
					delay_ms(100);
				}
		}
		else if(item_selection == 1)
		{
				if(key == 1){		//Up key
						if(temp_protect_min < 10){
								temp_protect_min++;
								 Draw_Control_Adj_TempProtect_Min_Content(item_selection);
						}
				}else if(key == 2){		//Down Key
						if(temp_protect_min > -10){
								temp_protect_min--;
								Draw_Control_Adj_TempProtect_Min_Content(item_selection);
						}
				}else if(key == 3){		//ENTER Key
						item_selection = 2;
						Draw_Control_Adj_TempProtect_Min_Content(item_selection);
				}
		}
		else if(item_selection == 2)
		{
				if((key == 1) || (key == 2)){		//Up Key or Down key
						if(temp_protect_min_switch){
								temp_protect_min_switch = 0;
						}else{
								temp_protect_min_switch = 1;
						}
						Draw_Control_Adj_TempProtect_Min_Content(item_selection);
				}
				else if(key == 3){
						item_selection = 3;
						Draw_Control_Adj_TempProtect_Min_Content(item_selection);
				}
		}
		else if(item_selection == 3)
		{
				if(key == 3){
						UI_state = STATE_CONTROL_ADJ_TEMP_PROTECT;
						Top_Bar_Active = 0;
						leave_icon_color = 0;
						edit_icon_color = 0;
						item_selection = 2;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Control_Adj_TempProtect_Page(item_selection, leave_icon_color, edit_icon_color);
				}
		}
	}
	else if(UI_state == STATE_CONTROL_ADJ_POWER_ON_STATE)
	{
		if(Top_Bar_Active == 1)  // TopBar mode: Edit/Leave
		{
				if(key == 1 || key == 2)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key == 3)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter Correct num setting
						Top_Bar_Active = 0;
						edit_icon_color = 0;
						leave_icon_color = 0;
						item_selection = 1;
						Draw_Control_Adj_Power_On_State_Arrow(item_selection);
					}
					else  // Leave Icon is red
					{
						// Go back to Control Adj Menu with Leave red
						UI_state = STATE_CONTROL_ADJ_MENU;
						Top_Bar_Active = 0;
						leave_icon_color = 0;
						edit_icon_color = 0;
						item_selection = 5;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Control_Adj_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					delay_ms(100);
				}
		}
		else if((item_selection == 1) || (item_selection == 2) || (item_selection == 3))
		{
				if(key == 1){		//Up key
						if(item_selection == 1){	//go back to Top Bar
								Top_Bar_Active = 1;
								leave_icon_color = 0;
								edit_icon_color = 1;
								item_selection = 0;
								Draw_Control_Adj_Power_On_State_Page(item_selection, leave_icon_color, edit_icon_color);
						}else{
								item_selection--;
								Draw_Control_Adj_Power_On_State_Arrow(item_selection);
						}
				}else if(key == 2){
						if(item_selection < 3){
								item_selection++;
								Draw_Control_Adj_Power_On_State_Arrow(item_selection);
						}
				}else if(key == 3){
						power_on_state = item_selection;
						item_selection = 4;
						Draw_Control_Adj_Power_On_State_Arrow(item_selection);
				}
		}else if(item_selection == 4){
				if(key == 3){
						UI_state = STATE_CONTROL_ADJ_MENU;
						Top_Bar_Active = 0;
						leave_icon_color = 0;
						edit_icon_color = 0;
						item_selection = 5;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						//control_adj_menu_scroll = 1;
						Draw_Control_Adj_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
				}
		}
	}
	else if(UI_state == STATE_USER_SETTING_MENU)
	{
			// Control Adj Menu page navigation
			if(Top_Bar_Active)  // TopBar mode: Edit/Leave
			{
				if(key == 1 || key == 2)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key == 3)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter選項模式, Edit/Leave both black, Sensor selected
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
						Top_Bar_Active = 0;
						item_selection = 3;
						leave_icon_color = 0;
						edit_icon_color = 0;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Function_Setting_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					delay_ms(100);
				}
			}
			else //if(control_adj_menu_substate == 1)  // 選項模式: Sensor/Temp.Correct/Temp.Limit/Temp.Protect/Power On State
			{
				if(key == 2)  // Down key pressed
				{
					if(item_selection < 5)  // Can go down (max=4, Power On State)
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
					delay_ms(100);
				}
				else if(key == 1)  // Up key pressed
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
					delay_ms(100);
				}
				else if(key == 3)  // Enter key pressed
				{
					if(item_selection == 1)  // Sensor selected
					{
						// Enter Child lock page
						UI_state = STATE_USER_SETTING_CHILD_LOCK;
						Top_Bar_Active = 1;
						item_selection = 0;
						leave_icon_color = 0;
						edit_icon_color = 1; 
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_User_Setting_Child_Lock_Page(item_selection, leave_icon_color, edit_icon_color);  // 0xFF = TopBar mode, Edit red
					}
					else if(item_selection == 2)  // Temp. Correct selected
					{
						// Enter Window Function page
						UI_state = STATE_USER_SETTING_WINDOW_FUN;
						Top_Bar_Active = 1;
						leave_icon_color = 0;
						edit_icon_color = 1;
						item_selection = 0;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_User_Setting_Window_Fun_Page(item_selection, leave_icon_color, edit_icon_color); // 0xFF = TopBar mode, Edit red
					}
					else if(item_selection == 3)		//Temp. Limit selected
					{
							UI_state = STATE_USER_SETTING_SET_TIME;
							Top_Bar_Active = 1;
							leave_icon_color = 0;
							edit_icon_color = 1;
							item_selection = 0;
							LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
							Draw_User_Setting_SetTime_Page(item_selection, leave_icon_color, edit_icon_color);
							
					}
					else if(item_selection == 4)
					{
							UI_state = STATE_USER_SETTING_BACKLIGHT;
							Top_Bar_Active = 1;
							leave_icon_color = 0;
							edit_icon_color = 1;
							item_selection = 0;
							LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
							Draw_User_Setting_Backlight_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					else if(item_selection == 5)
					{
							UI_state = STATE_USER_SETTING_RESET;
							Top_Bar_Active = 1;
							leave_icon_color = 0;
							edit_icon_color = 1;
							item_selection = 0;
							LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
							Draw_User_Setting_Reset_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					
					delay_ms(100);
				}
			}
	}
	else if(UI_state == STATE_USER_SETTING_CHILD_LOCK)
	{
		// User setting child lock page navigation
			if(Top_Bar_Active)  // TopBar mode: Edit/Leave
			{
				if(key == 1 || key == 2)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key == 3)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter選項模式, Edit/Leave both black, Sensor selected
						Top_Bar_Active = 0;
						item_selection = 1;
						leave_icon_color = 0;
						edit_icon_color = 0;
						Draw_User_Setting_Child_Lock_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					else  // Leave Icon is red
					{
						// Go back to Function Setting Edit mode with cursor on Control Adj
						UI_state = STATE_USER_SETTING_MENU;
						Top_Bar_Active = 0;
						item_selection = 1;
						leave_icon_color = 0;
						edit_icon_color = 0;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_User_Setting_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					delay_ms(100);
				}
			}
			else if((item_selection == 1) || (item_selection == 2))
			{
					if(key == 2){		//Down key
							if(item_selection == 1){
									item_selection = 2;
									Draw_User_Setting_Child_Lock_Choice(item_selection);
							}
							//delay_ms(100);
					}else if(key == 1){		//Up key
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
							//delay_ms(100);
					}else if(key == 3){		//Enter key
							if(item_selection == 1){
									child_lock = 0;
							}else if(item_selection == 2){
									child_lock = 1;
							}
							item_selection = 3;
							Draw_User_Setting_Child_Lock_Page(item_selection, leave_icon_color, edit_icon_color);
							//delay_ms(100);
					}
			}else if(item_selection == 3){
					if(key == 3){
							UI_state = STATE_USER_SETTING_MENU;
							Top_Bar_Active = 0;
							item_selection = 1;
							leave_icon_color = 0;
							edit_icon_color = 0;
							LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
							Draw_User_Setting_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}
			}
	}
	else if(UI_state == STATE_USER_SETTING_WINDOW_FUN)
	{
			if(Top_Bar_Active)  // TopBar mode: Edit/Leave
			{
				if(key == 1 || key == 2)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key == 3)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter選項模式, Edit/Leave both black, Sensor selected
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
						item_selection = 2;
						leave_icon_color = 0;
						edit_icon_color = 0;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_User_Setting_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					delay_ms(100);
				}
			}else if(item_selection == 1){
					if((key == 2) || (key == 1)){		
							if(window_fun){
									window_fun = 0;
							}else{
									window_fun = 1;
							}
							Draw_User_Setting_Window_Fun_Content(item_selection, leave_icon_color, edit_icon_color);
					}
					if(key == 3){
							if(window_fun){
									item_selection = 2;
									Draw_User_Setting_Window_Fun_Content(item_selection, leave_icon_color, edit_icon_color);
							}else{
									UI_state = STATE_USER_SETTING_MENU;
									Top_Bar_Active = 0;
									item_selection = 2;
									leave_icon_color = 0;
									edit_icon_color = 0;
									LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
									Draw_User_Setting_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
							}
					}
			}else if(item_selection == 2){
					if(key == 3){
							UI_state = STATE_USER_SETTING_WINDOW_TIME;
							Top_Bar_Active = 1;
							item_selection = 0;
							leave_icon_color = 0;
							edit_icon_color = 1;
							LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
							Draw_User_Setting_Window_Time_Page(item_selection, leave_icon_color, edit_icon_color);
					}
			}
	}
	else if(UI_state == STATE_USER_SETTING_WINDOW_TIME){
			if(Top_Bar_Active)  // TopBar mode: Edit/Leave
			{
				if(key == 1 || key == 2)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key == 3)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter選項模式, Edit/Leave both black, Sensor selected
						Top_Bar_Active = 0;
						item_selection = 1;
						leave_icon_color = 0;
						edit_icon_color = 0;
						Draw_User_Setting_Window_Time_Content(item_selection);
					}
					else  // Leave Icon is red
					{
						// Go back to Function Setting Edit mode with cursor on Control Adj
						UI_state = STATE_USER_SETTING_WINDOW_FUN;
						Top_Bar_Active = 0;
						item_selection = 2;
						leave_icon_color = 0;
						edit_icon_color = 0;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_User_Setting_Window_Fun_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					delay_ms(100);
				}
			}else if(item_selection == 1){
					if(key == 2){		//Down key
							if(window_fun_temp > 3){
									window_fun_temp--;
									Draw_User_Setting_Window_Time_Content(item_selection);
							}
					}else if(key == 1){		//Up key
							if(window_fun_temp < 20){
									window_fun_temp++;
									Draw_User_Setting_Window_Time_Content(item_selection);
							}
					}else if(key == 3){		//Enter key
							item_selection = 2;
							Draw_User_Setting_Window_Time_Content(item_selection);
					}
			}else if(item_selection == 2){
					if(key == 2){		//Down key
							if(window_fun_time > 5){
									window_fun_time--;
									Draw_User_Setting_Window_Time_Content(item_selection);
							}
					}else if(key == 1){		//Up key
							if(window_fun_time < 60){
									window_fun_time++;
									Draw_User_Setting_Window_Time_Content(item_selection);
							}
					}else if(key == 3){		//Enter key
							item_selection = 3;
							Draw_User_Setting_Window_Time_Content(item_selection);
					}
			}else if(item_selection == 3){
					if(key == 3){
							UI_state = STATE_USER_SETTING_MENU;
							Top_Bar_Active = 0;
							item_selection = 2;
							leave_icon_color = 0;
							edit_icon_color = 0;
							LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
							Draw_User_Setting_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}
			}
	}
	else if(UI_state == STATE_USER_SETTING_SET_TIME)
	{
			if(Top_Bar_Active)  // TopBar mode: Edit/Leave
			{
				if(key == 1 || key == 2)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key == 3)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter選項模式, Edit/Leave both black, Sensor selected
						Top_Bar_Active = 0;
						item_selection = 1;
						leave_icon_color = 0;
						edit_icon_color = 0;
						Draw_TopBar(leave_icon_color, edit_icon_color);
						Draw_User_Setting_SetTime_Content(item_selection);
					}
					else  // Leave Icon is red
					{
						// Go back to Function Setting Edit mode with cursor on Control Adj
						UI_state = STATE_USER_SETTING_MENU;
						Top_Bar_Active = 0;
						item_selection = 3;
						leave_icon_color = 0;
						edit_icon_color = 0;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_User_Setting_Window_Fun_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					delay_ms(100);
				}
			}else if(item_selection == 1){
					if(key == 2){		//Down key
							if(rtc_time.Year > 26){
									rtc_time.Year--;
									Draw_User_Setting_SetTime_Content(item_selection);
							}
					}else if(key == 1){		//Up key
							if(rtc_time.Year < 99){
									rtc_time.Year++;
									Draw_User_Setting_SetTime_Content(item_selection);
							}
					}else if(key == 3){		//Enter key
							item_selection = 2;
							Draw_User_Setting_SetTime_Content(item_selection);
					}
			}else if(item_selection == 2){
					if(key == 2){		//Down key
							if(rtc_time.Mon > 1){
									rtc_time.Mon--;
									Draw_User_Setting_SetTime_Content(item_selection);
							}
					}else if(key == 1){		//Up key
							if(rtc_time.Mon < 12){
									rtc_time.Mon++;
									Draw_User_Setting_SetTime_Content(item_selection);
							}
					}else if(key == 3){		//Enter key
							item_selection = 3;
							Draw_User_Setting_SetTime_Content(item_selection);
					}
			}else if(item_selection == 3){
					if(key == 2){		//Down key
							if(rtc_time.Date > 1){
									rtc_time.Mon--;
									Draw_User_Setting_SetTime_Content(item_selection);
							}
					}else if(key == 1){		//Up key
							if((rtc_time.Mon == 1) || (rtc_time.Mon == 3) || (rtc_time.Mon == 5) || (rtc_time.Mon == 7) || (rtc_time.Mon == 8) || (rtc_time.Mon == 10) || (rtc_time.Mon == 12)){
									if(rtc_time.Date < 31){
											rtc_time.Date++;
											Draw_User_Setting_SetTime_Content(item_selection);
									}
							}else if((rtc_time.Mon == 4) || (rtc_time.Mon == 6) || (rtc_time.Mon == 9) || (rtc_time.Mon == 11)){
									if(rtc_time.Date < 30){
											rtc_time.Date++;
											Draw_User_Setting_SetTime_Content(item_selection);
									}
							}else if(rtc_time.Mon == 2){
									if((rtc_time.Year%4) == 0){
											if(rtc_time.Date < 29){
													rtc_time.Date++;
											}
									}else{
											if(rtc_time.Date < 28){
													rtc_time.Date++;
											}
									}
									Draw_User_Setting_SetTime_Content(item_selection);
							}
					}else if(key == 3){		//Enter key
							item_selection = 4;
							Draw_User_Setting_SetTime_Content(item_selection);
					}
			}else if(item_selection == 4){
					if((key == 1) || (key == 2)){
							item_selection = 1;
							Draw_User_Setting_SetTime_Content(item_selection);
							Show_Str(87, 108, BLACK, WHITE, "Set Clock", 16, 0);
					}else if(key == 3){
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
				if(key == 1 || key == 2)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key == 3)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter選項模式, Edit/Leave both black, Sensor selected
						Top_Bar_Active = 0;
						item_selection = 1;
						leave_icon_color = 0;
						edit_icon_color = 0;
						Draw_TopBar(leave_icon_color, edit_icon_color);
						Draw_User_Setting_Setclk_Content(item_selection);
					}
					else  // Leave Icon is red
					{
						// Go back to Function Setting Edit mode with cursor on Control Adj
						UI_state = STATE_USER_SETTING_SET_TIME;
						Top_Bar_Active = 1;
						item_selection = 0;
						leave_icon_color = 1;
						edit_icon_color = 0;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_User_Setting_SetTime_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					delay_ms(100);
				}
			}else if(item_selection == 1){
					if(key == 2){		//Down key
							if(rtc_time.Hour > 0){
									rtc_time.Hour--;
									Draw_User_Setting_Setclk_Content(item_selection);
							}
					}else if(key == 1){		//Up key
							if(rtc_time.Hour < 23){
									rtc_time.Hour++;
									Draw_User_Setting_Setclk_Content(item_selection);
							}
					}else if(key == 3){		//Enter key
							item_selection = 2;
							Draw_User_Setting_Setclk_Content(item_selection);
					}
			}else if(item_selection == 2){
					if(key == 2){		//Down key
							if(rtc_time.Min > 0){
									rtc_time.Min--;
									Draw_User_Setting_Setclk_Content(item_selection);
							}
					}else if(key == 1){		//Up key
							if(rtc_time.Min < 60){
									rtc_time.Min++;
									Draw_User_Setting_Setclk_Content(item_selection);
							}
					}else if(key == 3){		//Enter key
							item_selection = 3;
							Draw_User_Setting_Setclk_Content(item_selection);
					}
			}else if(item_selection == 3){
					if((key == 1) || (key == 2)){
							item_selection = 1;
							Draw_User_Setting_Setclk_Content(item_selection);
					}else if(key == 3){
							UI_state = STATE_USER_SETTING_MENU;
							Top_Bar_Active = 0;
							item_selection = 3;
							leave_icon_color = 0;
							edit_icon_color = 0;
							LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
							Draw_User_Setting_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}
			}
	}
	else if(UI_state == STATE_USER_SETTING_BACKLIGHT){
			if(Top_Bar_Active)  // TopBar mode: Edit/Leave
			{
				if(key == 1 || key == 2)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key == 3)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter選項模式, Edit/Leave both black, Sensor selected
						Top_Bar_Active = 0;
						item_selection = 1;
						leave_icon_color = 0;
						edit_icon_color = 0;
						Draw_TopBar(leave_icon_color, edit_icon_color);
						Draw_User_Setting_Backlight_Content(item_selection);
					}
					else  // Leave Icon is red
					{
						// Go back to Function Setting Edit mode with cursor on Control Adj
						UI_state = STATE_USER_SETTING_MENU;
						Top_Bar_Active = 1;
						item_selection = 0;
						leave_icon_color = 1;
						edit_icon_color = 0;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_User_Setting_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					delay_ms(100);
				}
			}else if(item_selection == 1){
					if(key == 2){		//Down key
							if(sleep_backlight_duty > 0){
									sleep_backlight_duty -= 10;
									Draw_User_Setting_Backlight_Content(item_selection);
							}
					}else if(key == 1){		//Up key
							if(sleep_backlight_duty < 50){
									sleep_backlight_duty += 10;
									Draw_User_Setting_Backlight_Content(item_selection);
							}
					}else if(key == 3){		//Enter key
							item_selection = 2;
							Draw_User_Setting_Backlight_Content(item_selection);
					}
					
			}else if(item_selection == 2){
					if((key == 1) || (key == 2)){
							item_selection = 1;
							Draw_User_Setting_Backlight_Content(item_selection);
					}else if(key == 3){
							UI_state = STATE_USER_SETTING_MENU;
							item_selection = 4;
							Top_Bar_Active = 0;
							leave_icon_color = 0;
							edit_icon_color = 0;
							LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
							Draw_User_Setting_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}
			}
	}
	else if(UI_state == STATE_USER_SETTING_RESET)
	{
			if(Top_Bar_Active)  // TopBar mode: Edit/Leave
			{
				if(key == 1 || key == 2)  // Up or Down key pressed
				{
					Update_TopBar();
				}
				else if(key == 3)  // Enter key pressed
				{
					if(edit_icon_color)  // Edit Icon is red
					{
						// Enter選項模式, Edit/Leave both black, Sensor selected
						Top_Bar_Active = 0;
						item_selection = 1;
						leave_icon_color = 0;
						edit_icon_color = 0;
						Draw_TopBar(leave_icon_color, edit_icon_color);
						Draw_User_Setting_Reset_Contect(item_selection);
					}
					else  // Leave Icon is red
					{
						// Go back to Function Setting Edit mode with cursor on Control Adj
						UI_state = STATE_USER_SETTING_MENU;
						Top_Bar_Active = 1;
						item_selection = 0;
						leave_icon_color = 1;
						edit_icon_color = 0;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_User_Setting_Menu_Page(item_selection, leave_icon_color, edit_icon_color);
					}
					delay_ms(100);
				}
			}else if(item_selection == 1){
					if((key == 1) || (key == 2)){
							Top_Bar_Active = 1;
							item_selection = 0;
							leave_icon_color = 1;
							edit_icon_color = 0;
							Draw_User_Setting_Reset_Page(item_selection, leave_icon_color, edit_icon_color);
					}else if(key == 3){
							UI_state = STATE_ACTIVE;
							Top_Bar_Active = 0;
							item_selection = 0;
							leave_icon_color = 0;
							edit_icon_color = 0;
							LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
							Draw_Active_Menu();
					}
			}
	}
	delay_ms(10);  // Main loop delay (10ms)
	
	
}
