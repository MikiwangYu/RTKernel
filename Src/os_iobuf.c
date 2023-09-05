#include "os_iobuf.h"
#include "os_int.h"
#include "os_task.h"
#include "os_cpu.h"
#if OS_EVENT_EN

static uint32_t modadd(uint32_t a, uint32_t b, uint32_t p) {
    return a + b < p ? a + b : a + b - p;
}

/**
 * @brief ��Ϊ�ص��������û�������غ�����
 * @param src ��������
 * @param arg ������
*/
static void os_iobuf_delay_execute_callback(void* src, void* arg) {
    ((OS_Error_t(*)(OS_IOBuf*))arg)(src);
}

/**
 * @brief ��ʼ�����������е����ݡ�
 * @param data ���������ݴ�Ŵ���
 * @param size ��������С��
*/
static void os_iobuf_extra_init(OS_IOBuf iobuf, uint8_t* data, uint32_t size) {
    iobuf->type = OS_EVENT_TYPE_IOBUF;
    iobuf->_iobuf_data_p = data;
    iobuf->_iobuf_data_size = size;
    iobuf->_iobuf_queue_s = 0;
    iobuf->_iobuf_queue_size = 0;
    iobuf->_iobuf_queue_flushed_size = 0;
}

void OS_IOBuf_Init(OS_IOBuf iobuf, const char* name, uint8_t* data, uint32_t size) {
    /*���û��๹�캯����*/
    OS_Event_Init(iobuf, name);
    /*���������캯����*/
    os_iobuf_extra_init(iobuf, data, size);
}

OS_IOBuf OS_IOBuf_Create(const char* name, uint8_t* data, uint32_t size) {
    /*���û��๹�캯����*/
    OS_IOBuf iobuf = OS_Event_Create(name);
    /*���������캯����*/
    if (iobuf)
        os_iobuf_extra_init(iobuf, data, size);
    return iobuf;
}

OS_Error_t OS_IOBuf_Release(OS_IOBuf iobuf) {
    /*���ø�������������*/
    return OS_Event_Release(iobuf);
}
/**
 * @brief �����������ݸ�os_iobuf_stop_waiting_callback��
*/
typedef struct {
    /**
     * @brief ��Ҫ�����Ļ�������
    */
    OS_IOBuf iobuf;
    /**
     * @brief ��ȡ���ݵ���ʼλ�á�
    */
    uint32_t* sp;
    /**
     * @brief ʵ�ʿ��Զ�ȡ����������
    */
    uint32_t* ret_size;
    /**
     * @brief ��ȡ�������������
    */
    uint32_t size;
}Seg;


/**
 * @brief �������ڵȴ�������ˢ�±�����ʱ���ã���¼���Զ�ȡ�����ݶΡ�
 * @param src �����ѵ�����
 * @param arg һ��Seg����
*/
static void os_iobuf_stop_waiting_callback(void *src,void *arg) {
    OS_Prepare_Critical();
    OS_Enter_Critical();
    Seg* seg = arg;

    /*��¼��ǰ�����ס�*/
    *seg->sp = seg->iobuf->_iobuf_queue_s;
    /*�������ʣ����������*/
    *seg->ret_size = seg->iobuf->_iobuf_queue_flushed_size;
    /*ret_size���ܴ���size��*/
    if (seg->size < *seg->ret_size)
        *seg->ret_size = seg->size;
    /*���¶��г��ȡ�*/
    seg->iobuf->_iobuf_queue_s = modadd(seg->iobuf->_iobuf_queue_s, *seg->ret_size, seg->iobuf->_iobuf_data_size);
    seg->iobuf->_iobuf_queue_flushed_size -= *seg->ret_size;
    seg->iobuf->_iobuf_queue_size -= *seg->ret_size;

    OS_Exit_Critical();
}

OS_Error_t OS_IOBuf_Try_Read(OS_IOBuf iobuf, uint8_t* data, uint32_t size, uint32_t* ret_size) {

    os_param_assert(iobuf, OS_ILLEGAL_PARAM);
    os_param_assert(iobuf->type == OS_EVENT_TYPE_IOBUF, OS_ILLEGAL_PARAM);

    os_assert(!OS_In_Intr(), OS_PERMISSION_DENY);
    os_assert(OS_Task_Switchable(), OS_PERMISSION_DENY);

    /*���ret_sizeΪ�գ���ָ��һ�����ñ�����*/
    uint32_t _ret_size = 0;
    if (!ret_size)
        ret_size = &_ret_size;

    uint32_t s = 0;
    Seg seg = { iobuf,&s,ret_size,size };
    /*��ȡ���Զ�ȡ�����ݶΡ�*/
    os_iobuf_stop_waiting_callback(OS_Cur_Task, &seg);

    /*�������ݡ�*/
    for (uint32_t i = 0; i < *ret_size; i++)
        data[i] = iobuf->_iobuf_data_p[modadd(s, i, iobuf->_iobuf_data_size)];
    return *ret_size ? OS_OK : OS_ERROR;
}

