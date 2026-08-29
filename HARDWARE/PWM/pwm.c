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
//File Function: pwm.c
//		- Hardware PWM relative function call
//---------------------------------------------------------------------------------------------------------

#include "stm32f10x_gpio.h"

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
