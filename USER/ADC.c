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

// File Function: ADC driver with interrupt mode

//      - ADC1 Channel 8 (PB0) and Channel 9 (PB1)

//      - Channel 17 (Vrefint) for VCC measurement

//      - 16 samples per conversion via ADC EOC interrupt

//      - No DMA

//---------------------------------------------------------------------------------------------------------

#include "Thermostat.h"

static const uint32_t EXT_NTC_table[] =

{ 92250, 87100, 82270, 77740, 73480, 69490, 65730, 62190, 58870, 55750,

	52800, 50030, 47430, 44970, 42650, 40470, 38410, 36460, 34630, 32900,

	31260, 29710, 28240, 26860, 25550, 24310, 23140, 22040, 20990, 20000,

	19060, 18170, 17320, 16520, 15760, 15040, 14360, 13710, 13100, 12510,

	11960, 11430, 10930, 10450, 10000, 9570,  9161,  8772,  8402,  8049,

	7713,  7393,  7087,  6797,  6519,  6255,  6002,  5762,  5532,  5312,

	5103,  4903,  4711,  4529,  4354,  4187,  4027,  3874,  3728,  3588,

	3454,  3325,  3202,  3084,  2971,  2863,  2759,  2660,  2564,  2473,

	2385,  2301,  2221,  2143,  2069,  1998,  1929,  1864,  1800,  1740,

};  //B value 3950K +/- 1%

static const uint32_t INT_NTC_table[] =

{	64440, 61420, 58570, 55870, 53310, 50880, 48590, 46410, 44350, 42390,

	40500, 38700, 37000, 35380, 33850, 32390, 31000, 29690, 28400, 27250,

	26100, 25000, 23960, 22970, 22030, 21130, 20280, 19460, 18690, 17950,

	17230, 16550, 15900, 15270, 14680, 14110, 13570, 13050, 12560, 12090,

	11630, 11200, 10780, 10380, 10000, 9633,	9281,  8945,  8623,  8314,

	8016,  7730,  7456,  7193,  6941,  6700,  6468,  6246,  6033,  5829,

	5630,  5440,  5257,  5081,  4912,  4750,  4594,  4444,  4300,  4162,

	4027,  3897,  3773,  3653,  3537,  3426,  3319,  3216,  3117,  3022,

	2929,  2839,  2753,  2670,  2589,  2512,  2438,  2366,  2296,  2229,

};  //SEMITEC 103AP-2

#define NTC_TABLE_SIZE  (sizeof(INT_NTC_table) / sizeof(INT_NTC_table[0]))

#define  _R2												(10000)

#define  EXTERNAL_R_SERIES       470.0f

#define  EXTERNAL_R_PULLDOWN     4700.0f

#define  ADC_VREF                3.3f

#define  ADC_RESOLUTION          4096.0f

#define  VREFINT_VOLTAGE         1.2f

adc_thermostat_t adc_ctrl;

volatile float Average_INT_Temp = -999.0f;

volatile float Average_EXT_Temp = -999.0f;

volatile uint8_t display_update_throttle_trigger = 0;

static uint8_t adc_current_channel = 0;

#define DISPLAY_UPDATE_CALL_TARGET_MAX  16      // 57.6s / 3.6s = 16 calls

static uint8_t display_update_call_counter = 0;   // Calls since last display update

static uint8_t display_update_call_target = 1;    // Target calls before next update (1,2,3...16)

static uint8_t display_update_initialized = 0;

float  adc_process_channel(adc_channel_data_t *ch, uint8_t channel_num);

static float adc_process_vcc(void);

//---------------------------------------------------------------------------------------------------------

// ADC1 Interrupt Handler

// Description: Called on every EOC. Stores sample, restarts conversion until 16 samples collected.

//---------------------------------------------------------------------------------------------------------

void ADC1_2_IRQHandler(void)

