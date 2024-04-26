#include "bsp_dma.h"

uint16_t MyDMA_Size;


void MyDMA_Init(uint32_t AddrA,uint32_t AddrB,uint16_t Size)
{
	MyDMA_Size = Size;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
	
	
	DMA_InitTypeDef DMA_InitStructure;
	//外设参数
	DMA_InitStructure.DMA_PeripheralBaseAddr = AddrA;//起始地址
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//数据宽度
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Enable;//是否自增
	//存储器参数
	DMA_InitStructure.DMA_MemoryBaseAddr = AddrB;//起始地址
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//数据宽度
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//是否自增
	
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//传输方向
	DMA_InitStructure.DMA_BufferSize = Size;//缓冲区大小，计数器
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//传输模式，是否自动重装
	DMA_InitStructure.DMA_M2M = DMA_M2M_Enable;//硬件触发还是软件触发
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;//优先级

	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	
	DMA_Cmd(DMA1_Channel1,DISABLE);
}

void MyDMA_Trancfer(void)
{
	DMA_Cmd(DMA1_Channel1,DISABLE);
	DMA_SetCurrDataCounter(DMA1_Channel1,MyDMA_Size);
	DMA_Cmd(DMA1_Channel1,ENABLE);
	
	while (DMA_GetFlagStatus(DMA1_FLAG_TC1) == RESET);
	DMA_ClearFlag(DMA1_FLAG_TC1);
	
}

