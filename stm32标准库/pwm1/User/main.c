#include "stm32f10x.h" 
#include "bsp_SysTick.h"
#include "bsp_led.h"
#include "bsp_key.h"
#include "sys.h"
#include "./beep/bsp_beep.h" 
#include "./ldr/bsp_ldr.h"
#include "OLED.h"
#include "bsp_pwm.h"
#include "bsp_ic.h"

void Init(void);

void Init(void)
{
	//BEEP_GPIO_Config();
	SysTick_Init();
	//LED_GPIO_Config();
	//Key_GPIO_Config();
	//LDR_GPIO_Config();
	OLED_Init();
	PWM_Init();
	IC_Init();
}



int main(void)
{


	Init();
	OLED_Clear();
	OLED_ShowString(2,1,"hello frj");
	//OLED_ShowString(3,1,"Freq:00000HZ");
	OLED_ShowString(4,1,"Duty:00%");
	//PWM_SetPSC(720-1);
	PWM_SetCCR3(178);
	
	while(1)
	{
		//OLED_ShowNum(3,6,IC_GetFreq(),5);
		OLED_ShowNum(4,6,IC_GetDuty(),2);
	
	};

}




