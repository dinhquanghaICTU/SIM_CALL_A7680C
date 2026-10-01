#ifndef __RING_H__
#define __RING_H__

#include <stdint.h>

#define RING_GPIO_PORT GPIOB
#define RING_GPIO_CLK RCC_APB2Periph_GPIOB
#define RING_PIN GPIO_Pin_10

void ring_init(void);
void ring_on(void);
void ring_off(void);
void ring_beep(uint32_t ms);
void ring_beep_freq(uint32_t freq_hz, uint32_t duration_ms);

#endif //__RING_H__