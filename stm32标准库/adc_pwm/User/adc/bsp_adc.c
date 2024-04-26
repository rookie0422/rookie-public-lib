
#include "stm32f10x.h"                  // Device header

uint16_t AD_Value[4];

void AD_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
	//adc预分频
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	
	//GPIO  PC3
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; 
	// 设置光敏输入的引脚为模拟输入
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	
	
	ADC_RegularChannelConfig(ADC1 , ADC_Channel_13 , 1,ADC_SampleTime_55Cycles5);
//	ADC_RegularChannelConfig(ADC1 , ADC_Channel_15 , 2,ADC_SampleTime_55Cycles5);
	
	//初始化adc
	ADC_InitTypeDef ADC_InitStructure;
	//工作模式
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	//数据对齐
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	//选择外部触发来启动规则通道
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
		//连续
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	//扫描
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;

	//通道数目
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	
	ADC_Init(ADC1, &ADC_InitStructure);
	
	
	
	//DMA
	DMA_InitTypeDef DMA_InitStructure;
	//外设参数
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;//起始地址
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//数据宽度
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//是否自增
	//存储器参数
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)AD_Value;//起始地址
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;//数据宽度
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//是否自增
	
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//传输方向
	DMA_InitStructure.DMA_BufferSize = 2;//缓冲区大小，计数器
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;//传输模式，是否自动重装
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//硬件触发还是软件触发
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;//优先级

	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	
	
	DMA_Cmd(DMA1_Channel1,ENABLE);
	ADC_DMACmd(ADC1,ENABLE);
	ADC_Cmd(ADC1,ENABLE);
	
	//校准
	ADC_ResetCalibration(ADC1);
	while (ADC_GetResetCalibrationStatus(ADC1) == SET);
	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1) == SET);
	
	//开始ad
	ADC_SoftwareStartConvCmd(ADC1,ENABLE);
	
}




