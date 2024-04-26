#include "bsp_pwm.h"                  // Device header
#include "bsp_SysTick.h"

void PWM_Init(void)
{
	//开启挂载在apb1总线的tim2
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,	ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	//  GPIO  PA2
	GPIO_InitTypeDef GPIO_InitStructure;
	/*选择要控制的GPIO引脚*/
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_2;	
	/*设置引脚模式为复用推挽输出*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   
	/*设置引脚速率为50MHz */   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	/*调用库函数，初始化GPIO*/
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	//设置 TIM2 内部时钟
	TIM_InternalClockConfig(TIM2);
	
	//初始化结构体
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	//设置分频，这里是1分频
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	//TIM_CounterMode 选择计数器模式，向上计数
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	//自动重装寄存器，
	TIM_TimeBaseInitStructure.TIM_Period = ARR_TIM_Period;  //  ARR
	//预分频器，
	TIM_TimeBaseInitStructure.TIM_Prescaler = PSC_TIM_Prescaler; // PSC
	//重复计数器的值，是高级计数器的功能，这里用不到，给0
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	//初始化时基单元
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStructure);
	
	
	//初始化oc输出比较通道
	
	TIM_OCInitTypeDef TIM_OCInitStructure;
	//给结构体赋初始值
	TIM_OCStructInit(&TIM_OCInitStructure);
	//模式
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	//极性
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	//使能
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	//    CCR
	TIM_OCInitStructure.TIM_Pulse = CCR_TIM_Pulse;
	
	//使用TIM3的ch3  对应gpio  PB0
	TIM_OC3Init(TIM2, &TIM_OCInitStructure);
	
	
	
	TIM_Cmd(TIM2, ENABLE);
	
}


void PWM_SetCCR3(u16 Ccr)
{
	TIM_SetCompare3(TIM2,Ccr);
}

void PWM_SetPSC(u16 Psc)
{
	TIM_PrescalerConfig(TIM2,Psc,TIM_PSCReloadMode_Immediate);
}






