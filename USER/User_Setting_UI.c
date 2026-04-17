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
	uint16_t row_y[] = {30, 54, 78, 102};  // Y positions for 4 display rows
	uint16_t arrow_bg_color;
	char* menu_texts[] = {"Child lock", "Window Function", "Set Time", "Set Backlight", "Factory Reset"};
	uint8_t text_idx = user_setting_menu_scroll + row;  // 0-4
	
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

void Draw_User_Setting_Menu_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	uint8_t i;
	uint8_t num_visible_rows;
	
	// Ensure scroll is valid (0 or 1 for 5 items with 4 visible)
	if(user_setting_menu_scroll > 1) user_setting_menu_scroll = 1;
	
	Draw_TopBar(leave_col, edit_col);
	
	// Calculate how many rows to draw (4 or less if near end)
	num_visible_rows = 5 - user_setting_menu_scroll;
	if(num_visible_rows > 4) num_visible_rows = 4;
	
	// Draw rows based on scroll position
	for(i = 0; i < num_visible_rows; i++) {
		uint8_t option_idx = user_setting_menu_scroll + i;  // 0-4
		uint8_t is_selected = 0;
		
		if(!Top_Bar_Active && (option_idx == selection - 1)) {
			is_selected = 1;
		}
		Draw_User_Setting_Menu_Row(i, is_selected);
	}
}

void Draw_User_Setting_Child_Lock_Choice(uint8_t selection)
{
	uint16_t row_y[] = {52, 72};  // Y positions for Room, Floor (moved up, consistent spacing)
	uint8_t i;
	
	for(i = 0; i < 2; i++) {
		// Draw text (always black on white background)
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		if(i == 0) {
			Show_Str(45, row_y[i] + 4, BLACK, WHITE, "OFF", 16, 0);
		} else {
			Show_Str(45, row_y[i] + 4, BLACK, WHITE, "ON", 16, 0);
		}
		if(selection == 0){
				if(child_lock == i){
						GUI_DrawMonoIcon16x16(20, row_y[i] + 4, WHITE, BLACK, Icon16x16_Arrow);
				}else{
						LCD_Fill(20, row_y[i] + 4, 36, row_y[i] + 20, WHITE);
				}
		}else{
				// Draw Arrow Icon with RED BACKGROUND for selected item (edit mode style)
				// selection 0=Room, 1=Floor
				if((i+1) == selection) {
					GUI_DrawMonoIcon16x16(20, row_y[i] + 4, WHITE, RED, Icon16x16_Arrow);
				}else{
						LCD_Fill(20, row_y[i] + 4, 36, row_y[i] + 20, WHITE);
				}
		}
		
	}
}

void Draw_User_Setting_Child_Lock_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	Draw_TopBar(leave_col, edit_col);
	
	
	// Draw "Sensor" title closer to Top Bar (left aligned, y=24 - moved up)
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	Show_Str(10, 24, BLACK, WHITE, "Child lock", 16, 0);
	
	// Draw horizontal line below "Sensor" title (y=42 to y=43, leave 4px margin on both sides)
	LCD_Fill(4, 42, lcddev.width - 4, 43, BLACK);
	
	Draw_User_Setting_Child_Lock_Choice(selection);
	if(selection == 3){
			Show_Str(127, 108, RED, WHITE, "Save", 16, 0);
	}else{
			Show_Str(127, 108, BLACK, WHITE, "Save", 16, 0);
	}
	
}

void Draw_User_Setting_Window_Fun_Content(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
		int color = ((window_fun == 1) ? RED : BLACK);
	
		Draw_TopBar(leave_col, edit_col);
	
		LCD_Fill(24, 52, 74, 76, WHITE);  //Clear switch area
		
		LCD_Fill(24, 52, 74, 54, color); 
		LCD_Fill(24, 52, 26, 76, color); 
		LCD_Fill(24, 74, 74, 76, color); 
		LCD_Fill(72, 52, 74, 76, color); 
	
		if(window_fun == 0){
				Show_Str(30, 56, BLACK, WHITE, "OFF", 16, 0);
				GUI_DrawMonoIcon16x16(54, 56, BLACK, WHITE, Block_Icon_16x16);
		}else{
				Show_Str(50, 56, RED, WHITE, "ON", 16, 0);
				GUI_DrawMonoIcon16x16(30, 56, RED, WHITE, Block_Icon_16x16);
		}
		if(selection == 1){
				Show_Str(10, 92, GRAY, WHITE, "Trigger Temp./Time", 16, 0);
		}else if(selection == 2){
				Show_Str(10, 92, RED, WHITE, "Trigger Temp./Time", 16, 0);
		}
}

