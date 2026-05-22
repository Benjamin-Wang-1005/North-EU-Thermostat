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
	uint8_t text_idx = user_setting_menu_scroll + row;

	// Bounds check
	if(text_idx >= User_Setting_Menu_Item_Count) return;
	if(row >= MENU_VISIBLE_ROWS) return;

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
	Show_Str(30, row_y[row] + 4, BLACK, WHITE, User_Setting_Menu_Items[text_idx].text, 16, 0);
}

void Draw_User_Setting_Menu_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
	uint8_t i;
	uint8_t num_visible_rows;
	uint8_t max_scroll = (User_Setting_Menu_Item_Count > MENU_VISIBLE_ROWS) ? (User_Setting_Menu_Item_Count - MENU_VISIBLE_ROWS) : 0;

	// Auto-adjust scroll so that selected item is visible
	if(!Top_Bar_Active && selection > 0) {
		if(selection > user_setting_menu_scroll + MENU_VISIBLE_ROWS) {
			user_setting_menu_scroll = selection - MENU_VISIBLE_ROWS;
		}
		if(selection - 1 < user_setting_menu_scroll) {
			user_setting_menu_scroll = selection - 1;
		}
	}

	// Ensure scroll is valid
	if(user_setting_menu_scroll > max_scroll) user_setting_menu_scroll = max_scroll;

	Draw_TopBar(leave_col, edit_col);

	// Calculate how many rows to draw (MENU_VISIBLE_ROWS or less if near end)
	num_visible_rows = User_Setting_Menu_Item_Count - user_setting_menu_scroll;
	if(num_visible_rows > MENU_VISIBLE_ROWS) num_visible_rows = MENU_VISIBLE_ROWS;

	// Draw rows based on scroll position
	for(i = 0; i < num_visible_rows; i++) {
		uint8_t option_idx = user_setting_menu_scroll + i;
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
		if((selection == 0) || (selection == 3)){
				if(child_lock == i){
						GUI_DrawMonoIcon16x16(20, row_y[i] + 4, WHITE, BLACK, Icon16x16_Arrow);
				}else{
						LCD_Fill(20, row_y[i] + 4, 36, row_y[i] + 20, WHITE);
				}
		}else if((selection == 1) || (selection == 2)){
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
		//Show_Str(30, 56, color, WHITE, "OFF", 16, 0);
		//GUI_DrawMonoIcon16x16(54, 56, color, WHITE, Block_Icon_16x16);
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
				LCD_Fill(24, 52, 74, 54, BLACK); //
				LCD_Fill(24, 52, 26, 76, BLACK); //
				LCD_Fill(24, 74, 74, 76, BLACK); //
				LCD_Fill(72, 52, 74, 76, BLACK); //
	
				Show_Str(30, 56, BLACK, WHITE, "OFF", 16, 0);
				GUI_DrawMonoIcon16x16(54, 56, BLACK, WHITE, Block_Icon_16x16);
		}else{
				LCD_Fill(24, 52, 74, 54, RED); //
				LCD_Fill(24, 52, 26, 76, RED); //
				LCD_Fill(24, 74, 74, 76, RED); //
				LCD_Fill(72, 52, 74, 76, RED); //
	
				Show_Str(50, 56, RED, WHITE, "ON", 16, 0);
				GUI_DrawMonoIcon16x16(30, 56, RED, WHITE, Block_Icon_16x16);
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
				high_digi = temp_rtc.Year	/ 10;
				low_digi = temp_rtc.Year % 10;
				LCD_Fill(17, 58, 49, 80, WHITE);
				GUI_DrawBigDigit(17, 58, BLACK, WHITE, '0' + high_digi, 1);   
				GUI_DrawBigDigit(33, 58, BLACK, WHITE, '0' + low_digi, 1); 
				GUI_DrawMonoIcon16x16(25, 42, WHITE, RED, Icon16x16_Up_Arror);     
				GUI_DrawMonoIcon16x16(25, 94, WHITE, RED, Icon16x16_Down_Arror);
		}else if(selection == 2){
				high_digi = temp_rtc.Mon / 10;
				low_digi = temp_rtc.Mon % 10;
				LCD_Fill(25, 42, 41, 58, WHITE);
				LCD_Fill(25, 94, 41, 110, WHITE);
				LCD_Fill(65, 58, 98, 90, WHITE);
				GUI_DrawBigDigit(65, 58, BLACK, WHITE, '0' + high_digi, 1);  
				GUI_DrawBigDigit(81, 58, BLACK, WHITE, '0' + low_digi, 1);
				GUI_DrawMonoIcon16x16(73, 42, WHITE, RED, Icon16x16_Up_Arror);     
				GUI_DrawMonoIcon16x16(73, 94, WHITE, RED, Icon16x16_Down_Arror);
		}else if(selection == 3){
				high_digi = temp_rtc.Date / 10;
				low_digi = temp_rtc.Date % 10;
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
		high_digi = temp_rtc.Year	/ 10;
		low_digi = temp_rtc.Year % 10;
		Draw_TopBar(leave_col, edit_col);
	
		// Draw "Temp. Correct" title (left aligned, y=24 - same as Sensor Setting)
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		Show_Str(6, 24, BLACK, WHITE, "  Year Month Date", 16, 0);
		LOGD("Y:%d\r\n",temp_rtc.Year);
	
		
		GUI_DrawBigDigit(17, 58, BLACK, WHITE, '0' + high_digi, 1);   
		GUI_DrawBigDigit(33, 58, BLACK, WHITE, '0' + low_digi, 1);  
		LCD_Fill(57, 62, 58, 86, BLACK);
		
		high_digi = temp_rtc.Mon / 10;
		low_digi = temp_rtc.Mon % 10;
		GUI_DrawBigDigit(65, 58, BLACK, WHITE, '0' + high_digi, 1);  
		GUI_DrawBigDigit(81, 58, BLACK, WHITE, '0' + low_digi, 1);  
		LCD_Fill(105, 62, 106, 86, BLACK);
	
		high_digi = temp_rtc.Date / 10;
		low_digi = temp_rtc.Date % 10;
		GUI_DrawBigDigit(113, 58, BLACK, WHITE, '0' + high_digi, 1);  
		GUI_DrawBigDigit(129, 58, BLACK, WHITE, '0' + low_digi, 1); 
	
		Show_Str(87, 108, BLACK, WHITE, "Set Clock", 16, 0);
		
}

void Draw_User_Setting_Setclk_Content(uint8_t selection)
{
		uint8_t high_digi, low_digi;
	
		if(selection == 1){
				high_digi = temp_rtc.Hour / 10;
				low_digi = temp_rtc.Hour %10;
				LCD_Fill(38, 58, 70, 96, WHITE);
				GUI_DrawBigDigit(38, 58, BLACK, WHITE, '0' + high_digi, 1);   
				GUI_DrawBigDigit(54, 58, BLACK, WHITE, '0' + low_digi, 1); 
				GUI_DrawMonoIcon16x16(46, 42, WHITE, RED, Icon16x16_Up_Arror);     
				GUI_DrawMonoIcon16x16(46, 94, WHITE, RED, Icon16x16_Down_Arror);
		}else if(selection == 2){
				high_digi = temp_rtc.Min / 10;
				low_digi =temp_rtc.Min % 10;
				LCD_Fill(86, 58, 118, 96, WHITE);
				LCD_Fill(46, 42, 62, 58, WHITE);
				LCD_Fill(46, 94, 62, 110, WHITE);
				GUI_DrawBigDigit(86, 58, BLACK, WHITE, '0' + high_digi, 1);   
				GUI_DrawBigDigit(102, 58, BLACK, WHITE, '0' + low_digi, 1); 
				GUI_DrawMonoIcon16x16(94, 42, WHITE, RED, Icon16x16_Up_Arror);     
				GUI_DrawMonoIcon16x16(94, 94, WHITE, RED, Icon16x16_Down_Arror);
		}
		if(selection == 3){
				LCD_Fill(94, 42, 110, 58, WHITE);
				LCD_Fill(94, 94, 110, 110, WHITE);
				Show_Str(127, 108, RED, WHITE, "Save", 16, 0);
		}else{
				Show_Str(127, 108, BLACK, WHITE, "Save", 16, 0);
		}
}

void Draw_User_Setting_Setclk_Page(uint8_t selection, uint8_t leave_col, uint8_t edit_col)
{
		uint8_t high_digi, low_digi;
		high_digi = temp_rtc.Hour	/ 10;
		low_digi = temp_rtc.Hour % 10;
		Draw_TopBar(leave_col, edit_col);
	
		POINT_COLOR = BLACK;
		BACK_COLOR = WHITE;
		Show_Str(30, 24, BLACK, WHITE, " Hour  Minute", 16, 0);
	
		GUI_DrawBigDigit(38, 58, BLACK, WHITE, '0' + high_digi, 1);   
		GUI_DrawBigDigit(54, 58, BLACK, WHITE, '0' + low_digi, 1); 

		high_digi = temp_rtc.Min / 10;
		low_digi = temp_rtc.Min % 10;

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
				GUI_DrawMonoIcon16x16(36, 78, WHITE, RED, Icon16x16_Up_Arror);
				GUI_DrawMonoIcon16x16(116, 78, WHITE, RED, Icon16x16_Down_Arror);
				LCD_Fill(52, 50, 84, 114, WHITE);
				GUI_DrawBigDigit(52, 50, BLACK, WHITE, '0' + ten_digi, 0);
		}if(selection == 2){
				LCD_Fill(36, 78, 52, 94, WHITE);
				LCD_Fill(116, 78, 132, 94, WHITE);
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



void Draw_Pin_Input_Box(uint8_t* pin_buffer, uint8_t digit_index)
{
	uint8_t i;
	uint16_t box_x_start = 16;
	uint16_t box_y = 45;
	uint16_t box_width = 28;
	uint16_t box_height = 36;
	uint16_t box_spacing = 6;
	
	// Draw 4 boxes for PIN digits
	for(i = 0; i < 4; i++){
		uint16_t x = box_x_start + i * (box_width + box_spacing);
		
		// Draw box border
		if(i == digit_index){
			// Current digit box - highlight with red border
			LCD_Fill(x, box_y, x + box_width, box_y + box_height, WHITE);
			LCD_Fill(x, box_y, x + box_width, box_y + 2, RED);  // Top border
			LCD_Fill(x, box_y + box_height - 2, x + box_width, box_y + box_height, RED);  // Bottom border
			LCD_Fill(x, box_y, x + 2, box_y + box_height, RED);  // Left border
			LCD_Fill(x + box_width - 2, box_y, x + box_width, box_y + box_height, RED);  // Right border
		}else{
			// Other boxes - black border
			LCD_Fill(x, box_y, x + box_width, box_y + box_height, WHITE);
			LCD_Fill(x, box_y, x + box_width, box_y + 2, BLACK);  // Top border
			LCD_Fill(x, box_y + box_height - 2, x + box_width, box_y + box_height, BLACK);  // Bottom border
			LCD_Fill(x, box_y, x + 2, box_y + box_height, BLACK);  // Left border
			LCD_Fill(x + box_width - 2, box_y, x + box_width, box_y + box_height, BLACK);  // Right border
		}
		
		// Draw digit inside box (center the 16x32 digit in the box)
		if(pin_buffer[i] != 0xFF){
			GUI_DrawBigDigit(x + 6, box_y + 2, BLACK, WHITE, '0' + pin_buffer[i], 1);  // font_size 1 = 16x32
		}
	}
}

void Draw_Pin_Digit_Selector(uint8_t selected_digit)
{
	uint8_t i;
	uint16_t digit_y = 100;
	uint16_t start_x = 10;
	uint16_t digit_spacing = 14;
	char digit_str[2] = {0, 0};
	
	// Draw digits 0-9
	for(i = 0; i < 10; i++){
		uint16_t x = start_x + i * digit_spacing;
		digit_str[0] = '0' + i;
		
		if(i == selected_digit){
			// Selected digit - red background, white text (8x16 font)
			LCD_Fill(x, digit_y, x + 12, digit_y + 20, RED);
			Show_Str(x + 2, digit_y + 2, WHITE, RED, digit_str, 16, 0);  // 8x16 font
		}else{
			// Unselected - white background, black text (8x16 font)
			LCD_Fill(x, digit_y, x + 12, digit_y + 20, WHITE);
			Show_Str(x + 2, digit_y + 2, BLACK, WHITE, digit_str, 16, 0);  // 8x16 font
		}
	}
}

void Draw_Child_Lock_Pin_Page(uint8_t stage)
{
	// Clear screen
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	
	// Draw title
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	if(stage == 0){
		Show_Str(10, 10, BLACK, WHITE, "Please Setup Pin", 16, 0);
	}else{
		Show_Str(10, 10, BLACK, WHITE, "Please Confirm Pin", 16, 0);
	}
	
	// Draw PIN input boxes
	if(stage == 0){
		Draw_Pin_Input_Box(pin_code_input, pin_digit_index);
	}else{
		Draw_Pin_Input_Box(pin_code_confirm, pin_digit_index);
	}
	
	// Draw digit selector
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
