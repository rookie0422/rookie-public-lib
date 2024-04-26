#include "coroutine.h"
#include "cmsis_os.h"
#include "dyn_mem.h"
#include "vmsgexec.h"
#include <stdbool.h>


#if   defined ( __CC_ARM )
  #include "cmsis_armcc.h"
/*
 * Arm Compiler 6 (armclang)
 */
#elif defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
  #include "cmsis_armclang.h"
#endif


#define VWM_RUNTASK (VWM_USER+0x100)

//开启的通用线程函数
static void vtask_run_thread (void const *argument);

//事件处理函数
static void vtask__wincallback(vwm_message_t *pMsg);

//开启任务时发送的任务消息给线程
typedef struct {
	vtask_function_t 	task_function_p;
	void*				sender;
	void*				params;
	bool				if_auto_free_params;
}vwm_runtask_message_t;

//线程定义
osThreadDef(vtask_run_thread, osPriorityNormal, TASK_MAX_COUNT, 0);	// thread object

//线程对象的封装
typedef struct vthread_info_t {
	osThreadId 			thread_id; //线程ID
	volatile uint8_t	is_running;//是否正在运行任务

	//以下为动态生成邮箱所需要的数据，参照 osMailQDef 宏定义
	//----------------------------------------------------------------
	//邮箱需要的定义开始
	//----------------------------------------------------------------
	uint32_t *			os_mailQ_q;//4+(queue_sz)
	uint32_t *			os_mailQ_m;//[3+((sizeof(type)+3)/4)*(queue_sz)]; 
	void *   			os_mailQ_p[2];//{ (os_mailQ_q_##name), os_mailQ_m_##name }; 
	osMailQDef_t *		os_mailQ;

	osMailQId  			mailQ_id; // Mail queue ID
	//----------------------------------------------------------------
	//邮箱定义结束
	//----------------------------------------------------------------
}vthread_info_t;

osMutexDef (vtask_alloc_mutex);    // Declare mutex

typedef struct vtask_t {
	osMutexId task_alloc_mutex_id; // Mutex ID

	//vtask作为模块可以接收系统事件，主要用在中断中开启任务，需要发送开启任务的消息给本模块，在主线程中才可以真正开启任务
	vwm_win_handle_t win_handle;

	//运行任务的线程池
	vthread_info_t threads[TASK_MAX_COUNT];
} vtask_t;

vtask_t g_tasks = {0};

void vtask__init(void)
{
	uint8_t i;
	uint8_t queue_sz = 1;//只产生1个条目的消息队列

	g_tasks.task_alloc_mutex_id = osMutexCreate(osMutex(vtask_alloc_mutex));
	g_tasks.win_handle = msgexec_create_module(vtask__wincallback, 0);

	for (i = 0; i < TASK_MAX_COUNT; i++) {
		g_tasks.threads[i].os_mailQ_q = (uint32_t*)dm_alloc((4+queue_sz)*sizeof(uint32_t));
		g_tasks.threads[i].os_mailQ_m = (uint32_t*)dm_alloc((3+((sizeof(vwm_runtask_message_t)+3)/4)*(queue_sz))*sizeof(uint32_t));
		g_tasks.threads[i].os_mailQ_p[0] = g_tasks.threads[i].os_mailQ_q;
		g_tasks.threads[i].os_mailQ_p[1] = g_tasks.threads[i].os_mailQ_m;
		g_tasks.threads[i].os_mailQ = (osMailQDef_t *)dm_alloc(sizeof(osMailQDef_t));
		g_tasks.threads[i].os_mailQ->queue_sz = queue_sz;
		g_tasks.threads[i].os_mailQ->item_sz = sizeof(vwm_runtask_message_t);
		g_tasks.threads[i].os_mailQ->pool = g_tasks.threads[i].os_mailQ_p;

		g_tasks.threads[i].mailQ_id = osMailCreate(g_tasks.threads[i].os_mailQ, NULL);

		//生成线程，开启线程
		g_tasks.threads[i].thread_id = osThreadCreate(osThread(vtask_run_thread), (void*)i);		
	}
}

/**
 * @brief  开启一个任务，在单独的线程中运行
 * @note   
 * @param  task_function_p: 任务的函数指针
 * @param  sender: 函数依附的对象
 * @param  params: 函数参数，可以用结构体进行传入
 * @param  iffreeparams: 是否在执行完任务后，自动释放params指向的内容
 * @retval 0 加入运行队列，马上执行；1 没有可用的线程可用；2 在中断中调用，需要在主线程中处理; 3 其他错误
 */
