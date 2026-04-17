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
	LCD_Fill(5, 70, 117, 106, WHITE);
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

// Draw a single row in Control Adj Menu page (for optimized update)
// row: 0-3 (display row position on screen)
// selected: 1=selected (red arrow bg), 0=not selected (black arrow bg)
void Draw_Control_Adj_Menu_Row(uint8_t row, uint8_t selected)
{
	uint16_t row_y[] = {30, 54, 78, 102};  // Y positions for 4 display rows
	uint16_t arrow_bg_color;
	char* menu_texts[] = {"Sensor", "Temp.Correct", "Temp.Limit", "Temp.Protect", "Power On State"};
	uint8_t text_idx = control_adj_menu_scroll + row;  // 0-4
	
	// Bounds check
	if(text_idx > 4) return;
	if(row > 3) return;
	
	if(selected) {
		arrow_bg_color = RED;
	} else {
		arrow_bg_color = BLACK;
	}
	
	// Clear this row area
	LCD_Fill(0, row_y[row], lcddev.width, row_y[row] + 24, WHITE);
	
	// Draw Arrow Icon (16x16) with appropriate background color
	// Arrow icon lines are WHITE, background is RED (selected) or BLACK (not selected)
	GUI_DrawMonoIcon16x16(10, row_y[row] + 4, WHITE, arrow_bg_color, Icon16x16_Arrow);
	
	// Draw menu text
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	Show_Str(30, row_y[row] + 4, BLACK, WHITE, menu_texts[text_idx], 16, 0);
}


// Draw Control Adj Menu Page
// This page shows:
// - Top-left: Leave Icon (black/red)
// - Top-right: Edit Icon (black/red)
// - 4 visible rows (out of 5 total) with scrolling
// - Arrow Icon on the left of each row (red bg for selected, black bg for others)
// - 5 menu items: Sensor / Temp. Correct / Temp. Limit / Temp. Protect / Power On State
// selection: 0-4=options, 0xFF=TopBar Edit red, 0xFE=TopBar Leave red
void Draw_Control_Adj_Menu_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	uint8_t i;
	uint8_t num_visible_rows;
	
	// Ensure scroll is valid (0 or 1 for 5 items with 4 visible)
	if(control_adj_menu_scroll > 1) control_adj_menu_scroll = 1;
	
	Draw_TopBar(leave_col, edit_col);
	
	// Calculate how many rows to draw (4 or less if near end)
	num_visible_rows = 5 - control_adj_menu_scroll;
	if(num_visible_rows > 4) num_visible_rows = 4;
	
	// Draw rows based on scroll position
	for(i = 0; i < num_visible_rows; i++) {
		uint8_t option_idx = control_adj_menu_scroll + i;  // 0-4
		uint8_t is_selected = 0;
		
		if(!Top_Bar_Active && (option_idx == selection - 1)) {
			is_selected = 1;
		}
		Draw_Control_Adj_Menu_Row(i, is_selected);
	}
}

// Draw Save button for Sensor Setting page
// selected: 1=red, 0=black
void Draw_Control_Adj_Sensor_Save(uint8_t selected)
{
	uint16_t save_color = selected ? RED : BLACK;
	uint16_t row_y = 95;   // Bottom area (moved up with overall layout)
	uint16_t save_x = 127; // Moved right, 4px margin from right edge (160-4-29=127, "Save" is ~29px wide)
	
	// Clear Save area
	LCD_Fill(save_x, row_y, 160, row_y + 20, WHITE);
	
	POINT_COLOR = save_color;
	BACK_COLOR = WHITE;
	Show_Str(save_x, row_y, save_color, WHITE, "Save", 16, 0);
}

