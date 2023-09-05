#include "os_task_sem.h"
#include "os_int.h"
#include "os_mem.h"
#include "os_cpu.h"

#if OS_TASK_SEM_EN

OS_List OS_Task_Sem_List;
OS_List OS_Task_Sem_Free_List;

void OS_Task_Sem_Init(OS_Task_Sem sem, const char* name, OS_Task task, uint32_t init_sem, uint32_t max_sem) {

    /*自身属性设置。*/
    sem->owner = task;
    sem->_waiting = false;
    sem->_sem_cur = init_sem;
    sem->_sem_max = max_sem;

#if OS_TASK_SEM_NAME_EN
    sem->name = name ? name : "NULL";
#endif

    OS_List_Node_Init(&sem->_resource_node, sem);
    /*挂载。*/
    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_List_Insert(&OS_Task_Sem_List, NULL, &sem->_resource_node);
    OS_Exit_Protect();
}

OS_Task_Sem OS_Task_Sem_Create(OS_Task task, const char* name, uint32_t init_sem, uint32_t max_sem) {
    OS_Prepare_Protect();
    OS_Task_Sem sem;
    if (OS_Task_Sem_Free_List) {
        /*从资源池中取出任务信号量。*/
        OS_Enter_Protect();
        sem = OS_Task_Sem_Free_List->data;
        OS_List_Erase(&OS_Task_Sem_Free_List, &sem->_resource_node);
        OS_Exit_Protect();
    } else {
        /*动态分配任务信号量。*/
        sem = OS_Malloc(sizeof(OS_Task_SEM));
    }
    /*初始化任务信号量。*/
    if (sem)
        OS_Task_Sem_Init(sem, name, task, init_sem, max_sem);
    return sem;
}

OS_Error_t OS_Task_Sem_Release(OS_Task_Sem sem) {
    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_Error_t error = OS_OK;
    if (sem->_waiting) {
        error = OS_ERROR;
        goto end;
    }
    /*回收资源。*/
    OS_List_Erase(&OS_Task_Sem_List, &sem->_resource_node);
    OS_List_Insert(&OS_Task_Sem_Free_List, NULL, &sem->_resource_node);
end:
    OS_Exit_Protect();
    return error;
}



OS_Error_t OS_Task_Sem_Try_Pend(OS_Task_Sem sem) {
    os_param_assert(sem, OS_ILLEGAL_PARAM);
    os_assert(!OS_In_Intr(), OS_PERMISSION_DENY);
    os_assert(OS_Task_Switchable(), OS_PERMISSION_DENY);
    os_assert(sem->owner == OS_Cur_Task, OS_PERMISSION_DENY);

    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_Error_t error = OS_OK;
    if (sem->_sem_cur)
        /*修改信号量计数。*/
        sem->_sem_cur--;
    else
        error = OS_ERROR;
    OS_Exit_Protect();
    return error;
}


/**
 * @brief 当任务在等待任务信号量被唤醒时调用，标记该任务不再等待任务信号量。
 * @param src NULL。
 * @param arg 正在等待的信号量。
*/
static void os_task_sem_pend_timeout_callback(void* src, void* arg) {
    OS_Task_Sem sem = arg;
    sem->_waiting = false;
}

OS_Error_t OS_Task_Sem_Pend(OS_Task_Sem sem, OS_Tick timeout) {
    os_param_assert(sem, OS_ILLEGAL_PARAM);
    os_assert(!OS_In_Intr(), OS_PERMISSION_DENY);
    os_assert(OS_Task_Switchable(), OS_PERMISSION_DENY);
    os_assert(sem->owner == OS_Cur_Task, OS_PERMISSION_DENY);

    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_Error_t error = OS_OK;
    if (sem->_sem_cur) {
        /*修改信号量计数。*/
        sem->_sem_cur--;
        goto end;
    }
    /*阻塞当前任务。*/
    OS_Task_Start_Waiting(NULL, timeout, os_task_sem_pend_timeout_callback, sem);
    /*标记当前任务正在等待信号量。*/
    sem->_waiting = true;
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

static void os_task_sem_delay_execute_callback(void* src, void* arg) {
    ((OS_Error_t(*)(OS_Task_Sem))arg)(src);
}


OS_Error_t OS_Task_Sem_Post(OS_Task_Sem sem) {
    os_param_assert(sem, OS_ILLEGAL_PARAM);



#if OS_INTR_DELAY_EN
    /*延迟发布。*/
    if (OS_In_Intr())
        return OS_Delay_Execute(os_task_sem_delay_execute_callback, sem, OS_Task_Sem_Post);
#endif
    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_Error_t error = OS_OK;
    if (sem->_waiting) {
        /*如果该任务正在等待信号量，则将其唤醒。*/
        OS_Task_Stop_Waiting(sem->owner, OS_OK);
        goto end;
    }
    if (sem->_sem_cur < sem->_sem_max)
        /*修改信号量计数。*/
        sem->_sem_cur++;
    else
        error = OS_ERROR;
end:
    OS_Exit_Protect();
    if (OS_Need_Sched())
        OS_Sched();
    return error;
}

#if OS_TASK_SEM_NAME_EN
#include "string.h"
OS_Task_Sem OS_Task_Sem_Find(const char* name) {
    OS_Prepare_Protect();
    OS_Enter_Protect();
    bool found = false;
    OS_Task_Sem sem = NULL;
    for (OS_List p = OS_Task_Sem_List; p; p = p->next) {
        sem = p->data;
        if (!strcmp(name, sem->name)) {
            found = true;
            break;
        }
    }
    OS_Exit_Protect();
    return found ? sem : NULL;
}
#endif



#endif