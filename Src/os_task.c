#include "os_task.h"
#include "os_cpu.h"
#include "os_mem.h"
#include "os_int.h"


OS_Prio_Table OS_Task_Ready_Table;

OS_Task OS_Cur_Task, OS_Next_Task;

OS_List OS_Task_List;

OS_List OS_Task_Free_List;

/**
 * @brief �ȴ����С�
*/
static OS_Priority_Queue os_task_wait_queue;

/**
 * @brief ���������״̬��־������
*/
static bool os_task_switchable;

/**
 * @brief ��һ���л������ʱ�䡣
*/
static OS_Tick last_switch;
/**
 * @brief ��һ�μ�鳬�������ʱ�䡣
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
    /*��ȡ��ǰʱ�䡣*/
    OS_Tick tick = OS_Get_Tick_Count();
    /*���������̣�����Ҫ��⡣*/
    if (tick - last_check_timeout < OS_CHECK_TIMEOUT_FREQ)
        ret = false;
    /*��û������ʱ������Ҫ��⡣*/
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
    /*�ӳٷ�����*/
    if (OS_In_Intr()) {
        OS_Delay_Execute(os_check_timeout_callback, NULL, NULL);
        return;
    }
#endif
    OS_Prepare_Protect();
    OS_Enter_Protect();
    
    while (!OS_Priority_Queue_Empty(os_task_wait_queue) && OS_Priority_Queue_Top(os_task_wait_queue)->key <= OS_Get_Tick_Count()) {
        /*��ȡ��ʱ������*/
        OS_Task task = OS_Priority_Queue_Top(os_task_wait_queue)->data;
        /*���ѳ�ʱ������*/
        OS_Task_Stop_Waiting(task, OS_TIMEOUT);
    }
    /*���¼�ⳬʱ��ʱ��*/
    last_check_timeout = OS_Get_Tick_Count();
    OS_Exit_Protect();
}

bool OS_Need_Sched(void) {
    OS_Prepare_Protect();
    OS_Enter_Protect();
    bool ret = false;
    /*�����û���л�����������Ҫ�л���*/
    if (!OS_Cur_Task)
        ret = true;
    /*�����ǰ���񲻴�������״̬������Ҫ�л���*/
    if (OS_Cur_Task->status != OS_TASK_STATUS_RUNNING)
        ret = true;
    /*����и������ȼ�����������Ҫ�л���*/
    if (OS_Prio_Table_Get_Prio(&OS_Task_Ready_Table) != OS_Task_Get_Prio(OS_Cur_Task))
        ret = true;
#if OS_TASK_SLICE_EN
    /*���ʱ��Ƭ��ת����������Ҫ�л���*/
    if (OS_Get_Tick_Count() - last_switch >= OS_TASK_SLICE)
        ret = true;
#endif
    OS_Exit_Protect();
    return ret;
}

void OS_Sched_New(void) {
    /*��ȡ��һ�����е�����*/
    OS_Next_Task = OS_Prio_Table_Get_Next(&OS_Task_Ready_Table, OS_Task_Get_Prio(OS_Cur_Task))->data;
    /*�����ǰ�����������У�û�б����������������ھ���״̬��*/
    if (OS_Cur_Task && OS_Cur_Task->status == OS_TASK_STATUS_RUNNING)
        OS_Cur_Task->status = OS_TASK_STATUS_READY;
    /*������һ������*/
    OS_Next_Task->status = OS_TASK_STATUS_RUNNING;
    /*���������л���ʱ��*/
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
    /*���浱ǰ������״̬��*/
    bool old = os_task_switchable;
    /*���õ�����״̬��*/
    os_task_switchable = bl;
    OS_Exit_Critical();
    return old;
}

/**
 * @brief �������ʱ���С�
*/
static void task_end(void) {
    OS_Prepare_Protect();
    OS_Enter_Protect();
    /*���õ�ǰ�����ѽ�����*/
    OS_Cur_Task->status = OS_TASK_STATUS_FINISHED;
    /*����ǰ����Ӿ���������ж�ء�*/
    OS_Prio_Table_UnMount(&OS_Task_Ready_Table, &OS_Cur_Task->ready_node, OS_Task_Get_Prio(OS_Cur_Task));
    OS_Exit_Protect();
    while (1);
}


