/*********************************************************************
 * @file co_port_hs6620.h
 * @brief 
 * @version 1.0
 * @date Mon 17 Nov 2014 01:45:02 PM CST
 * @author liqiang
 *
 * @note 
 */

#ifndef __CO_PORT_HS6620_H__
#define __CO_PORT_HS6620_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "features.h"
#include "hs66xx.h"
#include "rwip.h"

/*********************************************************************
 * MACROS
 */

#define CO_CPU_CLOCK_MHZ             32

/// IRQ priority
#define IRQ_PRIORITY_HIGHEST         1
#define IRQ_PRIORITY_HIGH            3
#define IRQ_PRIORITY_NORMAL          5
#define IRQ_PRIORITY_LOW             6
#define IRQ_PRIORITY_LOWEST          7

/*********************************************************************
 * TYPEDEFS
 */


/*********************************************************************
 * INLINE FUNCTION
 */

#ifdef CONFIG_USE_RWIP_CO_TIMER
// Native Bluetooth Clock is 28bit and unit 312.5us
#define CO_MAX_SYS_TIME                 RWIP_MAX_CLOCK_TIME
#define CO_MAX_TIMER_DELAY              (RWIP_MAX_CLOCK_TIME >> 1)
// Invalid system time
#define CO_INVALID_SYS_TIME             RWIP_INVALID_TARGET_TIME
// HS66xx bt timer resolution is 2.5ms, RTC is 31.25us
#define CO_SYS_TIMER_RESOLUTION         1
// unit transmite 1ms to 312.5us
#define CO_TIME_MS2SYS(time)            ((time) * 16 / 5)
#define CO_TIME_SYS2MS(time)            ((time) * 5 / 16)
#else
// 32bit, 30.52us (1/32768)s
#define CO_MAX_SYS_TIME                 0xFFFFFFFF
#define CO_MAX_TIMER_DELAY              (CO_MAX_SYS_TIME/2 - 1)
// Invalid system time
#define CO_INVALID_SYS_TIME             0xFFFFFFFF
#define CO_SYS_TIMER_RESOLUTION         2
// unit transmite 1ms to 30.52us
#define CO_TIME_MS2SYS(time)            (((time) << 12) / 125)
#define CO_TIME_SYS2MS(time)            ((uint32_t)((((uint64_t)(time)) * 125) >> 12))
#endif

// irq latency and irq process time
#define CO_DISABLE_IRQ_BEGIN_CB()       co_disable_irq_begin_cb()
#define CO_DISABLE_IRQ_END_CB()         co_disable_irq_end_cb()
#define CO_IRQ_PROCESS_TIME_BEGIN()     do{uint32_t __irq_process_begin_cycle = DWT->CYCCNT
#define CO_IRQ_PROCESS_TIME_END()       co_set_irq_process_time(DWT->CYCCNT-__irq_process_begin_cycle);}while(0)

// Disable all irq
#define CO_DISABLE_IRQ()                do{uint32_t __irq_save = __get_PRIMASK(); __set_PRIMASK(1);
#define CO_RESTORE_IRQ()                __set_PRIMASK(__irq_save);} while(0)
#define CO_ENABLE_IRQ()                 __set_PRIMASK(0)
#define CO_DISABLE_IRQ_EX(irq_value)    do{irq_value = __get_PRIMASK(); __set_PRIMASK(1);}while(0)
#define CO_RESTORE_IRQ_EX(irq_value)    do{__set_PRIMASK(irq_value);}while(0)
// Disable irq except highest
#define CO_DISABLE_IRQ_EXCEPT_HIGHEST() do{__irq_save = __get_BASEPRI(); __set_BASEPRI_MAX(IRQ_PRIORITY_HIGH<<(8U-__NVIC_PRIO_BITS));
#define CO_RESTORE_IRQ_EXCEPT_HIGHEST() __set_BASEPRI(__irq_save);} while(0)
#define CO_DISABLE_IRQ_EX_EXCEPT_HIGHEST(irq_value) do{irq_value = __get_BASEPRI(); __set_BASEPRI_MAX(IRQ_PRIORITY_HIGH<<(8U-__NVIC_PRIO_BITS));}while(0)
#define CO_RESTORE_IRQ_EX_EXCEPT_HIGHEST(irq_value) do{__set_BASEPRI(irq_value);}while(0)

#define CO_CPU_OVER_CLOCK()            do {uint32_t __cpu_cfg_save = HS_PSO->CPU_CFG; HS_PSO->CPU_CFG = 0
#define CO_CPU_RESTORE_CLOCK()         HS_PSO->CPU_CFG = __cpu_cfg_save;} while(0)

// Calculate BEGIN-END delay time
// CO_DELAY_CALC_END() return is us or just print __delay_us
#define CO_DELAY_CALC_INIT() \
    uint32_t __delay_cycle, __delay_us; \
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; \
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk
#define CO_DELAY_CALC_BEGIN() \
     __delay_cycle = DWT->CYCCNT;
#define CO_DELAY_CALC_END() \
    (__delay_cycle = DWT->CYCCNT - __delay_cycle, __delay_us = __delay_cycle / CO_CPU_CLOCK_MHZ)
#define CO_DELAY_CALC_CYCLE_END() \
    (__delay_cycle = DWT->CYCCNT - __delay_cycle, __delay_cycle)

// Is in irq ?
#define CO_IS_URGENCY() (__get_IPSR() != 0)

#if defined  (__GNUC__)
#define CO_ALIGN(x) __attribute__((aligned(x)))
#else
#define CO_ALIGN(x) __align(x)
#endif

// section for RAM text
#define CO_SECTION_RAMTEXT        __attribute__((section(".ramtext")))
#define CO_SECTION_RAMTEXT_ONCE   __attribute__((section(".ramtext.once")))

/*********************************************************************
 * Extra INCLUDES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */

/**
 * @brief co_port_init()
 *
 * @return 
 **/
void co_port_init(void);

/**
 * @brief co_sys_time()
 *
 * @return the current system tick, its uint is 312.5us in HS6600
 **/
uint32_t co_sys_time(void);

/**
 * @brief co_stack_info()
 *
 * @param[in] base  
 * @param[in] size  
 *
 * @return 
 **/
void co_stack_info(uint8_t **base, uint32_t *size);

/**
 * @brief co_heap_info()
 *
 * @param[in] base  
 * @param[in] size  
 *
 * @return 
 **/
void co_heap_info(uint8_t **base, uint32_t *size);

/**
 * @brief co_generate_random_32bit()
 *
 * @return 
 **/
uint32_t co_generate_random_32bit(void);

/**
 * @brief co_generate_random()
 *
 * @param[in] rand  
 * @param[in] bytes  
 *
 * @return 
 **/
void co_generate_random(void *rand, unsigned bytes);

/**
 * @brief co_sleep()
 *
 * @param[in] status  
 *
 * @return 
 **/
void co_sleep(co_power_status_t status);

/**
 * @brief co_tick()
 *
 * Note: Only for debug
 *
 * @return 
 **/
STATIC_INLINE uint32_t co_tick(void)
{
#ifdef CONFIG_SLEEP_SUPPORT
    return ((uint64_t)HS_RTC->SR * 1000000UL) / 32768UL;
#else
    return DWT->CYCCNT / CO_CPU_CLOCK_MHZ;
#endif
}

/**
 * @brief  co cpu check
 **/
void co_cpu_check(void);

/**
 * @brief  co cpu suspend
 **/
void co_cpu_suspend(void);

/**
 * @brief  co ram reset handler
 **/
void co_ram_reset_handler (void);

#ifdef __cplusplus
}
#endif

#endif


