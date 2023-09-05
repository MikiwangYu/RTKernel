#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "os_def.h"


/**
 * @brief ����Ƿ����ж��С�
 * @return true����������ж��У�false������������жϡ�
*/
bool OS_In_Intr(void);
/**
 * @brief ��ȡ��ǰ�ж�Ƕ�ײ���
 * @return �ж�Ƕ�ײ���
*/
uint32_t OS_Intr_Cnt(void);

/**
 * @brief �����ж�ʱ���� ��
*/
void OS_Intr_Enter(void);
/**
 * @brief �˳��ж�ʱ���ã������Ƿ���Ҫ�л����� 
*/
void OS_Intr_Exit(void);

#if OS_INTR_DELAY_EN
/*����ֲ������Ա��������״̬��*/
#define OS_Prepare_Protect()    bool __task_switchable
/*���������״̬��Ȼ��ر������������*/
#define OS_Enter_Protect()      (__task_switchable = OS_Task_Set_Switchable(false))
/*�ָ������������״̬��*/
#define OS_Exit_Protect()       OS_Task_Set_Switchable(__task_switchable)
#else
/*׼���ٽ�������Ҫ�ľֲ�����*/
#define OS_Prepare_Protect()    OS_Prepare_Critical();
/*�����ٽ���*/
#define OS_Enter_Protect()      OS_Enter_Critical();
/*�˳��ٽ���*/
#define OS_Exit_Protect()       OS_Exit_Critical();
#endif

#ifdef __cplusplus
}
#endif