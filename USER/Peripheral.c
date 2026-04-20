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
//File Function: Peripheral.c to define peripheral HW operation
//		- Key operation
//		- Log USART
//---------------------------------------------------------------------------------------------------------

#include "Thermostat.h"

#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)

USART_InitTypeDef USART_InitStructure;

USART_TypeDef* COM_USART[COMn] = {EVAL_COM1, EVAL_COM2}; 
GPIO_TypeDef* COM_TX_PORT[COMn] = {EVAL_COM1_TX_GPIO_PORT, EVAL_COM2_TX_GPIO_PORT};
GPIO_TypeDef* COM_RX_PORT[COMn] = {EVAL_COM1_RX_GPIO_PORT, EVAL_COM2_RX_GPIO_PORT};
const uint32_t COM_USART_CLK[COMn] = {EVAL_COM1_CLK, EVAL_COM2_CLK};
const uint32_t COM_TX_PORT_CLK[COMn] = {EVAL_COM1_TX_GPIO_CLK, EVAL_COM2_TX_GPIO_CLK};
const uint32_t COM_RX_PORT_CLK[COMn] = {EVAL_COM1_RX_GPIO_CLK, EVAL_COM2_RX_GPIO_CLK};
const uint16_t COM_TX_PIN[COMn] = {EVAL_COM1_TX_PIN, EVAL_COM2_TX_PIN};
const uint16_t COM_RX_PIN[COMn] = {EVAL_COM1_RX_PIN, EVAL_COM2_RX_PIN};

// Scan keys
// Return: 0 = no key, 1 = Up key, 2 = Down key, 3 = Enter key
uint8_t Key_Scan(void)
{
	static uint8_t key_up = 1;  // Key release flag
	static uint8_t key_release_cnt = 0;  // Key release debounce counter

	if(key_up)
	{
		if(GPIO_ReadInputDataBit(UP_KEY_PORT, UP_KEY_PIN) == 1)
		{
			delay_ms(5);  // Debounce (reduced from 10ms)
			if(GPIO_ReadInputDataBit(UP_KEY_PORT, UP_KEY_PIN) == 1)
			{
				key_up = 0;
				key_release_cnt = 0;
				return 1;  // Up key pressed
			}
		}
		else if(GPIO_ReadInputDataBit(DOWN_KEY_PORT, DOWN_KEY_PIN) == 1)
		{
			delay_ms(5);  // Debounce (reduced from 10ms)
			if(GPIO_ReadInputDataBit(DOWN_KEY_PORT, DOWN_KEY_PIN) == 1)
			{
				key_up = 0;
				key_release_cnt = 0;
				return 2;  // Down key pressed
			}
		}
		else if(GPIO_ReadInputDataBit(ENTER_KEY_PORT, ENTER_KEY_PIN) == 1)
		{
			delay_ms(5);  // Debounce (reduced from 10ms)
			if(GPIO_ReadInputDataBit(ENTER_KEY_PORT, ENTER_KEY_PIN) == 1)
			{
				key_up = 0;
				key_release_cnt = 0;
				return 3;  // Enter key pressed
			}
		}
	}
	else
	{
		// Wait for key release with debounce
		if(GPIO_ReadInputDataBit(UP_KEY_PORT, UP_KEY_PIN) == 0 &&
		   GPIO_ReadInputDataBit(DOWN_KEY_PORT, DOWN_KEY_PIN) == 0 &&
		   GPIO_ReadInputDataBit(ENTER_KEY_PORT, ENTER_KEY_PIN) == 0)
		{
			key_release_cnt++;
			if(key_release_cnt >= 3)  // Require 3 consecutive scans to confirm release
			{
				key_up = 1;
				key_release_cnt = 0;
			}
		}
		else
		{
			key_release_cnt = 0;
		}
	}

	return 0;  // No key
}


/**
  * @brief  Configures COM port.
  * @param  COM: Specifies the COM port to be configured.
  *   This parameter can be one of following parameters:    
  *     @arg COM1
  *     @arg COM2  
  * @param  USART_InitStruct: pointer to a USART_InitTypeDef structure that
  *   contains the configuration information for the specified USART peripheral.
  * @retval None
  */
void STM_EVAL_COMInit(COM_TypeDef COM, USART_InitTypeDef* USART_InitStruct)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIO clock */
  RCC_APB2PeriphClockCmd(COM_TX_PORT_CLK[COM] | COM_RX_PORT_CLK[COM] | RCC_APB2Periph_AFIO, ENABLE);

  /* Enable UART clock */
  if (COM == COM1)
  {
    RCC_APB2PeriphClockCmd(COM_USART_CLK[COM], ENABLE); 
  }
  else
  {
    RCC_APB1PeriphClockCmd(COM_USART_CLK[COM], ENABLE);
  }

  /* Configure USART Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pin = COM_TX_PIN[COM];
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(COM_TX_PORT[COM], &GPIO_InitStructure);

  /* Configure USART Rx as input floating */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Pin = COM_RX_PIN[COM];
  GPIO_Init(COM_RX_PORT[COM], &GPIO_InitStructure);

  /* USART configuration */
  USART_Init(COM_USART[COM], USART_InitStruct);
    
  /* Enable USART */
  USART_Cmd(COM_USART[COM], ENABLE);
}


/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  //uint32_t timeout = 0;
  
  /* Loop until transmit data register is empty (with timeout) */
  while (USART_GetFlagStatus(EVAL_COM1, USART_FLAG_TXE) == RESET);
  
  USART_SendData(EVAL_COM1, (uint8_t) ch);
  
  return ch;
}

void Log_USART_Init(void)
{
		/* USARTx configured as follow:
        - BaudRate = 115200 baud  
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
		*/
		USART_InitStructure.USART_BaudRate = 115200;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
		USART_InitStructure.USART_Parity = USART_Parity_No;
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

		STM_EVAL_COMInit(COM1, &USART_InitStructure);
	
}
