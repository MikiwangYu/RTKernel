#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include "stdint.h"
#include "stdbool.h"

/**
 * @brief 优先队列，小根堆，采用左偏树实现。
*/
typedef struct _OS_Priority_Queue_Node OS_Priority_Queue_Node, * OS_Priority_Queue;
struct _OS_Priority_Queue_Node {
    uint32_t key;
    void* data;
    uint32_t _dep;
    OS_Priority_Queue_Node* _f;
    OS_Priority_Queue_Node* _son[2];
};
/**
 * @brief 初始化一个优先队列的节点。
 * @param p 需要初始化的节点。
 * @param key 键值，值越小离堆顶越近。
 * @param data 节点保存的数据。
*/
void OS_Priority_Queue_Node_Init(OS_Priority_Queue_Node* p, uint32_t key, void* data);
/**
 * @brief 初始化一个优先队列。
 * @param pq 需要初始化的优先队列。
*/
void OS_Priority_Queue_Init(OS_Priority_Queue *pq);

bool OS_Priority_Queue_Empty(OS_Priority_Queue queue);
/**
 * @brief 向优先队列中插入节点。
 * @param pq 需要操作的优先队列。
 * @param qb 需要插入的节点。
*/
void OS_Priority_Queue_Push(OS_Priority_Queue *pq, OS_Priority_Queue_Node *qb);
/**
 * @brief 获取优先队列的堆顶。
 * @param queue 需要操作的优先队列。
 * @return 堆顶节点。
*/
OS_Priority_Queue_Node* OS_Priority_Queue_Top(OS_Priority_Queue queue);
/**
 * @brief 弹出堆顶。
 * @param queue 需要操作的优先队列。
*/
void OS_Priority_Queue_Pop(OS_Priority_Queue *pq);
/**
 * @brief 删除一个节点。
 * @param pq 需要操作的优先队列。
 * @param node 需要删除的节点。
*/
void OS_Priority_Queue_Erase(OS_Priority_Queue* pq, OS_Priority_Queue_Node* node);
#ifdef __cplusplus
}
#endif