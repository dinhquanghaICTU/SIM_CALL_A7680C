/**
 ******************************************************************************
 * @file    stm32f10x_it.c
 * @brief   Main Interrupt Service Routines.
 ******************************************************************************
 */

#include "stm32f10x_it.h"
#include "component/uart.h"

/* Weak function for tick increment, can be overridden by user hardware timer */
__attribute__((weak)) void tick_ms_increment(void) {}

void NMI_Handler(void) {}

void HardFault_Handler(void) {
  while (1) {
  }
}

void MemManage_Handler(void) {
  while (1) {
  }
}

void BusFault_Handler(void) {
  while (1) {
  }
}

void UsageFault_Handler(void) {
  while (1) {
  }
}

void DebugMon_Handler(void) {}

/* Note: SVC_Handler, PendSV_Handler và SysTick_Handler được quản lý trực tiếp bởi FreeRTOS (port.c) */

void vApplicationTickHook(void) {
  tick_ms_increment();
}

/**
 * @brief  Ngắt USART1 (Dùng cho DEBUG: PA9 - TX, PA10 - RX)
 */
void USART1_IRQHandler(void) {
  volatile uint32_t sr = USART1->SR;
  if (sr & (USART_SR_RXNE | USART_SR_ORE | USART_SR_NE | USART_SR_FE)) {
    uint8_t data = (uint8_t)(USART1->DR & 0xFF);
    if (sr & USART_SR_RXNE) {
      uart_debug_rx_handler(data);
    }
  }
}
