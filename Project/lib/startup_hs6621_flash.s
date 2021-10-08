;******************************************************************************
;
;  Copyright (C), 2016, HunterSun Co., Ltd.
;
;******************************************************************************
; * @file     startup_HS6620.s
; * @brief    CMSIS Cortex-M3 Core Device Startup File for
; *           Device HS6620
; * @version  V1.00
; * @date     27. May 2016
; *
; * @note
; * Copyright (C) 2016 ARM Limited. All rights reserved.
; *
; * @par
; * ARM Limited (ARM) is supplying this software for use with Cortex-M
; * processor based microcontrollers.  This file can be freely distributed
; * within development tools that are supporting such ARM based processors.
; *
; * @par
; * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
; * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
; * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
; * ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
; * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
; *
; ******************************************************************************/

ROM_ID          EQU     0x6621C200
RUN2SF_FLAG     EQU     0x46533252

;/*
;//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------
;*/

; <h> Stack Configuration
;   <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Stack_Size      EQU     0x00001000

                AREA    STACK, NOINIT, READWRITE, ALIGN=3
Stack_Mem       SPACE   Stack_Size
__initial_sp


; <h> Heap Configuration
;   <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

Heap_Size       EQU     0x00000000

                AREA    HEAP, NOINIT, READWRITE, ALIGN=3
__heap_base
Heap_Mem        SPACE   Heap_Size
__heap_limit


                PRESERVE8
                THUMB


; Flash Vector Table Mapped to Address 0 at Reset (Only work for power on)

                AREA    RESET_FLASH, DATA, READONLY

__VectorsFLASH  DCD     __initial_sp              ; Top of Stack
                DCD     Reset_Handler             ; Reset Handler
                DCD     0xFFFFFFFF                ; NMI Handler
                DCD     HardFault_Handler         ; Hard Fault Handler
                DCD     0xFFFFFFFF                ; MPU Fault Handler
                DCD     0xFFFFFFFF                ; Bus Fault Handler
                DCD     0xFFFFFFFF                ; Usage Fault Handler
                DCD     SoftFault_Handler         ; Huntersun assign
                DCD     0xFFFFFFFF                ; Reserved
                DCD     __VectorsFLASH            ; Base address in flash
                DCD     RUN2SF_FLAG               ; Run to flash flag
                DCD     0xFFFFFFFF                ; SVCall Handler
                DCD     0xFFFFFFFF                ; Debug Monitor Handler
                DCD     ROM_ID                    ; @0x34 HS6620's rom_id
                DCD     0xFFFFFFFF                ; PendSV Handler
                DCD     0xFFFFFFFF                ; SysTick Handler


; RAM Vector Table Mapped to Address 0 at Reset (Real IRQ Vector Table)

                AREA    RESET_RAM, DATA, READONLY
                IMPORT  co_ram_reset_handler
                EXPORT  __Vectors
                EXPORT  __Vectors_End
                EXPORT  __Vectors_Size

