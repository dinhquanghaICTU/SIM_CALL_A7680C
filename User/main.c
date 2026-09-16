/**
 ******************************************************************************
 * @file    main.c
 * @brief   STM32F103C8T6 Standard Peripheral Library (StdPeriph) Demo
 *          Blink on-board LED PC13 (Blue Pill) & SysTick delay
 ******************************************************************************
 */

#include "stm32f10x.h"

/* Biến đếm mili-giây được tăng trong SysTick_Handler (stm32f10x_it.c) */
volatile uint32_t s_tick_ms = 0;

/* Hàm delay dựa trên SysTick 1ms */
void delay_ms(uint32_t ms) {
    uint32_t start = s_tick_ms;
    while ((s_tick_ms - start) < ms) {
        __NOP();
    }
}

/* Khởi tạo xung nhịp SysTick ngắt mỗi 1ms */
static void systick_init(void) {
    /* SystemCoreClock mặc định 72MHz khi chạy HSE PLL (hoặc 8MHz HSI) */
    SysTick_Config(SystemCoreClock / 1000);
}

/* Khởi tạo GPIO PC13 điều khiển LED tích hợp */
static void led_init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    /* 1. Bật Clock cho ngoại vi GPIOC */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    /* 2. Cấu hình chân PC13: Output Push-Pull, tốc độ 2MHz */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* Ban đầu tắt LED (PC13 active LOW, kéo lên 1 để tắt) */
    GPIO_SetBits(GPIOC, GPIO_Pin_13);
}

int main(void) {
    /* Cấu hình SystemInit đã được gọi trong startup assembly */
    SystemCoreClockUpdate();

    /* Khởi tạo SysTick và LED */
    systick_init();
    led_init();

    while (1) {
        /* Bật LED: Kéo chân PC13 xuống 0 */
        GPIO_ResetBits(GPIOC, GPIO_Pin_13);
        delay_ms(500);

        /* Tắt LED: Kéo chân PC13 lên 1 */
        GPIO_SetBits(GPIOC, GPIO_Pin_13);
        delay_ms(500);
    }
}
