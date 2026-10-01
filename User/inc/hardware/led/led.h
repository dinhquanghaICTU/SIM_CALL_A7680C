#ifndef __LED_H__
#define __LED_H__
#include "config.h"
#include "stdint.h"

#define LED_GPIO_PORT GPIOB
#define LED_GPIO_CLK RCC_APB2Periph_GPIOB
#define LED_PIN GPIO_Pin_6

void led_init();

void led_on();
void led_off();
void led_blink(uint32_t ms);

#endif //__LED_H__