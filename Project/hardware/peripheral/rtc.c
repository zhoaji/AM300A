/**
 * @file rtc.c
 * @brief 
 * @date Thu 11 May 2017 10:22:34 AM CST
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

#ifndef CONFIG_WITHOUT_RW
#include "bb.h"
#endif

/*********************************************************************
 * MACROS
 */


/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
    rtc_callback_t second_callback;
    rtc_callback_t alarm_callback;
}rtc_env_t;

/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */
static rtc_env_t rtc_env = {NULL, NULL};

/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * LOCAL FUNCTIONS
 */

//{BASE_COMPONENT_BEGIN
__STATIC_FORCEINLINE bool rtc_is_running(void)
{
    return !(HS_PMU->MISC_CTRL & PMU_MISC_RTC_APB_SOFT_RESET_MASK);
}
//}BASE_COMPONENT_END

#ifdef CONFIG_HS6621
/**
 * @brief  rtc write CR
 *
 * @param[in] value  value
 **/
static void rtc_write_CR(uint32_t value)
{
    while(!(HS_RTC->CR & RTC_READY_MASK));
    HS_RTC->CR = value;
}

/**
 * @brief  rtc write SR
 *
 * @param[in] value  value
 **/
static void rtc_write_SR(uint32_t value)
{
    while(!(HS_RTC->CR & RTC_READY_MASK));
    HS_RTC->SR = value;
}

/**
 * @brief  rtc write SAR
 *
 * @param[in] value  value
 **/
static void rtc_write_SAR(uint32_t value)
{
    while(!(HS_RTC->CR & RTC_READY_MASK));
    HS_RTC->SAR = value;
}

/**
 * @brief  rtc write GR
 *
 * @param[in] value  value
 **/
static void rtc_write_GR(uint32_t value)
{
    while(!(HS_RTC->CR & RTC_READY_MASK));
    HS_RTC->GR = value;
}
#else
/**
 * @brief  rtc write CR
 *
 * @param[in] value  value
 **/
static void rtc_write_CR(uint32_t value)
{
    HS_RTC->CR = value;
}

/**
 * @brief  rtc write SR
 *
 * @param[in] value  value
 **/
static void rtc_write_SR(uint32_t value)
{
    while(HS_RTC->CR & (1<<7));
    HS_RTC->SR = value;
}

/**
 * @brief  rtc write SAR
 *
 * @param[in] value  value
 **/
static void rtc_write_SAR(uint32_t value)
{
    HS_RTC->CR &= ~(1<<2);
    while(HS_RTC->CR & (1<<18));

    HS_RTC->SAR = value;

    HS_RTC->CR |=  (1<<2);
    while((HS_RTC->CR & (1<<18)) == 0);
}

/**
 * @brief  rtc write GR
 *
 * @param[in] value  value
 **/
static void rtc_write_GR(uint32_t value)
{
    HS_RTC->GR |=  (1u<<31);
    while((HS_RTC->GR & (1<<30)) == 0);

    HS_RTC->GR = value;

    HS_RTC->GR &= ~(1u<<31);
    while(HS_RTC->GR & (1<<30));
}
#endif

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief rtc set current time
 *
 * @param[in] tm  time value, NULL: not modify current time
 * @param[in] second_callback  second event callback
 *
 * @return  None
 **/
void rtc_time_set(struct tm *tm, rtc_callback_t second_callback)
{
    rtc_env.second_callback = second_callback;

    // Open clock
    register_set0(&HS_PSO->APB_CFG, CPM_RTC_APB_GATE_EN_MASK);
    HS_PSO_UPD_RDY();

    // Set time
    if(tm)
    {
        time_t time_sec = mktime(tm);

        // Init Second counter register
        rtc_write_SR(time_sec);

        // Enable RTC
        rtc_write_CR(HS_RTC->CR | RTC_CE_MASK);
    }

    // Callback
    if(second_callback)
    {
        // Enable 1Hz IRQ
        rtc_write_CR(HS_RTC->CR | RTC_1HZIE_MASK);

        // Enable NVIC for RTC
        NVIC_ClearPendingIRQ(RTC_1HZ_IRQn);
        NVIC_SetPriority(RTC_1HZ_IRQn, IRQ_PRIORITY_LOW);
        NVIC_EnableIRQ(RTC_1HZ_IRQn);
    }
    else
    {
        // Disable 1Hz IRQ
        rtc_write_CR(HS_RTC->CR & ~RTC_1HZIE_MASK);

        // Disable NVIC for RTC
        NVIC_DisableIRQ(RTC_1HZ_IRQn);
    }
}

