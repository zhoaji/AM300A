/**
 * @file co_power.h
 * @brief power manage module
 * @date Thu 16 Jul 2015 04:45:15 PM CST
 * @author liqiang
 *
 * @defgroup CO_POWER Power Manage
 * @ingroup HS662X
 * @brief Power Manage
 * @details
 *
 * HS6621 has 5 power modes: active mode, idle mode, sleep mode, deep sleep mode and shutdown mode.
 *
 * - In the active mode and idle mode, the clock of digital modules (Timer, UART, SPI, PWM …) can be
 * enabled or disabled independently. The power of analog modules which have independent power domain
 * can also be enabled or disabled by application.
 *
 * - In the idle mode, the clock of processor is gated and all of the interrupts can wakeup system.
 *
 * - In the sleep mode, the interrupts of GPIO, BLE sleep timer can wake up the system.
 *
 * - In the deep sleep mode, only the interrupts of GPIO can wakeup system.
 * 32k clock is power off, so BLE stack does not work in the deep sleep mode.
 *
 * Power mode setting depends on user setting, peripherals’ status and BLE’s status.
 *
 * The user power mode setting is managed by user who can use API co_power_register_user_status()
 * to set power mode user want the system to be.
 *
 * The peripherals’ status is managed by drivers. Actually the driver knows when the peripherals
 * cannot enter into sleep mode. For example when the ADC is sampling, the ADC driver knows the
 * system cannot enter into sleep mode at this time. User does not take care of the peripherals’
 * status.
 *
 * The status of BLE is managed by BLE stack. User will be called by API co_power_register_sleep_event()
 * to obtain the status of BLE.
 *
 * <b>After leave sleep (obtain the event @ref POWER_SLEEP_LEAVE_TOP_HALF), user should
 * reinitialize (or restore) all used peripheral</b>
 *
 * @{
 */

#ifndef __CO_POWER_H__
#define __CO_POWER_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */
/// Power status
typedef enum
{
    /// All modules are alive, and CPU run with full speed
    POWER_ACTIVE,
    /// All modules are alive, but CPU clock is gating
    POWER_IDLE,
    /// Power down most of module(CPU, Peripheral, etc),
    /// but 32K is alive, only gpio and sleep-timer can wake up chip
    POWER_SLEEP,
    /// Power down most of module(CPU, Peripheral, etc),
    /// 32K is not alive, only gpio can wake up chip
    POWER_DEEP_SLEEP,
    /// Power down all module,
    /// only the power pin can wake up chip
    POWER_SHUTDOWN,
}co_power_status_t;

/// Sleep state
typedef enum
{
    /// Event with sleep entering
    POWER_SLEEP_ENTRY,
    /// Event with sleep leaving (IO can not be controlled immediately)
    /// User should reinitialize (or restore) all used peripheral in this event.
    POWER_SLEEP_LEAVE_TOP_HALF,
    /// Event with sleep leaving (IO can be controlled immediately)
    POWER_SLEEP_LEAVE_BOTTOM_HALF,
}co_power_sleep_state_t;

/**
 * @brief sleep state event callback
 *
 * @param[in] sleep_state  current sleep state @ref co_power_sleep_state_t
 * @param[in] power_status  power status, only POWER_SLEEP and POWER_DEEP_SLEEP are valid @ref co_power_status_t
 *
 * @return None
 **/
typedef void (*co_power_sleep_callback_t)(co_power_sleep_state_t sleep_state, co_power_status_t power_status);

/**
 * @brief sleep execute event callback
 *
 * @param[in] status  current sleep status
 *
 * @return None
 **/
typedef void (*co_power_execute_callback_t)(co_power_status_t status);

/**
 * @brief power status event callback
 *
 * @return Wanted power status @ref co_power_status_t
 **/
typedef co_power_status_t (*co_power_status_callback_t)(void);

/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */

/// @cond

/**
 * @brief Power manager initialize, call by system
 *
 * @return None
 **/
void co_power_init(void);

/**
 * @brief co_power_sleep_handler()
 *
 * @param[in] sleep_state  
 * @param[in] power_status  
 *
 * @return None
 **/
void co_power_sleep_handler(co_power_sleep_state_t sleep_state, co_power_status_t power_status);

/**
 * @brief co_power_manage()
 *
 * @return None
 **/
void co_power_manage(void);

/**
 * @brief  co power manage rtos
 **/
void co_power_manage_rtos(void);

/// @endcond

/**
 * @brief Register user power status event callback
 *
 * When system want to enter one of power status (@ref co_power_status_t),
 * the event callback will be called
 *
 * @param[in] status_cb  event callback @ref co_power_status_callback_t
 *
 * @return None
 **/
void co_power_register_user_status(co_power_status_callback_t status_cb);

/**
 * @brief Register system power status event callback
 *
 * @param[in] status_cb  event callback @ref co_power_status_callback_t
 *
 * @return None
 **/
void co_power_register_system_status(co_power_status_callback_t status_cb);

/**
 * @brief Register sleep event callback
 *
 * When enter sleep and leave sleep (@ref POWER_SLEEP @ref POWER_DEEP_SLEEP),
 * the event callback will be called
 *
 * @param[in] event_cb  event callback @ref co_power_sleep_callback_t
 *
 * @return None
 **/
void co_power_register_sleep_event(co_power_sleep_callback_t event_cb);

/**
 * @brief Register sleep executor
 *
 * Two function:
 * 1. Retarget sleep executor
 * 2. For RTOS Support, Suspending BLE thread should be did in this callback
 *
 * @param[in] execute_cb  event callback @ref co_power_execute_callback_t
 *
 * @return None
 **/
void co_power_register_executor(co_power_execute_callback_t execute_cb);

/**
 * @brief Enable sleep state
 *
 * @param[in] enable  true: chip can enter POWER_SLEEP/POWER_DEEP_SLEEP status; false: can't enter those status
 *
 * @return None
 **/
void co_power_sleep_enable(bool enable);

/**
 * @brief whether is sleep state enabled
 *
 * @return whether is enabled
 **/
bool co_power_sleep_is_enabled(void);

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
void co_power_ultra_sleep_mode_enable(bool enable);

/**
 * @brief ultra sleep mode is enabled ?
 *
 * @return enabled or disabled
 **/
bool co_power_ultra_sleep_mode_is_enabled(void);

/**
 * @brief fast sleep mode enable
 *
 * @param[in] enable  
 *
 * @return None
 **/
void co_power_fast_sleep_mode_enable(bool enable);

/**
 * @brief fast sleep mode is enabled
 *
 * @return whether enabled
 **/
bool co_power_fast_sleep_mode_is_enabled(void);

/**
 * @brief  co power hib sleep mode enable
 *
 * @note:
 *   If enabled, HIB sleep mode will repace ultra sleep mode, so if use this mode, please enable ultra sleep mode firstly.
 *   HIB sleep mode will power off all SRAM and other modules, the wakeup-pin will trigger chip to reboot.
 *   Please use hib_wakeup_pin_set() to setup wakeup pin, use hib_timer_start to setup hib sleep timer.
 *
 *   Only HS6621C has this mode.
 *
 * @param[in] enable  enable
 **/
void co_power_hib_sleep_mode_enable(bool enable);

/**
 * @brief  co power hib sleep mode is enabled
 *
 * @return
 **/
bool co_power_hib_sleep_mode_is_enabled(void);

#ifdef __cplusplus
}
#endif

#endif

/** @} */

