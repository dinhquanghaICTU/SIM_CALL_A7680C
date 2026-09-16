#ifndef __DELAY_H__
#define __DELAY_H__

#include "stm32f10x.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void systick_init(void);

uint32_t get_tick_ms(void);

void delay_ms(uint32_t ms);

void delay_s(uint32_t s);

void delay_us(uint32_t us);

#ifdef __cplusplus
}
#endif

#endif // __DELAY_H__