{

    if (ADC_GetITStatus(ADC1, ADC_IT_EOC))

    {

        uint16_t value = ADC_GetConversionValue(ADC1);

        if (adc_current_channel == 8)

        {

            if (adc_ctrl.ch8.raw_count < ADC_SAMPLE_COUNT)

            {

                adc_ctrl.ch8.raw_buffer[adc_ctrl.ch8.raw_count++] = value;

            }

            if (adc_ctrl.ch8.raw_count < ADC_SAMPLE_COUNT)

            {

                ADC_SoftwareStartConvCmd(ADC1, ENABLE);

            }

            else

            {

                adc_ctrl.ch8.complete = 1;

                adc_ctrl.adc_busy = 0;

                ADC_Cmd(ADC1, DISABLE);

            }

        }

        else if (adc_current_channel == 9)

        {

            if (adc_ctrl.ch9.raw_count < ADC_SAMPLE_COUNT)

            {

                adc_ctrl.ch9.raw_buffer[adc_ctrl.ch9.raw_count++] = value;

            }

            if (adc_ctrl.ch9.raw_count < ADC_SAMPLE_COUNT)

            {

                ADC_SoftwareStartConvCmd(ADC1, ENABLE);

            }

            else

            {

                adc_ctrl.ch9.complete = 1;

                adc_ctrl.adc_busy = 0;

                ADC_Cmd(ADC1, DISABLE);

            }

        }

        else if (adc_current_channel == 17)

        {

            if (adc_ctrl.vcc_raw_count < ADC_SAMPLE_COUNT)

            {

                adc_ctrl.vcc_raw_buffer[adc_ctrl.vcc_raw_count++] = value;

            }

            if (adc_ctrl.vcc_raw_count < ADC_SAMPLE_COUNT)

            {

                ADC_SoftwareStartConvCmd(ADC1, ENABLE);

            }

            else

            {

                adc_ctrl.vcc_complete = 1;

                adc_ctrl.adc_busy = 0;

                ADC_TempSensorVrefintCmd(DISABLE);

                ADC_Cmd(ADC1, DISABLE);

            }

        }

        ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);

    }

}

//---------------------------------------------------------------------------------------------------------

// Function: Thermostat_ADC_Init

// Description: Initialize ADC1 hardware, NVIC, calibrate, measure VCC once.

//---------------------------------------------------------------------------------------------------------

void Thermostat_ADC_Init(void)

{

    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef  ADC_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    RCC_ADCCLKConfig(RCC_PCLK2_Div8);

    ADC_InitStructure.ADC_Mode               = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode       = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign          = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel       = 1;

    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_Cmd(ADC1, ENABLE);

    ADC_ResetCalibration(ADC1);

    while (ADC_GetResetCalibrationStatus(ADC1));

    ADC_StartCalibration(ADC1);

    while (ADC_GetCalibrationStatus(ADC1));

    NVIC_InitStructure.NVIC_IRQChannel                   = ADC1_2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);

    memset(&adc_ctrl, 0, sizeof(adc_ctrl));

    adc_ctrl.next_channel    = 8;
    adc_ctrl.ch8_log_counter = 0;
    adc_ctrl.ch9_log_counter = 0;
    adc_ctrl.adc_busy        = 0;
    adc_ctrl.last_vcc_tick   = 0;
		adc_ctrl.int_sensor_error = 0;
		adc_ctrl.ext_sensor_error = 0;
		
    display_update_call_counter = 0;
    display_update_call_target = 1;
    display_update_initialized = 0;

    Average_INT_Temp = -999.0f;
    Average_EXT_Temp = -999.0f;

    ADC_Start_VCC();

    while (!adc_ctrl.vcc_complete);

    adc_ctrl.vcc_voltage = adc_process_vcc();

    LOGD("ADC Init Complete, VCC: %.2fV\r\n", adc_ctrl.vcc_voltage);
		
		adc_ctrl.idle_comp_step_val = INT_NTC_COMPENSATE / INT_NTC_COMP_STEP;
		adc_ctrl.heatting_comp_step_val = HEAT_COMPENSATE_VAL / HEAT_COMP_STEP;

}

//---------------------------------------------------------------------------------------------------------

// Function: ADC_Start_Channel

// Description: Start 16-sample interrupt conversion for CH8 or CH9.

//---------------------------------------------------------------------------------------------------------

void ADC_Start_Channel(uint8_t channel)

