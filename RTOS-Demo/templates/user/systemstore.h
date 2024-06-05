#ifndef SYSTEMSTORE_HHH
#define SYSTEMSTORE_HHH


#ifdef OEMVERSION
#define FIRMWAREVERSIONMAJORMINJOR 0x62
#else
#define FIRMWAREVERSIONMAJORMINJOR 0x11

#endif
#define FIRMWAREVERSIONBUILD 0x23 //35
#include "stm32f0xx_hal.h"
#include <stdint.h>
/**
 * STM32F103VET 共512K字节Flash，2K为一页，计数值保存到最后一页，其他信息保存在倒数第2页
 * */
/**
 * GD32E230C8T6 共64K字节Flash，1K为一页，计数值保存到最后一页，其他信息保存在倒数第2页
 * */


#define DATASTORE_BASEADDR      (0x0800F000) //从0数第63页，倒数第1页



#pragma pack(1)
typedef struct systemstore_t {


	uint16_t 			continuous_speed;			//连续模式速度
	
	uint8_t				wire_back_length;       	//回抽长度
	uint8_t				wire_forward_length;    	//补丝长度	


	uint8_t				pulse_speed;				//脉冲模式速度

	uint16_t			pulse_period; 				//脉冲周期
	uint8_t				pulse_smoothness; 			//脉冲平滑度

	uint16_t			manual_back_speed; 			//手动回抽速度

	uint16_t			manual_forward_speed;  		//手动送丝速度

    //uint8_t             reserved[2]; 				//数组
	

} systemstore_t;
#pragma pack()





// flash 闪存
void systemstore__save();
static uint8_t systemstore__is_init();
void systemstore_load_default_config();
void systemstore__load();



#endif