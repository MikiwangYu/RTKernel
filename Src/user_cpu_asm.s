.section .text.user_cpu_asm

.global OS_CPU_SR_Save
.global OS_CPU_SR_Restore
.global OS_PendSV_Handler

.extern OS_Cur_Task
.extern OS_Next_Task
.extern OS_PendSV_Callback
.extern OS_Task_Stack_Overflow

.thumb
.syntax unified

.thumb_func
OS_CPU_SR_Save:
    MRS     R0,PRIMASK

    CPSID   I           @¹ØÖÐ¶Ï
    BX      LR

.thumb_func
OS_CPU_SR_Restore:
    MSR     PRIMASK,R0
    BX      LR








.thumb_func
OS_PendSV_Handler:
    CPSID   I

    @R0 = OS_Cur_Task
    LDR     R0,=OS_Cur_Task
    LDR     R0,[R0]

    CBZ     R0,LOAD

SAVE:

    MRS     R0,PSP



    STMFD   R0!,{R4-R11}

    AND     R1,LR,#0x10
    CBNZ    R1,NO_SAVE_FP
    VSTMDB  R0!,{S16-S31}

NO_SAVE_FP:

    STMFD   R0!,{LR}

    @OS_Cur_Task->stack_p = R0
    LDR     R1,=OS_Cur_Task
    LDR     R1,[R1]
    STR     R0,[R1]

    LDR     R0,=OS_Task_Stack_Overflow
    BLX     R0


    CBZ    R0,LOAD
LOOP:
    B       LOOP

LOAD:
    @R0 = OS_Next_Task->stack_p
    LDR     R0,=OS_Next_Task
    LDR     R0,[R0]
    LDR     R0,[R0]

    LDMFD   R0!,{LR}
    
    AND     R1,LR,#0x10
    CBNZ    R1,NO_LOAD_FP
    VLDMIA  R0!,{S16-S31}
NO_LOAD_FP:


    LDMFD   R0!,{R4-R11}



    MSR     PSP,R0



    @OS_Cur_Task = OS_Next_Task
    LDR     R0,=OS_Cur_Task
    LDR     R1,=OS_Next_Task
    LDR     R1,[R1]
    STR     R1,[R0]

    @OS_Next_Task = 0
    MOV     R0,#0x0
    LDR     R1,=OS_Next_Task
    STR     R0,[R1]



    CPSIE   I
    BX      LR
.end


















