#include "vkey.h"
#include "dyn_mem.h"
#include <stdbool.h>
#include <string.h>

//按键扫描的信息体
typedef struct key_scan_info_t {
	GPIO_TypeDef* 	GPIOx;//按键的GPIO端口
	uint16_t 		GPIO_Pin;//按键的GPIO管脚
	uint16_t 		key_code; //按键值，检测到按键变化后发送的按键值，在初始化时设置
	uint8_t			key_reversed;//按键是否反转

	GPIO_PinState 	key_state; //按键状态，GPIO_PIN_RESET GPIO_PIN_SET
	
	uint8_t 		key_xor_count; //按键状态变化计数，但连续变化超过KEY_ANSHAKE_COUNT就认为状态发生了变化
} key_scan_info_t;

typedef struct key_scan_t {
	vwm_win_handle_t 		rev_key_wnd;//接受按键消息的窗口句柄
	uint8_t					key_count;//按键个数
	uint8_t					key_antishake_count;//按键去抖时长，为扫描周期的倍数，默认3，30ms
	key_scan_info_t 		*keys;
} key_scan_t;
key_scan_t g_keyscan = {0};

/**
 * @brief  设置某个按键的具体信息，并将初始状态设置为未按下状态
 * @note
 * @param  *keyinfo:
 * @param  GPIO_Pin:
 * @param  keycode:
 * @retval None
 */
static void key_scan__init_key(key_scan_info_t *keyinfo, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint8_t keycode, bool readlinestate, GPIO_PinState initstate, uint8_t key_reversed)
{
	keyinfo->GPIOx = GPIOx;
	keyinfo->GPIO_Pin = GPIO_Pin;
	keyinfo->key_code = keycode;

	keyinfo->key_xor_count = 0;
	keyinfo->key_reversed = key_reversed;
	if (readlinestate == true) {
		keyinfo->key_state = HAL_GPIO_ReadPin(GPIOx, GPIO_Pin); //读取一次电平状态
	}
	else {
		keyinfo->key_state = initstate;//GPIO_PIN_SET;//HAL_GPIO_ReadPin(GPIOx, GPIO_Pin);
	}
}

/**
 * @brief 初始化按键扫描
 * @note
 * @param  hkeywnd: 接受按键消息的窗口
 * @retval None
 */
void key_scan__init(vwm_win_handle_t hkeywnd, const key_map_t *key_maps, uint8_t keycount, uint8_t key_antishake_count)
{
	uint8_t i;

	if (g_keyscan.keys != NULL) {
		dm_free(g_keyscan.keys);
		g_keyscan.keys = NULL;
	}

	memset(&g_keyscan, 0, sizeof(key_scan_t));

	g_keyscan.rev_key_wnd = hkeywnd;
	g_keyscan.key_count = keycount;
	g_keyscan.key_antishake_count = key_antishake_count;

	g_keyscan.keys = dm_alloc(sizeof(key_scan_info_t) * keycount);

	for (i = 0; i < keycount; i++) {
		key_scan__init_key(&g_keyscan.keys[i], key_maps[i].GPIOx, key_maps[i].GPIO_Pin, key_maps[i].key_code, true, GPIO_PIN_SET,  key_maps[i].key_reversed);
	}
	
}

/**
 * @brief  扫描按键，每10ms调用一次，一般用窗口定时器实现，在窗口定时器处理中调用
 * @note
 * @retval None
 */
void key_scan__scan()
{
	uint8_t i;
	key_scan_info_t *keyinfo;
	GPIO_PinState nowkeystate;

	for (i = 0; i <g_keyscan.key_count; i++) {
		keyinfo = &g_keyscan.keys[i];
		nowkeystate = HAL_GPIO_ReadPin(keyinfo->GPIOx, keyinfo->GPIO_Pin);
		if (nowkeystate != keyinfo->key_state) {
			keyinfo->key_xor_count++;
			if (keyinfo->key_xor_count >= g_keyscan.key_antishake_count) {
				keyinfo->key_state = nowkeystate;

				//发送按键消息
				if (nowkeystate == GPIO_PIN_RESET) {
					if (keyinfo->key_reversed == 0) {
						//按下
						vwm_send_message(VWM_KEY, g_keyscan.rev_key_wnd, g_keyscan.rev_key_wnd, keyinfo->key_code, 0);
					} else {
						//松开
						vwm_send_message(VWM_KEY, g_keyscan.rev_key_wnd, g_keyscan.rev_key_wnd, keyinfo->key_code, 0);
					}
				}
				else {
					if (keyinfo->key_reversed == 0) {
						//松开
						vwm_send_message(VWM_KEY, g_keyscan.rev_key_wnd, g_keyscan.rev_key_wnd, keyinfo->key_code, 0);
					} else {
						//按下
						vwm_send_message(VWM_KEY, g_keyscan.rev_key_wnd, g_keyscan.rev_key_wnd, keyinfo->key_code, 0);
					}
				}
			}
		}
		else {
			keyinfo->key_xor_count = 0;
		}
	}
}



/**
 * @brief  根据按键值返回此按键现在的状态
 * @note
 * @param  keycode: 要查询的按键
 * @retval GPIO_PinState 键的状态
 */
GPIO_PinState key_scan__getkeystate(uint16_t keycode)
{
	//keycode = keycode - KEY_FIRST_CODE;
	return g_keyscan.keys[keycode].key_state;
}
