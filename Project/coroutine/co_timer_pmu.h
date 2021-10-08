/**
 * @file co_timer_pmu.h
 * @brief software timer
 * @date Tue 09 Dec 2014 04:37:42 PM CST
 * @author liqiang
 *
 * @defgroup CO_TIMER Software Timer
 * @ingroup COROUTINE
 * @brief Software Timer Module (that can work in sleep mode)
 * @details The software timer
 *
 * @{
 */

#ifndef __CO_TIMER_PMU_H__
#define __CO_TIMER_PMU_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "co_list.h" // RW

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */

/// Timer mode for one shot or repeat
typedef enum
{
    /// Only run once
    TIMER_ONE_SHOT,
    /// Repeat until stop it
    TIMER_REPEAT,
}co_timer_mode_t;

/// @cond
/// timer object
typedef struct
{
    co_list_hdr_t hdr;

    uint32_t delay_tick;
    void *cb;
    co_timer_mode_t mode;
    void *param;

    uint32_t time;
}co_timer_t;
/// @endcond

/**
 * @brief software timer expire callback
 *
 * @param[in] id     Timer id
 * @param[in] param  Parameter with co_timer_set()
 *
 * @return None
 **/
typedef void (*co_timer_callback_t)(co_timer_t *timer, void *param);

/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */

/// @cond
/**
 * @brief timer init
 *
 * @return None
 **/
void co_timer_init(void);
/// @endcond

/**
 * @brief Setup a software timer with tick
 *
 * @param[in] timer  the timer object (must be static or global variable)
 * @param[in] delay_tick  it uint is 30.52us
 * @param[in] mode  one shot or repeat
 * @param[in] callback  expire callback
 * @param[in] param  params
 *
 * @return false: Set timer fail;
 *         true: Set timer success
 **/
bool co_timer_set_tick(co_timer_t *timer, uint32_t delay_tick, co_timer_mode_t mode,
        co_timer_callback_t callback, void *param);

/**
 * @brief Setup a software timer with millisecond
 *
 * @param[in] timer  the timer object (must be static or global variable)
 * @param[in] delay  it uint is 1ms
 * @param[in] mode  one shot or repeat
 * @param[in] callback  expire callback
 * @param[in] param  params
 *
 * @return false: Set timer fail;
 *         true: Set timer success
 **/
bool co_timer_set(co_timer_t *timer, uint32_t delay, co_timer_mode_t mode,
        co_timer_callback_t callback, void *param);

/**
 * @brief delete a timer
 *
 * @param[in] timer  the timer object (must be static or global variable)
 *
 * @return None
 **/
void co_timer_del(co_timer_t *timer);

/**
 * @brief timer dump
 *
 * @note If this function called in co_timer expired callback, the expired timer don't dump
 *
 * @param[in] printf_dump_func  dump function, link printf
 **/
void co_timer_dump(void *printf_dump_func);

#ifdef __cplusplus
}
#endif

#endif

/** @} */


