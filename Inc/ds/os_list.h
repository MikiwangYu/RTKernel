#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ����ͷ�ڵ�ĵ�����
*/
typedef struct _OS_Foward_List_Node OS_Forward_List_Node, * OS_Forward_List;

struct _OS_Foward_List_Node {
    OS_Forward_List next;
    void* data;
};
/**
 * @brief ��ʼ��һ������ڵ㣬��nextָ�����㣬������data��
 * @param p ��Ҫ��ʼ���Ľڵ㡣
 * @param data �ڵ���Ҫ��������ݡ�
*/
void OS_Forward_List_Node_Init(OS_Forward_List_Node* p, void* data);
/**
 * @brief ��ʼ��һ��������
 * @param pl ��Ҫ��ʼ��������
*/
void OS_Forward_List_Init(OS_Forward_List* pl);
/**
 * @brief ��һ���ڵ���뵽һ���������С�
 * @param pos ����λ��,*pos��ֵ�ᱻ��Ϊlist��
 * @param node ��Ҫ����ĵ�����ڵ㡣
*/
void OS_Forward_List_Insert(OS_Forward_List* pos, OS_Forward_List_Node* node);
/**
 * @brief ɾ�������е�һ���ڵ㡣
 * @param pos *posΪ��Ҫɾ���Ľڵ㡣
*/
void OS_Forward_List_Erase(OS_Forward_List* pos);

















/**
 * @brief ����ͷ�ڵ��˫����
*/
typedef struct _OS_List_Node OS_List_Node, * OS_List;

struct _OS_List_Node {
    OS_List pre, next;
    void* data;
};

/**
 * @brief ��ʼ��һ������ڵ㣬��next��preָ�����㣬������data��
 * @param p ��Ҫ��ʼ���Ľڵ㡣
 * @param data �ڵ���Ҫ��������ݡ�
*/
void OS_List_Node_Init(OS_List_Node* p, void* data);
/**
 * @brief ��ʼ��һ��˫����
 * @param pl ��Ҫ��ʼ��������
*/
void OS_List_Init(OS_List* pl);
/**
 * @brief ��һ��˫������뵽��һ��˫�����С�
 * @param pl ��Ҫ������˫����
 * @param pos ����λ�ã����ΪNULL����ʾ���뵽��ͷ��������뵽pos֮��
 * @param node ��Ҫ����Ľڵ㡣
*/
void OS_List_Insert(OS_List *pl,OS_List pos, OS_List_Node* node);
/**
 * @brief ɾ�������е�һ���ڵ㡣
 * @param pl ��Ҫ����������
 * @param node ��Ҫɾ���Ľڵ㡣
*/
void OS_List_Erase(OS_List* pl, OS_List_Node* node);

#ifdef __cplusplus
}
#endif