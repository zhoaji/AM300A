/**
 * @file sysdump.h
 * @brief 
 * @date Tue, Aug 13, 2019  5:20:45 PM
 * @author liqiang
 *
 * @defgroup 
 * @ingroup 
 * @brief 
 * @details 
 *
 * @{
 */

#ifndef __SYSDUMP_H__
#define __SYSDUMP_H__

#ifdef __cplusplus
extern "C"
{ /*}*/
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


/*********************************************************************
 * EXTERN VARIABLES
 */


/*********************************************************************
 * EXTERN FUNCTIONS
 */

/**
 * @brief pmu dump
 *
 * @param[in] printf_dump_func  like printf
 *
 * @note
 *
 * @verbatim
 * The dump infomation looks like this:
 *   [PMU] prevent_status=00000000
 *   [PMU] wakeup_pin=0001000004(cur_level=0001000004 sleep_level=0001000004)
 *   [PMU] pull_up=FFFD7F9CDF(cur_level=FFFD7F9CDC) pull_down=0000000000(cur_level=0000000000) all_cur_level=FFFFFFFFFC
 *   [PMU] clocking: CPU(128MHz) SRAM(000087FF,ULP:32MHz) SF0 SF1 UART0 GPIO ANA
 *
 * Explain:
 * 1st line:
 *   Something (peripheral, user...) prevent system sleep.
 *   bitmask reference @ref pmu_lowpower_peripheral_t
 * 2nd line:
 *   Bitmask of wakeup pin.
 *   If cur_level != sleep_level, system can't sleep.
 * 3rd line:
 *   Inside pull-up and pull-down status.
 *   if pull_up is not equal to it's cur_level, symtem has current leakage in sleep.
 *   if pull_down's cur_level is not equal to 0, system has current leakage in sleep.
 * 4th line:
 *   Working modules.
 *   SRAM: powered block, the more block are powered on, the greater the current consumption during sleep.
 *         reference: @ref pmu_ram_power_on and @ref pmu_ram_power_off_invalid_block
 * @endverbatim
 **/
void pmu_dump(void *printf_dump_func);

/**
 * @brief  sch arb dump
 *
 * @param[in] printf_dump_func  dump function, link printf
 **/
void sch_arb_dump(void *printf_dump_func);

/**
 * @brief  sch alarm dump
 *
 * @param[in] printf_dump_func  dump function, link printf
 **/
void sch_alarm_dump(void *printf_dump_func);

/**
 * @brief timer dump
 *
 * @note If this function called in co_timer expired callback, this expired timer don't dump
 *
 * @param[in] printf_dump_func  dump function, link printf
 **/
void co_timer_dump(void *printf_dump_func);

/**
 * @brief  cfg dump
 *
 * @param[in] printf_dump_func  dump function, link printf
 **/
void cfg_dump(void *printf_dump_func);

/**
 * @brief  sysdump
 **/
__STATIC_INLINE void sysdump(void)
{
    pmu_dump(printf);
    sch_arb_dump(printf);
    sch_alarm_dump(printf);
    co_timer_dump(printf);
}

#ifdef SYSDUMP2BUFFER

#include <stdarg.h>

static char sysdump2buffer_buffer[SYSDUMP2BUFFER];
static int  sysdump2buffer_index = 0;

/**
 * @brief  sysdump printf2buffer
 **/
static int sysdump2buffer_printf(const char *format, ...)
{
    int n;
    va_list args;

    va_start(args, format);
    n = vsnprintf(&sysdump2buffer_buffer[sysdump2buffer_index], SYSDUMP2BUFFER-sysdump2buffer_index, format, args);
    va_end(args);

    sysdump2buffer_index += n;

    return n;
}

/**
 * @brief  sysdump reset
 **/
__STATIC_INLINE void sysdump2buffer_reset(void)
{
    sysdump2buffer_index = 0;
}

/**
 * @brief  sysdump
 *
 * @note (GDB) printf "%s", sysdump2buffer_buffer
 **/
__STATIC_INLINE void sysdump2buffer(void)
{
    pmu_dump(sysdump2buffer_printf);
    sch_arb_dump(sysdump2buffer_printf);
    sch_alarm_dump(sysdump2buffer_printf);
    co_timer_dump(sysdump2buffer_printf);
}

#endif

#ifdef __cplusplus
/*{*/ }
#endif

#endif

/** @} */

