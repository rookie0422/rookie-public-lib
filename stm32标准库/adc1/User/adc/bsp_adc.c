
#include "stm32f10x.h"                  // Device header


void AD_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	//adcԤ��Ƶ
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	
	//GPIO  PC3
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_5; 
	// ���ù������������Ϊģ������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	
	
	
	//��ʼ��adc
	ADC_InitTypeDef ADC_InitStructure;
	//����ģʽ
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	//���ݶ���
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	//ѡ���ⲿ��������������ͨ��
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
		//����
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	//ɨ��
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;

	//ͨ����Ŀ
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	
	ADC_Init(ADC1, &ADC_InitStructure);
	
	
	ADC_Cmd(ADC1,ENABLE);
	
	//У׼
	ADC_ResetCalibration(ADC1);
	while (ADC_GetResetCalibrationStatus(ADC1) == SET);
	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1) == SET);
	
	//��ʼad
	
}

uint16_t AD_GetValue(uint8_t ADC_Channel)
{
	//����������ͨ��
	ADC_RegularChannelConfig(ADC1 , ADC_Channel , 1,ADC_SampleTime_55Cycles5);

	ADC_SoftwareStartConvCmd(ADC1,ENABLE);
	while(ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC) == RESET);
	return ADC_GetConversionValue(ADC1);
	
}



