//---------------------------------------------------------------------------------------------------------
//
// Copyright(c) 2026 E-Poly Technology Co., Ltd. All rights reserved.
//
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
// E-Poly North EU Thermostat Project
// Author: Benjamin Wang
// Date: 2026/07/16
// Email: Benjamin@epoly-tech.com
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
// File Function: Adaptive Start algorithm implementation
//      - Heuristic Rate-of-Rise (方案一) without outdoor sensor
//      - Zone-based R values: cold zone (<16C) and mild zone (>=16C)
//      - Asymmetric IIR learning: fast when slower, slow when faster
//      - Deadband uses g_parameter.temp_swing
//      - Safety cap at AS_MAX_PREHEAT_MIN (180 min)
//---------------------------------------------------------------------------------------------------------

#include "Thermostat.h"
#include "Adaptive_Start.h"
#include "ADC.h"

// Runtime instance
AdaptiveStart_Runtime_t as_runtime;

// Forward declarations
static float    get_current_temperature(void);
static uint8_t  get_next_schedule_change(uint32_t current_min, uint32_t *minutes_to_change,
                                          float *next_target, uint8_t *next_idx);
static void     adaptive_start_learn(void);
static uint8_t  is_in_program_mode(void);

//---------------------------------------------------------------------------------------------------------
// Funcation: adaptive_start_init
// Description: Initialize the adaptive start runtime state
//---------------------------------------------------------------------------------------------------------
void adaptive_start_init(void)
{
    as_runtime.active = 0;
    as_runtime.start_temp = 0.0f;
    as_runtime.start_tick = 0;
    as_runtime.target_temp = 0.0f;
    as_runtime.target_period = 0;
    as_runtime.was_cold_zone = 0;
}

//---------------------------------------------------------------------------------------------------------
// Funcation: get_current_temperature
// Description: Get current corrected temperature (same logic as g_relay_handler)
//---------------------------------------------------------------------------------------------------------
static float get_current_temperature(void)
{
    float temp = 0.0f;

    if (g_parameter.sensor_type == 0)
    {
        if (Average_INT_Temp > -900.0f)
        {
#if (IDLE_COMPENSATE && HEAT_COMPENSATE)
            temp = Average_INT_Temp - adc_ctrl.idle_comp_val - adc_ctrl.heating_comp_val;
#elif(IDLE_COMPENSATE)
            temp = Average_INT_Temp - adc_ctrl.idle_comp_val;
#else
						temp = Average_INT_Temp;
#endif
            temp += g_parameter.temp_correct_internal;
        }
    }
    else
    {
        if (Average_EXT_Temp > -900.0f)
        {
            temp = Average_EXT_Temp;
            temp += g_parameter.temp_correct_external;
        }
    }

    return temp;
}

//---------------------------------------------------------------------------------------------------------
// Funcation: is_in_program_mode
// Description: Check if we are in schedule (program) mode
//---------------------------------------------------------------------------------------------------------
static uint8_t is_in_program_mode(void)
{
    return (g_parameter.operation_mode == 1);
}

//---------------------------------------------------------------------------------------------------------
// Funcation: get_next_schedule_change
// Description: Find the next schedule period boundary after current_min
//      Handles wrap-around to next day (first period tomorrow)
// Return: 1 if found, 0 if no valid schedule change (all OFF or no schedule)
//---------------------------------------------------------------------------------------------------------
static uint8_t get_next_schedule_change(uint32_t current_min, uint32_t *minutes_to_change,
                                         float *next_target, uint8_t *next_idx)
{
    uint8_t i;
    uint8_t (*sched)[4];
    uint32_t sch_min;
    uint32_t day_minutes = 24 * 60;
    uint32_t first_on_min = day_minutes;
    float first_on_target = 0.0f;
    uint8_t first_on_idx = 0;
    uint8_t has_first_on = 0;

    // Select the correct schedule table (same logic as get_schedule_period)
    if (g_parameter.current_prog_type == 0)
    {
        sched = (weekday < 5) ? g_parameter.workday_schedule : g_parameter.holiday_schedule;
    }
    else if (g_parameter.current_prog_type == 1)
    {
        sched = (weekday < 6) ? g_parameter.workday_schedule : g_parameter.holiday_schedule;
    }
    else
    {
        sched = g_parameter.workday_schedule;
    }

    // Scan forward from current_min
    for (i = 0; i < AS_SCHEDULE_PERIODS; i++)
    {
        sch_min = (uint32_t)(sched[i][0]) * 60 + (uint32_t)(sched[i][1]);

        if (sch_min > current_min)
        {
            // Found a future period today
            *minutes_to_change = sch_min - current_min;
            *next_target = (float)(sched[i][2]);
            *next_idx = i;
            return 1;
        }

        // Track the earliest ON period of the day (for wrap-around)
        if (sched[i][3] == 1 && sch_min < first_on_min)
        {
            first_on_min = sch_min;
            first_on_target = (float)(sched[i][2]);
            first_on_idx = i;
            has_first_on = 1;
        }
    }

    // No more periods today - wrap to tomorrow's first ON period
    if (has_first_on)
    {
        // Minutes until midnight + first period time
        *minutes_to_change = (day_minutes - current_min) + first_on_min;
        *next_target = first_on_target;
        *next_idx = first_on_idx;
        return 1;
    }

    return 0; // No valid schedule found
}

