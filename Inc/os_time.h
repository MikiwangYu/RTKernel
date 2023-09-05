#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "os_def.h"


/**
 * @brief ��ȡ����ϵͳ������������Tick����
 * @return ����ϵͳ������������Tick����ʵ��������OS_TICK_PER_SEC������
 * @see OS_TICK_PER_SEC��
*/
OS_Tick OS_Get_Tick_Count(void);
/**
    * @brief
*/
void OS_Tick_Inc(void);
/**
    * @brief ������ǰ����delay��Tick��
    * @param delay ����ʱ����
*/
void OS_Delay(OS_Tick delay);

#if OS_TIMER_EN
/**
 * @brief ��ʱ�����ࡣ
*/
typedef enum _OS_Timer_OPT {
    /*���ζ�ʱ��*/
    OS_TIMER_OPT_ONE_SHOT = 0,
    /*ѭ����ʱ��*/
    OS_TIMER_OPT_PERIODIC
}OS_Timer_OPT;


/**
 * @brief ��ʱ��״̬��
*/
typedef enum _OS_Timer_Status {
    /**
     * @brief ������
    */
    OS_TIMER_STATUS_READY = 0,
    /**
     * @brief �����С�
    */
    OS_TIMER_STATUS_RUNNING,
    /**
     * @brief ��ͣ��
    */
    OS_TIMER_STATUS_SUSPEND,
    /**
     * @brief ִ����ɡ�
    */
    OS_TIMER_STATUS_FINISHED,
    /**
     * @brief �����١�
    */
    OS_TIMER_STATUS_RELEASED
}OS_Timer_Status;


/**
 * @brief ��ʱ�����ƿ顣
*/
typedef struct _OS_TMR {
    /**
     * @brief ��ʱ�����ࡣ
    */
    OS_Timer_OPT opt;
    /**
     * @brief ��ʱ�������ӳ١�
    */
    OS_Tick delay;
    /**
     * @brief ��ʱ�����ڡ�
    */
    OS_Tick period;
    /**
     * @brief ��ʱ���ص�������
    */
    OS_Callback_Func_t func;
    /**
     * @brief ��ʱ���ص�����������
    */
    OS_Callback_Arg_t arg;
    /**
     * @brief ��ʱ��״̬��
    */
    OS_Timer_Status status;
#if OS_TIMER_NAME_EN
    /**
     * @brief ��ʱ�����ơ�
    */
    const char* name;
#endif
    /**
     * @brief ������һ�δ�����ʱ�䡣
    */
    OS_Tick _rest;
    /**
     * @brief ��Դ����ڵ㡣�κα��ͷŵĶ�ʱ���������ô˽ڵ㽫�Լ����ص�OS_Timer_Free_List�ϣ��ȴ��ٴη���,�κ��ѷ��䣬δ�ͷŵĶ�ʱ���������ô˽ڵ㽫�Լ����ص�OS_Timer_List�ϡ�
     * @see OS_Timer_Free_List
    */
    OS_List_Node _resource_node;
    /**
     * @brief �������ȶ��нڵ㡣�κδ�������״̬�Ķ�ʱ�����Ὣ�Լ����ص�һ�����ȶ����ϣ���˳��ȴ�������
    */
    OS_Priority_Queue_Node  _pq_node;
}OS_TMR,*OS_Timer;


/**
 * @brief ��ʼ��һ����ʱ����
 * @param timer ��ʱ��ָ�롣
 * @param name ��ʱ�����ƣ�����NULLʱΪ"NULL"��
 * @param opt ��ʱ�����͡�
 * @param delay ���һ�δ�������ʱ��
 * @param period ���δ����ļ����
 * @param func ��ʱ���ص�������
 * @param arg ��ʱ���ص�����������
*/
void OS_Timer_Init(OS_Timer timer, const char* name, OS_Timer_OPT opt, OS_Tick delay, OS_Tick period, OS_Callback_Func_t func, OS_Callback_Arg_t arg);
/**
 * @brief ����һ����ʱ����
 * @param name ��ʱ�����ƣ�����NULLʱΪ"NULL"��
 * @param opt ��ʱ�����͡�
 * @param delay ���һ�δ�������ʱ��
 * @param period ���δ����ļ����
 * @param func ��ʱ���ص�������
 * @param arg ��ʱ���ص�����������
 * @return �����ɹ��򷵻ض�ʱ��ָ�룬����ʧ���򷵻�NULL��
*/
OS_Timer OS_Timer_Create(const char* name, OS_Timer_OPT opt, OS_Tick delay, OS_Tick period, OS_Callback_Func_t func, OS_Callback_Arg_t arg);

/**
 * @brief ����һ����ʱ������������ж����ҿ������ж��ӳٷ�������Ὣ�˲������͵����̡߳�
 * @param timer ��Ҫ���ٵĶ�ʱ����
 * @return OS_OK������ɹ����ٶ�ʱ����
*/
OS_Error_t OS_Timer_Release(OS_Timer timer);
/**
 * @brief ����δ���ٵĶ�ʱ��������
*/
extern OS_List OS_Timer_List;
/**
 * @brief ���������ٵĶ�ʱ��������
*/
extern OS_List OS_Timer_Free_List;

/**
 * @brief ����һ����ʱ������������һ������ͣ�Ķ�ʱ������������ж����ҿ������ж��ӳٷ�������Ὣ�˲������͵����̡߳�
 * @param timer ��Ҫ�����Ķ�ʱ����
 * @return ���ɹ�������ʱ���Ѿ������򷵻�OS_OK������ʱ���ѱ������򷵻�OS_ERROR��
*/
OS_Error_t OS_Timer_Start(OS_Timer timer);
/**
 * @brief ֹͣһ���������еĶ�ʱ������������ж����ҿ������ж��ӳٷ�������Ὣ�˲������͵����̡߳�
 * @param timer ��Ҫֹͣ�Ķ�ʱ����
 * @return ���ɹ�ֹͣ��ʱ���ѱ�ֹͣ�򷵻�OS_OK������ʱ���ѱ������򷵻�OS_ERROR��
*/
OS_Error_t OS_Timer_Stop(OS_Timer timer);
/**
 * @brief ��ͣһ���������еĶ�ʱ��������¼������һ�δ�����ʱ�䡣��������ж����ҿ������ж��ӳٷ�������Ὣ�˲������͵����̡߳�
 * @param timer ��Ҫ��ͣ�Ķ�ʱ����
 * @return ���ɹ���ͣ��ʱ������ʱ������ͣ����ʱ�������н������򷵻�OS_OK������ʱ���ѱ����٣��򷵻�OS_ERROR��
*/
OS_Error_t OS_Timer_Suspend(OS_Timer timer);
#if OS_TIMER_NAME_EN
/**
 * @brief ͨ����ʱ�����Ʋ��Ҷ�Ӧ�Ķ�ʱ����
 * @param name ��Ҫ���ҵĶ�ʱ�������ơ�
 * @return �������ʱ��������ͬ���򷵻ص�һ���ҵ��Ķ�ʱ������û���ҵ���Ӧ�Ķ�ʱ�����򷵻�NULL��
*/
OS_Timer OS_Timer_Find(const char* name);
#endif
#endif
#ifdef __cplusplus
}
#endif