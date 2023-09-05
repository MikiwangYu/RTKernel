
#include "os_time.h"
#include "os_cpu.h"
#include "os_int.h"
#include "os_task.h"
#include "os_mem.h"




static OS_Tick os_tick;

OS_Tick OS_Get_Tick_Count(void) {
    return os_tick;
}
#if !OS_TIMER_EN
void OS_Tick_Inc(void) {
    /*����ϵͳʱ�䡣*/
    os_tick++;
    /*��鳬ʱ����*/
    if (OS_Need_Check_Timeout())
        OS_Check_Timeout();
}
#endif

void OS_Delay(OS_Tick delay) {
    OS_Task_Suspend(delay);
}
#if OS_TIMER_EN

/**
 * @brief ��ʱ���������С�
*/
static OS_Priority_Queue os_timer_queue;
OS_List OS_Timer_List;
OS_List OS_Timer_Free_List;


static void os_timer_trigger_callback(void* src, void* arg) {
    while (!OS_Priority_Queue_Empty(os_timer_queue) && OS_Priority_Queue_Top(os_timer_queue)->key <= os_tick) {
        /*��ȡ��Ҫ�����Ķ�ʱ����*/
        OS_Timer timer = OS_Priority_Queue_Top(os_timer_queue)->data;
        /*ֹͣ��ʱ����*/
        OS_Timer_Stop(timer);
        /*��������ڶ�ʱ��������������*/
        if (timer->opt == OS_TIMER_OPT_PERIODIC) {
            timer->_rest = timer->period;
            /*��ֹ_restʹ��delay��*/
            timer->status = OS_TIMER_STATUS_SUSPEND;
            OS_Timer_Start(timer);
        }
        /*���ö�ʱ���ص�������*/
        timer->func(timer, timer->arg);
    }
}

void OS_Tick_Inc(void) {
    OS_Prepare_Critical();
    OS_Enter_Critical();
    /*����ϵͳʱ�䡣*/
    os_tick++;
    /*��鳬ʱ����*/
    if (OS_Need_Check_Timeout())
        OS_Check_Timeout();
    if (!OS_Priority_Queue_Empty(os_timer_queue) && OS_Priority_Queue_Top(os_timer_queue)->key <= os_tick) {
#if OS_INTR_DELAY_EN
    /*�ӳٷ�����*/
        OS_Delay_Execute(os_timer_trigger_callback, NULL, NULL);
#else
        os_timer_trigger_callback(NULL, NULL);
#endif
    }
    OS_Exit_Critical();
}


void OS_Timer_Init(
    OS_Timer timer,
    const char* name,
    OS_Timer_OPT opt,
    OS_Tick delay,
    OS_Tick period,
    OS_Callback_Func_t func,
    OS_Callback_Arg_t arg
) {
    /*�����������á�*/
#if OS_TIMER_NAME_EN
    timer->name = name ? name : "NULL";
#endif
    timer->opt = opt;
    timer->delay = delay;
    timer->period = period;
    timer->func = func;
    timer->arg = arg;


    timer->status = OS_TIMER_STATUS_READY;
    
    timer->_rest = timer->delay;

    OS_List_Node_Init(&timer->_resource_node, timer);

    /*���ء�*/
    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_List_Insert(&OS_Timer_List, NULL, &timer->_resource_node);
    OS_Exit_Protect();
}

OS_Timer OS_Timer_Create(
    const char* name,
    OS_Timer_OPT opt,
    OS_Tick delay,
    OS_Tick period,
    OS_Callback_Func_t func,
    OS_Callback_Arg_t arg
) {
    OS_Prepare_Protect();
    OS_Timer timer;
    if (OS_Timer_Free_List) {
        /*����Դ����ȡ����ʱ����*/
        OS_Enter_Protect();
        timer = OS_Timer_Free_List->data;
        OS_List_Erase(&OS_Timer_Free_List, &timer->_resource_node);
        OS_Exit_Protect();
    } else {
        /*��̬���䶨ʱ����*/
        timer = OS_Malloc(sizeof(OS_TMR));
    }
    /*��ʼ����ʱ����*/
    if(timer)
        OS_Timer_Init(timer, name, opt, delay, period, func, arg);
    return timer;
}


#if OS_INTR_DELAY_EN
/**
 * @brief ��Ϊ�ص��������ö�ʱ����غ�����
 * @param src ��ʱ����
 * @param arg ������
*/
static void os_timer_delay_execute_callback(void* src, void* arg) {
    ((OS_Error_t(*)(OS_Timer*))arg)(src);
}
#endif

