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
#include <math.h>


#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
	
//#define CMD_PREFIX          				"[CMD]"
//#define CMD_PREFIX_LEN      				(5)
#define DOUBLE_PRESS_COUNT						(250)

#define EVB_BOARD											(0)
	
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
	
enum{
		Key_Idle = 0,
		Key_Debounce,
		Key_Wait_Release
};
volatile uint8_t key_state = Key_Idle;




// Scan keys
void Key_Scan(void)
{
	static uint32_t local_tick;
	
	uint8_t up_press;
	uint8_t down_press;
	uint8_t enter_press;
	
	if(0 == g_clock_time_exceed(local_tick, 20)){								
				return;
	}
	local_tick = time_tick;

	// ========== read key status ==========	
#if(EVB_BOARD)
	up_press = (GPIO_ReadInputDataBit(UP_KEY_PORT, UP_KEY_PIN) == 1) ? 1 : 0;
	down_press = (GPIO_ReadInputDataBit(DOWN_KEY_PORT, DOWN_KEY_PIN) == 1) ? 1 : 0;
	enter_press = (GPIO_ReadInputDataBit(ENTER_KEY_PORT, ENTER_KEY_PIN) == 1) ? 1 : 0;
#else
	up_press = (GPIO_ReadInputDataBit(UP_KEY_PORT, UP_KEY_PIN) == 0) ? 1 : 0;
	down_press = (GPIO_ReadInputDataBit(DOWN_KEY_PORT, DOWN_KEY_PIN) == 0) ? 1 : 0;
	enter_press = (GPIO_ReadInputDataBit(ENTER_KEY_PORT, ENTER_KEY_PIN) == 0) ? 1 : 0;
#endif
	
	switch(key_state){
			case Key_Idle:
					if(key.key_active == 0){
							if(up_press){
									key.key_press_cnt[UPKEY - 1]++;
									key_state = Key_Debounce;
									key.key_press = UPKEY;
							}else if(down_press){
									key.key_press_cnt[DOWNKEY - 1]++;
									key_state = Key_Debounce;
									key.key_press = DOWNKEY;
							}else if(enter_press){
									key.key_press_cnt[ENTERKEY - 1]++;
									key_state = Key_Debounce;
									key.key_press = ENTERKEY;
							}
					}
			break;

			case Key_Debounce:
					if(key.key_press == UPKEY){
							if(up_press){
									if(key.key_press_cnt[UPKEY - 1] > 1){
											key_state = Key_Wait_Release;				//Ready for check release
									}else{
											key.key_press_cnt[UPKEY - 1]++;			//Count for debounce
									}
							}else{
									key.key_press = NONKEY;									//Noise, back to Idle
									key.key_press_cnt[UPKEY - 1] = 0;
									key_state = Key_Idle;
							}
					}else if(key.key_press == DOWNKEY){
							if(down_press){
									if(key.key_press_cnt[DOWNKEY - 1] > 1){
											key_state = Key_Wait_Release;
									}else{
											key.key_press_cnt[DOWNKEY - 1]++;
									}
							}else{
									key.key_press = NONKEY;
									key.key_press_cnt[DOWNKEY - 1] = 0;
									key_state = Key_Idle;
							}
					}else if(key.key_press == ENTERKEY){
							if(enter_press){
									if(key.key_press_cnt[ENTERKEY - 1] > 1){
											key_state = Key_Wait_Release;
									}else{
											key.key_press_cnt[ENTERKEY - 1]++;
									}
							}else{
									key.key_press = NONKEY;
									key.key_press_cnt[ENTERKEY - 1] = 0;
									key_state = Key_Idle;
							}
					}
			break;

			case Key_Wait_Release:
					if(key.key_press == UPKEY){
							if(up_press == 0){												//UP key release
									key.key_active = 1;
									key.key_val = UPKEY;
									key.key_press = NONKEY;
									key_state = Key_Idle;
									key.key_press_cnt[UPKEY - 1] = 0;
									if((UI_state != STATE_ACTIVE) && (UI_state != STATE_SLEEP) && (UI_state != STATE_SETTING)){
											update_alarm(Setting_Menu_Alarm);
									}else if(UI_state == STATE_SETTING){
											update_alarm(Setting_Digi_Alarm);
									}
									LOGD("Up Key\r\n");
							}else if(up_press && down_press){
									key.key_press = DOUBLEKEY;
									//key_state = Double_Key_Press;
							}
					}else if(key.key_press == DOWNKEY){
							if(down_press == 0){
									key.key_active = 1;
									key.key_val = DOWNKEY;
									key.key_press = NONKEY;
									key_state = Key_Idle;
									key.key_press_cnt[DOWNKEY - 1] = 0;
									if((UI_state != STATE_ACTIVE) && (UI_state != STATE_SLEEP) && (UI_state != STATE_SETTING)){
											update_alarm(Setting_Menu_Alarm);
									}else if(UI_state == STATE_SETTING){
											update_alarm(Setting_Digi_Alarm);
									}
									LOGD("Down Key\r\n");
							}else if(up_press && down_press){
									key.key_press = DOUBLEKEY;
									//key_state = Double_Key_Press;							//State in Key_Wait_Release
							}
					}else if(key.key_press == ENTERKEY){
							if(enter_press == 0){
									key.key_active = 1;
									key.key_val = ENTERKEY;
									key.key_press = NONKEY;
									key_state = Key_Idle;
									key.key_press_cnt[ENTERKEY - 1] = 0;
									if((UI_state != STATE_ACTIVE) && (UI_state != STATE_SLEEP) && (UI_state != STATE_SETTING)){
											update_alarm(Setting_Menu_Alarm);
									}else if(UI_state == STATE_SETTING){
											update_alarm(Setting_Digi_Alarm);
									}
									LOGD("Enter Key\r\n");
							}
					}else if(key.key_press == DOUBLEKEY){
							if(up_press && down_press){
									if((key.key_press_cnt[UPKEY - 1] > 250) && (key.key_press_cnt[DOWNKEY - 1] >250)){
											key.key_active = 1;
											key.key_val = SYS_RESET;
											if((UI_state != STATE_ACTIVE) && (UI_state != STATE_SLEEP) && (UI_state != STATE_SETTING)){
													update_alarm(Setting_Menu_Alarm);
											}else if(UI_state == STATE_SETTING){
													update_alarm(Setting_Digi_Alarm);
											}
											LOGD("Double Key\r\n");
											//key.key_press = NONKEY;
											//key_state = Key_Idle;
									}else{
											key.key_press_cnt[UPKEY - 1]++;
											key.key_press_cnt[DOWNKEY - 1]++;
									}
							}else if((up_press == 0) && (down_press == 0)){
									key.key_press = NONKEY;
									key_state = Key_Idle;
									key.key_press_cnt[UPKEY - 1] = 0;
									key.key_press_cnt[DOWNKEY - 1] = 0;
							}
					}
			break;
			
			default:
					LOGE("Key state error: %d\r\n",key_state);
			break;

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
		uint8_t period;
		const uint8_t *icons[6] = {P1_Icon_24x24, P2_Icon_24x24, P3_Icon_24x24, P4_Icon_24x24, P5_Icon_24x24, P6_Icon_24x24};
		switch(source){
				case Active_Alarm:
						delete_alarm(Active_Alarm);
						if(icon6_red_state)
						{
								icon6_red_state = 0;
							GUI_DrawMonoIcon24x24(4, 131, BLACK, WHITE, Menu_Icon_24x24);  // Draw black Manu Icon
						}
						// Window-open alert must stay visible: ignore sleep timeout, keep full backlight
						if(UI_state == STATE_WINDOW_OPEN_DETECTED){
								Backlight_SetDuty(BACKLIGHT_DUTY_ACTIVE);
						// If in Leave Schedule Confirm state, go to Active first (then Active will timeout to Sleep naturally)
						}else if(UI_state == STATE_LEAVE_SCHEDULE_CONFIRM){
								UI_state = STATE_ACTIVE;
								if(register_alarm(Active_Alarm, ACTIVE_ALIVE_TIME) == eFALSE){
										LOGE("Alarm Full!\r\n");
								}
						}else{
								UI_state = STATE_SLEEP;
								// Set backlight to 20%
								Backlight_SetDuty(g_parameter.sleep_backlight_duty);
								//LOGD("Backlight %d\r\n", g_parameter.sleep_backlight_duty);
						}
				break;
				
				case Setting_Digi_Alarm:
						delete_alarm(Setting_Digi_Alarm);
						UI_state = STATE_ACTIVE;
						//Clear_Number_Area();
						//Display_Number(g_parameter.setting_number, BLACK, 1);  // show_decimal = 1
						//GUI_DrawMonoIcon24x24(100, 45, BLACK, WHITE, Celsius_Icon_24x24);
						//if(register_alarm(Active_Alarm, ACTIVE_ALIVE_TIME) == eFALSE){
						//		LOGE("Alarm Fail\r\n");
						//}
						main_display_digi = Display_Room_Temp;
						Backlight_SetDuty(BACKLIGHT_DUTY_ACTIVE);
						if(register_alarm(Active_Alarm, ACTIVE_ALIVE_TIME) == eFALSE){
								LOGE("Alarm Fail\r\n");
						}
						Draw_Active_Menu();
				break;
						
				case Setting_Menu_Alarm:
							if(UI_state == STATE_WINDOW_OPEN_CONFIRM){
									delete_alarm(Setting_Menu_Alarm);
									UI_state = STATE_WINDOW_OPEN_DETECTED;
									LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
									Draw_Active_Menu();
							}else if(child_lock_flag == 0){ 

								delete_alarm(Setting_Menu_Alarm);
								UI_state = STATE_ACTIVE;
								main_display_digi = Display_Room_Temp;
								LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
								Backlight_SetDuty(BACKLIGHT_DUTY_ACTIVE);
								if(register_alarm(Active_Alarm, ACTIVE_ALIVE_TIME) == eFALSE){
										LOGE("Alarm Fail\r\n");
								}
								Draw_Active_Menu();
						}else{
								delete_alarm(Setting_Menu_Alarm);
								UI_state = STATE_SLEEP;
								icon6_red_state = 0;
								Backlight_SetDuty(g_parameter.sleep_backlight_duty);
								LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
								Draw_Active_Menu();
						}
						// Reset to Active state (for any state that uses this alarm)
						//UI_state = STATE_ACTIVE;
						// Register Active_Alarm for normal active timeout
						//if(register_alarm(Active_Alarm, ACTIVE_ALIVE_TIME) == eFALSE){
						//		LOGE("Alarm Full!\r\n");
						//}
				break;
				
				case Min_Update_Alarm:
						update_alarm(Min_Update_Alarm);

						// Adaptive Start: evaluate preheat decision every minute
						adaptive_start_minute_task();

						// Adaptive Start: per-minute CSV status log (experiment)
						adaptive_start_log_status();

						// Update Day/Night status for PWM mode
						PWM_Update_DayNight_State();

						if(adc_ctrl.idle_comp_count < INT_NTC_COMP_STEP){
								adc_ctrl.idle_comp_count++;
								adc_ctrl.idle_comp_val = adc_ctrl.idle_comp_step_val * adc_ctrl.idle_comp_count;
						}
						if(Relay == RELAY_ON){
								if(adc_ctrl.heating_comp_count < HEAT_COMP_STEP){
										adc_ctrl.heating_comp_count++;
										adc_ctrl.heating_comp_val = adc_ctrl.heatting_comp_step_val * adc_ctrl.heating_comp_count;
								}
						}else{
								if(adc_ctrl.heating_comp_count > 0){
										adc_ctrl.heating_comp_count--;
										adc_ctrl.heating_comp_val = adc_ctrl.heatting_comp_step_val * adc_ctrl.heating_comp_count;
								}
						}
						//Update Main Page Time
						if((UI_state == STATE_ACTIVE) || (UI_state == STATE_SLEEP) || (UI_state == STATE_WINDOW_OPEN_DETECTED)){
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
										// New day: refresh today's sunrise/sunset for PWM Astro Time
										if(g_parameter.operation_mode == 2){
												Sun_Update_Today();
												PWM_Update_DayNight_State();
										}
										LCD_Fill(84, 5, 128, 37, WHITE);
										if ((g_parameter.current_prog_type == 0 && (weekday == 5 || weekday == 6)) || (g_parameter.current_prog_type == 1 && weekday == 6)) {
												if(g_parameter.language == LANG_ENGLISH){
														Show_Str(86, 16, RED, WHITE, en_week_texts[weekday], 32, 1);
												}else if(g_parameter.language == LANG_Norwegian){
														Show_Str(86, 16, RED, WHITE, no_week_texts[weekday], 32, 1);
												}
										}else{
												if(g_parameter.language == LANG_ENGLISH){
														Show_Str(86, 16, BLUE, WHITE, en_week_texts[weekday], 32, 1);
												}else if(g_parameter.language == LANG_Norwegian){
														Show_Str(86, 16, BLUE, WHITE, no_week_texts[weekday], 32, 1);
												}
										}
								}
								//Update Schedule Period
								if(g_parameter.operation_mode == 1){
										period = Schedule_Period;
										get_schedule_period();
										LCD_Fill(28, 131, 52, 155, WHITE);
										GUI_DrawMonoIcon24x24(28, 131, BLACK, WHITE, icons[Schedule_Period]);
										if(period != Schedule_Period){
												LOGD("Schedule Period : %d\r\n", Schedule_Period);
										}
								}
								//LOGD("Min Alarm time out\r\n");
#if(SIMULATION)
								weekday = getWeekday(rtc_time.Year, rtc_time.Mon, rtc_time.Date);
								LCD_Fill(132, 106, 160, 120, WHITE);
								if ((g_parameter.current_prog_type == 0 && (weekday == 5 || weekday == 6)) || (g_parameter.current_prog_type == 1 && weekday == 6)) {
										Show_Str(132, 106, RED, WHITE, en_week_texts[weekday], 16, 0);
								}else{
										Show_Str(132, 106, BLUE, WHITE, en_week_texts[weekday], 16, 0);
								}
#endif
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
		//day += 1;
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


void get_schedule_period(void)
{
		uint8_t i;
		uint8_t (*sched_ptr)[4];
		uint32_t current_min, sch_min;
	
		// Default to workday schedule: prevents wild-pointer HardFault
		// if current_prog_type holds an invalid value (e.g. corrupted Flash)
		sched_ptr = g_parameter.workday_schedule;
		sch_table = sched_ptr;

		if(g_parameter.current_prog_type == 0){
								if(weekday < 5){
										sched_ptr = g_parameter.workday_schedule;
										sch_table = sched_ptr;
								}else{
										sched_ptr = g_parameter.holiday_schedule;
										sch_table = sched_ptr;
								}
						}else if(g_parameter.current_prog_type == 1){
								if(weekday < 6){
										sched_ptr = g_parameter.workday_schedule;
										sch_table = sched_ptr;
								}else{
										sched_ptr = g_parameter.holiday_schedule;
										sch_table = sched_ptr;
								}
						}else if(g_parameter.current_prog_type == 2){
								sched_ptr = g_parameter.workday_schedule;
								sch_table = sched_ptr;
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
						//LOGD("S_Period:%d\r\n", Schedule_Period);
}

//---------------------------------------------------------------------------------------------------------
// PWM Astro Time: sunrise / sunset calculation
//      - NOAA / Meeus simplified solar position equations ("the sunrise equation"),
//        accurate to about +/-1-2 minutes - more than enough for the day/night PWM
//        feature. Uses single-precision (float) math: Cortex-M3 has no FPU, so this
//        keeps the pulled-in soft-float trig library as small as possible.
//      - g_parameter.astro_day_hour/min and astro_night_hour/min are now COMPUTED
//        results (sunrise/sunset + user tolerance), refreshed once a day. They are
//        no longer directly user-edited (that UI is now Location/Tolerance), so this
//        deliberately does NOT call Flash_Save_Parameter() - only the Location and
//        Tolerance settings themselves are persisted, from the UI Save handlers.
//---------------------------------------------------------------------------------------------------------

#define SUN_PI								(3.14159265358979323846f)
#define SUN_DEG2RAD(x)				((x) * (SUN_PI / 180.0f))
#define SUN_RAD2DEG(x)				((x) * (180.0f / SUN_PI))

typedef struct
{
    float   lat;          // degrees, +N
    float   lon;          // degrees, +E
    int16_t tz_std_min;   // STANDARD (winter) time UTC offset, in minutes
} sun_city_t;

// Oslo/Stockholm are CET/CEST (UTC+1 standard); Helsinki is EET/EEST (UTC+2
// standard) - one hour further east. All three switch DST on the same EU/EEA
// dates (see rtc_driver.c), just from a different standard-time base.
static const sun_city_t sun_city_table[ASTRO_LOCATION_MAX] = {
    [ASTRO_LOCATION_OSLO]      = {59.9139f, 10.7522f, 60},
    [ASTRO_LOCATION_STOCKHOLM] = {59.3293f, 18.0686f, 60},
    [ASTRO_LOCATION_HELSINKI]  = {60.1699f, 24.9384f, 120},
};

//---------------------------------------------------------------------------------------------------------
// Function: Sun_DayOfYear
// Description: Day-of-year (1-366) for the current RTC date
//---------------------------------------------------------------------------------------------------------
static uint16_t Sun_DayOfYear(uint16_t full_year, uint8_t month, uint8_t day)
{
    const uint8_t dim[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    uint16_t doy = day;
    uint8_t m;
    uint8_t leap = ((full_year % 4 == 0 && full_year % 100 != 0) || (full_year % 400 == 0)) ? 1 : 0;

    for (m = 0; m < month - 1; m++)
    {
        doy += dim[m];
        if (m == 1 && leap) doy += 1;
    }
    return doy;
}

//---------------------------------------------------------------------------------------------------------
// Function: Sun_Calc_Rise_Set
// Description: Compute today's sunrise/sunset, in LOCAL clock minutes-after-midnight
//      (tz_offset_min already applied).
// Input: lat_deg/lon_deg - location; full_year/month/day - date;
//        tz_offset_min - local UTC offset in minutes (standard or DST, caller's choice)
// Output: *sunrise_min, *sunset_min - minutes after midnight, local time (0-1439)
// Return: 1 = normal day/night, 0 = sun never rises today (polar night),
//         2 = sun never sets today (midnight sun)
//      Note: none of Oslo/Stockholm/Helsinki (~59-60N) actually reach polar day/night
//      (that starts ~66.5N, the Arctic Circle) - this is defensive handling only.
//---------------------------------------------------------------------------------------------------------
static uint8_t Sun_Calc_Rise_Set(float lat_deg, float lon_deg, uint16_t full_year, uint8_t month, uint8_t day,
                                   int16_t tz_offset_min, int16_t *sunrise_min, int16_t *sunset_min)
{
    uint16_t doy = Sun_DayOfYear(full_year, month, day);
    float gamma, eqtime, decl, lat_rad;
    float cos_h, ha_deg;
    float solar_noon_utc_min;

    // Fractional year (radians)
    gamma = 2.0f * SUN_PI / 365.0f * (float)(doy - 1);

    // Equation of time (minutes) - Spencer's Fourier approximation
    eqtime = 229.18f * (0.000075f
                        + 0.001868f * cosf(gamma)
                        - 0.032077f * sinf(gamma)
                        - 0.014615f * cosf(2.0f * gamma)
                        - 0.040849f * sinf(2.0f * gamma));

    // Solar declination (radians) - Spencer's Fourier approximation
    decl = 0.006918f
           - 0.399912f * cosf(gamma)
           + 0.070257f * sinf(gamma)
           - 0.006758f * cosf(2.0f * gamma)
           + 0.000907f * sinf(2.0f * gamma)
           - 0.002697f * cosf(3.0f * gamma)
           + 0.00148f  * sinf(3.0f * gamma);

    lat_rad = SUN_DEG2RAD(lat_deg);

    // Hour angle at sunrise/sunset (90.833deg accounts for atmospheric refraction
    // + solar disk radius)
    cos_h = (cosf(SUN_DEG2RAD(90.833f)) - sinf(lat_rad) * sinf(decl)) / (cosf(lat_rad) * cosf(decl));

    if (cos_h > 1.0f)
    {
        return 0;   // Sun never rises today (polar night)
    }
    if (cos_h < -1.0f)
    {
        return 2;   // Sun never sets today (midnight sun)
    }

    ha_deg = SUN_RAD2DEG(acosf(cos_h));

    // Solar noon, in UTC minutes
    solar_noon_utc_min = 720.0f - 4.0f * lon_deg - eqtime;

    *sunrise_min = (int16_t)(solar_noon_utc_min - 4.0f * ha_deg + (float)tz_offset_min + 0.5f);
    *sunset_min  = (int16_t)(solar_noon_utc_min + 4.0f * ha_deg + (float)tz_offset_min + 0.5f);

    while (*sunrise_min < 0)     *sunrise_min += 1440;
    while (*sunrise_min >= 1440) *sunrise_min -= 1440;
    while (*sunset_min < 0)      *sunset_min += 1440;
    while (*sunset_min >= 1440)  *sunset_min -= 1440;

    return 1;
}

//---------------------------------------------------------------------------------------------------------
// Function: Sun_Update_Today
// Description: Recompute today's sunrise/sunset for g_parameter.location, apply
//      g_parameter.astro_tolerance_min, and store the result into
//      g_parameter.astro_day_hour/min and astro_night_hour/min (the same fields the
//      PWM day/night comparison already reads - no other code needs to change).
//      Called at boot (if PWM Mode), when switching into PWM Mode, right after saving
//      a new Location/Tolerance while already in PWM Mode, and every midnight.
//      Does NOT write to Flash - this is a computed value, refreshed daily.
//---------------------------------------------------------------------------------------------------------
void Sun_Update_Today(void)
{
    const sun_city_t *city;
    uint8_t city_idx;
    int16_t tz_offset_min;
    int16_t sunrise_min, sunset_min;
    int16_t day_min, night_min;
    uint8_t result;
    uint16_t full_year;

    city_idx = (g_parameter.location < ASTRO_LOCATION_MAX) ? g_parameter.location : ASTRO_LOCATION_OSLO;
    city = &sun_city_table[city_idx];
    tz_offset_min = city->tz_std_min + (RTC_Is_DST_Active_Today() ? 60 : 0);

    full_year = 2000 + rtc_time.Year;
    result = Sun_Calc_Rise_Set(city->lat, city->lon, full_year, rtc_time.Mon, rtc_time.Date,
                                tz_offset_min, &sunrise_min, &sunset_min);

    if (result == 0)
    {
        // Polar night: sun never rises - collapse the "day" window to zero width,
        // so the day/night compare (now >= day_start && now < night_start) is
        // always false => always night.
        day_min = 0;
        night_min = 0;
    }
    else if (result == 2)
    {
        // Midnight sun: sun never sets - make the "day" window cover the full day.
        day_min = 0;
        night_min = 1439;
    }
    else
    {
        day_min = sunrise_min + g_parameter.astro_tolerance_min;
        night_min = sunset_min + g_parameter.astro_tolerance_min;
        while (day_min < 0)     day_min += 1440;
        while (day_min >= 1440) day_min -= 1440;
        while (night_min < 0)     night_min += 1440;
        while (night_min >= 1440) night_min -= 1440;
    }

    g_parameter.astro_day_hour   = (uint8_t)(day_min / 60);
    g_parameter.astro_day_min    = (uint8_t)(day_min % 60);
    g_parameter.astro_night_hour = (uint8_t)(night_min / 60);
    g_parameter.astro_night_min  = (uint8_t)(night_min % 60);

    LOGD("Astro Calc: Location=%d Date=%d/%d/%d Sunrise=%02d:%02d Sunset=%02d:%02d Tolerance=%d -> Day=%02d:%02d Night=%02d:%02d\r\n",
         city_idx, full_year, rtc_time.Mon, rtc_time.Date,
         (result == 1) ? sunrise_min / 60 : 0, (result == 1) ? sunrise_min % 60 : 0,
         (result == 1) ? sunset_min / 60 : 0, (result == 1) ? sunset_min % 60 : 0,
         g_parameter.astro_tolerance_min,
         g_parameter.astro_day_hour, g_parameter.astro_day_min,
         g_parameter.astro_night_hour, g_parameter.astro_night_min);
}

//---------------------------------------------------------------------------------------------------------
// Function: PWM_Update_DayNight_State
// Description: Recompute pwm_is_daytime from the current RTC time against
//      g_parameter.astro_day_hour/min and astro_night_hour/min. Shared by boot
//      (main.c), the once-a-minute PWM update (below), and the Location/Tolerance
//      Save handlers (so a change takes effect immediately, not at next midnight).
//---------------------------------------------------------------------------------------------------------
void PWM_Update_DayNight_State(void)
{
    uint16_t now_min = rtc_time.Hour * 60 + rtc_time.Min;
    uint16_t day_start = g_parameter.astro_day_hour * 60 + g_parameter.astro_day_min;
    uint16_t night_start = g_parameter.astro_night_hour * 60 + g_parameter.astro_night_min;
    pwm_is_daytime = (now_min >= day_start && now_min < night_start) ? 1 : 0;
}

volatile uint16_t heating_count = 0;          // Consecutive ON ticks (500ms each)
volatile uint16_t power_off_count = 0;         // Ticks spent in forced-OFF period
volatile uint8_t power_limit_trigger = 0;      // 1 = currently in forced-OFF period

// PWM Mode variables
static uint8_t pwm_initialized = 0;
static uint32_t pwm_start_tick = 0;
uint8_t pwm_is_daytime = 0;  // 1=Day duty, 0=Night duty (updated every minute)

void relay_update(float temp)
{
		if(g_parameter.operation_mode == 0){			//Manual Mode
					if(g_parameter.setting_number <= temp){
							Relay = RELAY_OFF;
							GPIO_WriteBit(RELAY_PORT, RELAY_PIN, Bit_RESET);
							g_parameter.relay_staty = Relay;
							heating_count = 0;
					}else if(temp < (g_parameter.setting_number - g_parameter.temp_swing)){
							Relay = RELAY_ON;
							GPIO_WriteBit(RELAY_PORT, RELAY_PIN, Bit_SET);
							g_parameter.relay_staty = Relay;
					}
		}else{																		//Program Mode
					float target_temp;
					uint8_t period_on;

					get_schedule_period();								//Get time period and schedule table

					// Override target temperature during adaptive preheat
					if (adaptive_start_is_active())
					{
							target_temp = adaptive_start_get_target();
							period_on = 1;
					}
					else
					{
							target_temp = (float)(sch_table[Schedule_Period][2]);
							period_on = sch_table[Schedule_Period][3];
					}

					if(period_on){			//Thermostat is on
							if(target_temp <= temp){
									Relay = RELAY_OFF;
									GPIO_WriteBit(RELAY_PORT, RELAY_PIN, Bit_RESET);
									g_parameter.relay_staty = Relay;
									heating_count = 0;
							}else if(temp < (target_temp - g_parameter.temp_swing)){
									Relay = RELAY_ON;
									GPIO_WriteBit(RELAY_PORT, RELAY_PIN, Bit_SET);
									g_parameter.relay_staty = Relay;
							}
					}else{																			//Thermostat is off
							Relay = RELAY_OFF;
							GPIO_WriteBit(RELAY_PORT, RELAY_PIN, Bit_RESET);
							g_parameter.relay_staty = Relay;
							heating_count = 0;
					}
		}
	
}

void handle_pwm_relay(void)
{
		uint32_t cycle_ms = 1200000;  // 20 minutes = 1,200,000 ms
		uint32_t on_ms;
		uint8_t duty;
		static uint32_t pwm_cycle_pos = 0;	// Position within current cycle (ms)

		duty = pwm_is_daytime ? g_parameter.pwm_day_duty : g_parameter.pwm_night_duty;

		if(!pwm_initialized){
				pwm_cycle_pos = 0;
				pwm_start_tick = time_tick;
				pwm_initialized = 1;
		} else {
				// Advance cycle position by time elapsed since last call
				uint32_t now = time_tick;
				uint32_t delta = now - pwm_start_tick;
				if(delta > 0){
						pwm_cycle_pos += delta;
						// Keep position within a single cycle window
						if(pwm_cycle_pos >= cycle_ms){
								pwm_cycle_pos %= cycle_ms;
						}
						pwm_start_tick = now;
				}
		}

		on_ms = cycle_ms * duty / 100;

		if(pwm_cycle_pos < on_ms){
				Relay = RELAY_ON;
				GPIO_WriteBit(RELAY_PORT, RELAY_PIN, Bit_SET);
		}else{
				Relay = RELAY_OFF;
				GPIO_WriteBit(RELAY_PORT, RELAY_PIN, Bit_RESET);
		}
		g_parameter.relay_staty = Relay;
}

void check_temp_set(float temp)
{
			uint16_t on_limit, off_limit;
			
			if(g_parameter.power_limit_switch == 0){
					// Power limit disabled: normal temperature-based control
					relay_update(temp);
					return;
			}
			
			// Power limit active: divide 1 hour into 3 segments of 20 minutes
			// 20 min = 1200 seconds = 2400 ticks (at 500ms per tick)
			// OFF ticks per segment = (power_limit / 3 min) * 60 * 2 = power_limit * 40
			// ON ticks per segment = 2400 - OFF ticks
			off_limit = g_parameter.power_limit * 40;   // e.g. 6*40=240 ticks = 2 min
			on_limit = 2400 - off_limit;                 // e.g. 2400-240=2160 ticks = 18 min
			
			if(power_limit_trigger){
					// Forced OFF period: keep relay OFF regardless of temperature
					power_off_count++;
					Relay = RELAY_OFF;
					GPIO_WriteBit(RELAY_PORT, RELAY_PIN, Bit_RESET);
					g_parameter.relay_staty = Relay;
					
					if(power_off_count >= off_limit){
							// Forced OFF period complete: reset, let normal control resume
							power_limit_trigger = 0;
							power_off_count = 0;
							heating_count = 0;
					}
			}else{
					// Normal operation: temperature-based control
					relay_update(temp);
					
					if(Relay == RELAY_ON){
							heating_count++;
							if(heating_count >= on_limit){
									// Continuous heating exceeded: enter forced OFF
									power_limit_trigger = 1;
									power_off_count = 0;
									Relay = RELAY_OFF;
									GPIO_WriteBit(RELAY_PORT, RELAY_PIN, Bit_RESET);
									g_parameter.relay_staty = Relay;
							}
					}else{
							// Relay OFF due to temperature: reset counter
							heating_count = 0;
					}
			}
}

//---------------------------------------------------------------------------------------------------------
// Funcation: window_function_reset
// Description: Reset window function monitoring state
// Input: None
// Output: None
// Date: 2026/05/20
// Update:
//---------------------------------------------------------------------------------------------------------
void window_function_reset(void)
{
		memset((void*)window_fun_temp_buffer, 0, sizeof(window_fun_temp_buffer));
		window_fun_buffer_count = 0;
		window_fun_buffer_index = 0;
		memset((void*)window_fun_buffer_a, 0, sizeof(window_fun_buffer_a));
		window_fun_buffer_a_count = 0;
		window_fun_b_divider = 0;
		window_fun_int_updated = 0;
		window_fun_ext_updated = 0;
		window_fun_triggered = 0;
		LOGD("Window function reset\r\n");
}

//---------------------------------------------------------------------------------------------------------
// Funcation: g_relay_handler
// Description: Handler Relay status
// Input: 
// Output: 
// Date: 2026/05/17
// Update: 
//---------------------------------------------------------------------------------------------------------
void g_relay_handler(void)
{
		float temp;
		float ext_temp;
		//float display_temp;
		uint8_t old_relay_state;
		static uint32_t local_tick;
		uint8_t protect_high_triggered = 0;
		uint8_t protect_low_triggered = 0;
		static uint8_t last_window_fun_state = 0xFF;
		static uint8_t window_open_flash_cnt = 0;
		static float window_open_cached_temp = 0.0f;
		static uint8_t alert_backlight_active = 0;		// 1 = backlight currently forced by alert (QA Spec 5.2)

		if(0 == g_clock_time_exceed(local_tick, 500)){
				return;
		}
		local_tick = time_tick;

		if(Factory_testing != 0){
				return;
		}

		old_relay_state = Relay;

		//Phase 1 : wait dual sensor read data
#if(NO_Power_Board)
		if(adc_ctrl.int_ntc_valid == 0){
				return;
		}
#else
		if(adc_ctrl.int_ntc_valid == 0 || adc_ctrl.ext_ntc_valid == 0){
				return;
		}
#endif
		
		//Phase 2: Persistent error → Set Relay to default value
		if(adc_ctrl.int_sensor_error || adc_ctrl.ext_sensor_error) {
				// QA Spec 5.2: sensor error forces full backlight
				Backlight_SetDuty(BACKLIGHT_DUTY_ACTIVE);
				alert_backlight_active = 1;
				if(g_parameter.power_on_state == 2){				//Relay OFF
						Relay = RELAY_OFF;
						GPIO_WriteBit(RELAY_PORT, RELAY_PIN, Bit_RESET);
						g_parameter.relay_staty = Relay;
				}
				if(g_parameter.power_on_state == 3){				//Relay ON
						Relay = RELAY_ON;
						GPIO_WriteBit(RELAY_PORT, RELAY_PIN, Bit_SET);			//Open relay
						g_parameter.relay_staty = Relay;
				}
				return;
		}
		
		// Detect window function on/off transition
		if(last_window_fun_state != g_parameter.window_fun){
				last_window_fun_state = g_parameter.window_fun;
				if(g_parameter.window_fun == 1){
						window_function_reset();
				}else{
						window_fun_triggered = 0;
				}
		}
#if(!SIMULATION)
		if(g_parameter.sensor_type == 0){			//Room temp
				if(Average_INT_Temp > -900){
#if(IDLE_COMPENSATE && HEAT_COMPENSATE)
						temp = Average_INT_Temp - adc_ctrl.idle_comp_val - adc_ctrl.heating_comp_val;
#elif(IDLE_COMPENSATE)
						temp = Average_INT_Temp - adc_ctrl.idle_comp_val;
#else
						temp = Average_INT_Temp;
#endif

						temp += g_parameter.temp_correct_internal;
						if(Average_EXT_Temp > -900){
								ext_temp = Average_EXT_Temp;
								ext_temp += g_parameter.temp_correct_external;
						}
				}else{
						return;
				}
		}else{																//Floor temp
				if(Average_EXT_Temp > -900){
						temp = Average_EXT_Temp;
						temp += g_parameter.temp_correct_external;
						ext_temp = temp;
				}else{
						return;
				}
		}
#endif
#if(SIMULATION)
		temp = disp_fack_temp + g_parameter.temp_correct_internal;
		ext_temp = temp;
#else
		
#endif

			// Window function monitoring: always watch INT (room air) sensor -
			// open-window = room temp drop; floor sensor reacts too slowly
			if(g_parameter.window_fun == 1 && window_fun_triggered == 0){
				uint8_t new_temp_ready = 0;
				float win_temp = temp;
				if(window_fun_int_updated){
					window_fun_int_updated = 0;
					#if(!SIMULATION)
					if(adc_ctrl.int_ntc_valid > 0){
						uint8_t newest_idx = (adc_ctrl.int_ntc_count + NTC_BUFFER_SIZE - 1) % NTC_BUFFER_SIZE;
						win_temp = adc_ctrl.INT_NTC_Table[newest_idx];	// instantaneous, not 16-sample avg
						#if(IDLE_COMPENSATE && HEAT_COMPENSATE)
						win_temp -= adc_ctrl.idle_comp_val + adc_ctrl.heating_comp_val;
						#elif(IDLE_COMPENSATE)
						win_temp -= adc_ctrl.idle_comp_val;
						#endif
						win_temp += g_parameter.temp_correct_internal;
						new_temp_ready = 1;
					}
					#else
					new_temp_ready = 1;
					#endif
				}

					if(new_temp_ready){
							uint8_t compare_count;
							uint8_t i;
							uint8_t triggered_now = 0;
							float avg_a;

							// Put new sample into Buffer A (sliding window: keep newest 16, ~1 min)
							if(window_fun_buffer_a_count >= 16){
								for(i = 0; i < 15; i++){
									window_fun_buffer_a[i] = window_fun_buffer_a[i+1];
								}
								window_fun_buffer_a_count = 15;
							}
							window_fun_buffer_a[window_fun_buffer_a_count] = win_temp;
							window_fun_buffer_a_count++;
							
							// Every 16 samples (~1 min): push Buffer A average into Buffer B
							window_fun_b_divider++;
							if(window_fun_b_divider >= 16){
								window_fun_b_divider = 0;
								avg_a = 0.0f;
								for(i = 0; i < window_fun_buffer_a_count; i++){
									avg_a += window_fun_buffer_a[i];
								}
								avg_a /= window_fun_buffer_a_count;
								window_fun_temp_buffer[window_fun_buffer_index] = avg_a;
								window_fun_buffer_index = (window_fun_buffer_index + 1) % 60;
								if(window_fun_buffer_count < 60){
									window_fun_buffer_count++;
								}
							}

							// Check 1: newest sample vs all previous samples in Buffer A (fast drop within ~1 min)
							for(i = 0; i < window_fun_buffer_a_count - 1; i++){
									float temp_drop = window_fun_buffer_a[i] - win_temp;
									if(temp_drop > g_parameter.window_fun_temp){
											triggered_now = 1;
											LOGD("Window function triggered (Buffer A)! Drop: %.1f C\r\n", temp_drop);
											break;
									}
							}

							// Check 2: Buffer A average vs Buffer B history (each entry ~= 1 minute)
							if(triggered_now == 0){
									avg_a = 0.0f;
									for(i = 0; i < window_fun_buffer_a_count; i++){
											avg_a += window_fun_buffer_a[i];
									}
									avg_a /= window_fun_buffer_a_count;

									compare_count = g_parameter.window_fun_time;
									if(window_fun_buffer_count < compare_count){
											compare_count = window_fun_buffer_count;
									}
									for(i = 0; i < compare_count; i++){
											uint8_t check_idx = (window_fun_buffer_index + 60 - 1 - i) % 60;
											float temp_drop = window_fun_temp_buffer[check_idx] - avg_a;
											if(temp_drop > g_parameter.window_fun_temp){
													triggered_now = 1;
													LOGD("Window function triggered (Buffer B)! Drop: %.1f C\r\n", temp_drop);
													break;
											}
									}
							}

							if(triggered_now){
									window_fun_triggered = 1;
									window_open_cached_temp = win_temp;
									UI_state = STATE_WINDOW_OPEN_DETECTED;
									Backlight_SetDuty(BACKLIGHT_DUTY_ACTIVE);
									LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
									Draw_Active_Menu();
							}
					}
			}

			if(g_parameter.temp_protect_max_switch == 1 && ext_temp >= g_parameter.temp_protect_max){
					Relay = RELAY_OFF;
					GPIO_WriteBit(RELAY_PORT, RELAY_PIN, Bit_RESET);
					protect_high_triggered = 1;
					g_parameter.relay_staty = Relay;
			}else{
					if(g_parameter.temp_protect_max_switch == 1 && (ext_temp < g_parameter.temp_protect_max - g_parameter.temp_swing)){
							protect_high_triggered = 0;
					}
			}

			if(g_parameter.temp_protect_min_switch == 1 && ext_temp <= g_parameter.temp_protect_min){
					Relay = RELAY_ON;
					GPIO_WriteBit(RELAY_PORT, RELAY_PIN, Bit_SET);
					g_parameter.relay_staty = Relay;
					protect_low_triggered = 1;
			}else{
					if(g_parameter.temp_protect_min_switch == 1 && (ext_temp > g_parameter.temp_protect_min + g_parameter.temp_swing)){
							protect_low_triggered = 0;
					}
			}

			if(!protect_high_triggered && !protect_low_triggered){
					if(g_parameter.window_fun == 1 && window_fun_triggered == 1){
							// Window open detected: suspend heating in ALL modes (incl. PWM)
							Relay = RELAY_OFF;
							GPIO_WriteBit(RELAY_PORT, RELAY_PIN, Bit_RESET);
							g_parameter.relay_staty = Relay;
					}else{
							if(g_parameter.operation_mode == 2){
									handle_pwm_relay();
							}else{
									check_temp_set(temp);
							}
					}
			}
			
			

			// QA Spec 5.2: overheat/frost protection forces full backlight; restore on clear
		if(protect_high_triggered || protect_low_triggered){
				alert_backlight_active = 1;
				Backlight_SetDuty(BACKLIGHT_DUTY_ACTIVE);
		}else if(alert_backlight_active){
				alert_backlight_active = 0;		// Alert cleared: restore backlight for current UI state
				if(UI_state == STATE_SLEEP){
						Backlight_SetDuty(g_parameter.sleep_backlight_duty);
				}else{
						Backlight_SetDuty(BACKLIGHT_DUTY_ACTIVE);
				}
		}
		
		// Window open detected: flash pattern RED -> blank -> BLACK -> blank (each 500ms)
			// Update cached temperature when ADC signals new average temp is ready
			if(UI_state == STATE_WINDOW_OPEN_DETECTED){
					Backlight_SetDuty(BACKLIGHT_DUTY_ACTIVE);
					if(display_update_throttle_trigger){
							display_update_throttle_trigger = 0;
							if(g_parameter.sensor_type == 0) {
#if(IDLE_COMPENSATE && HEAT_COMPENSATE)
									window_open_cached_temp = Average_INT_Temp - adc_ctrl.idle_comp_val - adc_ctrl.heating_comp_val;
#elif(IDLE_COMPENSATE)
									window_open_cached_temp = Average_INT_Temp - adc_ctrl.idle_comp_val;
#else
									window_open_cached_temp = Average_INT_Temp;
#endif

									window_open_cached_temp += g_parameter.temp_correct_internal;
							} else {
									window_open_cached_temp = Average_EXT_Temp;
									window_open_cached_temp += g_parameter.temp_correct_external;
							}
					}
					
					window_open_flash_cnt++;
					if(window_open_flash_cnt >= 4){
							window_open_flash_cnt = 0;
					}
					if(window_open_cached_temp > -99){
							switch(window_open_flash_cnt){
									case 0:
											Display_Number(window_open_cached_temp, RED, 1);
											break;
									case 1:
											Clear_Number_Area();
											break;
									case 2:
											Display_Number(window_open_cached_temp, BLACK, 1);
											break;
									case 3:
											Clear_Number_Area();
											break;
							}
					}
			}


			if(old_relay_state != Relay){
					Update_Relay = eTRUE;
			}

}

//---------------------------------------------------------------------------------------------------------
// Funcation: g_check_power_off_time
// Description: Calculate shutdown time for temperature compensation
// Input: 
// Output: 
// Date: 2026/06/06
// Update: 
//---------------------------------------------------------------------------------------------------------
uint32_t g_check_power_off_time(void)
{
		uint32_t elapsed;
	
		elapsed = (rtc_time.Year - g_parameter.backup_rtc.Year) * 525600; 		// One Year 
		elapsed += (rtc_time.Mon - g_parameter.backup_rtc.Mon) * 43200;				// One Month
		elapsed += (rtc_time.Date - g_parameter.backup_rtc.Date) * 1440;			// One Date
		elapsed += (rtc_time.Hour - g_parameter.backup_rtc.Hour) * 60;				// One Hour
		elapsed += (rtc_time.Min - g_parameter.backup_rtc.Min);
	
		return elapsed;
	
}
