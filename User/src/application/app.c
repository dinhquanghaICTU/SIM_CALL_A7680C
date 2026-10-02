#include "application/app.h"
#include "Flame_Sensor/flame_sensor.h"
#include "GAS/gas.h"
#include "component/delay.h"
#include "component/uart.h"
#include "jsmn/jsmn.h"
#include "led/led.h"
#include "ring/ring.h"
#include <stdio.h>
#include <string.h>

#define JSON_RX_MAX_LEN 128
#define VERIFY_TIME_MS 100
#define REPORT_INTERVAL_MS 2000

static m_state_t g_app;
static char s_json_rx_buf[JSON_RX_MAX_LEN];
static uint16_t s_json_rx_idx = 0;
static char s_json_tx_buf[256];

static bool jsoneq(const char *json, jsmntok_t *tok, const char *s) {
  if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
      strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return true;
  }
  return false;
}

static int parse_int(const char *json, jsmntok_t *tok) {
  int val = 0;
  for (int i = tok->start; i < tok->end; i++) {
    if (json[i] >= '0' && json[i] <= '9') {
      val = val * 10 + (json[i] - '0');
    }
  }
  return val;
}

void app_send_status_json(void) {
  const char *state_names[] = {"IDLE", "VERIFYING", "ALARM", "MANUAL"};
  const char *st_str = "IDLE";

  if (g_app.current_state <= STATE_MANUAL) {
    st_str = state_names[g_app.current_state];
  }

  uint16_t gas_adc = gas_sensor_read_raw();

  snprintf(s_json_tx_buf, sizeof(s_json_tx_buf),
           "{\"fire\":%d,\"gas\":%d,\"raw\":%d,\"gas_raw\":%u,\"gas_thresh\":%"
           "u,\"led\":%d,\"buzzer\":%d,\"state\":\"%s\","
           "\"alarm_hold\":%lu,\"blink_speed\":%lu,\"verify_time\":%lu}\r\n",
           g_app.fire_detected ? 1 : 0, g_app.gas_detected ? 1 : 0,
           flame_sensor_read_raw(), gas_adc, gas_sensor_get_threshold(),
           g_app.led_status ? 1 : 0, g_app.buzzer_status ? 1 : 0, st_str,
           (unsigned long)g_app.alarm_hold_ms,
           (unsigned long)g_app.blink_speed_ms,
           (unsigned long)g_app.verify_time_ms);

  uart_debug_send_string(s_json_tx_buf);
}

