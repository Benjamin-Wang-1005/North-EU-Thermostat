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

	if(g_parameter.font_size == 0){
		row_y[0] = 27;
		row_y[1] = 59;
		row_y[2] = 91;
		row_y[3] = 123;
		row_height = 32;
		visible_rows = 4;
	}else if(g_parameter.font_size == 1){
		row_y[0] = 30;
		row_y[1] = 60;
		row_y[2] = 90;
		row_height = 30;
		visible_rows = 3;
	}

	// Bounds check
	if(text_idx >= Control_Adj_Menu_Item_Count) return;
	if(row >= visible_rows) return;

	if(selected) {
		text_color = WHITE;
		bg_color = RED;
		// Draw red background for selected item
		LCD_Fill(0, row_y[row], lcddev.width, row_y[row] + row_height, RED);
	} else {
		text_color = BLACK;
		bg_color = WHITE;
		// Clear background to white
		LCD_Fill(0, row_y[row], lcddev.width, row_y[row] + row_height, WHITE);
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
	if(g_parameter.font_size == 0){
			lan_str = LanguageTable[Control_Adj_Menu_Items[text_idx].text_id][g_parameter.language];
			Show_FuncSetting_Row_Text(30, row_y[row], text_color, bg_color, (char*)lan_str, 16);
	}else if(g_parameter.font_size == 1){
			lan_str = LanguageTable[Control_Adj_Menu_Items_12x24[text_idx].text_id][g_parameter.language];
			Show_Str(30, row_y[row] + (row_height - 24) / 2, text_color, bg_color, (char*)lan_str, 24, 1);
	}
}


// Draw Control Adj Menu Page (portrait mode, 32px rows)
// selection: 0-7=options, 0=TopBar mode
void Draw_Control_Adj_Menu_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
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
	max_scroll = (Control_Adj_Menu_Item_Count > visible_rows) ? (Control_Adj_Menu_Item_Count - visible_rows) : 0;

	// Auto-adjust scroll so that selected item is visible
	if(!Top_Bar_Active && selection > 0) {
		if(selection > control_adj_menu_scroll + visible_rows) {
			control_adj_menu_scroll = selection - visible_rows;
		}
		if(selection - 1 < control_adj_menu_scroll) {
			control_adj_menu_scroll = selection - 1;
		}
	}

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
}


