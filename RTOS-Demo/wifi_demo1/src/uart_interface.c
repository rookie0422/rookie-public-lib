#include "uart_interface.h"
#include "dyn_mem.h"
#include <string.h>
#include "vmsgexec.h"

//----------------------------------------------------------------
//类型定义开始
//----------------------------------------------------------------
#if WRITE_USE_DMA == 1

//DMA发送的消息结构体定义
typedef struct uart_dma_send_item_info_t {
	void*								send_obj;//调用发送函数的对象指针
	uart_send_data_complete_t			send_data_complete_callback;//发送数据完成的回调函数
	uint8_t*							send_buffer; //发送缓存，动态分配
	uint16_t							send_length; //发送的数据长度
} uart_dma_send_item_info_t;

#endif

static void uart_wincallback(vwm_message_t *pMsg);
static void uart_interface__handle_data_from_uart(uart_revmsg_t *revmsg);
static void uart_interface__transmit_complete_run(uart_interface_t *uartif);

void uart_interface__init(uart_interface_t *uartif,
                          UART_HandleTypeDef* huart,
                          bool is_485,
                          bool is_use_custom_frame, //是否使用自定义帧结构
                          GPIO_TypeDef* REDE_GPIO_Port,
                          uint16_t REDE_Pin)
{
	if (uartif == NULL)	return;

	memset(uartif, 0, sizeof(uart_interface_t));
	uartif->huart = huart;
	uartif->rev_process = (uart_rev_process_t*)dm_alloc(sizeof(uart_rev_process_t));

	uartif->rev_hwin = msgexec_create_module(uart_wincallback, (uint32_t)uartif);  // 创建主窗口

	uartif->receive_effective_frame_callback = NULL;

	#if WRITE_USE_DMA == 1
	uartif->send_buff_list = vlist__create();
	#endif

	uartif->is_485 = is_485;
	uartif->REDE_GPIO_Port = REDE_GPIO_Port;
	uartif->REDE_Pin = REDE_Pin;
}


// UART模块的窗口回调函数
static void uart_wincallback(vwm_message_t *pmsg)
{	
	uart_interface_t *uartif;
	uart_dma_send_item_info_t *psenditem;
	void *tmpobj;
	uart_send_data_complete_t tmpsendcallback;


	switch (pmsg->msg_id) {
	case VWM_INIT_DIALOG:  // 模块初始化
		break;
	case VWM_UART_REV_DATA:
		uart_interface__handle_data_from_uart((uart_revmsg_t*)pmsg->data.p);
		break;
#if WRITE_USE_DMA == 1
	case VWM_UART_SEND_DATA_REQ:
		psenditem = (uart_dma_send_item_info_t *)pmsg->data.p;
		if (psenditem != NULL) {
			//将发送请求加入到发送队列中
			uartif = (uart_interface_t *)msgexec_get_userdata(pmsg->dest_win_handle);

			//如果没有正在发送，则开启发送
			if (uartif->dma_send_state == DMASEND_IDLE) { 
				if (HAL_UART_Transmit_DMA(uartif->huart, psenditem->send_buffer, psenditem->send_length) == HAL_OK) {
					uartif->dma_send_state = DMASEND_SENDING;
					//放入缓存，等发送完成了，完成回调了再释放资源
					vlist__append(uartif->send_buff_list, psenditem);
				} else {
					tmpsendcallback = psenditem->send_data_complete_callback;
					tmpobj = psenditem->send_obj;

					dm_free(psenditem->send_buffer);
					dm_free(psenditem);
					psenditem = NULL;

					if (tmpsendcallback != NULL) {						
						tmpsendcallback(tmpobj, 2);
					}
				}
			} else {
				//如果正在发送，只需要放入缓存即可
				vlist__append(uartif->send_buff_list, psenditem);
			}

		}
		break;

	case VWM_UART_SEND_DATA_COMPLETE: //数据通过DMA发送完成了
		uartif = (uart_interface_t *)pmsg->data.p;

		if (uartif != NULL) {
			uart_interface__transmit_complete_run(uartif);
		}
		break;
#endif

	default:
		break;
	}
}


/**
 * @brief 分配一个串口接收所使用的消息
 *
 * @param payloadlength 要分配的负载的长度
 * @return 分配好的消息对象
 */
