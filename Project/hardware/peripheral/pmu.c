/**
 * @file pmu.c
 * @brief 
 * @date Thu 26 Nov 2015 04:42:20 PM CST
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
#include "co.h"
#include "cfg.h"

/*********************************************************************
 * MACROS
 */

//{BASE_COMPONENT_BEGIN

// unit: us
#define TIMEOUT_XTAL_32MHZ          100000   // General: 2ms
#define TIMEOUT_XTAL_32KHZ          8000000  // General: 0.5s
#define TIMEOUT_RAM_POWERON         1000     // General: 250us

// mode: 1, 2, 3
#define PMU_XTAL32M_FAST_STARTUP_DISTURB_MODE   0

// DEBUG
#define PMU_PIN_STATE_SLEEP_ENABLE //only for debug
#define PMU_DETECT_LOWPOWER_TIME

// USE RC64M when wakeup
//#define PMU_USE_RC64M_AFTER_WAKEUP

/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
    volatile uint32_t ana_mask;
    volatile uint32_t lp_state;
    pmu_32k_sel_t clock_32k;
    gpio_callback_t pin_wakeup_sleep_callback;
    gpio_callback_t pin_wakeup_deep_sleep_callback;
#ifndef CONFIG_GPIO1_ABSENT
    gpio_callback_t pin_wakeup_sleep_callback_ex;
    gpio_callback_t pin_wakeup_deep_sleep_callback_ex;
#endif
    volatile bool pin_wakeup_sleep_recently;
    volatile bool pin_wakeup_deep_sleep_recently;
    bool enable_32k_with_deep_sleep;

    // hook for startup
    pmu_startup_hook_callback_t startup_hook_callback;

    // timer
    pmu_timer_overflow_callback_t timer_overflow_callback;

#ifdef PMU_DETECT_LOWPOWER_TIME
    uint8_t detect_lowpower_time_pin;
#endif
}pmu_env_t;

/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */
#if !defined(CONFIG_USE_BASE_COMPONENT_SYMBOL) && !defined(CONFIG_USE_EXTERN_ENV)
static pmu_env_t pmu_env = {0};
#else
extern pmu_env_t pmu_env;
#endif

/*********************************************************************
 * GLOBAL VARIABLES
 */


/*********************************************************************
 * LOCAL FUNCTIONS
 */

#if 0
/**
 * @brief  pmu record daif dbg
 **/
void pmu_record_daif_dbg(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    DWT->CYCCNT = 0;

    HS_DAIF->DBG_REG = 0x16;

    uint16_t volatile dbg;
    uint16_t volatile dbg_pre = 0xFFFF;

    static volatile struct
    {
        uint16_t dbg;
        uint16_t time;
    }record[1000] = {0};
    int i = 0;

    while(i < 1000)
    {
        dbg = (HS_DAIF->DBG_REG >> (16+9)) & 0xF;
//        dbg = HS_DAIF->DBG_REG >> 16;

        if (dbg != dbg_pre)
        {
            record[i].dbg = dbg;
            record[i].time = DWT->CYCCNT/16;

            dbg_pre = dbg;
            ++i;
        }
    }

    while(1);

#if 0
    // open IO
    pmu_pin_input_enable(0xFFFFFFFF);
    register_set0(&HS_PMU->MISC_CTRL, PMU_MISC_GPIO_AUTO_LATCH_CTRL_MASK);

    // KEY1
    pinmux_config(24,  PINMUX_GPIO_MODE_CFG);
    pmu_pin_mode_set(1<<24, PMU_PIN_MODE_PU);
    gpio_open();
    gpio_set_direction(1<24, GPIO_INPUT);

    // wait
    while(gpio_read(1<<24));

    // enable xtal32m to do xtal 32m start sequence
    REGW1(&HS_PMU->MISC_CTRL_1, PMU_MISC_CRY32M_EN_MASK);
    REGW0(&HS_PMU->MISC_CTRL_1, PMU_MISC_REG_PD_CRY32M_MASK);

    while(1);
#endif
}
#endif

#ifndef CONFIG_USE_BASE_COMPONENT_SYMBOL

#ifdef PMU_DETECT_LOWPOWER_TIME
/**
 * @brief  pmu detect lowpower time pin toggle
 **/
static void pmu_detect_lowpower_time_pin_toggle(void)
{
    if(pmu_env.detect_lowpower_time_pin)
        HS_PMU->GPIO_ODA_CTRL ^= 1u<<pmu_env.detect_lowpower_time_pin;
}
#endif

/**
 * @brief pmu_nvic_restore()
 *
 * @return 
 **/
static void pmu_nvic_restore(void)
{
    // Enable PIN IRQ
    NVIC_SetPriority(PIN_WAKEUP_IRQn, IRQ_PRIORITY_NORMAL);
    NVIC_EnableIRQ(PIN_WAKEUP_IRQn);
    NVIC_SetPriority(POWER_DOWN_IRQn, IRQ_PRIORITY_HIGH);
    NVIC_EnableIRQ(POWER_DOWN_IRQn);

    // Enable timer
    NVIC_SetPriority(PMU_TIMER_IRQn, IRQ_PRIORITY_LOW);
    NVIC_EnableIRQ(PMU_TIMER_IRQn);
}

/**
 * @brief pmu_gpio_lowpower_is_ready()
 *
 * @return 
 **/
static bool pmu_gpio_lowpower_is_ready(void)
{
    uint32_t pin_wakeup_0_31  = HS_PMU->GPIO_WAKEUP;
    bool pin_0_31_ready = pin_wakeup_0_31 ? (gpio_read(pin_wakeup_0_31)==HS_PMU->GPIO_POL) : true;

#ifndef CONFIG_GPIO1_ABSENT
    uint32_t pin_wakeup_32_39 = HS_PMU->GPIO_WAKEUP_1 & PMU_GPIO_WAKEUP_39_32_MASK;
    bool pin_32_39_ready = pin_wakeup_32_39 ? (gpio_read_ex(pin_wakeup_32_39)==HS_PMU->GPIO_POL_1) : true;
    return pin_0_31_ready && pin_32_39_ready;
#else
    return pin_0_31_ready;
#endif
}

/**
 * @brief  pmu pin input enable
 *
 * @param[in] enable_mask  enable mask
 **/
static void pmu_pin_input_enable(uint32_t enable_mask)
{
#if defined(CONFIG_HS6621) && (CONFIG_HARDWARE_VERSION>1)
    // gpio[4:0] high active, gpio[39:5] low active, default: gpio[4:0] input, gpio[39:5] floating
    HS_PMU->GPIO_IE_CTRL = enable_mask ^ 0xFFFFFFE0;
#else
    HS_PMU->GPIO_IE_CTRL = enable_mask;
#endif
}

#ifndef CONFIG_GPIO1_ABSENT
/**
 * @brief  pmu pin input enable
 *
 * @param[in] enable_mask  enable mask
 **/
static void pmu_pin_input_enable_ex(uint32_t enable_mask)
{
#if defined(CONFIG_HS6621) && (CONFIG_HARDWARE_VERSION>1)
    // gpio[4:0] high active, gpio[39:5] low active, default: gpio[4:0] input, gpio[39:5] floating
    REGW(&HS_PMU->GPIO_IE_CTRL_1, MASK_1REG(PMU_GPIO_IE_CTRL_39_32, enable_mask ^ 0xFF));
#else
    REGW(&HS_PMU->GPIO_IE_CTRL_1, MASK_1REG(PMU_GPIO_IE_CTRL_39_32, enable_mask));
#endif
}
#endif

#ifdef CONFIG_HS6621

/**
 * @brief  pmu select 32k get from reg
 *
 * @return 32k
 **/
static pmu_32k_sel_t pmu_select_32k_get_reg(void)
{
    return (pmu_32k_sel_t)register_get(&HS_PMU->MISC_CTRL, MASK_POS(PMU_MISC_CLK_32K_SEL));
}

/**
 * @brief  pmu 32k switch to rc
 *
 * @param[in] calib  calib
 * @param[in] pd_others  pd others
 **/
static void pmu_32k_switch_to_rc(bool calib, bool pd_others)
{
    // Power on rc32k
    REGW0(&HS_PMU->MISC_CTRL, PMU_MISC_REG_PD_RC32K_MASK);
    while(!(HS_PMU->STATUS_READ & PMU_RC32K_READY_MASK));

    // calib it
    if(calib)
        calib_rc32k();

    // Switch
    REGW(&HS_PMU->MISC_CTRL, MASK_1REG(PMU_MISC_CLK_32K_SEL, PMU_32K_SEL_RC));
    while((HS_PMU->STATUS_READ & (PMU_CLK_32K_RC_CRY_DONE_MASK|PMU_CLK_32K_DIV_DONE_MASK)) !=
                                 (PMU_CLK_32K_RC_CRY_DONE_MASK|PMU_CLK_32K_DIV_DONE_MASK));

    if (pd_others)
    {
        // power down others
        REGW1(&HS_PMU->MISC_CTRL, PMU_MISC_CRY_24M_GATE_MASK | PMU_MISC_REG_PD_CRY32K_MASK);
        REGW0(&HS_PMU->MISC_CTRL_1, PMU_MISC_CRY32M_KEEP_ON_MASK);
    }
}
#else
/**
 * @brief  pmu select 32k get from reg
 *
 * @return 32k
 **/
static pmu_32k_sel_t pmu_select_32k_get_reg(void)
{
    if (HS_PMU->MISC_CTRL & PMU_MISC_CLK_32K_SEL_MASK)
        return PMU_32K_SEL_DIV;

    if (HREGRA(&HS_HIB->CONFIG) & HIB_CONFIG_CLK_32K_SEL_MASK)
        return PMU_32K_SEL_32768HZ_XTAL;

    return PMU_32K_SEL_RC;
}

/**
 * @brief  pmu 32k switch to rc
 *
 * @param[in] calib  calib
 * @param[in] pd_others  pd others
 **/
static void pmu_32k_switch_to_rc(bool calib, bool pd_others)
{
    // Power on rc32k
    REGW0(&HS_PMU->MISC_CTRL, PMU_MISC_REG_PD_RC32K_MASK);
    while(!(HS_PMU->STATUS_READ & PMU_RC32K_READY_MASK));

    // calib it
    if(calib)
        calib_rc32k();

    // Switch
    HREGW0(&HS_HIB->CONFIG, HIB_CONFIG_CLK_32K_SEL_MASK); //0:rc32k 1:xtal32k
    REGW0(&HS_PMU->MISC_CTRL, PMU_MISC_CLK_32K_SEL_MASK); //1:div32k 0:other
    while((HS_PMU->STATUS_READ & (PMU_CLK_32K_RC_CRY_DONE_MASK|PMU_CLK_32K_DIV_DONE_MASK)) !=
                                 (PMU_CLK_32K_RC_CRY_DONE_MASK|PMU_CLK_32K_DIV_DONE_MASK));

    if (pd_others)
    {
        // power down others
        HREGW1(&HS_HIB->CONFIG, HIB_CONFIG_PD_CLK_32K_XTAL_MASK); //xtal32k
        REGW0(&HS_SYS->RST_32K_OSC_CTRL, SYS_CRY32M_DIV_EN_MASK); //div32k
    }
}
#endif

#endif

/**
 * @brief pmu_wakeup_pin_wait_idle()
 *
 * @return 
 **/
static void pmu_wakeup_pin_wait_idle(void)
{
    while(HS_PMU->MISC_CTRL & PMU_MISC_CLR_PMU_INT_MASK);
    while(HS_PMU->STATUS_READ & PMU_CLR_PMU_INT_SYNC_APB_MASK);
}

/**
 * @brief pmu_reboot_prepare()
 *
 * @return None
 **/
static void pmu_reboot_prepare(void)
{
    // Disable ALL IRQ, MUST use __set_PRIMASK(1)
     __set_PRIMASK(1);

    // re-calib all thing
    HS_PMU->SW_STATUS &= ~(PMU_SW_STATUS_SYS_CALIBED_MASK|PMU_SW_STATUS_RF_CALIBED_MASK);

    // Set flag
    HS_PMU->SW_STATUS &= ~PMU_SW_STATUS_REBOOT_SW_MASK;
}

#ifndef CONFIG_USE_BASE_COMPONENT_SYMBOL
/**
 * @brief  pmu topclk double preset
 **/
static void pmu_topclk_x2_enable(bool enable)
{
#ifdef CONFIG_HS6621
    if (!enable) return;

    // Enable 32M x2
    REGW1(&HS_PMU->ANA_PD, PMU_ANA_EN_64M_MASK);
    // from RC32M x2
    REGW1(&HS_PMU->CLK_CTRL_2, PMU_REG_SEL_DIG_CLK_MASK); // 0:XTAL 1:RC

#if CONFIG_HARDWARE_VERSION==1
    // OPEN SYSPLL
    REGW0(&HS_DAIF->SYSPLL_CNS0, DAIF_PD_SYSPLL_ANALDO_MASK|DAIF_PD_SYSPLL_DIGLDO_MASK);
    co_delay_us(5);
    REGW0(&HS_DAIF->SYSPLL_CNS0, DAIF_PD_SYSPLL_ANA_MASK);
    co_delay_us(1);
    // Injection disturb signal
    REGW0(&HS_DAIF->SYSPLL_CNS0, DAIF_SYSPLL_RSTN_DIG_MASK);
    REGW1(&HS_DAIF->SYSPLL_CNS0, DAIF_SYSPLL_RSTN_DIG_MASK);
    REGW(&HS_DAIF->SYSPLL_CNS1, MASK_1REG(DAIF_SEL_CPUCLK, 0)); // 0:64m 1:PLL 2:PLL/2
    REGW0(&HS_DAIF->SYSPLL_CNS0, DAIF_SYSPLL_GT_CPUCLK_MASK);
    co_delay_10us(3);
    // PD SYSPLL
    REGW1(&HS_DAIF->SYSPLL_CNS0, DAIF_PD_SYSPLL_ANA_MASK);
#else
    // OPEN SYSPLL
    REGW0(&HS_DAIF->SYSPLL_CNS0, DAIF_PD_SYSPLL_ANALDO_MASK|DAIF_PD_SYSPLL_DIGLDO_MASK);
    co_delay_us(5);
    // Injection disturb signal
    REGW0(&HS_DAIF->SYSPLL_CNS0, DAIF_SYSPLL_RSTN_DIG_MASK);
    REGW1(&HS_DAIF->SYSPLL_CNS0, DAIF_SYSPLL_RSTN_DIG_MASK);
    REGW(&HS_DAIF->SYSPLL_CNS1, MASK_1REG(DAIF_SEL_CPUCLK, 0)); // 0:64m 1:PLL 2:PLL/2
    REGW0(&HS_DAIF->SYSPLL_CNS0, DAIF_SYSPLL_GT_CPUCLK_MASK);
    co_delay_us(1);
#endif

#else

    if (enable)
    {
        // digital ldo to 1.15v (0000=0.65;0111=1.00;1111=1.40;step=50mv)
        HREGW(&HS_HIB->CONFIG, MASK_2REG(HIB_CONFIG_CTRL_LDO_DIG, calib_repair_value_select(10+calib_repair_env.delta_dvdd_1p0,0,15), HIB_CONFIG_LOAD_CTRL_LDO_DIG,1));
        // enable
        REGW1(&HS_PMU->ANA_PD, PMU_ANA_EN_64M_MASK);
    }
    else
    {
        if(HS_PMU->ANA_PD & PMU_ANA_EN_64M_MASK)
        {
            // disable
            REGW0(&HS_PMU->ANA_PD, PMU_ANA_EN_64M_MASK);
            // digital ldo to 1.00v (0000=0.65;0111=1.00;1111=1.40;step=50mv)
            HREGW(&HS_HIB->CONFIG, MASK_2REG(HIB_CONFIG_CTRL_LDO_DIG, calib_repair_value_select(7+calib_repair_env.delta_dvdd_1p0,0,15), HIB_CONFIG_LOAD_CTRL_LDO_DIG,1));
        }
    }
#endif
}