// Draw Floor/Room choices for Sensor Setting page (portrait)
// selection: 0=Room, 1=Floor (swapped order)
void Draw_Control_Adj_Sensor_Choices(uint8_t selection)
{
	uint16_t row_y[2] = {52, 78};  // Room=52, Floor=78
	uint8_t row_height = 24;
	
	// Draw Room option
	LCD_Fill(0, row_y[0], lcddev.width, row_y[0] + row_height, WHITE);
	if(current_sensor_type == 0) {
		if(selection == 1) {
			GUI_DrawMonoIcon16x16(12, row_y[0] + (row_height - 16) / 2, WHITE, RED, Icon16x16_Arrow);
		} else {
			GUI_DrawMonoIcon16x16(12, row_y[0] + (row_height - 16) / 2, WHITE, BLACK, Icon16x16_Arrow);
		}
	} else {
		LCD_Fill(12, row_y[0] + (row_height - 16) / 2, 28, row_y[0] + (row_height - 16) / 2 + 16, WHITE);
	}
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	lan_str = LanguageTable[STR_Room][g_parameter.language];
	Show_Str(34, row_y[0] + (row_height - 16) / 2, BLACK, WHITE, (char*)lan_str, 16, 0);

	// Draw Floor option
	LCD_Fill(0, row_y[1], lcddev.width, row_y[1] + row_height, WHITE);
	if(current_sensor_type == 1) {
		if(selection == 1) {
			GUI_DrawMonoIcon16x16(12, row_y[1] + (row_height - 16) / 2, WHITE, RED, Icon16x16_Arrow);
		} else {
			GUI_DrawMonoIcon16x16(12, row_y[1] + (row_height - 16) / 2, WHITE, BLACK, Icon16x16_Arrow);
		}
	} else {
		LCD_Fill(12, row_y[1] + (row_height - 16) / 2, 28, row_y[1] + (row_height - 16) / 2 + 16, WHITE);
	}
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	lan_str = LanguageTable[STR_Floor][g_parameter.language];
	Show_Str(34, row_y[1] + (row_height - 16) / 2, BLACK, WHITE, (char*)lan_str, 16, 0);

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
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	Draw_TopBar(leave_col, edit_col);

	// Draw "Temp.Read From" title
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	lan_str = LanguageTable[STR_Temperature_Read_From][g_parameter.language];
	Show_Str(10, 24, BLACK, WHITE, (char*)lan_str, 16, 0);

	// Draw horizontal separator line
	LCD_Fill(4, 40, lcddev.width - 4, 41, BLACK);

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
	Show_Str(10, 48, (selection == 1) ? RED : BLACK, WHITE, (char*)lan_str, 16, 0);
	lan_str = LanguageTable[STR_Floor][g_parameter.language];
	Show_Str(80, 48, (selection == 2) ? RED : BLACK, WHITE, (char*)lan_str, 16, 0);

	// In edit mode (1 or 2): draw number, clear area first
	// In save mode (3): keep number displayed, just update Save button
	if(selection == 1 || selection == 2) {
		LCD_Fill(5, 44, lcddev.width, 133, WHITE);
		// Redraw labels (cleared by LCD_Fill)
		lan_str = LanguageTable[STR_Room][g_parameter.language];
		Show_Str(10, 48, (selection == 1) ? RED : BLACK, WHITE, (char*)lan_str, 16, 0);
		lan_str = LanguageTable[STR_Floor][g_parameter.language];
		Show_Str(80, 48, (selection == 2) ? RED : BLACK, WHITE, (char*)lan_str, 16, 0);

		disp_val = (selection == 1) ? temp_correct_internal : temp_correct_external;
		is_neg = (disp_val < 0) ? 1 : 0;
		int_part = (int)(is_neg ? -disp_val : disp_val);
		dec_part = (int)((disp_val >= 0 ? disp_val : -disp_val) * 10) % 10;
		d1 = int_part / 10;
		d2 = int_part % 10;
		// Minus sign (keep at original position, not shifted left)
		if(is_neg) LCD_Fill(12, 100, 27, 104, BLACK);
		// Integer digits (32x64 font, y=68, shifted left 12px)
		if(int_part >= 10) {
			GUI_DrawBigDigit(12, 68, BLACK, WHITE, '0' + d1, 0);
			GUI_DrawBigDigit(44, 68, BLACK, WHITE, '0' + d2, 0);
		} else {
			GUI_DrawBigDigit(36, 68, BLACK, WHITE, '0' + d2, 0);
		}
		// Decimal point (right after ones digit)
		LCD_Fill(70, 124, 74, 128, BLACK);
		LCD_Fill(69, 125, 75, 127, BLACK);
		// Decimal digit (32x64 font, shifted left 12px)
		GUI_DrawBigDigit(78, 68, BLACK, WHITE, '0' + dec_part, 0);
	}

	// Save button
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
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	Draw_TopBar(leave_col, edit_col);
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	lan_str = LanguageTable[STR_Sensor_Cal_Short][g_parameter.language];
	Show_Str(10, 24, BLACK, WHITE, (char*)lan_str, 16, 0);
	LCD_Fill(4, 40, lcddev.width - 4, 41, BLACK);
	Draw_Control_Adj_TempCorrect_Contect(selection);
}


