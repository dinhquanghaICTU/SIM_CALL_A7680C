
#include "component/delay.h"

static volatile uint32_t s_ticks = 0;
static uint8_t s_systick_initialized = 0;

void tick_ms_increment(void) { s_ticks++; }

void systick_init(void) {

  SystemCoreClockUpdate();

  SysTick_Config(SystemCoreClock / 1000);

  s_systick_initialized = 1;
}

uint32_t get_tick_ms(void) { return s_ticks; }

void delay_ms(uint32_t ms) {
  if (!s_systick_initialized) {
    systick_init();
  }

  uint32_t start = s_ticks;
  while ((s_ticks - start) < ms) {
    __NOP();
  }
}

void delay_s(uint32_t s) {
  while (s--) {
    delay_ms(1000);
  }
}

void delay_us(uint32_t us) {
  if (!s_systick_initialized) {
    systick_init();
  }

  uint32_t ticks_per_us = SystemCoreClock / 1000000;
  uint32_t total_ticks = us * ticks_per_us;

  uint32_t start_val = SysTick->VAL;
  uint32_t elapsed = 0;

  while (elapsed < total_ticks) {
    uint32_t current_val = SysTick->VAL;
    if (current_val <= start_val) {
      elapsed += (start_val - current_val);
    } else {

      elapsed += (start_val + (SysTick->LOAD - current_val));
    }
    start_val = current_val;
  }
}
