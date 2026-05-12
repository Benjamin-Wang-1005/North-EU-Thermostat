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
//File Function: Peripheral.c to define peripheral HW operation
//		- Key operation
//		- Log USART
//---------------------------------------------------------------------------------------------------------

#include "Thermostat.h"

#define EVB_BOARD										(0)

#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
	
#define CMD_PREFIX          				"[CMD]"
#define CMD_PREFIX_LEN      				(5)
	
volatile uint32_t time_tick = 0;  // Must be volatile to prevent compiler optimization
struct_key_t key;
uint8_t weekday;
cmd_queue_t cmd_queue;


USART_InitTypeDef USART_InitStructure;

USART_TypeDef* COM_USART[COMn] = {EVAL_COM1, EVAL_COM2}; 
GPIO_TypeDef* COM_TX_PORT[COMn] = {EVAL_COM1_TX_GPIO_PORT, EVAL_COM2_TX_GPIO_PORT};
GPIO_TypeDef* COM_RX_PORT[COMn] = {EVAL_COM1_RX_GPIO_PORT, EVAL_COM2_RX_GPIO_PORT};
const uint32_t COM_USART_CLK[COMn] = {EVAL_COM1_CLK, EVAL_COM2_CLK};
const uint32_t COM_TX_PORT_CLK[COMn] = {EVAL_COM1_TX_GPIO_CLK, EVAL_COM2_TX_GPIO_CLK};
const uint32_t COM_RX_PORT_CLK[COMn] = {EVAL_COM1_RX_GPIO_CLK, EVAL_COM2_RX_GPIO_CLK};
const uint16_t COM_TX_PIN[COMn] = {EVAL_COM1_TX_PIN, EVAL_COM2_TX_PIN};
const uint16_t COM_RX_PIN[COMn] = {EVAL_COM1_RX_PIN, EVAL_COM2_RX_PIN};
	


