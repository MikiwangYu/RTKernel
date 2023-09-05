#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "os_def.h"
#include "os_prio.h"
#include "os_time.h"
/**
 * @brief 线程ID。
*/
typedef uint32_t OS_Task_ID;
/**
 * @brief 任务状态。
*/
typedef enum _OS_Task_Status {
    /**
     * @brief 就绪状态。
    */
    OS_TASK_STATUS_READY = 0,
    /**
     * @brief 正在运行。
    */
    OS_TASK_STATUS_RUNNING,
    /**
     * @brief 被阻塞。
    */
    OS_TASK_STATUS_SUSPEND,
    /**
     * @brief 运行结束。
    */
    OS_TASK_STATUS_FINISHED,
    /**
     * @brief 资源已释放。
    */
    OS_TASK_STATUS_RELEASED
}OS_Task_Status;


#if OS_MUTEX_EN
/**
 * @brief 优先级天花板节点类型。
*/
typedef OS_Priority_Queue_Node OS_Mutex_Prio_Node;
#endif

/**
 * @brief 任务控制块。
*/
typedef struct _OS_TCB {
    /**
     * @brief 栈指针。
    */
    void* stack_p;
    /**
     * @brief 栈顶。
    */
    void* stack_buttom;
    /**
     * @brief 任务ID。
    */
    OS_Task_ID id;
    /**
     * @brief 任务状态。
    */
    OS_Task_Status status;
    /**
     * @brief 任务正挂载到的优先级表。
    */
    OS_Prio_Table* ready_table;
    /**
     * @brief 优先级表节点，用来挂载到优先级表上。通常会被挂载到OS_Task_Ready_Table或者某个事件控制块的就绪表上。
    */
    OS_Prio_Table_Node ready_node;

#if OS_TASK_NAME_EN
    /**
     * @brief 任务名。
    */
    const char* name;
#endif



    /**
     * @brief 任务等待超时节点，如果任务被阻塞，且指定了超时时间，则会使用该节点挂载到一个优先队列上。
    */
    OS_Priority_Queue_Node _wait_node;
    /**
     * @brief 任务等待回调函数，任务超时或者被正常唤醒时都会被调用。
    */
    OS_Callback_Func_t _wait_callback_func;
    /**
     * @brief 任务等待回调函数参数。
    */
    OS_Callback_Func_t _wait_callback_arg;

    /**
     * @brief 资源链表节点，任务被销毁后会利用此节点挂载到OS_Task_Free_List，未被销毁的任务会利用此节点挂载到OS_Task_List。
    */
    OS_List_Node _resource_node;
    

#if OS_MUTEX_EN
    /**
     * @brief 任务自身的优先级节点。
    */
    OS_Priority_Queue _pq;
    
    /**
     * @brief 确定任务优先级的优先队列。
    */
    OS_Priority_Queue_Node _pq_node;
#else
    /**
     * @brief 任务优先级。
    */
    OS_Prio _prio;
#endif
    /**
     * @brief 错误信息，通常用于阻塞唤醒时传递信息。
    */
    OS_Error_t _error;

}OS_TCB, * OS_Task;


/**
 * @brief 检测当前栈是否溢出。
 * @param task 需要检测的任务。
 * @return true，如果栈溢出。
*/
bool OS_Task_Stack_Overflow(OS_Task task);

/**
 * @brief 任务就绪表，里面的任务会按优先级依次执行。
*/
extern OS_Prio_Table OS_Task_Ready_Table;

/**
 * @brief 当前任务。
*/
extern OS_Task OS_Cur_Task;

/**
 * @brief 下一个任务。
*/
extern OS_Task OS_Next_Task;
/**
 * @brief 所有未被销毁的任务的链表。
*/
extern OS_List OS_Task_List;
/**
 * @brief 所有已被销毁的任务的链表。
*/
extern OS_List OS_Task_Free_List;
/**
 * @brief 检查是否有超时的任务。
 * @return true，如果有超时的任务；false，如果没有超时的任务。
*/
bool OS_Need_Check_Timeout(void);
/**
 * @brief 将所有超时的任务唤醒。
*/
void OS_Check_Timeout(void);

/**
 * @brief 检查是否需要进行任务调度。
 * @return true，如果当前任务不是最高优先级，或者时间片已轮转完毕，或者当前任务被阻塞；false，其他。
*/
bool OS_Need_Sched(void);
/**
 * @brief 寻找最高优先级的任务。
*/
void OS_Sched_New(void);
/**
 * @brief 如果允许，进行任务调度。
*/
void OS_Sched(void);

