#include "iap_uart.h"
#include "stm32f10x.h"
#include "stm32f10x_usart.h"

extern volatile uint32_t time_tick;

/* UART ring buffer for interrupt-driven reception */
volatile uint8_t  uart_rx_buf[UART_RX_BUF_SIZE];
volatile uint16_t uart_rx_head = 0;
volatile uint16_t uart_rx_tail = 0;

void IAP_UART_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    USART_Cmd(USART1, ENABLE);

    /* Enable USART1 RX interrupt */
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    NVIC_EnableIRQ(USART1_IRQn);
}

void Serial_PutByte(uint8_t param)
{
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, param);
}

void Serial_PutString(uint8_t *p_string)
{
    while (*p_string != 0)
    {
        Serial_PutByte(*p_string++);
    }
}

void USART1_IRQHandler(void)
{
    /* Check for overrun error -- read SR before DR to clear ORE */
    if (USART_GetFlagStatus(USART1, USART_FLAG_ORE) != RESET)
    {
        /* Read DR to clear ORE; the byte is lost, move on */
        (void)USART_ReceiveData(USART1);
    }

    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        uint8_t data = USART_ReceiveData(USART1);
        uint16_t next = (uart_rx_head + 1) & (UART_RX_BUF_SIZE - 1);
        if (next != uart_rx_tail)
        {
            uart_rx_buf[uart_rx_head] = data;
            uart_rx_head = next;
        }
    }
}

uint32_t Serial_GetByte(uint8_t *p_byte, uint32_t timeout_ms)
{
    uint32_t tickstart = time_tick;

    while (uart_rx_head == uart_rx_tail)
    {
        if (timeout_ms != 0xFFFFFFFF && (time_tick - tickstart > timeout_ms))
        {
            return 1;
        }
    }

    *p_byte = uart_rx_buf[uart_rx_tail];
    uart_rx_tail = (uart_rx_tail + 1) & (UART_RX_BUF_SIZE - 1);
    return 0;
}

void Serial_Flush(void)
{
    uart_rx_tail = uart_rx_head;
}
