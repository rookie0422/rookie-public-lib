#include "bsp.h"
#include "stm32f1xx_hal.h"
#include "main.h"

void led_on()
{
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
}

void led_off()
{
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
}

void led_toggole()
{
    HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
}