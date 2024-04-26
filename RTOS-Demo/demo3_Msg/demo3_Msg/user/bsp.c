#include "bsp.h"
#include "stm32f1xx_hal.h"
#include "main.h"
#include "elog.h"

void led_g_on(void)
{
    HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_RESET);

}

void led_g_off(void)
{
    HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_SET);

}

void led_g_toggle(void)
{
    HAL_GPIO_TogglePin(LED_G_GPIO_Port, LED_G_Pin);

}

void led_b_on(void)
{
    HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin, GPIO_PIN_RESET);

}

void led_b_off(void)
{
    HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin, GPIO_PIN_SET);

}

void led_b_toggle(void)
{
    HAL_GPIO_TogglePin(LED_B_GPIO_Port, LED_B_Pin);

}
void led_r_on(void)
{
    HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_RESET);

}

void led_r_off(void)
{
    HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_SET);

}

void led_r_toggle(void)
{
    HAL_GPIO_TogglePin(LED_R_GPIO_Port, LED_R_Pin);

}
void keyscan(void)
{
    if (HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) ==  SET){
        
        led_b_on();
        
    }
    
    if (HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) ==  SET){
        
        led_b_off();
    }

}