void Draw_Control_Adj_TempLimit_Content(uint8_t selection)
{
	int int_val;
	uint8_t is_neg = 0;
	uint8_t d1, d2;

	// Draw Max (left) / Min (right) labels - colored when selected
	lan_str = LanguageTable[STR_Max][g_parameter.language];
	Show_Str(10, 48, (selection == 1) ? RED : BLACK, WHITE, (char*)lan_str, 16, 0);
	lan_str = LanguageTable[STR_Min][g_parameter.language];
	Show_Str(80, 48, (selection == 2) ? RED : BLACK, WHITE, (char*)lan_str, 16, 0);

	// In edit mode: show number; in save mode: keep number
	if(selection == 1 || selection == 2) {
		LCD_Fill(5, 44, lcddev.width, 133, WHITE);
		lan_str = LanguageTable[STR_Max][g_parameter.language];
		Show_Str(10, 48, (selection == 1) ? RED : BLACK, WHITE, (char*)lan_str, 16, 0);
		lan_str = LanguageTable[STR_Min][g_parameter.language];
		Show_Str(80, 48, (selection == 2) ? RED : BLACK, WHITE, (char*)lan_str, 16, 0);

		int_val = (selection == 1) ? temp_limit_max : temp_limit_min;
		if(int_val < 0) {
			is_neg = 1;
			int_val = -int_val;
		}
		d1 = int_val / 10;
		d2 = int_val % 10;
		// Minus sign
		if(is_neg) LCD_Fill(12, 100, 27, 104, BLACK);
		// Integer digits (32x64 font, y=68)
		if(int_val >= 10) {
			GUI_DrawBigDigit(36, 68, BLACK, WHITE, '0' + d1, 0);
			GUI_DrawBigDigit(68, 68, BLACK, WHITE, '0' + d2, 0);
		} else {
			GUI_DrawBigDigit(48, 68, BLACK, WHITE, '0' + d2, 0);
		}
	}

	// Save button
	lan_str = LanguageTable[STR_Save][g_parameter.language];
	if(selection == 3) {
		Show_Str(86, 140, RED, WHITE, (char*)lan_str, 16, 0);
	} else {
		Show_Str(86, 140, BLACK, WHITE, (char*)lan_str, 16, 0);
	}
}

void Draw_Control_Adj_TempLimit_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	Draw_TopBar(leave_col, edit_col);
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	lan_str = LanguageTable[STR_Temperature_Limit][g_parameter.language];
	Show_Str(10, 24, BLACK, WHITE, (char*)lan_str, 16, 0);
	LCD_Fill(4, 40, lcddev.width - 4, 41, BLACK);
	Draw_Control_Adj_TempLimit_Content(selection);
}