/**
 * @brief  pmu topclk xtal32m wait ready
 **/
static void pmu_topclk_xtal32m_wait_ready(void)
{
#if (CONFIG_HS6621)
    // WAIT Ready
    WAIT_TIMEOUT(HS_DAIF->XTAL32M_CNS0 & DAIF_XTAL32M_CLK_RDY_MASK, TIMEOUT_XTAL_32MHZ);

    // Check
    if(!(HS_DAIF->XTAL32M_CNS0 & DAIF_XTAL32M_CLK_RDY_MASK))
        // System error, LOOP forever
        co_fault_set(CO_FAULT_HCLK_CRASH);
#else  //HS6621c
    // WAIT Ready
    WAIT_TIMEOUT(HS_DAIF->XTAL32M_INTRS & DAIF_XTAL32M_CLK_RDY_MASK, TIMEOUT_XTAL_32MHZ);

    // Check
    if(!(HS_DAIF->XTAL32M_INTRS & DAIF_XTAL32M_CLK_RDY_MASK))
        // System error, LOOP forever
        co_fault_set(CO_FAULT_HCLK_CRASH);
#endif  //(CONFIG_HS6621)

}

/**
 * @brief  pmu topclk switch to rc32m
 **/
static void pmu_topclk_switch_to_rc32m(void)
{
    // To RC32M
    register_set0(&HS_PMU->MISC_CTRL, PMU_MISC_CLK_64M_SEL_MASK); // 0:RC32MHz 1:64MHz
    while(!(HS_PSO->STATUS_READ & CPM_MAIN_CLK_SYNC_DONE_MASK));
}

#ifdef PMU_USE_RC64M_AFTER_WAKEUP
/**
 * @brief  pmu topclk switch to rc64m
 **/
static void pmu_topclk_switch_to_rc32m_x2(void)
{
    // from RC32M
    REGW1(&HS_PMU->CLK_CTRL_2, PMU_REG_SEL_DIG_CLK_MASK); // 0:XTAL 1:RC
#ifndef CONFIG_HS6621
    REGW1(&HS_PMU->XTAL32M_CNS0, PMU_XTAL32M_SEL_CPUCLK_MASK); // 0:XTAL32M 1:64M
#endif
    co_delay_us(1);
    // To 64M
    REGW1(&HS_PMU->MISC_CTRL, PMU_MISC_CLK_64M_SEL_MASK); // 0:RC32MHz 1:64MHz
    while(!(HS_PSO->STATUS_READ & CPM_MAIN_CLK_SYNC_DONE_MASK));
}
#endif

#ifndef CONFIG_HS6621
/**
 * @brief  pmu xtal32m switch to 32m
 **/
static void pmu_topclk_switch_to_xtal32m(void)
{
    // from XTAL32M
    REGW0(&HS_PMU->XTAL32M_CNS0, PMU_XTAL32M_SEL_CPUCLK_MASK); // 0:XTAL32M 1:64M
    co_delay_us(1);
    // To XTAL32M
    REGW1(&HS_PMU->MISC_CTRL, PMU_MISC_CLK_64M_SEL_MASK); // 0:RC32MHz 1:CPUCLK
    while(!(HS_PSO->STATUS_READ & CPM_MAIN_CLK_SYNC_DONE_MASK));
}
#endif
/**
 * @brief  pmu xtal32m switch to 64m
 **/
static void pmu_topclk_switch_to_xtal32m_x2(void)
{
    // from XTAL32M
    REGW0(&HS_PMU->CLK_CTRL_2, PMU_REG_SEL_DIG_CLK_MASK); // 0:XTAL 1:RC
#ifdef CONFIG_HS6621
    REGW(&HS_DAIF->SYSPLL_CNS1, MASK_1REG(DAIF_SEL_CPUCLK, 0)); // 0:64m 1:PLL 2:PLL/2
#else
    REGW1(&HS_PMU->XTAL32M_CNS0, PMU_XTAL32M_SEL_CPUCLK_MASK); // 0:XTAL32M 1:64M
#endif
    co_delay_us(1);
    // To 64M
    REGW1(&HS_PMU->MISC_CTRL, PMU_MISC_CLK_64M_SEL_MASK); // 0:RC32MHz 1:64MHz
    while(!(HS_PSO->STATUS_READ & CPM_MAIN_CLK_SYNC_DONE_MASK));
}

#ifdef CONFIG_HS6621
/**
 * @brief  pmu topclk switch to pll
 **/
static void pmu_topclk_switch_to_pll(void)
{
    // from PLL
    REGW(&HS_DAIF->SYSPLL_CNS1, MASK_1REG(DAIF_SEL_CPUCLK, 1)); // 0:64m 1:PLL 2:PLL/2
    co_delay_us(1);
    // To PLL
    REGW1(&HS_PMU->MISC_CTRL, PMU_MISC_CLK_64M_SEL_MASK); // 0:RC32MHz 1:64MHz
    while(!(HS_PSO->STATUS_READ & CPM_MAIN_CLK_SYNC_DONE_MASK));
}
#endif

/**
 * @brief  pmu rc32m power enable
 **/
static void pmu_topclk_rc32m_power_enable(bool enable)
{
    if(enable)
    {
        register_set0(&HS_PMU->MISC_CTRL_1, PMU_MISC_REG_PD_RC32M_MASK);
        // must delay more than 15us
        co_delay_10us(3);
    }
    else
    {
        register_set1(&HS_PMU->MISC_CTRL_1, PMU_MISC_REG_PD_RC32M_MASK);
    }
}

/**
 * @brief  pmu topclk xtal32m power enable
 *
 * @param[in] enable  
 *
 * XTAL32M
 *
 * if CRY32M_EN==1, xtal24m will be fast-startuped automatically after wakeup (ignore PD_CRY32M)
 *
 * CRY32M_EN does not control xtal24m directly, fucntion:
 *   - xtal24m fast-startup FLAG after wakeup (ignore PD_CRY32M)
 *   - 0 to reset xtal24m-startup-ready signal
 *
 * PD_CRY32M edge can control xtal24m directly, function:
 *   - rising edge: power down
 *   - falling edge: power on
 *
 **/
static void pmu_topclk_xtal32m_power_enable(bool enable)
{
    if(enable)
    {
        // open xtal32m ctrl clock
        REGW1(&HS_DAIF->CLK_ENS, DAIF_XTAL32M_CTRL_CLK_EN_MASK);

        // Power on
        REGW1(&HS_PMU->MISC_CTRL_1, PMU_MISC_CRY32M_EN_MASK);
        REGW0(&HS_PMU->MISC_CTRL_1, PMU_MISC_REG_PD_CRY32M_MASK);

        // wait
        pmu_topclk_xtal32m_wait_ready();

        // close xtal32m ctrl clock
        REGW1(&HS_DAIF->CLK_ENS, DAIF_XTAL32M_CTRL_CLK_EN_MASK);
    }
    else
    {
        REGW0(&HS_PMU->MISC_CTRL_1, PMU_MISC_CRY32M_EN_MASK);
        REGW1(&HS_PMU->MISC_CTRL_1, PMU_MISC_REG_PD_CRY32M_MASK); // rising edge
    }
}
#endif

#ifdef CONFIG_HS6621
/**
 * @brief  pmu topclk pll power enable
 *
 * @param[in] enable  enable
 * @param[in] mhz  144MHz, 128MHz, 112MHz, 96MHz, 80MHz
 **/
static void pmu_topclk_pll_power_enable(bool enable, uint32_t mhz)
{
    if(enable)
    {
        // ldo 1.1v
        REGW(&HS_PMU->ANA_REG, MASK_2REG(PMU_ANA_LDO_V0P9_DIG_VBAT,
                    calib_repair_value_select(9+calib_repair_env.delta_dvdd_1p0,0,15), PMU_ANA_CTRL_LDO_DIG_UPDATE,1));

        // RAM timing
        HS_PMU->RAM_CTRL_2 = 0x53000000;
        HS_PMU->RAM_CTRL_2 = 0x53200000;
        HS_PMU->RAM_CTRL_3 = 0x5300;

        // (mhz/16-5)  4:144MHz, 3:128MHz, 2:112MHz, 1:96MHz, 0:80MHz
        REGW(&HS_DAIF->SYSPLL_CNS0, MASK_1REG(DAIF_SYSPLL_SEL_FREQ, mhz/16-5));

        // power on pll
        REGW(&HS_DAIF->SYSPLL_CNS0, MASK_3REG(DAIF_PD_SYSPLL_ANA,1, DAIF_PD_SYSPLL_ANALDO,0, DAIF_PD_SYSPLL_DIGLDO,0));
        co_delay_10us(2);
        REGW(&HS_DAIF->SYSPLL_CNS0, MASK_3REG(DAIF_PD_SYSPLL_ANA,0, DAIF_PD_SYSPLL_ANALDO,0, DAIF_PD_SYSPLL_DIGLDO,0));
        co_delay_10us(2);

        // calib it
        REGW(&HS_DAIF->SYSPLL_CNS0, MASK_4REG(DAIF_SYSPLL_GT_CPUCLK,0, DAIF_SYSPLL_CTMN,0, DAIF_SYSPLL_RSTN_DIG,0, DAIF_SYSPLL_EN_AFC,0));
        REGW(&HS_DAIF->SYSPLL_CNS0, MASK_2REG(DAIF_SYSPLL_EN_AFC,0, DAIF_SYSPLL_RSTN_DIG,1));
        REGW(&HS_DAIF->SYSPLL_CNS0, MASK_1REG(DAIF_SYSPLL_EN_AFC,1));
        // wait
        while(!(HS_DAIF->SYSPLL_CNS1 & DAIF_LOCK_MASK));

        //clear syspll en afc
        REGW0(&HS_DAIF->SYSPLL_CNS0, DAIF_SYSPLL_EN_AFC_MASK);
    }
    else
    {
        // power off pll
        REGW(&HS_DAIF->SYSPLL_CNS0, MASK_3REG(DAIF_PD_SYSPLL_ANA,1, DAIF_PD_SYSPLL_ANALDO,1, DAIF_PD_SYSPLL_DIGLDO,1));

        // RAM timing
        HS_PMU->RAM_CTRL_2 = 0x52200000;
        HS_PMU->RAM_CTRL_2 = 0x52280000;
        HS_PMU->RAM_CTRL_3 = 0x5200;

        // ldo 1.0v
        REGW(&HS_PMU->ANA_REG, MASK_2REG(PMU_ANA_LDO_V0P9_DIG_VBAT,
                    calib_repair_value_select(7+calib_repair_env.delta_dvdd_1p0,0,15), PMU_ANA_CTRL_LDO_DIG_UPDATE,1));
    }
}
#endif

#ifndef CONFIG_USE_BASE_COMPONENT_SYMBOL

/**
 * @brief  pmu topclk xtal32m is enabled
 **/
static bool pmu_topclk_xtal32m_is_enabled(void)
{
    return (HS_PMU->MISC_CTRL_1 & PMU_MISC_REG_PD_CRY32M_MASK) ? false : true;
}

#ifdef CONFIG_HS6621
/**
 * @brief  pmu topclk pll is enabled
 **/
static bool pmu_topclk_pll_is_enabled(void)
{
    return (pmu_topclk_xtal32m_is_enabled() && (HS_DAIF->SYSPLL_CNS0 & DAIF_PD_SYSPLL_ANA_MASK)==0) ? true : false;
}
#else
/**
 * @brief  pmu topclk xtal32m is enabled
 **/
static bool pmu_topclk_xtal32m_x2_is_enabled(void)
{
    return (HS_PMU->ANA_PD & PMU_ANA_EN_64M_MASK) ? true : false;
}
#endif

/**
 * @brief  pmu xtal32m startup param setup
 **/
static void pmu_xtal32m_startup_param_setup(void)
{
#ifdef CONFIG_HS6621
    // XTAL GM MAX
    REGW(&HS_PMU->CLK_CTRL_1, MASK_1REG(PMU_REG_SEL_GM, 15));
    // ctune
    REGW(&HS_PMU->CLK_CTRL_2, MASK_1REG(PMU_REG_CTUNE_XTAL, 8));
    // boost GM
    REGW(&HS_DAIF->XTAL32M_CNS1, MASK_1REG(DAIF_XTAL32M_NRB_POR, 0));
    // wait time
    REGW(&HS_DAIF->XTAL32M_CNS3, MASK_1REG(DAIF_POR_WAIT_CTRL, 255));
#else
    // XTAL GM MAX
    REGW(&HS_PMU->CLK_CTRL_1, MASK_1REG(PMU_SEL_XTAL32M_GM, 15));
    // ctune
    REGW(&HS_PMU->CLK_CTRL_2, MASK_1REG(PMU_REG_CTUNE_XTAL, 8));
    // boost GM
    REGW(&HS_PMU->XTAL32M_CNS1, MASK_1REG(PMU_XTAL32M_NRB_POR, 0));
    // wait time
    REGW(&HS_PMU->XTAL32M_CNS2, MASK_1REG(PMU_XTAL32M_POR_WAIT_CTRL, 255));
#endif
}

/**
 * @brief  pmu xtal32m fast startup param setup
 **/
static void pmu_xtal32m_fast_startup_param_setup(void)
{
#ifdef CONFIG_HS6621
    REGW(&HS_DAIF->XTAL32M_CNS1, MASK_5REG(
            DAIF_DISTURB_MODE, PMU_XTAL32M_FAST_STARTUP_DISTURB_MODE,
            DAIF_XTAL32M_NRB_POR, 3, // boost GM (FIXME: ==0: lead fast startup hardware-fault, >0: +20mA)
            DAIF_DISTURB_M23_T1_CFG, 0,
#if CONFIG_HARDWARE_VERSION==1
            DAIF_T5_CFG, 7,
#else
            DAIF_T5_CFG, 3, // +1:300us (<70C:3, >70C:4, workaround by calib_repair_env.xtal32m_cap)
#endif
            DAIF_DISTURB_M1_T2_CFG, 3));
    REGW(&HS_DAIF->XTAL32M_CNS2, MASK_1REG(DAIF_DISTURB_M123_NUM_CFG,0));
    REGW(&HS_DAIF->XTAL32M_CNS3, MASK_4REG(DAIF_DN_MODE,1, DAIF_DN_M1T7_M2T1_CFG,1, DAIF_LAST_WAIT_CFG,2, DAIF_PKDVREF_THRSHLD,2));
    REGW(&HS_PMU->CLK_CTRL_1, MASK_2REG(PMU_REG_SEL_NRB,8, PMU_SEL_XTAL32M_PKDVREF_MO,4));
#else
    REGW(&HS_PMU->XTAL32M_CNS0, MASK_1REG(PMU_XTAL32M_DISTURB_M123_NUM_CFG, 0));
    REGW(&HS_PMU->XTAL32M_CNS1, MASK_3REG(
            PMU_XTAL32M_NRB_POR, 3, // boost GM (FIXME: ==0: lead fast startup hardware-fault, >0: +20mA)
            PMU_XTAL32M_DISTURB_M23_T1_CFG, 0,
            PMU_XTAL32M_T5_CFG, 3));
    REGW(&HS_PMU->XTAL32M_CNS2, MASK_4REG(PMU_XTAL32M_DN_MODE, 1, PMU_XTAL32M_DN_M1T7_M2T1_CFG, 1, PMU_XTAL32M_LAST_WAIT_CFG, 2, PMU_XTAL32M_PKDVREF_THRSHLD, 2));
    // XTAL GM MAX
    REGW(&HS_PMU->CLK_CTRL_1, MASK_1REG(PMU_SEL_XTAL32M_GM, 15));
#endif
}

