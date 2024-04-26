#include "bsp_Tim.h"                  // Device header

#include "OLED.h"
#include "sys.h"


//ͨ�ö�ʱ��  tim2
void Timer_Init(void)
{
	//����������apb1���ߵ�tim2
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,	ENABLE);
	//���� TIM2 �ڲ�ʱ��
	TIM_InternalClockConfig(TIM2);
	
	//��ʼ���ṹ��
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	//���÷�Ƶ��������1��Ƶ
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	//TIM_CounterMode ѡ�������ģʽ�����ϼ���
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	//�Զ���װ�Ĵ�����
	TIM_TimeBaseInitStructure.TIM_Period = GENERAL_TIM_Period;
	//Ԥ��Ƶ����
	TIM_TimeBaseInitStructure.TIM_Prescaler = GENERAL_TIM_Prescaler;
	//�ظ���������ֵ���Ǹ߼��������Ĺ��ܣ������ò�������0
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	//��ʼ��ʱ����Ԫ
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStructure);
	
	TIM_ClearFlag(TIM2, TIM_IT_Update);
	
	//ʹ�ܸ����ж�
	TIM_ITConfig(TIM2, TIM_IT_Update,  ENABLE);
	
	//NVIC
	NVIC_InitTypeDef NVIC_InitStructure;
	/* �����ж�Դ��TIM2 */
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
  /* ������ռ���ȼ� */
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =1;
  /* ���������ȼ� */
  NVIC_InitStructure.NVIC_IRQChannelSubPriority =2;
  /* ʹ���ж�ͨ�� */
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	//������ʱ��
	TIM_Cmd(TIM2, ENABLE);
	
}

