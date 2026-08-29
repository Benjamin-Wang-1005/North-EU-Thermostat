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


// Display number on LCD
// number: value to display
// color: text color
// show_decimal: 1 = show decimal point and fraction, 0 = show integer only
void Display_Adj_Number(float number, uint16_t color, uint8_t show_decimal)
{
	int integer_part;
	int decimal_part;
	uint8_t digit1, digit2;
	uint8_t has_tens = 0;
	uint8_t is_negative = 0;
	uint8_t y_pot;
	uint8_t x_pot;
	
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
	//clear digi area
	LCD_Fill(5, 70, 95, 106, WHITE);
	// Draw minus sign if negative
	if(is_negative)
	{
		// Draw minus sign - position depends on number of digits
		if(has_tens)
		{
			if((UI_state == STATE_CONTROL_ADJ_TEMP_PROTECT_MAX) || (UI_state == STATE_CONTROL_ADJ_TEMP_PROTECT_MIN)){
			// Two digits: minus sign at far left (x=5)
					LCD_Fill(5, 86, 20, 90, color);
			}else{
					LCD_Fill(5, 90, 21, 94, color);
			}
		}
		else
		{
			if((UI_state == STATE_CONTROL_ADJ_TEMP_PROTECT_MAX) || (UI_state == STATE_CONTROL_ADJ_TEMP_PROTECT_MIN)){
					LCD_Fill(20, 86, 36, 90, color);
			}else{
					// Single digit: minus sign closer to the digit (x=35)
					LCD_Fill(47, 90, 63, 94, color);
			}
		}
	}
	else
	{
		// Clear minus sign area (for when going from negative to positive)
		LCD_Fill(5, 90, 36, 94, WHITE);
	}
	
	// Draw digits
	if((UI_state == STATE_CONTROL_ADJ_TEMP_PROTECT_MAX) || (UI_state == STATE_CONTROL_ADJ_TEMP_PROTECT_MIN)){
			y_pot = 70;
			x_pot = 48;
	}else{
			y_pot = 74;
			x_pot = 72;
	}
	if(has_tens)
	{
		// Two digits - clear both digit areas first, then draw
		// This ensures no residual pixels from previous display
		LCD_Fill(x_pot - 16, y_pot, 72, y_pot + 32, WHITE);   // Clear tens digit area
		LCD_Fill(x_pot, y_pot, 88, y_pot +32, WHITE);   // Clear ones digit area
		// "1" at x=22, "4" at x=59
		GUI_DrawBigDigit(x_pot - 16, y_pot, color, WHITE, '0' + digit1, 1);  // 32x64 font - tens
		GUI_DrawBigDigit(x_pot, y_pot, color, WHITE, '0' + digit2, 1);  // 32x64 font - ones
	}
	else
	{
		// Single digit - draw at x=59
		// Note: Clear_Number_Area() already cleared everything before this function is called
		GUI_DrawBigDigit(x_pot, y_pot, color, WHITE, '0' + digit2, 1);  // 32x64 font (with bg)
	}
	
		// Draw decimal point and fraction if needed
	if(show_decimal)
	{
		// Draw decimal point (circular dot) at fixed position
		LCD_Fill(92, 101, 96, 105, color);
		LCD_Fill(91, 102, 97, 104, color);
		LCD_Fill(92, 106, 96, 106, color);
		LCD_Fill(92, 100, 96, 100, color);
		
		// Draw decimal digit using smaller font (16x32)
		GUI_DrawBigDigit(101, 74, color, WHITE, '0' + decimal_part, 1);
	}
}

// Draw a single row in Control Adj Menu page (portrait, red bg when selected)
// row: 0-3 (display row position on screen)
// selected: 1=selected (red bg + white text), 0=not selected
void Draw_Control_Adj_Menu_Row(uint8_t row, uint8_t selected)
{
	uint16_t row_y[4];  // Y positions for display rows (max 4)
	uint16_t text_color, bg_color;
	uint8_t row_height;
	uint8_t visible_rows;
	uint8_t text_idx = control_adj_menu_scroll + row;

	row_y[0] = 30;
	row_y[1] = 68;
	row_y[2] = 106;
	row_y[3] = 144;
	row_height = 32;
	visible_rows = 4;

	if(text_idx >= Control_Adj_Menu_Item_Count) return;
	if(row >= visible_rows) return;

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

	// Draw Arrow Icon (white on black when unselected, white on red when selected)
	if(selected) {
		GUI_DrawMonoIcon16x16(6, row_y[row] + (row_height - 16) / 2, WHITE, RED, Icon16x16_Arrow);
	} else {
		GUI_DrawMonoIcon16x16(6, row_y[row] + (row_height - 16) / 2, WHITE, BLACK, Icon16x16_Arrow);
	}

	// Draw menu text (two-line auto for long strings)
	POINT_COLOR = text_color;
	BACK_COLOR = bg_color;
	lan_str = LanguageTable[Control_Adj_Menu_Items[text_idx].text_id][g_parameter.language];
	Show_FuncSetting_Row_Text(30, row_y[row], text_color, bg_color, (char*)lan_str, 16);
}


// Draw Control Adj Menu Page (portrait mode, 32px rows)
// selection: 0-7=options, 0=TopBar mode
void Draw_Control_Adj_Menu_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	uint8_t i;
	uint8_t num_visible_rows;
	uint8_t visible_rows;
	uint8_t max_scroll;

	visible_rows = 4;
	max_scroll = (Control_Adj_Menu_Item_Count > 3) ? (Control_Adj_Menu_Item_Count - 3) : 0;

	// Ensure scroll is valid
	if(control_adj_menu_scroll > max_scroll) control_adj_menu_scroll = max_scroll;

	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	Draw_TopBar(leave_col, edit_col);

	// Draw horizontal separator line below TopBar
	LCD_Fill(4, 24, lcddev.width - 4, 25, BLACK);

	// Calculate how many rows to draw (visible_rows or less if near end)
	num_visible_rows = Control_Adj_Menu_Item_Count - control_adj_menu_scroll;
	if(num_visible_rows > visible_rows) num_visible_rows = visible_rows;

	// Draw rows based on scroll position
	for(i = 0; i < num_visible_rows; i++) {
		uint8_t option_idx = control_adj_menu_scroll + i;
		uint8_t is_selected = 0;

		if(!Top_Bar_Active && (option_idx + 1 == selection)) {
			is_selected = 1;
		}
		Draw_Control_Adj_Menu_Row(i, is_selected);
	}
	LCD_Fill(0, 155, lcddev.width, lcddev.height, WHITE);
}


