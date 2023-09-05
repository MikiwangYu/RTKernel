#include "os_task.h"
#include "os_cpu.h"
#include "os_mem.h"
#include "os_int.h"


OS_Prio_Table OS_Task_Ready_Table;

OS_Task OS_Cur_Task, OS_Next_Task;

OS_List OS_Task_List;

OS_List OS_Task_Free_List;

/**
 * @brief 等待队列。
*/
static OS_Priority_Queue os_task_wait_queue;

/**
 * @brief 任务调度器状态标志变量。
*/
static bool os_task_switchable;

/**
 * @brief 上一次切换任务的时间。
*/
static OS_Tick last_switch;
/**
 * @brief 上一次检查超市任务的时间。
*/
static OS_Tick last_check_timeout;

bool OS_Task_Stack_Overflow(OS_Task task) {
#if OS_STACK_CHECK_EN
    return task->stack_p < task->stack_buttom;
#else
    return false;
#endif
}

bool OS_Need_Check_Timeout(void) {
    OS_Prepare_Protect();
    OS_Enter_Protect();
    bool ret = true;
    /*获取当前时间。*/
    OS_Tick tick = OS_Get_Tick_Count();
    /*如果间隔过短，则不需要检测。*/
    if (tick - last_check_timeout < OS_CHECK_TIMEOUT_FREQ)
        ret = false;
    /*若没有任务超时，则不需要检测。*/
    if (tick < OS_Priority_Queue_Top(os_task_wait_queue)->key)
        ret = false;
    OS_Exit_Protect();
    return ret;
}

#if OS_INTR_DELAY_EN
static void os_check_timeout_callback(void* src, void* arg) {
    OS_Check_Timeout();
}
#endif

void OS_Check_Timeout(void) {
#if OS_INTR_DELAY_EN
    /*延迟发布。*/
    if (OS_In_Intr()) {
        OS_Delay_Execute(os_check_timeout_callback, NULL, NULL);
        return;
    }
#endif
    OS_Prepare_Protect();
    OS_Enter_Protect();
    
    while (!OS_Priority_Queue_Empty(os_task_wait_queue) && OS_Priority_Queue_Top(os_task_wait_queue)->key <= OS_Get_Tick_Count()) {
        /*获取超时的任务。*/
        OS_Task task = OS_Priority_Queue_Top(os_task_wait_queue)->data;
        /*唤醒超时的任务*/
        OS_Task_Stop_Waiting(task, OS_TIMEOUT);
    }
    /*更新检测超时的时间*/
    last_check_timeout = OS_Get_Tick_Count();
    OS_Exit_Protect();
}

bool OS_Need_Sched(void) {
    OS_Prepare_Protect();
    OS_Enter_Protect();
    bool ret = false;
    /*如果还没有切换过任务，则需要切换。*/
    if (!OS_Cur_Task)
        ret = true;
    /*如果当前任务不处于运行状态，则需要切换。*/
    if (OS_Cur_Task->status != OS_TASK_STATUS_RUNNING)
        ret = true;
    /*如果有更高优先级的任务，则需要切换。*/
    if (OS_Prio_Table_Get_Prio(&OS_Task_Ready_Table) != OS_Task_Get_Prio(OS_Cur_Task))
        ret = true;
#if OS_TASK_SLICE_EN
    /*如果时间片轮转结束，则需要切换。*/
    if (OS_Get_Tick_Count() - last_switch >= OS_TASK_SLICE)
        ret = true;
#endif
    OS_Exit_Protect();
    return ret;
}

void OS_Sched_New(void) {
    /*获取下一个运行的任务。*/
    OS_Next_Task = OS_Prio_Table_Get_Next(&OS_Task_Ready_Table, OS_Task_Get_Prio(OS_Cur_Task))->data;
    /*如果当前任务正在运行（没有被阻塞），则将其置于就绪状态。*/
    if (OS_Cur_Task && OS_Cur_Task->status == OS_TASK_STATUS_RUNNING)
        OS_Cur_Task->status = OS_TASK_STATUS_READY;
    /*运行下一个任务。*/
    OS_Next_Task->status = OS_TASK_STATUS_RUNNING;
    /*更新任务切换的时间*/
    last_switch = OS_Get_Tick_Count();
}

void OS_Sched(void) {
    os_assert_no_return(!OS_In_Intr());
    os_assert_no_return(OS_Task_Switchable());
    OS_Prepare_Critical();
    OS_Enter_Critical();
    OS_Sched_New();
    OS_Task_Switch();
    OS_Exit_Critical();
}

void OS_Task_Erase(void) {
    
}


bool OS_Task_Switchable(void) {
    return os_task_switchable;
}

