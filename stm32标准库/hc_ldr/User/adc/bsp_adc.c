
#include "stm32f10x.h"                  // Device header

uint16_t AD_Value[4];

void AD_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
	//adcԤ��Ƶ
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	
	//GPIO  PC3
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_5; 
	// ���ù������������Ϊģ������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	
	
	ADC_RegularChannelConfig(ADC1 , ADC_Channel_13 , 1,ADC_SampleTime_55Cycles5);
	ADC_RegularChannelConfig(ADC1 , ADC_Channel_15 , 2,ADC_SampleTime_55Cycles5);
	
	//��ʼ��adc
	ADC_InitTypeDef ADC_InitStructure;
	//����ģʽ
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	//���ݶ���
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	//ѡ���ⲿ��������������ͨ��
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
		//����
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	//ɨ��
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;

	//ͨ����Ŀ
	ADC_InitStructure.ADC_NbrOfChannel = 2;
	
	ADC_Init(ADC1, &ADC_InitStructure);
	
	
	
	//DMA
	DMA_InitTypeDef DMA_InitStructure;
	//�������
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;//��ʼ��ַ
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//���ݿ��
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//�Ƿ�����
	//�洢������
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)AD_Value;//��ʼ��ַ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;//���ݿ��
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//�Ƿ�����
	
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//���䷽��
	DMA_InitStructure.DMA_BufferSize = 2;//��������С��������
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;//����ģʽ���Ƿ��Զ���װ
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//Ӳ�����������������
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;//���ȼ�

	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	
	
	DMA_Cmd(DMA1_Channel1,ENABLE);
	ADC_DMACmd(ADC1,ENABLE);
	ADC_Cmd(ADC1,ENABLE);
	
	//У׼
	ADC_ResetCalibration(ADC1);
	while (ADC_GetResetCalibrationStatus(ADC1) == SET);
	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1) == SET);
	
	//��ʼad
	ADC_SoftwareStartConvCmd(ADC1,ENABLE);
	
}




