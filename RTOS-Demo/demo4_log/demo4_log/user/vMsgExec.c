#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "dyn_mem.h"
#include "vMsgExec.h"
//#include "datastore.h"
#include "cmsis_os.h"

MsgExec gMsgExec;

osMailQDef (MsgExecQ, 64, VWM_MESSAGE);  // Declare mail queue
osMailQId  MsgExecQID;                 // Mail queue ID


/**
* @brief 消息执行体的初始化
* @return 是否初始化成功，0 成功， 1 失败
*/
uint8_t MsgExec_Init(void)
{
	memset(&gMsgExec, 0, sizeof(MsgExec));
	MsgExecQID = osMailCreate(osMailQ(MsgExecQ), NULL);
	return 0;
}

/**
* @brief 执行消息处理，一般在消息处理线程中执行
* @return 0 执行成功 1 还有消息没有执行完毕
*/
uint8_t MsgExec_Exec(void)
{
	VWM_MESSAGE  *msg;
	osEvent  evt;
	VWindow *pdestwnd;

	evt = osMailGet(MsgExecQID, osWaitForever);        // wait for mail
	if (evt.status == osEventMail) {
		msg = evt.value.p;

		if (msg->hWin != NULL) {
			pdestwnd = (VWindow *)msg->hWin;

			if (pdestwnd->wndProc != NULL) {
				pdestwnd->wndProc(msg);
			}
		}
		
		//如果是按键消息，释放按键消息的内存
		if (msg->autoFreeP == 1) {
			if (msg->Data.p != NULL) {
				dm_free(msg->Data.p);
			}
		}
		
		osMailFree(MsgExecQID, msg);                    // free memory allocated for mail
	}
	
	return 0;
}

/**
* @brief 得到此模块在初始化时设置的用户数据
* @return 用户数据，确实为0
*/

uint32_t MsgExec_GetUserData(VWM_HWIN hwin)
{
	VWindow *thiswnd;
	thiswnd = (VWindow *)hwin;
	if (hwin == NULL)
		return 0;

	return thiswnd->userdata;
}


/**
* @brief	创建一个能被接收消息的模块
* @param	callback 此模块的消息处理回调函数
* @param	userdata 用户传入的数据
* @return 	返回模块的句柄
*/

VWM_HWIN MsgExec_CreateModule(VWM_CALLBACK callback, uint32_t userdata)
{
	VWindow *thiswnd = (VWindow *)dm_alloc(sizeof(VWindow));
	VWM_MESSAGE *pmsg;
	
	if (thiswnd == NULL)
		return NULL;
	memset(thiswnd, 0, sizeof(VWindow));
	
	thiswnd->userdata = userdata;
	thiswnd->wndProc = callback;
	
	if (gMsgExec.TopWindow == NULL) {
		gMsgExec.TopWindow = thiswnd;
		gMsgExec.BottomWindow = thiswnd;
		thiswnd->nextWnd = NULL;
		thiswnd->preWnd = NULL;
	}
	else {
		gMsgExec.TopWindow->nextWnd =  thiswnd;
		thiswnd->preWnd = gMsgExec.TopWindow;
		gMsgExec.TopWindow = thiswnd;
		thiswnd->nextWnd = NULL;
	}

	//发送初始化消息
	pmsg = MsgExec_AllocMessage();
	if (pmsg == NULL) {
		MsgExec_FreeModule((VWM_HWIN)thiswnd);
		return NULL;
	}
	pmsg->MsgId = VWM_INIT_DIALOG;
	pmsg->hWin = (VWM_HWIN)thiswnd;
	pmsg->hWinSrc = (VWM_HWIN)thiswnd;

	MsgExec_PutMessage(pmsg);
	
	return (VWM_HWIN)thiswnd;
}

/**
* @brief	释放模块
* @param	hwin 要释放的模块的句柄
* @return 	无返回值
*/
void VWM_ClearWndTimer(VWM_HWIN hwnd);

void MsgExec_FreeModule(VWM_HWIN hwin)
{
	VWindow *thiswnd;
	thiswnd = (VWindow *)hwin;

	if (hwin == NULL)
		return;

	if (gMsgExec.TopWindow == thiswnd || gMsgExec.BottomWindow == thiswnd) {
		if (gMsgExec.TopWindow == thiswnd) {
			gMsgExec.TopWindow =  thiswnd->preWnd;
			if (thiswnd->preWnd != NULL)
				thiswnd->preWnd->nextWnd = NULL;
		}

		if (gMsgExec.BottomWindow == thiswnd) {
			gMsgExec.BottomWindow = thiswnd->nextWnd;
			if (thiswnd->nextWnd != NULL)
				thiswnd->nextWnd->preWnd = NULL;
		}
	}
	else {
		if (thiswnd->preWnd != NULL)
			thiswnd->preWnd->nextWnd = thiswnd->nextWnd;
		if (thiswnd->nextWnd != NULL)
			thiswnd->nextWnd->preWnd = thiswnd->preWnd;
	}

	
	//释放此窗口关联的定时器
	VWM_ClearWndTimer(hwin);
	dm_free(thiswnd);
}


