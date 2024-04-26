#include "bsp_dma.h"

uint16_t MyDMA_Size;


void MyDMA_Init(uint32_t AddrA,uint32_t AddrB,uint16_t Size)
{
	MyDMA_Size = Size;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
	
	
	DMA_InitTypeDef DMA_InitStructure;
	//�������
	DMA_InitStructure.DMA_PeripheralBaseAddr = AddrA;//��ʼ��ַ
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//���ݿ��
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Enable;//�Ƿ�����
	//�洢������
	DMA_InitStructure.DMA_MemoryBaseAddr = AddrB;//��ʼ��ַ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//���ݿ��
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//�Ƿ�����
	
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//���䷽��
	DMA_InitStructure.DMA_BufferSize = Size;//��������С��������
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//����ģʽ���Ƿ��Զ���װ
	DMA_InitStructure.DMA_M2M = DMA_M2M_Enable;//Ӳ�����������������
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;//���ȼ�

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

