#include "ds/os_list.h"
#include "stddef.h"

void OS_Forward_List_Node_Init(OS_Forward_List_Node* p, void* data) {
    p->next = NULL;
    p->data = data;
}
 

void OS_Forward_List_Init(OS_Forward_List* pl) {
    *pl = NULL;
}

void OS_Forward_List_Insert(OS_Forward_List* pos, OS_Forward_List_Node* node) {
    node->next = *pos;
    *pos = node;
}

void OS_Forward_List_Erase(OS_Forward_List* pos) {
    OS_Forward_List tail = (*pos)->next;
    (*pos)->next = NULL;
    *pos = tail;
}

void OS_List_Node_Init(OS_List_Node* p, void* data) {
    p->pre = p->next = NULL;
    p->data = data;
}

void OS_List_Init(OS_List* pl) {
    *pl = NULL;
}


void OS_List_Insert(OS_List* pl, OS_List pos, OS_List_Node* node) {
    node->pre = pos;
    if (pos) {
        node->next = pos->next;
    } else {
        node->next = *pl;
        *pl = node;
    }
    if (node->pre)
        node->pre->next = node;
    if (node->next)
        node->next->pre = node;
}

void OS_List_Erase(OS_List* pl, OS_List_Node* node) {
    if (node->next == node) {
        *pl = NULL;
    } else {
        if (*pl == node)
            *pl = node->next;
        if (node->pre)
            node->pre->next = node->next;
        if (node->next)
            node->next->pre = node->pre;
    }
    OS_List_Node_Init(node, node->data);
}
