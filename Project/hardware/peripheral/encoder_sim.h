/**
 * @file encoder_sim.h
 * @brief encoder use gpio simulation
 * @date Thu 31 Aug 2017 03:14:21 PM CST
 * @author liqiang
 *
 * @defgroup ENCODER_SIM Encoder GPIO Simulation
 * @ingroup PERIPHERAL
 * @brief Encoder gpio simulation
 * @details Encoder driver
 *
 * @{
 */

#ifndef __ENCODER_SIM_H__
#define __ENCODER_SIM_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "peripheral.h"
#include "co.h"


/*********************************************************************
 * MACROS
 */


/*********************************************************************
 * TYPEDEFS
 */

/**
 * @brief encoder sim event callback
 *
 * @param[in] counter  counter
 **/
typedef void (*encoder_sim_event_callback_t)(int counter);

/// encoder config
typedef struct
{
    /// a pin
    uint8_t a_pin;
    /// b pin
    uint8_t b_pin;
    /// event callback
    encoder_sim_event_callback_t callback;
}encoder_sim_config_t;

/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */

/**
 * @brief encoder init
 *
 * @return None
 **/
void encoder_sim_config(const encoder_sim_config_t *config);

/**
 * @brief encoder gpio IRQ handler, PUT THIS TO GPIO IRQ HANDLER
 *
 * @param[in] pin_mask  pin mask
 *
 * @return None
 **/
void encoder_sim_gpio_handler(uint32_t pin_mask);


#ifdef __cplusplus
}
#endif

#endif

/** @} */

