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
//File Function: Sub-UI Control Adjument Section Relative UI Flow
//		- 128 * 160 RGB(8 bit) LCM
//---------------------------------------------------------------------------------------------------------

#include "Thermostat.h"
#include "UI.h"


	
void Draw_User_Setting_Menu_Row(uint8_t row, uint8_t selected)
{
	uint16_t row_y[4];  // Y positions for display rows (max 4)
	uint16_t text_color, bg_color;
	uint8_t row_height;
	uint8_t text_idx = user_setting_menu_scroll + row;

	row_y[0] = 30;
	row_y[1] = 68;
	row_y[2] = 106;
	row_y[3] = 144;
	row_height = 32;

	// Bounds check
	if(text_idx >= User_Setting_Menu_Item_Count) return;
	if(row >= 4) return;

	if(selected) {
		text_color = WHITE;
		bg_color = RED;
		// Draw red background for selected item
		LCD_Fill(4, row_y[row], lcddev.width - 4, row_y[row] + row_height, RED);
	} else {
		text_color = BLACK;
		bg_color = WHITE;
		// Clear background to white
		LCD_Fill(4, row_y[row], lcddev.width - 4, row_y[row] + row_height, WHITE);
	}

	// Draw Arrow Icon (vertically centered in 32px row)
	if(selected) {
		GUI_DrawMonoIcon16x16(6, row_y[row] + (row_height - 16) / 2, WHITE, RED, Icon16x16_Arrow);
	} else {
		GUI_DrawMonoIcon16x16(6, row_y[row] + (row_height - 16) / 2, WHITE, BLACK, Icon16x16_Arrow);
	}

	// Draw menu text: single line if string fits, two-line auto split otherwise
	POINT_COLOR = text_color;
	BACK_COLOR = bg_color;
	lan_str = LanguageTable[User_Setting_Menu_Items[text_idx].text_id][g_parameter.language];
	{
			uint8_t len = 0;
			while(((char*)lan_str)[len] != 0) len++;
			if(len <= 12) {
					Show_Str(30, row_y[row] + (row_height - 16) / 2, text_color, bg_color, (char*)lan_str, 16, 0);
			} else {
					Show_FuncSetting_Row_Text(30, row_y[row], text_color, bg_color, (char*)lan_str, 16);
			}
	}
}

void Draw_User_Setting_Menu_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	uint8_t i;
	uint8_t num_visible_rows;
	uint8_t max_scroll = (User_Setting_Menu_Item_Count > 3) ? (User_Setting_Menu_Item_Count - 3) : 0;

	// Ensure scroll is valid
	if(user_setting_menu_scroll > max_scroll) user_setting_menu_scroll = max_scroll;

	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	Draw_TopBar(leave_col, edit_col);

	// Draw horizontal separator line below TopBar
	LCD_Fill(4, 24, lcddev.width - 4, 25, BLACK);

	// Calculate how many rows to draw (4 or less if near end)
	num_visible_rows = User_Setting_Menu_Item_Count - user_setting_menu_scroll;
	if(num_visible_rows > 4) num_visible_rows = 4;

	// Draw rows based on scroll position
	for(i = 0; i < num_visible_rows; i++) {
		uint8_t option_idx = user_setting_menu_scroll + i;
		uint8_t is_selected = 0;

		if(!Top_Bar_Active && (option_idx == selection - 1)) {
			is_selected = 1;
		}
		Draw_User_Setting_Menu_Row(i, is_selected);
	}
	LCD_Fill(0, 155, lcddev.width, lcddev.height, WHITE);
}

