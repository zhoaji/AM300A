/**
 * @file cpm.h
 * @brief clock manage driver
 * @date Thu 26 Nov 2015 04:43:43 PM CST
 * @author liqiang
 *
 * @defgroup CPM CPM
 * @ingroup PERIPHERAL
 * @brief clock manage driver
 * @details clock manage driver
 *
 * @{
 */

#ifndef __CPM_H__
#define __CPM_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include <stdbool.h>
#include "peripheral.h"

/*********************************************************************
 * MACROS
 */

/// @cond

#define CPM_ANA_CLK_ON       0
#define CPM_ANA_CLK_OFF      1
#define CPM_ANA_CLK_IGNORE   0xFFFFFFFF

#define CPM_ANA_CLK_ENABLE_NOIRQ() \
    do { \
        uint32_t __ana_if_ahb_cfg = HS_PSO->ANA_IF_AHB_CFG; \
        uint32_t __ana_if_clk_cfg = HS_PSO->ANA_IF_CLK_CFG; \
        if (__ana_if_ahb_cfg || __ana_if_clk_cfg) \
            cpm_ana_clock_set(0, 0);

#define CPM_ANA_AHB_CLK_ENABLE_NOIRQ() \
    do { \
        uint32_t __ana_if_ahb_cfg = HS_PSO->ANA_IF_AHB_CFG; \
        uint32_t __ana_if_clk_cfg = HS_PSO->ANA_IF_CLK_CFG; \
        (void) __ana_if_clk_cfg; \
        if (__ana_if_ahb_cfg) \
            cpm_ana_clock_enable_ahb();

#define CPM_ANA_CLK_RESTORE_NOIRQ() \
        if (__ana_if_ahb_cfg || __ana_if_clk_cfg) \
            cpm_ana_clock_set(__ana_if_ahb_cfg, __ana_if_clk_cfg); \
    } while(0)

#define CPM_ANA_CLK_DISABLE_NOIRQ() \
        cpm_ana_clock_disable(); \
    } while(0)


#define CPM_ANA_CLK_ENABLE()        CO_DISABLE_IRQ(); CPM_ANA_CLK_ENABLE_NOIRQ()
#define CPM_ANA_AHB_CLK_ENABLE()    CO_DISABLE_IRQ(); CPM_ANA_AHB_CLK_ENABLE_NOIRQ()
#define CPM_ANA_CLK_RESTORE()       CPM_ANA_CLK_RESTORE_NOIRQ(); CO_RESTORE_IRQ()
#define CPM_ANA_CLK_DISABLE()       CPM_ANA_CLK_DISABLE_NOIRQ(); CO_RESTORE_IRQ()

/// @endcond

/// Max clock
#define CPM_CLK_MAX     0xFFFFFFFF

/*********************************************************************
 * TYPEDEFS
 */
/// CPM clock type
typedef enum
{
    CPM_TOP_CLK,
    CPM_CPU_CLK,
    CPM_AHB_CLK,
    CPM_APB_CLK,
    CPM_ULP_RAM_CLK,
    CPM_LP_RAM_CLK,
    CPM_ROM_CLK,
    CPM_SF0_CLK,
    CPM_SF1_CLK,
    CPM_TIM0_CLK,
    CPM_TIM1_CLK,
    CPM_TIM2_CLK,
    CPM_UART0_CLK,
    CPM_UART1_CLK,
    CPM_I2C_CLK,
    CPM_I2C1_CLK,
    CPM_I2C2_CLK,
    CPM_SPI_CLK,
    CPM_RTC_CLK,
    CPM_WDT_CLK,
    CPM_QDEC_CLK,
    CPM_KPP_CLK,
    CPM_I2S_CLK,
    CPM_AUDIO_CLK,
#ifdef CONFIG_HS6621C
    CPM_SF2_CLK,
#endif
} cpm_clk_t;

/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */
/// @cond
/**
 * @brief cpm initialize
 *
 * @return None
 **/
void cpm_init(void);
/// @endcond

/**
 * @brief Get specified peripheral clock
 *
 * @param[in] type  clock source type
 *
 * @return clock in Hz
 */
uint32_t cpm_get_clock(cpm_clk_t type);

/**
 * @brief Set specified peripheral clock
 *
 * @param[in] type  clock source type
 * @param[in] clk  clock in Hz. 0:gate CPM_CLK_MAX:no-division/bypass
 *
 * @retval true success
 * @retval false fail
 **/
bool cpm_set_clock(cpm_clk_t type, uint32_t clk);

/**
 * @brief Set specified peripheral clock
 *
 * @param[in] type  clock source type
 * @param[in] div  division
 *
 * @retval true success
 * @retval false fail
 **/
bool cpm_set_clock_div(cpm_clk_t type, uint32_t div);

/**
 * @brief  cpm optimize clock
 *
 * @param[in] want_topclk_mhz  want topclk mhz, 0:use reg value
 * @param[in] want_cpu_div  want cpu div, 0:use reg value
 **/
void cpm_optimize_clock(uint32_t want_topclk_mhz, uint32_t want_cpu_div);

/// @cond

/**
 * @brief cpm_store()
 *
 * Just for system call before sleep
 *
 * @return None
 **/
void cpm_store(void);

/**
 * @brief cpm_restore()
 *
 * Just for system call after sleep
 *
 * @return None
 **/
void cpm_restore(void);

/**
 * @brief cpm_ana_clock_set()
 *
 * @param[in] ana_if_ahb_cfg  
 * @param[in] ana_if_clk_cfg  
 *
 * @return 
 **/
__STATIC_INLINE void cpm_ana_clock_set(uint32_t ana_if_ahb_cfg, uint32_t ana_if_clk_cfg)
{
    // Close ANA if clock
    if (ana_if_ahb_cfg != CPM_ANA_CLK_IGNORE) HS_PSO->ANA_IF_AHB_CFG = ana_if_ahb_cfg;
    if (ana_if_clk_cfg != CPM_ANA_CLK_IGNORE) HS_PSO->ANA_IF_CLK_CFG = ana_if_clk_cfg;
    // Wait
    HS_PSO_XTAL32M_UPD_RDY();
}

/**
 * @brief Enable analog clock, call by system
 **/
__STATIC_INLINE void cpm_ana_clock_enable(void)
{
    cpm_ana_clock_set(CPM_ANA_CLK_ON, CPM_ANA_CLK_ON);
}

/**
 * @brief Disable analog clock, call by system
 **/
__STATIC_INLINE void cpm_ana_clock_disable(void)
{
    cpm_ana_clock_set(CPM_ANA_CLK_OFF, CPM_ANA_CLK_OFF);
}

/**
 * @brief Enable analog clock, call by system
 **/
__STATIC_INLINE void cpm_ana_clock_enable_ahb(void)
{
    // Close ANA if clock
    register_set0(&HS_PSO->ANA_IF_AHB_CFG, CPM_ANA_IF_AHB_GATE_EN_MASK);
    HS_PSO_UPD_RDY();
}

/**
 * @brief Disable analog clock, call by system
 **/
__STATIC_INLINE void cpm_ana_clock_disable_ahb(void)
{
    // Close ANA if clock
    register_set1(&HS_PSO->ANA_IF_AHB_CFG, CPM_ANA_IF_AHB_GATE_EN_MASK);
    HS_PSO_UPD_RDY();
}

/// @endcond

#ifdef __cplusplus
}
#endif

#endif

/** @} */

