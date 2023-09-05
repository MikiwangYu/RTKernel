#include "os_int.h"
#include "os_task.h"
#include "os_cpu.h"

/**
 * @brief 中断嵌套层数计数器。
*/
static uint32_t int_cnt;

bool OS_In_Intr(void) {
    /*如果计数器不为0，则视为处于中断中。*/
    return int_cnt;
}

uint32_t OS_Intr_Cnt(void) {
    return int_cnt;
}

void OS_Intr_Enter(void) {
    /*增加计数器。*/
    int_cnt++;
}

void OS_Intr_Exit(void) {
    OS_Prepare_Critical();
    OS_Enter_Critical();
    /*减少计数器。*/
    int_cnt--;
    /*如果推出了最后一层中断，则需要检查是否需要进行任务调度。*/
    if (!int_cnt && OS_Need_Sched()) {
        OS_Sched_New();
        OS_Intr_Task_Switch();
    }
    OS_Exit_Critical();
}
