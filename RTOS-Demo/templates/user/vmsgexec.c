#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "dyn_mem.h"
#include "vmsgexec.h"
//#include "datastore.h"
#include "cmsis_os.h"

msgexec_t g_msg_exec;

osMailQDef(msgexec_queue, 64, vwm_message_t); // Declare mail queue
osMailQId msgexec_queue_id;                   // Mail queue ID

/**
* @brief 消息执行体的初始化
* @return 是否初始化成功，0 成功， 1 失败
*/
uint8_t msgexec_init(void)
{
    memset(&g_msg_exec, 0, sizeof(msgexec_t));
    msgexec_queue_id = osMailCreate(osMailQ(msgexec_queue), NULL);
    return 0;
}

/**
* @brief 执行消息处理，一般在消息处理线程中执行
* @return 0 执行成功 1 还有消息没有执行完毕
*/
uint8_t msgexec_exec(void)
{
    vwm_message_t *msg;
    osEvent evt;
    vwindow_t *pdestwnd;

    evt = osMailGet(msgexec_queue_id, osWaitForever); // wait for mail
    if (evt.status == osEventMail) {
        msg = evt.value.p;

        if (msg->dest_win_handle != NULL) {
            pdestwnd = (vwindow_t *)msg->dest_win_handle;

            if (pdestwnd->wndProc != NULL) {
                pdestwnd->wndProc(msg);
            }
        }

        //如果是按键消息，释放按键消息的内存
        if (msg->auto_free_p == 1) {
            if (msg->data.p != NULL) {
                dm_free(msg->data.p);
            }
        }

        osMailFree(msgexec_queue_id, msg); // free memory allocated for mail
    }

    return 0;
}

/**
* @brief 得到此模块在初始化时设置的用户数据
* @return 用户数据，确实为0
*/

uint32_t msgexec_get_userdata(vwm_win_handle_t dest_win_handle)
{
    vwindow_t *thiswnd;
    thiswnd = (vwindow_t *)dest_win_handle;
    if (dest_win_handle == NULL)
        return 0;

    return thiswnd->userdata;
}

/**
* @brief	创建一个能被接收消息的模块
* @param	callback 此模块的消息处理回调函数
* @param	userdata 用户传入的数据
* @return 	返回模块的句柄
*/

vwm_win_handle_t msgexec_create_module(vwm_callback_t callback, uint32_t userdata)
{
    vwindow_t *thiswnd = (vwindow_t *)dm_alloc(sizeof(vwindow_t));
    vwm_message_t *pmsg;

    if (thiswnd == NULL)
        return NULL;
    memset(thiswnd, 0, sizeof(vwindow_t));

    thiswnd->userdata = userdata;
    thiswnd->wndProc = callback;

    if (g_msg_exec.top_window == NULL) {
        g_msg_exec.top_window = thiswnd;
        g_msg_exec.bottom_window = thiswnd;
        thiswnd->nextWnd = NULL;
        thiswnd->preWnd = NULL;
    } else {
        g_msg_exec.top_window->nextWnd = thiswnd;
        thiswnd->preWnd = g_msg_exec.top_window;
        g_msg_exec.top_window = thiswnd;
        thiswnd->nextWnd = NULL;
    }

    //发送初始化消息
    pmsg = msgexec_alloc_message();
    if (pmsg == NULL) {
        msgexec_free_module((vwm_win_handle_t)thiswnd);
        return NULL;
    }
    pmsg->msg_id = VWM_INIT_DIALOG;
    pmsg->dest_win_handle = (vwm_win_handle_t)thiswnd;
    pmsg->src_win_handle = (vwm_win_handle_t)thiswnd;

    msgexec_put_message(pmsg);

    return (vwm_win_handle_t)thiswnd;
}

/**
* @brief	释放模块
* @param	hwin 要释放的模块的句柄
* @return 	无返回值
*/
void vwm_clear_wnd_timer(vwm_win_handle_t hwnd);

void msgexec_free_module(vwm_win_handle_t hwin)
{
    vwindow_t *thiswnd;
    thiswnd = (vwindow_t *)hwin;

    if (hwin == NULL)
        return;

    if (g_msg_exec.top_window == thiswnd || g_msg_exec.bottom_window == thiswnd) {
        if (g_msg_exec.top_window == thiswnd) {
            g_msg_exec.top_window = thiswnd->preWnd;
            if (thiswnd->preWnd != NULL)
                thiswnd->preWnd->nextWnd = NULL;
        }

        if (g_msg_exec.bottom_window == thiswnd) {
            g_msg_exec.bottom_window = thiswnd->nextWnd;
            if (thiswnd->nextWnd != NULL)
                thiswnd->nextWnd->preWnd = NULL;
        }
    } else {
        if (thiswnd->preWnd != NULL)
            thiswnd->preWnd->nextWnd = thiswnd->nextWnd;
        if (thiswnd->nextWnd != NULL)
            thiswnd->nextWnd->preWnd = thiswnd->preWnd;
    }

    //释放此窗口关联的定时器
    vwm_clear_wnd_timer(hwin);
    dm_free(thiswnd);
}

