#include "esp8266.h"
#include "bsp.h"
#include <string.h>
#include <stdio.h>
#include "uart_interface.h"

#include "elog.h"
#define LOG_TAG "WiFi"

UART_HandleTypeDef *esp_usart;

char esp8266_buf[128];
unsigned short esp8266_cnt = 0, esp8266_cntPre = 0;

void usart_send_string(UART_HandleTypeDef *huart, unsigned char *str, unsigned short len)
{
    unsigned short count = 0;

    HAL_UART_Transmit(huart, (uint8_t *)str, len, 0xffff);
    while (HAL_UART_GetState(huart) == HAL_UART_STATE_BUSY_TX)
        ;
}

void esp8266_clear(void)
{
    memset(esp8266_buf, 0, sizeof(esp8266_buf));
    esp8266_cnt = 0;
}

uint8_t esp8266_wait_recive(void)
{
    if (esp8266_cnt == 0)
        return REV_WAIT;

    if (esp8266_cnt == esp8266_cntPre) {
        esp8266_cnt = 0;

        return REV_OK;
    }

    esp8266_cntPre = esp8266_cnt;

    return REV_WAIT;
}

uint8_t esp8266_send_cmd(char *cmd, char *res)
{
    unsigned char timeout = 200;

    usart_send_string(esp_usart, (unsigned char *)cmd, strlen((const char *)cmd));

    while (timeout--) {
        if (esp8266_wait_recive() == REV_OK) {
            log_i("%s\r\n", esp8266_buf);

            if (strstr((const char *)esp8266_buf, res) != NULL) {
                esp8266_clear();

                return 0;
            }
        }

        HAL_Delay(10);
    }

    return 1;
}

void esp8266_send_data(unsigned char *data, unsigned short len)
{
    char cmdBuf[32];

    esp8266_clear();
    sprintf(cmdBuf, "AT+CIPSEND=%d\r\n", len);
    if (!esp8266_send_cmd(cmdBuf, ">")) {
        usart_send_string(esp_usart, data, len);
    }
}

unsigned char *esp8266_get_ipd(unsigned short timeout)
{
    char *ptrIPD = NULL;

    do {
        if (esp8266_wait_recive() == REV_OK) {
            ptrIPD = strstr((char *)esp8266_buf, "IPD,");
            if (ptrIPD == NULL) {
                //printf("\"IPD\" not found\r\n");
            } else {
                ptrIPD = strchr(ptrIPD, ':');
                if (ptrIPD != NULL) {
                    ptrIPD++;
                    return (unsigned char *)(ptrIPD);
                } else
                    return NULL;
            }
        }

        HAL_Delay(5);
    } while (timeout--);

    return NULL;
}

void esp8266_init(UART_HandleTypeDef *huart)
{
    esp_usart = huart;

    HAL_UART_Receive_IT(esp_usart, (uint8_t *)&Uart3_RxData, 1);

    HAL_GPIO_WritePin(ESP8266_RST_GPIO_Port, ESP8266_RST_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(ESP8266_CH_PD_GPIO_Port, ESP8266_CH_PD_Pin, GPIO_PIN_SET);

    esp8266_clear();

    log_i("test AT...");
    while (esp8266_send_cmd("AT\r\n", "OK"))
        ;
    HAL_Delay(1000);

    log_i("set STA CWMODE...");
    while (esp8266_send_cmd("AT+CWMODE=1\r\n", "OK"))
        ;
    HAL_Delay(1000);

    log_i("get IP CIFSR...");
    while (esp8266_send_cmd("AT+CIFSR\r\n", "OK"))
        ;
    HAL_Delay(1000);

    log_i("reset ESP8266...");
    while (esp8266_send_cmd("AT+RST\r\n", "OK"))
        ;
    HAL_Delay(1000);

    log_i("connect AP CWJAP...");
    while (esp8266_send_cmd(ESP8266_WIFI_INFO, "OK"))
        ;
    HAL_Delay(1000);

    log_i("get IP CIFSR...");
    while (esp8266_send_cmd("AT+CIFSR\r\n", "OK"))
        ;
    HAL_Delay(1000);

    // log_i("get AP info...");
    // while (esp8266_send_cmd("AT+CWJAP?\r\n", "OK"))
    //     ;
    // HAL_Delay(1000);

    // log_i("get STA info...");
    // while (esp8266_send_cmd("AT+CIPSTA?\r\n", "OK"))
    //     ;
    // HAL_Delay(1000);

    // log_i("connect TCP Server CIPSTART...\r\n");
    // while (esp8266_send_cmd(ESP8266_TCP_INFO, "CONNECT"))
    //     ;
    // HAL_Delay(1000);

    log_i("ESP8266 Init OK");
}
