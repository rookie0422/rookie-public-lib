#ifndef VKEY_HHH
#define VKEY_HHH
#include "vconfig.h"
#include "vmsgexec.h"
#include <stdint.h>

typedef void (*vkey_press_callback_t)(void *rev_press_obj, uint16_t key_code);     //  按键按下时的回调函数
typedef void (*vkey_release_callback_t)(void *rev_release_obj, uint16_t key_code); // 按键松开时的回调函数

/*
void app_vkey_press_callback(void * rev_press_obj , uint16_t key_code);
void app_vkey_release_callback(void * rev_release_obj , uint16_t key_code);
*/

//----------------------------------------------------------------
// 按键配置开始
//----------------------------------------------------------------

typedef struct key_map_t {
    GPIO_TypeDef *GPIOx;  // 按键的GPIO端口
    uint16_t GPIO_Pin;    // 按键的GPIO管脚
    uint16_t key_code;    // 按键值，检测到按键变化后发送的按键值，在初始化时设置
    uint8_t key_reversed; // 按键是否反转
} key_map_t;

//按键扫描的信息体
typedef struct key_scan_info_t {
    GPIO_TypeDef *GPIOx;  //按键的GPIO端口
    uint16_t GPIO_Pin;    //按键的GPIO管脚
    uint16_t key_code;    //按键值，检测到按键变化后发送的按键值，在初始化时设置
    uint8_t key_reversed; //按键是否反转

    GPIO_PinState key_state; //按键状态，GPIO_PIN_RESET GPIO_PIN_SET

    uint8_t key_xor_count; //按键状态变化计数，但连续变化超过KEY_ANSHAKE_COUNT就认为状态发生了变化
} key_scan_info_t;

typedef struct key_scan_t {
    vwm_win_handle_t key_wnd;
    vwm_timer_handle_t key_scan_timer;
    uint8_t key_count;           //按键个数
    uint8_t key_antishake_count; //按键去抖时长，为扫描周期的倍数，默认3，30ms

    vkey_press_callback_t key_press_callback;
    vkey_release_callback_t key_release_callback;

    void *rev_press_obj;
    void *rev_release_obj;

    key_scan_info_t *_keys;
} key_scan_t;

extern key_scan_t g_keyscan;

//----------------------------------------------------------------
// 按键配置结束
//----------------------------------------------------------------

/**
 * @brief 初始化按键扫描
 * @note
 * @param  
 * @retval None
 */
void key_scan__init(const key_map_t *key_maps, uint8_t keycount, uint8_t key_antishake_count,
                    vkey_press_callback_t press_callback, void *rev_press_obj, vkey_release_callback_t release_callback,
                    void *rev_release_obj);

//开始扫描    初始化完成后 默认开始扫描
void key_scan__scan_start();

//停止按键扫描
void key_scan__scan_stop();

/**
 * @brief  根据按键值返回此按键现在的状态
 * @note
 * @param  keycode: 要查询的按键
 * @retval GPIO_PinState 键的状态
 */

GPIO_PinState key_scan__getkeystate(uint16_t keycode);
#endif
