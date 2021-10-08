/**
 * @file peripheral.c
 * @brief 
 * @date Fri 01 Dec 2017 09:06:10 AM CST
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


/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/**
 * @brief peripheral lowpower store
 *
 * @return None
 **/
void peripheral_lowpower_enter(co_power_status_t power_status)
{
    cpm_store();
    gpio_store();
    rtc_store();

    // Gate not sleep module.
    // Avoid IE_CTRL generatting unexpect interrupt
    register_set1(&HS_PSO->TIM_CFG[0], CPM_GATE_EN_MASK);
    register_set1(&HS_PSO->TIM_CFG[1], CPM_GATE_EN_MASK);
    register_set1(&HS_PSO->TIM_CFG[2], CPM_GATE_EN_MASK);
    register_set1(&HS_PSO->UART0_CFG,  CPM_GATE_EN_MASK);
    register_set1(&HS_PSO->UART1_CFG,  CPM_GATE_EN_MASK);
    register_set1(&HS_PSO->I2C_CFG,    CPM_GATE_EN_MASK);
    register_set1(&HS_PSO->SPI0_CFG,   CPM_GATE_EN_MASK);
    register_set1(&HS_PSO->SPI1_CFG,   CPM_GATE_EN_MASK);
    register_set1(&HS_PSO->I2S_CFG,    CPM_GATE_EN_MASK);
    register_set1(&HS_PSO->QDEC_CFG,   CPM_GATE_EN_MASK);
    HS_PSO_UPD_RDY();

    // SF XIP
    if (sfs_is_xip())
    {
        // auto disable
        if (sfs_is_auto_disen() || power_status==POWER_DEEP_SLEEP)
            sfs_disable();
        else
            sfs_lowpower_enter();
    }
}

/**
 * @brief peripheral lowpower restore
 *
 * @return None
 **/
void peripheral_lowpower_leave(co_power_status_t power_status)
{
    cpm_restore();
    pinmux_restore();
    gpio_restore();
    rtc_restore();
    encoder_restore();

    // SF XIP
    if (sfs_is_xip())
    {
        sfs_enable();

        // auto enable
        if (sfs_is_auto_disen() || power_status==POWER_DEEP_SLEEP)
            ;
        else
            sfs_lowpower_leave();
    }

    // always enable cache
    sfs_cache_enable(true);
}

/** @} */


