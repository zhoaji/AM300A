/**
 * @file wdt.c
 * @brief
 * @date Thu 04 May 2017 05:37:02 PM CST
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
#include "peripheral.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
    uint32_t timeout;
}wdt_env_t;


/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */
static wdt_env_t wdt_env = {0};

/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * LOCAL FUNCTIONS
 */

//{BASE_COMPONENT_BEGIN
#ifdef CONFIG_HS6621
/**
 * @brief  wdt write cr
 *
 * @param[in] value  value
 **/
static void wdt_write_CR(uint32_t value)
{
    while(!(HS_RTC->CR & RTC_READY_MASK));
    HS_RTC->WDT_CR = value;
}

/**
 * @brief  wdt write sr
 *
 * @param[in] value  value
 **/
static void wdt_write_SR(uint32_t value)
{
    while(!(HS_RTC->CR & RTC_READY_MASK));
    HS_RTC->WDT_SR = value;
}
#else
/**
 * @brief  wdt write cr
 *
 * @param[in] value  value
 **/
static void wdt_write_CR(uint32_t value)
{
    HS_RTC->WDT_CR = value;
}

/**
 * @brief  wdt write sr
 *
 * @param[in] value  value
 **/
static void wdt_write_SR(uint32_t value)
{
    while(HS_RTC->WDT_CR & (1<<3));
    HS_RTC->WDT_SR = value;
    HS_RTC->WDT_CR |= 1<<3;
}
#endif
//}BASE_COMPONENT_END

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief Get watchdog current counter value
 *
 * @return  Current counter value.
 */
uint32_t wdt_time_left(void)
{
    // Open RTC APB clock
    register_set0(&HS_PSO->APB_CFG, CPM_RTC_APB_GATE_EN_MASK);
    HS_PSO_UPD_RDY();

    // calculate
    return HS_RTC->WDT_SR / cpm_get_clock(CPM_RTC_CLK);
}

/**
 * @brief watch dog keepalive
 *
 * @return None
 **/
void wdt_keepalive(void)
{
    // Open RTC APB clock
    register_set0(&HS_PSO->APB_CFG, CPM_RTC_APB_GATE_EN_MASK);
    HS_PSO_UPD_RDY();

    // keep alive
    wdt_write_SR(wdt_env.timeout);
    wdt_write_CR(HS_RTC->WDT_CR | RTC_WDT_UPDATE_MASK | RTC_WDT_ENABLE_MASK);
}

/**
 * @brief enable watch dog
 *
 * @param[in] timeout  timeout with second, 0 to disable It
 *
 * @note !!! Working stop after sleep !!!
 *
 * @return None
 **/
void wdt_enable(uint32_t timeout)
{
    if(timeout)
    {
        // timeout save
        wdt_env.timeout = timeout * cpm_get_clock(CPM_RTC_CLK);

        // Disable
        register_set1(&HS_PMU->MISC_CTRL, PMU_MISC_RTC_WDT_DIS_EN_MASK);

        // Init RTC
        rtc_init();

        // clear wdt flag (Must do it before set 'MISC_CTRL_1' flag)
        register_set0(&HS_PMU->MISC_CTRL, PMU_MISC_RTC_WDT_FLAG_EN_MASK);

        // Init WDT
        wdt_write_SR(wdt_env.timeout);
        wdt_write_CR(HS_RTC->WDT_CR | RTC_WDT_UPDATE_MASK | RTC_WDT_ENABLE_MASK);

        // WDT will reset all chip
        register_set1(&HS_PMU->MISC_CTRL_1, PMU_MISC_WDT_RESET_PSO_DIS_MASK|PMU_MISC_WDT_RESET_ALL_DIS_MASK);

        // Enable
        register_set0(&HS_PMU->MISC_CTRL, PMU_MISC_RTC_WDT_DIS_EN_MASK);
        register_set1(&HS_PMU->MISC_CTRL, PMU_MISC_RTC_WDT_FLAG_EN_MASK);

        // Prevent into deep sleep
        pmu_lowpower_prevent(PMU_LP_WDT);
    }
    else
    {
        // timeout save
        wdt_env.timeout = 0;

        // Disable
        register_set1(&HS_PMU->MISC_CTRL, PMU_MISC_RTC_WDT_DIS_EN_MASK);

        // Open RTC APB clock
        register_set0(&HS_PSO->APB_CFG, CPM_RTC_APB_GATE_EN_MASK);
        HS_PSO_UPD_RDY();

        // Disable WDT count
        wdt_write_CR(HS_RTC->WDT_CR & ~RTC_WDT_ENABLE_MASK);

        // Allow into deep sleep
        pmu_lowpower_allow(PMU_LP_WDT);
    }
}

#ifdef CONFIG_BOOTLOADER
#ifndef CONFIG_USE_BASE_COMPONENT_SYMBOL //{BASE_COMPONENT_BEGIN
/**
 * @brief  wdt keepalive in bl
 **/
void wdt_keepalive_in_bl(void)
{
    if((HS_PMU->MISC_CTRL & PMU_MISC_RTC_WDT_DIS_EN_MASK) == 0)
    {
        // Open RTC APB clock
        register_set0(&HS_PMU->MISC_CTRL, PMU_MISC_RTC_CLOCK_SOFT_RESET_MASK | PMU_MISC_RTC_APB_SOFT_RESET_MASK);
        register_set0(&HS_PSO->APB_CFG, CPM_RTC_APB_GATE_EN_MASK);
        HS_PSO_UPD_RDY();

        // keep alive
        wdt_write_SR(16 * 32768);
        wdt_write_CR(HS_RTC->WDT_CR | RTC_WDT_UPDATE_MASK | RTC_WDT_ENABLE_MASK);
    }
}

/**
 * @brief  wdt disable in bl
 **/
void wdt_disable_in_bl(void)
{
    // Disable
    register_set1(&HS_PMU->MISC_CTRL, PMU_MISC_RTC_WDT_DIS_EN_MASK);
}
#endif //}BASE_COMPONENT_END
#endif

/** @} */