void Draw_Control_Adj_TempProtect_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	uint8_t i;
	uint16_t row_y[2] = {45, 81};  // Row 0=Max, Row 1=Min
	uint8_t row_height = 32;
	char *line1[2] = {"Max Temp.", "Min Temp."};
	char *line2 = "Protect";
	
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	Draw_TopBar(leave_col, edit_col);

	// Title: Temp.Protect (hardcoded)
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	Show_Str(10, 24, BLACK, WHITE, "Temp.Protect", 16, 0);

	// Separator
	LCD_Fill(4, 40, lcddev.width - 4, 41, BLACK);

	for(i = 0; i < 2; i++) {
		uint8_t is_sel = (!Top_Bar_Active && (i + 1 == selection)) ? 1 : 0;
		uint16_t text_color, bg_color, icon_color;
		uint8_t onoff;

		if(is_sel) {
			text_color = WHITE;
			bg_color = RED;
			LCD_Fill(0, row_y[i], lcddev.width, row_y[i] + row_height, RED);
		} else {
			text_color = BLACK;
			bg_color = WHITE;
			LCD_Fill(0, row_y[i], lcddev.width, row_y[i] + row_height, WHITE);
		}

		// Arrow icon (left of text)
		GUI_DrawMonoIcon16x16(6, row_y[i] + (row_height - 16) / 2, WHITE, (is_sel ? RED : BLACK), Icon16x16_Arrow);

		// Two-line text using Show_FuncSetting_Row_Text (auto split)
		POINT_COLOR = text_color;
		BACK_COLOR = bg_color;
		Show_FuncSetting_Row_Text(28, row_y[i], text_color, bg_color, line1[i], 16);
		Show_FuncSetting_Row_Text(28, row_y[i] + 16, text_color, bg_color, line2, 16);

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

	LCD_Fill(5, 44, lcddev.width - 5, 130, WHITE);

	// ON/OFF section (selection == 1)
	uint16_t onoff_color = (selection == 1) ? RED : BLACK;
	if(temp_protect_max_switch == 0) {
		GUI_DrawMonoIcon32x32(48, 48, onoff_color, WHITE, OFF_Icon_32x32);
	} else {
		GUI_DrawMonoIcon32x32(48, 48, onoff_color, WHITE, ON_Icon_32x32);
	}

	// ON/OFF edit arrows (selection == 1)
	if(selection == 1) {
		GUI_DrawMonoIcon16x16(20, 56, WHITE, RED, Icon16x16_Down_Arror);
		GUI_DrawMonoIcon16x16(92, 56, WHITE, RED, Icon16x16_Up_Arror);
	}

	// Number section (selection == 2)
	if(temp_protect_max_switch == 1) {
		int_val = temp_protect_max;
		if(int_val < 0) { is_neg = 1; int_val = -int_val; }
		d1 = int_val / 10;
		d2 = int_val % 10;
		if(is_neg) LCD_Fill(44, 109, 59, 113, BLACK);
		if(int_val >= 10) {
			GUI_DrawBigDigit(52, 95, BLACK, WHITE, '0' + d1, 1);
			GUI_DrawBigDigit(68, 95, BLACK, WHITE, '0' + d2, 1);
		} else {
			GUI_DrawBigDigit(60, 95, BLACK, WHITE, '0' + d2, 1);
		}
		// Number edit arrows (selection == 2)
		if(selection == 2) {
			GUI_DrawMonoIcon16x16(20, 105, WHITE, RED, Icon16x16_Down_Arror);
			GUI_DrawMonoIcon16x16(96, 105, WHITE, RED, Icon16x16_Up_Arror);
		}
	}

	// Save button (selection == 3)
	if(selection == 3) {
		Show_Str(86, 135, RED, WHITE, (char*)lan_str, 16, 0);
	} else {
		Show_Str(86, 135, BLACK, WHITE, (char*)lan_str, 16, 0);
	}
}

void Draw_Control_Adj_TempProtect_Max_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	Draw_TopBar(leave_col, edit_col);
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	Show_Str(10, 24, BLACK, WHITE, "Max Temp.Prot", 16, 0);
	LCD_Fill(4, 40, lcddev.width - 4, 41, BLACK);
	Draw_Control_Adj_TempProtect_Max_Content(selection);
}

