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
 * @brief ������ע����豸��
*/
static OS_Device registered_device[OS_DEVICE_CNT];
/**
 * @brief ��ע���豸��������
*/
static int registered_device_cnt;
/**
 * @brief �ļ���������Ӧ���豸��
*/
static OS_Device fd_to_device[OS_FD_CNT];
/**
 * @brief ���������ļ������������ȶ��нڵ㡣
*/
static OS_Priority_Queue_Node fd_node[OS_FD_CNT];
/**
 * @brief ���е��ļ����������ȶ��С�
*/
static OS_Priority_Queue fd_free_queue;

/**
 * @brief ���䵱ǰ���õ���С�ļ���������
 * @return -1���������ʧ�ܣ����򷵻ر����䵽���ļ���������
*/
static int fd_alloc(void);
/**
 * @brief �����ļ���������
 * @param fd ��Ҫ���յ��ļ���������
*/
static void fd_free(int fd);
/**
 * @brief ����һ���豸��
 * @param name �豸����
 * @return NULL�����δ���ҵ����豸�����򷵻��豸ָ�롣
*/
static OS_Device find_device(const char* name);


/*-------newlib------*/
void* _sbrk(ptrdiff_t __incr) {
    OS_Prepare_Critical();
    OS_Enter_Critical();
    /*��ǰ�ѿռ��С*/
    static uint32_t cur = 0;
    void* ret = (void*)-1;
    if (cur + __incr > OS_MEM_TOTAL_COST) {
        /*��������ռ����ƣ�ִ��ʧ�ܡ�*/
        errno = ENOMEM;
    } else {
        errno = 0;
        /*��¼�¶ε���ʼλ�á�*/
        ret = (char*)OS_Heap_Base() + cur;
        /*���µ�ǰ�ѿռ��С��*/
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
    /*��ȡ��Ҫ�رյ��豸*/
    OS_Device device = fd_to_device[__fildes];
    /*����ر�ʧ�ܣ����ش���*/
    if (device->close() == -1)
        goto end;
    /*���ӳ�䡣*/
    fd_to_device[__fildes] = NULL;
    /*�ͷ��ļ���������*/
    fd_free(__fildes);
    ret = 0;
end:
    return ret;
}
int _open(const char* name, int mode, ...) {
    errno = 0;
    int ret = -1;
    /*�����豸�������豸��*/
    OS_Device device = find_device(name);
    /*���δ�ҵ������ش���*/
    if (!device)
        goto end;
    /*�����ʧ�ܣ����ش���*/
    if (device->open(mode) == -1)
        goto end;
    /*�����ļ���������*/
    int fd = fd_alloc();
    /*�������ʧ�ܣ����ش���*/
    if (fd == -1)
        goto end;
    /*����ӳ���ϵ��*/
    fd_to_device[fd] = device;
    /*�����ļ���������*/
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
    /*��ʼ���ļ���������������*/
    OS_Priority_Queue_Init(&fd_free_queue);
    for (int i = 0; i < OS_FD_CNT; i++) {
        OS_Priority_Queue_Node_Init(fd_node + i, i, NULL);
        OS_Priority_Queue_Push(&fd_free_queue, fd_node + i);
    }
    /*���ñ�׼�����������*/
    fd_to_device[fd_alloc()] = stdin_dev;
    fd_to_device[fd_alloc()] = stdout_dev;
    fd_to_device[fd_alloc()] = stderr_dev;
}