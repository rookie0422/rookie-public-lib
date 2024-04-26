#ifndef VMSGEXEC_H_
#define VMSGEXEC_H_
#include <stdint.h>
#include "cmsis_os.h"

typedef uint32_t vwm_win_handle_t;
typedef uint32_t vwm_timer_handle_t;

typedef struct {
  int 				msg_id;            /* type of message */
  vwm_win_handle_t 	dest_win_handle;         /* Destination window */
  vwm_win_handle_t 	src_win_handle;      /* Source window  */
  uint8_t			auto_free_p;///<当消息处理完毕的时候，是否要释放p所指向的内存，0 不释放，1释放
  union {
    void * p;     /* Message specific data pointer */
    int v;
    void (* p_func)(void);
  } data;
}vwm_message_t;

//typedef void vwm_callback_t( VWM_MESSAGE * pmsg);

typedef void (*vwm_callback_t)(vwm_message_t * pmsg);

/**
* @brief	创建一个能被接收消息的模块
* @param	callback 此模块的消息处理回调函数
* @param	userdata 用户传入的数据
* @return 	返回模块的句柄
*/

vwm_win_handle_t msgexec_create_module(vwm_callback_t callback, uint32_t userdata);

/**
* @brief	释放模块
* @param	dest_win_handle 要释放的模块的句柄
* @return 	无返回值
*/
void msgexec_free_module(vwm_win_handle_t dest_win_handle);


/**
	窗口管理采用动态分配的方式，窗口句柄就是窗口结构体分配时得到的内存地址
*/
typedef struct vwindow_t{
	struct vwindow_t *	nextWnd;	//下一个窗口
	struct vwindow_t *	preWnd;		//上一个窗口
	vwm_callback_t 		wndProc;	//消息循环
	uint32_t			userdata;	//用户的数据，在消息循环函数中
}vwindow_t;

typedef struct msgexec_t{
	vwindow_t  *bottom_window;//窗口链表
	vwindow_t  *top_window;
}msgexec_t;

/**
* @brief 消息执行体的初始化
* @return 是否初始化成功，0 成功， 1 失败
*/
uint8_t msgexec_init(void);

/**
* @brief 执行消息处理，一般在消息处理线程中执行
* @return 0 执行成功 1 还有消息没有执行完毕
*/
uint8_t msgexec_exec(void);

/**
* @brief 等待一定的时长
*/
//void MsgExec_Delay(uint32_t ms);

/**
* @brief 得到此模块在初始化时设置的用户数据
* @return 用户数据，确实为0
*/

uint32_t msgexec_get_userdata(vwm_win_handle_t dest_win_handle);

/**
* @brief 返回顶层模块句柄
* @return 顶层模块句柄
*/

vwm_win_handle_t msgexec_get_topwindow(void);

/**
* @brief 让消息处理器分配一个消息对象
* @return 正常返回分配好的消息对象指针，否则返回NULL
*/

vwm_message_t * msgexec_alloc_message(void);

void msgexec_put_message(vwm_message_t *msg);

void vwm_default_proc(vwm_message_t * pmsg);
void vwm_send_message_no_para(int msg_id, vwm_win_handle_t hdestwin, vwm_win_handle_t hsrcwin);             /* not to be documented (may change in future */
void vwm_send_message(int msg_id, vwm_win_handle_t hdestwin, vwm_win_handle_t hsrcwin, int data, uint8_t auto_free_p);
void vwm_send_message_p(int msg_id, vwm_win_handle_t hdestwin, vwm_win_handle_t hsrcwin, void* data, uint8_t auto_free_p);



typedef struct vtimer_item_t{
	struct vtimer_item_t *nextTimer;///<下一个定时器
	struct vtimer_item_t *preTimer;///<上一个定时器
	vwm_win_handle_t windowHandle;///<发送定时器消息的模块句柄
	uint8_t  mode;///<定时器模式，0 单次，1 循环
	int 	userData;///<用户自己的数据
	uint32_t period;
	uint32_t os_timer_cb_internel[6];///<定时器所需要内存
	osTimerDef_t osTimerDef;///<在操作系统中对应的定义数据结构
	osTimerId osTimerID; ///<在操作系统中对应的定时器ID
	uint32_t os_timer_cb_internel_rev[6];///<定时器所需要内存
}vtimer_item_t;

typedef struct vtimer_t {
	vtimer_item_t *firstTimer;
	vtimer_item_t *lastTimer;
}vtimer_t;

uint8_t vwm_timer_init(void);

/**
* @brief 创建一个窗口能够使用的定时器
* @param mode 0 单次触发，1周期触发
* @return 正常返回分配好的定时器句柄，否则返回NULL
*/

vwm_timer_handle_t vwm_create_timer(vwm_win_handle_t dest_win_handle, int UserId, uint32_t Period, int Mode);

void vwm_delete_timer(vwm_timer_handle_t hTimer);

void vwm_restart_timer(vwm_timer_handle_t hTimer, int Period);

void vmw_stop_timer(vwm_timer_handle_t hTimer);

void vwm_start_timer(vwm_timer_handle_t hTimer);

//------------------------------------------------------------
typedef struct {
  int Key, PressedCnt;
} VWM_KEY_INFO;


void vgui_send_key_msg (int Key, int Pressed);

#define GUI_KEY_UP                17
#define GUI_KEY_DOWN              19
#define GUI_KEY_ENTER             13        /* ASCII: ENTER     Crtl-M */
#define GUI_KEY_ESCAPE            27        /* ASCII: ESCAPE    0x1b   */



#define VWM_INIT_DIALOG 				29			/* Inform dialog that it is ready for init */
#define VWM_TIMER                    	0x0113  	/* Timer has expired              (Keep the same as WIN32) */
#define VWM_KEY                      	14      	/* Key has been pressed */
#define VWM_USER                     0x0400  /* Reserved for user messages ... (Keep the same as WIN32) */

#endif

