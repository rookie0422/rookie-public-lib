#include "stm32f10x.h" 
#include "bsp_SysTick.h"
#include "bsp_led.h"
#include "bsp_key.h"
#include "sys.h"
#include "./beep/bsp_beep.h" 
#include "./ldr/bsp_ldr.h"
#include "OLED.h"
#include "bsp_adc.h"
void Init(void);

void Init(void)
{
	//BEEP_GPIO_Config();
	SysTick_Init();
	LED_GPIO_Config();
	//Key_GPIO_Config();
	//LDR_GPIO_Config();
	OLED_Init();
	AD_Init();

}



int main(void)
{
	Init();
	OLED_Clear();
	OLED_ShowString(2,1,"hello frj");
	uint16_t ad;
	
	
	while(1)
	{
		ad = (float)AD_GetValue() / 4095*100;
		OLED_ShowString(3,1,"Light:");
		OLED_ShowNum(3,7,100-ad,2);

	};

}




