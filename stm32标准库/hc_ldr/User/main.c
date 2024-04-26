#include "stm32f10x.h" 
#include "bsp_SysTick.h"
#include "bsp_led.h"
#include "bsp_key.h"
#include "sys.h"
#include "./beep/bsp_beep.h" 
#include "./ldr/bsp_ldr.h"
#include "OLED.h"
#include "./adc/bsp_adc.h"
#include "./usart/bsp_usart.h"
#include "./hc-06/bsp_hc_usart2.h"

void Init(void);

void Init(void)
{
	
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	BEEP_GPIO_Config();
	SysTick_Init();
	LED_GPIO_Config();
	//Key_GPIO_Config();
	//LDR_GPIO_Config();
	OLED_Init();
//	AD_Init();
//	Timer_Init();
	USART_Config(9600);
	Hc_USART2_Init(9600);
	printf("蓝牙初始化成功\r\n");
	Serial2_SendString("蓝牙初始化成功");
}



int main(void)
{


	Init();
	OLED_Clear();
	OLED_ShowString(1,1,"hello frj");
	printf("hello 冯荣杰\r\n");
//		uint16_t ad1;


	while(1)
	{	
//		ad1 = (float)AD_Value[0] / 4095*100;
//		OLED_ShowString(3,1,"Light1:");
//		OLED_ShowNum(3,8,100-ad1,2);


	};

}




