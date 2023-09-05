#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "os_evt.h"
#if OS_MUTEX_EN
/**
 * @brief 互斥量。如果一个任务获得了互斥量，那么他的优先级会被提高到不低于该互斥量的优先级。
*/
typedef OS_Event OS_Mutex;
/**
 * @brief 初始化一个互斥量。
 * @param mutex 互斥量指针。
 * @param name 互斥量名称。
 * @param prio 互斥量优先级。
*/
void OS_Mutex_Init(OS_Mutex mutex, const char* name, OS_Prio prio);
/**
 * @brief 创建一个互斥量。
 * @param name 互斥量名称。
 * @param prio 互斥量优先级。
 * @return 如果创建成功，返回互斥量指针；否则返回NULL。
*/
OS_Mutex OS_Mutex_Create(const char* name, OS_Prio prio);
/**
 * @brief 销毁一个互斥量。如果处于中断中且开启了中断延迟发布，则会将此操作发送到主线程。
 * @param mutex 需要销毁的互斥量。
 * @return OS_OK，如果销毁成功。
*/
OS_Error_t OS_Mutex_Release(OS_Mutex mutex);
/**
 * @brief 尝试锁定一个互斥量。不会阻塞当前任务。
 * @param mutex 需要锁定的互斥量。
 * @return OS_OK，如果锁定成功；OS_ERROR，如果锁定失败。
*/
OS_Error_t OS_Mutex_Try_Lock(OS_Mutex mutex);
/**
 * @brief 锁定一个互斥量，如果不能锁定获取，则阻塞当前任务。
 * @param mutex 需要锁定的互斥量。
 * @param timeout 如果为0，则无限等待，否则等待timeout个tick后返回。
 * @return OS_OK，如果锁定成功；OS_TIMEOUT，如果等待超时。
*/
OS_Error_t OS_Mutex_Lock(OS_Mutex mutex, OS_Tick timeout);
/**
 * @brief 解锁一个互斥量。如果处于中断中且开启了中断延迟发布，则会将此操作发送到主线程。
 * @param mutex 需要解锁的互斥量。
 * @return OS_OK，如果解锁成功；OS_ERROR，如果解锁失败。
*/
OS_Error_t OS_Mutex_UnLock(OS_Mutex mutex);


#endif



#ifdef __cplusplus
}
#endif