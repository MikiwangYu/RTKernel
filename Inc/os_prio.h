#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "os_def.h"
/*
* 当优先级数量小于等于64时，采用优先级位图法。
* 当优先级数量大于64时，使用优先队列。
*/
#if OS_MAX_PRIO<=64
/**
 * @brief 优先级表。
*/
typedef struct _OS_Prio_Table {
    /**
     * @brief 高三位位图。
    */
    uint8_t _high;
    /**
     * @brief 低三位位图。
    */
    uint8_t _low[8];
#if OS_PRIO_MULTI_TASK
    /**
     * @brief 就绪表。
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
 * @brief 优先级表。
*/
typedef struct _OS_Prio_Table {
    /**
     * @brief 就绪优先队列。
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
 * @brief 初始化一个优先级表。
 * @param table 需要初始化的优先级表。
*/
void OS_Prio_Table_Init(OS_Prio_Table* table);
/**
 * @brief 将一个节点挂载到优先级表上。
 * @param table 需要操作的优先级表。
 * @param node 需要挂载的节点。
 * @param prio 节点的优先级。
*/
void OS_Prio_Table_Mount(OS_Prio_Table* table, OS_Prio_Table_Node* node, OS_Prio prio);
/**
 * @brief 将一个节点从优先级表上卸载。
 * @param table 需要操作的优先级表。
 * @param node 需要卸载的节点。
 * @param prio 节点的优先级。
*/
void OS_Prio_Table_UnMount(OS_Prio_Table* table, OS_Prio_Table_Node* node, OS_Prio prio);
/**
 * @brief 获取优先级表的最高优先级。
 * @param table 需要操作的优先级表。
 * @return 若优先级表不为空，返回最高优先级，否则返回OS_MAX_PRIO。
 * @see OS_MAX_PRIO。
*/
OS_Prio OS_Prio_Table_Get_Prio(OS_Prio_Table* table);
/**
 * @brief 获取优先级表中最高优先级的一个节点。
 * @param table 需要操作的优先级表。
 * @param prio 当前已经获取到的节点的优先级。
 * @return 优先级最高的一个节点，如果最高优先级的节点有多个，则返回值不与上一次的返回值相同；如果优先级表为空，返回NULL。
*/
OS_Prio_Table_Node* OS_Prio_Table_Get_Next(OS_Prio_Table* table, OS_Prio prio);

#ifdef __cplusplus
}
#endif