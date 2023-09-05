#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "os_def.h"


/**
 * @brief 获取操作系统自启动以来的Tick数。
 * @return 操作系统自启动以来的Tick数，实际意义由OS_TICK_PER_SEC决定。
 * @see OS_TICK_PER_SEC。
*/
OS_Tick OS_Get_Tick_Count(void);
/**
    * @brief
*/
void OS_Tick_Inc(void);
/**
    * @brief 阻塞当前任务delay个Tick。
    * @param delay 阻塞时长。
*/
void OS_Delay(OS_Tick delay);

#if OS_TIMER_EN
/**
 * @brief 定时器种类。
*/
typedef enum _OS_Timer_OPT {
    /*单次定时器*/
    OS_TIMER_OPT_ONE_SHOT = 0,
    /*循环定时器*/
    OS_TIMER_OPT_PERIODIC
}OS_Timer_OPT;


/**
 * @brief 定时器状态。
*/
typedef enum _OS_Timer_Status {
    /**
     * @brief 就绪。
    */
    OS_TIMER_STATUS_READY = 0,
    /**
     * @brief 运行中。
    */
    OS_TIMER_STATUS_RUNNING,
    /**
     * @brief 暂停。
    */
    OS_TIMER_STATUS_SUSPEND,
    /**
     * @brief 执行完成。
    */
    OS_TIMER_STATUS_FINISHED,
    /**
     * @brief 已销毁。
    */
    OS_TIMER_STATUS_RELEASED
}OS_Timer_Status;


/**
 * @brief 定时器控制块。
*/
typedef struct _OS_TMR {
    /**
     * @brief 定时器种类。
    */
    OS_Timer_OPT opt;
    /**
     * @brief 定时器启动延迟。
    */
    OS_Tick delay;
    /**
     * @brief 定时器周期。
    */
    OS_Tick period;
    /**
     * @brief 定时器回调函数。
    */
    OS_Callback_Func_t func;
    /**
     * @brief 定时器回调函数参数。
    */
    OS_Callback_Arg_t arg;
    /**
     * @brief 定时器状态。
    */
    OS_Timer_Status status;
#if OS_TIMER_NAME_EN
    /**
     * @brief 定时器名称。
    */
    const char* name;
#endif
    /**
     * @brief 距离下一次触发的时间。
    */
    OS_Tick _rest;
    /**
     * @brief 资源链表节点。任何被释放的定时器都会利用此节点将自己挂载到OS_Timer_Free_List上，等待再次分配,任何已分配，未释放的定时器都会利用此节点将自己挂载到OS_Timer_List上。
     * @see OS_Timer_Free_List
    */
    OS_List_Node _resource_node;
    /**
     * @brief 触发优先队列节点。任何处于运行状态的定时器都会将自己挂载到一个优先队列上，按顺序等待触发。
    */
    OS_Priority_Queue_Node  _pq_node;
}OS_TMR,*OS_Timer;


/**
 * @brief 初始化一个定时器。
 * @param timer 定时器指针。
 * @param name 定时器名称，传入NULL时为"NULL"。
 * @param opt 定时器类型。
 * @param delay 距第一次触发的延时。
 * @param period 两次触发的间隔。
 * @param func 定时器回调函数。
 * @param arg 定时器回调函数参数。
*/
void OS_Timer_Init(OS_Timer timer, const char* name, OS_Timer_OPT opt, OS_Tick delay, OS_Tick period, OS_Callback_Func_t func, OS_Callback_Arg_t arg);
/**
 * @brief 创建一个定时器。
 * @param name 定时器名称，传入NULL时为"NULL"。
 * @param opt 定时器类型。
 * @param delay 距第一次触发的延时。
 * @param period 两次触发的间隔。
 * @param func 定时器回调函数。
 * @param arg 定时器回调函数参数。
 * @return 创建成功则返回定时器指针，创建失败则返回NULL。
*/
OS_Timer OS_Timer_Create(const char* name, OS_Timer_OPT opt, OS_Tick delay, OS_Tick period, OS_Callback_Func_t func, OS_Callback_Arg_t arg);

/**
 * @brief 销毁一个定时器。如果处于中断中且开启了中断延迟发布，则会将此操作发送到主线程。
 * @param timer 需要销毁的定时器。
 * @return OS_OK，如果成功销毁定时器。
*/
OS_Error_t OS_Timer_Release(OS_Timer timer);
/**
 * @brief 所有未销毁的定时器的链表。
*/
extern OS_List OS_Timer_List;
/**
 * @brief 所有已销毁的定时器的链表。
*/
extern OS_List OS_Timer_Free_List;

/**
 * @brief 启动一个定时器，或者重启一个被暂停的定时器。如果处于中断中且开启了中断延迟发布，则会将此操作发送到主线程。
 * @param timer 需要启动的定时器。
 * @return 若成功启动或定时器已经启动则返回OS_OK；若定时器已被销毁则返回OS_ERROR。
*/
OS_Error_t OS_Timer_Start(OS_Timer timer);
/**
 * @brief 停止一个正在运行的定时器。如果处于中断中且开启了中断延迟发布，则会将此操作发送到主线程。
 * @param timer 需要停止的定时器。
 * @return 若成功停止或定时器已被停止则返回OS_OK；若定时器已被销毁则返回OS_ERROR。
*/
OS_Error_t OS_Timer_Stop(OS_Timer timer);
/**
 * @brief 暂停一个正在运行的定时器，并记录距离下一次触发的时间。如果处于中断中且开启了中断延迟发布，则会将此操作发送到主线程。
 * @param timer 需要暂停的定时器。
 * @return 若成功暂停定时器，或定时器已暂停，或定时器已运行结束，则返回OS_OK；若定时器已被销毁，则返回OS_ERROR。
*/
OS_Error_t OS_Timer_Suspend(OS_Timer timer);
#if OS_TIMER_NAME_EN
/**
 * @brief 通过定时器名称查找对应的定时器。
 * @param name 需要查找的定时器的名称。
 * @return 若多个定时器名称相同，则返回第一个找到的定时器；若没有找到对应的定时器，则返回NULL。
*/
OS_Timer OS_Timer_Find(const char* name);
#endif
#endif
#ifdef __cplusplus
}
#endif