uart_revmsg_t * uart_interface__alloc_rev_uartmsg(uint16_t payloadlength)
{
	uart_revmsg_t *rs;

	rs = (uart_revmsg_t*)dm_alloc(sizeof(uart_revmsg_t) - 1 + payloadlength);

	memset(rs, 0, sizeof(uart_revmsg_t) - 1 + payloadlength);

	return rs;
}



/**
 * @brief 处理接口从串口发来的数据
 *
 * @param iscomplete 是否已经完成接收
 * @param
 * @retval void
 */
void uart_interface__rev_data(uart_interface_t *uartif, bool iscomplete)
{
	uart_revmsg_t *tmp_uart_revmsg = NULL;
	uint16_t revsize;

	uartif->rev_all_count++;
	if (iscomplete == true) {
		revsize = 0;
	}
	else {
		revsize = __HAL_DMA_GET_COUNTER(uartif->huart->hdmarx);
		if (UART_RECEIVE_MAXSIZE == revsize) {
			//说明没有接收到任何数据，应该还在接收状态，直接返回，不在处理
			return;
		}
		//HAL_UART_AbortReceive_IT(uartif->huart);//调用这个函数终止接收F4的HAL库会有问题，会再次进入DMA停止中断，改为下面的语句
		HAL_UART_AbortReceive(uartif->huart);
	}


	if (uartif->rev_process->uart_revmsg != NULL) {
		tmp_uart_revmsg = (uart_revmsg_t *)uartif->rev_process->uart_revmsg;
		tmp_uart_revmsg->payload_length = UART_RECEIVE_MAXSIZE - revsize;
		uartif->rev_last_length = tmp_uart_revmsg->payload_length;
		uartif->rev_process->uart_revmsg = NULL;
	}


	uartif->rev_process->uart_revmsg = uart_interface__alloc_rev_uartmsg(UART_RECEIVE_MAXSIZE);

	if (uartif->rev_process->uart_revmsg != NULL) {
		HAL_UART_Receive_DMA(uartif->huart, (uint8_t*)uartif->rev_process->uart_revmsg->payload, UART_RECEIVE_MAXSIZE);
	}
	else {
		//return;
	}
	if (tmp_uart_revmsg != NULL) {
		tmp_uart_revmsg->uartif = uartif;
		if (uartif->rev_hwin != NULL) {

//		 printf("receive data=");
//		for(int i=0;i<tmp_uart_revmsg->payload_length;i++)
//		 {
//			printf("%02x ",(uint8_t)tmp_uart_revmsg->payload[i]);
//		 }
//		 printf("\r\n");
//

			vwm_send_message_p(VWM_UART_REV_DATA, uartif->rev_hwin, uartif->rev_hwin, tmp_uart_revmsg, 1);
		}
	}
}


/**
 * @brief  开始接收数据
 * @note
 * @param  *uartif: 串口对象
 * @param  receive_effective_frame_callback: 接收到完整帧后调用的函数指针
 * @retval 0 成功  1 正在接收数据，不能再次接收，请先停止接收后再次接收 2 回调函数为空 3 其他错误
 */
uint8_t uart_interface__start_receive(uart_interface_t *uartif, void* rev_obj, uart_receive_effective_frame_callback_t receive_effective_frame_callback)
{
	if (uartif->receive_effective_frame_callback != NULL) {
		return 1;
	}

	if (receive_effective_frame_callback == NULL) {
		return 2;
	}


	uartif->receive_effective_frame_callback = receive_effective_frame_callback;
	uartif->rev_obj = rev_obj;

	if (uartif->rev_process->uart_revmsg != NULL) {
		dm_free((void*)uartif->rev_process->uart_revmsg);
		uartif->rev_process->uart_revmsg = NULL;
	}
	memset(uartif->rev_process, 0, sizeof(uart_rev_process_t));
	uartif->rev_process->uart_revmsg = uart_interface__alloc_rev_uartmsg(UART_RECEIVE_MAXSIZE);

	__HAL_UART_CLEAR_IDLEFLAG(uartif->huart);
	__HAL_UART_ENABLE_IT(uartif->huart, UART_IT_IDLE);

	if (uartif->rev_process->uart_revmsg != NULL) {
		if (uartif->is_485) {
			uartif->REDE_GPIO_Port->BSRR = ((uint32_t)uartif->REDE_Pin << 16);
		}

		HAL_UART_Receive_DMA(uartif->huart, (uint8_t*)uartif->rev_process->uart_revmsg->payload, UART_RECEIVE_MAXSIZE);
		return 0;
	}
	else {
		return 3;
	}
}


