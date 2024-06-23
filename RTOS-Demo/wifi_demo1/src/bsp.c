
#include "bsp.h"
#include "main.h"
#include <stdio.h>

//static uint32_t CpuID[3];
//void Get_ChipID(void)
//{
//	CpuID[0]=*(__IO uint32_t*)(0x1FFFF7AC);
//	CpuID[1]=*(__IO uint32_t*)(0x1FFFF7B0);
//	CpuID[2]=*(__IO uint32_t*)(0x1FFFF7B4);
//}

static volatile uint32_t gU32IdAdressVar; //这里一定要定义此变量，否则会被优化器优化掉

uint32_t bsp_getdeviceid(void)
{
    __IO uint32_t *u32IdAddress;
    uint32_t u32EorRslt = 0, u32AddRslt = 0;
#if 0
	//如果直接赋值0X1FFFF7E8,则程序编译结果里会有0X1FFFF7E8，这样破解人员会很轻松
	//的找到这个内容，然后非常容易进行修改，去掉软加密
	u32IdAddress = (uint32_t*)0x1ffff7e8;
#else
    //千万别显式的读取ID，即要把0X1FFFF7E8运算成隐式的，例如此例中0x1FFFF7E8 = (0x455873a * 4) + 0xEA9DB00;
    //这样，别人就算破解出了你的程序，也查找不到0X1FFFF7E8,这样就不能轻易的软解密，这样处理后如果要软解密，
    //一定要反汇编出来进行复杂逆向分析，难度极大，代价极高，很难搞定软加密了，达到保护产品的目的。
    gU32IdAdressVar = 0x455873a;
    gU32IdAdressVar <<= 2;                                    //0x11561CE8
    u32IdAddress = (uint32_t *)(gU32IdAdressVar + 0xEA95D28); //0x1fff7A10
#endif
    //读取单片机的ID，并进行运算，具体算法可以自己定，这里只用到简单的异或及和运算

    //读取单片机的ID，并进行运算，具体算法可以自己定，这里只用到简单的异或及和运算
    u32EorRslt = (*u32IdAddress) ^ (*(u32IdAddress + 1)) ^ (*(u32IdAddress + 2));
    //u32AddRslt = (*u32IdAddress) + (*(u32IdAddress + 1)) + (*(u32IdAddress + 2));
    //u32AddRslt = W25QXX_ReadUNIID();
    //进行对比，如果运算结果与FLASH保存的结果不一样，说明非法，运行错误代码
    return u32AddRslt + u32EorRslt;
}

/*---------------------------串口接收中断--------------------------*/

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