/*
// Draw Floor/Room choices with red line arrow for TopBar mode
// Shows arrow at current_sensor_type (last saved value), default is Room (0)
void Draw_Control_Adj_Sensor_Choices_NoArrow(void)
{
	uint16_t row_y[] = {52, 72};  // Y positions for Room, Floor (moved up, consistent spacing)
	uint8_t i;
	
	// Clear the choices area (below the line, y=44 to y=100)
	LCD_Fill(0, 44, lcddev.width, 100, WHITE);
	
	// Draw Room and Floor options (swapped order: Room first, then Floor)
	for(i = 0; i < 2; i++) {
		// Draw text (always black on white background)
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		if(i == 0) {
			Show_Str(45, row_y[i] + 4, BLACK, WHITE, "Room", 16, 0);
		} else {
			Show_Str(45, row_y[i] + 4, BLACK, WHITE, "Floor", 16, 0);
		}
	}
	
	// Show red line arrow at current saved value (default Room=0)
	// Use RED foreground with WHITE background (red line style)
	GUI_DrawMonoIcon16x16(20, row_y[current_sensor_type] + 4, RED, WHITE, Icon16x16_Arrow);
	
	// Draw Save text at bottom
	Draw_Control_Adj_Sensor_Save(0);
}*/


// Draw Floor/Room choices for Sensor Setting page (Edit mode - red background arrow)
// selection: 0=Room, 1=Floor (swapped order)
void Draw_Control_Adj_Sensor_Choices(uint8_t selection)
{
	uint16_t row_y[] = {52, 72};  // Y positions for Room, Floor (moved up, consistent spacing)
	uint8_t i;
	
	// Clear the choices area (below the line, y=44 to y=100)
	LCD_Fill(0, 44, lcddev.width, 100, WHITE);
	
	// Draw Room and Floor options (swapped order: Room first, then Floor)
	for(i = 0; i < 2; i++) {
		// Draw text (always black on white background)
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		if(i == 0) {
			Show_Str(45, row_y[i] + 4, BLACK, WHITE, "Room", 16, 0);
		} else {
			Show_Str(45, row_y[i] + 4, BLACK, WHITE, "Floor", 16, 0);
		}
		
		// Draw Arrow Icon with RED BACKGROUND for selected item (edit mode style)
		// selection 0=Room, 1=Floor
		if((i+1) == selection) {
			GUI_DrawMonoIcon16x16(20, row_y[i] + 4, WHITE, RED, Icon16x16_Arrow);
		}
		// Non-selected items have no arrow icon
	}
	
	// Draw Save text at bottom (not selected yet in choices mode)
	Draw_Control_Adj_Sensor_Save(0);
}



// Draw Control Adj Sensor Setting Page
// This page shows:
// - Top-left: Leave Icon (black/red)
// - Top-right: Edit Icon (black/red)
// - Horizontal line below Top Bar
// - "Sensor" title above the line
// - "Floor" and "Room" options below the line with Arrow Icon
// - "Save" text at bottom (red when selected)
// selection: 0=Floor, 1=Room, 0xFF=TopBar Edit red, 0xFE=TopBar Leave red
void Draw_Control_Adj_Sensor_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	
	Draw_TopBar(leave_col, edit_col);
	
	
	// Draw "Sensor" title closer to Top Bar (left aligned, y=24 - moved up)
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	Show_Str(10, 24, BLACK, WHITE, "Sensor", 16, 0);
	
	// Draw horizontal line below "Sensor" title (y=42 to y=43, leave 4px margin on both sides)
	LCD_Fill(4, 42, lcddev.width - 4, 43, BLACK);
	
	// Draw Floor/Room choices (always show, but arrow only in option mode)
	
	//if(Top_Bar_Active) {
		// TopBar mode: show choices without arrow
	//	Draw_Control_Adj_Sensor_Choices_NoArrow();
	//} else {
		// Option mode: show choices with arrow
	//	Draw_Control_Adj_Sensor_Choices(selection);
	//}
	Draw_Control_Adj_Sensor_Choices(selection);
}


