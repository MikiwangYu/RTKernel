#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "os_def.h"
#include "os_prio.h"
#include "os_time.h"
/**
 * @brief �߳�ID��
*/
typedef uint32_t OS_Task_ID;
/**
 * @brief ����״̬��
*/
typedef enum _OS_Task_Status {
    /**
     * @brief ����״̬��
    */
    OS_TASK_STATUS_READY = 0,
    /**
     * @brief �������С�
    */
    OS_TASK_STATUS_RUNNING,
    /**
     * @brief ��������
    */
    OS_TASK_STATUS_SUSPEND,
    /**
     * @brief ���н�����
    */
    OS_TASK_STATUS_FINISHED,
    /**
     * @brief ��Դ���ͷš�
    */
    OS_TASK_STATUS_RELEASED
}OS_Task_Status;


#if OS_MUTEX_EN
/**
 * @brief ���ȼ��컨��ڵ����͡�
*/
typedef OS_Priority_Queue_Node OS_Mutex_Prio_Node;
#endif

/**
 * @brief ������ƿ顣
*/
typedef struct _OS_TCB {
    /**
     * @brief ջָ�롣
    */
    void* stack_p;
    /**
     * @brief ջ����
    */
    void* stack_buttom;
    /**
     * @brief ����ID��
    */
    OS_Task_ID id;
    /**
     * @brief ����״̬��
    */
    OS_Task_Status status;
    /**
     * @brief ���������ص������ȼ���
    */
    OS_Prio_Table* ready_table;
    /**
     * @brief ���ȼ���ڵ㣬�������ص����ȼ����ϡ�ͨ���ᱻ���ص�OS_Task_Ready_Table����ĳ���¼����ƿ�ľ������ϡ�
    */
    OS_Prio_Table_Node ready_node;

#if OS_TASK_NAME_EN
    /**
     * @brief ��������
    */
    const char* name;
#endif



    /**
     * @brief ����ȴ���ʱ�ڵ㣬���������������ָ���˳�ʱʱ�䣬���ʹ�øýڵ���ص�һ�����ȶ����ϡ�
    */
    OS_Priority_Queue_Node _wait_node;
    /**
     * @brief ����ȴ��ص�����������ʱ���߱���������ʱ���ᱻ���á�
    */
    OS_Callback_Func_t _wait_callback_func;
    /**
     * @brief ����ȴ��ص�����������
    */
    OS_Callback_Func_t _wait_callback_arg;

    /**
     * @brief ��Դ����ڵ㣬�������ٺ�����ô˽ڵ���ص�OS_Task_Free_List��δ�����ٵ���������ô˽ڵ���ص�OS_Task_List��
    */
    OS_List_Node _resource_node;
    

#if OS_MUTEX_EN
    /**
     * @brief ������������ȼ��ڵ㡣
    */
    OS_Priority_Queue _pq;
    
    /**
     * @brief ȷ���������ȼ������ȶ��С�
    */
    OS_Priority_Queue_Node _pq_node;
#else
    /**
     * @brief �������ȼ���
    */
    OS_Prio _prio;
#endif
    /**
     * @brief ������Ϣ��ͨ��������������ʱ������Ϣ��
    */
    OS_Error_t _error;

}OS_TCB, * OS_Task;


/**
 * @brief ��⵱ǰջ�Ƿ������
 * @param task ��Ҫ��������
 * @return true�����ջ�����
*/
bool OS_Task_Stack_Overflow(OS_Task task);

/**
 * @brief ������������������ᰴ���ȼ�����ִ�С�
*/
extern OS_Prio_Table OS_Task_Ready_Table;

/**
 * @brief ��ǰ����
*/
extern OS_Task OS_Cur_Task;

/**
 * @brief ��һ������
*/
extern OS_Task OS_Next_Task;
/**
 * @brief ����δ�����ٵ����������
*/
extern OS_List OS_Task_List;
/**
 * @brief �����ѱ����ٵ����������
*/
extern OS_List OS_Task_Free_List;
/**
 * @brief ����Ƿ��г�ʱ������
 * @return true������г�ʱ������false�����û�г�ʱ������
*/
bool OS_Need_Check_Timeout(void);
/**
 * @brief �����г�ʱ�������ѡ�
*/
void OS_Check_Timeout(void);

/**
 * @brief ����Ƿ���Ҫ����������ȡ�
 * @return true�������ǰ������������ȼ�������ʱ��Ƭ����ת��ϣ����ߵ�ǰ����������false��������
*/
bool OS_Need_Sched(void);
/**
 * @brief Ѱ��������ȼ�������
*/
void OS_Sched_New(void);
/**
 * @brief �����������������ȡ�
*/
void OS_Sched(void);