static void os_task_init_mount_callback(void* src,void* arg) {
    OS_Task task = src;
    OS_Tick delay = (OS_Tick)arg;
    /*���ء�*/
    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_List_Insert(&OS_Task_List, NULL, &task->_resource_node);
    if (delay) {
        /*�����ǰ������Ҫ�ӳ�ִ�С�*/
        task->status = OS_TASK_STATUS_SUSPEND;
        task->_wait_callback_func = NULL;
        task->_wait_callback_arg = NULL;
        task->ready_table = NULL;
        /*����ǰ������ص��ȴ����С�*/
        task->_wait_node.key = delay + OS_Get_Tick_Count();
        OS_Priority_Queue_Push(&os_task_wait_queue, &task->_wait_node);
    } else {
        task->status = OS_TASK_STATUS_READY;
        /*��������ص��������С�*/
        task->ready_table = &OS_Task_Ready_Table;
        OS_Prio_Table_Mount(&OS_Task_Ready_Table, &task->ready_node, OS_Task_Get_Prio(task));

    }
    OS_Exit_Protect();
}

void OS_Task_Init(OS_Task task, const char* name, void(*func)(void*), void* arg, OS_Prio prio, OS_Tick delay, void* stack_buttom, size_t stack_size) {
    /*��������ʱ������ID��*/
    static OS_Task_ID last_id = 0;
    
    os_param_assert_no_return(task);
    os_param_assert_no_return(func);
    os_param_assert_no_return(prio<OS_MAX_PRIO);
    os_param_assert_no_return(stack_buttom);

    /*�����������á�*/



    task->stack_buttom=stack_buttom;
    /*��ʼ������ջ��*/
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

    
    /*����Ƿ���Ҫ���µ��ȡ�*/
    if (OS_Need_Sched())
        OS_Sched();
}

/**
 * @brief ������Դ�ء�
*/
static OS_TCB os_tcb_pool[OS_TASK_PRE_ALLOC_CNT] = {};
OS_Task OS_Task_Create(const char* name, void(*func)(void*), void* arg, OS_Prio prio, OS_Tick delay, void* stack_buttom, size_t stack_size) {
    static bool first = true;
    OS_Prepare_Critical();


    if (first) {
        first = false;
        /*��ʼ����Դ�ء�*/
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
        /*����Դ����ȡ������*/
        OS_Enter_Critical();
        task = OS_Task_Free_List->data;
        OS_List_Erase(&OS_Task_Free_List, &task->_resource_node);
        OS_Exit_Critical();
    } else {
        /*��̬��������*/
        task = OS_Malloc(sizeof(OS_TCB));
    }


    /*��ʼ������*/
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
        /*����趨�˳�ʱʱ�䣬�򽫵�ǰ������ص��ȴ����С�*/
        OS_Cur_Task->_wait_node.key = timeout + OS_Get_Tick_Count();
        OS_Priority_Queue_Push(&os_task_wait_queue, &OS_Cur_Task->_wait_node);
    } else {
        /*����������޵ȴ����򲻹��ء�*/
        OS_Cur_Task->_wait_node.key = 0;
    }

    /*��ȡ��ǰ��������ȼ���*/
    OS_Prio prio = OS_Task_Get_Prio(OS_Cur_Task);

    /*����ǰ����Ӿ���������ж�ء�*/
    OS_Prio_Table_UnMount(&OS_Task_Ready_Table, &OS_Cur_Task->ready_node, prio);
    
    /*����ǰ������ص�ָ���Ķ����ϡ�*/
    OS_Cur_Task->ready_table = ready_table;
    if (ready_table)
        OS_Prio_Table_Mount(ready_table, &OS_Cur_Task->ready_node, prio);
    

    /*���ûص�������*/
    OS_Cur_Task->_wait_callback_func = callback_func;
    OS_Cur_Task->_wait_callback_arg = callback_arg;

    /*����ǰ�����״̬����Ϊ����״̬��*/
    OS_Cur_Task->status = OS_TASK_STATUS_SUSPEND;
}

