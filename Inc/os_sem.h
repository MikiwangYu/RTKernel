#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "os_evt.h"

#if OS_EVENT_EN
/**
 * @brief �ź�����
*/
typedef OS_Event OS_Sem;
/**
 * @brief ��ʼ��һ���ź�����
 * @param sem ��Ҫ��ʼ�����ź�����
 * @param name �ź������ơ�
 * @param init_sem �ź�����ʼֵ��
 * @param max_sem �ź������ֵ��
*/
void OS_Sem_Init(OS_Sem sem, const char* name, uint32_t init_sem, uint32_t max_sem);
/**
 * @brief ����һ���ź�����
 * @param name �ź������ơ�
 * @param init_sem �ź�����ʼֵ��
 * @param max_sem �ź������ֵ��
 * @return ��������ɹ��������ź���ָ�룻���򷵻�NULL��
*/
OS_Sem OS_Sem_Create(const char* name, uint32_t init_sem, uint32_t max_sem);
/**
 * @brief ����һ���ź�����
 * @param sem ��Ҫ���ٵ��ź�����
 * @return OS_OK��������ٳɹ���
*/
OS_Error_t OS_Sem_Release(OS_Sem sem);
/**
 * @brief ���Ի�ȡ�ź���������������ǰ����
 * @param sem ��Ҫ��ȡ���ź�����
 * @return OS_OK�������ȡ�ɹ���OS_ERROR�������ȡʧ�ܡ�
*/
OS_Error_t OS_Sem_Try_Pend(OS_Sem sem);
/**
 * @brief ��ȡ�ź�����������������ȡ����������ǰ����
 * @param sem ��Ҫ��ȡ���ź�����
 * @param timeout ���Ϊ0�������޵ȴ�������ȴ�timeout��tick�󷵻ء�
 * @return OS_OK�������ȡ�ɹ���OS_TIMEOUT������ȴ���ʱ��
*/
OS_Error_t OS_Sem_Pend(OS_Sem sem, OS_Tick timeout);
/**
 * @brief �����ź�������������ж����ҿ������ж��ӳٷ�������Ὣ�˲������͵����̡߳�
 * @param sem ��Ҫ���͵��ź�����
 * @return OS_OK����������ɹ���OS_ERROR���������ʧ�ܡ�
*/
OS_Error_t OS_Sem_Post(OS_Sem sem);
#endif

#ifdef __cplusplus
}
#endif