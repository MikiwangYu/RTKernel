#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "os_def.h"
#if OS_NEWLIB_EN

/**
 * @brief �豸����Ҫ�û����д�����ע�ᡣ
*/
typedef struct _OS_DEV {
    /**
     * @brief �豸����
    */
    const char* name;
    /**
     * @brief ��ȡ�豸��
    */
    int (*read)(void*, size_t);
    /**
     * @brief д���豸��
    */
    int (*write)(const void*, size_t);
    /**
     * @brief ���豸��
    */
    int (*open)(int);
    /**
     * @brief �ر��豸��
    */
    int (*close)(void);
}OS_DEV, *OS_Device;

/**
 * @brief �����ϵͳע��һ���豸��
 * @param device ��Ҫע����豸��
 * @return �豸��������
*/
int OS_Device_Register(OS_Device device);
/**
 * @brief ��ʼ��ȫ��IO��
 * @param stdin_dev ��׼�������豸��
 * @param stdout_dev ��׼������豸��
 * @param stderr_dev ��׼�������豸��
*/
void OS_IO_Init(OS_Device stdin_dev, OS_Device stdout_dev, OS_Device stderr_dev);

#endif


#ifdef __cplusplus
}
#endif