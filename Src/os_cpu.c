#include "os_cpu.h"
#include "os_task.h"
#include "os_time.h"
#include "os_int.h"

#ifndef __weak
#define __weak __attribute__((weak))
#endif

__weak void User_SysTick_Handler(void) {
}



#if OS_MAIN_TASK_EN
typedef struct _OS_Callback_Obj {
    OS_Callback_Func_t func;
    OS_Callback_Src_t src;
    OS_Callback_Arg_t arg;
}OS_Callback_Obj;

static uint32_t modadd(uint32_t a, uint32_t b, uint32_t p) {
    return a + b < p ? a + b : a + b - p;
}

/*主线程回调函数对象队列。*/
static OS_Callback_Obj callback_obj_queue[OS_MAIN_TASK_CALLBACK_CNT];
static int callback_obj_queue_s, callback_obj_queue_size;

static OS_Task os_main_task;
static uint8_t os_main_task_stack[OS_MAIN_TASK_STACK_SIZE];

/**
 * @brief 主任务。
 * @param arg NULL。
*/
static void os_main_task_func(void* arg) {
    while (true) {
        OS_Prepare_Critical();
        OS_Enter_Critical();
        /*若队列不为空。*/
        while (callback_obj_queue_size) {
            /*取出回调函数对象。*/
            OS_Callback_Obj* obj = callback_obj_queue + callback_obj_queue_s ;
            callback_obj_queue_s = modadd(callback_obj_queue_s, 1, OS_MAIN_TASK_CALLBACK_CNT);
            callback_obj_queue_size--;
            OS_Exit_Critical();
            OS_Task_Set_Switchable(false);
            /*调用回调函数。*/
            obj->func(obj->src, obj->arg);
            OS_Task_Set_Switchable(true);
            OS_Enter_Critical();
        }
        /*所有回调函数调用完成后，阻塞主线程，等待唤醒。*/
        OS_Task_Start_Waiting(NULL, 0, NULL, NULL);
        OS_Exit_Critical();
        OS_Sched();
    }
}
#endif



static OS_Task os_idle_task;
static uint8_t os_idle_task_stack[OS_IDLE_TASK_STACK_SIZE];
/**
 * @brief 空闲任务。
 * @param arg NULL。
*/
static void os_idle_task_func(void* arg) {
    while (1);
}

void OS_Init(void) {
#if OS_IO_EN
    /*初始化全局IO控制块。*/
    OS_Global_IO_Init();
#endif
#if OS_MAIN_TASK_EN
    /*创建主任务。*/
    os_main_task = OS_Task_Create("OS Main Task", os_main_task_func, NULL, 0, 0, os_main_task_stack, OS_MAIN_TASK_STACK_SIZE);
#endif
    /*创建空闲任务。*/
    os_idle_task = OS_Task_Create("OS Idle Task", os_idle_task_func, NULL, OS_MAX_PRIO - 1, 0, os_idle_task_stack, OS_IDLE_TASK_STACK_SIZE);
}
/**
 * @brief 操作系统是否启动的标志变量。
*/
static bool os_started;

void OS_Start(void) {
    /*标记操作系统已启动*/
    os_started = true;
    /*使能任务调度器*/
    OS_Task_Set_Switchable(true);
    /*开始进行任务调度*/
    OS_Sched_New();
    OS_Task_Start();
}

bool OS_Started(void) {
    return os_started;
}

#if OS_MAIN_TASK_EN
OS_Error_t OS_Delay_Execute(OS_Callback_Func_t func, OS_Callback_Src_t src, OS_Callback_Arg_t arg) {

    os_param_assert(func, OS_ILLEGAL_PARAM);
    
    OS_Prepare_Critical();
    OS_Enter_Critical();
    OS_Error_t error = OS_OK;
    /*检测队列是否已满*/
    if (callback_obj_queue_size == OS_MAIN_TASK_CALLBACK_CNT) {
        error = OS_ERROR;
        goto end;
    }
    /*如果队列为空，需要唤醒主任务*/
    if (!callback_obj_queue_size)
        OS_Task_Stop_Waiting(os_main_task, OS_OK);
    /*将回调对象加入队列尾部*/
    OS_Callback_Obj* obj = callback_obj_queue + modadd(callback_obj_queue_s, callback_obj_queue_size, OS_MAIN_TASK_CALLBACK_CNT);
    callback_obj_queue_size++;
    /*设置回调对象*/
    obj->func = func;
    obj->src = src;
    obj->arg = arg;
end:
    OS_Exit_Critical();
    return error;
}
#endif


void OS_PendSV_Callback() {
}
