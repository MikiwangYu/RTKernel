#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "os_def.h"


/*ƽ̨��ֲ���*/

/**
 * @brief ���浱ǰ�����ж�״̬��Ȼ��ر��жϡ�
 * @return ��ǰ�ж�״̬��
*/
OS_CPU_SR OS_CPU_SR_Save(void);
/**
 * @brief �ָ��жϿ���״̬��
 * @param cpu_sr �жϿ���״̬��������OS_CPU_SR_Save�ķ���ֵ��
*/
void OS_CPU_SR_Restore(OS_CPU_SR cpu_sr);
/**
 * @brief �����������л���
*/
void OS_Task_Switch(void);
/**
 * @brief �жϼ��������л���
*/
void OS_Intr_Task_Switch(void);
/**
 * @brief ��������ϵͳ�ĵ�һ������
*/
void OS_Task_Start(void);
/**
 * @brief ��ʼ������ջ��
 * @param stack_p ջ����
 * @param exe_addr ������ʼ��ַ��
 * @param return_addr ����ִ�����֮��ķ��ص�ַ��
 * @param arg ���������
 * @return ��ʼ��֮���ջ����
*/
void* OS_Task_Stack_Init(void* stack_p, void* exe_addr, void* return_addr, void* arg);


/*ƽ̨��ֲ�޹�*/

/**
 * @brief ��ʼ������ϵͳ��
*/
void OS_Init(void);
/**
 * @brief ��������ϵͳ��
*/
void OS_Start(void);
/**
 * @brief ������ϵͳ�Ƿ��Ѿ�������
 * @return true���������ϵͳ�Ѿ�������false���������ϵͳδ������
*/
bool OS_Started(void);

#if OS_MAIN_TASK_EN
/**
 * @brief ͨ�����ж��б����á���һ���ص�������ӵ����߳��С�
 * @param func �ص�������
 * @param src �ص���������Դ��
 * @param arg �ص�����������
 * @return OS_OK������ɹ���ӣ�OS_ERROR������ص���������������
*/
OS_Error_t OS_Delay_Execute(OS_Callback_Func_t func,OS_Callback_Src_t src,OS_Callback_Arg_t arg);
#endif




/*׼���ٽ�������Ҫ�ľֲ�����*/
#define     OS_Prepare_Critical()   OS_CPU_SR __cpu_sr = 0
/*�����ٽ�����֧���ٽ���Ƕ��*/
#define     OS_Enter_Critical()     (__cpu_sr = OS_CPU_SR_Save())
/*�˳��ٽ���*/
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