void OS_Task_Stop_Waiting(OS_Task task, OS_Error_t error) {
    os_param_assert_no_return(task);
    /*�����ǰ�����趨�˳�ʱʱ�䣬����ӵȴ�������ж�ء�*/
    if(task->_wait_node.key)
        OS_Priority_Queue_Erase(&os_task_wait_queue, &task->_wait_node);
    

    task->_wait_node.key = 0;


    /*��ȡ��ǰ��������ȼ���*/
    OS_Prio prio = OS_Task_Get_Prio(task);

    /*�����ǰ���񱻹��ص�ĳ�����ȼ����ϣ�����ж�ء�*/
    if (task->ready_table)
        OS_Prio_Table_UnMount(task->ready_table, &task->ready_node, prio);
    
    /*����ǰ�������¹��ص��������С�*/
    task->ready_table = &OS_Task_Ready_Table;
    OS_Prio_Table_Mount(&OS_Task_Ready_Table, &task->ready_node, prio);
    
    /*���ô�����Ϣ��*/
    task->_error = error;
    
    /*����ǰ�����״̬����Ϊ����״̬��*/
    task->status = OS_TASK_STATUS_READY;
    
    /*���ûص�������*/
    if (task->_wait_callback_func)
        task->_wait_callback_func(task, task->_wait_callback_arg);


}


OS_Error_t OS_Task_Suspend(OS_Tick timeout) {
    os_assert(!OS_In_Intr(), OS_PERMISSION_DENY);
    os_assert(OS_Task_Switchable(), OS_PERMISSION_DENY);
    OS_Prepare_Protect();
    OS_Enter_Protect();
    /*������ǰ����*/
    OS_Task_Start_Waiting(NULL, timeout, NULL, NULL);
    OS_Exit_Protect();
    /*�л�����*/
    OS_Sched();
    return OS_Cur_Task->_error;
}

#if OS_INTR_DELAY_EN
/**
 * @brief ��Ϊ�ص���������������غ�����
 * @param src ����
 * @param arg ������
*/
static void os_task_delay_execute_callback(void* src, void* arg) {
    ((OS_Error_t(*)(OS_Task*))arg)(src);
}
#endif



OS_Error_t OS_Task_Resume(OS_Task task) {
    os_param_assert(task, OS_ILLEGAL_PARAM);

#if OS_INTR_DELAY_EN
    /*�ӳٷ�����*/
    if (OS_In_Intr())
        return OS_Delay_Execute(os_task_delay_execute_callback, task, OS_Task_Resume);
        
#endif
    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_Error_t error = OS_OK;
    /*�����ǰ����û�б����������ָܻ���*/
    if (task->status != OS_TASK_STATUS_SUSPEND) {
        error = OS_ERROR;
        goto end;
    }
    /*����ָ������*/
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
    /*�ӳٷ�����*/
    if (OS_In_Intr())
        return OS_Delay_Execute(os_task_change_prio_delay_execute_callback, task, (void*)prio);
        
#endif
    OS_Prepare_Protect();
    OS_Enter_Protect();
    /*����������ȼ�����ж�ء�*/
    if (task->ready_table)
        OS_Prio_Table_UnMount(task->ready_table, &task->ready_node, OS_Task_Get_Prio(task));
#if OS_MUTEX_EN
    /*���Լ��Ľڵ�����ȶ�����ɾ����*/
    OS_Priority_Queue_Erase(&task->_pq, &task->_pq_node);
    /*�������ȼ���*/
    task->_pq_node.key = prio;
    /*���²���ڵ㡣*/
    OS_Priority_Queue_Push(&task->_pq, &task->_pq_node);
#else
    /*�������ȼ���*/
    task->_prio = prio;
#endif
    /*���½�������ص����ȼ����ϡ�*/
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
    /*����������ȼ�����ж�ء�*/
    if(task->ready_table)
        OS_Prio_Table_UnMount(task->ready_table, &task->ready_node, OS_Task_Get_Prio(task));
    OS_Priority_Queue_Node_Init(node, prio, node->data);
    /*���ػ�������*/
    OS_Priority_Queue_Push(&task->_pq, node);
    /*���½�������ص����ȼ����ϡ�*/
    if (task->ready_table)
        OS_Prio_Table_Mount(task->ready_table, &task->ready_node, OS_Task_Get_Prio(task));
}
void OS_Task_UnMount_Mutex(OS_Task task, OS_Mutex_Prio_Node* node, OS_Prio prio) {
    os_param_assert_no_return(task);
    os_param_assert_no_return(node);
    os_param_assert_no_return(prio < OS_MAX_PRIO);
    /*����������ȼ�����ж�ء�*/
    if (task->ready_table)
        OS_Prio_Table_UnMount(task->ready_table, &task->ready_node, OS_Task_Get_Prio(task));
    /*���ػ�������*/
    OS_Priority_Queue_Erase(&task->_pq, node);
    /*���½�������ص����ȼ����ϡ�*/
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