void Draw_User_Setting_Child_Lock_Choice(uint8_t selection)
{
	uint16_t row_y[2] = {42, 70};  // OFF, ON (moved up 8px)
	uint8_t row_height = 24;
	uint8_t i;
	uint8_t is_edit = (selection == 1) || (selection == 2);

	for(i = 0; i < 2; i++) {
		uint16_t text_color, bg_color, arrow_bg;
		uint8_t show_arrow = 1;

		if(is_edit && (i + 1) == selection) {
			// Edit mode, selected item: red bg, white text, white arrow
			text_color = WHITE;
			bg_color = RED;
			arrow_bg = RED;
		} else if(is_edit) {
			// Edit mode, unselected item: white bg, black text, black arrow
			text_color = BLACK;
			bg_color = WHITE;
			arrow_bg = BLACK;
		} else if(selection == 0) {
			// TopBar mode: current child_lock value = red arrow, other = black arrow
			text_color = BLACK;
			bg_color = WHITE;
			arrow_bg = (child_lock == i) ? RED : BLACK;
		} else {
			// Save mode: current child_lock value = red arrow, other = black arrow
			text_color = BLACK;
			bg_color = WHITE;
			arrow_bg = (child_lock == i) ? RED : BLACK;
		}

		LCD_Fill(0, row_y[i], lcddev.width, row_y[i] + row_height, bg_color);

		POINT_COLOR = text_color;
		BACK_COLOR = bg_color;
		if(i == 0) {
			lan_str = LanguageTable[STR_OFF][g_parameter.language];
			Show_Str(34, row_y[i] + (row_height - 16) / 2, text_color, bg_color, (char*)lan_str, 16, 0);
		} else {
			lan_str = LanguageTable[STR_ON][g_parameter.language];
			Show_Str(34, row_y[i] + (row_height - 16) / 2, text_color, bg_color, (char*)lan_str, 16, 0);
		}

		if(show_arrow) {
			GUI_DrawMonoIcon16x16(12, row_y[i] + (row_height - 16) / 2, WHITE, arrow_bg, Icon16x16_Arrow);
		} else {
			LCD_Fill(12, row_y[i] + (row_height - 16) / 2, 28, row_y[i] + (row_height - 16) / 2 + 16, WHITE);
		}
	}
}

void Draw_User_Setting_Child_Lock_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	uint8_t len = 0;
	uint16_t title_x;

	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	Draw_TopBar(leave_col, edit_col);

	// Title centered in Top Bar (16px font, ASCII half-width = 8px/char)
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	lan_str = LanguageTable[STR_Child_lock][g_parameter.language];
	while(((char*)lan_str)[len] != 0) len++;
	title_x = (lcddev.width - (uint16_t)len * 8) / 2;
	Show_Str(title_x, 5, BLACK, WHITE, (char*)lan_str, 16, 0);

	// Separator line right below the TopBar (TopBar ends at y=21)
	LCD_Fill(4, 22, lcddev.width - 4, 23, BLACK);

	Draw_User_Setting_Child_Lock_Choice(selection);

	// Save button at bottom (not moved)
	lan_str = LanguageTable[STR_Save][g_parameter.language];
	if(selection == 3){
		Show_Str(86, 142, RED, WHITE, (char*)lan_str, 16, 0);
	}else{
		Show_Str(86, 142, BLACK, WHITE, (char*)lan_str, 16, 0);
	}
}

void Draw_User_Setting_Window_Fun_Content(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	int color = (window_fun == 1) ? RED : BLACK;
	lan_str = LanguageTable[STR_Window][g_parameter.language];
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	Draw_TopBar(leave_col, edit_col);
	Show_Str(40, 5, BLACK, WHITE, (char*)lan_str, 16, 0);
	
	LCD_Fill(4, 22, lcddev.width - 4, 23, BLACK);

	// Centered switch box (x=39..89, 50px wide)
	LCD_Fill(39, 52, 89, 76, WHITE);
	LCD_Fill(39, 52, 89, 54, color);
	LCD_Fill(39, 52, 41, 76, color);
	LCD_Fill(39, 74, 89, 76, color);
	LCD_Fill(87, 52, 89, 76, color);

	if(window_fun == 0){
		lan_str = LanguageTable[STR_OFF][g_parameter.language];
		Show_Str(45, 56, BLACK, WHITE, (char*)lan_str, 16, 0);
		GUI_DrawMonoIcon16x16(69, 56, BLACK, WHITE, Block_Icon_16x16);
	}else{
		lan_str = LanguageTable[STR_ON][g_parameter.language];
		Show_Str(65, 56, RED, WHITE, (char*)lan_str, 16, 0);
		GUI_DrawMonoIcon16x16(45, 56, RED, WHITE, Block_Icon_16x16);
	}

	// Up/Down arrows on left/right of switch (edit mode: selection == 1)
	if(selection == 1) {
		GUI_DrawMonoIcon16x16(19, 56, WHITE, RED, Icon16x16_Down_Arror);
		GUI_DrawMonoIcon16x16(93, 56, WHITE, RED, Icon16x16_Up_Arror);
	} else {
		LCD_Fill(19, 56, 35, 80, WHITE);
		LCD_Fill(93, 56, 109, 80, WHITE);
	}

	// Trigger Temp / Time: RED when ON, BLACK when OFF
	//uint16_t trigger_color = (window_fun == 1) ? RED : BLACK;
	if(window_fun == 1){
			uint16_t trigger_color = (selection == 2) ? RED : BLACK;
			lan_str = LanguageTable[STR_Trigger_Temp][g_parameter.language];
			Show_Str(10, 92, trigger_color, WHITE, (char*)lan_str, 16, 0);
			lan_str = LanguageTable[STR_And_Time][g_parameter.language];
			Show_Str(10, 108, trigger_color, WHITE, (char*)lan_str, 16, 0);
	}
}