void Draw_User_Setting_Window_Fun_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
		Draw_TopBar(leave_col, edit_col);
		
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		Show_Str(10, 24, BLACK, WHITE, "Window Function", 16, 0);
	
		// Draw horizontal line below "Sensor" title (y=42 to y=43, leave 4px margin on both sides)
		LCD_Fill(4, 42, lcddev.width - 4, 43, BLACK);
	
	
		if(window_fun == 0){
				// Draw square
				LCD_Fill(24, 52, 74, 54, BLACK); //上橫
				LCD_Fill(24, 52, 26, 76, BLACK); //左直
				LCD_Fill(24, 74, 74, 76, BLACK); //下橫
				LCD_Fill(72, 52, 74, 76, BLACK); //右直
	
				Show_Str(30, 56, BLACK, WHITE, "OFF", 16, 0);
				GUI_DrawMonoIcon16x16(54, 56, BLACK, WHITE, Block_Icon_16x16);
		}else{
				LCD_Fill(24, 52, 74, 54, BLACK); //上橫
				LCD_Fill(24, 52, 26, 76, BLACK); //左直
				LCD_Fill(24, 74, 74, 76, BLACK); //下橫
				LCD_Fill(72, 52, 74, 76, BLACK); //右直
	
				Show_Str(50, 56, BLACK, WHITE, "ON", 16, 0);
				GUI_DrawMonoIcon16x16(30, 56, BLACK, WHITE, Block_Icon_16x16);
		}
			
}

void Draw_User_Setting_Window_Time_Content(uint8_t selection)
{
		if(selection == 1) {
		// Arrow points to Max
		GUI_DrawMonoIcon16x16(4, 52, WHITE, RED, Icon16x16_Arrow);
		Display_Adj_Number((float)window_fun_temp, BLACK, 0);
		Show_Str(127, 108, BLACK, WHITE, "Save", 16, 0);
	} else if(selection == 2) {
		// Arrow points to Floor
		LCD_Fill(4, 52, 20, 68, WHITE);		//clearerr old Arrow
		LCD_Fill(5, 74, 117, 106, WHITE);	//clear digi area
		GUI_DrawMonoIcon16x16(80, 52, WHITE, RED, Icon16x16_Arrow);
		Display_Adj_Number((float)window_fun_time, BLACK, 0);
		Show_Str(127, 108, BLACK, WHITE, "Save", 16, 0);
	} else if(selection == 3) {
		LCD_Fill(80, 52, 96, 68, WHITE);		//clearerr old Arrow
		Show_Str(127, 108, RED, WHITE, "Save", 16, 0);
	}
}

void Draw_User_Setting_Window_Time_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
		Draw_TopBar(leave_col, edit_col);
	
		// Draw "Temp. Correct" title (left aligned, y=24 - same as Sensor Setting)
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		Show_Str(10, 24, BLACK, WHITE, "Window Function", 16, 0);
	
		// Draw horizontal line below "Temp. Limit" title (y=42 to y=43, leave 4px margin on both sides)
		LCD_Fill(4, 42, lcddev.width - 4, 43, BLACK);
	
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		Show_Str(25, 52, BLACK, WHITE, "Temp.", 16, 0);
		Show_Str(100, 52, BLACK, WHITE, "Time", 16, 0);
	
		Draw_User_Setting_Window_Time_Content(selection);
}