// Draw Floor/Room choices for Sensor Setting page (portrait)
// selection: 0=TopBar, 1=Edit, 2=Save; current_sensor_type: 0=Room, 1=Floor
void Draw_Control_Adj_Sensor_Choices(uint8_t selection)
{
	uint16_t row_y[2] = {36, 66};  // Room=36, Floor=66 (8px extra gap below divider)
	uint8_t row_height = 24;
	uint8_t i;

	uint8_t is_edit = (selection == 1);

	for(i = 0; i < 2; i++) {
		uint16_t text_color, bg_color;
		uint8_t is_selected = (current_sensor_type == i);

		if(is_edit && is_selected) {
			// Edit mode, selected item: red bg, white text
			text_color = WHITE;
			bg_color = RED;
			LCD_Fill(4, row_y[i], lcddev.width - 4, row_y[i] + row_height, RED);
		} else {
			// TopBar/Save mode or unselected: white bg, black text
			text_color = BLACK;
			bg_color = WHITE;
			LCD_Fill(4, row_y[i], lcddev.width - 4, row_y[i] + row_height, WHITE);
		}

		// Arrow icon
		uint8_t arrow_y = row_y[i] + (row_height - 16) / 2;
		if(is_selected) {
			if(is_edit) {
				GUI_DrawMonoIcon16x16(12, arrow_y, WHITE, RED, Icon16x16_Arrow);      // red bg, white line
			} else {
				GUI_DrawMonoIcon16x16(12, arrow_y, WHITE, RED, Icon16x16_Arrow);      // red bg, white line (current param)
			}
		} else {
			GUI_DrawMonoIcon16x16(12, arrow_y, WHITE, BLACK, Icon16x16_Arrow);        // black bg, white line
		}

		// Text
		POINT_COLOR = text_color;
		BACK_COLOR = bg_color;
		if(i == 0) {
			lan_str = LanguageTable[STR_Room][g_parameter.language];
		} else {
			lan_str = LanguageTable[STR_Floor][g_parameter.language];
		}
		Show_Str(34, row_y[i] + (row_height - 16) / 2, text_color, bg_color, (char*)lan_str, 16, 0);
	}

	// Draw Save button at bottom
	lan_str = LanguageTable[STR_Save][g_parameter.language];
	if(selection == 2){
		Show_Str(86, 135, RED, WHITE, (char*)lan_str, 16, 0);
	}else{
		Show_Str(86, 135, BLACK, WHITE, (char*)lan_str, 16, 0);
	}
}



// Draw Control Adj Sensor Setting Page (portrait)
void Draw_Control_Adj_Sensor_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	uint16_t title_x;
	uint8_t title_width;

	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	Draw_TopBar(leave_col, edit_col);

	// Draw "Sensor" title centered in the TopBar (font size 16, ASCII half-width = 8px)
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	lan_str = LanguageTable[STR_Sensor][g_parameter.language];
	title_width = strlen((char*)lan_str) * 8;
	title_x = (lcddev.width - title_width) / 2;
	Show_Str(title_x, 5, BLACK, WHITE, (char*)lan_str, 16, 0);

	// Draw horizontal separator line right below the TopBar (TopBar ends at y=21)
	LCD_Fill(4, 22, lcddev.width - 4, 23, BLACK);

	Draw_Control_Adj_Sensor_Choices(selection);
}


void Draw_Control_Adj_TempCorrect_Contect(uint8_t selection)
{
	float disp_val;
	uint8_t is_neg;
	int int_part;
	uint8_t dec_part, d1, d2;

	// Room (left) / Floor (right) - colored when selected
	lan_str = LanguageTable[STR_Room][g_parameter.language];
	Show_Str(10, 40, (selection == 1) ? RED : BLACK, WHITE, (char*)lan_str, 16, 0);
	lan_str = LanguageTable[STR_Floor][g_parameter.language];
	Show_Str(80, 40, (selection == 2) ? RED : BLACK, WHITE, (char*)lan_str, 16, 0);

	// In edit mode (1 or 2): draw number, clear area first
	// In save mode (3): keep number displayed, just update Save button
	if(selection == 1 || selection == 2) {
		LCD_Fill(5, 36, lcddev.width, 133, WHITE);
		// Redraw labels (cleared by LCD_Fill)
		lan_str = LanguageTable[STR_Room][g_parameter.language];
		Show_Str(10, 40, (selection == 1) ? RED : BLACK, WHITE, (char*)lan_str, 16, 0);
		lan_str = LanguageTable[STR_Floor][g_parameter.language];
		Show_Str(80, 40, (selection == 2) ? RED : BLACK, WHITE, (char*)lan_str, 16, 0);

		disp_val = (selection == 1) ? temp_correct_internal : temp_correct_external;
		is_neg = (disp_val < 0) ? 1 : 0;
		int_part = (int)(is_neg ? -disp_val : disp_val);
		dec_part = (int)((disp_val >= 0 ? disp_val : -disp_val) * 10) % 10;
		d1 = int_part / 10;
		d2 = int_part % 10;
		// Minus sign (keep at original position, not shifted left)
		if(is_neg) LCD_Fill(12, 92, 27, 96, BLACK);
		// Integer digits (32x64 font, y=60, shifted left 12px)
		if(int_part >= 10) {
			GUI_DrawBigDigit(12, 60, BLACK, WHITE, '0' + d1, 0);
			GUI_DrawBigDigit(44, 60, BLACK, WHITE, '0' + d2, 0);
		} else {
			GUI_DrawBigDigit(36, 60, BLACK, WHITE, '0' + d2, 0);
		}
		// Decimal point (right after ones digit)
		LCD_Fill(70, 116, 74, 120, BLACK);
		LCD_Fill(69, 117, 75, 119, BLACK);
		// Decimal digit (32x64 font, shifted left 12px)
		GUI_DrawBigDigit(78, 60, BLACK, WHITE, '0' + dec_part, 0);
	}

	// Save button (keep original position)
	lan_str = LanguageTable[STR_Save][g_parameter.language];
	if(selection == 3) {
		Show_Str(86, 140, RED, WHITE, (char*)lan_str, 16, 0);
	} else {
		Show_Str(86, 140, BLACK, WHITE, (char*)lan_str, 16, 0);
	}
}
// Draw Control Adj Temp. Correct Setting Page
// This page shows:
// - Top-left: Leave Icon (black/red)
// - Top-right: Edit Icon (black/red)
// - "Temp. Correct" title below Top Bar
// - Horizontal line below title (with 4px margin on both sides)
// - Internal/External options with value display
// selection: 0=Internal, 1=External, 2=Save, 0xFF=TopBar Edit red, 0xFE=TopBar Leave red
void Draw_Control_Adj_TempCorrect_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	uint16_t title_x;
	uint8_t title_width;

	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	Draw_TopBar(leave_col, edit_col);

	// Draw title centered in TopBar
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	lan_str = LanguageTable[STR_Sensor_Cal_Short][g_parameter.language];
	title_width = strlen((char*)lan_str) * 8;
	title_x = (lcddev.width - title_width) / 2;
	Show_Str(title_x, 5, BLACK, WHITE, (char*)lan_str, 16, 0);

	// Draw horizontal separator line right below TopBar
	LCD_Fill(4, 22, lcddev.width - 4, 23, BLACK);

	Draw_Control_Adj_TempCorrect_Contect(selection);
}


