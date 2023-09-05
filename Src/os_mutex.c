#include "os_mutex.h"
#include "os_task.h"
#include "os_cpu.h"
#include "os_int.h"
#if OS_MUTEX_EN

/**
 * @brief 初始化互斥量独有的内容。
 * @param mutex 需要初始化的互斥量。
 * @param prio 互斥量优先级。
*/
static void os_mutex_extra_init(OS_Mutex mutex, OS_Prio prio) {
    mutex->type = OS_EVENT_TYPE_MUTEX;
    mutex->_mutex_locked = false;
    OS_Mutex_Prio_Node_Init(&mutex->_mutex_node, mutex);
    mutex->_mutex_prio = prio;
}

void OS_Mutex_Init(OS_Mutex mutex, const char* name, OS_Prio prio) {
    /*调用基类构造函数。*/
    OS_Event_Init(mutex, name);
    /*调用自身构造函数。*/
    os_mutex_extra_init(mutex, prio);
}

OS_Mutex OS_Mutex_Create(const char* name, OS_Prio prio) {
    /*调用基类构造函数。*/
    OS_Mutex mutex = OS_Event_Create(name);
    /*调用自身构造函数。*/
    if (mutex)
        os_mutex_extra_init(mutex, prio);
    return mutex;
}

/**
 * @brief 作为回调函数调用互斥量相关函数。
 * @param src 互斥量。
 * @param arg 函数。
*/
static void os_mutex_delay_execute_callback(void* src, void* arg) {
    ((OS_Error_t(*)(OS_Mutex*))src)(arg);
}

OS_Error_t OS_Mutex_Release(OS_Mutex mutex) {
    os_param_assert(mutex, OS_ILLEGAL_PARAM);
    os_param_assert(mutex->type = OS_EVENT_TYPE_MUTEX, OS_ILLEGAL_PARAM);

#if OS_INTR_DELAY_EN
    /*延迟发布。*/
    if (OS_In_Intr())
        return OS_Delay_Execute(os_mutex_delay_execute_callback, mutex, OS_Mutex_Release);
#endif

    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_Error_t error = OS_OK;
    /*如果互斥量已被锁定，则不能销毁。*/
    if (mutex->_mutex_locked) {
        error = OS_ERROR;
        return error;
    }
    /*调用父类析构函数。*/
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
        /*锁定互斥量。*/
        mutex->_mutex_locked = true;
        /*修改当前任务的优先级。*/
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
        /*锁定互斥量。*/
        mutex->_mutex_locked = true;
        /*修改当前任务的优先级。*/
        OS_Task_Mount_Mutex(OS_Cur_Task, &mutex->_mutex_node, mutex->_mutex_prio);
        goto end;
    }
    /*阻塞当前任务。*/
    OS_Task_Start_Waiting(&mutex->_ready_table, timeout, NULL, NULL);
    OS_Exit_Protect();

    /*切换任务。*/
    OS_Sched();

    OS_Enter_Protect();
    /*设置等待后的返回值。*/
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
    /*解锁互斥量。*/
    mutex->_mutex_locked = false;
    /*修改当前任务的优先级。*/
    OS_Task_UnMount_Mutex(OS_Cur_Task, &mutex->_mutex_node, mutex->_mutex_prio);
    /*获取就绪队列的最高优先级。*/
    OS_Prio prio = OS_Prio_Table_Get_Prio(&mutex->_ready_table);
    /*若就绪队列不为空。*/
    if (prio != OS_MAX_PRIO) {
        /*取出优先级最高的任务。*/
        OS_Task task = OS_Prio_Table_Get_Next(&mutex->_ready_table, prio)->data;
        /*唤醒该任务。*/
        OS_Task_Stop_Waiting(task, OS_OK);
        /*修改该任务的优先级。*/
        OS_Task_Mount_Mutex(task, &mutex->_mutex_node, mutex->_mutex_prio);
    }
end:
    OS_Exit_Protect();
    /*检查是否需要重新调度。*/
    if (OS_Need_Sched())
        OS_Sched();
    return error;
}



#endif