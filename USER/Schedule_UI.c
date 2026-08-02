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
//File Function: Sub-UI Schedule Edit Relative UI Flow
//		- 128 * 160 RGB(8 bit) LCM
//---------------------------------------------------------------------------------------------------------

#include "Thermostat.h"
#include "UI.h"


// Draw a single row in Heating Schedule Menu page (for optimized update)
// row: 0=Program Type, 1=Workday Setting, 2=Restday Setting
// selected: 1=selected (red bg), 0=not selected
void Draw_Heating_Schedule_Menu_Row(uint8_t row, uint8_t selected)
{
	uint16_t row_y[3];  // Y positions for 3 rows
	uint16_t text_color, bg_color;
	uint8_t row_height;
	if(g_parameter.font_size == 0){
			row_y[0] = 29;
			row_y[1] = 65;
			row_y[2] = 101;
			row_height = 32;
	}else if(g_parameter.font_size == 1){
			row_y[0] = 30;
			row_y[1] = 60;
			row_y[2] = 90;
			row_height = 30;
	}
	row--;
	if(row > 2) return;  // Bounds check
	
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
	
	// Draw the row content based on row number
	if(row == 0) {
		POINT_COLOR = text_color;
		BACK_COLOR = bg_color;
		if(g_parameter.font_size == 0){
				GUI_DrawMonoIcon16x16(5, row_y[row] + (row_height - 16) / 2, text_color, bg_color, Program_Type_16x16);
				lan_str = LanguageTable[STR_Program_Type][g_parameter.language];
				Show_FuncSetting_Row_Text(30, row_y[row], text_color, bg_color, (char*)lan_str, 16);
		}else if(g_parameter.font_size == 1){
				GUI_DrawMonoIcon16x16(5, row_y[row] + 6, text_color, bg_color, Program_Type_16x16);
				lan_str = LanguageTable[STR_Type][g_parameter.language];
				Show_Str(30, row_y[row] + 2, text_color, bg_color, (char*)lan_str, 24, 1);
		}
	} else if(row == 1) {
		POINT_COLOR = text_color;
		BACK_COLOR = bg_color;
		if(g_parameter.font_size == 0){
				GUI_DrawMonoIcon16x16(5, row_y[row] + (row_height - 16) / 2, text_color, bg_color, Workday_Setting_16x16);
				lan_str = LanguageTable[STR_Workday_Setting][g_parameter.language];
				Show_FuncSetting_Row_Text(30, row_y[row], text_color, bg_color, (char*)lan_str, 16);
		}else if(g_parameter.font_size == 1){
				GUI_DrawMonoIcon16x16(5, row_y[row] + 6, text_color, bg_color, Workday_Setting_16x16);
				lan_str = LanguageTable[STR_Workday][g_parameter.language];
				Show_Str(30, row_y[row] + 2, text_color, bg_color, (char*)lan_str, 24, 1);
		}
	} else if(row == 2) {
		POINT_COLOR = text_color;
		BACK_COLOR = bg_color;
		if(g_parameter.font_size == 0){
				GUI_DrawMonoIcon16x16(5, row_y[row] + (row_height - 16) / 2, text_color, bg_color, Restday_Setting_16x16);
				lan_str = LanguageTable[STR_Restday_Setting][g_parameter.language];
				Show_FuncSetting_Row_Text(30, row_y[row], text_color, bg_color, (char*)lan_str, 16);
		}else if(g_parameter.font_size == 1){
				GUI_DrawMonoIcon16x16(5, row_y[row] + 6, text_color, bg_color, Restday_Setting_16x16);
				lan_str = LanguageTable[STR_Restday][g_parameter.language];
				Show_Str(30, row_y[row] + 2, text_color, bg_color, (char*)lan_str, 24, 1);
		}
	}
}
void Draw_Heating_Schedule_Menu_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
		uint8_t i;
	
		Draw_TopBar(leave_col, edit_col);
	
		LCD_Fill(4, 24, lcddev.width - 4, 25, BLACK);
	
		// Draw 3 rows (Program Type, Workday Setting, Restday Setting)
		for (i = 1; i < 4; i++) {
				if(selection == i){
						Draw_Heating_Schedule_Menu_Row(i, 1);		//selected
				}else{
						Draw_Heating_Schedule_Menu_Row(i, 0);  // Not selected	
				}
		}
}

