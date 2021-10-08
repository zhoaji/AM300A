/**
 * @file wdt.h
 * @brief watch dog driver
 * @date Thu 04 May 2017 05:37:07 PM CST
 * @author liqiang
 *
 * @defgroup WDT WDT
 * @ingroup PERIPHERAL
 * @brief Watch dog driver
 * @details WDT driver
 *
 * @{
 *
 * @example example_wdt.c
 * This is an example of how to use the wdt
 *
 */

#ifndef __WDT_H__
#define __WDT_H__

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
 * @brief Get watchdog current counter value
 *
 * @return  Current counter value.
 */
uint32_t wdt_time_left(void);

/**
 * @brief watch dog keepalive
 *
 * @return None
 **/
void wdt_keepalive(void);

/**
 * @brief enable watch dog
 *
 * @param[in] timeout  timeout with second, 0 to disable It
 *
 * @note !!! Working stop after sleep !!!
 *
 * @return None
 **/
void wdt_enable(uint32_t timeout);

/// @cond
/**
 * @brief wdt_restore()
 *
 * @return 
 **/
void wdt_restore(void);

/**
 * @brief  wdt keepalive in bl
 **/
void wdt_keepalive_in_bl(void);

/**
 * @brief  wdt disable in bl
 **/
void wdt_disable_in_bl(void);
/// @endcond

#ifdef __cplusplus
}
#endif

#endif

/** @} */