void OS_Task_Erase(void);
/**
 * @brief ��ȡ�����������״̬��
 * @return true��������Խ���������ȣ�false����������Խ���������ȡ�
*/
bool OS_Task_Switchable(void);
/**
 * @brief ���������������״̬��
 * @param bl true������������ȣ�false����ֹ������ȡ�
 * @return ����֮ǰ�����������״̬��
*/
bool OS_Task_Set_Switchable(bool bl);
/**
 * @brief ��ʼ��һ�����񣬲��������������С�
 * @param task Ҫ��ʼ���������ָ�롣
 * @param name �������ơ�
 * @param func ��������
 * @param arg ���������
 * @param prio �������ȼ���
 * @param delay ����������ʱ��
 * @param stack_buttom ����ջ����
 * @param stack_size ����ջ��С��
*/
void OS_Task_Init(OS_Task task, const char* name, void(*func)(void*), void* arg, OS_Prio prio, OS_Tick delay, void* stack_buttom, size_t stack_size);
/**
 * @brief ����һ�����񣬲��������������С�
 * @param name �������ơ�
 * @param func ��������
 * @param arg ���������
 * @param prio �������ȼ���
 * @param delay ����������ʱ��
 * @param stack_buttom ����ջ����
 * @param stack_size ����ջ��С��
 * @return �������ɹ������ظ����������ָ�룻���򷵻�NULL��
*/
OS_Task OS_Task_Create(const char* name, void(*func)(void*), void* arg, OS_Prio prio, OS_Tick delay, void* stack_buttom, size_t stack_size);

/**
 * @brief ��ȡ��������ȼ���
 * @param task ��Ҫ����������
 * @return ����������ȼ���
*/
OS_Prio OS_Task_Get_Prio(OS_Task task);

/**
 * @brief ����ǰ������ӵ��ȴ����У��ȴ�����������OS_Cur_Task->_error������ǰ����Ӿ���������ж�ء�ע�⣬�˺�����������������Ҳ����֤�̰߳�ȫ�����ú�����������OS_Sched��
 * @param timeout ���Ϊ0�������޵ȴ�������ȴ�timeout��tick�󷵻ء�
*/
void OS_Task_Start_Waiting(OS_Prio_Table* ready_table, OS_Tick timeout, OS_Callback_Func_t callback_func, OS_Callback_Arg_t callback_arg);
/**
 * @brief ����task->_error������ǰ����������ӵ��ȴ����У����ӵȴ�������ɾ����ע�⣬����֤�̰߳�ȫ��
 * @param task ��Ҫ���ѵ�����
 * @param error ���õĴ�����Ϣ��
*/
void OS_Task_Stop_Waiting(OS_Task task, OS_Error_t error);
/**
 * @brief ������ǰ���񣬵ȴ����ѡ�
 * @param timeout ���Ϊ0�������޵ȴ�������ȴ�timeout��tick�󷵻ء�
 * @return OS_OK�������OS_Task_Resume���ѣ�OS_TIMEOUT������ȴ���ʱ��
*/
OS_Error_t OS_Task_Suspend(OS_Tick timeout);
/**
 * @brief ����һ��������������ж����ҿ������ж��ӳٷ�������Ὣ�˲������͵����̡߳�
 * @param task ��Ҫ���ѵ�����
 * @return OS_OK��������ѳɹ���OS_ERROR���������δ��������
*/
OS_Error_t OS_Task_Resume(OS_Task task);
/**
 * @brief �ı�һ����������ȼ������������¹��ء���������ж����ҿ������ж��ӳٷ�������Ὣ�˲������͵����̡߳�
 * @param task ��Ҫ����������
 * @param prio �����ȼ���
 * @return OS_OK��������ĳɹ���
*/
OS_Error_t OS_Task_Change_Prio(OS_Task task, OS_Prio prio);





#if OS_MUTEX_EN
/**
 * @brief ��ʼ�����ȼ��컨��Ľڵ㡣
 * @param p ��Ҫ��ʼ���Ľڵ㡣
 * @param data ������ָ�롣
*/
void OS_Mutex_Prio_Node_Init(OS_Mutex_Prio_Node* p, void* data);
/**
 * @brief �ް�ȫ��ʩ����node��prio�����ȼ����ص��������ϣ������¹�������
 * @param task ��Ҫ����������
 * @param node ��Ҫж�صĽڵ㡣
 * @param prio �ڵ�����ȼ�
*/
void OS_Task_Mount_Mutex(OS_Task task, OS_Mutex_Prio_Node* node, OS_Prio prio);
/**
 * @brief �ް�ȫ��ʩ����node��prio�����ȼ��Ӹ�������ж�أ������¹�������
 * @param task ��Ҫ����������
 * @param node ��Ҫж�صĽڵ㡣
 * @param prio �ڵ�����ȼ���
*/
void OS_Task_UnMount_Mutex(OS_Task task, OS_Mutex_Prio_Node* node, OS_Prio prio);
#endif

#if OS_TASK_NAME_EN
/**
 * @brief ͨ�����Ʋ���һ������
 * @param name ��Ҫ���ҵ���������ơ�
 * @return ����ҵ������ظ������ָ�룬���򷵻�NULL�����ж�����������������ҵ��ĵ�һ����
*/
OS_Task OS_Find_Task(const char* name);
#endif

#ifdef __cplusplus
}
#endif