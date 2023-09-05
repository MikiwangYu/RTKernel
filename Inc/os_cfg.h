
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "stdbool.h"

/*----------���ܲü�----------*/

/*���ò�����顣*/
#define OS_PARAM_CHECK_EN   1
/*����ջ�����顣*/
#define OS_STACK_CHECK_EN   1
/*���ö�ʱ����*/
#define OS_TIMER_EN         1

/*����ͬ���ȼ�������*/
#define OS_PRIO_MULTI_TASK  0
/*�����������ơ�*/
#define OS_TASK_NAME_EN     1
/*���������ź�����*/
#define OS_TASK_SEM_EN      1
/*��������ʱ��Ƭ��ת��*/
#define OS_TASK_SLICE_EN    0

#if OS_TASK_SEM_EN
/*���������ź������ơ�*/
#define OS_TASK_SEM_NAME_EN 1
#endif

/*�����¼����ơ�*/
#define OS_EVENT_EN         1



#if OS_EVENT_EN
/*�����¼�����*/
#define OS_EVENT_NAME_EN    1
/*�������ȼ��컨�塣*/
#define OS_MUTEX_EN         1
#endif


/*���ö�ʱ�����ơ�*/
#define OS_TIMER_NAME_EN    1


/*�������̣߳��������߳����ڶ��������ܵ�ǰ��������*/
#define OS_MAIN_TASK_EN     1


/*���ò���ϵͳ����̬�ڴ档*/
#define OS_MEM_EN           0

#if OS_MAIN_TASK_EN

/*�����ж����¼��ӳٷ������������¼���������ر��жϣ��ж��е��¼��������ᱻ�ӳ١�*/
#define OS_INTR_DELAY_EN    1
#endif

#if !OS_MEM_EN
/*newlib��Сʵ�֡�*/
#define OS_NEWLIB_EN            1
#endif

/*----------��ֵ�趨----------*/
/*֧�ֵ�������ȼ������������趨Ϊ4~64��*/
#define OS_MAX_PRIO             16
/*Ԥ����������ƿ�������*/
#define OS_TASK_PRE_ALLOC_CNT   16


/*��ʱ���ʱ������*/
#define OS_CHECK_TIMEOUT_FREQ   20

#if OS_TASK_SLICE_EN
/*ͬ������ʱ��Ƭ���ȡ�*/
#define OS_TASK_SLICE           4
#endif

#if OS_EVENT_EN
/*Ԥ�����¼����ƿ�������*/
#define OS_EVENT_PRE_ALLOC_CNT  32
#endif

/*��������ջ��С��*/
#define OS_IDLE_TASK_STACK_SIZE     128

#if OS_MAIN_TASK_EN
/*Ԥ�������̻߳ص�����������*/
#define OS_MAIN_TASK_CALLBACK_CNT   64
/*���̵߳�ջ��С*/
#define OS_MAIN_TASK_STACK_SIZE     4096
#endif

#if OS_MEM_EN
/*��̬�ڴ�������С���С����λByte��*/
#define OS_MEM_PER_BLOCK    64
/*��̬�ڴ�㼶��*/
#define OS_MEM_LEVEL        10
/*��̬�ڴ��ܴ�С��*/
#define OS_MEM_TOTAL_COST   (OS_MEM_PER_BLOCK << OS_MEM_LEVEL)
#else
/*��̬�ڴ��ܴ�С��*/
#define OS_MEM_TOTAL_COST   (80 << 10)
#endif
/*ÿ���Ӳ���ϵͳ����SysTick�Ĵ�����*/
#define OS_TICK_PER_SEC     1000

#if OS_NEWLIB_EN
/*�豸������������*/
#define OS_DEVICE_CNT       16
/*�ļ�������������*/
#define OS_FD_CNT           16
#endif
#ifdef __cplusplus
}
#endif