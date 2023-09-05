#include "os_int.h"
#include "os_task.h"
#include "os_cpu.h"

/**
 * @brief �ж�Ƕ�ײ�����������
*/
static uint32_t int_cnt;

bool OS_In_Intr(void) {
    /*�����������Ϊ0������Ϊ�����ж��С�*/
    return int_cnt;
}

uint32_t OS_Intr_Cnt(void) {
    return int_cnt;
}

void OS_Intr_Enter(void) {
    /*���Ӽ�������*/
    int_cnt++;
}

void OS_Intr_Exit(void) {
    OS_Prepare_Critical();
    OS_Enter_Critical();
    /*���ټ�������*/
    int_cnt--;
    /*����Ƴ������һ���жϣ�����Ҫ����Ƿ���Ҫ����������ȡ�*/
    if (!int_cnt && OS_Need_Sched()) {
        OS_Sched_New();
        OS_Intr_Task_Switch();
    }
    OS_Exit_Critical();
}
