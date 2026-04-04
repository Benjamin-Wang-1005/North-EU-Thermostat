//////////////////////////////////////////////////////////////////////////////////	 
// STM32 LCD Controller with Key Input
// Display: Setting number with Up/Down key control
// PA0  = Up Key (High Active)
// PC13 = Down Key (High Active)
//////////////////////////////////////////////////////////////////////////////////	 	
#include "delay.h"
#include "sys.h"
#include "lcd.h"
#include "touch.h"
#include "gui.h"
#include "test.h"
#include "arial_digits_flash.h"
// icons_flash.h no longer needed - using monochrome icons from icons_16x16.h
#include "icons_32x32.h"  // New 32x32 monochrome icons
#include "icons_8x16.h"   // New 8x16 monochrome icons for Function Setting
#include "icons_16x16.h"  // New 16x16 monochrome icons for Schedule Edit
#include "icons_24x24.h"  // New 24x24 monochrome icons for Active page
#include "stm32f10x.h"
#include "stm32f10x_tim.h"
#include <stdio.h>
#include <string.h>

// Key definitions
#define UP_KEY_PIN    GPIO_Pin_0   // PA0
#define UP_KEY_PORT   GPIOA
#define DOWN_KEY_PIN  GPIO_Pin_13  // PC13
#define DOWN_KEY_PORT GPIOC
#define ENTER_KEY_PIN    GPIO_Pin_4   // PA4
#define ENTER_KEY_PORT   GPIOA

// State definitions
#define STATE_ACTIVE    0   // Active狀態：LCM initial完成後，背光90%
#define STATE_SLEEP     1   // Sleep狀態：10秒無按鍵，背光30%
#define STATE_SETTING   2   // Setting狀態：Active下按鍵觸發，可調整數字
#define STATE_FUNC_SETTING 3 // Function Setting狀態：紅色Manu下按Enter進入
#define STATE_FUNC_SETTING_EDIT 4 // Function Setting選項編輯模式：Edit紅色時按Enter進入
#define STATE_HEATING_SCHEDULE_MENU 5 // Heating Schedule Menu頁面：Function Setting中Heating Schedule Enter進入
#define STATE_HEATING_SCHEDULE_PROG_TYPE 6 // Heating Schedule Menu -> Program Type Enter : Program Type頁面
#define STATE_SCHEDULE_EDIT 7 // Schedule Edit頁面：Heating Schedule Menu中Weekly Schedule Enter進入
#define STATE_SCHEDULE_TIME_SETTING 8 // Schedule Time Setting頁面：P1~P6 Enter進入
#define STATE_CONTROL_ADJ_MENU 9     // Control Adj Menu頁面：Function Setting中Control Adj Enter進入
#define STATE_CONTROL_ADJ_SENSOR 10 // Control Adj Menu -> Sensor Enter : Sensor Setting頁面

// Timeout definitions (in 10ms units)
#define SLEEP_TIMEOUT   1000  // 10 seconds = 1000 * 10ms
#define EDIT_TIMEOUT    500   // 5 seconds for setting mode

// Backlight PWM definitions
#define BACKLIGHT_PWM_FREQ    500   // 500Hz PWM frequency
#define BACKLIGHT_DUTY_ACTIVE 100   // 100% duty cycle for active state (full brightness)
#define BACKLIGHT_DUTY_SLEEP  20    // 20% duty cycle for sleep state

// Color definitions
#define EDIT_COLOR      BLUE    // 編輯狀態顏色

// Function prototypes
void Key_Init(void);
void Backlight_Init(void);
void Backlight_SetDuty(uint8_t duty_percent);
uint8_t Key_Scan(void);
void Display_Number(float number, uint16_t color, uint8_t show_decimal);
void Clear_Number_Area(void);
void Draw_Icon6_Red(uint8_t red);
void Draw_Function_Setting_Page(uint8_t edit_red);
void Draw_Function_Setting_TopBar(uint8_t edit_red);
void Draw_Function_Setting_Edit_Page(uint8_t selection);
void Draw_Function_Setting_Edit_Row(uint8_t row, uint8_t selected);
void Draw_Main_Page(void);
void Draw_Heating_Schedule_Menu_Page(uint8_t selection);
void Draw_Heating_Schedule_Menu_Row(uint8_t row, uint8_t selected);
void Draw_Heating_Schedule_Menu_Row(uint8_t row, uint8_t selected);
void Draw_Heating_Schedule_Prog_Type_Page(void);
void Draw_Heating_Schedule_Prog_Type_TopBar(void);
void Draw_Heating_Schedule_Prog_Type_Choices(uint8_t old_sel, uint8_t new_sel);
void Draw_Generic_TopBar(uint8_t leave_red, uint8_t edit_red);
// Draw a single row in Schedule Edit page (for optimized update)
void Draw_Schedule_Edit_Page(uint8_t selection);
void Draw_Schedule_Edit_Row(uint8_t row, uint8_t period_idx, uint8_t selected);
void Draw_Schedule_Edit_Finish(uint8_t selected);
void Draw_Control_Adj_Menu_Page(uint8_t selection);
void Draw_Control_Adj_Menu_Row(uint8_t row, uint8_t selected);
void Draw_Control_Adj_Sensor_Page(uint8_t selection);
void Draw_Control_Adj_Sensor_TopBar(uint8_t leave_red, uint8_t edit_red);
void Draw_Control_Adj_Sensor_Choices(uint8_t selection);
void Draw_Control_Adj_Sensor_Choices_NoArrow(void);
void Draw_Control_Adj_Sensor_Save(uint8_t selected);
void Draw_Control_Adj_TempCorrect_Page(uint8_t selection);
void Draw_Control_Adj_TempCorrect_Options(uint8_t selection);
void Draw_Control_Adj_TempCorrect_Save(uint8_t selected);
void Draw_TempCorrect_Value(float value, uint16_t x, uint16_t y, uint16_t color);
void Draw_Schedule_Time_Setting_Page(uint8_t period_num);
void Draw_Schedule_Time_Setting_TopBar(uint8_t period_num);
void Draw_Schedule_Time_Setting_ClearArrows(void);
void Draw_Schedule_Time_Setting_HourArrows(uint8_t show);
void Draw_Schedule_Time_Setting_MinArrows(uint8_t show);
void Draw_Schedule_Time_Setting_TempArrows(uint8_t show);
void Draw_Schedule_Time_Setting_TimeDigits(void);
void Draw_Schedule_Time_Setting_TempDigits(void);
void Draw_Schedule_Time_Setting_OnOff(void);
void Draw_Schedule_Time_Setting_Save(void);

// Global variables
float setting_number = 4.0f;     // 設定數字，初始值 4.0
uint8_t system_state = STATE_ACTIVE;  // 開機後進入Active狀態
uint32_t sleep_timeout_counter = 0;   // Sleep狀態計時器
uint32_t edit_timeout_counter = 0;    // Setting狀態計時器
uint8_t icon6_red_state = 0;          // Icon6紅色狀態標記 (0=正常, 1=紅色)
uint8_t func_setting_edit_red = 1;    // Function Setting頁面: 1=Edit紅色, 0=Leave紅色
uint8_t func_setting_sel = 0;         // Function Setting選項編輯模式選擇: 0=Heating Schedule, 1=Control Adj, 2=User Settings

uint8_t current_prog_type = 0;        // 0="5+2", 1="6+1", 2="7"
uint8_t prog_type_substate = 0;       // 0=TopBar, 1=Choosing, 2=Save prompt
uint8_t prog_type_top_sel = 1;        // 0=Leave, 1=Edit
uint8_t prog_type_edit_sel = 0;       // Temporary selection for program type

// Heating Schedule Menu頁面變量 (新頁面)
uint8_t heating_schedule_menu_substate = 0;  // 0=TopBar, 1=選項模式
uint8_t heating_schedule_menu_top_sel = 1;   // TopBar: 0=Leave紅, 1=Edit紅
uint8_t heating_schedule_menu_sel = 0;       // 選項: 0=Program Type, 1=Workday Setting, 2=Restday Setting

// Schedule Edit頁面變量 (合併P1-P6)
uint8_t schedule_edit_substate = 0;   // 0=TopBar(Edit/Leave), 1=P1-P6/Finish選項模式
uint8_t schedule_edit_top_sel = 1;    // TopBar狀態: 0=Leave紅色, 1=Edit紅色
uint8_t schedule_edit_sel = 0;        // P1-P6/Finish選擇: 0=P1, 1=P2, 2=P3, 3=P4, 4=P5, 5=P6, 6=Finish
uint8_t schedule_edit_scroll = 0;     // 捲動偏移: 0=顯示P1-P4, 1=顯示P2-P5, 2=顯示P3-P6

// Control Adj Menu頁面變量
uint8_t control_adj_menu_substate = 0;  // 0=TopBar, 1=選項模式
uint8_t control_adj_menu_top_sel = 1;   // TopBar: 0=Leave紅, 1=Edit紅
uint8_t control_adj_menu_sel = 0;       // 選項: 0=Sensor, 1=Temp.Correct, 2=Temp.Limit, 3=Temp.Protect, 4=Power On State
uint8_t control_adj_menu_scroll = 0;    // 捲動偏移: 0=顯示row0-3, 1=顯示row1-4

// Control Adj Sensor Setting頁面變量
uint8_t control_adj_sensor_substate = 0;  // 0=TopBar, 1=Floor/Room選項模式, 2=Save模式
uint8_t control_adj_sensor_top_sel = 1;   // TopBar: 0=Leave紅, 1=Edit紅
uint8_t control_adj_sensor_sel = 0;       // 選項: 0=Room, 1=Floor (swapped order)
uint8_t current_sensor_type = 0;          // 儲存設定: 0=Room, 1=Floor (default Room)

// Schedule Time Setting頁面變量
uint8_t schedule_time_setting_period = 1;  // 當前設定的Period編號 (1~6)
uint8_t schedule_time_setting_substate = 0; // 0=TopBar, 1=小時編輯, 2=分編輯, 3=溫度編輯, 4=ON/OFF編輯, 5=Save
uint8_t schedule_time_setting_top_sel = 1;  // TopBar: 0=Leave紅, 1=Edit紅
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

// LCD color definitions (if not already defined)
#ifndef BLUE
#define BLUE    0x001F
#endif
#ifndef RED
#define RED     0xF800
#endif

