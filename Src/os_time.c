
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
    /*更新系统时间。*/
    os_tick++;
    /*检查超时任务。*/
    if (OS_Need_Check_Timeout())
        OS_Check_Timeout();
}
#endif

void OS_Delay(OS_Tick delay) {
    OS_Task_Suspend(delay);
}
#if OS_TIMER_EN

/**
 * @brief 定时器就绪队列。
*/
static OS_Priority_Queue os_timer_queue;
OS_List OS_Timer_List;
OS_List OS_Timer_Free_List;


static void os_timer_trigger_callback(void* src, void* arg) {
    while (!OS_Priority_Queue_Empty(os_timer_queue) && OS_Priority_Queue_Top(os_timer_queue)->key <= os_tick) {
        /*获取需要触发的定时器。*/
        OS_Timer timer = OS_Priority_Queue_Top(os_timer_queue)->data;
        /*停止定时器。*/
        OS_Timer_Stop(timer);
        /*如果是周期定时器，将其重启。*/
        if (timer->opt == OS_TIMER_OPT_PERIODIC) {
            timer->_rest = timer->period;
            /*防止_rest使用delay。*/
            timer->status = OS_TIMER_STATUS_SUSPEND;
            OS_Timer_Start(timer);
        }
        /*调用定时器回调函数。*/
        timer->func(timer, timer->arg);
    }
}

void OS_Tick_Inc(void) {
    OS_Prepare_Critical();
    OS_Enter_Critical();
    /*更新系统时间。*/
    os_tick++;
    /*检查超时任务。*/
    if (OS_Need_Check_Timeout())
        OS_Check_Timeout();
    if (!OS_Priority_Queue_Empty(os_timer_queue) && OS_Priority_Queue_Top(os_timer_queue)->key <= os_tick) {
#if OS_INTR_DELAY_EN
    /*延迟发布。*/
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
    /*自身属性设置。*/
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

    /*挂载。*/
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
        /*从资源池中取出定时器。*/
        OS_Enter_Protect();
        timer = OS_Timer_Free_List->data;
        OS_List_Erase(&OS_Timer_Free_List, &timer->_resource_node);
        OS_Exit_Protect();
    } else {
        /*动态分配定时器。*/
        timer = OS_Malloc(sizeof(OS_TMR));
    }
    /*初始化定时器。*/
    if(timer)
        OS_Timer_Init(timer, name, opt, delay, period, func, arg);
    return timer;
}


#if OS_INTR_DELAY_EN
/**
 * @brief 作为回调函数调用定时器相关函数。
 * @param src 定时器。
 * @param arg 函数。
*/
static void os_timer_delay_execute_callback(void* src, void* arg) {
    ((OS_Error_t(*)(OS_Timer*))arg)(src);
}
#endif

OS_Error_t OS_Timer_Release(OS_Timer timer) {
    os_param_assert(timer, OS_ILLEGAL_PARAM);



#if OS_INTR_DELAY_EN
    /*延迟发布。*/
    if (OS_In_Intr())
        return OS_Delay_Execute(os_timer_delay_execute_callback, timer, OS_Timer_Release);
#endif


    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_Error_t error = OS_OK;
    /*如果定时器已被销毁，则直接返回。*/
    if (timer->status == OS_TIMER_STATUS_RELEASED)
        goto end;
    /*如果定时器正在运行，则不能销毁。*/
    if (timer->status == OS_TIMER_STATUS_RUNNING) {
        error = OS_ERROR;
        goto end;
    }
    /*回收资源。*/
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
    /*延迟发布。*/
    if (OS_In_Intr())
        return OS_Delay_Execute(os_timer_delay_execute_callback, timer, OS_Timer_Release);
#endif

    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_Error_t error = OS_OK;
    /*如果定时器已被释放，则不能启动。*/
    if (timer->status == OS_TIMER_STATUS_RELEASED) {
        error = OS_ERROR;
        goto end;
    }
    /*如果定时器被暂停，则延时沿用_rest，否则将延时采用delay。*/
    if (timer->status != OS_TIMER_STATUS_SUSPEND)
        timer->_rest = timer->delay;
    /*将定时器加入就绪队列。*/
    OS_Priority_Queue_Node_Init(&timer->_pq_node, OS_Get_Tick_Count() + timer->_rest, timer);
    OS_Priority_Queue_Push(&os_timer_queue, &timer->_pq_node);
    /*标记定时器正在运行。*/
    timer->status = OS_TIMER_STATUS_RUNNING;
end:;
    OS_Exit_Protect();
    return error;
}





OS_Error_t OS_Timer_Stop(OS_Timer timer) {
    os_param_assert(timer, OS_ILLEGAL_PARAM);

#if OS_INTR_DELAY_EN
    /*延迟发布。*/
    if (OS_In_Intr())
        return OS_Delay_Execute(os_timer_delay_execute_callback, timer, OS_Timer_Stop);
#endif


    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_Error_t error = OS_OK;

    /*如果定时器已被释放，则不能停止。*/
    if (timer->status == OS_TIMER_STATUS_RELEASED) {
        error = OS_ERROR;
        goto end;
    }
    /*若定时器正在运行，将其从就绪队列中删除。*/
    if (timer->status == OS_TIMER_STATUS_RUNNING)
        OS_Priority_Queue_Erase(&os_timer_queue, &timer->_pq_node);
    
    /*标记当前定时器已运行结束。*/
    timer->status = OS_TIMER_STATUS_FINISHED;
end:;
    OS_Exit_Protect();
    return error;
}




OS_Error_t OS_Timer_Suspend(OS_Timer timer) {

    os_param_assert(timer, OS_ILLEGAL_PARAM);


#if OS_INTR_DELAY_EN
    /*延迟发布。*/
    if (OS_In_Intr())
        return OS_Delay_Execute(os_timer_delay_execute_callback, timer, OS_Timer_Suspend);
#endif

    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_Error_t error = OS_OK;

    /*如果定时器已被释放，则不能暂停。*/
    if (timer->status == OS_TIMER_STATUS_RELEASED) {
        error = OS_ERROR;
        goto end;
    }
   
    if (timer->status == OS_TIMER_STATUS_RUNNING) {
        /*将定时器从就绪队列中删除。*/
        OS_Priority_Queue_Erase(&os_timer_queue, &timer->_pq_node);
        if (timer->_pq_node.key < OS_Get_Tick_Count()) {
            /*如果定时器已超时，则恢复时立马触发。*/
            timer->_rest = 0;
        } else {
            /*如果定时器未超时，记录剩余时间。*/
            timer->_rest = timer->_pq_node.key - OS_Get_Tick_Count();
        }
    }
    /*标记当前定时器已暂停。*/
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
