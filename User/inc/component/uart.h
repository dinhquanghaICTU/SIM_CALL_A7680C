#ifndef __UART_H__
#define __UART_H__

#include "ringbuff/ringbuff.h"
#include "stm32f10x.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DEBUG_UART USART1
#define DEBUG_UART_CLK RCC_APB2Periph_USART1
#define DEBUG_UART_GPIO_CLK RCC_APB2Periph_GPIOA
#define DEBUG_UART_GPIO_PORT GPIOA
#define DEBUG_UART_TX_PIN GPIO_Pin_9
#define DEBUG_UART_RX_PIN GPIO_Pin_10
#define DEBUG_UART_IRQn USART1_IRQn

#define UART_BUFFER_SIZE 512

void uart_debug_init(uint32_t baudrate);
void uart_debug_send_char(char c);
void uart_debug_send_string(const char *str);
void uart_debug_send_bytes(const uint8_t *data, uint16_t len);
int uart_debug_printf(const char *format, ...);
uint16_t uart_debug_available(void);
uint16_t uart_debug_read(uint8_t *buf, uint16_t max_len);
bool uart_debug_read_byte(uint8_t *byte);
void uart_debug_flush(void);
void uart_debug_rx_handler(uint8_t data);

#ifdef __cplusplus
}
#endif

#endif // __UART_H__