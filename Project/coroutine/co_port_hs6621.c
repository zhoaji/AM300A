/*********************************************************************
 * @file co_port_hs6620.c
 * @brief 
 * @version 1.0
 * @date Mon 17 Nov 2014 01:40:33 PM CST
 * @author liqiang
 *
 * @note 
 */

/*********************************************************************
 * INCLUDES
 */

#include "co.h"
#include "peripheral.h"

#ifndef CONFIG_WITHOUT_RW
#include "rwip.h"
#include "rwip_int.h"        // RW internal definitions
#include "ll.h"
#include "dbg_swdiag.h"
#endif

#ifndef CONFIG_WITHOUT_PATCH
#include "patch_manage.h"
#endif

/*********************************************************************
 * MACROS
 */
#ifdef CONFIG_SLEEP_SUPPORT
#define __SLEEP() co_cpu_suspend()
#endif

#ifdef CONFIG_WITHOUT_RW
#define DBG_SWDIAG_EX(bank , field)
#endif

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */


/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * LOCAL FUNCTIONS
 */

#ifdef CONFIG_SLEEP_SUPPORT

#ifdef __GNUC__

static volatile uint32_t co_sp_save = 0;

#define CO_CPU_SP_SAVE      (co_sp_save)
#define CO_CPU_STATUS_SAVE  (HS_PMU->CPU_STATUS)
#define CO_RAM_GATE_REG     (HS_PSO->RAM_CFG)
#define CO_CPM_UPD_REG      (HS_PSO->REG_UPD)

__attribute__((noinline)) void co_cpu_check(void)
{
    // Check cpu status
    __ASM volatile ("LDR R0, [%0]" : : "r" (&CO_CPU_STATUS_SAVE));
    __ASM volatile ("CMP R0, #2");
    __ASM volatile ("BEQ CO_CPU_SUSPEND_RESUME");

    // Normal system reset
    __ASM volatile ("BX LR");
}

__attribute__((noinline)) void co_cpu_suspend(void)
{
    // Push register to stack
    __ASM volatile ("PUSH {R0-R12, LR}");

    // Save SP pointer
    __ASM volatile ("LDR R1, =%0" : : "" (&CO_CPU_SP_SAVE));
    __ASM volatile ("MRS R0, MSP");
    __ASM volatile ("STR R0, [R1]");

    // Save CPU status to PMU
    __ASM volatile ("LDR R1, =%0" : : "" (&CO_CPU_STATUS_SAVE));
    __ASM volatile ("MOV R0, #2");
    __ASM volatile ("STR R0, [R1]");

    // Good Night
    __WFI();

    // Resume Place
    __ASM volatile ("CO_CPU_SUSPEND_RESUME:");

    // Wait system ready, Check PMU_STATE (PMU_BASIC[31:27]) equal to 7
    __ASM volatile ("LDR R0, =0x400e0000");
    __ASM volatile ("CO_PMU_STATE_CHECK:");
    __ASM volatile ("LDR R1, [R0]");
    __ASM volatile ("LSR R1, R1, #27");
    __ASM volatile ("CMP R1, #7");
    __ASM volatile ("BNE CO_PMU_STATE_CHECK");

    // Open all RAM clock
    __ASM volatile ("LDR R0, =%0" : : "" (&CO_RAM_GATE_REG));
    __ASM volatile ("MOV R1, #0");
    __ASM volatile ("STR R1, [R0]");
    // do {
    //   HS_PSO->REG_UPD = CPM_REG_UPD_STATUS_CLR_MASK;
    // }while(HS_PSO->REG_UPD);
    __ASM volatile ("LDR R0, =%0" : : "" (&CO_CPM_UPD_REG));
    __ASM volatile ("MOV R1, #8");
    __ASM volatile ("CO_CPM_UPD_CHECK_1:");
    __ASM volatile ("STR R1, [R0]");
    __ASM volatile ("LDR R2, [R0]");
    __ASM volatile ("CMP R2, #0");
    __ASM volatile ("BNE CO_CPM_UPD_CHECK_1");
    // HS_PSO->REG_UPD = CPM_REG_UPD_CPU_MASK;
    // while((HS_PSO->REG_UPD & CPM_REG_UPD_CPU_STATUS_MASK) == 0);
    __ASM volatile ("MOVS R1, #1");
    __ASM volatile ("STR R1, [R0]");
    __ASM volatile ("CO_CPM_UPD_CHECK_2:");
    __ASM volatile ("LDR R2, [R0]");
    __ASM volatile ("LSLS R2, R2, #27");
    __ASM volatile ("BPL CO_CPM_UPD_CHECK_2");

    // Disable IRQ
    __ASM volatile ("MOV R0, #1");
    __ASM volatile ("MSR PRIMASK, R0");

    // Restore SP pointer
    __ASM volatile ("LDR R0, =%0" : : "" (&CO_CPU_SP_SAVE));
    __ASM volatile ("LDR R1, [R0]");
    __ASM volatile ("MSR MSP, R1");

    // Reset CPU status
    __ASM volatile ("LDR R1, =%0" : : "" (&CO_CPU_STATUS_SAVE));
    __ASM volatile ("MOV R0, #0");
    __ASM volatile ("STR R0, [R1]");

    // Pop register
    __ASM volatile ("POP {R0-R12, PC}");
}
#endif

