//---------------------------------------------------------------------------------------------------------
//
// Copyright(c) 2026 E-Poly Technology Co., Ltd. All rights reserved.
//
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
// E-poly North EU Thermostat Project
// Author: Benjamin Wang
// Date: 2026/04/06
// Email: Benjamin@epoly-tech.com
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
// File Function: Thermostat project peripheral header file
//---------------------------------------------------------------------------------------------------------

#ifndef _PERIPHERAL_H
#define _PERIPHERAL_H
//#include "Thermostat.h"
#include <stdio.h>
#include <stdint.h>

#define		MAX_ALARM_NUMBER							(5)
#define		ACTIVE_ALIVE_TIME							(20000)						//20 sec
#define		DIGI_UPDATE_TIME							(10000)						//10 sec
#define		SETTING_UPDATE_TIME						(20000)						//20 sec (QA Spec 2.1)
//#define		FACTORY_KEY_TEST_TIME					(20000)						//20 sec
#define		Upkey_Mask										(1)
#define		Downkey_Mask									(2)
#define		Enterkey_Mask									(4)
#define		KEY_QUT												(3)

/** @addtogroup STM32100E_EVAL_LOW_LEVEL_COM
  * @{
  */
#define COMn                             2

/**
 * @brief Definition for COM port1, connected to USART1
 */ 
#define EVAL_COM1                        USART1
#define EVAL_COM1_CLK                    RCC_APB2Periph_USART1
#define EVAL_COM1_TX_PIN                 GPIO_Pin_9
#define EVAL_COM1_TX_GPIO_PORT           GPIOA
#define EVAL_COM1_TX_GPIO_CLK            RCC_APB2Periph_GPIOA
#define EVAL_COM1_RX_PIN                 GPIO_Pin_10
#define EVAL_COM1_RX_GPIO_PORT           GPIOA
#define EVAL_COM1_RX_GPIO_CLK            RCC_APB2Periph_GPIOA
#define EVAL_COM1_IRQn                   USART1_IRQn

#define UART_FIFO_QTY										(32)
#define MAX_CMD_PARAMS 									(5)

/**
 * @brief Definition for COM port2, connected to USART2
 */ 
#define EVAL_COM2                        USART2
#define EVAL_COM2_CLK                    RCC_APB1Periph_USART2
#define EVAL_COM2_TX_PIN                 GPIO_Pin_2
#define EVAL_COM2_TX_GPIO_PORT           GPIOA
#define EVAL_COM2_TX_GPIO_CLK            RCC_APB2Periph_GPIOA
#define EVAL_COM2_RX_PIN                 GPIO_Pin_3
#define EVAL_COM2_RX_GPIO_PORT           GPIOA
#define EVAL_COM2_RX_GPIO_CLK            RCC_APB2Periph_GPIOA
#define EVAL_COM2_IRQn                   USART2_IRQn

#define FACTORT_TEST_MASK								0x01
#define LCD_TEST_MASK										0x02
#define KEY_TEST_MASK										0x04
#define SENSOR_TEST_MASK								0x08
#define RELAY_TEST_MASK									0x10
#define VCC_TEST_MASK										0x20
#define FW_TEST_MASK										0x40

#define LONG_PRESS_THRESHOLD						(250)
typedef enum 
{
  COM1 = 0,
  COM2 = 1
} COM_TypeDef;  

enum
{
		NONKEY = 0,
		UPKEY,
		DOWNKEY,
		ENTERKEY,
		DOUBLEKEY,
		SYS_RESET
};

typedef enum{
		NONE = 0,
		Active_Alarm,
		Setting_Digi_Alarm,
		Setting_Menu_Alarm,
		Min_Update_Alarm,
		//Factory_Key_Test_Alarm,
} alarm_source_t;

typedef struct{
		uint32_t 					tick;
		uint32_t					dur;
		alarm_source_t		source;
		uint8_t						f_time_up:1;
		uint8_t						f_alarm:1;
		uint8_t						reserve:6;
}Alarm_t;

typedef struct{
		uint8_t		key_val;
		uint8_t		key_active;
		uint8_t		key_press;
		uint8_t		key_press_cnt[KEY_QUT];
}struct_key_t;

typedef struct
{
		uint8_t 		F_received_complete;
		volatile uint8_t		rx_index;
		volatile uint8_t 		rx_buffer[UART_FIFO_QTY];
} cmd_queue_t;

typedef enum {
    UNKNOWN,
    CONNECT,
		EXIT,
		LCD,
		KEY_TEST,
		EVENT,
		GET,
		RELAY,
} test_cmd_t;

typedef struct
{
    test_cmd_t cmd;
    uint8_t extended;
    uint8_t query;
    int values[MAX_CMD_PARAMS];
} cmd_parse_t;

extern volatile uint32_t time_tick;
extern struct_key_t key;
extern uint32_t nop_per_us;
extern uint8_t weekday;
extern uint8_t rtc_empty;
extern uint8_t rtc_reset;
extern cmd_queue_t cmd_queue;
extern uint8_t pwm_is_daytime;


void Key_Scan(void);

// Function prototypes
void Log_USART_Init(void);
int fputc(int ch, FILE *f);
void Backlight_Init(void);
void Backlight_SetDuty(uint8_t duty_percent);
uint8_t g_clock_time_exceed(uint32_t tick_run, uint32_t duration);
void my_delay_ms(uint32_t ms);
void my_delay_us(uint32_t us);

void delete_alarm(alarm_source_t source);
void update_alarm(alarm_source_t in_source);
void g_check_alarm(void);
uint8_t getWeekday(uint16_t year, uint8_t month, uint8_t day);
void get_schedule_period(void);
void g_cmd_handler(void);
void g_relay_handler(void);
uint32_t g_check_power_off_time(void);


#endif
