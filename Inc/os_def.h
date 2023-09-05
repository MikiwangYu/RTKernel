
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
 * @brief 操作系统计时的最小单位类型，实际时长由OS_TICK_PER_SEC决定。
 * @see OS_TICK_PER_SEC。
*/
typedef uint32_t OS_Tick;
/**
 * @brief CPU状态寄存器类型。
*/
typedef uint32_t OS_CPU_SR;
/**
 * @brief 优先级类型，任务优先级，互斥量优先级，主线程回调函数优先级。
*/
typedef uint32_t OS_Prio;



/**
 * @brief 回调函数的触发源。
*/
typedef void* OS_Callback_Src_t;
/**
 * @brief 回调函数的参数。
*/
typedef void* OS_Callback_Arg_t;
/**
 * @brief 回调函数。
 * @param src 回调函数的触发源。
 * @param arg 回调函数的参数。
*/
typedef void (*OS_Callback_Func_t)(OS_Callback_Src_t src, OS_Callback_Arg_t arg);





/**
 * @brief 错误代码。
*/
typedef enum _OS_Error_t {
    /**
     * @brief 系统崩溃。
    */
    OS_CRASH = -1,
    /**
     * @brief 执行成功。
    */
    OS_OK = 0,
    /**
     * @brief 执行失败。
    */
    OS_ERROR,
    /**
     * @brief 等待超时。
    */
    OS_TIMEOUT,
    /**
     * @brief 权限错误。
    */
    OS_PERMISSION_DENY,
    /**
     * @brief 非法参数。
    */
    OS_ILLEGAL_PARAM,
    /**
     * @brief 等待超时。
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