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
 * @brief ����δ�����ٵ������ź���������.
*/
extern OS_List OS_Task_Sem_List;
/**
 * @brief �����ѱ����ٵ������ź���������
*/
extern OS_List OS_Task_Sem_Free_List;


/**
 * @brief ��ʼ�������ź�����
 * @param sem ��Ҫ��ʼ���������ź�����
 * @param name �����ź������ơ�
 * @param task �����ź����ĳ�������
 * @param init_sem �ź�����ʼֵ��
 * @param max_sem �ź������ֵ��
*/
void OS_Task_Sem_Init(OS_Task_Sem sem, const char *name, OS_Task task, uint32_t init_sem, uint32_t max_sem);
/**
 * @brief ����һ�������ź�����
 * @param task �����ź����ĳ�������
 * @param name �����ź������ơ�
 * @param init_sem �ź�����ʼֵ��
 * @param max_sem �ź������ֵ��
 * @return ��������ɹ������ش����������ź��������򷵻�NULL��
*/
OS_Task_Sem OS_Task_Sem_Create(OS_Task task, const char* name, uint32_t init_sem, uint32_t max_sem);

/**
 * @brief ����һ�������ź�����
 * @param sem ��Ҫ���ٵ������ź�����
 * @return OS_OK��������ٳɹ���
*/
OS_Error_t OS_Task_Sem_Release(OS_Task_Sem sem);

/**
 * @brief ���Ի�ȡ��ǰ����������ź���������������ǰ����
 * @param sem ��Ҫ�����������ź�����
 * @return OS_OK�������ȡ�ɹ���OS_ERROR�������ȡʧ�ܡ�
*/
OS_Error_t OS_Task_Sem_Try_Pend(OS_Task_Sem sem);
/**
 * @brief ��ȡ��ǰ����������ź�����������������ȡ����������ǰ����
 * @param sem ��Ҫ�����������ź�����
 * @param timeout ���Ϊ0�������޵ȴ�������ȴ�timeout��tick�󷵻ء�
 * @return OS_OK�������ȡ�ɹ���OS_TIMEOUT������ȴ���ʱ��
*/
OS_Error_t OS_Task_Sem_Pend(OS_Task_Sem sem, OS_Tick timeout);
/**
 * @brief ����һ�������ź�������������ж����ҿ������ж��ӳٷ�������Ὣ�˲������͵����̡߳�
 * @param sem ��Ҫ�����������ź�����
 * @return OS_OK����������ɹ���OS_ERROR���������ʧ�ܡ�
*/
OS_Error_t OS_Task_Sem_Post(OS_Task_Sem sem);

#if OS_TASK_SEM_NAME_EN
#endif
/**
 * @brief �������Ʋ���һ�������ź�����
 * @param name ʱ������ơ�
 * @return ����ҵ������ظ������ź��������򷵻�NULL��
*/
OS_Task_Sem OS_Task_Sem_Find(const char* name);
#endif
#ifdef __cplusplus
}
#endif