void OS_Task_Erase(void);
/**
 * @brief 获取任务调度器的状态。
 * @return true，如果可以进行任务调度；false，如果不可以进行任务调度。
*/
bool OS_Task_Switchable(void);
/**
 * @brief 设置任务调度器的状态。
 * @param bl true，允许任务调度；false，禁止任务调度。
 * @return 设置之前的任务调度器状态。
*/
bool OS_Task_Set_Switchable(bool bl);
/**
 * @brief 初始化一个任务，并将其加入就绪队列。
 * @param task 要初始化的任务的指针。
 * @param name 任务名称。
 * @param func 任务函数。
 * @param arg 任务参数。
 * @param prio 任务优先级。
 * @param delay 任务启动延时。
 * @param stack_buttom 任务栈顶。
 * @param stack_size 任务栈大小。
*/
void OS_Task_Init(OS_Task task, const char* name, void(*func)(void*), void* arg, OS_Prio prio, OS_Tick delay, void* stack_buttom, size_t stack_size);
/**
 * @brief 创建一个任务，并将其加入就绪队列。
 * @param name 任务名称。
 * @param func 任务函数。
 * @param arg 任务参数。
 * @param prio 任务优先级。
 * @param delay 任务启动延时。
 * @param stack_buttom 任务栈顶。
 * @param stack_size 任务栈大小。
 * @return 若创建成功，返回该任务的任务指针；否则返回NULL。
*/
OS_Task OS_Task_Create(const char* name, void(*func)(void*), void* arg, OS_Prio prio, OS_Tick delay, void* stack_buttom, size_t stack_size);

/**
 * @brief 获取任务的优先级。
 * @param task 需要操作的任务。
 * @return 该任务的优先级。
*/
OS_Prio OS_Task_Get_Prio(OS_Task task);

/**
 * @brief 将当前任务添加到等待队列，等待结束后设置OS_Cur_Task->_error。将当前任务从就绪队列上卸载。注意，此函数不会阻塞该任务，也不保证线程安全！调用后必须立马调用OS_Sched。
 * @param timeout 如果为0，则无限等待，否则等待timeout个tick后返回。
*/
void OS_Task_Start_Waiting(OS_Prio_Table* ready_table, OS_Tick timeout, OS_Callback_Func_t callback_func, OS_Callback_Arg_t callback_arg);
/**
 * @brief 设置task->_error，将当前任务重新添加到等待队列，并从等待队列中删除。注意，不保证线程安全！
 * @param task 需要唤醒的任务。
 * @param error 设置的错误信息。
*/
void OS_Task_Stop_Waiting(OS_Task task, OS_Error_t error);
/**
 * @brief 阻塞当前任务，等待唤醒。
 * @param timeout 如果为0，则无限等待，否则等待timeout个tick后返回。
 * @return OS_OK，如果被OS_Task_Resume唤醒；OS_TIMEOUT，如果等待超时。
*/
OS_Error_t OS_Task_Suspend(OS_Tick timeout);
/**
 * @brief 唤醒一个任务。如果处于中断中且开启了中断延迟发布，则会将此操作发送到主线程。
 * @param task 需要唤醒的任务。
 * @return OS_OK，如果唤醒成功；OS_ERROR，如果任务未被阻塞。
*/
OS_Error_t OS_Task_Resume(OS_Task task);
/**
 * @brief 改变一个任务的优先级，并将其重新挂载。如果处于中断中且开启了中断延迟发布，则会将此操作发送到主线程。
 * @param task 需要操作的任务。
 * @param prio 新优先级。
 * @return OS_OK，如果更改成功。
*/
OS_Error_t OS_Task_Change_Prio(OS_Task task, OS_Prio prio);





#if OS_MUTEX_EN
/**
 * @brief 初始化优先级天花板的节点。
 * @param p 需要初始化的节点。
 * @param data 互斥量指针。
*/
void OS_Mutex_Prio_Node_Init(OS_Mutex_Prio_Node* p, void* data);
/**
 * @brief 无安全措施，将node以prio的优先级挂载到该任务上，并重新挂载任务。
 * @param task 需要操作的任务
 * @param node 需要卸载的节点。
 * @param prio 节点的优先级
*/
void OS_Task_Mount_Mutex(OS_Task task, OS_Mutex_Prio_Node* node, OS_Prio prio);
/**
 * @brief 无安全措施，将node以prio的优先级从该任务上卸载，并重新挂载任务。
 * @param task 需要操作的任务。
 * @param node 需要卸载的节点。
 * @param prio 节点的优先级。
*/
void OS_Task_UnMount_Mutex(OS_Task task, OS_Mutex_Prio_Node* node, OS_Prio prio);
#endif

#if OS_TASK_NAME_EN
/**
 * @brief 通过名称查找一个任务。
 * @param name 需要查找的任务的名称。
 * @return 如果找到，返回该任务的指针，否则返回NULL；若有多个满足条件，返回找到的第一个。
*/
OS_Task OS_Find_Task(const char* name);
#endif

#ifdef __cplusplus
}
#endif