void uart_interface__stop_receive(uart_interface_t *uartif)
{
	HAL_UART_AbortReceive(uartif->huart);
	uartif->receive_effective_frame_callback = NULL;
}


void uart_interface__handle_one_byte(uart_interface_t *uartif, uint8_t key)
{
	if(uartif->rev_process->uart_revmsg_state == SEARCH_FRAMEHEAD) {///< 正在查找帧头
		///< 找到帧头
		if(key == RADIO_FRAME_HEAD)
			if(uartif->rev_process->uart_prekey == RADIO_FRAME_HEAD) {
				uartif->rev_process->uart_effective_write_index = 0;
				uartif->rev_process->uart_revmsg_state = SEARCH_FRAMETAIL;
			}
	}
	else if(uartif->rev_process->uart_revmsg_state == SEARCH_FRAMETAIL) {
		uartif->rev_process->uart_effective_data[uartif->rev_process->uart_effective_write_index] = key;
		uartif->rev_process->uart_effective_write_index++;

		if (uartif->rev_process->uart_effective_write_index >= UART_RECEIVE_MAXSIZE) {
			uartif->rev_process->uart_revmsg_state = SEARCH_FRAMEHEAD;
		}
		else {
			///< 找到帧尾
			if(key == RADIO_FRAME_TAIL && uartif->rev_process->uart_prekey == RADIO_FRAME_TAIL) {
				if (uartif->receive_effective_frame_callback != NULL) {
					uartif->receive_effective_frame_callback(uartif->rev_obj, (uint8_t*)uartif->rev_process->uart_effective_data, uartif->rev_process->uart_effective_write_index);
				}
				uartif->rev_process->uart_revmsg_state = SEARCH_FRAMEHEAD;
			}
		}
	}

	uartif->rev_process->uart_prekey = key;
}

/**
 * @brief UART接口从串口接收到数据的处理，由接收串口消息的模块调用，在串口接收消息处理函数中调用
 *
 * @param revframe 从无线模块接收到的数据
 */