void Draw_User_Setting_Window_Fun_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	Draw_User_Setting_Window_Fun_Content(selection, leave_col, edit_col);
}

void Draw_User_Setting_Window_Time_Content(uint8_t selection)
{
	LCD_Fill(0, 36, lcddev.width, 122, WHITE);

	// Temp / Time labels (selected = RED, unselected = BLACK)
	lan_str = LanguageTable[STR_Temperature][g_parameter.language];
	Show_Str(10, 36, (selection == 1) ? RED : BLACK, WHITE, (char*)lan_str, 16, 0);
	lan_str = LanguageTable[STR_Time][g_parameter.language];
	Show_Str(80, 36, (selection == 2) ? RED : BLACK, WHITE, (char*)lan_str, 16, 0);

	// In TopBar mode (selection == 0), show Temp value (first to edit)
	// In other modes, show the value of the selected item
	int int_val;
	if(selection == 0 || selection == 1 || selection == 3) {
		int_val = window_fun_temp;
	} else {
		int_val = window_fun_time;
	}
	// 32x64 font
	if(int_val >= 10) {
		GUI_DrawBigDigit(32, 60, BLACK, WHITE, '0' + (int_val/10), 0);
		GUI_DrawBigDigit(64, 60, BLACK, WHITE, '0' + (int_val%10), 0);
	} else {
		GUI_DrawBigDigit(48, 60, BLACK, WHITE, '0' + int_val, 0);
	}

	// Up/Down arrows (edit mode: selection == 1 or 2)
	if(selection == 1 || selection == 2) {
		GUI_DrawMonoIcon16x16(12, 86, WHITE, RED, Icon16x16_Down_Arror);
		GUI_DrawMonoIcon16x16(100, 86, WHITE, RED, Icon16x16_Up_Arror);
	} else {
		LCD_Fill(12, 86, 28, 102, WHITE);
		LCD_Fill(100, 86, 116, 102, WHITE);
	}

	// Save button
	lan_str = LanguageTable[STR_Save][g_parameter.language];
	if(selection == 3) {
		Show_Str(86, 142, RED, WHITE, (char*)lan_str, 16, 0);
	} else {
		Show_Str(86, 142, BLACK, WHITE, (char*)lan_str, 16, 0);
	}
}

void Draw_User_Setting_Window_Time_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	Draw_TopBar(leave_col, edit_col);

	// Title: Window Func
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	lan_str = LanguageTable[STR_Window][g_parameter.language];
	Show_Str(40, 5, BLACK, WHITE, (char*)lan_str, 16, 0);
	LCD_Fill(4, 22, lcddev.width - 4, 23, BLACK);

	Draw_User_Setting_Window_Time_Content(selection);
}