static void app_parse_json_command(const char *json_str) {
  jsmn_parser parser;
  jsmntok_t tokens[20];

  jsmn_init(&parser);
  int r = jsmn_parse(&parser, json_str, strlen(json_str), tokens, 20);

  if (r < 0 || tokens[0].type != JSMN_OBJECT) {
    return;
  }

  uint32_t now = get_tick_ms();
  bool state_changed = false;
  uint32_t duration_ms = 0;

  for (int i = 1; i < r; i++) {
    if (jsoneq(json_str, &tokens[i], "time") ||
        jsoneq(json_str, &tokens[i], "duration")) {
      duration_ms = (uint32_t)parse_int(json_str, &tokens[i + 1]);
      i++;
    }
  }

  for (int i = 1; i < r; i++) {

    if (jsoneq(json_str, &tokens[i], "led")) {
      char val_char = json_str[tokens[i + 1].start];
      bool new_led = (val_char == '1' || val_char == 't');
      g_app.led_status = new_led;
      if (new_led) {
        led_on();
        g_app.led_auto_off_tick = (duration_ms > 0) ? (now + duration_ms) : 0;
      } else {
        led_off();
        g_app.led_auto_off_tick = 0;
      }
      g_app.next_state = STATE_MANUAL;
      g_app.current_state = STATE_MANUAL;
      state_changed = true;
      i++;
    }

    else if (jsoneq(json_str, &tokens[i], "buzzer") ||
             jsoneq(json_str, &tokens[i], "ring")) {
      char val_char = json_str[tokens[i + 1].start];
      bool new_buzz = (val_char == '1' || val_char == 't');
      g_app.buzzer_status = new_buzz;
      if (new_buzz) {
        ring_on();
        g_app.buzzer_auto_off_tick =
            (duration_ms > 0) ? (now + duration_ms) : 0;
      } else {
        ring_off();
        g_app.buzzer_auto_off_tick = 0;
      }
      g_app.next_state = STATE_MANUAL;
      g_app.current_state = STATE_MANUAL;
      state_changed = true;
      i++;
    }

    else if (jsoneq(json_str, &tokens[i], "alarm_hold")) {
      int hold = parse_int(json_str, &tokens[i + 1]);
      if (hold >= 500 && hold <= 60000) {
        g_app.alarm_hold_ms = (uint32_t)hold;
      }
      state_changed = true;
      i++;
    }

    else if (jsoneq(json_str, &tokens[i], "blink_speed")) {
      int spd = parse_int(json_str, &tokens[i + 1]);
      if (spd >= 50 && spd <= 2000) {
        g_app.blink_speed_ms = (uint32_t)spd;
      }
      state_changed = true;
      i++;
    }

    else if (jsoneq(json_str, &tokens[i], "verify_time")) {
      int vt = parse_int(json_str, &tokens[i + 1]);
      if (vt >= 100 && vt <= 10000) {
        g_app.verify_time_ms = (uint32_t)vt;
      }
      state_changed = true;
      i++;
    }

    else if (jsoneq(json_str, &tokens[i], "gas_thresh") ||
             jsoneq(json_str, &tokens[i], "gas_threshold")) {
      int th = parse_int(json_str, &tokens[i + 1]);
      if (th >= 100 && th <= 4000) {
        gas_sensor_set_threshold((uint16_t)th);
      }
      state_changed = true;
      i++;
    }

    else if (jsoneq(json_str, &tokens[i], "calibrate_gas") ||
             jsoneq(json_str, &tokens[i], "gas_calib")) {
      gas_sensor_auto_calibrate();
      state_changed = true;
      i++;
    }

    else if (jsoneq(json_str, &tokens[i], "mode") || strstr(json_str, "mode")) {
      if (strstr(json_str, "auto")) {
        g_app.led_auto_off_tick = 0;
        g_app.buzzer_auto_off_tick = 0;
        g_app.sensor_calibrated = true;
        g_app.next_state = STATE_IDLE;
        g_app.current_state = STATE_IDLE;
        g_app.led_status = false;
        g_app.buzzer_status = false;
        led_off();
        ring_off();
        g_app.state_tick = now;
        g_app.flame_lost_tick = now;
        g_app.hazard_lost_tick = now;
        state_changed = true;
      }
      i++;
    }

    else if (jsoneq(json_str, &tokens[i], "get")) {
      state_changed = true;
      i++;
    }
  }

  if (state_changed) {
    app_send_status_json();
  }
}

static void app_check_uart_rx(void) {
  uint8_t ch;
  while (uart_debug_read_byte(&ch)) {
    if (ch == '\n' || ch == '\r') {
      if (s_json_rx_idx > 0) {
        s_json_rx_buf[s_json_rx_idx] = '\0';
        app_parse_json_command(s_json_rx_buf);
        s_json_rx_idx = 0;
      }
    } else if (s_json_rx_idx < (JSON_RX_MAX_LEN - 1)) {
      s_json_rx_buf[s_json_rx_idx++] = (char)ch;
    } else {
      s_json_rx_idx = 0;
    }
  }
}

void app_init(void) {
  memset(&g_app, 0, sizeof(m_state_t));
  g_app.current_state = STATE_IDLE;
  g_app.next_state = STATE_IDLE;

  led_init();
  ring_init();
  flame_sensor_init();
  gas_sensor_init();

  led_off();
  ring_off();

  g_app.alarm_hold_ms = 3000;
  g_app.blink_speed_ms = 150;
  g_app.verify_time_ms = 300;
  g_app.sensor_calibrated = true;
  g_app.sensor_safe_tick = 0;

  uint32_t now = get_tick_ms();
  g_app.state_tick = now;
  g_app.report_tick = now;
  g_app.alert_pattern_tick = now;
  g_app.flame_lost_tick = now;
  g_app.hazard_lost_tick = now;

  uart_debug_send_string(
      "\r\n{\"event\":\"SYSTEM_INIT\",\"msg\":\"App Started - Armed\"}\r\n");
  app_send_status_json();
}

