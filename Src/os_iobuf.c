#include "os_iobuf.h"
#include "os_int.h"
#include "os_task.h"
#include "os_cpu.h"
#if OS_EVENT_EN

static uint32_t modadd(uint32_t a, uint32_t b, uint32_t p) {
    return a + b < p ? a + b : a + b - p;
}

/**
 * @brief 作为回调函数调用缓冲区相关函数。
 * @param src 缓冲区。
 * @param arg 函数。
*/
static void os_iobuf_delay_execute_callback(void* src, void* arg) {
    ((OS_Error_t(*)(OS_IOBuf*))arg)(src);
}

/**
 * @brief 初始化缓冲区独有的内容。
 * @param data 缓冲区数据存放处。
 * @param size 缓冲区大小。
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
    /*调用基类构造函数。*/
    OS_Event_Init(iobuf, name);
    /*调用自身构造函数。*/
    os_iobuf_extra_init(iobuf, data, size);
}

OS_IOBuf OS_IOBuf_Create(const char* name, uint8_t* data, uint32_t size) {
    /*调用基类构造函数。*/
    OS_IOBuf iobuf = OS_Event_Create(name);
    /*调用自身构造函数。*/
    if (iobuf)
        os_iobuf_extra_init(iobuf, data, size);
    return iobuf;
}

OS_Error_t OS_IOBuf_Release(OS_IOBuf iobuf) {
    /*调用父类析构函数。*/
    return OS_Event_Release(iobuf);
}
/**
 * @brief 用作参数传递给os_iobuf_stop_waiting_callback。
*/
typedef struct {
    /**
     * @brief 需要操作的缓冲区。
    */
    OS_IOBuf iobuf;
    /**
     * @brief 读取数据的起始位置。
    */
    uint32_t* sp;
    /**
     * @brief 实际可以读取的数据量。
    */
    uint32_t* ret_size;
    /**
     * @brief 读取的最大数据量。
    */
    uint32_t size;
}Seg;


/**
 * @brief 当任务在等待缓冲区刷新被唤醒时调用，记录可以读取的数据段。
 * @param src 被唤醒的任务。
 * @param arg 一个Seg对象。
*/
static void os_iobuf_stop_waiting_callback(void *src,void *arg) {
    OS_Prepare_Critical();
    OS_Enter_Critical();
    Seg* seg = arg;

    /*记录当前队列首。*/
    *seg->sp = seg->iobuf->_iobuf_queue_s;
    /*计算队列剩余数据量。*/
    *seg->ret_size = seg->iobuf->_iobuf_queue_flushed_size;
    /*ret_size不能大于size。*/
    if (seg->size < *seg->ret_size)
        *seg->ret_size = seg->size;
    /*更新队列长度。*/
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

    /*如果ret_size为空，则指向一个无用变量。*/
    uint32_t _ret_size = 0;
    if (!ret_size)
        ret_size = &_ret_size;

    uint32_t s = 0;
    Seg seg = { iobuf,&s,ret_size,size };
    /*获取可以读取的数据段。*/
    os_iobuf_stop_waiting_callback(OS_Cur_Task, &seg);

    /*读出数据。*/
    for (uint32_t i = 0; i < *ret_size; i++)
        data[i] = iobuf->_iobuf_data_p[modadd(s, i, iobuf->_iobuf_data_size)];
    return *ret_size ? OS_OK : OS_ERROR;
}