void Draw_User_Setting_SetTime_Content(uint8_t selection)
{
	uint8_t i;
	uint16_t row_y[3] = {36, 70, 104};  // Y, M, D (32px spacing)
	uint8_t row_height = 30;
	uint8_t vals[3];

	vals[0] = temp_rtc.Year;
	vals[1] = temp_rtc.Mon;
	vals[2] = temp_rtc.Date;

	LCD_Fill(0, 32, lcddev.width, 136, WHITE);

	for(i = 0; i < 3; i++) {
		uint8_t is_sel = (i + 1 == selection);
		uint8_t d1 = vals[i] / 10;
		uint8_t d2 = vals[i] % 10;

		// Label Y / M / D (selected = RED)
		char label[2] = {0};
			label[0] = (i == 0) ? 'Y' : (i == 1) ? 'M' : 'D';
		POINT_COLOR = (is_sel) ? RED : BLACK;
		BACK_COLOR = WHITE;
		Show_Str(8, row_y[i] + (row_height - 16) / 2, (is_sel) ? RED : BLACK, WHITE, label, 16, 0);

		// Down arrow (left of digits)
		if(is_sel) {
			GUI_DrawMonoIcon16x16(26, row_y[i] + (row_height - 16) / 2, WHITE, RED, Icon16x16_Down_Arror);
		} else {
			LCD_Fill(26, row_y[i] + (row_height - 16) / 2, 42, row_y[i] + (row_height - 16) / 2 + 16, WHITE);
		}

		// Digits (16x32 font)
		GUI_DrawBigDigit(48, row_y[i], BLACK, WHITE, '0' + d1, 1);
		GUI_DrawBigDigit(64, row_y[i], BLACK, WHITE, '0' + d2, 1);

		// Up arrow (right of digits)
		if(is_sel) {
			GUI_DrawMonoIcon16x16(88, row_y[i] + (row_height - 16) / 2, WHITE, RED, Icon16x16_Up_Arror);
		} else {
			LCD_Fill(88, row_y[i] + (row_height - 16) / 2, 104, row_y[i] + (row_height - 16) / 2 + 16, WHITE);
		}
	}

	// Next button (enter Set Clock sub-page)
	lan_str = LanguageTable[STR_Next][g_parameter.language];
	if(selection == 4) {
		Show_Str(86, 142, RED, WHITE, (char*)lan_str, 16, 0);
	} else {
		Show_Str(86, 142, BLACK, WHITE, (char*)lan_str, 16, 0);
	}
}

void Draw_User_Setting_SetTime_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	Draw_TopBar(leave_col, edit_col);

	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	lan_str = LanguageTable[STR_Set_Date][g_parameter.language];
	Show_Str(32, 5, BLACK, WHITE, (char*)lan_str, 16, 0);
	LCD_Fill(4, 22, lcddev.width - 4, 23, BLACK);

	Draw_User_Setting_SetTime_Content(selection);
}

void Draw_User_Setting_Setclk_Content(uint8_t selection)
{
	LCD_Fill(0, 36, lcddev.width, 130, WHITE);

	uint8_t h1 = temp_rtc.Hour / 10;
	uint8_t h2 = temp_rtc.Hour % 10;
	uint8_t m1 = temp_rtc.Min / 10;
	uint8_t m2 = temp_rtc.Min % 10;

	uint16_t hour_color = (selection == 1) ? RED : BLACK;
	uint16_t min_color = (selection == 2) ? RED : BLACK;

	// Hour label (y=46)
	POINT_COLOR = hour_color;
	BACK_COLOR = WHITE;
	Show_Str(22, 30, hour_color, WHITE, (char*)LanguageTable[STR_Hour][g_parameter.language], 16, 0);

	// Hour arrows (y=58, centered between digits x=32)
	if(selection == 1) {
		GUI_DrawMonoIcon16x16(32, 52, WHITE, RED, Icon16x16_Up_Arror);
		GUI_DrawMonoIcon16x16(32, 106, WHITE, RED, Icon16x16_Down_Arror);
	} else {
		LCD_Fill(32, 52, 48, 68, WHITE);
		LCD_Fill(32, 106, 48, 122, WHITE);
	}

	// Hour digits (y=78)
	GUI_DrawBigDigit(24, 70, hour_color, WHITE, '0' + h1, 1);
	GUI_DrawBigDigit(40, 70, hour_color, WHITE, '0' + h2, 1);

	// Colon (moved right 4px: x=65)
	LCD_Fill(65, 85, 67, 87, BLACK);
	LCD_Fill(65, 91, 67, 93, BLACK);

	// Min label (y=46)
	POINT_COLOR = min_color;
	Show_Str(78, 30, min_color, WHITE, (char*)LanguageTable[STR_Min][g_parameter.language], 16, 0);

	// Min arrows (y=58, centered between digits x=84)
	if(selection == 2) {
		GUI_DrawMonoIcon16x16(84, 52, WHITE, RED, Icon16x16_Up_Arror);
		GUI_DrawMonoIcon16x16(84, 106, WHITE, RED, Icon16x16_Down_Arror);
	} else {
		LCD_Fill(84, 52, 100, 68, WHITE);
		LCD_Fill(84, 106, 100, 122, WHITE);
	}

	// Min digits (y=78)
	GUI_DrawBigDigit(76, 70, min_color, WHITE, '0' + m1, 1);
	GUI_DrawBigDigit(92, 70, min_color, WHITE, '0' + m2, 1);

	// Save button
	lan_str = LanguageTable[STR_Save][g_parameter.language];
	if(selection == 3) {
		Show_Str(86, 142, RED, WHITE, (char*)lan_str, 16, 0);
	} else {
		Show_Str(86, 142, BLACK, WHITE, (char*)lan_str, 16, 0);
	}
}