{

    if (channel != 8 && channel != 9) return;

    adc_ctrl.adc_busy = 1;

    adc_current_channel = channel;

    if (channel == 8)

    {

        adc_ctrl.ch8.raw_count = 0;

        adc_ctrl.ch8.complete  = 0;

        ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_239Cycles5);

    }

    else

    {

        adc_ctrl.ch9.raw_count = 0;

        adc_ctrl.ch9.complete  = 0;

        ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 1, ADC_SampleTime_239Cycles5);

    }

    ADC_Cmd(ADC1, ENABLE);

    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

}

//---------------------------------------------------------------------------------------------------------

// Function: ADC_Start_VCC

// Description: Start 16-sample interrupt conversion for Vrefint (CH17).

//---------------------------------------------------------------------------------------------------------

void ADC_Start_VCC(void)

{

    adc_current_channel = 17;

    adc_ctrl.vcc_raw_count = 0;

    adc_ctrl.vcc_complete  = 0;

    adc_ctrl.adc_busy = 1;

    adc_ctrl.last_vcc_tick = time_tick;

    ADC_TempSensorVrefintCmd(ENABLE);

    my_delay_us(100);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_17, 1, ADC_SampleTime_239Cycles5);

    ADC_Cmd(ADC1, ENABLE);

    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

}

//---------------------------------------------------------------------------------------------------------

// Helper: lookup temperature from NTC resistance table.

// Returns -999.0f if out of range.

//---------------------------------------------------------------------------------------------------------

static float ntc_lookup(float resistance, const uint32_t *ntc_table)

{

    uint8_t i;

    if (resistance > (float)ntc_table[0])

    {

        return -999.0f;

    }

    if (resistance < (float)ntc_table[NTC_TABLE_SIZE - 1])

    {

        return -999.0f;

    }

    for (i = 0; i < NTC_TABLE_SIZE - 1; i++)

    {

        if (resistance <= (float)ntc_table[i] && resistance >= (float)ntc_table[i + 1])

        {

            float temp = (float)(-19 + i);

            float frac = ((float)ntc_table[i] - resistance) / (float)(ntc_table[i] - ntc_table[i + 1]);

            return temp + frac;

        }

    }

    return -999.0f;

}

//---------------------------------------------------------------------------------------------------------

// Helper: process 16 raw samples, store average, run 18-sample filter when full.

//---------------------------------------------------------------------------------------------------------

float adc_process_channel(adc_channel_data_t *ch, uint8_t channel_num)

