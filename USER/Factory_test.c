//---------------------------------------------------------------------------------------------------------
//                                                                                                         
// Copyright(c) 2026 E-poly Technology Co., Ltd. All rights reserved.                                           
//                                                                                                         
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
// E-Poly North EU Thermostat Project 
// Author: Benjamin Wang
// Date: 2026/05/14
// Email: Benjamin@epoly-tech.com
//---------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------
//File Function: Peripheral.c to define peripheral HW operation
//		- Key operation
//		- Log USART
//---------------------------------------------------------------------------------------------------------

#include "Thermostat.h"


//------------------------------------------------------------------------------------------------------------------
//
// UART1 input command handle reative functions
// 
//
//------------------------------------------------------------------------------------------------------------------
#define CMD_PREFIX          				"[CMD]"
#define CMD_PREFIX_LEN      				(5)




volatile uint8_t		Factory_testing = 0;
//---------------------------------------------------------------------------------------------------------
// Funcation: _receiveCmd
// Description: Check RX buffer had received complete command 
// Input: None
// Output: eTRUE -> complere command had received / eFALSE -> received not complete
// Date: 2022/12/03
// Update:
//----------------------------------------------------------------------------------------------------------
static bool_t receiveCmd(void)
{
		uint8_t inChar = 0xFF;
		while((USART1->SR & USART_FLAG_RXNE) != 0){		//'0' no data can read
				inChar = USART_ReceiveData(EVAL_COM1);
				//LOGD("RX: 0x%02X '%c'\n", inChar, (inChar >= 32 && inChar <= 126) ? inChar : '.');
				if(inChar == 0x00 || inChar == '\r' || inChar == '\n') {
						cmd_queue.rx_buffer[cmd_queue.rx_index] = 0x00;
					cmd_queue.rx_index = 0;
                    return eTRUE;
        }
				// lowercase to uppercase
        if (inChar >= 'a' && inChar <= 'z') {
						inChar -= 0x20;
        }
				
				if(cmd_queue.rx_index < UART_FIFO_QTY - 1)  // Reserved onr byte for '\0'
        {
            cmd_queue.rx_buffer[cmd_queue.rx_index] = inChar;
            cmd_queue.rx_index++;
        }
        else
        {
            // FIFO full
            LOGD("UART buffer overflow!\n\r");
            // clear FIFO
            cmd_queue.rx_index = 0;
        }
		}
		
		return eFALSE;
}

//---------------------------------------------------------------------------------------------------------
// Function: USART1_IRQHandler
// Description: UART1 interrupt handle
// Input: None
// Output: None
// Date: 2026/05/03
//---------------------------------------------------------------------------------------------------------
void USART1_IRQHandler(void)
{
		bool_t completed = eFALSE;
		// check rx index
		if(USART_GetITStatus(EVAL_COM1, USART_IT_RXNE) != RESET)
		{
				completed = receiveCmd();
				if(completed){
						cmd_queue.F_received_complete = eTRUE;
				}					
				// clear interrupt flag
				USART_ClearITPendingBit(EVAL_COM1, USART_IT_RXNE);
		}
}

//---------------------------------------------------------------------------------------------------------
// Function: parse_command
// Description: Parse received command string and extract command name and parameter
// Input: cmd_str - received command string (without newline)
// Output: cmd_name - extracted command name (max 32 chars)
//         param - extracted parameter (may be NULL)
// Return: 0 on success, -1 on failure
//---------------------------------------------------------------------------------------------------------
bool_t parse_command(const char* cmd_str, char* cmd_name, char* param)
{
    const char* p = cmd_str;
    const char* colon_pos = NULL;
    int cmd_len;

    // Check prefix [CMD]
    if (strncmp(p, CMD_PREFIX, CMD_PREFIX_LEN) != 0) {
        return eFALSE;
    }
    p += CMD_PREFIX_LEN;

    // Find colon ':'
    colon_pos = strchr(p, ':');
    if (colon_pos == NULL) {
        // No param, whole string is command name
        cmd_len = strlen(p);
        if (cmd_len >= 32) cmd_len = 31;
        strncpy(cmd_name, p, cmd_len);
        cmd_name[cmd_len] = '\0';
        param[0] = '\0';
    } else {
        // Has param
        cmd_len = colon_pos - p;
        if (cmd_len >= 32) cmd_len = 31;
        strncpy(cmd_name, p, cmd_len);
        cmd_name[cmd_len] = '\0';

        // Copy param
        strncpy(param, colon_pos + 1, 31);
        param[31] = '\0';
    }
    
    return eTRUE;
}