int main(void)
{
	uint8_t key;

	SystemInit();        // Initialize RCC, system clock to 72MHZ
	delay_init(72);	     // Delay initialization
	LCD_Init();	         // LCD initialization
	Key_Init();          // Key initialization
	Backlight_Init();    // Backlight PWM initialization

	// Set rotation to 90 degrees
	LCD_direction(1);

	// Clear screen with white background
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);

	// Display static elements (time, icons)
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	GUI_DrawBigDigit(5, 5, BLACK, WHITE, '1', 1);
	GUI_DrawBigDigit(21, 5, BLACK, WHITE, '0', 1);
	// colon
	LCD_Fill(40, 15, 43, 18, BLACK);
	LCD_Fill(40, 25, 43, 28, BLACK);
	GUI_DrawBigDigit(47, 5, BLACK, WHITE, '3', 1);
	GUI_DrawBigDigit(63, 5, BLACK, WHITE, '2', 1);
	Draw_Static_Icons();

	// Initial display: 4.0 in BLACK (no blinking)
	Display_Number(setting_number, BLACK, 1);  // show_decimal = 1

	// Set initial state: Active with 90% backlight
	system_state = STATE_ACTIVE;
	Backlight_SetDuty(BACKLIGHT_DUTY_ACTIVE);
	sleep_timeout_counter = 0;

	// Main loop
	while(1)
	{
		// Scan keys
		key = Key_Scan();

		if(system_state == STATE_ACTIVE)
		{
			// Active state: handle key presses
			if(key == 1 || key == 2)  // Up or Down key pressed -> enter Setting mode
			{
				// If Manu Icon is red, change it back to black first
				if(icon6_red_state)
				{
					icon6_red_state = 0;
					Draw_Icon6_Red(icon6_red_state);  // Draw black Manu Icon
				}

				// Transition to Setting state
				system_state = STATE_SETTING;
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
					system_state = STATE_FUNC_SETTING;
					sleep_timeout_counter = 0;
					func_setting_edit_red = 1;  // Initial state: Edit Icon red
					// Draw Function Setting page
					Draw_Function_Setting_Page(func_setting_edit_red);
				}
				else
				{
					// Manu Icon is black -> toggle to red
					icon6_red_state = 1;
					Draw_Icon6_Red(icon6_red_state);  // Draw red Manu Icon
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
						Draw_Icon6_Red(icon6_red_state);  // Draw black Manu Icon
					}

					system_state = STATE_SLEEP;
					sleep_timeout_counter = 0;

					// Set backlight to 20%
					Backlight_SetDuty(BACKLIGHT_DUTY_SLEEP);
				}
			}
		}
		else if(system_state == STATE_SLEEP)
		{
			// Sleep state: wait for any key press to wake up
			if(key != 0)  // Up or Down key pressed
			{
				// Wake up - return to Active state
				system_state = STATE_ACTIVE;
				sleep_timeout_counter = 0;

				// Set backlight back to 90%
				Backlight_SetDuty(BACKLIGHT_DUTY_ACTIVE);

				delay_ms(100);  // Debounce
			}
		}
		else if(system_state == STATE_SETTING)
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
				system_state = STATE_ACTIVE;
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
				system_state = STATE_ACTIVE;
				edit_timeout_counter = 0;
				sleep_timeout_counter = 0;

				// Display final number with decimal in BLACK
				Clear_Number_Area();
				Display_Number(setting_number, BLACK, 1);  // show_decimal = 1
			}
		}
		else if(system_state == STATE_FUNC_SETTING)
		{
			// Function Setting state
			if(key == 1 || key == 2)  // Up or Down key pressed -> toggle Icon colors
			{
				// Toggle between Edit red and Leave red
				func_setting_edit_red = !func_setting_edit_red;
				// Only redraw Top Bar (much faster than full page redraw)
				Draw_Function_Setting_TopBar(func_setting_edit_red);
				delay_ms(100);  // Debounce
			}
			else if(key == 3)  // Enter key pressed
			{
				if(func_setting_edit_red)
				{
					// Edit Icon is red -> enter Function Setting 選項編輯模式
					system_state = STATE_FUNC_SETTING_EDIT;
					func_setting_sel = 0;  // Start with Heating Schedule selected
					Draw_Function_Setting_Edit_Page(func_setting_sel);
				}
				else
				{
					// Leave Icon is red -> return to Active state
					system_state = STATE_ACTIVE;
					sleep_timeout_counter = 0;
					icon6_red_state = 0;  // Reset Manu Icon to black

					// Redraw main page
					Draw_Main_Page();
				}
				delay_ms(100);  // Debounce
			}
		}
		else if(system_state == STATE_FUNC_SETTING_EDIT)
		{
			// Function Setting Edit mode navigation
			if(key == 2)  // Down key pressed
			{
				if(func_setting_sel < 2)
				{
					// Move selection down - only update the changed rows
					uint8_t old_sel = func_setting_sel;
					func_setting_sel++;
					Draw_Function_Setting_Edit_Row(old_sel, 0);
					Draw_Function_Setting_Edit_Row(func_setting_sel, 1);
				}
				delay_ms(100);
			}
			else if(key == 1)  // Up key pressed
			{
				if(func_setting_sel > 0)
				{
					// Move selection up - only update the changed rows
					uint8_t old_sel = func_setting_sel;
					func_setting_sel--;
					Draw_Function_Setting_Edit_Row(old_sel, 0);
					Draw_Function_Setting_Edit_Row(func_setting_sel, 1);
				}
				else
				{
					// At top, go back to Function Setting with Leave red
					system_state = STATE_FUNC_SETTING;
					func_setting_edit_red = 0;  // Leave red, Edit black
					Draw_Function_Setting_Page(func_setting_edit_red);
				}
				delay_ms(100);
			}
			else if(key == 3)  // Enter key pressed
			{
				if(func_setting_sel == 0) // "Heating Schedule" selected
				{
					// Enter Heating Schedule Menu頁面
					system_state = STATE_HEATING_SCHEDULE_MENU;
					heating_schedule_menu_substate = 0;   // Start with TopBar
					heating_schedule_menu_top_sel = 1;    // Edit red by default
					heating_schedule_menu_sel = 0;        // Default select Program Type
					Draw_Heating_Schedule_Menu_Page(0xFF);  // 0xFF = TopBar mode with Edit red
				}
				else if(func_setting_sel == 1) // "Control Adj." selected
				{
					// Enter Control Adj Menu頁面
					system_state = STATE_CONTROL_ADJ_MENU;
					control_adj_menu_substate = 0;   // Start with TopBar
					control_adj_menu_top_sel = 1;    // Edit red by default
					control_adj_menu_sel = 0;        // Default select Sensor
					control_adj_menu_scroll = 0;     // Reset scroll
					Draw_Control_Adj_Menu_Page(0xFF);  // 0xFF = TopBar mode with Edit red
				}
				// TODO: User Settings 的處理
				delay_ms(100);
			}
		}
		else if(system_state == STATE_HEATING_SCHEDULE_MENU)
		{
			// Heating Schedule Menu page navigation
			if(heating_schedule_menu_substate == 0)  // TopBar mode: Edit/Leave
			{
				if(key == 1 || key == 2)  // Up or Down key pressed
				{
					// Toggle between Edit red and Leave red
					heating_schedule_menu_top_sel = !heating_schedule_menu_top_sel;
					// Only update Top Bar
					Draw_Generic_TopBar(heating_schedule_menu_top_sel ? 0 : 1, heating_schedule_menu_top_sel ? 1 : 0);
					delay_ms(100);
				}
				else if(key == 3)  // Enter key pressed
				{
					if(heating_schedule_menu_top_sel)  // Edit Icon is red
					{
						// Enter選項模式, Edit/Leave both black, Program Type selected
						heating_schedule_menu_substate = 1;
						heating_schedule_menu_sel = 0;  // Start at Program Type
						Draw_Heating_Schedule_Menu_Page(heating_schedule_menu_sel);
					}
					else  // Leave Icon is red
					{
						// Go back to Function Setting with Leave red
						system_state = STATE_FUNC_SETTING;
						func_setting_edit_red = 0;  // Leave red, Edit black
						Draw_Function_Setting_Page(func_setting_edit_red);
					}
					delay_ms(100);
				}
			}
			else if(heating_schedule_menu_substate == 1)  // Program Type / Workday Setting / Restday Setting mode
			{
				if(key == 2)  // Down key pressed
				{
					if(heating_schedule_menu_sel < 2)  // Can go to Restday Setting (2)
					{
						// Move selection down - only update changed rows
						uint8_t old_sel = heating_schedule_menu_sel;
						heating_schedule_menu_sel++;
						Draw_Heating_Schedule_Menu_Row(old_sel, 0);  // Old row unselected
						Draw_Heating_Schedule_Menu_Row(heating_schedule_menu_sel, 1);  // New row selected
					}
					delay_ms(100);
				}
				else if(key == 1)  // Up key pressed
				{
					if(heating_schedule_menu_sel > 0)
					{
						// Move selection up - only update changed rows
						uint8_t old_sel = heating_schedule_menu_sel;
						heating_schedule_menu_sel--;
						Draw_Heating_Schedule_Menu_Row(old_sel, 0);  // Old row unselected
						Draw_Heating_Schedule_Menu_Row(heating_schedule_menu_sel, 1);  // New row selected
					}
					else  // At Program Type, go back to TopBar mode
					{
						heating_schedule_menu_substate = 0;
						heating_schedule_menu_top_sel = 1;  // Edit red
						// Redraw entire page to clear Program Type selection and show TopBar with Edit red
						Draw_Heating_Schedule_Menu_Page(0xFF);  // 0xFF = TopBar mode with Edit red
					}
					delay_ms(100);
				}
				else if(key == 3)  // Enter key pressed
				{
					if(heating_schedule_menu_sel == 0)  // Program Type selected
					{
						// Enter Program Type頁面
						system_state = STATE_HEATING_SCHEDULE_PROG_TYPE;
						prog_type_substate = 0;
						prog_type_top_sel = 1; // Default Edit Red
						Draw_Heating_Schedule_Prog_Type_Page();
					}
					else if(heating_schedule_menu_sel == 1 || heating_schedule_menu_sel == 2)  // Workday or Restday Setting selected
					{
						// Set schedule edit source: 0=Workday, 1=Restday
						schedule_edit_source = (heating_schedule_menu_sel == 1) ? 0 : 1;
						// Enter Schedule Edit頁面 (先進入TopBar模式，游標在Edit Icon)
						system_state = STATE_SCHEDULE_EDIT;
						schedule_edit_substate = 0;   // 先進入TopBar模式
						schedule_edit_top_sel = 1;    // Edit Icon 紅色
						schedule_edit_sel = 0;        // 預設P1，但還沒進入選項模式
						schedule_edit_scroll = 0;     // 重置捲動位置
						Draw_Schedule_Edit_Page(0xFF);  // 0xFF = TopBar模式，Edit紅色
					}
					delay_ms(100);
				}
			}
		}
		else if(system_state == STATE_HEATING_SCHEDULE_PROG_TYPE)
		{
			// Program Type page navigation
			if(prog_type_substate == 0) // Top Bar (Leave / Edit)
			{
				if(key == 1 || key == 2)  // Up or Down
				{
					prog_type_top_sel = !prog_type_top_sel;
					// Only update Top Bar (much faster than full page redraw)
					Draw_Heating_Schedule_Prog_Type_TopBar();
					delay_ms(100);
				}
				else if(key == 3)  // Enter key
				{
					if(prog_type_top_sel == 0) { // Leave selected
						// Go back to Heating Schedule Menu with Leave red
						system_state = STATE_HEATING_SCHEDULE_MENU;
						heating_schedule_menu_substate = 0;
						heating_schedule_menu_top_sel = 0;  // Leave red
						Draw_Heating_Schedule_Menu_Page(0xFE);  // 0xFE = TopBar mode with Leave red
					} else { // Edit selected
						prog_type_substate = 1; // Enter Choosing mode
						prog_type_edit_sel = current_prog_type; // Start at current saved value
						Draw_Heating_Schedule_Prog_Type_Page();
					}
					delay_ms(100);
				}
			}
			else if(prog_type_substate == 1) // Choosing Type
			{
				if(key == 1) // Up (Left)
				{
					if(prog_type_edit_sel > 0) {
						// Move selection left - only update changed choices
						uint8_t old_sel = prog_type_edit_sel;
						prog_type_edit_sel--;
						Draw_Heating_Schedule_Prog_Type_Choices(old_sel, prog_type_edit_sel);
					}
					delay_ms(100);
				}
				else if(key == 2) // Down (Right)
				{
					if(prog_type_edit_sel < 2) {
						// Move selection right - only update changed choices
						uint8_t old_sel = prog_type_edit_sel;
						prog_type_edit_sel++;
						Draw_Heating_Schedule_Prog_Type_Choices(old_sel, prog_type_edit_sel);
					}
					delay_ms(100);
				}
				else if(key == 3) // Enter -> Move to Save button
				{
					prog_type_substate = 2; // Save Prompt
					Draw_Heating_Schedule_Prog_Type_Page();
					delay_ms(100);
				}
			}
			else if(prog_type_substate == 2) // Save prompt
			{
				if(key == 1 || key == 2) // Up or Down -> Cancel save, go back to Choosing
				{
					prog_type_substate = 1;
					Draw_Heating_Schedule_Prog_Type_Page();
					delay_ms(100);
				}
				else if(key == 3) // Enter -> Save and go back to Heating Schedule Menu
				{
					current_prog_type = prog_type_edit_sel; // Save selection
					// Go back to Heating Schedule Menu with Leave red
					system_state = STATE_HEATING_SCHEDULE_MENU;
					heating_schedule_menu_substate = 0;   // TopBar模式
					heating_schedule_menu_top_sel = 0;    // Leave red
					Draw_Heating_Schedule_Menu_Page(0xFE);  // 0xFE = TopBar mode with Leave red
					delay_ms(100);
				}
			}
		}
		else if(system_state == STATE_SCHEDULE_EDIT)
		{
			// Schedule Edit page navigation (合併P1-P6)
			if(schedule_edit_substate == 0)  // TopBar mode: Edit/Leave
			{
				if(key == 1 || key == 2)  // Up or Down key pressed
				{
					// Toggle between Edit red and Leave red
					schedule_edit_top_sel = !schedule_edit_top_sel;
					// Only update Top Bar using generic function
					Draw_Generic_TopBar(schedule_edit_top_sel ? 0 : 1, schedule_edit_top_sel ? 1 : 0);
					delay_ms(100);
				}
				else if(key == 3)  // Enter key pressed
				{
					if(schedule_edit_top_sel)  // Edit Icon is red
					{
						// Enter P1-P6/Finish mode, Edit/Leave both black, P1 Arrow red
						schedule_edit_substate = 1;
						schedule_edit_sel = 0;  // Start at P1
						Draw_Schedule_Edit_Page(schedule_edit_sel);
					}
					else  // Leave Icon is red
					{
						// Go back to Heating Schedule Menu with Leave red
						system_state = STATE_HEATING_SCHEDULE_MENU;
						heating_schedule_menu_substate = 0;
						heating_schedule_menu_top_sel = 0;  // Leave red
						Draw_Heating_Schedule_Menu_Page(0xFE);  // 0xFE = TopBar mode with Leave red
					}
					delay_ms(100);
				}
			}
			else if(schedule_edit_substate == 1)  // P1-P6/Finish mode
			{
				if(key == 2)  // Down key pressed
				{
					if(schedule_edit_sel < 6)  // Can go to Finish (6)
					{
						uint8_t old_sel = schedule_edit_sel;
						uint8_t old_scroll;
						schedule_edit_sel++;
						
						// Check if scroll position needs to change
						// scroll=0: P1-P4, scroll=1: P2-P5, scroll=2: P3-P6, scroll=3: P4-P6+Finish
						// Down scroll logic: scroll when new selection goes below visible area
						old_scroll = schedule_edit_scroll;
						if(schedule_edit_sel <= 5) {
							// Regular period (P1-P6): scroll if below visible bottom
							if(schedule_edit_sel > schedule_edit_scroll + 3) {
								schedule_edit_scroll = schedule_edit_sel - 3;
							}
						} else {
							// Finish (sel=6): need scroll=3 to show Finish at row 3
							if(schedule_edit_scroll < 3) {
								schedule_edit_scroll = 3;
							}
						}
						
						// If scroll changed, redraw entire page
						if(old_scroll != schedule_edit_scroll) {
							Draw_Schedule_Edit_Page(schedule_edit_sel);
						} else {
							// Only update changed rows within the 4 visible rows
							uint8_t old_row = old_sel - schedule_edit_scroll;
							uint8_t new_row = schedule_edit_sel - schedule_edit_scroll;
							
							// Handle transition to/from Finish
							if(old_sel == 6) {
								// From Finish to P6
								Draw_Schedule_Edit_Finish(0);
								Draw_Schedule_Edit_Row(new_row, schedule_edit_sel, 1);
							} else if(schedule_edit_sel == 6) {
								// From P6 to Finish
								Draw_Schedule_Edit_Row(old_row, old_sel, 0);
								Draw_Schedule_Edit_Finish(1);
							} else {
								// Normal row transition
								Draw_Schedule_Edit_Row(old_row, old_sel, 0);
								Draw_Schedule_Edit_Row(new_row, schedule_edit_sel, 1);
							}
						}
					}
					delay_ms(100);
				}
				else if(key == 1)  // Up key pressed
				{
					if(schedule_edit_sel > 0)
					{
						uint8_t old_sel = schedule_edit_sel;
						uint8_t old_scroll;
						schedule_edit_sel--;
						
						// Check if scroll position needs to change
						// scroll=0: P1-P4, scroll=1: P2-P5, scroll=2: P3-P6, scroll=3: P4-P6+Finish
						// Up scroll logic: scroll when new selection goes above visible area
						old_scroll = schedule_edit_scroll;
						if(schedule_edit_sel < schedule_edit_scroll) {
							schedule_edit_scroll = schedule_edit_sel;
						}
						
						// If scroll changed, redraw entire page
						if(old_scroll != schedule_edit_scroll) {
							Draw_Schedule_Edit_Page(schedule_edit_sel);
						} else {
							// Only update changed rows within the 4 visible rows
							uint8_t old_row = old_sel - schedule_edit_scroll;
							uint8_t new_row = schedule_edit_sel - schedule_edit_scroll;
							
							// Handle transition to/from Finish
							if(old_sel == 6) {
								// From Finish to P6
								Draw_Schedule_Edit_Finish(0);
								Draw_Schedule_Edit_Row(new_row, schedule_edit_sel, 1);
							} else if(schedule_edit_sel == 6) {
								// From P6 to Finish
								Draw_Schedule_Edit_Row(old_row, old_sel, 0);
								Draw_Schedule_Edit_Finish(1);
							} else {
								// Normal row transition
								Draw_Schedule_Edit_Row(old_row, old_sel, 0);
								Draw_Schedule_Edit_Row(new_row, schedule_edit_sel, 1);
							}
						}
					}
					else  // At P1, go back to TopBar mode
					{
						schedule_edit_substate = 0;
						schedule_edit_top_sel = 1;  // Edit red
						// Redraw entire page to show TopBar with Edit red and clear P1 selection
						Draw_Schedule_Edit_Page(0xFF);  // 0xFF = TopBar mode, Edit red
					}
					delay_ms(100);
				}
				else if(key == 3)  // Enter key pressed
				{
					if(schedule_edit_sel == 6)  // Finish selected
					{
						// Go back to Heating Schedule Menu with Leave red
						system_state = STATE_HEATING_SCHEDULE_MENU;
						heating_schedule_menu_substate = 0;
						heating_schedule_menu_top_sel = 0;  // Leave red
						Draw_Heating_Schedule_Menu_Page(0xFE);  // 0xFE = TopBar mode with Leave red
					}
					else if(schedule_edit_sel <= 5)  // P1-P6 selected
					{
						// Enter Schedule Time Setting頁面
						system_state = STATE_SCHEDULE_TIME_SETTING;
						schedule_time_setting_period = schedule_edit_sel + 1;  // P1=1, P2=2, ..., P6=6
						schedule_time_setting_substate = 0;  // Start with TopBar
						schedule_time_setting_top_sel = 1;   // Edit red
						// Initialize edit values from schedule_settings array (Workday or Restday)
						{
							uint8_t (*sched_ptr)[4] = schedule_edit_source ? schedule_settings_restday : schedule_settings;
							schedule_time_edit_hour = sched_ptr[schedule_time_setting_period - 1][0];
							schedule_time_edit_min = sched_ptr[schedule_time_setting_period - 1][1];
							schedule_time_edit_temp = sched_ptr[schedule_time_setting_period - 1][2];
							schedule_time_on_off = sched_ptr[schedule_time_setting_period - 1][3];
						}
						Draw_Schedule_Time_Setting_Page(schedule_time_setting_period);
					}
					delay_ms(100);
				}
			}
		}
		else if(system_state == STATE_SCHEDULE_TIME_SETTING)
		{
			// Schedule Time Setting頁面 navigation
			if(schedule_time_setting_substate == 0)  // TopBar mode: Edit/Leave
			{
				if(key == 1 || key == 2)  // Up or Down key pressed
				{
					// Toggle between Edit red and Leave red
					schedule_time_setting_top_sel = !schedule_time_setting_top_sel;
					// Only update Top Bar (much faster than full page redraw)
					Draw_Schedule_Time_Setting_TopBar(schedule_time_setting_period);
					delay_ms(100);
				}
				else if(key == 3)  // Enter key pressed
				{
					if(schedule_time_setting_top_sel)  // Edit Icon is red
					{
						// Enter 小時編輯模式
						schedule_time_setting_substate = 1;
						// Initialize edit values from schedule_settings array (Workday or Restday)
						{
							uint8_t (*sched_ptr)[4] = schedule_edit_source ? schedule_settings_restday : schedule_settings;
							schedule_time_edit_hour = sched_ptr[schedule_time_setting_period - 1][0];
							schedule_time_edit_min = sched_ptr[schedule_time_setting_period - 1][1];
							schedule_time_edit_temp = sched_ptr[schedule_time_setting_period - 1][2];
							schedule_time_on_off = sched_ptr[schedule_time_setting_period - 1][3];
						}
						// Update Top Bar (both icons black in edit mode) and draw hour arrows
						Draw_Schedule_Time_Setting_TopBar(schedule_time_setting_period);
						Draw_Schedule_Time_Setting_HourArrows(1);
					}
					else  // Leave Icon is red
					{
						// Go back to Schedule Edit
						system_state = STATE_SCHEDULE_EDIT;
						schedule_edit_substate = 1;
						schedule_edit_sel = schedule_time_setting_period - 1;  // P1-P6 -> 0-5
						// Set scroll based on selection to ensure correct rows are displayed
						if(schedule_edit_sel <= 3) {
							schedule_edit_scroll = 0;  // P1-P4 visible
						} else {
							schedule_edit_scroll = schedule_edit_sel - 3;  // Ensure selected item is at bottom row
						}
						Draw_Schedule_Edit_Page(schedule_edit_sel);
					}
					delay_ms(100);
				}
			}
			else if(schedule_time_setting_substate == 1)  // 小時編輯模式
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
					schedule_time_setting_substate = 2;  // 分編輯模式
					// Clear all arrows and draw minute arrows only
					Draw_Schedule_Time_Setting_ClearArrows();
					Draw_Schedule_Time_Setting_MinArrows(1);
					delay_ms(100);
				}
			}
			else if(schedule_time_setting_substate == 2)  // 分編輯模式
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
					schedule_time_setting_substate = 3;  // 溫度編輯模式
					// Clear all arrows and draw temperature arrows only
					Draw_Schedule_Time_Setting_ClearArrows();
					Draw_Schedule_Time_Setting_TempArrows(1);
					delay_ms(100);
				}
			}
			else if(schedule_time_setting_substate == 3)  // 溫度編輯模式
			{
				if(key == 1)  // Up key - increase temperature
				{
					if(schedule_time_edit_temp < 45) schedule_time_edit_temp++;
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
					schedule_time_setting_substate = 4;  // ON/OFF編輯模式
					// Clear all arrows and update ON/OFF section
					Draw_Schedule_Time_Setting_ClearArrows();
					Draw_Schedule_Time_Setting_OnOff();
					delay_ms(100);
				}
			}
			else if(schedule_time_setting_substate == 4)  // ON/OFF編輯模式
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
					schedule_time_setting_substate = 5;  // Save模式
					// Update ON/OFF section (exit edit mode) and Save button
					Draw_Schedule_Time_Setting_OnOff();
					Draw_Schedule_Time_Setting_Save();
					delay_ms(100);
				}
			}
			else if(schedule_time_setting_substate == 5)  // Save mode
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
					system_state = STATE_SCHEDULE_EDIT;
					schedule_edit_substate = 1;
					schedule_edit_sel = schedule_time_setting_period - 1;  // P1-P6 -> 0-5
					// Set scroll based on selection to ensure correct rows are displayed
					if(schedule_edit_sel <= 3) {
						schedule_edit_scroll = 0;  // P1-P4 visible
					} else {
						schedule_edit_scroll = schedule_edit_sel - 3;  // Ensure selected item is at bottom row
					}
					Draw_Schedule_Edit_Page(schedule_edit_sel);
					delay_ms(100);
				}
			}
		}
		else if(system_state == STATE_CONTROL_ADJ_MENU)
		{
			// Control Adj Menu page navigation
			if(control_adj_menu_substate == 0)  // TopBar mode: Edit/Leave
			{
				if(key == 1 || key == 2)  // Up or Down key pressed
				{
					// Toggle between Edit red and Leave red
					control_adj_menu_top_sel = !control_adj_menu_top_sel;
					// Only update Top Bar
					Draw_Generic_TopBar(control_adj_menu_top_sel ? 0 : 1, control_adj_menu_top_sel ? 1 : 0);
					delay_ms(100);
				}
				else if(key == 3)  // Enter key pressed
				{
					if(control_adj_menu_top_sel)  // Edit Icon is red
					{
						// Enter選項模式, Edit/Leave both black, Sensor selected
						control_adj_menu_substate = 1;
						control_adj_menu_sel = 0;     // Start at Sensor
						control_adj_menu_scroll = 0;  // Reset scroll
						Draw_Control_Adj_Menu_Page(control_adj_menu_sel);
					}
					else  // Leave Icon is red
					{
						// Go back to Function Setting Edit mode with cursor on Control Adj
						system_state = STATE_FUNC_SETTING_EDIT;
						func_setting_sel = 1;  // Control Adj
						Draw_Function_Setting_Edit_Page(func_setting_sel);
					}
					delay_ms(100);
				}
			}
			else if(control_adj_menu_substate == 1)  // 選項模式: Sensor/Temp.Correct/Temp.Limit/Temp.Protect/Power On State
			{
				if(key == 2)  // Down key pressed
				{
					if(control_adj_menu_sel < 4)  // Can go down (max=4, Power On State)
					{
						uint8_t old_sel = control_adj_menu_sel;
						uint8_t old_scroll = control_adj_menu_scroll;
						control_adj_menu_sel++;
						
						// Check if scroll position needs to change
						// 4 visible rows at a time, scroll when selection goes below visible area
						if(control_adj_menu_sel > control_adj_menu_scroll + 3) {
							control_adj_menu_scroll = control_adj_menu_sel - 3;
						}
						
						// If scroll changed, redraw entire page
						if(old_scroll != control_adj_menu_scroll) {
							Draw_Control_Adj_Menu_Page(control_adj_menu_sel);
						} else {
							// Only update changed rows
							Draw_Control_Adj_Menu_Row(old_sel - control_adj_menu_scroll, 0);  // Old row unselected
							Draw_Control_Adj_Menu_Row(control_adj_menu_sel - control_adj_menu_scroll, 1);  // New row selected
						}
					}
					delay_ms(100);
				}
				else if(key == 1)  // Up key pressed
				{
					if(control_adj_menu_sel > 0)
					{
						uint8_t old_sel = control_adj_menu_sel;
						uint8_t old_scroll = control_adj_menu_scroll;
						control_adj_menu_sel--;
						
						// Check if scroll position needs to change
						if(control_adj_menu_sel < control_adj_menu_scroll) {
							control_adj_menu_scroll = control_adj_menu_sel;
						}
						
						// If scroll changed, redraw entire page
						if(old_scroll != control_adj_menu_scroll) {
							Draw_Control_Adj_Menu_Page(control_adj_menu_sel);
						} else {
							// Only update changed rows
							Draw_Control_Adj_Menu_Row(old_sel - control_adj_menu_scroll, 0);  // Old row unselected
							Draw_Control_Adj_Menu_Row(control_adj_menu_sel - control_adj_menu_scroll, 1);  // New row selected
						}
					}
					else  // At Sensor (sel=0), go back to TopBar mode
					{
						control_adj_menu_substate = 0;
						control_adj_menu_top_sel = 1;  // Edit red
						// Redraw entire page to show TopBar with Edit red
						Draw_Control_Adj_Menu_Page(0xFF);  // 0xFF = TopBar mode, Edit red
					}
					delay_ms(100);
				}
				else if(key == 3)  // Enter key pressed
				{
					if(control_adj_menu_sel == 0)  // Sensor selected
					{
						// Enter Sensor Setting頁面
						system_state = STATE_CONTROL_ADJ_SENSOR;
						control_adj_sensor_substate = 0;   // Start with TopBar mode
						control_adj_sensor_top_sel = 1;    // Edit red by default
						control_adj_sensor_sel = current_sensor_type;  // 預設選擇目前設定的感測器類型
						Draw_Control_Adj_Sensor_Page(0xFF);  // 0xFF = TopBar mode, Edit red
					}
					// TODO: 其他選項的子頁面處理 (Temp.Correct, Temp.Limit, Temp.Protect, Power On State)
					delay_ms(100);
				}
			}
		}
		else if(system_state == STATE_CONTROL_ADJ_SENSOR)
		{
			// Control Adj Sensor Setting page navigation
			if(control_adj_sensor_substate == 0)  // TopBar mode: Edit/Leave
			{
				if(key == 1 || key == 2)  // Up or Down key pressed
				{
					// Toggle between Edit red and Leave red
					control_adj_sensor_top_sel = !control_adj_sensor_top_sel;
					// Redraw page with new Top Bar state
					if(control_adj_sensor_top_sel) {
						Draw_Control_Adj_Sensor_Page(0xFF);  // Edit red
					} else {
						Draw_Control_Adj_Sensor_Page(0xFE);  // Leave red
					}
					delay_ms(100);
				}
				else if(key == 3)  // Enter key pressed
				{
					if(control_adj_sensor_top_sel)  // Edit Icon is red
					{
						// Enter Floor/Room選項模式
						control_adj_sensor_substate = 1;
						Draw_Control_Adj_Sensor_Page(control_adj_sensor_sel);
					}
					else  // Leave Icon is red
					{
						// Go back to Control Adj Menu with Leave red
						system_state = STATE_CONTROL_ADJ_MENU;
						control_adj_menu_substate = 0;
						control_adj_menu_top_sel = 0;  // Leave red
						Draw_Control_Adj_Menu_Page(0xFE);  // 0xFE = TopBar mode, Leave red
					}
					delay_ms(100);
				}
			}
			else if(control_adj_sensor_substate == 1)  // Room/Floor選項模式 (swapped order)
			{
				if(key == 2)  // Down key pressed
				{
					if(control_adj_sensor_sel < 1)  // Can go to Floor (1)
					{
						// Move selection down (Room -> Floor)
						control_adj_sensor_sel++;
						// Update choices display
						Draw_Control_Adj_Sensor_Choices(control_adj_sensor_sel);
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
						Draw_Control_Adj_Sensor_Choices(control_adj_sensor_sel);
					}
					else  // At Room, go back to TopBar mode
					{
						control_adj_sensor_substate = 0;
						control_adj_sensor_top_sel = 1;  // Edit red
						Draw_Control_Adj_Sensor_Page(0xFF);  // 0xFF = TopBar mode, Edit red
					}
					delay_ms(100);
				}
				else if(key == 3)  // Enter key pressed
				{
					// Enter Save mode
					control_adj_sensor_substate = 2;
					Draw_Control_Adj_Sensor_Save(1);  // Save red
					delay_ms(100);
				}
			}
			else if(control_adj_sensor_substate == 2)  // Save模式
			{
				if(key == 3)  // Enter key pressed
				{
					// Save setting and return to Control Adj Menu
					current_sensor_type = control_adj_sensor_sel;  // 0=Room, 1=Floor (swapped order)
					system_state = STATE_CONTROL_ADJ_MENU;
					control_adj_menu_substate = 0;
					control_adj_menu_top_sel = 0;  // Leave red
					Draw_Control_Adj_Menu_Page(0xFE);  // 0xFE = TopBar mode, Leave red
					delay_ms(100);
				}
				else if(key == 1 || key == 2)  // Up or Down key in Save mode - cancel save
				{
					// Cancel Save mode, back to choices mode
					control_adj_sensor_substate = 1;
					Draw_Control_Adj_Sensor_Choices(control_adj_sensor_sel);
					delay_ms(100);
				}
			}
		}

		delay_ms(10);  // Main loop delay (10ms)
	}
}