{

    uint32_t sum = 0;
    uint8_t  i;
    uint16_t avg;
	  static float int_temp;
    float    result = -999.0f;
		char str_1[] = "INT_NTC";
		char str_2[] = "EXT_NTC";
		char *prt;

    for (i = 0; i < ADC_SAMPLE_COUNT; i++)
    {
        sum += ch->raw_buffer[i];
    }

    avg = (uint16_t)(sum / ADC_SAMPLE_COUNT);
    ch->avg_buffer[ch->avg_index] = avg;
    ch->avg_index = (ch->avg_index + 1) % ADC_AVG_BUFFER_SIZE;

    if (ch->avg_count < ADC_AVG_BUFFER_SIZE)
    {
        ch->avg_count++;
    }
    ch->raw_count = 0;
    ch->complete  = 0;

    if (ch->avg_count >= ADC_AVG_BUFFER_SIZE)
    {
        uint8_t *log_counter = (channel_num == 8) ? &adc_ctrl.ch8_log_counter : &adc_ctrl.ch9_log_counter;
        (*log_counter)++;

        if (*log_counter >= ADC_AVG_BUFFER_SIZE)
        {
            uint16_t sorted[ADC_AVG_BUFFER_SIZE];
            uint16_t filtered_avg;
            float voltage;
            float resistance;
            float temperature;

            memcpy(sorted, (uint16_t *)ch->avg_buffer, sizeof(sorted));

            for (i = 0; i < ADC_AVG_BUFFER_SIZE - 1; i++)
            {
                uint8_t j;
                for (j = 0; j < ADC_AVG_BUFFER_SIZE - 1 - i; j++)
                {
                    if (sorted[j] > sorted[j + 1])
                    {
                        uint16_t tmp = sorted[j];
                        sorted[j]     = sorted[j + 1];
                        sorted[j + 1] = tmp;
                    }
                }
            }

            sum = 0;
            for (i = 1; i < ADC_AVG_BUFFER_SIZE - 1; i++)
            {
                sum += sorted[i];
            }

            filtered_avg = (uint16_t)(sum / (ADC_AVG_BUFFER_SIZE - 2));
            voltage = (filtered_avg * ADC_VREF) / ADC_RESOLUTION;
            if (channel_num == 8)
            {
                resistance = (adc_ctrl.vcc_voltage - voltage) * _R2 / voltage;
                temperature = ntc_lookup(resistance, INT_NTC_table);
            }
            else
            {
                resistance = (adc_ctrl.vcc_voltage * EXTERNAL_R_PULLDOWN / voltage) - (EXTERNAL_R_SERIES + EXTERNAL_R_PULLDOWN);
                temperature = ntc_lookup(resistance, EXT_NTC_table);
            }

            if (temperature <= -100.0f)
            {
								if(UI_state != STATE_FACTORY_TEST){
										LOGE("ADC CH%d Temperature out of range! Voltage: %.1f\r\n", channel_num, voltage);
										if(channel_num == 8){
												adc_ctrl.int_sensor_error = 1;
										}else if(channel_num == 9){
#if(!NO_Power_Board)
												adc_ctrl.ext_sensor_error = 1;
#endif
										}
								}
            }
            else
            {
                volatile float *ntc_table = (channel_num == 8) ? adc_ctrl.INT_NTC_Table : adc_ctrl.EXT_NTC_Table;
                uint8_t *ntc_count = (channel_num == 8) ? &adc_ctrl.int_ntc_count : &adc_ctrl.ext_ntc_count;
                uint8_t *ntc_valid = (channel_num == 8) ? &adc_ctrl.int_ntc_valid : &adc_ctrl.ext_ntc_valid;
                prt = (channel_num == 8) ? str_1 : str_2;
                ntc_table[*ntc_count] = temperature;
                *ntc_count = (*ntc_count + 1) % NTC_BUFFER_SIZE;
                if (*ntc_valid < NTC_BUFFER_SIZE)
                {
                    (*ntc_valid)++;
                }

								if(UI_state != STATE_FACTORY_TEST){
										LOGD("%s ,%.2f \r\n", prt, temperature);
										if(channel_num == 8){
												int_temp = temperature;
										}else if(channel_num == 9){
											  LOGD("Diff ,%.2f \r\n", int_temp - temperature - adc_ctrl.idle_comp_val - adc_ctrl.heating_comp_val);
												LOGD("Idle Comp , %.2f\r\n", adc_ctrl.idle_comp_val);
												LOGD("Heating Comp, %2f\r\n", adc_ctrl.heating_comp_val);
										}
								}
                ADC_Update_Display_Temperature(channel_num);
            }
            result = temperature;
            *log_counter = 0;
        }
    }
    return result;
}

//---------------------------------------------------------------------------------------------------------

// Helper: process 16 Vrefint raw samples and return VCC voltage.

//---------------------------------------------------------------------------------------------------------

static float adc_process_vcc(void)

{

    uint32_t sum = 0;

    uint8_t  i;

    uint16_t avg;

    for (i = 0; i < ADC_SAMPLE_COUNT; i++)

    {

        sum += adc_ctrl.vcc_raw_buffer[i];

    }

    avg = (uint16_t)(sum / ADC_SAMPLE_COUNT);

    adc_ctrl.vcc_raw_count = 0;

    adc_ctrl.vcc_complete  = 0;

    //adc_ctrl.adc_busy = 1;

    //adc_ctrl.last_vcc_tick = time_tick;

    return (VREFINT_VOLTAGE * ADC_RESOLUTION) / avg;

}

//---------------------------------------------------------------------------------------------------------

// Function: Thermostat_Update

// Description: Called from main loop.

//      - Every 100 ms: start CH8 or CH9

//      - When CH8/CH9 completes (ISR): immediately process and start VCC

//      - When VCC completes (ISR): process and store voltage

//---------------------------------------------------------------------------------------------------------