void Draw_Control_Adj_TempCorrect_Contect(uint8_t selection)
{
	
	if(selection == 1) {
		// Arrow points to Room
		GUI_DrawMonoIcon16x16(4, 52, WHITE, RED, Icon16x16_Arrow);
		Display_Adj_Number(temp_correct_internal, BLACK, 1);
		Show_Str(127, 108, BLACK, WHITE, "Save", 16, 0);
	} else if(selection == 2) {
		// Arrow points to Floor
		LCD_Fill(4, 52, 20, 68, WHITE);		//clear old Arrow
		LCD_Fill(5, 74, 117, 106, WHITE);	//clear digi area
		GUI_DrawMonoIcon16x16(80, 52, WHITE, RED, Icon16x16_Arrow);
		Display_Adj_Number(temp_correct_external, BLACK, 1);
		Show_Str(127, 108, BLACK, WHITE, "Save", 16, 0);
	} else if(selection == 3) {
		LCD_Fill(80, 52, 96, 68, WHITE);		//clearerr old Arrow
		Show_Str(127, 108, RED, WHITE, "Save", 16, 0);
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
	
	Draw_TopBar(leave_col, edit_col);
	
	// Draw "Temp. Correct" title (left aligned, y=24 - same as Sensor Setting)
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	Show_Str(10, 24, BLACK, WHITE, "Temp.Correct", 16, 0);
	
	// Draw horizontal line below "Temp. Correct" title (y=42 to y=43, leave 4px margin on both sides)
	LCD_Fill(4, 42, lcddev.width - 4, 43, BLACK);
	
	// Draw Internal/External options on the same row (y=52, below the line)
	// Internal at x=30, Arrow at x=90, External at x=110
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	Show_Str(25, 52, BLACK, WHITE, "Room", 16, 0);
	Show_Str(100, 52, BLACK, WHITE, "Floor", 16, 0);
	
	// Draw Arrow Icon between Internal and External
	// selection: 0=Internal (arrow at x=90), 1=External (arrow at x=90)
	Draw_Control_Adj_TempCorrect_Contect(selection);
	
}


void Draw_Control_Adj_TempLimit_Content(uint8_t selection)
{
	if(selection == 1) {
		// Arrow points to Max
		GUI_DrawMonoIcon16x16(4, 52, WHITE, RED, Icon16x16_Arrow);
		Display_Adj_Number((float)temp_limit_max, BLACK, 0);
		Show_Str(127, 108, BLACK, WHITE, "Save", 16, 0);
	} else if(selection == 2) {
		// Arrow points to Floor
		LCD_Fill(4, 52, 20, 68, WHITE);		//clearerr old Arrow
		LCD_Fill(5, 74, 117, 106, WHITE);	//clear digi area
		GUI_DrawMonoIcon16x16(80, 52, WHITE, RED, Icon16x16_Arrow);
		Display_Adj_Number((float)temp_limit_min, BLACK, 0);
		Show_Str(127, 108, BLACK, WHITE, "Save", 16, 0);
	} else if(selection == 3) {
		LCD_Fill(80, 52, 96, 68, WHITE);		//clearerr old Arrow
		Show_Str(127, 108, RED, WHITE, "Save", 16, 0);
	}
}

void Draw_Control_Adj_TempLimit_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	
		Draw_TopBar(leave_col, edit_col);
	
		// Draw "Temp. Correct" title (left aligned, y=24 - same as Sensor Setting)
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		Show_Str(10, 24, BLACK, WHITE, "Temp.Limit", 16, 0);
	
		// Draw horizontal line below "Temp. Limit" title (y=42 to y=43, leave 4px margin on both sides)
		LCD_Fill(4, 42, lcddev.width - 4, 43, BLACK);
	
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		Show_Str(25, 52, BLACK, WHITE, "Max", 16, 0);
		Show_Str(100, 52, BLACK, WHITE, "Min", 16, 0);
	
		Draw_Control_Adj_TempLimit_Content(selection);
	
}