void LCD_FillCircle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color)
{
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	LCD_Fill(x0, y0 - r, x0, y0 + r, color);
	LCD_Fill(x0 - r, y0, x0 + r, y0, color);

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;
		LCD_Fill(x0 - x, y0 + y, x0 + x, y0 + y, color);
		LCD_Fill(x0 - x, y0 - y, x0 + x, y0 - y, color);
		LCD_Fill(x0 - y, y0 + x, x0 + y, y0 + x, color);
		LCD_Fill(x0 - y, y0 - x, x0 + y, y0 - x, color);
	}
}

char* prog_type_strs[] = {"5+2", "6+1", "7"};
void Draw_Heating_Schedule_Prog_Type_Content(uint8_t selected)
{
	uint8_t i;
	uint8_t row_height = 24;
	uint16_t row_y[] = {50, 78, 106};  // Y positions for 3 options (5+2, 6+1, 7)
	uint16_t sel_bg_color;

	// Clear content area (below separator to bottom)
	LCD_Fill(0, 44, lcddev.width, lcddev.height, WHITE);

	// TopBar mode: selected=BLACK; Edit mode: selected=RED (Save mode: BLACK)
	if(Top_Bar_Active || item_selection == 2) {
		sel_bg_color = BLACK;
	} else {
		sel_bg_color = RED;
	}

	for(i = 0; i < 3; i++) {
		uint16_t text_color, bg_color;

		if(selected == i) {
			text_color = WHITE;
			bg_color = sel_bg_color;
			// Draw background for selected item
			LCD_Fill(0, row_y[i], lcddev.width, row_y[i] + row_height, sel_bg_color);
		} else {
			text_color = BLACK;
			bg_color = WHITE;
			// Clear background to white
			LCD_Fill(0, row_y[i], lcddev.width, row_y[i] + row_height, WHITE);
		}

		// Draw Arrow icon (vertically centered)
		GUI_DrawMonoIcon16x16(12, row_y[i] + (row_height - 16) / 2, WHITE, bg_color, Icon16x16_Arrow);

		// Draw program type text
		POINT_COLOR = text_color;
		BACK_COLOR = bg_color;
		Show_Str(34, row_y[i] + (row_height - 16) / 2, text_color, bg_color, prog_type_strs[i], 16, 0);
	}

	// Draw Save button (always visible when not in TopBar mode)
	if(!Top_Bar_Active) {
		uint16_t save_color = (item_selection == 2) ? RED : BLACK;
		lan_str = LanguageTable[STR_Save][g_parameter.language];
		Show_Str(86, 135, save_color, WHITE, (char*)lan_str, 16, 0);
	}
}
void Draw_Heating_Schedule_Prog_Type_Page(uint8_t selected, uint8_t leave_col, uint8_t edit_col)
{
	Draw_TopBar(leave_col, edit_col);

	// Draw "Program Type" title
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	lan_str = LanguageTable[STR_Program_Type][g_parameter.language];
	Show_Str(10, 25, BLACK, WHITE, (char*)lan_str, 16, 0);

	// Draw horizontal separator line below title
	LCD_Fill(4, 42, lcddev.width - 4, 43, BLACK);

	Draw_Heating_Schedule_Prog_Type_Content(current_prog_type);
}


// Draw Finish button in Schedule Edit page (at row 3 position, y=102)
// selected: 1=selected (red), 0=not selected
void Draw_Schedule_Edit_Finish(uint8_t selected)
{
	uint16_t finish_color = selected ? RED : BLACK;
	uint16_t row_y = 121;  // Same y position as row 3
	uint8_t row_height = 32;
	// Clear Finish area
	LCD_Fill(0, row_y, lcddev.width, row_y + row_height, WHITE);
	POINT_COLOR = finish_color;
	BACK_COLOR = WHITE;
	lan_str =LanguageTable[STR_Finish][g_parameter.language];
	Show_Str(68, row_y + (row_height - 16) / 2, finish_color, WHITE, (char*)lan_str, 16, 0);
}