/**
* @brief 让消息处理器分配一个消息对象
* @return 正常返回分配好的消息对象指针，否则返回NULL
*/

VWM_MESSAGE * MsgExec_AllocMessage(void)
{
	return osMailCAlloc(MsgExecQID, 0);       // Allocate memory
}

void MsgExec_PutMessage(VWM_MESSAGE *msg)
{
	if (msg != NULL)
		osMailPut(MsgExecQID, msg);                         // Send Mail
}


/**
* @brief 返回顶层模块句柄
* @return 顶层模块句柄
*/

VWM_HWIN MsgExec_GetTopWindow()
{
	return (VWM_HWIN)gMsgExec.TopWindow;
}

void VWM_DefaultProc(VWM_MESSAGE * pMsg)
{
}

void VWM_SendMessageNoPara    (int MsgId, VWM_HWIN hDestWin, VWM_HWIN hSrcWin)
{
	VWM_MESSAGE *pmsg;
	//发送初始化消息
	pmsg = MsgExec_AllocMessage();
	if (pmsg == NULL) {
		return;
	}
	pmsg->MsgId = MsgId;
	pmsg->hWin = (VWM_HWIN)hDestWin;
	pmsg->hWinSrc = (VWM_HWIN)hSrcWin;
	pmsg->Data.v = 0;
	pmsg->autoFreeP = 0;

	MsgExec_PutMessage(pmsg);
} 

void VWM_SendMessage(int MsgId, VWM_HWIN hDestWin, VWM_HWIN hWinSrc, int data, uint8_t autoFreeP)
{
	VWM_MESSAGE *pmsg;
	pmsg = MsgExec_AllocMessage();
	if (pmsg == NULL) {
		return;
	}		
	pmsg->MsgId = MsgId;
	pmsg->hWin = hDestWin;
	pmsg->hWinSrc = hWinSrc;		
	pmsg->autoFreeP = autoFreeP;//自动释放
	pmsg->Data.v = data;
	MsgExec_PutMessage(pmsg);
}
void VWM_SendMessageP(int MsgId, VWM_HWIN hDestWin, VWM_HWIN hWinSrc, void* data, uint8_t autoFreeP)
{
	VWM_MESSAGE *pmsg;
	pmsg = MsgExec_AllocMessage();
	if (pmsg == NULL) {
		return;
	}		
	pmsg->MsgId = MsgId;
	pmsg->hWin = hDestWin;
	pmsg->hWinSrc = hWinSrc;		
	pmsg->autoFreeP = autoFreeP;//自动释放
	pmsg->Data.p = data;
	MsgExec_PutMessage(pmsg);
}

VTimer gVTimer;


uint8_t VWM_TimerInit(void)
{
	memset(&gVTimer, 0, sizeof(VTimer));
	return 0;
}

void osTimer_Callback  (void const *arg)                   // prototypes for timer callback function
{
	VTimerItem *thistimer = (VTimerItem *)arg;
	VWM_MESSAGE *pmsg;

	if (arg == NULL)
		return;

	if (thistimer->windowHandle != NULL) {
		//发送定时器消息给对应的模块
		pmsg = MsgExec_AllocMessage();
		if (pmsg == NULL) {
			return;
		}
		pmsg->MsgId = VWM_TIMER;
		pmsg->hWin = (VWM_HWIN)thistimer->windowHandle;
		pmsg->hWinSrc = (VWM_HWIN)thistimer->windowHandle;
		pmsg->Data.p = thistimer;

		MsgExec_PutMessage(pmsg);
	}
}


osTimerDef(vostimer, osTimer_Callback);

/**
* @brief 创建一个窗口能够使用的定时器
* @param mode 0 单次触发，1周期触发
* @return 正常返回分配好的定时器句柄，否则返回NULL
*/

