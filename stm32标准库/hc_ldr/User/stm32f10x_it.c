/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "bsp_SysTick.h"
#include "./ldr/bsp_ldr.h"
#include "OLED.h"
#include "sys.h"
#include <stdio.h>
#include "./adc/bsp_adc.h"
#include "./hc-06/bsp_hc_usart2.h"


/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 

//void LDR_IRQHandler(void)
//{
//  //ȷ���Ƿ������EXTI Line�ж�
//	if(EXTI_GetITStatus(LDR_INT_EXTI_LINE) != RESET) 
//	{
//		
//		LDR_Led();
//    //����жϱ�־λ
//		EXTI_ClearITPendingBit(LDR_INT_EXTI_LINE);     
//	}  
//}

void USART1_IRQHandler(void)
{
	uint8_t Serial_Rxdata;
	if(USART_GetITStatus(USART1,USART_IT_RXNE) != RESET)
	{
		Serial_Rxdata = USART_ReceiveData(USART1);
		OLED_ShowChar(4,1,Serial_Rxdata);
	}
}


void USART2_IRQHandler(void)
{
	uint8_t Serial_Rxdata;
	if(USART_GetITStatus(USART2,USART_IT_RXNE) != RESET)
	{
		Serial_Rxdata = USART_ReceiveData(USART2);

		if(Serial_Rxdata=='A')
		{
			PBout(5) = 0;
			printf("�����\r\n");
			SysTick_Delay_Ms(1000);
			PBout(5) = 1;

		}
		if(Serial_Rxdata=='B')
		{
			PBout(1) = 0;
			SysTick_Delay_Ms(1000);
			PBout(1) = 1;
		}
		if(Serial_Rxdata=='C')
		{
			PAout(8) = 1;
			SysTick_Delay_Ms(1000);
			PAout(8) = 0;
		}

	}
}

void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		

		char String[100];
		sprintf(String,"Light = %.0f%%\r\n",100-(float)AD_Value[0] / 4095*100);
		Serial2_SendString(String);
		

		
		
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}

}



/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