/**
 * @brief  pmu dcdc lowpower enter
 *
 * @param[in] enter  enter
 *
 * @note
 *  Flow:
 *    sleep:  {open-LDO -> delay 50us  -> close-DCDC} -> [close-LDO]
 *    wakeup: [open-LDO -> delay 200us] -> {open-DCDC  -> delay 100us -> close-LDO}
 *    {...}: software flow
 *    [...]: digital flow
 **/
static void pmu_dcdc_lowpower_enter(bool enter)
{
    if (HS_PMU->SW_STATUS & PMU_SW_STATUS_DCDC_ENABLED_MASK)
    {
        if(enter)
        {
            REGW(&HS_PMU->ANA_PD, MASK_1REG(PMU_ANA_LDO_1P5_DIS, 0));
            co_delay_10us(5);
#if defined(CONFIG_HS6621) && (CONFIG_HARDWARE_VERSION>1)
            REGW(&HS_PMU->ANA_PD, MASK_1REG(PMU_ANA_BUCK_EN, 0));
#else
            REGW(&HS_PMU->ANA_PD, MASK_1REG(PMU_ANA_BUCK_DIS, 1));
#endif
        }
        else
        {
#if defined(CONFIG_HS6621) && (CONFIG_HARDWARE_VERSION>1)
            REGW(&HS_PMU->ANA_PD, MASK_1REG(PMU_ANA_BUCK_EN, 1));
#else
            REGW(&HS_PMU->ANA_PD, MASK_1REG(PMU_ANA_BUCK_DIS, 0));
#endif
            co_delay_10us(10);
            REGW(&HS_PMU->ANA_PD, MASK_1REG(PMU_ANA_LDO_1P5_DIS, 1));
        }
    }
}

/**
 * @brief  pmu clock lowpower enter
 *
 * @param[in] enter  enter
 * @param[in] state  state
 **/
static void pmu_clock_lowpower_enter(bool enter, pmu_lowpower_state_t state)
{
    CPM_ANA_AHB_CLK_ENABLE();

    if(enter)
    {
        // power on and switch RC32M
        pmu_topclk_rc32m_power_enable(true);
        pmu_topclk_switch_to_rc32m();

#ifdef CONFIG_HS6621
        // check pll and power off
        if(pmu_topclk_pll_is_enabled())
            pmu_topclk_pll_power_enable(false, 0/*freq*/);
#endif

        // check xtal32m and power off
        if(pmu_topclk_xtal32m_is_enabled())
        {
            pmu_topclk_xtal32m_power_enable(false);
            pmu_topclk_x2_enable(false);
        }
    }
    else
    {
        // restore calibed value
        calib_sys_restore();

        if(state != PMU_SLEEP_HXTAL_OFF)
        {
            pmu_xtal32m_fast_startup();
        }
#ifdef PMU_USE_RC64M_AFTER_WAKEUP
        else
        {
            // RC64M
            pmu_topclk_x2_enable(true);
            pmu_topclk_switch_to_rc32m_x2();
        }
#endif

        // disable all daif clock
        HS_DAIF->CLK_ENS = 0;
    }

    CPM_ANA_CLK_RESTORE();
}

/**
 * @brief pmu_random_seed_is_valid()
 *
 * It shall have no more than six consecutive zeros or ones.
 * It shall have no more than 24 transitions.
 *
 * @param[in] random_seed  
 *
 * @return 
 **/
static bool pmu_random_seed_is_valid(uint32_t random_seed)
{
    // No More than 6 consequtive zeros or ones.
    // No more than 24 transitions

    uint8_t num_consequtive_ones = 0;
    uint8_t num_consequtive_zeros = 0;
    uint8_t num_transitions = 0;
    uint8_t pre_bitval = 0;
    uint8_t j;
    uint8_t bitval;

    pre_bitval = random_seed & 1;
    for (j=1; j<32; j++)
    {
        bitval = (random_seed >> j) & 1;

        if (bitval == pre_bitval)
        {
            if(bitval)
                num_consequtive_ones++;
            else
                num_consequtive_zeros++;
        }
        else
        {
            num_consequtive_ones = 0;
            num_consequtive_zeros = 0;
            num_transitions++;
        }

        pre_bitval = bitval;

        if ((num_consequtive_ones > 5) || (num_consequtive_zeros > 5) || (num_transitions > 24))
            return false;
    }

    return true;
}

#endif

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

#ifndef CONFIG_USE_BASE_COMPONENT_SYMBOL

/**
 * @brief pmu initialize
 *
 * @return None
 **/
void pmu_init(void)
{
#ifdef CONFIG_HS6621
    // RAM timing (1.0v)
    HS_PMU->RAM_CTRL_2 = 0x52200000;
    HS_PMU->RAM_CTRL_2 = 0x52280000;
    HS_PMU->RAM_CTRL_3 = 0x5200;
#endif

    // Pinmux control the pin
    register_set0(&HS_PMU->MISC_CTRL, PMU_MISC_GPIO_AUTO_LATCH_CTRL_MASK);

    // GPIO wakeup mask disable (by default all is open)
    HS_PMU->GPIO_WAKEUP = 0;
    HS_PMU->GPIO_POL = 0;
    // Enable IE control
    pmu_pin_input_enable(0xFFFFFFFF);

#ifndef CONFIG_GPIO1_ABSENT
    HS_PMU->GPIO_WAKEUP_1 = 0;
    HS_PMU->GPIO_POL_1 = 0;
    pmu_pin_input_enable_ex(0xFF);
#endif

    // Disable test_en
#ifdef CONFIG_HS6621
    REGW1(&HS_PMU->ANA_PD, PMU_ANA_TEST_EN_DISABLE_MASK);
    REGW0(&HS_PMU->MISC_CTRL_1, PMU_MISC_CPU_JTAG_TEST_EN_MASK|PMU_MISC_CPU_JTAG_ICG_EN_MASK);
#else
    HREGW0(&HS_HIB->CONFIG_1, HIB_CONFIG_CHIP_JTAG_ICE_EN_MASK|HIB_CONFIG_CHIP_JTAG_TEST_EN_MASK);
#endif

#ifndef CONFIG_HS6621
    // clear hib gpio latch
    HREGW1(&HS_HIB->CONFIG_1, HIB_CONFIG_GPIO_LATCH_ANA_CLR_MASK);

    // HIB RAM1 power down
    HREGW1(&HS_HIB->CONFIG, HIB_CONFIG_RAM1_PSW_HIB_PD_MASK);

    // 32k
    HREGW(&HS_HIB->CONFIG_1, MASK_2REG(HIB_CONFIG_PD_RC32K_LDOSTARTUP,1, HIB_CONFIG_SEL_IGLOB,2));
    REGW1(&HS_PMU->ANA_PD, PMU_ANA_PD_CKOPMU32K_MASK);
#endif

    // Store wdt-reset and clear it
    if(HS_PMU->AHB_REMAP & PMU_WDT_RESET_MASK)
    {
        // Clear
        register_set0(&HS_PMU->MISC_CTRL, PMU_MISC_RTC_WDT_FLAG_EN_MASK);
        register_set0(&HS_PMU->AHB_REMAP, PMU_WDT_RESET_MASK);

        // Soft flag
        register_set1(&HS_PMU->SW_STATUS, PMU_SW_STATUS_REBOOT_FROM_WDT_MASK);
        register_set0(&HS_PMU->SW_STATUS, PMU_SW_STATUS_REBOOT_FROM_SOFT_RESET_MASK);
    }
    else
    {
        register_set0(&HS_PMU->SW_STATUS, PMU_SW_STATUS_REBOOT_FROM_WDT_MASK);
    }

#ifndef CONFIG_HS6621
    if((HS_PMU->SW_STATUS & PMU_SW_STATUS_REBOOT_MASK) == 0)
    {
        if(HREGRA(&HS_HIB->CONFIG) & (HIB_CONFIG_TIMER_INT_EN_MASK|HIB_CONFIG_GPIO_WAKE_INT_EN_MASK))
            REGW1(&HS_PMU->SW_STATUS, PMU_SW_STATUS_REBOOT_FROM_SLEEP_MASK);
    }
#endif

    // pin wakeup debounce time to 30us
    register_set(&HS_PMU->WAKE_DEB, MASK_2REG(PMU_PIN_DEBOUNCE_CYCLE_WAKE, 0,
                                              PMU_PIN_DEBOUNCE_COEFF_WAKE, 0));
    register_set1(&HS_PMU->WAKE_DEB, PMU_PIN_DEB_RST_MASK); //重新设debounce时间前，都要把debounce逻辑清零

    // Aggressive CPU gate feature:
    REGW0(HS_SYS->PINMUX, SYS_PINMUX_SYSPLL_GT_CPUCLK_HW_CTRL_MASK);

    // RAM deep sleep ctrl
    REGW1(&HS_PMU->RAM_CTRL_2, PMU_RAM_DS_HW_CTRL_EN_MASK);
}

#ifdef CONFIG_HS6621
/**
 * @brief pmu select xtal24m as top clock, call by system
 **/
void pmu_xtal32m_startup(void)
{
    CPM_ANA_AHB_CLK_ENABLE();

    // check: whether is xtal24m opened
    if(!pmu_topclk_xtal32m_is_enabled())
    {
        // Next XTAL32M startup use normal-startup mode.
        REGW1(&HS_PMU->ANA_CON, PMU_FIRST_RUN_REG_MASK);

        // power on rc32m and switch to it
        // Make sure CPU running on RC32M
        pmu_topclk_rc32m_power_enable(true);
        pmu_topclk_switch_to_rc32m();

        // Try open xtal32m
        pmu_xtal32m_startup_param_setup();
        pmu_topclk_xtal32m_power_enable(true);

        // calib RC
        calib_sys_rc();

        // preset x2 param
        pmu_topclk_x2_enable(true);

        // to xtal64m
        pmu_topclk_switch_to_xtal32m_x2();

        // calib RC32M
        calib_sys_rc32m();

        // power off rc32m
        pmu_topclk_rc32m_power_enable(false);
    }

    // disable all daif clock
    HS_DAIF->CLK_ENS = 0;

    CPM_ANA_CLK_DISABLE();
}

/**
 * @brief  pmu xtal32m fast startup
 **/
void pmu_xtal32m_fast_startup(void)
{
    CPM_ANA_AHB_CLK_ENABLE();

    // check
    if(!pmu_topclk_xtal32m_is_enabled())
    {
        // Next XTAL32M startup use fast-startup mode.
        REGW0(&HS_PMU->ANA_CON, PMU_FIRST_RUN_REG_MASK);

        // Make sure CPU running on RC32M
        pmu_topclk_rc32m_power_enable(true);
        pmu_topclk_switch_to_rc32m();

        // xtal32m_ldo=1.1v (00:0.95V, 01:1V, 10:1.05V, 11:1.1V)
        REGW(&HS_PMU->CLK_CTRL_2, MASK_1REG(PMU_CTRL_XTAL32M_LDO, 3));

        // preset x2 param
        pmu_topclk_x2_enable(true);

        // Try open xtal24m
        pmu_xtal32m_fast_startup_param_setup();
        pmu_topclk_xtal32m_power_enable(true);

        // xtal32m_ldo=1.0v
        REGW(&HS_PMU->CLK_CTRL_2, MASK_1REG(PMU_CTRL_XTAL32M_LDO, 1));

        // to xtal64m
        pmu_topclk_switch_to_xtal32m_x2();

        // power off rc32m
        pmu_topclk_rc32m_power_enable(false);

        /*
         * Aggressive CPU gate feature:
         *   when enter WFI, all CPU clock will be gated and SWD can't visit.
         * Enable dependent:
         *   1. XTAL32M must be startuped.
         *   2. DMA must be stoped.
         */
        if(HS_PMU->SW_STATUS & PMU_SW_STATUS_AGGRESSIVE_CPUGATE_MASK)
        {
            if (HS_PSO->DMA_CFG & CPM_DMA_GATE_EN_MASK)
                REGW1(HS_SYS->PINMUX, SYS_PINMUX_SYSPLL_GT_CPUCLK_HW_CTRL_MASK);
        }
    }

    CPM_ANA_CLK_RESTORE();
}

/**
 * @brief  pmu recalib sysclk
 **/
void pmu_topclk_recalib(void)
{
    CPM_ANA_AHB_CLK_ENABLE();

    if(!pmu_topclk_xtal32m_is_enabled())
        pmu_xtal32m_fast_startup();

    // power on rc32m and switch to it
    // Make sure CPU running on RC32M
    pmu_topclk_rc32m_power_enable(true);
    pmu_topclk_switch_to_rc32m();

    // calib RC
    calib_sys_rc();

    // preset x2 param
    pmu_topclk_x2_enable(true);

    // to xtal64m
    pmu_topclk_switch_to_xtal32m_x2();

    // calib RC32M
    calib_sys_rc32m();

    // power off rc32m
    pmu_topclk_rc32m_power_enable(false);

    CPM_ANA_CLK_RESTORE();
}
#else

/**
 * @brief pmu select xtal24m as top clock, call by system
 **/
void pmu_xtal32m_startup(void)
{
    CPM_ANA_AHB_CLK_ENABLE();

    // check: whether is xtal24m opened
    if(!pmu_topclk_xtal32m_is_enabled())
    {
        // Next XTAL32M startup use normal-startup mode.
        REGW1(&HS_PMU->MISC_CTRL, PMU_MISC_FIRST_RUN_REG_MASK);

        // power on rc32m and switch to it
        // Make sure CPU running on RC32M
        pmu_topclk_rc32m_power_enable(true);
        pmu_topclk_switch_to_rc32m();

        // Try open xtal32m
        pmu_xtal32m_startup_param_setup();
        pmu_topclk_xtal32m_power_enable(true);

        // calib RC
        calib_sys_rc();

        // to xtal32m
        pmu_topclk_switch_to_xtal32m();

        // calib RC32M
        calib_sys_rc32m();

        // power off rc32m
        pmu_topclk_rc32m_power_enable(false);
    }

    // disable all daif clock
    HS_DAIF->CLK_ENS = 0;

    CPM_ANA_CLK_DISABLE();
}

/**
 * @brief  pmu xtal32m fast startup
 **/
