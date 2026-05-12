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
// File Function: ADC initialization and VCC measurement
//      - ADC1 Channel 8 (PB0) and Channel 9 (PB1)
//      - Continuous conversion with DMA
//      - 16 samples per channel
//---------------------------------------------------------------------------------------------------------

#include "Thermostat.h"

// ADC buffer for each channel (16 samples each)
volatile uint16_t adc_ch8_buffer[16];  // 16 samples for CH8 (PB0)
volatile uint16_t adc_ch9_buffer[16];  // 16 samples for CH9 (PB1)
volatile uint8_t adc_ch8_complete = 0;
volatile uint8_t adc_ch9_complete = 0;
volatile uint8_t adc_current_channel = 0;  // 0=none, 8=CH8, 9=CH9

// Temporary buffer for DMA transfer
volatile uint16_t adc_buffer_temp[16];

// ADC calibration values
#define ADC_VREF                3.3f    // Reference voltage
#define ADC_RESOLUTION          4096.0f // 12-bit ADC

// VCC measurement using internal reference
// Vrefint = 1.2V (typical), used to calculate actual VCC
#define VREFINT_VOLTAGE         1.2f    // Internal reference voltage (typical)
#define VREFINT_CHANNEL         17      // ADC Channel 17 is internal reference

//---------------------------------------------------------------------------------------------------------
// Function: ADC_DMA_IRQHandler
// Description: DMA interrupt handler for ADC conversion complete
//---------------------------------------------------------------------------------------------------------
void DMA1_Channel1_IRQHandler(void)
{
    uint8_t i;
    
    if(DMA_GetITStatus(DMA1_IT_TC1))
    {
        DMA_ClearITPendingBit(DMA1_IT_TC1);
        
        // Copy data to appropriate buffer based on current channel
        if(adc_current_channel == 8)
        {
            for(i = 0; i < 16; i++)
            {
                adc_ch8_buffer[i] = adc_buffer_temp[i];
            }
            adc_ch8_complete = 1;
        }
        else if(adc_current_channel == 9)
        {
            for(i = 0; i < 16; i++)
            {
                adc_ch9_buffer[i] = adc_buffer_temp[i];
            }
            adc_ch9_complete = 1;
        }
        
        // Disable DMA and ADC after conversion complete
        DMA_Cmd(DMA1_Channel1, DISABLE);
        ADC_DMACmd(ADC1, DISABLE);
        ADC_Cmd(ADC1, DISABLE);
        
        //LOGD("ADC CH%d DMA Complete\n\r", adc_current_channel);
        
        adc_current_channel = 0;  // Clear current channel
    }
}

//---------------------------------------------------------------------------------------------------------
// Function: ADC_Init_Config
// Description: Initialize ADC1 with DMA for continuous conversion
//              - Channel 8 (PB0) and Channel 9 (PB1)
//              - 16 samples per channel
//---------------------------------------------------------------------------------------------------------
void ADC_Init_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    // Enable clocks
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOB, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    
    // Configure PB0 (ADC12_IN8) and PB1 (ADC12_IN9) as analog input
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;  // Analog input
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // DMA configuration
    DMA_DeInit(DMA1_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)adc_buffer_temp;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = 16;  // 16 samples for single channel
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  // Single shot, not circular
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);
    
    // Enable DMA interrupt
    DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
    
    // NVIC configuration for DMA
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // ADC configuration
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;  // Scan mode for multiple channels
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;  // Continuous conversion
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 2;  // 2 channels
    ADC_Init(ADC1, &ADC_InitStructure);
    
    // Configure ADC for single channel mode (will be set at start conversion)
    ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_239Cycles5);
    
    // Enable ADC
    ADC_Cmd(ADC1, ENABLE);
    
    // ADC calibration
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
    
    LOGD("ADC Init Complete\n\r");
}

//---------------------------------------------------------------------------------------------------------
// Function: ADC_Start_Channel
// Description: Start ADC conversion for specific channel
// Parameters:
//      - channel: 8 for CH8 (PB0), 9 for CH9 (PB1), 17 for Vrefint
//---------------------------------------------------------------------------------------------------------
void ADC_Start_Channel(uint8_t channel)
{
    DMA_InitTypeDef DMA_InitStructure;
    
    if(channel != 8 && channel != 9 && channel != 17) return;

    // Clear completion flag
    if(channel == 8)
        adc_ch8_complete = 0;
    else if(channel == 9)
        adc_ch9_complete = 0;

    adc_current_channel = channel;
    
    // Re-initialize DMA (required after DMA was disabled in IRQ)
    DMA_DeInit(DMA1_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)adc_buffer_temp;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = 16;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);
    
    // Re-enable DMA interrupt
    DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);

    // Configure ADC channel
    if(channel == 8)
        ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_239Cycles5);
    else if(channel == 9)
        ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 1, ADC_SampleTime_239Cycles5);
    else if(channel == 17)
        ADC_RegularChannelConfig(ADC1, ADC_Channel_17, 1, ADC_SampleTime_239Cycles5);

    // Enable DMA
    DMA_Cmd(DMA1_Channel1, ENABLE);

    // Enable ADC DMA request
    ADC_DMACmd(ADC1, ENABLE);

    // Enable ADC
    ADC_Cmd(ADC1, ENABLE);

    // Start ADC conversion
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    //LOGD("ADC CH%d Started\n\r", channel);
}