//---------------------------------------------------------------------------------------------------------
// Funcation: adaptive_start_minute_task
// Description: Called every 60 seconds from Min_Update_Alarm
//      Core decision logic and learning
//---------------------------------------------------------------------------------------------------------
void adaptive_start_minute_task(void)
{
    float temp;
    float current_r;
    uint32_t minutes_to_change;
    float next_target;
    uint8_t next_idx;
    uint32_t required_min;

    // Only active in schedule mode and when adaptive start is enabled
    if (!is_in_program_mode() || !g_parameter.adaptive_start)
    {
        return;
    }

    temp = get_current_temperature();
    if (temp < -50.0f || temp > 100.0f)
    {
        return; // Invalid temperature reading
    }

    //--- Learning check: if preheating and target is reached ---
    if (as_runtime.active)
    {
        float target = as_runtime.target_temp;
        float deadband = g_parameter.temp_swing;

        if (temp >= (target - deadband * 0.5f))
        {
            adaptive_start_learn();
            as_runtime.active = 0;
            // Continue to check if another preheat is needed
        }
        else
        {
            // Still preheating, nothing more to do this tick
            return;
        }
    }

    //--- Decision: should we start preheating for the next schedule? ---
    if (!get_next_schedule_change(
            (uint32_t)(rtc_time.Hour) * 60 + (uint32_t)(rtc_time.Min),
            &minutes_to_change, &next_target, &next_idx))
    {
        return; // No schedule change found
    }

    // Deadband: skip if temperature difference is too small
    if (fabs(temp - next_target) < g_parameter.temp_swing)
    {
        return;
    }

    // Only preheat for temperature rise; skip cooling (setback) transitions.
    // (Negative delta would rely on undefined float->uint32 conversion.)
    if (next_target <= temp)
    {
        return;
    }

    // Select R based on current temperature zone
    if (temp < AS_TEMP_ZONE_THRESHOLD)
    {
        current_r = g_parameter.r_cold;
        as_runtime.was_cold_zone = 1;
    }
    else
    {
        current_r = g_parameter.r_mild;
        as_runtime.was_cold_zone = 0;
    }

    // Guard against zero or negative R
    if (current_r <= 0.001f)
    {
        current_r = AS_DEFAULT_R_MILD;
    }

    // Calculate required preheat time (minutes)
    required_min = (uint32_t)((next_target - temp) / current_r);

    // Safety cap
    if (required_min > AS_MAX_PREHEAT_MIN)
    {
        required_min = AS_MAX_PREHEAT_MIN;
    }

    // Start preheat if within range
    if (minutes_to_change <= required_min)
    {
        as_runtime.active = 1;
        as_runtime.start_temp = temp;
        as_runtime.start_tick = time_tick;
        as_runtime.target_temp = next_target;
        as_runtime.target_period = next_idx;

        LOGD("AS: start preheat for P%d target=%.1fC in %lu min (R=%.3f)\r\n",
             next_idx, next_target, required_min, current_r);
    }
}