void Draw_User_Setting_Setclk_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	Draw_TopBar(leave_col, edit_col);

	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	lan_str = LanguageTable[STR_Set_Time][g_parameter.language];
	Show_Str(32, 5, BLACK, WHITE, (char*)lan_str, 16, 0);
	LCD_Fill(4, 22, lcddev.width - 4, 23, BLACK);

	Draw_User_Setting_Setclk_Content(selection);
}

void Draw_User_Setting_Backlight_Content(uint8_t selection)
{
	uint8_t ten_digi, one_digi;
	ten_digi = sleep_backlight_duty / 10;
	one_digi = sleep_backlight_duty % 10;

	LCD_Fill(0, 36, lcddev.width, 130, WHITE);

	// Value (32x64 font, centered)
	GUI_DrawBigDigit(32, 63, BLACK, WHITE, '0' + ten_digi, 0);
	GUI_DrawBigDigit(64, 63, BLACK, WHITE, '0' + one_digi, 0);
	GUI_DrawMonoIcon16x16(98, 58, BLACK, WHITE, Percent_Icon_16x16);

	// Up/Down arrows (selection == 1: edit mode)
	if(selection == 1) {
		GUI_DrawMonoIcon16x16(16, 83, WHITE, RED, Icon16x16_Down_Arror);
		GUI_DrawMonoIcon16x16(100, 83, WHITE, RED, Icon16x16_Up_Arror);
	} else {
		LCD_Fill(16, 83, 32, 99, WHITE);
		LCD_Fill(100, 83, 116, 99, WHITE);
	}

	// Save button
	lan_str = LanguageTable[STR_Save][g_parameter.language];
	if(selection == 2) {
		Show_Str(86, 142, RED, WHITE, (char*)lan_str, 16, 0);
	} else {
		Show_Str(86, 142, BLACK, WHITE, (char*)lan_str, 16, 0);
	}
}
void Draw_User_Setting_Backlight_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	Draw_TopBar(leave_col, edit_col);

	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	lan_str = LanguageTable[STR_Backlight][g_parameter.language];
	Show_Str(28, 5, BLACK, WHITE, (char*)lan_str, 16, 0);
	LCD_Fill(4, 22, lcddev.width - 4, 23, BLACK);

	Draw_User_Setting_Backlight_Content(selection);
}

