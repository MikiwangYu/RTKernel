#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "os_evt.h"

#if OS_EVENT_EN
/**
 * @brief 信号量。
*/
typedef OS_Event OS_Sem;
/**
 * @brief 初始化一个信号量。
 * @param sem 需要初始化的信号量。
 * @param name 信号量名称。
 * @param init_sem 信号量初始值。
 * @param max_sem 信号量最大值。
*/
void OS_Sem_Init(OS_Sem sem, const char* name, uint32_t init_sem, uint32_t max_sem);
/**
 * @brief 创建一个信号量。
 * @param name 信号量名称。
 * @param init_sem 信号量初始值。
 * @param max_sem 信号量最大值。
 * @return 如果创建成功，返回信号量指针；否则返回NULL。
*/
OS_Sem OS_Sem_Create(const char* name, uint32_t init_sem, uint32_t max_sem);
/**
 * @brief 销毁一个信号量。
 * @param sem 需要销毁的信号量。
 * @return OS_OK，如果销毁成功。
*/
OS_Error_t OS_Sem_Release(OS_Sem sem);
/**
 * @brief 尝试获取信号量，不会阻塞当前任务。
 * @param sem 需要获取的信号量。
 * @return OS_OK，如果获取成功；OS_ERROR，如果获取失败。
*/
OS_Error_t OS_Sem_Try_Pend(OS_Sem sem);
/**
 * @brief 获取信号量，如果不能立马获取，则阻塞当前任务。
 * @param sem 需要获取的信号量。
 * @param timeout 如果为0，则无限等待，否则等待timeout个tick后返回。
 * @return OS_OK，如果获取成功；OS_TIMEOUT，如果等待超时。
*/
OS_Error_t OS_Sem_Pend(OS_Sem sem, OS_Tick timeout);
/**
 * @brief 发送信号量。如果处于中断中且开启了中断延迟发布，则会将此操作发送到主线程。
 * @param sem 需要发送的信号量。
 * @return OS_OK，如果发布成功；OS_ERROR，如果发布失败。
*/
OS_Error_t OS_Sem_Post(OS_Sem sem);
#endif

#ifdef __cplusplus
}
#endif