/**
* @brief 让消息处理器分配一个消息对象
* @return 正常返回分配好的消息对象指针，否则返回NULL
*/

vwm_message_t *msgexec_alloc_message(void)
{
    return osMailCAlloc(msgexec_queue_id, 0); // Allocate memory
}

void msgexec_put_message(vwm_message_t *msg)
{
    if (msg != NULL)
        osMailPut(msgexec_queue_id, msg); // Send Mail
}

/**
* @brief 返回顶层模块句柄
* @return 顶层模块句柄
*/

vwm_win_handle_t msgexec_get_topwindow()
{
    return (vwm_win_handle_t)g_msg_exec.top_window;
}

void vwm_default_proc(vwm_message_t *pMsg)
{
}

void vwm_send_message_no_para(int msg_id, vwm_win_handle_t hdestwin, vwm_win_handle_t hsrcwin)
{
    vwm_message_t *pmsg;
    //发送初始化消息
    pmsg = msgexec_alloc_message();
    if (pmsg == NULL) {
        return;
    }
    pmsg->msg_id = msg_id;
    pmsg->dest_win_handle = (vwm_win_handle_t)hdestwin;
    pmsg->src_win_handle = (vwm_win_handle_t)hsrcwin;
    pmsg->data.v = 0;
    pmsg->auto_free_p = 0;

    msgexec_put_message(pmsg);
}

void vwm_send_message(int msg_id, vwm_win_handle_t hdestwin, vwm_win_handle_t hsrcwin, int data, uint8_t auto_free_p)
{
    vwm_message_t *pmsg;
    pmsg = msgexec_alloc_message();
    if (pmsg == NULL) {
        return;
    }
    pmsg->msg_id = msg_id;
    pmsg->dest_win_handle = hdestwin;
    pmsg->src_win_handle = hsrcwin;
    pmsg->auto_free_p = auto_free_p; //自动释放
    pmsg->data.v = data;
    msgexec_put_message(pmsg);
}
void vwm_send_message_p(int msg_id, vwm_win_handle_t hdestwin, vwm_win_handle_t hsrcwin, void *data, uint8_t auto_free_p)
{
    vwm_message_t *pmsg;
    pmsg = msgexec_alloc_message();
    if (pmsg == NULL) {
        return;
    }
    pmsg->msg_id = msg_id;
    pmsg->dest_win_handle = hdestwin;
    pmsg->src_win_handle = hsrcwin;
    pmsg->auto_free_p = auto_free_p; //自动释放
    pmsg->data.p = data;
    msgexec_put_message(pmsg);
}

vtimer_t gVTimer;

uint8_t vwm_timer_init(void)
{
    memset(&gVTimer, 0, sizeof(vtimer_t));
    return 0;
}

void ostimer_callback(void const *arg) // prototypes for timer callback function
{
    vtimer_item_t *thistimer = (vtimer_item_t *)arg;
    vwm_message_t *pmsg;

    if (arg == NULL)
        return;

    if (thistimer->windowHandle != NULL) {
        //发送定时器消息给对应的模块
        pmsg = msgexec_alloc_message();
        if (pmsg == NULL) {
            return;
        }
        pmsg->msg_id = VWM_TIMER;
        pmsg->dest_win_handle = (vwm_win_handle_t)thistimer->windowHandle;
        pmsg->src_win_handle = (vwm_win_handle_t)thistimer->windowHandle;
        pmsg->data.p = thistimer;

        msgexec_put_message(pmsg);
    }
}

osTimerDef(vostimer, ostimer_callback);

/**
* @brief 创建一个窗口能够使用的定时器
* @param mode 0 单次触发，1周期触发
* @return 正常返回分配好的定时器句柄，否则返回NULL
*/