void Draw_Control_Adj_TempProtect_Min_Content(uint8_t selection)
{
	int int_val;
	uint8_t is_neg = 0, d1, d2;
	lan_str = LanguageTable[STR_Save][g_parameter.language];

	LCD_Fill(5, 44, lcddev.width - 5, 130, WHITE);

	// ON/OFF section (selection == 1)
	uint16_t onoff_color = (selection == 1) ? RED : BLACK;
	if(temp_protect_min_switch == 0) {
		GUI_DrawMonoIcon32x32(48, 48, onoff_color, WHITE, OFF_Icon_32x32);
	} else {
		GUI_DrawMonoIcon32x32(48, 48, onoff_color, WHITE, ON_Icon_32x32);
	}

	// ON/OFF edit arrows (selection == 1)
	if(selection == 1) {
		GUI_DrawMonoIcon16x16(20, 56, WHITE, RED, Icon16x16_Down_Arror);
		GUI_DrawMonoIcon16x16(92, 56, WHITE, RED, Icon16x16_Up_Arror);
	}

	// Number section (selection == 2)
	if(temp_protect_min_switch == 1) {
		int_val = temp_protect_min;
		if(int_val < 0) { is_neg = 1; int_val = -int_val; }
		d1 = int_val / 10;
		d2 = int_val % 10;
		if(is_neg) LCD_Fill(44, 109, 59, 113, BLACK);
		if(int_val >= 10) {
			GUI_DrawBigDigit(52, 95, BLACK, WHITE, '0' + d1, 1);
			GUI_DrawBigDigit(68, 95, BLACK, WHITE, '0' + d2, 1);
		} else {
			GUI_DrawBigDigit(60, 95, BLACK, WHITE, '0' + d2, 1);
		}
		// Number edit arrows (selection == 2)
		if(selection == 2) {
			GUI_DrawMonoIcon16x16(20, 105, WHITE, RED, Icon16x16_Down_Arror);
			GUI_DrawMonoIcon16x16(96, 105, WHITE, RED, Icon16x16_Up_Arror);
		}
	}

	// Save button (selection == 3)
	if(selection == 3) {
		Show_Str(86, 135, RED, WHITE, (char*)lan_str, 16, 0);
	} else {
		Show_Str(86, 135, BLACK, WHITE, (char*)lan_str, 16, 0);
	}
}

void Draw_Control_Adj_TempProtect_Min_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	Draw_TopBar(leave_col, edit_col);
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	Show_Str(10, 24, BLACK, WHITE, "Min Temp.Prot", 16, 0);
	LCD_Fill(4, 40, lcddev.width - 4, 41, BLACK);
	Draw_Control_Adj_TempProtect_Min_Content(selection);
}


void Draw_Control_Adj_Power_On_State_Arrow(uint8_t selection)
{
	uint8_t i;
	uint16_t row_y[3] = {42, 75, 108};
	uint8_t row_height = 32;
	
	// Clear content area (leave room for Save)
	LCD_Fill(0, 42, lcddev.width, 150, WHITE);

	for(i = 0; i < 3; i++) {
		uint8_t is_current = (i + 1 == power_on_state) ? 1 : 0;
		uint16_t text_color, bg_color;

		if(selection == 1 && is_current) {
			text_color = WHITE;
			bg_color = RED;
			LCD_Fill(0, row_y[i], lcddev.width, row_y[i] + row_height, RED);
		} else {
			text_color = BLACK;
			bg_color = WHITE;
			LCD_Fill(0, row_y[i], lcddev.width, row_y[i] + row_height, WHITE);
		}

		// Arrow icon on the left
		if(is_current) {
			GUI_DrawMonoIcon16x16(6, row_y[i] + (row_height - 16) / 2, WHITE, (selection == 1) ? RED : BLACK, Icon16x16_Arrow);
		} else {
			LCD_Fill(6, row_y[i] + (row_height - 16) / 2, 22, row_y[i] + (row_height - 16) / 2 + 16, WHITE);
		}

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

	// Save button (always visible)
	lan_str = LanguageTable[STR_Save][g_parameter.language];
	if(selection == 2) {
		Show_Str(86, 142, RED, WHITE, (char*)lan_str, 16, 0);
	} else {
		Show_Str(86, 142, BLACK, WHITE, (char*)lan_str, 16, 0);
	}
}
void Draw_Control_Adj_Power_On_State_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	Draw_TopBar(leave_col, edit_col);

	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	lan_str = LanguageTable[STR_Power_On_State][g_parameter.language];
	Show_Str(10, 24, BLACK, WHITE, (char*)lan_str, 16, 0);
	LCD_Fill(4, 40, lcddev.width - 4, 41, BLACK);

	Draw_Control_Adj_Power_On_State_Arrow(selection);
}