void pmu_xtal32m_fast_startup(void)
{
    CPM_ANA_AHB_CLK_ENABLE();

    // check
    if(!pmu_topclk_xtal32m_is_enabled())
    {
        // Next XTAL32M startup use fast-startup mode.
        REGW0(&HS_PMU->MISC_CTRL, PMU_MISC_FIRST_RUN_REG_MASK);

        // Make sure CPU running on RC32M
        pmu_topclk_rc32m_power_enable(true);
        pmu_topclk_switch_to_rc32m();

        // Try open xtal24m
        pmu_xtal32m_fast_startup_param_setup();
        pmu_topclk_xtal32m_power_enable(true);

        // to xtal32m
        pmu_topclk_switch_to_xtal32m();

        // power off rc32m
        pmu_topclk_rc32m_power_enable(false);

        /*
         * Aggressive CPU gate feature:
         *   when enter WFI, all CPU clock will be gated and SWD can't visit.
         * Enable dependent:
         *   1. XTAL32M must be startuped.
         *   2. DMA must be stoped.
         */
        if(HS_PMU->SW_STATUS & PMU_SW_STATUS_AGGRESSIVE_CPUGATE_MASK)
        {
            if (HS_PSO->DMA_CFG & CPM_DMA_GATE_EN_MASK)
                REGW1(HS_SYS->PINMUX, SYS_PINMUX_SYSPLL_GT_CPUCLK_HW_CTRL_MASK);
        }
    }

    CPM_ANA_CLK_RESTORE();
}

/**
 * @brief  pmu xtal32m x2 startup
 **/
void pmu_xtal32m_x2_startup(void)
{
    CPM_ANA_AHB_CLK_ENABLE();

    // startup xtal32m
    if(!pmu_topclk_xtal32m_is_enabled())
        pmu_xtal32m_fast_startup();

    // startup xtal32m x2
    if (!pmu_topclk_xtal32m_x2_is_enabled())
    {
        // to rc32m
        pmu_topclk_rc32m_power_enable(true);
        pmu_topclk_switch_to_rc32m();

        // delay 2us
        co_delay_us(2);

        // to xtal64m
        pmu_topclk_x2_enable(true);
        pmu_topclk_switch_to_xtal32m_x2();

        // power off rc32m
        pmu_topclk_rc32m_power_enable(false);
    }

    CPM_ANA_CLK_DISABLE();
}

/**
 * @brief  pmu xtal32m x2 close
 **/
void pmu_xtal32m_x2_close(void)
{
    CPM_ANA_AHB_CLK_ENABLE();

    // startup xtal32m x2
    if (pmu_topclk_xtal32m_is_enabled() && pmu_topclk_xtal32m_x2_is_enabled())
    {
        // to rc32m
        pmu_topclk_rc32m_power_enable(true);
        pmu_topclk_switch_to_rc32m();

        // delay 2us
        co_delay_us(2);

        // switch to xtal32m
        pmu_topclk_switch_to_xtal32m();

        // disable x2
        pmu_topclk_x2_enable(false);

        // power off rc32m
        pmu_topclk_rc32m_power_enable(false);
    }

    CPM_ANA_CLK_DISABLE();
}

/**
 * @brief  pmu recalib sysclk
 **/
void pmu_topclk_recalib(void)
{
    CPM_ANA_AHB_CLK_ENABLE();

    if(!pmu_topclk_xtal32m_is_enabled())
        pmu_xtal32m_fast_startup();

    // power on rc32m and switch to it
    // Make sure CPU running on RC32M
    pmu_topclk_rc32m_power_enable(true);
    pmu_topclk_switch_to_rc32m();

    // calib RC
    calib_sys_rc();

    // to xtal32m
    pmu_topclk_switch_to_xtal32m();

    // calib RC32M
    calib_sys_rc32m();

    if (pmu_topclk_xtal32m_x2_is_enabled())
    {
        // to rc32m
        pmu_topclk_switch_to_rc32m();

        // delay 2us
        co_delay_us(2);

        // to xtal64m
        pmu_topclk_x2_enable(true);
        pmu_topclk_switch_to_xtal32m_x2();
    }

    // power off rc32m
    pmu_topclk_rc32m_power_enable(false);

    CPM_ANA_CLK_RESTORE();
}
#endif

/**
 * @brief pmu ram power on all block, call by system
 *
 * @return None
 **/
void pmu_ram_power_on_all(void)
{
#ifdef CONFIG_HS6621
    // RAM power on.
    HS_PMU->RAM_PM_1 = PMU_PM_RAM_POWER_ON_MASK;
    // Wait all ram power on
    while(HS_PMU->RAM_PM_2 != PMU_PM_RAM_POWER_STATUS_MASK);
#else
    // In HS6621C: RAM auto power on when wakeup, RAM power down control by this REG when sleep.
    REGW1(&HS_PMU->RAM_PM_1, PMU_PM_RAM_POWER_ON_MASK);
#endif

    // RAM clock on. Default: ALL ULP RAM power on [first 120KB, last 8KB]
    // NOTE: Also do this in reset_handler for user code. reset_handler must be loacated in 1st block
    HS_PSO->RAM_CFG = 0;
    HS_PSO_UPD_RDY();
}

/**
 * @brief pmu reversion check, call by system
 *
 * @return None
 **/
void pmu_reversion_check(void)
{
    register uint32_t soft_rev = register_get(&HS_SYS->REV_ID, MASK_POS(SYS_REV_SOFT));

    // After HS6620A2, the REV_ID identify ROM version or RAM version
    switch (soft_rev)
    {
        case SYS_REV_SOFT_RAM_VERSION:
            // enable jlink
            register_set1(&HS_PMU->MISC_CTRL, PMU_MISC_JLINK_ENABLE_MASK);
            // Loop
            while(1);
//            break;

        case SYS_REV_SOFT_SFLASH_VERSION:
            __disable_irq();
            pmu_memery_remap(PMU_REMAP_FROM_SFLASH);
            pmu_cpu_reset();
            while(1);
            //break;

        default:
            break;
    }
}

#ifdef CONFIG_HS6621
/**
 * @brief pmu select 32k
 *
 * @param[in] clk32k  32k clock select
 *
 * @return None
 **/
void pmu_select_32k(pmu_32k_sel_t clk32k)
{
    // Default: rc32k powered on, xtal32k powered down
    switch(clk32k)
    {
        case PMU_32K_SEL_RC:
            pmu_select_32k(PMU_32K_SEL_DIV);
            pmu_32k_switch_to_rc(true /*calib*/, true/*pd_others*/);
            break;

        case PMU_32K_SEL_32768HZ_XTAL:
            // Power on rc32k
            REGW0(&HS_PMU->MISC_CTRL, PMU_MISC_REG_PD_RC32K_MASK);
            while(!(HS_PMU->STATUS_READ & PMU_RC32K_READY_MASK));

            // Power on xtal32k
            REGW0(&HS_PMU->MISC_CTRL, PMU_MISC_REG_PD_CRY32K_MASK);
            while(!(HS_PMU->BASIC & PMU_BASIC_ANA_CRY32K_READY_MASK));

            // Switch
            REGW(&HS_PMU->MISC_CTRL, MASK_1REG(PMU_MISC_CLK_32K_SEL, PMU_32K_SEL_32768HZ_XTAL));
            while((HS_PMU->STATUS_READ & (PMU_CLK_32K_RC_CRY_DONE_MASK|PMU_CLK_32K_DIV_DONE_MASK)) !=
                                         (PMU_CLK_32K_RC_CRY_DONE_MASK|PMU_CLK_32K_DIV_DONE_MASK));

            // power down others
            REGW1(&HS_PMU->MISC_CTRL, PMU_MISC_CRY_24M_GATE_MASK | PMU_MISC_REG_PD_RC32K_MASK);
            REGW0(&HS_PMU->MISC_CTRL_1, PMU_MISC_CRY32M_KEEP_ON_MASK);
            break;

        case PMU_32K_SEL_DIV:
            // Power on rc32k
            REGW0(&HS_PMU->MISC_CTRL, PMU_MISC_REG_PD_RC32K_MASK);
            while(!(HS_PMU->STATUS_READ & PMU_RC32K_READY_MASK));
            // Open clock
            REGW0(&HS_PMU->MISC_CTRL, PMU_MISC_CRY_24M_GATE_MASK);

            // Switch
            REGW(&HS_PMU->MISC_CTRL, MASK_1REG(PMU_MISC_CLK_32K_SEL, PMU_32K_SEL_DIV));
            while((HS_PMU->STATUS_READ & (PMU_CLK_32K_RC_CRY_DONE_MASK|PMU_CLK_32K_DIV_DONE_MASK)) !=
                                         (PMU_CLK_32K_RC_CRY_DONE_MASK|PMU_CLK_32K_DIV_DONE_MASK));

            // power down others
            REGW1(&HS_PMU->MISC_CTRL, PMU_MISC_REG_PD_CRY32K_MASK | PMU_MISC_REG_PD_RC32K_MASK);
            break;
    }

    pmu_env.clock_32k = clk32k;
}
#else
void pmu_select_32k(pmu_32k_sel_t clk32k)
{
    // Default: rc32k powered on, xtal32k powered down
    switch(clk32k)
    {
        case PMU_32K_SEL_RC:
            pmu_select_32k(PMU_32K_SEL_DIV);
            pmu_32k_switch_to_rc(true /*calib*/, true/*pd_others*/);
            break;

        case PMU_32K_SEL_32768HZ_XTAL:
            // Power on rc32k
            REGW0(&HS_PMU->MISC_CTRL, PMU_MISC_REG_PD_RC32K_MASK);
            while(!(HS_PMU->STATUS_READ & PMU_RC32K_READY_MASK));

            // Power on xtal32k
            HREGW0(&HS_HIB->CONFIG, HIB_CONFIG_PD_CLK_32K_XTAL_MASK);
            while(!(HS_PMU->BASIC & PMU_BASIC_ANA_CRY32K_READY_MASK));

            // Switch
            HREGW1(&HS_HIB->CONFIG, HIB_CONFIG_CLK_32K_SEL_MASK); //0:rc32k 1:xtal32k
            REGW0(&HS_PMU->MISC_CTRL, PMU_MISC_CLK_32K_SEL_MASK); //1:div32k 0:other
            while((HS_PMU->STATUS_READ & (PMU_CLK_32K_RC_CRY_DONE_MASK|PMU_CLK_32K_DIV_DONE_MASK)) !=
                                         (PMU_CLK_32K_RC_CRY_DONE_MASK|PMU_CLK_32K_DIV_DONE_MASK));

            // power down others
            REGW1(&HS_PMU->MISC_CTRL, PMU_MISC_REG_PD_RC32K_MASK); //rc32k
            REGW0(&HS_SYS->RST_32K_OSC_CTRL, SYS_CRY32M_DIV_EN_MASK); //div32k
            break;

        case PMU_32K_SEL_DIV:
            // Open clock
            REGW1(&HS_SYS->RST_32K_OSC_CTRL, SYS_CRY32M_DIV_EN_MASK);

            // Switch
            REGW1(&HS_PMU->MISC_CTRL, PMU_MISC_CLK_32K_SEL_MASK); //1:div32k 0:other
            while((HS_PMU->STATUS_READ & (PMU_CLK_32K_RC_CRY_DONE_MASK|PMU_CLK_32K_DIV_DONE_MASK)) !=
                                         (PMU_CLK_32K_RC_CRY_DONE_MASK|PMU_CLK_32K_DIV_DONE_MASK));

            // power down others
//            REGW1(&HS_PMU->MISC_CTRL, PMU_MISC_REG_PD_RC32K_MASK); //rc32k
//            REGW1(&HS_HIB->CONFIG, HIB_CONFIG_PD_CLK_32K_XTAL_MASK); //xtal32k
            break;
    }

    pmu_env.clock_32k = clk32k;
}
#endif

/**
 * @brief pmu get 32k select
 *
 * @return 32k select
 **/
pmu_32k_sel_t pmu_select_32k_get(void)
{
    return pmu_env.clock_32k;
}

/**
 * @brief pmu_wakeup_pin_get()
 *
 * @return
 **/
uint32_t pmu_wakeup_pin_get(void)
{
    return HS_PMU->GPIO_WAKEUP;
}

#ifndef CONFIG_GPIO1_ABSENT
/**
 * @brief pmu_wakeup_pin_get()
 *
 * @return gpio32-gpio39
 **/
uint32_t pmu_wakeup_pin_get_ex(void)
{
    return HS_PMU->GPIO_WAKEUP_1 & PMU_GPIO_WAKEUP_39_32_MASK;
}
#endif

/**
 * @brief pmu prevent lowpower entry
 *
 * @param[in] lpp  peripheral
 *
 * @return None
 **/
void pmu_lowpower_prevent(pmu_lowpower_peripheral_t lpp)
{
    CO_DISABLE_IRQ();
    pmu_env.lp_state |= (uint32_t)lpp;
    CO_RESTORE_IRQ();
}

/**
 * @brief pmu allow lowpower entry
 *
 * @param[in] lpp  peripheral
 *
 * @return None
 **/
void pmu_lowpower_allow(pmu_lowpower_peripheral_t lpp)
{
    CO_DISABLE_IRQ();
    pmu_env.lp_state &= ~((uint32_t)lpp);
    CO_RESTORE_IRQ();
}

/**
 * @brief lowpower status
 *
 * @return lowpower status
 **/
pmu_lowpower_peripheral_t pmu_lowpower_prevent_status(void)
{
    return (pmu_lowpower_peripheral_t)pmu_env.lp_state;
}

/**
 * @brief what power status should be entried
 *
 * @return power status
 **/
co_power_status_t pmu_power_status(void)
{
    co_power_status_t max_power_status = POWER_DEEP_SLEEP;

    if(pmu_env.lp_state)
    {
        // General peripheral lowpower control
        if(pmu_env.lp_state & ~(PMU_LP_RTC|PMU_LP_WDT))
        {
            return POWER_IDLE;
        }

        // WDT need 32K, can't into deep sleep (Must check WDT firster than RTC)
        else if(pmu_env.lp_state & PMU_LP_WDT)
        {
            max_power_status = POWER_SLEEP;
            // fall through
        }

        // RTC need 32K
        else if(pmu_env.lp_state & PMU_LP_RTC)
        {
            if(!pmu_env.enable_32k_with_deep_sleep || rtc_second_irq_is_enabled())
                max_power_status = POWER_SLEEP;
            // fall through
        }
    }

#ifndef CONFIG_USE_RWIP_CO_TIMER
    if(max_power_status == POWER_DEEP_SLEEP)
    {
        if(HS_PMU->MISC_CTRL_1 & PMU_MISC_TIMER_INT_CPU_EN_MASK)
        {
            // TODO: use sc.min_sleep_space
            uint32_t left = co_time_pasting_time(HS_PMU->TIMER_VAL, HS_PMU->TIMER_CNT);
            left = CO_TIME_SYS2MS(left);

            if(left > 4)
                max_power_status = POWER_SLEEP;
            else
                max_power_status = POWER_IDLE;
        }
    }
#endif

//    if(!(HS_PMU->BASIC & PMU_BASIC_POWER_DOWN_DISABLE_MASK))
//    {
//        if(!(HS_SYS->GPIO_POWER_UP_STATUS & SYS_GPIO_STAUS_POWERDOWN_MASK))
//            return POWER_IDLE;
//    }

    return pmu_gpio_lowpower_is_ready() ? max_power_status : POWER_IDLE;
}

