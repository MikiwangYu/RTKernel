#include "os_sem.h"
#include "os_task.h"
#include "os_int.h"
#include "os_cpu.h"
#if OS_EVENT_EN

/**
 * @brief ��ʼ���ź������е����ݡ�
 * @param sem ��Ҫ��ʼ�����ź�����
 * @param init_sem �ź�����ʼֵ��
 * @param max_sem �ź������ֵ��
*/
static void os_sem_extra_init(OS_Sem sem, uint32_t init_sem, uint32_t max_sem) {
    sem->type = OS_EVENT_TYPE_SEM;
    sem->_sem_cur = init_sem;
    sem->_sem_max = max_sem;
}

void OS_Sem_Init(OS_Sem sem, const char* name, uint32_t init_sem, uint32_t max_sem) {
    /*���û��๹�캯����*/
    OS_Event_Init(sem, name);
    /*���������캯����*/
    os_sem_extra_init(sem, init_sem, max_sem);
}

OS_Sem OS_Sem_Create(const char* name, uint32_t init_sem, uint32_t max_sem) {
    /*���û��๹�캯����*/
    OS_Sem sem = OS_Event_Create(name);
    /*���������캯����*/
    if (sem)
        os_sem_extra_init(sem, init_sem, max_sem);
    return sem;
}

OS_Error_t OS_Sem_Release(OS_Sem sem) {
    /*���ø�������������*/
    return OS_Event_Release(sem);
}

/**
 * @brief ��Ϊ�ص����������ź�����غ�����
 * @param src �ź�����
 * @param arg ������
*/
static void os_sem_delay_execute_callback(void* src, void* arg) {
    ((OS_Error_t(*)(OS_Sem))arg)(src);
}

OS_Error_t OS_Sem_Try_Pend(OS_Sem sem) {
    os_param_assert(sem, OS_ILLEGAL_PARAM);
    os_param_assert(sem->type == OS_EVENT_TYPE_SEM, OS_ILLEGAL_PARAM);


    os_assert(!OS_In_Intr(), OS_PERMISSION_DENY);
    os_assert(OS_Task_Switchable(), OS_PERMISSION_DENY);


    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_Error_t error = OS_OK;
    if (sem->_sem_cur)
        /*�޸��ź���������*/
        --sem->_sem_cur;
    else
        error = OS_ERROR;
    OS_Exit_Protect();
    return error;
}

OS_Error_t OS_Sem_Pend(OS_Sem sem, OS_Tick timeout) {
    os_param_assert(sem, OS_ILLEGAL_PARAM);
    os_param_assert(sem->type == OS_EVENT_TYPE_SEM, OS_ILLEGAL_PARAM);


    os_assert(!OS_In_Intr(), OS_PERMISSION_DENY);
    os_assert(OS_Task_Switchable(), OS_PERMISSION_DENY);


    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_Error_t error = OS_OK;
    if (sem->_sem_cur) {
        /*�޸��ź���������*/
        --sem->_sem_cur;
        goto end;
    }
    /*������ǰ����*/
    OS_Task_Start_Waiting(&sem->_ready_table, timeout, NULL, NULL);
    OS_Exit_Protect();

    /*�л�����*/
    OS_Sched();

    OS_Enter_Protect();
    /*���õȴ���ķ���ֵ��*/
    error = OS_Cur_Task->_error;
end:
    OS_Exit_Protect();
    return error;
}

OS_Error_t OS_Sem_Post(OS_Sem sem) {
    os_param_assert(sem, OS_ILLEGAL_PARAM);
    os_param_assert(sem->type == OS_EVENT_TYPE_SEM, OS_ILLEGAL_PARAM);


#if OS_INTR_DELAY_EN
    /*�ӳٷ�����*/
    if (OS_In_Intr())
        return OS_Delay_Execute(os_sem_delay_execute_callback, sem, OS_Sem_Post);
#endif


    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_Error_t error = OS_OK;
    if (OS_Prio_Table_Get_Prio(&sem->_ready_table) != OS_MAX_PRIO)
        /*�������ȼ���ߵ�����*/
        OS_Task_Stop_Waiting(OS_Prio_Table_Get_Next(&sem->_ready_table, OS_MAX_PRIO)->data, OS_OK);
    else if (sem->_sem_cur != sem->_sem_max)
        /*�޸��ź���������*/
        sem->_sem_cur++;
    else
        error = OS_ERROR;
    OS_Exit_Protect();
    /*����Ƿ���Ҫ���µ��ȡ�*/
    if (OS_Need_Sched())
        OS_Sched();
    return error;
}



#endif