void Draw_Control_Adj_TempLimit_Content(uint8_t selection)
{
	int int_val;
	uint8_t is_neg = 0;
	uint8_t d1, d2;

	// Draw Max (left) / Min (right) labels - colored when selected
	lan_str = LanguageTable[STR_Max][g_parameter.language];
	Show_Str(10, 40, (selection == 1) ? RED : BLACK, WHITE, (char*)lan_str, 16, 0);
	lan_str = LanguageTable[STR_Min][g_parameter.language];
	Show_Str(80, 40, (selection == 2) ? RED : BLACK, WHITE, (char*)lan_str, 16, 0);

	// In edit mode: show number; in save mode: keep number
	if(selection == 1 || selection == 2) {
		LCD_Fill(5, 36, lcddev.width, 133, WHITE);
		lan_str = LanguageTable[STR_Max][g_parameter.language];
		Show_Str(10, 40, (selection == 1) ? RED : BLACK, WHITE, (char*)lan_str, 16, 0);
		lan_str = LanguageTable[STR_Min][g_parameter.language];
		Show_Str(80, 40, (selection == 2) ? RED : BLACK, WHITE, (char*)lan_str, 16, 0);

		int_val = (selection == 1) ? temp_limit_max : temp_limit_min;
		if(int_val < 0) {
			is_neg = 1;
			int_val = -int_val;
		}
		d1 = int_val / 10;
		d2 = int_val % 10;
		// Minus sign
		if(is_neg) LCD_Fill(20, 92, 35, 96, BLACK);
		// Integer digits (32x64 font, y=60)
		if(int_val >= 10) {
			GUI_DrawBigDigit(36, 60, BLACK, WHITE, '0' + d1, 0);
			GUI_DrawBigDigit(68, 60, BLACK, WHITE, '0' + d2, 0);
		} else {
			GUI_DrawBigDigit(48, 60, BLACK, WHITE, '0' + d2, 0);
		}
		// Edit arrows (vertically centered on 32x64 digits)
		GUI_DrawMonoIcon16x16(6, 84, WHITE, RED, Icon16x16_Down_Arror);
		GUI_DrawMonoIcon16x16(108, 84, WHITE, RED, Icon16x16_Up_Arror);
	}

	// Save button (keep original position)
	lan_str = LanguageTable[STR_Save][g_parameter.language];
	if(selection == 3) {
		Show_Str(86, 140, RED, WHITE, (char*)lan_str, 16, 0);
	} else {
		Show_Str(86, 140, BLACK, WHITE, (char*)lan_str, 16, 0);
	}
}

void Draw_Control_Adj_TempLimit_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	uint16_t title_x;
	uint8_t title_width;

	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	Draw_TopBar(leave_col, edit_col);

	// Draw title centered in TopBar
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	lan_str = LanguageTable[STR_Limit][g_parameter.language];
	title_width = strlen((char*)lan_str) * 8;
	title_x = (lcddev.width - title_width) / 2;
	Show_Str(title_x, 5, BLACK, WHITE, (char*)lan_str, 16, 0);

	// Draw horizontal separator line right below TopBar
	LCD_Fill(4, 22, lcddev.width - 4, 23, BLACK);

	Draw_Control_Adj_TempLimit_Content(selection);
}


void Draw_Control_Adj_TempProtect_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	uint8_t i;
	uint16_t row_y[2] = {37, 73};  // Row 0=Max, Row 1=Min (shifted up 8px)
	uint8_t row_height = 32;
	uint16_t title_x;
	uint8_t title_width;
	char *line1[2] = {"Max Temp.", "Min Temp."};
	char *line2 = "Protect";
	char *line2_n = "Beskytte";

	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	Draw_TopBar(leave_col, edit_col);

	// Title: Protect centered in TopBar
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	lan_str = LanguageTable[STR_Protect_Temperature_short][g_parameter.language];
	title_width = strlen((char*)lan_str) * 8;
	title_x = (lcddev.width - title_width) / 2;
	Show_Str(title_x, 5, BLACK, WHITE, (char*)lan_str, 16, 0);

	// Separator right below TopBar
	LCD_Fill(4, 22, lcddev.width - 4, 23, BLACK);

	for(i = 0; i < 2; i++) {
		uint8_t is_sel = (!Top_Bar_Active && (i + 1 == selection)) ? 1 : 0;
		uint16_t text_color, bg_color, icon_color;
		uint8_t onoff;

		if(is_sel) {
			text_color = WHITE;
			bg_color = RED;
			LCD_Fill(4, row_y[i], lcddev.width - 4, row_y[i] + row_height, RED);
		} else {
			text_color = BLACK;
			bg_color = WHITE;
			LCD_Fill(4, row_y[i], lcddev.width - 4, row_y[i] + row_height, WHITE);
		}

		// Arrow icon (left of text)
		GUI_DrawMonoIcon16x16(6, row_y[i] + (row_height - 16) / 2, WHITE, (is_sel ? RED : BLACK), Icon16x16_Arrow);

		// Two-line text using direct Show_Str (always single-line strings)
		POINT_COLOR = text_color;
		BACK_COLOR = bg_color;
		Show_Str(28, row_y[i], text_color, bg_color, line1[i], 16, 0);
		if(g_parameter.language == LANG_ENGLISH){
				Show_Str(28, row_y[i] + 16, text_color, bg_color, line2, 16, 0);
		}else if(g_parameter.language == LANG_Norwegian){
				Show_Str(28, row_y[i] + 16, text_color, bg_color, line2_n, 16, 0);
		}

		// ON/OFF icon at right side (16x16)
		onoff = (i == 0) ? g_parameter.temp_protect_max_switch : g_parameter.temp_protect_min_switch;
		icon_color = is_sel ? RED : BLACK;
		if(onoff == 0) {
			GUI_DrawMonoIcon16x16(108, row_y[i] + (row_height - 16) / 2, WHITE, icon_color, OFF_Icon_16x16);
		} else {
			GUI_DrawMonoIcon16x16(108, row_y[i] + (row_height - 16) / 2, WHITE, icon_color, ON_Icon_16x16);
		}
	}

}