/**
 * @brief  co light sleep handle
 *
 * @param[in] pmustate  pmu state
 **/
__STATIC_INLINE void co_light_sleep_handle(pmu_lowpower_state_t pmustate)
{
    // Store user state
    co_power_sleep_handler(POWER_SLEEP_ENTRY, POWER_SLEEP);

    // peripheral state save
    peripheral_lowpower_enter(POWER_SLEEP);

    // Pin state save
    pmu_pin_state_store();

    // PMU enter sleep, power down
    pmu_lowpower_enter(pmustate);

    // Good Night! Except BASEBAND/CPU, other peripheral power down automatically
    __SLEEP();

    // PMU leave sleep, power on
    pmu_lowpower_leave(pmustate);

#ifndef CONFIG_WITHOUT_PATCH
    // Enable patch
    patch_restore();
#endif

    // peripheral state restore
    peripheral_lowpower_leave(POWER_SLEEP);

    // Restore user state
    co_power_sleep_handler(POWER_SLEEP_LEAVE_TOP_HALF, POWER_SLEEP);

#ifndef CONFIG_WITHOUT_RW
    // Enable BB irq
    bb_lowpower_leave(BB_SLEEP_SYS);
#endif

    // Pin state restore, must after notify user
    pmu_pin_state_restore();

    // User may do something
    co_power_sleep_handler(POWER_SLEEP_LEAVE_BOTTOM_HALF, POWER_SLEEP);
}

/**
 * @brief co_wakeup_restore()
 *
 * Restore PINMUX, HCI or DEBUG UART
 *
 * @return 
 **/
__STATIC_INLINE void co_deep_sleep_handle(void)
{
#ifndef CONFIG_WITHOUT_RW
    // Store baseband and rf state
    bb_lowpower_enter(BB_DEEP_SLEEP);
#endif

    // Store user state
    co_power_sleep_handler(POWER_SLEEP_ENTRY, POWER_DEEP_SLEEP);

    // peripheral state save
    peripheral_lowpower_enter(POWER_DEEP_SLEEP);

    // Pin state save
    pmu_pin_state_store();

    // PMU enter sleep, power down
    pmu_lowpower_enter(PMU_DEEP_SLEEP);

    // Good Night! Except BASEBAND/CPU, other peripheral power down automatically
    __SLEEP();

    // PMU leave sleep, power on
    pmu_lowpower_leave(PMU_DEEP_SLEEP);

#ifndef CONFIG_WITHOUT_PATCH
    // Enable patch
    patch_restore();
#endif

    // peripheral state save
    peripheral_lowpower_leave(POWER_DEEP_SLEEP);

    // Restore user state
    co_power_sleep_handler(POWER_SLEEP_LEAVE_TOP_HALF, POWER_DEEP_SLEEP);

#ifndef CONFIG_WITHOUT_RW
    // Restore baseband and rf state
    bb_lowpower_leave(BB_DEEP_SLEEP);
#endif

    // Pin state restore, must after notify user
    pmu_pin_state_restore();

    // User may do something
    co_power_sleep_handler(POWER_SLEEP_LEAVE_BOTTOM_HALF, POWER_DEEP_SLEEP);
}

/**
 * @brief co_ultra_sleep_handle()
 *
 * @return 
 **/
__STATIC_INLINE void co_ultra_sleep_handle(void)
{
#ifndef CONFIG_WITHOUT_RW
    // Store baseband and rf state
    bb_lowpower_enter(BB_ULTRA_DEEP_SLEEP);
#endif

    // Store user state
    co_power_sleep_handler(POWER_SLEEP_ENTRY, POWER_DEEP_SLEEP);

    // peripheral state save
    peripheral_lowpower_enter(POWER_DEEP_SLEEP);

    // Pin state save
    pmu_pin_state_store();

    // PMU enter sleep, power down
    pmu_lowpower_enter(PMU_DEEP_SLEEP);

    // Into reboot sleep mode
    pmu_force_into_reboot_sleep_mode(0 /*ram_retention*/, co_power_hib_sleep_mode_is_enabled() /*hib*/);
}

/**
 * @brief co_shutdown_handle()
 *
 * @return 
 **/
__STATIC_INLINE void co_shutdown_handle(void)
{
    // Shut down
    pmu_lowpower_enter(PMU_SHUTDOWN);
}
#endif

/*********************************************************************
 * LOCAL VAlITLE
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief __co_port_init()
 *
 * @return 
 **/
void co_port_init(void)
{

}

/**
 * @brief co_sys_time()
 *
 * @return the current system tick, its uint is 312.5us:
 **/
uint32_t co_sys_time(void)
{
#ifdef CONFIG_USE_RWIP_CO_TIMER
    uint32_t res = 0;
    GLOBAL_INT_DISABLE();
    res = rwip_time_get().hs;
    GLOBAL_INT_RESTORE();
    return (res);
#else
    return pmu_timer_time();
#endif
}

