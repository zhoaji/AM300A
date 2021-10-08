/**
 * @file encoder.h
 * @brief encoder driver
 * @date Fri 28 Apr 2017 11:11:50 AM CST
 * @author liqiang
 *
 * @defgroup Encoder Encoder
 * @ingroup PERIPHERAL
 * @brief Encoder driver for QDEC
 * @details Encoder driver
 *
 * This block decodes the pulse trains from a rotary encoder to provide
 * the step and the direction of the movement of an external device.
 * Three axes (X, Y, Z) are supported.
 *
 * The integrated quadrature decoder can automatically decode the signals
 * for the X, Y and Z axes of a HID input device, reporting step count and
 * direction: the channels are expected to provide a pulse train with 90
 * degrees phase difference; depending on whether the reference channel is
 * leading or lagging, the direction can be determined.
 *
 * This block can be used for waking up the chip as soon as there is any
 * kind of movement from the external device connected to it.
 *
 * @{
 *
 * @example example_encoder.c
 * This is an example of how to use the encoder
 *
 */

#ifndef __ENCODER_H__
#define __ENCODER_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "hs66xx.h"

/*********************************************************************
 * MACROS
 */


/*********************************************************************
 * TYPEDEFS
 */
/// Directory
typedef enum
{
    ENCODER_DIR_UP,
    ENCODER_DIR_DOWN,
}encoder_dir_t;

/// Mode
typedef enum
{
    /// Encoder mode 1 - Counter counts up/down on XB/YB/ZB edge
    ENCODER_COUNT_ON_B_EDGE,
    /// Encoder mode 2 - Counter counts up/down on XA/YA/ZA edge
    ENCODER_CONUT_ON_A_EDGE,
    /// Encoder mode 3 - Counter counts up/down on both A/B edges
    ENCODER_COUNT_ON_AB_EDGE,
}encoder_mode_t;

/// Polarity
typedef enum
{
    ENCODER_POL_NORMAL,
    ENCODER_POL_INVERTED,
}encoder_pol_t;

/// Clock select
typedef enum
{
    ENCODER_CLK_32K,
    ENCODER_CLK_HCLK,
}encoder_clk_t;

/**
 * @brief Encoder event callback
 *
 * @param[in] encoder  Encoder object
 * @param[in] event  Event type
 *
 * @return None
 **/
typedef void (* encoder_event_callback_t)(HS_ENCODER_Type *encoder, uint32_t cnt);

/// Encoder configuration
typedef struct
{
    /// a pin number
    uint8_t a_pin;
    /// b pin number
    uint8_t b_pin;
    /// mode
    encoder_mode_t mode;
    /// pol with XA/YA/ZA
    encoder_pol_t pol_a;
    /// pol with XB/YB/ZB
    encoder_pol_t pol_b;
    /// max counter
    uint32_t counter_max;
    /// clock
    encoder_clk_t clk;
    /// callback
    encoder_event_callback_t callback;
}encoder_config_t;

/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */

/**
 * @brief encoder get current counter
 *
 * @param[in] encoder  Encoder object
 *
 * @return Current encoder counter
 **/
__STATIC_INLINE uint32_t encoder_get_counter(HS_ENCODER_Type *encoder)
{
    return encoder->CNT;
}

/**
 * @brief encoder get current direction
 *
 * @param[in] encoder  Encoder object
 *
 * @return direction
 **/
__STATIC_INLINE encoder_dir_t encoder_get_direction(HS_ENCODER_Type *encoder)
{
    return (encoder_dir_t)register_get(&encoder->OF, MASK_POS(ENCODER_OF_DIR));
}

/**
 * @brief Encoder config
 *
 * @param[in] encoder  Encoder object
 * @param[in] config  configuration
 *
 * @return None
 **/
void encoder_config(HS_ENCODER_Type *encoder, const encoder_config_t *config);

/**
 * @brief Encoder start
 *
 * @param[in] encoder  Encoder object
 *
 * @return None
 **/
void encoder_start(HS_ENCODER_Type *encoder);

/**
 * @brief Encoder stop
 *
 * @param[in] encoder  Encoder object
 *
 * @return None
 **/
void encoder_stop(HS_ENCODER_Type *encoder);

/// @cond

/**
 * @brief encoder_pin_mask()
 *
 * @return 
 **/
uint32_t encoder_pin_mask(void);

/**
 * @brief encoder_restore()
 *
 * @return 
 **/
void encoder_restore(void);

/// @endcond

#ifdef __cplusplus
}
#endif

#endif

/** @} */