void Draw_User_Setting_SetTime_Content(uint8_t selection)
{
		uint8_t high_digi, low_digi;
		
		if(selection == 1){
				high_digi = rtc_time.Year	/ 10;
				low_digi = rtc_time.Year % 10;
				LCD_Fill(17, 58, 49, 80, WHITE);
				GUI_DrawBigDigit(17, 58, BLACK, WHITE, '0' + high_digi, 1);   
				GUI_DrawBigDigit(33, 58, BLACK, WHITE, '0' + low_digi, 1); 
				GUI_DrawMonoIcon16x16(25, 42, WHITE, RED, Icon16x16_Up_Arror);     
				GUI_DrawMonoIcon16x16(25, 94, WHITE, RED, Icon16x16_Down_Arror);
		}else if(selection == 2){
				high_digi = rtc_time.Mon / 10;
				low_digi = rtc_time.Mon % 10;
				LCD_Fill(25, 42, 41, 58, WHITE);
				LCD_Fill(25, 94, 41, 110, WHITE);
				LCD_Fill(65, 58, 98, 90, WHITE);
				GUI_DrawBigDigit(65, 58, BLACK, WHITE, '0' + high_digi, 1);  
				GUI_DrawBigDigit(81, 58, BLACK, WHITE, '0' + low_digi, 1);
				GUI_DrawMonoIcon16x16(73, 42, WHITE, RED, Icon16x16_Up_Arror);     
				GUI_DrawMonoIcon16x16(73, 94, WHITE, RED, Icon16x16_Down_Arror);
		}else if(selection == 3){
				high_digi = rtc_time.Date / 10;
				low_digi = rtc_time.Date % 10;
				LCD_Fill(73, 42, 89, 58, WHITE);
				LCD_Fill(73, 94, 89, 110, WHITE);
				LCD_Fill(113, 58, 145, 90, WHITE);
				GUI_DrawBigDigit(113, 58, BLACK, WHITE, '0' + high_digi, 1);  
				GUI_DrawBigDigit(129, 58, BLACK, WHITE, '0' + low_digi, 1);
				GUI_DrawMonoIcon16x16(121, 42, WHITE, RED, Icon16x16_Up_Arror);     
				GUI_DrawMonoIcon16x16(121, 94, WHITE, RED, Icon16x16_Down_Arror);
		}else if(selection == 4){
				LCD_Fill(121, 42, 137, 58, WHITE);
				LCD_Fill(121, 94, 137, 110, WHITE);
				Show_Str(87, 108, RED, WHITE, "Set Clock", 16, 0);
		}
		
}

void Draw_User_Setting_SetTime_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
		uint8_t high_digi, low_digi;
		high_digi = rtc_time.Year	/ 10;
		low_digi = rtc_time.Year % 10;
		Draw_TopBar(leave_col, edit_col);
	
		// Draw "Temp. Correct" title (left aligned, y=24 - same as Sensor Setting)
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		Show_Str(6, 24, BLACK, WHITE, "  Year Month Date", 16, 0);
	
		
		GUI_DrawBigDigit(17, 58, BLACK, WHITE, '0' + high_digi, 1);   
		GUI_DrawBigDigit(33, 58, BLACK, WHITE, '0' + low_digi, 1);  
		LCD_Fill(57, 62, 58, 86, BLACK);
		
		high_digi = rtc_time.Mon / 10;
		low_digi = rtc_time.Mon % 10;
		GUI_DrawBigDigit(65, 58, BLACK, WHITE, '0' + high_digi, 1);  
		GUI_DrawBigDigit(81, 58, BLACK, WHITE, '0' + low_digi, 1);  
		LCD_Fill(105, 62, 106, 86, BLACK);
	
		high_digi = rtc_time.Date / 10;
		low_digi = rtc_time.Date % 10;
		GUI_DrawBigDigit(113, 58, BLACK, WHITE, '0' + high_digi, 1);  
		GUI_DrawBigDigit(129, 58, BLACK, WHITE, '0' + low_digi, 1); 
	
		Show_Str(87, 108, BLACK, WHITE, "Set Clock", 16, 0);
		
}

void Draw_User_Setting_Setclk_Content(uint8_t selection)
{
		uint8_t high_digi, low_digi;
	
		if(selection == 1){
				high_digi = rtc_time.Hour / 10;
				low_digi = rtc_time.Hour %10;
				LCD_Fill(38, 58, 70, 96, WHITE);
				GUI_DrawBigDigit(38, 58, BLACK, WHITE, '0' + high_digi, 1);   
				GUI_DrawBigDigit(54, 58, BLACK, WHITE, '0' + low_digi, 1); 
				GUI_DrawMonoIcon16x16(46, 42, WHITE, RED, Icon16x16_Up_Arror);     
				GUI_DrawMonoIcon16x16(46, 94, WHITE, RED, Icon16x16_Down_Arror);
		}else if(selection == 2){
				high_digi = rtc_time.Min / 10;
				low_digi =rtc_time.Min % 10;
				LCD_Fill(86, 58, 118, 96, WHITE);
				LCD_Fill(46, 42, 62, 58, WHITE);
				LCD_Fill(46, 94, 62, 110, WHITE);
				GUI_DrawBigDigit(86, 58, BLACK, WHITE, '0' + high_digi, 1);   
				GUI_DrawBigDigit(102, 58, BLACK, WHITE, '0' + low_digi, 1); 
				GUI_DrawMonoIcon16x16(94, 42, WHITE, RED, Icon16x16_Up_Arror);     
				GUI_DrawMonoIcon16x16(94, 94, WHITE, RED, Icon16x16_Down_Arror);
		}else if(selection == 3){
				LCD_Fill(94, 42, 110, 58, WHITE);
				LCD_Fill(94, 94, 110, 110, WHITE);
				Show_Str(127, 108, RED, WHITE, "Save", 16, 0);
		}
}

