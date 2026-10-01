#include "Flame_Sensor/flame_sensor.h"

void flame_sensor_init(void) {
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(FLAME_GPIO_CLK, ENABLE);

  GPIO_InitStructure.GPIO_Pin = FLAME_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(FLAME_GPIO_PORT, &GPIO_InitStructure);
}

uint8_t flame_sensor_read_raw(void) {
  return (uint8_t)GPIO_ReadInputDataBit(FLAME_GPIO_PORT, FLAME_PIN);
}

bool flame_sensor_is_detected(void) {
  return (GPIO_ReadInputDataBit(FLAME_GPIO_PORT, FLAME_PIN) ==
          FLAME_DETECTED_LEVEL);
}