void app_process(void) {
  uint32_t now = get_tick_ms();

  app_check_uart_rx();

  g_app.fire_detected = flame_sensor_is_detected();
  uint16_t gas_raw = gas_sensor_read_raw();
  g_app.gas_detected = (gas_raw >= gas_sensor_get_threshold());
  bool hazard_detected = (g_app.fire_detected || g_app.gas_detected);

  if (g_app.current_state != g_app.next_state) {
    g_app.current_state = g_app.next_state;
    g_app.state_tick = now;

    if (g_app.current_state == STATE_IDLE) {
      g_app.led_status = false;
      g_app.buzzer_status = false;
      g_app.led_auto_off_tick = 0;
      g_app.buzzer_auto_off_tick = 0;
      led_off();
      ring_off();
      app_send_status_json();
    } else if (g_app.current_state == STATE_ALARM) {
      g_app.led_status = true;
      g_app.buzzer_status = true;
      led_on();
      ring_on();
      app_send_status_json();
    } else if (g_app.current_state == STATE_VERIFYING) {
      app_send_status_json();
    } else if (g_app.current_state == STATE_MANUAL) {
      app_send_status_json();
    }
  }

  if (g_app.current_state == STATE_MANUAL) {
    if (g_app.led_auto_off_tick > 0 && now >= g_app.led_auto_off_tick) {
      g_app.led_auto_off_tick = 0;
      g_app.led_status = false;
      led_off();
      app_send_status_json();
    }
    if (g_app.buzzer_auto_off_tick > 0 && now >= g_app.buzzer_auto_off_tick) {
      g_app.buzzer_auto_off_tick = 0;
      g_app.buzzer_status = false;
      ring_off();
      app_send_status_json();
    }
    if ((now - g_app.report_tick) >= REPORT_INTERVAL_MS) {
      g_app.report_tick = now;
      app_send_status_json();
    }
    return;
  }

  if (g_app.led_auto_off_tick > 0 && now >= g_app.led_auto_off_tick) {
    g_app.led_auto_off_tick = 0;
    g_app.led_status = false;
    led_off();
    app_send_status_json();
  }

  if (g_app.buzzer_auto_off_tick > 0 && now >= g_app.buzzer_auto_off_tick) {
    g_app.buzzer_auto_off_tick = 0;
    g_app.buzzer_status = false;
    ring_off();
    app_send_status_json();
  }

  if (hazard_detected) {
    g_app.flame_lost_tick = now;
    g_app.hazard_lost_tick = now;

    if (g_app.current_state == STATE_IDLE) {
      g_app.next_state = STATE_VERIFYING;
    } else if (g_app.current_state == STATE_VERIFYING) {
      if ((now - g_app.state_tick) >= g_app.verify_time_ms) {
        g_app.next_state = STATE_ALARM;
      }
    }
  } else {
    if (g_app.current_state == STATE_VERIFYING) {

      g_app.next_state = STATE_IDLE;
    } else if (g_app.current_state == STATE_ALARM) {

      if ((now - g_app.hazard_lost_tick) >= g_app.alarm_hold_ms) {
        g_app.next_state = STATE_IDLE;
      }
    }
  }

  if (g_app.current_state != g_app.next_state) {
    g_app.current_state = g_app.next_state;
    g_app.state_tick = now;

    if (g_app.current_state == STATE_IDLE) {
      g_app.led_status = false;
      g_app.buzzer_status = false;
      g_app.led_auto_off_tick = 0;
      g_app.buzzer_auto_off_tick = 0;
      led_off();
      ring_off();
      app_send_status_json();
    } else if (g_app.current_state == STATE_ALARM) {
      g_app.led_status = true;
      g_app.buzzer_status = true;
      led_on();
      ring_on();
      app_send_status_json();
    } else if (g_app.current_state == STATE_VERIFYING) {
      app_send_status_json();
    } else if (g_app.current_state == STATE_MANUAL) {
      app_send_status_json();
    }
  }

  switch (g_app.current_state) {
  case STATE_IDLE:

    if (g_app.led_status && g_app.led_auto_off_tick == 0) {
      g_app.led_status = false;
      led_off();
    }
    if (g_app.buzzer_status && g_app.buzzer_auto_off_tick == 0) {
      g_app.buzzer_status = false;
      ring_off();
    }
    break;

  case STATE_ALARM:

    if ((now - g_app.alert_pattern_tick) >= g_app.blink_speed_ms) {
      g_app.alert_pattern_tick = now;
      g_app.led_status = !g_app.led_status;
      g_app.buzzer_status = g_app.led_status;
      if (g_app.led_status) {
        led_on();
        ring_on();
      } else {
        led_off();
        ring_off();
      }
    }
    break;

  case STATE_VERIFYING:
  case STATE_MANUAL:
  default:
    break;
  }

  if ((now - g_app.report_tick) >= REPORT_INTERVAL_MS) {
    g_app.report_tick = now;
    app_send_status_json();
  }
}