static void uart_interface__handle_data_from_uart(uart_revmsg_t *revmsg)
{
	uint16_t i;
	uart_interface_t *uartif;
	uartif = revmsg->uartif;


	if (uartif->is_use_custom_frame) {
		if (revmsg->payload_length > 1) {
			i = revmsg->payload_length;
		}
		for(i=0; i < revmsg->payload_length; i++) { ///< 不能从 0开始, 会造成越界
			uart_interface__handle_one_byte(uartif, revmsg->payload[i]);
		}

	}
	else {
		if (uartif->receive_effective_frame_callback != NULL) {
			//		printf("receive data = %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x \r\n",
			//		 revmsg->payload[0],revmsg->payload[1],revmsg->payload[2],revmsg->payload[3],revmsg->payload[4],revmsg->payload[5],
			//		 revmsg->payload[6],revmsg->payload[7],revmsg->payload[8],revmsg->payload[9],revmsg->payload[10],revmsg->payload[11],
			//		 revmsg->payload[12],revmsg->payload[13]);
			uartif->receive_effective_frame_callback(uartif->rev_obj, revmsg->payload, revmsg->payload_length);
		}
		//	uint16_t i = 0;
		//	uart_interface_t *uartif;
		//	uartif = revmsg->uartif;
		//	if (revmsg->payload_length > 1) {
		//		i = revmsg->payload_length;
		//	}
		//	for(i = 0; i < revmsg->payload_length; i++)///< 不能从 0开始, 会造成越界
		//	{
		//		uartif->handle_rec_one_byte_t(uartif, revmsg->payload[i]);
		//	}
	}
}

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
uint8_t uart_interface__send_data(uart_interface_t *uartif,  uint8_t* psend, uint16_t data_len, 
	uint8_t sendmode, void *sendobj, uart_send_data_complete_t send_complete_callback)
{
	HAL_StatusTypeDef result;
	uart_dma_send_item_info_t *psenditem;

	//将要发送的数据放入队列
	if (sendmode == 0) { //同步发送		
		if (uartif->is_485) {
			uartif->REDE_GPIO_Port->BSRR = (uint32_t)uartif->REDE_Pin;
			osDelay(1);
		}
		result = HAL_UART_Transmit(uartif->huart, psend, data_len, data_len*10);

		if (uartif->is_485) {
			uartif->REDE_GPIO_Port->BSRR = ((uint32_t)uartif->REDE_Pin << 16);
		}
	} else if (sendmode == 1) {   //DMA发送
		//不在这里进行处理，在消息处理函数中处理，因为，这个函数也可能在中断里调用
		psenditem = (uart_dma_send_item_info_t*)dm_alloc(sizeof(uart_dma_send_item_info_t));

		if (psenditem != NULL) {
			psenditem->send_obj = sendobj;
			psenditem->send_data_complete_callback = send_complete_callback;
			psenditem->send_length = data_len;
			psenditem->send_buffer = dm_alloc(data_len);

			if (psenditem->send_buffer == NULL) {
				result = 3;//没有内存了
				dm_free(psenditem);
				psenditem = NULL;
			}
			else {
				memcpy(psenditem->send_buffer, psend, data_len);

				//发送消息到串口窗口
				if (uartif->rev_hwin != NULL) {
					result = 0;
					vwm_send_message_p(VWM_UART_SEND_DATA_REQ, uartif->rev_hwin, uartif->rev_hwin, psenditem, 0);//不自动释放，在DMA发送完毕后释放					
				} else {
					dm_free(psenditem);
					psenditem = NULL;
					result = 5;
				}
			}
		} else {
			result = 3;//没有内存了
		}
	}

	if (result == HAL_OK) {
		return 0;
	} else {
		return result;
	}
}


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
                                       uint8_t sendmode, void *sendobj, uart_send_data_complete_t send_complete_callback)
{
	HAL_StatusTypeDef result;
	uint16_t frameheadtail;	
	uart_dma_send_item_info_t *psenditem;
	
	
	if (sendmode == 0) {
		if (uartif->is_485) {
			//uartif->REDE_GPIO_Port->BSRR = (uint32_t)uartif->REDE_Pin;
			HAL_GPIO_WritePin(uartif->REDE_GPIO_Port, uartif->REDE_Pin, GPIO_PIN_SET);
			osDelay(1);
		}

		//发送帧头
		frameheadtail = (RADIO_FRAME_HEAD << 8) + RADIO_FRAME_HEAD;
		result = HAL_UART_Transmit(uartif->huart, (uint8_t*)&frameheadtail, 2, 2*10);
		if (result != HAL_OK) {
			if (uartif->is_485) {
				HAL_GPIO_WritePin(uartif->REDE_GPIO_Port, uartif->REDE_Pin, GPIO_PIN_RESET);
			}
			return result;
		}


		result = HAL_UART_Transmit(uartif->huart, psend, data_len, data_len*10);
		if (result != HAL_OK) {
			if (uartif->is_485) {
				HAL_GPIO_WritePin(uartif->REDE_GPIO_Port, uartif->REDE_Pin, GPIO_PIN_RESET);
			}
			return result;
		}

		//发送帧尾
		frameheadtail = (RADIO_FRAME_TAIL << 8) + RADIO_FRAME_TAIL;
		result = HAL_UART_Transmit(uartif->huart, (uint8_t*)&frameheadtail, 2, 2*10);
		if (result != HAL_OK) {
			if (uartif->is_485) {
				uartif->REDE_GPIO_Port->BSRR = ((uint32_t)(uartif->REDE_Pin) << 16);
			}
			return result;
		}
	}
	else if (sendmode == 1) {   //DMA模式发送
		psenditem = (uart_dma_send_item_info_t*)dm_alloc(sizeof(uart_dma_send_item_info_t));

		if (psenditem != NULL) {
			psenditem->send_obj = sendobj;
			psenditem->send_data_complete_callback = send_complete_callback;
			psenditem->send_length = data_len + 4;
			psenditem->send_buffer = dm_alloc(data_len + 4);

			if (psenditem->send_buffer == NULL) {
				result = 3;//没有内存了
				dm_free(psenditem);
				psenditem = NULL;
			}
			else {
				psenditem->send_buffer[0] = RADIO_FRAME_HEAD;
				psenditem->send_buffer[1] = RADIO_FRAME_HEAD;
				memcpy(&psenditem->send_buffer[2], psend, data_len);
				psenditem->send_buffer[data_len+1] = RADIO_FRAME_TAIL;
				psenditem->send_buffer[data_len+2] = RADIO_FRAME_TAIL;				

				//发送消息到串口窗口
				if (uartif->rev_hwin != NULL) {
					result = 0;
					vwm_send_message_p(VWM_UART_SEND_DATA_REQ, uartif->rev_hwin, uartif->rev_hwin, psenditem, 0);//不自动释放，在DMA发送完毕后释放					
				} else {
					dm_free(psenditem);
					psenditem = NULL;
					result = 5;
				}
			}
		} else {
			result = 3;//没有内存了
		}		
	}

	if (result == HAL_OK) {
		return 0;
	}
	else {
		return result;
	}
}


