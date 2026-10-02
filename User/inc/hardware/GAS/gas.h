#ifndef __GAS_H__
#define __GAS_H__

#include "stm32f10x.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include <stdbool.h>
#include <stdint.h>

#define GAS_GPIO_PORT GPIOA
#define GAS_GPIO_CLK RCC_APB2Periph_GPIOA
#define GAS_PIN GPIO_Pin_0
#define GAS_ADC ADC1
#define GAS_ADC_CLK RCC_APB2Periph_ADC1
#define GAS_ADC_CHANNEL ADC_Channel_0

#define GAS_DEFAULT_THRESHOLD 1700

void gas_sensor_init(void);

uint16_t gas_sensor_read_raw(void);

bool gas_sensor_is_detected(void);

void gas_sensor_set_threshold(uint16_t threshold);

uint16_t gas_sensor_get_threshold(void);

void gas_sensor_auto_calibrate(void);

#endif // __GAS_H__
