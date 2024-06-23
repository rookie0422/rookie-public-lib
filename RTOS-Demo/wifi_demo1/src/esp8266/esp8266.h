#ifndef _ESP8266_H_
#define _ESP8266_H_

#include "bsp.h"

#define REV_OK                 0
#define REV_WAIT               1

#define ESP8266_WIFI_INFO      "AT+CWJAP=\"test-WiFi\",\"12345678\"\r\n"

#define ESP8266_TCP_INFO       "AT+CIPSTART=\"TCP\",\"192.168.84.1\",777\r\n"
#define ESP8266_TCP_INFO_BAIDU "AT+CIPSTART=\"TCP\",\"14.215.177.38\",80\r\n"

void esp8266_init(UART_HandleTypeDef *huart);

void esp8266_clear(void);

void esp8266_send_data(unsigned char *data, unsigned short len);

unsigned char *esp8266_get_ipd(unsigned short timeOut);

#endif