/**
 * @brief 串口停止接收，释放串口对象的资源,但不是放串口对象自身
 *
 * @param uartif
 */
void uart_interface__free(uart_interface_t *uartif)
{
	if (uartif == NULL)
		return;

	// 关闭串口接收
	if (uartif->huart != NULL) {
		HAL_UART_AbortReceive(uartif->huart);
	}

	if (uartif->rev_process->uart_revmsg != NULL) {
		dm_free((uint8_t*)uartif->rev_process->uart_revmsg);
		uartif->rev_process->uart_revmsg = NULL;
	}

	if (uartif->rev_process!= NULL) {
		dm_free((uint8_t*)uartif->rev_process);
		uartif->rev_process = NULL;
	}

	#if WRITE_USE_DMA == 1
	if (uartif->send_buff_list != NULL) {
		vlist__clear(uartif->send_buff_list);
		dm_free(uartif->send_buff_list);
		uartif->send_buff_list = NULL;
	}
	#endif

}

#if WRITE_USE_DMA == 1	
void uart_interface__transmit_complete(uart_interface_t *uartif)
{
	//发送消息到串口窗口
	if (uartif->rev_hwin != NULL) {		
		vwm_send_message_p(VWM_UART_SEND_DATA_COMPLETE, uartif->rev_hwin, uartif->rev_hwin, uartif, 0);
	} else {
		//不可能出现
	}
}

//实际执行发送完成动作
static void uart_interface__transmit_complete_run(uart_interface_t *uartif)
{
	uart_dma_send_item_info_t *psenditem;
	void *tmpobj;
	uart_send_data_complete_t tmpsendcallback;

	if (uartif == NULL)
		return;

	if (uartif->send_buff_list != NULL) {
		//弹出最后一个
		psenditem = (uart_dma_send_item_info_t *)vlist__pop(uartif->send_buff_list);
		if (psenditem != NULL) {
			tmpsendcallback = psenditem->send_data_complete_callback;
			tmpobj = psenditem->send_obj;

			dm_free(psenditem->send_buffer);
			dm_free(psenditem);
			psenditem = NULL;

			if (tmpsendcallback != NULL) {						
				tmpsendcallback(tmpobj, 0);
			}			
		}

		//查看缓存是否还有未发送的数据
		while (1) {
			if (vlist__count(uartif->send_buff_list) > 0) {
				psenditem = (uart_dma_send_item_info_t *)vlist__virtual_pop(uartif->send_buff_list);
				if (psenditem == NULL) {
					//如果没有数据发送了，说明发送完了，设置空闲状态
					uartif->dma_send_state = DMASEND_IDLE;
					break;
				} else {
					//发送数据
					if (HAL_UART_Transmit_DMA(uartif->huart, psenditem->send_buffer, psenditem->send_length) == HAL_OK) {
						uartif->dma_send_state = DMASEND_SENDING;
						break;
					} else {
						//直接弹出来
						vlist__pop(uartif->send_buff_list);

						//释放资源
						tmpsendcallback = psenditem->send_data_complete_callback;
						tmpobj = psenditem->send_obj;

						dm_free(psenditem->send_buffer);
						dm_free(psenditem);
						psenditem = NULL;

						//回调结果函数
						if (tmpsendcallback != NULL) {						
							tmpsendcallback(tmpobj, 2);
						}

						continue;
					}
				}
			} else {
				//如果没有数据发送了，说明发送完了，设置空闲状态
				uartif->dma_send_state = DMASEND_IDLE;
				break;
			}
		}
		


	}
}
#endif

#if 0
//数据发送完毕的回调函数，在DMA模式下有用，请拷贝到BSP.c中
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart == &huart1) {
		uart_interface__transmit_complete(&g_input->uart_rs485_2_interface);
	}
}
#endif