// Initialize GPIO for keys
void Key_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	// Enable clocks
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);

	// PA0 - Up Key (Input with pull-down)
	GPIO_InitStructure.GPIO_Pin = UP_KEY_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;  // Input pull-down
	GPIO_Init(UP_KEY_PORT, &GPIO_InitStructure);

	// PC13 - Down Key (Input with pull-down)
	GPIO_InitStructure.GPIO_Pin = DOWN_KEY_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;  // Input pull-down
	GPIO_Init(DOWN_KEY_PORT, &GPIO_InitStructure);

	// PA4 - Enter Key (Input with pull-down)
	GPIO_InitStructure.GPIO_Pin = ENTER_KEY_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;  // Input pull-down
	GPIO_Init(ENTER_KEY_PORT, &GPIO_InitStructure);
}

// Initialize TIM4 CH4 (PB9) for backlight PWM control
// PWM frequency: 500Hz
void Backlight_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;

	// Enable clocks
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  // GPIOB clock
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);   // TIM4 clock

	// Configure PB9 as alternate function push-pull (TIM4_CH4)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  // Alternate function push-pull
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// TIM4 time base configuration
	// System clock: 72MHz
	// PWM frequency: 500Hz
	// Prescaler = 0 (no division), Period = (72MHz / 500Hz) - 1 = 143999
	// But TIM4 is 16-bit timer, max period is 65535
	// So we use Prescaler = 1 (divide by 2), Period = (72MHz / 2 / 500Hz) - 1 = 71999
	// Still too big, use Prescaler = 2 (divide by 3), Period = (72MHz / 3 / 500Hz) - 1 = 47999
	// Let's use Prescaler = 71 (divide by 72), Period = (72MHz / 72 / 500Hz) - 1 = 1999
	TIM_TimeBaseStructure.TIM_Period = 1999;           // Auto-reload value (2000 counts)
	TIM_TimeBaseStructure.TIM_Prescaler = 71;          // Prescaler = 72 (71 + 1)
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

	// TIM4 PWM Mode configuration: Channel 4
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;                 // Initial duty cycle = 0
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;  // High active
	TIM_OC4Init(TIM4, &TIM_OCInitStructure);

	// Enable TIM4 preload register
	TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);
	TIM_ARRPreloadConfig(TIM4, ENABLE);

	// Enable TIM4 counter
	TIM_Cmd(TIM4, ENABLE);
}

