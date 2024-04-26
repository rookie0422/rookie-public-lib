
#include "stm32f1xx_hal.h"
#include "main.h"

void led_g_on(void);

void led_g_off(void);

void led_g_toggle(void);

void led_r_on(void);

void led_r_off(void);

void led_r_toggle(void);

void led_b_on(void);

void led_b_off(void);

void led_b_toggle(void);

void keyscan(void);

extern TIM_HandleTypeDef htim4;

extern UART_HandleTypeDef huart1;
