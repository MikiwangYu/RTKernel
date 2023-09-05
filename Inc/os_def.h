
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "os_cfg.h"
#include "ds/os_list.h"
#include "ds/os_priority_queue.h"
#include "stddef.h"
#include "stdint.h"
/**
 * @brief ����ϵͳ��ʱ����С��λ���ͣ�ʵ��ʱ����OS_TICK_PER_SEC������
 * @see OS_TICK_PER_SEC��
*/
typedef uint32_t OS_Tick;
/**
 * @brief CPU״̬�Ĵ������͡�
*/
typedef uint32_t OS_CPU_SR;
/**
 * @brief ���ȼ����ͣ��������ȼ������������ȼ������̻߳ص��������ȼ���
*/
typedef uint32_t OS_Prio;



/**
 * @brief �ص������Ĵ���Դ��
*/
typedef void* OS_Callback_Src_t;
/**
 * @brief �ص������Ĳ�����
*/
typedef void* OS_Callback_Arg_t;
/**
 * @brief �ص�������
 * @param src �ص������Ĵ���Դ��
 * @param arg �ص������Ĳ�����
*/
typedef void (*OS_Callback_Func_t)(OS_Callback_Src_t src, OS_Callback_Arg_t arg);





/**
 * @brief ������롣
*/
typedef enum _OS_Error_t {
    /**
     * @brief ϵͳ������
    */
    OS_CRASH = -1,
    /**
     * @brief ִ�гɹ���
    */
    OS_OK = 0,
    /**
     * @brief ִ��ʧ�ܡ�
    */
    OS_ERROR,
    /**
     * @brief �ȴ���ʱ��
    */
    OS_TIMEOUT,
    /**
     * @brief Ȩ�޴���
    */
    OS_PERMISSION_DENY,
    /**
     * @brief �Ƿ�������
    */
    OS_ILLEGAL_PARAM,
    /**
     * @brief �ȴ���ʱ��
    */
    OS_TIME_OUT
}OS_Error_t;


#define os_assert(expr,ret)         do{if(!(expr))return ret;}while(false)
#define os_assert_no_return(expr)   do{if(!(expr))return;}while(false)

#if OS_PARAM_CHECK_EN
#define os_param_assert(expr,ret)       os_assert(expr,ret)
#define os_param_assert_no_return(expr) os_assert_no_return(expr)
#else
#define os_param_assert(expr,ret)       ((void)0)
#define os_param_assert_no_return(expr) ((void)0)
#endif


#ifdef __cplusplus
}
#endif