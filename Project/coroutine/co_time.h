/**
 * @file co_time.h
 * @brief time (tick)
 * @date Tue 09 Dec 2014 04:37:42 PM CST
 * @author liqiang
 *
 * @defgroup CO_TIME Time Tick
 * @ingroup COROUTINE
 * @brief Time Tick Module
 * @details
 *
 * @{
 */

#ifndef __CO_TIME_H__
#define __CO_TIME_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "co.h"

/*********************************************************************
 * MACROS
 */


/*********************************************************************
 * TYPEDEFS
 */


/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */

/**
 * @brief get current stack time(tick)
 *
 * @return current time, uint is 30.52us
 **/
STATIC_INLINE uint32_t co_time(void)
{
    return co_sys_time();
}

/**
 * @brief convert time to safe
 *
 * @param[in] time  time
 *
 * @return safe time
 **/
STATIC_INLINE uint32_t co_time_to_safe(uint32_t time)
{
#if (CO_MAX_SYS_TIME == 0xFFFFFFFF)
    return time;
#else
    return time & CO_MAX_SYS_TIME;
#endif
}

/**
 * @brief time increase +
 *
 * @param[in] time  time
 * @param[in] increase  +value
 *
 * @return increase result
 **/
STATIC_INLINE uint32_t co_time_increase(uint32_t time, uint32_t increase)
{
#if (CO_MAX_SYS_TIME == 0xFFFFFFFF)
    return time + increase;
#else
    co_assert(time <= CO_MAX_SYS_TIME);
    return co_time_to_safe(time + increase);
#endif
}

/**
 * @brief time decrease -
 *
 * @param[in] time  time value
 * @param[in] decrease  -value
 *
 * @return decrease result
 **/
STATIC_INLINE uint32_t co_time_decrease(uint32_t time, uint32_t decrease)
{
#if (CO_MAX_SYS_TIME == 0xFFFFFFFF)
    return time - decrease;
#else
    co_assert(time <= CO_MAX_SYS_TIME);
    return (time < decrease) ? (CO_MAX_SYS_TIME + 1 - (decrease - time)) : (time - decrease);
#endif
}

/**
 * @brief different between with two time
 *
 * @param[in] me  one time
 * @param[in] he  another time
 *
 * @return different
 **/
STATIC_INLINE uint32_t co_time_diff(uint32_t me, uint32_t he)
{
#if (CO_MAX_SYS_TIME == 0xFFFFFFFF)
    uint32_t diff1 = me - he;
    uint32_t diff2 = he - me;
#else
    uint32_t diff1 = (me > he) ? (me - he) : (he - me);
    uint32_t diff2 = CO_MAX_SYS_TIME - diff1 + 1;
#endif

    return MIN(diff1, diff2);
}

/**
 * @brief compare between with two time
 *
 * @param[in] me  one time
 * @param[in] he  another time
 *
 * @return if me is smaller or equeal, return true, otherwise return false.
 **/
STATIC_INLINE bool co_time_compare_equal_lesser(uint32_t me, uint32_t he)
{
#if (CO_MAX_SYS_TIME == 0xFFFFFFFF)
    return (he - me < CO_MAX_TIMER_DELAY);
#else
    return (me > he) ? (me - he > CO_MAX_TIMER_DELAY) : (he - me < CO_MAX_TIMER_DELAY);
#endif
}

/**
 * @brief whether time is pasted
 *
 * @param[in] time  time
 * @param[in] cur_time  current time
 *
 * @return whether pasted
 **/
STATIC_INLINE bool co_time_past(uint32_t time, uint32_t cur_time)
{
    return co_time_compare_equal_lesser(time, cur_time);
}

/**
 * @brief time pasting time
 *
 * @param[in] time  time
 * @param[in] cur_time  current time
 *
 * @return 0:pasted, >0:will past time
 **/
STATIC_INLINE uint32_t co_time_pasting_time(uint32_t time, uint32_t cur_time)
{
    uint32_t delay;

#if (CO_MAX_SYS_TIME == 0xFFFFFFFF)
    delay = time - cur_time;
#else
    if (time > cur_time)
        delay = time - cur_time;
    else
        delay = CO_MAX_SYS_TIME - cur_time + time;
#endif

    return (delay>CO_MAX_TIMER_DELAY) ? 0 : delay;
}

/**
 * @brief time delay(ms) is pasted
 *
 * @param[in] delay_ms  delay with ms
 * @param[in] prev_past_time  prev delay time, if delay pasted, the value will be modify to current time
 *
 * @return pasted
 **/
STATIC_INLINE bool co_time_delay_ms_past(uint32_t delay_ms, uint32_t *prev_past_time)
{
    bool pasted;
    uint32_t cur_time = co_time();

    if(*prev_past_time == CO_INVALID_SYS_TIME)
        *prev_past_time = cur_time;

    pasted = co_time_diff(cur_time, *prev_past_time) > CO_TIME_MS2SYS(delay_ms);
    if(pasted)
        *prev_past_time = cur_time;

    return pasted;
}

/**
 * @brief time intersection
 *
 * @param[in] time1  time1
 * @param[in] time1_duration  time1_duration
 * @param[in] time2  time2
 * @param[in] time2_duration  time2_duration
 *
 * @return whether is intersection
 **/
STATIC_INLINE bool co_time_intersection(uint32_t time1, uint32_t time1_duration,
                                        uint32_t time2, uint32_t time2_duration)
{
    uint32_t time1_tail = co_time_increase(time1, time1_duration);
    uint32_t time2_tail = co_time_increase(time2, time2_duration);

    return !(co_time_compare_equal_lesser(time2_tail, time1) || co_time_compare_equal_lesser(time1_tail, time2));
}

#ifdef __cplusplus
}
#endif

#endif

/** @} */

