#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "os_def.h"
#include "os_evt.h"
#if OS_EVENT_EN
/**
 * @brief IO缓冲。
*/
typedef OS_Event OS_IOBuf;
/**
 * @brief 初始化一个缓冲区。
 * @param iobuf 需要初始化的缓冲区。
 * @param name 缓冲区名称。
 * @param data 缓冲区数据存放处。
 * @param size 缓冲区大小。
*/
void OS_IOBuf_Init(OS_IOBuf iobuf, const char* name, uint8_t* data, uint32_t size);
/**
 * @brief 创建一个缓冲区。
 * @param name 缓冲区名称。
 * @param data 缓冲区数据存放处。
 * @param size 缓冲区大小。
 * @return 如果创建成功，则然会缓冲区指针；否则返回NULL。
*/
OS_IOBuf OS_IOBuf_Create(const char* name, uint8_t* data, uint32_t size);
/**
 * @brief 销毁一个缓冲区。
 * @param iobuf 需要销毁的缓冲区。
 * @return OS_OK，如果销毁成功。
*/
OS_Error_t OS_IOBuf_Release(OS_IOBuf iobuf);
/**
 * @brief 尝试从缓冲区中读取数据。
 * @param iobuf 需要操作的缓冲区。
 * @param data 读取的数据存放处。
 * @param size 读取的最大数据量。
 * @param ret_size 实际读取的数据量，可以传入NULL。
 * @return OS_OK，如果至少读取了1字节数据。
*/
OS_Error_t OS_IOBuf_Try_Read(OS_IOBuf iobuf, uint8_t* data, uint32_t size, uint32_t *ret_size);
/**
 * @brief 从缓冲区中读取数据。
 * @param iobuf 需要操作的缓冲区。
 * @param data 读取的数据存放处。
 * @param size 读取的最大数据量。
 * @param ret_size 实际读取的数据量，可以传入NULL。
 * @param timeout 如果为0，则无限等待，否则等待timeout个tick后返回。
 * @return OS_OK，如果至少读取了1字节数据；OS_TIMEOUT，如果等待超时。
*/
OS_Error_t OS_IOBuf_Read(OS_IOBuf iobuf, uint8_t* data, uint32_t size, uint32_t* ret_size, OS_Tick timeout);
/**
 * @brief 将数据写入缓冲区，尝试读取数据的任务不会立马获取到数据，除非缓冲区刷新。
 * @param iobuf 需要操作的缓冲区。
 * @param data 需要写入的数据。
 * @param size 写入的数据量。
 * @param ret_size 实际写入的数据量，可以传入NULL。
 * @return OS_OK，如果至少写入了1字节数据。
*/
OS_Error_t OS_IOBuf_Write(OS_IOBuf iobuf, const uint8_t* data, uint32_t size, uint32_t *ret_size);
/**
 * @brief 刷新缓冲区，并唤醒被阻塞的任务。如果处于中断中且开启了中断延迟发布，则会将此操作发送到主线程。
 * @param iobuf 需要刷新的缓冲区。
 * @return OS_OK，如果刷新成功。
*/
OS_Error_t OS_IOBuf_Flush(OS_IOBuf iobuf);
#endif
#ifdef __cplusplus
}
#endif