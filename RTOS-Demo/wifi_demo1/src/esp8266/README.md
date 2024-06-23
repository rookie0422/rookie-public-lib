
### 添加 usart中断接收处理

```c
    extern unsigned short esp8266_cnt;
    extern unsigned char esp8266_buf[128];

    #define RXBUFFERSIZE3 256           //最大接收字节数	串口3
    char Uart3_RxBuffer[RXBUFFERSIZE3]; //接收数据
    uint8_t Uart3_RxData;               //接收中断缓冲
    uint8_t Uart3_Rx_Cnt = 0;           //接收缓冲计数

    void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
    {
        if (huart == &huart3) {
            if (esp8266_cnt >= sizeof(esp8266_buf)) {
                esp8266_cnt = 0;
            }
            esp8266_buf[esp8266_cnt++] = Uart3_RxData;

            HAL_UART_Receive_IT(&huart3, (uint8_t *)&Uart3_RxData, 1);
        }
    }
```