// Set backlight PWM duty cycle
// duty_percent: 0-100
void Backlight_SetDuty(uint8_t duty_percent)
{
	uint16_t pulse;

	// Clamp duty cycle to 0-100
	if(duty_percent > 100) duty_percent = 100;

	// Calculate pulse value
	// Period = 1999, so valid pulse range is 0-1999
	pulse = (uint16_t)((duty_percent * 2000) / 100);
	if(pulse > 1999) pulse = 1999;

	// Update TIM4 Channel 4 pulse value
	TIM_SetCompare4(TIM4, pulse);
}

// Scan keys
// Return: 0 = no key, 1 = Up key, 2 = Down key, 3 = Enter key
uint8_t Key_Scan(void)
{
	static uint8_t key_up = 1;  // Key release flag
	static uint8_t key_release_cnt = 0;  // Key release debounce counter

	if(key_up)
	{
		if(GPIO_ReadInputDataBit(UP_KEY_PORT, UP_KEY_PIN) == 1)
		{
			delay_ms(5);  // Debounce (reduced from 10ms)
			if(GPIO_ReadInputDataBit(UP_KEY_PORT, UP_KEY_PIN) == 1)
			{
				key_up = 0;
				key_release_cnt = 0;
				return 1;  // Up key pressed
			}
		}
		else if(GPIO_ReadInputDataBit(DOWN_KEY_PORT, DOWN_KEY_PIN) == 1)
		{
			delay_ms(5);  // Debounce (reduced from 10ms)
			if(GPIO_ReadInputDataBit(DOWN_KEY_PORT, DOWN_KEY_PIN) == 1)
			{
				key_up = 0;
				key_release_cnt = 0;
				return 2;  // Down key pressed
			}
		}
		else if(GPIO_ReadInputDataBit(ENTER_KEY_PORT, ENTER_KEY_PIN) == 1)
		{
			delay_ms(5);  // Debounce (reduced from 10ms)
			if(GPIO_ReadInputDataBit(ENTER_KEY_PORT, ENTER_KEY_PIN) == 1)
			{
				key_up = 0;
				key_release_cnt = 0;
				return 3;  // Enter key pressed
			}
		}
	}
	else
	{
		// Wait for key release with debounce
		if(GPIO_ReadInputDataBit(UP_KEY_PORT, UP_KEY_PIN) == 0 &&
		   GPIO_ReadInputDataBit(DOWN_KEY_PORT, DOWN_KEY_PIN) == 0 &&
		   GPIO_ReadInputDataBit(ENTER_KEY_PORT, ENTER_KEY_PIN) == 0)
		{
			key_release_cnt++;
			if(key_release_cnt >= 3)  // Require 3 consecutive scans to confirm release
			{
				key_up = 1;
				key_release_cnt = 0;
			}
		}
		else
		{
			key_release_cnt = 0;
		}
	}

	return 0;  // No key
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

// Draw Icon6 in normal or red color using 24x24 monochrome icon
// red: 1 = draw in RED, 0 = draw in BLACK
void Draw_Icon6_Red(uint8_t red)
{
	u16 x = 136;  // Match Draw_Static_Icons() position
	u16 y = 85;   // Match Draw_Static_Icons() position
	u16 color = red ? RED : BLACK;
	
	// Clear the icon area first (24x24 pixels)
	LCD_Fill(x, y, x + 23, y + 23, WHITE);
	
	// Draw Icon6 with specified color using 24x24 monochrome icon
	GUI_DrawMonoIcon24x24(x, y, color, WHITE, Icon06_24x24);
}

// Draw Function Setting page
// This page shows:
// - Top-left: Icon12 (Leave Icon) / Icon13 (Leave Icon red)
// - Top-right: Icon14 (Edit Icon) / Icon15 (Edit Icon red)
// - Row 2: Icon16 + "Heating Schedule"
// - Row 3: Icon17 + "Control Adjustment"
// - Row 4: Icon18 + "User Settings"
// edit_red: 1 = Edit Icon red, 0 = Leave Icon red
// Draw only the Top Bar icons for Function Setting (for cursor movement update)
// This is faster than redrawing the entire page
void Draw_Function_Setting_TopBar(uint8_t edit_red)
{
	// Only clear the Top Bar area (y=0 to y=25)
	LCD_Fill(0, 0, lcddev.width, 25, WHITE);
	
	// Draw top-left Leave Icon
	if(edit_red)
	{
		GUI_DrawMonoIcon16x16(5, 5, BLACK, WHITE, Icon16x16_Leave);  // Leave black
	}
	else
	{
		GUI_DrawMonoIcon16x16(5, 5, RED, WHITE, Icon16x16_Leave);   // Leave red
	}

	// Draw top-right Edit Icon
	if(edit_red)
	{
		GUI_DrawMonoIcon16x16(144, 5, RED, WHITE, Icon16x16_Edit);  // Edit red
	}
	else
	{
		GUI_DrawMonoIcon16x16(144, 5, BLACK, WHITE, Icon16x16_Edit);  // Edit black
	}
}

void Draw_Function_Setting_Page(uint8_t edit_red)
{
	// Clear screen
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);

	// Draw top-left Leave Icon (Icon12 black or Icon13 red)
	if(edit_red)
	{
		GUI_DrawMonoIcon16x16(5, 5, BLACK, WHITE, Icon16x16_Leave);  // Leave black
	}
	else
	{
		GUI_DrawMonoIcon16x16(5, 5, RED, WHITE, Icon16x16_Leave);   // Leave red
	}

	// Draw top-right Edit Icon (Icon14 black or Icon15 red)
	if(edit_red)
	{
		GUI_DrawMonoIcon16x16(144, 5, RED, WHITE, Icon16x16_Edit);  // Edit red
	}
	else
	{
		GUI_DrawMonoIcon16x16(144, 5, BLACK, WHITE, Icon16x16_Edit);  // Edit black
	}

	// Draw Row 2: Icon16 + "Heating Schedule" (y=28, closer to top)
	GUI_DrawMonoIcon8x16(10, 30, BLACK, WHITE, Icon16_8x16);
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	Show_Str(30, 28, BLACK, WHITE, "Heating Schedule", 16, 0);

	// Draw Row 3: Icon17 + "Control Adj." (y=52, smaller gap)
	GUI_DrawMonoIcon8x16(10, 52, BLACK, WHITE, Icon17_8x16);
	Show_Str(30, 52, BLACK, WHITE, "Control Adj.", 16, 0);

	// Draw Row 4: Icon18 + "User Settings" (y=76, smaller gap)
	GUI_DrawMonoIcon8x16(10, 76, BLACK, WHITE, Icon18_8x16);
	Show_Str(30, 76, BLACK, WHITE, "User Settings", 16, 0);
}

