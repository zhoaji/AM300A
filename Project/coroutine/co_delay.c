/**
 * @file co_delay.c
 * @brief 
 * @date Fri, Aug 31, 2018 11:16:42 AM
 * @author liqiang
 *
 * @addtogroup 
 * @ingroup 
 * @details 
 *
 * @{
 */

/*********************************************************************
 * INCLUDES
 */
#include "co.h"
#include "peripheral.h"

/*********************************************************************
 * MACROS
 */

#define CONFIG_DELAY_BY_CPU_CYCCNT

#ifdef CONFIG_DELAY_BY_CPU_CYCCNT

// WDT CYCCNT will increase 70uA current in while(1)
#define CO_ENABLE_CPU_CYCCNT() \
    do { \
        uint32_t __trcen_save; \
        CO_DISABLE_IRQ(); \
        __trcen_save = CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk; \
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; \
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk; \
        CO_RESTORE_IRQ()

#define CO_RESTORE_CPU_CYCCNT() \
        CO_DISABLE_IRQ(); \
        if (__trcen_save == 0) { \
            DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk; \
            CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk; \
        } \
        CO_RESTORE_IRQ(); \
    } while(0)

// CPU 128MHz, WDT max calc time: 33.6 second
#define CO_CPU_CYCCNT() DWT->CYCCNT

#else

//#define DELAY_CPU_CLOCK_KNOWN

#define DELAY_FUNC_CPU_CYCLE_NUM_DEFAULT    6

#ifdef CONFIG_HS6621
#define CALIB_DELAY_COUNTER_MIN             9 // 3-4us for co_delay_get_cpu_clock_mhz()
#else
#define CALIB_DELAY_COUNTER_MIN             2 // 1us for co_delay_get_cpu_clock_mhz()
#endif

#define DELAY_CPU_CLOCK_KNOWN

#ifdef CONFIG_CALIBRATE_DELAY
static uint32_t calib_counter_1ms  = CONFIG_CALIBRATED_DELAY_1MS_COUNTER;
static uint32_t calib_counter_10us = CONFIG_CALIBRATED_DELAY_10US_COUNTER;
static uint32_t calib_counter_1us  = CONFIG_CALIBRATED_DELAY_1US_COUNTER;
static uint32_t calib_cpu_mhz      = CONFIG_CALIBRATED_DELAY_CPU_MHZ;
#else
#define calib_counter_1ms  CONFIG_CALIBRATED_DELAY_1MS_COUNTER
#define calib_counter_10us CONFIG_CALIBRATED_DELAY_10US_COUNTER
#define calib_counter_1us  CONFIG_CALIBRATED_DELAY_1US_COUNTER
#define calib_cpu_mhz      CONFIG_CALIBRATED_DELAY_CPU_MHZ;
#endif

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

/**
 * @brief @ref cpm_get_clock(CPM_CPU_CLK)
 *
 * Note: Better same flow for each clock selecting
 *
 * @return cpu MHz
 **/
static uint32_t co_delay_get_cpu_clock_mhz(void)
{
    uint32_t topclk, div;

    CO_DISABLE_IRQ();

#ifdef CONFIG_HS6621
    // save anaif config
    uint32_t anaif_ahb_cfg_save = HS_PSO->ANA_IF_AHB_CFG;

    // HS_DAIF->SYSPLL_CNS0 requset
    HS_PSO->ANA_IF_AHB_CFG = 0;
    HS_PSO_UPD_RDY();

    // check
    if(HS_PMU->MISC_CTRL & PMU_MISC_CLK_64M_SEL_MASK)
    {
        uint32_t selclk = REGR(&HS_DAIF->SYSPLL_CNS1, MASK_POS(DAIF_SEL_CPUCLK));
        if(selclk == 0)
        {
            topclk = 64; // RC32M*2 or XTAL32M*2
        }
        else
        {
            uint32_t vcodiv = selclk==1 ? 1 : 2;
            uint32_t selfreq = REGR(&HS_DAIF->SYSPLL_CNS0, MASK_POS(DAIF_SYSPLL_SEL_FREQ));
            topclk = 16 * (5 + selfreq) / vcodiv; // PLL
        }
    }
    else
    {
        topclk = 32; // RC32M
    }

    // restore
    HS_PSO->ANA_IF_AHB_CFG = anaif_ahb_cfg_save;
    HS_PSO_UPD_RDY();

    // get cpu divider
    div = REGR(&HS_PSO->CPU_CFG, MASK_POS(CPM_CPU_DIV_COEFF));
    div = div ? div : 1;

#else

    if(HS_PMU->MISC_CTRL & PMU_MISC_CLK_64M_SEL_MASK)
    {
        if (HS_PMU->XTAL32M_CNS0 & PMU_XTAL32M_SEL_CPUCLK_MASK)
            topclk = 64; // RC32M*2 or XTAL32M*2
        else
            topclk = 32; // XTAL32M
    }
    else
    {
        topclk = 32; // RC32M
    }

    // get cpu divider
    if (HS_PSO->CPU_CFG & CPM_CPU_DIV_SEL_MASK)
        div = REGR(&HS_PSO->CPU_CFG, MASK_POS(CPM_CPU_DIV_COEFF));
    else
        div = 1;
#endif

    CO_RESTORE_IRQ();

    return topclk / div;
}