/**
 * @brief pmu enter lowpower, call by system
 *
 * @param[in] state  lowpower state
 *
 * @return None
 **/
void pmu_lowpower_enter(pmu_lowpower_state_t state)
{
    pmu_32k_sel_t clock_32k = pmu_select_32k_get();

    switch(state)
    {
        case PMU_DEEP_SLEEP:
            if(!pmu_env.enable_32k_with_deep_sleep)
            {
                switch(clock_32k)
                {
                    case PMU_32K_SEL_RC:
                        break;

                    case PMU_32K_SEL_32768HZ_XTAL:
                    case PMU_32K_SEL_DIV:
                        // Power on 32k rc and switch to it
                        pmu_32k_switch_to_rc(false /*calib*/, true /*pd_others*/);
                        break;
                }
                // Set a flag to power down 32K
                register_set1(&HS_PMU->BASIC, PMU_BASIC_SLEEP_WO_32K_MASK);
                HS_PMU_UPD_RDY();
                break;
            }
            // fall through

        case PMU_SLEEP:
        case PMU_SLEEP_HXTAL_OFF:
            switch(clock_32k)
            {
                case PMU_32K_SEL_RC:
                case PMU_32K_SEL_32768HZ_XTAL:
                    break;

                case PMU_32K_SEL_DIV:
#ifdef CONFIG_HS6621
                    // Set a flag to keep on div32k
                    register_set1(&HS_PMU->MISC_CTRL_1, PMU_MISC_CRY32M_KEEP_ON_MASK);
#else
                    co_assert(0);
#endif
                    break;
            }
            // Set a flag to keep on 32K
            register_set0(&HS_PMU->BASIC, PMU_BASIC_SLEEP_WO_32K_MASK);
            HS_PMU_UPD_RDY();
            break;

        case PMU_SHUTDOWN:
            register_set1(&HS_PMU->ANA_PD, PMU_ANA_REG_POWER_DOWN_MASK);
            // Never come here
            // NOTE: If come here, please check the "pwron_en" pin which should be connected with Vbat.
            //       If the "pwron_en" pin connecte with the GND, the chip will never shutdown.
            while(1);
    }

#ifdef PMU_PIN_STATE_SLEEP_ENABLE
    // Only wakeup pin as INPUT
    // NOTE: the not IE_CTRL controling IO will force to LOW level
    pmu_pin_input_enable(HS_PMU->GPIO_WAKEUP | encoder_pin_mask());
#ifndef CONFIG_GPIO1_ABSENT
    pmu_pin_input_enable_ex(HS_PMU->GPIO_WAKEUP_1 & PMU_GPIO_WAKEUP_39_32_MASK);
#endif
#endif

    // Wait wake IO clear ok
    pmu_wakeup_pin_wait_idle();

    // clock lowpoer enter
    pmu_clock_lowpower_enter(true, state);

    // dcdc lowpower enter (must do after cpu switch to RC32M)
    pmu_dcdc_lowpower_enter(true);
}

/**
 * @brief pmu leave lowpower status, call by system
 *
 * @note The time between wake trigger (sleep timer or gpio) and CPU running is 450us
 *       pmu_basic[3:0] ctrl ldo startup time
 *
 * @param[in] state  lowpower state
 *
 * @return None
 **/
void pmu_lowpower_leave(pmu_lowpower_state_t state)
{
#ifdef PMU_DETECT_LOWPOWER_TIME
    pmu_detect_lowpower_time_pin_toggle();
#endif

    // dcdc lowpower leave
    pmu_dcdc_lowpower_enter(false);

#ifdef PMU_DETECT_LOWPOWER_TIME
    pmu_detect_lowpower_time_pin_toggle();
#endif

    // clock lowpoer leave
    pmu_clock_lowpower_enter(false, state);

#ifdef PMU_DETECT_LOWPOWER_TIME
    pmu_detect_lowpower_time_pin_toggle();
#endif

#ifdef PMU_PIN_STATE_SLEEP_ENABLE
    // Enable all pin can INPUT
    pmu_pin_input_enable(0xFFFFFFFF);
#ifndef CONFIG_GPIO1_ABSENT
    pmu_pin_input_enable_ex(0xFF);
#endif
#endif

    switch(state)
    {
        case PMU_SLEEP:
        case PMU_SLEEP_HXTAL_OFF:
            pmu_env.pin_wakeup_sleep_recently = true;
            break;

        case PMU_DEEP_SLEEP:
            pmu_env.pin_wakeup_deep_sleep_recently = true;
            if(!pmu_env.enable_32k_with_deep_sleep)
            {
                switch(pmu_env.clock_32k)
                {
                    case PMU_32K_SEL_RC:
                        break;

                    case PMU_32K_SEL_32768HZ_XTAL:
                    case PMU_32K_SEL_DIV:
                        pmu_select_32k(pmu_env.clock_32k);
                        break;
                }
            }
            break;

        default:
            break;
    }

    pmu_env.ana_mask = 0;

    pmu_nvic_restore();
}

/**
 * @brief pmu pin state store, call by system
 *
 * @return None
 **/
void pmu_pin_state_store(void)
{
#ifdef PMU_PIN_STATE_SLEEP_ENABLE
#ifdef PMU_DETECT_LOWPOWER_TIME
    if(pmu_env.detect_lowpower_time_pin)
    {
        // make sure all gpio is stable. FIXME: better idea
        co_delay_10us(1);

        uint32_t saved_output;
        uint32_t pin_output_mask = ~HS_PMU->GPIO_WAKEUP;
        // Get ouput data level
        saved_output = gpio_read(pin_output_mask);
        // direction: OE#
        HS_PMU->GPIO_OE_CTRL = ~pin_output_mask;
        // output level
        HS_PMU->GPIO_ODA_CTRL = saved_output;

#ifndef CONFIG_GPIO1_ABSENT
        uint32_t saved_output_ex;
        uint32_t pin_output_mask_ex = ~HS_PMU->GPIO_WAKEUP_1 & 0xFF;
        saved_output_ex = gpio_read_ex(pin_output_mask_ex);
        HS_PMU->GPIO_OE_CTRL_1 = ~pin_output_mask_ex & 0xFF;
        HS_PMU->GPIO_ODA_CTRL_1 = saved_output_ex;
#endif

        // Pin control through PMU
        REGW1(&HS_PMU->GPIO_OE_CTRL_1, PMU_GPIO_OEB_SEL_MASK);
    }
    else
#endif

    // Pin control through PMU
    REGW1(&HS_PMU->MISC_CTRL, PMU_MISC_GPIO_AUTO_LATCH_CTRL_MASK);
#endif
}

/**
 * @brief pmu pin state restore, call by system
 *
 * @return None
 **/
void pmu_pin_state_restore(void)
{
#ifdef PMU_PIN_STATE_SLEEP_ENABLE
#ifdef PMU_DETECT_LOWPOWER_TIME
    if(pmu_env.detect_lowpower_time_pin)
    {
        pmu_detect_lowpower_time_pin_toggle();
        // Pin control through PINMUX
        REGW0(&HS_PMU->GPIO_OE_CTRL_1, PMU_GPIO_OEB_SEL_MASK);
    }
    else
#endif

    // Pin control through PINMUX
    REGW0(&HS_PMU->MISC_CTRL, PMU_MISC_GPIO_AUTO_LATCH_CTRL_MASK);
#endif
}

/**
 * @brief Set pin mode
 *
 * @param[in] pin_mask  pin mask
 * @param[in] mode  pin mode
 *
 * @return None
 **/
void pmu_pin_mode_set(uint32_t pin_mask, pmu_pin_mode_t mode)
{
#if defined(CONFIG_HS6621) && (CONFIG_HARDWARE_VERSION>1)
    // gpio[39:14] low active, gpio[13:0] high active
    uint32_t raw  = pin_mask & 0x00003FFF;
    uint32_t swap = pin_mask & 0xFFFFC000;
#endif

    switch(mode)
    {
        case PMU_PIN_MODE_FLOAT:
        case PMU_PIN_MODE_PP:
            HS_PMU->GPIO_ODE_CTRL &= ~pin_mask;
#if defined(CONFIG_HS6621) && (CONFIG_HARDWARE_VERSION>1)
            HS_PMU->GPIO_PU_CTRL  &= ~raw;
            HS_PMU->GPIO_PU_CTRL  |= swap;
#else
            HS_PMU->GPIO_PU_CTRL  &= ~pin_mask;
#endif
            HS_PMU->GPIO_PD_CTRL  &= ~pin_mask;
            break;

        case PMU_PIN_MODE_PD:
            HS_PMU->GPIO_ODE_CTRL &= ~pin_mask;
#if defined(CONFIG_HS6621) && (CONFIG_HARDWARE_VERSION>1)
            HS_PMU->GPIO_PU_CTRL  &= ~raw;
            HS_PMU->GPIO_PU_CTRL  |= swap;
#else
            HS_PMU->GPIO_PU_CTRL  &= ~pin_mask;
#endif
            HS_PMU->GPIO_PD_CTRL  |=  pin_mask;
            break;

        case PMU_PIN_MODE_PU:
            HS_PMU->GPIO_ODE_CTRL &= ~pin_mask;
#if defined(CONFIG_HS6621) && (CONFIG_HARDWARE_VERSION>1)
            HS_PMU->GPIO_PU_CTRL  |= raw;
            HS_PMU->GPIO_PU_CTRL  &= ~swap;
#else
            HS_PMU->GPIO_PU_CTRL  |=  pin_mask;
#endif
            HS_PMU->GPIO_PD_CTRL  &= ~pin_mask;
            break;

        case PMU_PIN_MODE_OD:
            HS_PMU->GPIO_ODE_CTRL |=  pin_mask;
#if defined(CONFIG_HS6621) && (CONFIG_HARDWARE_VERSION>1)
            HS_PMU->GPIO_PU_CTRL  &= ~raw;
            HS_PMU->GPIO_PU_CTRL  |= swap;
#else
            HS_PMU->GPIO_PU_CTRL  &= ~pin_mask;
#endif
            HS_PMU->GPIO_PD_CTRL  &= ~pin_mask;
            break;

        case PMU_PIN_MODE_OD_PU:
            HS_PMU->GPIO_ODE_CTRL |=  pin_mask;
#if defined(CONFIG_HS6621) && (CONFIG_HARDWARE_VERSION>1)
            HS_PMU->GPIO_PU_CTRL  |= raw;
            HS_PMU->GPIO_PU_CTRL  &= ~swap;
#else
            HS_PMU->GPIO_PU_CTRL  |=  pin_mask;
#endif
            HS_PMU->GPIO_PD_CTRL  &= ~pin_mask;
            break;

        default:
            break;
    }
}

#ifndef CONFIG_GPIO1_ABSENT
/**
 * @brief Set pin mode
 *
 * @param[in] pin_mask  pin mask
 * @param[in] mode  pin mode
 *
 * @return None
 **/
void pmu_pin_mode_set_ex(uint32_t pin_mask, pmu_pin_mode_t mode)
{
    pin_mask &= PMU_GPIO_ODE_REG_39_32_MASK;

    switch(mode)
    {
        case PMU_PIN_MODE_FLOAT:
        case PMU_PIN_MODE_PP:
            HS_PMU->GPIO_ODE_CTRL_1 &= ~pin_mask | PMU_GPIO_PMU_DBG_MASK;
#if defined(CONFIG_HS6621) && (CONFIG_HARDWARE_VERSION>1)
            HS_PMU->GPIO_PU_CTRL_1  |=  pin_mask;
#else
            HS_PMU->GPIO_PU_CTRL_1  &= ~pin_mask;
#endif
            HS_PMU->GPIO_PD_CTRL_1  &= ~pin_mask;
            break;

        case PMU_PIN_MODE_PD:
            HS_PMU->GPIO_ODE_CTRL_1 &= ~pin_mask | PMU_GPIO_PMU_DBG_MASK;
#if defined(CONFIG_HS6621) && (CONFIG_HARDWARE_VERSION>1)
            HS_PMU->GPIO_PU_CTRL_1  |=  pin_mask;
#else
            HS_PMU->GPIO_PU_CTRL_1  &= ~pin_mask;
#endif
            HS_PMU->GPIO_PD_CTRL_1  |=  pin_mask;
            break;

        case PMU_PIN_MODE_PU:
            HS_PMU->GPIO_ODE_CTRL_1 &= ~pin_mask | PMU_GPIO_PMU_DBG_MASK;
#if defined(CONFIG_HS6621) && (CONFIG_HARDWARE_VERSION>1)
            HS_PMU->GPIO_PU_CTRL_1  &= ~pin_mask;
#else
            HS_PMU->GPIO_PU_CTRL_1  |=  pin_mask;
#endif
            HS_PMU->GPIO_PD_CTRL_1  &= ~pin_mask;
            break;

        case PMU_PIN_MODE_OD:
            HS_PMU->GPIO_ODE_CTRL_1 |=  pin_mask & (~PMU_GPIO_PMU_DBG_MASK);
#if defined(CONFIG_HS6621) && (CONFIG_HARDWARE_VERSION>1)
            HS_PMU->GPIO_PU_CTRL_1  |=  pin_mask;
#else
            HS_PMU->GPIO_PU_CTRL_1  &= ~pin_mask;
#endif
            HS_PMU->GPIO_PD_CTRL_1  &= ~pin_mask;
            break;

        case PMU_PIN_MODE_OD_PU:
            HS_PMU->GPIO_ODE_CTRL_1 |=  pin_mask & (~PMU_GPIO_PMU_DBG_MASK);
#if defined(CONFIG_HS6621) && (CONFIG_HARDWARE_VERSION>1)
            HS_PMU->GPIO_PU_CTRL_1  &= ~pin_mask;
#else
            HS_PMU->GPIO_PU_CTRL_1  |=  pin_mask;
#endif
            HS_PMU->GPIO_PD_CTRL_1  &= ~pin_mask;
            break;

        default:
            break;
    }
}
#endif

/**
 * @brief Set gpio driven current
 *
 * @param[in] pin_mask  pin mask
 * @param[in] driven  current driven (Large driven current should be push-pull output)
 *
 * @return None
 **/
void pmu_pin_driven_current_set(uint32_t pin_mask, pmu_pin_driven_current_t driven)
{
    if(driven & 0x01)
        HS_PMU->GPIO_DRV_CTRL_0 |= pin_mask;
    else
        HS_PMU->GPIO_DRV_CTRL_0 &= ~pin_mask;

    if(driven & 0x02)
        HS_PMU->GPIO_DRV_CTRL_2 |= pin_mask;
    else
        HS_PMU->GPIO_DRV_CTRL_2 &= ~pin_mask;
}

#ifndef CONFIG_GPIO1_ABSENT
/**
 * @brief Set gpio driven current
 *
 * @param[in] pin_mask  pin mask
 * @param[in] driven  current driven (Large driven current should be push-pull output)
 *
 * @return None
 **/