// Draw Main page (return from Function Setting)
void Draw_Main_Page(void)
{
	// Clear screen
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);

	// Display static elements (time, icons)
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	GUI_DrawBigDigit(5, 5, BLACK, WHITE, '1', 1);
	GUI_DrawBigDigit(21, 5, BLACK, WHITE, '0', 1);
	// colon
	LCD_Fill(40, 15, 43, 18, BLACK);
	LCD_Fill(40, 25, 43, 28, BLACK);
	GUI_DrawBigDigit(47, 5, BLACK, WHITE, '3', 1);
	GUI_DrawBigDigit(63, 5, BLACK, WHITE, '2', 1);
	Draw_Static_Icons();

	// Draw black Manu Icon
	Draw_Icon6_Red(0);

	// Display current number
	Display_Number(setting_number, BLACK, 1);
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
// - Top-left: Leave Icon (black)
// - Top-right: Edit Icon (black)
// - Row 1: Icon16 + "Heating Schedule" (selected=white on red, normal=black)
// - Row 2: Icon17 + "Control Adj." (selected=white on red, normal=black)
// - Row 3: Icon18 + "User Settings" (selected=white on red, normal=black)
// selection: 0=Heating Schedule, 1=Control Adj, 2=User Settings
void Draw_Function_Setting_Edit_Page(uint8_t selection)
{
	// Clear screen
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);

	// Draw Top Bar (both icons black in edit mode)
	GUI_DrawMonoIcon16x16(5, 5, BLACK, WHITE, Icon16x16_Leave);
	GUI_DrawMonoIcon16x16(144, 5, BLACK, WHITE, Icon16x16_Edit);

	// Draw all three rows
	Draw_Function_Setting_Edit_Row(0, (selection == 0) ? 1 : 0);
	Draw_Function_Setting_Edit_Row(1, (selection == 1) ? 1 : 0);
	Draw_Function_Setting_Edit_Row(2, (selection == 2) ? 1 : 0);
}

// Draw a single row in Heating Schedule page (for optimized update)
// row: 0=Heating Schedule, 1=Control Adj, 2=User Settings
// selected: 1=selected (red bg), 0=not selected
void Draw_Heating_Schedule_Row(uint8_t row, uint8_t selected)
{
	uint16_t bg_y_start[] = {26, 50, 74};  // Background Y start for each row
	uint16_t bg_y_end[] = {46, 70, 94};    // Background Y end for each row
	uint16_t text_color, bg_color;
	
	if(selected) {
		text_color = WHITE;
		bg_color = RED;
		// Draw red background
		LCD_Fill(0, bg_y_start[row], lcddev.width, bg_y_end[row], RED);
	} else {
		text_color = BLACK;
		bg_color = WHITE;
		// Clear background to white
		LCD_Fill(0, bg_y_start[row], lcddev.width, bg_y_end[row], WHITE);
	}
	
	// Draw the row content based on row number
	// Icon and text moved down by 2 pixels to center in the background row
	if(row == 0) {
		GUI_DrawMonoIcon8x16(12, 30, text_color, bg_color, Icon16_8x16);
		POINT_COLOR = text_color;
		BACK_COLOR = bg_color;
		Show_Str(26, 30, text_color, bg_color, "Heating Schedule", 16, 0);
	} else if(row == 1) {
		GUI_DrawMonoIcon8x16(12, 54, text_color, bg_color, Icon17_8x16);
		POINT_COLOR = text_color;
		BACK_COLOR = bg_color;
		Show_Str(26, 54, text_color, bg_color, "Control Adj.", 16, 0);
	} else {
		GUI_DrawMonoIcon8x16(12, 78, text_color, bg_color, Icon18_8x16);
		POINT_COLOR = text_color;
		BACK_COLOR = bg_color;
		Show_Str(26, 78, text_color, bg_color, "User Settings", 16, 0);
	}
}

// Draw only the Top Bar for Heating Schedule page (for initial draw or full refresh)
void Draw_Heating_Schedule_TopBar(void)
{
	// Only clear the Top Bar area (y=0 to y=25)
	LCD_Fill(0, 0, lcddev.width, 25, WHITE);
	
	// Draw top-left Leave Icon (black)
	GUI_DrawMonoIcon16x16(5, 5, BLACK, WHITE, Icon16x16_Leave);

	// Draw top-right Edit Icon (black)
	GUI_DrawMonoIcon16x16(144, 5, BLACK, WHITE, Icon16x16_Edit);
}

// Draw Heating Schedule page
// - Leave Icon (black), Edit Icon (black)
// - Row 2: Icon16 + "Heating Schedule" (selected=white on red, normal=black)
// - Row 3: Icon17 + "Control Adjustment" (selected=white on red, normal=black)
// - Row 4: Icon18 + "User Settings" (selected=white on red, normal=black)
// selection: 0=Heating Schedule, 1=Control Adjustment, 2=User Settings
void Draw_Heating_Schedule_Page(uint8_t selection)
{
	// Clear screen
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);

	// Draw Top Bar
	Draw_Heating_Schedule_TopBar();

	// Draw all three rows
	Draw_Heating_Schedule_Row(0, (selection == 0) ? 1 : 0);
	Draw_Heating_Schedule_Row(1, (selection == 1) ? 1 : 0);
	Draw_Heating_Schedule_Row(2, (selection == 2) ? 1 : 0);
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

// Draw a filled circle
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

// Draw Heating Schedule Program Type page
// Draw only the Top Bar for Heating Schedule Prog Type page (for Top Bar mode update)
// Generic Top Bar update function for pages with Leave/Edit icons
// leave_red: 1=Leave red, 0=Leave black
// edit_red: 1=Edit red, 0=Edit black
void Draw_Generic_TopBar(uint8_t leave_red, uint8_t edit_red)
{
	// Only clear the Top Bar area (y=0 to y=25)
	LCD_Fill(0, 0, lcddev.width, 25, WHITE);
	
	// Draw Leave Icon
	if(leave_red) {
		GUI_DrawMonoIcon16x16(5, 5, RED, WHITE, Icon16x16_Leave);  // Leave Red
	} else {
		GUI_DrawMonoIcon16x16(5, 5, BLACK, WHITE, Icon16x16_Leave); // Leave Black
	}

	// Draw Edit Icon
	if(edit_red) {
		GUI_DrawMonoIcon16x16(144, 5, RED, WHITE, Icon16x16_Edit);  // Edit Red
	} else {
		GUI_DrawMonoIcon16x16(144, 5, BLACK, WHITE, Icon16x16_Edit);  // Edit Black
	}
}

