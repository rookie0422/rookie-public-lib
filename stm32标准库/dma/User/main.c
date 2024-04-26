#include "stm32f10x.h" 
#include "bsp_SysTick.h"
#include "bsp_led.h"
#include "bsp_key.h"
#include "sys.h"
#include "./beep/bsp_beep.h" 
#include "./ldr/bsp_ldr.h"
#include "OLED.h"
#include "bsp_dma.h"

uint8_t DataA[] = {0x01,0x02,0x03,0x04};
uint8_t DataB[] = {0,0,0,0};

void Init(void);

void Init(void)
{
	//BEEP_GPIO_Config();
	SysTick_Init();
	LED_GPIO_Config();
	//Key_GPIO_Config();
	//LDR_GPIO_Config();
	OLED_Init();
	MyDMA_Init((uint32_t)DataA,(uint32_t)DataB,4);
	
}

uint8_t aa = 0x69;

int main(void)
{


	Init();
	OLED_Clear();
	//OLED_ShowString(1,1,"hello frj");
	OLED_ShowString(1,1,"DataA:");
	OLED_ShowString(3,1,"DataB:");
	
	while(1)
	{
		DataA[0]++;
		DataA[1]++;
		DataA[2]++;
		DataA[3]++;
		
		OLED_ShowString(1,1,"DataA:");
		OLED_ShowHexNum(2,1,DataA[0],2);
		OLED_ShowHexNum(2,4,DataA[1],2);
		OLED_ShowHexNum(2,7,DataA[2],2);
		OLED_ShowHexNum(2,10,DataA[3],2);
		SysTick_Delay_Ms(1000);

		
		MyDMA_Trancfer();
		

		OLED_ShowHexNum(4,1,DataB[0],2);
		OLED_ShowHexNum(4,4,DataB[1],2);
		OLED_ShowHexNum(4,7,DataB[2],2);
		OLED_ShowHexNum(4,10,DataB[3],2);
		
		
	
	};

}