void Draw_Control_Adj_TempProtect_Max_Content(uint8_t selection)
{
	int int_val;
	uint8_t is_neg = 0, d1, d2;
	lan_str = LanguageTable[STR_Save][g_parameter.language];

	LCD_Fill(5, 36, lcddev.width - 5, 130, WHITE);

	// ON/OFF section (selection == 1)
	uint16_t onoff_color = (selection == 1) ? RED : BLACK;
	if(temp_protect_max_switch == 0) {
		GUI_DrawMonoIcon32x32(48, 40, onoff_color, WHITE, OFF_Icon_32x32);
	} else {
		GUI_DrawMonoIcon32x32(48, 40, onoff_color, WHITE, ON_Icon_32x32);
	}

	// ON/OFF edit arrows (selection == 1)
	if(selection == 1) {
		GUI_DrawMonoIcon16x16(20, 48, WHITE, RED, Icon16x16_Down_Arror);
		GUI_DrawMonoIcon16x16(92, 48, WHITE, RED, Icon16x16_Up_Arror);
	}

	// Number section (selection == 2)
	if(temp_protect_max_switch == 1) {
		int_val = temp_protect_max;
		if(int_val < 0) { is_neg = 1; int_val = -int_val; }
		d1 = int_val / 10;
		d2 = int_val % 10;
		if(is_neg) LCD_Fill(44, 101, 59, 105, BLACK);
		if(int_val >= 10) {
			GUI_DrawBigDigit(52, 87, BLACK, WHITE, '0' + d1, 1);
			GUI_DrawBigDigit(68, 87, BLACK, WHITE, '0' + d2, 1);
		} else {
			GUI_DrawBigDigit(60, 87, BLACK, WHITE, '0' + d2, 1);
		}
		// Number edit arrows (selection == 2)
		if(selection == 2) {
			GUI_DrawMonoIcon16x16(20, 97, WHITE, RED, Icon16x16_Down_Arror);
			GUI_DrawMonoIcon16x16(96, 97, WHITE, RED, Icon16x16_Up_Arror);
		}
	}

	// Save button (selection == 3) — keep original position
	if(selection == 3) {
		Show_Str(86, 135, RED, WHITE, (char*)lan_str, 16, 0);
	} else {
		Show_Str(86, 135, BLACK, WHITE, (char*)lan_str, 16, 0);
	}
}

void Draw_Control_Adj_TempProtect_Max_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	uint16_t title_x;
	uint8_t title_width;

	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	Draw_TopBar(leave_col, edit_col);

	// Draw title centered in TopBar
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	lan_str = LanguageTable[STR_Max_Temperature_Protect][g_parameter.language];
	title_width = strlen((char*)lan_str) * 8;
	title_x = (lcddev.width - title_width) / 2;
	Show_Str(title_x, 5, BLACK, WHITE, (char*)lan_str, 16, 0);

	// Draw horizontal separator line right below TopBar
	LCD_Fill(4, 22, lcddev.width - 4, 23, BLACK);

	Draw_Control_Adj_TempProtect_Max_Content(selection);
}

void Draw_Control_Adj_TempProtect_Min_Content(uint8_t selection)
{
	int int_val;
	uint8_t is_neg = 0, d1, d2;
	lan_str = LanguageTable[STR_Save][g_parameter.language];

	LCD_Fill(5, 36, lcddev.width - 5, 130, WHITE);

	// ON/OFF section (selection == 1)
	uint16_t onoff_color = (selection == 1) ? RED : BLACK;
	if(temp_protect_min_switch == 0) {
		GUI_DrawMonoIcon32x32(48, 40, onoff_color, WHITE, OFF_Icon_32x32);
	} else {
		GUI_DrawMonoIcon32x32(48, 40, onoff_color, WHITE, ON_Icon_32x32);
	}

	// ON/OFF edit arrows (selection == 1)
	if(selection == 1) {
		GUI_DrawMonoIcon16x16(20, 48, WHITE, RED, Icon16x16_Down_Arror);
		GUI_DrawMonoIcon16x16(92, 48, WHITE, RED, Icon16x16_Up_Arror);
	}

	// Number section (selection == 2)
	if(temp_protect_min_switch == 1) {
		int_val = temp_protect_min;
		if(int_val < 0) { is_neg = 1; int_val = -int_val; }
		d1 = int_val / 10;
		d2 = int_val % 10;
		if(is_neg) LCD_Fill(44, 101, 59, 105, BLACK);
		if(int_val >= 10) {
			GUI_DrawBigDigit(52, 87, BLACK, WHITE, '0' + d1, 1);
			GUI_DrawBigDigit(68, 87, BLACK, WHITE, '0' + d2, 1);
		} else {
			GUI_DrawBigDigit(60, 87, BLACK, WHITE, '0' + d2, 1);
		}
		// Number edit arrows (selection == 2)
		if(selection == 2) {
			GUI_DrawMonoIcon16x16(20, 97, WHITE, RED, Icon16x16_Down_Arror);
			GUI_DrawMonoIcon16x16(96, 97, WHITE, RED, Icon16x16_Up_Arror);
		}
	}

	// Save button (selection == 3) — keep original position
	if(selection == 3) {
		Show_Str(86, 135, RED, WHITE, (char*)lan_str, 16, 0);
	} else {
		Show_Str(86, 135, BLACK, WHITE, (char*)lan_str, 16, 0);
	}
}

void Draw_Control_Adj_TempProtect_Min_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	uint16_t title_x;
	uint8_t title_width;

	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	Draw_TopBar(leave_col, edit_col);

	// Draw title centered in TopBar
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	lan_str = LanguageTable[STR_Min_Temperature_Protect][g_parameter.language];
	title_width = strlen((char*)lan_str) * 8;
	title_x = (lcddev.width - title_width) / 2;
	Show_Str(title_x, 5, BLACK, WHITE, (char*)lan_str, 16, 0);

	// Draw horizontal separator line right below TopBar
	LCD_Fill(4, 22, lcddev.width - 4, 23, BLACK);

	Draw_Control_Adj_TempProtect_Min_Content(selection);
}


void Draw_Control_Adj_Power_On_State_Arrow(uint8_t selection)
{
	uint8_t i;
	uint16_t row_y[3] = {34, 67, 100};  // shifted up 8px
	uint8_t row_height = 32;
	uint8_t is_edit = (selection == 1);

	// Clear content area (leave room for Save)
	LCD_Fill(0, 34, lcddev.width, 150, WHITE);

	for(i = 0; i < 3; i++) {
		// In TopBar mode show the saved value; in Edit mode show the current selection
		uint8_t display_state = (selection == 0) ? g_parameter.power_on_state : power_on_state;
		uint8_t is_current = (i + 1 == display_state) ? 1 : 0;
		uint16_t text_color, bg_color;
		uint16_t arrow_bg;

		if(is_edit && is_current) {
			// Edit mode, selected item: red bg, white text
			text_color = WHITE;
			bg_color = RED;
			LCD_Fill(4, row_y[i], lcddev.width - 4, row_y[i] + row_height, RED);
		} else {
			// TopBar/Save mode or unselected: white bg, black text
			text_color = BLACK;
			bg_color = WHITE;
			LCD_Fill(4, row_y[i], lcddev.width - 4, row_y[i] + row_height, WHITE);
		}

		// Arrow icon on the left: red for current, black for others
		arrow_bg = is_current ? RED : BLACK;
		GUI_DrawMonoIcon16x16(6, row_y[i] + (row_height - 16) / 2, WHITE, arrow_bg, Icon16x16_Arrow);

		POINT_COLOR = text_color;
		BACK_COLOR = bg_color;
		if(i == 0) {
			lan_str = LanguageTable[STR_Keep][g_parameter.language];
			Show_Str(26, row_y[i], text_color, bg_color, (char*)lan_str, 16, 0);
			lan_str = LanguageTable[STR_State][g_parameter.language];
			Show_Str(26, row_y[i] + 16, text_color, bg_color, (char*)lan_str, 16, 0);
		} else if(i == 1) {
			lan_str = LanguageTable[STR_Device_Close][g_parameter.language];
			Show_FuncSetting_Row_Text(26, row_y[i], text_color, bg_color, (char*)lan_str, 16);
		} else {
			lan_str = LanguageTable[STR_Device_Open][g_parameter.language];
			Show_FuncSetting_Row_Text(26, row_y[i], text_color, bg_color, (char*)lan_str, 16);
		}
	}

	// Save button (always visible) — keep original position
	lan_str = LanguageTable[STR_Save][g_parameter.language];
	if(selection == 2) {
		Show_Str(86, 142, RED, WHITE, (char*)lan_str, 16, 0);
	} else {
		Show_Str(86, 142, BLACK, WHITE, (char*)lan_str, 16, 0);
	}
}
void Draw_Control_Adj_Power_On_State_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	uint16_t title_x;
	uint8_t title_width;

	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	Draw_TopBar(leave_col, edit_col);

	// Draw title centered in TopBar
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	lan_str = LanguageTable[STR_State][g_parameter.language];
	title_width = strlen((char*)lan_str) * 8;
	title_x = (lcddev.width - title_width) / 2;
	Show_Str(title_x, 5, BLACK, WHITE, (char*)lan_str, 16, 0);

	// Draw horizontal separator line right below TopBar
	LCD_Fill(4, 22, lcddev.width - 4, 23, BLACK);

	Draw_Control_Adj_Power_On_State_Arrow(selection);
}



