#ifndef VMSGEXEC_H_
#define VMSGEXEC_H_
#include <stdint.h>
#include "cmsis_os.h"

typedef uint32_t VWM_HWIN;
typedef uint32_t VWM_HTIMER;

typedef struct {
  int MsgId;            /* type of message */
  VWM_HWIN 	hWin;         /* Destination window */
  VWM_HWIN 	hWinSrc;      /* Source window  */
  uint8_t	autoFreeP;///<当消息处理完毕的时候，是否要释放p所指向的内存，0 不释放，1释放
  union {
    void * p;     /* Message specific data pointer */
    int v;
    void (* pFunc)(void);
  } Data;
}VWM_MESSAGE;

//typedef void VWM_CALLBACK( VWM_MESSAGE * pMsg);

typedef void (*VWM_CALLBACK)(VWM_MESSAGE * pMsg);

/**
* @brief	创建一个能被接收消息的模块
* @param	callback 此模块的消息处理回调函数
* @param	userdata 用户传入的数据
* @return 	返回模块的句柄
*/

VWM_HWIN MsgExec_CreateModule(VWM_CALLBACK callback, uint32_t userdata);

/**
* @brief	释放模块
* @param	hwin 要释放的模块的句柄
* @return 	无返回值
*/
void MsgExec_FreeModule(VWM_HWIN hwin);


#define MAXHWINCOUNT 5
/**
	窗口管理采用动态分配的方式，窗口句柄就是窗口结构体分配时得到的内存地址
*/
typedef struct VWindow{
	struct VWindow *nextWnd;///<下一个窗口
	struct VWindow *preWnd;///<上一个窗口
	VWM_CALLBACK wndProc;///<消息循环
	uint32_t	userdata;///<用户的数据，在消息循环函数中
}VWindow;

typedef struct MsgExec{
	VWindow  *BottomWindow;///<窗口链表
	VWindow  *TopWindow;
}MsgExec;

/**
* @brief 消息执行体的初始化
* @return 是否初始化成功，0 成功， 1 失败
*/
uint8_t MsgExec_Init(void);

/**
* @brief 执行消息处理，一般在消息处理线程中执行
* @return 0 执行成功 1 还有消息没有执行完毕
*/
uint8_t MsgExec_Exec(void);

/**
* @brief 等待一定的时长
*/
//void MsgExec_Delay(uint32_t ms);

/**
* @brief 得到此模块在初始化时设置的用户数据
* @return 用户数据，确实为0
*/

uint32_t MsgExec_GetUserData(VWM_HWIN hwin);

/**
* @brief 返回顶层模块句柄
* @return 顶层模块句柄
*/

VWM_HWIN MsgExec_GetTopWindow(void);

/**
* @brief 让消息处理器分配一个消息对象
* @return 正常返回分配好的消息对象指针，否则返回NULL
*/

VWM_MESSAGE * MsgExec_AllocMessage(void);

void MsgExec_PutMessage(VWM_MESSAGE *msg);

void VWM_DefaultProc(VWM_MESSAGE * pMsg);
void VWM_SendMessageNoPara(int MsgId, VWM_HWIN hDestWin, VWM_HWIN hSrcWin);             /* not to be documented (may change in future */
void VWM_SendMessage(int MsgId, VWM_HWIN hDestWin, VWM_HWIN hWinSrc, int data, uint8_t autoFreeP);
void VWM_SendMessageP(int MsgId, VWM_HWIN hDestWin, VWM_HWIN hWinSrc, void* data, uint8_t autoFreeP);



typedef struct VTimerItem{
	struct VTimerItem *nextTimer;///<下一个定时器
	struct VTimerItem *preTimer;///<上一个定时器
	VWM_HWIN windowHandle;///<发送定时器消息的模块句柄
	uint8_t  mode;///<定时器模式，0 单次，1 循环
	int 	userData;///<用户自己的数据
	uint32_t period;
	uint32_t os_timer_cb_internel[6];///<定时器所需要内存
	osTimerDef_t osTimerDef;///<在操作系统中对应的定义数据结构
	osTimerId osTimerID; ///<在操作系统中对应的定时器ID
	uint32_t os_timer_cb_internel_rev[6];///<定时器所需要内存
}VTimerItem;

typedef struct VTimer {
	VTimerItem *firstTimer;
	VTimerItem *lastTimer;
}VTimer;

uint8_t VWM_TimerInit(void);

/**
* @brief 创建一个窗口能够使用的定时器
* @param mode 0 单次触发，1周期触发
* @return 正常返回分配好的定时器句柄，否则返回NULL
*/

VWM_HTIMER VWM_CreateTimer(VWM_HWIN hWin, int UserId, uint32_t Period, int Mode);

void VWM_DeleteTimer(VWM_HTIMER hTimer);

void VWM_RestartTimer(VWM_HTIMER hTimer, int Period);

void VWM_StopTimer(VWM_HTIMER hTimer);

void VWM_StartTimer(VWM_HTIMER hTimer);

//------------------------------------------------------------
typedef struct {
  int Key, PressedCnt;
} VWM_KEY_INFO;


void VGUI_SendKeyMsg (int Key, int Pressed);

#define GUI_KEY_UP                17
#define GUI_KEY_DOWN              19
#define GUI_KEY_ENTER             13        /* ASCII: ENTER     Crtl-M */
#define GUI_KEY_ESCAPE            27        /* ASCII: ESCAPE    0x1b   */



#define VWM_INIT_DIALOG 				29			/* Inform dialog that it is ready for init */
#define VWM_TIMER                    	0x0113  	/* Timer has expired              (Keep the same as WIN32) */
#define VWM_KEY                      	14      	/* Key has been pressed */
#define VWM_USER                     0x0400  /* Reserved for user messages ... (Keep the same as WIN32) */

#endif

