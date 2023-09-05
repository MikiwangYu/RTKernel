#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "os_def.h"


/**
 * @brief 检测是否处于中断中。
 * @return true，如果处于中断中；false，如果不处于中断。
*/
bool OS_In_Intr(void);
/**
 * @brief 获取当前中断嵌套层数
 * @return 中断嵌套层数
*/
uint32_t OS_Intr_Cnt(void);

/**
 * @brief 进入中断时调用 。
*/
void OS_Intr_Enter(void);
/**
 * @brief 退出中断时调用，会检测是否需要切换任务。 
*/
void OS_Intr_Exit(void);

#if OS_INTR_DELAY_EN
/*分配局部变量以保存调度器状态。*/
#define OS_Prepare_Protect()    bool __task_switchable
/*保存调度器状态，然后关闭任务调度器。*/
#define OS_Enter_Protect()      (__task_switchable = OS_Task_Set_Switchable(false))
/*恢复任务调度器的状态。*/
#define OS_Exit_Protect()       OS_Task_Set_Switchable(__task_switchable)
#else
/*准备临界区所需要的局部变量*/
#define OS_Prepare_Protect()    OS_Prepare_Critical();
/*进入临界区*/
#define OS_Enter_Protect()      OS_Enter_Critical();
/*退出临界区*/
#define OS_Exit_Protect()       OS_Exit_Critical();
#endif

#ifdef __cplusplus
}
#endif