// Draw a single row in Schedule Edit page (for optimized update)
// row: 0-3 (display row position)
// period_idx: 0-5 (actual period index P1-P6)
// selected: 1=selected (red arrow bg), 0=not selected
void Draw_Schedule_Edit_Row(uint8_t row, uint8_t period_idx, uint8_t selected)
{
	uint16_t row_y[] = {25, 57, 89, 121};  // Y positions for 4 display rows (32px each)
	uint8_t row_height = 32;
	uint16_t text_color, bg_color;
	char time_str[6];
	char temp_str[4];
	char period_name[3];

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

	// Draw vertical separator line (shorter, ~half row height)
	LCD_Fill(26, row_y[row] + 8, 27, row_y[row] + row_height - 8, BLACK);

	// Draw Period name (P1-P6)
	sprintf(period_name, "P%d", period_idx + 1);
	POINT_COLOR = text_color;
	BACK_COLOR = bg_color;
	Show_Str(6, row_y[row] + (row_height - 16) / 2, text_color, bg_color, period_name, 16, 0);

	// Get time and temperature from schedule_settings (Workday or Restday)
	{
		uint8_t (*sched_ptr)[4] = schedule_edit_source ? g_parameter.holiday_schedule : g_parameter.workday_schedule;
		sprintf(time_str, "%02d:%02d", sched_ptr[period_idx][0], sched_ptr[period_idx][1]);
		sprintf(temp_str, "%d", sched_ptr[period_idx][2]);

	// Draw Time
	POINT_COLOR = text_color;
	BACK_COLOR = bg_color;
	Show_Str(32, row_y[row] + (row_height - 16) / 2, text_color, bg_color, time_str, 16, 0);

	// Draw Temperature (closer to time)
	Show_Str(78, row_y[row] + (row_height - 16) / 2, text_color, bg_color, temp_str, 16, 0);

	// Draw ON/OFF indicator (replaces C icon)
	if(sched_ptr[period_idx][3] == 0) {
		GUI_DrawMonoIcon16x16(100, row_y[row] + (row_height - 16) / 2, text_color, bg_color, OFF_Icon_16x16);
	} else {
		// ON indicator: 16x16 box + filled circle
		//POINT_COLOR = text_color;
		//BACK_COLOR = bg_color;
		//LCD_DrawRectangle(100, row_y[row] + (row_height - 16) / 2, 115, row_y[row] + (row_height - 16) / 2 + 15);
		//LCD_FillCircle(108, row_y[row] + (row_height - 16) / 2 + 8, 5, text_color);
		GUI_DrawMonoIcon16x16(100, row_y[row] + (row_height - 16) / 2, text_color, bg_color, ON_Icon_16x16);
	}
	}  // end of sched_ptr block
}



// Draw Schedule Edit Page with scrolling (P1-P6)
// This page shows:
// - Top-left: Leave Icon (black/red)
// - Top-right: Edit Icon (black/red)
// - 4 rows showing P1-P4, P2-P5, or P3-P6 based on scroll position
// - Finish button at bottom
// selection: 0-5=P1-P6, 6=Finish, 0xFF=TopBar Edit red, 0xFE=TopBar Leave red
void Draw_Schedule_Edit_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	uint8_t i;

	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	Draw_TopBar(leave_col, edit_col);

	// Draw rows based on scroll position
	// scroll=0: show P1-P4, scroll=1: show P2-P5, scroll=2: show P3-P6/Finish
	for(i = 0; i < 4; i++) {
		uint8_t period_idx = schedule_edit_scroll + i;  // 0-5
		uint8_t is_selected = 0;
		
		// Check if this is the Finish position (when period_idx > 5)
		if(period_idx > 5) {
			// This is the Finish row position
			if(!Top_Bar_Active && (selection == 7)) {
				Draw_Schedule_Edit_Finish(1);  // Finish selected
			} else {
				Draw_Schedule_Edit_Finish(0);  // Finish not selected
			}
		} else {
			// Regular period row
			if(!Top_Bar_Active && (period_idx == (selection - 1))) {
				is_selected = 1;
			}
			Draw_Schedule_Edit_Row(i, period_idx, is_selected);
		}
	}
}