__Vectors       DCD     __initial_sp              ; Top of Stack
                DCD     co_ram_reset_handler      ; Reset Handler
                DCD     NMI_Handler               ; NMI Handler
                DCD     HardFault_Handler         ; Hard Fault Handler
                DCD     MemManage_Handler         ; MPU Fault Handler
                DCD     BusFault_Handler          ; Bus Fault Handler
                DCD     UsageFault_Handler        ; Usage Fault Handler
                DCD     SoftFault_Handler         ; Huntersun assign
                DCD     0xFFFFFFFF                ; Reserved
                DCD     0xFFFFFFFF                ; Reserved
                DCD     0xFFFFFFFF                ; Reserved
                DCD     SVC_Handler               ; SVCall Handler
                DCD     DebugMon_Handler          ; Debug Monitor Handler
                DCD     ROM_ID                    ; @0x34 HS6620's rom_id
                DCD     PendSV_Handler            ; PendSV Handler
                DCD     SysTick_Handler           ; SysTick Handler

                ; External Interrupts
                DCD     BT_IRQHandler             ; 0
                DCD     BT_OSCEN_IRQHandler       ; 1
                DCD     DMA_IRQHandler            ; 2
                DCD     GPIO_IRQHandler           ; 3
                DCD     TIM_IRQHandler            ; 4
                DCD     KPP_DEPRESS_IRQHandler    ; 5
                DCD     KPP_RELEASE_IRQHandler    ; 6
                DCD     PMU_TIMER_IRQHandler      ; 7
                DCD     UART0_IRQHandler          ; 8
                DCD     UART1_IRQHandler          ; 9
                DCD     I2C_IRQHandler            ; 10
                DCD     PIN_WAKEUP_IRQHandler     ; 11
                DCD     ADC_IRQHandler            ; 12
                DCD     SPI0_IRQHandler           ; 13
                DCD     SPI1_IRQHandler           ; 14
                DCD     GP_COMP_IRQHandler        ; 15
                DCD     DMA0_IRQHandler           ; 16
                DCD     DMA1_IRQHandler           ; 17
                DCD     DMA2_IRQHandler           ; 18
                DCD     DMA3_IRQHandler           ; 19
                DCD     RTC_AF_IRQHandler         ; 20
                DCD     RTC_1HZ_IRQHandler        ; 21
                DCD     RTC_BLE_IRQHandler        ; 22
                DCD     VTRACK_IRQHandler         ; 23
                DCD     CRY32M_DIG_IRQHandler     ; 24
                DCD     CRY32M_PLL_IRQHandler     ; 25
                DCD     GPIO0_IRQHandler          ; 26
                DCD     SF1_IRQHandler            ; 27
                DCD     TIM0_IRQHandler           ; 28
                DCD     TIM1_IRQHandler           ; 29
                DCD     TIM2_IRQHandler           ; 30
                DCD     CPM_IRQHandler            ; 31
                DCD     CRY32K_IRQHandler         ; 32
                DCD     SF_IRQHandler             ; 33
                DCD     QDEC_IRQHandler           ; 34
                DCD     POWER_DOWN_IRQHandler     ; 35
                DCD     I2S_TX_IRQHandler         ; 36
                DCD     I2S_RX_IRQHandler         ; 37
                DCD     MAC6200_RF_IRQHandler     ; 38
                DCD     MAC6200_SPI_IRQHandler    ; 39
                DCD     SOFT0_IRQHandler          ; 40
                DCD     SOFT1_IRQHandler          ; 41
                DCD     SOFT2_IRQHandler          ; 42
                DCD     SOFT3_IRQHandler          ; 43
                DCD     SOFT4_IRQHandler          ; 44
                DCD     SOFT5_IRQHandler          ; 45
                DCD     SOFT6_IRQHandler          ; 46
                DCD     SOFT7_IRQHandler          ; 47
                DCD     AUDIO_IRQHandler          ; 48
                DCD     I2C1_IRQHandler           ; 49
                DCD     I2C2_IRQHandler           ; 50
                DCD     SF2_IRQHandler            ; 51
                DCD     0                         ; 52
                DCD     0                         ; 53
                DCD     0                         ; 54
                DCD     0                         ; 55
                DCD     0                         ; 56
                DCD     0                         ; 57
                DCD     0                         ; 58
                DCD     0                         ; 59
                DCD     0                         ; 60
                DCD     0                         ; 61
                DCD     0                         ; 62
                DCD     0                         ; 63

__Vectors_End

__Vectors_Size  EQU     __Vectors_End - __Vectors


                ; Reset_Handler must be place at 1st RAM block
                AREA    |.text|, CODE, READONLY


; Reset Handler
Reset_Handler   PROC
                EXPORT  Reset_Handler             [WEAK]
                IMPORT  __main
                LDR     R0, =__main
                BX      R0
                ENDP

; Sub main (from __main)
|$Sub$$main|    PROC
                EXPORT  |$Sub$$main|
                IMPORT  system_init_run2sf
                IMPORT  |$Super$$main|
                ; Do system init
                LDR     R0, =system_init_run2sf
                BLX     R0
                ; Call main
                LDR     R0, =|$Super$$main|
                BX      R0
                ENDP


; Dummy Exception Handlers (infinite loops which can be modified)

NMI_Handler     PROC
                EXPORT  NMI_Handler               [WEAK]
                B       .
                ENDP
HardFault_Handler\
                PROC
                EXPORT  HardFault_Handler         [WEAK]
                B       .
                ENDP
MemManage_Handler\
                PROC
                EXPORT  MemManage_Handler         [WEAK]
                B       .
                ENDP
BusFault_Handler\
                PROC
                EXPORT  BusFault_Handler          [WEAK]
                B       .
                ENDP
UsageFault_Handler\
                PROC
                EXPORT  UsageFault_Handler        [WEAK]
                B       .
                ENDP
SoftFault_Handler \
                PROC
                EXPORT  SoftFault_Handler         [WEAK]
                B       .
                ENDP
SVC_Handler     PROC
                EXPORT  SVC_Handler               [WEAK]
                B       .
                ENDP
DebugMon_Handler\
                PROC
                EXPORT  DebugMon_Handler          [WEAK]
                B       .
                ENDP
PendSV_Handler\
                PROC
                EXPORT  PendSV_Handler            [WEAK]
                B       .
                ENDP
SysTick_Handler\
                PROC
                EXPORT  SysTick_Handler           [WEAK]
                B       .
                ENDP

