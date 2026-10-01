#include "ring/ring.h"
#include "component/delay.h"
#include "component/uart.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"

void ring_init(void) {
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RING_GPIO_CLK, ENABLE);
  GPIO_InitStructure.GPIO_Pin = RING_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(RING_GPIO_PORT, &GPIO_InitStructure);

  GPIO_ResetBits(RING_GPIO_PORT, RING_PIN);
}

void ring_on(void) { GPIO_SetBits(RING_GPIO_PORT, RING_PIN); }

void ring_off(void) { GPIO_ResetBits(RING_GPIO_PORT, RING_PIN); }

void ring_beep(uint32_t ms) {
  ring_on();
  delay_ms(ms);
  ring_off();
}

void ring_beep_freq(uint32_t freq_hz, uint32_t duration_ms) {
  if (freq_hz == 0)
    return;
  uint32_t half_period_us = 1000000 / (freq_hz * 2);
  uint32_t cycles = (freq_hz * duration_ms) / 1000;

  for (uint32_t i = 0; i < cycles; i++) {
    GPIO_SetBits(RING_GPIO_PORT, RING_PIN);
    delay_us(half_period_us);
    GPIO_ResetBits(RING_GPIO_PORT, RING_PIN);
    delay_us(half_period_us);
  }
}
