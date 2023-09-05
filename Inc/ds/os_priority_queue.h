#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include "stdint.h"
#include "stdbool.h"

/**
 * @brief ���ȶ��У�С���ѣ�������ƫ��ʵ�֡�
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
 * @brief ��ʼ��һ�����ȶ��еĽڵ㡣
 * @param p ��Ҫ��ʼ���Ľڵ㡣
 * @param key ��ֵ��ֵԽС��Ѷ�Խ����
 * @param data �ڵ㱣������ݡ�
*/
void OS_Priority_Queue_Node_Init(OS_Priority_Queue_Node* p, uint32_t key, void* data);
/**
 * @brief ��ʼ��һ�����ȶ��С�
 * @param pq ��Ҫ��ʼ�������ȶ��С�
*/
void OS_Priority_Queue_Init(OS_Priority_Queue *pq);

bool OS_Priority_Queue_Empty(OS_Priority_Queue queue);
/**
 * @brief �����ȶ����в���ڵ㡣
 * @param pq ��Ҫ���������ȶ��С�
 * @param qb ��Ҫ����Ľڵ㡣
*/
void OS_Priority_Queue_Push(OS_Priority_Queue *pq, OS_Priority_Queue_Node *qb);
/**
 * @brief ��ȡ���ȶ��еĶѶ���
 * @param queue ��Ҫ���������ȶ��С�
 * @return �Ѷ��ڵ㡣
*/
OS_Priority_Queue_Node* OS_Priority_Queue_Top(OS_Priority_Queue queue);
/**
 * @brief �����Ѷ���
 * @param queue ��Ҫ���������ȶ��С�
*/
void OS_Priority_Queue_Pop(OS_Priority_Queue *pq);
/**
 * @brief ɾ��һ���ڵ㡣
 * @param pq ��Ҫ���������ȶ��С�
 * @param node ��Ҫɾ���Ľڵ㡣
*/
void OS_Priority_Queue_Erase(OS_Priority_Queue* pq, OS_Priority_Queue_Node* node);
#ifdef __cplusplus
}
#endif