/**
  ******************************************************************************
  * @file    bsp_photoresistors.c
  * @author  fire
  * @version V1.0
  * @date    2019-xx-xx
  * @brief   ����ģ�����
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ�� F103-MINI STM32 ������ 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */ 
  
#include "./ldr/bsp_ldr.h"
#include "OLED_I2C.h"
#include "sys.h"

 
/**
  * @brief  ���ù����õ���I/O��,�ж�
  * @param  ��
  * @retval ��
  */
	
static void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  
  /* ����NVICΪ���ȼ���1 */
	/* ��ʾ NVIC_PriorityGroupConfig() ����������ֻ��Ҫ����һ�����������ȼ�����*/
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  
  /* �����ж�Դ��LDR */
  NVIC_InitStructure.NVIC_IRQChannel = LDR_INT_EXTI_IRQ;
  /* ������ռ���ȼ� */
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  /* ���������ȼ� */
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  /* ʹ���ж�ͨ�� */
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}
	
	
void LDR_GPIO_Config(void)
{

	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	/*�����˿ڵ�ʱ��   AFIO*/
	RCC_APB2PeriphClockCmd(LDR_GPIO_CLK,ENABLE);
	
	//ѡ�����������
	GPIO_InitStructure.GPIO_Pin = LDR_GPIO_PIN; 
	// ���ù������������Ϊ��������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 
	GPIO_Init(LDR_GPIO_PORT, &GPIO_InitStructure);
	
	NVIC_Configuration();
	
	GPIO_EXTILineConfig(LDR_INT_EXTI_PORTSOURCE, LDR_INT_EXTI_PINSOURCE); 
	
	
	//exti
	EXTI_InitStructure.EXTI_Line = LDR_INT_EXTI_LINE;  //
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	
}

 /*
 * �������������������
 * ����  �������������״̬
 * ����  ��GPIOx��x ������ A��B��C��D���� E
 *		     GPIO_Pin������ȡ�Ķ˿�λ 	
 * ���  ��LDR_ON���й�   LDR_OFF���޹�
 */
uint8_t LDR_Test(GPIO_TypeDef* GPIOx,uint16_t GPIO_Pin)
{			
	/*����������״̬ */
	if(GPIO_ReadInputDataBit(GPIOx,GPIO_Pin) == 1 )  
	{	  
		return LDR_OFF;    // �޹�
	}
	else
		return LDR_ON;    // �й�
}



//���ݹ�������״̬�ı�led��ɫ
void LDR_Led(void)
{
	
	
	OLED_CLS();
	if(LDR_Test(LDR_GPIO_PORT,LDR_GPIO_PIN)==LDR_ON)
		{
			
			OLED_ShowStr(0,3,"light source   ",1);
			
			PBout(5) = 0;
			PBout(1) = 1;
		}else if(LDR_Test(LDR_GPIO_PORT,LDR_GPIO_PIN)==LDR_OFF)
		{
			
			OLED_ShowStr(0,1,"no light source",1);
			
			PBout(5) = 1;
			PBout(1) = 0;
			
		}
	


}



/*********************************************END OF FILE**********************/
