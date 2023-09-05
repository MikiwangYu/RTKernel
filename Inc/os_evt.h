#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "os_def.h"
#if OS_EVENT_EN
#include "os_task.h"
/**
 * @brief �¼����͡�
*/
typedef enum _OS_Event_Type {
    /**
     * @brief ���¼���
    */
    OS_EVENT_TYPE_RAW = 0,
    /**
     * @brief �ź�����
    */
    OS_EVENT_TYPE_SEM,
    /**
     * @brief IO���塣
    */
    OS_EVENT_TYPE_IOBUF,
#if OS_MUTEX_EN
    /**
     * @brief ��������
    */
    OS_EVENT_TYPE_MUTEX
#endif
}OS_Event_Type;



/**
 * @brief �¼����ƿ顣
*/
typedef struct _OS_ECB {
    /**
     * @brief �¼����͡�
    */
    OS_Event_Type type;
#if OS_EVENT_NAME_EN
    /**
     * @brief �¼����ơ�
    */
    const char* name;
#endif
    union {
        /*�ź�����*/
        struct {
            /**
             * @brief �ź�����ǰֵ��
            */
            uint32_t _sem_cur;
            /**
             * @brief �ź������ֵ��
            */
            uint32_t _sem_max;
        };

        /*IO���塣*/
        struct {
            /**
             * @brief ��������
            */
            uint8_t* _iobuf_data_p;
            /**
             * @brief ��������С��
            */
            uint32_t _iobuf_data_size;
            /**
             * @brief ����ͷ������ʱ�����жϻ��⡣
            */
            uint32_t _iobuf_queue_s;
            /**
             * @brief ���г��ȡ�����ʱ�����жϻ��⡣
            */
            uint32_t _iobuf_queue_size;
            /**
             * @brief ��ˢ�µĶ��г��ȡ�
            */
            uint32_t _iobuf_queue_flushed_size;
        };

#if OS_MUTEX_EN
        /*��������*/
        struct {
            /**
             * @brief �������Ƿ�������
            */
            bool _mutex_locked;
            /**
             * @brief ���������ȼ���
            */
            OS_Prio _mutex_prio;
            /**
             * @brief ���������ȼ��ڵ㡣
            */
            OS_Mutex_Prio_Node _mutex_node;
        };
#endif
    };
    /**
     * @brief ��Դ����ڵ㣬�¼������ٺ�����ô˽ڵ���ص�OS_Event_Free_List, δ�����ٵ��¼������ô˽ڵ���ص�OS_Event_List��
    */
    OS_List_Node _resource_node;
    /**
     * @brief �¼��������С�
    */
    OS_Prio_Table _ready_table;
}OS_ECB,*OS_Event;

/**
 * @brief ����δ�����ٵ��¼�������.
*/
extern OS_List OS_Event_List;
/**
 * @brief �����ѱ����ٵ��¼�������
*/
extern OS_List OS_Event_Free_List;
/**
 * @brief ��ʼ��һ���¼���
 * @param event ��Ҫ��ʼ�����¼���
 * @param name �¼����ơ�
*/
void OS_Event_Init(OS_Event event, const char* name);
/**
 * @brief ����һ���¼���
 * @param name �¼����ơ�
 * @return ��������ɹ��������¼�ָ�룻���򷵻�NULL��
*/
OS_Event OS_Event_Create(const char* name);
/**
 * @brief ����һ���¼���
 * @param event ��Ҫ���ٵ��¼���
 * @return OS_OK��������ٳɹ���
*/
OS_Error_t OS_Event_Release(OS_Event event);
/**
 * @brief ������ǰ���񣬽���ǰ������뵽�¼��ľ��������У�ֱ����ʱ���߱����ѡ�
 * @param event ��Ҫ�������¼���
 * @param timeout ���Ϊ0�������޵ȴ�������ȴ�timeout��tick�󷵻ء�
 * @return OS_OK��������������ѣ�OS_TIMEOUT������ȴ���ʱ��
*/
OS_Error_t OS_Event_Wait(OS_Event event, OS_Tick timeout);
/**
 * @brief �����¼����������е�һ��������������ж����ҿ������ж��ӳٷ�������Ὣ�˲������͵����̡߳�
 * @param event ��Ҫ�������¼���
 * @return OS_OK��������ٻ�����һ������
*/
OS_Error_t OS_Event_Notify(OS_Event event);
/**
 * @brief �����¼����������е�����������������ж����ҿ������ж��ӳٷ�������Ὣ�˲������͵����̡߳�
 * @param event ��Ҫ�������¼���
 * @return OS_OK��������ٻ�����һ������
*/
OS_Error_t OS_Event_NotifyAll(OS_Event event);


#if OS_EVENT_NAME_EN
/**
 * @brief �������Ʋ���һ���¼���
 * @param name ʱ������ơ�
 * @return ����ҵ������ظ��¼������򷵻�NULL��
*/
OS_Event OS_Event_Find(const char* name);
#endif

#endif
#ifdef __cplusplus
}
#endif