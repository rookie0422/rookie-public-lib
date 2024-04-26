#include "stm32f10x.h"                  // Device header


void IC_Init(void)
{
	//开启挂载在apb1总线的tim3
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,	ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	//  GPIO  PA6
	GPIO_InitTypeDef GPIO_InitStructure;
	/*选择要控制的GPIO引脚*/
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_6;	
	/*设置引脚模式为复用输入*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;   
	/*设置引脚速率为50MHz */   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	/*调用库函数，初始化GPIO*/
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	//设置 TIM2 内部时钟
	TIM_InternalClockConfig(TIM3);
	
	//初始化结构体
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	//设置分频，这里是1分频
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	//TIM_CounterMode 选择计数器模式，向上计数
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	//自动重装寄存器，
	TIM_TimeBaseInitStructure.TIM_Period = 65536-1;  //  ARR
	//预分频器，
	TIM_TimeBaseInitStructure.TIM_Prescaler = 72-1; // PSC
	//重复计数器的值，是高级计数器的功能，这里用不到，给0
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	//初始化时基单元
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);

	//初始化输入捕获单元
	TIM_ICInitTypeDef TIM_ICInitStructure;
	//定时器通道
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
	//选择输入比较滤波器。该参数取值在0x0和0xF之间。
	TIM_ICInitStructure.TIM_ICFilter = 0xF;
	//输入活动沿
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	//设置输入捕获预分频器
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	//选择输入通道 直连还是交叉
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	//ic模式，直连
	//TIM_ICInit(TIM3,&TIM_ICInitStructure);
	//  PWMI模式，交叉
	TIM_PWMIConfig(TIM3,&TIM_ICInitStructure);
	
	
	
	//配置主从，，触发源选择
	TIM_SelectInputTrigger(TIM3, TIM_TS_TI1FP1);
	//配置从模式
	TIM_SelectSlaveMode(TIM3,TIM_SlaveMode_Reset);
	
	
	
	//启动
	TIM_Cmd(TIM3,ENABLE);



}


u32 IC_GetFreq(void)
{
	return 1000000 / ( TIM_GetCapture1(TIM3) + 1 );
}

u32 IC_GetDuty(void)
{
	return ( TIM_GetCapture2(TIM3) + 1) * 100 / ( TIM_GetCapture1(TIM3) + 1 );
}

