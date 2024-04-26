#ifndef _AT_UART_H
#define _AT_UART_H

void HAL_AT_Send(char *buf, int len);
void HAL_AT_Secv(char *c, int timeout);

#endif

