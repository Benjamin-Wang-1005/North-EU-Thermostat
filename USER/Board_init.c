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
//		- 
//		- 
//---------------------------------------------------------------------------------------------------------

#include "Thermostat.h"

void cmd_init(void);

//static uint8_t  fac_us=0;//us delay base
//static uint16_t fac_ms=0;//ms delay base

//---------------------------------------------------------------------------------------------------------
//
//---------------------------------------------------------------------------------------------------------

uint8_t rtc_rest = 0;
void my_RTC_Init(void)
	
{
		uint8_t rtc_status, need_ini = 0;
		// Initialize RTC
		rtc_status = RTC_Init();
		cmd_init();								//Initial UART1 fifo buffer
		
		if (rtc_status == RTC_OK)
		{
				// Read current time from RTC
				RTC_ReadTime(&rtc_time);
				// Check if RTC was previously configured
				if (!RTC_IsConfigured() || rtc_time.Year < 26)
				{
						need_ini = 1;
				}
				if(need_ini)
				{
						// First time boot - set default time
						RTC_GetDefaultTime(&rtc_time);
						RTC_SetTime(&rtc_time);
						rtc_rest = 1;
						//LOGD("RTC Reset!\n\r");
				}
		
				// Start RTC counting
				RTC_Start();
		}
		
		
}


// Initialize GPIO for keys and Relay
void Key_Init(void)
{
	uint8_t i;
	GPIO_InitTypeDef GPIO_InitStructure;

	// Enable clocks
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

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
	
	//PB8 - Relay IO 
	GPIO_InitStructure.GPIO_Pin = RELAY_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(RELAY_PORT, &GPIO_InitStructure);
	GPIO_WriteBit(RELAY_PORT, RELAY_PIN, Bit_RESET);		//Close Relay at power on
	Relay = RELAY_OFF;
	
	key.key_active = 0;
	key.key_press = 0;
	key.key_val = 0;
	
	for(i = 0; i < KEY_QUT; i++) key.key_press_cnt[i] = 0;
}



// SysTick initialization: 1ms interrupt mode
void sys_tick_Init(void)
{
    // System clock 64MHz, SysTick uses HCLK = 64MHz
    // For 1ms interrupt, reload value = 64000 - 1 = 63999
    
    // Method 1: Direct register operation
    //SysTick->CTRL &= ~SysTick_CTRL_CLKSOURCE_Msk;   // Select HCLK/8 (bit2=0)
    //SysTick->LOAD = 9000 - 1;                       // Load reload value
    //SysTick->VAL  = 0;                              // Clear current value
    //SysTick->CTRL = SysTick_CTRL_TICKINT_Msk |      // Enable interrupt (bit1=1)
    //                SysTick_CTRL_ENABLE_Msk;        // Enable counter (bit0=1)
    
    /* Method 2: Use standard library functions (equivalent)
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);  // 9MHz
    SysTick_Config(9000 - 1);   // Automatically enables interrupt and counter
    */
	
		if (SysTick_Config(SystemCoreClock / 1000))
  { 
    /* Capture error */ 
    while (1);
  }
}

static const uint8_t workday_schedule[6][4] = {
    {5, 30, 20, 1},   // P1: 05:30, 20 degC, ON
    {8, 30, 20, 1},   // P2: 08:30, 20 degC, ON
    {10, 30, 20, 1},  // P3: 10:30, 20 degC, ON
    {12, 30, 20, 1},  // P4: 12:30, 20 degC, ON
    {18, 30, 20, 1},  // P5: 18:30, 20 degC, ON
    {21, 30, 20, 1}   // P6: 21:30, 20 degC, ON
};

static const uint8_t holiday_schedule[6][4] = {
		{7, 0, 20, 1},    // P1: 07:00, 20 degC, ON
		{9, 0, 20, 1},    // P2: 09:00, 20 degC, ON
		{12, 0, 20, 1},   // P3: 12:00, 20 degC, ON
		{14, 0, 20, 1},   // P4: 14:00, 20 degC, ON
		{18, 0, 20, 1},   // P5: 18:00, 20 degC, ON
		{22, 0, 20, 1}    // P6: 22:00, 20 degC, ON
};

