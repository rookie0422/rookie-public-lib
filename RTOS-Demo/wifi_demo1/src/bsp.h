#ifndef BSP_HHHH
#define BSP_HHHH

#include "stm32f1xx_hal.h"
#include "main.h"
#include "cmsis_os.h"
#include "main.h"
#include "vmsgexec.h"

#define FIRMWAREVERSIONMAJORMINJOR 0x11
#define FIRMWAREVERSIONBUILD 0x01 //1

//#define DEVICE_REMOTE //远程模块

typedef union{
	unsigned short	s;
	unsigned char	c[2];
}UShortChar;

typedef union{
	signed int i;
	signed short s[2];
	signed char c[4];
}SIntShortChar;

typedef union{
	unsigned int i;
	unsigned short s[2];
	unsigned char c[4];
}UIntShortChar;

typedef union {
	float f;
	uint16_t u16arr[2];
	uint8_t u8arr[4];
}UInt16Float;


//-- Enumerations -------------------------------------------------------------

#define	Blue	0x001f
#define	Yellow	0xffe0
#define	Green	0x07e0
#define	Black	0x0000
#define	White	0xffff
#define	Red		0xf800

#define	FEED_DOG()	IWDG->KR = 0x0000AAAA
//----------------------------------------------------------------------------
extern volatile uint32_t 	os_time;

//void HAL_UART_IDLECallBack(UART_HandleTypeDef *huart);
uint32_t bsp_getdeviceid(void);




#if 0
void led_status_on(void);
void led_status_off(void);
void led_status_toggle(void);

void led_UART1_on(void);
void led_UART1_off(void);

void led_UART4_on(void);
void led_UART4_off(void);

void beep_on(void);
void beep_off(void);
#endif
//----------------------------------------------------------------
//芯片外设声明
//----------------------------------------------------------------

extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim4;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;


#endif





