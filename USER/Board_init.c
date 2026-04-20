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

//---------------------------------------------------------------------------------------------------------
//
//---------------------------------------------------------------------------------------------------------


void my_RTC_Init(void)
	
{
		uint8_t rtc_status;
		// Initialize RTC
		rtc_status = RTC_Init();
		if (rtc_status == RTC_OK)
		{
				// Check if RTC was previously configured
				if (RTC_IsConfigured())
				{
						// Read current time from RTC
						RTC_ReadTime(&rtc_time);
				}
				else
				{
						// First time boot - set default time
						RTC_GetDefaultTime(&rtc_time);
						RTC_SetTime(&rtc_time);
				}
		
				// Start RTC counting
				RTC_Start();
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