void pmu_pin_driven_current_set_ex(uint32_t pin_mask, pmu_pin_driven_current_t driven)
{
    if(driven & 0x01)
        HS_PMU->GPIO_DRV_CTRL_1 |= pin_mask;
    else
        HS_PMU->GPIO_DRV_CTRL_1 &= ~pin_mask;

    if(driven & 0x02)
        HS_PMU->GPIO_DRV_CTRL_3 |= pin_mask;
    else
        HS_PMU->GPIO_DRV_CTRL_3 &= ~pin_mask;
}
#endif

/**
 * @brief pmu analog power enable, call by system
 *
 * @param[in] enable  enable/disable
 * @param[in] ana  analog type
 *
 * @return None
 **/
void pmu_ana_enable(bool enable, pmu_ana_type_t ana)
{
    CO_DISABLE_IRQ();

    if(enable)
    {
        if ((pmu_env.ana_mask & ana) == 0)
        {
#if defined(CONFIG_HS6621) && (CONFIG_HARDWARE_VERSION>3)
            if(pmu_env.ana_mask == 0)
            {
                if(ana == PMU_ANA_RF)
                    cpm_ana_clock_set(CPM_ANA_CLK_IGNORE, CPM_ANA_CLK_ON);
                else
                    cpm_ana_clock_enable();
            }
            else if (pmu_env.ana_mask == PMU_ANA_RF)
            {
                if(ana != PMU_ANA_RF)
                    cpm_ana_clock_enable();
            }
#else
            // Digital BUG: RF 2M can't close ANA_AHB clock
            if(pmu_env.ana_mask == 0)
                cpm_ana_clock_enable();
#endif

            switch(ana)
            {
                case PMU_ANA_RF:
                    CPM_ANA_CLK_ENABLE();
                    REGW1(&HS_DAIF->CLK_ENS, DAIF_PLL_VTRACK_CLK_EN_MASK | DAIF_PLL_LUT_CLK_EN_MASK |
                            DAIF_MAIN_FSM_CLK_EN_MASK | DAIF_RX_AGC_CLK_EN_MASK | DAIF_DCOC_LUT_CLK_EN_MASK |
                            DAIF_SDM_CLK_EN_MASK | DAIF_PLL_CLK_REF_EN_MASK);
                    REGW1(&HS_DAIF->VCO_CTRL0, DAIF_VTRACK_EN_MASK);
#ifndef CONFIG_HS6621
                    REGW1(&HS_DAIF->CLK_CFG, DAIF_EN_CKO16M_XTAL_DIG_MASK|DAIF_EN_CKO16M_XTAL_ANA_MASK|DAIF_EN_CKO16M_XTAL_PLL_MASK);
#endif
                    CPM_ANA_CLK_RESTORE();
                    break;

                case PMU_ANA_ADC:
                    REGW1(&HS_DAIF->CLK_ENS, DAIF_ADC_CLK_EN_MASK);
#ifndef CONFIG_HS6621
                    REGW1(&HS_DAIF->CLK_CFG, DAIF_EN_CKO16M_XTAL_GPADC_MASK);
#endif

                    break;

                default:
                    break;
            }

            pmu_env.ana_mask |= ana;
        }
    }
    else
    {
        if (pmu_env.ana_mask & ana)
        {
            pmu_env.ana_mask &= ~ana;

            switch(ana)
            {
                case PMU_ANA_RF:
                    CPM_ANA_CLK_ENABLE();
                    REGW0(&HS_DAIF->CLK_ENS, DAIF_PLL_VTRACK_CLK_EN_MASK | DAIF_PLL_LUT_CLK_EN_MASK |
                            DAIF_MAIN_FSM_CLK_EN_MASK | DAIF_RX_AGC_CLK_EN_MASK | DAIF_DCOC_LUT_CLK_EN_MASK |
                            DAIF_SDM_CLK_EN_MASK | DAIF_PLL_CLK_REF_EN_MASK);
                    REGW0(&HS_DAIF->VCO_CTRL0, DAIF_VTRACK_EN_MASK);
#ifndef CONFIG_HS6621
                    REGW0(&HS_DAIF->CLK_CFG, DAIF_EN_CKO16M_XTAL_DIG_MASK|DAIF_EN_CKO16M_XTAL_ANA_MASK|DAIF_EN_CKO16M_XTAL_PLL_MASK);
#endif
                    CPM_ANA_CLK_RESTORE();
                    break;

                case PMU_ANA_ADC:
                    REGW0(&HS_DAIF->CLK_ENS, DAIF_ADC_CLK_EN_MASK);
#ifndef CONFIG_HS6621
                    REGW0(&HS_DAIF->CLK_CFG, DAIF_EN_CKO16M_XTAL_GPADC_MASK);
#endif
                    break;

                default:
                    break;
            }

            if(pmu_env.ana_mask == 0)
                cpm_ana_clock_disable();
        }
    }

    CO_RESTORE_IRQ();
}

/**
 * @brief analog is enabled
 *
 * @param[in] ana  analog module
 *
 * @return enabled?
 **/
bool pmu_ana_is_enabled(pmu_ana_type_t ana)
{
    return (pmu_env.ana_mask & ana) ? true : false;
}

/**
 * @brief reset cpu to ROM
 *
 * @return None
 **/
void pmu_reset_to_rom(void)
{
    __disable_irq();
    pmu_memery_remap(PMU_REMAP_FROM_ROM);
    pmu_cpu_reset();
    while(1);
}

/**
 * @brief reset cpu to RAM
 *
 * @return None
 **/
void pmu_reset_to_ram(void)
{
    __disable_irq();
    pmu_memery_remap(PMU_REMAP_FROM_RAM);
    pmu_cpu_reset();
    while(1);
}

/**
 * @brief reset cpu to RAM
 *
 * @return None
 **/
void pmu_reset_to_sflash(void)
{
    __disable_irq();
    pmu_memery_remap(PMU_REMAP_FROM_SFLASH);
    pmu_cpu_reset();
    while(1);
}

/**
 * @brief Force into reboot sleep mode
 *
 * Power consumption is lower than the deep sleep mode. All SRAM will be powered down.
 * But chip will be reboot from ROM when wakeup.
 *
 * @return None
 **/
void pmu_force_into_reboot_sleep_mode(uint32_t ram_retention, bool use_hib_mode)
{
    // Reboot Prepare
    pmu_reboot_prepare();

    // Set flag
    HS_PMU->SW_STATUS |= PMU_SW_STATUS_REBOOT_FROM_SLEEP_MASK | PMU_SW_STATUS_REBOOT_FROM_SOFT_RESET_MASK;

    // Make sure the isp bit is cleared
    HS_PMU->BOOT_SEL &= ~1u; // @ref BOOT_SEL_ISP_BIT

    /* !!! Please Make Sure Belowe has no stack used !!! */

    // Remap to ROM
    pmu_memery_remap(PMU_REMAP_FROM_ROM);

    // Power off specified RAM
    HS_PMU->RAM_PM_1 = ram_retention;

#ifndef CONFIG_HS6621
    if (use_hib_mode)
    {
        // clear hib gpio latch
        HREGW0(&HS_HIB->CONFIG_1, HIB_CONFIG_GPIO_LATCH_ANA_CLR_MASK);

        // hibernation mode make sure GPIO4(BOOT) IE is enabled
        HS_PMU->GPIO_IE_CTRL |= (1u<<4) | hib_wakeup_pin_get();

        // hibernation mode
        REGW1(&HS_PMU->BASIC, PMU_BASIC_HIB_EN_MASK);
        HS_PMU_UPD_RDY();
    }
#endif

    // Good Night!
    SCB->SCR |= SCB_SCR_SLEEPDEEP;
    __WFI();

    // refix bug
    pmu_wait_system_ready();

    // Must be some IRQ pending, Force reboot
    pmu_cpu_reset();

    // Never come here
    while(1);
}

/**
 * @brief pmu_pin_wakeup_out_of_date()
 *
 * @note:
 * PIN_WAKEUP_IRQ will be generated not only from pin-wakeup but also each GPIO irq(not sleep)
 * So do this to let PIN_WAKEUP_IRQ only generate from pin-wakeup
 *
 * @note: PIN_WAKEUP_IRQn must less then GPIO0_IRQn/GPIO1_IRQn
 *
 * @return None
 **/
void pmu_pin_wakeup_out_of_date(void)
{
    pmu_env.pin_wakeup_sleep_recently = false;
    pmu_env.pin_wakeup_deep_sleep_recently = false;
}

/**
 * @brief pmu_wait_system_ready()
 *
 * @return 
 **/
void pmu_wait_system_ready(void)
{
    // Check PMU_STATE (PMU_BASIC[31:27]) equal to 7
    while(register_get(&HS_PMU->BASIC, MASK_POS(PMU_BASIC_STATE)) != 7);
}

/**
 * @brief pmu_timer_init()
 *
 * @return 
 **/
void pmu_timer_init(void)
{
    register_set(&HS_PMU->BASIC, MASK_3REG(PMU_BASIC_TIMER_CLK_EN, 1,
                                           PMU_BASIC_TIMER_INT_MASK, 0,
                                           PMU_BASIC_TIMER_INT_WAKE_EN, 1));
    HS_PMU_UPD_RDY();

    register_set(&HS_PMU->MISC_CTRL_1, MASK_3REG(PMU_MISC_TIMER_INT_CPU_EN, 0,
                                                 PMU_MISC_TIMER_EN, 1,
                                                 PMU_MISC_TIMER_INT_CLR, 1));
}

/**
 * @brief pmu_timer_time()
 *
 * @return 
 **/
uint32_t pmu_timer_time(void)
{
    return HS_PMU->TIMER_CNT;
}

/**
 * @brief pmu_timer_start()
 *
 * @param[in] time  
 *
 * @return 
 **/
void pmu_timer_start(uint32_t time)
{
    CO_DISABLE_IRQ();
    HS_PMU->TIMER_VAL = time;
    // Enable IRQ
    register_set1(&HS_PMU->MISC_CTRL_1, PMU_MISC_TIMER_INT_CPU_EN_MASK);
    CO_RESTORE_IRQ();
}

/**
 * @brief pmu_timer_stop()
 *
 * @return 
 **/
void pmu_timer_stop(void)
{
    CO_DISABLE_IRQ();
    // Disable IRQ
    register_set0(&HS_PMU->MISC_CTRL_1, PMU_MISC_TIMER_INT_CPU_EN_MASK);
    CO_RESTORE_IRQ();
}

/**
 * @brief pmu_timer_register()
 *
 * @param[in] cb  
 *
 * @return 
 **/
void pmu_timer_register(pmu_timer_overflow_callback_t cb)
{
    pmu_env.timer_overflow_callback = cb;

    // Enable IRQ
    NVIC_ClearPendingIRQ(PMU_TIMER_IRQn);
    NVIC_SetPriority(PMU_TIMER_IRQn, IRQ_PRIORITY_LOW);
    NVIC_EnableIRQ(PMU_TIMER_IRQn);
}

/**
 * @brief pmu_random_seed_fetch()
 *
 * @return HS_RANDOM->RANDOM
 **/
uint32_t pmu_random_seed_fetch(uint32_t pre_random)
{
    int i;
    pmu_32k_sel_t sel32k = pmu_select_32k_get_reg();

    // Random seed init, it use 32k rc, need:32*30us
    if (sel32k != PMU_32K_SEL_RC)
        pmu_32k_switch_to_rc(false /*calib*/, false /*pd others*/);

    // Open Random and try to use 32k rc to genete TRUE random seed
    REGW0(&HS_PSO->RNG_CFG, CPM_RNG_GATE_EN_MASK);
    HS_PSO_32K_UPD_RDY();

    // try (max 20ms fetch)
    for(i=0; i<200; ++i)
    {
        // 32bit random fetch time: 33us * 32bit = 1056ms
        co_delay_10us(10);
        if(pmu_random_seed_is_valid(HS_RANDOM->RANDOM) && (pre_random!=HS_RANDOM->RANDOM))
            break;
    }

    // Gate RANDOM to hold it as random seed
    REGW1(&HS_PSO->RNG_CFG, CPM_RNG_GATE_EN_MASK);
    HS_PSO_32K_UPD_RDY();

    // restore 32k
    if (sel32k != PMU_32K_SEL_RC)
        pmu_select_32k(sel32k);

    return HS_RANDOM->RANDOM;
}

/**
 * @brief PIN_WAKEUP_IRQHandler()
 *
 * @return 
 **/
void PIN_WAKEUP_IRQHandler(void)
{
    uint32_t int_status = HS_PMU->GPIO_LATCH;
#ifndef CONFIG_GPIO1_ABSENT
    uint32_t int_status_1 = HS_PMU->GPIO_LATCH_1;
#endif

    // clear interrupt
    register_set1(&HS_PMU->MISC_CTRL, PMU_MISC_CLR_PMU_INT_MASK);

    if(pmu_env.pin_wakeup_sleep_recently || pmu_env.pin_wakeup_deep_sleep_recently)
    {
        // clear gpio irq pending
        gpio_clear_interrupt_pending(int_status);
#ifndef CONFIG_GPIO1_ABSENT
        gpio_clear_interrupt_pending_ex(int_status);
#endif
        // Sleep wakeup: may 2ms delay
        if(pmu_env.pin_wakeup_sleep_recently)
        {
            // Callback
            if(pmu_env.pin_wakeup_sleep_callback && int_status)
                pmu_env.pin_wakeup_sleep_callback(int_status);
#ifndef CONFIG_GPIO1_ABSENT
            if(pmu_env.pin_wakeup_sleep_callback_ex && int_status_1)
                pmu_env.pin_wakeup_sleep_callback_ex(int_status_1);
#endif
        }

        // Deep sleep wakeup: xtal32k startup is very very slow (0.5s-2s), so the deepsleep pin wakeup may not debounce
        else // if(pmu_env.pin_wakeup_deep_sleep_recently)
        {
            // Callback
            if(pmu_env.pin_wakeup_deep_sleep_callback && int_status)
                pmu_env.pin_wakeup_deep_sleep_callback(int_status);
#ifndef CONFIG_GPIO1_ABSENT
            if(pmu_env.pin_wakeup_deep_sleep_callback_ex && int_status_1)
                pmu_env.pin_wakeup_deep_sleep_callback_ex(int_status_1);
#endif
        }

        // wakeup out of data
        pmu_pin_wakeup_out_of_date();
    }
}

/**
 * @brief PMU_TIMER_IRQHandler()
 *
 * @return 
 **/
void PMU_TIMER_IRQHandler(void)
{
    register_set1(&HS_PMU->MISC_CTRL_1, PMU_MISC_TIMER_INT_CLR_MASK);
    while(HS_PMU->STATUS_READ & PMU_TIMER_INT_MASK);

    if(pmu_env.timer_overflow_callback)
        pmu_env.timer_overflow_callback();
}

#endif

//}BASE_COMPONENT_END

/*********************************************************************
 * NOT Base Component
 */

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

#ifdef CONFIG_HS6621
/**
 * @brief  pmu pll startup
 *
 * @param[in] mhz  144MHz, 128MHz, 112MHz, 96MHz, 80MHz
 **/
