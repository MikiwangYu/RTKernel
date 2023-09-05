#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "mkwos.h"

void OS_SysTick_Handler(void) {
    os_assert_no_return(OS_Started());
    OS_Intr_Enter();
    HAL_IncTick();
    OS_Tick_Inc();
    OS_Intr_Exit();
}

#define PENDSV() (SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk)
void OS_Task_Switch(void) {
    PENDSV();
}

void OS_Intr_Task_Switch(void) {
    PENDSV();
}

void OS_Task_Start(void) {
    SCB->SHP[SCB_SHCSR_PENDSVACT_Pos] = 0xff;
    PENDSV();
}

void* OS_Task_Stack_Init(void* _stack_p, void* exe_addr, void* return_addr, void* arg) {
    uint32_t* stack_p = _stack_p;
    *(--stack_p) = 0x01000000;      //xPSR
    *(--stack_p) = (uint32_t)exe_addr;          //PC
    *(--stack_p) = (uint32_t)return_addr;       //LR
    *(--stack_p) = 0x00000000;      //R12
    *(--stack_p) = 0x00000000;      //R3
    *(--stack_p) = 0x00000000;      //R2
    *(--stack_p) = 0x00000000;      //R1
    *(--stack_p) = (uint32_t)arg;   //R0



    *(--stack_p) = 0x00000000;      //R11
    *(--stack_p) = 0x00000000;      //R10
    *(--stack_p) = 0x00000000;      //R9
    *(--stack_p) = 0x00000000;      //R8
    *(--stack_p) = 0x00000000;      //R7
    *(--stack_p) = 0x00000000;      //R6
    *(--stack_p) = 0x00000000;      //R5
    *(--stack_p) = 0x00000000;      //R4
    *(--stack_p) = 0xfffffffd;      //LR
    return stack_p;
}