/**
 * @brief rtc current time get
 *
 * @param[out] tm  total seconds start form 1970.01.01, 00:00:00
 *
 * @return None
 **/
void rtc_time_get(struct tm *tm)
{
    time_t time_sec = HS_RTC->SR;

    // function inside memery manage, and should use gmtime() function but there have a problem
    *tm = *localtime(&time_sec);
}

/**
 * @brief rtc set alarm
 *
 * @param[in] tm  time value, NULL: not modify current alarm time
 * @param[in] alarm_callback  alarm event callback
 *
 * @return None
 **/
void rtc_alarm_set(struct tm *tm, rtc_callback_t alarm_callback)
{
    rtc_env.alarm_callback = alarm_callback;

    // Open clock
    register_set0(&HS_PSO->APB_CFG, CPM_RTC_APB_GATE_EN_MASK);
    HS_PSO_UPD_RDY();

    // Set alarm
    if(tm)
    {
        time_t time_sec = mktime(tm);

        // Init Second Alarm register
        rtc_write_SAR(time_sec);
    }

    // callback
    if(alarm_callback)
    {
        // Enable Alarm IRQ
        rtc_write_CR(HS_RTC->CR | RTC_AIE_MASK | RTC_AE_MASK);

        // Enable NVIC for RTC
        NVIC_ClearPendingIRQ(RTC_AF_IRQn);
        NVIC_SetPriority(RTC_AF_IRQn, IRQ_PRIORITY_LOW);
        NVIC_EnableIRQ(RTC_AF_IRQn);
    }
    else
    {
        // Disable Alarm IRQ
        rtc_write_CR(HS_RTC->CR & ~(RTC_AIE_MASK|RTC_AE_MASK));

        // Disable NVIC for RTC Alarm
        NVIC_DisableIRQ(RTC_AF_IRQn);
    }
}

/**
 * @brief rtc get alarm value
 *
 * @param[out] tm time value
 *
 * @return None
 **/
void rtc_alarm_get(struct tm *tm)
{
    time_t time_sec = HS_RTC->SAR;

    // function inside memery manage, and should use gmtime() function but there have a problem
    *tm = *localtime(&time_sec);
}

/**
 * @brief rtc initialize
 *
 * @note Work after POWER_SLEEP (with 32k)
 *
 * @return None
 **/
void rtc_init(void)
{
    // Open clock
    register_set0(&HS_PSO->APB_CFG, CPM_RTC_APB_GATE_EN_MASK);
    HS_PSO_UPD_RDY();

    // Check whether power on
    if(rtc_is_running())
        return;

    // Open RTC power and clock
    register_set1(&HS_PMU->BASEBAND_PM, PMU_PM_RTC_POWER_ON_MASK);
    register_set0(&HS_PMU->BASIC, PMU_BASIC_RTC_CLK_GATE_MASK);

    // Update and wait RTC power on
    register_set1(&HS_PMU->BASIC, PMU_BASIC_APB_BUS_UPD_REG_MASK);
    while(!register_get(&HS_PMU->BASEBAND_PM, MASK_POS(PMU_PM_RTC_POWER_STATUS)));
#ifdef CONFIG_HS6621
    // enable
    register_set0(&HS_PMU->BASIC, PMU_BASIC_RTC_OFF_EN_MASK);
#endif

    // RTC reset (!!! Must: power on firstly, then reset it)
    register_set0(&HS_PMU->MISC_CTRL, PMU_MISC_RTC_CLOCK_SOFT_RESET_MASK | PMU_MISC_RTC_APB_SOFT_RESET_MASK);

    // Calibration RTC
    rtc_write_GR(HS_RTC->GR & ~RTC_LOCK_MASK);
    rtc_write_GR((HS_RTC->GR & ~RTC_NC1HZ_MASK) | (cpm_get_clock(CPM_RTC_CLK)-1));
    rtc_write_GR(HS_RTC->GR | RTC_LOCK_MASK);
}

