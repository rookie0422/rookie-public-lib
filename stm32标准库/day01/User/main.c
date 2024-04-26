#include "stm32f10x.h" 
#include "bsp_SysTick.h"
#include "bsp_led.h"
#include "bsp_key.h"
#include "sys.h"
#include "./beep/bsp_beep.h" 
#include "./ldr/bsp_ldr.h"
#include "OLED_I2C.h"

void Init(void);

void Init(void)
{
	BEEP_GPIO_Config();
	SysTick_Init();
	LED_GPIO_Config();
	Key_GPIO_Config();
	LDR_GPIO_Config();
	I2C_Configuration();
	OLED_Init();
}



int main(void)
{


	Init();
	OLED_CLS();
	while(1)
	{

		
	
	};

}




