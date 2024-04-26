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
	uint16_t ad1;
	uint16_t ad2;
	
	
	while(1)
	{
		ad1 = (float)AD_Value[0] / 4095*100;
		ad2 = (float)AD_Value[1]  / 4095*100;
		OLED_ShowString(3,1,"Light1:");
		OLED_ShowString(4,1,"Light2:");
		OLED_ShowNum(3,8,100-ad1,2);
		OLED_ShowNum(4,8,100-ad2,2);
		if(ad1 < ad2)
		{
			LED_BLUE;
		}else if(ad1 > ad2)
		{
			LED_GREEN;
		}else
		{
			LED_RED;
		}

	};

}




