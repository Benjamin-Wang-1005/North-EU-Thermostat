#ifndef __IAP_UART_H
#define __IAP_UART_H

#include "stm32f10x.h"

#define TX_TIMEOUT      100
#define RX_TIMEOUT      0xFFFFFFFF

/* UART ring buffer for interrupt-driven reception */
#define UART_RX_BUF_SIZE    1024

extern volatile uint8_t  uart_rx_buf[UART_RX_BUF_SIZE];
extern volatile uint16_t uart_rx_head;
extern volatile uint16_t uart_rx_tail;

void IAP_UART_Init(void);
void Serial_PutByte(uint8_t param);
void Serial_PutString(uint8_t *p_string);
uint32_t Serial_GetByte(uint8_t *p_byte, uint32_t timeout_ms);
void Serial_Flush(void);

#endif