/**
 * @brief co_stack_info()
 *
 * @param[in] base  
 * @param[in] size  
 *
 * @return 
 **/
void co_stack_info(uint8_t **base, uint32_t *size)
{
#if defined(__GNUC__) // GCC
    extern unsigned _stack, _estack;
    uint8_t *stack_base  = (uint8_t *)&_stack;
    uint8_t *stack_limit = (uint8_t *)&_estack;
#elif defined(__CC_ARM) // Keil
    extern uint32_t Stack_Mem, __initial_sp;
    uint8_t *stack_base  = (uint8_t *)&Stack_Mem;
    uint8_t *stack_limit = (uint8_t *)&__initial_sp;
#elif defined(__ICCARM__) // IAR
#errer ""
#endif

    *base = stack_base;
    *size = stack_limit - stack_base;
}

/**
 * @brief co_heap_info()
 *
 * @param[in] base  
 * @param[in] size  
 *
 * @return 
 **/
void co_heap_info(uint8_t **base, uint32_t *size)
{
#if defined ( __ICCARM__ ) // IAR
    #pragma segment="HEAP"
    void *heap_base  = __sfb("HEAP");
    void *heap_limit = __sfe("HEAP");
    uint32_t heap_size = (uint32_t)heap_limit - (uint32_t)heap_base;
#else
    extern uint32_t __heap_base, __heap_limit;
    void *heap_base = (void *)&__heap_base;
    uint32_t heap_size = (uint32_t)&__heap_limit - (uint32_t)&__heap_base;
#endif

    *base = (uint8_t *)heap_base;
    *size = heap_size;
}

/**
 * @brief co_generate_random_32bit()
 *
 * @return 
 **/
uint32_t co_generate_random_32bit(void)
{
    return (uint32_t)rand();
}

/**
 * @brief co_generate_random()
 *
 * @param[in] rand  
 * @param[in] bytes  
 *
 * @return 
 **/
void co_generate_random(void *rand, unsigned bytes)
{
    uint32_t tmp;
    uint8_t *prand = rand;
    unsigned align_bytes = bytes & ~0x3;
    unsigned left_bytes  = bytes & 0x3;
    int i;

    for(i=0; i<align_bytes; i+=4)
    {
        tmp = co_generate_random_32bit();
        memcpy(prand, &tmp, 4);
        prand += 4;
    }

    if(left_bytes)
    {
        tmp = co_generate_random_32bit();
        memcpy(prand, &tmp, left_bytes);
    }
}

#if defined(CONFIG_XIP_FLASH_ALL) && defined(__GNUC__)
/**
 * @brief  reset handler
 **/
void __attribute__ ((naked,noinline)) co_ram_reset_handler (void)
{
#ifdef CONFIG_SLEEP_SUPPORT
    co_cpu_check();
#endif
    while(1);
}
#endif

/**
 * @brief co_sleep()
 *
 * @param[in] status  
 *
 * @return 
 **/
void co_sleep(co_power_status_t status)
{
    /* IRQ has been disabled */

    switch(status)
    {
#ifdef CONFIG_SLEEP_SUPPORT
        case POWER_SLEEP:
            SCB->SCR |= SCB_SCR_SLEEPDEEP;
            DBG_SWDIAG_EX(DBG_SWDIAG_POWER_SLEEP, DBG_SWDIAG_EVT_BEGIN);
            if(co_power_fast_sleep_mode_is_enabled())
                co_light_sleep_handle(PMU_SLEEP_HXTAL_OFF);
            else
                co_light_sleep_handle(PMU_SLEEP);
            DBG_SWDIAG_EX(DBG_SWDIAG_POWER_SLEEP, DBG_SWDIAG_EVT_END);
            break;

        case POWER_DEEP_SLEEP:
            SCB->SCR |= SCB_SCR_SLEEPDEEP;
            DBG_SWDIAG_EX(DBG_SWDIAG_POWER_DEEP_SLEEP, DBG_SWDIAG_EVT_BEGIN);
            if(co_power_ultra_sleep_mode_is_enabled())
                co_ultra_sleep_handle();
            else
                co_deep_sleep_handle();
            DBG_SWDIAG_EX(DBG_SWDIAG_POWER_DEEP_SLEEP, DBG_SWDIAG_EVT_END);
            break;

        case POWER_SHUTDOWN:
            co_shutdown_handle();
            break;
#else
        case POWER_SLEEP:
        case POWER_DEEP_SLEEP:
        case POWER_SHUTDOWN:
#endif

        case POWER_IDLE:
            SCB->SCR &= ~SCB_SCR_SLEEPDEEP;
            DBG_SWDIAG_EX(DBG_SWDIAG_POWER_CPU_GATE, DBG_SWDIAG_EVT_BEGIN);
            __WFI();
            DBG_SWDIAG_EX(DBG_SWDIAG_POWER_CPU_GATE, DBG_SWDIAG_EVT_END);
            break;

        case POWER_ACTIVE:
        default:
            break;
    }
}

