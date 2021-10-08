;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; @file co_port_hs6621_asm.s
; @brief 
; @version 1.0
; @date Mon 17 Nov 2014 01:40:33 PM CST
; @author liqiang
;
; @note 
;;

CO_CPU_STATUS_SAVE  EQU     0x400E00F0
CO_RAM_GATE_REG     EQU     0x40001050
CO_CPM_UPD_REG      EQU     0x4000100C
CO_PMU_STATE_REG    EQU     0x400e0000

                    ; AREA for cpu sp data
                    AREA    |.data|, DATA, READWRITE

CO_CPU_SP_SAVE      DCD     0


                    ; AREA for cpu check code
                    AREA    |.text|, CODE, READONLY

co_cpu_check        PROC
                    EXPORT co_cpu_check

                    ; Check cpu status
                    LDR R0, =CO_CPU_STATUS_SAVE
                    LDR R0, [R0]
                    CMP R0, #2
                    BEQ CO_CPU_SUSPEND_RESUME

                    ; Normal system reset
                    BX LR

                    ENDP


co_ram_reset_handler PROC
                    EXPORT co_ram_reset_handler

                    BL co_cpu_check
                    B .

                    ENDP


co_cpu_suspend      PROC
                    EXPORT co_cpu_suspend

                    ; Push register to stack
                    PUSH {R0-R12, LR}

                    ; Save SP pointer
                    LDR R1, =CO_CPU_SP_SAVE
                    MRS R0, MSP
                    STR R0, [R1]

                    ; Save CPU status to PMU
                    LDR R1, =CO_CPU_STATUS_SAVE
                    MOV R0, #2
                    STR R0, [R1]

                    ; Good Night
                    WFI

                    ; Resume Place
CO_CPU_SUSPEND_RESUME

                    ; Wait system ready, Check PMU_STATE (PMU_BASIC[31:27]) equal to 7
                    LDR R0, =CO_PMU_STATE_REG
CO_PMU_STATE_CHECK
                    LDR R1, [R0]
                    LSR R1, R1, #27
                    CMP R1, #7
                    BNE CO_PMU_STATE_CHECK

                    ; Open all RAM clock {
                    LDR R0, =CO_RAM_GATE_REG
                    MOV R1, #0
                    STR R1, [R0]
                    ; do {
                    ;   HS_PSO->REG_UPD = CPM_REG_UPD_STATUS_CLR_MASK;
                    ; }while(HS_PSO->REG_UPD);
                    LDR R0, =CO_CPM_UPD_REG
                    MOVS R1, #8
CO_CPM_UPD_CHECK_1
                    STR R1, [R0]
                    LDR R2, [R0]
                    CMP R2, #0
                    BNE CO_CPM_UPD_CHECK_1
                    ; HS_PSO->REG_UPD = CPM_REG_UPD_CPU_MASK;
                    ; while((HS_PSO->REG_UPD & CPM_REG_UPD_CPU_STATUS_MASK) == 0);
                    MOVS R1, #1
                    STR R1, [R0]
CO_CPM_UPD_CHECK_2
                    LDR R2, [R0]
                    LSLS R2, R2, #27
                    BPL CO_CPM_UPD_CHECK_2
                    ; }

                    ; Disable IRQ
                    MOV R0, #1
                    MSR PRIMASK, R0

                    ; Restore SP pointer
                    LDR R0, =CO_CPU_SP_SAVE
                    LDR R1, [R0]
                    MSR MSP, R1

                    ; Reset CPU status
                    LDR R1, =CO_CPU_STATUS_SAVE
                    MOV R0, #0
                    STR R0, [R1]

                    ; Pop register
                    POP {R0-R12, PC}

                    ENDP

                END