VWM_HTIMER VWM_CreateTimer(VWM_HWIN hWin, int UserId, uint32_t Period, int Mode)
{
	VTimerItem *thistimer = (VTimerItem *)dm_alloc(sizeof(VTimerItem));

	if (thistimer == NULL)
		return NULL;
	memset(thistimer, 0, sizeof(VTimerItem));
	
	thistimer->userData = UserId;
	thistimer->mode = Mode;
	thistimer->period = Period;
	thistimer->windowHandle = hWin;
	thistimer->osTimerDef.ptimer = osTimer_Callback;
	thistimer->osTimerDef.timer = thistimer->os_timer_cb_internel;
//	if (Mode == 0)
//		thistimer->osTimerID = osTimerCreate (osTimer(vostimer), osTimerOnce, thistimer);
//	else if (Mode == 1)
//		thistimer->osTimerID = osTimerCreate (osTimer(vostimer), osTimerPeriodic, thistimer);
	if (Mode == 0)
		thistimer->osTimerID = osTimerCreate (&thistimer->osTimerDef, osTimerOnce, thistimer);
	else if (Mode == 1)
		thistimer->osTimerID = osTimerCreate (&thistimer->osTimerDef, osTimerPeriodic, thistimer);
	
	if (gVTimer.lastTimer == NULL) {
		gVTimer.lastTimer = thistimer;
		gVTimer.firstTimer = thistimer;
		thistimer->nextTimer = NULL;
		thistimer->preTimer = NULL;
	}
	else {
		gVTimer.lastTimer->nextTimer =  thistimer;
		thistimer->preTimer = gVTimer.lastTimer;
		gVTimer.lastTimer = thistimer;
		thistimer->nextTimer = NULL;
	}

	return (VWM_HWIN)thistimer;
}

void VWM_DeleteTimer(VWM_HTIMER hTimer)
{
	VTimerItem *thistimer;
	thistimer = (VTimerItem *)hTimer;

	if (thistimer == NULL)
		return;

	osTimerDelete(thistimer->osTimerID);
	
	if (gVTimer.lastTimer == thistimer || gVTimer.firstTimer == thistimer) {
		if (gVTimer.lastTimer == thistimer) {
			gVTimer.lastTimer =  thistimer->preTimer;
			if (thistimer->preTimer != NULL)
				thistimer->preTimer->nextTimer = NULL;
		}

		if (gVTimer.firstTimer == thistimer) {
			gVTimer.firstTimer = thistimer->nextTimer;
			if (thistimer->nextTimer != NULL)
				thistimer->nextTimer->preTimer = NULL;
		}
	}
	else {
		if (thistimer->preTimer != NULL)
			thistimer->preTimer->nextTimer = thistimer->nextTimer;
		if (thistimer->nextTimer != NULL)
			thistimer->nextTimer->preTimer = thistimer->preTimer;
	}

	dm_free(thistimer);	
}

void VWM_RestartTimer(VWM_HTIMER hTimer, int Period)
{
	VTimerItem *thistimer;
	thistimer = (VTimerItem *)hTimer;
	
	if (thistimer == NULL)
		return;

	osTimerStart(thistimer->osTimerID, Period);
}

void VWM_StopTimer(VWM_HTIMER hTimer)
{
	VTimerItem *thistimer;
	thistimer = (VTimerItem *)hTimer;
	
	if (thistimer == NULL)
		return;

	osTimerStop(thistimer->osTimerID);
}

void VWM_StartTimer(VWM_HTIMER hTimer)
{
	VTimerItem *thistimer;
	thistimer = (VTimerItem *)hTimer;
	
	if (thistimer == NULL)
		return;

	osTimerStart(thistimer->osTimerID, thistimer->period);
}

VTimerItem * VWM_FindTimerByWnd(VWM_HWIN hwnd)
{
	VTimerItem * nowitem;

	nowitem = gVTimer.firstTimer;

	while (nowitem != NULL) {
		if (nowitem->windowHandle == hwnd)
			return nowitem;
		else
			nowitem = nowitem->nextTimer;
	}
	
	return nowitem;
}

void VWM_ClearWndTimer(VWM_HWIN hwnd)
{
	VTimerItem * nowitem;

	nowitem = VWM_FindTimerByWnd(hwnd);

	while (nowitem != NULL) {
		VWM_DeleteTimer((VWM_HTIMER)nowitem);
		nowitem = VWM_FindTimerByWnd(hwnd);
	}
}


//----------------------------------------------------
void VGUI_SendKeyMsg (int Key, int Pressed)
{
	VWM_MESSAGE *pmsg;

	VWM_KEY_INFO * keyinfo = (VWM_KEY_INFO *)dm_alloc(sizeof(VWM_KEY_INFO));
	keyinfo->Key = Key;
	keyinfo->PressedCnt = Pressed;

	pmsg = MsgExec_AllocMessage();
	if (pmsg == NULL) {
		return;
	}
	pmsg->MsgId = VWM_KEY;
	pmsg->hWin = MsgExec_GetTopWindow();
	pmsg->hWinSrc = pmsg->hWin;
	pmsg->Data.p = keyinfo;

	MsgExec_PutMessage(pmsg);
}