OS_Error_t OS_Timer_Release(OS_Timer timer) {
    os_param_assert(timer, OS_ILLEGAL_PARAM);



#if OS_INTR_DELAY_EN
    /*�ӳٷ�����*/
    if (OS_In_Intr())
        return OS_Delay_Execute(os_timer_delay_execute_callback, timer, OS_Timer_Release);
#endif


    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_Error_t error = OS_OK;
    /*�����ʱ���ѱ����٣���ֱ�ӷ��ء�*/
    if (timer->status == OS_TIMER_STATUS_RELEASED)
        goto end;
    /*�����ʱ���������У��������١�*/
    if (timer->status == OS_TIMER_STATUS_RUNNING) {
        error = OS_ERROR;
        goto end;
    }
    /*������Դ��*/
    OS_List_Erase(&OS_Timer_List, &timer->_resource_node);
    OS_List_Insert(&OS_Timer_Free_List, NULL, &timer->_resource_node);
    timer->status = OS_TIMER_STATUS_RELEASED;
end:;
    OS_Exit_Protect();
    return error;
}





OS_Error_t OS_Timer_Start(OS_Timer timer) {
    os_param_assert(timer, OS_ILLEGAL_PARAM);



#if OS_INTR_DELAY_EN
    /*�ӳٷ�����*/
    if (OS_In_Intr())
        return OS_Delay_Execute(os_timer_delay_execute_callback, timer, OS_Timer_Release);
#endif

    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_Error_t error = OS_OK;
    /*�����ʱ���ѱ��ͷţ�����������*/
    if (timer->status == OS_TIMER_STATUS_RELEASED) {
        error = OS_ERROR;
        goto end;
    }
    /*�����ʱ������ͣ������ʱ����_rest��������ʱ����delay��*/
    if (timer->status != OS_TIMER_STATUS_SUSPEND)
        timer->_rest = timer->delay;
    /*����ʱ������������С�*/
    OS_Priority_Queue_Node_Init(&timer->_pq_node, OS_Get_Tick_Count() + timer->_rest, timer);
    OS_Priority_Queue_Push(&os_timer_queue, &timer->_pq_node);
    /*��Ƕ�ʱ���������С�*/
    timer->status = OS_TIMER_STATUS_RUNNING;
end:;
    OS_Exit_Protect();
    return error;
}





OS_Error_t OS_Timer_Stop(OS_Timer timer) {
    os_param_assert(timer, OS_ILLEGAL_PARAM);

#if OS_INTR_DELAY_EN
    /*�ӳٷ�����*/
    if (OS_In_Intr())
        return OS_Delay_Execute(os_timer_delay_execute_callback, timer, OS_Timer_Stop);
#endif


    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_Error_t error = OS_OK;

    /*�����ʱ���ѱ��ͷţ�����ֹͣ��*/
    if (timer->status == OS_TIMER_STATUS_RELEASED) {
        error = OS_ERROR;
        goto end;
    }
    /*����ʱ���������У�����Ӿ���������ɾ����*/
    if (timer->status == OS_TIMER_STATUS_RUNNING)
        OS_Priority_Queue_Erase(&os_timer_queue, &timer->_pq_node);
    
    /*��ǵ�ǰ��ʱ�������н�����*/
    timer->status = OS_TIMER_STATUS_FINISHED;
end:;
    OS_Exit_Protect();
    return error;
}




OS_Error_t OS_Timer_Suspend(OS_Timer timer) {

    os_param_assert(timer, OS_ILLEGAL_PARAM);


#if OS_INTR_DELAY_EN
    /*�ӳٷ�����*/
    if (OS_In_Intr())
        return OS_Delay_Execute(os_timer_delay_execute_callback, timer, OS_Timer_Suspend);
#endif

    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_Error_t error = OS_OK;

    /*�����ʱ���ѱ��ͷţ�������ͣ��*/
    if (timer->status == OS_TIMER_STATUS_RELEASED) {
        error = OS_ERROR;
        goto end;
    }
   
    if (timer->status == OS_TIMER_STATUS_RUNNING) {
        /*����ʱ���Ӿ���������ɾ����*/
        OS_Priority_Queue_Erase(&os_timer_queue, &timer->_pq_node);
        if (timer->_pq_node.key < OS_Get_Tick_Count()) {
            /*�����ʱ���ѳ�ʱ����ָ�ʱ��������*/
            timer->_rest = 0;
        } else {
            /*�����ʱ��δ��ʱ����¼ʣ��ʱ�䡣*/
            timer->_rest = timer->_pq_node.key - OS_Get_Tick_Count();
        }
    }
    /*��ǵ�ǰ��ʱ������ͣ��*/
    timer->status = OS_TIMER_STATUS_SUSPEND;
end:;
    OS_Exit_Protect();
    return error;
}






#if OS_TIMER_NAME_EN
#include "string.h"
OS_Timer OS_Timer_Find(const char* name) {
    OS_Prepare_Protect();
    OS_Enter_Protect();
    bool found = false;
    OS_Timer timer = NULL;
    for (OS_List p = OS_Task_List; p; p = p->next) {
        timer = p->data;
        if (!strcmp(name, timer->name)) {
            found = true;
            break;
        }
    }
    OS_Exit_Protect();
    return found ? timer : NULL;
}
#endif
#endif