void Draw_User_Setting_Reset_Contect(uint8_t selection)
{
	// Reset button (red when selected)
	lan_str = LanguageTable[STR_Reset][g_parameter.language];
	if(selection == 1) {
		Show_Str(86, 142, RED, WHITE, (char*)lan_str, 16, 0);
	} else {
		Show_Str(86, 142, BLACK, WHITE, (char*)lan_str, 16, 0);
	}
}
void Draw_User_Setting_Reset_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	Draw_TopBar(leave_col, edit_col);

	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	lan_str = LanguageTable[STR_Reset][g_parameter.language];
	Show_Str(44, 5, BLACK, WHITE, (char*)lan_str, 16, 0);
	LCD_Fill(4, 22, lcddev.width - 4, 23, BLACK);

	// "Reset to" / "Default?" (two lines, RED)
	//Show_Str(10, 50, RED, WHITE, "Reset to", 16, 0);
	//Show_Str(10, 66, RED, WHITE, "Default?", 16, 0);
	lan_str = LanguageTable[STR_Reset_to][g_parameter.language];
  Show_Str(10, 34, RED, WHITE, (char*)lan_str, 16, 0);
	lan_str = LanguageTable[STR_Default][g_parameter.language];
	Show_Str(10, 50, RED, WHITE, (char*)lan_str, 16, 0);
	// "This will" / "erase all" / "saved setting" (three lines, BLACK)
	//Show_Str(10, 85, BLACK, WHITE, "This will", 16, 0);
	lan_str = LanguageTable[STR_This_will][g_parameter.language];
	Show_Str(10, 77, BLACK, WHITE, (char*)lan_str, 16, 0);
	//Show_Str(10, 101, BLACK, WHITE, "erase all", 16, 0);
	lan_str = LanguageTable[STR_erase_all][g_parameter.language];
	Show_Str(10, 93, BLACK, WHITE, (char*)lan_str, 16, 0);
	//Show_Str(10, 117, BLACK, WHITE, "saved setting", 16, 0);
	lan_str = LanguageTable[STR_saved_settings][g_parameter.language];
	Show_Str(10, 109, BLACK, WHITE, (char*)lan_str, 16, 0);

	Draw_User_Setting_Reset_Contect(selection);
}



void Draw_Pin_Input_Box(uint8_t* pin_buffer, uint8_t digit_index)
{
	uint8_t i;
	uint16_t box_x_start = 1;
	uint16_t box_y = 50;
	uint16_t box_width = 28;
	uint16_t box_height = 40;
	uint16_t box_spacing = 5;

	for(i = 0; i < 4; i++){
		uint16_t x = box_x_start + i * (box_width + box_spacing);

		if(i == digit_index){
			LCD_Fill(x, box_y, x + box_width, box_y + box_height, WHITE);
			LCD_Fill(x, box_y, x + box_width, box_y + 2, RED);
			LCD_Fill(x, box_y + box_height - 2, x + box_width, box_y + box_height, RED);
			LCD_Fill(x, box_y, x + 2, box_y + box_height, RED);
			LCD_Fill(x + box_width - 2, box_y, x + box_width, box_y + box_height, RED);
		}else{
			LCD_Fill(x, box_y, x + box_width, box_y + box_height, WHITE);
			LCD_Fill(x, box_y, x + box_width, box_y + 2, BLACK);
			LCD_Fill(x, box_y + box_height - 2, x + box_width, box_y + box_height, BLACK);
			LCD_Fill(x, box_y, x + 2, box_y + box_height, BLACK);
			LCD_Fill(x + box_width - 2, box_y, x + box_width, box_y + box_height, BLACK);
		}

		if(pin_buffer[i] != 0xFF){
			GUI_DrawBigDigit(x + 6, box_y + 4, BLACK, WHITE, '0' + pin_buffer[i], 1);
		}
	}
}

void Draw_Pin_Digit_Selector(uint8_t selected_digit)
{
	uint8_t i;
	uint16_t digit_y[2] = {105, 125};  // Row 0: 0-4, Row 1: 5-9
	uint8_t digits_per_row = 5;
	uint16_t start_x = 8;
	uint16_t digit_spacing = 24;
	char digit_str[2] = {0, 0};

	for(i = 0; i < 10; i++){
		uint8_t row = i / digits_per_row;
		uint8_t col = i % digits_per_row;
		uint16_t x = start_x + col * digit_spacing;
		uint16_t y = digit_y[row];
		digit_str[0] = '0' + i;

		if(i == selected_digit){
			LCD_Fill(x, y, x + 20, y + 18, RED);
			Show_Str(x + 4, y + 1, WHITE, RED, digit_str, 16, 0);
		}else{
			LCD_Fill(x, y, x + 20, y + 18, WHITE);
			Show_Str(x + 4, y + 1, BLACK, WHITE, digit_str, 16, 0);
		}
	}
}

