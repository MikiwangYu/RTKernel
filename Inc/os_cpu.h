#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "os_def.h"


/*平台移植相关*/

/**
 * @brief 保存当前开关中断状态，然后关闭中断。
 * @return 当前中断状态。
*/
OS_CPU_SR OS_CPU_SR_Save(void);
/**
 * @brief 恢复中断开关状态。
 * @param cpu_sr 中断开关状态，必须是OS_CPU_SR_Save的返回值。
*/
void OS_CPU_SR_Restore(OS_CPU_SR cpu_sr);
/**
 * @brief 任务级上下文切换。
*/
void OS_Task_Switch(void);
/**
 * @brief 中断级上下文切换。
*/
void OS_Intr_Task_Switch(void);
/**
 * @brief 启动操作系统的第一个任务。
*/
void OS_Task_Start(void);
/**
 * @brief 初始化任务栈。
 * @param stack_p 栈顶。
 * @param exe_addr 代码起始地址。
 * @param return_addr 任务执行完成之后的返回地址。
 * @param arg 任务参数。
 * @return 初始化之后的栈顶。
*/
void* OS_Task_Stack_Init(void* stack_p, void* exe_addr, void* return_addr, void* arg);


/*平台移植无关*/

/**
 * @brief 初始化操作系统。
*/
void OS_Init(void);
/**
 * @brief 启动操作系统。
*/
void OS_Start(void);
/**
 * @brief 检测操作系统是否已经启动。
 * @return true，如果操作系统已经启动；false，如果操作系统未启动。
*/
bool OS_Started(void);

#if OS_MAIN_TASK_EN
/**
 * @brief 通常在中断中被调用。将一个回调函数添加到主线程中。
 * @param func 回调函数。
 * @param src 回调函数触发源。
 * @param arg 回调函数参数。
 * @return OS_OK，如果成功添加；OS_ERROR，如果回调函数队列已满。
*/
OS_Error_t OS_Delay_Execute(OS_Callback_Func_t func,OS_Callback_Src_t src,OS_Callback_Arg_t arg);
#endif




/*准备临界区所需要的局部变量*/
#define     OS_Prepare_Critical()   OS_CPU_SR __cpu_sr = 0
/*进入临界区，支持临界区嵌套*/
#define     OS_Enter_Critical()     (__cpu_sr = OS_CPU_SR_Save())
/*退出临界区*/
#define     OS_Exit_Critical()      OS_CPU_SR_Restore(__cpu_sr)


#define OS_Atomic(expr) do{\
    OS_Prepare_Critical();\
    OS_Enter_Critical();\
    (expr);\
    OS_Exit_Critical();\
}while (false)



#ifdef __cplusplus
}
#endif