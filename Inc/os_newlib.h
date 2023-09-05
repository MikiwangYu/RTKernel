#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "os_def.h"
#if OS_NEWLIB_EN

/**
 * @brief 设备。需要用户自行创建并注册。
*/
typedef struct _OS_DEV {
    /**
     * @brief 设备名。
    */
    const char* name;
    /**
     * @brief 读取设备。
    */
    int (*read)(void*, size_t);
    /**
     * @brief 写入设备。
    */
    int (*write)(const void*, size_t);
    /**
     * @brief 打开设备。
    */
    int (*open)(int);
    /**
     * @brief 关闭设备。
    */
    int (*close)(void);
}OS_DEV, *OS_Device;

/**
 * @brief 向操作系统注册一个设备。
 * @param device 需要注册的设备。
 * @return 设备描述符。
*/
int OS_Device_Register(OS_Device device);
/**
 * @brief 初始化全局IO。
 * @param stdin_dev 标准输入流设备。
 * @param stdout_dev 标准输出流设备。
 * @param stderr_dev 标准错误流设备。
*/
void OS_IO_Init(OS_Device stdin_dev, OS_Device stdout_dev, OS_Device stderr_dev);

#endif


#ifdef __cplusplus
}
#endif