void Draw_Control_Adj_Power_Limit_Content(uint8_t selection)
{
	uint8_t ten_digi, one_digi;
	ten_digi = power_limit / 10;
	one_digi = power_limit % 10;
	lan_str = LanguageTable[STR_Save][g_parameter.language];

	// Clear content area (below separator)
	LCD_Fill(5, 36, lcddev.width - 5, 130, WHITE);

	// ===== ON/OFF section (y=40~72) =====
	uint16_t onoff_color = (selection == 1) ? RED : BLACK;
	if(power_limit_switch == 0) {
		GUI_DrawMonoIcon32x32(48, 40, onoff_color, WHITE, OFF_Icon_32x32);
	} else {
		GUI_DrawMonoIcon32x32(48, 40, onoff_color, WHITE, ON_Icon_32x32);
	}

	// ON/OFF edit arrows (selection == 1): left/right of icon
	if(selection == 1) {
		GUI_DrawMonoIcon16x16(20, 48, WHITE, RED, Icon16x16_Down_Arror);
		GUI_DrawMonoIcon16x16(92, 48, WHITE, RED, Icon16x16_Up_Arror);
	}

	// ===== Number + arrows section (y=90) =====
	if(power_limit_switch == 1) {
		// Draw number digits (16x32 font, shifted up to y=87)
		if(ten_digi != 0) {
			GUI_DrawBigDigit(52, 87, BLACK, WHITE, '0' + ten_digi, 1);
		}
		GUI_DrawBigDigit(68, 87, BLACK, WHITE, '0' + one_digi, 1);

		// Draw "Min" text (right of digits)
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		lan_str = LanguageTable[STR_Min][g_parameter.language];
		Show_Str(92, 80, BLACK, WHITE, (char*)lan_str, 16, 0);

		// Number edit arrows (selection == 2): left/right of number
		if(selection == 2) {
			// Down arrow on left (x=20, y=97)
			GUI_DrawMonoIcon16x16(20, 97, WHITE, RED, Icon16x16_Down_Arror);
			// Up arrow on right (x=96, y=97)
			GUI_DrawMonoIcon16x16(96, 97, WHITE, RED, Icon16x16_Up_Arror);
		}
	}

	// ===== Save button (keep original position) =====
	lan_str = LanguageTable[STR_Save][g_parameter.language];
	if(selection == 3) {
		Show_Str(86, 135, RED, WHITE, (char*)lan_str, 16, 0);
	} else {
		Show_Str(86, 135, BLACK, WHITE, (char*)lan_str, 16, 0);
	}
}

void Draw_Control_Adj_Power_Limit_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	uint16_t title_x;
	uint8_t title_width;

	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	Draw_TopBar(leave_col, edit_col);

	// Draw title centered in TopBar
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	lan_str = LanguageTable[STR_Limit][g_parameter.language];
	title_width = strlen((char*)lan_str) * 8;
	title_x = (lcddev.width - title_width) / 2;
	Show_Str(title_x, 5, BLACK, WHITE, (char*)lan_str, 16, 0);

	// Draw horizontal separator line right below TopBar
	LCD_Fill(4, 22, lcddev.width - 4, 23, BLACK);

	Draw_Control_Adj_Power_Limit_Content(selection);
}


void Draw_Control_Adj_Temp_Swing_Content(uint8_t selection)
{
	uint8_t int_part;
	uint8_t dec_part;
	int_part = (uint8_t)temp_swing;
	dec_part = (uint8_t)(temp_swing * 10.0f) % 10;
	lan_str = LanguageTable[STR_Save][g_parameter.language];
	if(selection == 0) {
		// TopBar mode: show number + Save (no arrows)
		LCD_Fill(5, 40, lcddev.width - 5, 126, WHITE);
		GUI_DrawBigDigit(30, 50, BLACK, WHITE, '0' + int_part, 0);
		LCD_Fill(63, 102, 66, 105, BLACK);
		GUI_DrawBigDigit(68, 50, BLACK, WHITE, '0' + dec_part, 0);
		GUI_DrawMonoIcon24x24(100, 54, BLACK, WHITE, Celsius_Icon_24x24);
		Show_Str(86, 135, BLACK, WHITE, (char*)lan_str, 16, 0);
	} else if(selection == 1) {
		// Edit mode: show arrows + number + Save
		LCD_Fill(5, 40, lcddev.width - 5, 126, WHITE);
		GUI_DrawMonoIcon16x16(6, 82, WHITE, RED, Icon16x16_Up_Arror);
		GUI_DrawMonoIcon16x16(106, 82, WHITE, RED, Icon16x16_Down_Arror);
		GUI_DrawBigDigit(30, 50, BLACK, WHITE, '0' + int_part, 0);
		LCD_Fill(63, 102, 66, 105, BLACK);
		GUI_DrawBigDigit(68, 50, BLACK, WHITE, '0' + dec_part, 0);
		GUI_DrawMonoIcon24x24(100, 54, BLACK, WHITE, Celsius_Icon_24x24);
		Show_Str(86, 135, BLACK, WHITE, (char*)lan_str, 16, 0);
	} else if(selection == 2) {
		// Save mode: clear arrows, Save red
		LCD_Fill(6, 82, 22, 98, WHITE);
		LCD_Fill(106, 82, 122, 98, WHITE);
		Show_Str(86, 135, RED, WHITE, (char*)lan_str, 16, 0);
	}
}

