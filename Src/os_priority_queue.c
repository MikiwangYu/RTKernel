#include "ds/os_priority_queue.h"
#include "stddef.h"

static void refresh(OS_Priority_Queue q) {
    uint32_t d[2] = {
        q->_son[0] ? q->_son[0]->_dep : 0,
        q->_son[1] ? q->_son[1]->_dep : 0,
    };
    if (d[0] < d[1]) {
        q->_dep = d[0] + 1;
        void* ptr = q->_son[0];
        q->_son[0] = q->_son[1];
        q->_son[1] = ptr;
    } else {
        q->_dep = d[1] + 1;
    }
}

void OS_Priority_Queue_Node_Init(OS_Priority_Queue_Node* p, uint32_t key, void* data) {
    p->key = key;
    p->data = data;
    p->_dep = 1;
    p->_f = NULL;
    p->_son[0] = p->_son[1] = NULL;
}

void OS_Priority_Queue_Init(OS_Priority_Queue* pq) {
    *pq = 0;
}

bool OS_Priority_Queue_Empty(OS_Priority_Queue queue) {
    return !queue;
}

OS_Priority_Queue merge(OS_Priority_Queue qa, OS_Priority_Queue qb) {
    if (!qa)return qb;
    if (!qb)return qa;
    if (qa->key < qb->key) {
        qa->_son[1] = merge(qa->_son[1], qb);
        if(qa->_son[1])
            qa->_son[1]->_f = qa;
        refresh(qa);
        return qa;
    } else {
        qb->_son[1] = merge(qb->_son[1], qa);
        if (qb->_son[1])
            qb->_son[1]->_f = qb;
        refresh(qb);
        return qb;
    }
}

void OS_Priority_Queue_Push(OS_Priority_Queue* pq, OS_Priority_Queue_Node* qb) {
    *pq = merge(*pq, qb);
}

OS_Priority_Queue_Node* OS_Priority_Queue_Top(OS_Priority_Queue queue) {
    return queue;
}

void OS_Priority_Queue_Pop(OS_Priority_Queue* pq) {
    OS_Priority_Queue_Erase(pq, OS_Priority_Queue_Top(*pq));
}

void OS_Priority_Queue_Erase(OS_Priority_Queue* pq, OS_Priority_Queue_Node* node) {
    if (*pq == node) {
        *pq = merge(node->_son[0], node->_son[1]);
        if (*pq)
            (* pq)->_f = NULL;
    } else {
        int bl = (node->_f->_son[1] == node);
        OS_Priority_Queue_Node* f = node->_f;
        f->_son[bl] = merge(node->_son[0], node->_son[1]);
        if (f->_son[bl])
            f->_son[bl] = f;
        OS_Priority_Queue_Node* p = f;
        while (p) {
            refresh(p);
            p = p->_f;
        }
    }
    OS_Priority_Queue_Node_Init(node, node->key, node->data);
}