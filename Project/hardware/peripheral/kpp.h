/**
 * @file kpp.h
 * @brief keyboard driver
 * @date Wed 31 May 2017 07:15:40 PM CST
 * @author liqiang
 *
 * @defgroup Keyboard Keyboard
 * @ingroup PERIPHERAL
 * @brief Keyboad driver
 * @details Keyboard driver
 *
 * The keyboard controller can be used for debouncing the incoming GPIO
 * signals when implementing a keyboard scanning engine. It generates
 * an interrupt to the CPU.
 *
 * <pre>
 * Features:
 *   • Generates a keyboard interrupt on key press or key release
 *   • Implements debouncing time up to 31 ms
 * </pre>
 *
 * @{
 *
 * @example example_kpp.c
 * This is an example of how to use the keyboard
 *
 */

#ifndef __KPP_H__
#define __KPP_H__

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
#define KPP_COL_NUM                       18
/// keyboard row max num
#define KPP_ROW_NUM                       8


/*********************************************************************
 * TYPEDEFS
 */
/// keyboard data
typedef uint8_t kpp_data_t[KPP_COL_NUM];

/// Event
typedef enum
{
    KPP_EVENT_DEPRESS,
    KPP_EVENT_RELEASE,
}kpp_event_t;

/// ROW enable bits
typedef enum
{
    KPP_ROW0 = 1<<0,
    KPP_ROW1 = 1<<1,
    KPP_ROW2 = 1<<2,
    KPP_ROW3 = 1<<3,
    KPP_ROW4 = 1<<4,
    KPP_ROW5 = 1<<5,
    KPP_ROW6 = 1<<6,
    KPP_ROW7 = 1<<7,
}kpp_row_t;

/// Colume enable bits
typedef enum
{
    KPP_COL0  = 1<<0,
    KPP_COL1  = 1<<1,
    KPP_COL2  = 1<<2,
    KPP_COL3  = 1<<3,
    KPP_COL4  = 1<<4,
    KPP_COL5  = 1<<5,
    KPP_COL6  = 1<<6,
    KPP_COL7  = 1<<7,
    KPP_COL8  = 1<<8,
    KPP_COL9  = 1<<9,
    KPP_COL10 = 1<<10,
    KPP_COL11 = 1<<11,
    KPP_COL12 = 1<<12,
    KPP_COL13 = 1<<13,
    KPP_COL14 = 1<<14,
    KPP_COL15 = 1<<15,
    KPP_COL16 = 1<<16,
    KPP_COL17 = 1<<17,
}kpp_col_t;

/**
 * @brief event callback
 *
 * @param[in] event  Event type
 * @param[in] data  @ref kpp_data_t
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
typedef void (*kpp_event_callback_t)(kpp_event_t event, const kpp_data_t data);

/// Keyboad config struct
typedef struct
{
    /// ROW enable bitmask
    kpp_row_t row_mask;
    /// Colume enable bitmask
    kpp_col_t col_mask;
    /// Colume select and row scan, unit:us
    uint16_t col_output_delay;
    /// Keys scan interval, unit:ms
    uint16_t scan_interval;
    /// Event callback
    kpp_event_callback_t callback;
}kpp_config_t;

/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */
/**
 * @brief Keyboad initialize
 *
 * @return None
 **/
void kpp_init(void);

/**
 * @brief Keyboad config
 *
 * @param[in] config  configuration @ref kpp_config_t
 *
 * @return None
 **/
void kpp_config(const kpp_config_t *config);

/**
 * @brief Keyboad start
 *
 * @return None
 **/
void kpp_start(void);

/**
 * @brief Keyboad stop
 *
 * @return None
 **/
void kpp_stop(void);

#ifdef __cplusplus
}
#endif

#endif

/** @} */

