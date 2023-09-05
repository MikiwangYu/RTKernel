#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "os_def.h"
#include "os_task.h"
#if OS_TASK_SEM_EN

typedef struct _OS_Task_SEM {
    OS_Task owner;
    bool _waiting;
    uint32_t _sem_cur;
    uint32_t _sem_max;
#if OS_TASK_SEM_NAME_EN
    const char* name;
#endif
    OS_List_Node _resource_node;
}OS_Task_SEM, *OS_Task_Sem;

/**
 * @brief 所有未被销毁的任务信号量的链表.
*/
extern OS_List OS_Task_Sem_List;
/**
 * @brief 所有已被销毁的任务信号量的链表。
*/
extern OS_List OS_Task_Sem_Free_List;


/**
 * @brief 初始化任务信号量。
 * @param sem 需要初始化的任务信号量。
 * @param name 任务信号量名称。
 * @param task 任务信号量的持有任务。
 * @param init_sem 信号量初始值。
 * @param max_sem 信号量最大值。
*/
void OS_Task_Sem_Init(OS_Task_Sem sem, const char *name, OS_Task task, uint32_t init_sem, uint32_t max_sem);
/**
 * @brief 创建一个任务信号量。
 * @param task 任务信号量的持有任务。
 * @param name 任务信号量名称。
 * @param init_sem 信号量初始值。
 * @param max_sem 信号量最大值。
 * @return 如果创建成功，返回创建的任务信号量；否则返回NULL。
*/
OS_Task_Sem OS_Task_Sem_Create(OS_Task task, const char* name, uint32_t init_sem, uint32_t max_sem);

/**
 * @brief 销毁一个任务信号量。
 * @param sem 需要销毁的任务信号量。
 * @return OS_OK，如果销毁成功。
*/
OS_Error_t OS_Task_Sem_Release(OS_Task_Sem sem);

/**
 * @brief 尝试获取当前任务的任务信号量，不会阻塞当前任务。
 * @param sem 需要操作的任务信号量。
 * @return OS_OK，如果获取成功；OS_ERROR，如果获取失败。
*/
OS_Error_t OS_Task_Sem_Try_Pend(OS_Task_Sem sem);
/**
 * @brief 获取当前任务的任务信号量，如果不能立马获取，则阻塞当前任务。
 * @param sem 需要操作的任务信号量。
 * @param timeout 如果为0，则无限等待，否则等待timeout个tick后返回。
 * @return OS_OK，如果获取成功；OS_TIMEOUT，如果等待超时。
*/
OS_Error_t OS_Task_Sem_Pend(OS_Task_Sem sem, OS_Tick timeout);
/**
 * @brief 发送一个任务信号量。如果处于中断中且开启了中断延迟发布，则会将此操作发送到主线程。
 * @param sem 需要操作的任务信号量。
 * @return OS_OK，如果发布成功；OS_ERROR，如果发布失败。
*/
OS_Error_t OS_Task_Sem_Post(OS_Task_Sem sem);

#if OS_TASK_SEM_NAME_EN
#endif
/**
 * @brief 根据名称查找一个任务信号量。
 * @param name 时间的名称。
 * @return 如果找到，返回该任务信号量；否则返回NULL。
*/
OS_Task_Sem OS_Task_Sem_Find(const char* name);
#endif
#ifdef __cplusplus
}
#endif