OS_Error_t OS_IOBuf_Read(OS_IOBuf iobuf, uint8_t* data, uint32_t size, uint32_t* ret_size, OS_Tick timeout) {

    os_param_assert(iobuf, OS_ILLEGAL_PARAM);
    os_param_assert(iobuf->type == OS_EVENT_TYPE_IOBUF, OS_ILLEGAL_PARAM);


    os_assert(!OS_In_Intr(), OS_PERMISSION_DENY);
    os_assert(OS_Task_Switchable(), OS_PERMISSION_DENY);

    /*���ret_sizeΪ�գ���ָ��һ�����ñ�����*/
    uint32_t _ret_size = 0;
    if (!ret_size)
        ret_size = &_ret_size;

    uint32_t s = 0;
    Seg seg = { iobuf,&s,ret_size,size };
    OS_Prepare_Protect();
    OS_Enter_Protect();
    /*��ȡ���Զ�ȡ�����ݶΡ�*/
    os_iobuf_stop_waiting_callback(NULL, &seg);
    if (!*ret_size) {
        /*��������Զ�ȡ�κ����ݣ���ʼ�ȴ���*/
        OS_Task_Start_Waiting(&iobuf->_ready_table, timeout, os_iobuf_stop_waiting_callback, &seg);
        OS_Exit_Protect();
        /*�л�����*/
        OS_Sched();
    } else {
        OS_Exit_Protect();
    }
    /*�������ݡ�*/
    for(uint32_t i=0;i<*ret_size;i++)
        data[i] = iobuf->_iobuf_data_p[modadd(s, i, iobuf->_iobuf_data_size)];
    return *ret_size ? OS_OK : OS_TIMEOUT;
}

OS_Error_t OS_IOBuf_Write(OS_IOBuf iobuf, const uint8_t* data, uint32_t size, uint32_t* ret_size) {
    os_param_assert(iobuf, OS_ILLEGAL_PARAM);
    os_param_assert(iobuf->type == OS_EVENT_TYPE_IOBUF, OS_ILLEGAL_PARAM);
    os_param_assert(data, OS_ILLEGAL_PARAM);
    os_param_assert(size, OS_ILLEGAL_PARAM);
    /*���ret_sizeΪ�գ���ָ��һ�����ñ�����*/
    uint32_t _ret_size = 0;
    if (!ret_size)
        ret_size = &_ret_size;

    OS_Prepare_Critical();
    OS_Enter_Critical();

    /*��¼��ǰ����β��*/
    uint32_t s = modadd(iobuf->_iobuf_queue_s, iobuf->_iobuf_queue_size, iobuf->_iobuf_data_size);
    /*�������ʣ��������*/
    *ret_size = iobuf->_iobuf_data_size - iobuf->_iobuf_queue_size;

    /*ret_size���ܴ���size��*/
    if (size < *ret_size)
        *ret_size = size;
    /*���¶��г��ȡ�*/
    iobuf->_iobuf_queue_size += *ret_size;
    OS_Exit_Critical();
    /*д�����ݡ�*/
    for (uint32_t i = 0; i < *ret_size; i++)
        iobuf->_iobuf_data_p[modadd(s, i, iobuf->_iobuf_data_size)] = data[i];
    return *ret_size ? OS_OK : OS_ERROR;
}
OS_Error_t OS_IOBuf_Flush(OS_IOBuf iobuf) {
    os_param_assert(iobuf, OS_ILLEGAL_PARAM);
    os_param_assert(iobuf->type == OS_EVENT_TYPE_IOBUF, OS_ILLEGAL_PARAM);

#if OS_INTR_DELAY_EN
    /*�ӳٷ�����*/
    if (OS_In_Intr())
        return OS_Delay_Execute(os_iobuf_delay_execute_callback, iobuf, OS_IOBuf_Flush);
#endif

    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_Error_t error = OS_OK;
    /*�����ѻ�������ݴ�С��*/
    iobuf->_iobuf_queue_flushed_size = iobuf->_iobuf_queue_size;
    /*�������ȼ���ߵ�����*/
    while (iobuf->_iobuf_queue_flushed_size && OS_Prio_Table_Get_Prio(&iobuf->_ready_table) != OS_MAX_PRIO)
        OS_Task_Stop_Waiting(OS_Prio_Table_Get_Next(&iobuf->_ready_table, OS_MAX_PRIO)->data, OS_OK);
    OS_Exit_Protect();


    /*����Ƿ���Ҫ���µ��ȡ�*/
    if (OS_Need_Sched())
        OS_Sched();
    return error;
}
#endif