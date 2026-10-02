#ifndef __APP_H__
#define __APP_H__

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  STATE_IDLE = 0,
  STATE_VERIFYING,
  STATE_ALARM,
  STATE_MANUAL
} state_t;

typedef struct {
  state_t current_state;
  state_t next_state;

  bool fire_detected;
  bool gas_detected;
  bool led_status;
  bool buzzer_status;

  bool sensor_calibrated;

  uint32_t state_tick;
  uint32_t report_tick;
  uint32_t alert_pattern_tick;
  uint32_t flame_lost_tick;
  uint32_t hazard_lost_tick;
  uint32_t sensor_safe_tick;

  uint32_t led_auto_off_tick;
  uint32_t buzzer_auto_off_tick;
  uint32_t alarm_hold_ms;
  uint32_t blink_speed_ms;
  uint32_t verify_time_ms;
} m_state_t;

void app_init(void);
void app_process(void);
void app_send_status_json(void);

#endif // __APP_H__
