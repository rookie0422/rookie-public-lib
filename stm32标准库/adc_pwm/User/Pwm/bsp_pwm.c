#include "bsp_pwm.h"                  // Device header
#include "bsp_SysTick.h"

void PWM_Init(void)
{
	//����������apb1���ߵ�tim2
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,	ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	//  GPIO  PB0
	GPIO_InitTypeDef GPIO_InitStructure;
	/*ѡ��Ҫ���Ƶ�GPIO����*/
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_0;	
	/*��������ģʽΪ�����������*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   
	/*������������Ϊ50MHz */   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	/*���ÿ⺯������ʼ��GPIO*/
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
	
	//���� TIM2 �ڲ�ʱ��
	TIM_InternalClockConfig(TIM3);
	
	//��ʼ���ṹ��
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	//���÷�Ƶ��������1��Ƶ
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	//TIM_CounterMode ѡ�������ģʽ�����ϼ���
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	//�Զ���װ�Ĵ�����
	TIM_TimeBaseInitStructure.TIM_Period = ARR_TIM_Period;  //  ARR
	//Ԥ��Ƶ����
	TIM_TimeBaseInitStructure.TIM_Prescaler = PSC_TIM_Prescaler; // PSC
	//�ظ���������ֵ���Ǹ߼��������Ĺ��ܣ������ò�������0
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	//��ʼ��ʱ����Ԫ
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
	
	
	//��ʼ��oc����Ƚ�ͨ��
	
	TIM_OCInitTypeDef TIM_OCInitStructure;
	//���ṹ�帳��ʼֵ
	TIM_OCStructInit(&TIM_OCInitStructure);
	//ģʽ
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	//����
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	//ʹ��
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	//    CCR
	TIM_OCInitStructure.TIM_Pulse = CCR_TIM_Pulse;
	
	//ʹ��TIM3��ch3  ��Ӧgpio  PB0
	TIM_OC3Init(TIM3, &TIM_OCInitStructure);
	
	
	
	TIM_Cmd(TIM3, ENABLE);
	
}


void PWM_Led(void)
{
	u16 i;
	for(i=0;i<100;i++)
	{
		TIM_SetCompare3(TIM3,i);
		SysTick_Delay_Ms(10);
	}
	for(i=0;i<100;i++)
	{
		TIM_SetCompare3(TIM3,100-i);
		SysTick_Delay_Ms(10);
	}
	
	
}