OS_Error_t OS_IOBuf_Read(OS_IOBuf iobuf, uint8_t* data, uint32_t size, uint32_t* ret_size, OS_Tick timeout) {

    os_param_assert(iobuf, OS_ILLEGAL_PARAM);
    os_param_assert(iobuf->type == OS_EVENT_TYPE_IOBUF, OS_ILLEGAL_PARAM);


    os_assert(!OS_In_Intr(), OS_PERMISSION_DENY);
    os_assert(OS_Task_Switchable(), OS_PERMISSION_DENY);

    /*如果ret_size为空，则指向一个无用变量。*/
    uint32_t _ret_size = 0;
    if (!ret_size)
        ret_size = &_ret_size;

    uint32_t s = 0;
    Seg seg = { iobuf,&s,ret_size,size };
    OS_Prepare_Protect();
    OS_Enter_Protect();
    /*获取可以读取的数据段。*/
    os_iobuf_stop_waiting_callback(NULL, &seg);
    if (!*ret_size) {
        /*如果不可以读取任何数据，开始等待。*/
        OS_Task_Start_Waiting(&iobuf->_ready_table, timeout, os_iobuf_stop_waiting_callback, &seg);
        OS_Exit_Protect();
        /*切换任务。*/
        OS_Sched();
    } else {
        OS_Exit_Protect();
    }
    /*读出数据。*/
    for(uint32_t i=0;i<*ret_size;i++)
        data[i] = iobuf->_iobuf_data_p[modadd(s, i, iobuf->_iobuf_data_size)];
    return *ret_size ? OS_OK : OS_TIMEOUT;
}

OS_Error_t OS_IOBuf_Write(OS_IOBuf iobuf, const uint8_t* data, uint32_t size, uint32_t* ret_size) {
    os_param_assert(iobuf, OS_ILLEGAL_PARAM);
    os_param_assert(iobuf->type == OS_EVENT_TYPE_IOBUF, OS_ILLEGAL_PARAM);
    os_param_assert(data, OS_ILLEGAL_PARAM);
    os_param_assert(size, OS_ILLEGAL_PARAM);
    /*如果ret_size为空，则指向一个无用变量。*/
    uint32_t _ret_size = 0;
    if (!ret_size)
        ret_size = &_ret_size;

    OS_Prepare_Critical();
    OS_Enter_Critical();

    /*记录当前队列尾。*/
    uint32_t s = modadd(iobuf->_iobuf_queue_s, iobuf->_iobuf_queue_size, iobuf->_iobuf_data_size);
    /*计算队列剩余容量。*/
    *ret_size = iobuf->_iobuf_data_size - iobuf->_iobuf_queue_size;

    /*ret_size不能大于size。*/
    if (size < *ret_size)
        *ret_size = size;
    /*更新队列长度。*/
    iobuf->_iobuf_queue_size += *ret_size;
    OS_Exit_Critical();
    /*写入数据。*/
    for (uint32_t i = 0; i < *ret_size; i++)
        iobuf->_iobuf_data_p[modadd(s, i, iobuf->_iobuf_data_size)] = data[i];
    return *ret_size ? OS_OK : OS_ERROR;
}
OS_Error_t OS_IOBuf_Flush(OS_IOBuf iobuf) {
    os_param_assert(iobuf, OS_ILLEGAL_PARAM);
    os_param_assert(iobuf->type == OS_EVENT_TYPE_IOBUF, OS_ILLEGAL_PARAM);

#if OS_INTR_DELAY_EN
    /*延迟发布。*/
    if (OS_In_Intr())
        return OS_Delay_Execute(os_iobuf_delay_execute_callback, iobuf, OS_IOBuf_Flush);
#endif

    OS_Prepare_Protect();
    OS_Enter_Protect();
    OS_Error_t error = OS_OK;
    /*更新已缓冲的数据大小。*/
    iobuf->_iobuf_queue_flushed_size = iobuf->_iobuf_queue_size;
    /*唤醒优先级最高的任务。*/
    while (iobuf->_iobuf_queue_flushed_size && OS_Prio_Table_Get_Prio(&iobuf->_ready_table) != OS_MAX_PRIO)
        OS_Task_Stop_Waiting(OS_Prio_Table_Get_Next(&iobuf->_ready_table, OS_MAX_PRIO)->data, OS_OK);
    OS_Exit_Protect();


    /*检查是否需要重新调度。*/
    if (OS_Need_Sched())
        OS_Sched();
    return error;
}
#endif