#include "os_sem.h"
#include "os_task.h"
#include "os_int.h"
#include "os_cpu.h"
#if OS_EVENT_EN

/**
 * @brief 初始化信号量独有的内容。
 * @param sem 需要初始化的信号量。
 * @param init_sem 信号量初始值。
 * @param max_sem 信号量最大值。
*/
static void os_sem_extra_init(OS_Sem sem, uint32_t init_sem, uint32_t max_sem) {
    sem->type = OS_EVENT_TYPE_SEM;
    sem->_sem_cur = init_sem;
    sem->_sem_max = max_sem;
}

void OS_Sem_Init(OS_Sem sem, const char* name, uint32_t init_sem, uint32_t max_sem) {
    /*调用基类构造函数。*/
    OS_Event_Init(sem, name);
    /*调用自身构造函数。*/
    os_sem_extra_init(sem, init_sem, max_sem);
}

OS_Sem OS_Sem_Create(const char* name, uint32_t init_sem, uint32_t max_sem) {
    /*调用基类构造函数。*/
    OS_Sem sem = OS_Event_Create(name);
    /*调用自身构造函数。*/
    if (sem)
        os_sem_extra_init(sem, init_sem, max_sem);
    return sem;
}

OS_Error_t OS_Sem_Release(OS_Sem sem) {
    /*调用父类析构函数。*/
    return OS_Event_Release(sem);
}

/**
 * @brief 作为回调函数调用信号量相关函数。
 * @param src 信号量。
 * @param arg 函数。
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
        /*修改信号量计数。*/
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
        /*修改信号量计数。*/
        --sem->_sem_cur;
        goto end;
    }
    /*阻塞当前任务。*/
    OS_Task_Start_Waiting(&sem->_ready_table, timeout, NULL, NULL);
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

OS_Error_t OS_Sem_Post(OS_Sem sem) {
    os_param_assert(sem, OS_ILLEGAL_PARAM);
    os_param_assert(sem->type == OS_EVENT_TYPE_SEM, OS_ILLEGAL_PARAM);


#if OS_INTR_DELAY_EN
    /*延迟发布。*/
    if (OS_In_Intr())
        return OS_Delay_Execute(os_sem_delay_execute_callback, sem, OS_Sem_Post);
#endif


    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_Error_t error = OS_OK;
    if (OS_Prio_Table_Get_Prio(&sem->_ready_table) != OS_MAX_PRIO)
        /*唤醒优先级最高的任务。*/
        OS_Task_Stop_Waiting(OS_Prio_Table_Get_Next(&sem->_ready_table, OS_MAX_PRIO)->data, OS_OK);
    else if (sem->_sem_cur != sem->_sem_max)
        /*修改信号量计数。*/
        sem->_sem_cur++;
    else
        error = OS_ERROR;
    OS_Exit_Protect();
    /*检查是否需要重新调度。*/
    if (OS_Need_Sched())
        OS_Sched();
    return error;
}



#endif