void Draw_Child_Lock_Pin_Page(uint8_t stage)
{
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);

	// Title
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	if(stage == 0){
		lan_str = LanguageTable[STR_Setup_Pin][g_parameter.language];
		Show_Str(10, 10, BLACK, WHITE, (char*)lan_str, 16, 0);
	}else{
		lan_str = LanguageTable[STR_Confirm_Pin][g_parameter.language];
		Show_Str(10, 10, BLACK, WHITE, (char*)lan_str, 16, 0);
	}

	// Draw PIN input boxes
	if(stage == 0){
		Draw_Pin_Input_Box(pin_code_input, pin_digit_index);
	}else{
		Draw_Pin_Input_Box(pin_code_confirm, pin_digit_index);
	}

	// Draw digit selector (0-9, two rows)
	Draw_Pin_Digit_Selector(pin_digit_selected);
}



uint8_t Verify_Pin_Code(void)
{
	uint8_t i;
	for(i = 0; i < 4; i++){
		if(pin_code_input[i] != g_parameter.pin_code[i]){
			return 0;  // PIN mismatch
		}
	}
	return 1;  // PIN match
}

void Draw_User_Setting_Language_Choice(uint8_t selection)
{
	uint16_t row_y[] = {44, 70};  // Y positions for English, Norwegian (moved up, consistent spacing)
	uint8_t i;
	
	for(i = 0; i < 2; i++) {
		// Draw text (always black on white background)
		LCD_Fill(0, row_y[i] - 2, lcddev.width, row_y[i] + 24, WHITE);
		
		if((selection == 0) || (selection == 3)){
				if(i == 0) {
					lan_str = LanguageTable[STR_English][g_parameter.language];
					Show_Str(34, row_y[i] + 4, BLACK, WHITE, (char*)lan_str, 16, 0);
					if(language_temp == 0){
							GUI_DrawMonoIcon16x16(12, row_y[i] + 4, WHITE, RED, Icon16x16_Arrow);
					}else{
							GUI_DrawMonoIcon16x16(12, row_y[i] + 4, WHITE, BLACK, Icon16x16_Arrow);
					}
				} else {
					lan_str = LanguageTable[STR_Norwegian][g_parameter.language];
					Show_Str(34, row_y[i] + 4, BLACK, WHITE, (char*)lan_str, 16, 0);
					if(language_temp == 1){
							GUI_DrawMonoIcon16x16(12, row_y[i] + 4, WHITE, RED, Icon16x16_Arrow);
					}else{
							GUI_DrawMonoIcon16x16(12, row_y[i] + 4, WHITE, BLACK, Icon16x16_Arrow);
					}
				}
		}else{
				if(i == 0){
						lan_str = LanguageTable[STR_English][g_parameter.language];
						if(selection == 1){
								LCD_Fill(4, row_y[i] - 2, lcddev.width-4, row_y[i]+24, RED);
								Show_Str(34, row_y[i] + 4, WHITE, RED, (char*)lan_str, 16, 0);
								GUI_DrawMonoIcon16x16(12, row_y[i] + 4, WHITE, RED, Icon16x16_Arrow);
						}else{
								Show_Str(34, row_y[i] + 4, BLACK, WHITE, (char*)lan_str, 16, 0);
								GUI_DrawMonoIcon16x16(12, row_y[i] + 4, WHITE, BLACK, Icon16x16_Arrow);
						}
				}else{
						lan_str = LanguageTable[STR_Norwegian][g_parameter.language];
						if(selection == 2){
								LCD_Fill(4, row_y[i] - 2, lcddev.width-4, row_y[i]+24, RED);
								Show_Str(34, row_y[i] + 4, WHITE, RED, (char*)lan_str, 16, 0);
								GUI_DrawMonoIcon16x16(12, row_y[i] + 4, WHITE, RED, Icon16x16_Arrow);
						}else{
								Show_Str(34, row_y[i] + 4, BLACK, WHITE, (char*)lan_str, 16, 0);
								GUI_DrawMonoIcon16x16(12, row_y[i] + 4, WHITE, BLACK, Icon16x16_Arrow);
						}
				}
		}
	}
}