//---------------------------------------------------------------------------------------------------------
// Funcation: adaptive_start_learn
// Description: Update learned R values after a successful preheat cycle
//      - Uses asymmetric IIR filter
//      - Only updates if heating duration is within valid range
//      - Updates the zone (cold/mild) that was used for this preheat
//---------------------------------------------------------------------------------------------------------
static void adaptive_start_learn(void)
{
    float elapsed_min;
    float temp;
    float actual_r;
    float old_r;
    float new_r;
    float eta;

    elapsed_min = (float)(time_tick - as_runtime.start_tick) / 60000.0f;

    // Sanity checks
    if (elapsed_min < (float)AS_MIN_LEARN_MINUTES ||
        elapsed_min > (float)AS_MAX_LEARN_MINUTES)
    {
        LOGD("AS: skip learn elapsed=%.0fmin (out of range)\r\n", elapsed_min);
        return;
    }

    temp = get_current_temperature();
    if (temp < -50.0f)
    {
        return;
    }

    // Calculate actual R achieved
    actual_r = (as_runtime.target_temp - as_runtime.start_temp) / elapsed_min;

    // Sanity: R must be positive and reasonable
    if (actual_r <= 0.0001f || actual_r > 1.0f)
    {
        LOGD("AS: skip learn actual_R=%.4f (unreasonable)\r\n", actual_r);
        return;
    }

    // Choose which zone to update and select asymmetric learning rate
    if (as_runtime.was_cold_zone)
    {
        old_r = g_parameter.r_cold;
        eta = (actual_r < old_r) ? AS_LEARN_RATE_DOWN : AS_LEARN_RATE_UP;
        new_r = (1.0f - eta) * old_r + eta * actual_r;
        g_parameter.r_cold = new_r;
    }
    else
    {
        old_r = g_parameter.r_mild;
        eta = (actual_r < old_r) ? AS_LEARN_RATE_DOWN : AS_LEARN_RATE_UP;
        new_r = (1.0f - eta) * old_r + eta * actual_r;
        g_parameter.r_mild = new_r;
    }

    LOGD("Adaptive Start: learn zone=%s actual_R=%.4f old_R=%.4f new_R=%.4f eta=%.2f\r\n",
         as_runtime.was_cold_zone ? "cold" : "mild",
         actual_r, old_r, new_r, eta);

    // Persist updated R values to Flash
    Flash_Save_Parameter();
}

//---------------------------------------------------------------------------------------------------------
// Funcation: adaptive_start_get_target
// Description: Return the target temperature to use for relay control
//      Returns adaptive start target if active, otherwise 0 (caller uses default)
//---------------------------------------------------------------------------------------------------------
float adaptive_start_get_target(void)
{
    if (as_runtime.active)
    {
        return as_runtime.target_temp;
    }
    return 0.0f;
}

//---------------------------------------------------------------------------------------------------------
// Funcation: adaptive_start_is_active
// Description: Return whether adaptive preheat is currently active
//---------------------------------------------------------------------------------------------------------
uint8_t adaptive_start_is_active(void)
{
    return as_runtime.active;
}

//---------------------------------------------------------------------------------------------------------
// Funcation: adaptive_start_log_status
// Description: Output one CSV status line every minute (for experiment logging)
//      Format: ASL,HH:MM,temp,target,active,relay,r_mild,r_cold,min_to_change
//---------------------------------------------------------------------------------------------------------
void adaptive_start_log_status(void)
{
    uint32_t minutes_to_change = 0;
    float next_target = 0.0f;
    uint8_t next_idx = 0;
    float temp = get_current_temperature();
    float target;

    if (as_runtime.active)
    {
        target = as_runtime.target_temp;
    }
    else if (is_in_program_mode())
    {
        target = (float)(sch_table[Schedule_Period][2]);
    }
    else
    {
        target = 0.0f;
    }

    get_next_schedule_change((uint32_t)(rtc_time.Hour) * 60 + (uint32_t)(rtc_time.Min),
                             &minutes_to_change, &next_target, &next_idx);

    LOGD("ASL,%02d:%02d,%.2f,%.1f,%d,%d,%.4f,%.4f,%lu\r\n",
         rtc_time.Hour, rtc_time.Min, temp, target,
         as_runtime.active, g_parameter.relay_staty,
         g_parameter.r_mild, g_parameter.r_cold,
         (unsigned long)minutes_to_change);
}
