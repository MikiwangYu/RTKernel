#include "os_evt.h"
#include "os_cpu.h"
#include "os_int.h"
#include "os_mem.h"
#if OS_EVENT_EN


OS_List OS_Event_List;
OS_List OS_Event_Free_List;

/**
 * @brief �¼���Դ�ء�
*/
static OS_ECB os_ecb_pool[OS_EVENT_PRE_ALLOC_CNT];

void OS_Event_Init(OS_Event event, const char* name) {
    /*�����������á�*/
    event->type = OS_EVENT_TYPE_RAW;
#if OS_EVENT_NAME_EN
    event->name = name ? name : "NULL";
#endif
    OS_List_Node_Init(&event->_resource_node, event);
    OS_Prio_Table_Init(&event->_ready_table);
    /*���ء�*/
    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_List_Insert(&OS_Event_List, NULL, &event->_resource_node);
    OS_Exit_Protect();
}

OS_Event OS_Event_Create(const char* name) {
    static bool first = true;
    OS_Prepare_Protect();
    if (first) {
        first = false;
        /*��ʼ����Դ�ء�*/
        OS_Enter_Protect();
        OS_List_Init(&OS_Event_Free_List);
        for (int i = 0; i < OS_EVENT_PRE_ALLOC_CNT; i++) {
            OS_List_Node_Init(&os_ecb_pool[i]._resource_node, os_ecb_pool + i);
            OS_List_Insert(&OS_Event_Free_List, NULL, &os_ecb_pool[i]._resource_node);
        }
        OS_Exit_Protect();
    }


    OS_Event event = NULL;
    if (OS_Event_Free_List) {
        /*����Դ����ȡ���¼���*/
        OS_Enter_Protect();
        event = OS_Event_Free_List->data;
        OS_List_Erase(&OS_Event_Free_List, &event->_resource_node);
        OS_Exit_Protect();
    } else {
        /*��̬�����¼���*/
        event = OS_Malloc(sizeof(OS_ECB));
    }
    /*��ʼ���¼���*/
    if (event)
        OS_Event_Init(event, name);
    return event;
}
/**
 * @brief ��Ϊ�ص����������¼���غ�����
 * @param src �¼���
 * @param arg ������
*/
static void os_event_delay_execute_callback(void* src, void* arg) {
    ((OS_Error_t(*)(OS_Event*))arg)(src);
}

OS_Error_t OS_Event_Release(OS_Event event) {
    os_param_assert(event, OS_ILLEGAL_PARAM);
#if OS_INTR_DELAY_EN
    /*�ӳٷ�����*/
    if (OS_In_Intr())
        return OS_Delay_Execute(os_event_delay_execute_callback, event, OS_Event_Release);
#endif

    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_Error_t error = OS_OK;
    /*������������ڵȴ��¼����������١�*/
    if (OS_Prio_Table_Get_Prio(&event->_ready_table) == OS_MAX_PRIO) {
        error = OS_ERROR;
        goto end;
    }
    /*������Դ��*/
    OS_List_Erase(&OS_Event_List, &event->_resource_node);
    OS_List_Insert(&OS_Event_Free_List, NULL, &event->_resource_node);
end:
    OS_Exit_Protect();
    return error;
}


OS_Error_t OS_Event_Wait(OS_Event event, OS_Tick timeout) {
    os_param_assert(event, OS_ILLEGAL_PARAM);
    os_param_assert(event->type == OS_EVENT_TYPE_RAW, OS_ILLEGAL_PARAM);

    os_assert(!OS_In_Intr(), OS_PERMISSION_DENY);
    os_assert(OS_Task_Switchable(), OS_PERMISSION_DENY);

    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_Error_t error = OS_OK;
    /*������ǰ����*/
    OS_Task_Start_Waiting(&event->_ready_table, timeout, NULL, NULL);
    OS_Exit_Protect();
    /*�л�����*/
    OS_Sched();

    OS_Enter_Protect();
    /*���õȴ���ķ���ֵ��*/
    error = OS_Cur_Task->_error;
    OS_Exit_Protect();
    return error;
}

OS_Error_t OS_Event_Notify(OS_Event event) {
    os_param_assert(event, OS_ILLEGAL_PARAM);
    os_param_assert(event->type == OS_EVENT_TYPE_RAW, OS_ILLEGAL_PARAM);

#if OS_INTR_DELAY_EN
    /*�ӳٷ�����*/
    if (OS_In_Intr())
        return OS_Delay_Execute(os_event_delay_execute_callback, event, OS_Event_Notify);
#endif
    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_Error_t error = OS_OK;
    /*�����ȼ���ߵ������ѡ�*/
    if (OS_Prio_Table_Get_Prio(&event->_ready_table) != OS_MAX_PRIO)
        OS_Task_Stop_Waiting(OS_Prio_Table_Get_Next(&event->_ready_table, OS_MAX_PRIO)->data, OS_OK);
    OS_Exit_Protect();
    /*����Ƿ���Ҫ���µ��ȡ�*/
    if (OS_Need_Sched())
        OS_Sched();
    return error;
}


OS_Error_t OS_Event_NotifyAll(OS_Event event) {
    os_param_assert(event, OS_ILLEGAL_PARAM);
    os_param_assert(event->type == OS_EVENT_TYPE_RAW, OS_ILLEGAL_PARAM);

#if OS_INTR_DELAY_EN
    /*�ӳٷ�����*/
    if (OS_In_Intr())
        return OS_Delay_Execute(os_event_delay_execute_callback, event, OS_Event_NotifyAll);
#endif
    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_Error_t error = OS_OK;
    /*�����������ѡ�*/
    while (OS_Prio_Table_Get_Prio(&event->_ready_table) != OS_MAX_PRIO)
        OS_Task_Stop_Waiting(OS_Prio_Table_Get_Next(&event->_ready_table, OS_MAX_PRIO)->data, OS_OK);
    OS_Exit_Protect();
    /*����Ƿ���Ҫ���µ��ȡ�*/
    if (OS_Need_Sched())
        OS_Sched();
    return error;
}
#endif

#if OS_EVENT_NAME_EN
#include "string.h"
OS_Event OS_Event_Find(const char* name) {
    OS_Prepare_Protect();
    OS_Enter_Protect();
    bool found = false;
    OS_Event event = NULL;
    for (OS_List p = OS_Event_List; p; p = p->next) {
        event = p->data;
        if (!strcmp(name, event->name)) {
            found = true;
            break;
        }
    }
    OS_Exit_Protect();
    return found ? event : NULL;
}
#endif