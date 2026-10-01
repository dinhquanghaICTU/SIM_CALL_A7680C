#ifndef __FLAME_SENSOR_H__
#define __FLAME_SENSOR_H__

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include <stdbool.h>
#include <stdint.h>

#define FLAME_GPIO_PORT GPIOA
#define FLAME_GPIO_CLK RCC_APB2Periph_GPIOA
#define FLAME_PIN GPIO_Pin_1

#define FLAME_DETECTED_LEVEL Bit_RESET

void flame_sensor_init(void);

uint8_t flame_sensor_read_raw(void);

bool flame_sensor_is_detected(void);

#endif // __FLAME_SENSOR_H__