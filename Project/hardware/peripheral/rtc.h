/**
 * @file rtc.h
 * @brief RTC dirver
 * @date Thu 11 May 2017 10:22:38 AM CST
 * @author liqiang
 *
 * @defgroup RTC RTC
 * @ingroup PERIPHERAL
 * @brief Real Time Clock driver
 * @details RTC dirver
 *
 * @{
 *
 * @example example_rtc.c
 * This is an example of how to use the RTC
 *
 */

#ifndef __RTC_H__
#define __RTC_H__

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "peripheral.h"
#include "time.h"

/*********************************************************************
 * MACROS
 */


/*********************************************************************
 * TYPEDEFS
 */

/**
 * @brief rtc event callback
 *
 * @param[in] tm:
 *   - .tm_sec:   seconds after the minute (0-59)
 *   - .tm_min:   minutes after the hour (0-59)
 *   - .tm_hour:  hours since midnight (0-23)
 *   - .tm_mday:  day of the month (1-31)
 *   - .tm_mon:   months since January (0-11)
 *   - .tm_year:  years since 1900
 *   - .tm_wday:  days since Sunday (0-6)
 *   - .tm_yday:  days since January 1 (0-365)
 *   - .tm_isdst: Daylight Saving Time flag
 *
 * @return None
 **/
typedef void (*rtc_callback_t)(const struct tm *tm);

/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */

/**
 * @brief rtc set current time
 *
 * @param[in] tm  time value, NULL: not modify current time
 * @param[in] second_callback  second event callback
 *
 * @return  None
 **/
void rtc_time_set(struct tm *tm, rtc_callback_t second_callback);

/**
 * @brief rtc current time get
 *
 * @param[out] tm  total seconds start form 1970.01.01, 00:00:00
 *
 * @return None
 **/
void rtc_time_get(struct tm *tm);

/**
 * @brief rtc set alarm
 *
 * @param[in] tm  time value, NULL: not modify current alarm time
 * @param[in] alarm_callback  alarm event callback
 *
 * @return None
 **/
void rtc_alarm_set(struct tm *tm, rtc_callback_t alarm_callback);

/**
 * @brief rtc get alarm value
 *
 * @param[out] tm time value
 *
 * @return None
 **/
void rtc_alarm_get(struct tm *tm);

/**
 * @brief rtc initialize
 *
 * @note Work after POWER_SLEEP (with 32k)
 *
 * @return None
 **/
void rtc_init(void);

/**
 * @brief rtc start
 *
 * @return None
 **/
void rtc_start(void);

/**
 * @brief rtc stop
 *
 * @return None
 **/
void rtc_stop(void);

/**
 * @brief rtc store
 *
 * Just for system call before sleep
 *
 * @return None
 **/
void rtc_store(void);

/**
 * @brief rtc restore
 *
 * Just for system call after sleep
 *
 * @return None
 **/
void rtc_restore(void);

/**
 * @brief rtc second irq is enabled
 *
 * @return true or false
 **/
bool rtc_second_irq_is_enabled(void);

/**
 * @brief calculate 32k ppm
 *
 * @note Just for test
 *
 * @return ppm
 **/
int rtc_calc_32k_ppm(void);

#ifdef __cplusplus
}
#endif

#endif

/** @} */

