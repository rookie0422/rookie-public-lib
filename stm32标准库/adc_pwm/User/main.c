#include "stm32f10x.h" 
#include "bsp_SysTick.h"
#include "bsp_led.h"
#include "bsp_key.h"
#include "sys.h"
#include "./beep/bsp_beep.h" 
#include "./ldr/bsp_ldr.h"
#include "OLED.h"
#include "bsp_adc.h"
#include "bsp_pwm.h" 

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
	PWM_Init();

}



int main(void)
{
	Init();
	OLED_Clear();
	OLED_ShowString(2,1,"hello frj");
	uint16_t ad1;
	u16 i;
	
	
	while(1)
	{
		ad1 = (float)AD_Value[0] / 4095*100;

		OLED_ShowString(3,1,"Light1:");

		OLED_ShowNum(3,8,100-ad1,2);
		//PWM_Led();
		i = 100-(float)AD_Value[0] / 4095*100;
		TIM_SetCompare3(TIM3,i);
		SysTick_Delay_Ms(10);


		

	};

}




