#include "Flame_Sensor/flame_sensor.h"
#include "component/delay.h"
#include "component/uart.h"
#include "led/led.h"
#include "ring/ring.h"

int main(void) {
  systick_init();
  uart_debug_init(115200);
  led_init();
  ring_init();
  flame_sensor_init();

  delay_ms(500);

  bool last_fire_state = false;

  while (1) {
    bool fire_detected = flame_sensor_is_detected();

    if (fire_detected != last_fire_state) {
      last_fire_state = fire_detected;

      if (fire_detected) {
        uart_debug_send_string("PHAT HIEN CO LUA!\r\n");
        led_on();
        ring_on();
      } else {
        uart_debug_send_string("Da an toan (het lua).\r\n");
        led_off();
        ring_off();
      }
    }

    while (uart_debug_available() > 0) {
      uint8_t ch;
      if (uart_debug_read_byte(&ch)) {
        uart_debug_send_char((char)ch);
      }
    }

    delay_ms(50);
  }

  return 0;
}
