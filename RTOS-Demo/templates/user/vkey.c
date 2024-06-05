#include "vkey.h"
#include "dyn_mem.h"
#include "vmsgexec.h"
#include <stdbool.h>
#include <string.h>

void key_dlgcallback(vwm_message_t *pmsg);

/**
 * @brief 扫描按键，每10ms调用一次，一般用窗口定时器实现，在窗口定时器处理中调用
 * @note
 * @retval None
 */
void key_scan__scan(void);

key_scan_t g_keyscan = { 0 };

/**
 * @brief  设置某个按键的具体信息，并将初始状态设置为未按下状态
 * @note
 * @param  *keyinfo:
 * @param  GPIO_Pin:
 * @param  keycode:
 * @retval None
 */
static void key_scan__init_key(key_scan_info_t *keyinfo, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint8_t keycode,
                               bool readlinestate, GPIO_PinState initstate, uint8_t key_reversed)
{
    keyinfo->GPIOx = GPIOx;
    keyinfo->GPIO_Pin = GPIO_Pin;
    keyinfo->key_code = keycode;

    keyinfo->key_xor_count = 0;
    keyinfo->key_reversed = key_reversed;
    if (readlinestate == true) {
        keyinfo->key_state = HAL_GPIO_ReadPin(GPIOx, GPIO_Pin); //读取一次电平状态
    } else {
        keyinfo->key_state = initstate; //GPIO_PIN_SET;//HAL_GPIO_ReadPin(GPIOx, GPIO_Pin);
    }
}

/**
 * @brief 初始化按键扫描
 * @note
 * @param  hkeywnd: 接受按键消息的窗口
 * @retval None
 */
void key_scan__init(const key_map_t *key_maps, uint8_t keycount, uint8_t key_antishake_count,
                    vkey_press_callback_t press_callback, void *rev_press_obj, vkey_release_callback_t release_callback,
                    void *rev_release_obj)
{
    uint8_t i;

    if (g_keyscan._keys != NULL) {
        dm_free(g_keyscan._keys);
        g_keyscan._keys = NULL;
    }

    memset(&g_keyscan, 0, sizeof(key_scan_t));

    g_keyscan.key_wnd = msgexec_create_module(key_dlgcallback, 0);
    g_keyscan.key_count = keycount;
    g_keyscan.key_antishake_count = key_antishake_count;

    g_keyscan._keys = dm_alloc(sizeof(key_scan_info_t) * keycount);

    g_keyscan.key_press_callback = press_callback;
    g_keyscan.rev_press_obj = rev_press_obj;
    g_keyscan.key_release_callback = release_callback;
    g_keyscan.rev_release_obj = rev_release_obj;
    for (i = 0; i < keycount; i++) {
        key_scan__init_key(&g_keyscan._keys[i], key_maps[i].GPIOx, key_maps[i].GPIO_Pin, key_maps[i].key_code, true,
                           GPIO_PIN_SET, key_maps[i].key_reversed);
    }
}

void key_dlgcallback(vwm_message_t *pmsg)
{
    switch (pmsg->msg_id) {
        case VWM_INIT_DIALOG:
            g_keyscan.key_scan_timer = vwm_create_timer(pmsg->dest_win_handle, 0, 10, 1);
            if (g_keyscan.key_scan_timer == NULL) {
                //log_e("app_init VWM_CreateTimer error!");
            } else {
                //log_i("app_init VWM_CreateTimer ok!");
                key_scan__scan_start();
            }

            break;

        case VWM_TIMER:
            if (pmsg->data.v == g_keyscan.key_scan_timer) {
                key_scan__scan();
            }

        default:
            break;
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

    for (i = 0; i < g_keyscan.key_count; i++) {
        keyinfo = &g_keyscan._keys[i];
        nowkeystate = HAL_GPIO_ReadPin(keyinfo->GPIOx, keyinfo->GPIO_Pin);
        if (nowkeystate != keyinfo->key_state) {
            keyinfo->key_xor_count++;
            if (keyinfo->key_xor_count >= g_keyscan.key_antishake_count) {
                keyinfo->key_state = nowkeystate;

                //发送按键消息
                if (nowkeystate == GPIO_PIN_RESET) {
                    if (keyinfo->key_reversed == 0) {
                        //按下
                        //vwm_send_message(VWM_KEY, g_keyscan.rev_key_wnd, g_keyscan.rev_key_wnd, keyinfo->key_code, 0);
                        if (g_keyscan.key_press_callback != NULL) {
                            g_keyscan.key_press_callback(g_keyscan.rev_press_obj, keyinfo->key_code);
                        }
                    } else {
                        //松开
                        //vwm_send_message(VWM_KEY, g_keyscan.rev_key_wnd, g_keyscan.rev_key_wnd, keyinfo->key_code, 0);
                        g_keyscan.key_release_callback(g_keyscan.rev_release_obj, keyinfo->key_code);
                    }
                } else {
                    if (keyinfo->key_reversed == 0) {
                        //松开
                        //vwm_send_message(VWM_KEY, g_keyscan.rev_key_wnd, g_keyscan.rev_key_wnd, keyinfo->key_code, 0);
                        g_keyscan.key_release_callback(g_keyscan.rev_press_obj, keyinfo->key_code);
                    } else {
                        //按下
                        //vwm_send_message(VWM_KEY, g_keyscan.rev_key_wnd, g_keyscan.rev_key_wnd, keyinfo->key_code, 0);
                        g_keyscan.key_press_callback(g_keyscan.rev_release_obj, keyinfo->key_code);
                    }
                }
            }
        } else {
            keyinfo->key_xor_count = 0;
        }
    }
}

void key_scan__scan_start()
{
    vwm_start_timer(g_keyscan.key_scan_timer);
}
void key_scan__scan_stop()
{
    vmw_stop_timer(g_keyscan.key_scan_timer);
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
    return g_keyscan._keys[keycode].key_state;
}