void golbal_par_init(void)
{
		uint8_t i;
		g_parameter.child_lock = 0;
		// Initialize PIN code to default 0000
		for(i = 0; i < 4; i++){
				g_parameter.pin_code[i] = 0;
		}
		g_parameter.current_prog_type = 0;					// 0="5+2", 1="6+1", 2="7"
		g_parameter.power_limit = 6;								// Turn on 90%
		g_parameter.power_limit_switch = 0;					// close power limit
		g_parameter.comfort_mode = 0;								// close comfort mode
		g_parameter.floor_material = 1;							// '0'=Wood/Laminate, '1'=Tile/Concrete, '2'=Fast Response
		g_parameter.power_on_state = 2;							// 1=Keep state 2=Device close 3=Device on 
		g_parameter.relay_staty = 0;								//Relay state while poer off
		g_parameter.room_temp = 15;									//For test
		g_parameter.sensor_type = 0;								//'0'=Room, '1'=Floor
		g_parameter.setting_number = 18.0f;					//Initial set to 18
		g_parameter.sleep_backlight_duty = 20;			//Duty 20%
		g_parameter.temp_correct_external = -0.5f;	//Minus 0.5
		g_parameter.temp_correct_internal = -1.0f;	//Minus 1
		g_parameter.temp_limit_max = INPUT_TEMPERATURE_MAX_DEFAULT;						// Setup temp limit max value
		g_parameter.temp_limit_min = INPUT_TEMPERATURE_MIN_DEFAULT;
		g_parameter.temp_swing = 1;									// swing range 0.5 ~ 3, 0.5 as one step
		g_parameter.operation_mode = 0;							//'0'= Manual Mode '1'= Schedule Mode;
		g_parameter.temp_protect_max = DEVICE_PROTECT_TEMP_MAX_DEFAULT;
		g_parameter.temp_protect_max_switch = 0;
		g_parameter.temp_protect_min = DEVICE_PROTECT_TEMP_MIN_DEFAULT;
		g_parameter.temp_protect_min_switch = 1;
		g_parameter.window_fun = 0;
		g_parameter.window_fun_temp = 5;
		g_parameter.window_fun_time = 30;
		g_parameter.language = 0;										//English
		memcpy(g_parameter.workday_schedule, workday_schedule, sizeof(workday_schedule));
		memcpy(g_parameter.holiday_schedule, holiday_schedule, sizeof(holiday_schedule));
		
		child_lock_flag = 0;
		//Display_INT_Temp = -100;
		//Display_EXT_Temp = -100;

}

//---------------------------------------------------------------------------------------------------------
// Funcation: relay_init
// Description: Initial Relay status
// Input: None
// Output: None
// Date: 2026/05/04
// Update:
//----------------------------------------------------------------------------------------------------------

void relay_init(void)
{
		
		if(g_parameter.power_on_state == 1){						//Keep State
				Relay = g_parameter.relay_staty;
				if(Relay == RELAY_ON){
						GPIO_WriteBit(RELAY_PORT, RELAY_PIN, Bit_SET);			//Open relay
				}else{
						GPIO_WriteBit(RELAY_PORT, RELAY_PIN, Bit_RESET);		//Close relay
				}
		}else if(g_parameter.power_on_state == 2){			//Relay Off
				Relay = RELAY_OFF;
				GPIO_WriteBit(RELAY_PORT, RELAY_PIN, Bit_RESET);		//Close relay
		}else if(g_parameter.relay_staty == 3){					//Relay on
				Relay = RELAY_ON;
				GPIO_WriteBit(RELAY_PORT, RELAY_PIN, Bit_SET);			//Open relay
		}
		
		//old_relay_state = Relay;
	
}

//---------------------------------------------------------------------------------------------------------
// Funcation: cmd_init
// Description: Initial AT command relative parameter
// Input: None
// Output: None
// Date: 2022/12/02
// Update:
//----------------------------------------------------------------------------------------------------------
void cmd_init(void)
{
		uint8_t i;
		
		//inst = 0;
		cmd_queue.F_received_complete = eFALSE;
		cmd_queue.rx_index = 0;
		for(i=0; i<UART_FIFO_QTY; i++){
				cmd_queue.rx_buffer[i] = 0;
		}	
}

