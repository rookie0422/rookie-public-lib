#include "vchannel.h"


vchannel_t * vchannel_new(uint16_t queue_sz)
{
	vchannel_t * result;
	result = (vchannel_t*)dm_alloc(sizeof(vchannel_t));

	result->os_mailQ_q = (uint32_t*)dm_alloc((4+queue_sz)*sizeof(uint32_t));
	result->os_mailQ_m = (uint32_t*)dm_alloc((3+((sizeof(vwm_message_t)+3)/4)*(queue_sz))*sizeof(uint32_t));
	result->os_mailQ_p[0] = result->os_mailQ_q;
	result->os_mailQ_p[1] = result->os_mailQ_m;
	result->os_mailQ = (osMailQDef_t *)dm_alloc(sizeof(osMailQDef_t));
	result->os_mailQ->queue_sz = queue_sz;
	result->os_mailQ->item_sz = sizeof(vwm_message_t);
	result->os_mailQ->pool = result->os_mailQ_p;

	result->mailQ_id = osMailCreate(result->os_mailQ, NULL);

	//产生等待列表对象
	result->waiting_msg_list = vlist__create();

	return result;
}

void vchannel_free(vchannel_t *channel)
{
	if (channel == NULL) {
		return;
	}
	
	dm_free(channel->os_mailQ);
	dm_free(channel->os_mailQ_m);
	dm_free(channel->os_mailQ_q);
	dm_free(channel);
}

vwm_message_t * vchannel_alloc_message(vchannel_t* channel)
{	
	if (channel == NULL) {
		return NULL;
	}

	return osMailCAlloc(channel->mailQ_id, 0);       // Allocate memory
}

void vchannel_put_message(vchannel_t* channel, vwm_message_t *msg)
{
	if (channel == NULL) {
		return;
	}

	if (msg != NULL)
		osMailPut(channel->mailQ_id, msg);                         // Send Mail
}

vwm_message_t* vchannel_send_message_no_para(vchannel_t* channel, int msg_id, vwm_win_handle_t hdestwin, vwm_win_handle_t hsrcwin)
{
	vwm_message_t *pmsg;

	if (channel == NULL) {
		return NULL;
	}

	//发送初始化消息
	pmsg = vchannel_alloc_message(channel);
	if (pmsg == NULL) {
		return NULL;
	}
	pmsg->msg_id = msg_id;
	pmsg->dest_win_handle = (vwm_win_handle_t)hdestwin;
	pmsg->src_win_handle = (vwm_win_handle_t)hsrcwin;
	pmsg->data.v = 0;
	pmsg->auto_free_p = 0;

	vchannel_put_message(channel, pmsg);
	return pmsg;
}
vwm_message_t* vchannel_send_message(vchannel_t* channel, int msg_id, vwm_win_handle_t hdestwin, vwm_win_handle_t hsrcwin, int data, uint8_t auto_free_p)
{
	vwm_message_t *pmsg;

	if (channel == NULL) {
		return NULL;
	}

	pmsg = vchannel_alloc_message(channel);
	if (pmsg == NULL) {
		return NULL;
	}		
	pmsg->msg_id = msg_id;
	pmsg->dest_win_handle = hdestwin;
	pmsg->src_win_handle = hsrcwin;		
	pmsg->auto_free_p = auto_free_p;//自动释放
	pmsg->data.v = data;
	vchannel_put_message(channel, pmsg);
	return pmsg;
}

vwm_message_t* vchannel_send_message_p(vchannel_t* channel, int msg_id, vwm_win_handle_t hdestWin, vwm_win_handle_t hsrcwin, void* data, uint8_t auto_free_p)
{
	vwm_message_t *pmsg;

	if (channel == NULL) {
		return NULL;
	}

	pmsg = vchannel_alloc_message(channel);
	if (pmsg == NULL) {
		return NULL;
	}		
	pmsg->msg_id = msg_id;
	pmsg->dest_win_handle = hdestWin;
	pmsg->src_win_handle = hsrcwin;		
	pmsg->auto_free_p = auto_free_p;//自动释放
	pmsg->data.p = data;
	vchannel_put_message(channel, pmsg);
	return pmsg;
}

void vchannel_free_message(vchannel_t* channel, vwm_message_t *msg)
{
	if (channel == NULL) {
		return;
	}

	if (msg != NULL) {
		//如果消息所携带的指针是需要自动释放的内存指针，则释放对应的内存
		if (msg->auto_free_p == 1) {
			if (msg->data.p != NULL) {
				dm_free(msg->data.p);
			}
		}
		osMailFree(channel->mailQ_id, msg);  // free memory allocated for mail	
	}
}