void Draw_Heating_Schedule_Prog_Type_TopBar(void)
{
	// Use generic Top Bar with prog_type state
	uint8_t leave_red = (prog_type_substate == 0 && prog_type_top_sel == 0) ? 1 : 0;
	uint8_t edit_red = (prog_type_substate == 0 && prog_type_top_sel == 1) ? 1 : 0;
	Draw_Generic_TopBar(leave_red, edit_red);
}

// Draw only the Type Choices area (5+2/6+1/7) for Heating Schedule Prog Type page
// old_sel: previously selected type (0=5+2, 1=6+1, 2=7)
// new_sel: newly selected type
void Draw_Heating_Schedule_Prog_Type_Choices(uint8_t old_sel, uint8_t new_sel)
{
	char* type_strs[] = {"5+2", "6+1", "7"};
	uint16_t x_pos[] = {20, 70, 120};
	uint8_t i;
	
	// Update both old and new selection
	for(i = 0; i < 2; i++) {
		uint8_t sel = (i == 0) ? old_sel : new_sel;
		uint16_t text_color = BLACK;
		uint16_t bg_color = WHITE;
		uint16_t box_width = 24;
		uint16_t text_w = (sel == 2) ? 8 : 24;
		uint16_t text_x = x_pos[sel] + (box_width - text_w) / 2;
		
		// Clear this choice area (enlarged to cover pill shape completely)
		// Pill shape: radius 10, center at y=60, so y range is 50-70
		// Add extra margin to ensure complete clearing
		LCD_Fill(x_pos[sel] - 15, 45, x_pos[sel] + 45, 75, WHITE);
		
		// If this is the new selection, draw Red Pill Shape
		if(i == 1) {
			uint16_t y_center = 60;
			uint8_t r = 10;
			uint16_t px1 = x_pos[sel] - 1;
			uint16_t px2 = x_pos[sel] + box_width + 1;
			
			text_color = WHITE;
			bg_color = RED;
			
			// Draw red pill shape
			LCD_FillCircle(px1, y_center, r, RED);
			LCD_FillCircle(px2, y_center, r, RED);
			LCD_Fill(px1, y_center - r, px2, y_center + r, RED);
		}
		
		POINT_COLOR = text_color;
		BACK_COLOR = bg_color;
		Show_Str(text_x, 52, text_color, bg_color, (u8*)type_strs[sel], 16, 0);
	}
}

void Draw_Heating_Schedule_Prog_Type_Page(void)
{
	int i;
	char* type_strs[] = {"5+2", "6+1", "7"};
	uint16_t x_pos[] = {20, 70, 120};
	uint16_t save_color;

	// Clear screen
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);

	// Draw Top Bar
	Draw_Heating_Schedule_Prog_Type_TopBar();

	// Draw Current Type Text (Size 16, Moved up)
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	Show_Str(10, 25, BLACK, WHITE, (u8*)"Current Type: ", 16, 0);
	Show_Str(122, 25, BLUE, WHITE, (u8*)type_strs[current_prog_type], 16, 0);

	// Draw Choices (5+2, 6+1, 7)
	for (i = 0; i < 3; i++) {
		uint16_t text_color = BLACK;
		uint16_t bg_color = WHITE;
		uint16_t box_width = 24; // Force same pill width for all options
		uint16_t text_w = (i == 2) ? 8 : 24; // text "7" is 8px wide, others are 24px
		uint16_t text_x = x_pos[i] + (box_width - text_w) / 2; // Center text in pill

		// If editing and this is the selected item, draw Red Pill Shape
		if ((prog_type_substate == 1 || prog_type_substate == 2) && prog_type_edit_sel == i) {
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
		Show_Str(text_x, 52, text_color, bg_color, (u8*)type_strs[i], 16, 0);
	}

	// Draw Save text/button
	save_color = (prog_type_substate == 2) ? RED : BLACK;
	Show_Str(115, 105, save_color, WHITE, (u8*)"Save", 16, 0);
}

// Draw Schedule Edit page
// This page shows:
// - Top-left: Leave Icon (black/red)
// - Top-right: Edit Icon (black/red)
// - Row 1: P1 + Arrow Icon + Time + Temperature + Hook/X Icon
// - Row 2: P2 + Arrow Icon + Time + Temperature + Hook/X Icon
// - Row 3: P3 + Arrow Icon + Time + Temperature + Hook/X Icon
// - Bottom: "Next >>" text
// selection: 0=P1, 1=P2, 2=P3, 3=Next, 0xFF=TopBar Edit red, 0xFE=TopBar Leave red
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
	Show_Str(10, row_y[row], BLACK, WHITE, (u8*)period_name, 16, 0);
	
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
	Show_Str(55, row_y[row], text_color, WHITE, (u8*)time_str, 16, 0);
	
	// Draw Temperature
	Show_Str(100, row_y[row], text_color, WHITE, (u8*)temp_str, 16, 0);
	
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
	Show_Str(100, row_y + 4, finish_color, WHITE, (u8*)"Finish", 16, 0);
}

// Draw Schedule Edit Page with scrolling (P1-P6)
// This page shows:
// - Top-left: Leave Icon (black/red)
// - Top-right: Edit Icon (black/red)
// - 4 rows showing P1-P4, P2-P5, or P3-P6 based on scroll position
// - Finish button at bottom
// selection: 0-5=P1-P6, 6=Finish, 0xFF=TopBar Edit red, 0xFE=TopBar Leave red
void Draw_Schedule_Edit_Page(uint8_t selection)
{
	uint8_t i;
	uint8_t topbar_mode = 0;
	uint8_t edit_red = 0;
	uint8_t leave_red = 0;
	
	// Handle special selection values for TopBar mode
	if(selection == 0xFF) {
		topbar_mode = 1;
		edit_red = 1;
	} else if(selection == 0xFE) {
		topbar_mode = 1;
		leave_red = 1;
	}
	
	// Note: schedule_edit_scroll is set by the caller (Down/Up key handler)
	// This function only uses the current scroll value to display the correct rows
	// scroll=0: show P1-P4 (cursor P0-P3), scroll=1: show P2-P5 (cursor P2-P4), scroll=2: show P3-P6+Finish (cursor P4-P6)
	
	// Clear screen
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	
	// Draw Top Bar
	if(leave_red) {
		GUI_DrawMonoIcon16x16(5, 5, RED, WHITE, Icon16x16_Leave);
	} else {
		GUI_DrawMonoIcon16x16(5, 5, BLACK, WHITE, Icon16x16_Leave);
	}
	
	if(edit_red) {
		GUI_DrawMonoIcon16x16(144, 5, RED, WHITE, Icon16x16_Edit);
	} else {
		GUI_DrawMonoIcon16x16(144, 5, BLACK, WHITE, Icon16x16_Edit);
	}
	
	// Draw rows based on scroll position
	// scroll=0: show P1-P4, scroll=1: show P2-P5, scroll=2: show P3-P6/Finish
	for(i = 0; i < 4; i++) {
		uint8_t period_idx = schedule_edit_scroll + i;  // 0-5
		uint8_t is_selected = 0;
		
		// Check if this is the Finish position (when period_idx > 5)
		if(period_idx > 5) {
			// This is the Finish row position
			if(!topbar_mode && (selection == 6)) {
				Draw_Schedule_Edit_Finish(1);  // Finish selected
			} else {
				Draw_Schedule_Edit_Finish(0);  // Finish not selected
			}
		} else {
			// Regular period row
			if(!topbar_mode && (period_idx == selection)) {
				is_selected = 1;
			}
			Draw_Schedule_Edit_Row(i, period_idx, is_selected);
		}
	}
}