void Draw_Control_Adj_Power_Limit_Content(uint8_t selection)
{
	uint8_t ten_digi, one_digi;
	ten_digi = power_limit / 10;
	one_digi = power_limit % 10;
	lan_str = LanguageTable[STR_Save][g_parameter.language];

	// Clear content area (below separator)
	LCD_Fill(5, 44, lcddev.width - 5, 130, WHITE);

	// ===== ON/OFF section (y=48~80) =====
	uint16_t onoff_color = (selection == 1) ? RED : BLACK;
	if(power_limit_switch == 0) {
		GUI_DrawMonoIcon32x32(48, 48, onoff_color, WHITE, OFF_Icon_32x32);
	} else {
		GUI_DrawMonoIcon32x32(48, 48, onoff_color, WHITE, ON_Icon_32x32);
	}

	// ON/OFF edit arrows (selection == 1): left/right of icon
	if(selection == 1) {
		GUI_DrawMonoIcon16x16(20, 56, WHITE, RED, Icon16x16_Down_Arror);
		GUI_DrawMonoIcon16x16(92, 56, WHITE, RED, Icon16x16_Up_Arror);
	}

	// ===== Number + arrows section (y=98) =====
	if(power_limit_switch == 1) {
		// Draw number digits (16x32 font, shifted down to y=95)
		if(ten_digi != 0) {
			GUI_DrawBigDigit(52, 95, BLACK, WHITE, '0' + ten_digi, 1);
		}
		GUI_DrawBigDigit(68, 95, BLACK, WHITE, '0' + one_digi, 1);

		// Draw "Min" text (right of digits)
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		lan_str = LanguageTable[STR_Min][g_parameter.language];
		Show_Str(92, 88, BLACK, WHITE, (char*)lan_str, 16, 0);

		// Number edit arrows (selection == 2): left/right of number
		if(selection == 2) {
			// Down arrow on left (x=20, y=105)
			GUI_DrawMonoIcon16x16(20, 105, WHITE, RED, Icon16x16_Down_Arror);
			// Up arrow on right (x=100, y=105)
			GUI_DrawMonoIcon16x16(96, 105, WHITE, RED, Icon16x16_Up_Arror);
		}
	}

	// ===== Save button =====
	lan_str = LanguageTable[STR_Save][g_parameter.language];
	if(selection == 3) {
		Show_Str(86, 135, RED, WHITE, (char*)lan_str, 16, 0);
	} else {
		Show_Str(86, 135, BLACK, WHITE, (char*)lan_str, 16, 0);
	}
}

void Draw_Control_Adj_Power_Limit_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	Draw_TopBar(leave_col, edit_col);
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	lan_str = LanguageTable[STR_Power_Limit][g_parameter.language];
	Show_Str(10, 24, BLACK, WHITE, (char*)lan_str, 16, 0);
	LCD_Fill(4, 40, lcddev.width - 4, 41, BLACK);
	Draw_Control_Adj_Power_Limit_Content(selection);
}

void Draw_Control_Adj_Comfort_Mode_Content(uint8_t selection)
{
			LCD_Fill(24, 52, 74, 76, WHITE);
			LCD_Fill(10, 92, 158, 124, WHITE);
			if(comfort_mode == 0){
					// Draw square
					LCD_Fill(24, 52, 74, 54, BLACK); //
					LCD_Fill(24, 52, 26, 76, BLACK); //
					LCD_Fill(24, 74, 74, 76, BLACK); //
					LCD_Fill(72, 52, 74, 76, BLACK); //
					lan_str = LanguageTable[STR_OFF][g_parameter.language];
					Show_Str(30, 56, BLACK, WHITE, (char*)lan_str, 16, 0);
					GUI_DrawMonoIcon16x16(54, 56, BLACK, WHITE, Block_Icon_16x16);
					LCD_Fill(0, 108, lcddev.width, lcddev.height, WHITE);
					lan_str = LanguageTable[STR_Save][g_parameter.language];
					if((selection == 0) || (selection == 1)){
							Show_Str(127, 108, BLACK, WHITE, (char*)lan_str, 16, 0);
					}else if(selection == 2){
							Show_Str(127, 108, RED, WHITE, (char*)lan_str, 16, 0);
					}
			}else{
					LCD_Fill(24, 52, 74, 54, RED); //
					LCD_Fill(24, 52, 26, 76, RED); //
					LCD_Fill(24, 74, 74, 76, RED); //
					LCD_Fill(72, 52, 74, 76, RED); //
					lan_str = LanguageTable[STR_ON][g_parameter.language];
					Show_Str(50, 56, RED, WHITE, (char*)lan_str, 16, 0);
					GUI_DrawMonoIcon16x16(30, 56, RED, WHITE, Block_Icon_16x16);
					lan_str = LanguageTable[STR_Set_Floor_Material][g_parameter.language];
					if((selection == 0) || (selection == 1)){
							Show_Str(10, 92, BLACK, WHITE, (char*)lan_str, 16, 0);
					}else if(selection == 2){
							Show_Str(10, 92, RED, WHITE, (char*)lan_str, 16, 0);
					}
			}
			
}


