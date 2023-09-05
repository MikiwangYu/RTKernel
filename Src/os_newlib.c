#define _COMPILING_NEWLIB
#include "errno.h"
#include "unistd.h"
#include "os_int.h"
#include "os_task.h"
#include "os_iobuf.h"
#include "os_mem.h"
#include "string.h"
#include "os_newlib.h"
#include "sys/stat.h"
#include "os_cpu.h"

/**
 * @brief 所有已注册的设备。
*/
static OS_Device registered_device[OS_DEVICE_CNT];
/**
 * @brief 已注册设备的数量。
*/
static int registered_device_cnt;
/**
 * @brief 文件描述符对应的设备。
*/
static OS_Device fd_to_device[OS_FD_CNT];
/**
 * @brief 用来分配文件描述符的优先队列节点。
*/
static OS_Priority_Queue_Node fd_node[OS_FD_CNT];
/**
 * @brief 空闲的文件描述符优先队列。
*/
static OS_Priority_Queue fd_free_queue;

/**
 * @brief 分配当前可用的最小文件描述符。
 * @return -1，如果分配失败；否则返回被分配到的文件描述符。
*/
static int fd_alloc(void);
/**
 * @brief 回收文件描述符。
 * @param fd 需要回收的文件描述符。
*/
static void fd_free(int fd);
/**
 * @brief 查找一个设备。
 * @param name 设备名。
 * @return NULL，如果未能找到该设备；否则返回设备指针。
*/
static OS_Device find_device(const char* name);


/*-------newlib------*/
void* _sbrk(ptrdiff_t __incr) {
    OS_Prepare_Critical();
    OS_Enter_Critical();
    /*当前堆空间大小*/
    static uint32_t cur = 0;
    void* ret = (void*)-1;
    if (cur + __incr > OS_MEM_TOTAL_COST) {
        /*如果超过空间限制，执行失败。*/
        errno = ENOMEM;
    } else {
        errno = 0;
        /*记录新段的起始位置。*/
        ret = (char*)OS_Heap_Base() + cur;
        /*更新当前堆空间大小。*/
        cur += __incr;
    }
    OS_Exit_Critical();
    return ret;
}
int _read(int fd, void* data, size_t len) {
    errno = 0;
    return fd_to_device[fd]->read(data, len);
}
int _write(int fd, const void* data, size_t len) {
    errno = 0;
    return fd_to_device[fd]->write(data, len);
}
int _close(int __fildes) {
    errno = 0;
    int ret = -1;
    /*获取需要关闭的设备*/
    OS_Device device = fd_to_device[__fildes];
    /*如果关闭失败，返回错误。*/
    if (device->close() == -1)
        goto end;
    /*清除映射。*/
    fd_to_device[__fildes] = NULL;
    /*释放文件描述符。*/
    fd_free(__fildes);
    ret = 0;
end:
    return ret;
}
int _open(const char* name, int mode, ...) {
    errno = 0;
    int ret = -1;
    /*根据设备名查找设备。*/
    OS_Device device = find_device(name);
    /*如果未找到，返回错误。*/
    if (!device)
        goto end;
    /*如果打开失败，返回错误。*/
    if (device->open(mode) == -1)
        goto end;
    /*分配文件描述符。*/
    int fd = fd_alloc();
    /*如果分配失败，返回错误。*/
    if (fd == -1)
        goto end;
    /*建立映射关系。*/
    fd_to_device[fd] = device;
    /*返回文件描述符。*/
    ret = fd;
end:
    return ret;
}
_off_t _lseek(int __fildes, _off_t __offset, int __whence) {
    errno = 0;
    return 0;
}
int	_fstat(int __fd, struct stat* __sbuf) {
    errno = 0;
    __sbuf->st_mode = S_IFCHR;
    return 0;
}
int	_isatty(int __fildes) {
    errno = 0;
    return 0;
}

pid_t _getpid(void) {
    errno = ENOTSUP;
    return -1;
}

void _exit(int __status) {
    while (1);
}

int _kill(pid_t pid) {
    errno = ENOTSUP;
    return 0;
}
/*---------mkwos------------*/
static int fd_alloc(void) {
    int fd = OS_Priority_Queue_Top(fd_free_queue)->key;
    OS_Priority_Queue_Erase(&fd_free_queue, fd_node + fd);
    return fd;
}
static void fd_free(int fd) {
    OS_Priority_Queue_Push(&fd_free_queue, fd_node + fd);
}
static OS_Device find_device(const char* name) {
    for (int i = 0; i < registered_device_cnt; i++)
        if (!strcmp(name, registered_device[i]->name))
            return registered_device[i];
    return NULL;
}

int OS_Device_Register(OS_Device _device) {
    registered_device[registered_device_cnt] = _device;
    return registered_device_cnt++;
}

void OS_IO_Init(OS_Device stdin_dev, OS_Device stdout_dev, OS_Device stderr_dev) {
    /*初始化文件描述符分配器。*/
    OS_Priority_Queue_Init(&fd_free_queue);
    for (int i = 0; i < OS_FD_CNT; i++) {
        OS_Priority_Queue_Node_Init(fd_node + i, i, NULL);
        OS_Priority_Queue_Push(&fd_free_queue, fd_node + i);
    }
    /*设置标准输入输出流。*/
    fd_to_device[fd_alloc()] = stdin_dev;
    fd_to_device[fd_alloc()] = stdout_dev;
    fd_to_device[fd_alloc()] = stderr_dev;
}