void Draw_Control_Adj_TempProtect_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
		Draw_TopBar(leave_col, edit_col);
	
		// Draw "Temp. Correct" title (left aligned, y=24 - same as Sensor Setting)
		//POINT_COLOR = BLACK;
		//BACK_COLOR = WHITE;
		//Show_Str(10, 24, BLACK, WHITE, "Temp.Protect", 16, 0);
	
		// Draw horizontal line below "Temp. Limit" title (y=42 to y=43, leave 4px margin on both sides)
		//LCD_Fill(4, 42, lcddev.width - 4, 43, BLACK);
	
		if(selection == 1){
				LCD_Fill(0, 30, lcddev.width, 48, RED);
				POINT_COLOR = WHITE;
				BACK_COLOR = RED;
				Show_Str(5, 32, WHITE, RED, "Max Temp.Protect", 16, 0);
				if(temp_protect_max_switch == 0){
						GUI_DrawMonoIcon16x16(140, 32, WHITE, BLACK, OFF_Icon_16x16);
				}else{
						GUI_DrawMonoIcon16x16(140, 32, WHITE, BLACK, ON_Icon_16x16);
				}
		}else{
				LCD_Fill(0, 30, lcddev.width, 48, WHITE);
				POINT_COLOR = BLACK;
				BACK_COLOR = WHITE;
				Show_Str(5, 32, BLACK, WHITE, "Max Temp.Protect", 16, 0);
				if(temp_protect_max_switch == 0){
						GUI_DrawMonoIcon16x16(140, 32, BLACK, WHITE, OFF_Icon_16x16);
				}else{
						GUI_DrawMonoIcon16x16(140, 32, RED, WHITE, ON_Icon_16x16);
				}
		}
		
		if(selection == 2){
				LCD_Fill(0, 62, lcddev.width, 80, RED);
				POINT_COLOR = WHITE;
				BACK_COLOR = RED;
				Show_Str(5, 64, WHITE, RED, "Min Temp.Protect", 16, 0);
				if(temp_protect_min_switch == 0){
						GUI_DrawMonoIcon16x16(140, 64, WHITE, BLACK, OFF_Icon_16x16);
				}else{
						GUI_DrawMonoIcon16x16(140, 64, WHITE, BLACK, ON_Icon_16x16);
				}
		}else{
				LCD_Fill(0, 62, lcddev.width, 80, WHITE);
				POINT_COLOR = BLACK;
				BACK_COLOR = WHITE;
				Show_Str(5, 64, BLACK, WHITE, "Min Temp.Protect", 16, 0);
				if(temp_protect_min_switch == 0){
						GUI_DrawMonoIcon16x16(140, 64, BLACK, WHITE, OFF_Icon_16x16);
				}else{
						GUI_DrawMonoIcon16x16(140, 64, RED, WHITE, ON_Icon_16x16);
				}
		}
	
}

void Draw_Control_Adj_TempProtect_Max_Content(uint8_t selection)
{
		
		if(temp_protect_max_switch == 0){
				GUI_DrawMonoIcon32x32(96, 70, BLACK, WHITE, OFF_Icon_32x32);
		}else{
				GUI_DrawMonoIcon32x32(96, 70, RED, WHITE, ON_Icon_32x32);
		}
		
		if(selection == 1){
				Display_Adj_Number((float)temp_protect_max, BLACK, 0);
				GUI_DrawMonoIcon16x16(40, 54, WHITE, RED, Icon16x16_Up_Arror);     // Up arrow above hour (red bg)
				GUI_DrawMonoIcon16x16(40, 104, WHITE, RED, Icon16x16_Down_Arror);   // Down arrow below hour (red bg)
		}else if(selection == 2){
				LCD_Fill(40, 54, 56, 70, WHITE);
				LCD_Fill(40, 104, 56, 120, WHITE);
				GUI_DrawMonoIcon16x16(104, 54, WHITE, RED, Icon16x16_Up_Arror);     // Up arrow above hour (red bg)
				GUI_DrawMonoIcon16x16(104, 104, WHITE, RED, Icon16x16_Down_Arror);   // Down arrow below hour (red bg)
		}else if(selection == 3){
				LCD_Fill(104, 54, 120, 68, WHITE);
				LCD_Fill(104, 104, 120, 120, WHITE);
				Show_Str(127, 108, RED, WHITE, "Save", 16, 0);
		}
			
}

void Draw_Control_Adj_TempProtect_Max_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
		Draw_TopBar(leave_col, edit_col);
	
		// Draw "Temp. Correct" title (left aligned, y=24 - same as Sensor Setting)
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		Show_Str(10, 24, BLACK, WHITE, "Max Temp.Protect", 16, 0);
	
		// Draw horizontal line below "Temp. Limit" title (y=42 to y=43, leave 4px margin on both sides)
		LCD_Fill(4, 42, lcddev.width - 4, 43, BLACK);
		Display_Adj_Number((float)temp_protect_max, BLACK, 0);
		Show_Str(127, 108, BLACK, WHITE, "Save", 16, 0);
		Draw_Control_Adj_TempProtect_Max_Content(selection);
}