Default_Handler PROC
                EXPORT  BT_IRQHandler             [WEAK]
                EXPORT  BT_OSCEN_IRQHandler       [WEAK]
                EXPORT  DMA_IRQHandler            [WEAK]
                EXPORT  GPIO_IRQHandler           [WEAK]
                EXPORT  TIM_IRQHandler            [WEAK]
                EXPORT  KPP_DEPRESS_IRQHandler    [WEAK]
                EXPORT  KPP_RELEASE_IRQHandler    [WEAK]
                EXPORT  PMU_TIMER_IRQHandler      [WEAK]
                EXPORT  UART0_IRQHandler          [WEAK]
                EXPORT  UART1_IRQHandler          [WEAK]
                EXPORT  I2C_IRQHandler            [WEAK]
                EXPORT  PIN_WAKEUP_IRQHandler     [WEAK]
                EXPORT  ADC_IRQHandler            [WEAK]
                EXPORT  SPI0_IRQHandler           [WEAK]
                EXPORT  SPI1_IRQHandler           [WEAK]
                EXPORT  GP_COMP_IRQHandler        [WEAK]
                EXPORT  DMA0_IRQHandler           [WEAK]
                EXPORT  DMA1_IRQHandler           [WEAK]
                EXPORT  DMA2_IRQHandler           [WEAK]
                EXPORT  DMA3_IRQHandler           [WEAK]
                EXPORT  RTC_AF_IRQHandler         [WEAK]
                EXPORT  RTC_1HZ_IRQHandler        [WEAK]
                EXPORT  RTC_BLE_IRQHandler        [WEAK]
                EXPORT  VTRACK_IRQHandler         [WEAK]
                EXPORT  CRY32M_DIG_IRQHandler     [WEAK]
                EXPORT  CRY32M_PLL_IRQHandler     [WEAK]
                EXPORT  GPIO0_IRQHandler          [WEAK]
                EXPORT  SF1_IRQHandler            [WEAK]
                EXPORT  TIM0_IRQHandler           [WEAK]
                EXPORT  TIM1_IRQHandler           [WEAK]
                EXPORT  TIM2_IRQHandler           [WEAK]
                EXPORT  CPM_IRQHandler            [WEAK]
                EXPORT  CRY32K_IRQHandler         [WEAK]
                EXPORT  SF_IRQHandler             [WEAK]
                EXPORT  QDEC_IRQHandler           [WEAK]
                EXPORT  POWER_DOWN_IRQHandler     [WEAK]
                EXPORT  I2S_TX_IRQHandler         [WEAK]
                EXPORT  I2S_RX_IRQHandler         [WEAK]
                EXPORT  MAC6200_RF_IRQHandler     [WEAK]
                EXPORT  MAC6200_SPI_IRQHandler    [WEAK]
                EXPORT  SOFT0_IRQHandler          [WEAK]
                EXPORT  SOFT1_IRQHandler          [WEAK]
                EXPORT  SOFT2_IRQHandler          [WEAK]
                EXPORT  SOFT3_IRQHandler          [WEAK]
                EXPORT  SOFT4_IRQHandler          [WEAK]
                EXPORT  SOFT5_IRQHandler          [WEAK]
                EXPORT  SOFT6_IRQHandler          [WEAK]
                EXPORT  SOFT7_IRQHandler          [WEAK]
                EXPORT  AUDIO_IRQHandler          [WEAK]
                EXPORT  I2C1_IRQHandler           [WEAK]
                EXPORT  I2C2_IRQHandler           [WEAK]
                EXPORT  SF2_IRQHandler            [WEAK]

BT_IRQHandler
BT_OSCEN_IRQHandler
DMA_IRQHandler
GPIO_IRQHandler
TIM_IRQHandler
KPP_DEPRESS_IRQHandler
KPP_RELEASE_IRQHandler
PMU_TIMER_IRQHandler
UART0_IRQHandler
UART1_IRQHandler
I2C_IRQHandler
PIN_WAKEUP_IRQHandler
ADC_IRQHandler
SPI0_IRQHandler
SPI1_IRQHandler
GP_COMP_IRQHandler
DMA0_IRQHandler
DMA1_IRQHandler
DMA2_IRQHandler
DMA3_IRQHandler
RTC_AF_IRQHandler
RTC_1HZ_IRQHandler
RTC_BLE_IRQHandler
VTRACK_IRQHandler
CRY32M_DIG_IRQHandler
CRY32M_PLL_IRQHandler
GPIO0_IRQHandler
SF1_IRQHandler
TIM0_IRQHandler
TIM1_IRQHandler
TIM2_IRQHandler
CPM_IRQHandler
CRY32K_IRQHandler
SF_IRQHandler
QDEC_IRQHandler
POWER_DOWN_IRQHandler
I2S_TX_IRQHandler
I2S_RX_IRQHandler
MAC6200_RF_IRQHandler
MAC6200_SPI_IRQHandler
SOFT0_IRQHandler
SOFT1_IRQHandler
SOFT2_IRQHandler
SOFT3_IRQHandler
SOFT4_IRQHandler
SOFT5_IRQHandler
SOFT6_IRQHandler
SOFT7_IRQHandler
AUDIO_IRQHandler
I2C1_IRQHandler
I2C2_IRQHandler
SF2_IRQHandler
                B       .
                ENDP


                ALIGN


; User Initial Stack & Heap

                IF      :DEF:__MICROLIB

                EXPORT  Stack_Mem
                EXPORT  __initial_sp
                EXPORT  __heap_base
                EXPORT  __heap_limit

                ELSE

                IMPORT  __use_two_region_memory
                EXPORT  __user_initial_stackheap

__user_initial_stackheap PROC
                LDR     R0, =  Heap_Mem
                LDR     R1, = (Stack_Mem + Stack_Size)
                LDR     R2, = (Heap_Mem  + 0)
                LDR     R3, = Stack_Mem
                BX      LR
                ENDP

                ALIGN

                ENDIF


                END