void Draw_Control_Adj_Temp_Swing_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	uint16_t title_x;
	uint8_t title_width;

	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	Draw_TopBar(leave_col, edit_col);
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	lan_str = LanguageTable[STR_Temp_Swing][g_parameter.language];
	title_width = strlen((char*)lan_str) * 8;
	title_x = (lcddev.width - title_width) / 2;
	Show_Str(title_x, 5, BLACK, WHITE, (char*)lan_str, 16, 0);
	LCD_Fill(4, 22, lcddev.width - 4, 23, BLACK);
	Draw_Control_Adj_Temp_Swing_Content(selection);
}


// Draw PWM Duty Cycle Page
// Draw PWM Duty Cycle content (Day / Night duty editing)
void Draw_Control_Adj_PWM_Duty_Cycle_Content(uint8_t selection)
{
		uint8_t is_day = (selection == 1);
		uint8_t is_night = (selection == 2);

		uint16_t day_color = is_day ? RED : BLACK;
		uint16_t night_color = is_night ? RED : BLACK;
		uint8_t duty_val;
		uint8_t ten_digi, one_digi;

		LCD_Fill(0, 34, lcddev.width, 160, WHITE);

		// Day / Night labels
		POINT_COLOR = day_color;
		BACK_COLOR = WHITE;
		lan_str = LanguageTable[STR_Day][g_parameter.language];
		Show_Str(12, 36, day_color, WHITE, (char*)lan_str, 16, 0);

		POINT_COLOR = night_color;
		lan_str = LanguageTable[STR_Night][g_parameter.language];
		Show_Str(78, 36, night_color, WHITE, (char*)lan_str, 16, 0);

		if(selection == 0) {
				// TopBar mode — show both values compactly
				char buf[24];
				sprintf(buf, "%d%%  /  %d%%",
						g_parameter.pwm_day_duty,
						g_parameter.pwm_night_duty);
				Show_Str(16, 66, BLACK, WHITE, buf, 16, 0);
				return;
		}

		// Edit mode — pick which value to display
		duty_val = is_day ? temp_pwm_day_duty : temp_pwm_night_duty;
		ten_digi = duty_val / 10;
		one_digi = duty_val % 10;

		// Value (32x64 font, centered) — keep original position
		GUI_DrawBigDigit(32, 63, BLACK, WHITE, '0' + ten_digi, 0);
		GUI_DrawBigDigit(64, 63, BLACK, WHITE, '0' + one_digi, 0);
		GUI_DrawMonoIcon16x16(96, 70, BLACK, WHITE, Percent_Icon_16x16);

		// Up/Down arrows (edit mode: selection == 1 or 2) — keep original position
		if(selection == 1 || selection == 2) {
				GUI_DrawMonoIcon16x16(12, 91, WHITE, RED, Icon16x16_Down_Arror);
				GUI_DrawMonoIcon16x16(100, 91, WHITE, RED, Icon16x16_Up_Arror);
		} else {
				LCD_Fill(16, 91, 32, 107, WHITE);
				LCD_Fill(100, 91, 116, 107, WHITE);
		}

		// Save button (bottom right) — keep original position
		lan_str = LanguageTable[STR_Save][g_parameter.language];
		if(selection == 3)
				Show_Str(94, 142, RED, WHITE, (char*)lan_str, 16, 0);
		else
				Show_Str(94, 142, BLACK, WHITE, (char*)lan_str, 16, 0);
}

// Draw full PWM Duty Cycle page (TopBar + title + content)
void Draw_Control_Adj_PWM_Duty_Cycle_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
		uint16_t title_x;
		uint8_t title_width;

		LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
		Draw_TopBar(leave_col, edit_col);

		// Draw title centered in TopBar
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		lan_str = LanguageTable[STR_PWM_Duty][g_parameter.language];
		title_width = strlen((char*)lan_str) * 8;
		title_x = (lcddev.width - title_width) / 2;
		Show_Str(title_x, 5, BLACK, WHITE, (char*)lan_str, 16, 0);
		LCD_Fill(4, 22, lcddev.width - 4, 23, BLACK);

		Draw_Control_Adj_PWM_Duty_Cycle_Content(selection);
}

// Draw Operation Mode PWM Setting page content (Duty Cycle / Duration)
void Draw_Operation_Mode_PWM_Setting_Content(uint8_t selection)
{
	uint8_t ten_digi, one_digi;
	uint8_t display_val;

	// Clear content area (below separator, keep TopBar/title intact)
	LCD_Fill(0, 44, lcddev.width, 130, WHITE);

	// Draw Duty (left) / Duration (right) labels - colored when selected
	lan_str = LanguageTable[STR_Duty][g_parameter.language];
	Show_Str(10, 48, (selection == 1) ? RED : BLACK, WHITE, (char*)lan_str, 16, 0);
	lan_str = LanguageTable[STR_Duration][g_parameter.language];
	Show_Str(80, 48, (selection == 2) ? RED : BLACK, WHITE, (char*)lan_str, 16, 0);

	// Get current value
	if(selection == 1 || selection == 0 || selection == 3) {
		display_val = pwm_duty_cycle;
	} else {
		display_val = pwm_duration;
	}

	// Value (32x64 font)
	ten_digi = display_val / 10;
	one_digi = display_val % 10;
	if(display_val >= 10) {
		GUI_DrawBigDigit(32, 68, BLACK, WHITE, '0' + ten_digi, 0);
		GUI_DrawBigDigit(64, 68, BLACK, WHITE, '0' + one_digi, 0);
	} else {
		GUI_DrawBigDigit(48, 68, BLACK, WHITE, '0' + one_digi, 0);
	}

	// Unit label
	if(selection == 1) {
		GUI_DrawMonoIcon16x16(96, 75, BLACK, WHITE, Percent_Icon_16x16);
	} else if(selection == 2 || selection == 3) {
		Show_Str(96, 72, BLACK, WHITE, "H", 16, 0);
	} else {
		// In TopBar mode, show Duty Cycle value with % icon
		GUI_DrawMonoIcon16x16(96, 75, BLACK, WHITE, Percent_Icon_16x16);
	}

	// Up/Down arrows (edit mode: selection == 1 or 2)
	if(selection == 1 || selection == 2) {
		GUI_DrawMonoIcon16x16(12, 100, WHITE, RED, Icon16x16_Down_Arror);
		GUI_DrawMonoIcon16x16(100, 100, WHITE, RED, Icon16x16_Up_Arror);
	} else {
		LCD_Fill(12, 100, 28, 116, WHITE);
		LCD_Fill(100, 100, 116, 116, WHITE);
	}

	// Save button
	lan_str = LanguageTable[STR_Save][g_parameter.language];
	if(selection == 3) {
		Show_Str(86, 142, RED, WHITE, (char*)lan_str, 16, 0);
	} else {
		Show_Str(86, 142, BLACK, WHITE, (char*)lan_str, 16, 0);
	}
}