void Draw_Control_Adj_Comfort_Mode_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
			Draw_TopBar(leave_col, edit_col);
			POINT_COLOR = BLACK;
			BACK_COLOR = WHITE;
			lan_str = LanguageTable[STR_Comfort_Mode][g_parameter.language];
			Show_Str(10, 24, BLACK, WHITE, (char*)lan_str, 16, 0);
	
			// Draw horizontal line below "Temp. Limit" title (y=42 to y=43, leave 4px margin on both sides)
			LCD_Fill(4, 40, lcddev.width - 4, 41, BLACK);
	
			Draw_Control_Adj_Comfort_Mode_Content(selection);
}


void Draw_Control_Adj_Comfort_Mode_Setting_Content(uint8_t selection)
{
			uint8_t y_pos[3] = {50, 70, 90};
			uint16_t color;
			lan_str = LanguageTable[STR_Wood_Laminate][g_parameter.language];
			Show_Str(25, 50, BLACK, WHITE, (char*)lan_str, 16, 0);
			lan_str = LanguageTable[STR_Tile_Concrete][g_parameter.language];
			Show_Str(25, 70, BLACK, WHITE, (char*)lan_str, 16, 0);
			lan_str = LanguageTable[STR_Fast_Response][g_parameter.language];
			Show_Str(25, 90, BLACK, WHITE, (char*)lan_str, 16, 0);
			LCD_Fill(5, 50, 21, 114, WHITE);
			color = (selection == 1) ? RED : BLACK;
			GUI_DrawMonoIcon16x16(5, y_pos[floor_material], WHITE, color, Icon16x16_Arrow);
			lan_str = LanguageTable[STR_Save][g_parameter.language];
			if(selection == 2){
					Show_Str(127, 108, RED, WHITE, (char*)lan_str, 16, 0);
			}else{
					Show_Str(127, 108, BLACK, WHITE, (char*)lan_str, 16, 0);
			}
			
}
void Draw_Control_Adj_Comfort_Mode_Setting_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
			Draw_TopBar(leave_col, edit_col);
			POINT_COLOR = BLACK;
			BACK_COLOR = WHITE;
			lan_str = LanguageTable[STR_Floor_Type][g_parameter.language];
			Show_Str(10, 24, BLACK, WHITE, (char*)lan_str, 16, 0);
			// Draw horizontal line below "Temp. Limit" title (y=42 to y=43, leave 4px margin on both sides)
			LCD_Fill(4, 40, lcddev.width - 4, 41, BLACK);
			Draw_Control_Adj_Comfort_Mode_Setting_Content(selection);
	
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
		LCD_Fill(5, 44, lcddev.width - 5, 130, WHITE);
		GUI_DrawBigDigit(30, 54, BLACK, WHITE, '0' + int_part, 0);
		LCD_Fill(63, 106, 66, 109, BLACK);
		GUI_DrawBigDigit(68, 54, BLACK, WHITE, '0' + dec_part, 0);
		GUI_DrawMonoIcon24x24(100, 58, BLACK, WHITE, Celsius_Icon_24x24);
		Show_Str(86, 135, BLACK, WHITE, (char*)lan_str, 16, 0);
	} else if(selection == 1) {
		// Edit mode: show arrows + number + Save
		LCD_Fill(5, 44, lcddev.width - 5, 130, WHITE);
		GUI_DrawMonoIcon16x16(6, 86, WHITE, RED, Icon16x16_Up_Arror);
		GUI_DrawMonoIcon16x16(106, 86, WHITE, RED, Icon16x16_Down_Arror);
		GUI_DrawBigDigit(30, 54, BLACK, WHITE, '0' + int_part, 0);
		LCD_Fill(63, 106, 66, 109, BLACK);
		GUI_DrawBigDigit(68, 54, BLACK, WHITE, '0' + dec_part, 0);
		GUI_DrawMonoIcon24x24(100, 58, BLACK, WHITE, Celsius_Icon_24x24);
		Show_Str(86, 135, BLACK, WHITE, (char*)lan_str, 16, 0);
	} else if(selection == 2) {
		// Save mode: clear arrows, Save red
		LCD_Fill(6, 86, 22, 102, WHITE);
		LCD_Fill(106, 86, 122, 102, WHITE);
		Show_Str(86, 135, RED, WHITE, (char*)lan_str, 16, 0);
	}
}

