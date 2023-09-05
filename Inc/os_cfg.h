
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "stdbool.h"

/*----------功能裁剪----------*/

/*启用参数检查。*/
#define OS_PARAM_CHECK_EN   1
/*启用栈溢出检查。*/
#define OS_STACK_CHECK_EN   1
/*启用定时器。*/
#define OS_TIMER_EN         1

/*启用同优先级多任务。*/
#define OS_PRIO_MULTI_TASK  0
/*启用任务名称。*/
#define OS_TASK_NAME_EN     1
/*启用任务信号量。*/
#define OS_TASK_SEM_EN      1
/*启用任务时间片轮转。*/
#define OS_TASK_SLICE_EN    0

#if OS_TASK_SEM_EN
/*启用任务信号量名称。*/
#define OS_TASK_SEM_NAME_EN 1
#endif

/*启用事件机制。*/
#define OS_EVENT_EN         1



#if OS_EVENT_EN
/*启用事件名。*/
#define OS_EVENT_NAME_EN    1
/*启用优先级天花板。*/
#define OS_MUTEX_EN         1
#endif


/*启用定时器名称。*/
#define OS_TIMER_NAME_EN    1


/*启用主线程，启用主线程是众多其他功能的前提条件。*/
#define OS_MAIN_TASK_EN     1


/*启用操作系统管理动态内存。*/
#define OS_MEM_EN           0

#if OS_MAIN_TASK_EN

/*启用中断中事件延迟发布，开启后事件处理将不会关闭中断，中断中的事件发布将会被延迟。*/
#define OS_INTR_DELAY_EN    1
#endif

#if !OS_MEM_EN
/*newlib最小实现。*/
#define OS_NEWLIB_EN            1
#endif

/*----------数值设定----------*/
/*支持的最大优先级数量，建议设定为4~64。*/
#define OS_MAX_PRIO             16
/*预分配任务控制块数量。*/
#define OS_TASK_PRE_ALLOC_CNT   16


/*超时检查时间间隔。*/
#define OS_CHECK_TIMEOUT_FREQ   20

#if OS_TASK_SLICE_EN
/*同级任务时间片长度。*/
#define OS_TASK_SLICE           4
#endif

#if OS_EVENT_EN
/*预分配事件控制块数量。*/
#define OS_EVENT_PRE_ALLOC_CNT  32
#endif

/*空闲任务栈大小。*/
#define OS_IDLE_TASK_STACK_SIZE     128

#if OS_MAIN_TASK_EN
/*预分配主线程回调对象数量。*/
#define OS_MAIN_TASK_CALLBACK_CNT   64
/*主线程的栈大小*/
#define OS_MAIN_TASK_STACK_SIZE     4096
#endif

#if OS_MEM_EN
/*动态内存分配的最小块大小，单位Byte。*/
#define OS_MEM_PER_BLOCK    64
/*动态内存层级。*/
#define OS_MEM_LEVEL        10
/*动态内存总大小。*/
#define OS_MEM_TOTAL_COST   (OS_MEM_PER_BLOCK << OS_MEM_LEVEL)
#else
/*动态内存总大小。*/
#define OS_MEM_TOTAL_COST   (80 << 10)
#endif
/*每秒钟操作系统触发SysTick的次数。*/
#define OS_TICK_PER_SEC     1000

#if OS_NEWLIB_EN
/*设备描述符数量。*/
#define OS_DEVICE_CNT       16
/*文件描述符数量。*/
#define OS_FD_CNT           16
#endif
#ifdef __cplusplus
}
#endif