// Draw full Operation Mode PWM Setting page (TopBar + title + content)
void Draw_Operation_Mode_PWM_Setting_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	Draw_TopBar(leave_col, edit_col);

	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	lan_str = LanguageTable[STR_PWM_Setting][g_parameter.language];
	Show_Str(10, 24, BLACK, WHITE, (char*)lan_str, 16, 0);
	LCD_Fill(4, 42, lcddev.width - 4, 43, BLACK);

	Draw_Operation_Mode_PWM_Setting_Content(selection);
}

// Draw Control Adj PWM Setting Page (entry page with 2 options)
// Modeled after Draw_Control_Adj_Sensor_Choices
void Draw_Control_Adj_PWM_Setting_Choices(uint8_t selection)
{
		uint16_t row_y[2] = {34, 72};  // Option 1=34, Option 2=72 (extra 4px gap below divider, 6px between rows)
		uint8_t row_height = 32;        // exactly two 16px lines
		uint8_t i;
		uint8_t is_edit = (selection == 1);

		for(i = 0; i < 2; i++) {
				uint16_t text_color, bg_color;
				uint8_t is_selected = (pwm_setting_selection == i);

				if(is_edit && is_selected) {
						// Edit mode, selected item: red bg, white text
						text_color = WHITE;
						bg_color = RED;
						LCD_Fill(4, row_y[i], lcddev.width - 4, row_y[i] + row_height, RED);
				} else {
						// TopBar mode or unselected: white bg, black text
						text_color = BLACK;
						bg_color = WHITE;
						LCD_Fill(4, row_y[i], lcddev.width - 4, row_y[i] + row_height, WHITE);
				}

				// Arrow icon: only red in edit mode for selected option; black in TopBar mode
				uint8_t arrow_y = row_y[i] + (row_height - 16) / 2;
				uint16_t arrow_bg = (is_edit && is_selected) ? RED : BLACK;
				GUI_DrawMonoIcon16x16(12, arrow_y, WHITE, arrow_bg, Icon16x16_Arrow);

				// Text (two 16px lines, exactly filling 32px row height)
				POINT_COLOR = text_color;
				BACK_COLOR = bg_color;
				if(i == 0) {
						lan_str = LanguageTable[STR_Astro_Time][g_parameter.language];
				} else {
						lan_str = LanguageTable[STR_PWM_Duty][g_parameter.language];
				}
				Show_Str(34, row_y[i], text_color, bg_color, (char*)lan_str, 16, 0);
				lan_str = LanguageTable[STR_Setting][g_parameter.language];
				Show_Str(34, row_y[i] + 16, text_color, bg_color, (char*)lan_str, 16, 0);
		}
}

void Draw_Control_Adj_PWM_Setting_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
		uint16_t title_x;
		uint8_t title_width;

		LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
		Draw_TopBar(leave_col, edit_col);

		// Draw title centered in TopBar
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		lan_str = LanguageTable[STR_PWM][g_parameter.language];
		title_width = strlen((char*)lan_str) * 8;
		title_x = (lcddev.width - title_width) / 2;
		Show_Str(title_x, 5, BLACK, WHITE, (char*)lan_str, 16, 0);

		// Draw horizontal separator line right below TopBar
		LCD_Fill(4, 22, lcddev.width - 4, 23, BLACK);

		Draw_Control_Adj_PWM_Setting_Choices(selection);
}

// Astro Time Setting sub-page
void Draw_Control_Adj_PWM_Astro_Time_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
		uint16_t title_x;
		uint8_t title_width;

		LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
		Draw_TopBar(leave_col, edit_col);

		// Draw title centered in TopBar
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		lan_str = LanguageTable[STR_Astro_Setting][g_parameter.language];
		title_width = strlen((char*)lan_str) * 8;
		title_x = (lcddev.width - title_width) / 2;
		Show_Str(title_x, 5, BLACK, WHITE, (char*)lan_str, 16, 0);

		// Draw horizontal separator line right below TopBar
		LCD_Fill(4, 22, lcddev.width - 4, 23, BLACK);

		Draw_Control_Adj_PWM_Astro_Time_Content(selection);
}

// Astro Time entry page content — 2-option chooser: Location / Tolerance
// Modeled after Draw_Control_Adj_PWM_Setting_Choices (up/down two options, left-side arrow icon)
// selection: 0=TopBar (read-only echo), 1=option selection mode (uses pwm_astro_time_selection)
void Draw_Control_Adj_PWM_Astro_Time_Content(uint8_t selection)
{
		uint16_t row_y[2] = {34, 72};  // Location=34, Tolerance=72 (same layout as PWM Setting Choices)
		uint8_t row_height = 32;
		uint8_t i;
		uint8_t is_edit = (selection == 1);

		for(i = 0; i < 2; i++) {
				uint16_t text_color, bg_color;
				uint8_t is_selected = (pwm_astro_time_selection == i);

				if(is_edit && is_selected) {
						// Edit mode, selected item: red bg, white text
						text_color = WHITE;
						bg_color = RED;
						LCD_Fill(4, row_y[i], lcddev.width - 4, row_y[i] + row_height, RED);
				} else {
						// TopBar mode or unselected: white bg, black text
						text_color = BLACK;
						bg_color = WHITE;
						LCD_Fill(4, row_y[i], lcddev.width - 4, row_y[i] + row_height, WHITE);
				}

				// Arrow icon: only red in edit mode for selected option; black in TopBar mode
				uint8_t arrow_y = row_y[i] + (row_height - 16) / 2;
				uint16_t arrow_bg = (is_edit && is_selected) ? RED : BLACK;
				GUI_DrawMonoIcon16x16(12, arrow_y, WHITE, arrow_bg, Icon16x16_Arrow);

				// Text (single line, vertically centered in row)
				POINT_COLOR = text_color;
				BACK_COLOR = bg_color;
				if(i == 0) {
						lan_str = LanguageTable[STR_Location][g_parameter.language];
				} else {
						lan_str = LanguageTable[STR_Tolerance][g_parameter.language];
				}
				Show_Str(34, row_y[i] + (row_height - 16) / 2, text_color, bg_color, (char*)lan_str, 16, 0);
		}
}

