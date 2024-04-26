#ifndef VKEY_HHH
#define VKEY_HHH
#include "vconfig.h"
#include "vmsgexec.h"
#include <stdint.h>
//----------------------------------------------------------------
// 按键配置开始
//----------------------------------------------------------------

typedef struct key_map_t {
    GPIO_TypeDef *GPIOx; // 按键的GPIO端口
    uint16_t GPIO_Pin;   // 按键的GPIO管脚
    uint16_t key_code; // 按键值，检测到按键变化后发送的按键值，在初始化时设置
    uint8_t key_reversed; // 按键是否反转
} key_map_t;

//----------------------------------------------------------------
// 按键配置结束
//----------------------------------------------------------------

/**
 * @brief 初始化按键扫描
 * @note
 * @param  hkeywnd: 接受按键消息的窗口
 * @retval None
 */
void key_scan__init(vwm_win_handle_t hkeywnd, const key_map_t *key_maps,
                    uint8_t keycount, uint8_t key_antishake_count);

/**
 * @brief 扫描按键，每10ms调用一次，一般用窗口定时器实现，在窗口定时器处理中调用
 * @note
 * @retval None
 */
void key_scan__scan(void);

/**
 * @brief  根据按键值返回此按键现在的状态
 * @note
 * @param  keycode: 要查询的按键
 * @retval GPIO_PinState 键的状态
 */

 GPIO_PinState key_scan__getkeystate(uint16_t keycode);
#endif