void pmu_pll_startup(uint32_t mhz)
{
    CPM_ANA_AHB_CLK_ENABLE();

    // check
    if(!pmu_topclk_pll_is_enabled())
    {
        // optimize
        cpm_optimize_clock(mhz, 0);

        // xtal32m startup
        pmu_xtal32m_fast_startup();

        // power on rc32m and switch to it
        pmu_topclk_rc32m_power_enable(true);
        pmu_topclk_switch_to_rc32m();

        // power on xtal32m and pll, switch to it
        pmu_topclk_pll_power_enable(true, mhz);
        pmu_topclk_switch_to_pll();

        // power off rc32m
        pmu_topclk_rc32m_power_enable(false);
    }

    CPM_ANA_CLK_RESTORE();
}

/**
 * @brief  pmu pll close
 **/
void pmu_pll_close(void)
{
    CPM_ANA_AHB_CLK_ENABLE();

    // power off pll
    if(pmu_topclk_pll_is_enabled())
    {
        // to xtal64m
        pmu_topclk_switch_to_xtal32m_x2();

        // pll power off
        pmu_topclk_pll_power_enable(false, 0/*freq*/);
    }

    CPM_ANA_CLK_RESTORE();
}
#endif

/**
 * @brief pmu ram power on
 *
 * @param[in] blocks  RAM black mask.
 *
 * ulp [0:15]   clock=8kB*16 power=32kB*4(0x000F@1st,0x00F0@2nd,0x0F00@3rd,0xF000@4th)
 * lp  [16:31]  clock=8kB*16 power=8kB*16
 *
 * @return None
 **/
void pmu_ram_power_on(uint32_t blocks)
{
    uint32_t ulp = blocks & 0xFFFF;
    uint32_t lp = (blocks>>16) & 0xFFFF;
    uint32_t pm_mask = lp << 4;
    int i;

    for (i=0; i<4; ++i)
    {
        if ((ulp >> (i*4)) & 0xF)
            pm_mask |= (1<<i);
    }

    // RAM clock on. Default: only 1st and last block have power
    // NOTE: Also do this in reset_handler for user code. reset_handler must be loacated in 1st block
    HS_PSO->RAM_CFG = ~blocks;
    HS_PSO_UPD_RDY();

#ifdef CONFIG_HS6621
    // RAM power on. Default: Only 1st block have power
    HS_PMU->RAM_PM_1 = pm_mask;

    // Wait all ram power on
    WAIT_TIMEOUT(HS_PMU->RAM_PM_2==pm_mask, TIMEOUT_RAM_POWERON);

    // Check crash
    if(HS_PMU->RAM_PM_2 != pm_mask)
        co_fault_set(CO_FAULT_SRAM_CRASH);
#else
    // power
    REGW(&HS_PMU->RAM_PM_1, MASK_1REG(PMU_PM_RAM_POWER_ON, pm_mask));
#endif
}

#ifndef CONFIG_BOOTLOADER
/**
 * @brief Power off invalid SRAM block
 *
 * @return None
 **/
void pmu_ram_power_off_invalid_block(void)
{
    const uint32_t block_size = 8 * 1024;
    uint32_t sram_mask;

#if defined(__GNUC__) // GCC

    (void)block_size;
    (void)sram_mask;
    return;

#elif defined(__CC_ARM) // Keil

    extern unsigned char Image$$APP$$ZI$$Limit;
    uint32_t app_limit = ((uint32_t)&Image$$APP$$ZI$$Limit) & 0x000FFFFF;
    uint32_t sram_num = app_limit / block_size;

    if(app_limit % block_size)
        ++sram_num;

    sram_mask = ((1<<sram_num)-1);

#ifdef CONFIG_HS6621
    uint32_t ulp, lp;
    ulp = (sram_mask >> 0)  & 0x7FFF;
    lp  = (sram_mask >> 15) & 0xFFFF;
    sram_mask = ulp | 0x8000 | (lp << 16);
#else
    sram_mask = sram_mask | 0x8000;
#endif

#elif defined(__ICCARM__) // IAR
#errer ""
#endif

    pmu_ram_power_on(sram_mask);
}
#endif

/**
 * @brief xtal32k enable in deep sleep
 *
 * @note
 *  The xtal 32768Hz crystal startup is very very slow (0.5s~2s),
 *  Enable it in deepsleep, can make the wakeup faster
 *
 * @param[in] enable  enable or disable
 *
 * @return None
 **/
void pmu_32k_enable_in_deep_sleep(bool enable)
{
    pmu_env.enable_32k_with_deep_sleep = enable;
}

/**
 * @brief Change xtal 32k params
 *
 * @param[in] load_capacitance  load capacitance, range:0~15 default:8 step:0.84pF, max:12.6pF
 * @param[in] drive_current  drive current, range:0-3, default:1
 *
 * @note load_capacitance will effect xtal 32k startup time and precision,
 *       appropriate value can speed up startup time.
 *
 * @return None
 **/
void pmu_xtal32k_change_param(int load_capacitance, int drive_current)
{
#ifdef CONFIG_HS6621
    if(load_capacitance >= 0)
        register_set(&HS_PMU->CLK_CTRL_2, MASK_1REG(PMU_XTAL32K_CTUNE, load_capacitance));

    if(drive_current >= 0)
        register_set(&HS_PMU->CLK_CTRL_2, MASK_1REG(PMU_XTAL32K_SEL_I, drive_current));
#else
    if(load_capacitance >= 0)
        HREGW(&HS_HIB->CONFIG_1, MASK_1REG(HIB_CONFIG_CTUNE_RC32K, load_capacitance));

    if(drive_current >= 0)
        HREGW(&HS_HIB->CONFIG_1, MASK_1REG(HIB_CONFIG_SEL_I_XTAL32K, drive_current));
#endif
}

/**
 * @brief Change xtal 32m params
 *
 * @param[in] load_capacitance  load capacitance, range:0~31, default:15, step:0.75pf, max:23.25pF
 *
 * @note load_capacitance will effect xtal 32m precision and frequency offset.
 *
 * @return None
 **/
void pmu_xtal32m_change_param(int load_capacitance)
{
    if(load_capacitance >= 0)
    {
        int cur = REGR(&HS_PMU->CLK_CTRL_2, MASK_POS(PMU_REG_CTUNE_XTAL));

        if (cur > load_capacitance)
        {
            while(cur-- > load_capacitance)
            {
                REGW(&HS_PMU->CLK_CTRL_2, MASK_1REG(PMU_REG_CTUNE_XTAL, cur));
                co_delay_10us(1);
            }
        }
        else if (cur < load_capacitance)
        {
            while(cur++ < load_capacitance)
            {
                REGW(&HS_PMU->CLK_CTRL_2, MASK_1REG(PMU_REG_CTUNE_XTAL, cur));
                co_delay_10us(1);
            }
        }

        co_assert(REGR(&HS_PMU->CLK_CTRL_2, MASK_POS(PMU_REG_CTUNE_XTAL)) == load_capacitance);
    }
}

/**
 * @brief pmu enable INSIDE dcdc
 *
 * @param[in] enable  enable/disable
 *
 * @return None
 **/
void pmu_dcdc_enable(bool enable)
{
    if(enable)
    {
#if defined(CONFIG_HS6621) && (CONFIG_HARDWARE_VERSION>1)
        REGW(&HS_PMU->ANA_PD, MASK_1REG(PMU_ANA_BUCK_EN, 1));
#else
        REGW(&HS_PMU->ANA_PD, MASK_1REG(PMU_ANA_BUCK_DIS, 0));
#endif
        co_delay_10us(20);
        REGW(&HS_PMU->ANA_PD, MASK_1REG(PMU_ANA_LDO_1P5_DIS, 1));
        REGW1(&HS_PMU->SW_STATUS, PMU_SW_STATUS_DCDC_ENABLED_MASK);
    }
    else
    {
        REGW(&HS_PMU->ANA_PD, MASK_1REG(PMU_ANA_LDO_1P5_DIS, 0));
        co_delay_10us(20);
#if defined(CONFIG_HS6621) && (CONFIG_HARDWARE_VERSION>1)
        REGW(&HS_PMU->ANA_PD, MASK_1REG(PMU_ANA_BUCK_EN, 0));
#else
        REGW(&HS_PMU->ANA_PD, MASK_1REG(PMU_ANA_BUCK_DIS, 1));
#endif
        REGW0(&HS_PMU->SW_STATUS, PMU_SW_STATUS_DCDC_ENABLED_MASK);
    }
}

/**
 * @brief Enable external dcdc
 *
 * @note If use this function, must be called after gpio_init
 *
 * @param[in] en_pin  
 * @param[in] on_level  
 * @param[in] startup_us  
 *
 * @return None
 **/
void pmu_external_dcdc_enable(uint8_t en_pin, gpio_level_t on_level, uint16_t startup_us)
{

}

/**
 * @brief pmu gpio wakeup pin setup
 *
 * @param[in] pin_mask  pin mask
 * @param[in] trigger_type  wakeup trigger type
 *
 * @return None
 **/
void pmu_wakeup_pin_set(uint32_t pin_mask, pmu_pin_wakeup_type_t trigger_type)
{
    CO_DISABLE_IRQ();

    switch(trigger_type)
    {
        case PMU_PIN_WAKEUP_DISABLE:
            register_set0(&HS_PMU->GPIO_WAKEUP, pin_mask);
            register_set0(&HS_PMU->GPIO_POL, pin_mask);
            break;

        case PMU_PIN_WAKEUP_LOW_LEVEL: // FALLING_EDGE
            register_set1(&HS_PMU->GPIO_WAKEUP, pin_mask);
            register_set1(&HS_PMU->GPIO_POL, pin_mask);
            break;

        case PMU_PIN_WAKEUP_HIGH_LEVEL: // RISING_EDGE
            register_set1(&HS_PMU->GPIO_WAKEUP, pin_mask);
            register_set0(&HS_PMU->GPIO_POL, pin_mask);
            break;
    }

    register_set(&HS_PMU->WAKE_DEB, PMU_PIN_WAKE_LEVEL_EDGE_SEL_MASK | PMU_PIN_DEBOUNCE_CYCLE_WAKE_MASK |
                                    PMU_PIN_DEBOUNCE_COEFF_WAKE_MASK | PMU_PIN_DEB_RST_MASK,
                                    PMU_PIN_WAKE_LEVEL_EDGE_SEL_1ST_MASK | PMU_PIN_DEB_RST_MASK);
    HS_PMU_UPD_RDY();

    // clear interrupt
    register_set1(&HS_PMU->MISC_CTRL, PMU_MISC_CLR_PMU_INT_MASK);
    pmu_wakeup_pin_wait_idle();

    CO_RESTORE_IRQ();

    if(trigger_type != PMU_PIN_WAKEUP_DISABLE)
    {
        // Enable IRQ
        NVIC_ClearPendingIRQ(PIN_WAKEUP_IRQn);
        NVIC_SetPriority(PIN_WAKEUP_IRQn, IRQ_PRIORITY_NORMAL);
        NVIC_EnableIRQ(PIN_WAKEUP_IRQn);
    }
}

#ifndef CONFIG_GPIO1_ABSENT
/**
 * @brief pmu gpio32-gpio39 wakeup pin setup
 *
 * @param[in] pin_mask  pin mask
 * @param[in] trigger_type  wakeup trigger type
 *
 * @return None
 **/
void pmu_wakeup_pin_set_ex(uint32_t pin_mask, pmu_pin_wakeup_type_t trigger_type)
{
    CO_DISABLE_IRQ();

    pin_mask &= PMU_GPIO_WAKEUP_39_32_MASK;

    switch(trigger_type)
    {
        case PMU_PIN_WAKEUP_DISABLE:
            register_set0(&HS_PMU->GPIO_WAKEUP_1, pin_mask);
            register_set0(&HS_PMU->GPIO_POL_1, pin_mask);
            break;

        case PMU_PIN_WAKEUP_LOW_LEVEL: // FALLING_EDGE
            register_set1(&HS_PMU->GPIO_WAKEUP_1, pin_mask);
            register_set1(&HS_PMU->GPIO_POL_1, pin_mask);
            break;

        case PMU_PIN_WAKEUP_HIGH_LEVEL: // RISING_EDGE
            register_set1(&HS_PMU->GPIO_WAKEUP_1, pin_mask);
            register_set0(&HS_PMU->GPIO_POL_1, pin_mask);
            break;
    }

    register_set(&HS_PMU->WAKE_DEB, PMU_PIN_WAKE_LEVEL_EDGE_SEL_MASK | PMU_PIN_DEBOUNCE_CYCLE_WAKE_MASK |
                                    PMU_PIN_DEBOUNCE_COEFF_WAKE_MASK | PMU_PIN_DEB_RST_MASK,
                                    PMU_PIN_WAKE_LEVEL_EDGE_SEL_1ST_MASK | PMU_PIN_DEB_RST_MASK);
    HS_PMU_UPD_RDY();

    // clear interrupt
    register_set1(&HS_PMU->MISC_CTRL, PMU_MISC_CLR_PMU_INT_MASK);
    pmu_wakeup_pin_wait_idle();

    CO_RESTORE_IRQ();

    if(trigger_type != PMU_PIN_WAKEUP_DISABLE)
    {
        // Enable IRQ
        NVIC_ClearPendingIRQ(PIN_WAKEUP_IRQn);
        NVIC_SetPriority(PIN_WAKEUP_IRQn, IRQ_PRIORITY_NORMAL);
        NVIC_EnableIRQ(PIN_WAKEUP_IRQn);
    }
}
#endif

/**
 * @brief pmu wakeup pin register callback
 *
 * @param[in] sleep_callback  callback
 * @param[in] deep_sleep_callback callback
 *
 * @note
 *  When deepsleep, xtal32k startup is very very slow (0.5s-2s),
 *  So the deepsleep pin wakeup irq may don't debounce
 *  @ref pmu_32k_enable_in_deep_sleep()
 *
 * @return None
 **/
void pmu_wakeup_pin_register_callback(gpio_callback_t sleep_callback, gpio_callback_t deep_sleep_callback)
{
    pmu_env.pin_wakeup_sleep_callback = sleep_callback;
    pmu_env.pin_wakeup_deep_sleep_callback = deep_sleep_callback;
}

#ifndef CONFIG_GPIO1_ABSENT
/**
 * @brief pmu wakeup pin(gpio32-gpio39) register callback
 *
 * @param[in] sleep_callback  callback
 * @param[in] deep_sleep_callback callback
 *
 * @note
 *  When deepsleep, xtal32k startup is very very slow (0.5s-2s),
 *  So the deepsleep pin wakeup irq may don't debounce
 *  @ref pmu_32k_enable_in_deep_sleep()
 *
 * @return None
 **/
void pmu_wakeup_pin_register_callback_ex(gpio_callback_t sleep_callback, gpio_callback_t deep_sleep_callback)
{
    pmu_env.pin_wakeup_sleep_callback_ex = sleep_callback;
    pmu_env.pin_wakeup_deep_sleep_callback_ex = deep_sleep_callback;
}
#endif

/**
 * @brief pmu enable shutdown pin
 *
 * when shutdown pin is low level (2s), it will trigger to shut-down or power-on.
 * when shutdown pin is high level, keep current status.
 *
 * @return None
 **/