bool OS_Task_Set_Switchable(bool bl) {
    OS_Prepare_Critical();
    OS_Enter_Critical();
    /*保存当前调度器状态。*/
    bool old = os_task_switchable;
    /*设置调度器状态。*/
    os_task_switchable = bl;
    OS_Exit_Critical();
    return old;
}

/**
 * @brief 任务结束时运行。
*/
static void task_end(void) {
    OS_Prepare_Protect();
    OS_Enter_Protect();
    /*设置当前任务已结束。*/
    OS_Cur_Task->status = OS_TASK_STATUS_FINISHED;
    /*将当前任务从就绪队列上卸载。*/
    OS_Prio_Table_UnMount(&OS_Task_Ready_Table, &OS_Cur_Task->ready_node, OS_Task_Get_Prio(OS_Cur_Task));
    OS_Exit_Protect();
    while (1);
}


static void os_task_init_mount_callback(void* src,void* arg) {
    OS_Task task = src;
    OS_Tick delay = (OS_Tick)arg;
    /*挂载。*/
    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_List_Insert(&OS_Task_List, NULL, &task->_resource_node);
    if (delay) {
        /*如果当前任务需要延迟执行。*/
        task->status = OS_TASK_STATUS_SUSPEND;
        task->_wait_callback_func = NULL;
        task->_wait_callback_arg = NULL;
        task->ready_table = NULL;
        /*将当前任务挂载到等待队列。*/
        task->_wait_node.key = delay + OS_Get_Tick_Count();
        OS_Priority_Queue_Push(&os_task_wait_queue, &task->_wait_node);
    } else {
        task->status = OS_TASK_STATUS_READY;
        /*将任务挂载到就绪队列。*/
        task->ready_table = &OS_Task_Ready_Table;
        OS_Prio_Table_Mount(&OS_Task_Ready_Table, &task->ready_node, OS_Task_Get_Prio(task));

    }
    OS_Exit_Protect();
}

void OS_Task_Init(OS_Task task, const char* name, void(*func)(void*), void* arg, OS_Prio prio, OS_Tick delay, void* stack_buttom, size_t stack_size) {
    /*创建任务时的任务ID。*/
    static OS_Task_ID last_id = 0;
    
    os_param_assert_no_return(task);
    os_param_assert_no_return(func);
    os_param_assert_no_return(prio<OS_MAX_PRIO);
    os_param_assert_no_return(stack_buttom);

    /*自身属性设置。*/



    task->stack_buttom=stack_buttom;
    /*初始化任务栈。*/
    task->stack_p = OS_Task_Stack_Init(stack_buttom + stack_size, func, task_end, arg);
    task->id = last_id++;
    task->ready_table = NULL;
    OS_Prio_Table_Node_Init(&task->ready_node, task);
#if OS_TASK_NAME_EN
    task->name = name ? name : "NULL";
#endif




    OS_Priority_Queue_Node_Init(&task->_wait_node, 0, task);
    task->_wait_callback_func = NULL;
    task->_wait_callback_arg = NULL;
    OS_List_Node_Init(&task->_resource_node, task);


#if OS_MUTEX_EN
    OS_Priority_Queue_Init(&task->_pq);
    OS_Priority_Queue_Node_Init(&task->_pq_node, prio, task);
    OS_Priority_Queue_Push(&task->_pq, &task->_pq_node);
#else
    task->_prio = prio;
#endif
    task->_error = OS_OK;

#if OS_INTR_DELAY_EN
    if (OS_In_Intr())
        OS_Delay_Execute(os_task_init_mount_callback, task, (void*)delay);
    else
        os_task_init_mount_callback(task, (void*)delay);
#else
    os_task_init_mount_callback(task, delay);
#endif

    
    /*检查是否需要重新调度。*/
    if (OS_Need_Sched())
        OS_Sched();
}

/**
 * @brief 任务资源池。
*/
static OS_TCB os_tcb_pool[OS_TASK_PRE_ALLOC_CNT] = {};
OS_Task OS_Task_Create(const char* name, void(*func)(void*), void* arg, OS_Prio prio, OS_Tick delay, void* stack_buttom, size_t stack_size) {
    static bool first = true;
    OS_Prepare_Critical();


    if (first) {
        first = false;
        /*初始化资源池。*/
        OS_Enter_Critical();
        OS_List_Init(&OS_Task_Free_List);
        for (int i = 0; i < OS_TASK_PRE_ALLOC_CNT; i++) {
            OS_List_Node_Init(&os_tcb_pool[i]._resource_node, os_tcb_pool + i);
            OS_List_Insert(&OS_Task_Free_List, NULL, &os_tcb_pool[i]._resource_node);
        }
        OS_Exit_Critical();
    }


    OS_Task task = NULL;
    if (OS_Task_Free_List) {
        /*从资源池中取出任务。*/
        OS_Enter_Critical();
        task = OS_Task_Free_List->data;
        OS_List_Erase(&OS_Task_Free_List, &task->_resource_node);
        OS_Exit_Critical();
    } else {
        /*动态分配任务。*/
        task = OS_Malloc(sizeof(OS_TCB));
    }


    /*初始化任务。*/
    if (task)
        OS_Task_Init(task, name, func, arg, prio, delay, stack_buttom, stack_size);
    return task;
}






