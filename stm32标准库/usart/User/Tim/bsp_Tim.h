#ifndef __BSP_TIM_H
#define	__BSP_TIM_H


#include "stm32f10x.h"                  // Device header





#define            GENERAL_TIM_Period            (10000 - 1)
#define            GENERAL_TIM_Prescaler         (7200 - 1)


void Timer_Init(void);

#endif