vwm_timer_handle_t vwm_create_timer(vwm_win_handle_t dest_win_handle, int UserId, uint32_t Period, int Mode)
{
    vtimer_item_t *thistimer = (vtimer_item_t *)dm_alloc(sizeof(vtimer_item_t));

    if (thistimer == NULL)
        return NULL;
    memset(thistimer, 0, sizeof(vtimer_item_t));

    thistimer->userData = UserId;
    thistimer->mode = Mode;
    thistimer->period = Period;
    thistimer->windowHandle = dest_win_handle;
    thistimer->osTimerDef.ptimer = ostimer_callback;
    thistimer->osTimerDef.timer = thistimer->os_timer_cb_internel;
    //	if (Mode == 0)
    //		thistimer->osTimerID = osTimerCreate (osTimer(vostimer), osTimerOnce, thistimer);
    //	else if (Mode == 1)
    //		thistimer->osTimerID = osTimerCreate (osTimer(vostimer), osTimerPeriodic, thistimer);
    if (Mode == 0)
        thistimer->osTimerID = osTimerCreate(&thistimer->osTimerDef, osTimerOnce, thistimer);
    else if (Mode == 1)
        thistimer->osTimerID = osTimerCreate(&thistimer->osTimerDef, osTimerPeriodic, thistimer);

    if (gVTimer.lastTimer == NULL) {
        gVTimer.lastTimer = thistimer;
        gVTimer.firstTimer = thistimer;
        thistimer->nextTimer = NULL;
        thistimer->preTimer = NULL;
    } else {
        gVTimer.lastTimer->nextTimer = thistimer;
        thistimer->preTimer = gVTimer.lastTimer;
        gVTimer.lastTimer = thistimer;
        thistimer->nextTimer = NULL;
    }

    return (vwm_win_handle_t)thistimer;
}

void vwm_delete_timer(vwm_timer_handle_t hTimer)
{
    vtimer_item_t *thistimer;
    thistimer = (vtimer_item_t *)hTimer;

    if (thistimer == NULL)
        return;

    osTimerDelete(thistimer->osTimerID);

    if (gVTimer.lastTimer == thistimer || gVTimer.firstTimer == thistimer) {
        if (gVTimer.lastTimer == thistimer) {
            gVTimer.lastTimer = thistimer->preTimer;
            if (thistimer->preTimer != NULL)
                thistimer->preTimer->nextTimer = NULL;
        }

        if (gVTimer.firstTimer == thistimer) {
            gVTimer.firstTimer = thistimer->nextTimer;
            if (thistimer->nextTimer != NULL)
                thistimer->nextTimer->preTimer = NULL;
        }
    } else {
        if (thistimer->preTimer != NULL)
            thistimer->preTimer->nextTimer = thistimer->nextTimer;
        if (thistimer->nextTimer != NULL)
            thistimer->nextTimer->preTimer = thistimer->preTimer;
    }

    dm_free(thistimer);
}

void vwm_restart_timer(vwm_timer_handle_t hTimer, int Period)
{
    vtimer_item_t *thistimer;
    thistimer = (vtimer_item_t *)hTimer;

    if (thistimer == NULL)
        return;

    osTimerStart(thistimer->osTimerID, Period);
}

void vmw_stop_timer(vwm_timer_handle_t hTimer)
{
    vtimer_item_t *thistimer;
    thistimer = (vtimer_item_t *)hTimer;

    if (thistimer == NULL)
        return;

    osTimerStop(thistimer->osTimerID);
}

void vwm_start_timer(vwm_timer_handle_t hTimer)
{
    vtimer_item_t *thistimer;
    thistimer = (vtimer_item_t *)hTimer;

    if (thistimer == NULL)
        return;

    osTimerStart(thistimer->osTimerID, thistimer->period);
}

vtimer_item_t *vwm_find_timer_by_wnd(vwm_win_handle_t hwnd)
{
    vtimer_item_t *nowitem;

    nowitem = gVTimer.firstTimer;

    while (nowitem != NULL) {
        if (nowitem->windowHandle == hwnd)
            return nowitem;
        else
            nowitem = nowitem->nextTimer;
    }

    return nowitem;
}

void vwm_clear_wnd_timer(vwm_win_handle_t hwnd)
{
    vtimer_item_t *nowitem;

    nowitem = vwm_find_timer_by_wnd(hwnd);

    while (nowitem != NULL) {
        vwm_delete_timer((vwm_timer_handle_t)nowitem);
        nowitem = vwm_find_timer_by_wnd(hwnd);
    }
}

//----------------------------------------------------
void vgui_send_key_msg(int Key, int Pressed)
{
    vwm_message_t *pmsg;

    VWM_KEY_INFO *keyinfo = (VWM_KEY_INFO *)dm_alloc(sizeof(VWM_KEY_INFO));
    keyinfo->Key = Key;
    keyinfo->PressedCnt = Pressed;

    pmsg = msgexec_alloc_message();
    if (pmsg == NULL) {
        return;
    }
    pmsg->msg_id = VWM_KEY;
    pmsg->dest_win_handle = msgexec_get_topwindow();
    pmsg->src_win_handle = pmsg->dest_win_handle;
    pmsg->data.p = keyinfo;

    msgexec_put_message(pmsg);
}
