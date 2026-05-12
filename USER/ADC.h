//---------------------------------------------------------------------------------------------------------
//                                                                                                         
// Copyright(c) 2026 E-poly Technology Co., Ltd. All rights reserved.                                           
//                                                                                                         
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
// E-Poly North EU Thermostat Project 
// Author: Benjamin Wang
// Date: 2026/04/21
// Email: Benjamin@epoly-tech.com
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
// File Function: ADC header file
//---------------------------------------------------------------------------------------------------------

#ifndef _ADC_H_
#define _ADC_H_

#include <stdint.h>

// Function prototypes
void ADC_Init_Config(void);
void ADC_Start_Channel(uint8_t channel);
void ADC_Start_Conversion(void);  // Deprecated, use ADC_Start_Channel
uint8_t ADC_Wait_Channel_Complete(uint8_t channel, uint32_t timeout_ms);
uint8_t ADC_Wait_Complete(uint32_t timeout_ms);  // Deprecated, use ADC_Wait_Channel_Complete
uint16_t ADC_Calculate_Average(uint8_t channel);
float ADC_Measure_Channel(uint8_t channel);
float ADC_Measure_VCC(void);
void ADC_Init_And_Measure(void);

// External variables
extern volatile uint16_t adc_ch8_buffer[16];   // 16 samples for CH8 (PB0)
extern volatile uint16_t adc_ch9_buffer[16];   // 16 samples for CH9 (PB1)
extern volatile uint8_t adc_ch8_complete;
extern volatile uint8_t adc_ch9_complete;
extern volatile uint8_t adc_current_channel;

#endif // _ADC_H_