static uint8_t vtask__run_task_raw(vtask_function_t task_function_p, void* sender, void* params, bool iffreeparams)
{
	uint8_t i;
	osStatus status;
	vwm_runtask_message_t *postmsg;

	//查找空闲线程
	for (i = 0; i <TASK_MAX_COUNT; i++) {
		if (g_tasks.threads[i].is_running == 0) {
			status = osMutexWait(g_tasks.task_alloc_mutex_id, 1000);

			if (status == osOK) {
				//得到了锁开始向这个线程发送消息，让它运行任务，并标记为忙
				g_tasks.threads[i].is_running = 1;
				osMutexRelease(g_tasks.task_alloc_mutex_id);

				postmsg = osMailCAlloc(g_tasks.threads[i].mailQ_id, 100);

				if (postmsg != NULL) {
					postmsg->task_function_p = task_function_p;
					postmsg->sender = sender;
					postmsg->params = params;
					postmsg->if_auto_free_params = iffreeparams;

					osMailPut(g_tasks.threads[i].mailQ_id, postmsg);
					return 0;
				} else {
					if (iffreeparams == true) {
						if (params != NULL) {
							dm_free(params);
						}
					}
					return 3;
				}				
			}
		}
	}

	if (iffreeparams == true) {
		if (params != NULL) {
			dm_free(params);
		}
	}

	return 1;

}

uint8_t vtask__run_task(vtask_function_t task_function_p, void* sender, void* params, bool iffreeparams)
{
	vwm_runtask_message_t *postmsg;

	if (task_function_p == NULL) {
		if (iffreeparams == true) {
			if (params != NULL) {
				dm_free(params);
				params = NULL;
			}
		}

		return 3;
	}
	
	//如果从中断中调用，则发送消息给窗口，在主线程进行处理
	if (__get_IPSR() != 0U) {
		postmsg = (vwm_runtask_message_t*)dm_alloc(sizeof(vwm_runtask_message_t));
		postmsg->task_function_p = task_function_p;
		postmsg->sender = sender;
		postmsg->params = params;
		postmsg->if_auto_free_params = iffreeparams;
		vwm_send_message_p(VWM_RUNTASK, g_tasks.win_handle, g_tasks.win_handle, postmsg, 1);
		return 2;
	} else {
		return vtask__run_task_raw(task_function_p, sender, params, iffreeparams);	
	}
}

static void vtask__wincallback(vwm_message_t *pmsg)
{	
	vwm_runtask_message_t *postmsg;

	switch (pmsg->msg_id)
	{
		case VWM_INIT_DIALOG: /* 初始化消息 */
			break;

		case VWM_RUNTASK://运行任务，如果在中断服务函数中调用了RunTask，则会发送此消息，并在主线程进行实际执行
			postmsg = (vwm_runtask_message_t *)pmsg->data.p;
			if (postmsg != NULL) {
				vtask__run_task_raw(postmsg->task_function_p, postmsg->sender, postmsg->params, postmsg->if_auto_free_params);
			}
			break;		
	}
}


static void vtask_run_thread (void const *argument)			// thread function
{
	uint8_t threadindex;
	vthread_info_t *pthread;

	vwm_runtask_message_t  *msg;
	osEvent  evt;
	osStatus status;
	
	threadindex = (uint8_t)argument;

	pthread = &g_tasks.threads[threadindex];

	while (1) {
		
		evt = osMailGet(pthread->mailQ_id, osWaitForever);        // wait for mail
		if (evt.status == osEventMail) {
			msg = evt.value.p;

			if (msg->task_function_p != NULL) {				
				msg->task_function_p(msg->sender, msg->params);
			}

			if (msg->if_auto_free_params == true) {
				if (msg->params != NULL) {
					dm_free(msg->params);
					msg->params = NULL;
				}
			}

			osMailFree(pthread->mailQ_id, msg);  // free memory allocated for mail

			//置线程状态为空闲，可以运行其他任务
			status = osMutexWait(g_tasks.task_alloc_mutex_id, 1000);
			//不管得不得到锁，直接修改，防止等待过长时间
			pthread->is_running = 0;

			if (status == osOK) {
				osMutexRelease(g_tasks.task_alloc_mutex_id);
			}			
		}
	}
}

