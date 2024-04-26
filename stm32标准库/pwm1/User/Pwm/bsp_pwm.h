#ifndef __BSP_PWM_H
#define __BSP_PWM_H

#include "stm32f10x.h"


#define            ARR_TIM_Period            (1000 - 1)
#define            PSC_TIM_Prescaler         (4 - 1)
#define            CCR_TIM_Pulse        		 500



void PWM_Init(void);
void PWM_SetCCR3(u16 Ccr);
void PWM_SetPSC(u16 Psc);

#endif
