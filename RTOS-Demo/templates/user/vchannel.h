#ifndef VCHANNEL_HHH
#define VCHANNEL_HHH
//通道的C语言实现，封装了操作系统的Mail

#include <stdint.h>

#include "vmsgexec.h"
#include "dyn_mem.h"
#include "vlist.h"

typedef struct vchannel_t {
	//以下为动态生成邮箱所需要的数据，参照 osMailQDef 宏定义
	uint32_t *os_mailQ_q;//4+(queue_sz)
	uint32_t *os_mailQ_m;//[3+((sizeof(type)+3)/4)*(queue_sz)]; 
	void *   os_mailQ_p[2];//{ (os_mailQ_q_##name), os_mailQ_m_##name }; 
	osMailQDef_t *os_mailQ;

	osMailQId  mailQ_id; // Mail queue ID

	vlist_t*	waiting_msg_list;//等待消息的列表
	int			waiting_msg_id;//正在等待的消息
}vchannel_t;

vchannel_t * vchannel_new(uint16_t queue_sz);
void vchannel_free(vchannel_t *channel);

vwm_message_t * vchannel_alloc_message(vchannel_t* channel);
void vchannel_free_message(vchannel_t* channel, vwm_message_t *msg);

void vchanel_put_message(vchannel_t* channel, vwm_message_t *msg);
vwm_message_t* vchannel_send_message_no_para(vchannel_t* channel, int msg_id, vwm_win_handle_t hdestwin, vwm_win_handle_t hsrcwin);
vwm_message_t* vchannel_send_message(vchannel_t* channel, int msg_id, vwm_win_handle_t hdestwin, vwm_win_handle_t hsrcwin, int data, uint8_t auto_free_p);
vwm_message_t* vchannel_send_message_p(vchannel_t* channel, int msg_id, vwm_win_handle_t hdestWin, vwm_win_handle_t hsrcwin, void* data, uint8_t auto_free_p);

/**
 * @brief  清除在此通道上想等待的消息列表
 * @note   
 * @param  channel: 通道对象
 * @retval None
 */
void vchannel_waitlist_clear(vchannel_t* channel);

void vchannel_waitlist_add(vchannel_t* channel, int msgid);

/**
 * @brief  在此通道上执行等待多个消息的操作，消息ID已经通过vchannel_waitlist_add进行了添加
 * @note   
 * @param  channel: 通道对象
 * @param  waittimelong: 要等待的时长，单位ms，osWaitForever一直等待
 * @param  resultcode:返回的结果代码。0 成功 1 超时 2 错误
 * @retval None
 */
vwm_message_t* vchannel_wait_multi_messages(vchannel_t* channel, uint32_t waittimelong, uint8_t *resultcode);

/**
 * @brief  在此通道上等待单个消息
 * @note   
 * @param  channel: 通道对象
 * @param  msgid: 要等待的消息ID
 * @param  waittimelong: 要等待的时长，单位ms，osWaitForever一直等待
 * @param  resultcode:返回的结果代码。0 成功 1 超时 2 错误
 * @retval None
 */
vwm_message_t* vchannel_wait_message(vchannel_t* channel, int msgid, uint32_t waittimelong, uint8_t *resultcode);

/**
 * @brief  在此通道上执行消息处理
 * @note   
 * @param  channel: 通道对象
 * @param  msgexeccallbackfunc: 消息处理回调函数，不能为空
 * @param  waittimelong: 等待单个消息的时长，超过此时长则退出执行循环，单位ms，osWaitForever一直等待
 * @param  resultcode: 返回的结果代码。0 成功 1 超时 2 错误
 * @retval None
 */
void vchannel_execute_message(vchannel_t* channel, vwm_callback_t msgexeccallbackfunc, uint32_t waittimelong, uint8_t *resultcode);

#endif
