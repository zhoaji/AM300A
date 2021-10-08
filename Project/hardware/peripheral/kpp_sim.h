/**
 * @file kpp_sim.h
 * @brief Keyboard GPIO Simulation
 * @date Thu 02 Nov 2017 04:41:44 PM CST
 * @author liqiang
 *
 * @defgroup KPP Keyboard GPIO Simulation
 * @ingroup PERIPHERAL
 * @brief Keyboard GPIO Simulation
 * @details Keyboard GPIO Simulation
 *
 * @{
 *
 * @example example_kpp_sim.c
 * This is an example of how to use the kpp_sim
 *
 */

#ifndef __KPP_SIM_H__
#define __KPP_SIM_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "peripheral.h"


/*********************************************************************
 * MACROS
 */
/// Keyboard colume max num
#define KPP_SIM_COL_NUM                       18
/// keyboard row max num
#define KPP_SIM_ROW_NUM                       8

/*********************************************************************
 * TYPEDEFS
 */
/// keyboard data
typedef uint8_t kpp_sim_data_t[KPP_SIM_COL_NUM];

/// Event
typedef enum
{
    KPP_SIM_EVENT_DEPRESS,
    KPP_SIM_EVENT_RELEASE,
}kpp_sim_event_t;

/**
 * @brief event callback
 *
 * @param[in] event  Event type
 * @param[in] data  @ref kpp_sim_data_t
 *
 * <pre>
 * data[0] bit-0 is col-0,row-0
 * data[0] bit-1 is col-0,row-1
 * data[0] bit-2 is col-0,row-2
 * ...
 * data[0] bit-0 is col-0,row-0
 * data[1] bit-0 is col-1,row-0
 * data[2] bit-1 is col-2,row-1
 * ...
 * </pre>
 *
 * @return None
 **/
typedef void (*kpp_sim_event_callback_t)(kpp_sim_event_t event, const kpp_sim_data_t data);

/// Keyboad config struct
typedef struct
{
    /// ROW enable bitmask (pull-up)
    /// row_pin_mask=0x02200111, means: pin0 is row0, pin4 is row1, pin8 is row2, ...
    uint32_t row_pin_mask;
    /// Colume enable bitmask (open-drain)
    /// col_pin_mask=0x01100222, means: pin1 is col0, pin5 is col1, pin9 is col2, ...
    uint32_t col_pin_mask;
    /// Colume select and row scan, unit:us
    uint16_t col_output_delay;
    /// Keys scan interval, unit:ms
    uint16_t scan_interval;
    /// Event callback
    kpp_sim_event_callback_t callback;
}kpp_sim_config_t;

/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */

/**
 * @brief kpp gpio IRQ handler, PUT THIS TO GPIO IRQ HANDLER
 *
 * @param[in] pin_mask  pin mask
 *
 * @return None
 **/
void kpp_sim_gpio_handler(uint32_t pin_mask);

/**
 * @brief Keyboad config
 *
 * @param[in] config  configuration @ref kpp_sim_config_t
 *
 * @return None
 **/
void kpp_sim_config(const kpp_sim_config_t *config);


#ifdef __cplusplus
}
#endif

#endif

/** @} */

