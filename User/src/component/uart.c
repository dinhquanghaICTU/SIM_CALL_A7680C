#include "component/uart.h"
#include "component/delay.h"

static uint8_t s_debug_rx_buf[UART_BUFFER_SIZE];
static ringbuff_t s_debug_rb;

void uart_debug_init(uint32_t baudrate) {
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  ringbuff_init(&s_debug_rb, s_debug_rx_buf, sizeof(s_debug_rx_buf));

  RCC_APB2PeriphClockCmd(
      DEBUG_UART_GPIO_CLK | DEBUG_UART_CLK | RCC_APB2Periph_AFIO, ENABLE);

  GPIO_PinRemapConfig(GPIO_Remap_USART1, DISABLE);

  GPIO_InitStructure.GPIO_Pin = DEBUG_UART_TX_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(DEBUG_UART_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = DEBUG_UART_RX_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(DEBUG_UART_GPIO_PORT, &GPIO_InitStructure);

  USART_InitStructure.USART_BaudRate = baudrate;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl =
      USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(DEBUG_UART, &USART_InitStructure);

  USART_ITConfig(DEBUG_UART, USART_IT_RXNE, ENABLE);

  NVIC_InitStructure.NVIC_IRQChannel = DEBUG_UART_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  USART_Cmd(DEBUG_UART, ENABLE);
}

void uart_debug_send_char(char c) {
  uint32_t timeout = 100000;
  while ((USART_GetFlagStatus(DEBUG_UART, USART_FLAG_TXE) == RESET) &&
         --timeout)
    ;
  USART_SendData(DEBUG_UART, (uint16_t)c);
  timeout = 100000;
  while ((USART_GetFlagStatus(DEBUG_UART, USART_FLAG_TC) == RESET) && --timeout)
    ;
}

void uart_debug_send_string(const char *str) {
  if (!str)
    return;
  while (*str) {
    uart_debug_send_char(*str++);
  }
}

void uart_debug_send_bytes(const uint8_t *data, uint16_t len) {
  if (!data)
    return;
  for (uint16_t i = 0; i < len; i++) {
    uart_debug_send_char((char)data[i]);
  }
}

int uart_debug_printf(const char *format, ...) {
  char buf[256];
  va_list args;
  va_start(args, format);
  int len = vsnprintf(buf, sizeof(buf), format, args);
  va_end(args);

  if (len > 0) {
    uart_debug_send_string(buf);
  }
  return len;
}

void uart_debug_rx_handler(uint8_t data) {
  ringbuff_write(&s_debug_rb, &data, 1);
}

uint16_t uart_debug_available(void) {
  return (uint16_t)ringbuff_get_full(&s_debug_rb);
}

uint16_t uart_debug_read(uint8_t *buf, uint16_t max_len) {
  if (!buf || max_len == 0)
    return 0;
  return (uint16_t)ringbuff_read(&s_debug_rb, buf, max_len);
}

bool uart_debug_read_byte(uint8_t *byte) {
  if (!byte)
    return false;
  return (ringbuff_read(&s_debug_rb, byte, 1) == 1);
}

void uart_debug_flush(void) { ringbuff_reset(&s_debug_rb); }