/**
 * @brief  清除在此通道上想等待的消息列表
 * @note   
 * @param  channel: 通道对象
 * @retval None
 */
void vchannel_waitlist_clear(vchannel_t* channel)
{
	if (channel == NULL) {
		return;
	}

	vlist__clear(channel->waiting_msg_list);
}

void vchannel_waitlist_add(vchannel_t* channel, int msgid)
{
	if (channel == NULL) {
		return;
	}

	if (vlist__search(channel->waiting_msg_list, (void*)msgid) == NULL) {
		vlist__append(channel->waiting_msg_list, (void*)msgid);
	}
}

/**
 * @brief  在此通道上执行等待多个消息的操作，消息ID已经通过vchannel_waitlist_add进行了添加
 * @note   
 * @param  channel: 通道对象
 * @param  waittimelong: 要等待的时长，单位ms，osWaitForever一直等待
 * @param  resultcode:返回的结果代码。0 成功 1 超时 2 错误
 * @retval None
 */
vwm_message_t* vchannel_wait_multi_messages(vchannel_t* channel, uint32_t waittimelong, uint8_t *resultcode)
{
	vwm_message_t  *msg;
	osEvent  evt;

	if (channel == NULL) {
		*resultcode = 2;
		return NULL;
	}

	while (1) {		
		evt = osMailGet(channel->mailQ_id, waittimelong);        // wait for mail
		if (evt.status == osEventMail) {
			msg = evt.value.p;

			if (msg != NULL) {
				if (vlist__search(channel->waiting_msg_list, (void*)(msg->msg_id)) != NULL) { 				
					*resultcode = 0;
					return msg;
				}
			}		
		} else if (evt.status == osEventTimeout) {
			*resultcode = 1;
			return NULL;
		} else {
			*resultcode = 2;
			return NULL;
		}
	}
}

/**
 * @brief  在此通道上等待单个消息
 * @note   
 * @param  channel: 通道对象
 * @param  msgid: 要等待的消息ID
 * @param  waittimelong: 要等待的时长，单位ms，osWaitForever一直等待
 * @param  resultcode:返回的结果代码。0 成功 1 超时 2 错误
 * @retval None
 */
vwm_message_t* vchannel_wait_message(vchannel_t* channel, int msgid, uint32_t waittimelong, uint8_t *resultcode)
{
	vwm_message_t  *msg;
	osEvent  evt;	

	if (channel == NULL) {
		*resultcode = 2;
		return NULL;
	}

	while (1) {		
		evt = osMailGet(channel->mailQ_id, waittimelong);        // wait for mail
		if (evt.status == osEventMail) {
			msg = evt.value.p;

			if (msg != NULL) {
				if (msg->msg_id == msgid) {
					*resultcode = 0;
					return msg;
				}
			}		
		} else if (evt.status == osEventTimeout) {
			*resultcode = 1;
			return NULL;
		} else {
			*resultcode = 2;
			return NULL;
		}
	}
}

/**
 * @brief  在此通道上执行消息处理
 * @note   
 * @param  channel: 通道对象
 * @param  msgexeccallbackfunc: 消息处理回调函数，不能为空
 * @param  waittimelong: 等待单个消息的时长，超过此时长则退出执行循环，单位ms，osWaitForever一直等待
 * @param  resultcode: 返回的结果代码。0 成功 1 超时 2 错误
 * @retval None
 */
void vchannel_execute_message(vchannel_t* channel, vwm_callback_t msgexeccallbackfunc, uint32_t waittimelong, uint8_t *resultcode)
{
	vwm_message_t  *msg;
	osEvent  evt;
	vwindow_t *pdestwnd;

	if (channel == NULL) {
		*resultcode = 2;
		return;
	}

	while (1) {		
		evt = osMailGet(channel->mailQ_id, waittimelong);        // wait for mail
		if (evt.status == osEventMail) {
			msg = evt.value.p;

			if (msgexeccallbackfunc != NULL) {
				msgexeccallbackfunc(msg);
			} else {
				if (msg->dest_win_handle != NULL) {
					pdestwnd = (vwindow_t *)msg->dest_win_handle;

					if (pdestwnd->wndProc != NULL) {
						pdestwnd->wndProc(msg);
					}
				}
			}
			
			//如果是按键消息，释放按键消息的内存
			if (msg->auto_free_p == 1) {
				if (msg->data.p != NULL) {
					dm_free(msg->data.p);
				}
			}
			
			osMailFree(channel->mailQ_id, msg);  // free memory allocated for mail

		}
		else if (evt.status == osEventTimeout) {
			*resultcode = 1;
			return;
		}
	}
}