//---------------------------------------------------------------------------------------------------------
// Function: lcd_test_cmd(char *param)
// Description: Factory test LCD function
// Input: LCD test command
// Output: None
// Date: 2026/05/14
// Update: 
//----------------------------------------------------------------------------------------------------------
void lcd_test_cmd(char *param)
{
	
		if(strcmp(param, "START") == 0){
				TCMD("RES:LCD:OK\n");
				Factory_testing |= LCD_TEST_MASK;
				Backlight_SetDuty(100);
		}else if(strcmp(param, "STOP") == 0){
				TCMD("RES:LCD:OK\n");
				LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
				Factory_testing ^= LCD_TEST_MASK;
		}else if(strcmp(param, "RED") == 0){
				TCMD("RES:LCD:RED\n");
				LCD_Fill(0, 0, lcddev.width, lcddev.height, RED);
		}else if(strcmp(param, "BLUE") == 0){
				TCMD("RES:LCD:BLUE\n");
				LCD_Fill(0, 0, lcddev.width, lcddev.height, BLUE);
		}else if(strcmp(param, "GREEN") == 0){
				TCMD("RES:LCD:GREEN\n");
				LCD_Fill(0, 0, lcddev.width, lcddev.height, GREEN);
		}else if(strcmp(param, "WHITE") == 0){
				TCMD("RES:LCD:WHITE\n");
				LCD_Fill(0, 0, lcddev.width, lcddev.height, WHITE);
		}else if(strcmp(param, "BLACK") == 0){
				TCMD("RES:LCD:BLACK\n");
				LCD_Fill(0, 0, lcddev.width, lcddev.height, BLACK);
		}
	
}

//---------------------------------------------------------------------------------------------------------
// Function: key_test_cmd(char *param)
// Description: Factory test key function
//		- Get key scan result feedback to host
// Input: key test start / stop cmd
// Output: None
// Date: 2026/05/14
// Update: 
//----------------------------------------------------------------------------------------------------------
void key_test_cmd(char *param)
{
		uint8_t test_flag = 0;
		static uint32_t local_tick;
		if(strcmp(param, "START") == 0){
				TCMD("RES:KEY_TEST:OK\n");
				Factory_testing |= KEY_TEST_MASK;
				key.key_val = NONKEY;
				key.key_active = 0;
				//key.key_press = 0;
		}else if(strcmp(param, "STOP") == 0){
				TCMD("RES:KEY_TEST:OK\n");
				Factory_testing ^= KEY_TEST_MASK;
				key.key_val = NONKEY;
				key.key_active = 0;
				//key.key_press = 0;
		}
		if((Factory_testing & KEY_TEST_MASK) == KEY_TEST_MASK){
				local_tick = time_tick;
				
				while(1){
						Key_Scan();
						if(key.key_val == UPKEY){
								TCMD("EVENT:KEY:UP\n");
								key.key_val = NONKEY;
								key.key_active = 0;
								test_flag |= 0x01;
								local_tick = time_tick;
						}else if(key.key_val == DOWNKEY){
								TCMD("EVENT:KEY:DOWN\n");
								key.key_val = NONKEY;
								key.key_active = 0;
								test_flag |= 0x02;
								local_tick = time_tick;
						}else if(key.key_val == ENTERKEY){
								TCMD("EVENT:KEY:ENTER\n");
								key.key_val = NONKEY;
								key.key_active = 0;
								test_flag |= 0x04;
								local_tick = time_tick;
						}
						if(test_flag == 7) break;
						if(g_clock_time_exceed(local_tick, 10000)){
								TCMD("EVENT:KEY:TIMEOUT\n");
								key.key_val = NONKEY;
								key.key_active = 0;
								break;
						}
				}
		}

}

//---------------------------------------------------------------------------------------------------------
// Function: test_relay_cmd(char *param)
// Description: Factory test for relay on/off test
// Input: Relay test parameter string
// Output: None
// Date: 2026/05/14
// Update: 
//----------------------------------------------------------------------------------------------------------
void test_relay_cmd(char *param)
{
		if(strcmp(param, "START") == 0){
				TCMD("RES:RELAY:OK\n");
				Factory_testing |= RELAY_TEST_MASK;
		}else if(strcmp(param, "STOP") == 0){
				TCMD("RES:RELAY:OK\n");
				Factory_testing ^= RELAY_TEST_MASK;
				Relay = RELAY_OFF;
		}else if(strcmp(param, "ON") == 0){
				TCMD("RES:RELAY:ON\n");
				Relay = RELAY_ON;
		}else if(strcmp(param, "OFF") == 0){
				TCMD("RES:RELAY:OFF\n");
				Relay = RELAY_OFF;
		}
		
}