void Draw_User_Setting_Setclk_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
		uint8_t high_digi, low_digi;
		high_digi = rtc_time.Hour	/ 10;
		low_digi = rtc_time.Hour % 10;
		Draw_TopBar(leave_col, edit_col);
	
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		Show_Str(30, 24, BLACK, WHITE, " Hour  Minute", 16, 0);
	
		GUI_DrawBigDigit(38, 58, BLACK, WHITE, '0' + high_digi, 1);   
		GUI_DrawBigDigit(54, 58, BLACK, WHITE, '0' + low_digi, 1); 

		high_digi = rtc_time.Min / 10;
		low_digi = rtc_time.Min % 10;

		GUI_DrawBigDigit(86, 58, BLACK, WHITE, '0' + high_digi, 1);   
		GUI_DrawBigDigit(102, 58, BLACK, WHITE, '0' + low_digi, 1); 
	
		LCD_Fill(76, 67, 80, 71, BLACK);
		LCD_Fill(76, 79, 80, 83, BLACK);
	
		Show_Str(127, 108, BLACK, WHITE, "Save", 16, 0);
}

void Draw_User_Setting_Backlight_Content(uint8_t selection)
{
		uint8_t ten_digi;
		ten_digi = sleep_backlight_duty / 10;
		if(selection == 1){
				GUI_DrawMonoIcon16x16(36, 66, WHITE, RED, Icon16x16_Up_Arror);
				GUI_DrawMonoIcon16x16(116, 66, WHITE, RED, Icon16x16_Down_Arror);
				LCD_Fill(52, 50, 84, 114, WHITE);
				GUI_DrawBigDigit(52, 50, BLACK, WHITE, '0' + ten_digi, 0);
		}if(selection == 2){
				LCD_Fill(36, 66, 52, 82, WHITE);
				LCD_Fill(116, 66, 132, 82, WHITE);
				Show_Str(127, 108, RED, WHITE, "Save", 16, 0);
		}
		
}
void Draw_User_Setting_Backlight_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
			uint8_t ten_digi;
	
			Draw_TopBar(leave_col, edit_col);
			// Draw "Temp. Correct" title (left aligned, y=24 - same as Sensor Setting)
			POINT_COLOR = BLACK;
			BACK_COLOR = WHITE;
			Show_Str(10, 24, BLACK, WHITE, "Set Backlight", 16, 0);
	
			// Draw horizontal line below "Temp. Limit" title (y=42 to y=43, leave 4px margin on both sides)
			LCD_Fill(4, 42, lcddev.width - 4, 43, BLACK);
			
			ten_digi = sleep_backlight_duty / 10;
			GUI_DrawBigDigit(52, 50, BLACK, WHITE, '0' + ten_digi, 0);
			GUI_DrawBigDigit(84, 50, BLACK, WHITE, '0', 0);
			
			GUI_DrawMonoIcon16x16(116, 50, BLACK, WHITE, Percent_Icon_16x16);
			Show_Str(127, 108, BLACK, WHITE, "Save", 16, 0);
}

void Draw_User_Setting_Reset_Contect(uint8_t selection)
{
		Show_Str(119, 108, RED, WHITE, "Reset", 16, 0);
}
void Draw_User_Setting_Reset_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
			Draw_TopBar(leave_col, edit_col);
			// Draw "Temp. Correct" title (left aligned, y=24 - same as Sensor Setting)
			POINT_COLOR = BLACK;
			BACK_COLOR = WHITE;
			Show_Str(10, 24, BLACK, WHITE, "Factory Reset", 16, 0);
	
			// Draw horizontal line below "Temp. Limit" title (y=42 to y=43, leave 4px margin on both sides)
			LCD_Fill(4, 42, lcddev.width - 4, 43, BLACK);
	
			Show_Str(10, 50, RED, WHITE, "Reset to Default?", 16, 0);
			Show_Str(10, 66, BLACK, WHITE, "This will erease ", 16, 0);
			Show_Str(10, 82, BLACK, WHITE, "all saved settings.", 16, 0);
	
			Show_Str(119, 108, BLACK, WHITE, "Reset", 16, 0);
}
