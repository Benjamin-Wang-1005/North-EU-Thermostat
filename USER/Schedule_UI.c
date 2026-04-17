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
	uint16_t row_y[] = {30, 54, 78};  // Y positions for 3 rows
	uint16_t text_color, bg_color;
	row--;
	if(row > 2) return;  // Bounds check
	
	if(selected) {
		text_color = WHITE;
		bg_color = RED;
		// Draw red background
		LCD_Fill(0, row_y[row], lcddev.width, row_y[row] + 24, RED);
	} else {
		text_color = BLACK;
		bg_color = WHITE;
		// Clear background to white
		LCD_Fill(0, row_y[row], lcddev.width, row_y[row] + 24, WHITE);
	}
	
	// Draw the row content based on row number
	if(row == 0) {
		// Program Type - uses Icon16
		GUI_DrawMonoIcon8x16(12, row_y[row] + 4, text_color, bg_color, Icon16_8x16);
		POINT_COLOR = text_color;
		BACK_COLOR = bg_color;
		Show_Str(30, row_y[row] + 4, text_color, bg_color, "Program Type", 16, 0);
	} else if(row == 1) {
		// Workday Setting - uses Icon17
		GUI_DrawMonoIcon8x16(12, row_y[row] + 4, text_color, bg_color, Icon17_8x16);
		POINT_COLOR = text_color;
		BACK_COLOR = bg_color;
		Show_Str(30, row_y[row] + 4, text_color, bg_color, "Workday Setting", 16, 0);
	} else if(row == 2) {
		// Restday Setting - uses Icon18
		GUI_DrawMonoIcon8x16(12, row_y[row] + 4, text_color, bg_color, Icon18_8x16);
		POINT_COLOR = text_color;
		BACK_COLOR = bg_color;
		Show_Str(30, row_y[row] + 4, text_color, bg_color, "Restday Setting", 16, 0);
	}
}

// Draw Heating Schedule Menu Page
// This page shows:
// - Top-left: Leave Icon (black/red)
// - Top-right: Edit Icon (black/red)
// - Row 1: Icon16 + "Program Type" (selected=white on red, normal=black)
// - Row 2: Icon17 + "Workday Setting" (selected=white on red, normal=black)
// - Row 3: Icon18 + "Restday Setting" (selected=white on red, normal=black)
// selection: 0=Program Type, 1=Workday Setting, 2=Restday Setting, 0xFF=TopBar Edit red, 0xFE=TopBar Leave red
void Draw_Heating_Schedule_Menu_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
		uint8_t i;
	
		Draw_TopBar(leave_col, edit_col);
	
	
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
	uint16_t x_pos[] = {20, 70, 120};
	
	// Clear this choice area (enlarged to cover pill shape completely)
	// Pill shape: radius 10, center at y=60, so y range is 50-70
	// Add extra margin to ensure complete clearing
	LCD_Fill(0, 50, 160, 70, WHITE);
	// Draw Choices (5+2, 6+1, 7)
	for (i = 0; i < 3; i++) {
		uint16_t text_color = BLACK;
		uint16_t bg_color = WHITE;
		uint16_t box_width = 24; // Force same pill width for all options
		uint16_t text_w = (i == 2) ? 8 : 24; // text "7" is 8px wide, others are 24px
		uint16_t text_x = x_pos[i] + (box_width - text_w) / 2; // Center text in pill

		
		// If editing and this is the selected item, draw Red Pill Shape
		if ((Top_Bar_Active == 0) &&  selected == i) {
			uint16_t y_center = 60;   // Centered with text at y=52 (16px font ends at 68)
			uint8_t r = 10;           // Radius 10 pixels
			uint16_t px1 = x_pos[i] - 1;           // Left circle center
			uint16_t px2 = x_pos[i] + box_width + 1; // Right circle center

			text_color = WHITE;
			bg_color = RED;
			
			// Draw left and right filled circles for perfect round ends
			LCD_FillCircle(px1, y_center, r, RED);
			LCD_FillCircle(px2, y_center, r, RED);
			
			// Fill central rectangle connecting the circles
			LCD_Fill(px1, y_center - r, px2, y_center + r, RED);
		}

		POINT_COLOR = text_color; BACK_COLOR = bg_color;
		// Instead of drawing the text directly with bg_color, FMD Show_Str might just draw the text size box.
		// So we just draw it centered!
		Show_Str(text_x, 52, text_color, bg_color, prog_type_strs[i], 16, 0);
	}
	
}
void Draw_Heating_Schedule_Prog_Type_Page(uint8_t selected, uint8_t leave_col, uint8_t edit_col)
{
	//int i;
	
	uint16_t save_color;

	Draw_TopBar(leave_col, edit_col);
	

	// Draw Current Type Text (Size 16, Moved up)
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	Show_Str(10, 25, BLACK, WHITE, "Current Type: ", 16, 0);
	Show_Str(122, 25, BLUE, WHITE, prog_type_strs[current_prog_type], 16, 0);

	Draw_Heating_Schedule_Prog_Type_Content(selected);

	// Draw Save text/button
	save_color = (Top_Bar_Active == 2) ? RED : BLACK;
	Show_Str(115, 105, save_color, WHITE, "Save", 16, 0);
}


