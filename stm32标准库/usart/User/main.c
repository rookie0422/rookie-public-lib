#include "stm32f10x.h" 
#include "bsp_SysTick.h"
#include "bsp_led.h"
#include "bsp_key.h"
#include "sys.h"
#include "./beep/bsp_beep.h" 
#include "./ldr/bsp_ldr.h"
#include "OLED.h"
#include "./usart/bsp_usart.h"

void Init(void);

void Init(void)
{
	
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	//BEEP_GPIO_Config();
	SysTick_Init();
	LED_GPIO_Config();
	//Key_GPIO_Config();
	//LDR_GPIO_Config();
	OLED_Init();
	USART_Config();
}



int main(void)
{


	Init();
	OLED_Clear();
	OLED_ShowString(2,1,"hello frj");
	printf("hello ∑Î»ŸΩ‹\r\n  %d\r\n",12345);

	while(1)
	{	

	};

}




