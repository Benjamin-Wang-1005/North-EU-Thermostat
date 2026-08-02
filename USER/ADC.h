//---------------------------------------------------------------------------------------------------------
//
// Copyright(c) 2026 E-Poly Technology Co., Ltd. All rights reserved.
//
//---------------------------------------------------------------------------------------------------------



//---------------------------------------------------------------------------------------------------------
// E-Poly North EU Thermostat Project
// Author: Benjamin Wang
// Date: 2026/05/13
// Email: Benjamin@epoly-tech.com
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
// File Function: ADC header file
//      - ADC1 Channel 8 (PB0) and Channel 9 (PB1) via interrupt
//      - 16 samples per conversion, averaged
//      - 18-slot filtered buffer with min/max discard
//---------------------------------------------------------------------------------------------------------



#ifndef _ADC_H_
#define _ADC_H_


#include <stdint.h>

#define ADC_SAMPLE_COUNT        16
#define NTC_BUFFER_SIZE					16
#define ADC_AVG_BUFFER_SIZE     18
#define INT_NTC_COMPENSATE			1.75f
#define INT_NTC_COMP_STEP				40								//Thermostat increase 1.75C at 40 min
#define IDLE_COMPENSATE					1
#define HEAT_COMPENSATE_VAL			6.5f
#define HEAT_COMP_STEP					40
#define HEAT_COMPENSATE					1


typedef struct {
    volatile uint16_t raw_buffer[ADC_SAMPLE_COUNT];
    volatile uint8_t  raw_count;
    volatile uint8_t  complete;
    uint16_t avg_buffer[ADC_AVG_BUFFER_SIZE];
    uint8_t  avg_index;
    uint8_t  avg_count;
} adc_channel_data_t;



typedef struct {
    adc_channel_data_t ch8;
    adc_channel_data_t ch9;
    float    vcc_voltage;
		float		 idle_comp_step_val;
		float		 idle_comp_val;
		uint8_t  idle_comp_count;
		float		 heatting_comp_step_val;
		float 	 heating_comp_val;
		uint8_t  heating_comp_count;
    volatile uint16_t vcc_raw_buffer[ADC_SAMPLE_COUNT];
    volatile uint8_t  vcc_raw_count;
    volatile uint8_t  vcc_complete;
    uint8_t  next_channel;
    uint32_t last_tick;
    uint8_t  ch8_log_counter;
    uint8_t  ch9_log_counter;
    volatile uint8_t  adc_busy;
    uint32_t last_vcc_tick;
    uint8_t  int_ntc_count;
    uint8_t  ext_ntc_count;
    uint8_t  int_ntc_valid;
    uint8_t  ext_ntc_valid;
		uint8_t  int_sensor_error;						//while set to 0, never clear
		uint8_t  ext_sensor_error;
    volatile float INT_NTC_Table[NTC_BUFFER_SIZE];
    volatile float EXT_NTC_Table[NTC_BUFFER_SIZE];
} adc_thermostat_t;







// External instance



extern adc_thermostat_t adc_ctrl;







// Display temperature globals


extern adc_thermostat_t adc_ctrl;
extern volatile float Average_INT_Temp;
extern volatile float Average_EXT_Temp;
extern volatile float current_display_temp;
extern volatile uint8_t display_update_throttle_trigger;  // Set when throttle counter triggers (same rhythm as ACTIVE display update)
extern volatile uint8_t display_refresh_request;







// Function prototypes



void Thermostat_ADC_Init(void);



void ADC_Start_Channel(uint8_t channel);



void ADC_Start_VCC(void);



void Thermostat_Update(void);



void ADC_Update_Display_Temperature(uint8_t);







uint16_t ADC_Get_Channel_Average(uint8_t channel);



float    ADC_Get_VCC_Voltage(void);







#endif // _ADC_H_



