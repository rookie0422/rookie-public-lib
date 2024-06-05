#ifndef COROUTINE_HHHH
#define COROUTINE_HHHH
#include <stdint.h>
#include <stdbool.h>

#define TASK_MAX_COUNT 5

typedef void (*vtask_function_t)(void * sender, void *params);


/**
 * @brief  任务系统初始化，在主函数中初始化
 * @note   
 * @retval None
 */
void vtask__init(void);


/**
 * @brief  开启一个任务，在单独的线程中运行
 * @note   
 * @param  task_function_p: 任务的函数指针
 * @param  sender: 函数依附的对象
 * @param  params: 函数参数，可以用结构体进行传入
 * @param  iffreeparams: 是否在执行完任务后，自动释放params指向的内容
 * @retval 0 加入运行队列，马上执行；1 没有可用的线程可用；2 在中断中调用，需要在主线程中处理; 3 其他错误
 */
uint8_t vtask__run_task(vtask_function_t task_function_p, void* sender, void* params, bool iffreeparams);

#endif
