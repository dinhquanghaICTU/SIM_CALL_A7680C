#include "led/led.h"
#include "component/delay.h"
#include "component/uart.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"

void led_init() {

  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(LED_GPIO_CLK, ENABLE);
  GPIO_InitStructure.GPIO_Pin = LED_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(LED_GPIO_PORT, &GPIO_InitStructure);
  GPIO_ResetBits(LED_GPIO_PORT, LED_PIN);
}

void led_on() { GPIO_SetBits(LED_GPIO_PORT, LED_PIN); }

void led_off() { GPIO_ResetBits(LED_GPIO_PORT, LED_PIN); }

void led_blink(uint32_t ms) {
  // uart_debug_send_string("da vao day\r\n");
  led_on();
  delay_ms(ms);
  led_off();
  delay_ms(ms);
}