void set_vcc_cmd(char *param)
{
		if(strcmp(param, "START") == 0){
				TCMD("RES:VCC:OK\n");
				Factory_testing |= VCC_TEST_MASK;
		}else if(strcmp(param, "STOP") == 0){
				TCMD("RES:VCC:OK\n");
				Factory_testing ^= VCC_TEST_MASK;
		}
}

void get_vcc_cmd(void)
{
		char buffer[10];
	
		snprintf(buffer, sizeof(buffer), "%.2f", adc_ctrl.vcc_voltage);
	
		TCMD("DATA:VCC:%sV\n", buffer);
}
//---------------------------------------------------------------------------------------------------------
// Function: processCmd
// Description: Process received command from UART
// Input: None
// Output: None
// Date: 2026/05/03
// Update: Added factory test protocol parsing
//----------------------------------------------------------------------------------------------------------
static void processCmd(void)
{
    char cmd_name[32];
    char param[32];
    char* cmd_str;
		int len;
    
    // Get the received command string
    cmd_str = (char*)cmd_queue.rx_buffer;

    LOGD("Received: %s\n", cmd_str);

    // Remove trailing newline/carriage return if present
    len = strlen(cmd_str);
    while (len > 0 && (cmd_str[len-1] == '\n' || cmd_str[len-1] == '\r')) {
        cmd_str[len-1] = '\0';
        len--;
    }
    
    // Parse command
    if (parse_command(cmd_str, cmd_name, param) == eFALSE) {
        LOGD("Invalid command format: %s\n", cmd_str);
        return;
    }

    LOGD("Parsed: cmd=%s, param=%s\n", cmd_name, param);

    // Dispatch to appropriate handler
    if (strcmp(cmd_name, "CONNECT") == 0) {
				if((UI_state == STATE_ACTIVE) || (UI_state == STATE_SLEEP)){
						if(Factory_testing == 0){
								UI_state = STATE_FACTORY_TEST;
								LCD_Fill(0, 0, lcddev.width, lcddev.height, BLACK);
								delete_alarm(Min_Update_Alarm);
								delete_alarm(Active_Alarm);
								Factory_testing |= FACTORT_TEST_MASK;
								TCMD("RES:CONNECT:OK\n");					//Enable enter test Mode
						}
				}
    }
    else if (strcmp(cmd_name, "EXIT") == 0) {
        if(Factory_testing & FACTORT_TEST_MASK){
						Factory_testing = 0;
						TCMD("RES:EXIT:OK\n");						//Leave factory test
						UI_state = STATE_ACTIVE;
						if(register_alarm(Min_Update_Alarm, 60000) == 0){			//recall min update
								LOGE("Alarm Full!\r\n");
						}
						main_display_digi = Display_Room_Temp;
						Backlight_SetDuty(BACKLIGHT_DUTY_ACTIVE);
						if(register_alarm(Active_Alarm, ACTIVE_ALIVE_TIME) == eFALSE){
								LOGE("Alarm Fail\r\n");
						}
						Draw_Active_Menu();								//Go back normal mode
				}
    }
    else if (strcmp(cmd_name, "LCD") == 0) {
				if(Factory_testing & FACTORT_TEST_MASK){
						lcd_test_cmd(param);
				}
    }
    else if (strcmp(cmd_name, "KEY_TEST") == 0) {
				if(Factory_testing & FACTORT_TEST_MASK){
						key_test_cmd(param);
				}
    }
		else if (strcmp(cmd_name, "SENSOR") == 0) {
				if(Factory_testing & FACTORT_TEST_MASK){
						
				}
		}
		else if (strcmp(cmd_name, "VCC") == 0) {
				if(Factory_testing & FACTORT_TEST_MASK){
						set_vcc_cmd(param);
				}
		}
    else if (strcmp(cmd_name, "GET") == 0) {
        // Handle GET:TEMP or GET:VCC
        if (strcasecmp(param, "TEMP") == 0) {
            //get_temp_cmd();
        }
        else if (strcasecmp(param, "VCC") == 0) {
            get_vcc_cmd();
        }
        else {
            //send_response("GET", "INVALID");
        }
    }
    else if (strcmp(cmd_name, "RELAY") == 0) {
				if(Factory_testing & FACTORT_TEST_MASK){
						test_relay_cmd(param);
				}
    }
    else {
        LOGD("Unknown command: %s\n", cmd_name);
        TCMD("RES:UNKNOWN:COMMAND\n");
    }
}

//---------------------------------------------------------------------------------------------------------
// Funcation: g_cmd_handler
// Description: Handle UART1 received command process
// Input: None
// Output: None
// Date: 2026/05/03
// Update:
//----------------------------------------------------------------------------------------------------------
void g_cmd_handler(void)
{

		if(cmd_queue.F_received_complete){
				processCmd();

				cmd_queue.F_received_complete = eFALSE;
		}

}