void Draw_User_Setting_Language_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	Draw_TopBar(leave_col, edit_col);

	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	lan_str = LanguageTable[STR_Language][g_parameter.language];
	Show_Str(32, 5, BLACK, WHITE, (char*)lan_str, 16, 0);
	LCD_Fill(4, 22, lcddev.width - 4, 23, BLACK);

	Draw_User_Setting_Language_Choice(selection);

	lan_str = LanguageTable[STR_Save][g_parameter.language];
	if(selection == 3){
		Show_Str(86, 142, RED, WHITE, (char*)lan_str, 16, 0);
	}else{
		Show_Str(86, 142, BLACK, WHITE, (char*)lan_str, 16, 0);
	}
}

void Draw_User_Setting_Adaptive_Start_Choice(uint8_t selection)
{
	uint16_t row_y[] = {44, 70};
	uint8_t i;

	for(i = 0; i < 2; i++) {
		// Draw text (always black on white background)
		LCD_Fill(0, row_y[i] - 2, lcddev.width, row_y[i] + 24, WHITE);
		
		if((selection == 0) || (selection == 3)){
				if(i == 0) {
					lan_str = LanguageTable[STR_OFF][g_parameter.language];
					Show_Str(34, row_y[i] + 4, BLACK, WHITE, (char*)lan_str, 16, 0);
					if(adaptive_start == 0){
							GUI_DrawMonoIcon16x16(12, row_y[i] + 4, WHITE, RED, Icon16x16_Arrow);
					}else{
							GUI_DrawMonoIcon16x16(12, row_y[i] + 4, WHITE, BLACK, Icon16x16_Arrow);
					}
				} else {
					lan_str = LanguageTable[STR_ON][g_parameter.language];
					Show_Str(34, row_y[i] + 4, BLACK, WHITE, (char*)lan_str, 16, 0);
					if(adaptive_start == 1){
							GUI_DrawMonoIcon16x16(12, row_y[i] + 4, WHITE, RED, Icon16x16_Arrow);
					}else{
							GUI_DrawMonoIcon16x16(12, row_y[i] + 4, WHITE, BLACK, Icon16x16_Arrow);
					}
				}
		}else{
				if(i == 0){
						lan_str = LanguageTable[STR_OFF][g_parameter.language];
						if(selection == 1){
								LCD_Fill(4, row_y[i] - 2, lcddev.width-4, row_y[i]+24, RED);
								Show_Str(34, row_y[i] + 4, WHITE, RED, (char*)lan_str, 16, 0);
								GUI_DrawMonoIcon16x16(12, row_y[i] + 4, WHITE, RED, Icon16x16_Arrow);
						}else{
								Show_Str(34, row_y[i] + 4, BLACK, WHITE, (char*)lan_str, 16, 0);
								GUI_DrawMonoIcon16x16(12, row_y[i] + 4, WHITE, BLACK, Icon16x16_Arrow);
						}
				}else{
						lan_str = LanguageTable[STR_ON][g_parameter.language];
						if(selection == 2){
								LCD_Fill(4, row_y[i] - 2, lcddev.width-4, row_y[i]+24, RED);
								Show_Str(34, row_y[i] + 4, WHITE, RED, (char*)lan_str, 16, 0);
								GUI_DrawMonoIcon16x16(12, row_y[i] + 4, WHITE, RED, Icon16x16_Arrow);
						}else{
								Show_Str(34, row_y[i] + 4, BLACK, WHITE, (char*)lan_str, 16, 0);
								GUI_DrawMonoIcon16x16(12, row_y[i] + 4, WHITE, BLACK, Icon16x16_Arrow);
						}
				}
		}
	}
}

void Draw_User_Setting_Adaptive_Start_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	Draw_TopBar(leave_col, edit_col);

	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	lan_str = LanguageTable[STR_Adaptive][g_parameter.language];
	Show_Str(32, 5, BLACK, WHITE, (char*)lan_str, 16, 0);
	LCD_Fill(4, 22, lcddev.width - 4, 23, BLACK);

	Draw_User_Setting_Adaptive_Start_Choice(selection);

	lan_str = LanguageTable[STR_Save][g_parameter.language];
	if(selection == 3){
		Show_Str(86, 142, RED, WHITE, (char*)lan_str, 16, 0);
	}else{
		Show_Str(86, 142, BLACK, WHITE, (char*)lan_str, 16, 0);
	}
}
