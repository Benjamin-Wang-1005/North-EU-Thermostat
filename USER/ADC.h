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
#define SENSOR_RECOVER_COUNT		10								// Consecutive valid readings needed to clear a sensor error

// Factory test background measurement (2026/10/12)
// VCC and Int/Ext temperature are measured during the LCD test (before the board is pressed
// in the key test), then the result is locked for GET:VCC / GET:TEMP.
#define MEAS_CAPTURE_MS				3000							// Measurement window
#define MEAS_REDO_DELAY_MS			300								// Delay before re-measuring (MEAS:START)
#define MEAS_TEMP_MAX					32								// Max temperature samples kept per channel

#define MEAS_IDLE						0								// Never measured: GET returns live values
#define MEAS_WAIT						1								// Waiting MEAS_REDO_DELAY_MS before measuring
#define MEAS_RUN						2								// Measuring
#define MEAS_DONE						3								// Result locked


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
		uint8_t  int_sensor_error;						// Set on out-of-range reading; cleared after SENSOR_RECOVER_COUNT valid readings
		uint8_t  ext_sensor_error;
		uint8_t  int_valid_run;							// Consecutive valid readings (for sensor error recovery)
		uint8_t  ext_valid_run;
    volatile float INT_NTC_Table[NTC_BUFFER_SIZE];
    volatile float EXT_NTC_Table[NTC_BUFFER_SIZE];
} adc_thermostat_t;







typedef struct {
    uint8_t  state;
    uint32_t tick;
    uint16_t delay_ms;
    float    vcc_sum;
    uint16_t vcc_n;
    float    int_buf[MEAS_TEMP_MAX];
    uint8_t  int_n;
    float    ext_buf[MEAS_TEMP_MAX];
    uint8_t  ext_n;
    float    vcc;                // Locked results (valid when state == MEAS_DONE)
    float    int_temp;
    float    ext_temp;
    uint8_t  vcc_ok;
    uint8_t  int_ok;
    uint8_t  ext_ok;
} meas_capture_t;

extern meas_capture_t meas_cap;

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

void     Meas_Capture_Start(uint16_t delay_ms);







#endif // _ADC_H_



