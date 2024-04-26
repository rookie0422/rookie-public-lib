#ifndef __BSP_HC_USART2_H
#define	__BSP_HC_USART2_H


#include "stm32f10x.h"


void Hc_USART2_Init(uint32_t baud);
void Serial2_SendByte(uint8_t Byte);
void Serial2_SendString(char* String);



#endif 