void Thermostat_Update(void)
{
		static uint8_t write_flag = 0;
    // Process any completed conversions as soon as possible
    if (adc_ctrl.ch8.complete)
    {
        adc_process_channel(&adc_ctrl.ch8, 8);
    }

    if (adc_ctrl.ch9.complete)
    {
        adc_process_channel(&adc_ctrl.ch9, 9);
    }

    if (adc_ctrl.vcc_complete)
    {
        adc_ctrl.vcc_voltage = adc_process_vcc();
				if((adc_ctrl.vcc_voltage < 2.9) && (write_flag == 0)){
						g_parameter.backup_rtc.Year = rtc_time.Year;
						g_parameter.backup_rtc.Mon = rtc_time.Mon;
						g_parameter.backup_rtc.Date = rtc_time.Date;
						g_parameter.backup_rtc.Hour = rtc_time.Hour;
						g_parameter.backup_rtc.Min = rtc_time.Min;
						g_parameter.idle_comp_count = adc_ctrl.idle_comp_count;
						g_parameter.heatting_comp_count = adc_ctrl.heating_comp_count;
						if(Flash_Save_Parameter() != FLASH_OK){
								LOGE("Flash write fail!\r\n");
						}
						write_flag = 1;
				}else if(adc_ctrl.vcc_voltage > 3.1){
						write_flag = 0;
				}
    }

    // Idle time: check if we need to start a conversion
    if (adc_ctrl.adc_busy == 0)
    {
        // Priority 1: Every 100 ms, start CH8 or CH9
        if (g_clock_time_exceed(adc_ctrl.last_tick, 100))
        {
            adc_ctrl.last_tick = time_tick;

            if (adc_ctrl.next_channel == 8)
            {
                ADC_Start_Channel(8);
                adc_ctrl.next_channel = 9;
            }
            else
            {
                ADC_Start_Channel(9);
                adc_ctrl.next_channel = 8;
            }
        }
        // Priority 2: Every 10 ms, monitor VCC
        else if (g_clock_time_exceed(adc_ctrl.last_vcc_tick, 10))
        {
            ADC_Start_VCC();
        }
    }
}

