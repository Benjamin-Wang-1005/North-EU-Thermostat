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
// File Function: Adaptive Start header
//      Heuristic Rate-of-Rise algorithm with zone-based learning
//      No outdoor sensor required - uses indoor temp zones (cold/mild)
//---------------------------------------------------------------------------------------------------------

#ifndef __ADAPTIVE_START_H
#define __ADAPTIVE_START_H

#include <stdint.h>
#include <math.h>

// Maximum preheat time limit (minutes) per EN 50559 guidance
#define AS_MAX_PREHEAT_MIN          180

// Temperature threshold for zone selection (Celsius)
#define AS_TEMP_ZONE_THRESHOLD      16.0f

// Asymmetric learning rates
// When heating slows (winter): fast learning to ensure comfort
#define AS_LEARN_RATE_DOWN          0.2f
// When heating speeds up (spring): slow learning to prevent overcool
#define AS_LEARN_RATE_UP            0.05f

// Minimum heating duration to qualify for learning (minutes)
#define AS_MIN_LEARN_MINUTES        5
// Maximum plausible heating duration (minutes)
#define AS_MAX_LEARN_MINUTES        240

// Default initial R values (Celsius/minute)
// Conservatively low so preheat starts slightly early
#define AS_DEFAULT_R_MILD           0.08f
#define AS_DEFAULT_R_COLD           0.05f

// Number of schedule periods
#define AS_SCHEDULE_PERIODS         6

// Adaptive Start runtime state
typedef struct {
    uint8_t  active;             // 1 = currently in preheat mode
    float    start_temp;         // Room temp when preheat started
    uint32_t start_tick;         // time_tick when preheat started
    float    target_temp;        // Target temp for current preheat
    uint8_t  target_period;      // Which schedule period we are preheating for
    uint8_t  was_cold_zone;      // 1 = used r_cold, 0 = used r_mild
} AdaptiveStart_Runtime_t;

// Expose the runtime instance
extern AdaptiveStart_Runtime_t as_runtime;

// Public API
void     adaptive_start_init(void);
void     adaptive_start_minute_task(void);
void     adaptive_start_log_status(void);
float    adaptive_start_get_target(void);
uint8_t  adaptive_start_is_active(void);

#endif /* __ADAPTIVE_START_H */
