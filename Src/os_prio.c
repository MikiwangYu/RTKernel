#include "os_prio.h"

#if OS_MAX_PRIO<=64
#if !OS_PRIO_MULTI_TASK
static OS_Prio_Table_Node *os_prio_table[OS_MAX_PRIO];
#endif

static const uint8_t lowbit[1 << 8] = {
    0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
};


void OS_Prio_Table_Node_Init(OS_Prio_Table_Node* p, void* data) {
    OS_List_Node_Init(p, data);
}

void OS_Prio_Table_Init(OS_Prio_Table* table) {
    os_param_assert_no_return(table);
    table->_high = 0;
    for (int i = 0; i < 8; i++)
        table->_low[i] = 0;
#if OS_PRIO_MULTI_TASK
    for (int i = 0; i < OS_MAX_PRIO; i++)
        OS_List_Init(&table->_head[i]);
#endif
}



void OS_Prio_Table_Mount(OS_Prio_Table* table, OS_Prio_Table_Node* node, OS_Prio prio) {
    os_param_assert_no_return(table);
    os_param_assert_no_return(node);
    os_param_assert_no_return(prio<OS_MAX_PRIO);
    OS_Prio h = prio >> 3;
    OS_Prio l = prio & 7;
#if OS_PRIO_MULTI_TASK
    OS_List_Insert(&table->_head[prio], table->_head[prio], node);
    if (!table->_head[prio]->next) {
        OS_List_Node* node = table->_head[prio];
        node->next = node->pre = node;
    }
#else
    os_prio_table[prio] = node;
#endif
    table->_low[h] |= 1 << l;
    table->_high |= 1 << h;
}


void OS_Prio_Table_UnMount(OS_Prio_Table* table, OS_Prio_Table_Node* node, OS_Prio prio) {
    os_param_assert_no_return(table);
    os_param_assert_no_return(node);
    os_param_assert_no_return(prio < OS_MAX_PRIO);
    OS_Prio h = prio >> 3;
    OS_Prio l = prio & 7;
#if OS_PRIO_MULTI_TASK
    OS_List_Erase(&table->_head[prio], node);
    if (!table->_head[prio])
        table->_low[h] &= ~(1 << l);
#else
    os_prio_table[prio] = NULL;
    table->_low[h] &= ~(1 << l);
#endif
    if (!table->_low[h])
        table->_high &= ~(1 << h);
}

OS_Prio OS_Prio_Table_Get_Prio(OS_Prio_Table* table) {
    os_param_assert(table, OS_MAX_PRIO);
    if (!table->_high)
        return OS_MAX_PRIO;
    OS_Prio h = lowbit[table->_high];
    OS_Prio l = lowbit[table->_low[h]];
    return h << 3 | l;
}

OS_Prio_Table_Node* OS_Prio_Table_Get_Next(OS_Prio_Table* table, OS_Prio prio) {
    os_param_assert(table, NULL);
    OS_Prio new_prio = OS_Prio_Table_Get_Prio(table);
    if (new_prio == OS_MAX_PRIO)
        return NULL;
#if OS_PRIO_MULTI_TASK
    table->_head[new_prio] = table->_head[new_prio]->next;
    return table->_head[new_prio];
#else
    return os_prio_table[new_prio];
#endif
}
#else
#error OS_MAX_PRIO must not be greater than 64!
#endif