void pmu_shutdown_pin_enable(void)
{
    // POWERDOWN debounce delay: (COEFF_PD<<CYCLE_PD)/32 ms
    register_set(&HS_PMU->WAKE_DEB, MASK_2REG(PMU_PIN_DEBOUNCE_COEFF_PD, 2, // 2s
                                              PMU_PIN_DEBOUNCE_CYCLE_PD, 15));
    register_set1(&HS_PMU->WAKE_DEB, PMU_PIN_DEB_RST_MASK);

    // GPIO pol and mask init, power down pin enable
    register_set1(&HS_PMU->GPIO_POL_1, PMU_GPIO_POL_POWERDOWN_MASK);
    register_set1(&HS_PMU->GPIO_WAKEUP_1, PMU_GPIO_WAKEUP_POWERDOWN_MASK);

    // clear interrupt
    register_set1(&HS_PMU->MISC_CTRL, PMU_MISC_CLR_PMU_INT_MASK);
    pmu_wakeup_pin_wait_idle();

    // enable
    register_set0(&HS_PMU->BASIC, PMU_BASIC_POWER_DOWN_DISABLE_MASK);
    HS_PMU_UPD_RDY();

    // Power down IRQ enable
    NVIC_ClearPendingIRQ(POWER_DOWN_IRQn);
    NVIC_SetPriority(POWER_DOWN_IRQn, IRQ_PRIORITY_HIGH);
    NVIC_EnableIRQ(POWER_DOWN_IRQn);
}

/**
 * @brief Force system to reboot
 *
 * @return None
 **/
void pmu_force_reboot(void)
{
    // Reboot Prepare
    pmu_reboot_prepare();

#ifdef CONFIG_HS6621
    // close pll
    pmu_pll_close();

    // Must disable and invalid cache in HS6621
    sfs_cache_enable(false);
    sfs_cache_invalidate_all();
#endif

    // Set flag
    HS_PMU->SW_STATUS |= PMU_SW_STATUS_REBOOT_FORCE_MASK;

    // Make sure the isp bit is cleared
    HS_PMU->BOOT_SEL &= ~1u; // @ref BOOT_SEL_ISP_BIT

    // Remap and Reset
    pmu_reset_to_rom();

    // Never come here
    while(1);
}

#ifdef CONFIG_HS6621
/**
 * @brief Force chip into OTA mode
 *
 * @return None
 **/
void pmu_force_into_ota_isp_mode(void)
{
    // Reboot Prepare
    pmu_reboot_prepare();

#ifdef CONFIG_HS6621
    // close pll
    pmu_pll_close();

    // Must disable and invalid cache in HS6621
    sfs_cache_enable(false);
    sfs_cache_invalidate_all();
#endif

    // Set flag
    HS_PMU->SW_STATUS |= PMU_SW_STATUS_REBOOT_FROM_OTA_ISP_MASK;

    // Make sure the isp bit is setted
    HS_PMU->BOOT_SEL |= 1u; // @ref BOOT_SEL_ISP_BIT

    // Remap and Reset
    pmu_reset_to_rom();

    // Never come here
    while(1);
}
#endif

/**
 * @brief pmu validate the configuration
 *
 * @return None
 **/
void pmu_configuration_validate(void)
{
#ifndef CONFIG_WITHOUT_CFG
    uint16_t len;
    uint8_t xtal32m_ctune, xtal32k_ctune;
//    uint8_t dcdc_enable;
    int res;

    // xtal24m ctune
    len = 1;
    res = cfg_get(CFG_SECTION_SYS, CFG_TAG_XTAL32M_CTUNE, &xtal32m_ctune, &len);
    if(res>=0 && xtal32m_ctune<=31)
        pmu_xtal32m_change_param(xtal32m_ctune);

    // xtal32k ctune
    len = 1;
    res = cfg_get(CFG_SECTION_SYS, CFG_TAG_XTAL32K_CTUNE, &xtal32k_ctune, &len);
    if(res>=0 && xtal32k_ctune<=15)
        pmu_xtal32k_change_param(xtal32k_ctune, -1);

//    // DCDC
//    len = 1;
//    res = cfg_get(CFG_SECTION_SYS, CFG_TAG_DCDC_ENABLE, &dcdc_enable, &len);
//    if(res>=0 && dcdc_enable<=1)
//        pmu_dcdc_enable((bool)dcdc_enable);
#endif
}

/**
 * @brief Get charge status
 *
 * @return status
 **/
pmu_charge_status_t pmu_charge_status(void)
{
    volatile uint32_t charge_status = HS_SYS->CHRGR_STAT;

    if(charge_status & SYS_CHRGR_INSERT_DETECT_MASK)
    {
        if(charge_status & SYS_CHRGR_FINISH_MASK)
            return PMU_CHARGE_COMPLETE;
        else
            return PMU_CHARGE_CHARGING;
    }
    else
    {
        return PMU_CHARGE_EXTRACT;
    }
}

/**
 * @brief pmu_reboot_reason()
 *
 * @return 
 **/
pmu_reboot_reason_t pmu_reboot_reason(void)
{
    if(HS_PMU->SW_STATUS & PMU_SW_STATUS_REBOOT_FROM_WDT_MASK)
        return PMU_REBOOT_FROM_WDT;

    if(HS_PMU->SW_STATUS & PMU_SW_STATUS_REBOOT_FROM_SLEEP_MASK)
        return PMU_REBOOT_FROM_ULTRA_DEEP_SLEEP;

    if(HS_PMU->SW_STATUS & PMU_SW_STATUS_REBOOT_FROM_OTA_ISP_MASK)
        return PMU_REBOOT_FROM_OTA_ISP;

    if(HS_PMU->SW_STATUS & PMU_SW_STATUS_REBOOT_FORCE_MASK)
        return PMU_REBOOT_FROM_USER;

    return PMU_REBOOT_FROM_POWER_ON;
}

/**
 * @brief get retention reg value
 *
 * @note This reg value will lost only after power down. default is 0x0000
 *
 * @return retention reg value
 **/
uint16_t pmu_retention_reg_get(void)
{
    return (uint16_t)register_get(&HS_PMU->SW_STATUS, MASK_POS(PMU_SW_STATUS_USER_RETENTION));
}

/**
 * @brief set retention reg
 *
 * @note This reg value will lost only after power down. default is 0x0000
 *
 * @param[in] data  reg value
 *
 * @return None
 **/
void pmu_retention_reg_set(uint16_t value)
{
    register_set(&HS_PMU->SW_STATUS, MASK_1REG(PMU_SW_STATUS_USER_RETENTION, value));
}

/**
 * @brief set startup hook callback
 *
 * @param[in] hook_cb  callback
 *
 * @return None
 **/
void pmu_startup_hook_set(pmu_startup_hook_callback_t hook_cb)
{
    pmu_env.startup_hook_callback = hook_cb;

    if(hook_cb)
        register_set1(&HS_PMU->SW_STATUS, PMU_SW_STATUS_STARTUP_HOOK_MASK);
    else
        register_set0(&HS_PMU->SW_STATUS, PMU_SW_STATUS_STARTUP_HOOK_MASK);
}

/**
 * @brief check startup hook
 *
 * @return None
 **/
void pmu_startup_hook_check(void)
{
    if(HS_PMU->SW_STATUS & PMU_SW_STATUS_STARTUP_HOOK_MASK)
        pmu_env.startup_hook_callback();
}

#ifdef PMU_DETECT_LOWPOWER_TIME
/**
 * @brief Detect lowpower time
 *
 * @param[in] pin  detect pin num, 0 to disable detect
 *
 * @return None
 **/
void pmu_detect_lowpower_time(uint8_t pin)
{
    pinmux_config(pin, PINMUX_GPIO_MODE_CFG);
    pmu_pin_mode_set(BIT_MASK(pin), PMU_PIN_MODE_PP);
    gpio_write(BIT_MASK(pin), (pin&0x80) ? GPIO_HIGH : GPIO_LOW);
    gpio_set_direction(BIT_MASK(pin), GPIO_OUTPUT);

    pmu_env.detect_lowpower_time_pin = pin;
}
#endif

/**
 * @brief pmu dump
 *
 * @param[in] printf_dump_func  like printf
 *
 * @note
 *
 * The dump infomation looks like this:
 *   [PMU] prevent_status=00000000
 *   [PMU] wakeup_pin=0001000004(cur_level=0001000004 sleep_level=0001000004)
 *   [PMU] pull_up=FFFD7F9CDF(cur_level=FFFD7F9CDC) pull_down=0000000000(cur_level=0000000000) all_cur_level=FFFFFFFFFC
 *   [PMU] clocking: CPU(128MHz) SRAM(000087FF,ULP:32MHz) SF0 SF1 UART0 GPIO ANA
 *
 * Explain:
 * 1st line:
 *   Something (peripheral, user...) prevent system sleep.
 *   bitmask reference @ref pmu_lowpower_peripheral_t
 * 2nd line:
 *   Bitmask of wakeup pin.
 *   If cur_level != sleep_level, system can't sleep.
 * 3rd line:
 *   Inside pull-up and pull-down status.
 *   if pull_up is not equal to it's cur_level, symtem has current leakage in sleep.
 *   if pull_down's cur_level is not equal to 0, system has current leakage in sleep.
 * 4th line:
 *   Working modules.
 *   SRAM: powered block, the more block are powered on, the greater the current consumption during sleep.
 *         reference: @ref pmu_ram_power_on and @ref pmu_ram_power_off_invalid_block
 **/
void pmu_dump(void *printf_dump_func)
{
    void (*__printf)(const char *format, ...) = (void (*)(const char *format, ...))printf_dump_func;
    uint32_t current_level_mask = HS_GPIO0->DATA;
    uint32_t wakeup_mask = HS_PMU->GPIO_WAKEUP;
    uint32_t pull_up_mask = HS_PMU->GPIO_PU_CTRL;
    uint32_t pull_down_mask = HS_PMU->GPIO_PD_CTRL;
    uint32_t pol_mask = HS_PMU->GPIO_POL;
#ifndef CONFIG_GPIO1_ABSENT
    uint32_t current_level_mask_ex = HS_GPIO1->DATA & 0xFF;
    uint32_t wakeup_mask_ex = HS_PMU->GPIO_WAKEUP_1 & 0xFF;
    uint32_t pull_up_mask_ex = HS_PMU->GPIO_PU_CTRL_1 & 0xFF;
    uint32_t pull_down_mask_ex = HS_PMU->GPIO_PD_CTRL_1 & 0xFF;
    uint32_t pol_mask_ex = HS_PMU->GPIO_POL_1 & 0xFF;
#endif

#if defined(CONFIG_HS6621) && (CONFIG_HARDWARE_VERSION>1)
    // gpio[39:14] low active, gpio[13:0] high active
    pull_up_mask ^= 0xFFFFC000;
    pull_up_mask_ex ^= 0xFF;
#endif

    // Prevent status
    __printf("[PMU] prevent_status=%08X\n", pmu_lowpower_prevent_status());

#ifndef CONFIG_GPIO1_ABSENT
    // Wakeup pin
    __printf("[PMU] wakeup_pin=%02X%08X(cur_level=%02X%08X sleep_level=%02X%08X)\n",
            wakeup_mask_ex, wakeup_mask,
            current_level_mask_ex&wakeup_mask_ex, current_level_mask&wakeup_mask,
            pol_mask_ex, pol_mask);

    // Pull status
    __printf("[PMU] pull_up=%02X%08X(cur_level=%02X%08X) pull_down=%02X%08X(cur_level=%02X%08X) all_cur_level=%02X%08X\n",
            pull_up_mask_ex, pull_up_mask,
            current_level_mask_ex&pull_up_mask_ex, current_level_mask&pull_up_mask,
            pull_down_mask_ex, pull_down_mask,
            current_level_mask_ex&pull_down_mask_ex, current_level_mask&pull_down_mask,
            current_level_mask_ex, current_level_mask);
#else
    // Wakeup pin
    __printf("[PMU] wakeup_pin=%08X(cur_level=%08X sleep_level=%08X)\n",
            wakeup_mask, current_level_mask&wakeup_mask, pol_mask);

    // Pull status
    __printf("[PMU] pull_up=%08X(cur_level=%08X) pull_down=%08X(cur_level=%08X) all_cur_level=%08X\n",
            pull_up_mask, current_level_mask&pull_up_mask, pull_down_mask, current_level_mask&pull_down_mask, current_level_mask);
#endif

    // Clocking
    __printf("[PMU] clocking: CPU(%dMHz)", cpm_get_clock(CPM_CPU_CLK)/1000000);
#ifdef CONFIG_HS6621
    __printf(" SRAM(%08X,ULP:%dMHz)", (~HS_PSO->RAM_CFG), cpm_get_clock(CPM_ULP_RAM_CLK)/1000000);
#else
    __printf(" SRAM(%02X)", (~HS_PSO->RAM_CFG) & 0xFF);
#endif

    if(!(HS_PSO->APB_CFG & CPM_RTC_APB_GATE_EN_MASK))  __printf(" RTC(WDT)");
    if(!(HS_PSO->SF0_CFG & CPM_GATE_EN_MASK))          __printf(" SF0");
    if(!(HS_PSO->SF1_CFG & CPM_GATE_EN_MASK))          __printf(" SF1");
    if(!(HS_PSO->TIM_CFG[0] & CPM_GATE_EN_MASK))       __printf(" TIMER0");
    if(!(HS_PSO->TIM_CFG[1] & CPM_GATE_EN_MASK))       __printf(" TIMER1");
    if(!(HS_PSO->TIM_CFG[2] & CPM_GATE_EN_MASK))       __printf(" TIMER2");
    if(!(HS_PSO->UART0_CFG & CPM_GATE_EN_MASK))        __printf(" UART0");
    if(!(HS_PSO->UART1_CFG & CPM_GATE_EN_MASK))        __printf(" UART1");
    if(!(HS_PSO->I2C_CFG & CPM_GATE_EN_MASK))          __printf(" I2C0");
    if(!(HS_PSO->I2C1_CFG & CPM_GATE_EN_MASK))         __printf(" I2C1");
    if(!(HS_PSO->I2C2_CFG & CPM_GATE_EN_MASK))         __printf(" I2C2");
    if(!(HS_PSO->SPI0_CFG & CPM_GATE_EN_MASK))         __printf(" SPI0");
    if(!(HS_PSO->SPI1_CFG & CPM_GATE_EN_MASK))         __printf(" SPI1");
    if(!(HS_PSO->KPP_CFG & CPM_GATE_EN_MASK))          __printf(" KPP");
    if(!(HS_PSO->DMA_CFG & CPM_GATE_EN_MASK))          __printf(" DMA");
    if(!(HS_PSO->GPIO_CFG & CPM_GATE_EN_MASK))         __printf(" GPIO");
    if(!(HS_PSO->QDEC_CFG & CPM_GATE_EN_MASK))         __printf(" QDEC");
    if(!(HS_PSO->AUDIO_CFG & CPM_GATE_EN_MASK))        __printf(" AUDIO");

//    if(!(HS_PSO->I2S_CFG & CPM_GATE_EN_MASK))          __printf(" I2S"); //digital can't read
    if(!(HS_PSO->ANA_IF_CLK_CFG & CPM_GATE_EN_MASK))   __printf(" ANA");
    __printf("\n");
}

/**
 * @brief POWER_DOWN_IRQHandler()
 *
 * @return 
 **/
void POWER_DOWN_IRQHandler(void)
{
    register_set1(&HS_PMU->BASIC, PMU_BASIC_POWER_DOWN_ACK_MASK);
    HS_PMU_UPD_RDY();
    while(1);
}

/** @} */