//---------------------------------------------------------------------------------------------------------
// Function: ADC_Start_Conversion (deprecated, use ADC_Start_Channel instead)
// Description: Start ADC conversion with DMA for both channels
//---------------------------------------------------------------------------------------------------------
void ADC_Start_Conversion(void)
{
    // Start both channels sequentially
    ADC_Start_Channel(8);
}

//---------------------------------------------------------------------------------------------------------
// Function: ADC_Wait_Channel_Complete
// Description: Wait for specific channel conversion to complete
// Parameters:
//      - channel: 8 for CH8, 9 for CH9
//      - timeout_ms: timeout in milliseconds
// Return: 1 if success, 0 if timeout
//---------------------------------------------------------------------------------------------------------
uint8_t ADC_Wait_Channel_Complete(uint8_t channel, uint32_t timeout_ms)
{
    uint32_t start_tick = time_tick;
    volatile uint8_t *complete_flag;
    
    if(channel == 8)
        complete_flag = &adc_ch8_complete;
    else if(channel == 9)
        complete_flag = &adc_ch9_complete;
    else if(channel == 17)
        complete_flag = &adc_ch8_complete;  // Use CH8 flag for Vrefint (single sample)
    else
        return 0;
    
    // For Vrefint (channel 17), we use a simple timeout without flag
    if(channel == 17)
    {
        // Wait for DMA transfer complete (single sample)
        while(DMA_GetCurrDataCounter(DMA1_Channel1) > 0)
        {
            if((time_tick - start_tick) > timeout_ms)
            {
                LOGD("ADC Vrefint Timeout\n\r");
                return 0;
            }
        }
        return 1;
    }
    
    while(!(*complete_flag))
    {
        if((time_tick - start_tick) > timeout_ms)
        {
            LOGD("ADC CH%d Timeout\n\r", channel);
            return 0;
        }
    }
    
    return 1;
}

//---------------------------------------------------------------------------------------------------------
// Function: ADC_Wait_Complete (deprecated, use ADC_Wait_Channel_Complete instead)
// Description: Wait for ADC conversion to complete
// Return: 1 if success, 0 if timeout
//---------------------------------------------------------------------------------------------------------
uint8_t ADC_Wait_Complete(uint32_t timeout_ms)
{
    return ADC_Wait_Channel_Complete(8, timeout_ms);
}

//---------------------------------------------------------------------------------------------------------
// Function: ADC_Calculate_Average
// Description: Calculate average from ADC samples
// Parameters: 
//      - channel: 8 for CH8 (PB0), 9 for CH9 (PB1)
// Return: Average ADC value
//---------------------------------------------------------------------------------------------------------
uint16_t ADC_Calculate_Average(uint8_t channel)
{
    uint32_t sum = 0;
    uint8_t i;
    volatile uint16_t *buffer;
    
    if(channel == 8)
        buffer = adc_ch8_buffer;
    else if(channel == 9)
        buffer = adc_ch9_buffer;
    else
        return 0;
    
    for(i = 0; i < 16; i++)
    {
        sum += buffer[i];
    }
    
    return (uint16_t)(sum / 16);
}

//---------------------------------------------------------------------------------------------------------
// Function: ADC_Measure_Channel
// Description: Measure specific ADC channel voltage
// Parameters:
//      - channel: 8 for CH8 (PB0), 9 for CH9 (PB1)
// Return: Voltage in volts
//---------------------------------------------------------------------------------------------------------
float ADC_Measure_Channel(uint8_t channel)
{
    uint16_t avg_value;
    float voltage;
    
    if(channel != 8 && channel != 9) return 0.0f;
    
    // Start conversion for specific channel
    ADC_Start_Channel(channel);
    
    // Wait for conversion complete (timeout 100ms)
    if(!ADC_Wait_Channel_Complete(channel, 100))
    {
        LOGD("ADC CH%d Conversion Failed\n\r", channel);
        return 0.0f;
    }
    
    // Calculate average
    avg_value = ADC_Calculate_Average(channel);
    
    // Convert to voltage
    voltage = (avg_value * ADC_VREF) / ADC_RESOLUTION;
    
    LOGD("ADC CH%d: %d, Voltage: %.3fV\n\r", channel, avg_value, voltage);
    
    return voltage;
}