OS_Prio OS_Task_Get_Prio(OS_Task task) {
    os_param_assert(task, OS_MAX_PRIO);
#if OS_MUTEX_EN
    return OS_Priority_Queue_Top(task->_pq)->key;
#else
    return task->_prio;
#endif
}






void OS_Task_Start_Waiting(OS_Prio_Table *ready_table, OS_Tick timeout, OS_Callback_Func_t callback_func,OS_Callback_Arg_t callback_arg) {
    if (timeout) {
        /*如果设定了超时时间，则将当前任务挂载到等待队列。*/
        OS_Cur_Task->_wait_node.key = timeout + OS_Get_Tick_Count();
        OS_Priority_Queue_Push(&os_task_wait_queue, &OS_Cur_Task->_wait_node);
    } else {
        /*如果可以无限等待，则不挂载。*/
        OS_Cur_Task->_wait_node.key = 0;
    }

    /*获取当前任务的优先级。*/
    OS_Prio prio = OS_Task_Get_Prio(OS_Cur_Task);

    /*将当前任务从就绪队列上卸载。*/
    OS_Prio_Table_UnMount(&OS_Task_Ready_Table, &OS_Cur_Task->ready_node, prio);
    
    /*将当前任务挂载到指定的队列上。*/
    OS_Cur_Task->ready_table = ready_table;
    if (ready_table)
        OS_Prio_Table_Mount(ready_table, &OS_Cur_Task->ready_node, prio);
    

    /*设置回调函数。*/
    OS_Cur_Task->_wait_callback_func = callback_func;
    OS_Cur_Task->_wait_callback_arg = callback_arg;

    /*将当前任务的状态设置为阻塞状态。*/
    OS_Cur_Task->status = OS_TASK_STATUS_SUSPEND;
}

void OS_Task_Stop_Waiting(OS_Task task, OS_Error_t error) {
    os_param_assert_no_return(task);
    /*如果当前任务设定了超时时间，则将其从等待队列上卸载。*/
    if(task->_wait_node.key)
        OS_Priority_Queue_Erase(&os_task_wait_queue, &task->_wait_node);
    

    task->_wait_node.key = 0;


    /*获取当前任务的优先级。*/
    OS_Prio prio = OS_Task_Get_Prio(task);

    /*如果当前任务被挂载到某个优先级表上，将其卸载。*/
    if (task->ready_table)
        OS_Prio_Table_UnMount(task->ready_table, &task->ready_node, prio);
    
    /*将当前任务重新挂载到就绪队列。*/
    task->ready_table = &OS_Task_Ready_Table;
    OS_Prio_Table_Mount(&OS_Task_Ready_Table, &task->ready_node, prio);
    
    /*设置错误信息。*/
    task->_error = error;
    
    /*将当前任务的状态设置为就绪状态。*/
    task->status = OS_TASK_STATUS_READY;
    
    /*调用回调函数。*/
    if (task->_wait_callback_func)
        task->_wait_callback_func(task, task->_wait_callback_arg);


}


OS_Error_t OS_Task_Suspend(OS_Tick timeout) {
    os_assert(!OS_In_Intr(), OS_PERMISSION_DENY);
    os_assert(OS_Task_Switchable(), OS_PERMISSION_DENY);
    OS_Prepare_Protect();
    OS_Enter_Protect();
    /*阻塞当前任务。*/
    OS_Task_Start_Waiting(NULL, timeout, NULL, NULL);
    OS_Exit_Protect();
    /*切换任务。*/
    OS_Sched();
    return OS_Cur_Task->_error;
}

#if OS_INTR_DELAY_EN
/**
 * @brief 作为回调函数调用任务相关函数。
 * @param src 任务。
 * @param arg 函数。
*/
static void os_task_delay_execute_callback(void* src, void* arg) {
    ((OS_Error_t(*)(OS_Task*))arg)(src);
}
#endif



