#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "os_evt.h"
#if OS_MUTEX_EN
/**
 * @brief �����������һ���������˻���������ô�������ȼ��ᱻ��ߵ������ڸû����������ȼ���
*/
typedef OS_Event OS_Mutex;
/**
 * @brief ��ʼ��һ����������
 * @param mutex ������ָ�롣
 * @param name ���������ơ�
 * @param prio ���������ȼ���
*/
void OS_Mutex_Init(OS_Mutex mutex, const char* name, OS_Prio prio);
/**
 * @brief ����һ����������
 * @param name ���������ơ�
 * @param prio ���������ȼ���
 * @return ��������ɹ������ػ�����ָ�룻���򷵻�NULL��
*/
OS_Mutex OS_Mutex_Create(const char* name, OS_Prio prio);
/**
 * @brief ����һ������������������ж����ҿ������ж��ӳٷ�������Ὣ�˲������͵����̡߳�
 * @param mutex ��Ҫ���ٵĻ�������
 * @return OS_OK��������ٳɹ���
*/
OS_Error_t OS_Mutex_Release(OS_Mutex mutex);
/**
 * @brief ��������һ��������������������ǰ����
 * @param mutex ��Ҫ�����Ļ�������
 * @return OS_OK����������ɹ���OS_ERROR���������ʧ�ܡ�
*/
OS_Error_t OS_Mutex_Try_Lock(OS_Mutex mutex);
/**
 * @brief ����һ�����������������������ȡ����������ǰ����
 * @param mutex ��Ҫ�����Ļ�������
 * @param timeout ���Ϊ0�������޵ȴ�������ȴ�timeout��tick�󷵻ء�
 * @return OS_OK����������ɹ���OS_TIMEOUT������ȴ���ʱ��
*/
OS_Error_t OS_Mutex_Lock(OS_Mutex mutex, OS_Tick timeout);
/**
 * @brief ����һ������������������ж����ҿ������ж��ӳٷ�������Ὣ�˲������͵����̡߳�
 * @param mutex ��Ҫ�����Ļ�������
 * @return OS_OK����������ɹ���OS_ERROR���������ʧ�ܡ�
*/
OS_Error_t OS_Mutex_UnLock(OS_Mutex mutex);


#endif



#ifdef __cplusplus
}
#endif