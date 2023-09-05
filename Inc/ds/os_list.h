#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 不带头节点的单链表。
*/
typedef struct _OS_Foward_List_Node OS_Forward_List_Node, * OS_Forward_List;

struct _OS_Foward_List_Node {
    OS_Forward_List next;
    void* data;
};
/**
 * @brief 初始化一个链表节点，将next指针置零，并设置data。
 * @param p 需要初始化的节点。
 * @param data 节点需要保存的数据。
*/
void OS_Forward_List_Node_Init(OS_Forward_List_Node* p, void* data);
/**
 * @brief 初始化一个单链表。
 * @param pl 需要初始化的链表。
*/
void OS_Forward_List_Init(OS_Forward_List* pl);
/**
 * @brief 将一个节点插入到一条单链表中。
 * @param pos 插入位置,*pos的值会被置为list。
 * @param node 需要插入的单链表节点。
*/
void OS_Forward_List_Insert(OS_Forward_List* pos, OS_Forward_List_Node* node);
/**
 * @brief 删除链表中的一个节点。
 * @param pos *pos为需要删除的节点。
*/
void OS_Forward_List_Erase(OS_Forward_List* pos);

















/**
 * @brief 不带头节点的双链表。
*/
typedef struct _OS_List_Node OS_List_Node, * OS_List;

struct _OS_List_Node {
    OS_List pre, next;
    void* data;
};

/**
 * @brief 初始化一个链表节点，将next和pre指针置零，并设置data。
 * @param p 需要初始化的节点。
 * @param data 节点需要保存的数据。
*/
void OS_List_Node_Init(OS_List_Node* p, void* data);
/**
 * @brief 初始化一个双链表。
 * @param pl 需要初始化的链表。
*/
void OS_List_Init(OS_List* pl);
/**
 * @brief 将一条双链表插入到另一条双链表中。
 * @param pl 需要操作的双链表。
 * @param pos 插入位置，如果为NULL，表示插入到开头，否则插入到pos之后。
 * @param node 需要插入的节点。
*/
void OS_List_Insert(OS_List *pl,OS_List pos, OS_List_Node* node);
/**
 * @brief 删除链表中的一个节点。
 * @param pl 需要操作的链表。
 * @param node 需要删除的节点。
*/
void OS_List_Erase(OS_List* pl, OS_List_Node* node);

#ifdef __cplusplus
}
#endif