//---------------------------------------------------------------------------------------------------------
// Function: ADC_Measure_VCC
// Description: Measure VCC voltage using ADC
// Note: Measures both channels and returns CH8 voltage as VCC reference
//---------------------------------------------------------------------------------------------------------
float ADC_Measure_VCC(void)
{
    float voltage_ch8, voltage_ch9;
    
    // Measure CH8
    voltage_ch8 = ADC_Measure_Channel(8);
    
    // Measure CH9
    voltage_ch9 = ADC_Measure_Channel(9);
    
    LOGD("VCC Measurement - CH8: %.3fV, CH9: %.3fV\n\r", voltage_ch8, voltage_ch9);
    
    // Return CH8 voltage as VCC reference
    return voltage_ch8;
}

//---------------------------------------------------------------------------------------------------------
// Function: ADC_Measure_VCC_Internal
// Description: Measure system VCC voltage using internal reference (Channel 17)
// Principle: Vrefint (1.2V) is connected to ADC, by measuring its ADC value,
//            we can calculate actual VCC voltage
// Formula: VCC = VREFINT_VOLTAGE * 4096 / ADC_Vrefint_Value
// Return: VCC voltage in volts
//---------------------------------------------------------------------------------------------------------
float ADC_Measure_VCC_Internal(void)
{
    uint32_t vrefint_sum = 0;
    uint16_t vrefint_adc_value;
    float vcc_voltage;
    uint8_t i;
    uint32_t timeout;
    ADC_InitTypeDef ADC_InitStructure;
    
    // Enable internal reference voltage channel
    ADC_TempSensorVrefintCmd(ENABLE);
    
    // Wait for internal reference to stabilize (at least 10us)
    my_delay_ms(1);
    
    // Configure ADC for single conversion mode (no DMA)
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);
    
    // Configure Channel 17 (Vrefint)
    ADC_RegularChannelConfig(ADC1, ADC_Channel_17, 1, ADC_SampleTime_239Cycles5);
    
    // Take 16 samples and average
    for(i = 0; i < 16; i++)
    {
        // Start conversion
        ADC_SoftwareStartConvCmd(ADC1, ENABLE);
        
        // Wait for conversion complete
        timeout = 10000;
        while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC))
        {
            if(--timeout == 0)
            {
                LOGD("VCC Measurement Timeout\n\r");
                ADC_TempSensorVrefintCmd(DISABLE);
                return 0.0f;
            }
        }
        
        // Read ADC value
        vrefint_sum += ADC_GetConversionValue(ADC1);
    }
    
    // Calculate average
    vrefint_adc_value = (uint16_t)(vrefint_sum / 16);
    
    // Calculate VCC: VCC = Vrefint * 4096 / ADC_Value
    // When VCC drops, ADC_Value drops proportionally
    vcc_voltage = (VREFINT_VOLTAGE * ADC_RESOLUTION) / vrefint_adc_value;
    
    //LOGD("Vrefint ADC Value: %d (avg of 16 samples)\n\r", vrefint_adc_value);
    //LOGD("System VCC Voltage: %.3fV\n\r", vcc_voltage);
    
    // Disable internal reference to save power
    ADC_TempSensorVrefintCmd(DISABLE);
    
    // Restore ADC configuration for DMA mode
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);
    
    return vcc_voltage;
}

//---------------------------------------------------------------------------------------------------------
// Function: ADC_Init_And_Measure
// Description: Initialize ADC and measure VCC voltage
// This function should be called before entering main loop
//---------------------------------------------------------------------------------------------------------
void ADC_Init_And_Measure(void)
{
    float vcc_voltage;
		float voltage_ch8, voltage_ch9;
    
    //LOGD("Starting ADC Initialization...\n\r");
    
    // Initialize ADC
    ADC_Init_Config();
    
   
    
    // Measure System VCC using internal reference
    vcc_voltage = ADC_Measure_VCC_Internal();
    LOGD("System VCC: %.2fV\n\r", vcc_voltage);
    
    //LOGD("ADC Initialization Complete\n\r");
	
		 // Measure CH8 (PB0) - wait a bit for system to stabilize
    my_delay_ms(10);
    voltage_ch8 = ADC_Measure_Channel(8);
    
    
    // Measure CH9 (PB1) - add delay between measurements
    my_delay_ms(10);
    voltage_ch9 = ADC_Measure_Channel(9);
   
}