void Draw_Control_Adj_TempProtect_Min_Content(uint8_t selection)
{
		
		if(temp_protect_min_switch == 0){
				GUI_DrawMonoIcon32x32(96, 70, BLACK, WHITE, OFF_Icon_32x32);
		}else{
				GUI_DrawMonoIcon32x32(96, 70, RED, WHITE, ON_Icon_32x32);
		}
		
		if(selection == 1){
				Display_Adj_Number((float)temp_protect_min, BLACK, 0);
				GUI_DrawMonoIcon16x16(40, 54, WHITE, RED, Icon16x16_Up_Arror);     // Up arrow above hour (red bg)
				GUI_DrawMonoIcon16x16(40, 104, WHITE, RED, Icon16x16_Down_Arror);   // Down arrow below hour (red bg)
		}else if(selection == 2){
				LCD_Fill(40, 54, 56, 70, WHITE);
				LCD_Fill(40, 104, 56, 120, WHITE);
				GUI_DrawMonoIcon16x16(104, 54, WHITE, RED, Icon16x16_Up_Arror);     // Up arrow above hour (red bg)
				GUI_DrawMonoIcon16x16(104, 104, WHITE, RED, Icon16x16_Down_Arror);   // Down arrow below hour (red bg)
		}else if(selection == 3){
				LCD_Fill(104, 54, 120, 68, WHITE);
				LCD_Fill(104, 104, 120, 120, WHITE);
				Show_Str(127, 108, RED, WHITE, "Save", 16, 0);
		}
			
}

void Draw_Control_Adj_TempProtect_Min_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
		Draw_TopBar(leave_col, edit_col);
	
		// Draw "Temp. Correct" title (left aligned, y=24 - same as Sensor Setting)
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		Show_Str(10, 24, BLACK, WHITE, "Min Temp.Protect", 16, 0);
	
		// Draw horizontal line below "Temp. Limit" title (y=42 to y=43, leave 4px margin on both sides)
		LCD_Fill(4, 42, lcddev.width - 4, 43, BLACK);
		Display_Adj_Number((float)temp_protect_min, BLACK, 0);
		Show_Str(127, 108, BLACK, WHITE, "Save", 16, 0);
		Draw_Control_Adj_TempProtect_Min_Content(selection);
}


void Draw_Control_Adj_Power_On_State_Arrow(uint8_t selection)
{
		switch(selection){
				case 1:
						GUI_DrawMonoIcon16x16(10, 48, WHITE, RED, Icon16x16_Arrow);
						LCD_Fill(10, 68, 26, 84, WHITE);
				break;
						
				case 2:
						GUI_DrawMonoIcon16x16(10, 68, WHITE, RED, Icon16x16_Arrow);
						LCD_Fill(10, 48, 26, 64, WHITE);
						LCD_Fill(10, 88, 26, 104, WHITE);
				break;
				
				case 3:
						GUI_DrawMonoIcon16x16(10, 88, WHITE, RED, Icon16x16_Arrow);
						LCD_Fill(10, 68, 26, 84, WHITE);
				break;
				
				case 4:
						LCD_Fill(10, 88, 26, 104, WHITE);
						Show_Str(127, 108, RED, WHITE, "Save", 16, 0);
				break;
		}
}
void Draw_Control_Adj_Power_On_State_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
		Draw_TopBar(leave_col, edit_col);
	
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		Show_Str(10, 24, BLACK, WHITE, "Power On State", 16, 0);
		// Draw horizontal line below "Temp. Limit" title (y=42 to y=43, leave 4px margin on both sides)
		LCD_Fill(4, 42, lcddev.width - 4, 43, BLACK);
		
		//show 3 stream
		if(power_on_state == 1){
				GUI_DrawMonoIcon16x16(10, 48, WHITE, BLACK, Icon16x16_Arrow);
		}
		Show_Str(32, 48, BLACK, WHITE, "Keep State", 16, 0);
		if(power_on_state == 2){
				GUI_DrawMonoIcon16x16(10, 68, WHITE, BLACK, Icon16x16_Arrow);
		}
		Show_Str(32, 68, BLACK, WHITE, "Device Close", 16, 0);
		if(power_on_state == 3){
				GUI_DrawMonoIcon16x16(10, 88, WHITE, BLACK, Icon16x16_Arrow);
		}
		Show_Str(32, 88, BLACK, WHITE, "Device Open", 16, 0);
		Show_Str(127, 108, BLACK, WHITE, "Save", 16, 0);
		
}
