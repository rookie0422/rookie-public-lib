#include "bsp_Tim.h"                  // Device header

#include "OLED.h"
#include "sys.h"


//通用定时器  tim2
void Timer_Init(void)
{
	//开启挂载在apb1总线的tim2
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,	ENABLE);
	//设置 TIM2 内部时钟
	TIM_InternalClockConfig(TIM2);
	
	//初始化结构体
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	//设置分频，这里是1分频
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	//TIM_CounterMode 选择计数器模式，向上计数
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	//自动重装寄存器，
	TIM_TimeBaseInitStructure.TIM_Period = GENERAL_TIM_Period;
	//预分频器，
	TIM_TimeBaseInitStructure.TIM_Prescaler = GENERAL_TIM_Prescaler;
	//重复计数器的值，是高级计数器的功能，这里用不到，给0
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	//初始化时基单元
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStructure);
	
	TIM_ClearFlag(TIM2, TIM_IT_Update);
	
	//使能更新中断
	TIM_ITConfig(TIM2, TIM_IT_Update,  ENABLE);
	
	//NVIC
	NVIC_InitTypeDef NVIC_InitStructure;
	/* 配置中断源：TIM2 */
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
  /* 配置抢占优先级 */
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =1;
  /* 配置子优先级 */
  NVIC_InitStructure.NVIC_IRQChannelSubPriority =2;
  /* 使能中断通道 */
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	//启动定时器
	TIM_Cmd(TIM2, ENABLE);
	
}

