#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "os_def.h"
#if OS_EVENT_EN
#include "os_task.h"
/**
 * @brief 事件类型。
*/
typedef enum _OS_Event_Type {
    /**
     * @brief 裸事件。
    */
    OS_EVENT_TYPE_RAW = 0,
    /**
     * @brief 信号量。
    */
    OS_EVENT_TYPE_SEM,
    /**
     * @brief IO缓冲。
    */
    OS_EVENT_TYPE_IOBUF,
#if OS_MUTEX_EN
    /**
     * @brief 互斥量。
    */
    OS_EVENT_TYPE_MUTEX
#endif
}OS_Event_Type;



/**
 * @brief 事件控制块。
*/
typedef struct _OS_ECB {
    /**
     * @brief 事件类型。
    */
    OS_Event_Type type;
#if OS_EVENT_NAME_EN
    /**
     * @brief 事件名称。
    */
    const char* name;
#endif
    union {
        /*信号量。*/
        struct {
            /**
             * @brief 信号量当前值。
            */
            uint32_t _sem_cur;
            /**
             * @brief 信号量最大值。
            */
            uint32_t _sem_max;
        };

        /*IO缓冲。*/
        struct {
            /**
             * @brief 缓冲区。
            */
            uint8_t* _iobuf_data_p;
            /**
             * @brief 缓冲区大小。
            */
            uint32_t _iobuf_data_size;
            /**
             * @brief 队列头。操作时需与中断互斥。
            */
            uint32_t _iobuf_queue_s;
            /**
             * @brief 队列长度。操作时需与中断互斥。
            */
            uint32_t _iobuf_queue_size;
            /**
             * @brief 已刷新的队列长度。
            */
            uint32_t _iobuf_queue_flushed_size;
        };

#if OS_MUTEX_EN
        /*互斥量。*/
        struct {
            /**
             * @brief 互斥量是否被锁定。
            */
            bool _mutex_locked;
            /**
             * @brief 互斥量优先级。
            */
            OS_Prio _mutex_prio;
            /**
             * @brief 互斥量优先级节点。
            */
            OS_Mutex_Prio_Node _mutex_node;
        };
#endif
    };
    /**
     * @brief 资源链表节点，事件被销毁后会利用此节点挂载到OS_Event_Free_List, 未被销毁的事件会利用此节点挂载到OS_Event_List。
    */
    OS_List_Node _resource_node;
    /**
     * @brief 事件就绪队列。
    */
    OS_Prio_Table _ready_table;
}OS_ECB,*OS_Event;

/**
 * @brief 所有未被销毁的事件的链表.
*/
extern OS_List OS_Event_List;
/**
 * @brief 所有已被销毁的事件的链表。
*/
extern OS_List OS_Event_Free_List;
/**
 * @brief 初始化一个事件。
 * @param event 需要初始化的事件。
 * @param name 事件名称。
*/
void OS_Event_Init(OS_Event event, const char* name);
/**
 * @brief 创建一个事件。
 * @param name 事件名称。
 * @return 如果创建成功，返回事件指针；否则返回NULL。
*/
OS_Event OS_Event_Create(const char* name);
/**
 * @brief 销毁一个事件。
 * @param event 需要销毁的事件。
 * @return OS_OK，如果销毁成功。
*/
OS_Error_t OS_Event_Release(OS_Event event);
/**
 * @brief 阻塞当前任务，将当前任务加入到事件的就绪队列中，直到超时或者被唤醒。
 * @param event 需要操作的事件。
 * @param timeout 如果为0，则无限等待，否则等待timeout个tick后返回。
 * @return OS_OK，如果被正常唤醒；OS_TIMEOUT，如果等待超时。
*/
OS_Error_t OS_Event_Wait(OS_Event event, OS_Tick timeout);
/**
 * @brief 唤醒事件就绪队列中的一个任务。如果处于中断中且开启了中断延迟发布，则会将此操作发送到主线程。
 * @param event 需要操作的事件。
 * @return OS_OK，如果至少唤醒了一个任务。
*/
OS_Error_t OS_Event_Notify(OS_Event event);
/**
 * @brief 唤醒事件就绪队列中的所有任务。如果处于中断中且开启了中断延迟发布，则会将此操作发送到主线程。
 * @param event 需要操作的事件。
 * @return OS_OK，如果至少唤醒了一个任务。
*/
OS_Error_t OS_Event_NotifyAll(OS_Event event);


#if OS_EVENT_NAME_EN
/**
 * @brief 根据名称查找一个事件。
 * @param name 时间的名称。
 * @return 如果找到，返回该事件；否则返回NULL。
*/
OS_Event OS_Event_Find(const char* name);
#endif

#endif
#ifdef __cplusplus
}
#endif