OS_Error_t OS_Task_Resume(OS_Task task) {
    os_param_assert(task, OS_ILLEGAL_PARAM);

#if OS_INTR_DELAY_EN
    /*延迟发布。*/
    if (OS_In_Intr())
        return OS_Delay_Execute(os_task_delay_execute_callback, task, OS_Task_Resume);
        
#endif
    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_Error_t error = OS_OK;
    /*如果当前任务没有被阻塞，则不能恢复。*/
    if (task->status != OS_TASK_STATUS_SUSPEND) {
        error = OS_ERROR;
        goto end;
    }
    /*唤醒指定任务。*/
    OS_Task_Stop_Waiting(task, OS_OK);
end:
    OS_Exit_Protect();
    return error;
}

#if OS_INTR_DELAY_EN
void os_task_change_prio_delay_execute_callback(void* src, void* arg) {
    OS_Task_Change_Prio(src, (OS_Prio)arg);
}
#endif

OS_Error_t OS_Task_Change_Prio(OS_Task task, OS_Prio prio) {

    os_param_assert(task, OS_ILLEGAL_PARAM);
    os_param_assert(prio < OS_MAX_PRIO, OS_ILLEGAL_PARAM);

#if OS_INTR_DELAY_EN
    /*延迟发布。*/
    if (OS_In_Intr())
        return OS_Delay_Execute(os_task_change_prio_delay_execute_callback, task, (void*)prio);
        
#endif
    OS_Prepare_Protect();
    OS_Enter_Protect();
    /*将任务从优先级表上卸载。*/
    if (task->ready_table)
        OS_Prio_Table_UnMount(task->ready_table, &task->ready_node, OS_Task_Get_Prio(task));
#if OS_MUTEX_EN
    /*将自己的节点从优先队列中删除。*/
    OS_Priority_Queue_Erase(&task->_pq, &task->_pq_node);
    /*更改优先级。*/
    task->_pq_node.key = prio;
    /*重新插入节点。*/
    OS_Priority_Queue_Push(&task->_pq, &task->_pq_node);
#else
    /*更改优先级。*/
    task->_prio = prio;
#endif
    /*重新将任务挂载到优先级表上。*/
    if (task->ready_table)
        OS_Prio_Table_Mount(task->ready_table, &task->ready_node, OS_Task_Get_Prio(task));

    OS_Exit_Protect();
    return OS_OK;
}

#if OS_MUTEX_EN

void OS_Mutex_Prio_Node_Init(OS_Mutex_Prio_Node* p, void* data) {
    OS_Priority_Queue_Node_Init(p, 0, data);
}

void OS_Task_Mount_Mutex(OS_Task task, OS_Mutex_Prio_Node* node, OS_Prio prio) {
    os_param_assert_no_return(task);
    os_param_assert_no_return(node);
    os_param_assert_no_return(prio < OS_MAX_PRIO);
    /*将任务从优先级表上卸载。*/
    if(task->ready_table)
        OS_Prio_Table_UnMount(task->ready_table, &task->ready_node, OS_Task_Get_Prio(task));
    OS_Priority_Queue_Node_Init(node, prio, node->data);
    /*挂载互斥量。*/
    OS_Priority_Queue_Push(&task->_pq, node);
    /*重新将任务挂载到优先级表上。*/
    if (task->ready_table)
        OS_Prio_Table_Mount(task->ready_table, &task->ready_node, OS_Task_Get_Prio(task));
}
void OS_Task_UnMount_Mutex(OS_Task task, OS_Mutex_Prio_Node* node, OS_Prio prio) {
    os_param_assert_no_return(task);
    os_param_assert_no_return(node);
    os_param_assert_no_return(prio < OS_MAX_PRIO);
    /*将任务从优先级表上卸载。*/
    if (task->ready_table)
        OS_Prio_Table_UnMount(task->ready_table, &task->ready_node, OS_Task_Get_Prio(task));
    /*挂载互斥量。*/
    OS_Priority_Queue_Erase(&task->_pq, node);
    /*重新将任务挂载到优先级表上。*/
    if (task->ready_table)
        OS_Prio_Table_Mount(task->ready_table, &task->ready_node, OS_Task_Get_Prio(task));

}
#endif

#if OS_TASK_NAME_EN
#include "string.h"
OS_Task OS_Find_Task(const char* name) {
    OS_Prepare_Protect();
    OS_Enter_Protect();
    bool found = false;
    OS_Task task = NULL;
    for (OS_List p = OS_Task_List; p; p = p->next) {
        task = p->data;
        if (!strcmp(name, task->name)) {
            found = true;
            break;
        }
    }
    OS_Exit_Protect();
    return found ? task : NULL;
}
#endif