// Draw Schedule Time Setting Page (vertical layout for portrait mode)
// This page shows:
// - Top-left: Leave Icon (black/red)
// - Title: "P1 ON" / "P1 OFF"
// - Time (HH:MM) - selected field turns RED
// - Temperature with degC - selected field turns RED
// - ON / OFF toggle - selected state turns RED
// - Save button
// period_num: 1~6 (P1~P6)
void Draw_Schedule_Time_Setting_Page(uint8_t period_num, uint8_t leave_col, uint8_t edit_col)
{
	uint16_t save_color;
	uint8_t hour_high, hour_low, min_high, min_low;
	uint8_t temp_high, temp_low;
	char period_text[10];
	uint16_t hour_color, min_color, temp_color;
	uint16_t on_color, off_color;

	// Calculate digits from edit values
	hour_high = schedule_time_edit_hour / 10;
	hour_low = schedule_time_edit_hour % 10;
	min_high = schedule_time_edit_min / 10;
	min_low = schedule_time_edit_min % 10;
	temp_high = schedule_time_edit_temp / 10;
	temp_low = schedule_time_edit_temp % 10;

	// Set colors based on current focus (red when selected)
	if(!Top_Bar_Active) {
		hour_color = (item_selection == 1) ? RED : BLACK;
		min_color = (item_selection == 2) ? RED : BLACK;
		temp_color = (item_selection == 3) ? RED : BLACK;
		on_color = (item_selection == 4) ? RED : BLACK;
		off_color = (item_selection == 5) ? RED : BLACK;
	} else {
		hour_color = BLACK;
		min_color = BLACK;
		temp_color = BLACK;
		on_color = BLACK;
		off_color = BLACK;
	}

	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	Draw_TopBar(leave_col, edit_col);

	// Draw title centered between Leave/Edit icons
	sprintf(period_text, "P%d %s", period_num, schedule_time_on_off ? "ON" : "OFF");
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	Show_Str(44, 5, BLACK, WHITE, period_text, 16, 0);

	// Draw horizontal separator line (moved up to y=30)
	LCD_Fill(4, 30, lcddev.width - 4, 31, BLACK);

	// ===== Time: HH:MM centered (16x32, y=36~68) =====
	GUI_DrawBigDigit(30, 36, hour_color, WHITE, '0' + hour_high, 1);
	GUI_DrawBigDigit(46, 36, hour_color, WHITE, '0' + hour_low, 1);
	LCD_Fill(63, 51, 65, 53, (item_selection == 1 || item_selection == 2) ? RED : BLACK);
	LCD_Fill(63, 57, 65, 59, (item_selection == 1 || item_selection == 2) ? RED : BLACK);
	GUI_DrawBigDigit(70, 36, min_color, WHITE, '0' + min_high, 1);
	GUI_DrawBigDigit(86, 36, min_color, WHITE, '0' + min_low, 1);

	// ===== Temperature: ## degC centered + shifted right 8px (y=76~108) =====
	GUI_DrawBigDigit(48, 76, temp_color, WHITE, '0' + temp_high, 1);
	GUI_DrawBigDigit(64, 76, temp_color, WHITE, '0' + temp_low, 1);
	GUI_DrawMonoIcon16x16(80, 76, temp_color, WHITE, Icon16x16_C);

	// ===== ON/OFF: 12x24 font (y=116~140) =====
	POINT_COLOR = on_color;
	BACK_COLOR = WHITE;
	lan_str = LanguageTable[STR_ON][g_parameter.language];
	Show_Str(30, 116, on_color, WHITE, (char*)lan_str, 24, 1);
	POINT_COLOR = off_color;
	lan_str = LanguageTable[STR_OFF][g_parameter.language];
	Show_Str(70, 116, off_color, WHITE, (char*)lan_str, 24, 1);

	// ===== Save button (y=142~158) =====
	save_color = (!Top_Bar_Active && item_selection == 5) ? RED : BLACK;
	POINT_COLOR = save_color;
	BACK_COLOR = WHITE;
	lan_str = LanguageTable[STR_Save][g_parameter.language];
	Show_Str(86, 142, save_color, WHITE, (char*)lan_str, 16, 0);
}

