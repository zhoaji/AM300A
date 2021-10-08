/**
 * @file co_power.c
 * @brief 
 * @date Thu 16 Jul 2015 04:45:10 PM CST
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
#include "hci.h"

#ifndef CONFIG_WITHOUT_RW
#include "ke_event.h"
#include "rwip_int.h"
#endif

/*********************************************************************
 * MACROS
 */


/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
    bool sleep_enable;
    bool ultra_sleep_enable;
    bool fast_sleep_enable;
    bool hib_sleep_enable;

    co_power_status_callback_t usr_status_cb;
    co_power_status_callback_t sys_status_cb;

    co_power_sleep_callback_t sleep_state_cb;

    // For RTOS Support and Retarget sleep execute
    co_power_execute_callback_t execute_cb;
}co_power_env_t;

/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * LOCAL VARIABLES
 */

volatile static co_power_env_t co_power_env = {
    /* sleep_enable */       false,
    /* ultra_sleep_enable */ false,
    /* fast_sleep_enable */  false,
    /* hib_sleep_enable */   false,
    /* usr_status_cb */      NULL,
    /* sys_status_cb */      NULL,
    /* sleep_state_cb */     NULL,
    /* execute_cb */         co_sleep,
};

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
 * @brief co_power_init()
 *
 * @return 
 **/
void co_power_init(void)
{

}

/**
 * @brief co_power_sleep_enable()
 *
 * @param[in] enable  
 *
 * @return 
 **/
void co_power_sleep_enable(bool enable)
{
    co_power_env.sleep_enable = enable;
}

/**
 * @brief co_power_sleep_is_enabled()
 *
 * @return 
 **/
bool co_power_sleep_is_enabled(void)
{
    return co_power_env.sleep_enable;
}

/**
 * @brief ultra sleep mode setting.
 *
 * @note:
 *   If enabled, ultra sleep mode will repace deepsleep mode.
 *   ultra sleep mode will power off all SRAM, the wakeup-pin will trigger chip to reboot.
 *
 * @param[in] enable  enable or disable
 *
 * @return None
 **/
void co_power_ultra_sleep_mode_enable(bool enable)
{
    co_power_env.ultra_sleep_enable = enable;
}

/**
 * @brief co_power_ultra_sleep_mode_is_enabled()
 *
 * @return enabled or disabled
 **/
bool co_power_ultra_sleep_mode_is_enabled(void)
{
    return co_power_env.ultra_sleep_enable;
}

/**
 * @brief fast sleep mode enable
 *
 * @param[in] enable  
 *
 * @return None
 **/
void co_power_fast_sleep_mode_enable(bool enable)
{
    co_power_env.fast_sleep_enable = enable;
}

/**
 * @brief fast sleep mode is enabled
 *
 * @return whether enabled
 **/
bool co_power_fast_sleep_mode_is_enabled(void)
{
    return co_power_env.fast_sleep_enable;
}

/**
 * @brief  co power hib sleep mode enable
 *
 * @param[in] enable  enable
 **/
void co_power_hib_sleep_mode_enable(bool enable)
{
    co_power_env.hib_sleep_enable = enable;
}

/**
 * @brief  co power hib sleep mode is enabled
 *
 * @return
 **/
bool co_power_hib_sleep_mode_is_enabled(void)
{
    return co_power_env.hib_sleep_enable;
}

/**
 * @brief co_power_register_user_status()
 *
 * @param[in] status_cb  
 *
 * @return 
 **/
void co_power_register_user_status(co_power_status_callback_t status_cb)
{
    co_power_env.usr_status_cb = status_cb;
}

/**
 * @brief co_power_register_system_status()
 *
 * @param[in] status_cb  
 *
 * @return 
 **/
void co_power_register_system_status(co_power_status_callback_t status_cb)
{
    co_power_env.sys_status_cb = status_cb;
}

/**
 * @brief co_power_register_sleep_event()
 *
 * @param[in] event_cb  
 *
 * @return 
 **/
void co_power_register_sleep_event(co_power_sleep_callback_t event_cb)
{
    co_power_env.sleep_state_cb = event_cb;
}

/**
 * @brief co_power_register_executor()
 *
 * Two function:
 * 1. Retarget sleep executor
 * 2. For RTOS Support, Suspending BLE thread should be did in this callback
 *
 * cooperate with co_power_register_wakener()
 *
 * @param[in] execute_cb  
 *
 * @return 
 **/
void co_power_register_executor(co_power_execute_callback_t execute_cb)
{
    co_power_env.execute_cb = execute_cb;
}

/**
 * @brief co_power_sleep_handler()
 *
 * @param[in] sleep_state  
 * @param[in] power_status  
 *
 * @return None
 **/
void co_power_sleep_handler(co_power_sleep_state_t sleep_state, co_power_status_t power_status)
{
    if(co_power_env.sleep_state_cb)
        co_power_env.sleep_state_cb(sleep_state, power_status);
}

/**
 * @brief co_power_manage()
 *
 * @return 
 **/
void co_power_manage(void)
{
    co_power_status_t status;

    co_assert(co_power_env.sys_status_cb != NULL);
    co_assert(co_power_env.usr_status_cb != NULL);

    // !!!NOTE!!!
    // Must not use NVIC to disable IRQ
    // NVIC will lead: IRQ pending does not wakeup from WFI
    __set_PRIMASK(1);

#ifndef CONFIG_WITHOUT_RW
    // Check if some kernel processing is ongoing (during wakeup, kernel events are not processed)
    if (((rwip_env.prevent_sleep & RW_WAKE_UP_ONGOING) == 0) && ke_event_get_all())
        status = POWER_ACTIVE;
    else
#endif
        status = co_power_env.usr_status_cb(); // ckeck usr sleep status

    // Only usr sleep status to check sys
    if(status > POWER_IDLE)
    {
#ifndef CONFIG_WITHOUT_RW
        // ckeck sys sleep status (Ignore sys POWER_ACTIVE mode)
        co_power_status_t sys_status = co_power_env.sys_status_cb();
        status = (co_power_status_t)MIN((int)sys_status, (int)status);
        co_assert(status != POWER_ACTIVE);
#endif

        // Read user setting
        if(!co_power_sleep_is_enabled())
            status = POWER_IDLE;
    }

    co_power_env.execute_cb(status);

    __set_PRIMASK(0);
}

/** @} */