// Draw Control Adj PWM Location Page content — 3 choices: Oslo / Stockholm / Helsinki
// Modeled after Draw_Control_Adj_Sensor_Choices (arrow always red on the current selection,
// row highlighted red only while actively cycling, Save confirm at bottom)
// selection: 0=TopBar (read-only echo), 1=choosing (uses temp_location), 2=Save confirm
void Draw_Control_Adj_PWM_Location_Choices(uint8_t selection)
{
		uint16_t row_y[ASTRO_LOCATION_MAX] = {34, 62, 90};
		uint8_t row_height = 24;
		uint8_t i;
		uint8_t is_edit = (selection == 1);

		for(i = 0; i < ASTRO_LOCATION_MAX; i++) {
				uint16_t text_color, bg_color;
				uint8_t is_selected = (temp_location == i);

				if(is_edit && is_selected) {
						// Edit mode, selected item: red bg, white text
						text_color = WHITE;
						bg_color = RED;
						LCD_Fill(4, row_y[i], lcddev.width - 4, row_y[i] + row_height, RED);
				} else {
						// TopBar/Save mode or unselected: white bg, black text
						text_color = BLACK;
						bg_color = WHITE;
						LCD_Fill(4, row_y[i], lcddev.width - 4, row_y[i] + row_height, WHITE);
				}

				// Arrow icon: red for the current selection in any mode, black otherwise
				uint8_t arrow_y = row_y[i] + (row_height - 16) / 2;
				if(is_selected) {
						GUI_DrawMonoIcon16x16(12, arrow_y, WHITE, RED, Icon16x16_Arrow);
				} else {
						GUI_DrawMonoIcon16x16(12, arrow_y, WHITE, BLACK, Icon16x16_Arrow);
				}

				// Text
				POINT_COLOR = text_color;
				BACK_COLOR = bg_color;
				if(i == ASTRO_LOCATION_OSLO) {
						lan_str = LanguageTable[STR_Oslo][g_parameter.language];
				} else if(i == ASTRO_LOCATION_STOCKHOLM) {
						lan_str = LanguageTable[STR_Stockholm][g_parameter.language];
				} else {
						lan_str = LanguageTable[STR_Helsinki][g_parameter.language];
				}
				Show_Str(34, row_y[i] + (row_height - 16) / 2, text_color, bg_color, (char*)lan_str, 16, 0);
		}

		// Save button at bottom
		lan_str = LanguageTable[STR_Save][g_parameter.language];
		if(selection == 2){
				Show_Str(86, 125, RED, WHITE, (char*)lan_str, 16, 0);
		}else{
				Show_Str(86, 125, BLACK, WHITE, (char*)lan_str, 16, 0);
		}
}

// Draw Control Adj PWM Location Page (title "Location")
void Draw_Control_Adj_PWM_Location_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
		uint16_t title_x;
		uint8_t title_width;

		LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
		Draw_TopBar(leave_col, edit_col);

		// Draw title centered in TopBar
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		lan_str = LanguageTable[STR_Location][g_parameter.language];
		title_width = strlen((char*)lan_str) * 8;
		title_x = (lcddev.width - title_width) / 2;
		Show_Str(title_x, 5, BLACK, WHITE, (char*)lan_str, 16, 0);

		// Draw horizontal separator line right below TopBar
		LCD_Fill(4, 22, lcddev.width - 4, 23, BLACK);

		Draw_Control_Adj_PWM_Location_Choices(selection);
}

// Draw Control Adj PWM Tolerance Page content — signed number editor, -120..+120 minutes.
// Uses the full-size 32x64 digit font (font_size=0, same as Temp Limit/Protect) instead of
// the compact 16x32 font, per Benjamin's request for a bigger/easier-to-read number. This is
// noticeably tighter than the compact layout - worst case is the negative 3-digit "-120",
// where sign+digits is 116px wide, leaving only ~6px margin on each side of a 128px screen,
// and vertically the up-arrow + 64px-tall digit + down-arrow block runs from y=36 to y=138,
// right up against the Save button at y=142. Checked: it fits, just without much headroom.
// selection: 0=TopBar (read-only echo), 1=editing (uses temp_astro_tolerance_min), 2=Save confirm
void Draw_Control_Adj_PWM_Tolerance_Content(uint8_t selection)
{
		int val = temp_astro_tolerance_min;
		uint8_t is_neg = (val < 0) ? 1 : 0;
		int abs_val = is_neg ? -val : val;
		uint8_t num_digits;
		uint8_t d[3];
		uint8_t i;
		uint16_t num_color = (selection == 1) ? RED : BLACK;
		const uint8_t digit_w = 32;
		//const uint8_t digit_h = 64;
		const uint8_t sign_w = 20;
		const uint16_t digit_y = 56;
		const uint16_t up_arrow_y = 36;
		const uint16_t down_arrow_y = 122;
		uint16_t total_w, start_x, digits_x;

		LCD_Fill(0, 34, lcddev.width, 138, WHITE);

		if(abs_val >= 100) {
				num_digits = 3;
				d[0] = abs_val / 100;
				d[1] = (abs_val / 10) % 10;
				d[2] = abs_val % 10;
		} else if(abs_val >= 10) {
				num_digits = 2;
				d[0] = abs_val / 10;
				d[1] = abs_val % 10;
		} else {
				num_digits = 1;
				d[0] = abs_val;
		}

		total_w = num_digits * digit_w + (is_neg ? sign_w : 0);
		start_x = (lcddev.width - total_w) / 2;
		digits_x = start_x + (is_neg ? sign_w : 0);

		// Minus sign (horizontal bar, vertically centered on the 64px-tall digit glyph)
		if(is_neg) {
				LCD_Fill(start_x, digit_y + 30, start_x + 16, digit_y + 34, BLACK);
		}

		// Digits (full 32x64 size, centered as a block in the middle of the editable area)
		for(i = 0; i < num_digits; i++) {
				GUI_DrawBigDigit(digits_x + i * digit_w, digit_y, num_color, WHITE, '0' + d[i], 0);
		}

		// Up/Down red arrows above/below the number, centered over the DIGITS only
		// (excludes the minus sign, so a negative two-digit value keeps the arrows
		// centered between the tens and units digit, same as a positive value -
		// otherwise the sign's extra width pulls the center towards the tens digit)
		if(selection == 1) {
				uint16_t arrow_x = digits_x + (num_digits * digit_w) / 2 - 8;
				GUI_DrawMonoIcon16x16(arrow_x, up_arrow_y, WHITE, RED, Icon16x16_Up_Arror);
				GUI_DrawMonoIcon16x16(arrow_x, down_arrow_y, WHITE, RED, Icon16x16_Down_Arror);
		}

		// "min" unit label - top-right corner of the editable area (not under the number)
		lan_str = LanguageTable[STR_Min][g_parameter.language];
		Show_Str(lcddev.width - 4 - strlen((char*)lan_str) * 8, 36, BLACK, WHITE, (char*)lan_str, 16, 0);

		// Save button
		lan_str = LanguageTable[STR_Save][g_parameter.language];
		if(selection == 2){
				Show_Str(86, 142, RED, WHITE, (char*)lan_str, 16, 0);
		}else{
				Show_Str(86, 142, BLACK, WHITE, (char*)lan_str, 16, 0);
		}
}

// Draw Control Adj PWM Tolerance Page (title "Tolerance")
void Draw_Control_Adj_PWM_Tolerance_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
		uint16_t title_x;
		uint8_t title_width;

		LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
		Draw_TopBar(leave_col, edit_col);

		// Draw title centered in TopBar
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		lan_str = LanguageTable[STR_Tolerance][g_parameter.language];
		title_width = strlen((char*)lan_str) * 8;
		title_x = (lcddev.width - title_width) / 2;
		Show_Str(title_x, 5, BLACK, WHITE, (char*)lan_str, 16, 0);

		// Draw horizontal separator line right below TopBar
		LCD_Fill(4, 22, lcddev.width - 4, 23, BLACK);

		Draw_Control_Adj_PWM_Tolerance_Content(selection);
}
