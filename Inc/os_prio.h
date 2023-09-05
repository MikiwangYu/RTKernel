#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "os_def.h"
/*
* �����ȼ�����С�ڵ���64ʱ���������ȼ�λͼ����
* �����ȼ���������64ʱ��ʹ�����ȶ��С�
*/
#if OS_MAX_PRIO<=64
/**
 * @brief ���ȼ���
*/
typedef struct _OS_Prio_Table {
    /**
     * @brief ����λλͼ��
    */
    uint8_t _high;
    /**
     * @brief ����λλͼ��
    */
    uint8_t _low[8];
#if OS_PRIO_MULTI_TASK
    /**
     * @brief ������
    */
    OS_List _head[OS_MAX_PRIO], _cur[OS_MAX_PRIO];
#endif
}OS_Prio_Table;
/**
 * @brief 
*/
typedef OS_List_Node OS_Prio_Table_Node;
#else
/**
 * @brief ���ȼ���
*/
typedef struct _OS_Prio_Table {
    /**
     * @brief �������ȶ��С�
    */
    OS_Priority_Queue _queue;
}OS_Prio_Table;
/**
 * @brief 
*/
typedef OS_Priority_Queue_Node OS_Prio_Table_Node;
#endif

void OS_Prio_Table_Node_Init(OS_Prio_Table_Node* p, void* data);
/**
 * @brief ��ʼ��һ�����ȼ���
 * @param table ��Ҫ��ʼ�������ȼ���
*/
void OS_Prio_Table_Init(OS_Prio_Table* table);
/**
 * @brief ��һ���ڵ���ص����ȼ����ϡ�
 * @param table ��Ҫ���������ȼ���
 * @param node ��Ҫ���صĽڵ㡣
 * @param prio �ڵ�����ȼ���
*/
void OS_Prio_Table_Mount(OS_Prio_Table* table, OS_Prio_Table_Node* node, OS_Prio prio);
/**
 * @brief ��һ���ڵ�����ȼ�����ж�ء�
 * @param table ��Ҫ���������ȼ���
 * @param node ��Ҫж�صĽڵ㡣
 * @param prio �ڵ�����ȼ���
*/
void OS_Prio_Table_UnMount(OS_Prio_Table* table, OS_Prio_Table_Node* node, OS_Prio prio);
/**
 * @brief ��ȡ���ȼ����������ȼ���
 * @param table ��Ҫ���������ȼ���
 * @return �����ȼ���Ϊ�գ�����������ȼ������򷵻�OS_MAX_PRIO��
 * @see OS_MAX_PRIO��
*/
OS_Prio OS_Prio_Table_Get_Prio(OS_Prio_Table* table);
/**
 * @brief ��ȡ���ȼ�����������ȼ���һ���ڵ㡣
 * @param table ��Ҫ���������ȼ���
 * @param prio ��ǰ�Ѿ���ȡ���Ľڵ�����ȼ���
 * @return ���ȼ���ߵ�һ���ڵ㣬���������ȼ��Ľڵ��ж�����򷵻�ֵ������һ�εķ���ֵ��ͬ��������ȼ���Ϊ�գ�����NULL��
*/
OS_Prio_Table_Node* OS_Prio_Table_Get_Next(OS_Prio_Table* table, OS_Prio prio);

#ifdef __cplusplus
}
#endif