#ifndef CONFIG_DELAY_BY_CPU_CYCCNT
/**
 * @brief co_delay_convert_counter()
 *
 * @param[in] n  
 *
 * @return 
 **/
static uint32_t co_delay_convert_counter(uint32_t n)
{
#ifdef CONFIG_HS6621
    if (HS_HCACHE->STATUS)
        n += n / 2;
#endif

#if CALIB_DELAY_COUNTER_MIN > 0
    n = (n>CALIB_DELAY_COUNTER_MIN) ? (n-CALIB_DELAY_COUNTER_MIN) : 0;
#endif

    return n * co_delay_get_cpu_clock_mhz() / calib_cpu_mhz;
}

#ifdef __GNUC__
/**
 * @brief co_delay_counter()
 *
 * @param[in] counter  
 * @param[in] calibration  
 *
 * @return 
 **/
static uint32_t __attribute__((noinline)) co_delay_counter(uint32_t counter)
{
    __ASM volatile ("CO_DELAY_COUNT_LOOP:");
    __ASM volatile ("SUB    R0, #1");
    __ASM volatile ("CMP    R0, #0");
    __ASM volatile ("BNE    CO_DELAY_COUNT_LOOP");
    return counter;
}
#else
static __ASM void __attribute__((noinline)) co_delay_counter(uint32_t counter)
{
CO_DELAY_COUNT_LOOP
    SUB    R0, #1
    CMP    R0, #0
    BNE    CO_DELAY_COUNT_LOOP
    BX     LR
}
#endif

#ifdef CONFIG_CALIBRATE_DELAY

#ifndef DELAY_CPU_CLOCK_KNOWN
/**
 * @brief co_delay_calc_cpu_mhz()
 *
 * @return 
 **/
static uint32_t co_delay_calc_cpu_mhz(void)
{
    uint32_t cpu_hz, cpu_mhz;
    uint32_t delay_cycle;
    rwip_time_t rwtime;

    // Align
    rwtime = rwip_time_get();
    while(rwip_time_get().hs == rwtime.hs);

    // Evaluate CPU clock
    delay_cycle = DWT->CYCCNT;
    rwtime.hs = rwip_time_get().hs + 4*8; // 10ms
    while(rwip_time_get().hs != rwtime.hs);
    delay_cycle = DWT->CYCCNT - delay_cycle;

    // Calc CPU clock
    cpu_hz = 800/8 * delay_cycle;
    cpu_mhz = cpu_hz / 1000000;
    cpu_mhz = (cpu_mhz&1) ? (cpu_mhz+1) : (cpu_mhz);

    log_debug("cpu=%dHz(%dMHz)\n", cpu_hz, cpu_mhz);

    return cpu_mhz;
}
#endif

/**
 * @brief co_calibrate()
 *
 * @return 
 **/
