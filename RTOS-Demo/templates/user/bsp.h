#include "stm32f1xx_hal.h"

extern TIM_HandleTypeDef htim4;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

void HAL_UART_IDLECallBack(UART_HandleTypeDef *huart);
void led_on();
void led_off();
void led_toggole();