#ifndef CONFIG_USE_BASE_COMPONENT_SYMBOL //{BASE_COMPONENT_BEGIN
/**
 * @brief rtc store
 *
 * @note Before sleep to POWER_SLEEP, call this function
 *
 * @return None
 **/
void rtc_store(void)
{
//    if(rtc_is_running())
//    {
//        NVIC_DisableIRQ(RTC_1HZ_IRQn);
//    }
}

/**
 * @brief rtc restore
 *
 * @note After weakup from POWER_SLEEP, call this function
 *
 * @return None
 **/
void rtc_restore(void)
{
    if(rtc_is_running())
    {
        register_set0(&HS_PSO->APB_CFG, CPM_RTC_APB_GATE_EN_MASK);
        HS_PSO_UPD_RDY();

        if(HS_RTC->CR & RTC_1HZIE_MASK)
        {
            NVIC_SetPriority(RTC_1HZ_IRQn, IRQ_PRIORITY_LOW);
            NVIC_EnableIRQ(RTC_1HZ_IRQn);
        }

        if(HS_RTC->CR & RTC_AIE_MASK)
        {
            NVIC_SetPriority(RTC_AF_IRQn, IRQ_PRIORITY_LOW);
            NVIC_EnableIRQ(RTC_AF_IRQn);
        }
    }
}

/**
 * @brief rtc_second_irq_is_enabled()
 *
 * @return 
 **/
bool rtc_second_irq_is_enabled(void)
{
    return (HS_RTC->CR & RTC_1HZIE_MASK) ? true : false;
}
#endif //}BASE_COMPONENT_END

/**
 * @brief rtc start
 *
 * @return None
 **/
void rtc_start(void)
{
    pmu_lowpower_prevent(PMU_LP_RTC);
}

/**
 * @brief rtc stop
 *
 * @return None
 **/
void rtc_stop(void)
{
    // Open RTC power and clock
    register_set0(&HS_PMU->BASEBAND_PM, PMU_PM_RTC_POWER_ON_MASK);
    register_set1(&HS_PMU->BASIC, PMU_BASIC_RTC_CLK_GATE_MASK);

    // Update and wait RTC power on
    register_set1(&HS_PMU->BASIC, PMU_BASIC_APB_BUS_UPD_REG_MASK);
    while(!register_get(&HS_PMU->BASEBAND_PM, MASK_POS(PMU_PM_RTC_POWER_STATUS)));
#ifdef CONFIG_HS6621
    // Disable
    register_set0(&HS_PMU->BASIC, PMU_BASIC_RTC_OFF_EN_MASK);
#endif

    pmu_lowpower_allow(PMU_LP_RTC);
}

/**
 * @brief rtc_calc_32k_ppm()
 *
 * @note Just for test
 *
 * @return ppm
 **/