// Draw only the Top Bar for Schedule Time Setting page (optimized update)
// This function only updates the Top Bar area without redrawing the entire page
// period_num: 1~6 (P1~P6)
void Draw_Schedule_Time_Setting_TopBar(uint8_t period_num)
{
	char period_text[10];
	
	// Only clear the Top Bar area (y=0 to y=25)
	LCD_Fill(0, 0, lcddev.width, 25, WHITE);
	
	// Draw Top Bar
	if(schedule_time_setting_substate == 0 && !schedule_time_setting_top_sel) {
		// Leave red, Edit black
		GUI_DrawMonoIcon16x16(5, 5, RED, WHITE, Icon16x16_Leave);   // Leave red (9x10)
		GUI_DrawMonoIcon16x16(144, 5, BLACK, WHITE, Icon16x16_Edit); // Edit black
	} else if(schedule_time_setting_substate == 0 && schedule_time_setting_top_sel) {
		// Leave black, Edit red
		GUI_DrawMonoIcon16x16(5, 5, BLACK, WHITE, Icon16x16_Leave);   // Leave black
		GUI_DrawMonoIcon16x16(144, 5, RED, WHITE, Icon16x16_Edit); // Edit red
	} else {
		// Both black (in edit mode)
		GUI_DrawMonoIcon16x16(5, 5, BLACK, WHITE, Icon16x16_Leave);   // Leave black
		GUI_DrawMonoIcon16x16(144, 5, BLACK, WHITE, Icon16x16_Edit); // Edit black
	}
	
	// Draw "Period X On/Off" text in the middle of Top Bar
	// ON/OFF follows schedule_time_on_off and changes during ON/OFF edit mode
	if(schedule_time_on_off) {
		sprintf(period_text, "Period %d On", period_num);
	} else {
		sprintf(period_text, "Period %d Off", period_num);
	}
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	Show_Str(35, 5, BLACK, WHITE, (u8*)period_text, 16, 0);
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
void Draw_Schedule_Time_Setting_HourArrows(uint8_t show)
{
	if(show) {
		GUI_DrawMonoIcon16x16(13, 30, WHITE, RED, Icon16x16_Up_Arror);     // Up arrow above hour (red bg)
		GUI_DrawMonoIcon16x16(13, 82, WHITE, RED, Icon16x16_Down_Arror);   // Down arrow below hour (red bg)
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
	if(schedule_time_setting_substate == 4) {
		// ON/OFF edit mode - show selected one in red
		if(schedule_time_on_off) {
			// ON selected - ON red, OFF black
			Show_Str(123, 63, RED, WHITE, (u8*)"ON", 16, 0);
			Show_Str(123, 80, BLACK, WHITE, (u8*)"OFF", 16, 0);
			// Left Arrow Icon next to ON (y=63, aligned with ON) - shifted right by 2 pixels
			GUI_DrawMonoIcon8x16(147, 63, RED, WHITE, IconLeft_Arrow_8x16);
		} else {
			// OFF selected - ON black, OFF red
			Show_Str(123, 63, BLACK, WHITE, (u8*)"ON", 16, 0);
			Show_Str(123, 80, RED, WHITE, (u8*)"OFF", 16, 0);
			// Left Arrow Icon next to OFF (y=80, aligned with OFF) - shifted right by 2 pixels
			GUI_DrawMonoIcon8x16(147, 80, RED, WHITE, IconLeft_Arrow_8x16);
		}
	} else {
		// Not in ON/OFF edit mode - both black
		Show_Str(123, 63, BLACK, WHITE, (u8*)"ON", 16, 0);
		Show_Str(123, 80, BLACK, WHITE, (u8*)"OFF", 16, 0);
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
	if(schedule_time_setting_substate == 5) {
		save_color = RED;  // Save is selected
	} else {
		save_color = BLACK;  // Save is not selected
	}
	POINT_COLOR = save_color;
	BACK_COLOR = WHITE;
	Show_Str(120, 110, save_color, WHITE, (u8*)"Save", 16, 0);
}

// Draw a single row in Heating Schedule Menu page (for optimized update)
// row: 0=Program Type, 1=Workday Setting, 2=Restday Setting
// selected: 1=selected (red bg), 0=not selected
void Draw_Heating_Schedule_Menu_Row(uint8_t row, uint8_t selected)
{
	uint16_t row_y[] = {30, 54, 78};  // Y positions for 3 rows
	uint16_t text_color, bg_color;
	
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
		Show_Str(30, row_y[row] + 4, text_color, bg_color, (u8*)"Program Type", 16, 0);
	} else if(row == 1) {
		// Workday Setting - uses Icon17
		GUI_DrawMonoIcon8x16(12, row_y[row] + 4, text_color, bg_color, Icon17_8x16);
		POINT_COLOR = text_color;
		BACK_COLOR = bg_color;
		Show_Str(30, row_y[row] + 4, text_color, bg_color, (u8*)"Workday Setting", 16, 0);
	} else {
		// Restday Setting - uses Icon18
		GUI_DrawMonoIcon8x16(12, row_y[row] + 4, text_color, bg_color, Icon18_8x16);
		POINT_COLOR = text_color;
		BACK_COLOR = bg_color;
		Show_Str(30, row_y[row] + 4, text_color, bg_color, (u8*)"Restday Setting", 16, 0);
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
void Draw_Heating_Schedule_Menu_Page(uint8_t selection)
{
	uint8_t i;
	uint8_t topbar_mode = 0;
	uint8_t edit_red = 0;
	uint8_t leave_red = 0;
	
	// Handle special selection values for TopBar mode
	// 0xFF = TopBar mode with Edit red
	// 0xFE = TopBar mode with Leave red
	if(selection == 0xFF) {
		topbar_mode = 1;
		edit_red = 1;  // Edit red, Leave black
	} else if(selection == 0xFE) {
		topbar_mode = 1;
		leave_red = 1;  // Leave red, Edit black
	}
	
	// Clear screen
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	
	// Draw Top Bar (Leave and Edit icons)
	if(leave_red) {
		GUI_DrawMonoIcon16x16(5, 5, RED, WHITE, Icon16x16_Leave);   // Leave red
	} else {
		GUI_DrawMonoIcon16x16(5, 5, BLACK, WHITE, Icon16x16_Leave);  // Leave black
	}
	
	if(edit_red) {
		GUI_DrawMonoIcon16x16(144, 5, RED, WHITE, Icon16x16_Edit);  // Edit red
	} else {
		GUI_DrawMonoIcon16x16(144, 5, BLACK, WHITE, Icon16x16_Edit);  // Edit black
	}
	
	// Draw 3 rows (Program Type, Workday Setting, Restday Setting)
	for (i = 0; i < 3; i++) {
		if(!topbar_mode && (i == selection)) {
			Draw_Heating_Schedule_Menu_Row(i, 1);  // Selected
		} else {
			Draw_Heating_Schedule_Menu_Row(i, 0);  // Not selected
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
void Draw_Schedule_Time_Setting_Page(uint8_t period_num)
{
	uint16_t save_color;
	uint8_t hour_high, hour_low, min_high, min_low;
	uint8_t temp_high, temp_low;
	
	// Calculate digits from edit values
	hour_high = schedule_time_edit_hour / 10;
	hour_low = schedule_time_edit_hour % 10;
	min_high = schedule_time_edit_min / 10;
	min_low = schedule_time_edit_min % 10;
	temp_high = schedule_time_edit_temp / 10;
	temp_low = schedule_time_edit_temp % 10;
	
	// Clear screen
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	
	// Draw Top Bar using the optimized function
	Draw_Schedule_Time_Setting_TopBar(period_num);
	
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
	if(schedule_time_setting_substate == 1) {
		GUI_DrawMonoIcon16x16(13, 30, WHITE, RED, Icon16x16_Up_Arror);     // Up arrow above hour (red bg)
		GUI_DrawMonoIcon16x16(13, 82, WHITE, RED, Icon16x16_Down_Arror);   // Down arrow below hour (red bg)
	}
	
	// Draw Up/Down Arrow Icons for minute edit (substate 2)
	// Position: centered above/below the two minute digits (x=45~61, center=53)
	if(schedule_time_setting_substate == 2) {
		GUI_DrawMonoIcon16x16(53, 30, WHITE, RED, Icon16x16_Up_Arror);     // Up arrow above minute (red bg)
		GUI_DrawMonoIcon16x16(53, 82, WHITE, RED, Icon16x16_Down_Arror);   // Down arrow below minute (red bg)
	}
	
	// Draw separator "|" between time and temperature (small font)
	Show_Str(80, 60, BLACK, WHITE, (u8*)"|", 16, 0);
	
	// Draw Temperature with 16x32 font
	GUI_DrawBigDigit(88, 50, BLACK, WHITE, '0' + temp_high, 1);
	GUI_DrawBigDigit(104, 50, BLACK, WHITE, '0' + temp_low, 1);
	
	// Draw Up/Down Arrow Icons for temperature edit (substate 3)
	// Position: centered above/below the two temperature digits (x=88~104, center=96)
	if(schedule_time_setting_substate == 3) {
		GUI_DrawMonoIcon16x16(96, 30, WHITE, RED, Icon16x16_Up_Arror);     // Up arrow above temperature (red bg)
		GUI_DrawMonoIcon16x16(96, 82, WHITE, RED, Icon16x16_Down_Arror);   // Down arrow below temperature (red bg)
	}
	
	// Draw degree C (small icon at top-right of temperature)
	GUI_DrawMonoIcon16x16(121, 45, BLACK, WHITE, Icon16x16_C);
	
	// Draw ON/OFF text below degree C
	// ON at y=63, OFF at y=80
	// Default: both black when not in ON/OFF edit mode
	if(schedule_time_setting_substate == 4) {
		// ON/OFF edit mode - show selected one in red - shifted right by 2 pixels
		if(schedule_time_on_off) {
			// ON selected - ON red, OFF black
			Show_Str(123, 63, RED, WHITE, (u8*)"ON", 16, 0);
			Show_Str(123, 80, BLACK, WHITE, (u8*)"OFF", 16, 0);
			// Left Arrow Icon next to ON (y=63, aligned with ON) - shifted right by 2 pixels
			GUI_DrawMonoIcon8x16(147, 63, RED, WHITE, IconLeft_Arrow_8x16);
		} else {
			// OFF selected - ON black, OFF red
			Show_Str(123, 63, BLACK, WHITE, (u8*)"ON", 16, 0);
			Show_Str(123, 80, RED, WHITE, (u8*)"OFF", 16, 0);
			// Left Arrow Icon next to OFF (y=80, aligned with OFF) - shifted right by 2 pixels
			GUI_DrawMonoIcon8x16(147, 80, RED, WHITE, IconLeft_Arrow_8x16);
		}
	} else {
		// Not in ON/OFF edit mode - both black - shifted right by 2 pixels
		Show_Str(123, 63, BLACK, WHITE, (u8*)"ON", 16, 0);
		Show_Str(123, 80, BLACK, WHITE, (u8*)"OFF", 16, 0);
		// Left Arrow Icon stays at the last selected position (ON or OFF) - shifted right by 2 pixels
		if(schedule_time_on_off) {
			GUI_DrawMonoIcon8x16(147, 63, RED, WHITE, IconLeft_Arrow_8x16);  // Next to ON
		} else {
			GUI_DrawMonoIcon8x16(147, 80, RED, WHITE, IconLeft_Arrow_8x16);  // Next to OFF
		}
	}
	
	// Draw "Save" at bottom right
	if(schedule_time_setting_substate == 5) {
		save_color = RED;  // Save is selected
	} else {
		save_color = BLACK;  // Save is not selected
	}
	POINT_COLOR = save_color;
	BACK_COLOR = WHITE;
	Show_Str(120, 110, save_color, WHITE, (u8*)"Save", 16, 0);
}

// ============================================================
// Control Adj Menu Page Drawing Functions
// ============================================================

// Draw a single row in Control Adj Menu page (for optimized update)
// row: 0-3 (display row position on screen)
// selected: 1=selected (red arrow bg), 0=not selected (black arrow bg)
void Draw_Control_Adj_Menu_Row(uint8_t row, uint8_t selected)
{
	uint16_t row_y[] = {30, 54, 78, 102};  // Y positions for 4 display rows
	uint16_t arrow_bg_color;
	char* menu_texts[] = {"Sensor", "Temp. Correct", "Temp. Limit", "Temp. Protect", "Power On State"};
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
	Show_Str(30, row_y[row] + 4, BLACK, WHITE, (u8*)menu_texts[text_idx], 16, 0);
}

// Draw Control Adj Menu Page
// This page shows:
// - Top-left: Leave Icon (black/red)
// - Top-right: Edit Icon (black/red)
// - 4 visible rows (out of 5 total) with scrolling
// - Arrow Icon on the left of each row (red bg for selected, black bg for others)
// - 5 menu items: Sensor / Temp. Correct / Temp. Limit / Temp. Protect / Power On State
// selection: 0-4=options, 0xFF=TopBar Edit red, 0xFE=TopBar Leave red
void Draw_Control_Adj_Menu_Page(uint8_t selection)
{
	uint8_t i;
	uint8_t topbar_mode = 0;
	uint8_t edit_red = 0;
	uint8_t leave_red = 0;
	uint8_t num_visible_rows;
	
	// Handle special selection values for TopBar mode
	if(selection == 0xFF) {
		topbar_mode = 1;
		edit_red = 1;
	} else if(selection == 0xFE) {
		topbar_mode = 1;
		leave_red = 1;
	}
	
	// Ensure scroll is valid (0 or 1 for 5 items with 4 visible)
	if(control_adj_menu_scroll > 1) control_adj_menu_scroll = 1;
	
	// Clear screen
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	
	// Draw Top Bar
	if(leave_red) {
		GUI_DrawMonoIcon16x16(5, 5, RED, WHITE, Icon16x16_Leave);
	} else {
		GUI_DrawMonoIcon16x16(5, 5, BLACK, WHITE, Icon16x16_Leave);
	}
	
	if(edit_red) {
		GUI_DrawMonoIcon16x16(144, 5, RED, WHITE, Icon16x16_Edit);
	} else {
		GUI_DrawMonoIcon16x16(144, 5, BLACK, WHITE, Icon16x16_Edit);
	}
	
	// Calculate how many rows to draw (4 or less if near end)
	num_visible_rows = 5 - control_adj_menu_scroll;
	if(num_visible_rows > 4) num_visible_rows = 4;
	
	// Draw rows based on scroll position
	for(i = 0; i < num_visible_rows; i++) {
		uint8_t option_idx = control_adj_menu_scroll + i;  // 0-4
		uint8_t is_selected = 0;
		
		if(!topbar_mode && (option_idx == selection)) {
			is_selected = 1;
		}
		Draw_Control_Adj_Menu_Row(i, is_selected);
	}
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
void Draw_Control_Adj_Sensor_Page(uint8_t selection)
{
	uint8_t topbar_mode = 0;
	uint8_t edit_red = 0;
	uint8_t leave_red = 0;
	
	// Handle special selection values for TopBar mode
	// 0xFF = TopBar mode with Edit red (initial state)
	// 0xFE = TopBar mode with Leave red
	if(selection == 0xFF) {
		topbar_mode = 1;
		edit_red = 1;
	} else if(selection == 0xFE) {
		topbar_mode = 1;
		leave_red = 1;
	}
	
	// Clear screen
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	
	// Draw Top Bar (Leave and Edit icons)
	if(leave_red) {
		GUI_DrawMonoIcon16x16(5, 5, RED, WHITE, Icon16x16_Leave);
	} else {
		GUI_DrawMonoIcon16x16(5, 5, BLACK, WHITE, Icon16x16_Leave);
	}
	
	if(edit_red) {
		GUI_DrawMonoIcon16x16(144, 5, RED, WHITE, Icon16x16_Edit);
	} else {
		GUI_DrawMonoIcon16x16(144, 5, BLACK, WHITE, Icon16x16_Edit);
	}
	
	// Draw "Sensor" title closer to Top Bar (left aligned, y=24 - moved up)
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	Show_Str(10, 24, BLACK, WHITE, (u8*)"Sensor", 16, 0);
	
	// Draw horizontal line below "Sensor" title (y=42 to y=43, leave 4px margin on both sides)
	LCD_Fill(4, 42, lcddev.width - 4, 43, BLACK);
	
	// Draw Floor/Room choices (always show, but arrow only in option mode)
	if(topbar_mode) {
		// TopBar mode: show choices without arrow
		Draw_Control_Adj_Sensor_Choices_NoArrow();
	} else {
		// Option mode: show choices with arrow
		Draw_Control_Adj_Sensor_Choices(selection);
	}
}

// Draw only the Top Bar for Sensor Setting page
// leave_red: 1=Leave red, 0=Leave black
// edit_red: 1=Edit red, 0=Edit black
void Draw_Control_Adj_Sensor_TopBar(uint8_t leave_red, uint8_t edit_red)
{
	// Only clear the Top Bar area (y=0 to y=25)
	LCD_Fill(0, 0, lcddev.width, 25, WHITE);
	
	// Draw Leave Icon
	if(leave_red) {
		GUI_DrawMonoIcon16x16(5, 5, RED, WHITE, Icon16x16_Leave);
	} else {
		GUI_DrawMonoIcon16x16(5, 5, BLACK, WHITE, Icon16x16_Leave);
	}

	// Draw Edit Icon
	if(edit_red) {
		GUI_DrawMonoIcon16x16(144, 5, RED, WHITE, Icon16x16_Edit);
	} else {
		GUI_DrawMonoIcon16x16(144, 5, BLACK, WHITE, Icon16x16_Edit);
	}
}

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
			Show_Str(45, row_y[i] + 4, BLACK, WHITE, (u8*)"Room", 16, 0);
		} else {
			Show_Str(45, row_y[i] + 4, BLACK, WHITE, (u8*)"Floor", 16, 0);
		}
		
		// Draw Arrow Icon with RED BACKGROUND for selected item (edit mode style)
		// selection 0=Room, 1=Floor
		if(i == selection) {
			GUI_DrawMonoIcon16x16(20, row_y[i] + 4, WHITE, RED, Icon16x16_Arrow);
		}
		// Non-selected items have no arrow icon
	}
	
	// Draw Save text at bottom (not selected yet in choices mode)
	Draw_Control_Adj_Sensor_Save(0);
}

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
			Show_Str(45, row_y[i] + 4, BLACK, WHITE, (u8*)"Room", 16, 0);
		} else {
			Show_Str(45, row_y[i] + 4, BLACK, WHITE, (u8*)"Floor", 16, 0);
		}
	}
	
	// Show red line arrow at current saved value (default Room=0)
	// Use RED foreground with WHITE background (red line style)
	GUI_DrawMonoIcon16x16(20, row_y[current_sensor_type] + 4, RED, WHITE, Icon16x16_Arrow);
	
	// Draw Save text at bottom
	Draw_Control_Adj_Sensor_Save(0);
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
	Show_Str(save_x, row_y, save_color, WHITE, (u8*)"Save", 16, 0);
}

// ============================================================
// Control Adj Temp. Correct Setting Page Drawing Functions
// ============================================================
/* Commented out to clear page content due to previous rate-limiting errors.
// Original functionality related to Temp. Correct page drawing.

// Draw Temp. Correct value with sign and decimal
// value: float value to display (e.g., 1.0, -0.5)
// x, y: position
// color: text color
void Draw_TempCorrect_Value(float value, uint16_t x, uint16_t y, uint16_t color)
{
	int integer_part;
	int decimal_part;
	uint8_t is_negative = 0;
	uint16_t current_y_offset = y; // Y offset for vertical display

	// Handle negative values
	if(value < 0) {
		is_negative = 1;
		value = -value; // Work with positive value for digit extraction
	}
	
	integer_part = (int)value;
	decimal_part = (int)((value - integer_part) * 10); // Get first decimal place

	// Round to nearest 0.5 (optional, if values are strictly 0.0 or 0.5)
	if (decimal_part >= 8) { // e.g., 0.8 -> 1.0
		integer_part++;
		decimal_part = 0;
	} else if (decimal_part >= 3) { // e.g., 0.3 -> 0.5
		decimal_part = 5;
	} else { // e.g., 0.1 -> 0.0
		decimal_part = 0;
	}

	// Clear value area (adjust size for vertical display)
	// A single 16x32 digit is 16px wide, 32px high. Max 2 digits + decimal + sign = ~32x(3*16+8) = 32x56
	LCD_Fill(x, y, x + 32, y + 64, WHITE); // Clear an area of ~32x64 for digits and sign

	// Draw minus sign if negative
	if(is_negative) {
		LCD_Fill(x + 5, current_y_offset, x + 11, current_y_offset + 3, color); // Minus sign (small horizontal bar)
		current_y_offset += 16; // Shift Y down for next digit
	}
	
	// Draw integer part (16x32 font)
	// If integer part is 0 and decimal is also 0, show "0"
	if (integer_part == 0 && decimal_part == 0 && !is_negative) {
	    GUI_DrawBigDigit(x, current_y_offset, color, WHITE, '0', 1); // 16x32 font
		current_y_offset += 32;
	} else if (integer_part >= 10) {
		GUI_DrawBigDigit(x, current_y_offset, color, WHITE, '0' + (integer_part / 10), 1);
		current_y_offset += 32;
		GUI_DrawBigDigit(x, current_y_offset, color, WHITE, '0' + (integer_part % 10), 1);
		current_y_offset += 32;
	} else if (integer_part > 0) {
		GUI_DrawBigDigit(x, current_y_offset, color, WHITE, '0' + integer_part, 1);
		current_y_offset += 32;
	}
	
	// Draw decimal point
	LCD_Fill(x + 5, current_y_offset, x + 11, current_y_offset + 3, color); // Decimal point (small horizontal bar)
	current_y_offset += 16; // Shift Y down for next digit

	// Draw decimal part
	GUI_DrawBigDigit(x, current_y_offset, color, WHITE, '0' + decimal_part, 1);
}


// Draw Control Adj Temp. Correct Setting Page
// This page shows:
// - Top-left: Leave Icon (black/red)
// - Top-right: Edit Icon (black/red)
// - "Temp. Correct" title below Top Bar
// - Horizontal line below title (with 4px margin on both sides)
// - Internal/External options with value display
// selection: 0=Internal, 1=External, 2=Save, 0xFF=TopBar Edit red, 0xFE=TopBar Leave red
void Draw_Control_Adj_TempCorrect_Page(uint8_t selection)
{
	uint8_t edit_red = 0;
	uint8_t leave_red = 0;
	uint8_t topbar_mode = 0; // Added back

	// Handle special selection values for TopBar mode
	if(selection == 0xFF) {
		edit_red = 1;
		topbar_mode = 1;
	} else if(selection == 0xFE) {
		leave_red = 1;
		topbar_mode = 1;
	}
	
	// Clear screen
	LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
	
	// Draw Top Bar (Leave and Edit icons)
	if(leave_red) {
		GUI_DrawMonoIcon16x16(5, 5, RED, WHITE, Icon16x16_Leave);
	} else {
		GUI_DrawMonoIcon16x16(5, 5, BLACK, WHITE, Icon16x16_Leave);
	}
	
	if(edit_red) {
		GUI_DrawMonoIcon16x16(144, 5, RED, WHITE, Icon16x16_Edit);
	} else {
		GUI_DrawMonoIcon16x16(144, 5, BLACK, WHITE, Icon16x16_Edit);
	}
	
	// Draw "Temp. Correct" title (right aligned for vertical text)
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	Show_Str(lcddev.width - 24, 10, BLACK, WHITE, (u8*)"Temp.", 16, 0); // X = 160 - 24 = 136
	Show_Str(lcddev.width - 24, 30, BLACK, WHITE, (u8*)"Correct", 16, 0); // Shifted down for vertical display
	
	// Draw horizontal line below "Temp. Correct" title (y=42 to y=43, leave 4px margin on both sides)
	// This line will now be a vertical line in the rotated view
	LCD_Fill(lcddev.width - 28, 0, lcddev.width - 27, lcddev.height, BLACK); // A vertical line (originally horizontal)
	
	// Draw Internal/External options and values
	if (topbar_mode) {
		// TopBar mode: Arrow Icon uses red line, based on current_temp_correct_type
		Draw_Control_Adj_TempCorrect_Options(current_temp_correct_type);
	} else {
		// Edit mode: Arrow Icon uses red background, based on current selection
		Draw_Control_Adj_TempCorrect_Options(selection);
	}
	
	// Draw Save button (black when not selected, red when selected)
	Draw_Control_Adj_TempCorrect_Save((selection == 3) ? 1 : 0); // Selection 3 is Save
}

// Draw Internal/External options and values (for optimized update)
// selection: 0=Internal, 1=External
void Draw_Control_Adj_TempCorrect_Options(uint8_t selection)
{
	uint16_t row_x_internal = 30; // X position for Internal text
	uint16_t row_x_external = 60; // X position for External text (closer spacing)
	uint16_t arrow_icon_fc, arrow_icon_bc; // Foreground and background for arrow icon
	
	// Clear the options area (x=0 to x=80, y=0 to lcddev.height) - needs to be wider to clear values
	LCD_Fill(0, 0, 120, lcddev.height, WHITE); // Clear a wider area to the left
	
	// Determine arrow icon colors based on current substate
	if (control_adj_temp_correct_substate == 1 || control_adj_temp_correct_substate == 2) { // In edit mode
		arrow_icon_fc = WHITE; // White arrow lines
		arrow_icon_bc = RED;   // Red arrow background
	} else { // In TopBar mode
		arrow_icon_fc = RED;   // Red arrow lines
		arrow_icon_bc = WHITE; // White arrow background
	}
	
	// Draw Internal option text (smaller font, adjust Y to be vertical from bottom up)
	POINT_COLOR = BLACK;
	BACK_COLOR = WHITE;
	Show_Str(row_x_internal, lcddev.height - 30, BLACK, WHITE, (u8*)"Internal", 12, 0); // X, Y (vertical text)
	
	// Draw External option text (smaller font, adjust Y to be vertical from bottom up)
	Show_Str(row_x_external, lcddev.height - 30, BLACK, WHITE, (u8*)"External", 12, 0); // X, Y (vertical text)
	
	// Draw arrow for selected option (Arrow icon is 16x16)
	// Position arrow to the "left" of the vertical text
	if (selection == 0) { // Internal selected
		GUI_DrawMonoIcon16x16(row_x_internal - 18, lcddev.height - 26, arrow_icon_fc, arrow_icon_bc, Icon16x16_Arrow);
	} else if (selection == 1) { // External selected
		GUI_DrawMonoIcon16x16(row_x_external - 18, lcddev.height - 26, arrow_icon_fc, arrow_icon_bc, Icon16x16_Arrow);
	}
	
	// Draw current values (adjusted X, Y for vertical text)
	Draw_TempCorrect_Value(temp_correct_internal, row_x_internal + 15, lcddev.height - 30, BLACK); // Shifted right (down in physical view)
	Draw_TempCorrect_Value(temp_correct_external, row_x_external + 15, lcddev.height - 30, BLACK); // Shifted right (down in physical view)
}

// Draw Save button for Temp. Correct page (vertical display)
// selected: 1=red, 0=black
void Draw_Control_Adj_TempCorrect_Save(uint8_t selected)
{
	uint16_t save_color = selected ? RED : BLACK;
	uint16_t save_x = 4;   // Left-aligned (4px margin from left edge)
	uint16_t save_y = lcddev.height - 30; // Bottom-aligned (text from bottom up)

	// Clear Save area
	LCD_Fill(save_x, save_y, save_x + 40, lcddev.height, WHITE); // Clear a vertical area
	
	POINT_COLOR = save_color;
	BACK_COLOR = WHITE;
	Show_Str(save_x, save_y, save_color, WHITE, (u8*)"Save", 16, 0); // Vertical text
}