void ADC_Update_Display_Temperature(uint8_t channel){

    float sum;

    uint8_t i;

    float display_temp;

    uint16_t color;

		uint8_t force_update = 0;
	
		if(adc_ctrl.int_sensor_error == 1){
				Clear_Number_Area();
				LCD_Fill(5, 66, 123, 98, RED);
				lan_str = LanguageTable[STR_Internal][g_parameter.language];
				Show_Str(21, 66, WHITE, RED, (char*)lan_str, 16, 0);
				lan_str = LanguageTable[STR_Sensor_Error][g_parameter.language];
				Show_Str(21, 82, WHITE, RED, (char*)lan_str, 16, 0);
				return;
		}
		
		if(adc_ctrl.ext_sensor_error == 1){
				Clear_Number_Area();
				LCD_Fill(5, 66, 123, 98, RED);
				lan_str = LanguageTable[STR_External][g_parameter.language];
				Show_Str(21, 66, WHITE, RED, (char*)lan_str, 16, 0);
				lan_str = LanguageTable[STR_Sensor_Error][g_parameter.language];
				Show_Str(21, 82, WHITE, RED, (char*)lan_str, 16, 0);
				return;
		}
    // Step 1: Update global display temperature for the requested channel

    if(channel == 8)

    {

        if(adc_ctrl.int_ntc_valid > 0)

        {

            sum = 0.0f;

            for(i = 0; i < adc_ctrl.int_ntc_valid; i++)

            {

                sum += adc_ctrl.INT_NTC_Table[i];

            }

            Average_INT_Temp = sum / (float)adc_ctrl.int_ntc_valid;

        }

				window_fun_int_updated = 1;

    }

    else if(channel == 9)

    {

        if(adc_ctrl.ext_ntc_valid > 0)

        {

            sum = 0.0f;

            for(i = 0; i < adc_ctrl.ext_ntc_valid; i++)

            {

                sum += adc_ctrl.EXT_NTC_Table[i];

            }

            Average_EXT_Temp = sum / (float)adc_ctrl.ext_ntc_valid;

        }

				window_fun_ext_updated = 1;

    }

    // Signal window function that new average temperature is ready

    //if(channel == 8)

    //{

    //    window_fun_int_updated = 1;

    //}

    //else if(channel == 9)

    //{

    //    window_fun_ext_updated = 1;

    //}

    // Step 2: Check display update call counter (gradual interval lengthening)

    // Each call to this function represents approximately 3.6 seconds.

    

    if((channel == 8) && (adc_ctrl.int_ntc_valid == 1))

    {

        force_update = 1;

    }

    else if((channel == 9) && (adc_ctrl.ext_ntc_valid == 1))

    {

        force_update = 1;

    }

    if((display_update_initialized == 0) || force_update)

    {

        // First temperature calculated: initialize and update display immediately

        display_update_initialized = 1;

        display_update_call_counter = 0;

    }

    else

    {

        display_update_call_counter++;

        if(display_update_call_counter < display_update_call_target)

        {

            // Not enough calls yet to update display

            return;

        }

    }

    // Step 3: Display update call target reached

    display_update_call_counter = 0;

    display_update_throttle_trigger = 1;  // Signal window flash to update cached temp

    // Step 4: Increase call target for next time (gradual lengthening: 1,2,3...16 calls ~= 3.6s,7.2s,10.8s...57.6s)

    if(display_update_call_target < DISPLAY_UPDATE_CALL_TARGET_MAX)

    {

        display_update_call_target++;

    }

    // Step 5: Only update LCD when UI_state is ACTIVE or SLEEP

    if((UI_state != STATE_ACTIVE) && (UI_state != STATE_SLEEP))

    {

        return;

    }

    // Step 6: Only update when main display is showing room temperature

    if(main_display_digi != Display_Room_Temp)

    {

        return;

    }

    // Step 7: Select temperature based on sensor_type

    if(g_parameter.sensor_type == 0)

    {

        // Room (Internal)
#if(IDLE_COMPENSATE)
        display_temp = Average_INT_Temp - adc_ctrl.idle_comp_val - adc_ctrl.heating_comp_val;
#else
				display_temp = Average_INT_Temp;
#endif
				display_temp += g_parameter.temp_correct_internal;

				//LOGD("Avg_INT_Temp : %.2f\r\n", display_temp);

    }

    else

    {

        // Floor (External)

        display_temp = Average_EXT_Temp;

				display_temp += g_parameter.temp_correct_external;

    }

    // Step 8: Update LCD temperature display

    if(display_temp > -99)

    {

        if(Relay == RELAY_OFF)

        {

            color = BLACK;

        }

        else

        {

            color = RED;

        }

        Display_Number(display_temp, color, 1);

        GUI_DrawMonoIcon24x24(93, 53, BLACK, WHITE, Celsius_Icon_24x24);

    }

    else

    {

        // Temperature not ready yet: draw two horizontal lines

        //LCD_Fill(24, 74, 51, 78, BLACK);
				LCD_Fill(23, 87, 53, 91, BLACK);
        //LCD_Fill(60, 74, 91, 78, BLACK);
				LCD_Fill(57, 87, 87, 91, BLACK);
    }

}

//---------------------------------------------------------------------------------------------------------

// Function: ADC_Get_Channel_Average

// Description: Return the latest averaged ADC value for a channel (0 if none yet).

//---------------------------------------------------------------------------------------------------------

//uint16_t ADC_Get_Channel_Average(uint8_t channel)

//{

//    if (channel == 8 && adc_ctrl.ch8.avg_count > 0)

//    {

//        uint8_t idx = (adc_ctrl.ch8.avg_index + ADC_AVG_BUFFER_SIZE - 1) % ADC_AVG_BUFFER_SIZE;

//        return adc_ctrl.ch8.avg_buffer[idx];

//    }

//    if (channel == 9 && adc_ctrl.ch9.avg_count > 0)

//    {

//        uint8_t idx = (adc_ctrl.ch9.avg_index + ADC_AVG_BUFFER_SIZE - 1) % ADC_AVG_BUFFER_SIZE;

//        return adc_ctrl.ch9.avg_buffer[idx];

//    }

//    return 0;

//}

//---------------------------------------------------------------------------------------------------------

// Function: ADC_Get_VCC_Voltage

// Description: Return the last measured VCC voltage.

//---------------------------------------------------------------------------------------------------------

//float ADC_Get_VCC_Voltage(void)

//{

//    return adc_ctrl.vcc_voltage;

//}