int rtc_calc_32k_ppm(void)
{
//    pmu_select_32k(PMU_32K_SEL_DIV);

    volatile int elapse_32m_us, elapse_32k_us, ppm;
    uint32_t begin_32k_time, end_32k_time, save_32k_time;
    uint32_t begin_32m_time, end_32m_time;

    // Force cpu clock to 32MHz
//    cpm_set_clock(CPM_CPU_CLK, 32000000);

    // Init CPU Counter
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0;

    // Init RTC
    register_set0(&HS_PSO->APB_CFG, CPM_RTC_APB_GATE_EN_MASK);
    HS_PSO_UPD_RDY();
    register_set0(&HS_PMU->MISC_CTRL, PMU_MISC_RTC_CLOCK_SOFT_RESET_MASK | PMU_MISC_RTC_APB_SOFT_RESET_MASK);
    register_set1(&HS_PMU->BASEBAND_PM, PMU_PM_RTC_POWER_ON_MASK);
    register_set0(&HS_PMU->BASIC, PMU_BASIC_RTC_CLK_GATE_MASK);
    register_set1(&HS_PMU->BASIC, PMU_BASIC_APB_BUS_UPD_REG_MASK);
    while(!register_get(&HS_PMU->BASEBAND_PM, MASK_POS(PMU_PM_RTC_POWER_STATUS)));

    rtc_write_GR(HS_RTC->GR & ~RTC_LOCK_MASK);
    rtc_write_GR((HS_RTC->GR & RTC_NC1HZ_MASK) | (0));
    rtc_write_GR(HS_RTC->GR | RTC_LOCK_MASK);
    rtc_write_SR(0);
    rtc_write_CR(HS_RTC->CR | RTC_1HZIE_MASK | RTC_CE_MASK);

    // Critical region enter
    CO_DISABLE_IRQ();

    // Align
    save_32k_time = HS_RTC->SR;
    while(HS_RTC->SR == save_32k_time);
    save_32k_time = HS_RTC->SR;
    while(HS_RTC->SR == save_32k_time);

    // BEGIN
    begin_32m_time = DWT->CYCCNT;
    begin_32k_time = HS_RTC->SR;

    // Delay 5s
    co_delay_ms(1*1000);

    // Align
    save_32k_time = HS_RTC->SR;
    while(HS_RTC->SR == save_32k_time);
    save_32k_time = HS_RTC->SR;
    while(HS_RTC->SR == save_32k_time);

    // END
    end_32m_time = DWT->CYCCNT;
    end_32k_time = HS_RTC->SR;

    // Critical region exit
    CO_RESTORE_IRQ();

    // Unint RTC
    register_set1(&HS_PMU->MISC_CTRL, PMU_MISC_RTC_CLOCK_SOFT_RESET_MASK | PMU_MISC_RTC_APB_SOFT_RESET_MASK);
    register_set0(&HS_PMU->BASEBAND_PM, PMU_PM_RTC_POWER_ON_MASK);
    register_set1(&HS_PMU->BASIC, PMU_BASIC_RTC_CLK_GATE_MASK);
    register_set1(&HS_PMU->BASIC, PMU_BASIC_APB_BUS_UPD_REG_MASK);
    register_set1(&HS_PSO->APB_CFG, CPM_RTC_APB_GATE_EN_MASK);
    HS_PSO_UPD_RDY();

    // Calc
    elapse_32k_us = (int64_t)1000000 * (end_32k_time - begin_32k_time) / cpm_get_clock(CPM_RTC_CLK);
    elapse_32m_us = (int64_t)(end_32m_time - begin_32m_time) / 32;
    ppm = (int64_t)1000000 * (elapse_32k_us - elapse_32m_us) / elapse_32m_us;

    // negative value: frequency smaller
    // positive value: frequency larger
    return ppm;
}

void RTC_AF_IRQHandler(void)
{
    if (HS_RTC->CR & RTC_AF_FLAG_MASK)
    {
#ifdef CONFIG_HS6621
        rtc_write_CR(HS_RTC->CR & ~RTC_AF_MASK);
#else
        rtc_write_CR(HS_RTC->CR | RTC_AF_MASK);
#endif

        if(rtc_env.alarm_callback)
        {
            struct tm tm;

            rtc_time_get(&tm);
            rtc_env.alarm_callback(&tm);
        }
    }
}

void RTC_1HZ_IRQHandler(void)
{
    if (HS_RTC->CR & RTC_1HZ_FLAG_MASK)
    {
#ifdef CONFIG_HS6621
        rtc_write_CR(HS_RTC->CR & ~RTC_1HZ_MASK);
#else
        rtc_write_CR(HS_RTC->CR | RTC_1HZ_MASK);
#endif

        if(rtc_env.second_callback)
        {
            struct tm tm;

            rtc_time_get(&tm);
            rtc_env.second_callback(&tm);
        }
    }
}

/** @} */