// Clear arrow area (for switching between edit modes)
// Clears all three arrow positions (hour, minute, temperature)
void Draw_Schedule_Time_Setting_ClearArrows(void)
{
	// Clear hour arrow area (x=13, y=30~98)
	LCD_Fill(13, 30, 13 + 16, 30 + 16, WHITE);   // Up arrow
	LCD_Fill(13, 82, 13 + 16, 82 + 16, WHITE);   // Down arrow
	
	// Clear minute arrow area (x=53, y=30~98)
	LCD_Fill(53, 30, 53 + 16, 30 + 16, WHITE);   // Up arrow
	LCD_Fill(53, 82, 53 + 16, 82 + 16, WHITE);   // Down arrow
	
	// Clear temperature arrow area (x=96, y=30~98)
	LCD_Fill(96, 30, 96 + 16, 30 + 16, WHITE);   // Up arrow
	LCD_Fill(96, 82, 96 + 16, 82 + 16, WHITE);   // Down arrow
}


// Draw hour edit arrows only (optimized update)
void Draw_Schedule_Time_Setting_Arrows(uint8_t show)
{
	Draw_Schedule_Time_Setting_ClearArrows();
	switch (show){
		case Hour:
				GUI_DrawMonoIcon16x16(13, 30, WHITE, RED, Icon16x16_Up_Arror);     // Up arrow above hour (red bg)
				GUI_DrawMonoIcon16x16(13, 82, WHITE, RED, Icon16x16_Down_Arror);   // Down arrow below hour (red bg)
		break;
		
		case Minume:
				GUI_DrawMonoIcon16x16(53, 30, WHITE, RED, Icon16x16_Up_Arror);     // Up arrow above minute (red bg)
				GUI_DrawMonoIcon16x16(53, 82, WHITE, RED, Icon16x16_Down_Arror);   // Down arrow below minute (red bg)
		break;
		
		case Temperature:
				GUI_DrawMonoIcon16x16(96, 30, WHITE, RED, Icon16x16_Up_Arror);     // Up arrow above temperature (red bg)
				GUI_DrawMonoIcon16x16(96, 82, WHITE, RED, Icon16x16_Down_Arror);   // Down arrow below temperature (red bg)
		break;
		
		case Time_Clear:
			
		break;
	}
	
}

void Clear_Schedule_Time_Setting_Arror(uint8_t clr)
{
		switch(clr){
			case Hour:
					LCD_Fill(13, 30, 29, 46, WHITE);
					LCD_Fill(13, 82, 29, 98, WHITE);
			break;
			
			case Minume:
					LCD_Fill(53, 30, 69, 46, WHITE);
					LCD_Fill(53, 82, 69, 98, WHITE);
			break;
			
			case Temperature:
					LCD_Fill(96, 30, 112, 46, WHITE);
					LCD_Fill(96, 82, 112, 98, WHITE);
			break;
			
		}
}

// Draw minute edit arrows only (optimized update)
void Draw_Schedule_Time_Setting_MinArrows(uint8_t show)
{
	if(show) {
		GUI_DrawMonoIcon16x16(53, 30, WHITE, RED, Icon16x16_Up_Arror);     // Up arrow above minute (red bg)
		GUI_DrawMonoIcon16x16(53, 82, WHITE, RED, Icon16x16_Down_Arror);   // Down arrow below minute (red bg)
	}
}

