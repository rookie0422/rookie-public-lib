/**
  ******************************************************************************
  * @file    bsp_photoresistors.c
  * @author  fire
  * @version V1.0
  * @date    2019-xx-xx
  * @brief   光敏模块测试
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火 F103-MINI STM32 开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */ 
  
#include "./ldr/bsp_ldr.h"
#include "OLED_I2C.h"
#include "sys.h"

 
/**
  * @brief  配置光敏用到的I/O口,中断
  * @param  无
  * @retval 无
  */
	
static void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  
  /* 配置NVIC为优先级组1 */
	/* 提示 NVIC_PriorityGroupConfig() 在整个工程只需要调用一次来配置优先级分组*/
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  
  /* 配置中断源：LDR */
  NVIC_InitStructure.NVIC_IRQChannel = LDR_INT_EXTI_IRQ;
  /* 配置抢占优先级 */
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  /* 配置子优先级 */
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  /* 使能中断通道 */
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}
	
	
void LDR_GPIO_Config(void)
{

	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	/*开启端口的时钟   AFIO*/
	RCC_APB2PeriphClockCmd(LDR_GPIO_CLK,ENABLE);
	
	//选择输入的引脚
	GPIO_InitStructure.GPIO_Pin = LDR_GPIO_PIN; 
	// 设置光敏输入的引脚为下拉输入
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
 * 函数名：光敏电阻测试
 * 描述  ：检测光敏电阻的状态
 * 输入  ：GPIOx：x 可以是 A，B，C，D或者 E
 *		     GPIO_Pin：待读取的端口位 	
 * 输出  ：LDR_ON：有光   LDR_OFF：无光
 */
uint8_t LDR_Test(GPIO_TypeDef* GPIOx,uint16_t GPIO_Pin)
{			
	/*检测光敏输入状态 */
	if(GPIO_ReadInputDataBit(GPIOx,GPIO_Pin) == 1 )  
	{	  
		return LDR_OFF;    // 无光
	}
	else
		return LDR_ON;    // 有光
}



//根据光敏电阻状态改变led颜色
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
