#include "os_mutex.h"
#include "os_task.h"
#include "os_cpu.h"
#include "os_int.h"
#if OS_MUTEX_EN

/**
 * @brief ��ʼ�����������е����ݡ�
 * @param mutex ��Ҫ��ʼ���Ļ�������
 * @param prio ���������ȼ���
*/
static void os_mutex_extra_init(OS_Mutex mutex, OS_Prio prio) {
    mutex->type = OS_EVENT_TYPE_MUTEX;
    mutex->_mutex_locked = false;
    OS_Mutex_Prio_Node_Init(&mutex->_mutex_node, mutex);
    mutex->_mutex_prio = prio;
}

void OS_Mutex_Init(OS_Mutex mutex, const char* name, OS_Prio prio) {
    /*���û��๹�캯����*/
    OS_Event_Init(mutex, name);
    /*���������캯����*/
    os_mutex_extra_init(mutex, prio);
}

OS_Mutex OS_Mutex_Create(const char* name, OS_Prio prio) {
    /*���û��๹�캯����*/
    OS_Mutex mutex = OS_Event_Create(name);
    /*���������캯����*/
    if (mutex)
        os_mutex_extra_init(mutex, prio);
    return mutex;
}

/**
 * @brief ��Ϊ�ص��������û�������غ�����
 * @param src ��������
 * @param arg ������
*/
static void os_mutex_delay_execute_callback(void* src, void* arg) {
    ((OS_Error_t(*)(OS_Mutex*))src)(arg);
}

OS_Error_t OS_Mutex_Release(OS_Mutex mutex) {
    os_param_assert(mutex, OS_ILLEGAL_PARAM);
    os_param_assert(mutex->type = OS_EVENT_TYPE_MUTEX, OS_ILLEGAL_PARAM);

#if OS_INTR_DELAY_EN
    /*�ӳٷ�����*/
    if (OS_In_Intr())
        return OS_Delay_Execute(os_mutex_delay_execute_callback, mutex, OS_Mutex_Release);
#endif

    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_Error_t error = OS_OK;
    /*����������ѱ��������������١�*/
    if (mutex->_mutex_locked) {
        error = OS_ERROR;
        return error;
    }
    /*���ø�������������*/
    error = OS_Event_Release(mutex);
    OS_Exit_Protect();
    return error;
}

OS_Error_t OS_Mutex_Try_Lock(OS_Mutex mutex) {
    os_param_assert(mutex, OS_ILLEGAL_PARAM);
    os_param_assert(mutex->type = OS_EVENT_TYPE_MUTEX, OS_ILLEGAL_PARAM);

    os_assert(!OS_In_Intr(), OS_PERMISSION_DENY);
    os_assert(OS_Task_Switchable(), OS_PERMISSION_DENY);
        
    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_Error_t error = OS_OK;
    if (!mutex->_mutex_locked) {
        /*������������*/
        mutex->_mutex_locked = true;
        /*�޸ĵ�ǰ��������ȼ���*/
        OS_Task_Mount_Mutex(OS_Cur_Task, &mutex->_mutex_node, mutex->_mutex_prio);
    } else {
        error = OS_ERROR;
    }
    OS_Exit_Protect();
    return error;
}

OS_Error_t OS_Mutex_Lock(OS_Mutex mutex, OS_Tick timeout) {
    os_param_assert(mutex, OS_ILLEGAL_PARAM);
    os_param_assert(mutex->type = OS_EVENT_TYPE_MUTEX, OS_ILLEGAL_PARAM);


    os_assert(!OS_In_Intr(), OS_PERMISSION_DENY);
    os_assert(OS_Task_Switchable(), OS_PERMISSION_DENY);

    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_Error_t error = OS_OK;
    if (!mutex->_mutex_locked) {
        /*������������*/
        mutex->_mutex_locked = true;
        /*�޸ĵ�ǰ��������ȼ���*/
        OS_Task_Mount_Mutex(OS_Cur_Task, &mutex->_mutex_node, mutex->_mutex_prio);
        goto end;
    }
    /*������ǰ����*/
    OS_Task_Start_Waiting(&mutex->_ready_table, timeout, NULL, NULL);
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

OS_Error_t OS_Mutex_UnLock(OS_Mutex mutex) {
    os_param_assert(mutex, OS_ILLEGAL_PARAM);
    os_param_assert(mutex->type = OS_EVENT_TYPE_MUTEX, OS_ILLEGAL_PARAM);

    os_assert(!OS_In_Intr(), OS_PERMISSION_DENY);
    os_assert(OS_Task_Switchable(), OS_PERMISSION_DENY);

    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_Error_t error = OS_OK;
    if (!mutex->_mutex_locked) {
        error = OS_ERROR;
        goto end;
    }
    /*������������*/
    mutex->_mutex_locked = false;
    /*�޸ĵ�ǰ��������ȼ���*/
    OS_Task_UnMount_Mutex(OS_Cur_Task, &mutex->_mutex_node, mutex->_mutex_prio);
    /*��ȡ�������е�������ȼ���*/
    OS_Prio prio = OS_Prio_Table_Get_Prio(&mutex->_ready_table);
    /*���������в�Ϊ�ա�*/
    if (prio != OS_MAX_PRIO) {
        /*ȡ�����ȼ���ߵ�����*/
        OS_Task task = OS_Prio_Table_Get_Next(&mutex->_ready_table, prio)->data;
        /*���Ѹ�����*/
        OS_Task_Stop_Waiting(task, OS_OK);
        /*�޸ĸ���������ȼ���*/
        OS_Task_Mount_Mutex(task, &mutex->_mutex_node, mutex->_mutex_prio);
    }
end:
    OS_Exit_Protect();
    /*����Ƿ���Ҫ���µ��ȡ�*/
    if (OS_Need_Sched())
        OS_Sched();
    return error;
}



#endif