// Draw temperature edit arrows only (optimized update)
void Draw_Schedule_Time_Setting_TempArrows(uint8_t show)
{
	if(show) {
		GUI_DrawMonoIcon16x16(96, 30, WHITE, RED, Icon16x16_Up_Arror);     // Up arrow above temperature (red bg)
		GUI_DrawMonoIcon16x16(96, 82, WHITE, RED, Icon16x16_Down_Arror);   // Down arrow below temperature (red bg)
	}
}


// Draw time digits only (optimized update)
void Draw_Schedule_Time_Setting_TimeDigits(void)
{
	uint8_t hour_high, hour_low, min_high, min_low;
	uint16_t hour_color, min_color;

	hour_high = schedule_time_edit_hour / 10;
	hour_low = schedule_time_edit_hour % 10;
	min_high = schedule_time_edit_min / 10;
	min_low = schedule_time_edit_min % 10;
	hour_color = (item_selection == 1) ? RED : BLACK;
	min_color = (item_selection == 2) ? RED : BLACK;

	// Clear time area (x=28~88, y=36~68)
	LCD_Fill(28, 36, 88, 68, WHITE);

	// Draw Time (16x32 font, centered)
	GUI_DrawBigDigit(30, 36, hour_color, WHITE, '0' + hour_high, 1);
	GUI_DrawBigDigit(46, 36, hour_color, WHITE, '0' + hour_low, 1);
	LCD_Fill(63, 51, 65, 53, (item_selection == 1 || item_selection == 2) ? RED : BLACK);
	LCD_Fill(63, 57, 65, 59, (item_selection == 1 || item_selection == 2) ? RED : BLACK);
	GUI_DrawBigDigit(70, 36, min_color, WHITE, '0' + min_high, 1);
	GUI_DrawBigDigit(86, 36, min_color, WHITE, '0' + min_low, 1);
}

// Draw temperature digits only (optimized update)
void Draw_Schedule_Time_Setting_TempDigits(void)
{
	uint8_t temp_high, temp_low;
	uint16_t temp_color;

	temp_high = schedule_time_edit_temp / 10;
	temp_low = schedule_time_edit_temp % 10;
	temp_color = (item_selection == 3) ? RED : BLACK;

	// Clear temperature area (x=46~98, y=76~108)
	LCD_Fill(46, 76, 98, 108, WHITE);

	// Draw Temperature (16x32 font, shifted right 8px)
	GUI_DrawBigDigit(48, 76, temp_color, WHITE, '0' + temp_high, 1);
	GUI_DrawBigDigit(64, 76, temp_color, WHITE, '0' + temp_low, 1);
	GUI_DrawMonoIcon16x16(80, 76, temp_color, WHITE, Icon16x16_C);
}

// Draw ON/OFF section only (optimized update)
void Draw_Schedule_Time_Setting_OnOff(void)
{
	uint16_t on_color, off_color;

	// In ON/OFF mode (item_selection=4): selected state in red
	if(item_selection == 4) {
		on_color = schedule_time_on_off ? RED : BLACK;
		off_color = schedule_time_on_off ? BLACK : RED;
	} else {
		on_color = BLACK;
		off_color = BLACK;
	}

	LCD_Fill(28, 116, 118, 140, WHITE);

	POINT_COLOR = on_color;
	BACK_COLOR = WHITE;
	lan_str = LanguageTable[STR_ON][g_parameter.language];
	Show_Str(30, 116, on_color, WHITE, (char*)lan_str, 24, 1);

	POINT_COLOR = off_color;
	lan_str = LanguageTable[STR_OFF][g_parameter.language];
	Show_Str(70, 116, off_color, WHITE, (char*)lan_str, 24, 1);
}

// Draw Save button only (optimized update)
void Draw_Schedule_Time_Setting_Save(void)
{
	uint16_t save_color;

	save_color = (item_selection == 5) ? RED : BLACK;

	// Clear Save area (x=84~110, y=142~158)
	LCD_Fill(84, 142, 110, 158, WHITE);

	// Draw "Save"
	POINT_COLOR = save_color;
	BACK_COLOR = WHITE;
	lan_str = LanguageTable[STR_Save][g_parameter.language];
	Show_Str(86, 142, save_color, WHITE, (char*)lan_str, 16, 0);
}