// Draw Finish button in Schedule Edit page (at row 3 position, y=102)
// selected: 1=selected (red), 0=not selected
void Draw_Schedule_Edit_Finish(uint8_t selected)
{
	uint16_t finish_color = selected ? RED : BLACK;
	uint16_t row_y = 102;  // Same y position as row 3
	// Clear Finish area (same height as a regular row)
	LCD_Fill(0, row_y, lcddev.width, row_y + 24, WHITE);
	POINT_COLOR = finish_color;
	BACK_COLOR = WHITE;
	Show_Str(100, row_y + 4, finish_color, WHITE, "Finish", 16, 0);
}


// Draw a single row in Schedule Edit page (for optimized update)
// row: 0-3 (display row position)
// period_idx: 0-5 (actual period index P1-P6)
// selected: 1=selected (red arrow bg), 0=not selected
void Draw_Schedule_Edit_Row(uint8_t row, uint8_t period_idx, uint8_t selected)
{
	uint16_t row_y[] = {30, 54, 78, 102};  // Y positions for 4 display rows
	uint16_t arrow_icon_color, arrow_bg_color, text_color;
	char time_str[6];
	char temp_str[3];
	char period_name[3];
	
	if(selected) {
		arrow_icon_color = WHITE;
		arrow_bg_color = RED;
		text_color = BLACK;
	} else {
		arrow_icon_color = WHITE;
		arrow_bg_color = BLACK;
		text_color = BLACK;
	}
	
	// Clear this row area
	LCD_Fill(0, row_y[row], lcddev.width, row_y[row] + 24, WHITE);
	
	// Draw Period name (P1-P6)
	sprintf(period_name, "P%d", period_idx + 1);
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	Show_Str(10, row_y[row], BLACK, WHITE, period_name, 16, 0);
	
	// Draw Arrow Icon
	GUI_DrawMonoIcon16x16(35, row_y[row], arrow_icon_color, arrow_bg_color, Icon16x16_Arrow);
	
	// Get time and temperature from schedule_settings (Workday or Restday)
	{
		uint8_t (*sched_ptr)[4] = schedule_edit_source ? schedule_settings_restday : schedule_settings;
		sprintf(time_str, "%02d:%02d", sched_ptr[period_idx][0], sched_ptr[period_idx][1]);
		sprintf(temp_str, "%d", sched_ptr[period_idx][2]);
	
	// Draw Time
	POINT_COLOR = text_color;
	BACK_COLOR = WHITE;
	Show_Str(55, row_y[row], text_color, WHITE, time_str, 16, 0);
	
	// Draw Temperature
	Show_Str(100, row_y[row], text_color, WHITE, temp_str, 16, 0);
	
	// Draw C Icon
	GUI_DrawMonoIcon16x16(116, row_y[row], BLACK, WHITE, Icon16x16_C);
	
	// Draw Hook or X Icon based on ON/OFF
	if(sched_ptr[period_idx][3] == 0) {
		GUI_DrawMonoIcon16x16(135, row_y[row], BLACK, WHITE, Icon16x16_X);
	} else {
		GUI_DrawMonoIcon16x16(135, row_y[row], BLACK, WHITE, Icon16x16_Hook);
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
	//uint8_t edit_red = 0;
	//uint8_t leave_red = 0;
	
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


// Draw Schedule Time Setting Page
// This page shows:
// - Top-left: Leave Icon (black/red)
// - Top-center: "Period X" text (X = 1~6)
// - Top-right: Edit Icon (black/red)
// - Middle: Time | Temperature | ON/OFF
// - Bottom: "Save" text
// period_num: 1~6 (P1~P6)
void Draw_Schedule_Time_Setting_Page(uint8_t period_num, uint8_t leave_col, uint8_t edit_col)
{
	uint16_t save_color;
	uint8_t hour_high, hour_low, min_high, min_low;
	uint8_t temp_high, temp_low;
	char period_text[10];
	
	// Calculate digits from edit values
	hour_high = schedule_time_edit_hour / 10;
	hour_low = schedule_time_edit_hour % 10;
	min_high = schedule_time_edit_min / 10;
	min_low = schedule_time_edit_min % 10;
	temp_high = schedule_time_edit_temp / 10;
	temp_low = schedule_time_edit_temp % 10;
	
	Draw_TopBar(leave_col, edit_col);
	
	if(schedule_time_on_off) {
		sprintf(period_text, "Period %d On", period_num);
	} else {
		sprintf(period_text, "Period %d Off", period_num);
	}
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	Show_Str(35, 5, BLACK, WHITE, period_text, 16, 0);
	
	
	// Draw Time section with 16x32 large font
	// Position: left aligned (x=5), y=50
	GUI_DrawBigDigit(5, 50, BLACK, WHITE, '0' + hour_high, 1);   // Hour high digit
	GUI_DrawBigDigit(21, 50, BLACK, WHITE, '0' + hour_low, 1);   // Hour low digit
	// Draw colon
	LCD_Fill(38, 65, 40, 67, BLACK);
	LCD_Fill(38, 71, 40, 73, BLACK);
	GUI_DrawBigDigit(45, 50, BLACK, WHITE, '0' + min_high, 1);   // Minute high digit
	GUI_DrawBigDigit(61, 50, BLACK, WHITE, '0' + min_low, 1);    // Minute low digit
	
	// Draw Up/Down Arrow Icons for hour edit (substate 1)
	// Position: centered above/below the two hour digits (x=5~21, center=13)
	if(item_selection == 1) {
		GUI_DrawMonoIcon16x16(13, 30, WHITE, RED, Icon16x16_Up_Arror);     // Up arrow above hour (red bg)
		GUI_DrawMonoIcon16x16(13, 82, WHITE, RED, Icon16x16_Down_Arror);   // Down arrow below hour (red bg)
	}
	
	// Draw Up/Down Arrow Icons for minute edit (substate 2)
	// Position: centered above/below the two minute digits (x=45~61, center=53)
	if(item_selection == 2) {
		GUI_DrawMonoIcon16x16(53, 30, WHITE, RED, Icon16x16_Up_Arror);     // Up arrow above minute (red bg)
		GUI_DrawMonoIcon16x16(53, 82, WHITE, RED, Icon16x16_Down_Arror);   // Down arrow below minute (red bg)
	}
	
	// Draw separator "|" between time and temperature (small font)
	Show_Str(80, 60, BLACK, WHITE, "|", 16, 0);
	
	// Draw Temperature with 16x32 font
	GUI_DrawBigDigit(88, 50, BLACK, WHITE, '0' + temp_high, 1);
	GUI_DrawBigDigit(104, 50, BLACK, WHITE, '0' + temp_low, 1);
	
	// Draw Up/Down Arrow Icons for temperature edit (substate 3)
	// Position: centered above/below the two temperature digits (x=88~104, center=96)
	if(item_selection == 3) {
		GUI_DrawMonoIcon16x16(96, 30, WHITE, RED, Icon16x16_Up_Arror);     // Up arrow above temperature (red bg)
		GUI_DrawMonoIcon16x16(96, 82, WHITE, RED, Icon16x16_Down_Arror);   // Down arrow below temperature (red bg)
	}
	
	// Draw degree C (small icon at top-right of temperature)
	GUI_DrawMonoIcon16x16(121, 45, BLACK, WHITE, Icon16x16_C);
	
	// Draw ON/OFF text below degree C
	// ON at y=63, OFF at y=80
	// Default: both black when not in ON/OFF edit mode
	if(item_selection == 4) {
		// ON/OFF edit mode - show selected one in red - shifted right by 2 pixels
		if(schedule_time_on_off) {
			// ON selected - ON red, OFF black
			Show_Str(123, 63, RED, WHITE, "ON", 16, 0);
			Show_Str(123, 80, BLACK, WHITE, "OFF", 16, 0);
			// Left Arrow Icon next to ON (y=63, aligned with ON) - shifted right by 2 pixels
			GUI_DrawMonoIcon8x16(147, 63, RED, WHITE, IconLeft_Arrow_8x16);
		} else {
			// OFF selected - ON black, OFF red
			Show_Str(123, 63, BLACK, WHITE, "ON", 16, 0);
			Show_Str(123, 80, RED, WHITE, "OFF", 16, 0);
			// Left Arrow Icon next to OFF (y=80, aligned with OFF) - shifted right by 2 pixels
			GUI_DrawMonoIcon8x16(147, 80, RED, WHITE, IconLeft_Arrow_8x16);
		}
	} else {
		// Not in ON/OFF edit mode - both black - shifted right by 2 pixels
		Show_Str(123, 63, BLACK, WHITE, "ON", 16, 0);
		Show_Str(123, 80, BLACK, WHITE, "OFF", 16, 0);
		// Left Arrow Icon stays at the last selected position (ON or OFF) - shifted right by 2 pixels
		if(schedule_time_on_off) {
			GUI_DrawMonoIcon8x16(147, 63, RED, WHITE, IconLeft_Arrow_8x16);  // Next to ON
		} else {
			GUI_DrawMonoIcon8x16(147, 80, RED, WHITE, IconLeft_Arrow_8x16);  // Next to OFF
		}
	}
	
	// Draw "Save" at bottom right
	if(item_selection == 5) {
		save_color = RED;  // Save is selected
	} else {
		save_color = BLACK;  // Save is not selected
	}
	POINT_COLOR = save_color;
	BACK_COLOR = WHITE;
	Show_Str(120, 110, save_color, WHITE, "Save", 16, 0);
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
	
	hour_high = schedule_time_edit_hour / 10;
	hour_low = schedule_time_edit_hour % 10;
	min_high = schedule_time_edit_min / 10;
	min_low = schedule_time_edit_min % 10;
	
	// Clear time area first (x=5~78, y=50~82)
	LCD_Fill(5, 50, 78, 82, WHITE);
	
	// Draw Time section with 16x32 large font
	GUI_DrawBigDigit(5, 50, BLACK, WHITE, '0' + hour_high, 1);   // Hour high digit
	GUI_DrawBigDigit(21, 50, BLACK, WHITE, '0' + hour_low, 1);   // Hour low digit
	// Draw colon
	LCD_Fill(38, 65, 40, 67, BLACK);
	LCD_Fill(38, 71, 40, 73, BLACK);
	GUI_DrawBigDigit(45, 50, BLACK, WHITE, '0' + min_high, 1);   // Minute high digit
	GUI_DrawBigDigit(61, 50, BLACK, WHITE, '0' + min_low, 1);    // Minute low digit
}

// Draw temperature digits only (optimized update)
void Draw_Schedule_Time_Setting_TempDigits(void)
{
	uint8_t temp_high, temp_low;
	
	temp_high = schedule_time_edit_temp / 10;
	temp_low = schedule_time_edit_temp % 10;
	
	// Clear temperature area first (x=88~121, y=50~82)
	LCD_Fill(88, 50, 121, 82, WHITE);
	
	// Draw Temperature with 16x32 font
	GUI_DrawBigDigit(88, 50, BLACK, WHITE, '0' + temp_high, 1);
	GUI_DrawBigDigit(104, 50, BLACK, WHITE, '0' + temp_low, 1);
	
	// Draw degree C (small icon at top-right of temperature)
	GUI_DrawMonoIcon16x16(121, 45, BLACK, WHITE, Icon16x16_C);
}

// Draw ON/OFF section only (optimized update)
void Draw_Schedule_Time_Setting_OnOff(void)
{
	// Clear ON/OFF area (x=123~162, y=63~96) - shifted right by 2 pixels
	LCD_Fill(123, 63, 162, 96, WHITE);
	
	// Draw ON/OFF text - shifted right by 2 pixels
	if(item_selection == 4) {
		// ON/OFF edit mode - show selected one in red
		if(schedule_time_on_off) {
			// ON selected - ON red, OFF black
			Show_Str(123, 63, RED, WHITE, "ON", 16, 0);
			Show_Str(123, 80, BLACK, WHITE, "OFF", 16, 0);
			// Left Arrow Icon next to ON (y=63, aligned with ON) - shifted right by 2 pixels
			GUI_DrawMonoIcon8x16(147, 63, RED, WHITE, IconLeft_Arrow_8x16);
		} else {
			// OFF selected - ON black, OFF red
			Show_Str(123, 63, BLACK, WHITE, "ON", 16, 0);
			Show_Str(123, 80, RED, WHITE, "OFF", 16, 0);
			// Left Arrow Icon next to OFF (y=80, aligned with OFF) - shifted right by 2 pixels
			GUI_DrawMonoIcon8x16(147, 80, RED, WHITE, IconLeft_Arrow_8x16);
		}
	} else {
		// Not in ON/OFF edit mode - both black
		Show_Str(123, 63, BLACK, WHITE, "ON", 16, 0);
		Show_Str(123, 80, BLACK, WHITE, "OFF", 16, 0);
		// Left Arrow Icon stays at the last selected position (ON or OFF) - shifted right by 2 pixels
		if(schedule_time_on_off) {
			GUI_DrawMonoIcon8x16(147, 63, RED, WHITE, IconLeft_Arrow_8x16);  // Next to ON
		} else {
			GUI_DrawMonoIcon8x16(147, 80, RED, WHITE, IconLeft_Arrow_8x16);  // Next to OFF
		}
	}
}

// Draw Save button only (optimized update)
void Draw_Schedule_Time_Setting_Save(void)
{
	uint16_t save_color;
	
	// Clear Save area (x=120~160, y=110~126)
	LCD_Fill(120, 110, 160, 126, WHITE);
	
	// Draw "Save" at bottom right
	if(item_selection == 5) {
		save_color = RED;  // Save is selected
	} else {
		save_color = BLACK;  // Save is not selected
	}
	POINT_COLOR = save_color;
	BACK_COLOR = WHITE;
	Show_Str(120, 110, save_color, WHITE, "Save", 16, 0);
}
