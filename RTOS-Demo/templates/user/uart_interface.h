#ifndef UART_INTERFACE_H
#define UART_INTERFACE_H
#include <stdint.h>
#include <stdbool.h>
#include "bsp.h"
#include "vmsgexec.h"
#include "vconfig.h"

#define WRITE_USE_DMA 1

#if WRITE_USE_DMA == 1
#include "vlist.h"
#endif

#define UART_RECEIVE_MAXSIZE (255+16) //接收一帧数据的最长长度，在每次开始接收时分配，发送数据时采用动态分配

//------------------------------------------------------------------------
//UART串口对象实现开始
//------------------------------------------------------------------------

//消息引擎消息定义
#define VWM_UART_REV_DATA			(VWM_USER + 0x1001)
#define VWM_UART_SEND_DATA_REQ		(VWM_USER + 0x1002)
#define VWM_UART_SEND_DATA_COMPLETE	(VWM_USER + 0x1003)

//串口通信的帧头
#define RADIO_FRAME_HEAD 0x55
#define RADIO_FRAME_TAIL 0xAA

typedef enum revmsg_process_state_t {
	SEARCH_FRAMEHEAD,///< 正在寻找帧头
	SEARCH_FRAMETAIL,///< 已寻找到帧头, 正在寻找帧尾
	SEARCH_FRAMETAIL_MODBUS,///< 已寻找到帧头, 正在寻找帧尾,Modbus模式
} revmsg_process_state_t;

#pragma pack(1)
typedef struct {
	struct uart_interface_t *uartif;
	uint16_t payload_length;
	uint8_t  payload[1];
}uart_revmsg_t;
#pragma pack()


typedef struct uart_rev_process_t {
    volatile revmsg_process_state_t     uart_revmsg_state;///< 串口接收数据状态
    volatile uart_revmsg_t*			    uart_revmsg;///<正在接收的串口数据
    volatile uint8_t				    uart_effective_data[UART_RECEIVE_MAXSIZE];	
	volatile uint16_t					uart_effective_write_index;///< 数据接收长度
    volatile uint8_t					uart_prekey;
}uart_rev_process_t;

typedef void (*uart_receive_effective_frame_callback_t)(void* revobj, uint8_t *revbuff, uint16_t revlen);

typedef enum {
	DMASEND_IDLE,   //空闲，没有发送数据
	DMASEND_SENDING //正在发送
}uart_dma_send_state_t;

//发送数据完成的回调函数原型
//status 0 成功发送 1 超时  2 错误
typedef void (*uart_send_data_complete_t)(void *sendobj, uint8_t status);
 
typedef struct uart_interface_t {
    UART_HandleTypeDef *				huart;///<使用的单片机UART接口对象
    uart_rev_process_t *				rev_process;//接收状态对象
	vwm_win_handle_t					rev_hwin;//接收串口消息的窗口句柄，自己创建的模块句柄

	void*								rev_obj;//回调函数的接收对象指针
	uart_receive_effective_frame_callback_t  receive_effective_frame_callback;//接收完整一帧数据后的回调函数

	#if WRITE_USE_DMA == 1	
	vlist_t*							send_buff_list;//发送数据的缓存	
	uart_dma_send_state_t				dma_send_state;//DMA发送状态
	#endif
	
	uint32_t							rev_all_count;
	uint16_t							rev_last_length;
	bool								is_485;//是否是485
	bool								is_use_custom_frame;//是否使用自定义帧结构，帧头5555，帧尾AAAA
	GPIO_TypeDef* 						REDE_GPIO_Port;//是485的话，这里设置控制读写的端口
	uint16_t							REDE_Pin;//是485的话，这里设置控制读写的管脚
}uart_interface_t;


uart_revmsg_t * uart_interface__alloc_rev_uartmsg(uint16_t payloadlength);

void uart_interface__init(uart_interface_t *uartif, 
	UART_HandleTypeDef* huart, 	
	bool is_485, 
	bool is_use_custom_frame, //是否使用自定义帧结构
	GPIO_TypeDef* REDE_GPIO_Port, 
	uint16_t REDE_Pin);


uint8_t uart_interface__start_receive(uart_interface_t *uartif, void* rev_obj, uart_receive_effective_frame_callback_t receive_effective_frame_callback);

void uart_interface__stop_receive(uart_interface_t *uartif);

/**
 * @brief 处理接口从串口发来的数据，在串口中断中被调用
 * 
 * @param iscomplete 是否已经完成接收
 * @param 
 * @retval void
 */
void uart_interface__rev_data(uart_interface_t *uartif, bool iscomplete);

/**
 * @brief  通过串口发送数据
 * @note   
 * @param  *uartif: 串口接口
 * @param  psend: 要发送的数据
 * @param  data_len: 要发送的数据的长度
 * @param sendmode:发送模式 0 同步发送 1 DMA发送
 * @param send_complete_callback:发送数据完毕的回调函数，只在DMA模式有效
 * @retval 0 成功 1 正在发送 3 内存不够了， 其他HAL_UART_Transmit返回的状态值
 */
uint8_t uart_interface__send_data(uart_interface_t *uartif,  uint8_t* psend, uint16_t data_len, uint8_t sendmode, void *sendobj, uart_send_data_complete_t send_complete_callback);

/**
 * @brief  发送一帧数据
 * @note   自动增加帧头和帧尾
 * @param  *uartif: 串口接口
 * @param  psend: 要发送的数据
 * @param  data_len: 要发送的数据的长度
 * @param sendmode:发送模式 0 同步发送 1 DMA发送
 * @param send_complete_callback:发送数据完毕的回调函数，只在DMA模式有效
 * @retval 0 成功 1 正在发送 其他HAL_UART_Transmit返回的状态值
 */
uint8_t uart_interface__send_one_frame(uart_interface_t *uartif,  uint8_t* psend, uint16_t data_len, 
	uint8_t sendmode, void *sendobj, uart_send_data_complete_t send_complete_callback);

void uart_interface__free(uart_interface_t *uartif);

//--------------------------------------------------------------------------------
//需要在外部调用的如下所示：
#if 0
void HAL_UART_IDLECallBack(UART_HandleTypeDef *huart)
{
	//RS485-2
	if (huart == &huart2) {
		uart_interface__rev_data(&g_input->uart_rs485_2_interface, false);
	} else if (huart == &huart1) {
		uart_interface__rev_data(&g_input->uart_pc_interface, false);
	} else if (huart == &huart4) {
		uart_interface__rev_data(&g_input->uart_4g_interface, false);
	}

}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart == &huart2) {
		uart_interface__rev_data(&g_input->uart_rs485_2_interface, true);
	}
	else if (huart == &huart1) {
		uart_interface__rev_data(&g_input->uart_pc_interface, true);
	} else if (huart == &huart4) {
		uart_interface__rev_data(&g_input->uart_4g_interface, true);
	}
}

#endif

#endif
