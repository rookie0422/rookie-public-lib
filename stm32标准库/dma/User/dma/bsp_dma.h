#ifndef __BSP_DAM_H
#define __BSP_DAM_H

#include "stm32f10x.h"

void MyDMA_Init(uint32_t AddrA,uint32_t AddrB,uint16_t Size);
void MyDMA_Trancfer(void);


#endif
