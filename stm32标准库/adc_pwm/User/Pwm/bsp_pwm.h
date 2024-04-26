#ifndef __BSP_PWM_H
#define __BSP_PWM_H

#include "stm32f10x.h"


#define            ARR_TIM_Period            (100 - 1)
#define            PSC_TIM_Prescaler         (720 - 1)
#define            CCR_TIM_Pulse        		 10



void PWM_Init(void);
void PWM_Led(void);

#endif