// Scan keys
// Return: 0 = no key, 1 = Up key, 2 = Down key, 3 = Enter key, 4 = System Reset
void Key_Scan(void)
{
	static uint32_t local_tick;
	static uint8_t long_press_triggered = 0;  // 
	static uint8_t both_press_ongoing = 0;    // both key in press
	
	uint8_t up_physical;
	uint8_t down_physical;
	uint8_t enter_physical;
	
	if(0 == g_clock_time_exceed(local_tick, 20)){								
				return;
	}
	local_tick = time_tick;

	// ========== read key status  high key press==========	
#if(EVB_BOARD)
	up_physical = (GPIO_ReadInputDataBit(UP_KEY_PORT, UP_KEY_PIN) == 1) ? 1 : 0;
	down_physical = (GPIO_ReadInputDataBit(DOWN_KEY_PORT, DOWN_KEY_PIN) == 1) ? 1 : 0;
	enter_physical = (GPIO_ReadInputDataBit(ENTER_KEY_PORT, ENTER_KEY_PIN) == 1) ? 1 : 0;
#else
	up_physical = (GPIO_ReadInputDataBit(UP_KEY_PORT, UP_KEY_PIN) == 0) ? 1 : 0;
	down_physical = (GPIO_ReadInputDataBit(DOWN_KEY_PORT, DOWN_KEY_PIN) == 0) ? 1 : 0;
	enter_physical = (GPIO_ReadInputDataBit(ENTER_KEY_PORT, ENTER_KEY_PIN) == 0) ? 1 : 0;
#endif
	
	// ========== detect long press��UP + DOWN both press 5 sec ==========
	if(up_physical && down_physical)
	{
		if(!both_press_ongoing)
		{
			// enter both key press
			both_press_ongoing = 1;
			long_press_triggered = 0;
		}
		
		// acc counter��both key need accumulate��
		if(key.key_press_cnt[UPKEY - 1] < 255){ 
					key.key_press_cnt[UPKEY - 1]++;
			//LOGD("Upkey_cnt:%d\r\n",key.key_press_cnt[UPKEY - 1]);
		}
		if(key.key_press_cnt[DOWNKEY - 1] < 255) key.key_press_cnt[DOWNKEY - 1]++;
		
		// Check if the long press threshold has been reached, and if it has not yet been triggered.
		if(!long_press_triggered && 
		   key.key_press_cnt[UPKEY - 1] >= LONG_PRESS_THRESHOLD && 
		   key.key_press_cnt[DOWNKEY - 1] >= LONG_PRESS_THRESHOLD)
		{
			long_press_triggered = 1;
			LOGD("Up+Down Long Press 5s: Trigger SYS_RESET\n\r");
			key.key_active = 1;
			key.key_val = SYS_RESET;
			
			// Note: Do not clear the counter; let it continue to accumulate, but `long_press_triggered` prevents repeated triggering.
		}
	}
	else if(!up_physical && !down_physical)
	{
		// Releasing any key clears the state where both keys were pressed simultaneously.
		if(both_press_ongoing)
		{
			both_press_ongoing = 0;
			long_press_triggered = 0;
			// Do not clear key.key_press_cnt; let the original short press logic determine whether a short press is valid.
		}
	}
	
	// ========== Existing short-press logic (modified to avoid overwriting SYS_RESET).==========
	if(key.key_press != 0)
	{
		// If a SYS_RESET key is already pending, no further key presses will be processed.
		if(key.key_val == SYS_RESET)
		{
			// Retain SYS_RESET and do nothing.
			return;
		}
		
		if((key.key_press & Upkey_Mask) != 0)
		{
			if(!up_physical)
			{
				LOGD("Up Key\n\r");
				key.key_active = 1;
				key.key_val = UPKEY;
				if((UI_state != STATE_ACTIVE) && (UI_state != STATE_SLEEP) && (UI_state != STATE_SETTING)){
						update_alarm(Setting_Menu_Alarm);
				}else if(UI_state == STATE_SETTING){
						update_alarm(Setting_Digi_Alarm);
				}
			}
		}
		else if((key.key_press & Downkey_Mask) != 0)
		{
			if(!down_physical)
			{
				LOGD("Down Key\n\r");
				key.key_active = 1;
				key.key_val = DOWNKEY;
				if((UI_state != STATE_ACTIVE) && (UI_state != STATE_SLEEP) && (UI_state != STATE_SETTING)){
						update_alarm(Setting_Menu_Alarm);
				}else if(UI_state == STATE_SETTING){
						update_alarm(Setting_Digi_Alarm);
				}
			}
		}
		else if((key.key_press & Enterkey_Mask) != 0)
		{
			if(!enter_physical)
			{
				LOGD("Enter Key\n\r");
				key.key_active = 1;
				key.key_val = ENTERKEY;
				if((UI_state != STATE_ACTIVE) && (UI_state != STATE_SLEEP) && (UI_state != STATE_SETTING)){
						update_alarm(Setting_Menu_Alarm);
				}else if(UI_state == STATE_SETTING){
						update_alarm(Setting_Digi_Alarm);
				}
			}
		}
	}
	else
	{
		if(!long_press_triggered){
				if(key.key_active == 0){				//No key wait to process
						// Up-key detection (add long_press_triggered check)
#if(EVB_BOARD)
						if(GPIO_ReadInputDataBit(UP_KEY_PORT, UP_KEY_PIN) == 1){
#else
						if(GPIO_ReadInputDataBit(UP_KEY_PORT, UP_KEY_PIN) == 0){
#endif
								// If it's triggered by a long press, a short press won't trigger it.
								if(key.key_press_cnt[UPKEY - 1] > 1){
										key.key_press_cnt[UPKEY - 1] = 0;
										key.key_press = key.key_press | Upkey_Mask;
								}else if(key.key_press_cnt[UPKEY - 1] <= 1){
										key.key_press_cnt[UPKEY - 1]++;
								}
						}else{
								key.key_press_cnt[UPKEY - 1] = 0;
						}
				
						// Down Key Detection
#if(EVB_BOARD)
						if(GPIO_ReadInputDataBit(DOWN_KEY_PORT, DOWN_KEY_PIN) == 1){
#else
						if(GPIO_ReadInputDataBit(DOWN_KEY_PORT, DOWN_KEY_PIN) == 0){
#endif
								if(key.key_press_cnt[DOWNKEY - 1] > 1){
										key.key_press_cnt[DOWNKEY - 1] = 0;
										key.key_press = key.key_press | Downkey_Mask;
								}else if(key.key_press_cnt[DOWNKEY - 1] <= 1){
										key.key_press_cnt[DOWNKEY - 1]++;
								}
						}else{
								key.key_press_cnt[DOWNKEY - 1] = 0;
						}
				
						// Enter Key Check ( Keep as is)
#if(EVB_BOARD)
						if(GPIO_ReadInputDataBit(ENTER_KEY_PORT, ENTER_KEY_PIN) == 1){
#else
						if(GPIO_ReadInputDataBit(ENTER_KEY_PORT, ENTER_KEY_PIN) == 0){
#endif
								if(key.key_press_cnt[ENTERKEY - 1] > 1){
										key.key_press_cnt[ENTERKEY - 1] = 0;
										key.key_press = key.key_press | Enterkey_Mask;
								}else{
										key.key_press_cnt[ENTERKEY - 1]++;
								}
						}else{
								key.key_press_cnt[ENTERKEY - 1] = 0;
						}
				}
		}
		
	}

}


//------------------------------------------------------------------------------------------------------------------
//
// USART1 Relative Functions
// Define USART1 as log output port and command input port
//				USARTx configured as follow:
//        - BaudRate = 115200 baud  
//        - Word Length = 8 Bits
//        - One Stop Bit
//        - No parity
//        - Hardware flow control disabled (RTS and CTS signals)
//        - Receive and transmit enabled
//
//------------------------------------------------------------------------------------------------------------------

/**
  * @brief  Configures COM port.
  * @param  COM: Specifies the COM port to be configured.
  *   This parameter can be one of following parameters:    
  *     @arg COM1
  *     @arg COM2  
  * @param  USART_InitStruct: pointer to a USART_InitTypeDef structure that
  *   contains the configuration information for the specified USART peripheral.
  * @retval None
  */
void STM_EVAL_COMInit(COM_TypeDef COM, USART_InitTypeDef* USART_InitStruct)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIO clock */
  RCC_APB2PeriphClockCmd(COM_TX_PORT_CLK[COM] | COM_RX_PORT_CLK[COM] | RCC_APB2Periph_AFIO, ENABLE);

  /* Enable UART clock */
  if (COM == COM1)
  {
    RCC_APB2PeriphClockCmd(COM_USART_CLK[COM], ENABLE); 
  }
  else
  {
    RCC_APB1PeriphClockCmd(COM_USART_CLK[COM], ENABLE);
  }

  /* Configure USART Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pin = COM_TX_PIN[COM];
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(COM_TX_PORT[COM], &GPIO_InitStructure);

  /* Configure USART Rx as input floating */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Pin = COM_RX_PIN[COM];
  GPIO_Init(COM_RX_PORT[COM], &GPIO_InitStructure);

  /* USART configuration */
  USART_Init(COM_USART[COM], USART_InitStruct);
    
  /* Enable USART */
  USART_Cmd(COM_USART[COM], ENABLE);
}


/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  //uint32_t timeout = 0;
  
  /* Loop until transmit data register is empty (with timeout) */
  while (USART_GetFlagStatus(EVAL_COM1, USART_FLAG_TXE) == RESET);
  
  USART_SendData(EVAL_COM1, (uint8_t) ch);
  
  return ch;
}

void Log_USART_Init(void)
{
		NVIC_InitTypeDef NVIC_InitStructure;
		/* USARTx configured as follow:
        - BaudRate = 115200 baud  
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
		*/
		USART_InitStructure.USART_BaudRate = 115200;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
		USART_InitStructure.USART_Parity = USART_Parity_No;
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

		STM_EVAL_COMInit(COM1, &USART_InitStructure);
	
		USART_ITConfig(EVAL_COM1, USART_IT_RXNE, ENABLE);		//Enable UART1 received interrupt
	
		// Enable NVIC interrupt 
		NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
		
		// clear received buffer
		memset((void*)cmd_queue.rx_buffer, 0, UART_FIFO_QTY);
	
}



//---------------------------------------------------------------------------------------------------------
// Funcation: _receiveCmd
// Description: Check RX buffer had received complete command 
// Input: None
// Output: eTRUE -> complere command had received / eFALSE -> received not complete
// Date: 2022/12/03
// Update:
//----------------------------------------------------------------------------------------------------------
static bool_t receiveCmd(void)
{
		uint8_t inChar = 0xFF;
		while((USART1->SR & USART_FLAG_RXNE) != 0){		//'0' no data can read
				inChar = USART_ReceiveData(EVAL_COM1);
				if(inChar == 0x00 || inChar == '\r' || inChar == '\n') {
						cmd_queue.rx_buffer[cmd_queue.rx_index] = 0x00;
                    return eTRUE;
        }
				// lowercase to uppercase
        if (inChar >= 'a' && inChar <= 'z') {
						inChar -= 0x20;
        }
				
				if(cmd_queue.rx_index < UART_FIFO_QTY - 1)  // Reserved onr byte for '\0'
        {
            cmd_queue.rx_buffer[cmd_queue.rx_index] = inChar;
            cmd_queue.rx_index++;
        }
        else
        {
            // FIFO full
            LOGD("UART buffer overflow!\n\r");
            // clear FIFO
            cmd_queue.rx_index = 0;
        }
		}
		
		return eFALSE;
}

//---------------------------------------------------------------------------------------------------------
// Function: USART1_IRQHandler
// Description: UART1 interrupt handle
// Input: None
// Output: None
// Date: 2026/05/03
//---------------------------------------------------------------------------------------------------------
void USART1_IRQHandler(void)
{
		bool_t completed = eFALSE;
		// check rx index
		if(USART_GetITStatus(EVAL_COM1, USART_IT_RXNE) != RESET)
		{
				completed = receiveCmd();
				if(completed){
						cmd_queue.F_received_complete = eTRUE;
				}					
				// clear interrupt flag
				USART_ClearITPendingBit(EVAL_COM1, USART_IT_RXNE);
		}
}

//------------------------------------------------------------------------------------------------------------------
//
// UART1 input command handle reative functions
// 
//
//------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
// Function: parse_command
// Description: Parse received command string and extract command name and parameter
// Input: cmd_str - received command string (without newline)
// Output: cmd_name - extracted command name (max 32 chars)
//         param - extracted parameter (may be NULL)
// Return: 0 on success, -1 on failure
//---------------------------------------------------------------------------------------------------------
bool_t parse_command(const char* cmd_str, char* cmd_name, char* param)
{
    const char* p = cmd_str;
    const char* colon_pos = NULL;
    int cmd_len;
    
    // Check prefix [CMD]
    if (strncmp(p, CMD_PREFIX, CMD_PREFIX_LEN) != 0) {
        return eFALSE;
    }
    p += CMD_PREFIX_LEN;
    
    // Find colon ':'
    colon_pos = strchr(p, ':');
    if (colon_pos == NULL) {
        // No param, whole string is command name
        cmd_len = strlen(p);
        if (cmd_len >= 32) cmd_len = 31;
        strncpy(cmd_name, p, cmd_len);
        cmd_name[cmd_len] = '\0';
        param[0] = '\0';
    } else {
        // Has param
        cmd_len = colon_pos - p;
        if (cmd_len >= 32) cmd_len = 31;
        strncpy(cmd_name, p, cmd_len);
        cmd_name[cmd_len] = '\0';
        
        // Copy param
        strncpy(param, colon_pos + 1, 31);
        param[31] = '\0';
    }
    
    // Convert command name to uppercase for case-insensitive comparison
    // had implement in receiveCmd(void)
    //for (int i = 0; cmd_name[i]; i++) {
    //    cmd_name[i] = toupper(cmd_name[i]);
    //}
    
    return eTRUE;
}

//---------------------------------------------------------------------------------------------------------
// Function: processCmd
// Description: Process received command from UART
// Input: None
// Output: None
// Date: 2026/05/03
// Update: Added factory test protocol parsing
//----------------------------------------------------------------------------------------------------------
static void processCmd(void)
{
    char cmd_name[10];
    char param[10];
    char* cmd_str;
		int len;
    
    // Get the received command string
    cmd_str = (char*)cmd_queue.rx_buffer;
    
    //LOGD("Received: %s\n", cmd_str);
    
    // Remove trailing newline/carriage return if present
    len = strlen(cmd_str);
    while (len > 0 && (cmd_str[len-1] == '\n' || cmd_str[len-1] == '\r')) {
        cmd_str[len-1] = '\0';
        len--;
    }
    
    // Parse command
    if (parse_command(cmd_str, cmd_name, param) == eFALSE) {
        LOGD("Invalid command format: %s\n", cmd_str);
        return;
    }
    
    LOGD("Parsed: cmd=%s, param=%s\n", cmd_name, param);
    
    // Dispatch to appropriate handler
    if (strcmp(cmd_name, "CONNECT") == 0) {
        //connect_cmd();
    }
    else if (strcmp(cmd_name, "EXIT") == 0) {
        //exit_cmd();
    }
    else if (strcmp(cmd_name, "LCD") == 0) {
        //lcd_test_cmd(param);
    }
    else if (strcmp(cmd_name, "KEY_TEST") == 0) {
        //key_test_cmd(param);
    }
    else if (strcmp(cmd_name, "GET") == 0) {
        // Handle GET:TEMP or GET:VCC
        if (strcasecmp(param, "TEMP") == 0) {
            //get_temp_cmd();
        }
        else if (strcasecmp(param, "VCC") == 0) {
            //get_vcc_cmd();
        }
        else {
            //send_response("GET", "INVALID");
        }
    }
    else if (strcmp(cmd_name, "RELAY") == 0) {
        //relay_cmd(param);
    }
    else {
        LOGD("Unknown command: %s\n", cmd_name);
        printf("[CMD]RES:UNKNOWN:COMMAND\n");
    }
}


//---------------------------------------------------------------------------------------------------------
// Funcation: g_cmd_handler
// Description: Handle UART1 received command process
// Input: None
// Output: None
// Date: 2026/05/03
// Update:
//----------------------------------------------------------------------------------------------------------
void g_cmd_handler(void)
{
		
		if(cmd_queue.F_received_complete){
				processCmd();
				
				cmd_queue.F_received_complete = eFALSE;
		}
}

//------------------------------------------------------------------------------------------------------------------
//
// Initialize TIM4 CH4 (PB9) for backlight PWM control
// PWM frequency: 500Hz
//
//------------------------------------------------------------------------------------------------------------------

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



//------------------------------------------------------------------------------------------------------------------
//
//		System Tick relative functions
//
//------------------------------------------------------------------------------------------------------------------

uint8_t g_clock_time_exceed(uint32_t tick_run, uint32_t duration)
{
		uint32_t current_tick, elapsed;
		
		current_tick = time_tick;
		
		// Handle overflow case
		if(current_tick >= tick_run){
				elapsed = current_tick - tick_run;
		}else{
				// Overflow occurred: (0xFFFFFFFF - tick_run + 1) + current_tick
				elapsed = (0xFFFFFFFF - tick_run + 1) + current_tick;
		}
		
		return (elapsed >= duration) ? 1 : 0;
}

// Get current system time (milliseconds)
uint32_t get_tick_ms(void)
{
    return time_tick;
}

// Simple delay function (milliseconds)
void my_delay_ms(uint32_t ms)
{
    uint32_t start = get_tick_ms();
    while((get_tick_ms() - start) < ms);
}

void my_delay_us(uint32_t us)
{
    uint32_t start = SysTick->VAL;
    // SysTick decrements at HCLK frequency (SystemCoreClock Hz)
    uint32_t ticks_per_us = SystemCoreClock / 1000000;
    uint32_t ticks_needed = us * ticks_per_us;
    uint32_t current;
    uint32_t elapsed;

    do {
        current = SysTick->VAL;
        elapsed = (current <= start) ? (start - current) : (start + (SysTick->LOAD - current));
    } while(elapsed < ticks_needed);
}

//------------------------------------------------------------------------------------------------------------------
//
//		SW Timer relative functions
//
//------------------------------------------------------------------------------------------------------------------

uint8_t alarm_index;
Alarm_t Alarm[MAX_ALARM_NUMBER];
//---------------------------------------------------------------------------------------------------------
// Funcation: register_alarm(alarm_source_t source, uint32_t dur)
// Description: Regist an alarm source, duration and make it work
// Input: 
//		- source : alarm source
//		- dur : duration time
// Output: return eTRUE for regist suceed 
// Date: 2024/09/07
// Update: 2025/01/02
//---------------------------------------------------------------------------------------------------------

bool_t register_alarm(alarm_source_t source, uint32_t dur)
{
		uint8_t i, index = 1;
		for(i=0; i<MAX_ALARM_NUMBER; i++){
				if((index & alarm_index) !=0){
						index <<= 1;
				}else{
						Alarm[i].source = source;
						Alarm[i].tick = time_tick;
						Alarm[i].dur = dur;
						Alarm[i].f_alarm = eTRUE;
						alarm_index |= index;						//Set index bit to 1
						LOGD("R_Alarm: %d\r\n", source);
						return eTRUE;
				}
		}
		return eFALSE;
}

void update_alarm(alarm_source_t in_source)
{	
		uint8_t i;
	
		for(i=0; i<MAX_ALARM_NUMBER; i++){
				if(Alarm[i].source == in_source){
						Alarm[i].tick = time_tick;
				}
		}
		

	
}

//---------------------------------------------------------------------------------------------------------
// Funcation: delete_alarm(alarm_source_t source)
// Description: delete one alarm had been regist but do not time up
// Input: 
//		- source : alarm source
// Output: 
// Date: 2024/09/07
// Update: 2025/01/02
//---------------------------------------------------------------------------------------------------------
void delete_alarm(alarm_source_t source)
{
		uint8_t i, index = 1;
		for(i=0; i<MAX_ALARM_NUMBER; i++){
				if(Alarm[i].source == source){
						Alarm[i].f_alarm = eFALSE;
						Alarm[i].source = NONE;
						LOGD("D_Alarm: %d\r\n", source);
						index = index ^ 0xFF;
						alarm_index &= index;						//Set index bit to 0
						return;
				}
				index <<= 1;
		}
		LOGD("Not found alarm source: %d\r\n", source);
}

void get_schedule_period(void)
{
		uint8_t i;
		uint8_t (*sched_ptr)[4];
		uint32_t current_min, sch_min;
	
		if(g_parameter.current_prog_type == 0){
								if(weekday < 5){
										sched_ptr = g_parameter.workday_schedule;
								}else{
										sched_ptr = g_parameter.holiday_schedule;
								}
						}else if(g_parameter.current_prog_type == 1){
								if(weekday < 6){
										sched_ptr = g_parameter.workday_schedule;
								}else{
										sched_ptr = g_parameter.holiday_schedule;
								}
						}else if(g_parameter.current_prog_type == 2){
								sched_ptr = g_parameter.workday_schedule;
						}
						current_min = (rtc_time.Hour * 60) + rtc_time.Min;
						
						for(i=0; i<6; i++){
								sch_min = (sched_ptr[i][0] * 60) + sched_ptr[i][1];
								//LOGD("sch_min:%d\r\n", sch_min);
								//LOGD("current_min:%d\r\n", current_min);
								//LOGD("i=%d\r\n", i);
								if(sch_min > current_min)	{	
										break;
								}
								
						}
						
						if(i > 0){
								Schedule_Period = i - 1;
						}else{
								Schedule_Period = 5;
						}
						LOGD("S_Period:%d\r\n", Schedule_Period);
}

//---------------------------------------------------------------------------------------------------------
// Funcation: l_process_alarm_source(alarm_source_t source)
// Description: Input alarm source had time up, to do relative process
// Input: 
//		- source : alarm source
// Output: 
// Date: 2024/09/07
// Update: 2025/01/02
//---------------------------------------------------------------------------------------------------------
void l_process_alarm_source(alarm_source_t source)
{
		const uint8_t *icons[6] = {P1_Icon_24x24, P2_Icon_24x24, P3_Icon_24x24, P4_Icon_24x24, P5_Icon_24x24, P6_Icon_24x24};
		switch(source){
				case Active_Alarm:
						delete_alarm(Active_Alarm);
						if(icon6_red_state)
						{
								icon6_red_state = 0;
							GUI_DrawMonoIcon24x24(136, 72, BLACK, WHITE, Menu_Icon_24x24);  // Draw black Manu Icon
						}
						// If in Leave Schedule Confirm state, go to Active first (then Active will timeout to Sleep naturally)
						if(UI_state == STATE_LEAVE_SCHEDULE_CONFIRM){
								UI_state = STATE_ACTIVE;
								if(register_alarm(Active_Alarm, ACTIVE_ALIVE_TIME) == eFALSE){
										LOGE("Alarm Full!\r\n");
								}
						}else{
								UI_state = STATE_SLEEP;
								// Set backlight to 20%
								Backlight_SetDuty(g_parameter.sleep_backlight_duty);
						}
				break;
				
				case Setting_Digi_Alarm:
						delete_alarm(Setting_Digi_Alarm);
						UI_state = STATE_ACTIVE;
						Clear_Number_Area();
						Display_Number(g_parameter.setting_number, BLACK, 1);  // show_decimal = 1
						GUI_DrawMonoIcon24x24(100, 45, BLACK, WHITE, Celsius_Icon_24x24);
						if(register_alarm(Active_Alarm, ACTIVE_ALIVE_TIME) == eFALSE){
								LOGE("Alarm Fail\r\n");
						}
				break;
						
				case Setting_Menu_Alarm:
						delete_alarm(Setting_Menu_Alarm);
						main_display_digi = Display_Room_Temp;
						LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
						Draw_Active_Menu();
						// Reset to Active state (for any state that uses this alarm)
						//UI_state = STATE_ACTIVE;
						// Register Active_Alarm for normal active timeout
						//if(register_alarm(Active_Alarm, ACTIVE_ALIVE_TIME) == eFALSE){
						//		LOGE("Alarm Full!\r\n");
						//}
				break;
				
				case Min_Update_Alarm:
						update_alarm(Min_Update_Alarm);
						//Update Main Page Time
						if((UI_state == STATE_ACTIVE) || (UI_state == STATE_SLEEP)){
								RTC_ReadTime(&rtc_time);
								LCD_Fill(5, 5, 21, 37, WHITE);
								LCD_Fill(21, 5, 37, 37, WHITE);
								GUI_DrawBigDigit(5, 5, BLACK, WHITE, (rtc_time.Hour/10)+0x30 , 1);
								GUI_DrawBigDigit(21, 5, BLACK, WHITE, (rtc_time.Hour%10)+0x30, 1);
								LCD_Fill(47, 5, 63, 37, WHITE);
								LCD_Fill(63, 5, 79, 37, WHITE);
								GUI_DrawBigDigit(47, 5, BLACK, WHITE, (rtc_time.Min/10)+0x30, 1);
								GUI_DrawBigDigit(63, 5, BLACK, WHITE, (rtc_time.Min%10)+0x30, 1);
							
								//Update Weekday
								if((rtc_time.Hour == 0) && (rtc_time.Min == 1)){
										weekday = getWeekday(rtc_time.Year, rtc_time.Mon, rtc_time.Date);
										LCD_Fill(5, 105, 21, 120, WHITE);
										Show_Str(5, 105, BLACK, WHITE, week_texts[weekday], 16, 0);
								}
								//Update Schedule Period
								if(g_parameter.operation_mode == 1){
										get_schedule_period();
										LCD_Fill(136, 45, 160, 69, WHITE);
										GUI_DrawMonoIcon24x24(136, 45, BLACK, WHITE, icons[Schedule_Period]);	
								}
								//LOGD("Min Alarm time out\r\n");
						}
						
						
						
				break;
				
				default :
						LOGE("Unknow Alarm source: %d\r\n", source);
				break;
		}
	
}

//---------------------------------------------------------------------------------------------------------
// Funcation: g_check_alarm(void)
// Description: Check all alram source time status
// Input: 
// Output: 
// Date: 2024/09/07
// Update: 2025/01/02
//---------------------------------------------------------------------------------------------------------
void g_check_alarm(void)
{
		uint8_t i, index;
		uint8_t j = 1;
		index = alarm_index;
		for(i=0; i<MAX_ALARM_NUMBER; i++){
				if((index & j) != 0){
						if(g_clock_time_exceed(Alarm[i].tick, Alarm[i].dur)){				//Time expired
								LOGD("Time out: %d\r\n", Alarm[i].source);
								l_process_alarm_source(Alarm[i].source);
						}
				}
				j <<= 1;
		}
}

//---------------------------------------------------------------------------------------------------------
// Funcation: getWeekday(uint16_t year, uint8_t month, uint8_t day)
// Description: Check all alram source time status
// Input: Year / Month / Date
// Output: Weekday
// Date: 2026/04/27
// Update: 
//---------------------------------------------------------------------------------------------------------
uint8_t getWeekday(uint16_t year, uint8_t month, uint8_t day) {
		uint8_t week;
		year += 2000;		//RTC base is 2000/01/01
    // If month is Jan or Feb, treat as month 13 or 14 of previous year
    if (month == 1 || month == 2) {
        month += 12;
        year--;
    }
    
    // Calculate weekday (0=Monday, 1=Tuesday, ..., 6=Sunday)
    week = (day + 2*month + 3*(month+1)/5 + year + year/4 - year/100 + year/400) % 7;
    //LOGD("Year / weekday: %d / %d\r\n", year, week);
    return week;
}
