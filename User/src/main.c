#include "Flame_Sensor/flame_sensor.h"
#include "FreeRTOS.h"
#include "application/app.h"
#include "component/delay.h"
#include "component/uart.h"
#include "led/led.h"
#include "ring/ring.h"
#include "task.h"

static void vAppTask(void *pvParameters) {
  (void)pvParameters;

  app_init();

  while (1) {
    app_process();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

int main(void) {

  uart_debug_init(115200);
  led_init();
  ring_init();
  flame_sensor_init();

  xTaskCreate(vAppTask, "AppTask", 256, NULL, 2, NULL);

  vTaskStartScheduler();

  while (1) {
  }

  return 0;
}