static void co_delay_calibrate(void)
{
    CO_DISABLE_IRQ();

    uint32_t delay_cycle, delay_us, delay_5us, delay_50us, delay_5ms;
    uint32_t delay_func_cpu_cycle_num;

    // Enable DWT
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

#ifndef DELAY_CPU_CLOCK_KNOWN
    calib_cpu_mhz = co_delay_calc_cpu_mhz();
#endif

    // Evaluate delay_func_cpu_cycle_num (100us)
    delay_cycle = DWT->CYCCNT;
    co_delay_counter(100*calib_cpu_mhz/DELAY_FUNC_CPU_CYCLE_NUM_DEFAULT);
    delay_cycle = DWT->CYCCNT - delay_cycle;
    // Calc
    delay_us = delay_cycle/calib_cpu_mhz;
    delay_func_cpu_cycle_num = 0.5 + DELAY_FUNC_CPU_CYCLE_NUM_DEFAULT * delay_us / 100.0;
    log_debug("delay_func_cpu_cycle_num=%d\n", delay_func_cpu_cycle_num);

    // Calc calib counter
    calib_counter_1us  = 0.5 + 1.0    * calib_cpu_mhz / delay_func_cpu_cycle_num;
    calib_counter_10us = 0.5 + 10.0   * calib_cpu_mhz / delay_func_cpu_cycle_num;
    calib_counter_1ms  = 0.5 + 1000.0 * calib_cpu_mhz / delay_func_cpu_cycle_num;
    log_debug("Calculate calib counter: %d/1ms %d/10us %d/1us\n", calib_counter_1ms, calib_counter_10us, calib_counter_1us);

#if 0 // FIXME
    // Correct calib_counter_1us
    delay_cycle = DWT->CYCCNT;
    co_delay_counter(calib_counter_1us * 5);
    delay_cycle = DWT->CYCCNT - delay_cycle;
    calib_counter_1us = 1 + calib_counter_1us * 5.0 * calib_cpu_mhz / delay_cycle;
    // Correct calib_counter_10us
    delay_cycle = DWT->CYCCNT;
    co_delay_counter(calib_counter_10us * 5);
    delay_cycle = DWT->CYCCNT - delay_cycle;
    calib_counter_10us = 1 + calib_counter_10us * 50.0 * calib_cpu_mhz / delay_cycle;
    // Correct calib_counter_1ms
    delay_cycle = DWT->CYCCNT;
    co_delay_counter(calib_counter_1ms * 5);
    delay_cycle = DWT->CYCCNT - delay_cycle;
    calib_counter_1ms = 1 + calib_counter_1ms * 5000.0 * calib_cpu_mhz / delay_cycle;
    log_debug("Correct   calib counter: %d/1ms %d/10us %d/1us\n", calib_counter_1ms, calib_counter_10us, calib_counter_1us);
#endif

    // Verify
    delay_cycle = DWT->CYCCNT;
    co_delay_counter(calib_counter_1us * 5);
    delay_5us = (DWT->CYCCNT - delay_cycle) / calib_cpu_mhz;
    // Verify
    delay_cycle = DWT->CYCCNT;
    co_delay_counter(calib_counter_10us * 5);
    delay_50us = (DWT->CYCCNT - delay_cycle) / calib_cpu_mhz;
    // Verify
    delay_cycle = DWT->CYCCNT;
    co_delay_counter(calib_counter_1ms * 5);
    delay_5ms = (DWT->CYCCNT - delay_cycle) / calib_cpu_mhz;
    log_debug("Verify delay: 5us=?%dus 50us=?%dus 5ms=?%dus\n", delay_5us, delay_50us, delay_5ms);

    CO_RESTORE_IRQ();
}

#endif

#endif

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief co_delay_init()
 *
 * @return 
 **/
void co_delay_init(void)
{
#ifndef CONFIG_DELAY_BY_CPU_CYCCNT
#ifdef CONFIG_CALIBRATE_DELAY
    co_delay_calibrate();
#endif
#endif
}

/**
 * @brief co_delay_ms()
 *
 * @param[in] ms  
 *
 * @return 
 **/
void co_delay_ms(uint32_t ms)
{
#ifdef CONFIG_DELAY_BY_CPU_CYCCNT
    co_delay_us(ms * 1000);
#else
    uint32_t counter = co_delay_convert_counter(calib_counter_1ms * ms);

    if (counter)
        co_delay_counter(counter);
#endif
}

/**
 * @brief co_delay_us()
 *
 * @param[in] us  
 *
 * @return 
 **/
void co_delay_us(uint32_t us)
{
#ifdef CONFIG_DELAY_BY_CPU_CYCCNT
    if (us == 0)
        return;

    CO_ENABLE_CPU_CYCCNT();

    // Must be read cyccnt firstly
    uint32_t cyccnt = CO_CPU_CYCCNT();
    uint32_t cyccnt_total = co_delay_get_cpu_clock_mhz() * us;

    // Waiting
    while(CO_CPU_CYCCNT() - cyccnt < cyccnt_total);

    CO_RESTORE_CPU_CYCCNT();
#else
    uint32_t counter = co_delay_convert_counter(calib_counter_1us * us);

    if(counter)
        co_delay_counter(counter);
#endif
}

/**
 * @brief co_delay_10us()
 *
 * @param[in] us10  
 *
 * @return 
 **/
void co_delay_10us(uint32_t us10)
{
#ifdef CONFIG_DELAY_BY_CPU_CYCCNT
    co_delay_us(us10 * 10);
#else
    uint32_t counter = co_delay_convert_counter(calib_counter_10us * us10);

    if(counter)
        co_delay_counter(counter);
#endif
}

/** @} */

