#include "GAS/gas.h"

static uint16_t s_gas_threshold = GAS_DEFAULT_THRESHOLD;

void gas_sensor_init(void) {
  GPIO_InitTypeDef GPIO_InitStructure;
  ADC_InitTypeDef ADC_InitStructure;

  RCC_APB2PeriphClockCmd(GAS_GPIO_CLK | GAS_ADC_CLK, ENABLE);

  RCC_ADCCLKConfig(RCC_PCLK2_Div6);

  GPIO_InitStructure.GPIO_Pin = GAS_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GAS_GPIO_PORT, &GPIO_InitStructure);

  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfChannel = 1;
  ADC_Init(GAS_ADC, &ADC_InitStructure);

  ADC_Cmd(GAS_ADC, ENABLE);

  ADC_ResetCalibration(GAS_ADC);
  while (ADC_GetResetCalibrationStatus(GAS_ADC))
    ;
  ADC_StartCalibration(GAS_ADC);
  while (ADC_GetCalibrationStatus(GAS_ADC))
    ;
}

uint16_t gas_sensor_read_raw(void) {
  uint32_t sum = 0;
  const uint8_t SAMPLES = 4;

  for (uint8_t i = 0; i < SAMPLES; i++) {
    ADC_RegularChannelConfig(GAS_ADC, GAS_ADC_CHANNEL, 1,
                             ADC_SampleTime_55Cycles5);
    ADC_SoftwareStartConvCmd(GAS_ADC, ENABLE);
    while (ADC_GetFlagStatus(GAS_ADC, ADC_FLAG_EOC) == RESET)
      ;
    sum += ADC_GetConversionValue(GAS_ADC);
  }

  return (uint16_t)(sum / SAMPLES);
}

bool gas_sensor_is_detected(void) {
  return (gas_sensor_read_raw() >= s_gas_threshold);
}

void gas_sensor_set_threshold(uint16_t threshold) {
  if (threshold >= 100 && threshold <= 4000) {
    s_gas_threshold = threshold;
  }
}

uint16_t gas_sensor_get_threshold(void) { return s_gas_threshold; }

void gas_sensor_auto_calibrate(void) {
  uint16_t current = gas_sensor_read_raw();
  uint32_t new_th = (uint32_t)current + 450;
  if (new_th > 3800) {
    new_th = 3800;
  }
  s_gas_threshold = (uint16_t)new_th;
}