void Draw_Control_Adj_Temp_Swing_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	Draw_TopBar(leave_col, edit_col);
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	Show_Str(10, 24, BLACK, WHITE, "Temp.Swing", 16, 0);
	LCD_Fill(4, 40, lcddev.width - 4, 41, BLACK);
	Draw_Control_Adj_Temp_Swing_Content(selection);
}


// Draw PWM Duty Cycle Page
// Draw only the content area (value + arrows + Save)
// Called on UP/DOWN to avoid full screen redraw
void Draw_Control_Adj_PWM_Duty_Cycle_Content(uint8_t selection)
{
	uint8_t ten_digi, one_digi;
	ten_digi = pwm_duty_cycle / 10;
	one_digi = pwm_duty_cycle % 10;

	// Clear only the content area (below separator, keep TopBar/title intact)
	LCD_Fill(0, 44, lcddev.width, 130, WHITE);

	// Value (32x64 font)
	GUI_DrawBigDigit(32, 55, BLACK, WHITE, '0' + ten_digi, 0);
	GUI_DrawBigDigit(64, 55, BLACK, WHITE, '0' + one_digi, 0);
	GUI_DrawMonoIcon16x16(96, 62, BLACK, WHITE, Percent_Icon_16x16);

	// Up/Down arrows (edit mode: selection == 1)
	if(selection == 1) {
		GUI_DrawMonoIcon16x16(12, 79, WHITE, RED, Icon16x16_Down_Arror);
		GUI_DrawMonoIcon16x16(100, 79, WHITE, RED, Icon16x16_Up_Arror);
	} else {
		LCD_Fill(12, 79, 28, 95, WHITE);
		LCD_Fill(100, 79, 116, 95, WHITE);
	}

	// Save button
	lan_str = LanguageTable[STR_Save][g_parameter.language];
	if(selection == 2) {
		Show_Str(86, 142, RED, WHITE, (char*)lan_str, 16, 0);
	} else {
		Show_Str(86, 142, BLACK, WHITE, (char*)lan_str, 16, 0);
	}
}

// Draw full PWM Duty Cycle page (TopBar + title + content)
// Called only when entering the page
void Draw_Control_Adj_PWM_Duty_Cycle_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	Draw_TopBar(leave_col, edit_col);

	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	Show_Str(10, 24, BLACK, WHITE, "Duty Cycle", 16, 0);
	LCD_Fill(4, 42, lcddev.width - 4, 43, BLACK);

	Draw_Control_Adj_PWM_Duty_Cycle_Content(selection);
}
