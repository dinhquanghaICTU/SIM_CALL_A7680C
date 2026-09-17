#include "component/delay.h"
#include "component/uart.h"

static const struct {
  const char *cmd;
  const char *desc;
} g_check_sim_sequence[] = {
    {"AT\r", "1. Test ket noi UART module SIM"},
    {"ATE0\r", "2. Tat che do phan hoi lap lai (Echo OFF)"},
    {"AT+CPIN?\r", "3. Kiem tra the SIM (Can phai tra ve +CPIN: READY)"},
    {"AT+CCID\r", "4. Doc ma Seri the SIM Viettel (Dau 898404...)"},
    {"AT+CSQ\r", "5. Kiem tra cuong do song Viettel (Tot nhat: 15-31)"},
    {"AT+CEREG?\r", "6. Kiem tra dang ky mang 4G LTE (Can phai tra ve 0,1)"},
    {"AT+COPS?\r", "7. Kiem tra ten nha mang (Can phai ra 'Viettel')"}};

#define TOTAL_CMDS                                                             \
  (sizeof(g_check_sim_sequence) / sizeof(g_check_sim_sequence[0]))

int main(void) {

  systick_init();

  uart_debug_init(115200);

  uart_sim_init(115200);

  delay_s(1);

  uart_sim_send_string("\r");
  delay_ms(300);

  uint32_t last_time = get_tick_ms();
  uint8_t cmd_index = 0;

  while (1) {

    while (uart_sim_available() > 0) {
      uint8_t ch;
      if (uart_sim_read_byte(&ch)) {
        uart_debug_send_char((char)ch);
      }
    }

    while (uart_debug_available() > 0) {
      uint8_t ch;
      if (uart_debug_read_byte(&ch)) {
        uart_sim_send_char((char)ch);
      }
    }

    if (get_tick_ms() - last_time >= 2500) {
      last_time = get_tick_ms();

      if (cmd_index < TOTAL_CMDS) {
        uart_debug_printf(
            "\r\n---------------------------------------------------\r\n");
        uart_debug_printf("[BUOC %s]\r\n",
                          g_check_sim_sequence[cmd_index].desc);
        uart_debug_printf("[GUI LENH]: %s\r\n",
                          g_check_sim_sequence[cmd_index].cmd);

        uart_sim_send_string(g_check_sim_sequence[cmd_index].cmd);
        cmd_index++;
      } else {

        uart_debug_printf("\r\n[KIEM TRA SONG DINH KY]: AT+CSQ\r\n");
        uart_sim_send_string("AT+CSQ\r");
      }
    }
  }

  return 0;
}
