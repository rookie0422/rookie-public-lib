#include "dht11.h"
#include "main.h"
#include "bsp.h"

#define DHT11_DQ_OUT  DHT11_IO_GPIO_Port
#define DHT11_DQ_IN   DHT11_IO_GPIO_Port
#define Delay_ms(x)   (HAL_Delay(x))

void DHT11_RST(void);
void DHT11_IO_OUT(void); 
void DHT11_IO_IN(void);
uint8_t DHT11_Check(void);
uint8_t DHT11_Read_Bit(void);
uint8_t DHT11_Read_Byte(void);

void Delay_us(uint16_t nus);


void DHT11_RST(void)
{
	DHT11_IO_OUT();     
	HAL_GPIO_WritePin(DHT11_DQ_OUT, DHT11_IO_Pin,GPIO_PIN_RESET);
	Delay_ms(20);
	HAL_GPIO_WritePin(DHT11_DQ_OUT, DHT11_IO_Pin,GPIO_PIN_SET);	
	Delay_us(30);
}

void DHT11_IO_OUT(void)                          //IO口状态切换为输出
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

   HAL_GPIO_WritePin(DHT11_IO_GPIO_Port, DHT11_IO_Pin, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = DHT11_IO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DHT11_IO_GPIO_Port, &GPIO_InitStruct);
}

void DHT11_IO_IN(void)                         //IO口状态切换为输入
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Pin = DHT11_IO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(DHT11_IO_GPIO_Port, &GPIO_InitStruct);
}

uint8_t DHT11_Check(void)                     //检查DHT11模块
{
	uint8_t retry = 0;
	DHT11_IO_IN();          //配置为输入
	
	while(HAL_GPIO_ReadPin(DHT11_DQ_IN, DHT11_IO_Pin) && retry<100)   //拉低40-80us
	{
		retry++;
		Delay_us(1);
	};
	if(retry >= 100)     //未检测到模块
		return 1;          //出错
	else
		retry = 0;
	
	while(!HAL_GPIO_ReadPin(DHT11_DQ_IN, DHT11_IO_Pin) && retry<100)  //拉高40 - 80us
	{
		retry++;
		Delay_us(1);
	};
	
	if(retry >= 100)     //未检测到模块
		return 1;

	return 0;           //正常
}

uint8_t DHT11_Read_Bit(void)
{
	uint8_t retry;
	
	while(HAL_GPIO_ReadPin(DHT11_DQ_IN, DHT11_IO_Pin) && retry < 100)   //等待变为低电平
	{
		retry++;
		Delay_us(1);
	}
	retry = 0;
	
	while(!HAL_GPIO_ReadPin(DHT11_DQ_IN, DHT11_IO_Pin) && retry < 100)   //等待变为高电平
	{
		retry++;
		Delay_us(1);
	}
	Delay_us(40);
	
	if(HAL_GPIO_ReadPin(DHT11_DQ_IN, DHT11_IO_Pin))
		return 1;       //高电平
	else
		return 0;       //低电平
}

uint8_t DHT11_Read_Byte(void)
{
	uint8_t i, dat;
	dat = 0;
	
	for(i  = 0; i < 8;i++)
	{
		dat <<= 1;
		dat |= DHT11_Read_Bit();
	}
	return dat;
}

uint8_t DHT11_Read_Data(uint8_t *temp,uint8_t *humi)
{
	uint8_t buf[5];
	uint8_t i;
	DHT11_RST();
	
	if(DHT11_Check() == 0)
	{
		for(i = 0; i < 5; i++)
		{
			buf[i] = DHT11_Read_Byte();
		}
		
		if((buf[0] + buf[1] + buf[2] + buf[3]) == buf[4])
		{
			*humi = buf[0];
			*temp = buf[2];
		}
	}
	else
		return 1;
	return 0;
}


/**
定时器设置中，需要将分频系数 Prescal设为总线频率MHz-1，如72MHz，则应该设为71
*/
void Delay_us(uint16_t nus)
{
//	__HAL_TIM_SetCounter(&htim4, 0); 

//	__HAL_TIM_ENABLE(&htim4);

//	while(__HAL_TIM_GetCounter(&htim4) < (  nus));

//	__HAL_TIM_DISABLE(&htim4);
	
	
	__HAL_TIM_SET_COUNTER(&htim4, 0);
	
	HAL_TIM_Base_Start(&htim4);
	
	while(__HAL_TIM_GET_COUNTER(&htim4) <= ( nus));
	
	HAL_TIM_Base_Stop(&htim4);
	
	
}