// Child Lock PIN Code Functions

void Clear_Pin_Input(void)
{
	uint8_t i;
	for(i = 0; i < 4; i++){
		pin_code_input[i] = 0xFF;  // 0xFF means not entered yet
		pin_code_confirm[i] = 0xFF;
	}
}

// Draw Leave Schedule Mode Confirm dialog (80x120 centered box)
// selection: 0=Cancel (left), 1=Yes (right)
void Draw_Leave_Schedule_Confirm_Page(uint8_t selection)
{
	uint16_t box_x = 9;   // (128-110)/2 = 9
	uint16_t box_y = 40;
	uint16_t box_w = 110;
	uint16_t box_h = 80;

	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);

	// Draw box border (RED)
	POINT_COLOR = RED;
	LCD_DrawRectangle(box_x, box_y, box_x + box_w, box_y + box_h);

	// Draw title: "Leave" / "Schedule Mode"
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	lan_str = LanguageTable[STR_Leave_Schedule][g_parameter.language];
	Show_Str(box_x + 6, box_y + 10, BLACK, WHITE, (char*)lan_str, 16, 0);
	lan_str = LanguageTable[STR_Schedule_Mode][g_parameter.language];
	Show_Str(box_x + 6, box_y + 28, BLACK, WHITE, (char*)lan_str, 16, 0);

	// Draw Cancel (left): selection=0 -> RED, selection=1 -> BLACK
	lan_str = LanguageTable[STR_Cancel][g_parameter.language];
	if(selection == 0){
		Show_Str(box_x + 10, box_y + box_h - 22, RED, WHITE, (char*)lan_str, 16, 0);
	}else{
		Show_Str(box_x + 10, box_y + box_h - 22, BLACK, WHITE, (char*)lan_str, 16, 0);
	}

	// Draw Yes (right): selection=1 -> RED, selection=0 -> BLACK
	lan_str = LanguageTable[STR_Yes][g_parameter.language];
	if(selection == 1){
		Show_Str(box_x + box_w - 28, box_y + box_h - 22, RED, WHITE, (char*)lan_str, 16, 0);
	}else{
		Show_Str(box_x + box_w - 28, box_y + box_h - 22, BLACK, WHITE, (char*)lan_str, 16, 0);
	}
}

void Draw_Window_Open_Confirm_Page(uint8_t selection)
{
	uint16_t box_x = 9;   // (128-110)/2 = 9
	uint16_t box_y = 35;
	uint16_t box_w = 110;
	uint16_t box_h = 85;

	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);

	POINT_COLOR = RED;
	LCD_DrawRectangle(box_x, box_y, box_x + box_w, box_y + box_h);

	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	lan_str = LanguageTable[STR_Window_Open][g_parameter.language];
	Show_Str(box_x + 6, box_y + 10, BLACK, WHITE, (char*)lan_str, 16, 0);
	lan_str = LanguageTable[STR_Detected][g_parameter.language];
	Show_Str(box_x + 6, box_y + 28, BLACK, WHITE, (char*)lan_str, 16, 0);

	lan_str = LanguageTable[STR_Cancel_Detect][g_parameter.language];
	if(selection == 0){
		Show_Str(box_x + 10, box_y + box_h - 36, RED, WHITE, (char*)lan_str, 16, 0);
	}else{
		Show_Str(box_x + 10, box_y + box_h - 36, BLACK, WHITE, (char*)lan_str, 16, 0);
	}
	lan_str = LanguageTable[STR_Reset_Detect][g_parameter.language];
	if(selection == 1){
		Show_Str(box_x + 10, box_y + box_h - 20, RED, WHITE, (char*)lan_str, 16, 0);
	}else{
		Show_Str(box_x + 10, box_y + box_h - 20, BLACK, WHITE, (char*)lan_str, 16, 0);
	}
}
